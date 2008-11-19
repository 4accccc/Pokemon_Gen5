//======================================================================
/**
 *
 * @file	field_debug.c
 * @brief	フィールドデバッグ
 * @author	kagaya
 * @data	08.09.29
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/bmp_menu.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "message.naix"

#include "test_graphic/d_taya.naix"
#include "msg/msg_d_field.h"

#include "field_debug.h"
#include "field_comm/field_comm_main.h"

#include "field/zonedata.h"

//======================================================================
//	define
//======================================================================
#define DEBUG_BGFRAME_MENU (GFL_BG_FRAME1_M)	//使用フレーム
#define DEBUG_BGFRAME_MSG  (GFL_BG_FRAME2_M)	//使用フレーム
#define DEBUG_MENU_PANO	(14)
#define DEBUG_FONT_PANO	(15)

#define D_MENU_CHARSIZE_X (8)		//メニュー横幅
#define D_MENU_CHARSIZE_Y (16)		//メニュー縦幅

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	メニュー呼び出し関数
//--------------------------------------------------------------
typedef void (* D_MENU_CALLPROC)(DEBUG_FLDMENU*);

//--------------------------------------------------------------
///	DEBUG_MENU_LIST
//--------------------------------------------------------------
typedef struct
{
	u32 str_id;				//表示する文字列ID
	D_MENU_CALLPROC callp;	//選択された際に呼び出す関数 NULL=呼び出し無し
}DEBUG_MENU_LIST;

//--------------------------------------------------------------
///	DEBUG_MENU_LISTTBL
//--------------------------------------------------------------
typedef struct
{
	u16 charsize_x;
	u16 charsize_y;
	u32 max;
	const DEBUG_MENU_LIST *list;
}DEBUG_MENU_LISTTBL;

//--------------------------------------------------------------
///	DEBUG_FLDMENU
//--------------------------------------------------------------
struct _TAG_DEBUG_FLDMENU
{
	HEAPID heapID;	//デバッグ用ヒープID
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
//	ARCHANDLE *arcHandleMsg;
	
	BMP_MENULIST_DATA *menulistdata;
	BMPMENU_WORK *bmpmenu;
	D_MENU_CALLPROC call_proc;

	//通信メニュー用
	int commSeq;	//追加 Ari1111
	FIELD_COMM_MAIN	*commSys;
	
	void *pUserWork;
};

//======================================================================
//	proto
//======================================================================
static void DMenuCallProc_GridCamera( DEBUG_FLDMENU *wk );
static void DMenuCallProc_GridScaleSwitch( DEBUG_FLDMENU *wk );
static void DMenuCallProc_GridScaleControl( DEBUG_FLDMENU *wk );

static void DMenuCallProc_OpenStartComm( DEBUG_FLDMENU *wk );
static void DMenuCallProc_OpenStartInvasion( DEBUG_FLDMENU *wk );

static void DMenuCallProc_MapZoneSelect( DEBUG_FLDMENU *wk );
static int DMenuZoneSelect_Main( DEBUG_FLDMENU *wk );

static void DEBUG_SetMenuWorkZoneIDName(
		BMP_MENULIST_DATA *list, HEAPID heapID );

//======================================================================
//	メニューリスト一覧
//======================================================================
//--------------------------------------------------------------
///	デバッグメニューリスト　汎用
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_DebugMenuList[] =
{
	{ DEBUG_FIELD_C_CHOICE00, DMenuCallProc_OpenStartComm },
	{ DEBUG_FIELD_C_CHOICE01, DMenuCallProc_OpenStartInvasion },
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
	{ DEBUG_FIELD_C_CHOICE00, DMenuCallProc_OpenStartComm },
	{ DEBUG_FIELD_C_CHOICE01, DMenuCallProc_OpenStartInvasion },
//	{ DEBUG_FIELD_STR01, NULL },
//	{ DEBUG_FIELD_STR01, NULL },
};

//--------------------------------------------------------------
///	デバッグメニューリストテーブル
//--------------------------------------------------------------
static const DEBUG_MENU_LISTTBL DATA_DebugMenuListTbl[] =
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

//======================================================================
//	フィールドデバッグメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　初期化
 * @param	heapID	ヒープID
 * @retval	DEBUG_FLDMENU
 */
//--------------------------------------------------------------
DEBUG_FLDMENU * FldDebugMenu_Init(
	FIELD_MAIN_WORK *fieldWork, u32 menu_num, u32 heapID )
{
	DEBUG_FLDMENU *d_menu;
	
	d_menu = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEBUG_FLDMENU) );
	
	d_menu->heapID = heapID;
	d_menu->fieldWork = fieldWork;
	d_menu->bgFrame = DEBUG_BGFRAME_MENU;
	
	if( menu_num >= D_MENULISTTBL_MAX ){
		OS_Printf( "debug menu number error\n" );
		menu_num = 0;
	}

	d_menu->menu_num = menu_num;
	
	{	//bmp font いずれメイン側で
		GFL_BMPWIN_Init( d_menu->heapID );
		GFL_FONTSYS_Init();
	}
	
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
void FldDebugMenu_Delete( DEBUG_FLDMENU *d_menu )
{
	const u8 msgBgFrame = DEBUG_BGFRAME_MSG;
	{	//とりあえずこちらで　いずれはメイン側
		GFL_BMPWIN_Exit();
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
void FldDebugMenu_Create( DEBUG_FLDMENU *d_menu )
{
	{	//work
		d_menu->seq_no = 0;
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( d_menu->bgFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, d_menu->heapID );
	}
	
	{	//bmpwin
		const DEBUG_MENU_LISTTBL *d_menu_tbl;
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
		#if 0		
		d_menu->arcHandleMsg = GFL_ARC_OpenDataHandle(
				ARCID_D_TAYA, d_menu->heapID );
		
		d_menu->fontHandle = GFL_FONT_CreateHandle(
			d_menu->arcHandleMsg, NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, d_menu->heapID );
		#else
			d_menu->fontHandle = GFL_FONT_Create(
				ARCID_D_TAYA,
				NARC_d_taya_lc12_2bit_nftr,
				GFL_FONT_LOADTYPE_FILE,
				FALSE,
				d_menu->heapID );

		#endif
//		PRINTSYS_Init( d_menu->heapID );	//メイン側で呼ぶようになった
		d_menu->printQue = PRINTSYS_QUE_Create( d_menu->heapID );
		PRINT_UTIL_Setup( d_menu->printUtil, d_menu->bmpwin );	
	}
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　メイン
 * @param	d_menu	DEBUG_FLDMENU
 * @retval	BOOL	TRUE=終了
 */
//--------------------------------------------------------------
BOOL FldDebugMenu_Main( DEBUG_FLDMENU *d_menu )
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
		BmpMenu_Exit( d_menu->bmpmenu, NULL );
		BmpMenuWork_ListDelete( d_menu->menulistdata );	//freeheap

		//プリントユーティリティ削除はいらない
		PRINTSYS_QUE_Delete( d_menu->printQue );
//		PRINTSYS_Delete();	//freeheap
//		FontDataMan_Delete( d_menu->fontHandle );
		GFL_FONT_Delete( d_menu->fontHandle );
//		GFL_ARC_CloseDataHandle( d_menu->arcHandleMsg ); //マネージャ側で済み
		
		GFL_STR_DeleteBuffer( d_menu->strbuf );
		GFL_MSG_Delete( d_menu->msgdata );
		GFL_BMPWIN_Delete( d_menu->bmpwin );

		if( d_menu->call_proc != NULL ){
			d_menu->call_proc( d_menu );
		}
		
		d_menu->call_proc = NULL;
		
		//通信に行くときはそのままにしたいので
		if( d_menu->seq_no == 2 ){
			return( TRUE );
		}
		
		if( d_menu->seq_no == 20 ){
			break;
		}

		//通信メニュー用処理
	case 10:
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
			return (TRUE);
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
			return (TRUE);
			break;
		}
		break;
	case 20:	//何処でもジャンプ
		{
			if( DMenuZoneSelect_Main(d_menu) == TRUE ){
				GFL_HEAP_FreeMemory( d_menu->pUserWork );
				return( TRUE );
			}
		}
		break;
	}
	
	return( FALSE );
}

void FldDebugMenu_SetCommSystem( void* commSys , DEBUG_FLDMENU *d_menu )
{
	d_menu->commSys = (FIELD_COMM_MAIN*)commSys;
}


//======================================================================
//	デバッグメニュー呼び出し
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用カメラ
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_GridCamera( DEBUG_FLDMENU *wk )
{
	DEBUG_FLDMENU *d_menu = wk;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_Camera( fieldWork );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用スケール切り替え
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_GridScaleSwitch( DEBUG_FLDMENU *wk )
{
	DEBUG_FLDMENU *d_menu = wk;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_ScaleChange( fieldWork );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用スケール調節
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_GridScaleControl( DEBUG_FLDMENU *wk )
{
	DEBUG_FLDMENU *d_menu = wk;
	FIELD_MAIN_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_ScaleControl( fieldWork );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　通信開始メニュー
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_OpenStartComm( DEBUG_FLDMENU *wk )
{
	wk->seq_no = 10;
	wk->commSeq = 0;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　侵入開始メニュー
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_OpenStartInvasion( DEBUG_FLDMENU *wk )
{
	wk->seq_no = 11;
	wk->commSeq = 0;
}

//======================================================================
//	デバッグメニュー どこでもジャンプ
//======================================================================
//--------------------------------------------------------------
///	DEBUG_ZONESEL どこでもジャンプ処理用ワーク
//--------------------------------------------------------------
typedef struct
{
	int seq_no;
	HEAPID heapID;
	
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
	BMP_MENULIST_DATA *pMenuList;
	BMPMENULIST_WORK *pMenuListWork;

	STRBUF *strbuf;
	GFL_MSGDATA *msgdata;
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	PRINT_UTIL printUtil[1];
//	ARCHANDLE *arcHandleMsg;
}DEBUG_ZONESEL;

//--------------------------------------------------------------
///	どこでもジャンプ　メニューヘッダー
//--------------------------------------------------------------
static const BMPMENULIST_HEADER DATA_DebugMenuList_ZoneSel =
{
	NULL,	//表示文字データポインタ
	NULL,	//カーソル移動ごとのコールバック関数
	NULL,	//一列表示ごとのコールバック関数
	NULL,	//GFL_BMPWIN

	1,		//リスト項目数
	12,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	11,		//項目表示Ｘ座標
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
///	どこでもジャンプメニューリスト　仮
//--------------------------------------------------------------
#if 0
static const DEBUG_MENU_LIST DATA_DebugMenuListZoneSel[] =
{
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
	{ DEBUG_FIELD_STR01, NULL },
};
#endif

static void DMenuZoneSelect_Init( DEBUG_ZONESEL *work, HEAPID heap_id );

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　どこでもジャンプ
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_MapZoneSelect( DEBUG_FLDMENU *wk )
{
	wk->pUserWork = GFL_HEAP_AllocClearMemory(
			wk->heapID, sizeof(DEBUG_ZONESEL) );
	DMenuZoneSelect_Init( wk->pUserWork, wk->heapID );
	wk->seq_no = 20;
}

//--------------------------------------------------------------
/**
 * どこでもジャンプ　初期化
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void DMenuZoneSelect_Init( DEBUG_ZONESEL *work, HEAPID heap_id )
{
	work->heapID = heap_id;
	
	{	//window frame
		BmpWinFrame_GraphicSet( DEBUG_BGFRAME_MENU, 1,
				DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, work->heapID );
	}
	
	{	//bmp window
		work->bmpwin = GFL_BMPWIN_Create(
			DEBUG_BGFRAME_MENU,
			1, 1, 11, 16,
			DEBUG_FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		work->bmp = GFL_BMPWIN_GetBmp( work->bmpwin );
		
		GFL_BMP_Clear( work->bmp, 0xff );
		GFL_BMPWIN_MakeScreen( work->bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( work->bmpwin );
		GFL_BG_LoadScreenReq( DEBUG_BGFRAME_MENU );
	}
	
	{	//message
		work->msgdata = GFL_MSG_Create(
			GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
			NARC_message_d_field_dat, work->heapID );
		
		work->strbuf = GFL_STR_CreateBuffer( 1024, work->heapID );

		#if 0
		work->arcHandleMsg = GFL_ARC_OpenDataHandle(
				ARCID_D_TAYA, work->heapID );
		work->fontHandle = GFL_FONT_CreateHandle(
			work->arcHandleMsg, NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, work->heapID );
		#else
			work->fontHandle = GFL_FONT_Create(
				ARCID_D_TAYA,
				NARC_d_taya_lc12_2bit_nftr,
				GFL_FONT_LOADTYPE_FILE,
				FALSE,
				work->heapID );
		#endif
		work->printQue = PRINTSYS_QUE_Create( work->heapID );
		PRINT_UTIL_Setup( work->printUtil, work->bmpwin );
	}
}

//--------------------------------------------------------------
/**
 * どこでもジャンプ　メイン
 * @param
 * @retval
 */
//--------------------------------------------------------------
static int DMenuZoneSelect_Main( DEBUG_FLDMENU *wk )
{
	DEBUG_ZONESEL *work = wk->pUserWork;
	
	switch( work->seq_no ){
	case 0:
		{	//Menu List
			int i;
			u32 max = ZONEDATA_GetZoneIDMax();
			BMPMENULIST_HEADER head = DATA_DebugMenuList_ZoneSel;
//			const DEBUG_MENU_LIST *d_menu_list;
			
			BmpWinFrame_Write( work->bmpwin,
				WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
			
			work->pMenuList = BmpMenuWork_ListCreate( max, work->heapID );
//			d_menu_list = DATA_DebugMenuListZoneSel;
			DEBUG_SetMenuWorkZoneIDName( work->pMenuList, work->heapID );
			
			head.count = max;
			head.msgdata = work->msgdata;
			head.print_util = work->printUtil;
			head.print_que = work->printQue;
			head.font_handle = work->fontHandle;
			head.win = work->bmpwin;
			head.list = work->pMenuList;
			
			work->pMenuListWork =
				BmpMenuList_Set( &head, 0, 0, work->heapID );
			BmpMenuList_SetCursorString( work->pMenuListWork, 0 );
		}
		
		work->seq_no++;
		break;
	case 1:
		{
			u32 ret;
			ret = BmpMenuList_Main( work->pMenuListWork );
			PRINTSYS_QUE_Main( work->printQue );
			
			if( PRINT_UTIL_Trans(work->printUtil,work->printQue) ){
			}

			switch( ret ){
			case BMPMENULIST_NULL:
				break;
			case BMPMENULIST_CANCEL:	//キャンセル
				work->seq_no++;
				break;
			default:					//決定
				//ret == zoneID
				work->seq_no++;
				break;
			}
		}
		break;
	case 2:
		BmpWinFrame_Clear( work->bmpwin, 0 );

		BmpMenuList_Exit( work->pMenuListWork, NULL, NULL );
		BmpMenuWork_ListDelete( work->pMenuList );
		
		PRINTSYS_QUE_Delete( work->printQue );
//		FontDataMan_Delete( work->fontHandle );
		GFL_FONT_Delete( work->fontHandle );
		GFL_STR_DeleteBuffer( work->strbuf );
		GFL_MSG_Delete( work->msgdata );
		GFL_BMPWIN_Delete( work->bmpwin );
		
		return( TRUE );
	}

	return( FALSE );
}

//======================================================================
//	デバッグ用文字列変換
//======================================================================
#define ASCII_EOM (0x00)
#define ASCII_0 (0x30)
#define ASCII_9 (0x39)
#define ASCII_A (0x41)
#define ASCII_Z (0x5a)

//半角
#define UTF16_U8_0 (0x0030)
#define UTF16_U8_9 (0x0039)
#define UTF16_U8_A (0x0041)
#define UTF16_U8_Z (0x005a)

//全角
#define UTF16_0 (0xff10)
#define UTF16_9 (0xff19)
#define UTF16_A (0xff21)
#define UTF16_Z (0xff3a)

//--------------------------------------------------------------
/**
 * ASCII->UTF-16
 * @param	str	ASCII 半角英数字
 * @retval	u16 ASCII->UTF-16
 */
//--------------------------------------------------------------
static u16 DEBUG_ASCIICODE_UTF16( u8 code )
{
	if( code == ASCII_EOM ){
		return( GFL_STR_GetEOMCode() );
	}
	
	if( code >= ASCII_0 && code <= ASCII_9 ){
		code -= ASCII_0;
		return( UTF16_0 + code );
	}
	
	if( code >= ASCII_A && code <= ASCII_Z ){
		code -= ASCII_A;
		return( UTF16_A + code );
	}
	
	GF_ASSERT( 0 ); //未対応文字
	return( GFL_STR_GetEOMCode() );
}

//--------------------------------------------------------------
/**
 * ZONE_ID->STRCODE
 * @param
 * @retval
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
 * @param
 * @retval
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
 * @param
 * @retval
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

