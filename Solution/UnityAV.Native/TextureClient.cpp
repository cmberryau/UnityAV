#include "stdafx.h"
#include "TextureClient.h"

namespace UnityAV
{
    TextureClient::TextureClient(unique_ptr<TextureWriter> writer) 
        : _writer(move(writer))
    {
            
    }

    TextureClient::~TextureClient()
    {
            
    }

    PixelFormat TextureClient::Format() const
    {
        return _writer->Format();
    }

    int TextureClient::Width() const
    {
        return _writer->Width();
    }

    int TextureClient::Height() const
    {
        return _writer->Height();
    }

    void TextureClient::OnFrameReady(VideoFrame& frame)
    {
        _writer->Read(frame.Buffers());
    }

    void TextureClient::Write()
    {
        _writer->Write(false);
    }
}