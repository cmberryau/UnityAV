#pragma once
#include "FixedSizeQueue.h"
#include "Live555Packet.h"

namespace UnityAV
{
    namespace Media
    {
        /**
        * \brief Responsible for recycling live555 packets
        */
        class Live555PacketRecycler
        {
        public:
            // Default deconstructor
            virtual ~Live555PacketRecycler() {};
            /**
            * \brief Initializes a new Live555PacketRecycler
            * \param maxCount the maximum number of packets to keep at once
            */
            explicit Live555PacketRecycler(int maxCount, unsigned int bufferSize);
            // Disabled copy constructor
            explicit Live555PacketRecycler(const Live555PacketRecycler&& other) = delete;
            // Disabled copy assignment
            Live555PacketRecycler& operator=(const Live555PacketRecycler&& other) = delete;
            // Disabled move constructor
            explicit Live555PacketRecycler(Live555PacketRecycler&& other) = delete;
            // Disabled move assignment
            Live555PacketRecycler& operator=(Live555PacketRecycler&& other) = delete;

            /**
            * \brief Recycles a packet
            * \param packet The packet to recycle
            */
            void Recycle(unique_ptr<Live555Packet> packet);
            /**
            * \brief Gets a packet
            * \return A packet ready to use
            */
            unique_ptr<Live555Packet> GetPacket();

        private:
            FixedSizeQueue<unique_ptr<Live555Packet>> _readyPackets;
            unsigned int _bufferSize;

            // meta
            int _givenPackets, _returnedPackets, _recycledPackets;
        };
    }
}
