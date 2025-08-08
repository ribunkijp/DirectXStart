/**********************************************************************************
    Player.cpp

                                                                LI WENHUI
                                                                2025/08/08

**********************************************************************************/

#include "Player.h"
#include "BufferUtils.h"
#include "TextureLoader.h"
#include <DirectXMath.h>

Player::Player()
	: vertexBuffer(nullptr),
	indexBuffer(nullptr),
	constantBuffer(nullptr),
	texOffset{ 0.0f, 0.0f },
	texScale{ 1.0f, 1.0f },
	fps(8.0f),
	animationTimer(0.0f),
	frameIndex(0),
	totalFrames(1),
	columns(1),
	rows(1),
	indexCount(0),
	modelMatrix(DirectX::XMMatrixIdentity())
{
	texOffset[0] = texOffset[1] = 0.0f;
	texScale[0] = texScale[1] = 1.0f;
}

Player::~Player() {
	Release();
}

void Player::Release() {
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

bool Player::Load(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	float width,
	float height,
	std::vector<AnimationData>& animationData
) {
	this->animationData = animationData;

	InitVertexData(device, context, width, height);


	totalFrames = animationData[0].totalFrames;
	columns = animationData[0].columns;
	rows = animationData[0].rows;
	fps = animationData[0].fps;

	// �萔�o�b�t�@���쐬
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&cbd, nullptr, &constantBuffer);



	textureSrvs.resize(static_cast<size_t>(PlayerAnimationIndex::Count));


	for (size_t i = 0; i < animationData.size(); i++) {
		// �e�N�X�`���̓ǂݍ���
		if (FAILED(LoadTextureAndCreateSRV(device, animationData[i].texturePath.c_str(), &textureSrvs[i], &textureWidth, &textureHeight))) {
			return false;
		}
	}





	// �A�j���[�V�����łȂ��ꍇ�̓e�N�X�`���̃I�t�Z�b�g�E�X�P�[����ݒ�
	if (!isAnimated) {
		texOffset[0] = 0.0f;
		texOffset[1] = 0.0f;
		texScale[0] = 1.0f;
		texScale[1] = 1.0f;
	}


	objW = width;
	objH = height;


	return true;
}

void Player::InitVertexData(ID3D11Device* device, ID3D11DeviceContext* context, float width, float height) {
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


float Player::GetPosX() const {
	return modelMatrix.r[3].m128_f32[0];// x
}
float Player::GetPosY() const {
	return modelMatrix.r[3].m128_f32[1];// y
}

float Player::GetW() const {
	return objW;
}
float Player::GetH() const {
	return objH;
}

void Player::SetPos(float x, float y) {
	modelMatrix = DirectX::XMMatrixTranslation(x, y, 0.0f);
}

void Player::SetFrameIndex(int idx) { frameIndex = idx; }

void Player::ResetAnimationTimer() {
	animationTimer = 0.0f;
}

void Player::SetAnimationData(PlayerAnimationIndex index) {

	totalFrames = animationData[static_cast<size_t>(index)].totalFrames;
	columns = animationData[static_cast<size_t>(index)].columns;
	rows = animationData[static_cast<size_t>(index)].rows;
	fps = animationData[static_cast<size_t>(index)].fps;

	if (!isAnimated) {
		texOffset[0] = 0.0f;
		texOffset[1] = 0.0f;
		texScale[0] = 1.0f;
		texScale[1] = 1.0f;
	}


}

float Player::GetSpeed() const {
	return speed;
}

void Player::SetSpeed(float speed_new) {
	speed = speed_new;
}


void Player::Update(float deltaTime) {
	if (!isAnimated) return;

	animationTimer += deltaTime;
	if (animationTimer >= (1.0f / fps)) {
		frameIndex = (frameIndex + 1) % totalFrames;
		animationTimer = 0.0f;
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

void Player::UpdateConstantBuffer(ID3D11DeviceContext* context,
	const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection) {

	// GPU��constantBuffer��CPU���A�N�Z�X�ł��郁�����Ƀ}�b�s���O���A�V�����f�[�^����������
	// Direct3D��Map��ɁA�������݉\�ȃ������A�h���X�ivoid*�^�j�𓾂���
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

void Player::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& projection) {

	UpdateConstantBuffer(context, view, projection);

	// ���_�o�b�t�@�̃X�g���C�h�istride�j���X�V
	// Vertex�\���̂̃T�C�Y�ƈ�v�����邱��
	UINT stride = sizeof(Vertex);
	// offset�i�I�t�Z�b�g�j�F���_�o�b�t�@�̐擪���牽�o�C�g�ڂ���f�[�^��ǂނ��B������0�ŁA�o�b�t�@�̐擪����ǂ�
	UINT offset = 0;
	// ���_�o�b�t�@��ݒ�
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	// �C���f�b�N�X�o�b�t�@��ݒ�B�e�C���f�b�N�X��32�r�b�g�̕����Ȃ�����
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// HLSL��PSMain��cbuffer ConstantBuffer���g���ۂɃZ�b�g
	context->VSSetConstantBuffers(0, 1, &constantBuffer);
	// PSSetShaderResources�i�J�n�X���b�g�A�r���[���ASRV�z��|�C���^�j
	// t0���W�X�^���X���b�g0�ɑΉ�
	if (state == PlayerAnimationState::Idle) {
		if (direction == PlayerDirection::Left) {
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Idle)]);
		}
		else if (direction == PlayerDirection::Right) {
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Idle)]);
		}

	}
	else if (state == PlayerAnimationState::Walk) {
		if (direction == PlayerDirection::Left) {
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Walk)]);
		}
		else if (direction == PlayerDirection::Right) {
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Walk)]);
		}
	}
	else if (state == PlayerAnimationState::Jump) {
		if (direction == PlayerDirection::Left) {
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Jump)]);
		}
		else if (direction == PlayerDirection::Right) {
			context->PSSetShaderResources(0, 1, &textureSrvs[static_cast<size_t>(PlayerAnimationIndex::Jump)]);
		}
	}
	//
	context->DrawIndexed(indexCount, 0, 0);
}