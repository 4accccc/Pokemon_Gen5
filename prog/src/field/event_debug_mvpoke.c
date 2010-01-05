/*
 *  @file   event_debug_mvpoke.c
 *  @brief  移動ポケモンデバッグ
 *  @author Miyuki Iwasawa
 *  @date   09.12.28
 */
#ifdef PM_DEBUG

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/rtc_tool.h"
#include "system/bmp_winframe.h"

#include "arc/fieldmap/zone_id.h"
#include "savedata/encount_sv.h"
#include "savedata/mystatus.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "battle/battle.h"

#include "gamesystem/btl_setup.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "message.naix"
#include "msg/msg_d_field.h"
#include "msg/msg_d_iwasawa.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "field/fieldmap.h"
#include "./event_fieldmap_control.h"
#include "event_debug_item.h"
#include "font/font.naix" //NARC_font_large_gftr
#include "sound/pm_sndsys.h"

#include "field/zonedata.h"
#include "field/weather_no.h"
#include "move_pokemon_def.h"
#include "move_pokemon.h"

#include "event_debug_local.h"
#include "event_debug_mvpoke.h"

enum{
 SEQ_INIT,
 SEQ_MAIN,
 SEQ_ZONE_CHANGE,
 SEQ_END,
};

#define ZONE_IDX_MAX  (MVPOKE_LOCATION_MAX+1)

#define DEBUG_MVPOKE_FRAME    ( FLDBG_MFRM_MSG )
#define DEBUG_MVPOKE_MSGPAL   ( 11 )
#define DEBUG_MVPOKE_WINPAL   ( 12 )

#define WIN_PX  (1)
#define WIN_PY  (1)
#define WIN_SX  (30)
#define WIN_SY  (16)
#define WIN_PAL ( DEBUG_MVPOKE_WINPAL )

#define DEBUG_MVPOKE_BUFLEN (256)

#define WINCLR_COL(col)	(((col)<<4)|(col))
#define FCOL_WHITE  (15)

#define COL_DEF_MAIN    (2)
#define COL_DEF_SHADOW  (1)
#define COL_DEF_BACK    (FCOL_WHITE)

#define COL_CHG_MAIN    (3)
#define COL_CHG_SHADOW  (1)
#define COL_CHG_BACK    (FCOL_WHITE)

typedef struct _EVENT_DEB_MVPOKE{
  HEAPID  heapID;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;
  GMEVENT* event;
 	MPD_PTR	 mpd;
	ENC_SV_PTR enc;
  
  GFL_FONT    *font;
  GFL_MSGDATA *msg_deb;
  GFL_MSGDATA *msg_place;
  WORDSET     *wset;
  STRBUF*     str_buf;
  STRBUF*     str_poke;
  STRBUF*     str_map;
  STRBUF*     str_prm;

  GFL_ARCUTIL_TRANSINFO bgTransInfo;
  u32 bgTransPos;
  u32 bgTransSize;

  GFL_BMPWIN* win;
  GFL_BMP_DATA* win_bmp;

  u8  mvpoke_idx;
  u8  mvpoke_max;
  u8  zone_idx;
}EVENT_DEB_MVPOKE;


static const u16 DATA_DebugMvPokeNo[] = {
 MONSNO_RAIKAMI,MONSNO_KAZAKAMI,
};

static GMEVENT_RESULT EVENT_DebugMovePokemon(GMEVENT * event, int *  seq, void * work);

static void sub_MenuInit( EVENT_DEB_MVPOKE* wk );
static void sub_MenuRelease( EVENT_DEB_MVPOKE* wk );
static int sub_MenuMain( EVENT_DEB_MVPOKE* wk );
static int sub_ZoneChangeMain( EVENT_DEB_MVPOKE* wk );

static void sub_GraphicInit(EVENT_DEB_MVPOKE* wk);
static void sub_GraphicRelease( EVENT_DEB_MVPOKE* wk );

static void sub_MvPokeInfoDraw( EVENT_DEB_MVPOKE* wk, u8 idx );
static void sub_MvPokeZoneDraw( EVENT_DEB_MVPOKE* wk, u8 zone_idx, BOOL trans );

//------------------------------------------------------------------------------
/**
 * @brief   移動ポケモンデバッグ 
 * @retval  none
 */
//------------------------------------------------------------------------------
GMEVENT* EVENT_DebugMovePoke( GAMESYS_WORK* gsys, void* work )
{
  GMEVENT *event;
  EVENT_DEB_MVPOKE * wk;
  DEBUG_MENU_EVENT_WORK* dme_wk = (DEBUG_MENU_EVENT_WORK*)work;

  event = GMEVENT_Create( gsys, NULL, EVENT_DebugMovePokemon, sizeof(EVENT_DEB_MVPOKE) );
  wk = GMEVENT_GetEventWork(event);
  
  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData( wk->gsys );
  wk->fieldmap = dme_wk->fieldWork;
  wk->event = event;
  wk->heapID = dme_wk->heapID;

  wk->mvpoke_idx = 0;
  wk->mvpoke_max = 2;

  wk->enc = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( wk->gdata ) );

  return event;
}

/*
 *  @brief  メインイベント
 */
static GMEVENT_RESULT EVENT_DebugMovePokemon(GMEVENT * event, int *  seq, void * work)
{
  EVENT_DEB_MVPOKE * wk = work;
  GFL_PROC_RESULT ret;

  switch (*seq) {
  case SEQ_INIT:
    sub_MenuInit( wk );
    (*seq)++;
    break;
  case SEQ_MAIN:
    *seq = sub_MenuMain( wk );
    break;
  case SEQ_ZONE_CHANGE:
    *seq = sub_ZoneChangeMain( wk );
    break;
  case SEQ_END: 
    sub_MenuRelease( wk );
    return GMEVENT_RES_FINISH;
  default:
    break;
  }
  return GMEVENT_RES_CONTINUE;

}

/*
 *  @brief  メニュー初期化
 */
static void sub_MenuInit( EVENT_DEB_MVPOKE* wk )
{
  sub_GraphicInit(wk);

  wk->msg_deb = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                               NARC_message_d_iwasawa_dat, wk->heapID );

  //地名メッセージデータ
	wk->msg_place = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, wk->heapID );

  wk->wset = WORDSET_Create( wk->heapID );

  wk->str_buf = GFL_STR_CreateBuffer( DEBUG_MVPOKE_BUFLEN ,wk->heapID );
  wk->str_poke = GFL_MSG_CreateString( wk->msg_deb, d_iwa_mvpoke_01 );
  wk->str_map = GFL_MSG_CreateString( wk->msg_deb, d_iwa_mvpoke_02 );
  wk->str_prm = GFL_MSG_CreateString( wk->msg_deb, d_iwa_mvpoke_03 );

  sub_MvPokeInfoDraw( wk, wk->mvpoke_idx );
}

/*
 *  @brief  メニュー解放
 */
static void sub_MenuRelease( EVENT_DEB_MVPOKE* wk )
{
  GFL_STR_DeleteBuffer(wk->str_prm);
  GFL_STR_DeleteBuffer(wk->str_map);
  GFL_STR_DeleteBuffer(wk->str_poke);
  GFL_STR_DeleteBuffer(wk->str_buf);
  WORDSET_Delete(wk->wset);
  GFL_MSG_Delete(wk->msg_place);
  GFL_MSG_Delete(wk->msg_deb);

  sub_GraphicRelease(wk);
}

/*
 *  @brief  メニューメイン
 */
static int sub_MenuMain( EVENT_DEB_MVPOKE* wk )
{
  int code = 0;
  int trg = GFL_UI_KEY_GetTrg();

  if( trg & PAD_BUTTON_B ){
    return SEQ_END;
  }
  if( trg & PAD_KEY_UP ){
    wk->mvpoke_idx = (wk->mvpoke_idx+(wk->mvpoke_max-1))%wk->mvpoke_max;
    code = 1;
  }else if( trg & PAD_KEY_DOWN ){
    wk->mvpoke_idx = (wk->mvpoke_idx+1)%wk->mvpoke_max;
    code = 1;
  }

  if(code){
    sub_MvPokeInfoDraw( wk, wk->mvpoke_idx );
    return SEQ_MAIN;
  }

  if( wk->mpd == NULL ){
    return SEQ_MAIN;
  }

  if( trg & PAD_BUTTON_A ){
    GFL_FONTSYS_SetColor( COL_CHG_MAIN, COL_CHG_SHADOW, COL_CHG_BACK );
    sub_MvPokeZoneDraw( wk, wk->zone_idx, TRUE );
    return SEQ_ZONE_CHANGE;
  }
  return SEQ_MAIN;
}

/*
 *  @brief  ゾーン変更メイン
 */
static int sub_ZoneChangeMain( EVENT_DEB_MVPOKE* wk )
{
  u8 zone_idx,input = 0;
  int trg = GFL_UI_KEY_GetTrg();
  
  if( trg & PAD_BUTTON_B ){
    wk->zone_idx = EncDataSave_GetMovePokeZoneIdx( wk->enc, wk->mvpoke_idx );
    input = 1;
  }else if( trg & PAD_BUTTON_A ){
    EncDataSave_SetMovePokeZoneIdx( wk->enc , wk->mvpoke_idx, wk->zone_idx );
	  EncDataSave_SetMovePokeDataParam( wk->mpd, MP_PARAM_ZONE_ID, MP_MovePokeZoneIdx2ID(wk->zone_idx));
    input = 1;
  }
  if( input ){
    GFL_FONTSYS_SetColor( COL_DEF_MAIN, COL_DEF_SHADOW, COL_DEF_BACK );
    sub_MvPokeZoneDraw( wk, wk->zone_idx, TRUE );
    return SEQ_MAIN;
  }

  if( trg & PAD_KEY_LEFT ){
    wk->zone_idx = (wk->zone_idx+1)%ZONE_IDX_MAX; 
    input = 2;
  }else if( trg & PAD_KEY_RIGHT ){
    wk->zone_idx = (wk->zone_idx+(ZONE_IDX_MAX-1))%ZONE_IDX_MAX; 
    input = 2;
  }

  if(input == 2){
    sub_MvPokeZoneDraw( wk, wk->zone_idx, TRUE );
  }
  return SEQ_ZONE_CHANGE;
}

/*
 *  @brief  グラフィック初期化
 */
static void sub_GraphicInit(EVENT_DEB_MVPOKE* wk)
{
  GFL_FONTSYS_SetDefaultColor();
  wk->font = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                              GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapID );
 
  GFL_FONTSYS_SetColor( COL_DEF_MAIN, COL_DEF_SHADOW, COL_DEF_BACK );

  wk->bgTransInfo = BmpWinFrame_GraphicSetAreaMan(
                    DEBUG_MVPOKE_FRAME, DEBUG_MVPOKE_WINPAL, MENU_TYPE_SYSTEM, wk->heapID);

  wk->bgTransPos = GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgTransInfo);
  wk->bgTransSize = GFL_ARCUTIL_TRANSINFO_GetSize(wk->bgTransInfo);

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*DEBUG_MVPOKE_MSGPAL, 0x20, wk->heapID);

  wk->win = GFL_BMPWIN_Create( DEBUG_MVPOKE_FRAME,
    WIN_PX, WIN_PY, WIN_SX, WIN_SY, WIN_PAL, GFL_BMP_CHRAREA_GET_B );

  wk->win_bmp = GFL_BMPWIN_GetBmp( wk->win );
  GFL_BMP_Clear( wk->win_bmp, WINCLR_COL(FCOL_WHITE) );
  GFL_BMPWIN_MakeScreen( wk->win );
  BmpWinFrame_Write( wk->win, WINDOW_TRANS_ON, wk->bgTransPos, DEBUG_MVPOKE_WINPAL );
}

/*
 *  @brief  グラフィック解放
 */
static void sub_GraphicRelease( EVENT_DEB_MVPOKE* wk )
{
  GFL_BMPWIN_ClearScreen( wk->win );
  BmpWinFrame_Clear( wk->win, WINDOW_TRANS_OFF );
  GFL_BG_LoadScreenV_Req( DEBUG_MVPOKE_FRAME );
  GFL_BMPWIN_Delete( wk->win );

  GFL_BG_FreeCharacterArea( DEBUG_MVPOKE_FRAME, wk->bgTransPos, wk->bgTransSize );

  GFL_FONT_Delete(wk->font);
  GFL_FONTSYS_SetDefaultColor();
}

/*
 *  @brief  タグ展開
 */
static void sub_RegisterGmmWord( EVENT_DEB_MVPOKE* wk, u8 idx, u8 msg_id )
{
  GFL_MSG_GetString( wk->msg_deb, msg_id, wk->str_buf );
  //WORDSET, タグ位置、バッファ、性別、単複、言語コード
  WORDSET_RegisterWord( wk->wset, idx, wk->str_buf, 0, TRUE, PM_LANG );
}

/*
 *  @brief  ウィンドウ書き込み
 */
static void sub_MvPokeInfoDraw( EVENT_DEB_MVPOKE* wk, u8 idx )
{
	MPD_PTR	 mpd = NULL;
  
  GFL_BMP_Clear( wk->win_bmp, WINCLR_COL(FCOL_WHITE) );

  WORDSET_RegisterPokeMonsNameNo( wk->wset, 0, DATA_DebugMvPokeNo[idx] );

  if( EncDataSave_IsMovePokeValid( wk->enc, idx) == FALSE){
    sub_RegisterGmmWord( wk, 1, d_iwa_mvpoke_st_none+EncDataSave_GetMovePokeState( wk->enc, idx ));
    
    WORDSET_ExpandStr( wk->wset, wk->str_buf, wk->str_poke );
    PRINTSYS_Print( wk->win_bmp, 0, 0, wk->str_buf, wk->font );
    GFL_BMPWIN_TransVramCharacter(wk->win);

    wk->mpd = NULL;
    return;
  }
  sub_RegisterGmmWord( wk, 1, d_iwa_mvpoke_st_move);
  
  WORDSET_ExpandStr( wk->wset, wk->str_buf, wk->str_poke );
  PRINTSYS_Print( wk->win_bmp, 0, 0, wk->str_buf, wk->font );
	  
  mpd = EncDataSave_GetMovePokeDataPtr( wk->enc, idx );
  
  WORDSET_RegisterNumber( wk->wset, 2, EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_HP),
                          3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  sub_RegisterGmmWord( wk, 3, d_iwa_mvpoke_cond00+EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_COND));
  WORDSET_RegisterNumber( wk->wset, 4, EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_PER_RND),
                          10, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber( wk->wset, 5, EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_POW_RND),
                          10, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  
  WORDSET_ExpandStr( wk->wset, wk->str_buf, wk->str_prm );
  PRINTSYS_Print( wk->win_bmp, 0, 16, wk->str_buf, wk->font );

  wk->zone_idx = EncDataSave_GetMovePokeZoneIdx( wk->enc, idx );
  sub_MvPokeZoneDraw( wk, wk->zone_idx, FALSE );

  GFL_BMPWIN_TransVramCharacter(wk->win);
  
  wk->mpd = mpd;
}

/*
 *  @brief  移動ポケモンゾーン書き込み
 */
static void sub_MvPokeZoneDraw( EVENT_DEB_MVPOKE* wk, u8 zone_idx, BOOL trans )
{
  GFL_BMP_Fill( wk->win_bmp, 0,16*4, WIN_SX*8,16, FCOL_WHITE );
 
  if(zone_idx == MVPOKE_ZONE_IDX_HIDE){
		GFL_MSG_GetString( wk->msg_deb, d_iwa_mvpoke_st_hide, wk->str_buf );
  }else{
    u16 zone = MP_MovePokeZoneIdx2ID( zone_idx );
		GFL_MSG_GetString( wk->msg_place, ZONEDATA_GetPlaceNameID( zone ), wk->str_buf );
  }
  PRINTSYS_Print( wk->win_bmp, 0, 16*4, wk->str_buf, wk->font );

  if(trans){
    GFL_BMPWIN_TransVramCharacter(wk->win);
  }
}


#endif  //PM_DEBUG
