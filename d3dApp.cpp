/**********************************************************************************
    d3dApp.cpp

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/


#include "d3dApp.h"
#include "ConstantBuffer.h"
#include "PlayerObject.h"
#include <vector>
#include <memory>


bool InitD3D(HWND hwnd, ID3D11Device* device, StateInfo* pState, float clientWidth, float clientHeight) {
    /*
       元のモデル座標（モデル中心が原点）
            ↓ worldMatrix（平行移動・拡大縮小）
       ワールド座標（左上が原点）
            ↓ viewMatrix（単位行列・カメラなし）
       ビュー座標
            ↓ projectionMatrix（正射投影）
       NDC座標（X/Y は [-1, 1]、左上は -1,+1 に）
            ↓ グラフィックスパイプラインによる自動マッピング
       画面ピクセル位置
   */

   // カメラ位置と向きを設定
    pState->view = DirectX::XMMatrixIdentity(); // まずは単位行列、カメラ位置を設定したら更新できる
    // 3D/2D ワールドを画面にマッピング
    pState->projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, pState->logicalWidth,      // left から right：X軸は左から右へ
        pState->logicalHeight, 0.0f,     // bottom から top：Y軸は上から下へ
        0.0f, 1.0f              // near から far：Z軸は手前から奥へ
    );

    // DXGI_SWAP_CHAIN_DESC はスワップチェーンの設定用構造体です。
    // バッファ数、解像度、フォーマット、ウィンドウハンドル、フルスクリーン/ウィンドウ、アンチエイリアス等を設定。
    // この構造体は D3D11CreateDeviceAndSwapChain 関数に渡され、
    // 以下の主要オブジェクトが同時に作成されます：
    // - ID3D11Device（GPUデバイス、リソース作成用）
    // - ID3D11DeviceContext（デバイスコンテキスト、コマンド送信用）
    // - IDXGISwapChain（スワップチェーン、バッファ管理）
    DXGI_SWAP_CHAIN_DESC scd = {};

    // バックバッファ数（1つのみ使用）
    scd.BufferCount = 1;

    // 描画画面の幅・高さ（解像度）
    scd.BufferDesc.Width = static_cast<unsigned>(clientWidth);
    scd.BufferDesc.Height = static_cast<unsigned>(clientHeight);

    // バッファのカラーフォーマット（RGBA・各8bit 標準フォーマット）
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // バッファの用途（レンダーターゲットとして利用）
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // 出力先ウィンドウハンドル
    scd.OutputWindow = hwnd;

    // マルチサンプリング   アンチエイリアス  （1: 無効）
    scd.SampleDesc.Count = 1;

    // ウィンドウモードを有効に（TRUE: ウィンドウ表示）
    scd.Windowed = TRUE;

    // Direct3D デバイス・スワップチェーン・コマンドコンテキスト作成
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,                        // デフォルトGPU（アダプタ）を使用
        D3D_DRIVER_TYPE_HARDWARE,      // ハードウェアアクセラレーション（最速・デフォルト）
        nullptr,                        // ソフトウェアレンダDLLはなし（ハードのみ）
        0,                              // デバッグフラグなし（リリース時は0）
        nullptr, 0,                     // 機能レベル未指定（自動選択）
        D3D11_SDK_VERSION,             // Direct3D SDK バージョン
        &scd,                          // 準備済みスワップチェーン設定（DXGI_SWAP_CHAIN_DESC）
        &pState->swapChain,             // 出力：スワップチェーン
        &pState->device,                // 出力：デバイス
        nullptr,                       // 機能レベル不要
        &pState->context)))            // 出力：コマンドコンテキスト
        return false; // 失敗時はfalse返却


    // バックバッファテクスチャ（描画先）の取得
    ID3D11Texture2D* backBuffer = nullptr;
    /*
        1つ目の引数0はバッファ番号・最初のバッファ;
        2つ目 uuidof(ID3D11Texture2D)で求めるインターフェース型を指定;
        3つ目 (void**)&backBuffer は出力用、GetBufferがここに取得先を書き込む
        バックバッファのテクスチャを取ることで、後でレンダーターゲットビュー作成やGPUへの描画先指示が可能となる。
    */
    HRESULT hr = pState->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr) || backBuffer == nullptr) {
        return false; // 失敗時は初期化中断
    }
    // バックバッファ用レンダーターゲットビューの作成。RTVでGPUの描画先をバックバッファに指定
    hr = pState->device->CreateRenderTargetView(backBuffer, nullptr, &pState->rtv);
    if (FAILED(hr)) {
        return false; // 失敗もチェック
    }
    // バックバッファ解放（ビュー作成済みなのでOK）
    backBuffer->Release();



    // 深度/ステンシルバッファ
    ID3D11Texture2D* depthStencilBuffer = nullptr; // 深度・ステンシル用の2Dテクスチャポインタ
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};     // 2Dテクスチャの設定構造体を初期化
    depthBufferDesc.Width = static_cast<unsigned>(clientWidth);      // バッファの幅（クライアント領域と同じ）
    depthBufferDesc.Height = static_cast<unsigned>(clientHeight);    // バッファの高さ（クライアント領域と同じ）
    depthBufferDesc.MipLevels = 1;                                  // ミップマップレベルは1（不要なので1）
    depthBufferDesc.ArraySize = 1;                                  // 配列数は1（単一バッファ）
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;         // 24ビット深度＋8ビットステンシル形式
    depthBufferDesc.SampleDesc.Count = 1;                           // サンプル数（MSAA無効なら1、RTVと同じ値）
    depthBufferDesc.SampleDesc.Quality = 0;                         // サンプル品質（通常0、RTVと同じ値）
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;                    // 標準的な使い方（GPUで読み書き）
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;           // バインド用途：深度/ステンシルバッファ
    depthBufferDesc.CPUAccessFlags = 0;                             // CPUアクセス不要（0）
    depthBufferDesc.MiscFlags = 0;                                  // その他フラグなし（0）

    hr = pState->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer); // 深度/ステンシル用の2Dテクスチャを作成
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil buffer.", L"Error", MB_OK);     // 作成失敗時はエラーメッセージ
        return false;
    }

    // 深度/ステンシルビュー
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};                       // 深度ステンシルビューの設定構造体を初期化
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;                   // ビューのフォーマット（バッファと同じ形式）
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;            // 2Dテクスチャとして使用
    dsvDesc.Texture2D.MipSlice = 0;                                   // ミップマップレベルは0のみ

    hr = pState->device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &pState->depthStencilView); // 深度/ステンシルビューを作成
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil view.", L"Error", MB_OK);   // 作成失敗時はエラーメッセージ
        if (depthStencilBuffer) depthStencilBuffer->Release();                        // バッファ解放
        return false;
    }
    if (depthStencilBuffer) depthStencilBuffer->Release(); // ビュー作成後はバッファ本体を解放（Direct3D内部で参照カウント維持）


    // 作成したレンダーターゲットビューをGPUに設定（描画先の指定）
    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);

    // ビューポート（描画範囲）の設定
    D3D11_VIEWPORT vp = {};                // ビューポート（画面内の描画エリア）構造体を初期化
    vp.Width = clientWidth;                 // ビューポート幅（ウィンドウの幅と同じ）
    vp.Height = clientHeight;               // ビューポート高さ（ウィンドウの高さと同じ）
    vp.MinDepth = 0.0f;                     // 最小深度値（通常0.0f）
    vp.MaxDepth = 1.0f;                     // 最大深度値（通常1.0f）
    pState->context->RSSetViewports(1, &vp); // このビューポート情報をGPUに設定（ラスタライザステージ用）

   
    /*
       .hlsl ソースをGPU用のバイトコードにコンパイル
   */
   // 頂点シェーダ・ピクセルシェーダのコンパイル
    ID3DBlob* vsBlob = nullptr;         // 頂点シェーダバイナリ格納
    ID3DBlob* psBlob = nullptr;         // ピクセルシェーダバイナリ格納
    ID3DBlob* errorBlob = nullptr;

	// 顶点シェーダ（VS）コンパイル
	hr = D3DCompileFromFile(
		L"shader.hlsl",
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, // <--- 添加调试和严格模式标志
		0,
		&vsBlob,
		&errorBlob // <--- 传入 errorBlob 的地址来捕获错误
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			// 如果有错误信息，就显示出来
			const char* errorMsg = (const char*)errorBlob->GetBufferPointer();
			wchar_t wideErrorMsg[1024];
			mbstowcs_s(nullptr, wideErrorMsg, 1024, errorMsg, _TRUNCATE);
			MessageBox(hwnd, wideErrorMsg, L"Vertex Shader Compilation Error", MB_OK);
			errorBlob->Release();
		}
		else {
			// 如果没有错误信息，很可能是文件没找到
			MessageBox(hwnd, L"Failed to compile vertex shader. File not found?", L"Error", MB_OK);
		}
		return false;
	}
    // ピクセルシェーダ（PS）コンパイル
	hr = D3DCompileFromFile(
		L"shader.hlsl",
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
		0,
		&psBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			const char* errorMsg = (const char*)errorBlob->GetBufferPointer();
			wchar_t wideErrorMsg[1024];
			mbstowcs_s(nullptr, wideErrorMsg, 1024, errorMsg, _TRUNCATE);
			MessageBox(hwnd, wideErrorMsg, L"Pixel Shader Compilation Error", MB_OK);
			errorBlob->Release();
		}
		else {
			MessageBox(hwnd, L"Failed to compile pixel shader. File not found?", L"Error", MB_OK);
		}
		if (vsBlob) vsBlob->Release(); // 别忘了释放已成功的vsBlob
		return false;
	}

	// 如果编译成功，释放 errorBlob (如果它被创建过)
	if (errorBlob) errorBlob->Release();

    // シェーダオブジェクト作成（GPUで使える形式に変換）
    hr = pState->device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &pState->vertexShader
    );
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        MessageBox(hwnd, L"d3dApp.cpp 198", L"错误", MB_OK);
        return false;
    }
    hr = pState->device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &pState->pixelShader
    );
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }
    //入力レイアウト作成
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

    hr = pState->device->CreateInputLayout(
        layout,
        ARRAYSIZE(layout),  
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &pState->inputLayout
    );
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr)) return false;

    // 定数バッファ（Constant Buffer）
    D3D11_BUFFER_DESC cbd = {};// 記述構造体を初期化し、GPUにどんなバッファを作るか伝える
    cbd.Usage = D3D11_USAGE_DYNAMIC;// 用途はDYNAMIC：CPUが毎フレーム値を更新（例：行列）、GPUが読み取る
    cbd.ByteWidth = sizeof(ConstantBuffer);// サイズはConstantBuffer構造体と同じ
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;// このバッファは定数バッファとして使う
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// CPU書き込み許可（D3D11_USAGE_DYNAMICとセットで必要）
    cbd.MiscFlags = 0;// 高度な用途は0
    cbd.StructureByteStride = 0;

    // --- テクスチャサンプラーステート作成 ---
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 線形フィルタ・一般用途向き
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // U座標範囲外でリピート
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // V座標範囲外でリピート
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // W座標範囲外でリピート（2Dはあまり影響なし）
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pState->device->CreateSamplerState(&sampDesc, &pState->samplerState);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create sampler state.", L"Error", MB_OK);
        return false;
    }

    // --- 透明ブレンドステート作成 ---
    D3D11_BLEND_DESC blendDesc = {};
    // Normal
    // RenderTarget[0] は最初のレンダーターゲット
    blendDesc.RenderTarget[0].BlendEnable = TRUE; // ブレンド有効
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // ソースのα値
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 1-ソースのα値
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // ソース+デスティネーション
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // α成分（通常1）
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // α成分（通常0）
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // α加算
    // D3D11_COLOR_WRITE_ENABLE_ALL は全色成分(RGBA)書込可
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    pState->device->CreateBlendState(&blendDesc, &pState->blendStateNormal);
 
    // Additive
	D3D11_BLEND_DESC blendDescAdd = blendDesc;
	blendDescAdd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDescAdd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDescAdd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	pState->device->CreateBlendState(&blendDescAdd, &pState->blendStateAdditive);

	// Multiply
	D3D11_BLEND_DESC blendDescMul = blendDesc;
	blendDescMul.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
	blendDescMul.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDescMul.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	pState->device->CreateBlendState(&blendDescMul, &pState->blendStateMultiply);

	// Screen
	D3D11_BLEND_DESC blendDescScreen = blendDesc;
	blendDescScreen.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
	blendDescScreen.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDescScreen.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	pState->device->CreateBlendState(&blendDescScreen, &pState->blendStateScreen);

    
    
    // 透明物体用の深度/ステンシルステート作成
    D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = {};
    transparentDepthStencilDesc.DepthEnable = TRUE; // 深度テストは有効（不透明物体との比較）
    //// **深度書き込み無効化** 透明物体が奥の物体のZ値を「塗りつぶす」のを防ぐ
    transparentDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    transparentDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    transparentDepthStencilDesc.StencilEnable = FALSE;

    hr = pState->device->CreateDepthStencilState(&transparentDepthStencilDesc, &pState->depthStencilStateTransparent);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create transparent depth stencil state.", L"Error", MB_OK);
        return false;
    }



    std::vector<AnimationData> animationData;
	animationData.push_back({
	  L"assets\\player_idle.png",
	  10,
	  5,
	  2,
	  10.0f
	});
	animationData.push_back({
	  L"assets\\player_run.png",
	  8,
	  4,
	  2,
	  24.0f
	});

	pState->player = std::make_unique<PlayerObject>();
    pState->player->SetSpeed(200.0f);
    pState->player->SetPos(200.0f, 600.0f);
    pState->player->Load(
        pState->device,
        pState->context,
		576.0f,
		256.0f,
		animationData,
        true
	);
 




    

    return true; // 成功時はtrue
}




void CleanupD3D(StateInfo* s) {
	if (!s) return;

	// 销毁玩家对象
	if (s->player) s->player.reset();

	// 释放各类状态/视图等（OM/DS/采样器/着色器/布局/RTV）
	if (s->blendStateScreen) { s->blendStateScreen->Release();   s->blendStateScreen = nullptr; }
	if (s->blendStateMultiply) { s->blendStateMultiply->Release(); s->blendStateMultiply = nullptr; }
	if (s->blendStateAdditive) { s->blendStateAdditive->Release(); s->blendStateAdditive = nullptr; }
	if (s->blendStateNormal) { s->blendStateNormal->Release();   s->blendStateNormal = nullptr; }

	if (s->depthStencilStateTransparent) { s->depthStencilStateTransparent->Release(); s->depthStencilStateTransparent = nullptr; }
	if (s->depthStencilView) { s->depthStencilView->Release();             s->depthStencilView = nullptr; }

	if (s->samplerState) { s->samplerState->Release(); s->samplerState = nullptr; }
	if (s->pixelShader) { s->pixelShader->Release();  s->pixelShader = nullptr; }
	if (s->vertexShader) { s->vertexShader->Release(); s->vertexShader = nullptr; }
	if (s->inputLayout) { s->inputLayout->Release();  s->inputLayout = nullptr; }
	if (s->rtv) { s->rtv->Release();          s->rtv = nullptr; }

	// 解绑并清空管线状态，释放 context
	if (s->context) {
		s->context->OMSetRenderTargets(0, nullptr, nullptr);
		s->context->ClearState();
		s->context->Flush();
		s->context->Release();
		s->context = nullptr;
	}

	// 如果可能进过全屏，先切回窗口模式再释放 swapChain
	if (s->swapChain) {
		s->swapChain->SetFullscreenState(FALSE, nullptr);
		s->swapChain->Release();
		s->swapChain = nullptr;
	}

#ifdef _DEBUG
	if (s->device) {
		ID3D11Debug* debug = nullptr;
		if (SUCCEEDED(s->device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug))) {
			debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			debug->Release();
		}
	}
#endif

	if (s->device) { s->device->Release(); s->device = nullptr; }
}


// 
void OnResize(HWND hwnd, StateInfo* pState, UINT width, UINT height)
{
    if (!pState || !pState->swapChain || !pState->device || !pState->context) return;

    if (pState->rtv) { pState->rtv->Release(); pState->rtv = nullptr; }
    if (pState->depthStencilView) { pState->depthStencilView->Release(); pState->depthStencilView = nullptr; }
 
    pState->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = pState->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (SUCCEEDED(hr) && backBuffer) {
        pState->device->CreateRenderTargetView(backBuffer, nullptr, &pState->rtv);
        backBuffer->Release();
    }
    else {
        MessageBox(hwnd, L"Failed to get back buffer during resize.", L"Error", MB_OK);
        return;
    }

    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = width;
    depthBufferDesc.Height = height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthStencilBuffer = nullptr;
    hr = pState->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthBufferDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    if (SUCCEEDED(hr) && depthStencilBuffer) {
        pState->device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &pState->depthStencilView);
        depthStencilBuffer->Release();
    }
    else {
        MessageBox(hwnd, L"Failed to create depth stencil buffer during resize.", L"Error", MB_OK);
        return;
    }

    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    pState->context->RSSetViewports(1, &vp);

    pState->projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, pState->logicalWidth,
        pState->logicalHeight, 0.0f,
        0.0f, 1.0f);

}