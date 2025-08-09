/**********************************************************************************
    Player.cpp

                                                                LI WENHUI
                                                                2025/08/08

**********************************************************************************/

#include "PlayerObject.h"
#include "BufferUtils.h"
#include "TextureLoader.h"
#include <DirectXMath.h>

PlayerObject::PlayerObject()
	: vertexBuffer(nullptr),
	indexBuffer(nullptr),
	constantBuffer(nullptr),
	animationTimer(0.0f),
	frameIndex(0),
	indexCount(0)
{
	
}

PlayerObject::~PlayerObject() {
	Release();
}

void PlayerObject::Release() {
	if (vertexBuffer) { vertexBuffer->Release(); vertexBuffer = nullptr; }
	if (indexBuffer) { indexBuffer->Release(); indexBuffer = nullptr; }
	if (constantBuffer) { constantBuffer->Release(); constantBuffer = nullptr; }
	//
	for (auto& srv : textureSrvs) {
		if (srv) {
			srv->Release();
			srv = nullptr;
		}
	}
	textureSrvs.clear();
}

bool PlayerObject::Load(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	float width,
	float height,
	std::vector<AnimationData>& animationData,
	bool isAnimated
) {
	this->animationData = animationData;

	InitVertexData(device, context, width, height);


	totalFrames = animationData[0].totalFrames;
	columns = animationData[0].columns;
	rows = animationData[0].rows;
	fps = animationData[0].fps;

	//
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&cbd, nullptr, &constantBuffer);


	textureSrvs.resize(static_cast<size_t>(PlayerAnimationIndex::Count));


	for (size_t i = 0; i < animationData.size(); i++) {
		// テクスチャの読み込み
		if (FAILED(LoadTextureAndCreateSRV(device, animationData[i].texturePath.c_str(), &textureSrvs[i], &textureWidth, &textureHeight))) {
			return false;
		}
	}

	this->isAnimated = isAnimated;

	//
	if (!this->isAnimated) {
		texOffset[0] = 0.0f;
		texOffset[1] = 0.0f;
		texScale[0] = 1.0f;
		texScale[1] = 1.0f;
	}

	objW = width;
	objH = height;

	return true;
}

void PlayerObject::InitVertexData(ID3D11Device* device, ID3D11DeviceContext* context, float width, float height) {
	Vertex vertices[] = {
		{ { 0.0f,  0.0f,    0.0f }, { 1, 1, 1, 1 }, { 0.0f, 0.0f } },
		{ { width, 0.0f,    0.0f }, { 1, 1, 1, 1 }, { 1.0f, 0.0f } },
		{ { width, height, 0.0f }, { 1, 1, 1, 1 }, { 1.0f, 1.0f } },
		{ { 0.0f,  height, 0.0f }, { 1, 1, 1, 1 }, { 0.0f, 1.0f } }
	};
	vertexBuffer = CreateDynamicVertexBuffer(device, 4);
	indexBuffer = CreateDynamicIndexBuffer(device, 6);
	indexCount = 6;

	const UINT indices[] = { 0,1,2,  0,2,3 };

	//
	D3D11_MAPPED_SUBRESOURCE m{};
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
	memcpy(m.pData, vertices, sizeof(vertices));
	context->Unmap(vertexBuffer, 0);

	//
	context->Map(indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
	memcpy(m.pData, indices, sizeof(indices));
	context->Unmap(indexBuffer, 0);
}


float PlayerObject::GetPosX() const {
	return modelMatrix.r[3].m128_f32[0];
}
float PlayerObject::GetPosY() const {
	return modelMatrix.r[3].m128_f32[1];
}

float PlayerObject::GetW() const {
	return objW;
}
float PlayerObject::GetH() const {
	return objH;
}

void PlayerObject::SetPos(float x, float y) {
	translationMatrix = DirectX::XMMatrixTranslation(x, y, 0.0f);
}

void PlayerObject::SetFlip(bool flip) {
	isFlipX = flip;
}

void PlayerObject::SetFrameIndex(int idx) { frameIndex = idx; }

void PlayerObject::ResetAnimationTimer() {
	animationTimer = 0.0f;
}

void PlayerObject::SetAnimationData(PlayerAnimationIndex index) {

	totalFrames = animationData[static_cast<size_t>(index)].totalFrames;
	columns = animationData[static_cast<size_t>(index)].columns;
	rows = animationData[static_cast<size_t>(index)].rows;
	fps = animationData[static_cast<size_t>(index)].fps;

	if (!this->isAnimated) {
		texOffset[0] = 0.0f;
		texOffset[1] = 0.0f;
		texScale[0] = 1.0f;
		texScale[1] = 1.0f;
	}


}

float PlayerObject::GetSpeed() const {
	return speed;
}

void PlayerObject::SetSpeed(float speed_new) {
	speed = speed_new;
}


void PlayerObject::Update(float deltaTime) {
	if (!this->isAnimated || this->animationFinished) return;

	animationTimer += deltaTime;
	if (animationTimer >= (1.0f / fps)) {
		animationTimer = 0.0f;

        if (this->isLooping) {
            frameIndex = (frameIndex + 1) % totalFrames;
        }
        else {
            if (frameIndex == totalFrames - 1) {
                animationFinished = true;
            }
            else {
                frameIndex++;
            }
        }
	}

	float frameW = 1.0f / static_cast<float>(columns);
	float frameH = 1.0f / static_cast<float>(rows);
	int col = frameIndex % columns;
	int row = frameIndex / columns;

	texOffset[0] = static_cast<float>(col) * frameW;
	texOffset[1] = static_cast<float>(row) * frameH;
	texScale[0] = frameW;
	texScale[1] = frameH;
}

void PlayerObject::UpdateConstantBuffer(ID3D11DeviceContext* context,
	const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection) {

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	modelMatrix = translationMatrix;

	if (SUCCEEDED(context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		ConstantBuffer* cb = (ConstantBuffer*)mappedResource.pData;
		cb->model = DirectX::XMMatrixTranspose(modelMatrix);
		cb->view = DirectX::XMMatrixTranspose(view);
		cb->projection = DirectX::XMMatrixTranspose(projection);
		cb->texOffset[0] = texOffset[0];
		cb->texOffset[1] = texOffset[1];
		cb->texScale[0] = texScale[0];
		cb->texScale[1] = texScale[1];
		cb->uFlipX = isFlipX ? 1 : 0;
		context->Unmap(constantBuffer, 0);
	}
}

void PlayerObject::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& projection) {

	UpdateConstantBuffer(context, view, projection);

	
	// 
	UINT stride = sizeof(Vertex);
	// 
	UINT offset = 0;
	// 
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	// 
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// 
	context->VSSetConstantBuffers(0, 1, &constantBuffer);
	//
	
	if (state == PlayerAnimationState::Idle) {
		if (direction == PlayerDirection::Left) {
            SetFlip(true);
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Idle)]);
		}
		else if (direction == PlayerDirection::Right) {
            SetFlip(false);
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Idle)]);
		}

	}
	else if (state == PlayerAnimationState::Run) {
		if (direction == PlayerDirection::Left) {
            SetFlip(true);
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Run)]);
		}
		else if (direction == PlayerDirection::Right) {
            SetFlip(false);
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Run)]);
		}
	}
	else if (state == PlayerAnimationState::Jump) {
		if (direction == PlayerDirection::Left) {
            SetFlip(true);
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Jump)]);
		}
		else if (direction == PlayerDirection::Right) {
            SetFlip(false);
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Jump)]);
		}
	}
	//
	context->DrawIndexed(indexCount, 0, 0);
}
