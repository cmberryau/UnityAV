#pragma once

// can't be included as part of stdafx, causes problems in avlib code
#include <MediaSink.hh>

#include "FixedSizeQueue.h"
#include "Live555PacketRecycler.h"

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief The endpoint for data from a Live555 subsession
         */
        class Live555PacketSink : public MediaSink 
        {
        public:
            // Default destructor
            virtual ~Live555PacketSink();
            /**
            * \brief Initializes a new instance of Live555DummySink
            * \param usageEnvironment The usage environment for the sink
            * \param uri The uri of the stream the sink is receiving
            */
            explicit Live555PacketSink(UsageEnvironment& usageEnvironment, const string& uri);
            // Disabled move constructor
            Live555PacketSink(Live555PacketSink&& other) = delete;
            // Disabled move assignment
            Live555PacketSink& operator=(Live555PacketSink&& other) = delete;
            // Disabled copy constructor
            Live555PacketSink(const Live555PacketSink&& other) = delete;
            // Disabled copy assignment
            Live555PacketSink& operator=(const Live555PacketSink&& other) = delete;

            unique_ptr<Live555Packet> TryGetNext();
            void Recycle(unique_ptr<Live555Packet> packet);

        protected:
            bool continuePlaying() override;

        private:
            static const int DefaultBufferSize;
            static const int DefaultPacketQueueSize;

            // static callback method
            static void OnNewFrame(void* clientData, unsigned frameSize, 
                unsigned numTruncatedBytes, struct timeval presentationTime,
                unsigned durationInMicroseconds);

            // member callback method, called by the static callback method
            void OnNewFrame(unsigned frameSize, unsigned numTruncatedBytes,
                struct timeval presentationTime, unsigned durationInMicroseconds);            

            // core
            unique_ptr<uint8_t> _receiveBuffer;
            string _uri;

            FixedSizeQueue<unique_ptr<Live555Packet>> _packetQueue;
            Live555PacketRecycler _recycler;
        };
    }
}
