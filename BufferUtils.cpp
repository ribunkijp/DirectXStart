/**********************************************************************************
    BufferUtils.cpp

                                                                LI WENHUI
                                                                2025/07/15

**********************************************************************************/

#include "BufferUtils.h"
#include "Vertex.h"

ID3D11Buffer* CreateDynamicVertexBuffer(ID3D11Device* device, size_t vertices_count) {

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;              
    bd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices_count); 
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; 
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    // 
    ID3D11Buffer* vertexBuffer = nullptr;
    //
    HRESULT hr = device->CreateBuffer(&bd, nullptr, &vertexBuffer);
    if (FAILED(hr)) {
        // 
        return nullptr;
    }
    return vertexBuffer;
}
ID3D11Buffer* CreateDynamicIndexBuffer(ID3D11Device* device, size_t indexCount)
{
    // 
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = static_cast<UINT>(indexCount * sizeof(UINT));              
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    // 
    ID3D11Buffer* indexBuffer = nullptr;
    // 
    HRESULT hr = device->CreateBuffer(&bd, nullptr, &indexBuffer);
    if (FAILED(hr))
    {
        return nullptr;
    }
    //
    return indexBuffer;
}