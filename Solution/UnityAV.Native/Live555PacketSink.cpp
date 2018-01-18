#include "stdafx.h"
#include "Live555PacketSink.h"

namespace UnityAV
{
    namespace Media
    {
        const int Live555PacketSink::DefaultBufferSize = 1280 * 1280;
        const int Live555PacketSink::DefaultPacketQueueSize = 5;

        Live555PacketSink::Live555PacketSink(UsageEnvironment& env, const string& uri)
            : MediaSink(env), _packetQueue(DefaultPacketQueueSize), 
            _recycler(DefaultPacketQueueSize, DefaultBufferSize)
        {
            _uri = uri;
            _receiveBuffer = unique_ptr<uint8_t>(new uint8_t[DefaultBufferSize]);
        }

        Live555PacketSink::~Live555PacketSink() 
        {
            
        }

        void Live555PacketSink::OnNewFrame(void* rawSink, unsigned frameSize, 
            unsigned numTruncatedBytes, struct timeval presentationTime, 
            unsigned durationInMicroseconds) 
        {
            if(rawSink == nullptr)
            {
                Debug::LogError("Live555PacketSink::OnNewFrame - rawSink was nullptr");
                return;
            }

            auto sink = static_cast<Live555PacketSink*>(rawSink);
            sink->OnNewFrame(frameSize, numTruncatedBytes, presentationTime, 
                durationInMicroseconds);
        }

        void Live555PacketSink::OnNewFrame(unsigned frameSize, unsigned numTruncatedBytes,
            struct timeval presentationTime, unsigned durationInMicroseconds) 
        {
            auto packet = _recycler.GetPacket();
            packet->Read(_receiveBuffer.get(), frameSize);
            _packetQueue.Push(move(packet));

            // request the next frame of data:
            continuePlaying();
        }

        unique_ptr<Live555Packet> Live555PacketSink::TryGetNext()
        {
            return _packetQueue.Pop();
        }

        void Live555PacketSink::Recycle(unique_ptr<Live555Packet> packet)
        {
            if(packet == nullptr)
            {
                Debug::LogError("Live555PacketSink::Recycke - packet was nullptr");
                return;
            }

            _recycler.Recycle(move(packet));
        }

        bool Live555PacketSink::continuePlaying()
        {
            // sanity check (should not happen)
            if (fSource == nullptr)
            {
                Debug::LogError("Live5555PacketSink::continuePlaying - fSource was nullptr");
                return false;
            }

            // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
            fSource->getNextFrame(_receiveBuffer.get(), DefaultBufferSize, OnNewFrame, 
                this, onSourceClosure, this);

            return true;
        }
    }
}
