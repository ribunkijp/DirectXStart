/**********************************************************************************
    TextureLoader.h

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <DirectXTex.h>
#include <spine/TextureLoader.h>
#include <spine/Atlas.h>
#include <string>




HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv, float* outWidth = nullptr,
    float* outHeight = nullptr);


class SpineTextureLoader : public spine::TextureLoader {
public:
    SpineTextureLoader(ID3D11Device* device);
    void load(spine::AtlasPage& page, const spine::String& pat) override;
    void unload(void* texture) override;
private:
    ID3D11Device* m_device;
    std::wstring ToWString(const char* utf8);
};



#endif