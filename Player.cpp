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
    m_vertexBuffer(nullptr),
    m_indexBuffer(nullptr),
    constantBuffer(nullptr),
    modelMatrix(DirectX::XMMatrixIdentity())
{

}

bool Player::Load(ID3D11Device* device, const std::string& atlasPath, const std::string& skelPath) {
    m_loader = new SpineTextureLoader(device);
    m_atlas = new spine::Atlas(atlasPath.c_str(), m_loader);
    spine::SkeletonBinary binary(m_atlas);
    m_skeletonData = binary.readSkeletonDataFile(skelPath.c_str());
    m_skeleton = new spine::Skeleton(m_skeletonData);
   
    spine::AnimationStateData* animationStateData = new spine::AnimationStateData(m_skeletonData);
    animationStateData->setDefaultMix(0.1f);
    animationStateData->setMix("jump", "walk", 0.2f);
    spine::AnimationState* animationState = new spine::AnimationState(animationStateData);
    m_animationStateData = animationStateData;
    m_animationState = animationState;
    
    m_animationState->setAnimation(0, "walk", true);
   
    if (!InitBuffers(device)) {
        return false;
    }

    // 
    if (!InitConstantBuffer(device)) {
        return false;
    }


    return true;
}

void Player::Update(float deltaTime) {
    m_animationState->update(deltaTime);
    m_animationState->apply(*m_skeleton);
    m_skeleton->updateWorldTransform(spine::Physics_None);
}
bool Player::InitBuffers(ID3D11Device* device) {
    Vertex vertices[4] = {};
    m_vertexBuffer = CreateQuadVertexBuffer(device, vertices, 4);
    m_indexBuffer = CreateQuadIndexBuffer(device);
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

    // GPUのconstantBufferをCPUがアクセスできるメモリにマッピングし、新しいデータを書き込む
    // Direct3DのMap後に、書き込み可能なメモリアドレス（void*型）を得られる
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
    UpdateConstantBuffer(context, view, projection);
    context->VSSetConstantBuffers(0, 1, &constantBuffer); 

    auto& drawOrder = m_skeleton->getDrawOrder();
    for (size_t i = 0; i < drawOrder.size(); ++i) {
        spine::Slot* slot = drawOrder[i];
        spine::Attachment* attachment = slot->getAttachment();
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
        

        if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
            // ---- 下面这些代码直接放这里！----
            auto* region = static_cast<spine::RegionAttachment*>(attachment);

            // 1. 获取世界坐标
            float worldVertices[8];
            region->computeWorldVertices(*slot, worldVertices, 0, 2);

            // 2. 获取UV
            spine::Vector<float>& uvs = region->getUVs();
            float* uv_ptr = uvs.buffer();

            // 3. 获取SRV
            spine::TextureRegion* texRegion = region->getRegion();
            spine::AtlasRegion* atlasRegion = static_cast<spine::AtlasRegion*>(texRegion);
            spine::AtlasPage* page = atlasRegion->page;
            ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(page->texture);

            // 4. 组装你的 Vertex[]、上传到GPU、绑定SRV并 DrawIndexed...
            // ...你的后续渲染代码...
            Vertex vertices[4];
            for (int v = 0; v < 4; ++v) {
                vertices[v].x = worldVertices[v * 2 + 0];
                vertices[v].y = worldVertices[v * 2 + 1];
                vertices[v].u = uv_ptr[v * 2 + 0];
                vertices[v].v = uv_ptr[v * 2 + 1];
                vertices[v].r = 1.0f;
                vertices[v].g = 1.0f;
                vertices[v].b = 1.0f;
                vertices[v].a = 1.0f;
            }
            // 5. 写入顶点buffer
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr)) {
                wchar_t msg[256];
                swprintf(msg, 256, L"Failed to map vertex buffer. HRESULT=0x%08X", hr);
                MessageBox(nullptr, msg, L"Error", MB_OK | MB_ICONERROR);
                return;
            }
            memcpy(mapped.pData, vertices, sizeof(vertices));
            context->Unmap(m_vertexBuffer, 0);

            // 6. 绑定输入布局/缓冲区/常量缓冲区
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
            context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            // 7. 绑定纹理
            context->PSSetShaderResources(0, 1, &srv);

            context->DrawIndexed(6, 0, 0);
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