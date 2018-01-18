#pragma once
#include "PixelFormat.h"
#include "Frame.h"
#include "IVideoDescription.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Represents a single video frame
         */
        class VideoFrame : public Frame, public IVideoDescription
        {
        public:
            /**
             * \brief Initializes a new VideoFrame instance
             * \param width The width of the VideoFrame
             * \param height The height of the VideoFrame
             * \param format The format of the VideoFrame
             */
            explicit VideoFrame(int width, int height, PixelFormat format);

            // Default destructor
            virtual ~VideoFrame(){}
            // Move constructor
            explicit VideoFrame(VideoFrame&& other) noexcept;
            // Move assignment
            VideoFrame& operator=(VideoFrame&& other) noexcept;
            // Disabled copy constructor
            explicit VideoFrame(const VideoFrame&& other) = delete;
            // Disabled copy assignment
            VideoFrame& operator=(const VideoFrame&& other) = delete;

            PixelFormat Format() const override;
            int Width() const override;
            int Height() const override;
            double Time() const override;
            void Accept(IFrameVisitor& visitor) override;

            /**
             * \brief Sets the time for the frame to be displayed
             * \param time The time at which the frame should be displayed
             */
            void SetTime(double time);
            /**
             * \brief Evaluates the number of buffers
             * \return The number of buffers
             */
            int BufferCount() const;
            /**
             * \brief Evaluates the buffers size
             * \param index The buffer index to evaluate
             * \return The size of the buffer
             */
            int Size(int index) const;
            /**
            * \brief Evaluates the buffers stride
            * \param index The buffer index to evaluate
            * \return The stride of the buffer
            */
            int Stride(int index) const;
            /**
             * \brief Returns the buffer strides
             * \return The buffer strides
             */
            const int* Strides();
            /**
             * \brief Gets the buffers
             * \return The buffers
             */
            uint8_t* const* Buffers();

#if _DEBUG
            static atomic_int DefaultConstructed;
            static atomic_int MoveConstructed;
            static atomic_int MoveAssigned;
#endif

        private:
            void InitializeNone(int width, int height);
            void InitializeYUV420P(int width, int height);
            void InitializeRGBA32(int width, int height);

            static const int kYUV420PDataArraysCount;
            static const int kYUV420PBytesPerPixel;
            static const int kRGBA32DataArraysCount;
            static const int kRGBA32BytesPerPixel;

            // frame info
            int _width, _height;
            PixelFormat _format;
            double _time;

            // buffer info
            vector<int> _sizes;
            vector<int> _strides;
            vector<unique_ptr<uint8_t>> _buffers;
            unique_ptr<uint8_t*> _base;
        };
    }
}
