#include "stdafx.h"
#include "AVLibPacket.h"

namespace UnityAV
{
    namespace Media
    {
#if _DEBUG
        atomic_int AVLibPacket::DefaultConstructed;
        atomic_int AVLibPacket::MoveConstructed;
        atomic_int AVLibPacket::MoveAssigned;
#endif

        AVLibPacket::AVLibPacket(): _eof(false), _seek(false), _seekTime(0)
        {
#if _DEBUG
            ++DefaultConstructed;
#endif
        }

        AVLibPacket::AVLibPacket(AVLibPacket&& other) noexcept 
            : _packet(move(other._packet)), _eof(other._eof), _seek(other._seek)
        {
#if _DEBUG
            ++MoveConstructed;
#endif
        }

        AVLibPacket& AVLibPacket::operator=(AVLibPacket&& other) noexcept
        {
#if _DEBUG
            ++MoveAssigned;
#endif

            _packet = move(other._packet);
            _eof = other._eof;
            _seek = other._seek;
            _seekTime = other._seekTime;

            return *this;
        }

        bool AVLibPacket::IsEOF() const
        {
            return _eof;
        }

        bool AVLibPacket::IsSeekRequest() const
        {
            return _seek;
        }

        void AVLibPacket::SetAsEOF()
        {
            _eof = true;
        }

        void AVLibPacket::SetSeekRequest(double time)
        {
            _seek = true;
            _seekTime = time;
        }

        double AVLibPacket::SeekTime() const
        {
            return _seekTime;
        }

        void AVLibPacket::OnRecycle()
        {
            Clean();

            _eof = false;
            _seek = false;
            _seekTime = 0;
        }

        AVPacket& AVLibPacket::Packet()
        {
            if(!_packet)
            {
                _packet = unique_ptr<AVPacket, AVPacketDeleter>(new AVPacket());
                av_init_packet(_packet.get());
            }

            return *_packet;
        }

        void AVLibPacket::Clean()
        {
            av_packet_unref(_packet.get());
        }
    }
}