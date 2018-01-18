#pragma once

#include "TextureWriter.h"

namespace Rendering
{
    /**
        * \brief Responsible for the /dev/null implementation of TextureWriter
        */
    class NullTextureWriter : public TextureWriter
    {
    public:
        /**
            * \brief Initializes a new instance of NullTextureWriter
            * \param width The width of the texture
            * \param height The height of the texture
            */
        explicit NullTextureWriter(int width, int height);
        virtual ~NullTextureWriter();

        void Write(bool force) override;

    private:
        static const int kDefaultBPP;
    };
}