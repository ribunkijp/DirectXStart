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


class Player {
public:
    Player();
    ~Player();
    bool Load(ID3D11Device* device, const std::string& atlasPath, const std::string& skelPath);
    void Update(float delaTime);

private:
    SpineTextureLoader* m_loader;
    spine::Atlas* m_atlas;
    spine::SkeletonData* m_skeletonData;
    spine::Skeleton* m_skeleton;
    spine::AnimationState* m_animationState;
    spine::AnimationStateData* m_animationStateData;
};


#endif 
