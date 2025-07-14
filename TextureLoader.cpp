/**********************************************************************************
    TextureLoader.cpp

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#include "TextureLoader.h"

HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv, float* outWidth, float* outHeight) {
    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratchImage;
    HRESULT hr;

    // DDSファイルからテクスチャを読み込もうとする
    //hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);
    //png
    hr = DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);

    if (FAILED(hr)) {
        // 読み込みに失敗した場合、通常はファイルが存在しないか、フォーマットが正しくない
        // ここでMessageBoxやログ出力を追加して、デバッグしやすくすることも可能
        // MessageBox(nullptr, L"Failed to load DDS texture.", filename, MB_OK);

        return hr;
    }

    // 呼び出し元が幅と高さを取得したい場合、値を代入する
    if (outWidth)  *outWidth = static_cast<float>(metadata.width);
    if (outHeight) *outHeight = static_cast<float>(metadata.height);

    // 読み込んだ画像データからShader Resource Viewを作成
    // この関数は自動的にmipmapsやテクスチャフォーマットの変換を行う
    hr = DirectX::CreateShaderResourceView(device, scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, srv);
    return hr;
}

SpineTextureLoader::SpineTextureLoader(ID3D11Device* device) : m_device(device) {

}

void SpineTextureLoader::load(spine::AtlasPage& page, const spine::String& path) {
    std::wstring wpath = ToWString(path.buffer());
    ID3D11ShaderResourceView* srv = nullptr;
    float width = 0, height = 0;
    HRESULT hr = LoadTextureAndCreateSRV(m_device, wpath.c_str(), &srv, &width, &height);
    page.texture = srv;         
    page.width = (int)width;
    page.height = (int)height;
}
void SpineTextureLoader::unload(void* texture) {
    if (texture) ((ID3D11ShaderResourceView*)texture)->Release();
}

std::wstring SpineTextureLoader::ToWString(const char* utf8) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &ws[0], len);
    return ws;
}