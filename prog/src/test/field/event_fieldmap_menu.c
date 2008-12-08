//======================================================================
/**
 *
 * @file	event_fieldmap_menu.c
 * @brief	フィールドマップメニューの制御イベント
 * @author	kagaya
 * @date	2008.11.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "message.naix"
#include "msg/msg_fldmapmenu.h"
#include "test_graphic/d_taya.naix"
#include "test/easy_pokelist.h"

#include "event_fieldmap_menu.h"
#include "system/main.h"

//======================================================================
//	define
//======================================================================
#define BGFRAME_MENU	(GFL_BG_FRAME1_M)	//使用フレーム
#define MENU_PANO (14)						//メニューパレットNo
#define FONT_PANO (15)						//フォントパレットNo
#define MENU_CHARSIZE_X (8)					//メニュー横幅
#define MENU_CHARSIZE_Y (16)				//メニュー縦幅

//--------------------------------------------------------------
///	FMENURET
//--------------------------------------------------------------
typedef enum
{
	FMENURET_CONTINUE,
	FMENURET_FINISH,
	FMENURET_NEXTEVENT,
}FMENURET;

//======================================================================
//	typedef struct
//======================================================================
typedef struct _TAG_FMENU_EVENT_WORK FMENU_EVENT_WORK;

//--------------------------------------------------------------
///	メニュー呼び出し関数
///	BOOL TRUE=イベント継続 FALSE==フィールドマップメニューイベント終了
//--------------------------------------------------------------
typedef BOOL (* FMENU_CALLPROC)(FMENU_EVENT_WORK*);

//--------------------------------------------------------------
///	FMENU_LIST
//--------------------------------------------------------------
typedef struct
{
	u32 str_id;		//表示する文字列ID
	void *callp;	//選択された際に返すパラメタ
}FMENU_LIST;

//--------------------------------------------------------------
///	FMENU_LISTDATA
//--------------------------------------------------------------
typedef struct
{
	u16 charsize_x;
	u16 charsize_y;
	u32 max;
	const FMENU_LIST *list;
}FMENU_LISTDATA;

//--------------------------------------------------------------
///	FMENU_COMMON_WORK
//--------------------------------------------------------------
typedef struct
{
	u32 bgFrame;
	u32 heapID;
	
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
	
	STRBUF *strbuf;
	GFL_MSGDATA *msgdata;
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	PRINT_UTIL printUtil[1];
	
	const FMENU_LIST *pMenuList;
	BMP_MENULIST_DATA *pMenuListData;
	BMPMENULIST_WORK *pMenuListWork;
}FMENU_COMMON_WORK;

//--------------------------------------------------------------
///	FLDMAP_MSGWIN_WORK
//--------------------------------------------------------------
typedef struct
{
	u32 bgFrame;
	u32 heapID;

	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;

	STRBUF *strbuf;
	GFL_MSGDATA *msgdata;
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	PRINT_UTIL printUtil[1];
}FLDMAP_MSGWIN_WORK;

//--------------------------------------------------------------
///	FMENU_EVENT_WORK
//--------------------------------------------------------------
struct _TAG_FMENU_EVENT_WORK
{
	HEAPID heapID;
	GMEVENT *gmEvent;
	GAMESYS_WORK *gmSys;
	FIELD_MAIN_WORK *fieldWork;
	
	int menu_num;
	FMENU_CALLPROC call_proc;
	FMENU_COMMON_WORK menu_common_work;
	void *sub_proc_parent;
};

//======================================================================
//	proto
//======================================================================
static GMEVENT_RESULT FldMapMenuEvent( GMEVENT *event, int *seq, void *wk );

static BOOL FMenuCallProc_Zukan( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_PokeStatus( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_Bag( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_MyTrainerCard( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_Report( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_Config( FMENU_EVENT_WORK *mwk );

static void FMenuMsgWinChgEvent( GMEVENT *event, u32 heapID, u32 strID );
static GMEVENT_RESULT FMenuMsgWinEvent( GMEVENT *event, int *seq, void *wk );

static GMEVENT_RESULT FMenuProcChangeEvent( GMEVENT *event, int *seq, void *wk );

static void FldMapMenu_InitCommon(
	FMENU_COMMON_WORK *work,
	const BMPMENULIST_HEADER *pMenuHead,
	BMP_MENULIST_DATA *pMenuListData,
	const FMENU_LIST *pMenuList, int menuCount,
	int bmppos_x, int bmppos_y,
	int bmpsize_x, int bmpsize_y,
	int arcDatIDmsg, HEAPID heapID );
static void FldMapMenu_DeleteCommon( FMENU_COMMON_WORK *work );
static u32 FldMapMenu_ProcCommon( FMENU_COMMON_WORK *work );

static void FldMapMsgWin_Init(
	FLDMAP_MSGWIN_WORK *work, int arcDatIDmsg, HEAPID heapID );
static void FldMapMsgWin_Delete( FLDMAP_MSGWIN_WORK *work );
static void FldMapMsgWin_SetPrint( FLDMAP_MSGWIN_WORK *work, u32 strID );
static BOOL FldMapMsgWin_Draw( FLDMAP_MSGWIN_WORK *work );

//--------------------------------------------------------------
///	フィールドマップメニューリスト
//--------------------------------------------------------------
static const FMENU_LIST DATA_FldMapMenuList[] =
{
	{ FLDMAPMENU_STR01, FMenuCallProc_Zukan },
	{ FLDMAPMENU_STR02, FMenuCallProc_PokeStatus },
	{ FLDMAPMENU_STR03, FMenuCallProc_Bag },
	{ FLDMAPMENU_STR04, FMenuCallProc_MyTrainerCard },
	{ FLDMAPMENU_STR05, FMenuCallProc_Report },
	{ FLDMAPMENU_STR06, FMenuCallProc_Config },
	{ FLDMAPMENU_STR07, NULL },
};

//--------------------------------------------------------------
///	フィールドマップメニューリストテーブル
//--------------------------------------------------------------
static const FMENU_LISTDATA DATA_FldMapMenuListTbl[] =
{
	{
		MENU_CHARSIZE_X,
		MENU_CHARSIZE_Y,
		NELEMS(DATA_FldMapMenuList),
		DATA_FldMapMenuList
	},
};

//メニュー最大数
#define MENULISTTBL_MAX (NELEMS(DATA_FldMapMenuListTbl))

//--------------------------------------------------------------
///	メニューヘッダー
//--------------------------------------------------------------
static const BMPMENULIST_HEADER DATA_FldMapMenuListHeader =
{
	NULL,	//表示文字データポインタ
	NULL,	//カーソル移動ごとのコールバック関数
	NULL,	//一列表示ごとのコールバック関数
	NULL,	//GFL_BMPWIN
	
	1,		//リスト項目数
	9,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	6,		//文字間隔Ｙ
	BMPMENULIST_LRKEY_SKIP,	//ページスキップタイプ
	0,		//文字指定(本来はu8だけどそんなに作らないと思うので)
	0,		//ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
	
	NULL,	//ワークポインタ
	
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	
	NULL,	//表示に使用するメッセージバッファ
	NULL,	//表示に使用するプリントユーティリティ
	NULL,	//表示に使用するプリントキュー
	NULL,	//表示に使用するフォントハンドル
};

//======================================================================
//	イベント：フィールドマップメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップメニューイベント起動
 * @param	gsys	GAMESYS_WORK
 * @param	fieldWork	FIELD_MAIN_WORK
 * @param	heapID	HEAPID
 * @retval	GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_FieldMapMenu(
	GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, HEAPID heapID )
{
	FMENU_EVENT_WORK *mwk;
	GMEVENT * event;
	
	event = GMEVENT_Create(
		gsys, NULL, FldMapMenuEvent, sizeof(FMENU_EVENT_WORK));
	
	mwk = GMEVENT_GetEventWork(event);
	MI_CpuClear8( mwk, sizeof(FMENU_EVENT_WORK) );
	
	mwk->gmSys = gsys;
	mwk->gmEvent = event;
	mwk->fieldWork = fieldWork;
	mwk->heapID = heapID;

	return event;
}

//--------------------------------------------------------------
/**
 * イベント：フィールドフィールドマップメニュー
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FldMapMenuEvent( GMEVENT *event, int *seq, void *wk )
{
	FMENU_EVENT_WORK *mwk = wk;
	
	switch (*seq) {
	case 0:
		{
			u32 i,max;
			BMPMENULIST_HEADER menuH;
			const FMENU_LIST *menu_list;
			const FMENU_LISTDATA *menu_listdata;
			
			menuH = DATA_FldMapMenuListHeader;
			menu_listdata = &DATA_FldMapMenuListTbl[mwk->menu_num];
			menu_list = menu_listdata->list;
			max = menu_listdata->max;
			
			FldMapMenu_InitCommon(
				&mwk->menu_common_work,
				&menuH, NULL, menu_list, max,
				23, 1,
				menu_listdata->charsize_x,menu_listdata->charsize_y,
				NARC_message_fldmapmenu_dat, mwk->heapID );
		}
		
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FldMapMenu_ProcCommon( &mwk->menu_common_work );
			
			if( ret == BMPMENULIST_NULL ){	//操作無し
				break;
			}else if( ret == BMPMENULIST_CANCEL ){	//キャンセル
				(*seq)++;
			}else{							//決定
				mwk->call_proc = (FMENU_CALLPROC)ret;
				(*seq)++;
			}
		}
		break;
	case 2:
		{
			FldMapMenu_DeleteCommon( &mwk->menu_common_work );

			if( mwk->call_proc != NULL ){
				if( mwk->call_proc(mwk) == TRUE ){
					return( GMEVENT_RES_CONTINUE );
				}
			}
			
			return( GMEVENT_RES_FINISH );
		}
		break;
	}

	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//	フィールドマップメニュー呼び出し
//======================================================================
//--------------------------------------------------------------
/**
 * メニュー呼び出し 図鑑イベント
 * @param	mwk	FMENU_EVENT_WORK
 * @retval	BOOL	TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Zukan( FMENU_EVENT_WORK *mwk )
{
	FMenuMsgWinChgEvent( mwk->gmEvent, mwk->heapID, FLDMAPMENU_STR08 );
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し ポケモンステータス
 * @param	mwk	FMENU_EVENT_WORK
 * @retval	BOOL	TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_PokeStatus( FMENU_EVENT_WORK *mwk )
{
#if 1
	GMEVENT *event = mwk->gmEvent;
	FMENU_EVENT_WORK tempWork = *mwk;
	FMENU_EVENT_WORK *newWork;
	GMEVENT_Change( event , FMenuProcChangeEvent , sizeof(FMENU_EVENT_WORK) );
		
	newWork = GMEVENT_GetEventWork(event);
	*newWork = tempWork;
#else
	FMenuMsgWinChgEvent( mwk->gmEvent, mwk->heapID, FLDMAPMENU_STR09 );
#endif
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し バッグ
 * @param	mwk	FMENU_EVENT_WORK
 * @retval	BOOL	TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Bag( FMENU_EVENT_WORK *mwk )
{
	FMenuMsgWinChgEvent( mwk->gmEvent, mwk->heapID, FLDMAPMENU_STR10 );
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し 自分
 * @param	mwk	FMENU_EVENT_WORK
 * @retval	BOOL	TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_MyTrainerCard( FMENU_EVENT_WORK *mwk )
{
	//Ari 仮へ飛ばします
	GMEVENT *event = mwk->gmEvent;
	FMENU_EVENT_WORK tempWork = *mwk;
	FMENU_EVENT_WORK *newWork;
	GMEVENT_Change( event , FMenuProcChangeEvent , sizeof(FMENU_EVENT_WORK) );
		
	newWork = GMEVENT_GetEventWork(event);
	*newWork = tempWork;

	//FMenuMsgWinChgEvent( mwk->gmEvent, mwk->heapID, FLDMAPMENU_STR11 );
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し レポート
 * @param	mwk	FMENU_EVENT_WORK
 * @retval	BOOL	TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Report( FMENU_EVENT_WORK *mwk )
{
	FMenuMsgWinChgEvent( mwk->gmEvent, mwk->heapID, FLDMAPMENU_STR12 );
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し 設定
 * @param	mwk	FMENU_EVENT_WORK
 * @retval	BOOL	TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Config( FMENU_EVENT_WORK *mwk )
{
	FMenuMsgWinChgEvent( mwk->gmEvent, mwk->heapID, FLDMAPMENU_STR13 );
	return( TRUE );
}

//======================================================================
//	メッセージウィンドウ表示イベント
//======================================================================
//--------------------------------------------------------------
///	FMENU_MSGWIN_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
	u32 heapID;
	u32 strID;
	FLDMAP_MSGWIN_WORK msgwin;
}FMENU_MSGWIN_EVENT_WORK;

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FMenuMsgWinChgEvent( GMEVENT *event, u32 heapID, u32 strID )
{
	FMENU_MSGWIN_EVENT_WORK *work;

	GMEVENT_Change(
		event, FMenuMsgWinEvent, sizeof(FMENU_MSGWIN_EVENT_WORK) );
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(FMENU_MSGWIN_EVENT_WORK) );
	work->heapID = heapID;
	work->strID = strID;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FMenuMsgWinEvent( GMEVENT *event, int *seq, void *wk )
{
	FMENU_MSGWIN_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
		FldMapMsgWin_Init( &work->msgwin,
			NARC_message_fldmapmenu_dat, work->heapID );
		FldMapMsgWin_SetPrint( &work->msgwin, work->strID );
		(*seq)++;
		break;
	case 1:
		if( FldMapMsgWin_Draw(&work->msgwin) == TRUE ){
			(*seq)++;
		}	
		break;
	case 2:
		{
			int trg = GFL_UI_KEY_GetTrg();
			if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				(*seq)++;
			}
		}
		break;
	case 3:
		FldMapMsgWin_Delete( &work->msgwin );
		return( GMEVENT_RES_FINISH );
	}

	return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//	フィールドマップメニュー専用共通メニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップメニュー用　共通メニュー初期化
 * @param	work	FMENU_COMMON_WORK
 * @param	pMenuHead BMPMENULIST_HEADER
 * @param	pMenuListData 表示項目。pMenuListから自動生成する場合はNULL
 * @param	pMenuList pMenuListを自動生成する場合に指定。それ以外はNULL
 * @param	menuCount	メニュー項目数
 * @param	bmppos_x	ビットマップ左上座標X
 * @param	bmppos_y	ビットマップ左上座標Y
 * @param	bmpsize_x	ビットマップサイズX　キャラ単位
 * @param	bmpsize_y	ビットマップサイズY　キャラ単位
 * @param	arcDatIDMsg	メッセージアーカイブデータインデックスID
 * @param	heapID		使用するHEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMapMenu_InitCommon(
	FMENU_COMMON_WORK *work,
	const BMPMENULIST_HEADER *pMenuHead,
	BMP_MENULIST_DATA *pMenuListData,
	const FMENU_LIST *pMenuList, int menuCount,
	int bmppos_x, int bmppos_y,
	int bmpsize_x, int bmpsize_y,
	int arcDatIDmsg, HEAPID heapID )
{
	BMPMENULIST_HEADER menuH = *pMenuHead;
	
	work->heapID = heapID;
	work->bgFrame = BGFRAME_MENU;
	work->pMenuListData = pMenuListData;
	work->pMenuList = pMenuList;
	
	{	//BG初期化 いずれメイン側のを利用する形へ
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( work->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( work->bgFrame, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, 2 );
		
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_D_TAYA, NARC_d_taya_default_nclr,
			PALTYPE_MAIN_BG, FONT_PANO*32, 32, work->heapID );
		
		GFL_BG_FillCharacter( work->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( work->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( work->bgFrame, 1,
			MENU_PANO, MENU_TYPE_SYSTEM, heapID );
	}
	
	{	//bmpwin
		work->bmpwin = GFL_BMPWIN_Create(
			work->bgFrame, bmppos_x, bmppos_y, bmpsize_x, bmpsize_y,
			FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		work->bmp = GFL_BMPWIN_GetBmp( work->bmpwin );
		
		GFL_BMP_Clear( work->bmp, 0xff );
		GFL_BMPWIN_MakeScreen( work->bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( work->bmpwin );
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
	
	{	//message
		work->msgdata = GFL_MSG_Create(
			GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, arcDatIDmsg, work->heapID );
		
		work->strbuf = GFL_STR_CreateBuffer( 512, work->heapID );
		
		work->fontHandle = GFL_FONT_Create(
			ARCID_D_TAYA,
			NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE,
			FALSE,
			work->heapID );
		
		work->printQue = PRINTSYS_QUE_Create( work->heapID );
		PRINT_UTIL_Setup( work->printUtil, work->bmpwin );
	}
	
	{	//	menu window
		BmpWinFrame_Write( work->bmpwin,
			WINDOW_TRANS_ON, 1, MENU_PANO );
	}
	
	{	//menu list
		if( work->pMenuList != NULL ){	//自動生成対象有り
			int i;
			GF_ASSERT( work->pMenuListData == NULL );
			work->pMenuListData = BmpMenuWork_ListCreate(
					menuCount, work->heapID );
			for( i = 0; i < menuCount; i++ ){
				BmpMenuWork_ListAddArchiveString(
					work->pMenuListData, work->msgdata,
					pMenuList[i].str_id, (u32)pMenuList[i].callp,
					work->heapID );
			}
		}
	}

	{	//menu
		menuH.count = menuCount;
		menuH.msgdata = work->msgdata;
		menuH.print_util = work->printUtil;
		menuH.print_que = work->printQue;
		menuH.font_handle = work->fontHandle;
		menuH.win = work->bmpwin;
		menuH.list = work->pMenuListData;
		
		work->pMenuListWork =
			BmpMenuList_Set( &menuH, 0, 0, work->heapID );
		BmpMenuList_SetCursorString( work->pMenuListWork, 0 );
	}
}

//--------------------------------------------------------------
/**
 * フィールドマップメニュー用　共通メニュー削除
 * @param	work	FMENU_COMMON_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMapMenu_DeleteCommon( FMENU_COMMON_WORK *work )
{
	BmpWinFrame_Clear( work->bmpwin, 0 );
	BmpMenuList_Exit( work->pMenuListWork, NULL, NULL );
	
	if( work->pMenuList != NULL ){	//自動生成
		BmpMenuWork_ListDelete( work->pMenuListData );
	}
	
	PRINTSYS_QUE_Delete( work->printQue );
	GFL_FONT_Delete( work->fontHandle );
	GFL_STR_DeleteBuffer( work->strbuf );
	GFL_MSG_Delete( work->msgdata );
	GFL_BMPWIN_Delete( work->bmpwin );
	
	{	//とりあえずこちらで　いずれはメイン側
		GFL_BG_FreeCharacterArea( work->bgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( work->bgFrame );
	}
}

//--------------------------------------------------------------
/**
 * フィールドマップメニュー用　共通メニュー処理
 * @param	work	DEBUG_COMMON_WORK
 * @retval	u32		メニューリスト選択位置のパラメータ
 * BMPMENULIST_NULL = 選択中 BMPMENULIST_CANCEL	= キャンセル
 */
//--------------------------------------------------------------
static u32 FldMapMenu_ProcCommon( FMENU_COMMON_WORK *work )
{
	u32 ret = BmpMenuList_Main( work->pMenuListWork );
	
	PRINTSYS_QUE_Main( work->printQue );
	
	if( PRINT_UTIL_Trans(work->printUtil,work->printQue) ){
	}

	return( ret );
}

//======================================================================
//	メッセージウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * メッセージウィンドウ初期化
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FldMapMsgWin_Init(
	FLDMAP_MSGWIN_WORK *work, int arcDatIDmsg, HEAPID heapID )
{
	work->heapID = heapID;
	work->bgFrame = BGFRAME_MENU;
	
	{	//BG初期化 いずれメイン側のを利用する形へ
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( work->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( work->bgFrame, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, 2 );
		
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_D_TAYA, NARC_d_taya_default_nclr,
			PALTYPE_MAIN_BG, FONT_PANO*32, 32, work->heapID );
		
		GFL_BG_FillCharacter( work->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( work->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( work->bgFrame, 1,
			MENU_PANO, MENU_TYPE_SYSTEM, heapID );
	}
	
	{	//bmpwin
		work->bmpwin = GFL_BMPWIN_Create(
			work->bgFrame, 1, 19, 30, 4,
			FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		work->bmp = GFL_BMPWIN_GetBmp( work->bmpwin );
		
		GFL_BMP_Clear( work->bmp, 0xff );
		GFL_BMPWIN_MakeScreen( work->bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( work->bmpwin );
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
	
	{	//message
		work->msgdata = GFL_MSG_Create(
			GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, arcDatIDmsg, work->heapID );
		
		work->strbuf = GFL_STR_CreateBuffer( 512, work->heapID );
		
		work->fontHandle = GFL_FONT_Create(
			ARCID_D_TAYA,
			NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE,
			FALSE,
			work->heapID );
		
		work->printQue = PRINTSYS_QUE_Create( work->heapID );
		PRINT_UTIL_Setup( work->printUtil, work->bmpwin );
	}
	
	{	//	menu window
		BmpWinFrame_Write( work->bmpwin,
			WINDOW_TRANS_ON, 1, MENU_PANO );
	}
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ削除
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FldMapMsgWin_Delete( FLDMAP_MSGWIN_WORK *work )
{
	BmpWinFrame_Clear( work->bmpwin, 0 );
	
	PRINTSYS_QUE_Delete( work->printQue );
	GFL_FONT_Delete( work->fontHandle );
	GFL_STR_DeleteBuffer( work->strbuf );
	GFL_MSG_Delete( work->msgdata );
	GFL_BMPWIN_Delete( work->bmpwin );
	
	{	//とりあえずこちらで　いずれはメイン側
		GFL_BG_FreeCharacterArea( work->bgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( work->bgFrame );
	}
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ文字列プリント
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FldMapMsgWin_SetPrint( FLDMAP_MSGWIN_WORK *work, u32 strID )
{
	GFL_MSG_GetString( work->msgdata, strID, work->strbuf );
	PRINT_UTIL_Print( work->printUtil, work->printQue,
			0, 0, work->strbuf, work->fontHandle );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ描画
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL FldMapMsgWin_Draw( FLDMAP_MSGWIN_WORK *work )
{
	PRINTSYS_QUE_Main( work->printQue );
	if( PRINT_UTIL_Trans(work->printUtil,work->printQue) ){
		return( FALSE );
	}
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * Proc変更イベント(現在トレーナーカードのみ対応
 * @param
 * @retval
 */
//--------------------------------------------------------------
extern const GFL_PROC_DATA DebugAriizumiMainProcData;
static GMEVENT_RESULT FMenuProcChangeEvent( GMEVENT *event, int *seq, void *wk )
{
	FMENU_EVENT_WORK *work = GMEVENT_GetEventWork(event);
	switch( *seq )
	{
	case 0:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB ,
									 0 ,16 ,0 );
		*seq += 1;
		break;
	case 1:
		if( GFL_FADE_CheckFade() == FALSE )
		{
			FIELDMAP_Close(work->fieldWork);
			*seq += 1;
		}
		break;

	case 2:
		if( GAMESYSTEM_IsProcExists(work->gmSys) == FALSE )
		{
			*seq += 1;
		}
		break;

	case 3:
		//ワークに値保存して、Procを変えるだけで対応できると思います
		if(work->call_proc == (FMENU_CALLPROC)FMenuCallProc_PokeStatus){	//ポケモン
			EASY_POKELIST_PARENT *epp;
			
			GF_ASSERT(work->sub_proc_parent == NULL);
			epp = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(EASY_POKELIST_PARENT));
			epp->party = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(work->gmSys));
			work->sub_proc_parent = epp;
			GFL_PROC_SysCallProc(NO_OVERLAY_ID, &EasyPokeListData, epp);
		}
		else{	//トレーナーカード
			GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugAriizumiMainProcData, NULL);
		}
		*seq += 1;
		//ここでフィールドのProcを抜ける
		break;
	case 4:
		//復帰後にここに来る
		if(work->sub_proc_parent != NULL){
			GFL_HEAP_FreeMemory(work->sub_proc_parent);
			work->sub_proc_parent = NULL;
		}
		GAMESYSTEM_CallFieldProc(work->gmSys);
		*seq += 1;
		break;
	case 5:
		//フィールドマップを開始待ち
		if(GAMESYSTEM_IsProcExists(work->gmSys) == TRUE )
		{
			//この時点ではまだフィールドの初期化は完全ではない
			if( FieldMain_IsFieldUpdate( work->fieldWork ) == TRUE )
			{
				//一回更新をまわして、MAP座標をキャラの位置にあわせる
				FieldMain_UpdateFieldFunc( work->fieldWork );
				GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB ,
										 16 ,0 ,0);
				*seq += 1;
			}
		}
		break;
	case 6:
		if( GFL_FADE_CheckFade() == FALSE )
		{
			return GMEVENT_RES_FINISH;
		}
		break;
	}
	return GMEVENT_RES_CONTINUE;

}

