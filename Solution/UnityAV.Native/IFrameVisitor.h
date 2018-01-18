#pragma once

namespace UnityAV
{
    namespace Media
    {
        class VideoFrame;

        /**
         * \brief An interface to visit concrete types of Frame
         */
        class IFrameVisitor
        {        
        public:
            /**
             * \brief Visits the VideoFrame instance
             * \param frame The VideoFrame instance to visit
             */
            virtual void Visit(VideoFrame& frame) = 0;

        protected:
            virtual ~IFrameVisitor() {};
        };
    }
}