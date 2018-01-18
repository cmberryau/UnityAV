#include "stdafx.h"
#include "SDLWindowWriter.h"

namespace Rendering
{
    const int SDLWindowWriter::kDefaultBPP = 4;

    SDLWindowWriter::SDLWindowWriter(SDLWindow& window) : _window(window)
    {
        // set the required variables
        TargetWidth = window.Surface()->w;
        TargetHeight = window.Surface()->h;
        TargetFormat = PIXEL_FORMAT_RGBA32;

        // create the buffer
        BufferStrides.push_back(TargetWidth * kDefaultBPP);
        BufferSizes.push_back(BufferStrides[0] * TargetHeight);
        Buffers.push_back(unique_ptr<uint8_t>(new uint8_t[BufferSizes[0]]));

        _sdlTexture = SDL_CreateTexture(_window.Renderer(), SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING, TargetWidth, TargetHeight);

        if(_sdlTexture == nullptr)
        {
            Debug::LogError("SDL Could not create a texture");
        }

        Ready.store(true);
    }

    void SDLWindowWriter::Write(bool force)
    {
        // check changed state
        if (force || Changed.load())
        {
            auto surface = _window.Surface();

            SDL_RenderClear(_window.Renderer());
            SDL_UpdateTexture(_sdlTexture, nullptr, Buffers[0].get(), BufferStrides[0]);
            SDL_RenderCopy(_window.Renderer(), _sdlTexture, nullptr, nullptr);            
            SDL_RenderPresent(_window.Renderer());
        }
    }
}