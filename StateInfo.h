/**********************************************************************************
    StateInfo.h

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/


#ifndef STATEINFO_H
#define STATEINFO_H

#include <d3d11.h>//ID3D11Device
#include <DirectXMath.h>
#include "Player.h"


struct StateInfo {
    // デバイスオブジェクト：リソースやシェーダーの作成に使う
    ID3D11Device* device = nullptr; //ID3D11Device（グラフィックカードインターフェース）
    // デバイスコンテキスト：描画コマンドやリソースのバインドに使用 //D3D11DeviceContext（コマンドインターフェース）
    ID3D11DeviceContext* context = nullptr;//レンダリングパイプライン
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;// レンダーターゲットビュー：描画対象バッファのビューを指定
    ID3D11InputLayout* inputLayout = nullptr;// 入力レイアウト：頂点データの構造をGPUに伝える設定
    ID3D11VertexShader* vertexShader = nullptr;// 頂点シェーダー：各頂点の処理を行うシェーダー
    ID3D11PixelShader* pixelShader = nullptr; // ピクセルシェーダー：各ピクセルの色を決定するシェーダー
    ID3D11SamplerState* samplerState = nullptr;// テクスチャサンプリング用のサンプラーステート
    ID3D11BlendState* blendState = nullptr;// 透過ブレンド用
    ID3D11DepthStencilState* depthStencilStateTransparent = nullptr;// 透過物体用の深度ステンシル状態
    // 深度/ステンシルバッファビューのインターフェースポインタ。GPUが深度/ステンシルバッファへアクセス・操作するためのもの
    ID3D11DepthStencilView* depthStencilView = nullptr;
    //
    ID3D11RenderTargetView* renderTargetView = nullptr;
    //
    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX projection = DirectX::XMMatrixIdentity();

    float logicalWidth = 1888.0f;
    float logicalHeight = 1062.0f;

    Player* player = nullptr;

};


#endif