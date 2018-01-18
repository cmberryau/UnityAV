#pragma once
#include "AVLibPacket.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Interface for AVLib sources
         */
        class IAVLibSource
        {
        public:
            // Default destructor
            virtual ~IAVLibSource(){}

            /**
            * \brief Connects the source to all streams, must be non blocking
            */
            virtual void Connect() = 0;
            /**
            * \brief Evaluates if the source has connected to all streams
            * \return True if the source has connected to all streams
            */
            virtual bool IsConnected() const = 0;
            /**
            * \brief Evaluates the duration of the media loaded
            * \return The duration of the media loaded
            */
            virtual double Duration() const = 0;
            /**
            * \brief Evaluates the number of streams
            * \return Returns the number of streams
            */
            virtual int StreamCount() const = 0;
            /**
            * \brief Evaluates the stream type for the given index
            * \return Returns the stream type for the given index
            */
            virtual AVMediaType StreamType(int streamIndex) const = 0;
            /**
            * \brief Evaluates the stream for the given index
            * \return Returns the stream for the given index
            */
            virtual const AVStream& Stream(int streamIndex) const = 0;
            /**
            * \brief Evaluates the time base for the given stream
            * \param streamIndex The stream index to evaluate for
            * \return The time base in seconds
            */
            virtual double TimeBase(int streamIndex) const = 0;
            /**
            * \brief Evaluates the frame rate for the given stream
            * \param streamIndex The stream index to evaluate for
            * \return The frame rate in frames per second
            */
            virtual double FrameRate(int streamIndex) const = 0;
            /**
            * \brief Evaluates the frame duration for the given stream
            * \param streamIndex The stream index to evaluate for
            * \return The frame duration in seconds
            */
            virtual double FrameDuration(int streamIndex) const = 0;
            /**
             * \brief Evaluates if the source is realtime
             * \return True if the source is realtime, false otherwise
             */
            virtual bool IsRealtime() const = 0;
            /**
             * \brief Evaluates if the source can seek
             * \return True if the source can seek, false otherwise
             */
            virtual bool CanSeek() const = 0;  
            /**
             * \brief Instructs the source to seek
             * \param from The time to begin seeking from
             * \param to The time to seek to
             */
            virtual void Seek(double from, double to) = 0;
            /**
            * \brief Attempts to get the next packet for a stream
            * \param streamIndex The stream index to evaluate for
            * \return The next packet in the stream, nullptr if none
            */
            virtual unique_ptr<AVLibPacket> TryGetNext(int streamIndex) = 0;
            /**
            * \brief Recycles a packet
            * \param packet The packet to recycle
            */
            virtual void Recycle(unique_ptr<AVLibPacket> packet) = 0;
        };
    }
}
