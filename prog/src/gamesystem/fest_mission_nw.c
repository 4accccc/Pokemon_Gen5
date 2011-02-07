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

typedef struct _FMISSION_DATA_TYPE{
  u8  mission_type; ///<ミッションタイプ
  u8  param_wset;   ///<ミッションタイトルワードセットタイプ
}FMISSION_DATA_TYPE;

static const FMISSION_DATA_TYPE DATA_FMissionDataType[] = {
  { FMISSION_TYPE_WILDBTL_WIN, FMISSION_WSET_MONSNAME },
  { FMISSION_TYPE_WILDBTL_WIN, FMISSION_WSET_MONSNAME },
};

static void wset_MissionParam( WORDSET* wset, u8 bufID, FMISSION_WSET type, int value );

//---------------------------------------------------------
/*
 * @brief ミッションパラメータ一括ワードセット
 */
//---------------------------------------------------------
void FEST_MISSION_WordsetParamAll( WORDSET* wset, FEST_MISSION* fmwk )
{
  WORDSET_RegisterNumber( wset, 0, fmwk->time, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 1, fmwk->target, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  wset_MissionParam( wset, 2, DATA_FMissionDataType[fmwk->id].param_wset,fmwk->param);
  WORDSET_RegisterItemName( wset, 3, fmwk->reward_item );
  WORDSET_RegisterNumber( wset, 4, fmwk->reward_num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 5, fmwk->life/(60*60), 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 6, (fmwk->life/60)%60, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
}

//---------------------------------------------------------
/*
 * @brief ミッションタイトル文字列取得
 *
 * ミッションパラメータからミッションタイトル文字列を生成
 */
//---------------------------------------------------------
void FEST_MISSION_GetTitleStr( WORDSET* wset, GFL_MSGDATA* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_TITLE, GFL_HEAP_LOWID(heapID) );
  GFL_MSG_GetString( man, msg_fmission_title00+fmwk->id, tmp );

  wset_MissionParam( wset, 0, DATA_FMissionDataType[fmwk->id].param_wset, fmwk->param ); 
  WORDSET_ExpandStr( wset, str, tmp );

  GFL_STR_DeleteBuffer( tmp );
}

//---------------------------------------------------------
/*
 * @brief ミッション説明文文字列取得
 *
 * ミッションパラメータからミッション説明文を生成
 */
//---------------------------------------------------------
void FEST_MISSION_GetExpStr( WORDSET* wset, GFL_MSGDATA* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_EXP, GFL_HEAP_LOWID(heapID) );
  GFL_MSG_GetString( man, msg_fmission_exp00+fmwk->id, tmp );

  //時間
  WORDSET_RegisterNumber( wset, 0, fmwk->time, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  //目標
  WORDSET_RegisterNumber( wset, 1, fmwk->target, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  //パラメータ
  wset_MissionParam( wset, 2, DATA_FMissionDataType[fmwk->id].param_wset, fmwk->param ); 
  WORDSET_ExpandStr( wset, str, tmp );

  GFL_STR_DeleteBuffer( tmp );
}

//---------------------------------------------------------
/*
 * @brief ミッションパラメータワードセット
 */
//---------------------------------------------------------
static void wset_MissionParam( WORDSET* wset, u8 bufID, FMISSION_WSET type, int value )
{
  switch( type ){
  case FMISSION_WSET_MONSNAME:
    WORDSET_RegisterPokeMonsNameNo( wset, bufID, value );
    break;
  case FMISSION_WSET_ITEMNAME:
    WORDSET_RegisterItemName( wset, bufID, value );
    break;
  }
}

