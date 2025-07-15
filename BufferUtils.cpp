/**********************************************************************************
    BufferUtils.cpp

                                                                LI WENHUI
                                                                2025/07/15

**********************************************************************************/

#include "BufferUtils.h"
#include "Vertex.h"

ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count) {

    D3D11_BUFFER_DESC bd = {};// Direct3D 11 でバッファの属性を記述する構造体
    bd.Usage = D3D11_USAGE_DYNAMIC;              
    bd.ByteWidth = sizeof(Vertex) * vertices_count;             // バッファのサイズ＝頂点全体のサイズ
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;     // 頂点バッファとして使用
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 

    D3D11_SUBRESOURCE_DATA initData = {};//D3D11_SUBRESOURCE_DATA はバッファ初期化用の構造体
    initData.pSysMem = vertices;// GPU上のバッファを初期化するためのデータへのポインタ

    // Direct3D 11 の頂点バッファへのポインタ、初期値は nullptr（ヌルポインタ）
    ID3D11Buffer* vertexBuffer = nullptr;
    // GPU上にバッファを作成する
    HRESULT hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    if (FAILED(hr)) {
        // エラー処理
        return nullptr;
    }
    return vertexBuffer;
}
ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device)
{
    // 矩形のインデックス配列を定義。合計6つのインデックスで、2つの三角形から矩形を描画
    // 三角形1の頂点インデックス: 0, 1, 2
    // 三角形2の頂点インデックス: 0, 2, 3
    UINT indices[6] = { 0, 1, 2, 0, 2, 3 };

    // バッファ記述構造体の初期化
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;                // デフォルトの使い方。GPUが読み書きし、CPUからアクセス不可
    bd.ByteWidth = sizeof(UINT) * 6;               // バッファのサイズ。6つのインデックス、各インデックスはUINT型
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;        // インデックスバッファとしてバインド

    // 初期化データ構造体を定義し、D3Dにどのメモリを使うか教える
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;                     // インデックス配列へのポインタ

    // インデックスバッファへのポインタを初期化（nullptr）
    ID3D11Buffer* indexBuffer = nullptr;

    // デバイスインターフェースを呼び出してインデックスバッファを作成
    HRESULT hr = device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr))
    {
        // 作成失敗時はnullptrを返す。呼び出し元で失敗を検出して処理可能
        return nullptr;
    }

    // 作成成功時はインデックスバッファのポインタを返す
    return indexBuffer;
}