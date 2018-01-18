#pragma once

#include "stdafx.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        /**
        * \brief Responsible for deletion of AVFrame instances
        */
        struct AVFrameDeleter
        {
            void operator()(AVFrame* frame)
            {
                av_frame_free(&frame);
            }
        };

        class AVLibFrame
        {
        public:
            // Default constructor
            explicit AVLibFrame();
            // Default deconstructor
            virtual ~AVLibFrame() {}
            // Move constructor
            explicit AVLibFrame(AVLibFrame&& other) = delete;
            // Move assignment
            AVLibFrame& operator=(AVLibFrame&& other) = delete;
            // Disabled copy constructor
            explicit AVLibFrame(const AVLibFrame& other) = delete;
            // Disabled copy assignment
            AVLibFrame& operator=(const AVLibFrame& other) = delete;

            /**
             * \brief Returns a reference to the AVFrame held
             * \return The reference to the AVFrame held
             */
            AVFrame& Frame();
            /**
             * \brief Cleans the frame
             */
            void Clean();

#if _DEBUG
            static atomic_int DefaultConstructed;
#endif

        private:
            unique_ptr<AVFrame, AVFrameDeleter> _frame;
        };
    }
}
