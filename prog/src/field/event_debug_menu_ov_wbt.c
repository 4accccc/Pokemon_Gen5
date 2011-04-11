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

//アプリ

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 

#include "msg/debug/msg_debug_wbt.h"

//外部公開
#include "event_debug_menu_ov_wbt.h"

#include "wbt_dl.h"

#include "savedata/wbt_save.h"  //WBTSAVE
#include "app/wbt_list.h"
#include "event_wbt.h"  //WBT_EVENT_ShowTournamentList

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
	TRAINER_PTN_MAX	= ( 56 ),		// 最大(関数から取得だろう)
	//
	DWBT_WIN_OFF_X	= (  1 ),		// offx
	DWBT_WIN_OFF_Y	= ( 16 ),		// offx
	DWBT_WIN_WIDTH	= ( 30 ),		// width
	DWBT_WIN_HEIGHT	= (  8 ),		// height
};

// DebugMode
enum{
	DEBUG_MODE_TR	= 0,			// トレーナー選択
	DEBUG_MODE_DBG,					// debug
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
	//
	// 以下に追加していく予定
	s32			Step;							// 
	u32			Flag;							// フラグ
	u16			Trbuffer[WBT_TRAINER_NUM];		// セットするバッファ
	s32			Index;							// インデックス(0-8)
	s32			DebugNum;						// デバッグ
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
//
static BOOL debugMenuMain( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_tr( DEBUG_WBT_WORK *wk );
static BOOL debugMenuKeyInput_debug( DEBUG_WBT_WORK *wk );
//
static void createNumWin( DEBUG_WBT_WORK * wk );
static void deleteNumWin( DEBUG_WBT_WORK * wk );
static void printNumWin(  DEBUG_WBT_WORK * wk, const s32 num, const s32 mnum, const s32 xpos, const s32 ypos, const s32 mode );
static void printTrNameWin( DEBUG_WBT_WORK * wk, const u16 tr_id, const s32 xpos, const s32 ypos );
static void updateNumWin( DEBUG_WBT_WORK * wk );
static void clearNumWin(  DEBUG_WBT_WORK * wk );
static void dispInfo( DEBUG_WBT_WORK * wk );
static void dispInfo_debug( DEBUG_WBT_WORK * wk );
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
  { DEBUG_STR_WBT_01, (void*)DEBUG_STR_WBT_01, GFL_OVERLAY_BLANK_ID },  // データ構造チェック
  { DEBUG_STR_WBT_02, (void*)DEBUG_STR_WBT_02, GFL_OVERLAY_BLANK_ID },  // デバッグ出力
  { DEBUG_STR_WBT_03, (void*)DEBUG_STR_WBT_03, GFL_OVERLAY_BLANK_ID },   // バックアップ読み込み
  { DEBUG_STR_WBT_04, (void*)DEBUG_STR_WBT_04, GFL_OVERLAY_BLANK_ID },   // トーナメント表アプリ
  { DEBUG_STR_WBT_05, (void*)DEBUG_STR_WBT_05, GFL_OVERLAY_BLANK_ID },   // トーナメント表アプリ
  { DEBUG_STR_WBT_06, (void*)DEBUG_STR_WBT_06, GFL_OVERLAY_BLANK_ID },   // トーナメント表アプリ
  { DEBUG_STR_WBT_07, (void*)DEBUG_STR_WBT_07, GFL_OVERLAY_BLANK_ID },   // デバッグあり
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
  0xffff,//プレイヤー
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
		work->DebugNum	= 0;
		work->DebugMode = 0;
		// 値をセット
		for(i=0;i<WBT_TRAINER_NUM;i++){
			work->Trbuffer[i] = sample_trid[i];
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

    case DEBUG_STR_WBT_02:
      PMSND_PlaySE( SEQ_SE_KON );
      debugCommand_printf( NULL );
      return GMEVENT_RES_CONTINUE;

    case DEBUG_STR_WBT_03:
      (*seq)++;
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
    case DEBUG_STR_WBT_07:		// デバッグあり
		*seq = 3;
		work->DebugMode = DEBUG_MODE_DBG;
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
  OS_Printf("size of WBT DL Regulation = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBT_DL_RULE), sizeof(WBT_DL_RULE) );
  OS_Printf("size of WBT DL Pokemon = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBT_DL_POKEMON), sizeof(WBT_DL_POKEMON) );
  OS_Printf("size of WBT DL Character = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBT_DL_CHARACTER), sizeof(WBT_DL_CHARACTER) );
  OS_Printf("size of WBT DL DATA = \n");
  OS_Printf("%5d(0x%4x)\n", sizeof(WBT_DL_DATA), sizeof(WBT_DL_DATA) );
  OS_Printf("size of WBT SAVE DATA = \n");
  OS_Printf("%5d(0x%4x)\n", WBTSAVE_GetWorkSize(), WBTSAVE_GetWorkSize() );
  return FALSE;
}

//======================================================================
//  デバッグメニュー WBTリスト表示へ
//======================================================================
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
      WBTTYPE_ALL, WBTRULE_SINGLE, p_wk->stage, p_wk->Trbuffer, p_wk->DebugNum );
  new_event = WBT_EVENT_ShowTournamentList( p_wk->gmSys, p_wk->fieldWork, wbtlist_param, TRUE );
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
	if(wk->DebugMode == DEBUG_MODE_TR){
		bflg = debugMenuKeyInput_tr( wk );
	}else{
		bflg = debugMenuKeyInput_debug( wk );
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
		if(tmp == 0xffff){
			wk->Trbuffer[wk->Index] = TRAINER_PTN_MAX;
		}
		else{
			if(tmp == 0){
				// プレイヤにする
				wk->Trbuffer[wk->Index] = 0xffff;
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
		if(tmp == 0xffff){
			wk->Trbuffer[wk->Index] = 0;
		}
		else{
			if(tmp == TRAINER_PTN_MAX){
				// プレイヤにする
				wk->Trbuffer[wk->Index] = 0xffff;
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
		dispInfo( wk );
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
static BOOL debugMenuKeyInput_debug( DEBUG_WBT_WORK *wk )
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
	// 左(減らす)
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		wk->DebugNum--;
		if(wk->DebugNum < 0){
			wk->DebugNum = (DEBUG_MODE_MAX-1);
		}
		flg = 1;
	}
	// 右(増やす)
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		wk->DebugNum++;
		if(wk->DebugNum > (DEBUG_MODE_MAX-1)){
			wk->DebugNum = 0;
		}
		flg = 1;
	}
	// 更新
	if(wk->Flag & FLG_WIN_UPDATE){
		flg = 1;
	}
	
	// 更新
	if(flg == 1){
		// 表示まとめ
		dispInfo_debug( wk );
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
	if(tr_id == 0xffff){
		{
			GAMEDATA * gamedata = GAMESYSTEM_GetGameData( wk->gmSys );
			const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
			MyStatus_CopyNameString( mystat, strbuf );
		}
	}else{
		// トレーナー名を取得
		WBTTOOL_SetRomTrainerName( wk->heapID, tr_id, strbuf );
	}
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
 *  @brief	表示項目まとめ(トレーナー)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void dispInfo( DEBUG_WBT_WORK * wk )
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
/**
 *  @brief	表示項目まとめ(debug)
 *
 *  @param  DEBUG_WBT_WORK *wk
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void dispInfo_debug( DEBUG_WBT_WORK * wk )
{
	s32		i;
	s32		tmp;
	s32		mode=0;
	
	// クリア
	clearNumWin( wk );
	//
	// index
	printNumWin( wk, wk->DebugNum, 2, dwbt_info_tbl[0], dwbt_info_tbl[1], 0 );
	//
	// 更新
	updateNumWin( wk );
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


#if 0
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
#endif
