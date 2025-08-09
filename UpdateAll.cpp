/**********************************************************************************
    UpdateALL.cpp

                                                                LI WENHUI
                                                                2025/07/25

**********************************************************************************/

#include "UpdateAll.h"
#include "StateInfo.h"
#include "PlayerObject.h"


void UpdatePlayer(StateInfo* pState, float deltaTime) {

    pState->player->Update(deltaTime);

}

void UpdatePlayeState(StateInfo* pState, float deltaTime, bool leftPressed, bool rightPressed, bool spacePressed) {

}
