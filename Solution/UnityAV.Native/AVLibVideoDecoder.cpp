#include "stdafx.h"
#include "AVLibVideoDecoder.h"
#include "IAVLibDecoderVisitor.h"

namespace UnityAV
{
    namespace Media
    {
        const int AVLibVideoDecoder::kDefaultVideoFrameQueueSize = 25;

        AVLibVideoDecoder::AVLibVideoDecoder(IAVLibSource& source, unique_ptr
            <AVCodecContext, AVCodecContextDeleter> codecContext, int streamIndex,
            const IVideoDescription& targetDesc) 
            : AVLibDecoder(source, move(codecContext), streamIndex),
            _parsedFrames(kDefaultVideoFrameQueueSize), 
            _readyFrames(kDefaultVideoFrameQueueSize),
            _completeFramesQueueThreshold(kDefaultVideoFrameQueueSize / 2),
            _sourceWidth(GetCodecContext().width), _sourceHeight(GetCodecContext().height), 
            _targetWidth(targetDesc.Width()), _targetHeight(targetDesc.Height()),
            _targetFormat(targetDesc.Format()), _lastFrame(nullptr), _seekRequestTime(0),
            _givenFrames(0), _returnedFrames(0), _recycledFrames(0)
        {
            _seekRequest.test_and_set();

            // the sws context performs the pixel format transform
            _swsContext = unique_ptr<SwsContext, SwsContextDeleter>(sws_getContext(
                _sourceWidth, _sourceHeight, GetCodecContext().pix_fmt,
                _targetWidth, _targetHeight, ToAVPixelFormat(_targetFormat), SWS_BILINEAR,
                nullptr, nullptr, nullptr));

            // begin decoding
            StartDecoding();
        }

        AVLibVideoDecoder::~AVLibVideoDecoder()
        {
            // terminate all decoding before deconstruction
            StopDecoding();
        }

        unique_ptr<VideoFrame> AVLibVideoDecoder::TryGetNext(double time)
        {
            if(_parsedFrames.Count() <= _completeFramesQueueThreshold)
            {
                OnNeedMorePackets();
            }

            // if the decoder is realtime, just pop out the next frame
            if(IsRealtime())
            {
                return _parsedFrames.Pop();
            }

            auto seekRequest = !_seekRequest.test_and_set();

            // get a new frame if we don't have the last or we've had a seek request
            if(_lastFrame == nullptr || seekRequest)
            {
                _lastFrame = _parsedFrames.Pop();
            }

            // we have a frame to evaluate
            if (_lastFrame != nullptr)
            {
                if(seekRequest)
                {
                    auto seekDiff = _seekRequestTime - _lastFrame->Time();
                }

                // check if the frame is eof
                auto eof = _lastFrame->IsEOF();
                // is the frame behind our current time?
                auto behind = time >= _lastFrame->Time();

                // check out if we can eject some late frames
                if (!eof && behind)
                {
                    // check if the queue has any more frames
                    auto available = !_parsedFrames.Empty();
                    unique_ptr<VideoFrame> nextFrame;

                    // while we're still behind and can still get more, keep checking
                    while(behind && available && !eof)
                    {
                        // check for the next frame
                        nextFrame = _parsedFrames.Pop();
                        available = nextFrame != nullptr;

                        // we got a next frame
                        if(available)
                        {
                            // if the next frame is eof, return early
                            if (nextFrame->IsEOF())
                            {
                                // recycle our current frame
                                Recycle(move(_lastFrame));
                                // next frame becomes current frame
                                _lastFrame = move(nextFrame);
                                // mark that we've reached eof
                                eof = true;
                            }
                            else
                            {
                                behind = time >= nextFrame->Time();

                                // the next frame is behind
                                if (behind)
                                {
                                    // recycle our current frame
                                    Recycle(move(_lastFrame));
                                    // next frame becomes current frame
                                    _lastFrame = move(nextFrame);
                                }
                            }
                        }
                    }

                    // if we left because of eof, return the eof frame
                    if(eof)
                    {
                        return move(_lastFrame);
                    }

                    // if we left because nextFrame was no longer behind
                    if(!behind)
                    {                  
                        auto swapFrame = move(_lastFrame);
                        _lastFrame = move(nextFrame);

                        return move(swapFrame);
                    }

                    _givenFrames++;
                    return move(_lastFrame);
                }
            }
            else
            {
                // keep seek request set until we get the next frame after the request is made
                if(seekRequest)
                {
                    _seekRequest.clear();
                }
            }

            return nullptr;
        }

        void AVLibVideoDecoder::Recycle(unique_ptr<VideoFrame> videoFrame)
        {
            if(videoFrame == nullptr)
            {
                return;
            }

            videoFrame->OnRecycle();
            _readyFrames.Push(move(videoFrame));
            _returnedFrames++;
        }

        void AVLibVideoDecoder::Accept(IAVLibDecoderVisitor& visitor)
        {
            visitor.Visit(*this);
        }

        bool AVLibVideoDecoder::CanDecodeMore()
        {
            return !_parsedFrames.Full();
        }

        bool AVLibVideoDecoder::TryDecode(AVLibPacket& packet)
        {
            auto result = avcodec_send_packet(&GetCodecContext(), &packet.Packet());

            // if result < 0, there was a decoding failure
            if (result < 0)
            {
                Debug::LogError("AVLibVideoDecoder::TryDecode: Could not send packet to decoder");
                return false;
            }
            
            return true;
        }

        bool AVLibVideoDecoder::TryGetDecodedFrame(AVLibFrame& frame)
        {
            auto result = avcodec_receive_frame(&GetCodecContext(), &frame.Frame());

            if(result < 0)
            {
                // just means the decoder has reached EOF
                if (result == AVERROR_EOF)
                {
                    // push an EOF video frame onto the queue
                    auto eofFrame = GetRecycledFrame();
                    eofFrame->SetAsEOF();
                    _parsedFrames.Push(move(eofFrame));
                    
                    return false;
                }

                // just means the decoder needs more packets
                if (result == AVERROR(EAGAIN))
                {
                    return false;
                }

                // otherwise it's an actual decoding failure
                Debug::LogError("AVLibVideoDecoder::TryDecode: Decoder failed to decode input");
                return false;
            }

            return true;
        }

        bool AVLibVideoDecoder::TryParse(AVLibFrame& frame)
        {
            // set the pts of the frame
            frame.Frame().pts = av_frame_get_best_effort_timestamp(&frame.Frame());
            auto time = frame.Frame().pts * GetTimeBase();

            auto videoFrame = GetRecycledFrame();
            videoFrame->SetTime(time);
            
            auto result = sws_scale(_swsContext.get(), frame.Frame().data, 
                frame.Frame().linesize, 0, _sourceHeight, videoFrame->Buffers(),
                videoFrame->Strides());

            // frame must be cleaned after usage
            frame.Clean();

            if(result != _targetHeight)
            {
                return false;
            }

            _parsedFrames.Push(move(videoFrame));

            return true;
        }

        void AVLibVideoDecoder::FlushQueue()
        {
            // flush the buffers
            avcodec_flush_buffers(&GetCodecContext());
            // flush the queue
            _parsedFrames.Flush();
        }

        unique_ptr<VideoFrame> AVLibVideoDecoder::GetRecycledFrame()
        {
            auto frame = _readyFrames.Pop();

            if (frame == nullptr)
            {
                frame = make_unique<VideoFrame>(_targetWidth, _targetHeight, _targetFormat);
            }
            else
            {
                _recycledFrames++;
            }

            return move(frame);
        }

        void AVLibVideoDecoder::OnEOF()
        {
            // sending a nullptr to the decoder notifies it that it's eof
            auto result = avcodec_send_packet(&GetCodecContext(), nullptr);
        }

        void AVLibVideoDecoder::OnSeek(double to)
        {
            // flush the queue
            FlushQueue();

            // cache the time and mark that there is a request
            _seekRequestTime = to;
            _seekRequest.clear();            
        }
    }
}
