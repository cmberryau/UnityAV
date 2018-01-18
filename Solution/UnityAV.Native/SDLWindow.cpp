#include "stdafx.h"
#include "SDLWindow.h"

namespace UnityAV
{
    SDLWindow::~SDLWindow()
    {
        if (_surface != nullptr)
        {
            SDL_FreeSurface(_surface);
            _surface = nullptr;
        }

        if(_window != nullptr)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }

        if(_renderer != nullptr)
        {
            SDL_DestroyRenderer(_renderer);
            _renderer = nullptr;
        }
        
    }

    SDLWindow::SDLWindow(const string& title, int x, int y, int w, int h)
        : _window(nullptr), _windowId(0), _surface(nullptr)
    {
        _window = SDL_CreateWindow(title.c_str(), x, y, w, h, SDL_WINDOW_SHOWN);
        if (_window == nullptr)
        {
            Debug::Log("SDL Failed to create a window");
            return;
        }

        _renderer = SDL_CreateRenderer(_window, -1, 0);
        if(_renderer == nullptr)
        {
            Debug::Log("SDL Failed to create a renderer");
            return;
        }

        _windowId = SDL_GetWindowID(_window);

        _surface = SDL_GetWindowSurface(_window);
        if (_surface == nullptr)
        {
            Debug::Log("SDL Failed to create a surface");
            return;
        }
    }

    Uint32 SDLWindow::WindowId() const
    {
        return _windowId;
    }

    void SDLWindow::HandleEvent(const SDL_Event& event)
    {

    }

    SDL_Surface* SDLWindow::Surface()
    {
        return _surface;
    }

    SDL_Window* SDLWindow::Window()
    {
        return _window;
    }

    SDL_Renderer* SDLWindow::Renderer()
    {
        return _renderer;
    }
}
