#include "stdafx.h"
#include "AVLibFileSource.h"

namespace UnityAV
{
    namespace Media
    {
        const int AVLibFileSource::DefaultVideoPacketQueueSize = 50;
        const int AVLibFileSource::DefaultAudioPacketQueueSize = 100;
        const int AVLibFileSource::DefaultSubtitlePacketQueueSize = 50;
        const double AVLibFileSource::SeekThreshold = 0.5;

        AVLibFileSource::AVLibFileSource(string uri) : 
            _recycler(DefaultVideoPacketQueueSize + DefaultAudioPacketQueueSize +
            DefaultSubtitlePacketQueueSize), _lowestDTS(INT64_MAX),_lowestPTS(INT64_MAX),
            _seekStreamIndex(0), _seekTimeBase(0), _seekToTime(0),_seekFromTime(0), 
            _failedPackets(0), _successfulPackets(0),_skippedPackets(0)
        {
            // allocate a format context 
            _formatContext = unique_ptr<AVFormatContext, AVFormatContextDeleter>(
                avformat_alloc_context());

            if (!_formatContext)
            {
                Debug::LogError("AVLibPlayer::Load: Unable to allocate AVFormatContext");
                return;
            }

            // set up the blocking call interrupt info
            _formatContext->interrupt_callback.callback = BlockingIOInterruptCallback;
            _formatContext->interrupt_callback.opaque = this;

            // open the file and create the packet queues
            OpenFile(*_formatContext, uri);
            Initialize();

            _eof.store(false);            
            _duration = _formatContext->duration * kMicrosecondToSecond;
            _seekRequest.test_and_set();

            // start the reading thread
            _stayAlive.test_and_set();
            _thread = thread(&AVLibFileSource::ReadThread, this);
        }

        AVLibFileSource::~AVLibFileSource()
        {
            // terminate the running thread
            _stayAlive.clear();
            _continue.notify_all();
            if(_thread.joinable())
            {
                _thread.join();
            }
            else
            {
                _thread.detach();
            }
        }

        double AVLibFileSource::Duration() const
        {
            return _duration;
        }

        void AVLibFileSource::Connect()
        {
            // AVLibFileSource connects in constructor, do nothing
        }

        bool AVLibFileSource::IsConnected() const
        {
            // AVLibFileSource blocks until all are resolved in constructor
            return true;
        }

        int AVLibFileSource::StreamCount() const
        {
            return static_cast<int>(_streamIndices.size());
        }

        AVMediaType AVLibFileSource::StreamType(int streamIndex) const
        {
            return _streamTypes[streamIndex];
        }

        const AVStream& AVLibFileSource::Stream(int streamIndex) const
        {
            return _streams[streamIndex];
        }

        double AVLibFileSource::TimeBase(int streamIndex) const
        {
            return _timeBases[streamIndex];
        }

        double AVLibFileSource::FrameRate(int streamIndex) const
        {
            return _frameRates[streamIndex];
        }

        double AVLibFileSource::FrameDuration(int streamIndex) const
        {
            return static_cast<double>(1) / FrameRate(streamIndex);
        }

        bool AVLibFileSource::IsRealtime() const
        {
            return false;
        }

        bool AVLibFileSource::CanSeek() const
        {
            return true;
        }

        void AVLibFileSource::Seek(double from, double to)
        {
            _seekFromTime = from;
            _seekToTime = to;
            _seekRequest.clear();

            Continue();
        }

        unique_ptr<AVLibPacket> AVLibFileSource::TryGetNext(int streamIndex)
        {            
            auto& streamQueue = _packetQueues[streamIndex];
            
            if (streamQueue.Count() <= _queueThresholds[streamIndex])
            {
                Continue();
            }

            auto packet = streamQueue.Pop();
            
            return move(packet);
        }

        void AVLibFileSource::Recycle(unique_ptr<AVLibPacket> packet)
        {
            _recycler.Recycle(move(packet));
        }

        int AVLibFileSource::BlockingIOInterruptCallback(void* source)
        {
            return 0;
        }

        bool AVLibFileSource::OpenFile(AVFormatContext& formatContext, string uri)
        {
            auto rawFormatContext = &formatContext;
            auto result = avformat_open_input(&rawFormatContext, 
                uri.c_str(), formatContext.iformat, nullptr);

            if (result < 0)
            {
                auto errbuf = unique_ptr<char>(new char[1024]);
                av_strerror(result, errbuf.get(), 1024);
                Debug::LogError("AVLibPlayer::Load: Failed to open input: %s", *errbuf);
                return false;
            }

            result = avformat_find_stream_info(&formatContext, nullptr);

            if (result < 0)
            {
                auto errbuf = unique_ptr<char>(new char[1024]);
                av_strerror(result, errbuf.get(), 1024);
                Debug::LogError("AVLibPlayer::Load: avformat_find_stream_info failed to open input: %s", *errbuf);
                return false;
            }

            return true;
        }

        void AVLibFileSource::Initialize()
        {
            auto bestIndices = BestStreamIndices(*_formatContext);
            auto highestIndex = 0;
            for(auto it = bestIndices.begin(); it != bestIndices.end(); ++it)
            {
                auto mediaType = (*it).first;
                auto streamIndex = (*it).second;

                if(streamIndex > highestIndex)
                {
                    highestIndex = streamIndex;
                }

                _streamIndices.push_back(streamIndex);
                _streamTypes.push_back(mediaType);           
                _streams.push_back(*_formatContext->streams[streamIndex]);
                _timeBases.push_back(av_q2d(_formatContext->streams[streamIndex]->time_base));
                _frameRates.push_back(av_q2d(av_guess_frame_rate(_formatContext.get(), 
                    _formatContext->streams[streamIndex], nullptr)));

                switch (mediaType)
                {
                case AVMEDIA_TYPE_UNKNOWN: break;
                case AVMEDIA_TYPE_VIDEO:
                    // todo: hack video preferencing for now
                    _seekStreamIndex = streamIndex;
                    _seekTimeBase = av_q2d(
                        _formatContext->streams[_seekStreamIndex]->time_base);
                    _activeQueues.push_back(true);
                    _queueThresholds.push_back(DefaultVideoPacketQueueSize / 2);
                    _packetQueues.push_back(FixedSizeQueue<unique_ptr<AVLibPacket>>(
                        DefaultVideoPacketQueueSize));
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    // todo: hack video preferencing for now
                    _activeQueues.push_back(false);
                    _queueThresholds.push_back(DefaultAudioPacketQueueSize / 2);
                    //_packetQueues.push_back(FixedSizeQueue<unique_ptr<AVLibPacket>>(
                    //    DefaultAudioPacketQueueSize));
                    break;
                case AVMEDIA_TYPE_DATA: break;
                case AVMEDIA_TYPE_SUBTITLE:
                    // todo: hack video preferencing for now
                    _activeQueues.push_back(false);
                    _queueThresholds.push_back(DefaultSubtitlePacketQueueSize / 2);
                    //_packetQueues.push_back(FixedSizeQueue<unique_ptr<AVLibPacket>>(
                    //    DefaultSubtitlePacketQueueSize));
                    break;
                case AVMEDIA_TYPE_ATTACHMENT: break;
                case AVMEDIA_TYPE_NB: break;
                default: break;
                }
            }

            // stream indices begin at 0
            for(auto i = 0; i < highestIndex + 1; ++i)
            {
                auto found = false;
                for (auto j = 0; j < _streamIndices.size() && !found; ++j)
                {
                    if(_streamIndices[j] == i)
                    {
                        _streamIndicesToInternal.push_back(j);
                        found = true;
                    }
                }

                if(!found)
                {
                    _streamIndicesToInternal.push_back(-1);
                }
            }
        }

        void AVLibFileSource::ReadThread()
        {
            while(_stayAlive.test_and_set())
            {
                auto seekRequest = !_seekRequest.test_and_set();
                auto read = !AnyQueueFull() && !seekRequest && !_eof;

                // until any queue is full, error forces out or a seek
                while(read)
                {
                    auto packet = _recycler.GetPacket();
                    auto result = av_read_frame(_formatContext.get(), &packet->Packet());

                    if(result < 0)
                    {
                        read = HandleReadError(result);
                    } 
                    else
                    {
                        UpdateMeta(*packet);
                        read = QueuePacket(move(packet));
                    }

                    // if seek req, then stop reading
                    read &= !((seekRequest = !_seekRequest.test_and_set()));
                }

                if(seekRequest)
                {
                    OnSeekRequest();
                }

                if(!Wait())
                {
                    break;
                }
            }
        }

        void AVLibFileSource::Continue()
        {
            _continue.notify_all();
        }

        bool AVLibFileSource::Wait()
        {
            // check alive flag again before waiting
            if (_stayAlive.test_and_set())
            {
                // wait for external notification
                auto lock = unique_lock<mutex>(_continueMutex);
                _continue.wait(lock);
            }
            else
            {
                // return false if we should kill the read thread
                return false;
            }

            // return true after wait is over
            return true;
        }

        void AVLibFileSource::OnEOF()
        {
            _eof.store(true);

            // add a eof packet to every active queue
            for (auto i = 0; i < _packetQueues.size(); ++i)
            {                
                if (_activeQueues[i])
                {
                    auto eofPacket = _recycler.GetPacket();
                    eofPacket->SetAsEOF();
                    _packetQueues[i].Push(move(eofPacket));
                }
            }
        }

        void AVLibFileSource::OnSeekRequest()
        {
            auto to = _seekToTime;
            auto from = _seekFromTime;

            // clamp both times
            if (to > _duration)
            {
                to = _duration;
            }
            else if (to < 0)
            {
                to = 0;
            }

            if (from > _duration)
            {
                from = _duration;
            }
            else if (from < 0)
            {
                from = 0;
            }
            
            auto diff = to - from;

            if(abs(diff) > SeekThreshold)
            {
                auto flags = 0;

                // going forwards in time
                if(diff > 0)
                {
                    flags |= AVSEEK_FLAG_ANY;
                }
                // going backwards in time
                else
                {
                    flags |= AVSEEK_FLAG_BACKWARD;
                }

                auto timestamp = static_cast<int64_t>(to / _seekTimeBase);
                auto result = av_seek_frame(_formatContext.get(), _seekStreamIndex, 
                    timestamp, flags);

                if(result < 0)
                {
                    Debug::LogError("Failed to seek to timestamp %d", timestamp);
                }
                else
                {
                    _eof.store(false);
                    FlushQueues();
                    InjectSeekPackets(to);
                }
            }
        }

        bool AVLibFileSource::HandleReadError(int error)
        {
            if (error == AVERROR_EOF)
            {                
                OnEOF();
                return false;
            }

            // unexpected failure            
            Debug::Log("AVLibFileSource::HandleReadError - av_read_frame failed with %d", error);
            _failedPackets++;

            return false;
        }

        void AVLibFileSource::UpdateMeta(AVLibPacket& packet)
        {
            if (packet.Packet().pts < _lowestPTS)
            {
                _lowestPTS = packet.Packet().pts;
            }

            if (packet.Packet().dts < _lowestDTS)
            {
                _lowestDTS = packet.Packet().dts;
            }
        }

        bool AVLibFileSource::QueuePacket(unique_ptr<AVLibPacket> packet)
        {
            // find which stream the packet belongs to
            auto streamIndex = packet->Packet().stream_index;
            auto internalIndex = _streamIndicesToInternal[streamIndex];

            // only store the packet if it's an active stream
            if (_activeQueues[internalIndex])
            {
                auto& packetQueue = _packetQueues[internalIndex];
                packetQueue.Push(move(packet));

                // returns true if packet queue is now full
                if (packetQueue.Full())
                {
                    return false;
                }

                _successfulPackets++;
            }
            else
            {
                // recycle the packet if its not going to an active queue
                Recycle(move(packet));
                _skippedPackets++;
            }

            return true;
        }

        void AVLibFileSource::FlushQueues()
        {
            for (auto i = 0; i < _packetQueues.size(); ++i)
            {
                if (_activeQueues[i])
                {
                    _packetQueues[i].Flush();
                }
            }
        }

        void AVLibFileSource::InjectSeekPackets(double time)
        {
            for (auto i = 0; i < _packetQueues.size(); ++i)
            {
                if (_activeQueues[i])
                {
                    auto packet = _recycler.GetPacket();
                    packet->SetSeekRequest(time);
                    _packetQueues[i].Push(move(packet));
                }
            }
        }

        bool AVLibFileSource::AnyQueueFull() const
        {
            // evaluate if any queue is full
            for (auto i = 0; i < _packetQueues.size(); ++i)
            {
                if (_activeQueues[i] && _packetQueues[i].Full())
                {
                    return true;
                }
            }

            return false;
        }
    }
}