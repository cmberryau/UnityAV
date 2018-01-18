#include "stdafx.h"
#include "Live555PacketRecycler.h"

namespace UnityAV
{
    namespace Media
    {
        Live555PacketRecycler::Live555PacketRecycler(int maxCount, 
            unsigned int bufferSize) : _readyPackets(maxCount), _bufferSize(bufferSize),
            _givenPackets(0), _returnedPackets(0), _recycledPackets(0)
        {

        }

        void Live555PacketRecycler::Recycle(unique_ptr<Live555Packet> packet)
        {
            _readyPackets.Push(move(packet));
            _returnedPackets++;
        }

        unique_ptr<Live555Packet> Live555PacketRecycler::GetPacket()
        {
            auto packet = _readyPackets.Pop();

            if (packet == nullptr)
            {
                packet = make_unique<Live555Packet>(_bufferSize);
            }
            else
            {
                _recycledPackets++;
            }

            _givenPackets++;

            return move(packet);
        }
    }
}