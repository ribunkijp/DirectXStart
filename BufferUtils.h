/**********************************************************************************
    BufferUtils.h

                                                                LI WENHUI
                                                                2025/07/15

**********************************************************************************/


#include "d3dApp.h"

struct Vertex;


ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count);

ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device);