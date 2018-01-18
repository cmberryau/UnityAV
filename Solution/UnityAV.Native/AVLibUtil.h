#pragma once
#include "stdafx.h"
#include "PixelFormat.h"

using namespace std;

namespace UnityAV
{
    namespace Media
    {
        const double kSecondToMicrosecond = 1e6;
        const double kMicrosecondToSecond = 1e-6;

        /**
        * \brief Responsible for deletion of AVCodecContext instances
        */
        struct AVCodecContextDeleter
        {
            void operator()(AVCodecContext* context)
            {
                avcodec_close(context);
            }
        };

        /**
        * \brief Responsible for deletion of AVFormatContext instances
        */
        struct AVFormatContextDeleter
        {
            void operator()(AVFormatContext* context)
            {
                avformat_close_input(&context);
            }
        };

        /**
        * \brief Responsible for the deletion of AVDictionary instances
        */
        struct AVDictionaryDeleter
        {
            void operator()(AVDictionary* dictionary)
            {
                av_dict_free(&dictionary);
            }
        };

        /**
        * \brief ONLY DELETES THE BASE POINTER, Responsible for deletion of AVDictionary arrays
        */
        struct AVDictionaryArrayDeleter
        {
            void operator()(AVDictionary** dictionaryArray)
            {
                av_freep(dictionaryArray);
            }
        };

        /**
        * \brief Responsible for deletion of SwsContext instances
        */
        struct SwsContextDeleter
        {
            void operator()(SwsContext* context)
            {
                sws_freeContext(context);
            }
        };

        /**
        * \brief Convert a PixelFormat enum to a AVPixelFormat enum
        * \param pixelFormat The PixelFormat enum to convert
        * \return The corresponding AVPixelFormat enum
        */
        AVPixelFormat ToAVPixelFormat(PixelFormat pixelFormat);
        /**
        * \brief Convert a AVPixelFormat enum to a PixelFormat enum
        * \param avPixelFormat The AVPixelFormat enum to convert
        * \return The corresponding PixelFormat enum
        */
        PixelFormat ToPixelFormat(AVPixelFormat avPixelFormat);
        /**
        * \brief Evaluates the best stream index for a given media type in an AVFormatContext instance
        * \param formatContext The AVFormatContext instance to evaluate against
        * \param mediaType The media type to evaluate for
        * \param desiredStream The desired stream index, defaults to auto selection
        * \param videoStream Any already resolved video stream, defaults to none resolved yet
        * \param audioStream Any already resolved audio stream, defaults to none resolved yet
        * \return The best stream index
        */
        int BestStreamIndex(AVFormatContext& formatContext, AVMediaType mediaType,
            int desiredStream = -1, int videoStream = -1, int audioStream = -1);
        /**
        * \brief Evaluates the best streams for given media types in an AVFormatContext instance
        * \param formatContext The AVFormatContext instance to evaluate against
        * \return A map of media types and the corresponding best stream indices
        */
        unordered_map<AVMediaType, int> BestStreamIndices(AVFormatContext& formatContext);
    }
}
