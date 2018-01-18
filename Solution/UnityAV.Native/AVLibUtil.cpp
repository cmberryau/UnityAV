#include "stdafx.h"
#include "AVLibUtil.h"

namespace UnityAV
{
    namespace Media
    {
        AVPixelFormat ToAVPixelFormat(PixelFormat pixelFormat)
        {
            switch (pixelFormat)
            {
            case PIXEL_FORMAT_YUV420P:
                return AV_PIX_FMT_YUV420P;
            case PIXEL_FORMAT_RGBA32:
                return AV_PIX_FMT_RGBA;
            default:
                return AV_PIX_FMT_NONE;
            }
        }

        PixelFormat ToPixelFormat(AVPixelFormat avPixelFormat)
        {
            switch (avPixelFormat)
            {
            case AV_PIX_FMT_NONE:
                return PIXEL_FORMAT_NONE;
            case AV_PIX_FMT_YUV420P:
                return PIXEL_FORMAT_YUV420P;
            case AV_PIX_FMT_RGBA:
                return PIXEL_FORMAT_RGBA32;
            default:
                return PIXEL_FORMAT_NONE;
            }
        }

        int BestStreamIndex(AVFormatContext& formatContext, AVMediaType mediaType,
            int desiredStream, int videoStream, int audioStream)
        {
            auto result = -1;

            switch (mediaType)
            {
            case AVMEDIA_TYPE_UNKNOWN: break;
            case AVMEDIA_TYPE_VIDEO:
                // when finding video stream, use audio as related
                result = av_find_best_stream(&formatContext, AVMEDIA_TYPE_VIDEO,
                    desiredStream, videoStream, nullptr, 0);
                break;
            case AVMEDIA_TYPE_AUDIO:
                // when finding audio stream, use video as related
                result = av_find_best_stream(&formatContext, AVMEDIA_TYPE_AUDIO,
                    desiredStream, videoStream, nullptr, 0);
                break;
            case AVMEDIA_TYPE_DATA: break;
            case AVMEDIA_TYPE_SUBTITLE:
                // when finding subtitle stream, prefer to use video stream but use
                // audio stream if video stream is not yet resolved
                result = av_find_best_stream(&formatContext, AVMEDIA_TYPE_SUBTITLE,
                    desiredStream, videoStream = -1 ? audioStream : videoStream,
                    nullptr, 0);
                break;
            case AVMEDIA_TYPE_ATTACHMENT: break;
            case AVMEDIA_TYPE_NB: break;
            default: break;
            }

            return result;
        }

        unordered_map<AVMediaType, int> BestStreamIndices(AVFormatContext& formatContext)
        {
            // get the total number of streams
            auto streamCount = formatContext.nb_streams;

            // get the indices for streams for media types
            int streamsForType[AVMEDIA_TYPE_NB];
            memset(streamsForType, -1, sizeof(streamsForType));

            // get the count of streams for each media type
            int streamCountForMedia[AVMEDIA_TYPE_NB];
            memset(streamCountForMedia, 0, sizeof(streamCountForMedia));

            for (unsigned int i = 0; i < streamCount; i++)
            {
                streamCountForMedia[formatContext.streams[i]->codecpar->codec_type]++;
                streamsForType[formatContext.streams[i]->codecpar->codec_type] = i;
            }

            auto bestStreams = unordered_map<AVMediaType, int>();
            // for each media type, get the best stream until we have every stream
            for (unsigned int i = 0, found = 0; i < AVMEDIA_TYPE_NB, found < streamCount; i++)
            {
                // only one stream for this media type
                if (streamCountForMedia[i] == 1)
                {
                    auto mediaType = AVMediaType(i);
                    auto bestStream = BestStreamIndex(formatContext, mediaType,
                        streamsForType[mediaType]);

                    if (bestStream < 0)
                    {
                        Debug::LogWarning("AVLibPlayer::GetBestStreams: Could not find stream");
                    }

                    bestStreams[mediaType] = bestStream;
                    found++;
                }
                // multiple streams for this media type
                if (streamCountForMedia[i] > 1)
                {
                    auto mediaType = AVMediaType(i);
                    auto bestStream = BestStreamIndex(formatContext, mediaType);

                    if (bestStream < 0)
                    {
                        Debug::LogWarning("AVLibPlayer::GetBestStreams: Could not find stream");
                    }

                    bestStreams[mediaType] = bestStream;
                    found++;
                }
                // otherwise no stream for this media type, oh dear
            }

            return bestStreams;
        }
    }
}
