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


typedef struct _MISSION_SCORE_CALC_PARAM{
  MISSION_SCORE_CALC_FUNC func;
  u32 log_act_f;  //どのログアクションがスコアアップに対応しているか？
}MISSION_SCORE_CALC_PARAM;


//====================================================
///ミッションタイプ別スコアインクリメントチェック関数
static u32 score_CalcTimeBonus( FEST_MISSION* fmwk );
static BOOL score_CalcWildBtlWin( const FEST_MISSION* fmwk, u16 param );
static BOOL score_CalcEvent( const FEST_MISSION* fmwk, u16 param );
static BOOL score_CalcItem( const FEST_MISSION* fmwk, u16 param );

/*
 *  ミッションスコア計算に必要なパラメータテーブル
 *
 *  @todo いずれ外部化する予定
 */
static const MISSION_SCORE_CALC_PARAM DATA_ScoreCalcParamTbl[] = {
  { NULL, 0 },
  { score_CalcWildBtlWin, (1<<FMISSION_LOG_ACT_POKE_WIN)|(1<<FMISSION_LOG_ACT_POKE_CAPTURE)},
  { score_CalcEvent, (1<<FMISSION_LOG_ACT_EVENT_CLEAR) },
  { score_CalcItem, (1<<FMISSION_LOG_ACT_ITEM_GET) },
};

//---------------------------------------------------------
/*
 * @brief ミッション管理領域クリア
 */
//---------------------------------------------------------
void FEST_MISSION_Clear( FEST_MISSION* fmwk )
{
  MI_CpuClear8( fmwk, sizeof(FEST_MISSION));

  //無効値で初期化
  fmwk->target = FMISSION_TARGET_NULL;
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
  }else{
    fmwk->my_score = 0; //継承ミッションの場合、スコアクリア
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
//  if( fmwk->type == FMISSION_TYPE_NULL || fmwk->life <= 0 ){
  if( fmwk->type == FMISSION_TYPE_NULL ){
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
 * @brief ミッション　スコアインクリメントチェック
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsScoreUp( FEST_MISSION* fmwk, FMISSION_LOG_ACTION action, u16 value )
{
  const MISSION_SCORE_CALC_PARAM* prm = &DATA_ScoreCalcParamTbl[ fmwk->type ];
  if( (prm->log_act_f & (1<<action)) == FALSE ){
    return FALSE;
  }
  return prm->func(fmwk,value );
}

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
BOOL FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk,  FMISSION_LOG_ACTION action, u16 param, u16 inc )
{
  if( FEST_MISSION_IsLive( fmwk ) == FALSE ){
    return FALSE;
  }
  if( !FEST_MISSION_IsScoreUp( fmwk, action, param )){
    return FALSE;
  }
  if( fmwk->type == FMISSION_TYPE_EVENT && fmwk->my_score > 0){
    return FALSE; //イベントタイプは一回しかスコアがあがらない
  }
  if( (fmwk->my_score + inc ) > FMISSION_SCORE_MAX ){
    fmwk->my_score = FMISSION_SCORE_MAX;
  }else{
    fmwk->my_score += inc;
  }
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"ミッション type=%d key=%08x スコア加算 %d ノルマ %d\n",fmwk->type, fmwk->key, fmwk->my_score, fmwk->target );
  return TRUE;
}

//---------------------------------------------------------
/*
 * @brief ミッション　タイムボーナス計算
 *
 * @return  秒単位のボーナス(1秒未満は1を返す)
 */
//---------------------------------------------------------
u32 FEST_MISSION_TimeBonusCalc( FEST_MISSION* fmwk,  u16 score_diff )
{
  u32 bonus = score_CalcTimeBonus( fmwk ) * score_diff;
  
  if( bonus == 0 ){
    return 0;
  }
  if( fmwk->life + bonus > FMISSION_LIFE_MAX ){
    bonus = FMISSION_LIFE_MAX-fmwk->life;
    fmwk->life = FMISSION_LIFE_MAX;
  }else{
    fmwk->life += bonus;
  }
  //フレームを秒に変換して返す
  if( (bonus/60) == 0 ){
    bonus = 1; //最低1
  }else{
    bonus /= 60;
  }
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"ミッション type=%d key=%08x スコア加算 %d ボーナス %d\n",fmwk->type, fmwk->key, score_diff, bonus );
  return bonus;
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

//---------------------------------------------------------
/*
 * @brief スコア計算 アイテムゲット
 */
//---------------------------------------------------------
static BOOL score_CalcItem( const FEST_MISSION* fmwk, u16 param )
{
  return ( fmwk->param == 0 || fmwk->param == param );
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


