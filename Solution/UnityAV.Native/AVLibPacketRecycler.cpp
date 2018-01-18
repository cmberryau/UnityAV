#include "stdafx.h"
#include "AVLibPacketRecycler.h"

namespace UnityAV
{
    namespace Media
    {
        AVLibPacketRecycler::AVLibPacketRecycler(int maxCount) : _readyPackets(maxCount),
            _givenPackets(0), _returnedPackets(0), _recycledPackets(0)
        {

        }

        void AVLibPacketRecycler::Recycle(unique_ptr<AVLibPacket> packet)
        {
            packet->OnRecycle();
            _readyPackets.Push(move(packet));
            _returnedPackets++;
        }

        unique_ptr<AVLibPacket> AVLibPacketRecycler::GetPacket()
        {
            auto packet = _readyPackets.Pop();

            if (packet == nullptr)
            {
                packet = make_unique<AVLibPacket>();
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