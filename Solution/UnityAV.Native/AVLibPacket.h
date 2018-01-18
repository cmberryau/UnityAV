#pragma once
#include "AVLibUtil.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        /**
        * \brief Responsible for deletion of AVPacket instances
        */
        struct AVPacketDeleter
        {
            void operator()(AVPacket* packet)
            {
                av_packet_unref(packet);
                free(packet);
            }
        };

        /**
         * \brief Responsible for wrapping an AVPacket instance
         */
        class AVLibPacket
        {
        public:
            // Default destructor
            virtual ~AVLibPacket(){}
            // Default constructor
            explicit AVLibPacket();
            // Move constructor
            explicit AVLibPacket(AVLibPacket&& other) noexcept;
            // Move assignment
            AVLibPacket& operator=(AVLibPacket&& other) noexcept;
            // Disabled copy constructor
            explicit AVLibPacket(const AVLibPacket& other) = delete;
            // Disabled copy assignment
            AVLibPacket& operator=(const AVLibPacket& other) = delete;

            /**
            * \brief Is the packet marked EOF?
            * \return True if the packet is marked EOF, false otherwise
            */
            bool IsEOF() const;
            /**
             * \brief Is the packet marked as a seek request?
             * \return True if the packet is marked as a seek request, false otherwise
             */
            bool IsSeekRequest() const;
            /**
            * \brief Mark the packet as EOF
            */
            void SetAsEOF();
            /**
            * \brief Marks the packet as a seek request
            * \param time The time of the seek request
            */
            void SetSeekRequest(double time);
            /**
             * \brief The seek request time of the packet
             * \return The seek request time of the packet, if not a seek packet then 0 
             */
            double SeekTime() const;
            /**
            * \brief Performs the needed reset when the packet is recycled
            */
            void OnRecycle();
            /**
             * \brief Returns a reference to the AVPacket held
             * \return A reference to the AVPacket held 
             */
            AVPacket& Packet();
            /**
             * \brief Cleans the packet
             */
            void Clean();

#if _DEBUG
            static atomic_int DefaultConstructed;
            static atomic_int MoveConstructed;
            static atomic_int MoveAssigned;
#endif

        private:

            unique_ptr<AVPacket, AVPacketDeleter> _packet;
            bool _eof, _seek;
            double _seekTime;
        };
    }
}
