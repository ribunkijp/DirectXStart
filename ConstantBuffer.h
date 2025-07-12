/**********************************************************************************
    ConstantBuffer.h

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/

#ifndef CONSTANTBUFFER_H
#define CONSTANTBUFFER_H

#include <DirectXMath.h>

struct ConstantBuffer
{
    DirectX::XMMATRIX world;      // 世界矩阵
    DirectX::XMMATRIX view;       // 视图矩阵
    DirectX::XMMATRIX proj;       // 投影矩阵
};


#endif