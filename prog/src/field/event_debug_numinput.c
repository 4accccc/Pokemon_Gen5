//======================================================================
/**
 *  @file   event_debug_numinput.c
 *  @brief  デバッグ数値入力
 *  @author Miyuki Iwasawa
 *  @date   09.11.25
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "font/font.naix"

#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"

#include "sound/pm_sndsys.h"

#include "field_sound.h"
#include "eventwork.h"

#include "event_debug_local.h"
#include "event_debug_numinput.h"

#include "message.naix"
#include "msg/msg_d_numinput.h"

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode
#include "print/wordset.h"  //WORDSET
#include "msg/msg_d_field.h"

#include "arc/fieldmap/debug_symbol.naix"  //DEBUG_SYM_

#include "arc/fieldmap/debug_list.h"  //DEBUG_SCR_

#include "effect_encount.h"

//======================================================================
//======================================================================
///項目選択用データの定義
typedef enum {
  D_NINPUT_DATA_LIST,    ///<固定データ
  D_NINPUT_DATA_BIN,    ///<自動生成データ
}D_NINPUT_DATA_TYPE;

//数値入力メインイベント制御ワーク
typedef struct _DNINPUT_EV_WORK{
  HEAPID  heapID;

  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldWork;

  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DNINPUT_EV_WORK;

///数値入力リストデータ
typedef struct {
  u16 gmm_id;
  u32 param;
  const DEBUG_NUMINPUT_PARAM * dni_param;
}DEBUG_NUMINPUT_LIST;

///数値入力リスト初期化パラメータ定義
typedef struct{ 
  D_NINPUT_DATA_TYPE data_type;       ///<データの種別

  /** D_NINPUT_DATA_LISTの場合：リストの最大数
   *  D_NINPUT_DATA_BINの場合：生成時に使用する参照ファイルのID
   */
  const u32 count_or_id;               ///<リストの大きさ
  
  /** D_NINPUT_DATA_LISTの場合：各項目の数値入力時処理指定データ
   *  D_NINPUT_DATA_BINの場合：生成時に使用するデフォルトデータ
   */
  const DEBUG_NUMINPUT_LIST * pList;

}DEBUG_NUMINPUT_INITIALIZER;



//======================================================================
//プロトタイプ
//======================================================================
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work);

//--------------------------------------------------------------
///サブメニューイベント：デバッグフラグ操作
//--------------------------------------------------------------
static GMEVENT * DEBUG_EVENT_FLDMENU_FlagWork(
    GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_INITIALIZER * init );

//======================================================================
//データ定義
//======================================================================
#include "debug_numinput.cdat"

static const DEBUG_NUMINPUT_PARAM DNUM_DATA_EventFlag;
static const DEBUG_NUMINPUT_PARAM DNUM_DATA_EventWork;

static const DEBUG_NUMINPUT_LIST listPrototype_flag = {
  //gmm_id, param, dni_param
  0, 0, &DNUM_DATA_EventFlag,
};
static const DEBUG_NUMINPUT_LIST listPrototype_work = {
  //gmm_id, param, dni_param
  0, 0, &DNUM_DATA_EventWork,
};

static const DEBUG_NUMINPUT_INITIALIZER DATA_event_flag = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_event_flag_bin, &listPrototype_flag, };
static const DEBUG_NUMINPUT_INITIALIZER DATA_sys_flag = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_sys_flag_bin, &listPrototype_flag, };
static const DEBUG_NUMINPUT_INITIALIZER DATA_vanish_flag = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_vanish_flag_bin, &listPrototype_flag, };
static const DEBUG_NUMINPUT_INITIALIZER DATA_item_flag = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_item_flag_bin, &listPrototype_flag, };
static const DEBUG_NUMINPUT_INITIALIZER DATA_etc_flag = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_etc_flag_bin, &listPrototype_flag, };

static const DEBUG_NUMINPUT_INITIALIZER DATA_sys_work = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_sys_work_bin, &listPrototype_work, };
static const DEBUG_NUMINPUT_INITIALIZER DATA_scene_work = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_scene_work_bin, &listPrototype_work, };
static const DEBUG_NUMINPUT_INITIALIZER DATA_other_work = {
  D_NINPUT_DATA_BIN, NARC_debug_symbol_other_work_bin, &listPrototype_work, };

static const DEBUG_NUMINPUT_INITIALIZER DATA_eff_enc = { 
  D_NINPUT_DATA_LIST,   NELEMS( DNI_EffectEncountList ), DNI_EffectEncountList, };

static const DEBUG_NUMINPUT_INITIALIZER DATA_Initializer[D_NUMINPUT_MODE_MAX] = { 
  { D_NINPUT_DATA_LIST,   NELEMS( DNI_EffectEncountList ), DNI_EffectEncountList, },
  { D_NINPUT_DATA_LIST,   NELEMS( DNI_EffectEncountList ), DNI_EffectEncountList, },
  { D_NINPUT_DATA_LIST,   NELEMS( DNI_EffectEncountList ), DNI_EffectEncountList, },
};

/// 数値入力　メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DNumInput_MenuFuncHeader =
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

static const FLDMENUFUNC_LIST DATA_DNumInputMenu[] =
{
  { dni_event_flag, (void*)&DATA_event_flag },
  { dni_sys_flag, (void*)&DATA_sys_flag },
  { dni_item_flag, (void*)&DATA_item_flag },
  { dni_vanish_flag, (void*)&DATA_vanish_flag },
  { dni_etc_flag, (void*)&DATA_etc_flag },
  { dni_sys_work, (void*)&DATA_sys_work },
  { dni_scene_work, (void*)&DATA_scene_work },
  { dni_other_work, (void*)&DATA_other_work },
  { dni_top_effect_encount, (void*)&DATA_eff_enc },
  { dni_top_scenario, (void*)NULL },
};

static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer = {
  NARC_message_d_numinput_dat,  //メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu),  //項目数max
  DATA_DNumInputMenu, //メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader, //メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};

static const DEBUG_MENU_INITIALIZER DATA_DNumInput_ListMenuInitializer = {
  NARC_message_d_numinput_dat,  //メッセージアーカイブ
  0,  //項目数max
  NULL, //メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader, //メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};

//======================================================================
//
//関数定義
//
//======================================================================
//--------------------------------------------------------------
/**
 * 数値入力のジャンル選択メニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_NumInput( GAMESYS_WORK* gsys, void* work )
{
  DEBUG_MENU_EVENT_WORK* dme_wk = (DEBUG_MENU_EVENT_WORK*)work;
  DNINPUT_EV_WORK* wk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, dninput_MainEvent, sizeof(DNINPUT_EV_WORK) );

  wk = GMEVENT_GetEventWork( event );
  wk->gsys = gsys; 
  wk->gdata = dme_wk->gdata; 
  wk->fieldWork = dme_wk->fieldWork;
  wk->heapID = dme_wk->heapID;

  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  数値入力メインメニューイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  DNINPUT_EV_WORK* wk = (DNINPUT_EV_WORK*)work;

  switch(*seq)
  {
  case 0:
    wk->menuFunc = DEBUGFLDMENU_Init( wk->fieldWork, wk->heapID, &DATA_DNumInput_MenuInitializer );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){
        break;
      }
      FLDMENUFUNC_DeleteMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){
        return GMEVENT_RES_FINISH;
      }
      if ( ret != NULL ) {
        const DEBUG_NUMINPUT_INITIALIZER *init;
        init = (const DEBUG_NUMINPUT_INITIALIZER *)ret;
        GMEVENT_CallEvent( event, DEBUG_EVENT_FLDMENU_FlagWork( gsys, init ) );
        (*seq)++;
        break;
      }
      return GMEVENT_RES_FINISH;
    }
    break;
  case 2:
    *seq = 0;
    break;

  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//======================================================================
//
//
//    デバッグ数値入力  本体
//
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  _DISP_INITX = 1,
  _DISP_INITY = 18,
  _DISP_SIZEX = 30,
  _DISP_SIZEY = 4,

  PANO_FONT = 14,
  FBMP_COL_WHITE = 15,
};

#define WINCLR_COL(col) (((col)<<4)|(col))
//--------------------------------------------------------------
/**
 * @brief デバッグ数値入力制御ワーク
 */
//--------------------------------------------------------------
typedef struct {
  /// 操作対象指定ID
  u32 select_id;
  /// 操作制御パラメータ
  const DEBUG_NUMINPUT_PARAM * dni_param;

  GAMESYS_WORK * gsys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldmap;
  HEAPID heapID;
  GFL_FONT * fontHandle;

	GFL_MSGDATA* msgman;						//メッセージマネージャー
  WORDSET * wordset;
  GFL_BMPWIN * bmpwin;

  u32 value;
}DEBUG_NUMINPUT_WORK;

//--------------------------------------------------------------
/// 数値入力ウィンドウ：生成
//--------------------------------------------------------------
static void createNumWin( DEBUG_NUMINPUT_WORK * wk )
{
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
  
  wk->wordset = WORDSET_Create( wk->heapID );
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_MESSAGE, NARC_message_d_numinput_dat, wk->heapID);

	bmpwin = GFL_BMPWIN_Create( FLDBG_MFRM_MSG,
		_DISP_INITX, _DISP_INITY, _DISP_SIZEX, _DISP_SIZEY,
		PANO_FONT, GFL_BMP_CHRAREA_GET_B );
  wk->bmpwin = bmpwin;
  
	bmp = GFL_BMPWIN_GetBmp( bmpwin );
	
	GFL_BMP_Clear( bmp, WINCLR_COL(FBMP_COL_WHITE) );
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BMPWIN_TransVramCharacter( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );
}

//--------------------------------------------------------------
/// 数値入力ウィンドウ：削除
//--------------------------------------------------------------
static void deleteNumWin( DEBUG_NUMINPUT_WORK * wk )
{
  GFL_BMPWIN * bmpwin = wk->bmpwin;

	GFL_BMPWIN_ClearScreen( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );

  GFL_BMPWIN_Delete( bmpwin );

  GFL_MSG_Delete( wk->msgman );
  WORDSET_Delete( wk->wordset );
}

//--------------------------------------------------------------
/// 数値入力ウィンドウ：表示更新
//--------------------------------------------------------------
static void printNumWin( DEBUG_NUMINPUT_WORK * wk, u32 num )
{
  GFL_BMPWIN * bmpwin = wk->bmpwin;

  STRBUF * strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
  STRBUF * expandBuf = GFL_STR_CreateBuffer( 64, wk->heapID );

  GFL_MSG_GetString( wk->msgman, dni_number_string, strbuf );
  WORDSET_RegisterNumber(wk->wordset, 0, num,
                         10, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( wk->wordset, expandBuf, strbuf );

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp( bmpwin ), WINCLR_COL(FBMP_COL_WHITE) );

  GFL_FONTSYS_SetColor( 1, 2, 15 );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( bmpwin ), 1, 10, expandBuf, wk->fontHandle );		

  GFL_BMPWIN_TransVramCharacter( bmpwin );  //transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );  //transfer screen

  GFL_STR_DeleteBuffer( strbuf );
  GFL_STR_DeleteBuffer( expandBuf );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void dniSetValue( DEBUG_NUMINPUT_WORK * dni_wk , u32 value )
{
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;
  if (def->set_func == NULL) return;
  def->set_func( dni_wk->gsys, dni_wk->gamedata, dni_wk->select_id, value );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 dniGetValue( DEBUG_NUMINPUT_WORK * dni_wk )
{
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;
  if (def->get_func == NULL) return 0;
  return def->get_func( dni_wk->gsys, dni_wk->gamedata, dni_wk->select_id );
}
//--------------------------------------------------------------
/**
 * @brief デバッグ数値入力制御イベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DebugNumInputEvent( GMEVENT * event , int *seq, void * work )
{
  DEBUG_NUMINPUT_WORK * dni_wk = work;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( dni_wk->gamedata );
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;

  switch (*seq)
  {
  case 0:
    {
      dni_wk->value = dniGetValue( dni_wk );
      createNumWin( dni_wk );
      printNumWin( dni_wk, dni_wk->value );
      (*seq) ++;
      break;
    }
  case 1:
    {
      int trg = GFL_UI_KEY_GetRepeat();
      int cont = GFL_UI_KEY_GetCont();
      int diff;
      u32 before, after;
      if ( trg & PAD_BUTTON_B ) { //キャンセル
        (*seq) ++;
        break;
      }
      if ( trg & PAD_BUTTON_A ) { //決定
        dniSetValue( dni_wk, dni_wk->value );
        (*seq) ++;
        break;
      }
      after = before = dni_wk->value;
      diff = 0;
      if (trg & PAD_KEY_UP){
        diff = 1;
      } else if (trg & PAD_KEY_DOWN) {
        diff = -1;
      } else if (trg & PAD_KEY_LEFT){
        diff = -10;
      } else if (trg & PAD_KEY_RIGHT){
        diff = 10;
      }
      if( cont & PAD_BUTTON_R ){
        diff *= 10;
      }else if( cont & PAD_BUTTON_L ){
        diff *= 100;
      }
      if(diff == 0){
        break;
      }
      if( diff < 0 && ( (after-def->min) < (diff*-1))){
        after = def->max;
      }else if( diff > 0 && ((def->max-after) < diff)){
        after = def->min;
      }else{
        after += diff;
      }

      if (after != before ) {
        printNumWin( dni_wk, after );
        dni_wk->value = after;
      }
    }
    break;
  case 2:
    deleteNumWin( dni_wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief デバッグ数値入力制御イベント生成
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_NumInput(
    GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_PARAM * dni_param, u32 id )
{
  GMEVENT * event;
  DEBUG_NUMINPUT_WORK * wk;
  event = GMEVENT_Create( gsys, NULL, DebugNumInputEvent, sizeof(DEBUG_NUMINPUT_WORK) );
  wk = GMEVENT_GetEventWork( event );

  wk->select_id = id;
  wk->dni_param = dni_param;
  wk->gsys = gsys;
  wk->gamedata = GAMESYSTEM_GetGameData( gsys );
  wk->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  wk->fontHandle = FLDMSGBG_GetFontHandle( FIELDMAP_GetFldMsgBG( wk->fieldmap ) );
  wk->heapID = FIELDMAP_GetHeapID( wk->fieldmap );

  return event;
}

//======================================================================
//
//
//  デバッグ数値入力呼び出し：フラグ操作
//
//
//======================================================================
//--------------------------------------------------------------
//  フラグ操作用Getter/Setter
//--------------------------------------------------------------
static u32 DNUM_Getter_EventFlag( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param )
{
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gamedata );
  return EVENTWORK_CheckEventFlag( evwork , param );
}

static void DNUM_Setter_EventFlag( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value )
{
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gamedata );
  if ( value == 0 ) {
    EVENTWORK_ResetEventFlag( evwork, param );
  } else {
    EVENTWORK_SetEventFlag( evwork, param );
  }
}
static const DEBUG_NUMINPUT_PARAM DNUM_DATA_EventFlag = {
  0,1,
  DNUM_Getter_EventFlag,
  DNUM_Setter_EventFlag,
};

//--------------------------------------------------------------
//  ワーク操作用Getter/Setter
//--------------------------------------------------------------
static u32 DNUM_Getter_EventWork( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param )
{
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gamedata );
  u16 * work = EVENTWORK_GetEventWorkAdrs( evwork, param );
  return *work;
}

static void DNUM_Setter_EventWork( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value )
{
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gamedata );
  u16 * work = EVENTWORK_GetEventWorkAdrs( evwork, param );
  *work = value;
}

static const DEBUG_NUMINPUT_PARAM DNUM_DATA_EventWork = {
  0, 0x3fff,
  DNUM_Getter_EventWork,
  DNUM_Setter_EventWork,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldWork;
  
  DEBUG_MENU_CALLPROC call_proc;
  FLDMENUFUNC *menuFunc;

  const DEBUG_NUMINPUT_INITIALIZER * init;
}DEBUG_FLAGMENU_EVENT_WORK;

//--------------------------------------------------------------
/// メニュー項目最大数取得：デバッグスクリプト
//--------------------------------------------------------------
static u16 DEBUG_GetDebugListMax( GAMESYS_WORK* gsys, void* cb_work )
{
  DEBUG_FLAGMENU_EVENT_WORK * df_work = cb_work;
  return df_work->init->count_or_id;
}

//--------------------------------------------------------------
/// メニュー項目生成：デバッグスクリプト
//--------------------------------------------------------------
static void DEBUG_SetMenuWork_DebugList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  DEBUG_FLAGMENU_EVENT_WORK * df_work = cb_work;
  const DEBUG_NUMINPUT_INITIALIZER * init = df_work->init;
  int id,max = DEBUG_GetDebugListMax(gsys, cb_work);
  GFL_MSGDATA * msgman = GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_numinput_dat, df_work->heapID);
  STRBUF *str = GFL_STR_CreateBuffer( 64, heapID );

  for( id = 0; id < max; id++ ){
    u16 real_id;
    GFL_STR_ClearBuffer( str );
    GFL_MSG_GetString( msgman, init->pList[id].gmm_id, str );
    FLDMENUFUNC_AddStringListData( list, str, id, heapID );
  }
  GFL_HEAP_FreeMemory( str );
  GFL_MSG_Delete( msgman );
}
//--------------------------------------------------------------
/**
 * @brief フィールドデバッグメニュー初期化データ：デバッグフラグ・ワーク操作
 */
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugListSelectData = {
  0,
  0,                                ///<項目最大数（固定リストでない場合、０）
  NULL,                             ///<参照するメニューデータ（生成する場合はNULL)
  &DATA_DebugMenuList_ZoneSel,      ///<メニュー表示指定データ（流用）
  1, 1, 20, 16,
  DEBUG_SetMenuWork_DebugList,    ///<メニュー生成関数へのポインタ
  DEBUG_GetDebugListMax,          ///<項目最大数取得関数へのポインタ
};

//--------------------------------------------------------------
/// メニュー項目最大数取得：デバッグスクリプト
//--------------------------------------------------------------
static u16 DEBUG_GetDebugFlagMax( GAMESYS_WORK* gsys, void* cb_work )
{
  DEBUG_FLAGMENU_EVENT_WORK * df_work = cb_work;
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_DEBUG_SYM, df_work->heapID );
  u16 size = GFL_ARC_GetDataSizeByHandle(p_handle, df_work->init->count_or_id );
  GFL_ARC_CloseDataHandle( p_handle );
  GF_ASSERT( size % DEBUG_SCR_EACH_SIZE == 0 );
  return size / DEBUG_SCR_EACH_SIZE;
}

//--------------------------------------------------------------
/// メニュー項目生成：デバッグスクリプト
//--------------------------------------------------------------
static void DEBUG_SetMenuWork_DebugFlag(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  DEBUG_FLAGMENU_EVENT_WORK * df_work = cb_work;
  int id,max = DEBUG_GetDebugFlagMax(gsys, cb_work);
  
  u8 buffer[DEBUG_SCR_EACH_SIZE];
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_DEBUG_SYM, heapID );
  u16 * utfStr = GFL_HEAP_AllocClearMemory( heapID, DEBUG_SCR_NAME_LEN * sizeof(u16) );
  STRBUF *strBuf = GFL_STR_CreateBuffer( DEBUG_SCR_NAME_LEN + 1, heapID );

  for( id = 0; id < max; id++ ){
    u16 real_id;
    GFL_STR_ClearBuffer( strBuf );
    {
      GFL_ARC_LoadDataOfsByHandle(p_handle, df_work->init->count_or_id,
          id * DEBUG_SCR_EACH_SIZE, DEBUG_SCR_EACH_SIZE, buffer);
      real_id = *((u16*)&buffer[DEBUG_SCR_OFS_ID]);
      DEB_STR_CONV_SjisToStrcode(
          (const char*)(buffer + DEBUG_SCR_OFS_NAME), utfStr, DEBUG_SCR_NAME_LEN );
      //OS_Printf("DEBUG Flag %5d:%s\n", real_id, buffer + DEBUG_SCR_OFS_NAME);
      GFL_STR_SetStringCode( strBuf, utfStr );
    }
    FLDMENUFUNC_AddStringListData( list, strBuf, real_id, heapID );
  }
  GFL_HEAP_FreeMemory( strBuf );
  GFL_HEAP_FreeMemory( utfStr );
  GFL_ARC_CloseDataHandle( p_handle );
}

//--------------------------------------------------------------
/**
 * @brief フィールドデバッグメニュー初期化データ：デバッグフラグ・ワーク操作
 */
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugFlagSelectData = {
  0,
  0,                                ///<項目最大数（固定リストでない場合、０）
  NULL,                             ///<参照するメニューデータ（生成する場合はNULL)
  &DATA_DebugMenuList_ZoneSel,      ///<メニュー表示指定データ（流用）
  1, 1, 20, 16,
  DEBUG_SetMenuWork_DebugFlag,    ///<メニュー生成関数へのポインタ
  DEBUG_GetDebugFlagMax,          ///<項目最大数取得関数へのポインタ
};

//--------------------------------------------------------------
/**
 * イベント：デバッグスクリプト選択
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugFlagWorkSelectMenuEvent(
    GMEVENT *event, int *seq, void *wk )
{
	DEBUG_FLAGMENU_EVENT_WORK * new_wk = wk;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( new_wk->gamedata );
  const DEBUG_NUMINPUT_INITIALIZER * init = new_wk->init;
  GMEVENT * input_ev;
  
  switch( (*seq) ){
  case 0:
    {
      static const DEBUG_MENU_INITIALIZER * menu_init;
      switch ( init->data_type )
      {
      case D_NINPUT_DATA_LIST: 
        menu_init = &DebugListSelectData;
        break;
      case D_NINPUT_DATA_BIN:
        menu_init = &DebugFlagSelectData;
        break;
      default:
        GF_ASSERT(0);
      }
      new_wk->menuFunc = DEBUGFLDMENU_InitEx(
          new_wk->fieldWork, new_wk->heapID,  menu_init, new_wk );
    }
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( new_wk->menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        FLDMENUFUNC_DeleteMenu( new_wk->menuFunc );
        return( GMEVENT_RES_FINISH );
      }
      switch ( init->data_type )
      {
      case D_NINPUT_DATA_LIST: 
        input_ev = DEBUG_EVENT_NumInput(
            new_wk->gmSys, init->pList[ret].dni_param, init->pList[ret].param );
        break;
      case D_NINPUT_DATA_BIN:
        input_ev = DEBUG_EVENT_NumInput( new_wk->gmSys, init->pList->dni_param, ret );
        break;
      default:
        GF_ASSERT(0);
      }
      GMEVENT_CallEvent( event, input_ev );
      break;
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * DEBUG_EVENT_FLDMENU_FlagWork(
    GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_INITIALIZER * init )
{
	GMEVENT * new_event = GMEVENT_Create( gsys, NULL,
      debugFlagWorkSelectMenuEvent, sizeof(DEBUG_FLAGMENU_EVENT_WORK) );
	DEBUG_FLAGMENU_EVENT_WORK * new_wk = GMEVENT_GetEventWork( new_event );

	GFL_STD_MemClear( new_wk, sizeof(DEBUG_FLAGMENU_EVENT_WORK) );
  new_wk->heapID = HEAPID_PROC;
  new_wk->gmSys = gsys;
  new_wk->gamedata = GAMESYSTEM_GetGameData( gsys );
  new_wk->fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  new_wk->call_proc = NULL;
  new_wk->menuFunc = NULL;

  new_wk->init = init;

  return new_event;
}


