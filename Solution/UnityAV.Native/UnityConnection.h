#pragma once

#include "Player.h"
#include "Rendering/TextureWriter.h"

using namespace UnityAV;
using namespace Media;
using namespace Rendering;

extern IUnityGraphics * gUnityGraphics;
extern IUnityInterfaces * gUnityInterfaces;

/**
 * \brief The cache of media players currently registered in Unity3D
 */
extern unique_ptr<vector<unique_ptr<Player>>> gPlayers;

/**
 * \brief Forces all media players to write to their targets
 */
extern "C" void ForcePlayersWrite();

/**
* \brief Gets a media player
* \param path The uri of the media to play
* \param targetTexture The raw pointer to a target texture
* \return Returns Non-negative unique id of the player, negative on failure
*/
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    GetPlayer(const char * path, void * targetTexture);

/**
* \brief Releases a media player
* \param id The unique id of the player to release
* \return Returns Non-negative value on success, negative on failure
*/
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    ReleasePlayer(int id);

/**
* \brief Forces a media player to write to it's targets
* \param id The unique id of the player
* \return Returns Non-negative value on success, negative on failure
*/
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    ForcePlayerWrite(int id);

/**
 * \brief Evaluates the duration of a media player
 * \param id The player id to evaluate
 * \return The duration of the player in seconds, negative on failure
 */
extern "C" double UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Duration(int id);

/**
 * \brief Evaluates the current time of a media player from start of play
 * \param id The player id to evaluate
 * \return The current time of the player in seconds from start of play, negative on failure
 */
extern "C" double UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Time(int id);

/**
 * \brief Begins or resumes a media players playback
 * \param id The player id to play
 * \return Returns Non-negative value on success, negative on failure
 */
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Play(int id);

/**
* \brief Stops a players media playback
* \param id The player id to stop
* \return Returns Non-negative value on success, negative on failure
*/
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Stop(int id);

/**
* \brief Seeks a media player
* \param id The player id to seek
* \param time The time to seek to
* \return Returns Non-negative value on success, negative on failure
*/
extern "C" double UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    Seek(int id, double time);

/**
* \brief Sets a media player to loop or not
* \param id The player id to set looping for
* \param loop True if the player should loop, false if not
* \return Returns Non-negative value on success, negative on failure
*/
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    SetLoop(int id, bool loop);

/**
 * \brief Validates the media players unique id
 * \param id The unique id to validate
 * \return True when the id is valid, false otherwise
 */
bool ValidatePlayerId(int id);

/**
 * \brief Clears all media players if all are disabled
 */
void TryCleanPlayersCache();