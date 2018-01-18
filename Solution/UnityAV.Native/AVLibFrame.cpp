#include "stdafx.h"
#include "AVLibFrame.h"

namespace UnityAV
{
    namespace Media
    {
#if _DEBUG
        atomic_int AVLibFrame::DefaultConstructed = 0;
#endif

        AVLibFrame::AVLibFrame() : _frame(unique_ptr<AVFrame, AVFrameDeleter>(
            av_frame_alloc()))
        {
#if _DEBUG
            ++DefaultConstructed;
#endif
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        AVFrame& AVLibFrame::Frame()
        {
            return *_frame;
        }

        void AVLibFrame::Clean()
        {
            av_frame_unref(_frame.get());
        }
    }
}