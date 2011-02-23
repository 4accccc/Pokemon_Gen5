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

SDK_COMPILER_ASSERT( sizeof(FEST_MISSION) <= 20); //フェスミッション構造体型サイズはmax20byteまで

typedef BOOL (*MISSION_SCORE_CALC_FUNC)( const FEST_MISSION* fmwk, u16 param );

static u32 score_CalcTimeBonus( FEST_MISSION* fmwk );
static BOOL score_CalcWildBtlWin( const FEST_MISSION* fmwk, u16 param );
static BOOL score_CalcEvent( const FEST_MISSION* fmwk, u16 param );


static const MISSION_SCORE_CALC_FUNC DATA_ScoreCalcFuncTbl[] = {
  NULL,
  score_CalcWildBtlWin,
  score_CalcEvent,
  NULL,
};

//---------------------------------------------------------
/*
 * @brief ミッション管理領域クリア
 */
//---------------------------------------------------------
void FEST_MISSION_Clear( FEST_MISSION* fmwk )
{
  MI_CpuClear8( fmwk, sizeof(FEST_MISSION));

  fmwk->type = FMISSION_TYPE_NULL;
}

#if 0
//---------------------------------------------------------
/*
 * @brief ミッションパラメータセット
 */
//---------------------------------------------------------
void FEST_MISSION_ParamSet( FEST_MISSION* fmwk, FEST_MISSION_ID type, u16 item, u16 item_num, u16 target, u16 param, u32 time )
{
  FEST_MISSION_Clear( fmwk );
  fmwk->type = type;
  fmwk->key = GFUser_GetPublicRand(GFL_STD_RAND_MAX);  //32bitランダムキー生成
  fmwk->time = time;
  fmwk->target = target;
  fmwk->param = param;
  fmwk->item = item;
  fmwk->item_num = item_num;
  fmwk->life = fmwk->time*60;
  
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション パラメータセット type=%d, key = %08x, target=%d, param=%d, time=%dsec\n",
    type, fmwk->key, target, param, time);
}
#endif

//---------------------------------------------------------
/*
 * @brief ミッション開始処理
 */
//---------------------------------------------------------
void FEST_MISSION_Start( FEST_MISSION* fmwk, const FEST_MISSION* src, BOOL succeed_f )
{
  FEST_MISSION_Clear( fmwk );
  *fmwk = *src;
 
  if( !succeed_f ){ //継承ミッションでなければキー生成
    fmwk->key = GFUser_GetPublicRand(GFL_STD_RAND_MAX);  //32bitランダムキー生成
  }

  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション スタート type=%d, key = %08x, target=%d, param=%d, time=%d min life=%d sec\n",
    fmwk->type, fmwk->key, fmwk->target, fmwk->param, fmwk->time, fmwk->life/60);
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
  if( fmwk->type == FMISSION_TYPE_NULL ){
    return;
  }
  if( fmwk->life ){
    --fmwk->life;
    if( fmwk->life == 0 ){
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション %d key = %08x 時間切れ\n", fmwk->type, fmwk->key );
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
BOOL FEST_MISSION_IsEnd( const FEST_MISSION* fmwk )
{
  if( fmwk->type == FMISSION_TYPE_NULL ){
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
BOOL FEST_MISSION_IsLive( const FEST_MISSION* fmwk )
{
  if( fmwk->type == FMISSION_TYPE_NULL || fmwk->life <= 0 ){
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
BOOL FEST_MISSION_IsEntry( const FEST_MISSION* fmwk )
{
  return fmwk->type != FMISSION_TYPE_NULL;
}

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
BOOL FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk, u16 param, u16 inc, u32* out_bonus )
{
  u32 bonus = 0;
  
  *out_bonus = 0;
  if( FEST_MISSION_IsLive( fmwk ) == FALSE ){
    return FALSE;
  }
  if( !DATA_ScoreCalcFuncTbl[ fmwk->type ](fmwk,param) ){
    return FALSE;
  }
  if( (fmwk->my_score + inc ) > FMISSION_SCORE_MAX ){
    fmwk->my_score = FMISSION_SCORE_MAX;
  }else{
    fmwk->my_score += inc;
  }
  bonus = score_CalcTimeBonus( fmwk );
  if( fmwk->life + bonus > FMISSION_LIFE_MAX ){
    *out_bonus = FMISSION_LIFE_MAX-fmwk->life;
    fmwk->life = FMISSION_LIFE_MAX;
  }else{
    *out_bonus = bonus;
    fmwk->life += bonus;
  }
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"ミッション type=%d key=%08x スコア加算 %d ボーナス %d ノルマ %d\n",fmwk->type, fmwk->key, fmwk->my_score, bonus/60, fmwk->target );
  return TRUE;
}

//========================================================================
/*
  サブルーチン
*/
//========================================================================

//---------------------------------------------------------
/*
 * @brief スコアボーナス計算
 */
//---------------------------------------------------------
static u32 score_CalcTimeBonus( FEST_MISSION* fmwk )
{
  return (fmwk->bonus * 10 * 60);
}

//---------------------------------------------------------
/*
 * @brief スコア計算 野生ポケモンバトル 
 */
//---------------------------------------------------------
static BOOL score_CalcWildBtlWin( const FEST_MISSION* fmwk, u16 param )
{
  return ( fmwk->param == 0 || fmwk->param == param );
}

//---------------------------------------------------------
/*
 * @brief スコア計算 イベントクリア 
 */
//---------------------------------------------------------
static BOOL score_CalcEvent( const FEST_MISSION* fmwk, u16 param )
{
  return ( fmwk->param == param );
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
  if( fmwk->type == FMISSION_TYPE_NULL ){
    return;
  }
  fmwk->life = 0;
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション %d key = %08x 寿命強制破棄\n", fmwk->type, fmwk->key );
}

#endif  //PM_DEBUG


