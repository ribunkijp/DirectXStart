/**********************************************************************************
    d3dApp.h

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/

#ifndef D3DAPP_H
#define D3DAPP_H


#include <new>
#include <d3d11.h>//ID3D11Device
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")//Direct3D 11 の静的リンクライブラリ
#pragma comment(lib, "d3dcompiler.lib")//D3DCompiler API の静的リンクライブラリ
#include <DirectXMath.h>
#include "StateInfo.h"


//
// D3Dを初期化する関数
bool InitD3D(HWND hwnd, StateInfo* state, float clientWidth, float clientHeight);

//
// D3Dのリソースを解放する関数
void CleanupD3D(StateInfo* state);

//
// ウィンドウサイズ変更時の処理
void OnResize(HWND hwnd, StateInfo* state, UINT width, UINT height);


#endif  