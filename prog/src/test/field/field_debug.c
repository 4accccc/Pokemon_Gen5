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
#include "message.naix"

#include "test_graphic/d_taya.naix"
#include "msg/msg_d_field.h"

#include "field_debug.h"
#include "field_comm/field_comm_main.h"

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
	ARCHANDLE *arcHandleMsg;
	
	BMP_MENULIST_DATA *menulistdata;
	BMPMENU_WORK *bmpmenu;
	D_MENU_CALLPROC call_proc;

	//通信メニュー用
	int commSeq;	//追加 Ari1111
	FIELD_COMM_MAIN	*commSys;
};

//======================================================================
//	proto
//======================================================================
static void DMenuCallProc_GridCamera( DEBUG_FLDMENU *wk );
static void DMenuCallProc_GridScaleSwitch( DEBUG_FLDMENU *wk );
static void DMenuCallProc_GridScaleControl( DEBUG_FLDMENU *wk );
static void DMenuCallProc_OpenStartComm( DEBUG_FLDMENU *wk );
static void DMenuCallProc_OpenStartInvasion( DEBUG_FLDMENU *wk );

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
//#define DEBUG_HEAPERROR

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
		#ifndef DEBUG_HEAPERROR
		BmpWinFrame_GraphicSet( d_menu->bgFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, d_menu->heapID );
		#endif
	}
	
	{	//bmpwin
		#ifndef DEBUG_HEAPERROR
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
		#endif
	}
	
	{	//msg
		d_menu->msgdata = GFL_MSG_Create(
			GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
			NARC_message_d_field_dat, d_menu->heapID );
		
		d_menu->strbuf = GFL_STR_CreateBuffer( 1024, d_menu->heapID );
		
		d_menu->arcHandleMsg = GFL_ARC_OpenDataHandle(
				ARCID_D_TAYA, d_menu->heapID );
		
		d_menu->fontHandle = GFL_FONT_CreateHandle(
			d_menu->arcHandleMsg, NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, d_menu->heapID );

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
			#ifndef DEBUG_HEAPERROR
			BmpWinFrame_Write( d_menu->bmpwin,
				WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
			#endif	
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

			#ifndef DEBUG_HEAPERROR
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
			#endif
		}
		
		d_menu->seq_no++;
		break;
	case 1:	//メニュー処理
		#ifdef DEBUG_HEAPERROR
		d_menu->seq_no++;
		break;
		#else
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
		#endif
	case 2:	//削除
		#ifndef DEBUG_HEAPERROR
		BmpMenu_Exit( d_menu->bmpmenu, NULL );
		BmpMenuWork_ListDelete( d_menu->menulistdata );	//freeheap
		#endif

		//プリントユーティリティ削除はいらない
		PRINTSYS_QUE_Delete( d_menu->printQue );
	//	PRINTSYS_Delete();	//freeheap
		FontDataMan_Delete( d_menu->fontHandle );
//		GFL_ARC_CloseDataHandle( d_menu->arcHandleMsg ); //マネージャ側で済み
		
		GFL_STR_DeleteBuffer( d_menu->strbuf );
		GFL_MSG_Delete( d_menu->msgdata );
		#ifndef DEBUG_HEAPERROR
		GFL_BMPWIN_Delete( d_menu->bmpwin );
		#endif
		if( d_menu->call_proc != NULL ){
			d_menu->call_proc( d_menu );
		}
		d_menu->call_proc = NULL;
		//通信に行くときはそのままにしたいので
		if( d_menu->seq_no == 2 ){
			return( TRUE );
		}

		//通信メニュー用処理
	case 10:
		switch( d_menu->commSeq )
		{
		case 0:
			FieldCommMain_InitStartCommMenu( d_menu->commSys );
			d_menu->commSeq++;
			break;
		case 1:
			if( FieldCommMain_LoopStartCommMenu( d_menu->commSys ) == TRUE ){
				d_menu->commSeq++;
			}
			break;
		case 2:
			FieldCommMain_TermStartCommMenu( d_menu->commSys );
			d_menu->commSeq++;
			return (TRUE);
			break;
		}
		break;
	case 11:
		switch( d_menu->commSeq )
		{
		case 0:
			FieldCommMain_InitStartInvasionMenu( d_menu->commSys );
			d_menu->commSeq++;
			break;
		case 1:
			if( FieldCommMain_LoopStartInvasionMenu( d_menu->commSys ) == TRUE ){
				d_menu->commSeq++;
			}
			break;
		case 2:
			FieldCommMain_TermStartInvasionMenu( d_menu->commSys );
			d_menu->commSeq++;
			return (TRUE);
			break;
		}
		break;
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

