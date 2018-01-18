#pragma once

#include <d3d11.h>
#include <IUnityGraphicsD3D11.h>
#include "TextureWriter.h"

namespace Rendering
{
    /**
        * \brief Responsible for the Direct3D 11 implementation of TextureWriter
        */
    class D3D11TextureWriter : public TextureWriter
    {
    public:
        /**
        * \brief Initializes a new instance of D3D11TextureWriter
        * \param device The D3D11 device
        * \param target The D3D11 target texture
        */
        explicit D3D11TextureWriter(ID3D11Device* device, ID3D11Texture2D* target);
        /**
            * \brief Deconstrucs an instance of D3D11TextureWriter
            */
        virtual ~D3D11TextureWriter();

        void Write(bool force) override;

    private:
        static const int kDefaultRGBA32BPP;
            
        ID3D11Device * _device;
        ID3D11DeviceContext * _context;
        ID3D11Texture2D * _target;
    };
}