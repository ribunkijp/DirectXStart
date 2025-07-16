/**********************************************************************************
    Render.cpp

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#include "Render.h"
#include "Player.h"
#include "StateInfo.h"



void Render(HWND hwnd, StateInfo* pState) {
 

    // レンダーターゲットビューと深度/ステンシルビューをバインド
    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);

    // 背景色をクリア
    float clearColor[4] = { 1.0f, 1.0f, 0.88f, 1.0f };
    pState->context->ClearRenderTargetView(pState->rtv, clearColor);

    // 深度とステンシルバッファをクリア。1.0fは深度のデフォルトで最遠の値。
    pState->context->ClearDepthStencilView(pState->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


    // 入力レイアウトとシェーダーを設定
    /*
        GPUに頂点データの構造と読み方を伝える；

        GPUに頂点データが頂点シェーダーでどう処理されるかを伝える；

        GPUにピクセルがピクセルシェーダーでどう処理されるかを伝える。
    */
    // 入力レイアウト（Input Layout）を設定
    pState->context->IASetInputLayout(pState->inputLayout);
    // 頂点シェーダー（Vertex Shader）をパイプラインにバインド
    pState->context->VSSetShader(pState->vertexShader, nullptr, 0);
    // ピクセルシェーダー（Pixel Shader）をパイプラインにバインド
    pState->context->PSSetShader(pState->pixelShader, nullptr, 0);

    // GPUに頂点をどのようにプリミティブ（図形）として描画するかを伝える。プリミティブタイプは三角形リスト（D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST）
    pState->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // --- ブレンドステートをバインド ---
    // 2番目のパラメータは定数配列。ブレンド記述でD3D11_BLEND_BLEND_FACTORやD3D11_BLEND_INV_BLEND_FACTORを使う場合に利用。
    // 通常、アルファブレンドの場合はnullptrか全て1の配列でOK。
    // 3番目はサンプルマスク。通常は0xffffffff。
    pState->context->OMSetBlendState(pState->blendStateNormal, nullptr, 0xffffffff);

    // 透過用の深度ステンシルステートを設定！これが重要！
    // 2番目（StencilRef）はここでは重要でないので0でOK。
    pState->context->OMSetDepthStencilState(pState->depthStencilStateTransparent, 0);

    // PSSetSamplers(開始スロット, サンプラー数, サンプラー配列ポインタ)
    // s0レジスタはスロット0に対応
    pState->context->PSSetSamplers(0, 1, &pState->samplerState);

    pState->player->Render(pState->context,pState, pState->view, pState->projection);

    //for (auto& obj : pState->sceneObjects)
    //{
    //    obj->Render(pState->context, pState->view, pState->projection);
    //}

    //// バックバッファ（描画が終わったバッファ）とフロントバッファ（画面に表示されているバッファ）を交換
    pState->swapChain->Present(1, 0);

}