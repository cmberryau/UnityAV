#pragma once
#include "IFrameVisitor.h"

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Represents a single media frame
         */
        class Frame
        {
        public:
            // Default deconstructor
            virtual ~Frame(){}
            // Move constructor
            explicit Frame(Frame&& other) noexcept;
            // Move assignment
            Frame& operator=(Frame&& other) noexcept;
            // Disabled copy constructor
            explicit Frame(const Frame&& other) = delete;
            // Disabled copy assignment
            Frame& operator=(const Frame&& other) = delete;

            /**
             * \brief Accept the IFrameVisitor
             * \param visitor The visitor to accept
             */
            virtual void Accept(IFrameVisitor& visitor) = 0;
            /**
             * \brief The presentation time of the Frame in seconds
             * \return The presentation time of the Frame in seconds
             */
            virtual double Time() const = 0;
            /**
            * \brief Is the frame marked EOF?
            * \return True if the frame is marked EOF, false otherwise
            */
            bool IsEOF() const;
            /**
            * \brief Mark the frame as EOF
            */
            void SetAsEOF();
            /**
            * \brief Performs the needed reset when the frame is recycled
            */
            void OnRecycle();

        protected:
            // Default constructor
            explicit Frame();

        private:
#if _DEBUG
            static atomic_int DefaultConstructed;
            static atomic_int MoveConstructed;
            static atomic_int MoveAssigned;
#endif
            bool _eof;//, _seek;
            //double _seekTime;
        };
    }
}
