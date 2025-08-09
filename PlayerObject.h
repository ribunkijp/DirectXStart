/**********************************************************************************
    Player.h

                                                                LI WENHUI
                                                                2025/08/08

**********************************************************************************/

#ifndef PLAYEROBJECT_H
#define PLAYEROBJECT_H

#include "Vertex.h"
#include "ConstantBuffer.h"
#include "AnimationData.h"
#include <vector>
#include "d3dApp.h"

enum class PlayerAnimationState {
    Idle,
    Run,
    Jump
};
enum class PlayerDirection {
    Left,
    Right
};
enum class PlayerAnimationIndex {
    Idle = 0,
    Run,
    Jump,
    Count
};

class PlayerObject {
public:
	PlayerObject();
	~PlayerObject();

	bool Load(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		float width,
		float height,
		std::vector<AnimationData>& animationData,
		bool isAnimated
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

	void SetFlip(bool flip);
	
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

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* constantBuffer;
	
	float texOffset[2] = { 0.0f, 0.0f };
	float texScale[2] = { 1.0f, 1.0f };
	float fps = 8.0f;
	float animationTimer;
	int frameIndex;
	int totalFrames = 1;
	int columns = 1;
	int rows = 1;
	float speed = 0.0f;
	float objW = 0.0f;
	float objH = 0.0f;
	bool isFlipX = false;


	std::vector<AnimationData> animationData;

	std::vector<ID3D11ShaderResourceView*> textureSrvs;

	UINT indexCount;

	DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixIdentity();

};


#endif