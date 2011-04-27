//==============================================================================
/**
 * @file    fest_mission.c
 * @brief   リンクフェスティバル ミッション管理(NotWifi)
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

#include "print/wordset.h"
#include "waza_tool/wazano_def.h"
#include "arc_def.h"
#include "power.naix"
#include "poke_tool/status_rcv.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_nw.h"

#include "msg/msg_fest_mission.h"

typedef enum{
 WSET_MODE_TITLE,
 WSET_MODE_EXP,
}WSET_MODE;

#define WORDSET_FMISSION_BUFLEN (18*2*2+EOM_SIZE)

typedef struct _FMISSION_DATA_TYPE{
  u8  mission_type; ///<ミッションタイプ
  u8  param_wset;   ///<ミッションタイトルワードセットタイプ
}FMISSION_DATA_TYPE;

static const FMISSION_DATA_TYPE DATA_FMissionDataType[FMISSION_TYPE_MAX] = {
  { 0, 0 },
  { FMISSION_TYPE_POKE_SEARCH, FMISSION_WSET_MONSNAME },
  { FMISSION_TYPE_EVENT, FMISSION_WSET_EVENT },
  { FMISSION_TYPE_BALL_ITEM, FMISSION_WSET_ITEMNAME },
};

static void wset_MissionParam( GFL_MSGDATA* man, WORDSET* wset, STRBUF* tmp, u8 bufID, FMISSION_WSET type, int value, WSET_MODE mode );

//---------------------------------------------------------
/*
 * @brief ミッションパラメータ一括ワードセット
 */
//---------------------------------------------------------
void FEST_MISSION_WordsetParamAll( WORDSET* wset, FEST_MISSION* fmwk )
{
  int sec = LINKFEST_FRAME2SEC(fmwk->life);

#if 0
  WORDSET_RegisterNumber( wset, 0, fmwk->time, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 1, fmwk->target, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  wset_MissionParam( wset, 2, DATA_FMissionDataType[fmwk->type].param_wset,fmwk->param, WSET_MODE_TITLE );
#endif
  WORDSET_RegisterItemName( wset, 3, fmwk->item );
  WORDSET_RegisterNumber( wset, 4, fmwk->item_num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 5, sec/60, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 6, sec%60, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
}

//---------------------------------------------------------
/*
 * @brief ミッションタイトル文字列取得
 *
 * ミッションパラメータからミッションタイトル文字列を生成
 */
//---------------------------------------------------------
void FEST_MISSION_GetTitleStr( GFL_MSGDATA* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  WORDSET* wset = WORDSET_CreateEx( 2, WORDSET_FMISSION_BUFLEN, GFL_HEAP_LOWID(heapID));
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_TITLE, GFL_HEAP_LOWID(heapID) );

  //パラメータ
  wset_MissionParam( man, wset, tmp, 0, DATA_FMissionDataType[fmwk->type].param_wset, fmwk->param, WSET_MODE_TITLE ); 
  //目標
  WORDSET_RegisterNumber( wset, 1, fmwk->target, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  
  GFL_MSG_GetString( man, msg_fmission_title00+fmwk->type, tmp );
  WORDSET_ExpandStr( wset, str, tmp );

  GFL_STR_DeleteBuffer( tmp );
  WORDSET_Delete( wset );
}

//---------------------------------------------------------
/*
 * @brief ミッション説明文文字列取得
 *
 * ミッションパラメータからミッション説明文を生成
 */
//---------------------------------------------------------
void FEST_MISSION_GetExpStr( GFL_MSGDATA* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  WORDSET* wset = WORDSET_CreateEx( 3, WORDSET_FMISSION_BUFLEN, GFL_HEAP_LOWID(heapID));
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_EXP, GFL_HEAP_LOWID(heapID) );
  
  //時間
  WORDSET_RegisterNumber( wset, 0, fmwk->time, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  //目標
  WORDSET_RegisterNumber( wset, 1, fmwk->target, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  //パラメータ
  wset_MissionParam( man, wset, tmp, 2, DATA_FMissionDataType[fmwk->type].param_wset, fmwk->param, WSET_MODE_EXP ); 
  
  GFL_MSG_GetString( man, msg_fmission_exp00+fmwk->type, tmp );
  WORDSET_ExpandStr( wset, str, tmp );

  GFL_STR_DeleteBuffer( tmp );
  WORDSET_Delete( wset );
}

//---------------------------------------------------------
/*
 * @brief ミッションパラメータワードセット
 */
//---------------------------------------------------------
static void wset_MissionParam( GFL_MSGDATA* man, WORDSET* wset, STRBUF* tmp, u8 bufID, FMISSION_WSET type, int value, WSET_MODE mode )
{
  switch( type ){
  case FMISSION_WSET_MONSNAME:
    WORDSET_RegisterPokeMonsNameNo( wset, bufID, value );
    break;
  case FMISSION_WSET_ITEMNAME:
    WORDSET_RegisterItemName( wset, bufID, value );
    break;
  case FMISSION_WSET_EVENT:
    if( mode == WSET_MODE_TITLE ){
      GFL_MSG_GetString( man, msg_fmission_evtitle00+value, tmp );
    }else{
      GFL_MSG_GetString( man, msg_fmission_evexp00+value, tmp );
    }
    WORDSET_RegisterWord( wset, bufID, tmp, PM_NEUTRAL, TRUE, PM_LANG );
    break;
  }
}

