#pragma once

#include "PixelFormat.h"

using namespace std;

namespace Rendering
{
    /**
    * \brief Responsible for writing texture data to a target buffer
    */
    class TextureWriter
    {
    public:
        // Default destructor
        virtual ~TextureWriter();

        /**
        * \brief Reads from the source and stores the read data in a buffer
        * \param source The source to read from
        */
        void Read(const uint8_t* const* source);
        /**
        * \brief Writes the current buffer to the target if the source has changed
        * \param force If true, forces a write regardless if the source has changed
        */
        virtual void Write(bool force) = 0;
        /**
        * \brief The pixel format the texture writer accepts
        * \return The pixel format the texture writer accepts
        */
        PixelFormat Format() const;
        /**
        * \brief The pixel width of the target texture
        */
        int Width() const;
        /**
        * \brief The pixel height of the target texture
        */
        int Height() const;
        /**
        * \brief Evaluates the number of buffers
        * \return The number of buffers
        */
        int BufferCount() const;
        /**
        * \brief Evaluates the buffer's size
        * \param index The buffer index to evaluate
        * \return The size of the buffer
        */
        int BufferSize(int index) const;
        /**
        * \brief Evaluates the buffer's stride
        * \param index The buffer index to evaluate
        * \return The stride of the buffer
        */
        int BufferStride(int index) const;

        /**
        * \brief Creates a new instance of TextureWriter
        * \param interfaces The Unity interfaces to resolve the correct API
        * \param target A raw pointer to a target texture
        * \return Returns an API appopriate TextureWriter
        */
        static unique_ptr<TextureWriter> Create(IUnityInterfaces* const interfaces, void* const target);

    protected:
        // Default constructor
        explicit TextureWriter();

        // state tracking
        atomic_bool Ready;
        atomic_bool Changed;

        // threading
        mutex BufferMutex;

        // required info
        int TargetWidth;
        int TargetHeight;
        PixelFormat TargetFormat;
        vector<int> BufferSizes;
        vector<int> BufferStrides;

        // buffers
        vector<unique_ptr<uint8_t>> Buffers;
        unique_ptr<uint8_t*> BufferArray;
    };
}