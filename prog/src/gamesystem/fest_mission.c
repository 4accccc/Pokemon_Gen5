//==============================================================================
/**
 * @file    fest_mission.c
 * @brief   リンクフェスティバル ミッション管理
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/rtc_tool.h"
#include "field/gpower_id.h"
#include "gamesystem/g_power.h"
#include "gamesystem/gamesystem.h"

#include "waza_tool/wazano_def.h"
#include "arc_def.h"
#include "power.naix"
#include "poke_tool/status_rcv.h"

#include "gamesystem/link_festival.h"

//---------------------------------------------------------
/*
 * @brief ミッション管理領域クリア
 */
//---------------------------------------------------------
void FEST_MISSION_Clear( FEST_MISSION* fmwk )
{
  MI_CpuClear8( fmwk, sizeof(FEST_MISSION));

  fmwk->id = FEST_MISSION_NULL;
}

//---------------------------------------------------------
/*
 * @brief ミッション開始処理
 */
//---------------------------------------------------------
void FEST_MISSION_Start( FEST_MISSION* fmwk, FEST_MISSION_ID id, u32 key, u16 param1, u16 param2, u32 time )
{
  FEST_MISSION_Clear( fmwk );
  fmwk->id = id;
  fmwk->key = key;
  fmwk->time = time;
  fmwk->prm[0] = param1;
  fmwk->prm[1] = param2;
  fmwk->reward = 3; //今適当な固定値
  fmwk->life = fmwk->time*60*60;

  OS_TPrintf("フェスミッションスタート id=%d, key = %08x, prm1=%d, prm2=%d, time=%dmin\n",
    id, key, param1, param2, time);
}

//---------------------------------------------------------
/*
 * @brief ミッション終了処理
 */
//---------------------------------------------------------
void FEST_MISSION_End( FEST_MISSION* fmwk )
{
  FEST_MISSION_Clear( fmwk );
  OS_TPrintf("フェスミッション終了\n");
}

//---------------------------------------------------------
/*
 * @brief ミッション状態監視処理
 */
//---------------------------------------------------------
void FEST_MISSION_Update( FEST_MISSION* fmwk )
{
  if( fmwk->id == FEST_MISSION_NULL ){
    return;
  }
  if( fmwk->life ){
    --fmwk->life;
    if( fmwk->life == 0 ){
      OS_TPrintf("フェスミッション %d key = %08x 時間切れ\n", fmwk->id, fmwk->key );
    }
  }
}

//---------------------------------------------------------
/*
 * @brief ミッション終了監視(ミッション中でない場合、常にFalse)
 *
 * @retval  TRUE   ミッション終了
 * @retval  FALSE  ミッション中でないorミッション継続中
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsEnd( FEST_MISSION* fmwk )
{
  if( fmwk->id == FEST_MISSION_NULL ){
    return FALSE;
  }
  if( fmwk->life == 0){
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief ミッションが生きているか取得
 *
 * @retval  TRUE   ミッション中且つライフ有
 * @retval  FALSE  ミッション中でないorミッションのライフ切れ
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsLive( FEST_MISSION* fmwk )
{
  if( fmwk->id == FEST_MISSION_NULL || fmwk->life <= 0 ){
    return FALSE;
  }
  return TRUE;
}


//=================================================
//以下デバッグルーチン
//=================================================
#ifdef PM_DEBUG
//---------------------------------------------------------
/*
 * @brief ミッション寿命強制破棄
 */
//---------------------------------------------------------
void FEST_MISSION_DEBUG_KillMission( FEST_MISSION* fmwk )
{
  if( fmwk->id == FEST_MISSION_NULL ){
    return;
  }
  fmwk->life = 0;
  OS_TPrintf("フェスミッション %d key = %08x 寿命強制破棄\n", fmwk->id, fmwk->key );
}

#endif  //PM_DEBUG


