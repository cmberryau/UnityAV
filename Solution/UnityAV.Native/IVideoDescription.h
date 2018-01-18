#pragma once
#include "PixelFormat.h"

namespace UnityAV
{
    namespace Media
    {
        /**
        * \brief Common interface for describing video frames
        */
        class IVideoDescription
        {
        public:
            // Default destructor
            virtual ~IVideoDescription() {};

            /**
            * \brief The format of the IVideoFrame
            * \return The format of the IVideoFrame
            */
            virtual PixelFormat Format() const = 0;
            /**
            * \brief The pixel width of the IVideoFrame
            * \return The pixel width of the IVideoFrame
            */
            virtual int Width() const = 0;
            /**
            * \brief The pixel height of the IVideoFrame
            * \return The pixel height of the IVideoFrame
            */
            virtual int Height() const = 0;

            /**
             * \brief Evaluates if two IVideoFrame instances are the same
             * \param a The first IVideoFrame
             * \param b The second IVideoFrame
             * \return True when they are the same, false otherwise
             */
            static bool Compatible(IVideoDescription& a, IVideoDescription& b)
            {
                return a.Width() == b.Width() &&
                       a.Height() == b.Height() &&
                       a.Format() == b.Format();
            }

        protected:
            // Default constructor
            explicit IVideoDescription() {}
        };
    }
}
