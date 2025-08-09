/**********************************************************************************
	Vertex.h

																LI WENHUI
																2025/08/08

**********************************************************************************/

#ifndef VERTEX_H
#define VERTEX_H

#include <DirectXMath.h>


// ���_�\����
struct Vertex {
	DirectX::XMFLOAT3 position;  // �ʒu
	DirectX::XMFLOAT4 color;     // �F
	DirectX::XMFLOAT2 texCoord;  // �e�N�X�`�����W (U, V)
};

#endif