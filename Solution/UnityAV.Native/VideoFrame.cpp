#include "stdafx.h"
#include "VideoFrame.h"

namespace UnityAV
{
    namespace Media
    {
        const int VideoFrame::kYUV420PDataArraysCount = 3;
        const int VideoFrame::kYUV420PBytesPerPixel = 1;
        const int VideoFrame::kRGBA32DataArraysCount = 1;
        const int VideoFrame::kRGBA32BytesPerPixel = 4;

#if _DEBUG
        atomic_int VideoFrame::DefaultConstructed;
        atomic_int VideoFrame::MoveConstructed;
        atomic_int VideoFrame::MoveAssigned;
#endif

        VideoFrame::VideoFrame(int width, int height, PixelFormat format) 
            : _width(width), _height(height), _format(format)
        {
            switch(format)
            {
            case PIXEL_FORMAT_YUV420P:
                InitializeYUV420P(width, height);
                break;
            case PIXEL_FORMAT_RGBA32:
                InitializeRGBA32(width, height);
                break;
            default:
                InitializeNone(width, height);
                break;
            }

#if _DEBUG
            ++DefaultConstructed;
#endif
        }

        VideoFrame::VideoFrame(VideoFrame&& other) noexcept
            : Frame(move(other)), 
            _width(other._width), 
            _height(other._height), 
            _format(other._format), 
            _time(other._time),
            _sizes(move(other._sizes)),
            _strides(move(other._strides)),
            _buffers(move(other._buffers)), 
            _base(move(other._base))
        {
#if _DEBUG
            ++MoveConstructed;
#endif
        }

        VideoFrame& VideoFrame::operator=(VideoFrame&& other) noexcept
        {
            Frame::operator=(move(other));

            _width = other._width;
            _height = other._height;
            _format = other._format;
            _time = other._time;
            _sizes = move(other._sizes);
            _strides = move(other._strides);
            _buffers = move(other._buffers);
            _base = move(other._base);
#if _DEBUG
            ++MoveAssigned;
#endif
            return *this;
        }

        PixelFormat VideoFrame::Format() const
        {
            return _format;
        }

        int VideoFrame::Width() const
        {
            return _width;
        }

        int VideoFrame::Height() const
        {
            return _height;
        }

        double VideoFrame::Time() const
        {
            return _time;
        }

        void VideoFrame::SetTime(double time)
        {
            _time = time;
        }

        int VideoFrame::BufferCount() const
        {
            return static_cast<int>(_sizes.size());
        }

        int VideoFrame::Size(int index) const
        {
            if(index > BufferCount())
            {
                return -1;
            }

            return _sizes[index];
        }

        int VideoFrame::Stride(int index) const
        {
            return _strides[index];
        }

        const int* VideoFrame::Strides()
        {
            return static_cast<int*>(_strides.data());
        }

        uint8_t* const* VideoFrame::Buffers()
        {
            return static_cast<uint8_t* const*>(_base.get());
        }

        void VideoFrame::Accept(IFrameVisitor& visitor)
        {
            visitor.Visit(*this);
        }

        void VideoFrame::InitializeNone(int width, int height)
        {
            
        }

        void VideoFrame::InitializeYUV420P(int width, int height)
        {
            auto widthHalved = width / 2;

            _base = unique_ptr<uint8_t*>(new uint8_t*[kYUV420PDataArraysCount]);

            _sizes.push_back(width * height * kYUV420PBytesPerPixel);
            _buffers.push_back(unique_ptr<uint8_t>(new uint8_t[_sizes[0]]));
            _strides.push_back(width * kYUV420PBytesPerPixel);
            _base.get()[0] = _buffers[0].get();

            _sizes.push_back(widthHalved * height * kYUV420PBytesPerPixel);
            _buffers.push_back(unique_ptr<uint8_t>(new uint8_t[_sizes[1]]));
            _strides.push_back(widthHalved * kYUV420PBytesPerPixel);
            _base.get()[1] = _buffers[1].get();

            _sizes.push_back(widthHalved * height  * kYUV420PBytesPerPixel);
            _buffers.push_back(unique_ptr<uint8_t>(new uint8_t[_sizes[2]]));
            _strides.push_back(widthHalved * kYUV420PBytesPerPixel);
            _base.get()[2] = _buffers[2].get();
        }

        void VideoFrame::InitializeRGBA32(int width, int height)
        {
            _base = unique_ptr<uint8_t*>(new uint8_t*[kRGBA32DataArraysCount]);

            _sizes.push_back(width * height * kRGBA32BytesPerPixel);
            _buffers.push_back(unique_ptr<uint8_t>(new uint8_t[_sizes[0]]));
            _strides.push_back(width * kRGBA32BytesPerPixel);
            _base.get()[0] = _buffers[0].get();
        }
    }
}