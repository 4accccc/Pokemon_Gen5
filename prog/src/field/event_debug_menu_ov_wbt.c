//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_wbt.c
 *	@brief  デバッグメニューWBT系オーバーレイ領域
 *	@author	tamada GAMEFREAK inc.
 *	@date		2011.02.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef  PM_DEBUG

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
#include "sound/pm_sndsys.h"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "event_mapchange.h"
#include "savedata/regulation.h"
#include "poke_tool/poke_regulation.h"

//アプリ

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 

#include "msg/debug/msg_debug_wbt.h"

//外部公開
#include "event_debug_menu_ov_wbt.h"

#include "savedata/wbt_ex_save.h"
#include "savedata/wbt_download.h"
#include "savedata/save_tbl.h"  //EXGMDATA_ID_WBT_00
#include "savedata/save_control.h"

#include "wbt_setup.h"
#include "wbt_tool.h" //WBTTOOL_
#include "savedata/wbt_save.h"  //WBTSAVE
#include "app/wbt_list.h"
#include "event_wbt.h"  //WBT_EVENT_ShowTournamentList
#include "event_wbt_wifi.h"
#include "debug_wbt.h"

#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  //SYS_FLAG_GAMECLEAR
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
// DEBUG_WBT_WORK.Flag
enum{
	FLG_SET_MODE	= (1<<0),		// 独自入力モードへ移行
	//
	FLG_WIN_UPDATE	= (1<<1),		// アップデートが必要(初期化時のみ)
};

enum{
	TRAINER_PTN_MAX	= ( 81 ),		// 最大(関数から取得だろう)
	//
	DWBT_WIN_OFF_X	= (  1 ),		// offx
	DWBT_WIN_OFF_Y	= ( 16 ),		// offx
	DWBT_WIN_WIDTH	= ( 30 ),		// width
	DWBT_WIN_HEIGHT	= (  8 ),		// height
};

// DebugMode
enum{
	DEBUG_MODE_TR	= 0,			    ///< トレーナー選択
  DEBUG_MODE_FORCE_TR_MODE,   ///< 強制トレーナーリストモード選択
	DEBUG_MODE_REVERSE_JUDGE,		///< 不利判定を必勝にするモード選択
  DEBUG_MODE_FORCE_RENT_MODE, ///< 強制レンタルモード選択
  DEBUG_MODE_RENT,            ///< レンタルポケモン選択
	//
	DEBUG_MODE_MAX,					// max
};

#define WINCLR_COL(col) (((col)<<4)|(col))
//=============================================================================
/**
 *					グローバル
*/
//=============================================================================

static GMEVENT_RESULT debugMenuWBTEvent( GMEVENT *event, int *seq, void *wk );
//=============================================================================
//=============================================================================
typedef struct DEBUG_WBT_WORK DEBUG_WBT_WORK;
struct DEBUG_WBT_WORK{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  //ZUKAN_SAVEDATA * sv;
  BOOL (*call_func)( DEBUG_WBT_WORK * );
  WBTSTAGE stage;
  // win_record用
  WIN_RECORD_STATUS status;
  // t_download用
  T_DOWNLOAD_STATUS	t_status;
	//
	// 以下に追加していく予定
	s32			Step;							// 
	u32			Flag;							// フラグ
	u16			Trbuffer[WBT_TRAINER_NUM];		// セットするバッファ
	s32			Index;							// インデックス(0-8)
	s32			DebugMode;						// モード(0:トレーナー、1:debug)
	WORDSET		*wordset;						// WordSet
	GFL_MSGDATA	*msgman;						// メッセージマネージャー
	GFL_BMPWIN	*bmpwin;
	GFL_FONT	*fontHandle;					// Font
};

static BOOL debugMenuCallProc_WBTList_entry( DEBUG_WBT_WORK *p_wk );
static BOOL debugMenuCallProc_WBTList_1st( DEBUG_WBT_WORK *p_wk );
static BOOL debugMenuCallProc_WBTList_2nd( DEBUG_WBT_WORK *p_wk );
static BOOL debugMenuCallProc_WBTList( DEBUG_WBT_WORK *p_wk );
static BOOL debugCommand_printf( DEBUG_WBT_WORK *p_wk );
static void debugCommand_CreateWBTDLData( DEBUG_WBT_WORK * p_wk, const u16 * tr_list );
// test
static BOOL debugMenuCallProc_WinRecord( DEBUG_WBT_WORK *p_wk );
static BOOL debugMenuCallProc_TDownload( DEBUG_WBT_WORK *p_wk );

//
static BOOL debugMenuMain( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_tr( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_forceTrMode( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_reverseJudgeMode( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_forceRentMode( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_rentalPokemon( DEBUG_WBT_WORK *wk );
//
static void createNumWin( DEBUG_WBT_WORK * wk );
static void deleteNumWin( DEBUG_WBT_WORK * wk );
static void printNumWin(  DEBUG_WBT_WORK * wk, const s32 num, const s32 mnum, const s32 xpos, const s32 ypos, const s32 mode );
static void printTrNameWin( DEBUG_WBT_WORK * wk, const u16 tr_id, const s32 xpos, const s32 ypos );
static void printMsgStrWin( DEBUG_WBT_WORK * wk, const u16 msg_id, const s32 xpos, const s32 ypos );
static void updateNumWin( DEBUG_WBT_WORK * wk );
static void clearNumWin(  DEBUG_WBT_WORK * wk );
static void dispInfo_TrainerList( DEBUG_WBT_WORK * wk );
static void dispInfo_RentalList( DEBUG_WBT_WORK * wk );
static void dispInfo_toggleMode( DEBUG_WBT_WORK * wk, u32 msg_id, u32 mode_value );
static void dispInfoPrintf( DEBUG_WBT_WORK * wk );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER WBTMenuHeader =
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
static const DEBUG_MENU_LIST WBTMenuList[] =
{
  { DEBUG_STR_WBT_01, (void*)DEBUG_STR_WBT_01, GFL_OVERLAY_BLANK_ID },  // トレーナーせってい
  { DEBUG_STR_WBT_02, (void*)DEBUG_STR_WBT_02, GFL_OVERLAY_BLANK_ID },  // 逆転判定モード
  { DEBUG_STR_WBT_03, (void*)DEBUG_STR_WBT_03, GFL_OVERLAY_BLANK_ID },   // 強制トレーナーモード
  { DEBUG_STR_WBT_18, (void*)DEBUG_STR_WBT_18, GFL_OVERLAY_BLANK_ID },   // 強制レンタルモード
  { DEBUG_STR_WBT_19, (void*)DEBUG_STR_WBT_19, GFL_OVERLAY_BLANK_ID },   // レンタルポケモン設定
  { DEBUG_STR_WBT_04, (void*)DEBUG_STR_WBT_04, GFL_OVERLAY_BLANK_ID },   // トーナメント表アプリ
  { DEBUG_STR_WBT_05, (void*)DEBUG_STR_WBT_05, GFL_OVERLAY_BLANK_ID },   // トーナメント表アプリ
  { DEBUG_STR_WBT_06, (void*)DEBUG_STR_WBT_06, GFL_OVERLAY_BLANK_ID },   // トーナメント表アプリ
  { DEBUG_STR_WBT_07, (void*)DEBUG_STR_WBT_07, GFL_OVERLAY_BLANK_ID },   // デバッグ出力
  //
  { DEBUG_STR_WBT_08, (void*)DEBUG_STR_WBT_08, GFL_OVERLAY_BLANK_ID },   // エントリーチェック:1
  { DEBUG_STR_WBT_09, (void*)DEBUG_STR_WBT_09, GFL_OVERLAY_BLANK_ID },   // エントリーチェック:2
  { DEBUG_STR_WBT_10, (void*)DEBUG_STR_WBT_10, GFL_OVERLAY_BLANK_ID },   // エントリーチェック:3
  //
  { DEBUG_STR_WBT_12, (void*)DEBUG_STR_WBT_12, GFL_OVERLAY_BLANK_ID },   // ダウンロード：2
  { DEBUG_STR_WBT_13, (void*)DEBUG_STR_WBT_13, GFL_OVERLAY_BLANK_ID },   // ダウンロード：3
  { DEBUG_STR_WBT_15, (void*)DEBUG_STR_WBT_15, GFL_OVERLAY_BLANK_ID },   // 全トーナメント解禁
  { DEBUG_STR_WBT_16, (void*)DEBUG_STR_WBT_16, GFL_OVERLAY_BLANK_ID },   // 固定ＤＬデータ生成
  { DEBUG_STR_WBT_17, (void*)DEBUG_STR_WBT_17, GFL_OVERLAY_BLANK_ID },   // 可変ＤＬデータ生成
};

static const DEBUG_MENU_INITIALIZER DebugMenuWBTImitializer = {
  NARC_debug_message_debug_wbt_dat,
  NELEMS(WBTMenuList),
  WBTMenuList,
  &WBTMenuHeader,
  1, 1, 24, 16,
  NULL,
  NULL
};

//------------------------------------------------------------------
//------------------------------------------------------------------
// 初期パラメータ
static const u16 sample_trid[WBT_TRAINER_NUM] = {
  12,//アロエ
  53,//モブ
  11,//デント
  WBT_TRCODE_PLAYER,//プレイヤー
  55,//モブ
  56,//モブ
  18,//シロナ
  13,//ハチク
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const s16 dwbt_info_tbl[] = {
	4,		0,			// index
	//
	4,		2+16,		// index(0)
	4+60,	2+16,		// index(1)
	4+120,	2+16,		// index(2)
	4+180,	2+16,		// index(3)
	//
	4,		2+32,		// 項目番号(0)
	4+60,	2+32,		// 項目番号(1)
	4+120,	2+32,		// 項目番号(2)
	4+180,	2+32,		// 項目番号(3)
	//
	4,		2+48,		// 項目番号(00)
	4+60,	2+48,		// 項目番号(01)
	4+120,	2+48,		// 項目番号(02)
	4+180,	2+48,		// 項目番号(03)
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　WBT
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_WBT( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_WBT_WORK *work;

  u16 zone_id = FIELDMAP_GetZoneID( fieldWork );
  if ( zone_id != ZONE_ID_D05R0101 )
  {
    //オーバーレイがないものを呼び出さないように使用箇所を制限
    OS_Printf("デバッグメニューWBTはD05R0101以外では呼べません\n");
    PMSND_PlaySE( SEQ_SE_BEEP );
    return FALSE;
  }
  GMEVENT_Change( event, debugMenuWBTEvent, sizeof(DEBUG_WBT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_WBT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  
  //work->sv = GAMEDATA_GetWBTSave( GAMESYSTEM_GetGameData(wk->gmSys) );
	// 追加
	{
		s32		i;
		
		work->Flag 	= 0;
		work->Index = 0;
		work->Step	= 0;
		work->DebugMode = 0;
		// 値をセット
    if ( DEBUG_WBT_IsEmptyTrainerList() )
    { //値がない場合は初期化して設定
      TAMADA_Printf(" DEBUG_WBT_ForceTrList == Empty\n");
      for(i=0;i<WBT_TRAINER_NUM;i++){
        work->Trbuffer[i] = sample_trid[i];
      }
      DEBUG_WBT_SetTrainerList( work->Trbuffer );
      DEBUG_WBT_ForceTrMode = FALSE;  //最初は無効値に設定
      DEBUG_WBT_ReverseJudgeMode = FALSE; //最初は無効値に設定
    }
    else
    { //セット済みの場合は取得する
      TAMADA_Printf(" DEBUG_WBT_ForceTrList != Empty\n");
      DEBUG_WBT_GetTrainerList( work->Trbuffer );
    }

		// 大丈夫?
		work->fontHandle = FLDMSGBG_GetFontHandle( FIELDMAP_GetFldMsgBG( wk->fieldWork ) );
	}
  return TRUE;
}

//--------------------------------------------------------------
/**
 * イベント：WBT
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWBTEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_WBT_WORK *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuWBTImitializer );
    work->call_func = NULL;
    (*seq)++;
    break;

  case 1:

    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){
    case FLDMENUFUNC_NULL:    // 操作無し
      break;

    case FLDMENUFUNC_CANCEL:  // キャンセル
      (*seq)++;
      break;

    case DEBUG_STR_WBT_01:		// トレーナーせってい
    	*seq = 3;
    	work->DebugMode = DEBUG_MODE_TR;
      break;

    case DEBUG_STR_WBT_02:		// 逆転判定モード設定
		*seq = 3;
		work->DebugMode = DEBUG_MODE_REVERSE_JUDGE;
      break;

    case DEBUG_STR_WBT_03:  //強制トレーナーリストモード設定
    	*seq = 3;
    	work->DebugMode = DEBUG_MODE_FORCE_TR_MODE;
      break;

    case DEBUG_STR_WBT_18:  // 強制レンタルモード
      *seq = 3;
      work->DebugMode = DEBUG_MODE_FORCE_RENT_MODE;
      break;

    case DEBUG_STR_WBT_19:  // レンタルポケモン設定
      *seq = 3;
      work->DebugMode = DEBUG_MODE_RENT;
      break;

    case DEBUG_STR_WBT_04:
      work->call_func = debugMenuCallProc_WBTList;
      work->stage = WBTSTAGE_ENTRY;
      (*seq)++;
      break;
    case DEBUG_STR_WBT_05:
      work->call_func = debugMenuCallProc_WBTList;
      work->stage = WBTSTAGE_1ST;
      (*seq)++;
      break;
    case DEBUG_STR_WBT_06:
      work->call_func = debugMenuCallProc_WBTList;
      work->stage = WBTSTAGE_2ND;
      (*seq)++;
      break;
    //
    case DEBUG_STR_WBT_07:
#if 1
      PMSND_PlaySE( SEQ_SE_KON );
      debugCommand_printf( NULL );
      return GMEVENT_RES_CONTINUE;
#else
//      	work->call_func = debugMenuCallProc_TDownload;
//      	work->t_status = T_DOWNLOAD_STATUS_VIEW;
		(*seq)++;
		break;
#endif
	
    //
    // 追加
    //
    case DEBUG_STR_WBT_08:		// エントリーチェック:1
		work->call_func = debugMenuCallProc_WinRecord;
		work->status = WIN_RECORD_STATUS_LV1;
		(*seq)++;
		break;
    //
    case DEBUG_STR_WBT_09:		// エントリーチェック:2
      work->call_func = debugMenuCallProc_WinRecord;
      work->status = WIN_RECORD_STATUS_LV2;
      (*seq)++;
      break;
    //
    case DEBUG_STR_WBT_10:		// エントリーチェック:3
      work->call_func = debugMenuCallProc_WinRecord;
      work->status = WIN_RECORD_STATUS_LV3;
      (*seq)++;
      break;
    //
    case DEBUG_STR_WBT_12:		// ダウンロード：2(選択)
      work->call_func = debugMenuCallProc_TDownload;
      work->t_status = T_DOWNLOAD_STATUS_SELECT;
      (*seq)++;
      break;
    //
    case DEBUG_STR_WBT_13:		// ダウンロード：3(参加)
      work->call_func = debugMenuCallProc_TDownload;
      work->t_status = T_DOWNLOAD_STATUS_JOIN;
      (*seq)++;
      break;
    //
    case DEBUG_STR_WBT_15:   // 全トーナメント解禁
      {
			  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( work->gmSys );
        EVENTWORK * evwork = GAMEDATA_GetEventWork( gamedata );
        WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );

        EVENTWORK_SetEventFlag( evwork, SYS_FLAG_GAME_CLEAR );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_HODOMOE, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_ISSYU, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_KANTO, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_JOUTO, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_HOUEN, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_SINOU, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_RENTAL, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_SHUFFLE, 1 );
        WBTSAVE_SetWinCount( wbtsave, WBTWIN_COUNT_WORLD, 10 );
      }
      PMSND_PlaySE( SEQ_SE_KON );
      (*seq)++;
      break;

    case DEBUG_STR_WBT_16:
      debugCommand_CreateWBTDLData( work, NULL );
      PMSND_PlaySE( SEQ_SE_SAVE );
      (*seq) ++;
      break;
    case DEBUG_STR_WBT_17:
      debugCommand_CreateWBTDLData( work, work->Trbuffer );
      PMSND_PlaySE( SEQ_SE_SAVE );
      (*seq) ++;
      break;
    }
    break;


  case 2:
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    if (work->call_func != NULL
        && work->call_func( work ) == TRUE )
    {
      return GMEVENT_RES_CONTINUE;
    } else {
      return GMEVENT_RES_FINISH;
    }
    
  // 数値入力画面に移行
  // (内部で戻し)
  case 3:
	if( debugMenuMain( work ) == TRUE ){
    //修正結果を強制トレーナーリストに反映しておく
    DEBUG_WBT_SetTrainerList( work->Trbuffer );
		*seq = 1;
	}
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * WBTデバッグ出力
 */
//--------------------------------------------------------------
static BOOL debugCommand_printf( DEBUG_WBT_WORK *p_wk )
{
  OS_Printf("size of Regulation = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(REGULATION), sizeof(REGULATION) );
  OS_Printf("size of WBT DL Regulation = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBTDL_RULE), sizeof(WBTDL_RULE) );
  OS_Printf("size of WBT DL Pokemon = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBTDL_POKEMON), sizeof(WBTDL_POKEMON) );
  OS_Printf("size of WBT DL Character = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBTDL_CHARACTER), sizeof(WBTDL_CHARACTER) );
  OS_Printf("size of WBT DL DATA = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBTDL_DATA), sizeof(WBTDL_DATA) );
  OS_Printf("size of WBT SAVE DATA = \n");
  OS_Printf("%5d(0x%4x)\n", WBTSAVE_GetWorkSize(), WBTSAVE_GetWorkSize() );
  return FALSE;
}

//=============================================================================
//=============================================================================
#if 0
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void setUTF16( STRCODE * strcode, const u16 * utf16code, u32 buflen )
{
  int i;
  u16 eom_code = GFL_STR_GetEOMCode();

  GFL_STD_MemFill16( strcode, eom_code, buflen );
  for ( i = 0; i < buflen - 1; i ++ )
  {
    if ( utf16code[i] == '\0' ) break;
    strcode[i] = utf16code[i];
  }
}
#endif

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void setupWBTDLTournamentInfo( WBTDL_DATA * wbtdl_data,
    GFL_MSGDATA *msgman, STRBUF *tmpbuf, u16 title_id, u16 reg_id, u16 content_id)
{
  GFL_MSG_GetString( msgman, title_id, tmpbuf );
  GFL_STR_GetStringCode( tmpbuf, wbtdl_data->tournament_name, WBTDL_TOURNAMENT_NAME_LEN );
  GFL_MSG_GetString( msgman, reg_id, tmpbuf );
  GFL_STR_GetStringCode( tmpbuf, wbtdl_data->regulation_explain, WBTDL_TOURNAMENT_REG_EXPLAIN_LEN );
  GFL_MSG_GetString( msgman, content_id, tmpbuf );
  GFL_STR_GetStringCode( tmpbuf, wbtdl_data->contents_explain, WBTDL_TOURNAMENT_EXPLAIN_LEN );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void setupWBTDLChar(
    WBTDL_CHARACTER * dl_char, GFL_MSGDATA *msgman, STRBUF *tmpbuf, u8 num )
{
  static const u8 names[WBTDL_CHAR_MAX] = {
    DEBUG_STR_WBT_DL04,
    DEBUG_STR_WBT_DL05,
    DEBUG_STR_WBT_DL06,
    DEBUG_STR_WBT_DL07,
  };
  static const u8 titles[WBTDL_CHAR_MAX] = {
    DEBUG_STR_WBT_DL11,
    DEBUG_STR_WBT_DL12,
    DEBUG_STR_WBT_DL13,
    DEBUG_STR_WBT_DL14,
  };
  GFL_STD_MemClear32( dl_char, sizeof(WBTDL_CHARACTER) );
  GFL_MSG_GetString( msgman, names[num], tmpbuf );
  GFL_STR_GetStringCode( tmpbuf, dl_char->name, SAVELEN_TRAINER_NAME );
  GFL_MSG_GetString( msgman, titles[num], tmpbuf );
  GFL_STR_GetStringCode( tmpbuf, dl_char->title, WBTDL_TRAINER_TITLE_LEN );

  dl_char->trtype_id = num + 1;
  dl_char->msg_before_id = 1;
  dl_char->msg_after_win_id = 1;
  dl_char->msg_after_lose_id = 1;
  dl_char->msg_win_id = 1;
  dl_char->msg_lose_id = 1;
  dl_char->msg_announce_id = 1;
  dl_char->lang_id = PM_LANG;
  dl_char->ai_bit = 0x00000087;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void setupWBTDLPokemon( WBTDL_POKEMON * dl_poke,
    GFL_MSGDATA * msgman, STRBUF *tmpbuf, HEAPID tempHeapID, u16 monsno, u8 level, u64 id, u8 num )
{
  POKEMON_PARAM * pp;
  BOOL fast_flag;
  int i;
  static const u8 names[WBTDL_CHAR_MAX] = {
    DEBUG_STR_WBT_DL15,
    DEBUG_STR_WBT_DL16,
    DEBUG_STR_WBT_DL17,
    DEBUG_STR_WBT_DL18,
  };
  GFL_STD_MemClear32( dl_poke, sizeof(WBTDL_POKEMON) );

  GFL_MSG_GetString( msgman, names[num], tmpbuf );
  GFL_STR_GetStringCode( tmpbuf, dl_poke->nickname, MONS_NAME_SIZE + EOM_SIZE );

  pp = PP_Create( monsno, level, id, tempHeapID );
  fast_flag = PP_FastModeOn( pp );

  dl_poke->monsno = monsno;
  dl_poke->level = level;
  dl_poke->item_no = 0; //PP_Get( pp, ID_PARA_item, NULL );
  dl_poke->rare_flag = PP_CheckRare( pp );
  //努力値はすべて０として入れない
  for ( i = 0; i < PTL_WAZA_MAX; i ++ )
  {
    dl_poke->waza_id[i] = PP_Get( pp, ID_PARA_waza1 + i, NULL );
  }
  //パワー乱数を一気に入れたあと、性別を入れる
  dl_poke->power_rnd = PP_Get( pp, ID_PARA_pow_rnd, NULL );
  dl_poke->sex = PP_Get( pp, ID_PARA_sex, NULL );

  dl_poke->seikaku = PP_Get( pp, ID_PARA_seikaku, NULL );
  dl_poke->tokusei = PP_Get( pp, ID_PARA_speabino, NULL );
  dl_poke->friend_rate = PP_Get( pp, ID_PARA_friend, NULL );

  dl_poke->ball_id = PP_Get( pp, ID_PARA_get_ball, NULL );
  
  PP_FastModeOff( pp, fast_flag );
  GFL_HEAP_FreeMemory( pp );
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static void setupWBTDLRegulation( WBTDL_RULE * wbtdl_rule, u32 reg_id, HEAPID tempHeapID )
{
  int i;
  const REGULATION * sample = PokeRegulation_LoadDataAlloc( reg_id, tempHeapID );

  GFL_STD_MemClear( wbtdl_rule, sizeof(WBTDL_RULE) );
  wbtdl_rule->num_lo        = sample->NUM_LO;
  wbtdl_rule->num_hi        = sample->NUM_HI;
  wbtdl_rule->level         = sample->LEVEL;
  wbtdl_rule->level_range   = sample->LEVEL_RANGE;
  wbtdl_rule->level_total   = sample->LEVEL_TOTAL;
  wbtdl_rule->both_poke     = sample->BOTH_POKE;
  wbtdl_rule->both_item     = sample->BOTH_ITEM;
  for ( i = 0; i < REG_POKENUM_MAX_BYTE; i ++ )
  {
    wbtdl_rule->veto_poke_bit[i] = sample->VETO_POKE_BIT[i];
  }
  for ( i = 0; i < REG_ITEMNUM_MAX_BYTE; i ++ )
  {
    wbtdl_rule->veto_item_bit[i] = sample->VETO_ITEM[i];
  }
  wbtdl_rule->btl_rule      = sample->BATTLE_TYPE;

  //以下はswan拡張用
  wbtdl_rule->bgm_flag      = 0;
  wbtdl_rule->veto_waza_no  = WAZANO_HURIIFOORU;
  wbtdl_rule->need_poketype = POKETYPE_NULL;

  GFL_HEAP_FreeMemory( (void*)sample );
}

//----------------------------------------------------------------------------
//ダミーデータ生成
//----------------------------------------------------------------------------
static void createWBTDLData( WBTDL_DATA * wbtdl_data, const u16 * tr_list, HEAPID tempHeapID )
{
  GFL_MSGDATA	*msgman;						// メッセージマネージャー
  STRBUF * tmpbuf;
  msgman  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
              ARCID_DEBUG_MESSAGE, NARC_debug_message_debug_wbt_dat, tempHeapID);
  tmpbuf = GFL_STR_CreateBuffer( 128, tempHeapID );

  wbtdl_data->cassette_version = 0xffff;  //仮：全バージョン遊べる？
  wbtdl_data->lang_id = LANG_JAPAN;

  GFL_HEAP_CheckHeapSafe( tempHeapID );

  setupWBTDLRegulation( &(wbtdl_data->wbtdl_rule), REG_WBT_SINGLE, tempHeapID );

  GFL_HEAP_CheckHeapSafe( tempHeapID );

  if ( tr_list != NULL )
  { //トレーナー手持ちに関するデータは生成していない
    int count, index;
    for ( index = 0, count = 0; index < WBT_TRAINER_NUM; index ++ )
    {
      if ( tr_list[index] != WBT_TRCODE_PLAYER && count < WBTDL_MATCH_MAX )
      {
        static const u8 pri[WBT_TRAINER_NUM] = { 3,4,5,2, 1,1,1,1 };
        wbtdl_data->match[count].type = WBTDL_MATCH_TRAINER_ROM;
        wbtdl_data->match[count].pri  = pri[index];
        wbtdl_data->match[count].id   = tr_list[index];
        count ++;
      }
    }
    setupWBTDLTournamentInfo( wbtdl_data, msgman, tmpbuf,
        DEBUG_STR_WBT_DL08, DEBUG_STR_WBT_DL09, DEBUG_STR_WBT_DL10 );
    OS_Printf("WBT DL DATA: 設定Listから作成\n");
  }
  else
  {
    int i, j;
    for ( i = 0; i < WBT_TRAINER_NUM - 1; i ++ )
    {
      wbtdl_data->match[i].type = WBTDL_MATCH_TRAINER_ROM;
      wbtdl_data->match[i].pri  = 3;
      wbtdl_data->match[i].id   = i;
      if ( i < WBTDL_CHAR_MAX )
      {
        wbtdl_data->match[i].type = WBTDL_MATCH_TRAINER_DATA;
        wbtdl_data->match[i].pri  = 3;
        wbtdl_data->match[i].id   = i;
        setupWBTDLChar( &(wbtdl_data->character[i]), msgman, tmpbuf, i );
        for ( j = 0; j < TEMOTI_POKEMAX; j ++)
        {
          setupWBTDLPokemon( &(wbtdl_data->character[i].pokemon[j]), msgman, tmpbuf,
                tempHeapID, 6 * i + j + 1, 50, PTL_SETUP_ID_AUTO, j );
        }
        GFL_HEAP_CheckHeapSafe( tempHeapID );
      }
    }
    setupWBTDLTournamentInfo( wbtdl_data, msgman, tmpbuf,
        DEBUG_STR_WBT_DL01, DEBUG_STR_WBT_DL02, DEBUG_STR_WBT_DL03 );
    OS_Printf("WBT DL DATA: 固定データから作成\n");
  }
  WBTEXSAVE_SetCrc( (WBTEXSAVE*)wbtdl_data );

  GFL_STR_DeleteBuffer( tmpbuf );
  GFL_MSG_Delete( msgman );
}

//----------------------------------------------------------------------------
//セーブ
//----------------------------------------------------------------------------
static void saveWBTDLData( const WBTDL_DATA * wbtdl_data, GAMESYS_WORK * gsys, SAVE_EXTRA_ID extra_id )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  LOAD_RESULT load_res;
  SAVE_RESULT save_ret;
  WBTEXSAVE * save_p;
  SAVE_CONTROL_WORK* save;

  save = GAMEDATA_GetSaveControlWork( gamedata );
  load_res = SaveControl_Extra_Load( save, extra_id, HEAPID_PROC );
  OS_Printf("WBT DL Data Load Result = %d\n", load_res );
  save_p = SaveControl_Extra_DataPtrGet( save, extra_id, EXGMDATA_ID_WBT_00 );
  GFL_STD_MemCopy32( wbtdl_data, save_p, sizeof(WBTDL_DATA) );

  GAMEDATA_ExtraSaveAsyncStart( gamedata, extra_id ); 

  do {
    save_ret = GAMEDATA_ExtraSaveAsyncMain( gamedata, extra_id ); 
  } while ( save_ret != SAVE_RESULT_OK && save_ret != SAVE_RESULT_NG );
  OS_Printf("WBT DL Data Save Result = %d\n", save_ret );

  // セーブ終了
  SaveControl_Extra_Unload( save, extra_id ); // 外部データを解放
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void debugCommand_CreateWBTDLData( DEBUG_WBT_WORK * p_wk, const u16 * tr_list )
{
  HEAPID tempHeapID = p_wk->heapID;
  WBTDL_DATA * wbtdl_data;
  
  wbtdl_data = GFL_HEAP_AllocMemoryLo( tempHeapID, sizeof(WBTDL_DATA) );
  GFL_STD_MemClear32( wbtdl_data, sizeof(WBTDL_DATA) );
  GFL_HEAP_CheckHeapSafe( tempHeapID );

  createWBTDLData( wbtdl_data, tr_list, tempHeapID );

  saveWBTDLData( wbtdl_data, p_wk->gmSys, SAVE_EXTRA_ID_WBT_00 );

  GFL_HEAP_CheckHeapSafe( tempHeapID );
  GFL_HEAP_FreeMemory( wbtdl_data );
}

//=============================================================================
//  デバッグメニュー WBTリスト表示へ
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用WBTトーナメントリスト表示イベント
 *
 *  @param  DEBUG_WBT_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_WBTList( DEBUG_WBT_WORK *p_wk )
{
  GMEVENT * new_event;
  WBTLIST_PARAM * wbtlist_param;
  wbtlist_param = WBTLIST_PARAM_Create( HEAPID_PROC, p_wk->gmSys );
  WBTLIST_PARAM_Setup_Debug(
      p_wk->gmSys, wbtlist_param,
      WBTCUP_HODOMOE, WBTRULE_SINGLE, p_wk->stage, p_wk->Trbuffer );
  wbtlist_param->call_fade_flag = TRUE; //呼び出し前後でフィールドフェード実行
  new_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_wbt), WBT_EVENT_ShowTournamentList, (void *)wbtlist_param );
  GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );

  OS_Printf( "WBT List Start\n" );

  return TRUE;

}

//=============================================================================
//
//
//
//=============================================================================
//----------------------------------------------------------------------------
// 以下に追加していく予定
// (event_debug_num_input)からほぼ持ってきた
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用処理(Main)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return BOOL			(FALSE:処理中、TRUE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuMain( DEBUG_WBT_WORK *wk )
{
	BOOL	bflg = FALSE;
	
	switch( wk->Step ){
		// 初期化
		case 0:
			wk->Step = 1;
			// ウインドウ作成
			createNumWin( wk );
			wk->Flag |= FLG_WIN_UPDATE;
			break;
		
		// 通常
		case 1:
			// 入力処理
			if( debugMenuKeyInput( wk ) == TRUE ){
				wk->Step = 2;
			}
			break;
		
		// 終了
		case 2:
			wk->Step = 0;
			// ウインドウ破棄
			deleteNumWin( wk );
			// 情報表示
			dispInfoPrintf( wk );
			bflg = TRUE;
			break;
		default:
			break;
	}
	return( bflg );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用処理(入力画面)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return BOOL			(FALSE:処理中、TRUE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuKeyInput( DEBUG_WBT_WORK *wk )
{
	BOOL	bflg=FALSE;
	
	// 入力(トレーナー選択)
  switch ( wk->DebugMode )
  {
  case DEBUG_MODE_TR:
		bflg = debugMenuKeyInput_tr( wk );
    break;
  case DEBUG_MODE_FORCE_TR_MODE:
		bflg = debugMenuKeyInput_forceTrMode( wk );
    break;
  case DEBUG_MODE_REVERSE_JUDGE:
		bflg = debugMenuKeyInput_reverseJudgeMode( wk );
    break;
  case DEBUG_MODE_FORCE_RENT_MODE:
    bflg = debugMenuKeyInput_forceRentMode( wk );
    break;
  case DEBUG_MODE_RENT:
    bflg = debugMenuKeyInput_rentalPokemon( wk );
    break;
  default:
    GF_ASSERT( 0 );
	}
	return( bflg );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用処理(入力画面:トレーナー選択)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return BOOL			(FALSE:処理中、TRUE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuKeyInput_tr( DEBUG_WBT_WORK *wk )
{
	u16		tmp;
	s16		flg=0;
	BOOL	bflg=FALSE;
	
	// 戻し
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		// 戻し
		wk->Flag &= ~FLG_SET_MODE;
		bflg = TRUE;
//		OS_Printf("end\n");
	}
	// 上
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		wk->Index--;
		if(wk->Index < 0){
			wk->Index = WBT_TRAINER_NUM-1;
		}
		flg = 1;
	}
	// 下
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		wk->Index++;
		if(wk->Index >= WBT_TRAINER_NUM){
			wk->Index = 0;
		}
		flg = 1;
	}
	// 左(減らす)
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		// プレイヤの場合=>最大番号に変更
		tmp = wk->Trbuffer[wk->Index];
		if(tmp == WBT_TRCODE_PLAYER){
			wk->Trbuffer[wk->Index] = TRAINER_PTN_MAX;
		}
		else{
			if(tmp == 0){
				// プレイヤにする
				wk->Trbuffer[wk->Index] = WBT_TRCODE_PLAYER;
			}else{
				wk->Trbuffer[wk->Index]--;
			}
		}
		flg = 1;
	}
	// 右(増やす)
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		// プレイヤの場合=>最大番号に変更
		tmp = wk->Trbuffer[wk->Index];
		if(tmp == WBT_TRCODE_PLAYER){
			wk->Trbuffer[wk->Index] = 0;
		}
		else{
			if(tmp == TRAINER_PTN_MAX){
				// プレイヤにする
				wk->Trbuffer[wk->Index] = WBT_TRCODE_PLAYER;
			}else{
				wk->Trbuffer[wk->Index]++;
			}
		}
		flg = 1;
	}
	// 更新
	if(wk->Flag & FLG_WIN_UPDATE){
		flg = 1;
	}
	
	// 更新
	if(flg == 1){
//		OS_Printf("index, num:%d,%d\n", wk->Index, wk->Trbuffer[wk->Index]);
		// 表示まとめ
		dispInfo_TrainerList( wk );
		wk->Flag &= ~FLG_WIN_UPDATE;
	}
	return( bflg );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static BOOL debugMenuKeyInput_rentalPokemon( DEBUG_WBT_WORK *wk )
{
  enum {
    RENT_POKE_MIN = 1,
    RENT_POKE_MAX = 908,
  };
	s16		flg=0;
	BOOL	bflg=FALSE;
  s32 tmp;
	
  if ( wk->Index >= TEMOTI_POKEMAX ) wk->Index = 0;
	// 戻し
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		bflg = TRUE;
	}
	// 上
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		wk->Index--;
		if(wk->Index < 0){
			wk->Index = TEMOTI_POKEMAX-1;
		}
		flg = 1;
	}
	// 下
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		wk->Index++;
		if(wk->Index >= TEMOTI_POKEMAX){
			wk->Index = 0;
		}
		flg = 1;
	}
	// 左(減らす)
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
    tmp = DEBUG_WBT_ForceRental[wk->Index] - 1;
    if(tmp < RENT_POKE_MIN){
      tmp = RENT_POKE_MAX;
		}
    DEBUG_WBT_ForceRental[wk->Index] = tmp;
		flg = 1;
	}
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L ){
    tmp = DEBUG_WBT_ForceRental[wk->Index] - 10;
    if(tmp < RENT_POKE_MIN){
      tmp = RENT_POKE_MAX;
		}
    DEBUG_WBT_ForceRental[wk->Index] = tmp;
		flg = 1;
  }
	// 右(増やす)
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
    tmp = DEBUG_WBT_ForceRental[wk->Index] + 1;
    if(tmp > RENT_POKE_MAX){
      tmp = RENT_POKE_MIN;
		}
    DEBUG_WBT_ForceRental[wk->Index] = tmp;
		flg = 1;
	}
  else if ( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R ){
    tmp = DEBUG_WBT_ForceRental[wk->Index] + 10;
    if(tmp > RENT_POKE_MAX){
      tmp = RENT_POKE_MIN;
		}
    DEBUG_WBT_ForceRental[wk->Index] = tmp;
		flg = 1;
  }

	// 更新
	if(wk->Flag & FLG_WIN_UPDATE){
		flg = 1;
	}
	
	// 更新
	if(flg == 1){
		// 表示まとめ
		dispInfo_RentalList( wk );
		wk->Flag &= ~FLG_WIN_UPDATE;
	}
	return( bflg );
}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用処理(入力画面:デバッグ)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return BOOL			(FALSE:処理中、TRUE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuKeyInput_forceTrMode( DEBUG_WBT_WORK *wk )
{
	s16		flg=0;
	BOOL	bflg=FALSE;
	
	// 戻し
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		// 戻し
		wk->Flag &= ~FLG_SET_MODE;
		bflg = TRUE;
//		OS_Printf("end\n");
	}
  else if ( (GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A )) != 0 )
  { //モードを反転
    DEBUG_WBT_ForceTrMode = !(DEBUG_WBT_ForceTrMode);
		flg = 1;
  }
	
	// 更新
	if(wk->Flag & FLG_WIN_UPDATE){
		flg = 1;
	}
	
	// 更新
	if(flg == 1){
		// 表示まとめ
    dispInfo_toggleMode( wk, DEBUG_STR_WBT_03, DEBUG_WBT_ForceTrMode );
		wk->Flag &= ~FLG_WIN_UPDATE;
	}
	return( bflg );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用処理(入力画面:デバッグ)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return BOOL			(FALSE:処理中、TRUE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuKeyInput_reverseJudgeMode( DEBUG_WBT_WORK *wk )
{
	s16		flg=0;
	BOOL	bflg=FALSE;
	
	// 戻し
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		// 戻し
		wk->Flag &= ~FLG_SET_MODE;
		bflg = TRUE;
//		OS_Printf("end\n");
	}
  else if ( (GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A )) != 0 )
  { //モードを反転
    DEBUG_WBT_ReverseJudgeMode = !(DEBUG_WBT_ReverseJudgeMode);
		flg = 1;
  }
	
	// 更新
	if(wk->Flag & FLG_WIN_UPDATE){
		flg = 1;
	}
	
	// 更新
	if(flg == 1){
		// 表示まとめ
    dispInfo_toggleMode( wk, DEBUG_STR_WBT_02, DEBUG_WBT_ReverseJudgeMode );
		wk->Flag &= ~FLG_WIN_UPDATE;
	}
	return( bflg );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用処理(入力画面:強制レンタルモード)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return BOOL			(FALSE:処理中、TRUE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuKeyInput_forceRentMode( DEBUG_WBT_WORK *wk )
{
	s16		flg=0;
	BOOL	bflg=FALSE;
	
	// 戻し
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		// 戻し
		wk->Flag &= ~FLG_SET_MODE;
		bflg = TRUE;
//		OS_Printf("end\n");
	}
  else if ( (GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A )) != 0 )
  { //モードを反転
    DEBUG_WBT_ForceRentalMode = !(DEBUG_WBT_ForceRentalMode);
		flg = 1;
  }
	
	// 更新
	if(wk->Flag & FLG_WIN_UPDATE){
		flg = 1;
	}
	
	// 更新
	if(flg == 1){
		// 表示まとめ
    dispInfo_toggleMode( wk, DEBUG_STR_WBT_02, DEBUG_WBT_ForceRentalMode );
		wk->Flag &= ~FLG_WIN_UPDATE;
	}
	return( bflg );
}
//----------------------------------------------------------------------------
/**
 *  @brief	数値入力ウインドウ：生成
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void createNumWin( DEBUG_WBT_WORK * wk )
{
	GFL_BMP_DATA	*bmp;
	GFL_BMPWIN		*bmpwin;
	
	wk->wordset = WORDSET_Create( wk->heapID );
	wk->msgman  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
							ARCID_DEBUG_MESSAGE, NARC_debug_message_d_numinput_dat, wk->heapID);
	
	bmpwin = GFL_BMPWIN_Create( FLDBG_MFRM_MSG, DWBT_WIN_OFF_X, DWBT_WIN_OFF_Y, DWBT_WIN_WIDTH, DWBT_WIN_HEIGHT, 14, GFL_BMP_CHRAREA_GET_B );
	wk->bmpwin = bmpwin;
	
	bmp = GFL_BMPWIN_GetBmp( bmpwin );
	
	GFL_BMP_Clear( bmp, WINCLR_COL( 15 ) );
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BMPWIN_TransVramCharacter( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );
}

//----------------------------------------------------------------------------
/**
 *  @brief	数値入力ウインドウ：削除
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void deleteNumWin( DEBUG_WBT_WORK * wk )
{
	GFL_BMPWIN * bmpwin = wk->bmpwin;
	
	GFL_BMPWIN_ClearScreen( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );
	
	GFL_BMPWIN_Delete( bmpwin );
	
	GFL_MSG_Delete( wk->msgman );
	WORDSET_Delete( wk->wordset );
}

//----------------------------------------------------------------------------
/**
 *  @brief	数値入力ウインドウ：表示のみ
 *
 *  @param  DEBUG_WBT_WORK *wk
 *  @param  const s32		数値
 *  @param  const s32		桁数
 *  @param  const s32		xpos
 *  @param  const s32		ypos
 *  @param  const s32		(0:通常、1:色変え(赤))
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void printNumWin( DEBUG_WBT_WORK * wk, const s32 num, const s32 mnum, const s32 xpos, const s32 ypos, const s32 mode )
{
	GFL_BMPWIN * bmpwin = wk->bmpwin;
	
	STRBUF *expandBuf	= GFL_STR_CreateBuffer( 64, wk->heapID );
	// 値セット
	STRTOOL_SetNumber( expandBuf, num, mnum, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	if(mode == 0){			// 通常
		GFL_FONTSYS_SetColor( 1, 2, 15 );
	}else{					// 色変え
		GFL_FONTSYS_SetColor( 3, 2, 15 );
	}
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( bmpwin ), xpos, ypos, expandBuf, wk->fontHandle );
	//
	GFL_STR_DeleteBuffer( expandBuf );
}

//----------------------------------------------------------------------------
/**
 *  @brief	トレーナー名表示
 *
 *  @param  DEBUG_WBT_WORK *wk
 *  @param  const u16		トレーナーのID
 *  @param  const s32		xpos
 *  @param  const s32		ypos
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void printTrNameWin( DEBUG_WBT_WORK * wk, const u16 tr_id, const s32 xpos, const s32 ypos )
{
	GFL_BMPWIN * bmpwin = wk->bmpwin;
	
	STRBUF *strbuf	= GFL_STR_CreateBuffer( 64, wk->heapID );
	//
	// プレイヤは要確認
	if(tr_id == WBT_TRCODE_PLAYER){
		{
			GAMEDATA * gamedata = GAMESYSTEM_GetGameData( wk->gmSys );
			const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
			MyStatus_CopyNameString( mystat, strbuf );
		}
	}else{
		// トレーナー名を取得
		WBTTOOL_GetRomTrainerName( wk->heapID, tr_id, strbuf );
	}
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( bmpwin ), xpos, ypos, strbuf, wk->fontHandle );
	//
	GFL_STR_DeleteBuffer( strbuf );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void printMsgStrWin( DEBUG_WBT_WORK * wk, const u16 msg_id, const s32 xpos, const s32 ypos )
{
	GFL_BMPWIN * bmpwin = wk->bmpwin;
	STRBUF *strbuf	= GFL_STR_CreateBuffer( 64, wk->heapID );
  GFL_MSGDATA * msgman;
	//
  msgman  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
              ARCID_DEBUG_MESSAGE, NARC_debug_message_debug_wbt_dat, wk->heapID);
  GFL_MSG_GetString( msgman, msg_id, strbuf );
  GFL_MSG_Delete( msgman );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( bmpwin ), xpos, ypos, strbuf, wk->fontHandle );
	//
	GFL_STR_DeleteBuffer( strbuf );
}
//----------------------------------------------------------------------------
/**
 *  @brief	数値入力ウインドウ：更新のみ
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void updateNumWin( DEBUG_WBT_WORK * wk )
{
	GFL_BMPWIN * bmpwin = wk->bmpwin;
	
	GFL_BMPWIN_TransVramCharacter( bmpwin );					//transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );	//transfer screen
}

//----------------------------------------------------------------------------
/**
 *  @brief	数値入力ウインドウ：画面クリア(updateはかけること)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void clearNumWin( DEBUG_WBT_WORK * wk )
{
	GFL_BMPWIN * bmpwin = wk->bmpwin;
	GFL_BMP_Clear(GFL_BMPWIN_GetBmp( bmpwin ), WINCLR_COL( 15 ) );
}

//----------------------------------------------------------------------------
/**
 *  @brief	表示項目まとめ(レンタルポケモン)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void dispInfo_RentalList( DEBUG_WBT_WORK * wk )
{
	s32		i;
	s32		tmp;
	s32		mode=0;
	
	// クリア
	clearNumWin( wk );
	//
	// index
	//
	// 番号を表示
	for(i=0;i<3;i++){
    s32 mode1 = 0, mode2 = 0;
    if ( wk->Index < 3 ) {
      if ( wk->Index == i ) mode1 = 1;
    } else {
      if ( wk->Index - 3 == i ) mode2 = 1;
    }
	  printNumWin( wk, 0+i, 2, dwbt_info_tbl[0]+60*i, dwbt_info_tbl[1], mode1 );
		printNumWin( wk, DEBUG_WBT_ForceRental[0+i], 5, dwbt_info_tbl[0]+60*i, dwbt_info_tbl[1]+16, 0 );
	  printNumWin( wk, 3+i, 2, dwbt_info_tbl[0]+60*i, dwbt_info_tbl[1]+32, mode2 );
		printNumWin( wk, DEBUG_WBT_ForceRental[3+i], 5, dwbt_info_tbl[0]+60*i, dwbt_info_tbl[1]+48, 0 );
	}
	// 更新
	updateNumWin( wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief	表示項目まとめ(トレーナー)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void dispInfo_TrainerList( DEBUG_WBT_WORK * wk )
{
	s32		i;
	s32		tmp;
	s32		mode=0;
	
	// クリア
	clearNumWin( wk );
	//
	// index
	printNumWin( wk, wk->Index, 2, dwbt_info_tbl[0], dwbt_info_tbl[1], 0 );
	//
	// offsetを求める
	tmp = wk->Index/4;
	tmp *= 4;
	//
	// indexを表示
	for(i=0;i<4;i++){
		mode = 0;
		if(wk->Index == tmp+i){
			mode = 1;
		}
		printNumWin( wk, tmp+i, 2, dwbt_info_tbl[2+2*i], dwbt_info_tbl[2+2*i+1], mode );
	}
	// 番号を表示
	for(i=0;i<4;i++){
		printNumWin( wk, wk->Trbuffer[tmp+i], 5, dwbt_info_tbl[2+2*(4+i)], dwbt_info_tbl[2+2*(4+i)+1], 0 );
	}
	// 名前を表示
	for(i=0;i<4;i++){
		printTrNameWin( wk, wk->Trbuffer[tmp+i], dwbt_info_tbl[2+2*(8+i)], dwbt_info_tbl[2+2*(8+i)+1] );
	}
	// 更新
	updateNumWin( wk );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void dispInfo_toggleMode( DEBUG_WBT_WORK * wk, u32 msg_id, u32 mode_value )
{
	s32		mode=0;
	
	// クリア
	clearNumWin( wk );
	//
	printMsgStrWin( wk, msg_id, 4,		0 );			// index
	printNumWin( wk, mode_value, 1, 4,		2+16, 0 );		// index(0)
	// 更新
	updateNumWin( wk );
}

//----------------------------------------------------------------------------
/**
 *  @brief	表示項目まとめ(debug)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void dispInfo_reverseJudgeMode( DEBUG_WBT_WORK * wk )
{
  dispInfo_toggleMode( wk, DEBUG_STR_WBT_02, DEBUG_WBT_ReverseJudgeMode );
}

//----------------------------------------------------------------------------
/**
 *  @brief	表示項目まとめ(printf)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void dispInfoPrintf( DEBUG_WBT_WORK * wk )
{
	s32		i;
	
	// 情報表示
	for(i=0;i<WBT_TRAINER_NUM;i++){
		OS_Printf("%d :%d\n",i,wk->Trbuffer[i]);
	}
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用エントリーチェッカー表示イベント
 *
 *  @param  DEBUG_WBT_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_WinRecord( DEBUG_WBT_WORK *p_wk )
{
	GMEVENT 			*new_event;
	WIN_RECORD_PARAM 	*wr_param;
	wr_param = WIN_RECORD_PARAM_Create( HEAPID_PROC, p_wk->gmSys, NULL );
	//
	// なくても大丈夫なので保留
//	WIN_RECORD_PARAM_Setup_Debug(
//		p_wk->gmSys, wr_param,
//		WBTTYPE_ALL, WBTRULE_SINGLE, p_wk->stage, p_wk->Trbuffer, p_wk->DebugNum );
	// debug用
	wr_param->status = p_wk->status;
	//
	new_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_wbt), WIN_RECORD_EVENT_Show, (void *)wr_param );
	GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );
	//
	OS_Printf( "Win Record Start\n" );
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用トーナメントダウンロードイベント
 *
 *  @param  DEBUG_WBT_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_TDownload( DEBUG_WBT_WORK *p_wk )
{
	GMEVENT 			*new_event;
	T_DOWNLOAD_PARAM 	*td_param;

  if( p_wk->t_status == T_DOWNLOAD_STATUS_SELECT )
  {
    new_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
        FS_OVERLAY_ID(event_wbt_download), T_DOWNLOAD_EVENT_ShowWifi, NULL );
  }
  else
  {
    td_param = T_DOWNLOAD_PARAM_Create( HEAPID_PROC, p_wk->gmSys, p_wk->t_status, NULL, NULL );
    //
    // なくても大丈夫なので保留
    //	WIN_RECORD_PARAM_Setup_Debug(
    //		p_wk->gmSys, wr_param,
    //		WBTTYPE_ALL, WBTRULE_SINGLE, p_wk->stage, p_wk->Trbuffer, p_wk->DebugNum );
    // debug用
    //td_param->status = p_wk->t_status;
    //
    new_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
        FS_OVERLAY_ID(notwifi_event_wbt), T_DOWNLOAD_EVENT_Show, (void *)td_param );
  }
	GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );
	//
	OS_Printf( "T Download Start\n" );
	return TRUE;
}

#endif  //  PM_DEBUG

