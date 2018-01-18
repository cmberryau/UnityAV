#include "stdafx.h"
#include "Live555Packet.h"

namespace UnityAV
{
    namespace Media
    {
        Live555Packet::Live555Packet(unsigned int bufferSize): _dataSize(0)
        {
            _buffer = unique_ptr<uint8_t>(new uint8_t[bufferSize]);
        }

        void Live555Packet::Read(const uint8_t* const data, unsigned dataSize)
        {
            memcpy(_buffer.get(), data, dataSize);
            _dataSize = dataSize;
        }

        unsigned Live555Packet::DataSize() const
        {
            return _dataSize;
        }

        uint8_t* Live555Packet::Data()
        {
            return _buffer.get();
        }
    }
}