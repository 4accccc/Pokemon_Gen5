//====================================================================
/*
 *  @file fest_mission_data.h
 *  @brief  フェスミッションアーカイブデータ操作
 *  @author Miyuki Iwasawa
 *  @date   11.02.22
 */
//====================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"

#include "field/zonedata.h"
#include "sound/pm_sndsys.h"

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode

#include "arc/fieldmap/zone_id.h"
#include "poke_tool/monsno_def.h"
#include "tr_tool/trno_def.h"
#include "item/itemsym.h"
#include "waza_tool/wazano_def.h"

#include "savedata/intrude_save.h"
#include "savedata/playtime.h"

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/game_beacon_local.h" //許可なく他でinclude禁止

#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_data.h"

#include "message.naix"
#include "msg/msg_fest_mission.h"

#define MIN2SEC(x)  (x*60)

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
  { FMISSION_TYPE_POKEBTL, FMISSION_WSET_MONSNAME },
  { FMISSION_TYPE_EVENT, FMISSION_WSET_EVENT },
  { FMISSION_TYPE_ITEM, FMISSION_WSET_ITEMNAME },
  { FMISSION_TYPE_NPC, FMISSION_WSET_ITEMNAME },
  { FMISSION_TYPE_TRBTL, FMISSION_WSET_ITEMNAME },
};

static void wset_MissionParam( WORDSET* wset, STRBUF* tmp, u8 bufID, FMISSION_WSET type, int value );

//------------------------------------------------------------
/*
 *  @brief  ミッションデータマネージャー初期化
 *
 * FMISSION_DATA_Release( FMISSION_DATA_MAN* man )で開放すること
 */
//------------------------------------------------------------
FMISSION_DATA_MAN*  FMISSION_DATA_Init( HEAPID heapID )
{
  FMISSION_DATA_MAN* man = GFL_HEAP_AllocClearMemory(heapID, sizeof(FMISSION_DATA_MAN));

  man->data_h = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID );
  man->msg = GFL_MSG_Create(
		GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_fest_mission_dat, heapID );
 
  return man;
}

//------------------------------------------------------------
/*
 *  @brief  ミッションデータマネージャー開放
 */
//------------------------------------------------------------
void FMISSION_DATA_Release( FMISSION_DATA_MAN* man )
{
  GFL_MSG_Delete( man->msg );
  GFL_ARC_CloseDataHandle( man->data_h );
  GFL_HEAP_FreeMemory( man );
}

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFMISSION_ARCDATA型ワークに取得する
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 */
//------------------------------------------------------------
void FMISSION_DATA_GetArcParamByHandle( ARCHANDLE* handle, FEST_MISSION_ID id, FMISSION_ARCDATA* fm_arc )
{
  GFL_ARC_LoadDataOfsByHandle( handle, 0,
      id*sizeof(FMISSION_ARCDATA), sizeof(FMISSION_ARCDATA), fm_arc );
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(ハンドル版)
 *
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARCDATA* FMISSION_DATA_AllocArcParamAllByHandle( ARCHANDLE* handle, HEAPID heapID )
{
  return (FMISSION_ARCDATA*)GFL_ARC_LoadDataAllocByHandle( handle, 0, heapID );
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(データマネージャ経由)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARCDATA* FMISSION_DATA_AllocArcParamAll( FMISSION_DATA_MAN* man, HEAPID heapID )
{
  return FMISSION_DATA_AllocArcParamAllByHandle( man->data_h, heapID );
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(アーカイブをその場で開く)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARCDATA* FMISSION_DATA_AllocArcParamAllDirect( HEAPID heapID )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID );
  FMISSION_ARCDATA* wk = FMISSION_DATA_AllocArcParamAllByHandle( handle, heapID );
  GFL_ARC_CloseDataHandle( handle );

  return wk;
}

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する(ハンドル指定版)
 */
//------------------------------------------------------------
void FMISSION_DATA_GetMissionParamByHandle( ARCHANDLE* handle, FEST_MISSION_ID id, FMISSION_LEVEL level, FEST_MISSION* fmwk )
{
  FMISSION_ARCDATA dat;

  MI_CpuClear8(fmwk,sizeof(FEST_MISSION));
  FMISSION_DATA_GetArcParamByHandle( handle, id, &dat);

  fmwk->m_id = id;
  fmwk->m_type = dat.m_type; 
  fmwk->param = dat.param;
  
  fmwk->m_time = dat.level[level].time;
  fmwk->goal = dat.level[level].goal;
  fmwk->pay = dat.level[level].pay;
  fmwk->item = dat.item;
  fmwk->end_type = dat.end_type;
  fmwk->time_bonus = dat.time_bonus;
  fmwk->pay_bonus = dat.pay_bonus;

  fmwk->life = fmwk->m_time*60; //sec2frame
}

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する(データマネージャ経由)
 */
//------------------------------------------------------------
void FMISSION_DATA_GetMissionParam( FMISSION_DATA_MAN* man, FEST_MISSION_ID id, FMISSION_LEVEL level, FEST_MISSION* fmwk )
{
  FMISSION_DATA_GetMissionParamByHandle( man->data_h, id, level, fmwk );
}

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する(アーカイブをその場で開く)
 */
//------------------------------------------------------------
void FMISSION_DATA_GetMissionParamDirect( FEST_MISSION_ID id, FMISSION_LEVEL level, FEST_MISSION* fmwk, HEAPID heapID )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID );
  FMISSION_DATA_GetMissionParamByHandle( handle, id, level, fmwk );
  GFL_ARC_CloseDataHandle( handle );

}


//---------------------------------------------------------
/*
 * @brief ミッションパラメータ一括ワードセット
 */
//---------------------------------------------------------
void FMISSION_DATA_WordsetParamAll( WORDSET* wset, FEST_MISSION* fmwk )
{
  int sec = LINKFEST_FRAME2SEC(fmwk->life);

#if 0
  WORDSET_RegisterNumber( wset, 0, fmwk->m_time, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 1, fmwk->goal, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  wset_MissionParam( wset, 2, DATA_FMissionDataType[fmwk->m_type].param_wset,fmwk->param, WSET_MODE_TITLE );
#endif
  WORDSET_RegisterItemName( wset, 3, fmwk->item );
  WORDSET_RegisterNumber( wset, 4, fmwk->pay, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
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
void FMISSION_DATA_GetTitleStr( FMISSION_DATA_MAN* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  FMISSION_ARCDATA  fm_arc;
  WORDSET* wset = WORDSET_CreateEx( 2, WORDSET_FMISSION_BUFLEN, GFL_HEAP_LOWID(heapID));
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_TITLE, GFL_HEAP_LOWID(heapID) );
  FMISSION_DATA_GetArcParamByHandle( man->data_h, fmwk->m_id, &fm_arc );

  //パラメータ
  wset_MissionParam( wset, tmp, 0, DATA_FMissionDataType[fmwk->m_type].param_wset, fmwk->param ); 
  IWASAWA_Printf("FmissionTitle type%d\n", fmwk->m_type );

  //目標
  WORDSET_RegisterNumber( wset, 1, fmwk->goal, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  
  GFL_MSG_GetString( man->msg, fm_arc.m_title_id, tmp );
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
void FMISSION_DATA_GetExpStr( FMISSION_DATA_MAN* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  FMISSION_ARCDATA  fm_arc;
  WORDSET* wset = WORDSET_CreateEx( 3, WORDSET_FMISSION_BUFLEN, GFL_HEAP_LOWID(heapID));
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_EXP, GFL_HEAP_LOWID(heapID) );
 
  FMISSION_DATA_GetArcParamByHandle( man->data_h, fmwk->m_id, &fm_arc );
  
  //時間
  WORDSET_RegisterNumber( wset, 0, fmwk->m_time, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  //目標
  WORDSET_RegisterNumber( wset, 1, fmwk->goal, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  //パラメータ
  wset_MissionParam( wset, tmp, 2, DATA_FMissionDataType[fmwk->m_type].param_wset, fmwk->param ); 
  
  GFL_MSG_GetString( man->msg, fm_arc.m_exp_id, tmp );
  WORDSET_ExpandStr( wset, str, tmp );

  GFL_STR_DeleteBuffer( tmp );
  WORDSET_Delete( wset );
}

//---------------------------------------------------------
/*
 * @brief ミッションパラメータワードセット
 */
//---------------------------------------------------------
static void wset_MissionParam( WORDSET* wset, STRBUF* tmp, u8 bufID, FMISSION_WSET type, int value )
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

