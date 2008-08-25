//===================================================================
/**
 * @file	test_flashread.c
 * @brief	フラッシュ読み込み
 * @author	GAME FREAK Inc.
 * @date	08.08.01
 */
//===================================================================


#include <nitro.h>
#include "gflib.h"
#include "tcb.h"
#include "test_flashread.h"

#if 0

typedef struct{
    OSTick tick;
    OSTick printt;
    u32 src;
    void* pDist;
    u32 size;
    u32 lockID;
    u8 state;
    
}TestFlashSt;

TestFlashSt aTFSt;

enum{
    _READ_START,
    _READ_END,


};



void TEST_ReadFlashEnd(void)
{
    GFL_HEAP_FreeMemory(aTFSt.pDist);
}


static void _callback(void* arg)
{
    OSTick tick = OS_GetTick() - aTFSt.tick;
    aTFSt.printt = OS_TicksToMicroSeconds(tick);
    CARD_UnlockBackup(aTFSt.lockID);
    aTFSt.state = _READ_END;
}



static void TEST_ReadFlash(void)
{
    if(aTFSt.state == _READ_END){
        aTFSt.lockID = OS_GetLockID();
        CARD_LockBackup(aTFSt.lockID);
        
        aTFSt.tick = OS_GetTick();
        CARD_ReadFlashAsync( aTFSt.src, aTFSt.pDist, aTFSt.size, _callback, &aTFSt);
        aTFSt.state = _READ_START;
    }
}

static void TEST_DEBUGDisp(GFL_TCB *pTCB, void *pWork)
{
    TEST_ReadFlash();
//    OS_TPrintf("ti %d\n",aTFSt.printt);
}

//------------------------------------------------------------------
/**
 * @brief		逐次読み込み開始
 */
//------------------------------------------------------------------

void TEST_ReadFlashStart(GFL_TCBSYS* pTCB,HEAPID id,u32 size)
{
    aTFSt.size = size;
    aTFSt.pDist = GFL_HEAP_AllocMemory(id,size);
    aTFSt.src = 0;
    aTFSt.state = _READ_END;

    GFL_TCB_AddTask(pTCB,&TEST_DEBUGDisp,NULL,1);
}



#else



static int lockID;


static void _callback(void* arg)
{
    CARD_UnlockBackup(lockID);
    lockID = OS_LOCK_ID_ERROR;
}



void TEST_ReadFlashStart(u8* pData, int size)
{
   // GFL_FLASH_Init();
    //GFL_FLASH_Save(0,pData,size);

#if 0
    lockID = OS_GetLockID();
    CARD_LockBackup(lockID);
    CARD_WriteFlash(0, pData, size);
    CARD_UnlockBackup(lockID);
#endif
    lockID = OS_LOCK_ID_ERROR;
}



void TEST_ReadFlashRead(u8* pData, int offset, int size)
{
    if(lockID == OS_LOCK_ID_ERROR){
        lockID = OS_GetLockID();
        CARD_LockBackup(lockID);
        CARD_ReadFlashAsync( offset, pData, size, _callback, NULL);
    }
    else{
        OS_TPrintf("読み込み遅延発生\n");
    }
}


#endif
