#pragma once
#include <SDL.h>

namespace UnityAV
{
    class SDLWindow
    {
    public:
        virtual ~SDLWindow();
        explicit SDLWindow(const string& title, int x, int y, int w, int h);

        Uint32 WindowId() const;
        void HandleEvent(const SDL_Event& event);

        SDL_Surface* Surface();
        SDL_Window* Window();
        SDL_Renderer* Renderer();

    private:
        SDL_Window* _window;
        SDL_Renderer* _renderer;
        Uint32 _windowId;
        SDL_Surface* _surface;
    };
}