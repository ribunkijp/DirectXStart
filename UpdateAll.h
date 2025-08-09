/**********************************************************************************
    UpdateAll.h

                                                                LI WENHUI
                                                                2025/07/25

**********************************************************************************/

#ifndef UPDATEALL_H
#define UPDATEALL_H

struct StateInfo;

void UpdatePlayer(StateInfo* pState, float deltaTime);

void UpdatePlayerState(StateInfo* pState, float deltaTime, bool leftPressed, bool rightPressed, bool spacePressed);





#endif