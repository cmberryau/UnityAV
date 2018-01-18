#include "stdafx.h"
#include "AVLibSink.h"

namespace UnityAV
{
    namespace Media
    {
        // 4096 * 4096 * 32 = should be large enough to deal with huge frames
        const int AVLibSink::DefaultBufferSize = 4096 * 4096 * 32;

        AVLibSink::AVLibSink(UsageEnvironment& env, MediaSubsession& subsession, 
            const string& uri) : MediaSink(env), _subsession(subsession)
        {
            _uri = uri;
            _receiveBuffer = unique_ptr<u_int8_t>(new u_int8_t[DefaultBufferSize]);
        }

        AVLibSink::~AVLibSink()
        {

        }

        void AVLibSink::OnNewFrame(void* clientData, unsigned frameSize,
            unsigned numTruncatedBytes, struct timeval presentationTime,
            unsigned durationInMicroseconds)
        {
            if (clientData == nullptr)
            {
                return;
            }

            auto sink = static_cast<AVLibSink*>(clientData);
            sink->OnNewFrame(frameSize, numTruncatedBytes, presentationTime,
                durationInMicroseconds);
        }

        void AVLibSink::OnNewFrame(unsigned frameSize, unsigned numTruncatedBytes,
            struct timeval presentationTime, unsigned durationInMicroseconds)
        {
            Debug::Log("uri: %s - type: %s/%s - size: %d bytes", _uri.c_str(),
                _subsession.mediumName(), _subsession.codecName(), frameSize);

            // request the next frame of data:
            continuePlaying();
        }

        bool AVLibSink::continuePlaying()
        {
            // sanity check (should not happen)
            if (fSource == nullptr)
            {
                return false;
            }

            // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
            fSource->getNextFrame(_receiveBuffer.get(), DefaultBufferSize, OnNewFrame, 
                this, onSourceClosure, this);

            return true;
        }
    }
}