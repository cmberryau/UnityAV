#pragma once

namespace UnityAV
{
    namespace Media
    {
        class Live555Packet
        {
        public:
            // Default destructor
            virtual ~Live555Packet(){}
            /**
             * \brief Initializes a new instance of Live555Packet
             * \param bufferSize The size of the buffer for the Live555Packet
             */
            explicit Live555Packet(unsigned int bufferSize);
            // Disabled move constructor
            Live555Packet(Live555Packet&& other) = delete;
            // Disabled move assignment
            Live555Packet& operator=(Live555Packet&& other) = delete;
            // Disabled copy constructor
            Live555Packet(const Live555Packet&& other) = delete;
            // Disabled copy assignment
            Live555Packet& operator=(const Live555Packet&& other) = delete;

            /**
             * \brief The packet reads the data and takes it on
             * \param data The data to read 
             * \param dataSize The size of the data to read
             */
            void Read(const uint8_t* const data, unsigned int dataSize);
            /**
             * \brief Evaluates the current size of the data held by the packet
             * \return The size of the data in bytes
             */
            unsigned int DataSize() const;
            /**
             * \brief Exposes the data held by the packet
             * \return A pointer to the data held by the packet
             */
            uint8_t* Data();

        private:
            unique_ptr<uint8_t> _buffer;
            unsigned int _dataSize;
        };
    }
}
