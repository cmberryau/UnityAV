#pragma once

#include "TextureWriter.h"
#include "SDLWindow.h"

using namespace UnityAV;

namespace Rendering
{
    class SDLWindowWriter : public TextureWriter
    {
    public:
        // Default deconstructor
        virtual ~SDLWindowWriter(){}
        /**
        * \brief Initializes a new instance of SDLTextureWriter
        * \param window The SDL window to write to
        */
        explicit SDLWindowWriter(SDLWindow& window);

        void Write(bool force) override;

    private:
        static const int kDefaultBPP;
        SDLWindow& _window;
        SDL_Texture* _sdlTexture;
    };
}
