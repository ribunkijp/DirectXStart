/**********************************************************************************
    StateInfo.h

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/


#ifndef STATEINFO_H
#define STATEINFO_H

#include <d3d11.h>//ID3D11Device
#include <DirectXMath.h>
#include <memory>

struct PlayerObject;

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = nullptr; } }


struct StateInfo {
    // D3D 设备与上下文
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;

    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr; 
    ID3D11SamplerState* samplerState = nullptr;
    ID3D11DepthStencilState* depthStencilStateTransparent = nullptr;
    ID3D11DepthStencilView* depthStencilView = nullptr;


	ID3D11BlendState* blendStateNormal = nullptr;
	ID3D11BlendState* blendStateAdditive = nullptr;
	ID3D11BlendState* blendStateMultiply = nullptr;
	ID3D11BlendState* blendStateScreen = nullptr;



    //
    float logicalWidth = 1888.0f;
    float logicalHeight = 1062.0f;


    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX projection = DirectX::XMMatrixIdentity();


    std::unique_ptr<PlayerObject> player;

   
    ~StateInfo();
};


#endif