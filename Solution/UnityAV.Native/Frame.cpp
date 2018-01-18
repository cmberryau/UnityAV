#include "stdafx.h"
#include "Frame.h"

namespace UnityAV
{
    namespace Media
    {
#if _DEBUG
        atomic_int Frame::DefaultConstructed;
        atomic_int Frame::MoveConstructed;
        atomic_int Frame::MoveAssigned;
#endif

        Frame::Frame(Frame&& other) noexcept : _eof(other._eof)
        {
#if _DEBUG
            ++MoveConstructed;
#endif
        }

        Frame& Frame::operator=(Frame&& other) noexcept
        {
#if _DEBUG
            ++MoveAssigned;
#endif

            _eof = other._eof;

            return *this;
        }

        bool Frame::IsEOF() const
        {
            return _eof;
        }

        void Frame::SetAsEOF()
        {
            _eof = true;
        }

        void Frame::OnRecycle()
        {
            _eof = false;
        }

        Frame::Frame() : _eof(false)
        {
#if _DEBUG
            ++DefaultConstructed;
#endif
        }
    }
}
