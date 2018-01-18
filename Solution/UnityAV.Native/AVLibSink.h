#pragma once
#include <MediaSink.hh>
#include <MediaSession.hh>

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief The AVLib endpoint for data from a Live555 stream
         */
        class AVLibSink : public MediaSink
        {
        public:
            // Default destructor
            virtual ~AVLibSink();
            /**
            * \brief Initializes a new instance of AVLibSink
            * \param usageEnvironment The usage environment for the sink
            * \param subsession The media subsession the sink will be receiving
            * \param uri The uri of the stream the sink is receiving
            */
            explicit AVLibSink(UsageEnvironment& usageEnvironment,
                MediaSubsession& subsession, const string& uri);
            // Disabled move constructor
            AVLibSink(AVLibSink&& other) = delete;
            // Disabled move assignment
            AVLibSink& operator=(AVLibSink&& other) = delete;
            // Disabled copy constructor
            AVLibSink(const AVLibSink&& other) = delete;
            // Disabled copy assignment
            AVLibSink& operator=(const AVLibSink&& other) = delete;

        protected:
            bool continuePlaying() override;

        private:
            static const int DefaultBufferSize;

            // static callback method
            static void OnNewFrame(void* clientData, unsigned frameSize,
                unsigned numTruncatedBytes, struct timeval presentationTime,
                unsigned durationInMicroseconds);

            // member callback method, called by the static callback method
            void OnNewFrame(unsigned frameSize, unsigned numTruncatedBytes,
                struct timeval presentationTime, unsigned durationInMicroseconds);

            // core
            unique_ptr<u_int8_t> _receiveBuffer;
            MediaSubsession& _subsession;
            string _uri;
        };
    }
}
