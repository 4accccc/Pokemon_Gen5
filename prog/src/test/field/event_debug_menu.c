//======================================================================
/**
 *
 * @file	event_debug_menu.c
 * @brief	フィールドデバッグメニューの制御イベント
 * @author	kagaya
 * @date	2008.11.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "arc_def.h"

#include "system/bmp_menu.h"
#include "system/bmp_winframe.h"
#include "message.naix"
#include "msg/msg_d_field.h"
#include "test_graphic/d_taya.naix"

#include "field/zonedata.h"
#include "field_comm/field_comm_main.h"

#include "event_debug_menu.h"
#include "event_mapchange.h"

#include "field_comm/field_comm_main.h"
#include "field_comm/field_comm_debug.h"

//======================================================================
//	define
//======================================================================

#define DEBUG_BGFRAME_MENU (GFL_BG_FRAME1_M)	//使用フレーム
#define DEBUG_BGFRAME_MSG  (GFL_BG_FRAME2_M)	//使用フレーム
#define DEBUG_MENU_PANO	(14)
#define DEBUG_FONT_PANO	(15)

#define D_MENU_CHARSIZE_X (8)		//メニュー横幅
#define D_MENU_CHARSIZE_Y (16)		//メニュー縦幅

//--------------------------------------------------------------
///	DMENURET
//--------------------------------------------------------------
typedef enum
{
	DMENURET_CONTINUE,
	DMENURET_FINISH,
	DMENURET_NEXTEVENT,
}DMENURET;

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	メニュー呼び出し関数
///	BOOL TRUE=イベント継続 FALSE==デバッグメニューイベント終了
//--------------------------------------------------------------
typedef BOOL (* D_MENU_CALLPROC)(DEBUG_MENU_EVENT_WORK*);

//--------------------------------------------------------------
///	DEBUG_MENU_LIST
//--------------------------------------------------------------
struct _TAG_DEBUG_MENU_LIST
{
	u32 str_id;		//表示する文字列ID
	void *callp;	//選択された際に返すパラメタ
};

//--------------------------------------------------------------
///	DEBUG_MENU_LISTDATA
//--------------------------------------------------------------
typedef struct
{
	u16 charsize_x;
	u16 charsize_y;
	u32 max;
	const DEBUG_MENU_LIST *list;
}DEBUG_MENU_LISTDATA;

//--------------------------------------------------------------
///	DMENU_COMMON_WORK
//--------------------------------------------------------------
struct _TAG_DMENU_COMMON_WORK
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
	
	const DEBUG_MENU_LIST *pMenuList;
	BMP_MENULIST_DATA *pMenuListData;
	BMPMENULIST_WORK *pMenuListWork;
};

//--------------------------------------------------------------
///	DEBUG_MENU_EVENT_WORK
//--------------------------------------------------------------
struct _TAG_DEBUG_MENU_EVENT_WORK
{
	u32 page_id;
	HEAPID heapID;
	GMEVENT *gmEvent;
	GAMESYS_WORK *gmSys;
	FIELD_MAIN_WORK * fieldWork;
	
	D_MENU_CALLPROC call_proc;
	DMENU_COMMON_WORK menuCommonWork;
};

//--------------------------------------------------------------
///	DEBUG_FLDMENU
//--------------------------------------------------------------
#if 0
struct _TAG_DEBUG_FLDMENU
{
	HEAPID heapID;			//デバッグ用ヒープID
	GMEVENT *gmEvent;
	FIELD_MAIN_WORK *fieldWork;
	
	u32 menu_num;
	
	int seq_no;
	u32 bgFrame;
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
	
	STRBUF *strbuf;
	GFL_MSGDATA *msgdata;
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	PRINT_UTIL printUtil[1];
	
	BMP_MENULIST_DATA *menulistdata;
	BMPMENU_WORK *bmpmenu;
	D_MENU_CALLPROC call_proc;
	
	//通信メニュー用
	int commSeq;	//追加 Ari1111
	FIELD_COMM_MAIN	*commSys;
};
#endif

//======================================================================
//	proto
//======================================================================
static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk );

#if 0
static DEBUG_FLDMENU * FldDebugMenu_Init(
 GMEVENT *gmEvent, FIELD_MAIN_WORK *fieldWork, u32 menu_num, u32 heapID );
static void FldDebugMenu_Delete( DEBUG_FLDMENU *d_menu );
static void FldDebugMenu_Create( DEBUG_FLDMENU *d_menu );
static DMENURET FldDebugMenu_Main( DEBUG_FLDMENU *d_menu );
#endif

static BOOL DMenuCallProc_GridCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL DMenuCallProc_GridScaleSwitch( DEBUG_MENU_EVENT_WORK *wk );
static BOOL DMenuCallProc_GridScaleControl( DEBUG_MENU_EVENT_WORK *wk );

static BOOL DMenuCallProc_OpenStartComm( DEBUG_MENU_EVENT_WORK *wk );
static BOOL DMenuCallProc_OpenStartInvasion( DEBUG_MENU_EVENT_WORK *wk );

static BOOL DMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk );
static BOOL DMenuCallProc_OpenCommDebugMenu( DEBUG_MENU_EVENT_WORK *wk );

static void DEBUG_SetMenuWorkZoneIDName(
		BMP_MENULIST_DATA *list, HEAPID heapID );

void DebugMenu_InitCommonMenu(
	DMENU_COMMON_WORK *work,
	const BMPMENULIST_HEADER *pMenuHead,
	BMP_MENULIST_DATA *pMenuListData,
	const DEBUG_MENU_LIST *pMenuList, int menuCount,
	int bmpsize_x, int bmpsize_y,
	int arcDatIDmsg, HEAPID heapID );
void DebugMenu_DeleteCommonMenu( DMENU_COMMON_WORK *work );
u32 DebugMenu_ProcCommonMenu( DMENU_COMMON_WORK *work );


//--------------------------------------------------------------
///	デバッグメニューリスト　汎用
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_DebugMenuList[] =
{
	{ DEBUG_FIELD_C_CHOICE00, DMenuCallProc_OpenCommDebugMenu },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
};

//--------------------------------------------------------------
///	デバッグメニューリスト	グリッド実験マップ用
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_DebugMenuListGrid[] =
{
	{ DEBUG_FIELD_STR02, DMenuCallProc_GridCamera },
	{ DEBUG_FIELD_STR03, DMenuCallProc_GridScaleSwitch },
	{ DEBUG_FIELD_STR04, DMenuCallProc_GridScaleControl },
	{ DEBUG_FIELD_STR05, DMenuCallProc_MapZoneSelect },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_C_CHOICE00, DMenuCallProc_OpenCommDebugMenu },
//	{ DEBUG_FIELD_STR01, NULL },
//	{ DEBUG_FIELD_STR01, NULL },
};

//--------------------------------------------------------------
///	デバッグメニューリストテーブル
//--------------------------------------------------------------
static const DEBUG_MENU_LISTDATA DATA_DebugMenuListTbl[] =
{
	{	//実験マップ 橋
		D_MENU_CHARSIZE_X,
		D_MENU_CHARSIZE_Y,
		NELEMS(DATA_DebugMenuList),
		DATA_DebugMenuList
	},
	{	//実験マップ　グリッド移動
		11,
		16,
		NELEMS(DATA_DebugMenuListGrid),
		DATA_DebugMenuListGrid
	},
	{	//実験マップ　橋
		D_MENU_CHARSIZE_X,
		D_MENU_CHARSIZE_Y,
		NELEMS(DATA_DebugMenuList),
		DATA_DebugMenuList
	},
	{	//実験マップ　グリッド移動
		11,
		16,
		NELEMS(DATA_DebugMenuListGrid),
		DATA_DebugMenuListGrid
	},
};

//メニュー最大数
#define D_MENULISTTBL_MAX (NELEMS(DATA_DebugMenuListTbl))

//--------------------------------------------------------------
///	メニューヘッダー
//--------------------------------------------------------------
static const BMPMENULIST_HEADER DATA_DebugMenuListHeader =
{
	NULL,	//表示文字データポインタ
	NULL,	//カーソル移動ごとのコールバック関数
	NULL,	//一列表示ごとのコールバック関数
	NULL,	//GFL_BMPWIN

	1,		//リスト項目数
	12,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	2,		//文字間隔Ｙ
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
//	イベント：フィールドデバッグメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグメニューイベント起動
 * @param	gsys	GAMESYS_WORK
 * @param	fieldWork	FIELD_MAIN_WORK
 * @param	heapID	HEAPID
 * @param	page_id	デバッグメニューページ
 * @retval	GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu(
	GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork,
	HEAPID heapID, u16 page_id )
{
	DEBUG_MENU_EVENT_WORK * dmew;
	GMEVENT * event;
	
	event = GMEVENT_Create(
		gsys, NULL, DebugMenuEvent, sizeof(DEBUG_MENU_EVENT_WORK));
	
	dmew = GMEVENT_GetEventWork(event);
	MI_CpuClear8( dmew, sizeof(DEBUG_MENU_EVENT_WORK) );
	
	dmew->gmSys = gsys;
	dmew->gmEvent = event;
	dmew->fieldWork = fieldWork;
	dmew->heapID = heapID;
	dmew->page_id = page_id;
	
	if( dmew->page_id >= D_MENULISTTBL_MAX ){
		OS_Printf( "debug menu number error\n" );
		dmew->page_id = 0;
	}
	
	return event;
}

//--------------------------------------------------------------
/**
 * イベント：フィールドデバッグメニュー
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk )
{
	DEBUG_MENU_EVENT_WORK *work = wk;
	
	switch (*seq) {
	case 0:
		{
			u32 i,max;
			BMPMENULIST_HEADER menuH;
			const DEBUG_MENU_LIST *d_menu_list;
			const DEBUG_MENU_LISTDATA *d_menu_listdata;
			
			menuH = DATA_DebugMenuListHeader;
			d_menu_listdata = &DATA_DebugMenuListTbl[work->page_id];
			d_menu_list = d_menu_listdata->list;
			max = d_menu_listdata->max;
			
			DebugMenu_InitCommonMenu(
				&work->menuCommonWork,
				&menuH, NULL, d_menu_list, max,
				d_menu_listdata->charsize_x, d_menu_listdata->charsize_y,
				NARC_message_d_field_dat, work->heapID );
		}
		
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = DebugMenu_ProcCommonMenu( &work->menuCommonWork );
			
			if( ret == BMPMENULIST_NULL ){	//操作無し
				break;
			}else if( ret == BMPMENULIST_CANCEL ){	//キャンセル
				(*seq)++;
			}else{							//決定
				work->call_proc = (D_MENU_CALLPROC)ret;
				(*seq)++;
			}
		}
		break;
	case 2:
		{
			DebugMenu_DeleteCommonMenu( &work->menuCommonWork );
			
			if( work->call_proc != NULL ){
				if( work->call_proc(work) == TRUE ){
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
//	フィールドデバッグメニュー	古い
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　初期化
 * @param	heapID	ヒープID
 * @retval	DEBUG_FLDMENU
 */
//--------------------------------------------------------------
static DEBUG_FLDMENU * FldDebugMenu_Init(
	GMEVENT *gmEvent, FIELD_MAIN_WORK *fieldWork, u32 menu_num, u32 heapID )
{
	DEBUG_FLDMENU *d_menu;
	
	d_menu = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEBUG_FLDMENU) );
	
	d_menu->gmEvent = gmEvent;
	d_menu->heapID = heapID;
	d_menu->fieldWork = fieldWork;
	d_menu->bgFrame = DEBUG_BGFRAME_MENU;
	
	if( menu_num >= D_MENULISTTBL_MAX ){
		OS_Printf( "debug menu number error\n" );
		menu_num = 0;
	}
	
	d_menu->menu_num = menu_num;
	
	{	//BG初期化 いずれメイン側のを利用する形へ
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl(
			d_menu->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( d_menu->bgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( d_menu->bgFrame, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, 2 );
		
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_D_TAYA, NARC_d_taya_default_nclr,
			PALTYPE_MAIN_BG, DEBUG_FONT_PANO*32, 32, d_menu->heapID );
		
		GFL_BG_FillCharacter( d_menu->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( d_menu->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_LoadScreenReq( d_menu->bgFrame );
	}

	{
		//もう１面追加します Ari1113
		GFL_BG_BGCNT_HEADER msgBgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		const u8 msgBgFrame = DEBUG_BGFRAME_MSG;
		
		GFL_BG_SetBGControl(
			msgBgFrame, &msgBgcntText, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( msgBgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( msgBgFrame, 1 );

		GFL_BG_FillCharacter( msgBgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( msgBgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_LoadScreenReq( msgBgFrame );
	}

	d_menu->commSys = (FIELD_COMM_MAIN*)FieldMain_GetCommSys( fieldWork );
	return( d_menu );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　削除
 * @param	d_menu	DEBUG_FLDMENU
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldDebugMenu_Delete( DEBUG_FLDMENU *d_menu )
{
	const u8 msgBgFrame = DEBUG_BGFRAME_MSG;
	
	{	//とりあえずこちらで　いずれはメイン側
		GFL_BG_FreeCharacterArea( d_menu->bgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( d_menu->bgFrame );
		GFL_BG_FreeCharacterArea( msgBgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( msgBgFrame );
	}
	
	GFL_HEAP_FreeMemory( d_menu );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　メニュー作成
 * @param	d_menu	DEBUG_FLDMENU
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldDebugMenu_Create( DEBUG_FLDMENU *d_menu )
{
	{	//work
		d_menu->seq_no = 0;
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( d_menu->bgFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, d_menu->heapID );
	}
	
	{	//bmpwin
		const DEBUG_MENU_LISTDATA *d_menu_tbl;
		d_menu_tbl = &DATA_DebugMenuListTbl[d_menu->menu_num];

		d_menu->bmpwin = GFL_BMPWIN_Create( d_menu->bgFrame,
			1, 1, d_menu_tbl->charsize_x, d_menu_tbl->charsize_y,
			DEBUG_FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		d_menu->bmp = GFL_BMPWIN_GetBmp( d_menu->bmpwin );
		
		GFL_BMP_Clear( d_menu->bmp, 0xff );
		GFL_BMPWIN_MakeScreen( d_menu->bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( d_menu->bmpwin );
		GFL_BG_LoadScreenReq( d_menu->bgFrame );
	}
	
	{	//msg
		d_menu->msgdata = GFL_MSG_Create(
			GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
			NARC_message_d_field_dat, d_menu->heapID );
		
		d_menu->strbuf = GFL_STR_CreateBuffer( 1024, d_menu->heapID );
		
		d_menu->fontHandle = GFL_FONT_Create(
			ARCID_D_TAYA,
			NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE,
			FALSE,
			d_menu->heapID );

		d_menu->printQue = PRINTSYS_QUE_Create( d_menu->heapID );
		PRINT_UTIL_Setup( d_menu->printUtil, d_menu->bmpwin );	
	}
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　メイン
 * @param	d_menu	DEBUG_FLDMENU
 * @retval	DMENURET	DMENURET
 */
//--------------------------------------------------------------
static DMENURET FldDebugMenu_Main( DEBUG_FLDMENU *d_menu )
{
	switch( d_menu->seq_no ){
	case 0:	//メニュー作成
		{	//window frame
			BmpWinFrame_Write( d_menu->bmpwin,
				WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
		}

		{	//menu create
			u32 i,lmax;
			BMPMENU_HEADER head;
			const DEBUG_MENU_LIST *d_menu_list;
			const DEBUG_MENU_LISTTBL *d_menu_tbl;
			
			d_menu_tbl = &DATA_DebugMenuListTbl[d_menu->menu_num];
			d_menu_list = d_menu_tbl->list;
			lmax = d_menu_tbl->max;
			
			head.x_max = 1;
			head.y_max = lmax;
			head.line_spc = 4;
			head.c_disp_f = 0;
			head.loop_f = 1;
			
			head.font_size_x = 12;
			head.font_size_y = 12;
			head.msgdata = d_menu->msgdata;
			head.print_util = d_menu->printUtil;
			head.print_que = d_menu->printQue;
			head.font_handle = d_menu->fontHandle;
			head.win = d_menu->bmpwin;
			
			d_menu->menulistdata =
				BmpMenuWork_ListCreate( lmax, d_menu->heapID );
			
			for( i = 0; i < lmax; i++ ){
				BmpMenuWork_ListAddArchiveString(
					d_menu->menulistdata, d_menu->msgdata,
					d_menu_list[i].str_id, (u32)d_menu_list[i].callp,
					d_menu->heapID );
			}
			
			head.menu = d_menu->menulistdata;
			
			d_menu->bmpmenu = BmpMenu_Add( &head, 0, d_menu->heapID );
			BmpMenu_SetCursorString( d_menu->bmpmenu, DEBUG_FIELD_STR00 );
		}
		
		d_menu->seq_no++;
		break;
	case 1:	//メニュー処理
		{
			u32 ret;
			
			ret = BmpMenu_Main( d_menu->bmpmenu );
			PRINTSYS_QUE_Main( d_menu->printQue );
			
			if( PRINT_UTIL_Trans(d_menu->printUtil,d_menu->printQue) ){
			}
			
			switch( ret ){
			case BMPMENU_NULL:
				break;
			case BMPMENU_CANCEL:
				GFL_BMP_Clear( d_menu->bmp, 0x00 );
				GFL_BMPWIN_MakeScreen( d_menu->bmpwin );
				GFL_BMPWIN_TransVramCharacter( d_menu->bmpwin );
				BmpWinFrame_Clear( d_menu->bmpwin, 0 );
				d_menu->seq_no++;
				break;
			default:
				GFL_BMP_Clear( d_menu->bmp, 0x00 );
				GFL_BMPWIN_MakeScreen( d_menu->bmpwin );
				GFL_BMPWIN_TransVramCharacter( d_menu->bmpwin );
				BmpWinFrame_Clear( d_menu->bmpwin, 0 );
				d_menu->call_proc = (D_MENU_CALLPROC)ret;
				d_menu->seq_no++;
				break;
			}
		}
		break;
	case 2:	//削除
		{
			BOOL ret = FALSE;
			
			BmpMenu_Exit( d_menu->bmpmenu, NULL );
			BmpMenuWork_ListDelete( d_menu->menulistdata );	//freeheap
			
			PRINTSYS_QUE_Delete( d_menu->printQue );
			GFL_FONT_Delete( d_menu->fontHandle );
			
			GFL_STR_DeleteBuffer( d_menu->strbuf );
			GFL_MSG_Delete( d_menu->msgdata );
			GFL_BMPWIN_Delete( d_menu->bmpwin );
			
			if( d_menu->call_proc != NULL ){
				ret = d_menu->call_proc( d_menu );
			}
			
			d_menu->call_proc = NULL;
			
			//通信に行くときはそのままにしたいので
			if( d_menu->seq_no == 2 ){
				if( ret == TRUE ){
					return( DMENURET_NEXTEVENT );
				}
				
				return( DMENURET_FINISH );
			}
		}
	
	case 10:	//通信メニュー用処理
		switch( d_menu->commSeq )
		{
		case 0:
			FIELD_COMM_MAIN_InitStartCommMenu( d_menu->commSys );
			d_menu->commSeq++;
			break;
		case 1:
			if( FIELD_COMM_MAIN_LoopStartCommMenu( d_menu->commSys ) == TRUE ){
				d_menu->commSeq++;
			}
			break;
		case 2:
			FIELD_COMM_MAIN_TermStartCommMenu( d_menu->commSys );
			d_menu->commSeq++;
			return (DMENURET_FINISH);
			break;
		}
		break;
	case 11:
		switch( d_menu->commSeq )
		{
		case 0:
			FIELD_COMM_MAIN_InitStartInvasionMenu( d_menu->commSys );
			d_menu->commSeq++;
			break;
		case 1:
			if( FIELD_COMM_MAIN_LoopStartInvasionMenu( d_menu->commSys ) == TRUE ){
				d_menu->commSeq++;
			}
			break;
		case 2:
			FIELD_COMM_MAIN_TermStartInvasionMenu( d_menu->commSys );
			d_menu->commSeq++;
			return (DMENURET_FINISH);
			break;
		}
		break;
	}

	return( DMENURET_CONTINUE );
}
#endif

//======================================================================
//	デバッグメニュー呼び出し
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用カメラ
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL DMenuCallProc_GridCamera( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_MENU_EVENT_WORK *d_menu = wk;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_Camera( fieldWork );
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用スケール切り替え
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL DMenuCallProc_GridScaleSwitch( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_MENU_EVENT_WORK *d_menu = wk;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_ScaleChange( fieldWork );
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用スケール調節
 * @param	wk	DEBUG_MENU_EVENT_WORK *
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL DMenuCallProc_GridScaleControl( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_MENU_EVENT_WORK *d_menu = wk;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_ScaleControl( fieldWork );
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　通信デバッグ子メニュー
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL DMenuCallProc_OpenCommDebugMenu( DEBUG_MENU_EVENT_WORK *wk )
{
	GMEVENT *event = wk->gmEvent;
	const HEAPID heapID = wk->heapID;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	FIELD_COMM_DEBUG_WORK *work;
	
	GMEVENT_Change( event,
		FIELD_COMM_DEBUG_CommDebugMenu, FIELD_COMM_DEBUG_GetWorkSize() );
	
	work = GMEVENT_GetEventWork( event );
	FIELD_COMM_DEBUG_InitWork( heapID , fieldWork , event , work );

	return( TRUE );
}

//======================================================================
//	デバッグメニュー どこでもジャンプ
//======================================================================
//--------------------------------------------------------------
///	DEBUG_ZONSEL_EVENT_WORK どこでもジャンプ処理用ワーク
//--------------------------------------------------------------
typedef struct
{
	int seq_no;
	HEAPID heapID;
	GAMESYS_WORK *gmSys;
	GMEVENT *gmEvent;
	FIELD_MAIN_WORK *fieldWork;
	
	BMP_MENULIST_DATA *pMenuListData;
	DMENU_COMMON_WORK menuCommonWork;
}DEBUG_ZONSEL_EVENT_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT DMenuZoneSelectEvent(
		GMEVENT *event, int *seq, void *work );

///	どこでもジャンプ　メニューヘッダー
static const BMPMENULIST_HEADER DATA_DebugMenuList_ZoneSel =
{
	NULL,	//表示文字データポインタ
	NULL,	//カーソル移動ごとのコールバック関数
	NULL,	//一列表示ごとのコールバック関数
	NULL,	//GFL_BMPWIN

	1,		//リスト項目数
	12,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
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

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　どこでもジャンプ
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL DMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	DEBUG_ZONSEL_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		DMenuZoneSelectEvent, sizeof(DEBUG_ZONSEL_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_ZONSEL_EVENT_WORK) );
	
	work->gmSys = gsys;
	work->gmEvent = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：どこでもジャンプ
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DMenuZoneSelectEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_ZONSEL_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
		{
			u32 max = ZONEDATA_GetZoneIDMax();
			BMPMENULIST_HEADER menuH = DATA_DebugMenuList_ZoneSel;
			
			work->pMenuListData =
				BmpMenuWork_ListCreate( max, work->heapID );
			DEBUG_SetMenuWorkZoneIDName(
				work->pMenuListData, work->heapID );
			
			DebugMenu_InitCommonMenu(
				&work->menuCommonWork,
				&menuH, work->pMenuListData, NULL, max,
				11, 16, NARC_message_d_field_dat, work->heapID );
		}
		
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = DebugMenu_ProcCommonMenu( &work->menuCommonWork );
			
			if( ret == BMPMENULIST_NULL ){	//操作無し
				break;
			}
			
			DebugMenu_DeleteCommonMenu( &work->menuCommonWork );
			BmpMenuWork_ListDelete( work->pMenuListData );
			
			if( ret == BMPMENULIST_CANCEL ){	//キャンセル
				return( GMEVENT_RES_FINISH );
			}
			
			DEBUG_EVENT_ChangeEventMapChange(	//決定
				work->gmSys, work->gmEvent, work->fieldWork, ret );
		}
		break;
	}

	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//	デバッグ用ZoneID文字列変換
//======================================================================
//--------------------------------------------------------------
//	文字コード
//--------------------------------------------------------------
#define ASCII_EOM (0x00)
#define ASCII_0 (0x30)
#define ASCII_9 (0x39)
#define ASCII_A (0x41)
#define ASCII_Z (0x5a)

//半角
#define UTF16H_0 (0x0030)
#define UTF16H_9 (0x0039)
#define UTF16H_A (0x0041)
#define UTF16H_Z (0x005a)

//全角
#define UTF16_0 (0xff10)
#define UTF16_9 (0xff19)
#define UTF16_A (0xff21)
#define UTF16_Z (0xff3a)

//--------------------------------------------------------------
/**
 * ASCII->UTF-16
 * @param	str	ASCII 半角英数字
 * @retval	u16 UTF-16 str
 */
//--------------------------------------------------------------
static u16 DEBUG_ASCIICODE_UTF16( u8 code )
{
	if( code == ASCII_EOM ){
		return( GFL_STR_GetEOMCode() );
	}
	
	if( code >= ASCII_0 && code <= ASCII_9 ){
		code -= ASCII_0;
		return( UTF16H_0 + code );
	}
	
	if( code >= ASCII_A && code <= ASCII_Z ){
		code -= ASCII_A;
		return( UTF16H_A + code );
	}
	
	GF_ASSERT( 0 ); 					//未対応文字
	return( GFL_STR_GetEOMCode() );
}

//--------------------------------------------------------------
/**
 * ZONE_ID->STRCODE
 * @param	heapID	文字列バッファ確保用ヒープID
 * @param	zoneID	文字列を取得したいzoneID
 * @retval	u16*	zoneID文字列が格納されたu16*(開放が必要
 */
//--------------------------------------------------------------
static u16 * DEBUG_GetZoneNameUTF16( u32 heapID, u32 zoneID )
{
	int i;
	u16 utf16,utf16_eom;
	u16 *pStrBuf;
	char name8[128];
	
	pStrBuf = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16)*128 );
	ZONEDATA_GetZoneName( heapID, name8, zoneID );
	utf16_eom = GFL_STR_GetEOMCode();
	OS_Printf( "変換 %s\n", name8 );
	
	for( i = 0; i < 128; i++ ){
		utf16 = DEBUG_ASCIICODE_UTF16( name8[i] );
		pStrBuf[i] = utf16;
		
		if( utf16 == utf16_eom ){
			return( pStrBuf );
		}
	}
	
	pStrBuf[i-1] = utf16_eom;	//文字数オーバー
	GF_ASSERT( 0 );
	return( pStrBuf );
}

//--------------------------------------------------------------
/**
 * Zone ID Name -> BMP_MENULIST_DATA 
 * @param	heapID	テンポラリ文字列確保用heapID
 * @param	zoneID	文字列を取得したいzoneID
 * @param	strBuf	文字列格納先STRBUF
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetSTRBUF_ZoneIDName(
		u32 heapID, u32 zoneID, STRBUF *strBuf )
{
	u16 *str = DEBUG_GetZoneNameUTF16( heapID, zoneID );
	GFL_STR_SetStringCode( strBuf, str );
	GFL_HEAP_FreeMemory( str );
}

//--------------------------------------------------------------
/**
 * 何処でもジャンプ用BMP_MENULIST_DATAセット
 * @param	list	セット先BMP_MENULIST_DATA
 * @param	heapID	文字列バッファ確保用HEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkZoneIDName(
		BMP_MENULIST_DATA *list, HEAPID heapID )
{
	int id,max = ZONEDATA_GetZoneIDMax();
	STRBUF *strBuf = GFL_STR_CreateBuffer( 128, heapID );
	
	for( id = 0; id < max; id++ ){
		GFL_STR_ClearBuffer( strBuf );
		DEBUG_SetSTRBUF_ZoneIDName( heapID, id, strBuf );
		BmpMenuWork_ListAddString( list, strBuf, id, heapID );
	}
	
	GFL_HEAP_FreeMemory( strBuf );
}

//======================================================================
//	デバッグメニュー専用共通メニュー
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー用　共通メニュー初期化
 * @param	work	DMENU_COMMON_WORK
 * @param	pMenuHead BMPMENULIST_HEADER
 * @param	pMenuListData 表示項目。pMenuListから自動生成する場合はNULL
 * @param	pMenuList pMenuListを自動生成する場合に指定。それ以外はNULL
 * @param	menuCount	メニュー項目数
 * @param	bmpsize_x	ビットマップサイズX　キャラ単位
 * @param	bmpsize_y	ビットマップサイズY　キャラ単位
 * @param	arcDatIDMsg	メッセージアーカイブデータインデックスID
 * @param	heapID		使用するHEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
void DebugMenu_InitCommonMenu(
	DMENU_COMMON_WORK *work,
	const BMPMENULIST_HEADER *pMenuHead,
	BMP_MENULIST_DATA *pMenuListData,
	const DEBUG_MENU_LIST *pMenuList, int menuCount,
	int bmpsize_x, int bmpsize_y,
	int arcDatIDmsg, HEAPID heapID )
{
	BMPMENULIST_HEADER menuH = *pMenuHead;
	
	work->heapID = heapID;
	work->bgFrame = DEBUG_BGFRAME_MENU;
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
			PALTYPE_MAIN_BG, DEBUG_FONT_PANO*32, 32, work->heapID );
		
		GFL_BG_FillCharacter( work->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( work->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( work->bgFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, heapID );
	}
	
	{	//bmpwin
		work->bmpwin = GFL_BMPWIN_Create(
			work->bgFrame, 1, 1, bmpsize_x, bmpsize_y,
			DEBUG_FONT_PANO, GFL_BMP_CHRAREA_GET_B );
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
			WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
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
 * デバッグメニュー用　共通メニュー削除
 * @param	work	DMENU_COMMON_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void DebugMenu_DeleteCommonMenu( DMENU_COMMON_WORK *work )
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
 * デバッグメニュー用　共通メニュー処理
 * @param	work	DEBUG_COMMON_WORK
 * @retval	u32		メニューリスト選択位置のパラメータ
 * BMPMENULIST_NULL = 選択中 BMPMENULIST_CANCEL	= キャンセル
 */
//--------------------------------------------------------------
u32 DebugMenu_ProcCommonMenu( DMENU_COMMON_WORK *work )
{
	u32 ret = BmpMenuList_Main( work->pMenuListWork );
	
	PRINTSYS_QUE_Main( work->printQue );
	
	if( PRINT_UTIL_Trans(work->printUtil,work->printQue) ){
	}

	return( ret );
}

//--------------------------------------------------------------
//	ワークサイズの取得
//--------------------------------------------------------------
const int DebugMenu_GetWorkSize(void)
{ 
	return sizeof(DMENU_COMMON_WORK);
}
