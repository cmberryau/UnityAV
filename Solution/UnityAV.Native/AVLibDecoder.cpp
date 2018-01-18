#include "stdafx.h"
#include "AVLibDecoder.h"
#include "AVLibVideoDecoder.h"
#include "IAVLibSource.h"

namespace UnityAV
{
    namespace Media
    {
        AVLibDecoder::AVLibDecoder(IAVLibSource& source, unique_ptr<AVCodecContext,
            AVCodecContextDeleter> codecContext, int streamIndex) 
            : _source(source), _codecContext(move(codecContext)), 
            _streamIndex(streamIndex), _successfulDecodes(0), _failedDecodes(0),
            _successfulParses(0), _failedParses(0)
        {
            _timeBase = source.TimeBase(streamIndex);
            _frameRate = source.FrameRate(streamIndex);
            _frameDuration = source.FrameDuration(streamIndex);
        }

        vector<unique_ptr<AVLibDecoder>> AVLibDecoder::Create(IAVLibSource& source,
            const IVideoDescription& requiredVideo)
        {
            // for each stream found by the source, create a decoder
            auto decoders = vector<unique_ptr<AVLibDecoder>>();
            for(auto i = 0; i < source.StreamCount(); ++i)
            {
                auto decoder = Create(source, i, requiredVideo);

                if(decoder)
                {
                    decoders.push_back(move(decoder));
                }
            }

            return decoders;
        }

        void AVLibDecoder::StopDecoding()
        {
            // terminate the running thread
            _stayAlive.clear();
            _continue.notify_all();           

            if (_thread.joinable())
            {
                _thread.join();
            }
            else
            {
                _thread.detach();
            }
        }

        void AVLibDecoder::StartDecoding()
        {
            // start the decoding thread
            _stayAlive.test_and_set();
            _thread = thread(&AVLibDecoder::DecodeThread, this);
        }

        void AVLibDecoder::OnNeedMorePackets()
        {
            ContinueDecoding();
        }

        AVCodecContext& AVLibDecoder::GetCodecContext()
        {
            return *_codecContext;
        }

        bool AVLibDecoder::IsRealtime() const
        {
            return _source.IsRealtime();
        }

        double AVLibDecoder::GetTimeBase() const
        {
            return _timeBase;
        }

        double AVLibDecoder::GetFrameRate() const
        {
            return _frameRate;
        }

        double AVLibDecoder::GetFrameDuration() const
        {
            return _frameDuration;
        }

        unique_ptr<AVLibDecoder> AVLibDecoder::Create(IAVLibSource& source, int streamIndex,
            const IVideoDescription& requiredVideo)
        {
            // we need a codec context
            auto codecContext = unique_ptr<AVCodecContext, AVCodecContextDeleter>(
                avcodec_alloc_context3(nullptr));
            if (!codecContext)
            {
                Debug::LogWarning("AVLibDecoder::Create: Could not find or allocate codec context");
                return nullptr;
            }

            // get the stream
            auto stream = source.Stream(streamIndex);

            // the codec context needs to be filled with parameters from the stream codec parameters
            auto result = avcodec_parameters_to_context(
                codecContext.get(), stream.codecpar);
            if (result < 0)
            {
                Debug::LogWarning("AVLibDecoder::Create: Could not fill codec context with parameters");
                return nullptr;
            }

            // we finally get codec from all of the above
            auto codec = avcodec_find_decoder(codecContext->codec_id);
            if (!codec)
            {
                Debug::LogWarning("AVLibDecoder::Create: Could not find codec");
                return nullptr;
            }
            
            if (!codec)
            {
                Debug::LogWarning("AVLibDecoder::Create: Could not find codec");
                return nullptr;
            }

            // we must open the codec before starting any decoding
            AVDictionary * fakeCodecOptions = nullptr;
            result = avcodec_open2(codecContext.get(), codec, &fakeCodecOptions);
            if (result < 0)
            {
                Debug::LogWarning("AVLibDecoder::Create: Could not open codec");
                return nullptr;
            }

            // validate that the codec type is the media type we're expecting
            if(codecContext->codec_type != source.StreamType(streamIndex))
            {
                Debug::LogWarning("AVLibDecoder::Create: Unexpected media type");
                return nullptr;
            }

            // now that we have everything set, go ahead and create our decoder
            switch (codecContext->codec_type)
            {
            case AVMEDIA_TYPE_UNKNOWN:break;
            case AVMEDIA_TYPE_VIDEO:
                return make_unique<AVLibVideoDecoder>(source, move(codecContext),
                    streamIndex, requiredVideo);
            case AVMEDIA_TYPE_AUDIO:break;
            case AVMEDIA_TYPE_DATA:break;
            case AVMEDIA_TYPE_SUBTITLE:break;
            case AVMEDIA_TYPE_ATTACHMENT:break;
            case AVMEDIA_TYPE_NB:break;
            default:
                break;
            }

            return nullptr;
        }

        void AVLibDecoder::DecodeThread()
        {
            while (_stayAlive.test_and_set())
            {
                auto decode = CanDecodeMore();

                // get packets while the decoder is still ready for more
                while(decode)
                {
                    auto packet = _source.TryGetNext(_streamIndex);

                    if(packet != nullptr)
                    {
                        decode = DecodePacket(*packet);
                        // we're finished with the packet
                        _source.Recycle(move(packet));
                    }
                    else
                    {
                        decode = false;
                    }

                    decode &= CanDecodeMore();
                }

                if(!Wait())
                {
                    break;
                }
            }
        }

        void AVLibDecoder::ContinueDecoding()
        {
            _continue.notify_all();
        }

        bool AVLibDecoder::Wait()
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

        void AVLibDecoder::OnEOF(const AVLibPacket& seekPacket)
        {
            OnEOF();
        }

        void AVLibDecoder::OnSeek(const AVLibPacket& seekPacket)
        {
            OnSeek(seekPacket.SeekTime());
        }

        bool AVLibDecoder::DecodePacket(AVLibPacket& packet)
        {
            auto keepDecoding = true;

            // seek requests require immediate exit from decoding
            if (packet.IsSeekRequest())
            {
                OnSeek(packet);
                keepDecoding = false;
            }
            else
            {
                // when eof comes in, we don't decode any new packets, but we get any
                // queued frames from the decoder and parse them
                if (packet.IsEOF())
                {
                    OnEOF(packet);
                    keepDecoding = false;
                }
                else
                {
                    if (TryDecode(packet))
                    {
                        _successfulDecodes++;
                    }
                    else
                    {
                        _failedDecodes++;
                    }
                }

                // sometimes we can get more than one decoded frame from a single packet
                while (TryGetDecodedFrame(_avLibFrame))
                {
                    if (TryParse(_avLibFrame))
                    {
                        _successfulParses++;
                    }
                    else
                    {
                        _failedParses++;
                    }
                }
            }

            return keepDecoding;
        }
    }
}
