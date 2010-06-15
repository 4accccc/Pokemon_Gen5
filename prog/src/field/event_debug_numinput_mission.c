#ifdef PM_DEBUG
////////////////////////////////////////////////////////////////////////
/**
 *  @brief  デバッグ機能：ミッション選択
 *  @file   event_debug_numinput_mission.c
 *  @author obata
 *  @date   2010.06.14
 */
//////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "fieldmap.h"
#include "field/eventwork.h"
#include "event_debug_local.h"
#include "gamesystem/gamesystem.h"
#include "event_debug_numinput.h"
#include "event_debug_numinput_record.h"

#include "system/main.h"
#include "field/field_msgbg.h"
#include "field_comm/mission_types.h"
#include "field_comm/intrude_mission.h"
#include "print/wordset.h"
#include "savedata/misc.h"
#include "savedata/situation.h"
#include "savedata/intrude_save.h"

#include "arc_def.h"
#include "debug_message.naix"
#include "msg/debug/msg_debug_intrude.h"

#include "event_debug_numinput_mission.h"


//--------------------------------------------------------------
//--------------------------------------------------------------
///数値入力リストデータ
typedef struct {
  u16 gmm_id;
  u32 param;
  const DEBUG_NUMINPUT_PARAM * dni_param;
}DEBUG_NUMINPUT_LIST;

///数値入力リスト初期化パラメータ定義
typedef struct{ 
  const u32 count_or_id; // リストの最大数
  const DEBUG_NUMINPUT_LIST * pList; // 各項目の数値入力時処理指定データ
} DEBUG_NUMINPUT_INITIALIZER;

//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 MissionGetFunc( GAMESYS_WORK* gsys, GAMEDATA* gamedata, u32 param );
static void MissionSetFunc( GAMESYS_WORK* gsys, GAMEDATA* gamedata, u32 mission_type, u32 mission_no );

static const DEBUG_NUMINPUT_PARAM DNIParam_Mission = { 0, 0, MissionGetFunc, MissionSetFunc }; 

static const DEBUG_NUMINPUT_LIST DNIList_Mission[] = 
{
  { STR_INTRUDE_MISSION_01, MISSION_TYPE_BASIC,       &DNIParam_Mission },
  { STR_INTRUDE_MISSION_02, MISSION_TYPE_SKILL,       &DNIParam_Mission },
  { STR_INTRUDE_MISSION_03, MISSION_TYPE_VICTORY,     &DNIParam_Mission },
  { STR_INTRUDE_MISSION_04, MISSION_TYPE_PERSONALITY, &DNIParam_Mission },
  { STR_INTRUDE_MISSION_05, MISSION_TYPE_ITEM,        &DNIParam_Mission },
  { STR_INTRUDE_MISSION_06, MISSION_TYPE_ATTRIBUTE,   &DNIParam_Mission },
};

static const DEBUG_NUMINPUT_INITIALIZER DNI_Init = { NELEMS( DNIList_Mission ), DNIList_Mission, };

static const FLDMENUFUNC_LIST DATA_DNumInputMenu[] =
{
  { STR_INTRUDE_MISSION_01, (void*)0 },
  { STR_INTRUDE_MISSION_02, (void*)1 },
  { STR_INTRUDE_MISSION_03, (void*)2 },
  { STR_INTRUDE_MISSION_04, (void*)3 },
  { STR_INTRUDE_MISSION_05, (void*)4 },
  { STR_INTRUDE_MISSION_06, (void*)5 },
}; 

//--------------------------------------------------------------
//-------------------------------------------------------------- 
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

static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer = {
  NARC_debug_message_debug_intrude_dat,  //メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu),  //項目数max
  DATA_DNumInputMenu, //メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader, //メニュヘッダ
  1, 4, 30, 16,
  NULL,
  NULL,
};


//////////////////////////////////////////////////////////////// 
static GMEVENT * DEBUG_EVENT_MissionSelect( GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_PARAM * dni_param, u32 id );

typedef struct {
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldWork; 
  DEBUG_MENU_CALLPROC call_proc;
  FLDMENUFUNC *menuFunc; 
  const DEBUG_NUMINPUT_INITIALIZER * init;
} MISSION_TYPE_SELECT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT MissionTypeSelectEvent( GMEVENT *event, int *seq, void *wk )
{
	MISSION_TYPE_SELECT_WORK * new_wk = wk;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( new_wk->gamedata );
  const DEBUG_NUMINPUT_INITIALIZER * init = new_wk->init;
  
  switch( (*seq) ){
  case 0:
    new_wk->menuFunc = DEBUGFLDMENU_InitEx( new_wk->fieldWork, new_wk->heapID,  &DATA_DNumInput_MenuInitializer, new_wk );
    (*seq)++;
    break;

  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( new_wk->menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){ break; } // 操作無し

      if( ret == FLDMENUFUNC_CANCEL ) {  //キャンセル
        FLDMENUFUNC_DeleteMenu( new_wk->menuFunc );
        return( GMEVENT_RES_FINISH );
      }

      GMEVENT_CallEvent( event, 
          DEBUG_EVENT_MissionSelect( new_wk->gmSys, init->pList[ret].dni_param, init->pList[ret].param ) );
      break;
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * DEBUG_NUM_INPUT_EVENT_MissionTypeSelect( GAMESYS_WORK * gsys )
{
	GMEVENT * new_event = GMEVENT_Create( gsys, NULL, MissionTypeSelectEvent, sizeof(MISSION_TYPE_SELECT_WORK) );
	MISSION_TYPE_SELECT_WORK * new_wk = GMEVENT_GetEventWork( new_event );

	GFL_STD_MemClear( new_wk, sizeof(MISSION_TYPE_SELECT_WORK) );
  new_wk->heapID = HEAPID_PROC;
  new_wk->gmSys = gsys;
  new_wk->gamedata = GAMESYSTEM_GetGameData( gsys );
  new_wk->fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  new_wk->call_proc = NULL;
  new_wk->menuFunc = NULL; 
  new_wk->init = &DNI_Init;

  return new_event;
}


////////////////////////////////////////////////////////////////

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
  int mission_no_search_work;
} DEBUG_NUMINPUT_WORK;

//--------------------------------------------------------------
/// 数値入力ウィンドウ：生成
//--------------------------------------------------------------
static void createNumWin( DEBUG_NUMINPUT_WORK * wk )
{
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
  
  wk->wordset = WORDSET_Create( wk->heapID );
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_debug_intrude_dat, wk->heapID);

	bmpwin = GFL_BMPWIN_Create( FLDBG_MFRM_MSG, _DISP_INITX, _DISP_INITY, _DISP_SIZEX, _DISP_SIZEY, PANO_FONT, GFL_BMP_CHRAREA_GET_B );
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
  const DEBUG_NUMINPUT_PARAM * def = wk->dni_param;
  GFL_BMPWIN * bmpwin = wk->bmpwin;

  STRBUF * strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
  STRBUF * expandBuf = GFL_STR_CreateBuffer( 64, wk->heapID );

  GFL_MSG_GetString( wk->msgman, STR_INTRUDE_NUMBER, strbuf );
  WORDSET_RegisterNumber(wk->wordset, 0, num, 10, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
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
  int mission_type = dni_wk->select_id;
  int mission_no = dni_wk->value;
  OCCUPY_INFO* occupy_info = GAMEDATA_GetMyOccupyInfo( dni_wk->gamedata );
  DEBUG_MISSION_TypeNoSet( occupy_info, mission_type, mission_no );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 dniGetValue( DEBUG_NUMINPUT_WORK * dni_wk )
{
  int mission_type = dni_wk->select_id;
  OCCUPY_INFO* occupy_info = GAMEDATA_GetMyOccupyInfo( dni_wk->gamedata );
  return DEBUG_MISSION_TypeNoGet( occupy_info, mission_type );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 dniGetNextValue( DEBUG_NUMINPUT_WORK * dni_wk )
{
  int mission_type = dni_wk->select_id;
  int mission_no = dni_wk->value;

  mission_no = DEBUG_MISSION_TypeSearch( mission_type, &dni_wk->mission_no_search_work );

  // 回り込み
  if( mission_no == -1 ) {
    dni_wk->mission_no_search_work = 0;
    mission_no = DEBUG_MISSION_TypeSearch( mission_type, &dni_wk->mission_no_search_work );
  }

  return mission_no;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 dniGetPrevValue( DEBUG_NUMINPUT_WORK * dni_wk )
{
  int mission_type = dni_wk->select_id;
  int mission_no = dni_wk->value;
  int prev_no = dni_wk->value;

  mission_no = DEBUG_MISSION_TypeSearch( mission_type, &dni_wk->mission_no_search_work );

  while( mission_no != dni_wk->value )
  {
    if( mission_no == -1 ) { // 回り込み
      dni_wk->mission_no_search_work = 0;
    }
    else {
      prev_no = mission_no;
    }
    mission_no = DEBUG_MISSION_TypeSearch( mission_type, &dni_wk->mission_no_search_work );
  }

  return prev_no;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT DebugNumInputEvent( GMEVENT * event , int *seq, void * work )
{
  DEBUG_NUMINPUT_WORK * dni_wk = work;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( dni_wk->gamedata );
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;

  switch(*seq) {
  case 0:
    dni_wk->value = dniGetValue( dni_wk );
    createNumWin( dni_wk );
    printNumWin( dni_wk, dni_wk->value );
    (*seq) ++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetRepeat();
      int cont = GFL_UI_KEY_GetCont();
      u32 after = dni_wk->value;
      u32 before = dni_wk->value;

      if( trg & PAD_BUTTON_B ) { // キャンセル
        (*seq) ++;
        break;
      }
      if( trg & PAD_BUTTON_A ) { // 決定
        dniSetValue( dni_wk, dni_wk->value );
        (*seq) ++;
        break;
      }

      if( trg & PAD_KEY_UP ) {
        after = dniGetNextValue( dni_wk );
      } 
      else if( trg & PAD_KEY_DOWN ) {
        after = dniGetPrevValue( dni_wk );
      }
      else if( trg & PAD_KEY_RIGHT ) {
        int i;
        for( i=0; i<5; i++ ) {
          after = dniGetNextValue( dni_wk );
        }
      }
      else if( trg & PAD_KEY_LEFT ) {
        int i;
        for( i=0; i<5; i++ ) {
          after = dniGetPrevValue( dni_wk );
        }
      }

      if( after != before ) {
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
//--------------------------------------------------------------
static GMEVENT * DEBUG_EVENT_MissionSelect( GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_PARAM * dni_param, u32 id )
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
  wk->mission_no_search_work = 0;

  return event;
}

//--------------------------------------------------------------
//-------------------------------------------------------------- 
static u32 MissionGetFunc( GAMESYS_WORK* gsys, GAMEDATA* gamedata, u32 param )
{
  return 0;
}

static void MissionSetFunc( GAMESYS_WORK* gsys, GAMEDATA* gamedata, u32 mission_type, u32 mission_no )
{
}

#endif  // PM_DEBUG
