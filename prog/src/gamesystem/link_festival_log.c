//==============================================================================
/**
 * @file    link_festival_log.c
 * @brief   リンクフェスティバル の開催ログ収集関連
 * @author  iwasawa
 * @date    2011.11.11
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_comm.h"
#include "arc_def.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/link_festival_local.h"
#include "savedata/fmission_save.h"

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ構造体クリア
 */
//---------------------------------------------------------
void LINKFEST_LOG_Clear( LINKFEST_LOG* log )
{
  MI_CpuClear8( log, sizeof(LINKFEST_LOG) );
  log->mission.m_id = FMISSION_ID_NULL;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログパラメータ構造体クリア
 */
//---------------------------------------------------------
void LINKFEST_LOG_PARAM_Clear( LINKFEST_LOG_PARAM* log_prm )
{
  MI_CpuClear8( log_prm, sizeof(LINKFEST_LOG_PARAM) );
  FEST_MISSION_Clear( &log_prm->m_data );
  LINKFEST_LOG_Clear( &log_prm->prm );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログストア
 *
 * @param log 情報を格納したLINKFEST_LOG構造体型ワークへのポインタ
 */
//---------------------------------------------------------
LINKFEST_SCORELOG_SET_CODE LINK_FESTIVAL_ScoreLogStore( LINK_FESTIVAL* wk, LINKFEST_LOG_PARAM* src, int* log_score )
{
  int i;
  LINKFEST_LOG* log;
  *log_score = 0;

  if( !FEST_MISSION_IsSame(&wk->mission, &src->m_data) ){
    return LINKFEST_SCORELOG_SET_FAILED;  //違うミッションなのではじく
  }
  for(i = 0;i < wk->log_num; i++){
    log = &wk->log[i];

    if( log->score.tr_id == src->prm.score.tr_id ){ //上書き
      BOOL score_up = FALSE;


      /* スコア更新
       * 新しく届いたスコアが保存されているものより小さければ更新しない
       * 参加→リタイア→別ミッション→再参加、でスコアが巻き戻る対策
       */
      if( src->prm.score.value > log->score.value ){
        log->score.value = src->prm.score.value;  //より高いスコアに更新
        score_up = TRUE;
      }
//    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS log%d up tr_id=%d, score=%d\n",i, log->score.tr_id,log->score.value );

      *log_score = log->score.value;  //今のスコアを返す

      //現在地を更新
      if( log->score.tmap_root != src->prm.score.tmap_root ){ //場所が変わっていたら移動ログ更新
        log->score.tmap_root = src->prm.score.tmap_root;
        return LINKFEST_SCORELOG_SET_MAP_CHANGE+score_up;
      }
      return LINKFEST_SCORELOG_SET_UPDATE+score_up;
    }
  }
  if( wk->log_num >= LINKFEST_LOG_MAX ){
//    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS BufFull\n");
    return LINKFEST_SCORELOG_SET_FAILED;  //もう入らない
  }
  //新規
  log = &wk->log[wk->log_num];
  log->score = src->prm.score;
  *log_score = log->score.value;  //今のスコアを返す
//  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS nLog%d tr_id=%d, score=%d\n",wk->log_num, log->score.tr_id,log->score.value );
  ++wk->log_num;
  return LINKFEST_SCORELOG_SET_NEW_ENTRY;
}

//以下デバッグルーチン
#ifdef PM_DEBUG
//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 参加人数セット
 */
//---------------------------------------------------------
static BOOL deb_check_hit_fake_id( LINK_FESTIVAL* wk, u16 id )
{
  int i;

  for(i = 0;i < wk->log_num; i++){
    if( wk->log[i].score.tr_id == id ) return TRUE;
  }
  return FALSE;
}
static u16 deb_make_fake_id( LINK_FESTIVAL* wk, u16 id )
{
  do{
    ++id;
  }while( deb_check_hit_fake_id( wk, id ) );

  return id;
}

void DEB_LINKFEST_SetEntryLogNum( LINK_FESTIVAL* wk, int value )
{
  int i;

  if( value == 0 ){
    for( i = 0;i < wk->log_num; i++ ){
      LINKFEST_LOG_Clear( &wk->log[i] );
    }
  }else if( value > wk->log_num ){  //指定値の方が現エントリー数より多いのでダミー追加
    u16 id = 0;
    for( i = wk->log_num; i < value; i++ ){
      id = deb_make_fake_id( wk, id );
      wk->log[i].score.tr_id = id;
      wk->log[i].score.tmap_root = 0;
      wk->log[i].score.value = 0;
    }
  }else{
    //指定値の方が現エントリーより少ないので、余分を強制削除
    for( i = wk->log_num; i < value; i++ ){
      LINKFEST_LOG_Clear( &wk->log[i] );
    }
  }
  wk->log_num = value;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 参加人数ゲット
 */
//---------------------------------------------------------
int DEB_LINKFEST_GetEntryLogNum( LINK_FESTIVAL* wk )
{
  return wk->log_num;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 参加メンバーパラメータゲット
 */
//---------------------------------------------------------
LINKFEST_LOG* DEB_LINKFEST_GetEntryLogParam( LINK_FESTIVAL* wk, int idx )
{
  return &wk->log[idx];
}


#endif  //PM_DEBUG


