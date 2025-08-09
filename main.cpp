/**********************************************************************************
    main.cpp

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <ShellScalingAPI.h> //dpi
#pragma comment(lib, "Shcore.lib")//Shcore.lib の静的リンクライブラリ
#include "StateInfo.h"
#include "d3dApp.h"
#include "Timer.h"
#include "Render.h"
#include "PlayerObject.h"  
#include "UpdateAll.h"




void GetScaledWindowSizeAndPosition(float logicalWidth, float logicalHeight,
    int& outW, int& outH, int& outLeft, int& outTop, DWORD C_WND_STYLE);
inline StateInfo* GetAppState(HWND hwnd);
void UpdateViewport(ID3D11DeviceContext* context, HWND hwnd);


// ウィンドウプロシージャ関数
LRESULT CALLBACK WindowProc(
    HWND hwnd, // ウィンドウハンドル（ウィンドウの「ID」）
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance, // インスタンスハンドル（アプリ自身の「ID」、現在の実行中インスタンスを表す。）
    _In_opt_ HINSTANCE hPrevInstance, // Windows 95時代の引数、常にNULL
    _In_ PWSTR pCmdLine, // コマンドライン引数（プログラム自身のパスを含まない文字列）
    _In_ int nCmdShow // ウィンドウ表示方法
) {
    // DPI感知を設定
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    (void)hPrevInstance; // 使わないが、警告回避のため明示的に参照
    (void)pCmdLine;      // 同上

    const wchar_t CLASS_NAME[] = L"Window";

    //登録するウィンドウクラス
    WNDCLASS wc = { };
    // ウィンドウメッセージ処理関数を設定
    wc.lpfnWndProc = WindowProc;
    // ウィンドウ所属アプリインスタンスを設定
    wc.hInstance = hInstance;
    // ウィンドウクラス名
    wc.lpszClassName = CLASS_NAME;

    // ウィンドウクラスを登録
    RegisterClass(&wc);


    // pState ポインタは全Direct3D描画状態を保存
    StateInfo* pState = new StateInfo(); 
    if (pState == NULL) {
        return 0;
    }

    //
    const DWORD C_WND_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    int winW = 0, winH = 0, winL = 0, winT = 0;
    GetScaledWindowSizeAndPosition(
        pState->logicalWidth,
        pState->logicalHeight,
        winW, winH, winL, winT, C_WND_STYLE);

    // 
    HWND hwnd = CreateWindowEx(
        0,                              // オプションのウィンドウスタイル
        CLASS_NAME,                     
        L"ホラーランニング",                   
        C_WND_STYLE,            
        winL,  
        winT, 
        winW,
        winH,
        NULL,       // 親ウィンドウ
        NULL,       // メニュー
        hInstance,  // インスタンスハンドル
        pState      // 追加アプリデータ ← これはlpParam、WM_CREATEで取得可能
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    //ShowWindow(hwnd, nCmdShow);
    ShowWindow(hwnd, SW_SHOWNORMAL);

    //
    RECT rect;
    GetClientRect(hwnd, &rect);
    auto clientWidth = static_cast<float>(rect.right - rect.left);
    auto clientHeight = static_cast<float>(rect.bottom - rect.top);

    if (!InitD3D(hwnd, pState->device, pState, clientWidth, clientHeight)) {
        MessageBox(hwnd, L"D3D 初始化失败!", L"错误", MB_OK);
        return 0;
    }

    //
    Timer timer;       // タイマーオブジェクト
    timer.Reset();     // プログラム起動時に一度だけ呼び出す

    //
    MSG msg = {};
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return 0;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		timer.Tick();    // 毎フレーム呼び出す
		float deltaTime = timer.GetDeltaTime();  

		bool leftPressed = (GetAsyncKeyState('A') & 0x8000) != 0;
		bool rightPressed = (GetAsyncKeyState('D') & 0x8000) != 0;
		bool spacePressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

        //UpdateAll(pState, deltaTime, leftPressed, rightPressed, spacePressed);



        UpdatePlayer(pState, deltaTime);

        Render(hwnd, pState);
    
    }

    return 0;

}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam, // 符号なし整数型
    LPARAM lParam  // 符号付き整数またはポインタ
) {
    // StateInfo構造体ポインタを保存（構造体 = データの集合体）
    StateInfo* pState = nullptr;

    if (uMsg == WM_CREATE)
    {
        // CreateWindowExで渡したpStateをウィンドウに保存
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pState);
    }
    else
    {
        pState = GetAppState(hwnd);
    }

    switch (uMsg) {
    case WM_CLOSE:
        //if (MessageBox(hwnd, L"really quit？", L"cancel", MB_OKCANCEL) == IDOK) {
        DestroyWindow(hwnd);
        //}
        return 0;

    case WM_DESTROY:
        if (pState) { // pStateが有効なポインタか確認
            CleanupD3D(pState);
            delete pState;
            pState = nullptr;
        }
        PostQuitMessage(0);

        return 0;
    case WM_SIZE:
    {
        //
        pState = GetAppState(hwnd);
        if (pState && pState->context) {
            // ビューポート更新
            UpdateViewport(pState->context, hwnd);

            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            if (width > 0 && height > 0) {
                OnResize(hwnd, pState, width, height);
            }
        }

        return 0;
    }

    }
    // switch文で明示的に処理しなかったメッセージは、デフォルトウィンドウプロシージャへ
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



inline StateInfo* GetAppState(HWND hwnd)
{
    // 指定したウィンドウ（hwnd）から以前SetWindowLongPtrで保存したカスタムポインタやデータを取り出す
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    StateInfo* pState = reinterpret_cast<StateInfo*>(ptr);
    return pState;
}
void UpdateViewport(ID3D11DeviceContext* context, HWND hwnd)
{
    if (!context) {
        OutputDebugStringA("UpdateViewport: context is nullptr!\n");
        return;
    }
    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);
}
void GetScaledWindowSizeAndPosition(float logicalWidth, float logicalHeight,
    int& outW, int& outH, int& outLeft, int& outTop, DWORD C_WND_STYLE)
{
    // --- メインディスプレイのDPI取得 ---
    HMONITOR monitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
    UINT dpiX = 96, dpiY = 96;
    GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

    // --- DPIに基づきクライアント領域の物理ピクセルサイズを計算 ---
    float dpiScale = static_cast<float>(dpiX) / 96.0f;
    int scaledClientW = static_cast<int>(logicalWidth * dpiScale);
    int scaledClientH = static_cast<int>(logicalHeight * dpiScale);

    // --- クライアントサイズとウィンドウスタイルに基づき、ウィンドウ全体の物理ピクセルサイズを計算 ---
    RECT rect = { 0, 0, scaledClientW, scaledClientH };
    AdjustWindowRectExForDpi(&rect, C_WND_STYLE, FALSE, 0, dpiX);
    outW = rect.right - rect.left;
    outH = rect.bottom - rect.top;

    // 作業領域を取得
    MONITORINFO mi = { sizeof(mi) };


    int workW = 0, workH = 0;
    if (GetMonitorInfo(monitor, &mi))
    {
        workW = mi.rcWork.right - mi.rcWork.left;
        workH = mi.rcWork.bottom - mi.rcWork.top;
    }
    else
    {
        workW = GetSystemMetricsForDpi(SM_CXSCREEN, dpiX);
        workH = GetSystemMetricsForDpi(SM_CYSCREEN, dpiX);
    }

    // ---- ウィンドウサイズが画面を超えないように制限 ----
    float wScale = static_cast<float>(workW) / static_cast<float>(outW);
    float hScale = static_cast<float>(workH) / static_cast<float>(outH);
    float scale = (wScale < hScale) ? wScale : hScale;
    if (scale < 1.0f) { // 縮小のみ、拡大しない
        outW = static_cast<int>(static_cast<float>(outW) * scale);
        outH = static_cast<int>(static_cast<float>(outH) * scale);
    }

    // 再度中央寄せ（ディスプレイrcMonitor領域基準、必ずはみ出さない）
    outLeft = mi.rcWork.left + (workW - outW) / 2;
    outTop = mi.rcWork.top + (workH - outH) / 2;
}
