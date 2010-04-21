//===============================================================================
/*
 *  @file event_debug_livecomm.c
 *  @brief  すれ違い通信デバッグメニューイベント
 *  @author Miyuki Iwasawa
 *  @date   10.04.19
 */
//===============================================================================

#ifdef PM_DEBUG

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

#include "sound/pm_sndsys.h"

#include "field_sound.h"
#include "eventwork.h"

#include "event_debug_local.h"

#include "message.naix"
#include "msg/msg_d_numinput.h"

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode
#include "print/wordset.h"  //WORDSET
#include "msg/msg_d_field.h"
#include "msg/msg_d_livecomm.h"

#include "arc/fieldmap/debug_symbol.naix"  //DEBUG_SYM_

#include "arc/fieldmap/debug_list.h"  //DEBUG_SCR_

#include "savedata/intrude_save.h"
#include "net/net_whpipe.h"

#include "savedata/playtime.h"

#include "field/beacon_view.h"
#include "field/field_comm/beacon_view_local.h"
#include "event_debug_livecomm.h"


typedef struct _DMENU_LIVE_COMM{
  HEAPID  heapID;
  HEAPID  tmpHeapID;

  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;

  SAVE_CONTROL_WORK* save;
  INTRUDE_SAVE_WORK* int_sv;
  MYITEM_PTR item_sv;
  MISC* misc_sv;

  BEACON_VIEW*    view_wk;

  BEACON_STATUS* b_status;
  GAMEBEACON_INFO_TBL*  infoStack;  //スタックワーク
  GAMEBEACON_INFO_TBL*  infoLog;    //ログテーブル
 
  GFL_FONT* fontHandle;
	GFL_MSGDATA* msgman;						//メッセージマネージャー
  WORDSET* wset;
  STRBUF *str_tmp;
  STRBUF *str_expand;
 
  FLDMENUFUNC *menuFunc;

  GAMEBEACON_INFO*  tmpInfo;
  u16               tmpTime;
  BMP_WIN           win_ninput;
}DMENU_LIVE_COMM;

typedef int (*NINPUT_GET_FUNC)( DMENU_LIVE_COMM* wk, int param );
typedef void (*NINPUT_SET_FUNC)( DMENU_LIVE_COMM* wk, int param, int value );

/// 数値入力：パラメータ
typedef struct {
  int min;  ///<入力最小値
  int max;  ///<入力最大値
  int* ret_wk;
  ///値を取得するための関数
  NINPUT_SET_FUNC set_func;
  NINPUT_GET_FUNC get_func;
}NINPUT_PARAM;

//======================================================================
//
//定数&データ定義
//
//======================================================================
typedef enum{
  SEQ_MENU_INIT,
  SEQ_MENU_MAIN,
  SEQ_EXIT,
}SEQ_ID;

enum{
 MENU_BEACON_REQ,
 MENU_STACK_CHECK,
 MENU_SURETIGAI_NUM,
 MENU_THANKS_NUM,
 MENU_STACK_CLEAR,
 MENU_MEMBER_CLEAR,
 MENU_COMM_BUF_CLEAR,
 MENU_EXIT,
};

#define PANO_FONT (14)
#define FBMP_COL_WHITE  (15)
#define FCOL_WIN_BASE (15)
#define FCOL_WIN_W  ( PRINTSYS_MACRO_LSB(1,2,FCOL_WIN_BASE) ) ///<BG白抜き

#define SCHECK_LINE_MAX (4)
#define SCHECK_LINE_OY  (16*3)
#define SCHECK_PAGE_MAX (8)

static const FLDMENUFUNC_LIST DATA_DebugLiveCommMenuList[] =
{
  { dlc_menu_01, (void*)MENU_BEACON_REQ },
  { dlc_menu_02, (void*)MENU_STACK_CHECK },
  { dlc_menu_03, (void*)MENU_SURETIGAI_NUM },
  { dlc_menu_04, (void*)MENU_THANKS_NUM },
  { dlc_menu_05, (void*)MENU_STACK_CLEAR },
  { dlc_menu_06, (void*)MENU_MEMBER_CLEAR },
  { dlc_menu_07, (void*)MENU_COMM_BUF_CLEAR },
  { dlc_menu_08, (void*)MENU_EXIT },
};

static const DEBUG_MENU_INITIALIZER DebugLiveCommMenuData = {
  NARC_message_d_livecomm_dat,
  NELEMS(DATA_DebugLiveCommMenuList),
  DATA_DebugLiveCommMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//======================================================================
//
//プロトタイプ
//
//======================================================================

static GMEVENT_RESULT event_LiveCommMain( GMEVENT * event, int *seq, void * work);
static void dmenu_WorkInit( DMENU_LIVE_COMM* wk );
static void dmenu_WorkRelease( DMENU_LIVE_COMM* wk );

static void sub_BmpWinAdd( BMP_WIN* bmpwin, u8 px, u8 py, u8 sx, u8 sy );
static void sub_BmpWinDel( BMP_WIN* bmpwin );
static void sub_GetMsgToBuf( DMENU_LIVE_COMM* wk, u8 msg_id );
static inline void sub_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str );

static GMEVENT* event_CreateEventNumInput( DMENU_LIVE_COMM* wk, int key, int min, int max, int* ret_wk, NINPUT_SET_FUNC set_func, NINPUT_GET_FUNC get_func );
static GMEVENT* event_CreateEventStackCheck( GAMESYS_WORK* gsys, DMENU_LIVE_COMM* wk );

static int ninput_GetMiscCount( DMENU_LIVE_COMM* wk, int param );
static void ninput_SetMiscCount( DMENU_LIVE_COMM* wk, int param, int value );

//======================================================================
//
//関数定義
//
//======================================================================
//--------------------------------------------------------------
/**
 * すれ違い通信デバッグメニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_LiveComm( GAMESYS_WORK* gsys, void* parent_work )
{
  DMENU_LIVE_COMM* wk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_LiveCommMain, sizeof(DMENU_LIVE_COMM) );

  wk = GMEVENT_GetEventWork( event );
  wk->gsys = gsys; 
  wk->gdata = GAMESYSTEM_GetGameData( gsys ); 
  wk->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  wk->save = GAMEDATA_GetSaveControlWork( wk->gdata ); 
  wk->int_sv = SaveData_GetIntrude( wk->save );
  wk->item_sv = GAMEDATA_GetMyItem( wk->gdata );
  wk->misc_sv = (MISC*)SaveData_GetMiscConst( wk->save );

  wk->view_wk = (BEACON_VIEW*)parent_work;

  wk->heapID = FIELDMAP_GetHeapID( wk->fieldmap );
  wk->tmpHeapID = GFL_HEAP_LOWID( wk->heapID );
  
  dmenu_WorkInit( wk );

  return event;
}

//-----------------------------------------------------------------
/*
 *  @brief  すれ違いデバッグメニューワーク初期化
 */
//-----------------------------------------------------------------
static void dmenu_WorkInit( DMENU_LIVE_COMM* wk )
{
  wk->fontHandle = FLDMSGBG_GetFontHandle( FIELDMAP_GetFldMsgBG( wk->fieldmap ) );

  wk->wset = WORDSET_Create( wk->heapID );
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_MESSAGE, NARC_message_d_livecomm_dat, wk->heapID );
  
  wk->str_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->str_expand = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );

  wk->tmpInfo = GAMEBEACON_Alloc( wk->heapID );
}

//-----------------------------------------------------------------
/*
 *  @brief  すれ違いデバッグメニューワーク解放
 */
//-----------------------------------------------------------------
static void dmenu_WorkRelease( DMENU_LIVE_COMM* wk )
{
  GFL_HEAP_FreeMemory( wk->tmpInfo );

  GFL_STR_DeleteBuffer( wk->str_expand );
  GFL_STR_DeleteBuffer( wk->str_tmp );

  GFL_MSG_Delete( wk->msgman );
  WORDSET_Delete( wk->wset );
}

//--------------------------------------------------------------
/*
 *  @brief  すれ違いデバッグメニューメインイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_LiveCommMain( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  DMENU_LIVE_COMM* wk = (DMENU_LIVE_COMM*)work;

  switch(*seq)
  {
  case SEQ_MENU_INIT:
    wk->menuFunc = DEBUGFLDMENU_Init( wk->fieldmap, wk->heapID,  &DebugLiveCommMenuData );
    (*seq)++;
    break;
  case SEQ_MENU_MAIN:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL || ret == MENU_EXIT ){  //キャンセル
        *seq = SEQ_EXIT; 
        break;
      }
      switch( ret ){
      case MENU_BEACON_REQ:
      case MENU_STACK_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventStackCheck( wk->gsys, wk ) );
        break;
      case MENU_SURETIGAI_NUM:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, 0, wk->view_wk->ctrl.max, CROSS_COMM_SURETIGAI_COUNT_MAX,
            NULL, ninput_SetMiscCount, ninput_GetMiscCount) );
        break;
      case MENU_THANKS_NUM:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, 1, 0, CROSS_COMM_THANKS_RECV_COUNT_MAX,
            NULL, ninput_SetMiscCount, ninput_GetMiscCount) );
        break;
      case MENU_STACK_CLEAR:
        GAMEBEACON_InfoTbl_Clear( wk->view_wk->infoStack );
        break;
      case MENU_MEMBER_CLEAR:
        DEBUG_BEACON_VIEW_MemberListClear( wk->view_wk );
        break;
      case MENU_COMM_BUF_CLEAR:
        DEBUG_RecvBeaconBufferClear();
        break;
      }
/*
        typedef void (* CHANGE_FUNC)(DMESSWORK*);
        CHANGE_FUNC func = (CHANGE_FUNC)ret;
        func(work);
        //FIELD_SUBSCREEN_ChangeForce(FIELDMAP_GetFieldSubscreenWork(work->fieldWork), ret);
*/
      *seq = SEQ_MENU_INIT; 
    }
    break;
  case SEQ_EXIT:
  default:
    dmenu_WorkRelease( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//-----------------------------------------------------------------
/*
 *  @brief  ウィンドウ追加 
 */
//-----------------------------------------------------------------
static void sub_BmpWinAdd( BMP_WIN* bmpwin, u8 px, u8 py, u8 sx, u8 sy )
{
  bmpwin->win = GFL_BMPWIN_Create( FLDBG_MFRM_MSG,
		px, py, sx, sy, PANO_FONT, GFL_BMP_CHRAREA_GET_B );

  bmpwin->bmp = GFL_BMPWIN_GetBmp( bmpwin->win );
  bmpwin->frm = GFL_BMPWIN_GetFrame( bmpwin->win );
	GFL_BMPWIN_MakeScreen( bmpwin->win );
	GFL_BMPWIN_TransVramCharacter( bmpwin->win );
	GFL_BG_LoadScreenV_Req( bmpwin->frm );
}

//-----------------------------------------------------------------
/*
 *  @brief  ウィンドウ削除
 */
//-----------------------------------------------------------------
static void sub_BmpWinDel( BMP_WIN* bmpwin )
{
  GFL_BMPWIN_ClearScreen( bmpwin->win );
	GFL_BG_LoadScreenV_Req( bmpwin->frm );

  GFL_BMPWIN_Delete( bmpwin->win );
}

//----------------------------------------------------------
/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
//----------------------------------------------------------
static void sub_GetMsgToBuf( DMENU_LIVE_COMM* wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->msgman, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wset, wk->str_expand, wk->str_tmp );
}

//----------------------------------------------------------
/*
 *  @brief  フリーワード展開
 */
//----------------------------------------------------------
static inline void sub_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str )
{
  WORDSET_RegisterWord( wset, idx, str, PM_NEUTRAL, TRUE, PM_LANG );
}

//----------------------------------------------------------
/*
 *  @brief  数値入力ウィンドウ追加
 */
//----------------------------------------------------------
static void sub_NumInputWinAdd( DMENU_LIVE_COMM* wk )
{
  sub_BmpWinAdd( &wk->win_ninput, 1, 23-2, 16, 2);
}
//----------------------------------------------------------
/*
 *  @brief  数値入力ウィンドウ削除
 */
//----------------------------------------------------------
static void sub_NumInputWinDel( DMENU_LIVE_COMM* wk )
{
  sub_BmpWinDel( &wk->win_ninput );
}

//===================================================================================
//数値入力イベント
//===================================================================================
typedef struct _EVWK_NINPUT{
  DMENU_LIVE_COMM* dlc_wk;

  u8    decide_f;
  int   key;
  int   value;

  NINPUT_PARAM  prm;
  BMP_WIN win;
}EVWK_NINPUT;

static GMEVENT_RESULT event_NumInputMain( GMEVENT * event, int *seq, void * work);
static void ninput_PrintNumWin( DMENU_LIVE_COMM* wk, BMP_WIN* bmpwin, int num );

//--------------------------------------------------------------
/**
 * 数値入力イベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventNumInput( DMENU_LIVE_COMM* wk, int key, int min, int max, int* ret_wk, NINPUT_SET_FUNC set_func, NINPUT_GET_FUNC get_func )
{
  EVWK_NINPUT* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( wk->gsys, NULL, event_NumInputMain, sizeof(EVWK_NINPUT) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlc_wk = wk; 

  evwk->prm.min = min;
  evwk->prm.max = max;
  evwk->prm.ret_wk = ret_wk;
  evwk->prm.set_func = set_func;
  evwk->prm.get_func = get_func;
 
  evwk->key = key;

  if( ret_wk != NULL ){
    evwk->value = *ret_wk;
  }else{
    evwk->value = (get_func)( wk, key );
  }
  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  スタックチェックイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_NumInputMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_NINPUT* evwk = (EVWK_NINPUT*)work;
  DMENU_LIVE_COMM* wk = evwk->dlc_wk;
  NINPUT_PARAM* prm = &evwk->prm;

  switch(*seq)
  {
  case 0:
    sub_BmpWinAdd( &evwk->win, 1, 23-2, 16, 2 );
    ninput_PrintNumWin( wk, &evwk->win, evwk->value );
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();
      int diff;
      int before, after;
      
      if( trg & PAD_BUTTON_B ){
        if( prm->ret_wk != NULL ){
          *prm->ret_wk = -1;
        }
        (*seq)++;
        break;
      }
      if( trg & PAD_BUTTON_A ){
        if( prm->ret_wk != NULL ){
          *prm->ret_wk = evwk->value;
        }else{
          (prm->set_func)( wk, evwk->key, evwk->value );
        }
        (*seq)++;
        break;
      }
      after = before = evwk->value;
      if( evwk->value < prm->min || evwk->value > prm->max ){
        break;
      }
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
      if( (diff < 0) && ( (after-prm->min) < (diff*-1))){
        if( before > prm->min ){
          after = prm->min; //いったんmin
        }else{
          after = prm->max; //回り込み
        }
      }else if( (diff > 0) && ((prm->max-after) < diff) ){
        if( before < prm->max ){
          after = prm->max; //いったんmax
        }else{
          after = prm->min; //回り込み
        }
      }else{
        after = before+diff;
      }
      if (after != before ) {
        ninput_PrintNumWin( wk, &evwk->win, after );
        evwk->value = after;
      }
    }
    break;
  default:
    sub_BmpWinDel( &evwk->win );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/// 数値入力ウィンドウ：表示更新
//--------------------------------------------------------------
static void ninput_PrintNumWin( DMENU_LIVE_COMM* wk, BMP_WIN* bmpwin, int num )
{
  WORDSET_RegisterNumber(wk->wset, 0, num, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  sub_GetMsgToBuf( wk, dlc_num_input );

  GFL_BMP_Clear( bmpwin->bmp, FCOL_WIN_BASE );
	PRINTSYS_PrintColor( bmpwin->bmp, 0, 0, wk->str_expand, wk->fontHandle, FCOL_WIN_W );

  GFL_BMPWIN_TransVramCharacter( bmpwin->win );  //transfer characters
	GFL_BG_LoadScreenV_Req( bmpwin->frm );  //transfer screen
}

//===================================================================================
//スタックチェックイベント
//===================================================================================
typedef struct _EVWK_STACK_CHECK{
  DMENU_LIVE_COMM* dlc_wk;

  u8      page;
  BMP_WIN win;
}EVWK_STACK_CHECK;

static GMEVENT_RESULT event_StackCheckMain( GMEVENT * event, int *seq, void * work);
static void scheck_ParamDraw( DMENU_LIVE_COMM* wk, EVWK_STACK_CHECK* evwk, u8 idx, u8 line );
static void scheck_PageDraw( DMENU_LIVE_COMM* wk, EVWK_STACK_CHECK* evwk, u8 page );

//--------------------------------------------------------------
/**
 * スタックチェックイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventStackCheck( GAMESYS_WORK* gsys, DMENU_LIVE_COMM* wk )
{
  EVWK_STACK_CHECK* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_StackCheckMain, sizeof(EVWK_STACK_CHECK) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlc_wk = wk; 

  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  スタックチェックイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_StackCheckMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_STACK_CHECK* evwk = (EVWK_STACK_CHECK*)work;
  DMENU_LIVE_COMM* wk = evwk->dlc_wk;

  switch(*seq)
  {
  case 0:
    sub_BmpWinAdd( &evwk->win, 0, 0, 32, 24 );
    scheck_PageDraw( wk, evwk, evwk->page );
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & PAD_BUTTON_B){
        (*seq)++;
      }else if( trg & PAD_KEY_LEFT ){
        evwk->page = (evwk->page+(SCHECK_PAGE_MAX-1))%SCHECK_PAGE_MAX;
        scheck_PageDraw( wk, evwk, evwk->page );
      }else if( trg & PAD_KEY_RIGHT ){
        evwk->page = (evwk->page+1)%SCHECK_PAGE_MAX;
        scheck_PageDraw( wk, evwk, evwk->page );
      }
    }
    break;
  default:
    sub_BmpWinDel( &evwk->win );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/*
 *  @brief  スタック情報書き出し
 */
//--------------------------------------------------------------
static void scheck_ParamDraw( DMENU_LIVE_COMM* wk, EVWK_STACK_CHECK* evwk, u8 idx, u8 line )
{
  u8 msg_id;
  GAMEBEACON_ACTION action;

  WORDSET_RegisterNumber( wk->wset, 0, idx, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( GAMEBEACON_InfoTbl_GetBeacon( wk->view_wk->infoStack, wk->tmpInfo, &wk->tmpTime, idx)){
    msg_id = dlc_stack_check_param01;
  
    //プレイヤー名取得
    GAMEBEACON_Get_PlayerNameToBuf( wk->tmpInfo, wk->str_tmp );
    sub_FreeWordset( wk->wset, 1, wk->str_tmp );

    //トレーナーID
    WORDSET_RegisterNumber( wk->wset, 2, GAMEBEACON_Get_TrainerID( wk->tmpInfo ), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    //アクション
    action = GAMEBEACON_Get_Action_ActionNo( wk->tmpInfo );
    WORDSET_RegisterNumber( wk->wset, 3, action, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    GFL_MSG_GetString( wk->msgman, dlc_action_01+action, wk->str_tmp);
    sub_FreeWordset( wk->wset, 4, wk->str_tmp );

  }else{
    msg_id = dlc_stack_check_param02;
  }
  sub_GetMsgToBuf( wk, msg_id );
	PRINTSYS_PrintColor( evwk->win.bmp, 0, line*SCHECK_LINE_OY, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
}

//--------------------------------------------------------------
/*
 *  @brief  スタック情報書き出し
 */
//--------------------------------------------------------------
static void scheck_PageDraw( DMENU_LIVE_COMM* wk, EVWK_STACK_CHECK* evwk, u8 page )
{
  int i,idx;
  GFL_BMP_Clear( evwk->win.bmp, FCOL_WIN_BASE );

  idx = page*SCHECK_LINE_MAX;
  for(i = 0;i < SCHECK_LINE_MAX;i++){
    if( idx+i >= BS_LOG_MAX ){
      break;
    }
    scheck_ParamDraw( wk, evwk, idx+i, i );
  }
	GFL_BMPWIN_TransVramCharacter( evwk->win.win );
	GFL_BG_LoadScreenV_Req( evwk->win.frm );
}


//===========================================================================
//数値入力ゲッター＆セッター
//===========================================================================
//--------------------------------------------------------------
/*
 *  @brief  Misc関係　ゲット 
 */
//--------------------------------------------------------------
static int ninput_GetMiscCount( DMENU_LIVE_COMM* wk, int param )
{
  switch(param){
  case 0:
    return MISC_CrossComm_GetSuretigaiCount( wk->misc_sv );
  case 1:
    return MISC_CrossComm_GetThanksRecvCount( wk->misc_sv );
  }
  return 0;
}

//--------------------------------------------------------------
/*
 *  @brief  Misc関係　セット 
 */
//--------------------------------------------------------------
static void ninput_SetMiscCount( DMENU_LIVE_COMM* wk, int param, int value )
{
  switch(param){
  case 0:
    MISC_CrossComm_SetSuretigaiCount( wk->misc_sv, value );
    DEBUG_BEACON_VIEW_SuretigaiCountSet( wk->view_wk, value );
  case 1:
    MISC_CrossComm_SetThanksRecvCount( wk->misc_sv, value );
  }
}









#endif  //PM_DEBUG

