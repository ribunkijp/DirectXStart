/**********************************************************************************
	Vertex.h

																LI WENHUI
																2025/08/08

**********************************************************************************/

#ifndef VERTEX_H
#define VERTEX_H

#include <DirectXMath.h>


//
struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 texCoord;
};

#endif