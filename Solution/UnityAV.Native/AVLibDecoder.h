#pragma once
#include "AVLibUtil.h"
#include "AVLibPacket.h"
#include "AVLibFrame.h"
#include "IAVLibSource.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        class IVideoDescription;
        class AVLibFileSource;
        class IAVLibDecoderVisitor;

        /**
         * \brief Responsible for decoding of a single avlib stream, concrete classes
         * must call StartDecoding in their constructor and StopDecoding in their
         * destructor
         */
        class AVLibDecoder
        {
        public:
            // Default destructor
            virtual ~AVLibDecoder(){}
            // Disabled copy constructor
            explicit AVLibDecoder(const AVLibDecoder&& other) = delete;
            // Disabled copy assignment
            AVLibDecoder& operator=(const AVLibDecoder&& other) = delete;
            // Disabled move constructor
            explicit AVLibDecoder(AVLibDecoder&& other) = delete;
            // Disabled move assignment
            AVLibDecoder& operator=(AVLibDecoder&& other) = delete;

            /**
            * \brief Creates all decoders for the given source
            * \param source The source to create the the decoders for
            * \param requiredVideo The required video parameters
            * \return A vector of decoders for the source
            */
            static vector<unique_ptr<AVLibDecoder>> Create(IAVLibSource& source,
                const IVideoDescription& requiredVideo);
            /**
             * \brief Accepts a visit from a IAVLibDecoderVisitor instance
             * \param visitor The visitor to accept
             */
            virtual void Accept(IAVLibDecoderVisitor& visitor) = 0;

            /**
            * \brief Evalutes the time base of the stream
            * \return The time base of the stream
            */
            double GetTimeBase() const;
            /**
             * \brief Evaluates the nominal frame rate of the stream
             * \return The nominal frame rate of the stream
             */
            double GetFrameRate() const;
            /**
             * \brief Evaluates the nominal frame duration of the stream
             * \return The nominal frame duration of the stream
             */
            double GetFrameDuration() const;

        protected:            
            /**
             * \brief Initializes a new instance of AVLibDecoder
             * \param source The source to get packets from
             * \param codecContext The codec context of the stream
             * \param streamIndex The stream index
             */
            explicit AVLibDecoder(IAVLibSource& source, unique_ptr<AVCodecContext,
                AVCodecContextDeleter> codecContext, int streamIndex);

            /**
            * \brief Evaluates if the decoder can decode more frames
            * \return True when the decoder can decode more, false otherwise
            */
            virtual bool CanDecodeMore() = 0;
            /**
             * \brief Tries to decode an AVLibPacket
             * \param packet The packet to decode
             * \return True on success, false on failure
             */
            virtual bool TryDecode(AVLibPacket& packet) = 0;
            /**
             * \brief Tries to get a decoded AVLibFrame
             * \return True on success, false on failure
             */
            virtual bool TryGetDecodedFrame(AVLibFrame& frame) = 0;
            /**
            * \brief Tries to parse a decoded AVLibFrame
            * \return True on success, false on failure
            */
            virtual bool TryParse(AVLibFrame& frame) = 0;
            /**
             * \brief Injects a EOF marker to the output stream
             */
            virtual void OnEOF() = 0;
            /**
             * \brief Injects a seek marker to the output stream
             * \param to The time to seek to
             */
            virtual void OnSeek(double to) = 0;

            /**
             * \brief Terminates the decoding thread, must be called in child destructors
             */
            void StopDecoding();
            /**
             * \brief Starts the decoding thread, should be called by concrete classes
             * after they've finished their initialization
             */
            void StartDecoding();
            /**
             * \brief Should be called by concrete classes when they need more packets
             */
            void OnNeedMorePackets();
            /**
             * \brief Returns a reference to the AVCodecContext
             * \return A reference to the AVCodecContext
             */
            AVCodecContext& GetCodecContext();
            /**
             * \brief Evaluates if the decoder is a realtime decoder
             * \return True if the decoder is a realtime decoder, false otherwise
             */
            bool IsRealtime() const;

        private:
            static unique_ptr<AVLibDecoder> Create(IAVLibSource& source, int streamIndex,
                const IVideoDescription& requiredVideo);
            
            void DecodeThread();
            void ContinueDecoding();
            bool Wait();
            void OnEOF(const AVLibPacket& seekPacket);
            void OnSeek(const AVLibPacket& seekPacket);
            bool DecodePacket(AVLibPacket& packet);

            // core
            IAVLibSource& _source;
            unique_ptr<AVCodecContext, AVCodecContextDeleter> _codecContext;
            int _streamIndex;
            double _timeBase, _frameRate, _frameDuration;
            AVLibFrame _avLibFrame;

            // threading
            thread _thread;
            mutex _continueMutex;
            condition_variable _continue;
            atomic_flag _stayAlive = ATOMIC_FLAG_INIT;

            // meta
            int _successfulDecodes, _failedDecodes;
            int _successfulParses, _failedParses;
        };
    }
}