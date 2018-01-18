#pragma once

#include "IAVLibSource.h"
#include "AVLibPacketRecycler.h"

#include "Live555Util.h"
#include "Live555RTSPClient.h"

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Responsible for reading a rtsp stream
         */
        class AVLibRTSPSource : public IAVLibSource
        {
        public:
            // Default destructor
            virtual ~AVLibRTSPSource();
            /**
            * \brief Initializes a new instance of AVLibRTSPSource
            * \param uri The uri of the rtsp stream to read
            */
            explicit AVLibRTSPSource(const string& uri);
            // Disabled copy constructor
            explicit AVLibRTSPSource(const AVLibRTSPSource&& other) = delete;
            // Disabled copy assignment
            AVLibRTSPSource& operator=(const AVLibRTSPSource&& other) = delete;
            // Disabled move constructor
            explicit AVLibRTSPSource(AVLibRTSPSource&& other) = delete;
            // Disabled move assignment
            AVLibRTSPSource& operator=(AVLibRTSPSource&& other) = delete;

            void Connect() override;
            bool IsConnected() const override;
            double Duration() const override;            
            int StreamCount() const override;
            AVMediaType StreamType(int streamIndex) const override;
            const AVStream& Stream(int streamIndex) const override;            
            double TimeBase(int streamIndex) const override;
            double FrameRate(int streamIndex) const override;
            double FrameDuration(int streamIndex) const override;
            bool IsRealtime() const override;
            bool CanSeek() const override;
            void Seek(double from, double to) override;
            unique_ptr<AVLibPacket> TryGetNext(int streamIndex) override;
            void Recycle(unique_ptr<AVLibPacket> packet) override;

        private:
            static const AVStream EmptyStream;
            static const int DefaultPacketQueueSize;            

            // live555 process wide members
            static mutex ProcessWideMutex;
            static atomic_flag ProcessWideInitialized;
            static int InstanceCount;
            static thread TaskSchedulerThread;
            static volatile char WatchVariable;
            static unique_ptr<BasicUsageEnvironment, 
                BasicUsageEnvrionmentDeleter> Environment;
            static unique_ptr<BasicTaskScheduler> TaskScheduler;

            // live555 process wide methods
            static void ProcessWideInitialize();
            static void ProcessWideTeardown();
            static void SchedulerThreadMethod();

            // live555 client
            unique_ptr<Live555RTSPClient, 
                Live555RTSPClient::Live555RTSPClientDeleter> _rtspClient;

            // packets
            AVLibPacketRecycler _recycler;

            // streams
            vector<AVMediaType> _streamTypes;
            vector<AVStream> _streams;
            vector<double> _timeBases;
            vector<double> _frameRates;

            string _uri;
        };
    }
}
