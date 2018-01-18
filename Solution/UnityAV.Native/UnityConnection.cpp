#include "stdafx.h"
#include "UnityConnection.h"
#include "TextureClient.h"
#include "AVLibPlayer.h"

unique_ptr<vector<unique_ptr<Player>>> gPlayers(nullptr);

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    GetPlayer(const char * path, void * targetTexture)
{
    auto result = -1;

    // validate the path and target are not null
    if (!path || !targetTexture)
    {
        return result;
    }

    // validate that the unity interface is not null
    if(!gUnityInterfaces)
    {
        return result;
    }
  
    // attempt to clean the cache
    TryCleanPlayersCache();

    // create the texture writer
    auto writer = TextureWriter::Create(gUnityInterfaces, targetTexture);
    // create the connector between the writer and the player
    auto connector = make_unique<TextureClient>(move(writer));
    // create the player, taking the connector    
    auto player = Player::Create(string(path), move(connector));

    // add the player to the cache of players
    gPlayers->push_back(move(player));
    // id is simply the index in cache
    return static_cast<int>(gPlayers->size()) - 1;
}

extern "C" void ForcePlayersWrite()
{
    auto count = gPlayers->size();
    for(auto i = 0; i < count; ++i)
    {
        if((*gPlayers)[i])
        {
            (*gPlayers)[i]->Write();
        }
    }
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    ReleasePlayer(int id)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        (*gPlayers)[id].reset();
        result = 1;
    }

    return result;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    ForcePlayerWrite(int id)
{        
    if (ValidatePlayerId(id))
    {
        (*gPlayers)[id]->Write();
    }
}

extern "C" double UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Duration(int id)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        return (*gPlayers)[id]->Duration();
    }

    return result;
}

extern "C" double UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Time(int id)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        return (*gPlayers)[id]->CurrentTime();
    }

    return result;
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Play(int id)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        (*gPlayers)[id]->Play();
        result = 0;
    }

    return result;
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Stop(int id)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        (*gPlayers)[id]->Stop();
        result = 0;
    }

    return result;
}

extern "C" double UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Seek(int id, double time)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        (*gPlayers)[id]->Seek(time);
        result = 0;
    }

    return result;
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    SetLoop(int id, bool loop)
{
    auto result = -1;

    if (ValidatePlayerId(id))
    {
        (*gPlayers)[id]->SetLoop(loop);
        result = 0;
    }

    return result;
}

bool ValidatePlayerId(int id)
{
    if (id < 0)
    {
        return false;
    }

    auto count = gPlayers->size();
    if (count <= 0 || count - 1 < id)
    {
        return false;
    }

    if (!(*gPlayers)[id])
    {
        return false;
    }

    return true;
}

void TryCleanPlayersCache()
{
    if (gPlayers->size() <= 0)
    {
        return;
    }

    auto anyEnabled = false;

    for (auto i = 0; i < gPlayers->size(); ++i)
    {
        if ((*gPlayers)[i])
        {
            anyEnabled = true;
            break;
        }
    }

    if (!anyEnabled)
    {
        gPlayers->clear();
    }
}