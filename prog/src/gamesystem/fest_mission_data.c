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
#include "fmission_data.naix"

#define MIN2SEC(x)  (x*60)

typedef enum{
 WSET_MODE_TITLE,
 WSET_MODE_EXP,
}WSET_MODE;

#define WORDSET_FMISSION_BUFLEN (18*2*2+EOM_SIZE)

static const u8 DATA_FMissionDataType[FMISSION_TYPE_MAX] = {
  0,
  FMISSION_WSET_MONSNAME, //TYPE_EFFECT_ENC
  FMISSION_WSET_MONSNAME, //TYPE_MUSHROOM_ENC
  FMISSION_WSET_EVENT,    //TYPE_EVENT
  FMISSION_WSET_ITEMNAME, //TYPE_ITEM
  FMISSION_WSET_ITEMNAME, //TYPE_NPC
};

static void wset_MissionParam( WORDSET* wset, STRBUF* tmp, u8 bufID, FMISSION_WSET type, int value );
//SDK_COMPILER_ASSERT( sizeof(FMISSION_ARC_COMMON) <= 6); //フェスミッションコモン構造体型サイズは6byte

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
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_fest_mission_dat, heapID );
 
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
 *  @brief  フェスミッション常駐データをアロケートしたメモリに取得する
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARC_COMMON* FMISSION_DATA_AllocArcCommonAllByHandle( ARCHANDLE* handle, HEAPID heapID )
{
  FMISSION_ARC_COMMON* buf = GFL_ARC_LoadDataAllocByHandle( handle, NARC_fmission_data_fmission_data_common_bin, heapID );
  return buf;
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッション常駐データをアロケートしたメモリに取得する(アーカイブをその場で開く)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARC_COMMON* FMISSION_DATA_AllocArcCommonAllDirect( HEAPID heapID )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, GFL_HEAP_LOWID(heapID) );
  FMISSION_ARC_COMMON* buf = FMISSION_DATA_AllocArcCommonAllByHandle( handle, heapID );
  GFL_ARC_CloseDataHandle( handle );

  return buf;
}

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFMISSION_ARCDATA型ワークに取得する
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 */
//------------------------------------------------------------
void FMISSION_DATA_GetArcByHandle( ARCHANDLE* handle, FEST_MISSION_ID id, FMISSION_ARCDATA* fm_arc )
{
  GFL_ARC_LoadDataOfsByHandle( handle, NARC_fmission_data_fmission_data_common_bin,
      id*sizeof(FMISSION_ARC_COMMON), sizeof(FMISSION_ARC_COMMON), &fm_arc->common );
  GFL_ARC_LoadDataOfsByHandle( handle, NARC_fmission_data_fmission_data_bin,
      id*sizeof(FMISSION_ARC_PARAM), sizeof(FMISSION_ARC_PARAM), &fm_arc->param );
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(ハンドル版)
 *
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARCDATA* FMISSION_DATA_AllocArcAllByHandle( ARCHANDLE* handle, HEAPID heapID )
{
  int i;
  FMISSION_ARCDATA* buf = GFL_HEAP_AllocClearMemory( heapID, sizeof(FMISSION_ARCDATA)*FMISSION_ID_MAX);

  for( i = 0;i < FMISSION_ID_MAX;i++){
    FMISSION_DATA_GetArcByHandle( handle, i, &buf[i] );
  }
  return buf;
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(データマネージャ経由)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARCDATA* FMISSION_DATA_AllocArcAll( FMISSION_DATA_MAN* man, HEAPID heapID )
{
  return FMISSION_DATA_AllocArcAllByHandle( man->data_h, heapID );
}

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(アーカイブをその場で開く)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
FMISSION_ARCDATA* FMISSION_DATA_AllocArcAllDirect( HEAPID heapID )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID );
  FMISSION_ARCDATA* wk = FMISSION_DATA_AllocArcAllByHandle( handle, heapID );
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
  FEST_BEACON* beacon;
  FMISSION_ARC_PARAM* pp;

  MI_CpuClear8(fmwk,sizeof(FEST_MISSION));
  FMISSION_DATA_GetArcByHandle( handle, id, &dat);

  beacon = &fmwk->beacon;
  pp = &dat.param;

  //照合フィールドデータ
  beacon->m_id = id;
  beacon->m_level = level;
  beacon->base_param = pp->base_param;
  beacon->ex_param = pp->ex_param;
  beacon->distribution_f = FALSE;
  
  if( GET_VERSION() == VERSION_W2 ){
    beacon->version_idx = FMISSION_VERSION_WHITE;
  }else{
    beacon->version_idx = FMISSION_VERSION_BLACK;
  }

  //各種パラメータ
  beacon->item = pp->item;
  beacon->pay = pp->level[level].pay;
  beacon->goal = pp->level[level].goal;
  beacon->end_type = pp->end_type;
  
  beacon->pattern = pp->pattern;
  beacon->entry_flag_id = pp->entry_flag_id;
  beacon->pay_bonus = pp->pay_bonus;

  beacon->act_param = 0;
  beacon->time = pp->level[level].time;
  beacon->sp_item = pp->sp_item;
  
  beacon->sp_pay = pp->sp_pay;
  beacon->unit = pp->unit;
  beacon->time_bonus = pp->time_bonus;

  beacon->m_title_id = pp->m_title_id;
  beacon->m_exp_id = pp->m_exp_id;
  
  //アーカイブコモンデータセット
  fmwk->arc = dat.common;

  //FES_ROM
  if(beacon->pay > FMISSION_PAY_NUM_BASE_MAX-1){ //報酬の基礎値表示が2桁までしか対応してないよバグ対策で-1補正
    beacon->pay = FMISSION_PAY_NUM_BASE_MAX-1; //99max
  }
#ifdef PM_DEBUG
  if( fmwk->arc.host == FMISSION_HOST_COMM ){
    beacon->distribution_f = TRUE;
  }
#endif
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
 * @brief ミッションタイトル文字列取得
 *
 * ミッションパラメータからミッションタイトル文字列を生成
 */
//---------------------------------------------------------
void FMISSION_DATA_GetTitleStr( FMISSION_DATA_MAN* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID )
{
  const FEST_BEACON* beacon = &fmwk->beacon;
  const FMISSION_ARC_COMMON* arc = &fmwk->arc;
  WORDSET* wset = WORDSET_CreateEx( 2, WORDSET_FMISSION_BUFLEN, GFL_HEAP_LOWID(heapID));
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_TITLE, GFL_HEAP_LOWID(heapID) );
  
  //パラメータ
  wset_MissionParam( wset, tmp, 0, DATA_FMissionDataType[arc->m_type], beacon->base_param ); 
  IWASAWA_Printf("FmissionTitle type%d\n", arc->m_type );

  //目標
  WORDSET_RegisterNumber( wset, 1, beacon->goal, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  
  GFL_MSG_GetString( man->msg, beacon->m_title_id, tmp );
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
  const FEST_BEACON* beacon = &fmwk->beacon;
  const FMISSION_ARC_COMMON* arc = &fmwk->arc;
  WORDSET* wset = WORDSET_CreateEx( 2, WORDSET_FMISSION_BUFLEN, GFL_HEAP_LOWID(heapID));
  STRBUF* tmp = GFL_STR_CreateBuffer( BUFLEN_FMISSION_EXP, GFL_HEAP_LOWID(heapID) );
 
  //パラメータ
  wset_MissionParam( wset, tmp, 0, DATA_FMissionDataType[arc->m_type], beacon->base_param ); 
  
  //目標
  WORDSET_RegisterNumber( wset, 1, beacon->goal, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  GFL_MSG_GetString( man->msg, beacon->m_exp_id, tmp );
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

