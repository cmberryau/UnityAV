#include "stdafx.h"

#include "TextureWriter.h"
#include "D3D11TextureWriter.h"

namespace Rendering
{
    TextureWriter::TextureWriter()
    {
        Ready.store(false);
        Changed.store(false);
    }

    TextureWriter::~TextureWriter()
    {

    }

    void TextureWriter::Read(const uint8_t* const* source)
    {
        if(!Ready.load())
        {
            return;
        }

        for(auto i = 0; i < BufferCount(); ++i)
        {
            if(!source[i])
            {
                return;
            }

            memcpy(Buffers[i].get(), source[i], BufferSizes[i]);
        }

        Changed.store(true);
    }

    PixelFormat TextureWriter::Format() const
    {
        return TargetFormat;
    }

    int TextureWriter::Width() const
    {
        return TargetWidth;
    }

    int TextureWriter::Height() const
    {
        return TargetHeight;
    }

    int TextureWriter::BufferCount() const
    {
        return static_cast<int>(Buffers.size());
    }

    int TextureWriter::BufferSize(int index) const
    {
        if (index > BufferCount())
        {
            return -1;
        }

        return BufferSizes[index];
    }

    int TextureWriter::BufferStride(int index) const
    {
        if (index > BufferCount())
        {
            return -1;
        }

        return BufferStrides[index];
    }

    unique_ptr<TextureWriter> TextureWriter::Create(
        IUnityInterfaces* const interfaces, void* const target)
    {
        auto graphics = interfaces->Get<IUnityGraphics>();
        auto renderer = graphics->GetRenderer();

        unique_ptr<TextureWriter> writer;

        switch (renderer)
        {
            case kUnityGfxRendererD3D11:
            {
                auto d3d11 = interfaces->Get<IUnityGraphicsD3D11>();
                writer = make_unique<D3D11TextureWriter>(d3d11->GetDevice(), 
                    static_cast<ID3D11Texture2D*>(target));
            }
            break;

            case kUnityGfxRendererD3D12:
            {
                throw exception("D3D12 not yet supported.");
            }
            break;

            case kUnityGfxRendererOpenGLCore:
            {
                throw exception("OpenGL Core not yet supported.");
            }
            break;

            case kUnityGfxRendererGCM: 
            {
                throw exception("GCM not yet supported.");
            }
            break;

            case kUnityGfxRendererOpenGLES20:
            {
                throw exception("OpenGLES20 not yet supported.");
            }
            break;

            case kUnityGfxRendererOpenGLES30:
            {
                throw exception("OpenGLES30 not yet supported.");
            }
            break;

            case kUnityGfxRendererGXM: 
            {
                throw exception("GXM not yet supported.");
            }
            break;

            case kUnityGfxRendererPS4: 
            {
                throw exception("PS4 not yet supported.");
            }
            break;

            case kUnityGfxRendererXboxOne:
            {
                throw exception("XboxOne not yet supported.");
            }
            break;

            case kUnityGfxRendererMetal:
            {
                throw exception("Metal not yet supported.");
            }
            break;

            case kUnityGfxRendererNull:
            {
                throw exception("Unity renderer is Null.");
            }
            break;

            default:
            {
                throw exception("Unknown API used.");
            }
            break;
        }

        return writer;
    }
}