/**********************************************************************************
    Player.h

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#pragma once
#include <string>
#include <spine/Skeleton.h>
#include <spine/Atlas.h>
#include <spine/AnimationState.h>
#include "TextureLoader.h"
#include <DirectXMath.h>
#include "ConstantBuffer.h"
#include "Vertex.h"

struct StateInfo;





class Player {
public:
    Player();
    ~Player();
    bool Load(ID3D11Device* device, const std::string& atlasPath, const std::string& skelPath);
    void Update(float deltaTime);
    void Render(ID3D11DeviceContext* context, StateInfo* pState,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection);
    bool InitBuffers(ID3D11Device* device);
    bool InitConstantBuffer(ID3D11Device* device);
    void UpdateConstantBuffer(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection);

private:
    SpineTextureLoader* m_loader;
    spine::Atlas* m_atlas;
    spine::SkeletonData* m_skeletonData;
    spine::Skeleton* m_skeleton;
    spine::AnimationState* m_animationState;
    spine::AnimationStateData* m_animationStateData;
    float texOffset[2];
    float texScale[2];
    size_t m_vertexBufferSize;
    size_t m_indexBufferSize;
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    ID3D11Buffer* constantBuffer;
    DirectX::XMMATRIX modelMatrix;
};


#endif 

