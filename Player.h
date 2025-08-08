/**********************************************************************************
    Player.h

                                                                LI WENHUI
                                                                2025/08/08

**********************************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#include "Vertex.h"
#include "ConstantBuffer.h"
#include "AnimationData.h"
#include <vector>
#include "d3dApp.h"

enum class PlayerAnimationState {
    Idle,
    Walk,
    Jump
};
enum class PlayerDirection {
    Left,
    Right
};
enum class PlayerAnimationIndex {
    Idle = 0,
    Walk,
    Jump,
    Count
};

class Player {
public:
	Player();
	~Player();

	bool Load(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		float width,
		float height,
		std::vector<AnimationData>& animationData
	);

	void Update(float deltaTime);
	void UpdateConstantBuffer(ID3D11DeviceContext* context,
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& projection);

	void Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& projection);

	void Release();

	float textureWidth = 0.0f;
	float textureHeight = 0.0f;



	float GetPosX() const;
	float GetPosY() const;
	float GetW() const;
	float GetH() const;

	void SetPos(float x, float y);

	void SetFrameIndex(int idx);

	void ResetAnimationTimer();

	void SetAnimationData(PlayerAnimationIndex index);

	float GetSpeed() const;
	void SetSpeed(float speed_new);
	bool isAnimated = false;
	PlayerAnimationState state = PlayerAnimationState::Idle;
	PlayerDirection direction = PlayerDirection::Right;

private:
	void InitVertexData(ID3D11Device* device, ID3D11DeviceContext* context, float width, float height);

	// GPUリソース
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* constantBuffer;

	// アニメーション制御
	float texOffset[2];
	float texScale[2];
	float fps;
	float animationTimer;
	int frameIndex;
	int totalFrames;
	int columns;
	int rows;
	float speed = 0.0f;
	float objW = 0.0f;
	float objH = 0.0f;

	//
	std::vector<AnimationData> animationData;

	//
	std::vector<ID3D11ShaderResourceView*> textureSrvs;

	// 描画
	UINT indexCount;

	// 
	DirectX::XMMATRIX modelMatrix;
};


#endif