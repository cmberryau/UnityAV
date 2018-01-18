#pragma once

namespace UnityAV
{
    namespace Media
    {
        class AVLibVideoDecoder;

        /**
         * \brief An interface to visit concrete types of IAVLibDecoder
         */
        class IAVLibDecoderVisitor
        {
        public:
            /**
             * \brief Visits the AVLibVideoDecoder instance
             * \param videoDecoder The AVLibVideoDecoder instance to visit
             */
            virtual void Visit(AVLibVideoDecoder& videoDecoder) = 0;

        protected:
            ~IAVLibDecoderVisitor(){}
        };
    }
}