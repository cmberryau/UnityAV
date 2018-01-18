#include "stdafx.h"
#include "NullTextureWriter.h"

namespace Rendering
{
    const int NullTextureWriter::kDefaultBPP = 4;

    NullTextureWriter::NullTextureWriter(int width, int height)
    {
        // set the required variables
        TargetWidth = width;
        TargetHeight = height;
        TargetFormat = PIXEL_FORMAT_NONE;

        // create the buffers
        BufferSizes.push_back(TargetWidth * TargetHeight * kDefaultBPP);
        BufferStrides.push_back(TargetWidth * kDefaultBPP);
        Buffers.push_back(unique_ptr<uint8_t>(new uint8_t[BufferSizes[0]]));
        
        // mark as ready
        Ready.store(true);
    }

    NullTextureWriter::~NullTextureWriter()
    {
            
    }

    void NullTextureWriter::Write(bool force)
    {

    }
}