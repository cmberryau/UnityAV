#pragma once

// can't be included as part of stdafx, causes problems in avlib code
#include <RTSPClient.hh>

#include "Live555PacketSink.h"

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Responsible for managing a connection to a RTSP stream
         */
        class Live555RTSPClient : public RTSPClient
        {
        public:
            /**
             * \brief Responsible for deleting Live555RTSPClient
             */
            struct Live555RTSPClientDeleter
            {
                void operator()(Live555RTSPClient* client)
                {
                    //delete client;
                    ShutdownStream(client);
                }
            };

            /**
             * \brief Initializes a new instance of Live555RTSPClient
             * \param usageEnvironment The usage environment for the client
             * \param uri The uri of the rtsp stream
             */
            explicit Live555RTSPClient(UsageEnvironment& usageEnvironment, const string& uri);
            // Disabled move constructor
            explicit Live555RTSPClient(Live555RTSPClient&& other) = delete;
            // Disabled move assignment
            Live555RTSPClient& operator=(Live555RTSPClient&& other) = delete;
            // Disabled copy constructor
            explicit Live555RTSPClient(const Live555RTSPClient&& other) = delete;
            // Disabled copy assignment
            Live555RTSPClient& operator=(const Live555RTSPClient&& other) = delete;

            /**
             * \brief Attempts to connect the client
             */
            void Connect();
            /**
             * \brief Evaluates if the client is connected
             * \return True if the client is connected, false otherwise
             */            
            bool IsConnected() const;
            /**
             * \brief Evaluates if the last attempted client connection failed
             * \return True if the last client connection failed, false otherwise
             */
            bool ConnectionFailed() const;
            /**
             * \brief Evaluates if the clients connection to the server was dropped
             * \return True if the connection was dropped, false otherwise
             */
            bool ConnectionDropped() const;
            /**
             * \brief Evaluates the number of subsessions the client has
             * \return The number of subsessions the client has
             */
            int SubsessionCount() const;
            /**
             * \brief Attempts to get the next packet
             * \param subsessionIndex The subsession to get the packet for
             * \return A live555 packet on success, otherwise nullptr
             */
            unique_ptr<Live555Packet> TryGetNext(int subsessionIndex);
            /**
             * \brief Recycles a live555 packet for reuse
             * \param subsessionIndex The subsession the packet belongs to
             * \param packet The live555 packet to reuse
             */
            void Recycle(unique_ptr<Live555Packet> packet, int subsessionIndex);

        protected:
            // Hidden destructor
            virtual ~Live555RTSPClient();

        private:
            static const int DefaultVerbosityLevel;
            static const string DefaultApplicationName;
            static const portNumBits DefaultTunnelOverHttpPort;
            static const int DefaultSocketNumToServer;
            static const int DefaultTimeoutCheckBeginSeconds;
            static const int DefaultTimeoutSeconds;

            // rtsp response handlers
            static void ContinueAfterDescribe(RTSPClient* rawClient, int resultCode,
                char* resultString);
            static void ContinueAfterTimeoutDescribe(RTSPClient* rawClient,
                int resultCode, char* rawResultString);
            static void ContinueAfterSetup(RTSPClient* rawClient, int resultCode,
                char* rawResultString);
            static void ContinueAfterPlay(RTSPClient* rawClient, int resultCode,
                char* rawResultString);
            static void ContinueAfterTeardown(RTSPClient* rawClient, int resultCode,
                char* rawResultString);

            // stream initialization and teardown
            static void SetupNextSubsession(RTSPClient* client);
            static void ShutdownStream(RTSPClient* client, int exitCode = 1);

            // other event handler functions:
            static void OnSubsessionEnded(void* clientData);
            static void OnSubsessionBye(void* clientData);
            static void OnStreamTimerExpired(void* clientData);
            
            void OnConnectionAttempt();
            void OnConnectionFailed();
            void OnConnectionSuccess();
            void OnConnectionClosed();
            void OnConnectionDropped();

            // core
            MediaSubsessionIterator* _subSessionIterator;
            MediaSession* _session;
            MediaSubsession* _subsession;
            TaskToken _streamTimerTask;
            double _duration;

            // subsessions and their sinks
            vector<MediaSubsession*> _subsessions;
            vector<Live555PacketSink*> _packetSinks;

            // connection
            bool _connected;
            bool _connecting;
            bool _connectionFailed;
            bool _connectionDropped;
            bool _checkingConnection;
            time_t _checkStartTime;
            time_t _lastServerActivityTime;
            int64_t _beginTimeoutCheck;
            int64_t _timeout;
            int64_t _packetCount;
            string _uri;
        };
    }
}
