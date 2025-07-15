/**********************************************************************************
    ConstantBuffer.h

                                                                LI WENHUI
                                                                2025/07/12

**********************************************************************************/

#ifndef CONSTANTBUFFER_H
#define CONSTANTBUFFER_H

#include <DirectXMath.h>

struct ConstantBuffer {
    DirectX::XMMATRIX model;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    float texOffset[2];
    float texScale[2];
    float padding[4];
};


#endif