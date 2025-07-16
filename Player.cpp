/**********************************************************************************
    Player.cpp

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#include "Player.h"
#include "TextureLoader.h"
#include <spine/SkeletonBinary.h>
#include <spine/spine.h>
#include <spine/RegionAttachment.h>
#include "BufferUtils.h"

Player::Player()
    : m_loader(nullptr),
    m_atlas(nullptr),
    m_skeletonData(nullptr),
    m_skeleton(nullptr),
    m_animationState(nullptr),
    m_animationStateData(nullptr),
    texOffset{ 0.0f, 0.0f },
    texScale{ 1.0f, 1.0f },
    m_vertexBufferSize(4),
    m_indexBufferSize(6),
    m_vertexBuffer(nullptr),
    m_indexBuffer(nullptr),
    constantBuffer(nullptr),
    modelMatrix(DirectX::XMMatrixIdentity())
{

}

bool Player::Load(ID3D11Device* device, const std::string& atlasPath, const std::string& skelPath) {
    m_loader = new SpineTextureLoader(device);//加载图片纹理到GPU，供后续渲染
    m_atlas = new spine::Atlas(atlasPath.c_str(), m_loader);//加载贴图信息和图片
    spine::SkeletonBinary binary(m_atlas);
    m_skeletonData = binary.readSkeletonDataFile(skelPath.c_str());// 加载骨骼动画数据
    m_skeleton = new spine::Skeleton(m_skeletonData);//创建运行时骨架对象

    if (!m_skeletonData) {
        OutputDebugStringA("skeletonData load failed!\n");
        return false;
    }
    if (!m_skeleton) {
        OutputDebugStringA("skeleton is null!\n");
    }

    m_skeleton->setX(300.0f);
    m_skeleton->setY(400.0f); 

    spine::AnimationStateData* animationStateData = new spine::AnimationStateData(m_skeletonData);//AnimationStateData、AnimationState 是Spine的动画状态机，能自动管理当前播放的动画、混合、切换、进度等
    animationStateData->setDefaultMix(0.1f);
    animationStateData->setMix("jump", "walk", 0.2f);
    spine::AnimationState* animationState = new spine::AnimationState(animationStateData);
    m_animationStateData = animationStateData;
    m_animationState = animationState;
    
    m_animationState->setAnimation(0, "hoverboard", true);
   
    if (!InitBuffers(device)) {//分配GPU内存，为每帧动画生成的顶点/索引数据准备空间
        return false;
    }

    // 
    if (!InitConstantBuffer(device)) {//常量缓冲区用于传递模型变换/贴图偏移等信息给Shader
        return false;
    }


    return true;
}

void Player::Update(float deltaTime) {
    m_animationState->update(deltaTime);//推进动画时间
    m_animationState->apply(*m_skeleton);//把动画结果应用到骨架
    m_skeleton->updateWorldTransform(spine::Physics_None);//计算所有骨骼的最终世界变换
}
bool Player::InitBuffers(ID3D11Device* device) {
    Vertex vertices[4] = {};
    m_vertexBuffer = CreateDynamicVertexBuffer(device, m_vertexBufferSize);
    m_indexBuffer = CreateDynamicIndexBuffer(device, m_indexBufferSize);
    return m_vertexBuffer && m_indexBuffer;
}
bool Player::InitConstantBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;
    cbd.StructureByteStride = 0;


    HRESULT hr = device->CreateBuffer(&cbd, nullptr, &constantBuffer);
    return SUCCEEDED(hr);
}
void Player::UpdateConstantBuffer(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    if (SUCCEEDED(context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        ConstantBuffer* cb = (ConstantBuffer*)mappedResource.pData;
        cb->model = DirectX::XMMatrixTranspose(modelMatrix);
        cb->view = DirectX::XMMatrixTranspose(view);
        cb->projection = DirectX::XMMatrixTranspose(projection);
        cb->texOffset[0] = texOffset[0];
        cb->texOffset[1] = texOffset[1];
        cb->texScale[0] = texScale[0];
        cb->texScale[1] = texScale[1];

        context->Unmap(constantBuffer, 0);
    }
}

void Player::Render(ID3D11DeviceContext* context, StateInfo* pState,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection){
    UpdateConstantBuffer(context, view, projection);//更新常量缓冲区（传递相机/模型/纹理偏移等）
    context->VSSetConstantBuffers(0, 1, &constantBuffer); 

    auto& drawOrder = m_skeleton->getDrawOrder();//DrawOrder是Spine确定的“前后遮挡顺序”
    for (size_t i = 0; i < drawOrder.size(); ++i) {
        spine::Slot* slot = drawOrder[i];//每个Slot就是一个挂载了图片或网格的骨骼点
        spine::Attachment* attachment = slot->getAttachment();//图片/网格
        if (!attachment) continue;

        spine::BlendMode blendMode = slot->getData().getBlendMode();
        switch (blendMode) {
        case spine::BlendMode_Normal:
            context->OMSetBlendState(pState->blendStateNormal, nullptr, 0xffffffff);
            break;
        case spine::BlendMode_Additive:
            context->OMSetBlendState(pState->blendStateAdditive, nullptr, 0xffffffff);
            break;
        case spine::BlendMode_Multiply:
            context->OMSetBlendState(pState->blendStateMultiply, nullptr, 0xffffffff);
            break;
        case spine::BlendMode_Screen:
            context->OMSetBlendState(pState->blendStateScreen, nullptr, 0xffffffff);
            break;
        }


        std::vector<Vertex> vertices;
        std::vector<UINT> indices;
        ID3D11ShaderResourceView* srv = nullptr;
        if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
            // ---- 下面这些代码直接放这里！----
            auto* region = static_cast<spine::RegionAttachment*>(attachment);

            // 获取世界坐标
            float worldVertices[8];
            region->computeWorldVertices(*slot, worldVertices, 0, 2);//调用 computeWorldVertices，得到当前帧图片四个顶点的世界坐标

            // 获取UV
            spine::Vector<float>& uvs = region->getUVs();
            float* uv_ptr = uvs.buffer();

            // 获取SRV
            spine::TextureRegion* texRegion = region->getRegion();
            spine::AtlasRegion* atlasRegion = static_cast<spine::AtlasRegion*>(texRegion);
            spine::AtlasPage* page = atlasRegion->page;
            srv = reinterpret_cast<ID3D11ShaderResourceView*>(page->texture);

            // 4. 组装你的 Vertex[]、上传到GPU、绑定SRV并 DrawIndexed...
            // ...你的后续渲染代码...
            vertices.resize(4);
            for (int v = 0; v < 4; ++v) {
                vertices[v].x = worldVertices[v * 2 + 0];
                vertices[v].y = pState->logicalHeight - worldVertices[v * 2 + 1];
                vertices[v].z = 0.0f;
                vertices[v].u = uv_ptr[v * 2 + 0];
                vertices[v].v = uv_ptr[v * 2 + 1];
                vertices[v].r = 1.0f;
                vertices[v].g = 1.0f;
                vertices[v].b = 1.0f;
                vertices[v].a = 1.0f;
            }

            indices = { 0, 1, 2, 2, 3, 0 };
        }
        else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            auto* mesh = static_cast<spine::MeshAttachment*>(attachment);
            // 获取动态顶点数
            int vertexCount = mesh->getWorldVerticesLength() / 2;
            std::vector<float> worldVertices(vertexCount * 2);
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices.data(), 0, 2);//计算当前帧所有网格顶点的世界坐标

            // UV
            float* uv_ptr = mesh->getUVs().buffer();

            //
            spine::Vector<unsigned short>& triangles = mesh->getTriangles();
            indices.resize(triangles.size());
            for (int i = 0; i < triangles.size(); ++i) indices[i] = triangles[i];

            // 获取SRV
            spine::TextureRegion* texRegion = mesh->getRegion();
            spine::AtlasRegion* atlasRegion = static_cast<spine::AtlasRegion*>(texRegion);
            spine::AtlasPage* page = atlasRegion->page;
            srv = reinterpret_cast<ID3D11ShaderResourceView*>(page->texture);

            // 组装 Vertex[]
            vertices.resize(vertexCount);
            for (int v = 0; v < vertexCount; ++v) {
                vertices[v].x = worldVertices[v * 2 + 0];
                vertices[v].y = pState->logicalHeight - worldVertices[v * 2 + 1];
                vertices[v].z = 0.0f;
                vertices[v].u = uv_ptr[v * 2 + 0];
                vertices[v].v = uv_ptr[v * 2 + 1];
                vertices[v].r = 1.0f;
                vertices[v].g = 1.0f;
                vertices[v].b = 1.0f;
                vertices[v].a = 1.0f;
            }
        }
        if (vertices.size() > m_vertexBufferSize) {
            SAFE_RELEASE(m_vertexBuffer);
            m_vertexBuffer = CreateDynamicVertexBuffer(pState->device, vertices.size());
            m_vertexBufferSize = vertices.size();
        }
        if (indices.size() > m_indexBufferSize) {
            SAFE_RELEASE(m_indexBuffer);
            m_indexBuffer = CreateDynamicIndexBuffer(pState->device, indices.size());
            m_indexBufferSize = indices.size();
        }
        // 写入顶点buffer
        if (srv && !vertices.empty() && !indices.empty()) {
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (SUCCEEDED(hr)) {
                memcpy(mapped.pData, vertices.data(), vertices.size() * sizeof(Vertex));
                context->Unmap(m_vertexBuffer, 0);
            }
            // 上传GPU缓冲区
            hr = context->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (SUCCEEDED(hr)) {
                memcpy(mapped.pData, indices.data(), indices.size() * sizeof(UINT));
                context->Unmap(m_indexBuffer, 0);
            }
            // 绑定输入布局/缓冲区/常量缓冲区
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
            context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
            // 绑定纹理
            context->PSSetShaderResources(0, 1, &srv);//把纹理SRV（Shader Resource View）绑定到像素着色器（Pixel Shader），
            //
            context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
        }
        
    }
}

Player::~Player () {
    delete m_loader;          m_loader = nullptr;
    delete m_atlas;           m_atlas = nullptr;
    delete m_skeletonData;    m_skeletonData = nullptr;
    delete m_skeleton;        m_skeleton = nullptr;
    delete m_animationStateData; m_animationStateData = nullptr;
    delete m_animationState;     m_animationState = nullptr;

    if (constantBuffer) { constantBuffer->Release(); constantBuffer = nullptr; }
    if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
    if (m_indexBuffer) { m_indexBuffer->Release(); m_indexBuffer = nullptr; }
}