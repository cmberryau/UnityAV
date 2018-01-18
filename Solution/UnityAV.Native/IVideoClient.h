#pragma once
#include "VideoFrame.h"
#include "IVideoDescription.h"

namespace UnityAV
{
    namespace Media
    {
        class VideoFrame;

        /**
        * \brief Interface for clients to register with a Player for callbacks
        */
        class IVideoClient : public IVideoDescription
        {
        public:
            // Default destructor
            virtual ~IVideoClient() {};

            /**
             * \brief The format the IVideoFrameClient accepts
             * \return The format the IVideoFrameClient accepts
             */
            virtual PixelFormat Format() const override = 0;
            /**
             * \brief The pixel width the IVideoFrameClient accepts
             * \return The pixel width the IVideoFrameClient accepts
             */
            virtual int Width() const override = 0;
            /**
             * \brief The pixel height the IVideoFrameClient accepts
             * \return The pixel height the IVideoFrameClient accepts
             */
            virtual int Height() const override = 0;

            /**
            * \brief Called when a video frame is ready for display
            * \param frame The frame that is ready for display
            */
            virtual void OnFrameReady(VideoFrame& frame) = 0;            
            /**
             * \brief Writes the video client
             */
            virtual void Write() = 0;

        protected:
            // Default constructor
            explicit IVideoClient() {}
        };
    }
}
