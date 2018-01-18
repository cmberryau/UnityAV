#include "stdafx.h"
#include "Live555RTSPClient.h"

namespace UnityAV
{
    namespace Media
    {
        const int Live555RTSPClient::DefaultVerbosityLevel = 0;
        const string Live555RTSPClient::DefaultApplicationName = "Live5555RTSPClient";
        const portNumBits Live555RTSPClient::DefaultTunnelOverHttpPort = 0;
        const int Live555RTSPClient::DefaultSocketNumToServer = -1;
        const int Live555RTSPClient::DefaultTimeoutCheckBeginSeconds = 3;
        const int Live555RTSPClient::DefaultTimeoutSeconds = 2;

        Live555RTSPClient::~Live555RTSPClient()
        {            
            delete _subSessionIterator;

            if (_session != nullptr)
            {
                _session->envir().taskScheduler().unscheduleDelayedTask(_streamTimerTask);
                Medium::close(_session);
            }
        }

        Live555RTSPClient::Live555RTSPClient(UsageEnvironment& usageEnvironment, const string& uri)
            : RTSPClient(usageEnvironment, uri.c_str(), DefaultVerbosityLevel,
            DefaultApplicationName.c_str(), DefaultTunnelOverHttpPort,
            DefaultSocketNumToServer), _subSessionIterator(nullptr), _session(nullptr),
            _subsession(nullptr), _streamTimerTask(nullptr), _duration(0),
            _connected(false), _connecting(false), _connectionFailed(false), 
            _connectionDropped(false), _checkingConnection(false), _checkStartTime(-1), 
            _lastServerActivityTime(-1), _beginTimeoutCheck(DefaultTimeoutCheckBeginSeconds),
            _timeout(DefaultTimeoutSeconds), _packetCount(0), _uri(uri)
        {
        }

        void Live555RTSPClient::Connect()
        {
            if(!_connecting && !_connected)
            {
                OnConnectionAttempt();
                sendDescribeCommand(ContinueAfterDescribe);
            }
        }

        bool Live555RTSPClient::IsConnected() const
        {
            return _connected;
        }

        bool Live555RTSPClient::ConnectionFailed() const
        {
            return _connectionFailed;
        }

        bool Live555RTSPClient::ConnectionDropped() const        
        {
            return _connectionDropped;
        }

        int Live555RTSPClient::SubsessionCount() const
        {
            return static_cast<int>(_subsessions.size());
        }

        unique_ptr<Live555Packet> Live555RTSPClient::TryGetNext(int subsessionIndex)
        {
            if(subsessionIndex >= SubsessionCount())
            {
                Debug::LogError("Live555RTSPClient::TryGetNext - subsessionIndex was out of range");
                return nullptr;
            }

            if(!IsConnected())
            {
                return nullptr;
            }

            auto packet = _packetSinks[subsessionIndex]->TryGetNext();

            time_t currentTime;
            time(&currentTime);

            if(packet == nullptr && _packetCount > 0)
            {
                if(!_checkingConnection)
                {
                    auto delta = currentTime - _lastServerActivityTime;

                    if (delta > _beginTimeoutCheck)
                    {
                        _checkingConnection = true;
                        time(&_checkStartTime);
                        sendDescribeCommand(ContinueAfterTimeoutDescribe);
                    }
                }
                else
                {
                    auto delta = currentTime - _checkStartTime;

                    if(delta > _timeout)
                    {
                        OnConnectionDropped();
                        _checkingConnection = false;
                    }
                }
            }
            else
            {
                _lastServerActivityTime = currentTime;
                ++_packetCount;
            }

            return packet;
        }

        void Live555RTSPClient::Recycle(unique_ptr<Live555Packet> packet, int subsessionIndex)
        {
            if (subsessionIndex >= SubsessionCount())
            {
                Debug::LogError("Live555RTSPClient::Recycle - subsessionIndex was out of range");
                return;
            }

            _packetSinks[subsessionIndex]->Recycle(move(packet));
        }

        void Live555RTSPClient::ContinueAfterDescribe(RTSPClient* rawClient, int resultCode,
            char* rawResultString)
        {
            // capture the result string immediately
            auto resultString = unique_ptr<char>(rawResultString);

            if(rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::ContinueAfterDescribe, rawClient was nullptr");
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);

            if (resultCode != 0)
            {
                client->OnConnectionFailed();
                return;
            }

            // check the media session
            auto mediaSession = MediaSession::createNew(client->envir(), rawResultString);

            // has no session
            if (mediaSession == nullptr)
            {
                client->OnConnectionFailed();
                return;
            }

            // has no subsession
            if (!mediaSession->hasSubsessions())
            {
                client->OnConnectionFailed();
                return;
            }

            // set up the subsessions
            auto subsessionIterator = new MediaSubsessionIterator(*mediaSession);            
            client->_session = mediaSession;
            client->_subSessionIterator = subsessionIterator;
            
            SetupNextSubsession(client);
        }

        void Live555RTSPClient::ContinueAfterTimeoutDescribe(RTSPClient* rawClient,
            int resultCode, char* rawResultString)
        {
            // capture the result string immediately
            auto resultstring = unique_ptr<char>(rawResultString);

            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::ContinueAfterDescribe, rawClient was nullptr");
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);

            // make sure the client was actually checking it's connection
            if(!client->_checkingConnection)
            {
                return;
            }

            // for some reason, adtf server throws a 404 when paused
            if(resultCode == 404)
            {
                client->_checkingConnection = false;
                time(&client->_lastServerActivityTime);
            }

            // otherwise the server is refusing / not responding
        }

        void Live555RTSPClient::ContinueAfterSetup(RTSPClient* rawClient, int resultCode,
            char* rawResultString)
        {
            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::ContinueAfterSetup, rawClient was nullptr");
                return;
            }

            // capture the result string immediately
            auto resultString = unique_ptr<char>(rawResultString);
            auto client = static_cast<Live555RTSPClient*>(rawClient);

            if (resultCode != 0)
            {
                client->OnConnectionFailed();
                return;
            }

            // cache a pointer to the subsession on the client
            client->_subsessions.push_back(client->_subsession);
            // create the subsession's sink
            auto sink = new Live555PacketSink(rawClient->envir(), rawClient->url());            
            if (sink == nullptr)
            {
                client->OnConnectionFailed();
                return;
            }

            // cache a pointer to the sink on the client
            client->_packetSinks.push_back(sink);

            client->_subsession->sink = sink;
            sink->startPlaying(*(client->_subsession->readSource()), OnSubsessionEnded, 
                client->_subsession);

            // set up a rtcp "BYE" handler if needed
            if(client->_subsession->rtcpInstance() != nullptr)
            {
                client->_subsession->rtcpInstance()->setByeHandler(OnSubsessionBye, 
                    client->_subsession);
            }

            // set up the next subsession
            SetupNextSubsession(rawClient);
        }

        void Live555RTSPClient::ContinueAfterPlay(RTSPClient* rawClient, int resultCode,
            char* rawResultString)
        {
            // capture the result string immediately
            auto resultString = unique_ptr<char>(rawResultString);

            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::ContinueAfterPlay, rawClient was nullptr");
                return;
            }

            if(resultCode != 0)
            {
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);
            client->OnConnectionSuccess();

            if(client->_duration > 0)
            {
                Debug::Log("Live555RTSPClient::ContinueAfterPlay - duration > 0");
            }
        }

        void Live555RTSPClient::ContinueAfterTeardown(RTSPClient* rawClient, int resultCode,
            char* rawResultString)
        {
            // capture the result string immediately
            auto resultString = unique_ptr<char>(rawResultString);

            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::ContinueAfterTeardown, rawClient was nullptr");
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);
            client->OnConnectionClosed();

            // just close the client now
            Medium::close(rawClient);            
        }

        void Live555RTSPClient::SetupNextSubsession(RTSPClient* rawClient)
        {
            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::SetupNextSubsession, rawClient was nullptr");
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);
            client->_subsession = client->_subSessionIterator->next();

            if (client->_subsession != nullptr)
            {
                if (!client->_subsession->initiate())
                {
                    SetupNextSubsession(rawClient);
                }
                else
                {
                    client->sendSetupCommand(*client->_subsession, ContinueAfterSetup, 
                        false, false);
                }
            }
            else
            {
                // all subsessions are up, now we send a rtsp "PLAY" command
                if (client->_session->absStartTime() != nullptr)
                {
                    // special case: the stream is indexed by absolute time, so send an
                    // appropriate "PLAY" command
                    client->sendPlayCommand(*client->_session, ContinueAfterPlay,
                        *client->_session->absStartTime(), *client->_session->absEndTime());
                }
                else
                {
                    client->_duration = client->_session->playEndTime() -
                        client->_session->playStartTime();
                    client->sendPlayCommand(*client->_session, ContinueAfterPlay);
                }
            }
        }

        void Live555RTSPClient::ShutdownStream(RTSPClient* rawClient, int exitCode)
        {
            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::ShutdownStream, rawClient was nullptr");
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);

            // first check if any subsessions still need closing
            if (client->_session != nullptr)
            {
                auto aSubsessionWasStillActive = false;

                MediaSubsessionIterator iterator(*client->_session);
                MediaSubsession* subsession;

                while ((subsession = iterator.next()) != nullptr)
                {
                    if (subsession->sink != nullptr)
                    {
                        Medium::close(subsession->sink);
                        subsession->sink = nullptr;
                    }

                    if (subsession->rtcpInstance() != nullptr)
                    {
                        // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
                        subsession->rtcpInstance()->setByeHandler(nullptr, nullptr);
                    }

                    aSubsessionWasStillActive = true;
                }

                // if a subsession was still active, we have to send a teardown command
                if (aSubsessionWasStillActive)
                {
                    client->sendTeardownCommand(*(client->_session), ContinueAfterTeardown);
                }
                // close the client immediately when no subsession was active
                else
                {
                    client->OnConnectionClosed();
                    Medium::close(client);                    
                }
            }
            // close the client immedatiately when no session was active
            else
            {
                client->OnConnectionClosed();
                Medium::close(client);                
            }
        }

        void Live555RTSPClient::OnSubsessionEnded(void* rawSubsession)
        {
            if (rawSubsession == nullptr)
            {
                Debug::LogError("Live555RTSPClient::OnSubsessionEnded, rawSubsession was nullptr");
                return;
            }

            auto subsession = static_cast<MediaSubsession*>(rawSubsession);
            auto client = static_cast<RTSPClient*>(subsession->miscPtr);

            // begin by closing this subsession's stream
            Medium::close(subsession->sink);
            subsession->sink = nullptr;

            // next, check if all subsessions streams have now been closed
            auto& session = subsession->parentSession();
            MediaSubsessionIterator iterator(session);

            while((subsession = iterator.next()) != nullptr)
            {
                if(subsession->sink != nullptr)
                {
                    return;
                }
            }

            // all are closed, so shutdown the client
            ShutdownStream(client);
        }

        void Live555RTSPClient::OnSubsessionBye(void* rawSubsession)
        {
            if (rawSubsession == nullptr)
            {
                Debug::LogError("Live555RTSPClient::OnSubsessionBye, rawSubsession was nullptr");
                return;
            }

            auto subsession = static_cast<MediaSubsession*>(rawSubsession);
            auto client = static_cast<RTSPClient*>(subsession->miscPtr);

            OnSubsessionEnded(subsession);
        }

        void Live555RTSPClient::OnStreamTimerExpired(void* rawClient)
        {
            if (rawClient == nullptr)
            {
                Debug::LogError("Live555RTSPClient::OnStreamTimerExpired, rawClient was nullptr");
                return;
            }

            auto client = static_cast<Live555RTSPClient*>(rawClient);
            client->_streamTimerTask = nullptr;

            ShutdownStream(client);
        }

        void Live555RTSPClient::OnConnectionAttempt()
        {
            Debug::Log("Opening connection to %s", _uri.c_str());
            _connected = false;
            _connectionFailed = false;
            _connecting = true;
            _connectionDropped = false;
        }

        void Live555RTSPClient::OnConnectionFailed()
        {
            Debug::Log("Connection to %s failed", _uri.c_str());
            _connected = false;
            _connectionFailed = true;
            _connecting = false;
            _connectionDropped = false;
        }

        void Live555RTSPClient::OnConnectionSuccess()
        {
            Debug::Log("Successfully Connected to %s", _uri.c_str());
            _connected = true;
            _connectionFailed = false;
            _connecting = false;
            _connectionDropped = false;
        }

        void Live555RTSPClient::OnConnectionClosed()
        {
            Debug::Log("Disconnected from %s", _uri.c_str());
            _connected = false;
            _connectionFailed = false;
            _connecting = false;
            _connectionDropped = false;
        }

        void Live555RTSPClient::OnConnectionDropped()
        {
            Debug::Log("Connection was dropped from %s", _uri.c_str());
            _connected = false;
            _connectionFailed = false;
            _connecting = false;
            _connectionDropped = true;
        }
    }
}
