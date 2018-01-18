#include "stdafx.h"
#include "D3D11TextureWriter.h"

namespace Rendering
{
    const int D3D11TextureWriter::kDefaultRGBA32BPP = 4;

    D3D11TextureWriter::D3D11TextureWriter(ID3D11Device* device, ID3D11Texture2D* target) 
    {
        if (device == nullptr)
        {
            Debug::LogError("Device pointer is null");
            return;
        }

        if (target == nullptr)
        {
            Debug::LogError("Texture pointer is null");
            return;
        }

        // get the target description
        D3D11_TEXTURE2D_DESC desc;
        target->GetDesc(&desc);

        // ensure the texture format is correct
        if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
        {
            Debug::LogError("Texture format is invalid, must be RGBA32");
            return;
        }

        // set the required information
        TargetWidth = desc.Width;
        TargetHeight = desc.Height;
        TargetFormat = PIXEL_FORMAT_RGBA32;

        // create the buffer strides
        BufferStrides.push_back(TargetWidth * kDefaultRGBA32BPP);
        // create the buffer sizes
        BufferSizes.push_back(BufferStrides[0] * TargetHeight);
        // create the buffer
        Buffers.push_back(unique_ptr<uint8_t>(new uint8_t[BufferSizes[0]]));
        // allocate and set the buffer base
        BufferArray = unique_ptr<uint8_t*>(new uint8_t*[1]);
        BufferArray.get()[0] = Buffers[0].get();

        // set all the api resources, don't own them - unity owns them
        _device = device;
        _device->GetImmediateContext(&_context);
        _target = target;
        _context->Release();

        // mark the writer as ready
        Ready.store(true);
    }

    D3D11TextureWriter::~D3D11TextureWriter()
    {
            
    }

    void D3D11TextureWriter::Write(bool force)
    {
        // check state
        if ((force || Changed.load()) && Ready.load())
        {
            _device->GetImmediateContext(&_context);

            // update the texture from our swap buffer
            _context->UpdateSubresource(_target,0, nullptr, Buffers[0].get(),
                BufferStrides[0], 0);
            _context->Release();

            // mark as unchanged
            Changed.store(false);
        }
    }
}