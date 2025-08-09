/**********************************************************************************
    TextureLoader.h

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <DirectXTex.h>





HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv, float* outWidth = nullptr,
    float* outHeight = nullptr);


#endif