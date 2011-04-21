//====================================================================================
/**
 *  @file   fmission_log_accessor_fv.c
 *  @brief  フェスミッションログ　データアクセス(フィールド FMISSION_VIEWオーバーレイ)
 *  @author Miyuki Iwasawa
 *  @date   11.03.23
 *
 *  fmission_logのルーチンのうち、フィールド上からだけアクセスできれば良い関数を集めてある
 *  FMissionViewバージョン
 */
//====================================================================================

#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission.h"
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"
#include "app/townmap_util.h"
#include "net_app\union\union_beacon_tool.h"
#include "app/research_radar/research_data.h"   //QUESTION_NUM_PER_TOPIC
#include "app/research_radar/question_id.h"
#include "app/research_radar/questionnaire_index.h"
#include "savedata/playtime.h"
#include "field/research_team_def.h"
#include "system/gfl_use.h"
#include "field/zonedata.h"
#include "savedata/resonance_resort_sv.h"

#include "game_beacon_local.h"
#include "debug/debug_flg.h"

#include "link_festival_local.h"
#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_fv.h"

#define BUFLEN_LOGACT_WORDSET_TMP (10+EOM_SIZE)

//------------------------------------------------------------------
/**
 * ログテーブルにエントリーしているログ数を取得
 *
 * @param   tbl	  ログテーブルへのポインタ
 * @retval  int		ログ登録数
 */
//------------------------------------------------------------------
int FMISSION_LOG_TBL_GetEntryNum( FMISSION_LOG_TBL* tbl )
{
  return tbl->entry_num;
}

//------------------------------------------------------------------
/**
 * ログテーブルの描画トップindexを取得
 *
 * @param   tbl	  ログテーブルへのポインタ
 * @retval  int	  描画トップindex	
 */
//------------------------------------------------------------------
int FMISSION_LOG_TBL_GetViewTopIdx( FMISSION_LOG_TBL* tbl )
{
  return tbl->view_top;
}

//------------------------------------------------------------------
/**
 * ログテーブルの描画トップindexをセット
 *
 * @param   tbl	  ログテーブルへのポインタ
 * @param   top		セットする描画Topのindex
 */
//------------------------------------------------------------------
void FMISSION_LOG_TBL_SetViewTopIdx( FMISSION_LOG_TBL* tbl, u8 top )
{
  tbl->view_top = top;
}

//------------------------------------------------------------------
/**
 * ログワークをAllocする
 */
//------------------------------------------------------------------
FMISSION_LOG* FMISSION_LOG_AllocWork( HEAPID heapID )
{
  return (FMISSION_LOG*)GFL_HEAP_AllocClearMemory(heapID, sizeof(FMISSION_LOG));
}

//------------------------------------------------------------------
/*
 *  ログスタック データ取得
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  p_log   取得するログ情報を格納するFMISSION_LOG型ワークへのポインタ
 *
 *  @retval TRUE    データ取得成功
 *  @retval FALSE   スタックテーブルにデータがない
 *
 *  情報取得に成功した場合、スタックテーブル内のデータが前詰めされます
 */
//------------------------------------------------------------------
BOOL FMISSION_LOG_StackTbl_GetLog( FMISSION_LOG_TBL* tbl, FMISSION_LOG* p_log )
{
  if( tbl->log[0].action == FMISSION_LOG_ACT_NULL ){
    return FALSE;
  }
  *p_log = tbl->log[0];

  //スタックテーブル前詰め
  GFL_STD_MemCopy32( &tbl->log[1], &tbl->log[0], sizeof(FMISSION_LOG)*FMISSION_LOG_END);
  FMISSION_LOG_Clear( &tbl->log[FMISSION_LOG_END] );
  --tbl->entry_num;

  return TRUE;
}

//------------------------------------------------------------------
/**
 * リング式ログテーブルのTopからのオフセットindexを配列への実参照indexに変換
 *
 * @param   tbl		  ログビューテーブルへのポインタ
 * @param   ofs     リングトップからのindexオフセット
 *
 * @retval  int		配列の実参照index
 */
//------------------------------------------------------------------
int FMISSION_LOG_ViewTbl_GetOfs2Idx( FMISSION_LOG_TBL *tbl, int ofs )
{
  int top = tbl->ring_top;

  if( ofs < 0 ){
    return ((top+ofs+FMISSION_LOG_MAX)%FMISSION_LOG_MAX);
  }else{
    return ((top+ofs)%FMISSION_LOG_MAX);
  }
}

//------------------------------------------------------------------
/**
 * ログビューテーブルへ新しいログを登録する(リングバッファ方式)
 *
 * @param   tbl		      ログテーブルへのポインタ
 * @param   p_log	    	登録するログ
 */
//------------------------------------------------------------------
int FMISSION_LOG_ViewTbl_SetLog( FMISSION_LOG_TBL* tbl, const FMISSION_LOG* p_log )
{
  int i, log_no;
  
  //一番古い一つを上書き
  log_no =  FMISSION_LOG_ViewTbl_GetOfs2Idx( tbl, -1 );
  tbl->log[log_no] = *p_log;

  //リングトップを更新
  tbl->ring_top =  log_no;
  if( tbl->entry_num < FMISSION_LOG_MAX ){
    tbl->entry_num++;
  }
  return 0;
}

//------------------------------------------------------------------
/**
 * リング式ログビューテーブルから指定オフセット位置のログ情報を取得する
 *
 * @param   tbl		      ログテーブルへのポインタ
 * @param   p_log	    	ログ情報代入先
 * @param   ofs		      リングバッファ先頭からのindexオフセット
 *
 * @retval  BOOL		    TRUE:ログ情報取得成功　　FALSE:取得失敗(データが無い)
 */
//------------------------------------------------------------------
BOOL FMISSION_LOG_ViewTbl_GetLog( FMISSION_LOG_TBL *tbl, FMISSION_LOG* p_log, int ofs )
{
  int log_no;
  GF_ASSERT( ofs < FMISSION_LOG_MAX);
  
  log_no = FMISSION_LOG_ViewTbl_GetOfs2Idx( tbl, ofs );
  if( tbl->log[log_no].action == FMISSION_LOG_ACT_NULL ){
    return FALSE;
  }
  *p_log = tbl->log[log_no];
  return TRUE;
}

//------------------------------------------------------------------
/*
 *  Gパワーテーブル データ取得
 *
 *  @param  tbl       FMISSION_GPOWER_TBL型構造体へのポインタ
 *  @param  name_buf  Gパワー発動者の名前を受けるSTRBUFへのポインタ
 *  @param  spot_f    配布(パワースポット)のデータかどうか？
 *
 *  @retval GPOWER_ID_XXXX    データ取得成功
 *  @retval GPOWER_ID_NULL    データ取得失敗
 *
 *  情報取得に成功した場合、テーブル内のデータが前詰めされます
 */
//------------------------------------------------------------------
GPOWER_ID FMISSION_GPOWER_TBL_GetData( FMISSION_GPOWER_TBL* tbl, STRBUF* name_buf, BOOL* spot_f )
{
  GPOWER_ID id;

  if( tbl->entry_num == 0 ){
    return GPOWER_ID_NULL;
  }
  GFL_STR_SetStringCodeOrderLength( name_buf, tbl->power[0].name, PERSON_NAME_SIZE+EOM_SIZE );
  *spot_f = tbl->power[0].spot_f;
  id = tbl->power[0].gpower_id;

  //テーブル前詰め
  GFL_STD_MemCopy32( &tbl->power[1], &tbl->power[0], sizeof(FMISSION_GPOWER)*FMISSION_GPOWER_TBL_END);
  FMISSION_GPOWER_Clear( &tbl->power[FMISSION_GPOWER_TBL_END] );
  --tbl->entry_num;

  return id;
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得　名前
 *
 * @param   p_log	    	ログ情報
 * @param   s_buf       PERSON_NAME_SIZE+EOM_SIZEを格納可能なSTRBUFのポインタ
 */
//------------------------------------------------------------------
void FMISSION_LOG_GetName( FMISSION_LOG* p_log, STRBUF* s_buf )
{
  GFL_STR_SetStringCodeOrderLength( s_buf, p_log->name, PERSON_NAME_SIZE+EOM_SIZE );
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得　 ポケモンニックネーム
 *
 * @param   p_log	    	ログ情報
 * @param   s_buf       MONS_NAME_SIZE+EOM_SIZEを格納可能なSTRBUFのポインタ
 */
//------------------------------------------------------------------
void FMISSION_LOG_GetPokeNickname( FMISSION_LOG* p_log, STRBUF* s_buf )
{
  GFL_STR_SetStringCodeOrderLength( s_buf, p_log->str, MONS_NAME_SIZE+EOM_SIZE );
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得　 一言メッセージ
 *
 * @param   p_log	    	ログ情報
 * @param   
 * @param   s_buf       GAMEBEACON_FREEWORD_MESSAGE_LENを格納可能なSTRBUFのポインタ
 */
//------------------------------------------------------------------
void FMISSION_LOG_GetFreeWord( FMISSION_LOG* p_log, STRBUF* s_buf )
{
  GFL_STR_SetStringCodeOrderLength( s_buf, p_log->str, GAMEBEACON_FREEWORD_MESSAGE_LEN );
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得   ユニオン見た目コード
 *
 * @param   p_log	    	ログ情報
 */
//------------------------------------------------------------------
u8 FMISSION_LOG_GetTrView( FMISSION_LOG* p_log )
{
  if( p_log->sex == PM_MALE){
    return p_log->tr_view;
  }
  return p_log->tr_view + UNION_VIEW_INDEX_WOMAN_START;
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得   性別
 *
 * @param   p_log	    	ログ情報
 */
//------------------------------------------------------------------
u8 FMISSION_LOG_GetSex( FMISSION_LOG* p_log )
{
  return p_log->sex;
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得   プレイヤー自身のログかどうか？
 *
 * @param   p_log	    	ログ情報
 */
//------------------------------------------------------------------
BOOL FMISSION_LOG_IsPlayerLog( FMISSION_LOG* p_log )
{
  return p_log->mine_f;
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得   ログアクション
 *
 * @param   p_log	    	ログ情報
 */
//------------------------------------------------------------------
FMISSION_LOG_ACTION FMISSION_LOG_GetAction( FMISSION_LOG* p_log )
{
  return p_log->action;
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得   ログアクションパラメータ
 *
 * @param   p_log	    	ログ情報
 */
//------------------------------------------------------------------
u16 FMISSION_LOG_GetActionParam( FMISSION_LOG* p_log )
{
  return p_log->param;
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得   ログアクションパラメータのWordset展開 
 *
 * @param   p_log	    	ログ情報
 */
//------------------------------------------------------------------
void FMISSION_LOG_ActionWordset( FMISSION_LOG* p_log,  WORDSET* wset, HEAPID tmpHeapID )
{
  u8 type;
  STRBUF *s_buf;

  WORDSET_ClearAllBuffer( wset );
  
  s_buf = GFL_STR_CreateBuffer( BUFLEN_LOGACT_WORDSET_TMP, GFL_HEAP_LOWID(tmpHeapID));
  if( s_buf == NULL ){
    return;
  }
  switch( p_log->action ){
  case FMISSION_LOG_ACT_ITEM_USE:
  case FMISSION_LOG_ACT_ITEM_GET:
    WORDSET_RegisterItemName( wset, 0, p_log->param );
    break;
  case FMISSION_LOG_ACT_MAP_CHANGE:
    WORDSET_RegisterPlaceName( wset, 0, ZONEDATA_GetPlaceNameID(p_log->param) );
    break;
  case FMISSION_LOG_ACT_FREEWORD:
    FMISSION_LOG_GetFreeWord( p_log, s_buf );
    WORDSET_RegisterWord( wset, 0, s_buf, p_log->sex, TRUE, PM_LANG);
    break;
  case FMISSION_LOG_ACT_POKE_DOWN:
    FMISSION_LOG_GetPokeNickname( p_log, s_buf );
    WORDSET_RegisterWord( wset, 0, s_buf, p_log->sex, TRUE, PM_LANG);
    break;
  case FMISSION_LOG_ACT_GPOWER:
    FMISSION_LOG_GetName( p_log, s_buf );
    WORDSET_RegisterWord( wset, 0, s_buf, p_log->sex, TRUE, PM_LANG);
    WORDSET_RegisterGPowerName( wset, 1, p_log->param );
    break;
  default:
    WORDSET_RegisterPokeMonsNameNo( wset, 0, p_log->param );
    break;
  }
 
  GFL_STR_DeleteBuffer(s_buf);
}

