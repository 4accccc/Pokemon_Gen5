//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_menu_ov_infinity.c
 *	@brief  無限ダンジョン用デバッグ
 *	@author	Masayuki Onoue
 *	@date		2011.03.29
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

// 外部公開
#include "event_debug_menu_ov_infinity.h"
#include "msg/debug/msg_debug_infinity.h"
#include "savedata/infinity_dungeon_save.h"
#include "field/infinity_dungeon.h"
#include "field/infinity_dungeon_local_def.h"

// 配置データ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_move_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.cdat" // INF_TR_ROOM_POS_TYPE_NONE

// グローバル変数
extern int g_debug_inf_tr_pos_type;
extern BOOL g_debug_inf_data_change;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
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
  INFINITYDUNGEON_SAVE  *p_save;
  GAMEDATA *p_gamedata;
  int num;
  u32 num_input_type;
} INFDUN_WORK;

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

///メニューリスト
static const DEBUG_MENU_LIST MenuList[] =
{
  { DEBUG_STR_INFINITY_CLEAR_RANK , (void*)DEBUG_STR_INFINITY_CLEAR_RANK , GFL_OVERLAY_BLANK_ID },  // クリアエリア指定
  { DEBUG_STR_INFINITY_AREA , (void*)DEBUG_STR_INFINITY_AREA , GFL_OVERLAY_BLANK_ID },  // クリアエリア指定
  { DEBUG_STR_INFINITY_JUMP_1 , (void*)DEBUG_STR_INFINITY_JUMP_1 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_2 , (void*)DEBUG_STR_INFINITY_JUMP_2 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_3 , (void*)DEBUG_STR_INFINITY_JUMP_3 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_4 , (void*)DEBUG_STR_INFINITY_JUMP_4 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_5 , (void*)DEBUG_STR_INFINITY_JUMP_5 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_6 , (void*)DEBUG_STR_INFINITY_JUMP_6 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_7 , (void*)DEBUG_STR_INFINITY_JUMP_7 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_8 , (void*)DEBUG_STR_INFINITY_JUMP_8 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_9 , (void*)DEBUG_STR_INFINITY_JUMP_9 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_10 , (void*)DEBUG_STR_INFINITY_JUMP_10 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_11 , (void*)DEBUG_STR_INFINITY_JUMP_11 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_JUMP_12 , (void*)DEBUG_STR_INFINITY_JUMP_12 , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_ENDLESS_CHECK , (void*)DEBUG_STR_INFINITY_ENDLESS_CHECK , GFL_OVERLAY_BLANK_ID },  // ダンジョン無限生成
  { DEBUG_STR_INFINITY_DEBUG_MODE , (void*)DEBUG_STR_INFINITY_DEBUG_MODE , GFL_OVERLAY_BLANK_ID },  // デバッグモード
  { DEBUG_STR_INFINITY_TR_POS_TYPE , (void*)DEBUG_STR_INFINITY_TR_POS_TYPE , GFL_OVERLAY_BLANK_ID },  // デバッグモード
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
static const DEBUG_MENU_INITIALIZER DebugMenuInfDunImitializer = {
  NARC_debug_message_debug_infinity_dat,
  NELEMS(MenuList),
  MenuList,
  &MenuHeader,
  1, 1, 30, 16,
  NULL,
  NULL
};

//-------------------------------------
///   数値入力タイプ
//=====================================
enum {
  STR_NUM_CLEAR_RANK,
  STR_NUM_AREA,
  STR_NUM_DEBUG_ON_OFF,
  STR_NUM_TR_POS_TYPE,
};

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンジャンプ
 *
 *	@param	work 
 *	@param	zone_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void JumpZone( INFDUN_WORK* work ,u16 zone_id)
{
  GMEVENT* p_subevent;
  FLDMENUFUNC_DeleteMenu( work->menuFunc );
  p_subevent = DEBUG_EVENT_QuickChangeMapDefaultPos( work->gmSys, work->fieldWork, zone_id );
  GMEVENT_CallEvent( work->gmEvent, p_subevent );
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
static void DrawNum( INFDUN_WORK* work )
{
  STRBUF *str;
  STRBUF *exp;
  FLDMSGBG * msgBG;
  WORDSET * wset;

  str = GFL_MSG_CreateString( work->msgData, DEBUG_STR_INFINITY_NUM );
  wset = WORDSET_Create( work->heapID );
  exp  = GFL_STR_CreateBuffer( 128, work->heapID );
  WORDSET_RegisterNumber( wset, 0, work->num, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
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
 *	@brief  共通数値入力
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void NumInput( INFDUN_WORK* p_wk )
{
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    p_wk->num--;
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    p_wk->num++;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン無限生成テスト
 *
 *	@retval FALSE = 継続
 */
//-----------------------------------------------------------------------------
static BOOL DebugDungeonMake()
{
  DEBUG_INFINITY_DUNGEON_MakeDungeonData();
  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_START )
  {
    return TRUE;
  }
  return FALSE;
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
static BOOL NumInputState( INFDUN_WORK* p_wk )
{
  switch( p_wk->sub_seq_no )
  {
  case 0:
    {
      FLDMSGBG * msgBG;
      // 数値の初期化
      switch( p_wk->num_input_type)
      {
      case STR_NUM_CLEAR_RANK:
        p_wk->num = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_wk->p_save );
        break;
      case STR_NUM_AREA:
        p_wk->num = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_wk->p_save );
        break;
      case STR_NUM_DEBUG_ON_OFF:
        p_wk->num = g_debug_inf_data_change;
        break;
      case STR_NUM_TR_POS_TYPE:
        p_wk->num = g_debug_inf_tr_pos_type;
        break;
      defautl:
        p_wk->num = 0;
        break;
      }
      p_wk->msgData = DEBUGFLDMENU_CreateMSGDATA(p_wk->fieldWork, DebugMenuInfDunImitializer.msg_dat_id );
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
      case STR_NUM_CLEAR_RANK:
        NumInput(p_wk);
        if( p_wk->num < 0 )
        {
          p_wk->num = 0;
        }
        INFINITYDUNGEON_SAVE_SetDungeonStatus( p_wk->p_save , p_wk->num );
        DrawNum( p_wk );
        break;
      case STR_NUM_AREA:
        {
          NumInput(p_wk);
          if( p_wk->num < 0 )
          {
            p_wk->num = 0;
          }
          INFINITYDUNGEON_SAVE_SetFloorID( p_wk->p_save , p_wk->num*10 );
          DrawNum( p_wk );
        }
        break;
      case STR_NUM_DEBUG_ON_OFF:
        {
          NumInput(p_wk);
          if( p_wk->num > 1 )
          {
            p_wk->num = 0;
          }
          if( p_wk->num < 0 )
          {
            p_wk->num = 1;
          }
          g_debug_inf_data_change = p_wk->num;
          DrawNum( p_wk );
        }
        break;
      case STR_NUM_TR_POS_TYPE:
        {
          NumInput(p_wk);
          if( p_wk->num > INF_TR_ROOM_POS_TYPE_NONE )
          {
            p_wk->num = 0;
          }
          if( p_wk->num < 0 )
          {
            p_wk->num = 0;
          }
          g_debug_inf_tr_pos_type = p_wk->num;
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
 *	@brief  メニュー選択
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMedalInfinityDungeon( GMEVENT *event, int *seq, void *wk )
{
  INFDUN_WORK *work = wk;

  switch( *seq ){
    // メニュー初期化
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuInfDunImitializer );
    work->sub_seq_no=0;
    (*seq)++;
    break;

    // メニュー選択処理
  case 1:
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
      // ランク操作
    case DEBUG_STR_INFINITY_CLEAR_RANK:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->num_input_type = STR_NUM_CLEAR_RANK;
        (*seq)++;
      }
      break;
      // フロア操作
    case DEBUG_STR_INFINITY_AREA:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->num_input_type = STR_NUM_AREA;
        (*seq)++;
      }
      break;
      // 無限生成テスト
    case DEBUG_STR_INFINITY_ENDLESS_CHECK:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        (*seq)=4;
      }
      break;
      // 無限ダンジョンのデータ差し替え機能ON・OFF
    case DEBUG_STR_INFINITY_DEBUG_MODE:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->num_input_type = STR_NUM_DEBUG_ON_OFF;
        (*seq)++;
      }
      break;
      // 無限ダンジョン内トレーナー配置固定
    case DEBUG_STR_INFINITY_TR_POS_TYPE:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->num_input_type = STR_NUM_TR_POS_TYPE;
        (*seq)++;
      }
      break;


      // ジャンプ
    case DEBUG_STR_INFINITY_JUMP_1: JumpZone(work,ZONE_ID_D31R0201); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_2: JumpZone(work,ZONE_ID_D31R0101); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_3: JumpZone(work,ZONE_ID_D31R1201); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_4: JumpZone(work,ZONE_ID_D31R1101); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_5: JumpZone(work,ZONE_ID_D31R0401); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_6: JumpZone(work,ZONE_ID_D31R0301); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_7: JumpZone(work,ZONE_ID_D31R0701); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_8: JumpZone(work,ZONE_ID_D31R0801); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_9: JumpZone(work,ZONE_ID_D31R0901); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_10: JumpZone(work,ZONE_ID_D31R1001); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_11: JumpZone(work,ZONE_ID_D31R0601); (*seq)=3; break;
    case DEBUG_STR_INFINITY_JUMP_12: JumpZone(work,ZONE_ID_D31R0501); (*seq)=3; break;
    }
    break;
    // 数値入力
  case 2:
    {
      if( !NumInputState( work ) )
      {
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  case 3:
    // 移動終了待ち
    return GMEVENT_RES_FINISH;
  case 4:
    // 終了待ち
    if( DebugDungeonMake( ) == TRUE)
    {
      return GMEVENT_RES_FINISH;
    }
    break;
  }


  return( GMEVENT_RES_CONTINUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデバッグメニュー起動
 *
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_InfinityDungeon(DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  INFDUN_WORK *work;
  SAVE_CONTROL_WORK *p_save_data;

  GMEVENT_Change( event, debugMenuMedalInfinityDungeon, sizeof(INFDUN_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(INFDUN_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->p_gamedata = GAMESYSTEM_GetGameData(work->gmSys);
  p_save_data = GAMEDATA_GetSaveControlWork(work->p_gamedata);
  work->p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  return TRUE;
}

