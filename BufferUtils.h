/**********************************************************************************
    BufferUtils.h

                                                                LI WENHUI
                                                                2025/07/15

**********************************************************************************/


#include "d3dApp.h"

struct Vertex;


ID3D11Buffer* CreateDynamicVertexBuffer(ID3D11Device* device, size_t vertices_count);

ID3D11Buffer* CreateDynamicIndexBuffer(ID3D11Device* device, size_t indexCount);