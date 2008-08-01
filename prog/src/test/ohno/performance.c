//===================================================================
/**
 * @file	performance
 * @brief	パフォーマンスメーター　デバッグ用
 * @author	GAME FREAK Inc.
 * @date	08.08.01
 */
//===================================================================

#ifdef PM_DEBUG

#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"
#include "performance.h"


static u32 CountKeep = 0;
static BOOL Onoff = FALSE;


//------------------------------------------------------------------
/**
 * @brief		パフォーマンスメーターのスタート 
 */
//------------------------------------------------------------------

void DEBUG_PerformanceStart(void)
{
    int debugButtonTrg = GFL_UI_KEY_GetTrg()&PAD_BUTTON_DEBUG;
    
    if(debugButtonTrg){
        Onoff = Onoff ? FALSE
            : TRUE;
        if(Onoff){
            G2_SetWnd0InsidePlane(0,FALSE);
            G2_SetWnd1InsidePlane(0,FALSE);
            G2_SetWndOutsidePlane(0xff,FALSE);
            G2_SetWnd0Position(0, 0, 3, 192);
            G2_SetWnd1Position(263/2-1, 178, 263/2+1, 180);
            GX_SetVisibleWnd(3);
        }
    }
    if(Onoff){
        CountKeep = OS_GetVBlankCount();
    }
}

//------------------------------------------------------------------
/**
 * @brief		パフォーマンスメーターの表示
 */
//------------------------------------------------------------------

void DEBUG_PerformanceDisp(void)
{
    if(Onoff){
        u32 vn,cnt;

        cnt = OS_GetVBlankCount() - CountKeep;
        vn = GX_GetVCount() + (cnt * 263);
        vn = vn / 2;
        if(vn > 263){
            vn = 263;
        }
        G2_SetWnd0Position(0, 180, vn, 182);
//        OS_TPrintf("%d %d\n",vn,CountKeep);
    }
}

#endif //PM_DEBUG
