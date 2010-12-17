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
 * @brief ミッションパラメータセット
 */
//---------------------------------------------------------
void FEST_MISSION_ParamSet( FEST_MISSION* fmwk, FEST_MISSION_ID id, u32 reward, u16 target, u16 param, u32 time )
{
  FEST_MISSION_Clear( fmwk );
  fmwk->id = id;
  fmwk->key = GFUser_GetPublicRand(GFL_STD_RAND_MAX);  //32bitランダムキー生成
  fmwk->time = time;
  fmwk->target = target;
  fmwk->param = param;
  fmwk->reward = reward;
  fmwk->life = fmwk->time*60*60;
  
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション パラメータセット id=%d, key = %08x, target=%d, param=%d, time=%dmin\n",
    id, fmwk->key, target, param, time);
}


//---------------------------------------------------------
/*
 * @brief ミッション開始処理
 */
//---------------------------------------------------------
void FEST_MISSION_Start( FEST_MISSION* fmwk, const FEST_MISSION* src )
{
  FEST_MISSION_Clear( fmwk );
  *fmwk = *src;

  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション スタート id=%d, key = %08x, target=%d, param=%d, time=%d min life=%d sec\n",
    fmwk->id, fmwk->key, fmwk->target, fmwk->param, fmwk->time, fmwk->life/60);
}

//---------------------------------------------------------
/*
 * @brief ミッション終了処理
 */
//---------------------------------------------------------
void FEST_MISSION_End( FEST_MISSION* fmwk )
{
  FEST_MISSION_Clear( fmwk );
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション終了\n");
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
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション %d key = %08x 時間切れ\n", fmwk->id, fmwk->key );
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

//---------------------------------------------------------
/*
 * @brief ミッションに参加しているか？
 *
 * @retval  TRUE   ミッション中
 * @retval  FALSE  ミッション中でない
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsEntry( FEST_MISSION* fmwk )
{
  return fmwk->id != FEST_MISSION_NULL;
}

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
void FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk, FMISSION_TYPE_ID type, u32 value )
{
  if( FEST_MISSION_IsLive( fmwk ) == FALSE ){
    return;
  }
  if( type == FMISSION_TYPE_WILDBTL_WIN ){
    fmwk->my_score += value;
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"ミッション id=%d key=%08x スコア加算 %d ノルマ %d\n",fmwk->id, fmwk->key, fmwk->my_score, fmwk->target );
  }
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
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション %d key = %08x 寿命強制破棄\n", fmwk->id, fmwk->key );
}

#endif  //PM_DEBUG


