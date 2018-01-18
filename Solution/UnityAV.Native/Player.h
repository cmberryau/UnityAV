#pragma once
#include "IVideoClient.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Responsible for playing media
         */
        class Player : public IFrameVisitor
        {
        public:            
            /**
             * \brief Deconstructs an instance of Player
             */
            virtual ~Player(){}
            // Disabled move constructor
            Player(Player&& other) = delete;
            // Disabled move assignment
            Player& operator=(Player&& other) = delete;
            // Disabled copy constructor
            Player(const Player&& other) = delete;
            // Disabled copy assignment
            Player& operator=(const Player&& other) = delete;

            /**
             * \brief Creates a player instance
             * \param uri The uri to evaluate for creating the instance
             * \return The player instance or a nullptr on failure
             */
            static unique_ptr<Player> Create(const string& uri, unique_ptr<IVideoClient> client);

            /**
            * \brief Starts or resumes playback of the media
            */
            virtual void Play() = 0;
            /**
            * \brief Stops playback of the media
            */
            virtual void Stop() = 0;
            /**
             * \brief Evaluates if the player can seek
             * \return True if the player can seek, false if not
             */
            virtual bool CanSeek() const = 0;
            /**
            * \brief Stops playback of the media
            */
            virtual void Seek(double to) = 0;
            /**
             * \brief Evaluates if the player can loop
             * \return True if the player can loop, false if not
             */
            virtual bool CanLoop() const = 0;
            /**
             * \brief Sets the playback to loop or not
             * \param loop True will loop, false will not
             */
            virtual void SetLoop(bool loop) = 0;
            /**
             * \brief Gets whether the playback is being looped or not
             * \return True if looping, false otherwise
             */
            virtual bool IsLooping() = 0;
            /**
            * \brief Evaluates if the player is currently playing
            * \return True if the player is currently playing, false if not
            */
            virtual bool IsPlaying() const = 0;
            /**
            * \brief Evaluates the current time of the player in seconds
            * \return The current time of the player in seconds
            */
            virtual double CurrentTime() const = 0;
            /**
            * \brief Evaluates the total duration of the media in seconds
            * \return The total duration of the media
            */
            virtual double Duration() const = 0;
            /**
             * \brief Evaluates if the player is a realtime player
             * \return True if the player is realtime, false otherwise
             */
            virtual bool IsRealtime() const = 0;
            /**
             * \brief Writes the playing media to all clients 
             */
            void Write();

            void Visit(VideoFrame& frame) override;

        protected:
            static const string RTSPPrefix;
            static const string FilePrefix;

            /**
             * \brief Intitializes a new instance of Player
             * \param uri The uri to load the media from
             * \param client The video client
             */
            explicit Player(const string& uri, unique_ptr<IVideoClient> client);

            /**
             * \brief Called by concrete players when a frame is ready
             * \param frame The frame that is ready
             */
            void OnFrameReady(Frame& frame);
            /**
             * \brief Evaluates the required video format
             * \return Returns the required video format
             */
            const IVideoDescription& RequiredVideoFrame() const;

        private:
            void OnDisplayVideoFrame(VideoFrame& frame);

            unique_ptr<IVideoClient> _videoClient;
            string _uri;
        };
    }
}