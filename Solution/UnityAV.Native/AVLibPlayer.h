#pragma once
#include "Player.h"
#include "AVLibUtil.h"
#include "AVLibDecoder.h"
#include "AVLibFileSource.h"
#include "IAVLibDecoderVisitor.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        /**
        * \brief Responsible for playing of media using the avlib library
        */
        class AVLibPlayer : public Player, IAVLibDecoderVisitor
        {
        public:
            /**
            * \brief Initializes a new instance of AVLibPlayer
            * \param uri The uri to load the media from
            * \param client The player client
            */
            explicit AVLibPlayer(const string& uri, unique_ptr<IVideoClient> client);
            /**
             * \brief Deconstructs an instance of AVLibPlayer
             */
            virtual ~AVLibPlayer();

            void Play() override;
            void Stop() override;
            bool CanSeek() const override;
            void Seek(double to) override;
            bool CanLoop() const override;
            void SetLoop(bool loop) override;
            bool IsLooping() override;
            double CurrentTime() const override;
            double Duration() const override;
            bool IsPlaying() const override;
            bool IsRealtime() const override;

            void Visit(AVLibVideoDecoder& videoDecoder) override;
        private:
            static const int ConnectRetryMilliseconds;

            static atomic_flag ProcessWideInitialized;
            static void ProcessWideInitialize();
            static int64_t SleepTime(vector<unique_ptr<AVLibDecoder>>& decoders);

            bool EnsureConnection();

            // threading
            void MainThreadMethod();
            thread _thread;
            atomic_flag _stayAlive = ATOMIC_FLAG_INIT;
            mutex _killMutex;
            condition_variable _killCondition;

            // playback and timing info
            atomic_bool _playing;
            atomic_bool _looping;
            int64_t _time;
            int64_t _lastTime;
            int64_t _sleepTime;

            // core
            unique_ptr<IAVLibSource> _source;
            vector<unique_ptr<AVLibDecoder>> _decoders;
        };
    }
}