#pragma once
#include "Player.h"
#include "Rendering/TextureWriter.h"

namespace UnityAV
{
    using namespace Rendering;
    using namespace Media;

    /**
        * \brief Responsible for exposing a TextureWriter as an IVideoClient instance
        */
    class TextureClient : public IVideoClient
    {
    public:
        /**
            * \brief Initializes a new instance of PlayerConnector
            * \param writer The texture writer to connect
            */
        explicit TextureClient(unique_ptr<TextureWriter> writer);
        /**
            * \brief Deconstructs an instance of PlayerConnector
            */
        virtual ~TextureClient();

        PixelFormat Format() const override;
        int Width() const override;
        int Height() const override;
        void OnFrameReady(VideoFrame& frame) override;
        void Write() override;

    private:
        unique_ptr<TextureWriter> _writer;
    };
}