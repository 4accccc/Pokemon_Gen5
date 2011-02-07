//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_staff.c
 *	@brief  デバッグメニュースタッフ系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
#include "font/font.naix"
#include "system/medaldata.h" // fro MEDALDATA

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "event_mapchange.h"

//アプリ
#include "debug_wfbc.h"
#include "../../../resource/fldmapdata/script/pasokon_def.h"  //SCRID_PC
#include "savedata/shortcut.h"
#include "event_gtsnego.h"

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 
#include "event_gts.h"
#include "bsubway_scr.h"
#include "event_wifi_bsubway.h"
#include "event_bsubway.h"
#include "scrcmd_bsubway.h"
#include "net_app\wifi_country.h"
#include "savedata\medalbox.h"

//外部公開
#include "event_debug_menu_ov_staff.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * パソコン
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_PCScript( DEBUG_MENU_EVENT_WORK *wk )
{
  SCRIPT_ChangeScript( wk->gmEvent, SCRID_PC, NULL, HEAPID_FIELDMAP );
  return TRUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　Yボタン登録操作
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk )
{
  int i;
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  SHORTCUT *p_shortcut_sv = SaveData_GetShortCut( p_sv );
  for( i = 0; i < SHORTCUT_ID_MAX; i++ )
  {
    SHORTCUT_SetRegister( p_shortcut_sv, i, TRUE );
  }

  return FALSE;
}
//--------------------------------------------------------------
/**
 * GTSNegoメニュー呼びだし
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_OpenGTSNegoMenu( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT *next_event;

  next_event = GMEVENT_CreateOverlayEventCall( gameSys, 
      FS_OVERLAY_ID(event_gtsnego), EVENT_GTSNegoChangeDebug, fieldWork );
  GMEVENT_ChangeEvent( event, next_event );
  return( TRUE );
}
//======================================================================================
//
//  デバッグメニュー　図鑑
//
//======================================================================================
//======================================================================
//======================================================================
typedef enum {
  DEBUG_ZKNCMD_GLOBAL_GET_FULL = 0,
  DEBUG_ZKNCMD_GLOBAL_SEE_FULL,
  DEBUG_ZKNCMD_RANDOM,
  DEBUG_ZKNCMD_FORM_FULL,
  DEBUG_ZKNCMD_ZENKOKU_FLAG,
  DEBUG_ZKNCMD_VERSION_UP,
  DEBUG_ZKNCMD_LOCAL_GET_FULL,
  DEBUG_ZKNCMD_LOCAL_SEE_FULL,   //7
  DEBUG_ZKNCMD_LANGUAGE_FULL,    //8
  DEBUG_ZKNCMD_LOCAL_SEE_25,
  DEBUG_ZKNCMD_LOCAL_SEE_60,
  DEBUG_ZKNCMD_LOCAL_SEE_120,
  DEBUG_ZKNCMD_MANUAL_SET,
}DEBUG_ZKNCMD;

typedef struct {
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  ZUKAN_SAVEDATA * sv;
}DEBUG_ZUKAN_WORK;

typedef struct {
  FIELDMAP_WORK * fieldWork;
  ZUKAN_SAVEDATA * sv;
  HEAPID  heapID;

  GFL_MSGDATA * msgData;
  FLDSYSWIN * win;

  u16 mode;
  u16 pos;
  s16 start;
  s16 end;

}DEBUG_ZUKAN_MANUAL_WORK;

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER ZukanMenuHeader =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

///メニューリスト
static const DEBUG_MENU_LIST ZukanMenuList[] =
{
  { DEBUG_FIELD_ZUKAN_01, (void*)DEBUG_ZKNCMD_GLOBAL_GET_FULL, GFL_OVERLAY_BLANK_ID },  // 全国捕獲コンプ
  { DEBUG_FIELD_ZUKAN_07, (void*)DEBUG_ZKNCMD_GLOBAL_SEE_FULL, GFL_OVERLAY_BLANK_ID },  // 全国見たコンプ
  { DEBUG_FIELD_ZUKAN_10, (void*)DEBUG_ZKNCMD_LOCAL_GET_FULL, GFL_OVERLAY_BLANK_ID },   // 地方捕獲コンプ
  { DEBUG_FIELD_ZUKAN_11, (void*)DEBUG_ZKNCMD_LOCAL_SEE_FULL, GFL_OVERLAY_BLANK_ID },   // 地方見たコンプ
  { DEBUG_FIELD_ZUKAN_09, (void*)DEBUG_ZKNCMD_RANDOM, GFL_OVERLAY_BLANK_ID },           // ランダムセット
  { DEBUG_FIELD_ZUKAN_03, (void*)DEBUG_ZKNCMD_FORM_FULL, GFL_OVERLAY_BLANK_ID },        // フォルムコンプ
  { DEBUG_FIELD_ZUKAN_12, (void*)DEBUG_ZKNCMD_LANGUAGE_FULL, GFL_OVERLAY_BLANK_ID },    // 言語コンプ
  { DEBUG_FIELD_ZUKAN_05, (void*)DEBUG_ZKNCMD_ZENKOKU_FLAG, GFL_OVERLAY_BLANK_ID },     // 全国フラグ
  { DEBUG_FIELD_ZUKAN_06, (void*)DEBUG_ZKNCMD_VERSION_UP, GFL_OVERLAY_BLANK_ID },       // バージョンアップ
  { DEBUG_FIELD_ZUKAN_13, (void*)DEBUG_ZKNCMD_LOCAL_SEE_25, GFL_OVERLAY_BLANK_ID },     // 地方見た２５
  { DEBUG_FIELD_ZUKAN_14, (void*)DEBUG_ZKNCMD_LOCAL_SEE_60, GFL_OVERLAY_BLANK_ID },     // 地方見た６０
  { DEBUG_FIELD_ZUKAN_15, (void*)DEBUG_ZKNCMD_LOCAL_SEE_120, GFL_OVERLAY_BLANK_ID },    // 地方見た１２０
  { DEBUG_FIELD_ZUKAN_16, (void*)DEBUG_ZKNCMD_MANUAL_SET, GFL_OVERLAY_BLANK_ID },       // 登録切り替え
};

static const DEBUG_MENU_INITIALIZER DebugMenuZukanImitializer = {
  NARC_debug_message_d_field_dat,
  NELEMS(ZukanMenuList),
  ZukanMenuList,
  &ZukanMenuHeader,
  1, 1, 16, 16,
  NULL,
  NULL
};

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZukanEvent( GMEVENT *event, int *seq, void *wk );
static void SetZukanDataOne( DEBUG_ZUKAN_WORK * wk, u16 mons, u16 form, u16 lang, u32 mode );
static void SetZukanLocal( DEBUG_ZUKAN_WORK * wk, u16 count, u32 mode );
static void SetZukanDataAll( DEBUG_ZUKAN_WORK * wk );
static GMEVENT_RESULT ZukanManualSet( GMEVENT *event, int *seq, void *wk );


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　図鑑
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_Zukan( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_ZUKAN_WORK *work;

  GMEVENT_Change( event, debugMenuZukanEvent, sizeof(DEBUG_ZUKAN_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_ZUKAN_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->sv = GAMEDATA_GetZukanSave( GAMESYSTEM_GetGameData(wk->gmSys) );

  return TRUE;
}

//--------------------------------------------------------------
/**
 * イベント：図鑑
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZukanEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZUKAN_WORK *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuZukanImitializer );
    (*seq)++;
    break;

  case 1:

    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){
    case FLDMENUFUNC_NULL:    // 操作無し
      break;

    case FLDMENUFUNC_CANCEL:  // キャンセル
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_GLOBAL_GET_FULL:     // 全国捕獲
      {
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          SetZukanDataOne( wk, i, 0, PM_LANG, 0 );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_GLOBAL_SEE_FULL:     // 全国見た
      {
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          SetZukanDataOne( wk, i, 0, PM_LANG, 1 );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_RANDOM:     // ランダム
      {
        u32 rand;
        u32 max;
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          rand = GFL_STD_MtRand( 3 );
          if( rand == 2 ){ continue; }
          max = ZUKANSAVE_GetFormMax( i );
          SetZukanDataOne( wk, i, GFL_STD_MtRand(max), PM_LANG, rand );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_FORM_FULL:     // フォルム
      SetZukanDataAll( wk );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_ZENKOKU_FLAG:     // 全国フラグ
      ZUKANSAVE_SetZenkokuZukanFlag( work->sv );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_VERSION_UP:     // バージョンアップ
      ZUKANSAVE_SetGraphicVersionUpFlag( work->sv );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_GET_FULL:     // 地方捕獲
      SetZukanLocal( wk, MONSNO_END, 0 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_FULL:     // 地方見た
      SetZukanLocal( wk, MONSNO_END, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_25:
      SetZukanLocal( wk, 25, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_60:
      SetZukanLocal( wk, 60, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_120:
      SetZukanLocal( wk, 120, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LANGUAGE_FULL:     // 言語
      {
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          SetZukanDataOne( wk, i, 0, LANG_JAPAN, 0 );
          SetZukanDataOne( wk, i, 0, LANG_ENGLISH, 0 );
          SetZukanDataOne( wk, i, 0, LANG_FRANCE, 0 );
          SetZukanDataOne( wk, i, 0, LANG_ITALY, 0 );
          SetZukanDataOne( wk, i, 0, LANG_GERMANY, 0 );
          SetZukanDataOne( wk, i, 0, LANG_SPAIN, 0 );
          SetZukanDataOne( wk, i, 0, LANG_KOREA, 0 );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_MANUAL_SET:       // 登録切り替え
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      {
        FIELDMAP_WORK * fieldWork = work->fieldWork;
        ZUKAN_SAVEDATA * sv = work->sv;
        HEAPID  heapID = work->heapID;
        DEBUG_ZUKAN_MANUAL_WORK * mwk;

        GMEVENT_Change( event, ZukanManualSet, sizeof(DEBUG_ZUKAN_MANUAL_WORK) );

        mwk = GMEVENT_GetEventWork( event );
        mwk->sv = sv;
        mwk->fieldWork = fieldWork;
        mwk->heapID = heapID;
      }
      return GMEVENT_RES_CONTINUE;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

static void SetZukanLocal( DEBUG_ZUKAN_WORK * wk, u16 count, u32 mode )
{
  u16 * buf;
  u32 i;
  buf = POKE_PERSONAL_GetZenkokuToChihouArray( wk->heapID, NULL );
  for( i=1; i<=MONSNO_END; i++ ){
    if( buf[i] != POKEPER_CHIHOU_NO_NONE ){
      SetZukanDataOne( wk, i, 0, PM_LANG, mode );
      count --;
    }
    if ( count == 0 ) break;
  }
  GFL_HEAP_FreeMemory( buf );
}

static void SetZukanDataOne( DEBUG_ZUKAN_WORK * wk, u16 mons, u16 form, u16 lang, u32 mode )
{
  POKEMON_PARAM * pp = PP_Create( mons, 50, 0, wk->heapID );
  PP_Put( pp, ID_PARA_form_no, form );
  PP_Put( pp, ID_PARA_country_code, lang );

  if( mode == 0 ){
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
  }else{
    ZUKANSAVE_SetPokeSee( wk->sv, pp );
  }

  GFL_HEAP_FreeMemory( pp );
}


static void SetZukanDataAllOne( DEBUG_ZUKAN_WORK * wk, POKEMON_PARAM * pp, u32 sex, BOOL rare, BOOL first )
{
  u32 mons;
  u32 id;
  u32 rand;
  u32 form_max;
  u32 i;

  mons = PP_Get( pp, ID_PARA_monsno, NULL );
  id   = PP_Get( pp, ID_PARA_id_no, NULL );
  rand = POKETOOL_CalcPersonalRandEx( id, mons, 0, sex, 0, rare );
  PP_SetupEx( pp, mons, 50, id, PTL_SETUP_POW_AUTO, rand );

  form_max = ZUKANSAVE_GetFormMax( mons );

  if( first == TRUE ){
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
  }else{
    ZUKANSAVE_SetPokeSee( wk->sv, pp );
  }

  for( i=1; i<form_max; i++ ){
    rand = POKETOOL_CalcPersonalRandEx( id, mons, i, sex, 0, rare );
    PP_Put( pp, ID_PARA_personal_rnd, rand );
    PP_Put( pp, ID_PARA_form_no, i );
    ZUKANSAVE_SetPokeSee( wk->sv, pp );
  }
}

static void SetZukanDataAll( DEBUG_ZUKAN_WORK * wk )
{
  POKEMON_PERSONAL_DATA * ppd;
  POKEMON_PARAM * pp;
  BOOL fast;
  u32 sex_vec;
  u32 rand;
  u16 i, j;

  for( i=1; i<=MONSNO_END; i++ ){
    // 性別ベクトル取得
    ppd = POKE_PERSONAL_OpenHandle( i, 0, wk->heapID );
    sex_vec = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
    POKE_PERSONAL_CloseHandle( ppd );
    // POKEMON_PARAM作成
    pp = PP_Create( i, 50, 0, wk->heapID );
    fast = PP_FastModeOn( pp );
    PP_Put( pp, ID_PARA_form_no, 0 );
    PP_Put( pp, ID_PARA_country_code, LANG_JAPAN );

    // 性別なし
    if( sex_vec == POKEPER_SEX_UNKNOWN ){
      SetZukanDataAllOne( wk, pp, PTL_SEX_UNKNOWN, FALSE, TRUE );   // レアじゃない
      SetZukanDataAllOne( wk, pp, PTL_SEX_UNKNOWN, TRUE, FALSE );   // レア
      // 性別あり
    }else{
      // ♂もいる
      if( sex_vec != POKEPER_SEX_FEMALE ){
        //        PP_Put( pp, ID_PARA_sex, PTL_SEX_MALE );
        SetZukanDataAllOne( wk, pp, PTL_SEX_MALE, FALSE, TRUE );    // レアじゃない
        SetZukanDataAllOne( wk, pp, PTL_SEX_MALE, TRUE, FALSE );    // レア
      }
      // ♀もいる
      if( sex_vec != POKEPER_SEX_MALE ){
        //        PP_Put( pp, ID_PARA_sex, PTL_SEX_FEMALE );
        SetZukanDataAllOne( wk, pp, PTL_SEX_FEMALE, FALSE, TRUE );  // レアじゃない
        SetZukanDataAllOne( wk, pp, PTL_SEX_FEMALE, TRUE, FALSE );  // レア
      }
    }

    // 各言語セット
    PP_Put( pp, ID_PARA_country_code, LANG_ENGLISH );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_FRANCE );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_ITALY );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_GERMANY );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_SPAIN );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_KOREA );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );

    PP_FastModeOff( pp, fast );
    GFL_HEAP_FreeMemory( pp );

    OS_Printf( "No.%d : 図鑑追加\n", i );
  }
}

static void DZM_ModePut( DEBUG_ZUKAN_MANUAL_WORK * wk )
{
  FLDSYSWIN_FillClearWindow( wk->win, 0, 40, 22*8, 16 );

  if( wk->mode == 0 ){
    FLDSYSWIN_Print( wk->win, 0, 40, DEBUG_FIELD_ZUKAN_17 );
  }else if( wk->mode == 1 ){
    FLDSYSWIN_Print( wk->win, 0, 40, DEBUG_FIELD_ZUKAN_18 );
  }else{
    FLDSYSWIN_Print( wk->win, 0, 40, DEBUG_FIELD_ZUKAN_19 );
  }
}

static void DZM_MonsPut( DEBUG_ZUKAN_MANUAL_WORK * wk, u32 pos )
{
  STRBUF * str;
  STRBUF * exp;
  WORDSET * wset;

  FLDSYSWIN_FillClearWindow( wk->win, 0, 56+pos*16, 22*8, 16 );

  wset = WORDSET_Create( wk->heapID );
  exp  = GFL_STR_CreateBuffer( 128, wk->heapID );

  if( pos == 0 ){
    str = GFL_MSG_CreateString( wk->msgData, DEBUG_FIELD_ZUKAN_20 );
    WORDSET_RegisterNumber( wset, 0, wk->start, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterPokeMonsNameNo( wset, 1, wk->start );
  }else{
    str = GFL_MSG_CreateString( wk->msgData, DEBUG_FIELD_ZUKAN_21 );
    WORDSET_RegisterNumber( wset, 0, wk->end, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterPokeMonsNameNo( wset, 1, wk->end );
  }

  WORDSET_ExpandStr( wset, exp, str );

  if( wk->pos == pos ){
    FLDSYSWIN_PrintStrBufColor( wk->win, 0, 56+pos*16, exp, PRINTSYS_LSB_Make(3,4,15) );
  }else{
    FLDSYSWIN_PrintStrBufColor( wk->win, 0, 56+pos*16, exp, PRINTSYS_LSB_Make(1,2,15) );
  }

  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( exp );
  WORDSET_Delete( wset );
}

static void ZukanManualCountUp( DEBUG_ZUKAN_MANUAL_WORK * wk, s16 val )
{
  s16 * dat;

  if( wk->pos == 0 ){
    dat = &wk->start;
  }else{
    dat = &wk->end;
  }

  *dat += val;

  if( *dat > MONSNO_END ){
    *dat = *dat - MONSNO_END;
  }else if( *dat < 1 ){
    *dat = *dat + MONSNO_END;
  }

  DZM_MonsPut( wk, wk->pos );
}

static void DZM_DataMake( DEBUG_ZUKAN_MANUAL_WORK * wk )
{
  u16 start, end;

  if( wk->start > wk->end ){
    start = wk->end;
    end   = wk->start;
  }else{
    start = wk->start;
    end   = wk->end;
  }

  // みた
  if( wk->mode == 0 ){
    ZUKANSAVE_DebugDataSetSee( wk->sv, start, end, wk->heapID );
    // ほかく
  }else if( wk->mode == 1 ){
    ZUKANSAVE_DebugDataSetGet( wk->sv, start, end, wk->heapID );
    // 範囲クリア
  }else{
    ZUKANSAVE_DebugDataClear( wk->sv, start, end );
  }
}

static GMEVENT_RESULT ZukanManualSet( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZUKAN_MANUAL_WORK * work = wk;

  switch( (*seq) ){
  case 0:
    work->mode  = 0;
    work->pos   = 0;
    work->start = 1;
    work->end   = 1;
    {
      FLDMSGBG * msgBG;
      msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA(
          work->fieldWork, NARC_debug_message_d_field_dat );
      work->win = FLDSYSWIN_AddEx( msgBG, work->msgData, 1, 1, 22, 11 );

      FLDSYSWIN_Print( work->win, 0, 0, DEBUG_FIELD_ZUKAN_22 );
      DZM_ModePut( work );
      DZM_MonsPut( work, 0 );
      DZM_MonsPut( work, 1 );
    }
    (*seq)++;
    break;

  case 1:
    // モード切り替え
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      work->mode += 1;
      if( work->mode >= 3 ){
        work->mode = 0;
      }
      DZM_ModePut( work );
      break;
    }
    // 反映
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
      DZM_DataMake( work );
      PMSND_PlaySE( SEQ_SE_SAVE );
      break;
    }
    // 決定
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      work->pos ^= 1;
      DZM_MonsPut( work, 0 );
      DZM_MonsPut( work, 1 );
      break;
    }
    // 終了
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
      FLDSYSWIN_ClearWindow( work->win );
      FLDSYSWIN_Delete( work->win );
      GFL_MSG_Delete( work->msgData );
      return GMEVENT_RES_FINISH;
    }
    // +1
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
      ZukanManualCountUp( work, 1 );
      break;
    }
    // -1
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
      ZukanManualCountUp( work, -1 );
      break;
    }
    // +10
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
      ZukanManualCountUp( work, 10 );
      break;
    }
    // -10
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
      ZukanManualCountUp( work, -10 );
      break;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 着信音
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_RingTone( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GAMESYS_WORK * gsys = p_wk->gmSys;
  GAMEDATA* gamedata  = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );

  if ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE3 ) == FALSE ) {
    FSND_RequestTVTRingTone( fsnd );
  } else {
    FSND_StopTVTRingTone( fsnd );
  }
  return FALSE;
}
//-----------------------------------------------------------------------------
/**
 *    WFBC自動チェック
 */
//-----------------------------------------------------------------------------
//-------------------------------------
/// イベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK* p_gsys;
  FIELDMAP_WORK* p_fieldmap;
} EV_DEBUGMENU_WFBC;


static GMEVENT_RESULT EV_debugMenu_WfbcDebug( GMEVENT *event, int *seq, void *wk );
//----------------------------------------------------------------------------
/**
 *  @brief  WFBCオートデバック
 */
//-----------------------------------------------------------------------------
BOOL debugMenu_WfbcDebug( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  EV_DEBUGMENU_WFBC *work;

  GMEVENT_Change( event, EV_debugMenu_WfbcDebug, sizeof(EV_DEBUGMENU_WFBC) );

  work = GMEVENT_GetEventWork( event );

  work->p_gsys = gsys;
  work->p_fieldmap = fieldWork;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  自動動作
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EV_debugMenu_WfbcDebug( GMEVENT *event, int *seq, void *wk )
{
  EV_DEBUGMENU_WFBC *work = wk;


  switch( *seq ){
  case 0:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_menu_wfbc_check) );
#ifdef DEBUG_ONLY_FOR_tomoya_takahashi
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
      GMEVENT_CallEvent( event, DEBUG_WFBC_PeopleCheck( work->p_gsys ) );
    }
    else
#endif //DEBUG_ONLY_FOR_tomoya_takahashi
    {
      GMEVENT_CallEvent( event, DEBUG_WFBC_View( work->p_gsys ) );
    }
    (*seq)++;
    break;

  case 1:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_menu_wfbc_check) );
    return ( GMEVENT_RES_FINISH );
  }

  return ( GMEVENT_RES_CONTINUE );
}


static void ichi_coutry_chekNG(u32 country_code, u32 local_code, u32 language)
{
    BOOL ret;
    ret = WIFI_COUNTRY_CheckNG( country_code, local_code, language );
    OS_Printf("ret %d ", ret);
    OS_Printf("coutry %d ", country_code);
    OS_Printf("local %d ", local_code);
    OS_Printf("language %d\n", language);
    return;
}

BOOL debugMenuCallProc_DebugTest_Ichi( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldMap = wk->fieldWork;
  u16 zone_id = FIELDMAP_GetZoneID( fieldMap );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( wk->gdata );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( wk->gdata );
  MYSTATUS *s_mystatus = SaveData_GetMyStatus( save );

  // BUGFIX_COUNTRY_NGCHECK_20100806
  // 自分でデータをいろいろ送って関数の単体テストをする
  {
    u32 country_code;
    u32 local_code;
    u32 language;

    country_code  = MyStatus_GetMyNation(mystatus);
    local_code    = MyStatus_GetMyArea(mystatus);
    language      = MyStatus_GetRegionCode(mystatus);

    ichi_coutry_chekNG( 0, 0, language );
    ichi_coutry_chekNG( country_code, local_code, language );
    ichi_coutry_chekNG( country_code, local_code, LANG_ENGLISH );
    ichi_coutry_chekNG( country_code, 0, language );
    ichi_coutry_chekNG( 138, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 137, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 186, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 185, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 138, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 1, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 2, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 51, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 52, LANG_ENGLISH );
    ichi_coutry_chekNG( 227, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 228, 0, LANG_ENGLISH );
  }
  {
    GMEVENT * mapchange_event;
    mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( gsys, fieldMap, zone_id );
    GMEVENT_ChangeEvent( event, mapchange_event );
  }
  return( TRUE );

}
//----------------------------------------------------------------------------
/**
 *	@brief  尾上用テスト
 *
 *	@param	DEBUG_MENU_EVENT_WORK *wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugTest_Onoue( DEBUG_MENU_EVENT_WORK *wk )
{
  int sex;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldMap = wk->fieldWork;
  u16 zone_id = FIELDMAP_GetZoneID( fieldMap );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( wk->gdata );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( wk->gdata );
  MYSTATUS *s_mystatus = SaveData_GetMyStatus( save );

  // 性別をひっくり返す
  sex = MyStatus_GetMySex( mystatus );
  sex ^= 1;
  MyStatus_SetMySex( mystatus, sex );
  MyStatus_SetMySex( s_mystatus, sex );

  // ユニオン見た目もひっくり返す
  MyStatus_SetTrainerView( mystatus,   sex*MYSTATUS_UNIONVIEW_MAX );
  MyStatus_SetTrainerView( s_mystatus, sex*MYSTATUS_UNIONVIEW_MAX );

  {
    GMEVENT * mapchange_event;
    mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( gsys, fieldMap, zone_id );
    GMEVENT_ChangeEvent( event, mapchange_event );
  }

  return( TRUE );

}

// ===========================================================================
//
//    メダルボックスデバッグメニュー
//
// ===========================================================================

//-------------------------------------
/// メダルボックスデバッグワーク	
//=====================================
typedef struct {
  int seq_no;
  int sub_seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  GFL_ARCUTIL_TRANSINFO bgchar;
  FLDSYSWIN * win;
  GFL_MSGDATA * msgData;
  MedalID       draw_medal_id;
} DEBUG_MEDALBOX_WORK;

///メニューリスト
static const DEBUG_MENU_LIST MedalBoxMenuList[] =
{
  { DEBUG_FIELD_MEDALBOX_MENU_MAKEDATA , (void*)DEBUG_FIELD_MEDALBOX_MENU_MAKEDATA , GFL_OVERLAY_BLANK_ID },  // ボックスをランクアップ
  { DEBUG_FIELD_MEDALBOX_MENU_ALL_TITLE_OPEN , (void*)DEBUG_FIELD_MEDALBOX_MENU_ALL_TITLE_OPEN , GFL_OVERLAY_BLANK_ID },  // メダルボックスを開く
  { DEBUG_FIELD_MEDALBOX_MENU_ALL_HINT_OPEN , (void*)DEBUG_FIELD_MEDALBOX_MENU_ALL_HINT_OPEN , GFL_OVERLAY_BLANK_ID },  // メダルボックスを開く
  { DEBUG_FIELD_MEDALBOX_MENU_ALL_TITLE_ACHIVE , (void*)DEBUG_FIELD_MEDALBOX_MENU_ALL_TITLE_ACHIVE , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
  { DEBUG_FIELD_MEDALBOX_MENU_ALL_HINT_ACHIVE , (void*)DEBUG_FIELD_MEDALBOX_MENU_ALL_HINT_ACHIVE , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
  { DEBUG_FIELD_MEDALBOX_MENU_RANDOMSET , (void*)DEBUG_FIELD_MEDALBOX_MENU_RANDOMSET , GFL_OVERLAY_BLANK_ID },  // ボックスをランクアップ
  { DEBUG_FIELD_MEDALBOX_MENU_OUTSAVE , (void*)DEBUG_FIELD_MEDALBOX_MENU_OUTSAVE , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
  { DEBUG_FIELD_MEDALBOX_MENU_BOXRANK , (void*)DEBUG_FIELD_MEDALBOX_MENU_BOXRANK , GFL_OVERLAY_BLANK_ID },  // メダルデータをランダム出力
  { DEBUG_FIELD_MEDALBOX_MENU_OUTDATA , (void*)DEBUG_FIELD_MEDALBOX_MENU_OUTDATA , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
};

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER MedalBoxMenuHeader =
{
  7,    //リスト項目数
  8,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

//-------------------------------------
///   メダルボックス初期化変数
//=====================================
static const DEBUG_MENU_INITIALIZER DebugMenuMedalBoxImitializer = {
  NARC_debug_message_d_field_dat,
  NELEMS(MedalBoxMenuList),
  MedalBoxMenuList,
  &MedalBoxMenuHeader,
  1, 1, 30, 16,
  NULL,
  NULL
};

// =======================
// プロトタイプ宣言
static GMEVENT_RESULT debugMenuMedalBoxEvent( GMEVENT *event, int *seq, void *wk );

//----------------------------------------------------------------------------
/**
 *	@brief  全タイトルメダル開放
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuAllTitleMedalOpen( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    // 設定前のメダルIDと開放状況を表示
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

    if( medal_state < MEDAL_STATE_TITLE_GET )
    {
      OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

      // タイトルメダルを開放
      
      // ヒントメダルを開放
      if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
      {
        MEDALBOX_SetTitleMedalAchive( medal_box , medal_id );
      }
      MEDALBOX_SetTitleMedalGetFlg( medal_box , medal_id , 99 , 12 , 31 );

      // 設定後の開放状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
      OS_Printf(" %u\n",medal_state);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  全ヒントメダル開放
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuAllHintMedalOpen( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , wk->heapID );
    MEDALDATA medal_data;
    GFL_ARC_LoadDataByHandle( p_handle , medal_id ,&medal_data );
    GFL_ARC_CloseDataHandle(p_handle);
    if( medal_data.hint_type != 0 )
    {
      // 設定前のメダルIDと開放状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

      if( medal_state < MEDAL_STATE_HINT_GET )
      {
        OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

        // ヒントメダルを開放
        if( medal_state < MEDAL_STATE_HINT_ACHIVE )
        {
          MEDALBOX_SetHintMedalAchive( medal_box , medal_id );
        }
        MEDALBOX_SetHintMedalGetFlg( medal_box , medal_id , GFL_STD_MtRand( 99 ) + 1 , GFL_STD_MtRand( 12 ) + 1 , GFL_STD_MtRand( 31 ) + 1 );

        // 設定後の開放状況を表示
        medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
        OS_Printf(" %u\n",medal_state);
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメダル全達成
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuTitleMedalAchive( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    // 設定前のタイトルメダルIDと達成状況を表示
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
    
    if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
    {
      OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

      // タイトルメダルを達成
      MEDALBOX_SetTitleMedalAchive( medal_box , medal_id );

      // 設定後のタイトル達成状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
      OS_Printf(" %u\n",medal_state);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメダル全達成
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuHintMedalAchive( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    // 設定前のヒントメダルIDと達成状況を表示
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

    if( medal_state < MEDAL_STATE_HINT_ACHIVE )
    { 
      OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

      // ヒントメダルを達成
      MEDALBOX_SetHintMedalAchive( medal_box , medal_id );

      // 設定後のヒント達成状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
      OS_Printf(" %u\n",medal_state);
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータをデバッグプリント
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalDataOutPut( DEBUG_MEDALBOX_WORK* wk )
{
  int i =0;
  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , wk->heapID );

  for( i = 0; i < MDLID_MAX ; i++ )
  {
    MedalID medal_id =i;
    MEDALDATA medal_data;

    // アーカイブからデータを取得
    GFL_ARC_LoadDataByHandle( p_handle , medal_id ,&medal_data );
    OS_Printf( "=======================================================\n");
    OS_Printf( "メダルID %u\n",medal_id);
    OS_Printf( "target_count : %u\n" ,medal_data.target_count );
    OS_Printf( "cmp_type : %u\n" ,medal_data.cmp_type );
    OS_Printf( "cmp_id : %u\n" ,medal_data.cmp_id );
    OS_Printf( "graphic_type : %u\n" ,medal_data.graphic_type );
    OS_Printf( "hint_type : %u\n" ,medal_data.hint_type );
    OS_Printf( "hint_level : %u\n" ,medal_data.hint_level );
  }
  GFL_ARC_CloseDataHandle(p_handle);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスをランクアップ
 *
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalBoxRankUp( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );

  // メダルボックスをランクアップさせる
  MEDALBOX_RankUpMedalBox( medal_box );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルセーブをデバッグプリント
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalSaveOutPut( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    u8 y,m,d;
    u8 medal_state = 0;
    OS_Printf("MEDALID %d---------------------------------\n",medal_id);
    
    medal_state =  MEDALBOX_GetMedalState( medal_box , medal_id );
    OS_Printf("メダル状態     : ");
    // メダル状態
    switch( medal_state )
    {
    case MEDAL_STATE_NONE:
      OS_Printf("未取得\n");
      break;
    case MEDAL_STATE_HINT_ACHIVE:
      OS_Printf("ヒント達成\n");
      break;
    case MEDAL_STATE_HINT_GET:
      OS_Printf("ヒント取得\n");
      break;
    case MEDAL_STATE_TITLE_ACHIVE:
      OS_Printf("タイトル達成\n");
      break;
    case MEDAL_STATE_TITLE_GET:
      OS_Printf("タイトル取得\n");
      break;
    default:
      GF_ASSERT(0);
      break;
    }
    OS_Printf("新着           : %u\n", MEDALBOX_GetNewMedalFlg( medal_box , medal_id ) );
    MEDALBOX_GetDate( medal_box , medal_id , &y , &m , &d );
    OS_Printf("年月日         : 20%u年 %2u月 %u日\n",y,m,d);

  }
  OS_Printf("ボックスランク : %u\n", MEDALBOX_GetMedalBoxRank( medal_box) );
  OS_Printf("イチオシメダル : %u\n", MEDALBOX_GetPushMedalID( medal_box) );

}

static void DrawMedalData( DEBUG_MEDALBOX_WORK* work , MedalID medal_id )
{
  STRBUF *str;
  STRBUF *exp;
  FLDMSGBG * msgBG;
  WORDSET * wset;
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( work->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state = MEDALBOX_GetMedalState( medal_box , work->draw_medal_id );

  str = GFL_MSG_CreateString( work->msgData, DEBUG_FIELD_MEDALBOX_MAKEDATA );
  wset = WORDSET_Create( work->heapID );
  exp  = GFL_STR_CreateBuffer( 128, work->heapID );
  WORDSET_RegisterMedalName( wset, 0,medal_id );
  WORDSET_RegisterNumber( wset, 1, medal_state, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 2, medal_id, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( wset, exp, str );

  msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
  FLDSYSWIN_ClearWindow( work->win );
  FLDSYSWIN_PrintStrBuf( work->win, 0, 0, exp );

  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( exp );
  WORDSET_Delete( wset );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータ作成
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuMedalDataMake( DEBUG_MEDALBOX_WORK* work )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( work->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );

  switch( work->sub_seq_no )
  {
  case 0:
    {
      FLDMSGBG * msgBG;
      work->draw_medal_id = 0;
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA(work->fieldWork, NARC_debug_message_d_field_dat );
      msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->win = FLDSYSWIN_AddEx( msgBG, work->msgData , 1, 1, 30, 2 );
      DrawMedalData( work , work->draw_medal_id );
      work->sub_seq_no++;
    }
    break;
  case 1:
    {
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
      {
        work->draw_medal_id--;
        if( work->draw_medal_id < 0 )
        {
          work->draw_medal_id = MDLID_MAX-1;
        }
        DrawMedalData( work , work->draw_medal_id );
      }

      if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
      {
        work->draw_medal_id++;
        if( work->draw_medal_id >= MDLID_MAX )
        {
          work->draw_medal_id = 0;
        }
        DrawMedalData( work , work->draw_medal_id );
      }

      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        u8 medal_state = MEDALBOX_GetMedalState( medal_box , work->draw_medal_id );
        medal_state = medal_state + 1;

        if( medal_state >= MEDAL_STATE_MAX )
        {
          medal_state = 0;
        }
        DEBUG_SetMedalState( medal_box , work->draw_medal_id , medal_state );
        DrawMedalData( work , work->draw_medal_id );
      }
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
      {
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief      メダルデータのランダムセット
 *
 *	@param	wk  ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalDataRandomSet( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    u32 rand;

    // 乱数の取得
    rand = GFL_STD_MtRand( 5 );
    
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

    // 設定されていないもののみランダムにせっていする
    if( medal_state != MEDAL_STATE_NONE )
    {
      continue;
    }

    switch( rand )
    {
    case 0:
      {

        // 設定前のメダルIDと開放状況を表示
        OS_Printf("タイトル開放 %d: %u ->",medal_id , medal_state);

        // タイトルメダルを開放
        MEDALBOX_SetTitleMedalAchive( medal_box , medal_id );
        MEDALBOX_SetTitleMedalGetFlg( medal_box , medal_id , GFL_STD_MtRand( 99 ) + 1 , GFL_STD_MtRand( 12 ) + 1 , GFL_STD_MtRand( 31 ) + 1  );

        // 設定後の開放状況を表示
        medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
        OS_Printf(" %u\n",medal_state);
      }
      break;
    case 1:
      {
        ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , wk->heapID );
        MEDALDATA medal_data;
        GFL_ARC_LoadDataByHandle( p_handle , medal_id ,&medal_data );
        GFL_ARC_CloseDataHandle(p_handle);
        if( medal_data.hint_type != 0 )
        {
          // 設定前のメダルIDと開放状況を表示
          OS_Printf("ヒント開放   %d: %u ->",medal_id , medal_state);

          // ヒントメダルを開放
          MEDALBOX_SetHintMedalAchive( medal_box , medal_id );
          MEDALBOX_SetHintMedalGetFlg( medal_box , medal_id , GFL_STD_MtRand( 99 ) + 1 , GFL_STD_MtRand( 12 ) + 1 , GFL_STD_MtRand( 31 ) + 1 );

          // 設定後の開放状況を表示
          medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
          OS_Printf(" %u\n",medal_state);
        }
      }
      break;
    default:
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスメニュー起動
 *
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_MedalBox(DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_MEDALBOX_WORK *work;

  GMEVENT_Change( event, debugMenuMedalBoxEvent, sizeof(DEBUG_MEDALBOX_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MEDALBOX_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  return TRUE;
}

//--------------------------------------------------------------
/**
 * イベント：メダルボックス
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMedalBoxEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MEDALBOX_WORK *work = wk;

  switch( *seq ){

  // メニュー初期化
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuMedalBoxImitializer );
    work->sub_seq_no=0;
    (*seq)++;
    break;

  // メニュー選択処理
  case 1:
    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){

    // 操作無し
    case FLDMENUFUNC_NULL:
      break;

    // キャンセル
    case FLDMENUFUNC_CANCEL:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // タイトル全部開放
    case DEBUG_FIELD_MEDALBOX_MENU_ALL_TITLE_OPEN:
      {
        debugMenuAllTitleMedalOpen( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // ヒント全開放
    case DEBUG_FIELD_MEDALBOX_MENU_ALL_HINT_OPEN:
      {
        debugMenuAllHintMedalOpen( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // タイトルメダル全達成
    case DEBUG_FIELD_MEDALBOX_MENU_ALL_TITLE_ACHIVE:
      {
        debugMenuTitleMedalAchive( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // ヒントメダル全達成
    case DEBUG_FIELD_MEDALBOX_MENU_ALL_HINT_ACHIVE:
      {
        debugMenuHintMedalAchive( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルセーブデータをデバッグ出力
    case DEBUG_FIELD_MEDALBOX_MENU_OUTSAVE:
      {
        debugMenuMedalSaveOutPut( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルデータをデバッグ出力
    case DEBUG_FIELD_MEDALBOX_MENU_OUTDATA:     
      {
        debugMenuMedalDataOutPut( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // ボックスをランクアップ
    case DEBUG_FIELD_MEDALBOX_MENU_BOXRANK:     
      {
        debugMenuMedalBoxRankUp( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルセーブをランダム生成
    case DEBUG_FIELD_MEDALBOX_MENU_RANDOMSET:     
      {
        debugMenuMedalDataRandomSet( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルセーブをランダム生成
    case DEBUG_FIELD_MEDALBOX_MENU_MAKEDATA:     
      {
        (*seq)++;
      }
      return GMEVENT_RES_CONTINUE;
    }
    break;
  case 2:
    {
      BOOL end = debugMenuMedalDataMake( work );
      if( end == TRUE )
      {
        FLDSYSWIN_ClearWindow( work->win );
        FLDSYSWIN_Delete( work->win );
        GFL_MSG_Delete( work->msgData );
        return GMEVENT_RES_FINISH;
      }
    }
    return GMEVENT_RES_CONTINUE;

  }

  return GMEVENT_RES_CONTINUE;
}

