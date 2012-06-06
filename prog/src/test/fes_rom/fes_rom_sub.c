//=============================================================================
/**
 *
 *	@file		fes_rom_sub.c
 *	@brief  フェスミッション配信画面 サブ
 *	@author	Miyuki Iwasawa
 *	@data		2010.02.01
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 簡易会話表示システム
#include "system/pms_draw.h"
#include "system/palanm.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "fes_rom_gra.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

#include "field/zonedata.h"
#include "field/map_change_type.h"
//外部公開
#include "fes_rom.h"

#include "message.naix"
#include "msg/msg_fest_mission.h"  // GMM
#include "beacon_status.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "config_gra.naix"
#include "wifi_unionobj.naix"
#include "wmi.naix"

#include "fes_rom_local.h"
#include "fes_rom_sub.h"
#include "fes_rom_comm.h"

#include "arc/wifi_unionobj_plt.cdat" //ユニオンキャラパレット参照テーブル
#include "arc/fieldmap/zone_id.h"

//////////////////////////////////////////////////////////////
//定数定義

typedef struct BEACON_WIN_COLOR{
 PRINTSYS_LSB color;
 u8           base;
}BEACON_WIN_COLOR;

static const BEACON_WIN_COLOR DATA_BeaconWinColor[] = {
  { FCOL_WIN01, FCOL_WIN_BASE1 }, 
  { FCOL_WIN02, FCOL_WIN_BASE2 }, 
};

#define FCOL_BEACON_BASE( n ) ( DATA_BeaconWinColor[n].base ) 
#define FCOL_BEACON_COL( n )  ( DATA_BeaconWinColor[n].color )

#include "fes_rom.cdat"

#define InPrmWSet( code ) ( DATA_InputParamTbl[code].type )
#define InPrmMin( code )  ( DATA_InputParamTbl[code].min )
#define InPrmMax( code )  ( DATA_InputParamTbl[code].max )
#define InPrmKeta( code ) ( DATA_InputParamTbl[code].keta )

//////////////////////////////////////////////////////////////

static void targetWin_Clear( TARGET_WIN* twin );
static void targetWin_ClearAll( FES_ROM_WORK* wk );

//プロトタイプ


//パラメータ系
static void param_Draw( FES_ROM_WORK* wk, int idx );
static void param_DrawAll( FES_ROM_WORK* wk, int target_idx );
static void param_DrawAllEx( FES_ROM_WORK* wk, int target_idx1, int target_idx2 );
static void param_Trans( FES_ROM_WORK* wk );
static void param_DrawTrans( FES_ROM_WORK* wk, int idx );
static void param_Clear( FES_ROM_WORK* wk, int idx );

static void param_BasePop( FES_ROM_WORK* wk );
static void param_BasePush( FES_ROM_WORK* wk );

static void param_MissionPop( FES_ROM_WORK* wk );
static void param_MissionPush( FES_ROM_WORK* wk );
static void param_MissionUpdate( FES_ROM_WORK* wk, M_UPDATE_CODE up_code );
static int param_MissionPopToFMWK( FES_ROM_WORK* wk, int edit_id, FEST_MISSION* fmwk );
static void param_MissionPopToFMWKEdit( FES_ROM_WORK* wk, FEST_MISSION* fmwk );
static int param_MissionEnableNum( FES_ROM_WORK* wk );

static void param_SendPop( FES_ROM_WORK* wk );

static INPUT_CODE meditEx_InputCodeGet( FES_ROM_WORK* wk );

//入力系
static int input_ParamNextPos( FES_ROM_WORK* wk, int dir );
static int input_BaseExe( FES_ROM_WORK* wk, const PARAM_SET* pset, int param );
static int input_MEditExe( FES_ROM_WORK* wk, const PARAM_SET* pset, int param );
static int exe_InputSwitch( FES_ROM_WORK* wk, const PARAM_SET* pset, int diff );
static int exe_InputSwitchEx( FES_ROM_WORK* wk, const PARAM_SET* pset, INPUT_CODE input_code, int diff );

//プリント系
static void print_GetMsgToBuf( FES_ROM_WORK* wk, u8 msg_id );
static inline void print_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str );
static BOOL print_ParamWordset( FES_ROM_WORK* wk, u8 idx, int value, WSET_TYPE type );
static BOOL print_ParamWordsetICode( FES_ROM_WORK* wk, u8 idx, int value, INPUT_CODE input_code );
static BOOL print_ParamWordsetICodeEx( FES_ROM_WORK* wk, u8 idx, int value, INPUT_CODE input_code, int arg );
static BOOL print_ParamWordsetEx( FES_ROM_WORK* wk, u8 idx, int value, WSET_TYPE type, int arg, int keta );

//汎用サブルーチン系
static void sub_PlaySE( u16 se_no );
static void sub_PlttVramTrans( u16* p_data, u8 target, u16 pos, u16 num );

static void act_AnmStart( GFL_CLWK* act, u8 anm_no );
static void act_SetPosition( GFL_CLWK* act, s16 x, s16 y, u16 setsf );

static void draw_MsgWinFromID( FES_ROM_WORK* wk, TARGET_VIEW view, int msg_id );
static void draw_MsgWin( FES_ROM_WORK* wk, TARGET_VIEW view, STRBUF* str );
static void draw_MsgClear( FES_ROM_WORK* wk, TARGET_VIEW view );

static void draw_UnionObjUpdate( FES_ROM_WORK* wk, u8 char_no );
static void draw_BeaconData( FES_ROM_WORK* wk, BMP_WIN* win, STRBUF* str, u8 px, u8 fx, u8 sx, u8 sy, u8 col_idx );

static void effReq_PopupMsg( FES_ROM_WORK* wk );
static void printReq_BmpwinPrint( FES_ROM_WORK* wk, BMP_WIN* win, u8 x, u8 y, STRBUF* str, PRINTSYS_LSB color );

//セーブリクエスト
static void taskReq_Save( FES_ROM_WORK* wk );



//==============================================================================
//==============================================================================
//==============================================================================
/*
 *  @brief  フェス配信画面画面初期描画
 */
void FesRom_InitialDraw( FES_ROM_WORK* wk )
{
  int i;

  targetWin_ClearAll( wk );
  wk->seq = 0;
  wk->sub_seq = 0;

  switch( wk->app_mode ){
  case FESROM_APP_MODE_BASE:
    wk->prmset_tbl = DATA_ParamSetBase;
    wk->prmset_num = PRM_BASE_ID_MAX;
    param_BasePop( wk );
    FES_ROM_SetFadeInOutSeq( wk, SEQ_BASE_MAIN, SEQ_EXIT );
    param_DrawAll( wk, POS_TARGET_NONE );
    break;
  case FESROM_APP_MODE_MEDIT:
    wk->prmset_tbl = DATA_ParamSetMEdit;
    wk->prmset_num = PRM_MEDIT_ID_MAX;
    param_MissionPop( wk );
    FES_ROM_SetFadeInOutSeq( wk, SEQ_MEDIT_MAIN, SEQ_EXIT );
    param_DrawAll( wk, POS_TARGET_NONE );
    break;
  case FESROM_APP_MODE_SEND:
    wk->prmset_tbl = DATA_ParamSetSend;
    wk->prmset_num = PRM_SEND_ID_MAX;
    if( wk->initial_f ){
      param_BasePop( wk );
    }
    param_SendPop( wk );
    
    wk->pos = 0xFF;
    wk->old_date_time = 0;

    FES_ROM_SetFadeInOutSeq( wk, SEQ_SEND_MAIN, SEQ_EXIT );
    param_Draw( wk, PRMIDX_SEND_START_TIME );
    param_Draw( wk, PRMIDX_SEND_END_TIME );
    param_DrawTrans( wk, PRMIDX_SEND_END_EXP );
    param_DrawTrans( wk, PRMIDX_SEND_NOW_RTC );
    break;
  }

#if 0
  GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, wk->list[wk->list_top]);
#endif

  wk->initial_f = FALSE;
  GFL_NET_ReloadIconTopOrBottom( FALSE, wk->tmpHeapID );
}

//========================================================
//ベースパラメータのPushPop
static void param_BasePop( FES_ROM_WORK* wk )
{
  int i;

  MI_CpuClear8( &wk->param[PRM_BASE_ID_START], sizeof(int)*PRM_BASE_ID_MAX );
  
  wk->param[PRM_BASE_BEACON_INTERVAL] = wk->sys_save->beacon_interval;
  wk->param[PRM_BASE_START_TIME] = wk->sys_save->start_time;
  wk->param[PRM_BASE_END_TIME] = wk->sys_save->end_time;
  wk->param[PRM_BASE_TR_VIEW] = wk->sys_save->tr_view;
  wk->param[PRM_BASE_MODE] = wk->sys_save->mode;

  for( i = 0;i < FESROM_MISSION_DATA_MAX; i++ ){
    int idx = PARAM_BASE_MSET_NUM*i;
    FESROM_MISSION* pm = &wk->sys_save->mission[i];
    wk->param[PRM_BASE_MSEND_ID01+idx] = pm->param.m_id;
    wk->param[PRM_BASE_MSEND_TIME01+idx] = pm->send.time;
    wk->param[PRM_BASE_MSEND_BLANK01+idx] = pm->send.blank;
    wk->param[PRM_BASE_MSEND_ODDS01+idx] = pm->send.odds;
  }
}
static void param_BasePush( FES_ROM_WORK* wk )
{
  int i;
  
  wk->sys_save->beacon_interval = wk->param[PRM_BASE_BEACON_INTERVAL];
  wk->sys_save->start_time = wk->param[PRM_BASE_START_TIME];
  wk->sys_save->end_time = wk->param[PRM_BASE_END_TIME];
  wk->sys_save->tr_view = wk->param[PRM_BASE_TR_VIEW];
  wk->sys_save->mode = wk->param[PRM_BASE_MODE];

  for( i = 0;i < FESROM_MISSION_DATA_MAX; i++ ){
    int idx = PARAM_BASE_MSET_NUM*i;
    FESROM_MISSION* pm = &wk->sys_save->mission[i];
  
    pm->param.m_id = wk->param[PRM_BASE_MSEND_ID01+idx];
    pm->send.time = wk->param[PRM_BASE_MSEND_TIME01+idx];
    pm->send.blank = wk->param[PRM_BASE_MSEND_BLANK01+idx];
    pm->send.odds = wk->param[PRM_BASE_MSEND_ODDS01+idx];
  }
}

//========================================================
//ミッションパラメータのPushPop
static void param_MissionPop( FES_ROM_WORK* wk )
{
  int i;

  FESROM_MISSION* fmp = &wk->sys_save->mission[wk->edit_id];
  FESROM_MISSION_PARAM* prm = &fmp->param;

  if( fmp->send.id == 0 ){
    wk->param[PRM_M_ID] = 1;  //デフォルト
    wk->param[PRM_M_LEVEL] = FMISSION_LEVEL_1;  //デフォルト
    param_MissionUpdate( wk, M_UPDATE_ID );
    return;
  }

  wk->param[PRM_M_ID] = fmp->send.id;
  wk->param[PRM_M_TYPE_CODE] = prm->m_type_code;  //param_GetTypeCodeFromMID( wk, wk->param[PRM_M_ID] );

  wk->param[PRM_M_T_BONUS] = prm->m_t_bonus;
  wk->param[PRM_M_BASE] = prm->m_base_param;
  wk->param[PRM_M_EX] = prm->m_ex_param;
  wk->param[PRM_M_LEVEL] = prm->m_level;
//  wk->param[PRM_M_GOAL] = prm->m_goal;
  wk->param[PRM_M_TIME] = prm->m_time;
  wk->param[PRM_M_VERSION] = prm->m_version;
//  wk->param[PRM_M_UNIT] = prm->m_unit;

  wk->param[PRM_M_ITEM] = prm->m_item;
  wk->param[PRM_M_PAY] = prm->m_pay;
  wk->param[PRM_M_SP_ITEM] = prm->m_sp_item;
  wk->param[PRM_M_SP_PAY] = prm->m_sp_pay;
  
  wk->param[PRM_M_TITLE] = prm->m_title;
  wk->param[PRM_M_EXP] = prm->m_exp;
  
  for( i = 0;i < FMISSION_MAP_ATTR_MAX;i++){
    wk->param[PRM_M_PAT_TOWN+i] = prm->m_pattern[i];
  }
  param_MissionUpdate( wk, M_UPDATE_CUSTOM );
}

static void param_MissionPush( FES_ROM_WORK* wk )
{
  int i;

  FESROM_MISSION* fmp = &wk->sys_save->mission[wk->edit_id];
  FESROM_MISSION_PARAM* prm = &fmp->param;

  fmp->send.id = wk->param[PRM_M_ID];

  prm->m_t_bonus = wk->param[PRM_M_T_BONUS];

  prm->m_base_param = wk->param[PRM_M_BASE];
  prm->m_ex_param = wk->param[PRM_M_EX];
  prm->m_type_code = wk->param[PRM_M_TYPE_CODE];
  prm->m_level = wk->param[PRM_M_LEVEL];
//  prm->m_goal = wk->param[PRM_M_GOAL];
  prm->m_time = wk->param[PRM_M_TIME];
  prm->m_version = wk->param[PRM_M_VERSION];
//  prm->m_unit = wk->param[PRM_M_UNIT];

  prm->m_item = wk->param[PRM_M_ITEM];
  prm->m_pay = wk->param[PRM_M_PAY];
  prm->m_sp_item = wk->param[PRM_M_SP_ITEM];
  prm->m_sp_pay = wk->param[PRM_M_SP_PAY];
  
  prm->m_title = wk->param[PRM_M_TITLE];
  prm->m_exp = wk->param[PRM_M_EXP];
  
  for( i = 0;i < FMISSION_MAP_ATTR_MAX;i++){
    prm->m_pattern[i] = wk->param[PRM_M_PAT_TOWN+i];
  }
}

//ミッションIDからタイプコードを逆引き
static int param_GetTypeCodeFromMID( FES_ROM_WORK* wk, FEST_MISSION_ID m_id )
{
  int i;
  FMISSION_TYPE_ID m_type = wk->fm_arc_tbl[m_id].common.m_type;
  const MEDIT_PRM_BASE_SET* mpbs = &DATA_MissionBaseParam[ m_type-1 ];

  for( i = 0;i < mpbs->type_num; i++ ){
    if( mpbs->type_tbl[i] == m_id ) return i;
  }
  GF_ASSERT(0);
  return 0;
}

//ミッションデフォルトパラメータ再取得
static void param_MissionUpdate( FES_ROM_WORK* wk, M_UPDATE_CODE up_code )
{
  int i;

  FESROM_MISSION* fmp = &wk->sys_save->mission[wk->edit_id];
  FESROM_MISSION_PARAM* prm = &fmp->param;
  FEST_BEACON* beacon = &wk->e_fmwk.beacon;
 
  if( up_code == M_UPDATE_ID || up_code == M_UPDATE_CUSTOM ){
    FMISSION_DATA_GetMissionParam( wk->fm_data_man, wk->param[PRM_M_ID], wk->param[PRM_M_LEVEL], &wk->e_fmwk );
    if( up_code == M_UPDATE_ID ){
      wk->param[PRM_M_TYPE_CODE] = param_GetTypeCodeFromMID( wk, wk->param[PRM_M_ID] );
    }
  }
  wk->p_base_set = &DATA_MissionBaseParam[ wk->e_fmwk.arc.m_type-1 ];
  wk->sub_id = wk->p_base_set->type_tbl[wk->param[PRM_M_TYPE_CODE]];
  wk->p_fmarc = &wk->fm_arc_tbl[ wk->sub_id ];

  switch( up_code ){
  case M_UPDATE_BASE:
    {
      INPUT_CODE icode = meditEx_InputCodeGet( wk );
      wk->param[PRM_M_EX] = ( (icode==INPUT_CODE_ERR)?  0 : InPrmMin( icode ));
//      wk->param[PRM_M_UNIT] = wk->e_fmwk.beacon.unit;
    }
    return;
#if 0
  case M_UPDATE_LEVEL:
    wk->param[PRM_M_GOAL] = beacon->goal;
    wk->param[PRM_M_TIME] = beacon->time;
    wk->param[PRM_M_PAY] = beacon->pay;
    return;
#endif
  case M_UPDATE_CUSTOM:
  case M_UPDATE_TYPE_CODE:
    return;
  default:
    break;
  }
  wk->param[PRM_M_T_BONUS] = beacon->time_bonus;
  wk->param[PRM_M_BASE] = beacon->base_param;
  wk->param[PRM_M_EX] = beacon->ex_param;
  wk->param[PRM_M_LEVEL] = beacon->m_level;
//  wk->param[PRM_M_GOAL] = beacon->goal;
  wk->param[PRM_M_TIME] = beacon->time;
  wk->param[PRM_M_VERSION] = beacon->version_idx;
//  wk->param[PRM_M_UNIT] = beacon->unit;

  wk->param[PRM_M_ITEM] = beacon->item;
  wk->param[PRM_M_PAY] = beacon->pay;
  wk->param[PRM_M_SP_ITEM] = beacon->sp_item;
  wk->param[PRM_M_SP_PAY] = beacon->sp_pay;
  
  wk->param[PRM_M_TITLE] = beacon->m_title_id;
  wk->param[PRM_M_EXP] = beacon->m_exp_id;
  
  for( i = 0;i < FMISSION_MAP_ATTR_MAX;i++){
    wk->param[PRM_M_PAT_TOWN+i] = (beacon->pattern>>(i*3))&0x7;
  }
}

static int param_MissionPopToFMWK( FES_ROM_WORK* wk, int edit_id, FEST_MISSION* fmwk )
{
  int i;

  FESROM_MISSION* fmp = &wk->sys_save->mission[edit_id];
  FESROM_MISSION_PARAM* prm = &fmp->param;
  FEST_BEACON* beacon = &fmwk->beacon;
  FEST_MISSION_ID s_id;
  const MEDIT_PRM_BASE_SET* mpbs;
  FMISSION_ARCDATA* arc;

  if( fmp->send.id == 0 ){
    FEST_MISSION_Clear( fmwk );
    return 0;
  }
  //デフォルトパラメータ取得
  FMISSION_DATA_GetMissionParam( wk->fm_data_man, fmp->send.id, prm->m_level, fmwk );
  mpbs = &DATA_MissionBaseParam[ fmwk->arc.m_type-1 ];
  s_id = mpbs->type_tbl[prm->m_type_code];
  arc = &wk->fm_arc_tbl[ s_id ];

  fmwk->key = GFUser_GetPublicRand(GFL_STD_RAND_MAX);  //32bitランダムキー生成 
  beacon->distribution_f = TRUE;  //配信フラグOn

  //カスタムパラメータ適用
  beacon->m_id = s_id;  //送信する実際のミッションID
  beacon->m_level = prm->m_level;
  //単位と目標値はアーカイブから取得
  beacon->unit = arc->param.unit;
  beacon->goal = arc->param.level[prm->m_level].goal;

  if( beacon->end_type == FMISSION_ENDTYPE_SCORE ){
    beacon->time_bonus = prm->m_t_bonus;
    beacon->sp_item = 0;
    beacon->sp_pay = 0;
  }else{
    beacon->sp_item = prm->m_sp_item;
    beacon->sp_pay = prm->m_sp_pay;
  }
  beacon->base_param = prm->m_base_param;
  beacon->ex_param = prm->m_ex_param;
  beacon->time = prm->m_time;
  beacon->version_idx = prm->m_version;

  beacon->item = prm->m_item;
  beacon->pay = prm->m_pay;
  
  beacon->m_title_id = prm->m_title;
  beacon->m_exp_id = prm->m_exp;

  beacon->pattern = 0;
  for( i = 0;i < FMISSION_MAP_ATTR_MAX;i++){
    u32 tmp = prm->m_pattern[i]&0x07;
    beacon->pattern |= (tmp<<(i*3));
  }
  beacon->entry_flag_id = 0;  //エントリーフラグ無し

  return fmp->send.id;
}

static void param_MissionPopToFMWKEdit( FES_ROM_WORK* wk, FEST_MISSION* fmwk )
{
  int i;
  FEST_BEACON* beacon = &fmwk->beacon;

  *fmwk = wk->e_fmwk;

  //編集中パラメータセット
  beacon->m_title_id = wk->param[PRM_M_TITLE];
  beacon->m_exp_id = wk->param[PRM_M_EXP];
  beacon->goal = wk->p_fmarc->param.level[ wk->param[PRM_M_LEVEL]].goal;//wk->param[PRM_M_GOAL];
  beacon->base_param = wk->param[PRM_M_BASE];
  beacon->ex_param = wk->param[PRM_M_EXP];
  beacon->m_level = wk->param[PRM_M_LEVEL];
}

static int param_MissionEnableNum( FES_ROM_WORK* wk )
{
  int i,ct = 0;
  for( i = 0;i < FESROM_MISSION_DATA_MAX;i++){
    if( wk->sys_save->mission[i].send.id != 0 ) ct++;
  }
  return ct;
}

static INPUT_CODE meditEx_InputCodeGet( FES_ROM_WORK* wk )
{
  if( wk->p_base_set->tbl ) return wk->p_base_set->tbl[ wk->param[PRM_M_BASE]];
  return INPUT_CODE_ERR;
}

//========================================================
//送信プロセス初期化時のPushPop
static void param_SendPop( FES_ROM_WORK* wk )
{
  int i;

  //ビーコン基本情報セット
  GAMEBEACON_FesRom_SendBeacon_Init( wk->save_wk->str_name, FES_ROM_TRAINER_ID, wk->save_wk->str_selfmsg, wk->sys_save->tr_view );

  wk->edit_id = 0;
  wk->send_id = 0;
  wk->send_id_ct = 0;

  //配信確率の合計値を算出しておく
  wk->send_odds = 0;
  for( i = 0;i < FESROM_MISSION_DATA_MAX; i++ ){
    if( wk->sys_save->mission[i].send.id != 0 ){
      wk->send_odds += wk->sys_save->mission[i].send.odds; 
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//入力取得

/*
 *
 */
static int input_ParamNextPos( FES_ROM_WORK* wk, int diff )
{
  int i, n_line;

  n_line = wk->prmset_tbl[wk->pos].area.line;

  if( diff > 0 ){
    for( i = wk->pos;i < wk->prmset_num;i++){
      if( n_line < wk->prmset_tbl[i].area.line ){
        return i;
      }
    }
    return 0;
  }else{
    for( i = wk->pos;i >= 0;i--){
      if( n_line > wk->prmset_tbl[i].area.line ){
        return i;
      }
    }
    return wk->prmset_num-1;
  }
}

/*
 *  @brief  フェス配信ベース画面　入力取得
 */
int FesRom_InputCheckBase( FES_ROM_WORK* wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int repeat = GFL_UI_KEY_GetRepeat();
  int old_pos = wk->pos;
  const PARAM_SET* pset = &wk->prmset_tbl[wk->pos];

  if( pset->exe_code == EXE_CODE_INPUT_SWITCH ||
      pset->exe_code == EXE_CODE_INPUT_ODDS ){
    if( repeat & PAD_BUTTON_A ){
      return input_BaseExe( wk, &wk->prmset_tbl[wk->pos], 1 );
    }else if( repeat & PAD_BUTTON_B ){
      return input_BaseExe( wk, &wk->prmset_tbl[wk->pos], -1 );
    }
  }else if( trg & PAD_BUTTON_A ){
    return input_BaseExe( wk, &wk->prmset_tbl[wk->pos], 0 );
  }
  if( trg & PAD_KEY_RIGHT ){
    wk->pos = (wk->pos+1)%wk->prmset_num; 
    param_DrawAllEx( wk, wk->pos, old_pos );
  }else if( trg & PAD_KEY_LEFT ){
    wk->pos = (wk->pos+(wk->prmset_num-1))%wk->prmset_num; 
    param_DrawAllEx( wk, wk->pos, old_pos );
  }else if( trg & PAD_KEY_UP ){
    wk->pos = input_ParamNextPos( wk, -1 );
    param_DrawAllEx( wk, wk->pos, old_pos );
  }else if( trg & PAD_KEY_DOWN ){
    wk->pos = input_ParamNextPos( wk, 1 );
    param_DrawAllEx( wk, wk->pos, old_pos );
  }
  return SEQ_BASE_MAIN;
}

/*
 *  @brief フェス配信ベース 入力実行
 */
static int input_BaseExe( FES_ROM_WORK* wk, const PARAM_SET* pset, int param )
{
  int i;

  switch( pset->exe_code ){
  case EXE_CODE_INPUT_SWITCH: //スイッチ式入力実行
    wk->ret_seq = SEQ_BASE_MAIN;
    return exe_InputSwitch( wk, pset, param );

  case EXE_CODE_MSEND_ID:
    wk->ret_pos = wk->pos;
    wk->pos = 0;
    wk->edit_id = pset->arg[0];
    wk->app_mode = FESROM_APP_MODE_MEDIT;
    param_BasePush( wk );
    FES_ROM_SetFadeInOutSeq( wk, SEQ_MEDIT_MAIN, SEQ_INI_DRAW );
    return SEQ_FADEOUT;

  case EXE_CODE_INPUT_NUM:      //数値入力実行
  case EXE_CODE_INPUT_SELFMSG:  //フリーワード入力実行
  case EXE_CODE_INPUT_NAME:     //名前入力実行
    wk->sys_wk->ret_mode = ( (pset->exe_code==EXE_CODE_INPUT_NAME)? FESROM_APP_RET_EDIT_NAME:FESROM_APP_RET_EDIT_SELFMSG );
    wk->sys_wk->init_pos = wk->pos;
    param_BasePush( wk );
    FES_ROM_SetFadeInOutSeq( wk, SEQ_BASE_MAIN, SEQ_EXIT );
    return SEQ_FADEOUT;

  case EXE_CODE_SAVE:         //セーブ実行
    taskReq_Save( wk );
    return SEQ_TASK_WAIT; 
  
  case EXE_CODE_SEND_START:     //配信開始
    //有効なミッションがあるかチェック
    if( param_MissionEnableNum( wk) == 0){
      return SEQ_BASE_MAIN;
    }
    param_BasePush( wk );
    wk->ret_pos = wk->pos;
    wk->pos = 0xFF;
    wk->edit_id = 0;
    wk->app_mode = FESROM_APP_MODE_SEND;
    FES_ROM_SetFadeInOutSeq( wk, SEQ_SEND_MAIN, SEQ_INI_DRAW );
    return SEQ_FADEOUT;
  
  case EXE_CODE_INPUT_ODDS:
    wk->ret_seq = SEQ_BASE_MAIN;
    return exe_InputSwitchEx( wk, pset, ((wk->param[PRM_BASE_MODE]==SEND_MODE_TURN)? INPUT_CODE_ERR:pset->input_code), param );
  }
  return wk->ret_seq; 
}

/*
 *  @brief  フェス配信 ミッション編集画面　入力取得
 */
int FesRom_InputCheckMEdit( FES_ROM_WORK* wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  int cont = GFL_UI_KEY_GetCont();
  int repeat = GFL_UI_KEY_GetRepeat();
  int old_pos = wk->pos;
  const PARAM_SET* pset = &wk->prmset_tbl[wk->pos];

  if( pset->exe_code == EXE_CODE_INPUT_SWITCH ||
      pset->exe_code == EXE_CODE_MEDIT_BASE ||
      pset->exe_code == EXE_CODE_MEDIT_EX ||
      pset->exe_code == EXE_CODE_MEDIT_TIME ||
      pset->exe_code == EXE_CODE_MEDIT_SP_ITEM ||
      pset->exe_code == EXE_CODE_MEDIT_SP_PAY ||
      pset->exe_code == EXE_CODE_MEDIT_T_BONUS ||
      pset->exe_code == EXE_CODE_MEDIT_TYPE_CODE ){
    int diff = 1;

    if( pset->exe_code == EXE_CODE_MEDIT_TIME ){
      diff = 30;
    }else{
      if( cont & PAD_BUTTON_R ) diff*=10;
      if( cont & PAD_BUTTON_L ) diff*=10;
    }
    if( repeat & PAD_BUTTON_A ){
      return input_MEditExe( wk, &wk->prmset_tbl[wk->pos], diff );
    }else if( repeat & PAD_BUTTON_B ){
      return input_MEditExe( wk, &wk->prmset_tbl[wk->pos], diff*-1 );
    }
  }else if( trg & PAD_BUTTON_A ){
    return input_MEditExe( wk, &wk->prmset_tbl[wk->pos], 0 );
  }else if( trg & PAD_BUTTON_SELECT ){
    wk->pos = wk->ret_pos;
    FES_ROM_SetFadeInOutSeq( wk, SEQ_BASE_MAIN, SEQ_INI_DRAW );
    return SEQ_FADEOUT;
  }
  if( trg & PAD_KEY_RIGHT ){
    wk->pos = (wk->pos+1)%wk->prmset_num; 
    param_DrawAllEx( wk, wk->pos, old_pos );
  }else if( trg & PAD_KEY_LEFT ){
    wk->pos = (wk->pos+(wk->prmset_num-1))%wk->prmset_num; 
    param_DrawAllEx( wk, wk->pos, old_pos );
  }else if( trg & PAD_KEY_UP ){
    wk->pos = input_ParamNextPos( wk, -1 );
    param_DrawAllEx( wk, wk->pos, old_pos );
  }else if( trg & PAD_KEY_DOWN ){
    wk->pos = input_ParamNextPos( wk, 1 );
    param_DrawAllEx( wk, wk->pos, old_pos );
  }
  return SEQ_MEDIT_MAIN;
}

static BOOL sub_ItemCheck( FES_ROM_WORK* wk )
{
  int value = wk->param[PRM_M_ITEM];
  if( ITEM_IsDisableItem( value ) ||
      ITEM_IsOnlyOneItem( value ) ||
      value == ITEM_MASUTAABOORU){
      return FALSE;
  }
  if( wk->e_fmwk.beacon.end_type == FMISSION_ENDTYPE_SCORE ){
    return TRUE;
  }
  value = wk->param[PRM_M_SP_ITEM];
  if( value == 0 ) return TRUE; //SPアイテム無しはOK

  if( ITEM_IsDisableItem( value ) ||
      ITEM_IsOnlyOneItem( value ) ){
      return FALSE;
  }
  return TRUE;
}

/*
 *  @brief フェス配信ミッション編集 入力実行
 */
static int input_MEditExe( FES_ROM_WORK* wk, const PARAM_SET* pset, int param )
{
  wk->ret_seq = SEQ_MEDIT_MAIN;

  switch( pset->exe_code ){
  case EXE_CODE_MEDIT_DECIDE:
    if( sub_ItemCheck( wk ) == FALSE ){
      sub_PlaySE( FESROM_SE_CANCEL );
      return wk->ret_seq;  //アイテムが不正なので決定できない
    }
    param_MissionPush( wk );
    //break throw
  case EXE_CODE_MEDIT_CANCEL:
    wk->pos = wk->ret_pos;
    wk->app_mode = FESROM_APP_MODE_BASE;
    FES_ROM_SetFadeInOutSeq( wk, SEQ_BASE_MAIN, SEQ_INI_DRAW );
    return SEQ_FADEOUT;
  
  case EXE_CODE_MEDIT_DELETE:
    wk->pos = wk->ret_pos;
    wk->app_mode = FESROM_APP_MODE_BASE;
    FesRomSys_MissionSaveInit( wk->sys_save, wk->edit_id );
    FES_ROM_SetFadeInOutSeq( wk, SEQ_BASE_MAIN, SEQ_INI_DRAW );
    return SEQ_FADEOUT;

  case EXE_CODE_MEDIT_BASE:
    return exe_InputSwitchEx( wk, pset, wk->p_base_set->input_code, param );
  
  case EXE_CODE_MEDIT_EX:
    return exe_InputSwitchEx( wk, pset, meditEx_InputCodeGet(wk), param );
  
  case EXE_CODE_MEDIT_SP_PAY:
    {
      int icode = INPUT_CODE_ERR;
      if( wk->e_fmwk.beacon.end_type == FMISSION_ENDTYPE_TIME ){
        icode = INPUT_CODE_MEDIT_SP_PAY;  //wk->p_base_set->input_code;
      }
      return exe_InputSwitchEx( wk, pset, icode, param );
    }
  
  case EXE_CODE_MEDIT_T_BONUS:
    {
      int icode = INPUT_CODE_ERR;
      if( wk->e_fmwk.beacon.end_type == FMISSION_ENDTYPE_SCORE ){
        icode = INPUT_CODE_MEDIT_T_BONUS; //meditEx_InputCodeGet( wk );
      }
      return exe_InputSwitchEx( wk, pset, icode, param );
    }
  case EXE_CODE_MEDIT_TYPE_CODE:
    return exe_InputSwitchEx( wk, pset, pset->input_code+wk->e_fmwk.arc.m_type-1, param );

  case EXE_CODE_INPUT_SWITCH: //スイッチ式入力実行
  case EXE_CODE_MEDIT_SP_ITEM:
  case EXE_CODE_MEDIT_TIME:
    return exe_InputSwitch( wk, pset, param );

  case EXE_CODE_SAVE:         //セーブ実行
    taskReq_Save( wk );
    return SEQ_TASK_WAIT; 
  }
  return wk->ret_seq; 
}

/*
 *  @brief  スイッチ式入力実行
 */
static int exe_InputSwitch( FES_ROM_WORK* wk, const PARAM_SET* pset, int diff )
{
  return exe_InputSwitchEx( wk, pset, pset->input_code, diff );
}
/*
 *  @brief  スイッチ式入力実行
 */
static int exe_InputSwitchEx( FES_ROM_WORK* wk, const PARAM_SET* pset, INPUT_CODE input_code, int diff )
{
  int i, value, all = FALSE;
  const INPUT_PARAM* iprm;
  
  if( input_code == INPUT_CODE_ERR ){
    sub_PlaySE( FESROM_SE_CANCEL );
    return wk->ret_seq;
  }
  iprm = &DATA_InputParamTbl[input_code]; 
  value = wk->param[ pset->id ];
   
  sub_PlaySE( FESROM_SE_DECIDE );

  if( diff > 0 && ( value+diff > iprm->max ) ){
    value = iprm->min;
  }else if( diff < 0 && (value+diff < iprm->min) ){
    value = iprm->max;
  }else{
    value += diff;
  }
  wk->param[pset->id] = value;

  switch( pset->id ){
  case PRM_BASE_MODE:
    for( i = 0; i < FESROM_MISSION_DATA_MAX; i++ ){
      param_Draw( wk, PRM_BASE_MSEND_ODDS01+(i*PARAM_BASE_MSET_NUM) );
    }
    break;
  case PRM_M_ID:
    param_MissionUpdate( wk, M_UPDATE_ID );
    all = TRUE;
    break; 
  case PRM_M_BASE:
    param_MissionUpdate( wk, M_UPDATE_BASE );
    all = TRUE;
    break;
  case PRM_M_TYPE_CODE:
    param_MissionUpdate( wk, M_UPDATE_TYPE_CODE );
    param_Draw( wk, PRM_M_LEVEL-PRM_MEDIT_ID_START );
    break;
#if 0
  case PRM_M_LEVEL:
    param_MissionUpdate( wk, M_UPDATE_LEVEL );
    all = TRUE;
    break;
#endif
  case PRM_M_LEVEL:
  case PRM_M_ITEM:
  case PRM_M_SP_ITEM:
    param_Draw( wk, 1 );
    break;
  }
  if( all ){
    param_DrawAll( wk, POS_TARGET_NONE );
  }else{
    param_Draw( wk, wk->pos );
  }
  param_Trans( wk );
  return wk->ret_seq;
}

///////////////////////////////////////////////////////////////////////////////
//配信メイン

static int sendSeq_CheckTime( FES_ROM_WORK* wk );
static int sendSeq_Overtime( FES_ROM_WORK* wk );
static int sendSeq_CommInitWait( FES_ROM_WORK* wk );
static int sendSeq_Update( FES_ROM_WORK* wk );
static int sendSeq_CommEndWait( FES_ROM_WORK* wk );
static int sendSeq_Interval( FES_ROM_WORK* wk );
static int sendSeq_End( FES_ROM_WORK* wk );
static BOOL subSend_CheckTime( FES_ROM_WORK* wk );

/*
 *  @brief  フェス配信 配信画面メイン　入力取得
 */
int FesRom_InputCheckSend( FES_ROM_WORK* wk )
{
  s64 now_rtc = GFL_RTC_GetDateTimeBySecond();

  if( now_rtc != wk->old_date_time ){
    param_DrawTrans( wk, PRMIDX_SEND_NOW_RTC );
    wk->old_date_time = now_rtc;
  }

  switch( wk->seq ){
  case SEQ_SEND_CHECK_TIME:
    wk->seq = sendSeq_CheckTime( wk );
    break;
  case SEQ_SEND_OVERTIME:
    wk->seq = sendSeq_Overtime( wk );
    break;
  case SEQ_SEND_COMM_INIT_WAIT:
    wk->seq = sendSeq_CommInitWait( wk );
    break;
  case SEQ_SEND_UPDATE:
    wk->seq = sendSeq_Update( wk );
    break;
  case SEQ_SEND_COMM_END_WAIT:
    wk->seq = sendSeq_CommEndWait( wk );
    break;
  case SEQ_SEND_INTERVAL:
    wk->seq = sendSeq_Interval( wk );
    break;
  case SEQ_SEND_END:
    wk->seq = sendSeq_End( wk );
    break;
  }
  return SEQ_SEND_MAIN;
}

//==================================================
//配信時間チェック
static int sendSeq_CheckTime( FES_ROM_WORK* wk )
{
  int i;
  FESROM_MISSION* m_tbl = wk->sys_save->mission;

  if( subSend_CheckTime( wk ) == FALSE ){
    param_DrawTrans( wk, PRMIDX_SEND_OVERTIME );
    return SEQ_SEND_OVERTIME;
  }
  //配信ミッション抽選
  if( wk->param[PRM_BASE_MODE] == SEND_MODE_TURN ){
    //ターン制
    for( i = 0;i < FESROM_MISSION_DATA_MAX;i++){
      int idx = (i+wk->send_id_ct)%FESROM_MISSION_DATA_MAX;
      if( m_tbl[idx].send.id == 0 ) continue;

      wk->send_id = idx;
      wk->send_id_ct = (idx+1)%FESROM_MISSION_DATA_MAX;
      OS_Printf("turn id=%d, ct=%d\n", idx, wk->send_id_ct );
      break;
    }
  }else{
    //ランダム
    int rnd = GFUser_GetPublicRand( wk->send_odds - 1 );
    for( i = 0;i < FESROM_MISSION_DATA_MAX;i++){
      if( m_tbl[i].send.id == 0 ) continue;

      OS_Printf("random id=%d, odds=%d, rnd=%d\n", i, m_tbl[i].send.odds,rnd );
      if( rnd < m_tbl[i].send.odds ){
        wk->send_id = i;
        break;
      }
      rnd -= m_tbl[i].send.odds;
    }
  }
  wk->send_m = &wk->sys_save->mission[wk->send_id];
  param_MissionPopToFMWK( wk, wk->send_id, &wk->send_fmwk );

  //ミッション送信時間補正
  {
    int sec = wk->send_m->send.time*60; //min2sec
    int m_time =  wk->send_fmwk.beacon.time;  //ミッション制限時間
    int limit = m_time - LINKFEST_LIFE_LIMIT;  //ミッションの有効残り時間が上限
  
    if( limit < sec ){
      sec = limit;  //送信時間補正
    }
    wk->send_time_sec = sec;  //送信を続ける時間
    wk->send_lest_sec = sec;
  }  
  wk->send_blank_sec = wk->send_m->send.blank*60; //min2sec
  wk->send_diff_sec = 0;

  OS_Printf("send_mission id=%d send_time=%dsec blank=%d\n", wk->send_id, wk->send_time_sec, wk->send_blank_sec );

  param_Draw( wk, PRMIDX_SEND_EXEC );
  param_Draw( wk, PRMIDX_SEND_M_TITLE );
  param_DrawTrans( wk, PRMIDX_SEND_M_EXP );

  //通信起動
  HaisinBeacon_Init( &wk->send_fmwk, wk->param[PRM_BASE_BEACON_INTERVAL] );

  return SEQ_SEND_COMM_INIT_WAIT;
}
//時間外チェック
static int sendSeq_Overtime( FES_ROM_WORK* wk )
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
    return SEQ_SEND_END;
  }
  if( subSend_CheckTime( wk ) == FALSE ){
    return SEQ_SEND_OVERTIME;
  }
  param_SendPop( wk );  //送信パラメータ初期化
  param_Clear( wk, PRMIDX_SEND_OVERTIME );
  return SEQ_SEND_CHECK_TIME;
}

//通信起動
static int sendSeq_CommInitWait( FES_ROM_WORK* wk )
{
  if( HaisinBeacon_InitWait() ){
    //ミッション時間カウント開始
    FEST_MISSION_PlayStart( &wk->send_fmwk );
    return SEQ_SEND_UPDATE;
  }
  return SEQ_SEND_COMM_INIT_WAIT;
}

//通信起動中メイン
static int sendSeq_Update( FES_ROM_WORK* wk )
{
  s64 now_rtc = GFL_RTC_GetDateTimeBySecond();
  int diff = (int)(now_rtc-wk->send_fmwk.start_rtc); //送信開始からの経過時間

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
    wk->send_exit_f = TRUE;
    HaisinBeacon_Exit();
    return SEQ_SEND_COMM_END_WAIT;
  }

  //ビーコン送信時間チェック
  if( diff >= wk->send_time_sec){
    //通信終了
    HaisinBeacon_Exit();
    return SEQ_SEND_COMM_END_WAIT;
  }
  if( wk->send_diff_sec != diff ){
    wk->send_lest_sec = wk->send_time_sec-diff;
    if( wk->send_lest_sec < 0 ) wk->send_lest_sec = 0;
   
    param_DrawTrans( wk, PRMIDX_SEND_EXEC );
    
    wk->send_diff_sec = diff;
  }
  HaisinBeacon_Update( &wk->send_fmwk );
  return SEQ_SEND_UPDATE;
}

//通信終了待ち
static int sendSeq_CommEndWait( FES_ROM_WORK* wk )
{
  if( HaisinBeacon_ExitWait() == FALSE){
    return SEQ_SEND_COMM_END_WAIT;
  }
  if( wk->send_exit_f ) return SEQ_SEND_END;

  wk->send_rtc = GFL_RTC_GetDateTimeBySecond(); //今の時間を覚えておく
  wk->send_diff_sec = 0;
  wk->send_lest_sec = wk->send_blank_sec;
  param_Draw( wk, PRMIDX_SEND_INTERVAL );
  param_Clear( wk, PRMIDX_SEND_M_TITLE );
  param_Clear( wk, PRMIDX_SEND_M_EXP );
  return SEQ_SEND_INTERVAL;
}

//インターバル待ち
static int sendSeq_Interval( FES_ROM_WORK* wk )
{
  s64 now_rtc = GFL_RTC_GetDateTimeBySecond();
  int diff = (int)(now_rtc-wk->send_rtc); //インターバル開始からの経過時間
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
    return SEQ_SEND_END;
  }
  
  //インターバル経過時間チェック
  if( diff >= wk->send_blank_sec ){
    wk->send_lest_sec = 0;
    param_DrawTrans( wk, PRMIDX_SEND_INTERVAL );
    return SEQ_SEND_CHECK_TIME;
  }
  if( wk->send_diff_sec != diff ){
    wk->send_lest_sec = wk->send_blank_sec-diff;
    if( wk->send_lest_sec < 0 ) wk->send_lest_sec = 0;
   
    param_DrawTrans( wk, PRMIDX_SEND_INTERVAL );
    wk->send_diff_sec = diff;
  }
  return SEQ_SEND_INTERVAL;

}
//電源Off待ち
static int sendSeq_End( FES_ROM_WORK* wk )
{
  switch( wk->sub_seq ){
  case 0:
    param_Draw( wk, PRMIDX_SEND_END );
    param_Clear( wk, PRMIDX_SEND_M_TITLE );
    param_Clear( wk, PRMIDX_SEND_M_EXP );
    wk->sub_seq++;
    break;
  default:
    break;
  }
  return SEQ_SEND_END;
}

//配信時間帯チェック
static BOOL subSend_CheckTime( FES_ROM_WORK* wk )
{
  RTCTime time;
  int now_min, s_min, e_min;
  GFL_RTC_GetTime( &time );
  
  now_min = time.hour*FES_ROM_TIME_UNIT_HOUR+(time.minute/FES_ROM_TIME_UNIT_MIN); //現在時刻を分計算
#if 0
  if( time.minute != 0 && time.minute != 30 ){  //00分,30分ちょうどでない時は+1
    now_min+=1;
  }
#endif
  s_min = wk->param[PRM_BASE_START_TIME];
  e_min = wk->param[PRM_BASE_END_TIME];

  OS_Printf("n=%d, s=%d, e=%d\n",now_min,s_min,e_min);
  if( s_min == e_min ) return TRUE; //24時間配信

  if( s_min < e_min ){  //05:00-20:00
    if( now_min >= s_min && now_min < e_min ){
      return TRUE;
    }
  }else{  //例20:00-05:00
    if( now_min >= s_min || now_min < e_min ){
      return TRUE;
    } 
  }
  return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//ターゲットウィンドウ
static void targetWin_Clear( TARGET_WIN* twin )
{
  GFL_BMP_Clear( twin->base.bmp, 0 );
  GFL_BMP_Clear( twin->input.bmp, 0 );
  GFL_BMP_Clear( twin->msg.bmp, 0 );
	GFL_BMPWIN_TransVramCharacter( twin->base.win );
	GFL_BMPWIN_TransVramCharacter( twin->input.win );
	GFL_BMPWIN_TransVramCharacter( twin->msg.win );
}

static void targetWin_ClearAll( FES_ROM_WORK* wk )
{
  int i;

  for( i = 0;i < TARGET_VIEW_MAX; i++ ){
    targetWin_Clear( &wk->target[i] );
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//パラメータ描画
/*
 *  @brief 個別パラメータ描画 
 */
static void param_Draw( FES_ROM_WORK* wk, int idx )
{
  int ret_wset = TRUE;
  PRINTSYS_LSB col = FCOL_WHITE_N;
  
  const PARAM_SET* pset = &wk->prmset_tbl[idx];
  TARGET_WIN* twin = &wk->target[pset->area.line/LINE_VIEW_NUM];
  s16 sx = pset->area.sx*LINE_W;
  s16 sy = (pset->area.line%LINE_VIEW_NUM)*LINE_H;

  GFL_BMP_Fill( twin->base.bmp, sx, sy, (pset->area.ex-pset->area.sx)*LINE_W, pset->area.sy*LINE_H, FCOL_WIN_CLEAR );

  switch( pset->id ){
  case PRM_M_BASE:
    {
      WORDSET_RegisterNumber( wk->wset, 0, wk->param[pset->id], 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      ret_wset = print_ParamWordsetEx( wk, 1, wk->param[pset->id], wk->p_base_set->wset_type, idx, -1 );
    }
    break;
  case PRM_M_EX:
    {
      INPUT_CODE icode = meditEx_InputCodeGet( wk );
      if( icode == INPUT_CODE_ERR ){
        WORDSET_RegisterClear( wk->wset, 0 );
        WORDSET_RegisterClear( wk->wset, 1 );
        ret_wset = FALSE;
      }else{
        WORDSET_RegisterNumber( wk->wset, 0, wk->param[pset->id], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
        ret_wset = print_ParamWordsetICodeEx( wk, 1, wk->param[pset->id], icode, idx );
      }
    }
    break;
  case PRM_BASE_SEND_START:
    ret_wset = param_MissionEnableNum( wk );
    break; 
  case PRM_M_SP_ITEM:
  case PRM_M_SP_PAY:
    ret_wset = print_ParamWordsetEx( wk, 0, wk->param[pset->id], pset->wset_type, idx, InPrmKeta( pset->input_code ) );
    if( wk->e_fmwk.beacon.end_type == FMISSION_ENDTYPE_SCORE ) ret_wset = FALSE;
    break;
  case PRM_M_T_BONUS:
    ret_wset = print_ParamWordsetEx( wk, 0, wk->param[pset->id], pset->wset_type, idx, InPrmKeta( pset->input_code ) );
    if( wk->e_fmwk.beacon.end_type == FMISSION_ENDTYPE_TIME ) ret_wset = FALSE;
    break;
  case PRM_M_DECIDE:
    ret_wset = sub_ItemCheck( wk );
    break;
  default:
    ret_wset = print_ParamWordsetEx( wk, 0, wk->param[pset->id], pset->wset_type, idx, InPrmKeta( pset->input_code ) );
    break;
  }
  print_GetMsgToBuf( wk, pset->msg );

  if( ret_wset ){
    if( wk->pos==idx ) col = FCOL_BLUE_N;
  }else{
    if( wk->pos==idx ){
      col = FCOL_PINK_N;
    }else if( (pset->id == PRM_M_ITEM) ||
              (pset->id == PRM_M_SP_ITEM && wk->e_fmwk.beacon.end_type == FMISSION_ENDTYPE_TIME ) ){
      col = FCOL_RED_N;
    }else{
      col = FCOL_GRAY_N;
    }
  }
	PRINTSYS_PrintColor( twin->base.bmp, sx, sy, wk->str_expand, wk->font, col );
}

/*
 *  @brief パラメータ描画 一括
 */
static void param_DrawAll(  FES_ROM_WORK* wk, int target_idx )
{
  param_DrawAllEx( wk, target_idx, POS_TARGET_NONE );
}
static void param_DrawAllEx( FES_ROM_WORK* wk, int target_idx1, int target_idx2 )
{
  int i;
  
  for( i = 0;i < wk->prmset_num;i++){
    if( target_idx1 == POS_TARGET_NONE ||
        i == target_idx1 ||
        i == target_idx2 ){
      param_Draw( wk, i );
    }
  }
  param_Trans( wk );
}
/*
 *  @brief パラメータ描画転送
 */
static void param_Trans(  FES_ROM_WORK* wk )
{
	GFL_BMPWIN_TransVramCharacter( wk->target[0].base.win );
	GFL_BMPWIN_TransVramCharacter( wk->target[1].base.win );
}
static void param_DrawTrans( FES_ROM_WORK* wk, int idx )
{
  param_Draw( wk, idx );
  param_Trans( wk );
}
static void param_Clear( FES_ROM_WORK* wk, int idx )
{
  const PARAM_SET* pset = &wk->prmset_tbl[idx];
  TARGET_WIN* twin = &wk->target[pset->area.line/LINE_VIEW_NUM];
  s16 sx = pset->area.sx*LINE_W;
  s16 sy = (pset->area.line%LINE_VIEW_NUM)*LINE_H;

  GFL_BMP_Fill( twin->base.bmp, sx, sy, (pset->area.ex-pset->area.sx)*LINE_W, pset->area.sy*LINE_H, FCOL_WIN_CLEAR );
  param_Trans( wk );
}



/////////////////////////////////////////////////////////////////////////////////
//サブルーチン

/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
static void print_GetMsgToBuf( FES_ROM_WORK* wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->msgman, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wset, wk->str_expand, wk->str_tmp );
}

//----------------------------------------------------------
/*
 *  @brief  フリーワード展開
 */
//----------------------------------------------------------
static inline void print_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str )
{
  WORDSET_RegisterWord( wset, idx, str, PM_NEUTRAL, TRUE, PM_LANG );
}

//----------------------------------------------------------
/*
 *  @brief  指定パラメータワード展開
 */
//----------------------------------------------------------
static BOOL print_ParamWordset( FES_ROM_WORK* wk, u8 idx, int value, WSET_TYPE type )
{
  return print_ParamWordsetEx( wk, idx, value, type, 0, -1 );
}
static BOOL print_ParamWordsetICode( FES_ROM_WORK* wk, u8 idx, int value, INPUT_CODE input_code )
{
  return print_ParamWordsetEx( wk, idx, value, InPrmWSet( input_code ), 0, InPrmKeta( input_code ) );
}
static BOOL print_ParamWordsetICodeEx( FES_ROM_WORK* wk, u8 idx, int value, INPUT_CODE input_code, int arg )
{
  return print_ParamWordsetEx( wk, idx, value, InPrmWSet( input_code ), arg, InPrmKeta( input_code ) );
}
static BOOL print_ParamWordsetEx( FES_ROM_WORK* wk, u8 idx, int value, WSET_TYPE type, int arg, int keta )
{
  switch( type ){
  case WSET_BEACON_INTERVAL:
    WORDSET_RegisterNumber( wk->wset, idx, value, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    return TRUE;
  case WSET_TIME:
    {
      int hour = value/FES_ROM_TIME_UNIT_HOUR;
      int min = (value%FES_ROM_TIME_UNIT_HOUR)*FES_ROM_TIME_UNIT_MIN;
      WORDSET_RegisterNumber( wk->wset, idx, hour, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( wk->wset, idx+1, min, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    }
    return TRUE;
  case WSET_MODE:
    GFL_MSG_GetString( wk->msgman, f_rom_param_mode_01+value, wk->str_tmp );
    break;
  case WSET_TR_NAME:
    if( value == 0){
      GFL_STR_CopyBuffer( wk->str_tmp, wk->save_wk->str_name );
    }else{
      WORDSET_RegisterTrainerName( wk->wset, idx, value );
      return TRUE;
    }
    break;
  case WSET_SELFMSG:
    GFL_STR_CopyBuffer( wk->str_tmp, wk->save_wk->str_selfmsg );
    break;
  case WSET_MSEND_ID:
    {
      const PARAM_SET* pset = &wk->prmset_tbl[arg];
      int m_id = param_MissionPopToFMWK( wk, pset->arg[0], &wk->tmp_fmwk );
      
      WORDSET_RegisterNumber( wk->wset, idx, m_id, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( wk->wset, idx+1, pset->arg[0]+1, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      if( m_id == 0 ){
        GFL_MSG_GetString( wk->msgman, f_rom_param_msend_id_none, wk->str_tmp );
        print_FreeWordset( wk->wset, idx+2, wk->str_tmp );
      }else{
        FMISSION_DATA_GetTitleStr( wk->fm_data_man, wk->str_tmp, &wk->tmp_fmwk, wk->heapID );
        print_FreeWordset( wk->wset, idx+2, wk->str_tmp );
      }
    }
    return TRUE;
  case WSET_MSEND_TIME:
    WORDSET_RegisterNumber( wk->wset, idx, value, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    return TRUE;
  case WSET_MSEND_BLANK:
    WORDSET_RegisterNumber( wk->wset, idx, value, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    return TRUE;
  case WSET_MSEND_ODDS:
    if( wk->param[PRM_BASE_MODE] == SEND_MODE_RANDOM){  //random
      WORDSET_RegisterNumber( wk->wset, idx, value, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      GFL_MSG_GetString( wk->msgman, f_rom_param_odds, wk->str_tmp );
      print_FreeWordset( wk->wset, idx+1, wk->str_tmp );
      return TRUE;
    }else{
      const PARAM_SET* pset = &wk->prmset_tbl[arg];
      WORDSET_RegisterNumber( wk->wset, idx, pset->arg[0]+1, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterClear( wk->wset, idx+1 );
      return FALSE;
    }


  case WSET_MONSNAME:
    WORDSET_RegisterPokeMonsNameNo( wk->wset, idx, value );
    return TRUE;
  case WSET_ITEMNAME:
    WORDSET_RegisterItemName( wk->wset, idx, value );
    return TRUE;
  case WSET_ITEMNAME_CHECK:
    WORDSET_RegisterNumber( wk->wset, idx, value, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterItemName( wk->wset, idx+1, value );
    if( ITEM_IsDisableItem( value ) ||
        ITEM_IsOnlyOneItem( value ) ||
        value == ITEM_MASUTAABOORU){
      return FALSE;
    }
    return TRUE;
  case WSET_ITEMNAME_CHECK_SP:
    WORDSET_RegisterNumber( wk->wset, idx, value, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterItemName( wk->wset, idx+1, value );
    if( value == 0 ) return TRUE;
    if( ITEM_IsDisableItem( value ) ||
        ITEM_IsOnlyOneItem( value ) ){
      return FALSE;
    }
    return TRUE;
  case WSET_FREEWORD:
    GFL_MSG_GetString( wk->msgman, f_rom_freeword_icon_01+value, wk->str_tmp );
    break;
  case WSET_TR_VIEW:
    GFL_MSG_GetString( wk->msgman, f_rom_tr_view_01+value, wk->str_tmp );
    break;
  case WSET_MBASE_EVENT:
    GFL_MSG_GetString( wk->msgman, f_rom_mission_base_event_01+value, wk->str_tmp );
    break;
  case WSET_MBASE_ITEM:
    GFL_MSG_GetString( wk->msgman, f_rom_mission_base_item_01+value, wk->str_tmp );
    break;
  case WSET_MBASE_NPC:
    GFL_MSG_GetString( wk->msgman, f_rom_mission_base_npc_01+value, wk->str_tmp );
    break;
  case WSET_MEX_EVENT_TRAIL:
    WORDSET_RegisterPokeMonsNameNo( wk->wset, idx, DATA_TrailMonsTbl[value] );
    return TRUE;
  case WSET_MEX_NPC_ITEM_TRADE:
    GFL_MSG_GetString( wk->msgman, f_rom_npc_item_trade_01+value, wk->str_tmp );
    break;
  case WSET_MEX_NPC_SHOPPING:
    GFL_MSG_GetString( wk->msgman, f_rom_npc_shopping_01+value, wk->str_tmp );
    break;
  case WSET_MEX_NPC_TR_BATTLE:
    GFL_MSG_GetString( wk->msgman, f_rom_npc_tr_battle_01+value, wk->str_tmp );
    break;
  case WSET_MEX_NPC_GET_A_PRESENT:
    GFL_MSG_GetString( wk->msgman, f_rom_npc_get_a_present_01+value, wk->str_tmp );
    break;
  case WSET_MEX_NPC_POKE_QUIZ:
    GFL_MSG_GetString( wk->msgman, f_rom_npc_poke_quiz_01+value, wk->str_tmp );
    OS_Printf("quiz_value=%d\n", value);
    break;
  case WSET_MISSION_ID_NAME:
    GFL_MSG_GetString( wk->msgman, f_rom_mission_id_name_01+value, wk->str_tmp );
    break;
  case WSET_MISSION_MSGID_NAME:
    GFL_MSG_GetString( wk->msgman, f_rom_mission_msg_id_01+value, wk->str_tmp );
    break;

  case WSET_MEDIT_ID:
    //DataNo展開
    WORDSET_RegisterNumber( wk->wset, idx+1, wk->edit_id+1, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //ミッションNo展開
    WORDSET_RegisterNumber( wk->wset, idx, value, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
    //デフォルトタイトル展開
    WORDSET_RegisterFMissionTitle( wk->wset, idx+2, value, FMISSION_LEVEL_1);
    //ミッションタイプ展開
    GFL_MSG_GetString( wk->msgman, f_rom_mission_type_01+wk->e_fmwk.arc.m_type-1, wk->str_tmp );
    print_FreeWordset( wk->wset, idx+3, wk->str_tmp );
    //ミッション終了タイプ展開
    GFL_MSG_GetString( wk->msgman, f_rom_mission_end_type_01+wk->e_fmwk.beacon.end_type, wk->str_tmp );
    print_FreeWordset( wk->wset, idx+4, wk->str_tmp );
    return TRUE;
  
  case WSET_MEDIT_TYPE_CODE:
    {
      int i;
      int id = DATA_MissionBaseParam[wk->e_fmwk.arc.m_type-1].type_tbl[value];

      //タイプコード展開
      WORDSET_RegisterNumber( wk->wset, idx, value, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
      //サブID展開
      WORDSET_RegisterNumber( wk->wset, idx+1, id, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
      //単位展開
      GFL_MSG_GetString( wk->msgman, f_rom_fes_unit_01+wk->fm_arc_tbl[id].param.unit, wk->str_tmp );
      print_FreeWordset( wk->wset, idx+2, wk->str_tmp );

      for( i = 0;i < 4; i++ ){
        WORDSET_RegisterNumber( wk->wset, idx+3+i, wk->fm_arc_tbl[id].param.level[i].goal, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      }
    }
    return TRUE;
  
  case WSET_MEDIT_LEVEL:
    WORDSET_RegisterNumber( wk->wset, idx, value+1, 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( wk->wset, idx+1, wk->p_fmarc->param.level[value].goal, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
    return TRUE;
  
  case WSET_MEDIT_TIME:
    WORDSET_RegisterNumber( wk->wset, idx, value, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    return TRUE;
  
  case WSET_MEDIT_VERSION:
    GFL_MSG_GetString( wk->msgman, f_rom_version_white+value, wk->str_tmp );
    break;
  
  case WSET_MEDIT_T_BONUS:
    {
      static const u8 bonus[] = {0,30,60,180};
      WORDSET_RegisterNumber( wk->wset, idx, bonus[value], 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
    }
    return TRUE;
  
  case WSET_MEDIT_UNIT:
    GFL_MSG_GetString( wk->msgman, f_rom_fes_unit_01+value, wk->str_tmp );
    break;
  
  case WSET_MEDIT_PATTERN:
    GFL_MSG_GetString( wk->msgman, f_rom_fes_attr_01+value, wk->str_tmp );
    break;
  
  case WSET_MEDIT_NUM:
    WORDSET_RegisterNumber( wk->wset, idx, value, keta, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    return TRUE;
  
  case WSET_MEDIT_TITLE:
  case WSET_MEDIT_EXP:
    WORDSET_RegisterNumber( wk->wset, idx, value, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    GFL_MSG_GetString( wk->msgman, f_rom_mission_msg_id_01+value, wk->str_tmp );
    print_FreeWordset( wk->wset, idx+1, wk->str_tmp );
    param_MissionPopToFMWKEdit( wk, &wk->tmp_fmwk );
    if( type == WSET_MEDIT_TITLE ){
      FMISSION_DATA_GetTitleStr( wk->fm_data_man, wk->str_tmp, &wk->tmp_fmwk, wk->heapID );
    }else{
      FMISSION_DATA_GetExpStr( wk->fm_data_man, wk->str_tmp, &wk->tmp_fmwk, wk->heapID );
    }
    print_FreeWordset( wk->wset, idx+2, wk->str_tmp );
    return TRUE;

  case WSET_SEND_NOW_RTC:
    {
      RTCDate date;
      RTCTime time;
      GFL_RTC_GetDateTime( &date, &time );
      WORDSET_RegisterNumber( wk->wset, idx+0, date.year+2000, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, idx+1, date.month, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, idx+2, date.day, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, idx+3, time.hour, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, idx+4, time.minute, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, idx+5, time.second, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    }
    return TRUE; 
  
  case WSET_SEND_TITLE:
    {
      int m_id = param_MissionPopToFMWK( wk, wk->send_id, &wk->tmp_fmwk );
      
      WORDSET_RegisterNumber( wk->wset, idx, m_id, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, idx+1, wk->send_id+1, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      if( m_id == 0 ){
        GF_ASSERT(0);
        GFL_MSG_GetString( wk->msgman, f_rom_param_msend_id_none, wk->str_tmp );
        print_FreeWordset( wk->wset, idx+2, wk->str_tmp );
        WORDSET_RegisterClear( wk->wset, idx+3 );
        WORDSET_RegisterClear( wk->wset, idx+4 );
      }else{
        FMISSION_DATA_GetTitleStr( wk->fm_data_man, wk->str_tmp, &wk->tmp_fmwk, wk->heapID );
        print_FreeWordset( wk->wset, idx+2, wk->str_tmp );
        //ミッションタイプ展開
        GFL_MSG_GetString( wk->msgman, f_rom_mission_type_01+wk->tmp_fmwk.arc.m_type-1, wk->str_tmp );
        print_FreeWordset( wk->wset, idx+3, wk->str_tmp );
        //ミッション終了タイプ展開
        GFL_MSG_GetString( wk->msgman, f_rom_mission_end_type_01+wk->tmp_fmwk.beacon.end_type, wk->str_tmp );
        print_FreeWordset( wk->wset, idx+4, wk->str_tmp );
      }
    }
    return TRUE;

  case WSET_SEND_EXP:
    {
      int m_id = param_MissionPopToFMWK( wk, wk->send_id, &wk->tmp_fmwk );
      
      if( m_id == 0 ){
        GF_ASSERT(0);
        WORDSET_RegisterClear( wk->wset, idx );
      }else{
        FMISSION_DATA_GetExpStr( wk->fm_data_man, wk->str_tmp, &wk->tmp_fmwk, wk->heapID );
        print_FreeWordset( wk->wset, idx, wk->str_tmp );
      }
    }
    return TRUE;
  
  case WSET_SEND_LEST_TIME:
    {
      int min = wk->send_lest_sec/60;
      int sec = wk->send_lest_sec%60;
      WORDSET_RegisterNumber( wk->wset, idx, min, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( wk->wset, idx+1, sec, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    }
    return TRUE;
  }
#if 0
  switch( type ){
  case BPRM_WSET_TRNAME_EX:
    if( value == 0){
      MyStatus_CopyNameString( wk->my_status, wk->str_tmp );
    }else{
      WORDSET_RegisterTrainerName( wk->wset, idx, value );
      return TRUE;
    }
    break;
  case BPRM_WSET_MONSNAME:
  case BPRM_WSET_MBASE_EFF_ENC:
    WORDSET_RegisterPokeMonsNameNo( wk->wset, idx, value );
    return TRUE;
  case BPRM_WSET_ITEMNAME:
    WORDSET_RegisterItemName( wk->wset, idx, value );
    return TRUE;
  case BPRM_WSET_ITEMNAME_CHECK:
    WORDSET_RegisterItemName( wk->wset, idx, value );
    if( ITEM_IsDisableItem( value ) ||
        ITEM_IsOnlyOneItem( value ) ||
        value == ITEM_MASUTAABOORU){
      return FALSE;
    }
    return TRUE;
  case BPRM_WSET_ITEMNAME_CHECK_SP:
    WORDSET_RegisterItemName( wk->wset, idx, value );
    if( value == 0 ) return TRUE;
    if( ITEM_IsDisableItem( value ) ||
        ITEM_IsOnlyOneItem( value ) ){
      return FALSE;
    }
    return TRUE;
  case BPRM_WSET_FWAZA:
    WORDSET_RegisterWazaName( wk->wset, idx, DATA_FieldWazaTbl[value]);
    return TRUE;
  case BPRM_WSET_GPOWER:
    if( GPOWER_IsEnableID( value ) ){
      WORDSET_RegisterGPowerName( wk->wset, idx, value );
      return TRUE;
    }else{
      GFL_MSG_GetString( wk->msgman, dlf_ng_gpower_name, wk->str_tmp );
      print_FreeWordset( wk->wset, idx, wk->str_tmp );
      return FALSE;
    }
  case BPRM_WSET_ACTION:
    GFL_MSG_GetString( wk->msgman, dlf_action_00+value, wk->str_tmp );
    break;
  case BPRM_WSET_LOG_REQ_TYPE:
    GFL_MSG_GetString( wk->msgman, dlf_log_req_type01+value, wk->str_tmp );
    break;
  case BPRM_WSET_SEX:
    GFL_MSG_GetString( wk->msgman, dlf_sex_male+value, wk->str_tmp );
    break;
  case BPRM_WSET_VERSION:
    GFL_MSG_GetString( wk->msgman, dlf_version_white+(value-VERSION_W2), wk->str_tmp );
    break;
  case BPRM_WSET_LANG:
    GFL_MSG_GetString( wk->msgman, dlf_lang_01+(value-LANG_JAPAN), wk->str_tmp );
    break;
  case BPRM_WSET_ZONE:
    WORDSET_RegisterPlaceName( wk->wset, 7, ZONEDATA_GetPlaceNameID(value) );
    sub_GetMsgToBuf( wk, dlf_tmp_buf );
    sub_GetZoneIDStr( wk->pAllZoneName, value, wk->str_tmp );
    GFL_STR_AddString( wk->str_tmp, wk->str_expand );
    break;
  case BPRM_WSET_FREEWORD:
    GFL_MSG_GetString( wk->msgman, dlf_freeword_icon_01+value, wk->str_tmp );
    break;
  case BPRM_WSET_TR_VIEW:
    {
      u8 ofs = 0;
      if( wk->fake_prm[ TR_PAT_FAKE-(idx==3)][FAKE_SEX] ){
        ofs = UNION_VIEW_INDEX_WOMAN_START;
      }
      GFL_MSG_GetString( wk->msgman, dlf_tr_view_01+value+ofs, wk->str_tmp );
    }
    break;
  case BPRM_WSET_MBASE_MUSHROOM_ENC:
    WORDSET_RegisterPokeMonsNameNo( wk->wset, idx, DATA_MushroomMonsTbl[value] );
    return TRUE;
  case BPRM_WSET_MISSION_ID_NAME:
    GFL_MSG_GetString( wk->msgman, dlf_mission_id_name_01+value, wk->str_tmp );
    break;
  case BPRM_WSET_MISSION_MSGID_NAME:
    GFL_MSG_GetString( wk->msgman, dlf_mission_msg_id_01+value, wk->str_tmp );
    break;
  }
#endif
  print_FreeWordset( wk->wset, idx, wk->str_tmp );
  return TRUE;
}
 
/*
 *  @brief  SE再生
 */
static void sub_PlaySE( u16 se_no )
{
  PMSND_PlaySE( se_no );
}

/*
 *  @パレット転送
 */
static void sub_PlttVramTrans( u16* p_data, u8 target, u16 pos, u16 num  )
{
  u32 adrs = pos*2;
  u32 siz = num*2;

  DC_FlushRange( p_data, siz );

  switch( target ){
  case FADE_MAIN_BG:
    GX_LoadBGPltt( p_data, adrs, siz );
    break;
  case FADE_SUB_BG:
    GXS_LoadBGPltt( p_data, adrs, siz );
    break;
  case FADE_MAIN_OBJ:
    GX_LoadOBJPltt( p_data, adrs, siz );
    break;
  case FADE_SUB_OBJ:
    GXS_LoadOBJPltt( p_data, adrs, siz );
    break;
  }
}

/*
 *  @brief  アクター アニメスタート
 */
static void act_AnmStart( GFL_CLWK* act, u8 anm_no )
{
  GFL_CLACT_WK_SetAnmSeq( act, anm_no );
  GFL_CLACT_WK_ResetAnm( act );
}

/*
 *  @brief  アクター　座標セット
 */
static void act_SetPosition( GFL_CLWK* act, s16 x, s16 y, u16 setsf )
{
  GFL_CLACTPOS pos;
  pos.x = x;
  pos.y = y;
  
  GFL_CLACT_WK_SetPos( act, &pos, setsf );
}


/*
 *  @brief  Unionアクター更新
 */
static void draw_UnionObjUpdate( FES_ROM_WORK* wk, u8 char_no )
{
  u8 sex, p_ofs;
 
  //キャラクタ転送
  GFL_CLGRP_CGR_Replace(  wk->objResUnion.cgr,
                          wk->resCharUnion[ char_no ].p_char );
  //パレット転送
  p_ofs = 16*sc_wifi_unionobj_plt[char_no];
  sub_PlttVramTrans( &wk->resPlttUnion.dat[p_ofs], FADE_MAIN_OBJ, PLTID_OBJ_UNION_M*16, 16 );
}

/*
 *  @brief  単発メッセージ描画
 */
static void draw_MsgWinFromID( FES_ROM_WORK* wk, TARGET_VIEW view, int msg_id )
{
  print_GetMsgToBuf( wk, msg_id );
  draw_MsgWin( wk, view, wk->str_expand);
}

/*
 *  @brief  メッセージウィンドウ描画 サブ
 */
static void draw_MsgWin( FES_ROM_WORK* wk, TARGET_VIEW view, STRBUF* str )
{
  TARGET_WIN* twin = &wk->target[ view ];

  GFL_BMP_Clear( twin->msg.bmp, FCOL_WIN_WHITE );
	PRINTSYS_PrintColor( twin->msg.bmp, 0, 0, str, wk->font, FCOL_BLACK_W );
  GFL_BMPWIN_MakeTransWindow( twin->msg.win );
}
/*
 *  @brief  メッセージウィンドウ描画 サブ
 */
static void draw_MsgClear( FES_ROM_WORK* wk, TARGET_VIEW view )
{
  TARGET_WIN* twin = &wk->target[ view ];
  GFL_BMP_Clear( twin->msg.bmp, FCOL_WIN_CLEAR );
  GFL_BMPWIN_MakeTransWindow( twin->msg.win );
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  セーブリクエスト
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_SAVE{
  u8  seq;
  int child_task;

  int wait;
  FES_ROM_WORK* few;
  int* task_ct;
}TASKWK_SAVE;

static void taskAdd_Save( FES_ROM_WORK* wk, int* task_ct );
static void tcb_Save( GFL_TCBL *tcb , void* work);

static void taskReq_Save( FES_ROM_WORK* wk )
{
  wk->ret_seq = SEQ_BASE_MAIN;
  taskAdd_Save( wk, &wk->task_ct);
}

/*
 *  @brief セーブリクエスト 
 */
static void taskAdd_Save( FES_ROM_WORK* wk, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_SAVE* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_Save, sizeof(TASKWK_SAVE), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_SAVE ));

  twk->few = wk;

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_Save( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_SAVE* twk = (TASKWK_SAVE*)tcb_wk;
  FES_ROM_WORK* wk = twk->few;

  switch( twk->seq ){
  case 0:
    draw_MsgWinFromID( wk, TARGET_VIEW_U, f_rom_msg_save_check );
    sub_PlaySE( FESROM_SE_POPUP );
    twk->seq++;
    return;
  case 1:
    if( twk->wait++ < 4 ){
      return;
    }
    twk->wait = 0;
    twk->seq++;
    return;
  case 2:
    {
      int trg = GFL_UI_KEY_GetTrg();

      if( trg & PAD_BUTTON_B ){
        draw_MsgClear( wk, TARGET_VIEW_U );
        break;
      }
      if( trg & PAD_BUTTON_A ){
        param_BasePush( wk );
        FesRomSys_SaveWrite( wk->sys_wk );
        draw_MsgWinFromID( wk, TARGET_VIEW_U, f_rom_msg_save_end );
        twk->seq++;
      }
    }
    return;
  case 3:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_MASK ){
      draw_MsgClear( wk, TARGET_VIEW_U );
      break;
    }
    return;
  }
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  数値入力 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_INPUT_NUM{
  u8  seq;
  int child_task;
 
  PARAM_SET* pset;

  FES_ROM_WORK* few;
  int* task_ct;
}TASKWK_INPUT_NUM;

static void taskAdd_InputNum( FES_ROM_WORK* wk, PARAM_SET* pset, int* task_ct );
static void tcb_InputNum( GFL_TCBL *tcb , void* work);

static void taskReq_InputNum( FES_ROM_WORK* wk, PARAM_SET* pset )
{
  taskAdd_InputNum( wk, pset, &wk->task_ct);
}

/*
 *  @brief  スイッチ式数値入力タスク登録
 */
static void taskAdd_InputNum( FES_ROM_WORK* wk, PARAM_SET* pset, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_INPUT_NUM* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_InputNum, sizeof(TASKWK_INPUT_NUM), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_INPUT_NUM ));

  twk->few = wk;

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_InputNum( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_INPUT_NUM* twk = (TASKWK_INPUT_NUM*)tcb_wk;

  switch( twk->seq ){
  case 0:
    sub_PlaySE( FESROM_SE_POPUP );
    twk->seq++;
    break;
  case 1:
    twk->seq++;
    return;
  case 2:
    break;
  }
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ ポップアップ 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_WIN_POPUP{
  u8  seq;
  u8  wait;
  int child_task;
 
  FES_ROM_WORK* bdw;
  int* task_ct;
}TASKWK_WIN_POPUP;

static void taskAdd_WinPopup( FES_ROM_WORK* wk, int* task_ct );
static void tcb_WinPopup( GFL_TCBL *tcb , void* work);

static void effReq_PopupMsg( FES_ROM_WORK* wk )
{
  taskAdd_WinPopup( wk, &wk->task_ct);
}

/*
 *  @brief  メッセージウィンドウ ポップアップタスク登録
 */
static void taskAdd_WinPopup( FES_ROM_WORK* wk, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_POPUP* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinPopup, sizeof(TASKWK_WIN_POPUP), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_POPUP ));

  twk->bdw = wk;
  twk->wait = 60*3;

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static BOOL popup_CheckTimeWait( TASKWK_WIN_POPUP* twk )
{
  if( GFL_UI_TP_GetTrg() || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_A)){
    twk->wait = 0;
    return FALSE;
  }
  if( twk->wait-- == 0){
    return FALSE;
  }
  return TRUE;
}

static void tcb_WinPopup( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_POPUP* twk = (TASKWK_WIN_POPUP*)tcb_wk;

  switch( twk->seq ){
  case 0:
    sub_PlaySE( FESROM_SE_POPUP );
    twk->seq++;
    break;
  case 1:
    if(popup_CheckTimeWait(twk)){
      return;
    }
    twk->seq++;
    return;
  case 2:
    break;
  }
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  BMPウィンドウメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_BMPWIN_PRINT{
  BMP_WIN* win;
  FES_ROM_WORK* bdw;
  int* task_ct;
}TASKWK_BMPWIN_PRINT;

static void taskAdd_BmpwinPrint( FES_ROM_WORK* wk, BMP_WIN* win, int* task_ct );
static void tcb_BmpwinPrint( GFL_TCBL *tcb , void* work);

static void printReq_BmpwinPrint( FES_ROM_WORK* wk, BMP_WIN* win, u8 x, u8 y, STRBUF* str, PRINTSYS_LSB color )
{
  PRINT_UTIL_PrintColor( &win->putil, wk->print_que, x, y, str, wk->font, color );
  taskAdd_BmpwinPrint( wk, win, &wk->task_ct );
}

/*
 *  @brief  メッセージウィンドウ パネルフラッシュタスク登録
 */
static void taskAdd_BmpwinPrint( FES_ROM_WORK* wk, BMP_WIN* win, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_BMPWIN_PRINT* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_BmpwinPrint, sizeof(TASKWK_BMPWIN_PRINT), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_BMPWIN_PRINT ));

  twk->bdw = wk;
  twk->win = win;

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_BmpwinPrint( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_BMPWIN_PRINT* twk = (TASKWK_BMPWIN_PRINT*)tcb_wk;
  FES_ROM_WORK* bdw = twk->bdw;

  if( !PRINT_UTIL_Trans( &twk->win->putil, bdw->print_que )){
    return;
  }

  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}

