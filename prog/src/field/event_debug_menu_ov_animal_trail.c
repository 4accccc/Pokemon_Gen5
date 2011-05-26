//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_menu_ov_animal_trail.c
 *  @brief  獣道用デバッグ
 *	@author	Masayuki Onoue
 *	@date		2011.05.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "system/gfl_use.h"       // GFUser_VIntr_CreateTCB
#include "system/main.h"          // HEAPID
#include "arc_def.h"              // ARC_ID
#include "message.naix"           // MESSAGE_ARC_ID
#include "debug_message.naix"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "field_debug.h" 
#include "fieldmap.h"
#include "fieldmap/zone_id.h"
#include "event_mapchange.h"
#include "script.h"
#include "event_fieldmap_control.h"

#include "field/animal_trail.h" //ANIMAL_TRAIL_SelectRandomTable
#include "msg/debug/msg_debug_animal_trail.h"
#include "../../../resource/fldmapdata/script/animal_trail_scr_def.h"  //SCRID_DEBUG_

// 外部公開
#include "event_debug_menu_ov_animal_trail.h"

// デバッグ用グローバル変数
int DebugAnimalTrailWalkCnt = ANIMAL_TRAIL_SELECT_WALK_CNT;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
typedef struct {
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  GFL_ARCUTIL_TRANSINFO bgchar;
  FLDSYSWIN * win;
  GAMEDATA *p_gamedata;
  int input_next_seq;
  
  // 数値入力
  int num;
  int sub_seq_no;
  u32 num_input_type;
  GFL_MSGDATA * msgData;
} ANIMAL_TRAIL_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void NumInput( ANIMAL_TRAIL_WORK* p_wk );
static BOOL NumInputState( ANIMAL_TRAIL_WORK* p_wk );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///   メインシーケンス
//=====================================
enum {
  SEQ_INIT,
  SEQ_MAIN,
  SEQ_NUM_INPUT,
  SEQ_CALL_SCRIPT,
  SEQ_SET_ANIMAL_TRAIL_OBJ,
  SEQ_SET_TABLE_ID,
  SEQ_NUM_EXIT,
};
//-------------------------------------
///   数値入力タイプ
//=====================================
enum {
  STR_NUM_ANIMAL_TRAIL_NO,
  STR_NUM_ANIMAL_TABLE_NO,
  STR_NUM_ANIMAL_WALK_CNT,
};

///メニューリスト
static const DEBUG_MENU_LIST MenuList[] =
{
  { DEBUG_STR_GO_ANIMAL_TRAIL , (void*)DEBUG_STR_GO_ANIMAL_TRAIL , GFL_OVERLAY_BLANK_ID },  // 詳細画面起動
  { DEBUG_STR_ANIMAL_TRAIL_SET_OBJECT , (void*)DEBUG_STR_ANIMAL_TRAIL_SET_OBJECT , GFL_OVERLAY_BLANK_ID },  // アイテム設置
  { DEBUG_STR_ANIMAL_TRAIL_SET_TABLE_ID , (void*)DEBUG_STR_ANIMAL_TRAIL_SET_TABLE_ID , GFL_OVERLAY_BLANK_ID },  // 指定テーブル配置
  { DEBUG_STR_ANIMAL_TRAIL_SET_WALK_CNT , (void*)DEBUG_STR_ANIMAL_TRAIL_SET_WALK_CNT , GFL_OVERLAY_BLANK_ID },  // 抽選に必要な歩数
};

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER MenuHeader =
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
///   メニュー初期化変数
//=====================================
static const DEBUG_MENU_INITIALIZER DebugMenuAnimalTrailImitializer = {
  NARC_debug_message_debug_animal_trail_dat,
  NELEMS(MenuList),
  MenuList,
  &MenuHeader,
  1, 1, 30, 16,
  NULL,
  NULL
};

//----------------------------------------------------------------------------
/**
 *	@brief  メニュー選択
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuAnimalTrail( GMEVENT *event, int *seq, void *wk )
{
  ANIMAL_TRAIL_WORK *work = wk;

  switch( *seq ){
    // メニュー初期化
  case SEQ_INIT:
    {
      work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuAnimalTrailImitializer );
      (*seq) = SEQ_MAIN;
    }
    break;

    // メニュー選択処理
  case SEQ_MAIN:
    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){
      // 操作なし
    case FLDMENUFUNC_NULL:
      break;
      // キャンセル
    case FLDMENUFUNC_CANCEL:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;
    case DEBUG_STR_GO_ANIMAL_TRAIL:
      {
        // メニュー削除
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->input_next_seq = SEQ_CALL_SCRIPT;
        work->num_input_type = STR_NUM_ANIMAL_TRAIL_NO;
        (*seq) = SEQ_NUM_INPUT;
      }
      break;
    case DEBUG_STR_ANIMAL_TRAIL_SET_OBJECT:
      {
        // メニュー削除
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->input_next_seq = SEQ_SET_ANIMAL_TRAIL_OBJ;
        work->num_input_type = STR_NUM_ANIMAL_TRAIL_NO;
        (*seq) = SEQ_NUM_INPUT;
      }
      break;
    case DEBUG_STR_ANIMAL_TRAIL_SET_TABLE_ID:
      {
        // メニュー削除
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->input_next_seq = SEQ_SET_TABLE_ID;
        work->num_input_type = STR_NUM_ANIMAL_TABLE_NO;
        (*seq) = SEQ_NUM_INPUT;
      }
      break;
    case DEBUG_STR_ANIMAL_TRAIL_SET_WALK_CNT:
      {
        // メニュー削除
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->input_next_seq = SEQ_NUM_EXIT;
        work->num_input_type = STR_NUM_ANIMAL_WALK_CNT;
        (*seq) = SEQ_NUM_INPUT;
      }
      break;
    }
    break;
  case SEQ_NUM_INPUT:
    {
      if( !NumInputState( work ) )
      {
        (*seq) = work->input_next_seq;
      }
    }
    break;
  case SEQ_CALL_SCRIPT:
      SCRIPT_CallScript( event,SCRID_ANIMAL_TRAIL_01+work->num, NULL, NULL, HEAPID_FIELDMAP );
      (*seq) = SEQ_NUM_EXIT;
    break;
  case SEQ_SET_ANIMAL_TRAIL_OBJ:
    {
      SAVE_CONTROL_WORK* saveData = GAMEDATA_GetSaveControlWork(work->p_gamedata);
      ETC_SAVE_WORK *p_etc_save = SaveData_GetEtc( saveData );
      ANIMAL_TRAIL_SelectRandomTable(p_etc_save ,work->num);
      (*seq) = SEQ_NUM_EXIT;
    }
    break;
  case SEQ_SET_TABLE_ID:
    {
      SAVE_CONTROL_WORK* saveData = GAMEDATA_GetSaveControlWork(work->p_gamedata);
      ETC_SAVE_WORK *p_etc_save = SaveData_GetEtc( saveData );
      {
        int i;
        for( i = 0 ; i<20 ; i++ )
        {
          ANIMAL_TRAIL_SelectRandomTable(p_etc_save ,i);
          OS_Printf("けものみち%d番のテーブルIDを%dに上書き\n",i,work->num);
          ANIMAL_TRAIL_SAVE_SetMainTableIndex( p_etc_save ,i ,work->num );
        }
      }
      (*seq) = SEQ_NUM_EXIT;
    }
  case SEQ_NUM_EXIT:
    return GMEVENT_RES_FINISH;
  }

  return( GMEVENT_RES_CONTINUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  けものみちデバッグメニュー起動
 *
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_AnimalTrail(DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  ANIMAL_TRAIL_WORK *work;

  GMEVENT_Change( event, debugMenuAnimalTrail, sizeof(ANIMAL_TRAIL_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(ANIMAL_TRAIL_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->p_gamedata = GAMESYSTEM_GetGameData( gsys );

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  数値入力
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawNum( ANIMAL_TRAIL_WORK* work )
{
  STRBUF *str;
  STRBUF *exp;
  FLDMSGBG * msgBG;
  WORDSET * wset;

  str = GFL_MSG_CreateString( work->msgData, DEBUG_STR_ANIMAL_TRAIL_NUM );
  wset = WORDSET_Create( work->heapID );
  exp  = GFL_STR_CreateBuffer( 128, work->heapID );
  WORDSET_RegisterNumber( wset, 0, work->num, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
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
 *	@brief  数値入力ステート
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL NumInputState( ANIMAL_TRAIL_WORK* p_wk )
{
  switch( p_wk->sub_seq_no )
  {
  case 0:
    {
      FLDMSGBG * msgBG;
      // 数値の初期化
      switch( p_wk->num_input_type)
      {
      case STR_NUM_ANIMAL_TRAIL_NO:
        p_wk->num = 0;
        break;
      case STR_NUM_ANIMAL_TABLE_NO:
        p_wk->num = 0;
        break;
      case STR_NUM_ANIMAL_WALK_CNT:
        p_wk->num = DebugAnimalTrailWalkCnt;
        break;
      defautl:
        p_wk->num = 0;
        break;
      }
      p_wk->msgData = DEBUGFLDMENU_CreateMSGDATA(p_wk->fieldWork, DebugMenuAnimalTrailImitializer.msg_dat_id );
      msgBG = FIELDMAP_GetFldMsgBG( p_wk->fieldWork );
      p_wk->win = FLDSYSWIN_AddEx( msgBG, p_wk->msgData , 1, 1, 30, 2 );
      DrawNum( p_wk );
      p_wk->sub_seq_no++;
    }
    break;
  case 1:
    {
      switch( p_wk->num_input_type)
      {
      case STR_NUM_ANIMAL_TRAIL_NO:
        {
          NumInput(p_wk);
          if( p_wk->num < 0 )
          {
            p_wk->num = 19;
          }
          if( p_wk->num > 19)
          {
            p_wk->num = 0;
          }
          DrawNum( p_wk );
        }
        break;
      case STR_NUM_ANIMAL_TABLE_NO:
        {
          NumInput(p_wk);
          if( p_wk->num < 0 )
          {
            p_wk->num = 10;
          }
          if( p_wk->num > 10)
          {
            p_wk->num = 0;
          }
          DrawNum( p_wk );
        }
        break;
      case STR_NUM_ANIMAL_WALK_CNT:
        {
          NumInput(p_wk);
          if( p_wk->num < 0 )
          {
            p_wk->num = 99999;
          }
          if( p_wk->num > 99999)
          {
            p_wk->num = 0;
          }
          DebugAnimalTrailWalkCnt = p_wk->num;
          DrawNum( p_wk );
        }
        break;
      }
      
      // 終了判定
      if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A )
      {
        FLDSYSWIN_ClearWindow( p_wk->win );
        FLDSYSWIN_Delete( p_wk->win );
        GFL_MSG_Delete( p_wk->msgData );
        return FALSE;
      }
      
      break;
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  共通数値入力
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void NumInput( ANIMAL_TRAIL_WORK* p_wk )
{
  if( (GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT) ||
      (GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN))
  {
    p_wk->num--;
  }
  if( (GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT) ||
      (GFL_UI_KEY_GetRepeat() & PAD_KEY_UP))
  {
    p_wk->num++;
  }

}

