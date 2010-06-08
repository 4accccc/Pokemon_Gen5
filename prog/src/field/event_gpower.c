//============================================================================================
/**
 * @file    event_gpower.c
 * @brief	  イベント：Gパワー関連イベント
 * @author  iwasawa	
 * @date	  2009.02.17
 */
//============================================================================================
#include <gflib.h>
#include "system/main.h"      //GFL_HEAPID_APP参照
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "./event_fieldmap_control.h"
#include "./event_gpower.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "script_message.naix"
#include "msg/script/msg_gpower_event.h"

#include "../../../resource/fldmapdata/script/gpower_scr_def.h" //SCRID_～

///////////////////////////////////////////////////////
//リテラル
///////////////////////////////////////////////////////
#define BUFLEN_TMP_MSG  (64*2+EOM_SIZE)

#define CWIN_STR_OX01  (0)
#define CWIN_STR_OX02  (8)
#define CWIN_STR_OY    (16)

typedef struct POWER_FLD_EFF{
  u8  ct;
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
}POWER_FLD_EFF;

typedef struct POWER_CHECK_WORK{
  u8  ct;
  HEAPID  heapID;
  HEAPID  tmpHeapID;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;

  FLDMSGBG* fmb;
  FLDSYSWIN* win;
  MMDLSYS * mmdl_sys;
  FLDSYSWIN_STREAM* winStream;
  GFL_BMPWIN* bmpwin;
  GFL_MSGDATA* msgData;
  WORDSET* wset;
  POWER_CONV_DATA* p_data;

  STRBUF* s_buf;
  STRBUF* s_tmp;
}POWER_CHECK_WORK;

static GMEVENT_RESULT event_GPowerUseEffectMain(GMEVENT * event, int *  seq, void * work);
static void sub_InstantPowerUse( GAMESYS_WORK* gsys, WORDSET* wordset, GPOWER_TYPE type );
static GMEVENT_RESULT event_GPowerEnableListCheckMain(GMEVENT * event, int *  seq, void * work);
static void sub_InitPowerCheckWork( POWER_CHECK_WORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap );
static void sub_ReleasePowerCheckWork( POWER_CHECK_WORK* wk );
static void sub_GPowerEnableListLineDraw( POWER_CHECK_WORK* wk, STRBUF* str, GPOWER_ID g_power, u16 time, u8 line );
static void sub_GPowerEnableListDraw( POWER_CHECK_WORK* wk );

/*
 *  @brief  Gパワー発動イベント
 *
 *  @param  g_power 発動するGパワーID
 *  @param  mine    自分のGパワーかどうか？
 */
GMEVENT* EVENT_GPowerEffectStart(GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  SCRIPT_WORK* sc;
  GPOWER_EFFECT_PARAM* prm = (GPOWER_EFFECT_PARAM*)work;
  HEAPID tmpHeapID = GFL_HEAP_LOWID( HEAPID_FIELDMAP );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData(gsys);
  POWER_CONV_DATA * p_data = GPOWER_PowerData_LoadAlloc( tmpHeapID );

  event = SCRIPT_SetEventScript( gsys, SCRID_GPOWER_EFFECT_START, NULL, HEAPID_FIELDMAP );
  sc = SCRIPT_GetScriptWorkFromEvent( event );
  {
    GPOWER_TYPE type = GPOWER_ID_to_Type( p_data, prm->g_power );
    GPOWER_Set_OccurID( prm->g_power, p_data, prm->mine_f );

    SCRIPT_SetScriptWorkParam( sc, prm->g_power, type, prm->mine_f, 0 );
    sub_InstantPowerUse( gsys, SCRIPT_GetWordSet( sc ), type );
  }
  if( prm->mine_f ){ //自分のを使った時
    u16 point = GPOWER_ID_to_Point( p_data, prm->g_power );
    MYITEM_SubItem( GAMEDATA_GetMyItem( gdata ), ITEM_DERUDAMA, point, tmpHeapID );
    GAMEBEACON_Set_GPower( prm->g_power );
  }else{  //他人のを使った時
    GAMEBEACON_Set_OtherGPowerUse( prm->g_power );
  }
  GPOWER_PowerData_Unload( p_data );

  return event;
}

/*
 *  @brief  Gパワー効果終了イベント
 */
GMEVENT* EVENT_GPowerEffectEnd( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;

  event = SCRIPT_SetEventScript( gsys, SCRID_GPOWER_EFFECT_END, NULL, HEAPID_FIELDMAP );
  return event;
}

/*
 *  @brief  Gパワー効果発動エフェクト
 */
GMEVENT* EVENT_GPowerUseEffect( GAMESYS_WORK * gsys )
{
  GMEVENT* event;
  POWER_FLD_EFF* wk;

  event = GMEVENT_Create(gsys, NULL, event_GPowerUseEffectMain, sizeof(POWER_FLD_EFF));
  
  wk = GMEVENT_GetEventWork(event);
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->gsys = gsys;

  return event;
}

////////////////////////////////////////////////////////////////////////////
//発動中のGパワーを確認するイベント
////////////////////////////////////////////////////////////////////////////
/*
 *  @brief  発動中のGパワー確認イベント
 */
GMEVENT* EVENT_GPowerEnableListCheck( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  POWER_CHECK_WORK* wk;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)work;

  event = GMEVENT_Create(gsys, NULL, event_GPowerEnableListCheckMain, sizeof(POWER_CHECK_WORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitPowerCheckWork( wk, gsys, fieldmap );
  return event;
}

static void sub_InitPowerCheckWork( POWER_CHECK_WORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap )
{
  wk->heapID = HEAPID_FIELDMAP;
  wk->tmpHeapID = GFL_HEAP_LOWID( wk->heapID );

  wk->gsys = gsys;
  wk->fieldmap = fieldmap;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fmb = FIELDMAP_GetFldMsgBG( wk->fieldmap );
  wk->mmdl_sys = FIELDMAP_GetMMdlSys( wk->fieldmap );

  wk->p_data = GPOWER_PowerData_LoadAlloc( wk->heapID );
  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          NARC_script_message_gpower_event_dat, wk->heapID );
 
  wk->wset = WORDSET_Create( wk->heapID );
 
  wk->s_buf = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->s_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  
  MMDLSYS_PauseMoveProc( wk->mmdl_sys );
}

static void sub_ReleasePowerCheckWork( POWER_CHECK_WORK* wk )
{
  GFL_STR_DeleteBuffer( wk->s_tmp );
  GFL_STR_DeleteBuffer( wk->s_buf );
  WORDSET_Delete( wk->wset );
  GFL_MSG_Delete( wk->msgData );
  GPOWER_PowerData_Unload( wk->p_data );

  MMDLSYS_ClearPauseMoveProc( wk->mmdl_sys );
}



//============================================================================================
//
//    サブイベント
//
//============================================================================================

//-----------------------------------------------------------------
/*
 *  @brief  Gパワー発動エフェクトメイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_GPowerUseEffectMain(GMEVENT * event, int *  seq, void * work)
{
  POWER_FLD_EFF* wk = work;

  switch (*seq) {
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 10, 16, 4);
    PMSND_PlaySE( SE_POWER_USE );
    (*seq)++;
    break;
  case 1:
		if( GFL_FADE_CheckFade() ){
      break;
    }
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, 8);
    (*seq)++;
  case 2:
		if( GFL_FADE_CheckFade() ){
      break;
    }
  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  イベント発動時に効果が現れるパワーの実行処理
 */
//-----------------------------------------------------------------
static void sub_InstantPowerUse( GAMESYS_WORK* gsys, WORDSET* wordset, GPOWER_TYPE type )
{
  u8 pos;
  POKEPARTY* party;
  POKEMON_PARAM* pp;

  party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData( gsys ) );
  pos = PokeParty_GetMemberTopIdxBattleEnable( party );
  pp = PokeParty_GetMemberPointer(party, pos);
  
  WORDSET_RegisterPokeNickName( wordset, 1, pp );

  switch(type){
  case GPOWER_TYPE_HP_RESTORE:
    GPOWER_Calc_HPRestore(party);
    break;
  case GPOWER_TYPE_PP_RESTORE:
    GPOWER_Calc_PPRestore(party);
    break;
  }
}

//-----------------------------------------------------------------
/*
 *  @brief  Gパワー発動中の効果エフェクトメイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_GPowerEnableListCheckMain(GMEVENT * event, int *  seq, void * work)
{
  POWER_CHECK_WORK* wk = work;

  switch (*seq) {
  case 0:
    wk->winStream = FLDSYSWIN_STREAM_Add( wk->fmb, wk->msgData, 19 );
    FLDSYSWIN_STREAM_PrintStart( wk->winStream, 0, 0, msg_gpower_check_start );
    (*seq)++;
    break;
  case 1:
    if( !FLDSYSWIN_STREAM_Print( wk->winStream )){
      break;
    }
    FLDSYSWIN_STREAM_Delete( wk->winStream );
    (*seq)++;
    break;
  case 2:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, 3, 1, 24, 22 );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_GPowerEnableListDraw( wk );
    (*seq)++;
    break;
  case 3:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)){
      (*seq)++;
    }
    break;
  default:
    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );
    sub_ReleasePowerCheckWork( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  発動中パワー名描画
 */
//-----------------------------------------------------------------
static void sub_GPowerEnableListLineDraw( POWER_CHECK_WORK* wk, STRBUF* str, GPOWER_ID g_power, u16 time, u8 line )
{
  WORDSET_RegisterGPowerName( wk->wset, 0, g_power );
  IWASAWA_Printf("GPower %d Life = %d\n",g_power,time);
  WORDSET_RegisterNumber( wk->wset, 1, time/60, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wk->wset, 2, time%60, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

  WORDSET_ExpandStr( wk->wset, wk->s_tmp, str );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX02, CWIN_STR_OY*line, wk->s_tmp );
}

//-----------------------------------------------------------------
/*
 *  @brief  Gパワー発動中の効果リスト描画
 */
//-----------------------------------------------------------------
static void sub_GPowerEnableListDraw( POWER_CHECK_WORK* wk )
{
  INTRUDE_SAVE_WORK* int_sv = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( wk->gdata ));
  u8  tmp,power[GPOWER_TYPE_MAX],i,j,line;
  u8  no[GPOWER_TYPE_MAX];
  u16 time[GPOWER_TYPE_MAX];

  //設定しているデルパワーラベル
  GFL_MSG_GetString( wk->msgData, msg_gpower_check_label01, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );
  
  //設定しているデルパワー
  GFL_MSG_GetString( wk->msgData, msg_gpower_check_list01, wk->s_buf );
  tmp = ISC_SAVE_GetGPowerID(int_sv);
  if( tmp != GPOWER_ID_NULL ){
    sub_GPowerEnableListLineDraw( wk, wk->s_buf, tmp, 0, 1 );
  }
  //発動しているデルパワー
  GFL_MSG_GetString( wk->msgData, msg_gpower_check_label02, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*2, wk->s_buf );

  GFL_MSG_GetString( wk->msgData, msg_gpower_check_list02, wk->s_buf );
  for( i = 0, line = 0 ;i < GPOWER_TYPE_MAX;i++){
    no[i] = i;
    tmp = GPOWER_Check_OccurType( i );
    if( tmp == GPOWER_ID_NULL ){
      continue;
    }
    time[line] = GPOWER_Check_OccurTypeLife( i );
    power[line] = tmp;
    line++;
//    sub_GPowerEnableListLineDraw( wk, wk->s_buf, power, time, 3+line++ );
  }
  for(i = 0;i < (line-1);i++){
    for(j = (line-1);j>i;j--){
      if(time[no[j-1]] > time[no[j]]){
        tmp = no[j];
        no[j] = no[j-1];
        no[j-1] = tmp;
      }
    }
  }
  for(i = 0;i < line;i++){
    sub_GPowerEnableListLineDraw( wk, wk->s_buf, power[no[i]], time[no[i]], 3+i );
  }
}
