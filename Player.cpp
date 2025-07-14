/**********************************************************************************
    Player.cpp

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#include "Player.h"
#include "TextureLoader.h"
#include <spine/SkeletonBinary.h>
#include <spine/spine.h>

Player::Player()
    : m_loader(nullptr),
    m_atlas(nullptr),
    m_skeletonData(nullptr),
    m_skeleton(nullptr),
    m_animationState(nullptr),
    m_animationStateData(nullptr)
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

    //spine::Bone* bone = m_skeleton->findBone("leftArm");
    //if (bone) {
    //    bone->setRotation(45.0f); 
    //    bone->setScaleX(1.2f);  
    //}
}

void Player::Update(float deltaTime) {
    m_animationState->update(deltaTime);
    m_animationState->apply(*m_skeleton);
    m_skeleton->updateWorldTransform(spine::Physics_None);
}

Player::~Player () {
    delete m_animationStateData;
    delete m_animationState;
    delete m_skeleton;
    delete m_skeletonData;
    delete m_atlas;
    delete m_loader;
}