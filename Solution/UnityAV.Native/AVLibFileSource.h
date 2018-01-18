#pragma once
#include "IAVLibSource.h"
#include "AVLibPacketRecycler.h"

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Responsible for reading files on disk
         */
        class AVLibFileSource : public IAVLibSource
        {
        public:
            // Default destructor
            virtual ~AVLibFileSource();
            /**
             * \brief Initializes a new instance of AVLibFileSource
             * \param uri The uri of the media file to read
             */
            explicit AVLibFileSource(string uri);
            // Disabled copy constructor
            explicit AVLibFileSource(const AVLibFileSource&& other) = delete;
            // Disabled copy assignment
            AVLibFileSource& operator=(const AVLibFileSource&& other) = delete;
            // Disabled move constructor
            explicit AVLibFileSource(AVLibFileSource&& other) = delete;
            // Disabled move assignment
            AVLibFileSource& operator=(AVLibFileSource&& other) = delete;

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
            static const int DefaultVideoPacketQueueSize;
            static const int DefaultAudioPacketQueueSize;
            static const int DefaultSubtitlePacketQueueSize;
            static const double SeekThreshold;

            static int BlockingIOInterruptCallback(void * source);
            static bool OpenFile(AVFormatContext& formatContext, string uri);            
            
            void Initialize();
            void ReadThread();
            void Continue();
            bool Wait();
            void OnEOF();
            void OnSeekRequest();
            bool HandleReadError(int error);
            void UpdateMeta(AVLibPacket& packet);
            bool QueuePacket(unique_ptr<AVLibPacket> packet);
            void FlushQueues();
            void InjectSeekPackets(double time);
            bool AnyQueueFull() const;

            // packets
            unique_ptr<AVFormatContext, AVFormatContextDeleter> _formatContext;
            vector<FixedSizeQueue<unique_ptr<AVLibPacket>>> _packetQueues;
            AVLibPacketRecycler _recycler;
            vector<int> _queueThresholds;
            vector<bool> _activeQueues;

            // streams
            vector<int> _streamIndices;
            vector<int> _streamIndicesToInternal;
            vector<AVMediaType> _streamTypes;
            vector<AVStream> _streams;
            vector<double> _timeBases;
            vector<double> _frameRates;
            
            atomic_bool _eof;

            // seeking
            double _duration;
            atomic_flag _seekRequest = ATOMIC_FLAG_INIT;
            int64_t _lowestDTS, _lowestPTS;
            int _seekStreamIndex;
            double _seekTimeBase, _seekToTime, _seekFromTime;

            // threading
            thread _thread;
            mutex _continueMutex;
            condition_variable _continue;
            atomic_flag _stayAlive = ATOMIC_FLAG_INIT;

            // meta
            int _failedPackets, _successfulPackets, _skippedPackets;
        };
    }
}
