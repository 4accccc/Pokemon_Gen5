//============================================================================================
/**
 * @file	test_layout.c
 * @brief	デバッグ用関数
 * @author	layout
 * @date	2008.08.27
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include "system/main.h"
#include "arc_def.h"

#include "layout.naix"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/bmp_menu.h"
#include "system/bmp_winframe.h"
#include "message.naix"
#include "msg/msg_d_field.h"
#include "font/font.naix" //NARC_font_large_gftr

//======================================================================
//	struct
//======================================================================
typedef struct _DEBUG_FLDMENU DEBUG_FLDMENU;

//======================================================================
//	extern
//======================================================================
extern DEBUG_FLDMENU * LayoutDebugMenu_Init( u32 heapID );
extern void LayoutDebugMenu_Delete( DEBUG_FLDMENU *d_menu );
extern void LayoutDebugMenu_Create( DEBUG_FLDMENU *d_menu );
extern BOOL LayoutDebugMenu_Main( DEBUG_FLDMENU *d_menu );


//-------------------------------------------------------------------------

typedef struct{
    int nscr;
    int ncgr;
    int nclr;
    int frame;
    int priority;
    int paletteBit;
} LAYOUTEDIT_BODY;



#include "layoutediter.c"



typedef struct
{
	int				heapID;	
	GFL_BMPWIN*		win;
	GFL_BMP_DATA*	bmp;
    int layoutno;
    DEBUG_FLDMENU* LayoutMenuHandle;
    
} LAYOUT_WORK;


//----------------------------------------------------------------------------
/**
 *	@brief	アサートから抜ける
 *
 *	L+R+X+Y
 */
//-----------------------------------------------------------------------------
static void GFUser_AssertDispFinishFunc( void )
{
	int key_cont;

	
	while(1){

		GFL_UI_Main();
		key_cont = GFL_UI_KEY_GetCont();
		// キーが押されるまでループ
		if( (key_cont & PAD_BUTTON_L) && (key_cont & PAD_BUTTON_R) &&
			(key_cont & PAD_BUTTON_X) && (key_cont & PAD_BUTTON_Y) ){
			return ;
		}

		// VBlank待ち
		// (これを有効にすると、デバッカで停止させたときにコールスタックが表示されない)
//		OS_WaitIrq(TRUE,OS_IE_V_BLANK);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	レイアウトデータに従い表示
 */
//-----------------------------------------------------------------------------

static void GFL_Layout_Disp(int no, LAYOUTEDIT_BODY* pBody, int* pHead,int heapID)
{
    int start = pHead[no];
    int end = pHead[no+1];
    int i,p,pal,startpal,endpal;
    int sizeOffset,addSize;
    int mainSizeOffset = 0,subSizeOffset = 0;

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

    for(i=start;i < end;i++){
        startpal = endpal = -1;
        if(pBody[i].frame < GFL_BG_FRAME0_S){
            sizeOffset = mainSizeOffset;
            pal = PALTYPE_MAIN_BG;
        }
        else{
            sizeOffset = subSizeOffset;
            pal = PALTYPE_SUB_BG;
        }
        addSize = GFL_ARC_UTIL_TransVramBgCharacter( ARCID_LAYOUT, pBody[i].ncgr , pBody[i].frame, 0, 0, 0, heapID );

        for(p = 0 ; p < 16 ; p++){
            if( pBody[i].paletteBit & (0x1<<p) ){
                OS_TPrintf("pal %d %d %d\n",i,startpal,endpal);
                if(startpal==-1){
                    startpal = p * 32;
                }
                if(endpal < p * 32){
                    endpal =  p * 32;
                }
            }
        }
        GFL_ARC_UTIL_TransVramPaletteEx( ARCID_LAYOUT, pBody[i].nclr , pal, startpal, startpal, endpal - startpal + 32, heapID );
        GFL_ARC_UTIL_TransVramScreen( ARCID_LAYOUT, pBody[i].nscr, pBody[i].frame, sizeOffset, 0, 0, heapID );

        if(pBody[i].frame < GFL_BG_FRAME0_S){
            mainSizeOffset += addSize;
        }
        else{
            subSizeOffset += addSize;
        }
        GFL_BG_SetPriority(pBody[i].frame,  pBody[i].priority);
		GFL_BG_SetVisible( pBody[i].frame,   VISIBLE_ON );
    }
}

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugLayoutMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	LAYOUT_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LAYOUT, 0x10000 );
	wk = GFL_PROC_AllocWork( proc, sizeof( LAYOUT_WORK ), HEAPID_LAYOUT );
	wk->heapID = HEAPID_LAYOUT;
    wk->layoutno = 0;
		
	{
		static const GFL_DISP_VRAM dispvramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット			
			GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
			GX_OBJVRAMMODE_CHAR_1D_128K,	// サブOBJマッピングモード
		};		
		GFL_DISP_SetBank( &dispvramBank );
	}	
	
	G2_BlendNone();
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );
  GFL_FONTSYS_Init();
  
/*		
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}
		
	{
		static const GFL_BG_BGCNT_HEADER bg0cntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		static const GFL_BG_BGCNT_HEADER bg1cntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bg0cntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bg1cntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bg0cntText,   GFL_BG_MODE_TEXT );

		///< main

		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0x00, 1, 0 );


        //	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_LAYOUT, NARC_C__home_layout_layout_kaseki_board_NCGR, GFL_BG_FRAME0_M, 0, 0, 0, wk->heapID );
///		GFL_ARC_UTIL_TransVramPalette( ARCID_LAYOUT, NARC_C__home_layout_layout_kaseki_board_NCLR, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
///		GFL_ARC_UTIL_TransVramScreen( ARCID_LAYOUT, NARC_C__home_layout_layout_kaseki_board1_NSCR, GFL_BG_FRAME0_M, 0, 0, 0, wk->heapID );
//		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
//		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		///< sub
		
		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
//		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
//		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
        GFL_Layout_Disp(wk->layoutno,LayoutEditorTable,LayoutEditorTableNum, wk->heapID);

    }		
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );

	CommErrorSys_Setup();

    // アサート回避関数の設定
    GFL_ASSERT_SetDisplayFunc( NULL, NULL, GFUser_AssertDispFinishFunc );
*/
    wk->LayoutMenuHandle = LayoutDebugMenu_Init( wk->heapID );
    LayoutDebugMenu_Create(wk->LayoutMenuHandle);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugLayoutMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	LAYOUT_WORK* wk = mywk;

    LayoutDebugMenu_Main(wk->LayoutMenuHandle);
    
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
	{	
		return GFL_PROC_RES_FINISH;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
	{
        
        
        GFL_Layout_Disp(wk->layoutno,LayoutEditorTable,LayoutEditorTableNum,wk->heapID);
        {
            int num = sizeof(LayoutEditorTableNum)/sizeof(int)-1;
            wk->layoutno++;
            if(wk->layoutno >= num){
                wk->layoutno = 0;
            }
        }
	}
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
	}
	
	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_INC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_DEC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_INC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_DEC, 1 );
	}
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
	}

	return GFL_PROC_RES_CONTINUE;	
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugLayoutMainProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	LAYOUT_WORK* wk = mywk;

	GFL_PROC_FreeWork( proc );
	
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();
	
	GFL_HEAP_DeleteHeap( HEAPID_LAYOUT );
	
	OS_TPrintf( "proc Exit\n" );

	return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugLayoutMainProcData = {
	DebugLayoutMainProcInit,
	DebugLayoutMainProcMain,
	DebugLayoutMainProcExit,
};

 


//======================================================================
/**
 *
 * @file	field_debug.c
 * @brief	フィールドデバッグ
 * @author	kagaya
 * @data	08.09.29
 */
//======================================================================


//======================================================================
//	define
//======================================================================
#define DEBUG_BGFRAME_MENU (GFL_BG_FRAME2_M)	//使用フレーム
#define DEBUG_MENU_PANO	(14)
#define DEBUG_FONT_PANO	(15)

#define D_MENU_CHARPOS_X (1)		//メニュー位置
#define D_MENU_CHARPOS_Y (18)		//メニュー位置
#define D_MENU_CHARSIZE_X (28)		//メニュー横幅
#define D_MENU_CHARSIZE_Y (4)		//メニュー縦幅

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
//	ARCHANDLE *arcHandleMsg;
	
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
DEBUG_FLDMENU * LayoutDebugMenu_Init( u32 heapID )
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
void LayoutDebugMenu_Delete( DEBUG_FLDMENU *d_menu )
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
void LayoutDebugMenu_Create( DEBUG_FLDMENU *d_menu )
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
			D_MENU_CHARPOS_X, D_MENU_CHARPOS_Y, D_MENU_CHARSIZE_X, D_MENU_CHARSIZE_Y,
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

    d_menu->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                                          GFL_FONT_LOADTYPE_FILE , FALSE , d_menu->heapID );
    
//		d_menu->fontHandle = GFL_FONT_Create( ARCID_D_TAYA,
//			 NARC_d_taya_lc12_2bit_nftr,
//			GFL_FONT_LOADTYPE_FILE, FALSE, d_menu->heapID );

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
BOOL LayoutDebugMenu_Main( DEBUG_FLDMENU *d_menu )
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
			
            if(!PRINT_UTIL_Trans(d_menu->printUtil,d_menu->printQue)){
                return( FALSE );
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
		
		GFL_FONT_Delete( d_menu->fontHandle );
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
