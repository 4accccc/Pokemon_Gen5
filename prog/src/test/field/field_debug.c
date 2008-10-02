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

//======================================================================
//	define
//======================================================================
#define DEBUG_BGFRAME_MENU (GFL_BG_FRAME1_M)	//使用フレーム
#define DEBUG_MENU_PANO	(14)
#define DEBUG_FONT_PANO	(15)

#define D_MENU_CHARSIZE_X (8)		//メニュー横幅
#define D_MENU_CHARSIZE_Y (16)		//メニュー縦幅

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	メニュー呼び出し関数
///	呼び出す際、引数void*はDEBUG_FLDMENUが指定されます
//--------------------------------------------------------------
typedef void (* D_MENU_CALLPROC)(void*);

//--------------------------------------------------------------
///	DEBUG_MENU_LIST
//--------------------------------------------------------------
typedef struct
{
	u32 str_id;				//表示する文字列ID
	D_MENU_CALLPROC callp;	//選択された際に呼び出す関数 NULL=呼び出し無し
}DEBUG_MENU_LIST;

//--------------------------------------------------------------
///	DEBUG_FLDMENU
//--------------------------------------------------------------
struct _DEBUG_FLDMENU
{
	int seq_no;
	
	u32 bgFrame;
	HEAPID heapID;
	
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
};

//======================================================================
//	proto
//======================================================================
static void DMenuCallProc_Test( void *wk );

//======================================================================
//	メニューリスト一覧
//======================================================================
//--------------------------------------------------------------
///	デバッグメニューリスト一覧
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_DebugMenuList[] =
{
	{ DEBUG_FIELD_STR01, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR02, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR03, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR04, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR05, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR06, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR07, DMenuCallProc_Test },
	{ DEBUG_FIELD_STR08, DMenuCallProc_Test },
};

//メニュー最大数
#define D_MENULIST_MAX (NELEMS(DATA_DebugMenuList))

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
DEBUG_FLDMENU * FldDebugMenu_Init( u32 heapID )
{
	DEBUG_FLDMENU *d_menu;
	
	d_menu = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEBUG_FLDMENU) );
	
	d_menu->heapID = heapID;
	d_menu->bgFrame = DEBUG_BGFRAME_MENU;
	
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
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, 1 );

		GFL_ARC_UTIL_TransVramPalette(
			ARCID_D_TAYA, NARC_d_taya_default_nclr,
			PALTYPE_MAIN_BG, DEBUG_FONT_PANO*32, 32, d_menu->heapID );

		GFL_BG_FillCharacter( d_menu->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( d_menu->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_LoadScreenReq( d_menu->bgFrame );
	}
	
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
	{	//とりあえずこちらで　いずれはメイン側
		GFL_BMPWIN_Exit();
		GFL_BG_FreeCharacterArea( d_menu->bgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( d_menu->bgFrame );
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
		d_menu->bmpwin = GFL_BMPWIN_Create( d_menu->bgFrame,
			1, 1, D_MENU_CHARSIZE_X, D_MENU_CHARSIZE_Y,
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
		
		d_menu->arcHandleMsg = GFL_ARC_OpenDataHandle(
				ARCID_D_TAYA, d_menu->heapID );
		
		d_menu->fontHandle = GFL_FONT_CreateHandle(
			d_menu->arcHandleMsg, NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, d_menu->heapID );

		PRINTSYS_Init( d_menu->heapID );
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
			
			lmax = D_MENULIST_MAX;
			
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
			
			d_menu_list = DATA_DebugMenuList;
			
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
		BmpMenuWork_ListSTRBUFDelete( d_menu->menulistdata );
		
		//プリントユーティリティ削除　いらない
		PRINTSYS_QUE_Delete( d_menu->printQue );
		
		FontDataMan_Delete( d_menu->fontHandle );
//		GFL_ARC_CloseDataHandle( d_menu->arcHandleMsg ); //マネージャ側で済み
		GFL_STR_DeleteBuffer( d_menu->strbuf );
		GFL_MSG_Delete( d_menu->msgdata );
		GFL_BMPWIN_Delete( d_menu->bmpwin );
		
		if( d_menu->call_proc != NULL ){
			d_menu->call_proc( d_menu );
		}
		
		d_menu->call_proc = NULL;
		return( TRUE );
	}
	
	return( FALSE );
}

//======================================================================
//	デバッグメニュー呼び出し　テスト
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　テスト
 * @param	wk	DEBUG_FLDMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DMenuCallProc_Test( void *wk )
{
	DEBUG_FLDMENU *d_menu = wk;
	HEAPID DebugHeapID = d_menu->heapID;
}
