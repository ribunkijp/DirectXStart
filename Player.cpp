/**********************************************************************************
    Player.cpp

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

#include "Player.h"
#include "TextureLoader.h"
#include <spine/SkeletonBinary.h>

Player::Player()
    : m_loader(nullptr),
    m_atlas(nullptr),
    m_skeletonData(nullptr),
    m_skeleton(nullptr),
    m_animationState(nullptr)
{

}

bool Player::load(ID3D11Device* device, const std::string& atlasPath, const std::string& skelPath) {
    m_loader = new SpineTextureLoader(device);
    m_atlas = new spine::Atlas(atlasPath.c_str(), m_loader);
    spine::SkeletonBinary binary(m_atlas);
    m_skeletonData = binary.readSkeletonDataFile(skelPath.c_str());
    m_skeleton = new spine::Skeleton(m_skeletonData);
}

Player::~Player () {
    delete m_animationState;
    delete m_skeleton;
    delete m_skeletonData;
    delete m_atlas;
    delete m_loader;
}