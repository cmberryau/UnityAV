using System;
using System.IO;
using System.Runtime.InteropServices;
using UnityEngine;

namespace UnityAV
{
    /// <summary>
    /// Responsible for playing streamed media
    /// </summary>
    public class MediaPlayer : MonoBehaviour
    {
        private const string RTSPPrefix = "rtsp://";

        private const int DefaultWidth = 1024;
        private const int DefaultHeight = 1024;
        private const int InvalidPlayerId = -1;

        /// <summary>
        /// The uri of the media to stream
        /// </summary>
        [Header("Media Properties:")]
        public string Uri;

        /// <summary>
        /// Should the media be looped?
        /// </summary>
        public bool Loop;

        /// <summary>
        /// Should the media play as soon as it's loaded?
        /// </summary>
        public bool AutoPlay;

        /// <summary>
        /// The width of the texture in pixels
        /// </summary>
        [Header("Video Target Properties:")]
        [Range(2, 4096)]
        public int Width = DefaultWidth;

        /// <summary>
        /// The height of the texture in pixels
        /// </summary>
        [Range(2, 4096)]
        public int Height = DefaultHeight;

        /// <summary>
        /// The material to apply any streaming video to
        /// </summary>
        public Material TargetMaterial;

        private Texture2D _targetTexture;
        private int _id = InvalidPlayerId;

        /// <summary>
        /// Gets a media player
        /// </summary>
        /// <param name="uri">The uri to the media to play</param>
        /// <param name="texturePointer">The texture pointer to stream to</param>
        /// <returns>Non-negative unique id of the player, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern int GetPlayer(string uri, IntPtr texturePointer);

        /// <summary>
        /// Releases a media player
        /// </summary>
        /// <param name="id">The id of the player to release</param>
        /// <returns>Non-negative value on success, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern int ReleasePlayer(int id);

        /// <summary>
        /// Evaluates the duration of a media player
        /// </summary>
        /// <param name="id">The player id to evaluate</param>
        /// <returns>The duration of the player in seconds, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern double Duration(int id);

        /// <summary>
        /// Evaluates the current time of a media player from start of play
        /// </summary>
        /// <param name="id">The player id to evaluate</param>
        /// <returns>The current time of the player in seconds from start of play, 
        /// negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern double Time(int id);

        /// <summary>
        /// Begins or resumes a media players playback
        /// </summary>
        /// <param name="id">The player id to evaluate</param>
        /// <returns>Non-negative value on success, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern int Play(int id);

        /// <summary>
        /// Stops a players media playback
        /// </summary>
        /// <param name="id">The player id to stop</param>
        /// <returns>Non-negative value on success, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern int Stop(int id);

        /// <summary>
        /// Seeks a media player
        /// </summary>
        /// <param name="id">The player id to evaluate</param>
        /// <param name="time">The time to seek to</param>
        /// <returns>Non-negative value on success, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern int Seek(int id, double time);

        /// <summary>
        /// Sets a media player to loop or not
        /// </summary>
        /// <param name="id">The player id to set looping for</param>
        /// <param name="loop">True if the player should loop, false if not</param>
        /// <returns>Non-negative value on success, negative on failure</returns>
        [DllImport("UnityAV.Native")]
        private static extern int SetLoop(int id, bool loop);

        /// <summary>
        /// Begins or resumes playback
        /// </summary>
        /// <exception cref="InvalidOperationException"></exception>
        public void Play()
        {
            if (!ValidatePlayerId(_id))
            {
                throw new InvalidOperationException($"{nameof(MediaPlayer)} has no " +
                    "underlying valid native player.");
            }

            Play(_id);
        }

        /// <summary>
        /// Stops playback
        /// </summary>
        /// <exception cref="InvalidOperationException">Thrown if MediaPlayer failed to 
        /// obtain a native player</exception>
        public void Stop()
        {
            if (!ValidatePlayerId(_id))
            {
                throw new InvalidOperationException($"{nameof(MediaPlayer)} has no " +
                    "underlying valid native player.");
            }

            var result = Stop(_id);

            if (result < 0)
            {
                throw new Exception($"Failed to stop with error {result}");
            }
        }

        /// <summary>
        /// Evaluates the duration
        /// </summary>
        /// <returns>The duration in seconds</returns>
        /// <exception cref="InvalidOperationException">Thrown if MediaPlayer failed to 
        /// obtain a native player</exception>
        public double Duration()
        {
            if (!ValidatePlayerId(_id))
            {
                throw new InvalidOperationException($"{nameof(MediaPlayer)} has no " +
                    "underlying valid native player.");
            }

            var result = Duration(_id);

            if (result < 0)
            {
                throw new Exception($"Failed to get duration");
            }

            return result;
        }

        /// <summary>
        /// Evaluates the current time
        /// </summary>
        /// <returns>The time in seconds since playback start</returns>
        /// <exception cref="InvalidOperationException">Thrown if MediaPlayer failed to 
        /// obtain a native player</exception>
        public double Time()
        {
            if (!ValidatePlayerId(_id))
            {
                throw new InvalidOperationException($"{nameof(MediaPlayer)} has no " +
                    "underlying valid native player.");
            }

            var result = Time(_id);

            if (result < 0)
            {
                throw new Exception("Failed to get time");
            }

            return result;
        }

        /// <summary>
        /// Seeks the playback
        /// </summary>
        /// <param name="time">The time to seek to in seconds</param>
        /// <exception cref="InvalidOperationException">Thrown if MediaPlayer failed to 
        /// obtain a native player</exception>
        public void Seek(double time)
        {
            if (!ValidatePlayerId(_id))
            {
                throw new InvalidOperationException($"{nameof(MediaPlayer)} has no " +
                    "underlying valid native player.");
            }

            var result = Seek(_id, time);

            if (result < 0)
            {
                throw new Exception($"Failed to seek with error {result}");
            }
        }

        private void Start()
        {
            NativeInitializer.Initialize(this);

            var uri = string.Copy(Uri);

            if (!Uri.Contains(RTSPPrefix))
            {
                uri = Application.streamingAssetsPath + Path.DirectorySeparatorChar + Uri;

                // ensure the file exists before moving forward
                if (!File.Exists(uri))
                {
                    throw new FileNotFoundException($"{uri} not found.");
                }
            }

            // create the texture to write to
            _targetTexture = new Texture2D(Width, Height, TextureFormat.ARGB32, false)
            {
                filterMode = FilterMode.Bilinear,
                name = Uri
            };

            // register the texture and get the id from the native plugin
            _id = GetPlayer(uri, _targetTexture.GetNativeTexturePtr());

            if (ValidatePlayerId(_id))
            {
                TargetMaterial.mainTexture = _targetTexture;
            }
            else
            {
                throw new Exception($"Failed to create player with error: {_id}");
            }

            if (AutoPlay)
            {
                Play();
            }
        }

        private void OnApplicationQuit()
        {
            if (ValidatePlayerId(_id))
            {
                var result = ReleasePlayer(_id);

                if (result < 0)
                {
                    throw new Exception($"Failed to release player with error: {_id}");
                }
            }

            NativeInitializer.Teardown();
        }

        private void OnApplicationPause(bool pauseStatus)
        {
            if (ValidatePlayerId(_id))
            {
                if (pauseStatus)
                {
                    Stop(_id);
                }
                else
                {
                    Play(_id);
                }                
            }
        }

        private static bool ValidatePlayerId(int id)
        {
            return id >= 0;
        }
    }
}