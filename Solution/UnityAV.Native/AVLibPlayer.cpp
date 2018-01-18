#include "stdafx.h"
#include "AVLibPlayer.h"
#include "AVLibVideoDecoder.h"
#include "AVLibRTSPSource.h"

namespace UnityAV
{
    namespace Media
    {
        const int AVLibPlayer::ConnectRetryMilliseconds = 2500;
        atomic_flag AVLibPlayer::ProcessWideInitialized = ATOMIC_FLAG_INIT;

        AVLibPlayer::AVLibPlayer(const string& uri, unique_ptr<IVideoClient> client)
            : Player(uri, move(client)), _time(0), _lastTime(0), _sleepTime(0)
        {
            // initialize avlib across the process
            ProcessWideInitialize();

            // create an appropriate source
            if (uri.find(RTSPPrefix) != string::npos)
            {
                _source = make_unique<AVLibRTSPSource>(uri);
            }
            else
            {
                _source = make_unique<AVLibFileSource>(uri);
            }
            
            // connect to streams
            _source->Connect();

            _playing.store(false);
            _looping.store(false);

            // start the main thread
            _stayAlive.test_and_set();
            _thread = thread(&AVLibPlayer::MainThreadMethod, this);
        }

        AVLibPlayer::~AVLibPlayer()
        {
            // terminate the running thread first, it accesses both decoders and sources
            _stayAlive.clear();
            _killCondition.notify_all();

            if (_thread.joinable())
            {
                _thread.join();
            }
            else
            {
                _thread.detach();
            }

            // decoders must go next, they access the source
            _decoders.clear();
            _source.reset();
        }

        void AVLibPlayer::Play()
        {
            if(!_playing.load())
            {
                _lastTime = av_gettime_relative();
            }
            
            _playing.store(true);
        }

        void AVLibPlayer::Stop()
        {
            _playing.store(false);
        }

        bool AVLibPlayer::CanSeek() const
        {
            return _source->CanSeek();
        }

        void AVLibPlayer::Seek(double to)
        {
            if (!_playing.load() || !_source->CanSeek())
            {
                return;
            }

            // clamp it
            auto duration = Duration();
            if (to > duration)
            {
                to = duration;
            }
            else if (to < 0)
            {
                to = 0;
            }
            
            _source->Seek(CurrentTime(), to);
            _time = static_cast<int64_t>(to * kSecondToMicrosecond);
        }

        bool AVLibPlayer::CanLoop() const
        {
            return _source->CanSeek();
        }

        void AVLibPlayer::SetLoop(bool loop)
        {
            if(!CanLoop())
            {
                return;
            }

            _looping.store(loop);
        }

        bool AVLibPlayer::IsLooping()
        {
            return _looping.load();
        }

        double AVLibPlayer::CurrentTime() const
        {
            return _time * kMicrosecondToSecond;
        }

        double AVLibPlayer::Duration() const    
        {
            return _source->Duration();
        }

        bool AVLibPlayer::IsPlaying() const
        {
            return _playing.load();
        }

        bool AVLibPlayer::IsRealtime() const
        {
            return _source->IsRealtime();
        }

        void AVLibPlayer::Visit(AVLibVideoDecoder& videoDecoder)
        {
            auto currentTime = CurrentTime();
            auto frame = videoDecoder.TryGetNext(currentTime);

            if(frame != nullptr)
            {
                if(frame->IsEOF())
                {
                    if(_looping.load())
                    {
                        Seek(0);
                    }
                    else
                    {
                        _playing.store(false);
                    }
                }
                else
                {
                    OnFrameReady(*frame);
                    videoDecoder.Recycle(move(frame));
                }
            }
        }

        void AVLibPlayer::ProcessWideInitialize()
        {
            if (!ProcessWideInitialized.test_and_set())
            {
                av_register_all();
                av_log_set_level(AV_LOG_VERBOSE);
            }
        }

        int64_t AVLibPlayer::SleepTime(vector<unique_ptr<AVLibDecoder>>& decoders)
        {
            auto result = INT64_MAX;

            for(auto i = 0; i < decoders.size(); ++i)
            {
                // find the smallest frame duration
                if(decoders[i]->GetFrameDuration() * kSecondToMicrosecond < result)
                {
                    result = static_cast<int64_t>(decoders[i]->GetFrameDuration() *
                                kSecondToMicrosecond);
                }
            }

            return result / 2;
        }

        void AVLibPlayer::MainThreadMethod()
        {
            // ensure we're connected
            auto stayAlive = EnsureConnection();

            // if we're staying alive, create decoders and get on with it
            if (stayAlive)
            {
                // only create the decoders after source is connected
                _decoders = AVLibDecoder::Create(*_source, RequiredVideoFrame());

                // evaluate the sleeptime if not realtime
                if (!_source->IsRealtime())
                {
                    _sleepTime = SleepTime(_decoders);
                }
                else
                {
                    // temporary hack for realtime sources
                    _sleepTime = 50000;
                }
            }

            while (stayAlive)
            {
                // ensure we're still connected
                stayAlive &= EnsureConnection();

                if (_playing.load() && stayAlive)
                {
                    // update the time vars
                    auto d = av_gettime_relative() - _lastTime;
                    _time += d;
                    _lastTime = av_gettime_relative();

                    for (auto i = 0; i < _decoders.size(); ++i)
                    {
                        _decoders[i]->Accept(*this);
                    }
                }

                // sleep the thread for a small amount of time
                this_thread::sleep_for(chrono::microseconds(_sleepTime));
                stayAlive &= _stayAlive.test_and_set();
            }
        }

        bool AVLibPlayer::EnsureConnection()
        {
            auto stayAlive = _stayAlive.test_and_set();

            // while the source is not connected, retry
            while (!_source->IsConnected() && stayAlive)
            {
                _source->Connect();

                // wait and use kill condition for early exit
                auto killLock = unique_lock<mutex>(_killMutex);
                _killCondition.wait_for(killLock, chrono::milliseconds(ConnectRetryMilliseconds));
                stayAlive &= _stayAlive.test_and_set();
            }

            return stayAlive;
        }
    }
}
