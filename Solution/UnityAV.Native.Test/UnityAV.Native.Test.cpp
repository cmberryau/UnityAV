// UnityAV.Native.Test.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <SDL.h>
#include "SDLWindow.h"
#include "Player.h"
#include "Rendering/SDLWindowWriter.h"
#include "TextureClient.h"
#include "Rendering/NullTextureWriter.h"

mutex gMutex;

IUnityGraphics * gUnityGraphics;
IUnityInterfaces * gUnityInterfaces;

using namespace UnityAV;
using namespace Media;
using namespace Rendering;

void RunTestPlayer(const string& uri, int x, int y, int w, int h)
{
    // lock for setup, otherwise access errors can occur
    auto setupLock = unique_lock<mutex>(gMutex);
    auto window = make_unique<SDLWindow>(uri, x, y, w, h);
}

vector<unique_ptr<Player>> CreatePlayers(const vector<string>& uris, const vector<unique_ptr<SDLWindow>>& windows)
{
    vector<unique_ptr<Player>> players;

    for(auto i = 0; i < uris.size(); ++i)
    {
        auto windowWriter = unique_ptr<TextureWriter>(make_unique<SDLWindowWriter>(*windows[i]));
        auto connector = unique_ptr<IVideoClient>(make_unique<TextureClient>(move(windowWriter)));
        players.push_back(Player::Create(uris[i], move(connector)));

        if(players[i] == nullptr)
        {
            players.clear();
            return players;
        }
    }

    return players;
}

vector<unique_ptr<SDLWindow>> CreateWindows(const vector<string>& uris, int w, int h, int xp, int yp)
{
    vector<unique_ptr<SDLWindow>> windows;

    for (auto i = 0; i < uris.size(); ++i)
    {
        windows.push_back(make_unique<SDLWindow>(uris[i], xp * (i+1) + i * w, yp, w, h));

        if(windows[i] == nullptr)
        {
            windows.clear();
            return windows;
        }
    }

    return windows;
}

void CreatePlayersAndRun(const vector<string>& uris, int w, int h, int xp, int yp, 
    bool loopPlayers)
{
    auto windows = CreateWindows(uris, w, h, xp, yp);
    if(windows.size() <= 0)
    {
        return;
    }

    auto players = CreatePlayers(uris, windows);
    if(players.size() <= 0)
    {
        return;
    }

    SDL_Event e;
    auto quit = false;
    auto windowCount = windows.size();
    auto playerCount = players.size();

    for (auto i = 0; i < playerCount; ++i)
    {
        players[i]->Play();

        if(loopPlayers)
        {
            players[i]->SetLoop(true);
        }
    }

    while(!quit)
    {
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_WINDOWEVENT)
            {
                for(auto i = 0; i < windowCount; ++i)
                {
                    if(windows[i]->WindowId() == e.window.windowID)
                    {
                        if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                        {
                            windows.erase(windows.begin() + i);
                            --windowCount;

                            players.erase(players.begin() + i);
                            --playerCount;

                            if(windowCount == 0)
                            {
                                quit = true;
                            }
                        }
                        else
                        {
                            windows[i]->HandleEvent(e);
                        }

                        break;
                    }
                }
            }
        }

        for (auto i = 0; i < playerCount; ++i)
        {
            players[i]->Write();
        }

        // hack to drastically reduce cpu time
        this_thread::sleep_for(chrono::microseconds(50000));
    }
}

void RunTest(const vector<string> & uris, bool loopPlayers = false)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        Debug::Log("SDL Failed to initialize");
        return;
    }

    auto width = 640;
    auto height = 400;
    auto xPadding = 30;
    auto yPadding = 60;

    CreatePlayersAndRun(uris, width, height, xPadding, yPadding, loopPlayers);

    SDL_Quit();
}

void AllocationTest()
{
    auto windowWriter = unique_ptr<TextureWriter>(make_unique<NullTextureWriter>(1280, 800));
    auto connector = unique_ptr<IVideoClient>(make_unique<TextureClient>(move(windowWriter)));
    Player::Create("rtsp://localhost:554/stream0", move(connector));
}

void RTSPTest(bool multiple)
{
    vector<string> uris;

    uris.push_back("rtsp://localhost:554/stream0");

    if(multiple)
    {
        uris.push_back("rtsp://localhost:555/stream1");
        uris.push_back("rtsp://localhost:556/stream2");
        uris.push_back("rtsp://localhost:557/stream3");
    }

    RunTest(uris);
}

void FileTest(bool loopPlayers = false)
{
    vector<string> uris;

    uris.push_back("../TestFiles/SampleVideo_1280x720_10mb.mp4");

    RunTest(uris, loopPlayers);
}

void FileTestInvalidUri()
{
    vector<string> uris;

    uris.push_back("invaliduri.invaliduri");

    RunTest(uris);
}

int main(int argc, char** argv)
{
    _CrtMemState memState;
    _CrtMemCheckpoint(&memState);

    Debug::Initialize(false);

    FileTest(true);
    //RTSPTest(true);

    Debug::Teardown();

    _CrtMemDumpAllObjectsSince(&memState);

    return 0;
}
