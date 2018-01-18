#include "stdafx.h"
#include "UnityConnection.h"

IUnityGraphics * gUnityGraphics = nullptr;
IUnityInterfaces * gUnityInterfaces = nullptr;

// called when there is a graphics device event in unity
extern "C" void UNITY_INTERFACE_API 
    OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    switch (eventType)
    {
        case kUnityGfxDeviceEventInitialize:
            break;
        case kUnityGfxDeviceEventShutdown:
            break;
        case  kUnityGfxDeviceEventBeforeReset:
            break;
        case kUnityGfxDeviceEventAfterReset:
            break;
    }
}

// called when there is a render event in unity
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    OnRenderEvent(int eventId)
{
    ForcePlayersWrite();
}

// called when the plugin is loaded
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API 
    UnityPluginLoad(IUnityInterfaces * interfaces)
{
    // get the unity interfaces
    if (interfaces)
    {
        gUnityInterfaces = interfaces;
        gUnityGraphics = interfaces->Get<IUnityGraphics>();

        // register the device event callback
        if (gUnityGraphics)
        {
            gUnityGraphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
        }
    }

    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);

    // create the map of players if it doesn't already exist
    if (!gPlayers)
    {
        gPlayers = make_unique<vector<unique_ptr<Player>>>();
    }
}

// called when the plugin is about to be unloaded
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API 
    UnityPluginUnload()
{
    // unregister the device event callback
    if (gUnityGraphics)
    {
        gUnityGraphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
    }

    // destroy the map of players if it exists
    if (gPlayers)
    {
        gPlayers.reset();
    }
}

// used to give a function pointer to unity for calling on render events
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API 
    GetRenderEventFunc()
{
    return OnRenderEvent;
}