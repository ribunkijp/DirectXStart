/**********************************************************************************
	Vertex.h

																LI WENHUI
																2025/08/08

**********************************************************************************/

#ifndef VERTEX_H
#define VERTEX_H

#include <DirectXMath.h>


// 頂点構造体
struct Vertex {
	DirectX::XMFLOAT3 position;  // 位置
	DirectX::XMFLOAT4 color;     // 色
	DirectX::XMFLOAT2 texCoord;  // テクスチャ座標 (U, V)
};

#endif