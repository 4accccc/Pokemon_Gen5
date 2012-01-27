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
#ifdef  PM_DEBUG

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

// グローバル変数
extern u32  g_debug_inf_master_seed;
extern BOOL g_debug_inf_is_map_info;


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
  u32 seed;
  u32 num_input_type;
} INFDUN_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

static void GetInfTrDataCheck( void );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

///メニューリスト
static const DEBUG_MENU_LIST MenuList[] =
{
  { DEBUG_STR_INFINITY_CLEAR_RANK , (void*)DEBUG_STR_INFINITY_CLEAR_RANK , GFL_OVERLAY_BLANK_ID },  // クリアエリア指定
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
  { DEBUG_STR_INFINITY_SEED_CHANGE , (void*)DEBUG_STR_INFINITY_SEED_CHANGE , GFL_OVERLAY_BLANK_ID },  // シード
  { DEBUG_STR_INFINITY_BLACK_VERSION , (void*)DEBUG_STR_INFINITY_BLACK_VERSION , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_WHITE_VERSION , (void*)DEBUG_STR_INFINITY_WHITE_VERSION , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_AUTO_MAP , (void*)DEBUG_STR_INFINITY_AUTO_MAP , GFL_OVERLAY_BLANK_ID },  // ジャンプ
  { DEBUG_STR_INFINITY_TRDATA_CHECK , (void*)DEBUG_STR_INFINITY_TRDATA_CHECK , GFL_OVERLAY_BLANK_ID },  // トレーナーデータチェック
  { DEBUG_STR_INFINITY_SEED_CHECK , (void*)DEBUG_STR_INFINITY_SEED_CHECK , GFL_OVERLAY_BLANK_ID },  // シード確認
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
  STR_NUM_SEED,
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
        {
          FIELD_WFBC_CORE_TYPE wfbc_type = GAMEDATA_GetMyWFBCType( p_wk->p_gamedata );
          p_wk->num = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_wk->p_save ,wfbc_type );
        }
        break;
      case STR_NUM_SEED:
        p_wk->num = g_debug_inf_master_seed;
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
        {
          FIELD_WFBC_CORE_TYPE wfbc_type = GAMEDATA_GetMyWFBCType( p_wk->p_gamedata );
          NumInput(p_wk);
          if( p_wk->num < 0 )
          {
            p_wk->num = 0;
          }
          INFINITYDUNGEON_SAVE_SetDungeonStatus( p_wk->p_save , p_wk->num ,wfbc_type );
          DrawNum( p_wk );
        }
        break;
      case STR_NUM_SEED:
        {
          NumInput(p_wk);
          if( p_wk->num < 0 )
          {
            p_wk->num = 0;
          }
          if( p_wk->num > 0xFFFFFFFF )
          {
            p_wk->num = 0;
          }
          g_debug_inf_master_seed = p_wk->num;
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
 *	@brief  数値表示のみの
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL DrawNumState( INFDUN_WORK* p_wk )
{
  switch( p_wk->sub_seq_no )
  {
  case 0:
    {
      FLDMSGBG * msgBG;
      // 数値の初期化
      FIELD_WFBC_CORE_TYPE wfbc_type = GAMEDATA_GetMyWFBCType( p_wk->p_gamedata );
      p_wk->num = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_wk->p_save ,wfbc_type );
      p_wk->msgData = DEBUGFLDMENU_CreateMSGDATA(p_wk->fieldWork, DebugMenuInfDunImitializer.msg_dat_id );
      msgBG = FIELDMAP_GetFldMsgBG( p_wk->fieldWork );
      p_wk->win = FLDSYSWIN_AddEx( msgBG, p_wk->msgData , 1, 1, 30, 6 );
      p_wk->sub_seq_no++;
    }
    break;
  case 1:
    {
      STRBUF *str;
      STRBUF *exp;
      FLDMSGBG * msgBG;
      WORDSET * wset;

      str = GFL_MSG_CreateString( p_wk->msgData, DEBUG_STR_INFINITY_SEED_CHECK_NUM );
      wset = WORDSET_Create( p_wk->heapID );
      exp  = GFL_STR_CreateBuffer( 128, p_wk->heapID );
      WORDSET_RegisterNumber( wset, 0, p_wk->seed, 10, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterHexNumber( wset, 1, p_wk->seed, 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wset, exp, str );

      msgBG = FIELDMAP_GetFldMsgBG( p_wk->fieldWork );
      FLDSYSWIN_ClearWindow( p_wk->win );
      FLDSYSWIN_PrintStrBuf( p_wk->win, 0, 0, exp );

      GFL_STR_DeleteBuffer( str );
      GFL_STR_DeleteBuffer( exp );
      WORDSET_Delete( wset );
      p_wk->sub_seq_no++;
    }
    break;
  case 2:
    {
      // 終了判定
      if( GFL_UI_KEY_GetTrg() )
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
      // シード操作
    case DEBUG_STR_INFINITY_SEED_CHANGE:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->num_input_type = STR_NUM_SEED;
        (*seq)++;
      }
      break;

    case DEBUG_STR_INFINITY_BLACK_VERSION:
      {
        // BLACKバージョンに切り替えてからジャンプ
        FIELD_WFBC_CORE* p_wk = GAMEDATA_GetMyWFBCCoreData( work->p_gamedata );
        p_wk->type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
        FIELD_WFBC_CORE_SetUpZoneData( p_wk );
        JumpZone(work,ZONE_ID_D31R0201); 
        (*seq)=3; 
        break;
      }

    case DEBUG_STR_INFINITY_WHITE_VERSION:
      {
        // ホワイトバージョンに切り替えてからジャンプ
        FIELD_WFBC_CORE* p_wk = GAMEDATA_GetMyWFBCCoreData( work->p_gamedata );
        p_wk->type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
        FIELD_WFBC_CORE_SetUpZoneData( p_wk );
        JumpZone(work,ZONE_ID_D31R0101); 
        (*seq)=3; 
        break;
      }
    case DEBUG_STR_INFINITY_AUTO_MAP:
      {
        // デバッグプリント設定
        FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( work->fieldWork );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        FIELD_DEBUG_SetPrint(debug,FIELD_DEBUG_PRINT_TYPE_NONE);
        // 表示状態設定
        if( g_debug_inf_is_map_info == FALSE )
        {
          g_debug_inf_is_map_info = TRUE;
        }
        else
        {
          g_debug_inf_is_map_info = FALSE;
        }
      }
      return GMEVENT_RES_FINISH;

    case DEBUG_STR_INFINITY_TRDATA_CHECK:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        GetInfTrDataCheck();
      }
      return GMEVENT_RES_FINISH;

    case DEBUG_STR_INFINITY_SEED_CHECK:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        work->seed = INFINITYDUNGEON_SAVE_GetRandomSeed( work->p_save );
        (*seq) = 4;
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
    return GMEVENT_RES_FINISH;
  case 4:
    {
      if( !DrawNumState( work ) )
      {
        return GMEVENT_RES_FINISH;
      }
      break;
    }
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

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンに出現するトレーナーデータチェック
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------

// トレーナーデータ参照用
#include "savedata/bsubway_savedata.h"
#include "battle/bsubway_battle_data.h"
#include "fld_btl_inst_tool.h"

// アイテムID
#include "item/itemsym.h"

#include "infinity_dungeon_tool.h"

static void GetInfTrDataCheck( void )
{
  BSUBWAY_PARTNER_DATA tr_data;
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  u16 tr_id = 0;

  // 使用可能アイテムテーブル
  const u32 c_item_table[]=
  {
    ITEM_DUMMY_DATA,          // ダミーデータ
    ITEM_HONOONOZYUERU,				// ほのおのジュエル
    ITEM_MIZUNOZYUERU,				// みずのジュエル
    ITEM_DENKINOZYUERU,				// でんきのジュエル
    ITEM_KUSANOZYUERU,				// くさのジュエル
    ITEM_KOORINOZYUERU,				// こおりのジュエル
    ITEM_KAKUTOUZYUERU,				// かくとうジュエル
    ITEM_DOKUNOZYUERU,				// どくのジュエル
    ITEM_ZIMENNOZYUERU,				// じめんのジュエル
    ITEM_HIKOUNOZYUERU,				// ひこうのジュエル
    ITEM_ESUPAAZYUERU,				// エスパージュエル
    ITEM_MUSINOZYUERU,				// むしのジュエル
    ITEM_IWANOZYUERU,				  // いわのジュエル
    ITEM_GOOSUTOZYUERU,				// ゴーストジュエル
    ITEM_DORAGONZYUERU,				// ドラゴンジュエル
    ITEM_AKUNOZYUERU,				  // あくのジュエル
    ITEM_HAGANENOZYUERU,			// はがねのジュエル
    ITEM_NOOMARUZYUERU,				// ノーマルジュエル

    ITEM_KURABONOMI,				// クラボのみ
    ITEM_KAGONOMI,				  // カゴのみ
    ITEM_MOMONNOMI,				  // モモンのみ
    ITEM_TIIGONOMI,				  // チーゴのみ
    ITEM_NANASINOMI,				// ナナシのみ
    ITEM_HIMERINOMI,				// ヒメリのみ
    ITEM_ORENNOMI,				  // オレンのみ
    ITEM_KIINOMI,				    // キーのみ
    ITEM_RAMUNOMI,				  // ラムのみ
    ITEM_OBONNOMI,				  // オボンのみ
    ITEM_FIRANOMI,				  // フィラのみ
    ITEM_UINOMI,				    // ウイのみ
    ITEM_MAGONOMI,				  // マゴのみ
    ITEM_BANZINOMI,				  // バンジのみ
    ITEM_IANOMI,				    // イアのみ
    ITEM_ZURINOMI,				  // ズリのみ
    ITEM_BURIINOMI,				  // ブリーのみ
    ITEM_NANANOMI,				  // ナナのみ
    ITEM_SESINANOMI,				// セシナのみ
    ITEM_PAIRUNOMI,				  // パイルのみ
    ITEM_ZAROKUNOMI,				// ザロクのみ
    ITEM_NEKOBUNOMI,				// ネコブのみ
    ITEM_TAPORUNOMI,				// タポルのみ
    ITEM_ROMENOMI,				  // ロメのみ
    ITEM_UBUNOMI,				    // ウブのみ
    ITEM_MATOMANOMI,				// マトマのみ
    ITEM_MOKOSINOMI,				// モコシのみ
    ITEM_GOSUNOMI,				  // ゴスのみ
    ITEM_RABUTANOMI,				// ラブタのみ
    ITEM_NOMERUNOMI,				// ノメルのみ
    ITEM_NOWAKINOMI,				// ノワキのみ
    ITEM_SIIYANOMI,				  // シーヤのみ
    ITEM_KAISUNOMI,				  // カイスのみ
    ITEM_DORINOMI,				  // ドリのみ
    ITEM_BERIBUNOMI,				// ベリブのみ
    ITEM_OKKANOMI,				  // オッカのみ
    ITEM_ITOKENOMI,				  // イトケのみ
    ITEM_SOKUNONOMI,				// ソクノのみ
    ITEM_RINDONOMI,				  // リンドのみ
    ITEM_YATHENOMI,				  // ヤチェのみ
    ITEM_YOPUNOMI,				  // ヨプのみ
    ITEM_BIAANOMI,				  // ビアーのみ
    ITEM_SYUKANOMI,				  // シュカのみ
    ITEM_BAKOUNOMI,				  // バコウのみ
    ITEM_UTANNOMI,				  // ウタンのみ
    ITEM_TANGANOMI,				  // タンガのみ
    ITEM_YOROGINOMI,				// ヨロギのみ
    ITEM_KASIBUNOMI,				// カシブのみ
    ITEM_HABANNOMI,				  // ハバンのみ
    ITEM_NAMONOMI,				  // ナモのみ
    ITEM_RIRIBANOMI,				// リリバのみ
    ITEM_HOZUNOMI,				  // ホズのみ
    ITEM_TIIRANOMI,				  // チイラのみ
    ITEM_RYUGANOMI,				  // リュガのみ
    ITEM_KAMURANOMI,				// カムラのみ
    ITEM_YATAPINOMI,				// ヤタピのみ
    ITEM_ZUANOMI,				    // ズアのみ
    ITEM_SANNOMI,				    // サンのみ
    ITEM_SUTAANOMI,				  // スターのみ
    ITEM_NAZONOMI,				  // ナゾのみ
    ITEM_MIKURUNOMI,				// ミクルのみ
    ITEM_IBANNOMI,				  // イバンのみ
    ITEM_ZYAPONOMI,				  // ジャポのみ
    ITEM_RENBUNOMI,				  // レンブのみ
  };

  OS_Printf("TRIDは1オリジンで表示します(バトルサブウェイデータメーカーでの表示にあわせている)\n");

  for( tr_id = 0; tr_id < INF_DUNGEON_TR_ID_MAX  ; tr_id++ )
  {
    BOOL set_item   = FALSE;
    BOOL same_item  = FALSE;
    u16 item_no = 0;
    s32 i;
    u32 poke_index;
    BSUBWAY_POKEMON_ROM_DATA  prd_d;

    //トレーナーデータセット
    trd = FBI_TOOL_AllocTrainerRomData(&tr_data ,ARCID_INFINITY_DUGENON_TR ,
        tr_id,NARC_message_infinity_trname_dat ,HEAPID_FIELDMAP);

    // 手持ちになる可能性のあるポケモンを全てチェック
    for( poke_index = 0; poke_index < trd->use_poke_cnt ; poke_index ++ )
    {
      BOOL item_find = FALSE;
      GFL_ARC_LoadData( &prd_d,ARCID_INFINITY_DUGENON_PD,
          trd->use_poke_table[poke_index] );

      // アイテム重複チェック
      if( set_item == FALSE )
      {
        // 先頭のアイテムを保存
        item_no   = prd_d.item_no;
        same_item = TRUE;
        set_item = TRUE;
      }
      else
      {
        // 異なったアイテムを所持しているかチェック
        if( item_no != prd_d.item_no )
        {
          same_item = FALSE;
        }
      }


      // 持ち物チェック
      for( i = 0 ; i < NELEMS( c_item_table ) ; i++ )
      {
        if( prd_d.item_no == c_item_table[i] )
        {
          item_find = TRUE;
          break;
        }
      }
      if( item_find == FALSE )
      {
        GF_ASSERT_MSG(0,"TRID:%d 使用を想定していないアイテムを装備したポケモンを所持しています\n",tr_id+1);
      }

      // 努力値チェック
      {
        /**
         *  ステータスIndex
         */
        enum {
          _HP = 0,
          _POW,
          _DEF,
          _AGI,
          _SPOW,
          _SDEF,
          _PARAM_MAX,
        };
        ///< パーソナル贈与努力値ID
        u8  param_id_tbl[_PARAM_MAX] =
        {
          POKEPER_ID_pains_hp,
          POKEPER_ID_pains_pow,
          POKEPER_ID_pains_def,
          POKEPER_ID_pains_agi,
          POKEPER_ID_pains_spepow,
          POKEPER_ID_pains_spedef,
        };
        u8  exp[ _PARAM_MAX ];
        u8 exp_cnt = 0;

        // 基本努力値をパーソナルから取得
        for(i=0; i<_PARAM_MAX; ++i){
          exp[ i ] = POKETOOL_GetPersonalParam( prd_d.mons_no, prd_d.form_no, param_id_tbl[i] );
        }

        // 複数の努力値チェック
        for(i=0; i<_PARAM_MAX; ++i){
          if( exp[ i ] > 0 )
          {
            exp_cnt++;
          }
          if( exp_cnt > 1 )
          {
            GF_ASSERT_MSG(0,"TRID:%d \n複数の努力値が混ざっています\n",tr_id+1);
          }
        }
    
        // 抽選トレーナーのみチェック
        if( tr_id < INF_DUNGEON_TR_DATA_MAX )
        {
          const INF_TR_AREA_DATA* sc_tr_data = INF_TOOL_GetTrainerParam(tr_id);

          if( sc_tr_data->version == VERSION_W2 )
          {
            if( exp[_SPOW] > 0 )
            {
              GF_ASSERT_MSG(0,"TRID:%d モンスター番号:%d\n特攻の努力値を持ったポケモンがホワイトに出現しています。\n",tr_id+1,prd_d.mons_no);
            }
            if( exp[_SDEF] > 0 )
            {
              GF_ASSERT_MSG(0,"TRID:%d モンスター番号:%d\n特防の努力値を持ったポケモンがホワイトに出現しています。\n",tr_id+1,prd_d.mons_no);
            }
            if( exp[_AGI] > 0 )
            {
              GF_ASSERT_MSG(0,"TRID:%d モンスター番号:%d\n素早さの努力値を持ったポケモンがホワイトに出現しています。\n",tr_id+1,prd_d.mons_no);
            }
          }
          else if( sc_tr_data->version == VERSION_B2 )
          {
            if( exp[_POW] > 0 )
            {
              GF_ASSERT_MSG(0,"TRID:%d モンスター番号:%d\n攻撃の努力値を持ったポケモンがブラックに出現しています。\n",tr_id+1,prd_d.mons_no);
            }
            if( exp[_DEF] > 0 )
            {
              GF_ASSERT_MSG(0,"TRID:%d モンスター番号:%d\n防御の努力値を持ったポケモンがブラックに出現しています。\n",tr_id+1,prd_d.mons_no);
            }
            if( exp[_HP] > 0 )
            {
              GF_ASSERT_MSG(0,"TRID:%d モンスター番号:%d\nHPの努力値を持ったポケモンがブラックに出現しています。\n",tr_id+1,prd_d.mons_no);
            }
          }
          else
          {
            GF_ASSERT(0);
          }
        }
      }
    }

    // 手持ちポケモンのアイテムチェック
    if( (same_item == TRUE) && (ITEM_DUMMY_DATA != item_no) )
    {
      GF_ASSERT_MSG(0,"TRID:%d\n手持ちポケモンのアイテムが全て同じです。",tr_id+1);
    }

    GFL_HEAP_FreeMemory( trd );
  }

  OS_Printf("チェック完了\n");
}

#endif  //  PM_DEBUG

