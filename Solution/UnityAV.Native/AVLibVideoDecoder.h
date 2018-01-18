#pragma once
#include "AVLibDecoder.h"
#include "AVLibFrame.h"
#include "FixedSizeQueue.h"
#include "VideoFrame.h"

namespace UnityAV
{
    namespace Media
    {
        class IAVLibDecoderVisitor;

        /**
        * \brief Responsible for decoding of an avlib video stream
        */
        class AVLibVideoDecoder : public AVLibDecoder
        {
        public:
            /**
             * \brief Initializes a new instance of AVLibVideoDecoder
             * \param source The source to get packets from
             * \param codecContext The codec context of the stream
             * \param streamIndex The stream index
             * \param targetDesc The target video description
             */
            explicit AVLibVideoDecoder(IAVLibSource& source, unique_ptr<AVCodecContext,
                AVCodecContextDeleter> codecContext, int streamIndex, 
                const IVideoDescription& targetDesc);
            virtual ~AVLibVideoDecoder();

            /**
             * \brief Attempts to get a VideoFrame from the decoder, and optionally 
             * recycle it after you're finished with it (see Recycle)
             * \param time The time for the next VideoFrame
             * \return The next video frame, nullptr otherwise
             */
            unique_ptr<VideoFrame> TryGetNext(double time);

            /**
             * \brief Returns a VideoFrame to the decoder
             * \param videoFrame The VideoFrame to return
             */
            void Recycle(unique_ptr<VideoFrame> videoFrame);

            void Accept(IAVLibDecoderVisitor & visitor) override;

        protected:
            bool CanDecodeMore() override;
            bool TryDecode(AVLibPacket& packet) override;
            bool TryGetDecodedFrame(AVLibFrame& frame) override;
            bool TryParse(AVLibFrame& frame) override;
            void OnEOF() override;
            void OnSeek(double to) override;

        private:
            static const int kDefaultVideoFrameQueueSize;
            
            void FlushQueue();
            unique_ptr<VideoFrame> GetRecycledFrame();

            // core
            unique_ptr<SwsContext, SwsContextDeleter> _swsContext;
            FixedSizeQueue<unique_ptr<VideoFrame>> _parsedFrames;
            FixedSizeQueue<unique_ptr<VideoFrame>> _readyFrames;
            int _completeFramesQueueThreshold;
            int _sourceWidth, _sourceHeight;
            int _targetWidth, _targetHeight;
            PixelFormat _targetFormat;
            unique_ptr<VideoFrame> _lastFrame;

            // seeking
            atomic_flag _seekRequest = ATOMIC_FLAG_INIT;
            double _seekRequestTime;

            // meta
            int _givenFrames, _returnedFrames, _recycledFrames;
        };
    }
}
