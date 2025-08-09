
#include "StateInfo.h"
#include "PlayerObject.h"

StateInfo::~StateInfo() {
	SAFE_RELEASE(blendStateNormal);
	SAFE_RELEASE(blendStateAdditive);
	SAFE_RELEASE(blendStateMultiply);
	SAFE_RELEASE(blendStateScreen);
}