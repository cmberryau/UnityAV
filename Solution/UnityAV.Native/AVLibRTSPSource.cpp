#include "stdafx.h"
#include "AVLibRTSPSource.h"

namespace UnityAV
{
    namespace Media
    {
        const AVStream AVLibRTSPSource::EmptyStream = AVStream();
        const int AVLibRTSPSource::DefaultPacketQueueSize = 10;

        mutex AVLibRTSPSource::ProcessWideMutex;
        atomic_flag AVLibRTSPSource::ProcessWideInitialized = ATOMIC_FLAG_INIT;
        int AVLibRTSPSource::InstanceCount = 0;
        thread AVLibRTSPSource::TaskSchedulerThread;
        volatile char AVLibRTSPSource::WatchVariable = 0;
        unique_ptr<BasicTaskScheduler> AVLibRTSPSource::TaskScheduler;
        unique_ptr<BasicUsageEnvironment, BasicUsageEnvrionmentDeleter> AVLibRTSPSource::Environment;

        AVLibRTSPSource::~AVLibRTSPSource()
        {
            // destroy client before process wide teardown
            _rtspClient.reset();

            // teardown live555 across the process
            ProcessWideTeardown();
        }

        AVLibRTSPSource::AVLibRTSPSource(const string& uri) : _recycler(DefaultPacketQueueSize),
            _uri(uri)
        {
            // initialize live555 across the process
            ProcessWideInitialize();

            // fill in the fake codec parameters
            auto codecParameters = new AVCodecParameters();
            codecParameters->width = 1280;
            codecParameters->height = 800;
            codecParameters->format = AV_PIX_FMT_YUV420P;
            codecParameters->codec_type = AVMEDIA_TYPE_VIDEO;
            codecParameters->codec_id = AV_CODEC_ID_MJPEG;

            // create the fake stream
            auto stream = AVStream();
            stream.index = 0;
            stream.nb_frames = 0;
            stream.codecpar = codecParameters;

            _streams.push_back(stream);
            _streamTypes.push_back(AVMEDIA_TYPE_VIDEO);
        }

        double AVLibRTSPSource::Duration() const
        {
            Debug::LogWarning("AVLibRTSPSource::Duration - AVLibRTSPSource is realtime and duration cannot be determined");
            // is realtime, no duration known
            return -1;
        }

        void AVLibRTSPSource::Connect()
        {
            if(_rtspClient == nullptr)
            {
                // create and connect our rtsp client
                _rtspClient = unique_ptr<Live555RTSPClient,
                    Live555RTSPClient::Live555RTSPClientDeleter>(new Live555RTSPClient(
                        *Environment, _uri));
            }
            else if (_rtspClient->ConnectionDropped())
            {
                // reset client when connection has failed or been dropped
                _rtspClient = unique_ptr<Live555RTSPClient,
                    Live555RTSPClient::Live555RTSPClientDeleter>(new Live555RTSPClient(
                        *Environment, _uri));
            }

            if (_rtspClient != nullptr)
            {
                _rtspClient->Connect();
            }
            else
            {
                Debug::LogError("AVLibRTSPSource::Connect - failed to create a rtsp client instance");
            }
        }

        bool AVLibRTSPSource::IsConnected() const
        {
            if (_rtspClient == nullptr)
            {
                return false;
            }

            return _rtspClient->IsConnected();
        }

        int AVLibRTSPSource::StreamCount() const
        {
            return static_cast<int>(_streams.size());
        }

        double AVLibRTSPSource::TimeBase(int streamIndex) const
        {
            Debug::LogWarning("AVLibRTSPSource::TimeBase - AVLibRTSPSource is realtime and timebase cannot be determined");
            // is realtime, no pts used
            return -1;
        }

        double AVLibRTSPSource::FrameRate(int streamIndex) const
        {
            Debug::LogWarning("AVLibRTSPSource::FrameRate - AVLibRTSPSource is realtime and framerate cannot be determined");
            // is realtime, frames are shown as they come in
            return -1;
        }

        double AVLibRTSPSource::FrameDuration(int streamIndex) const
        {
            Debug::LogWarning("AVLibRTSPSource::FrameDuration - AVLibRTSPSource is realtime and frames have no known duration");
            // is realtime, frames exist until the next one arrives
            return -1;
        }

        AVMediaType AVLibRTSPSource::StreamType(int streamIndex) const
        {
            if(streamIndex >= _streamTypes.size())
            {
                Debug::LogError("AVLibRTSPSource::StreamType - streamIndex was out of range");
                return AVMEDIA_TYPE_UNKNOWN;
            }

            if(!IsConnected())
            {
                return AVMEDIA_TYPE_UNKNOWN;
            }

            return _streamTypes[streamIndex];
        }

        const AVStream& AVLibRTSPSource::Stream(int streamIndex) const
        {
            if (streamIndex >= _streamTypes.size())
            {
                Debug::LogError("AVLibRTSPSource::Stream - streamIndex was out of range");
                return EmptyStream;
            }

            if(!IsConnected())
            {
                return EmptyStream;
            }

            return _streams[streamIndex];
        }

        bool AVLibRTSPSource::IsRealtime() const
        {
            return true;
        }

        bool AVLibRTSPSource::CanSeek() const
        {
            return false;
        }

        void AVLibRTSPSource::Seek(double from, double to)
        {
            Debug::LogWarning("AVLibRTSPSource::Seek - AVLibRTSPSource is realtime and cannot seek");
            // do nothing, can't seek
        }

        unique_ptr<AVLibPacket> AVLibRTSPSource::TryGetNext(int streamIndex)
        {
            if (streamIndex >= _streamTypes.size())
            {
                Debug::LogError("AVLibRTSPSource::TryGetNext - streamIndex was out of range");
                return nullptr;
            }

            if(!IsConnected())
            {
                return nullptr;
            }

            auto live555Packet = _rtspClient->TryGetNext(streamIndex);

            if (live555Packet != nullptr)
            {
                auto avlibPacket = _recycler.GetPacket();

                // point the packets data to the live555's packets data
                avlibPacket->Packet().data = live555Packet->Data();
                avlibPacket->Packet().size = live555Packet->DataSize();

                _rtspClient->Recycle(move(live555Packet), streamIndex);

                return avlibPacket;
            }

            return nullptr;
        }

        void AVLibRTSPSource::Recycle(unique_ptr<AVLibPacket> packet)
        {
            if(packet == nullptr)
            {
                Debug::LogError("AVLibRTSPSource::Recycle - packet was nullptr");
                return;
            }

            _recycler.Recycle(move(packet));
        }

        void AVLibRTSPSource::ProcessWideInitialize()
        {
            auto lock = unique_lock<mutex>(ProcessWideMutex);

            if (++InstanceCount <= 1)
            {
                // the task scheduler should not exit the eventloop, zero does the job
                WatchVariable = 0;

                TaskScheduler = unique_ptr<BasicTaskScheduler>(BasicTaskScheduler::createNew());
                Environment = unique_ptr<BasicUsageEnvironment,
                    BasicUsageEnvrionmentDeleter>(BasicUsageEnvironment::createNew(*TaskScheduler));
                TaskSchedulerThread = thread(SchedulerThreadMethod);
            }
        }

        void AVLibRTSPSource::ProcessWideTeardown()
        {
            auto lock = unique_lock<mutex>(ProcessWideMutex);

            if (--InstanceCount <= 0)
            {
                // notify the task scheduler to exit the eventloop, nonzero does the job
                WatchVariable = -1;

                // enter the event loop thread to ensure it exits before moving on
                if (TaskSchedulerThread.joinable())
                {
                    TaskSchedulerThread.join();
                }

                Environment.reset();
                TaskScheduler.reset();
            }
        }

        void AVLibRTSPSource::SchedulerThreadMethod()
        {
            // enter the event loop
            Environment->taskScheduler().doEventLoop(&WatchVariable);
        }
    }
}