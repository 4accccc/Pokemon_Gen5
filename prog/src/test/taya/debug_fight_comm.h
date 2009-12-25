


#pragma once

#include <gflib.h>
#include "net\network_define.h"


extern void DFC_NET_Init(const GFLNetInitializeStruct* pNetInit, NetStepEndCallback callback, void* pWork, BOOL bParent,int num);
extern BOOL DFC_NET_Process(void);
