/**********************************************************************************
    d3dApp.cpp

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/


#include "d3dApp.h"
#include "ConstantBuffer.h"


bool InitD3D(HWND hwnd, StateInfo* pState, float clientWidth, float clientHeight) {
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

    // 頂点シェーダ（VS）コンパイル
    hr = D3DCompileFromFile(
        L"shader.hlsl",                 // HLSLファイルパス
        nullptr, nullptr,
        "VSMain", "vs_5_0",
        0, 0,
        &vsBlob, nullptr
    );
    if (FAILED(hr)) return false;
    // ピクセルシェーダ（PS）コンパイル
    hr = D3DCompileFromFile(
        L"shader.hlsl",
        nullptr, nullptr,
        "PSMain", "ps_5_0",
        0, 0,
        &psBlob, nullptr
    );
    if (FAILED(hr)) {
        vsBlob->Release();
        return false;
    }

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
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0},  
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0},  
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 40,  D3D11_INPUT_PER_VERTEX_DATA, 0},  
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

    hr = pState->device->CreateBlendState(&blendDesc, &pState->blendState);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create blend state.", L"Error", MB_OK);
        return false;
    }

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

    return true; // 成功時はtrue
}




// Direct3D リソース解放関数
void CleanupD3D(StateInfo* pState) {

    if (!pState) return;

    // StateInfoが持つD3Dリソースを全て解放
    if (pState->samplerState) { // サンプラーステート解放
        pState->samplerState->Release();
        pState->samplerState = nullptr;
    }
    if (pState->pixelShader) { // ピクセルシェーダ解放
        pState->pixelShader->Release();
        pState->pixelShader = nullptr;
    }
    if (pState->vertexShader) { // 頂点シェーダ解放
        pState->vertexShader->Release();
        pState->vertexShader = nullptr;
    }
    if (pState->inputLayout) { // 入力レイアウト解放
        pState->inputLayout->Release();
        pState->inputLayout = nullptr;
    }
    if (pState->rtv) { // レンダーターゲットビュー解放
        pState->rtv->Release();
        pState->rtv = nullptr;
    }
    if (pState->blendState) { // ブレンドステート解放
        pState->blendState->Release();
        pState->blendState = nullptr;
    }


    // デバイスコンテキスト・スワップチェーン解放前に、全操作完了を確認
    // 通常はデバイス解放前にまずコンテキストを解放し、未完了の操作がないことを確認
    if (pState->context) {
        pState->context->Release();
        pState->context = nullptr;
    }

    if (pState->swapChain) {
        pState->swapChain->Release();
        pState->swapChain = nullptr;
    }

    // 最後にデバイス本体の解放
    // デバッグ時は未解放のCOMインターフェースがないか確認
    if (pState->device) {

        // 【重要修正】デバッグコードは device有効のときのみ
#ifdef _DEBUG
        ID3D11Debug* debug = nullptr;
        if (SUCCEEDED(pState->device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug)))
        {
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
            debug->Release();
        }
#endif
        // そしてデバイス本体の解放
        pState->device->Release();
        pState->device = nullptr;
    }

}


// ウィンドウサイズ変更時の処理
void OnResize(HWND hwnd, StateInfo* pState, UINT width, UINT height)
{
    if (!pState || !pState->swapChain || !pState->device || !pState->context) return;

    // 既存リソースを解放
    if (pState->rtv) { pState->rtv->Release(); pState->rtv = nullptr; }
    if (pState->depthStencilView) { pState->depthStencilView->Release(); pState->depthStencilView = nullptr; }

    // スワップチェーンのバッファをリサイズ
    pState->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    // 新しいバックバッファ取得
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

    // 新しい深度バッファ作成
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

    // 深度ビュー作成
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

    // 新しいレンダーターゲットを再バインド
    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);

    //    ClearRenderTargetView でウィンドウ全体をクリア
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    pState->context->RSSetViewports(1, &vp);
    // この方法が推奨


    pState->projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, pState->logicalWidth,
        pState->logicalHeight, 0.0f,
        0.0f, 1.0f);

}