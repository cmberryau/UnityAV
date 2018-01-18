#pragma once
#include "FixedSizeQueue.h"
#include "AVLibPacket.h"

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Responsible for recycling avlib packets
         */
        class AVLibPacketRecycler
        {
        public:
            // Default deconstructor
            virtual ~AVLibPacketRecycler(){}
            /**
             * \brief Initializes a new AVLibPacketRecycler
             * \param maxCount the maximum number of packets to keep at once
             */
            explicit AVLibPacketRecycler(int maxCount);
            // Disabled copy constructor
            explicit AVLibPacketRecycler(const AVLibPacketRecycler&& other) = delete;
            // Disabled copy assignment
            AVLibPacketRecycler& operator=(const AVLibPacketRecycler&& other) = delete;
            // Disabled move constructor
            explicit AVLibPacketRecycler(AVLibPacketRecycler&& other) = delete;
            // Disabled move assignment
            AVLibPacketRecycler& operator=(AVLibPacketRecycler&& other) = delete;

            /**
            * \brief Recycles a packet
            * \param packet The packet to recycle
            */
            void Recycle(unique_ptr<AVLibPacket> packet);
            /**
             * \brief Gets a packet
             * \return A packet ready to use
             */
            unique_ptr<AVLibPacket> GetPacket();

        private:
            FixedSizeQueue<unique_ptr<AVLibPacket>> _readyPackets;

            // meta
            int _givenPackets, _returnedPackets, _recycledPackets;
        };
    }
}
