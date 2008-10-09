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
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "message.naix"

#include "test_graphic/d_taya.naix"
#include "msg/msg_d_field.h"
#include "msg/msg_d_comm_menu.h"

#include "ari_field_menu.h"

//======================================================================
//	define
//======================================================================
#define DEBUG_BGFRAME_MENU (GFL_BG_FRAME1_M)	//使用フレーム
#define DEBUG_BGFRAME_MSG  (GFL_BG_FRAME2_M)	//使用フレーム
#define DEBUG_BGFRAME_SUB  (GFL_BG_FRAME3_M)	//使用フレーム
#define DEBUG_MENU_PANO	(14)
#define DEBUG_FONT_PANO	(15)

#define D_MENU_CHARSIZE_X (14)		//メニュー横幅
#define D_MENU_CHARSIZE_Y (18)		//メニュー縦幅

#define D_MSG_CHAR_TOP	 ( 1)
#define D_MSG_CHAR_LEFT	 (19)

#define D_MSG_CHARSIZE_X (30)		//メッセージウィンドウ横幅
#define D_MSG_CHARSIZE_Y ( 4)		//メッセージウィンドウ縦幅

#define D_MSG_STR_TOP	 (0)
#define D_MSG_STR_LEFT   (0)


//======================================================================
//	enum
//======================================================================
enum ARI_FLDMENU_MAIN_STATE
{
	FMS_INIT_TOPMENU,	//対戦方法選択
	FMS_LOOP_TOPMENU,
	FMS_TERM_TOPMENU,
	FMS_INIT_RULETOP,	//ルールの使用・不使用選択
	FMS_LOOP_RULETOP,
	FMS_TERM_RULETOP,
	
	FMS_END,			//終了時に与えるステート

	FMS_MAX,
	
};


//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	メニュー呼び出し関数
///	呼び出す際、引数void*はDEBUG_COMMMENUが指定されます
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
///	DEBUG_COMMMENU
//--------------------------------------------------------------
struct _DEBUG_COMMMENU
{
	int seq_no;
	
	u32 bgFrame;
	u32 msgFrame;
	u32 subFrame;
	HEAPID heapID;
	//メインウィンドウ用
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
	//メッセージウィンドウ用
	GFL_BMP_DATA *msg_bmp;
	GFL_BMPWIN *msg_bmpwin;
	//選択肢用
	GFL_BMP_DATA *sub_bmp;
	GFL_BMPWIN *sub_bmpwin;
	
	STRBUF *strbuf;
	GFL_MSGDATA *msgdata;
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	PRINT_UTIL printUtil[1];
	PRINT_QUE *printQueMsg;
	PRINT_UTIL printUtilMsg[1];
	PRINT_QUE *printQueSub;
	PRINT_UTIL printUtilSub[1];
	ARCHANDLE *arcHandleMsg;
	
	BMP_MENULIST_DATA *menulistdata;
	BMPMENU_WORK *bmpmenu;
	BMP_MENULIST_DATA *menulistdata_sub;
	BMPMENU_WORK *bmpmenu_sub;
	
	D_MENU_CALLPROC call_proc;
};

//======================================================================
//	proto
//======================================================================
static void AriFldMenuCallProc_Test( void *wk );
static void AriCommMenuCallProc_ToSelectRule( void *wk );
static void AriCommMenuCallProc_CloseMenu( void *wk );

void	AriCommMenu_SetDefault_BmpMenuHeader( DEBUG_COMMMENU *d_menu, BMPMENU_HEADER *head ,u8 itemNum );

void	AriCommMenu_OpenTopMenu( DEBUG_COMMMENU *d_menu );
void	AriCommMenu_CloseTopMenu( DEBUG_COMMMENU *d_menu );
void	AriCommMenu_OpenRuleTopMenu( DEBUG_COMMMENU *d_menu );
void	AriCommMenu_CloseRuleTopMenu( DEBUG_COMMMENU *d_menu );

void	AriCommMenu_ChangeExplanation( DEBUG_COMMMENU *d_menu , u32 msgID);
void	AriCommMenu_ExplanationMain( DEBUG_COMMMENU *d_menu );

//======================================================================
//	メニューリスト一覧
//======================================================================
//--------------------------------------------------------------
///	デバッグメニューリスト一覧
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_DebugMenuList[] =
{
	{ DEB_COMM_MENU_CAPTION1, NULL },
	{ DEB_COMM_MENU_2MODE_1	, AriCommMenuCallProc_ToSelectRule },
	{ DEB_COMM_MENU_2MODE_2 , AriCommMenuCallProc_ToSelectRule },
	{ DEB_COMM_MENU_2MODE_3 , AriCommMenuCallProc_ToSelectRule },
	{ DEB_COMM_MENU_CAPTION2, NULL },
	{ DEB_COMM_MENU_4MODE_1	, AriCommMenuCallProc_ToSelectRule },
	{ DEB_COMM_MENU_4MODE_2	, AriCommMenuCallProc_ToSelectRule },
	{ DEB_COMM_MENU_MANUAL	, AriFldMenuCallProc_Test },
	{ DEB_COMM_MENU_END     , AriCommMenuCallProc_CloseMenu },
};

//メニュー最大数
#define D_MENULIST_MAX (NELEMS(DATA_DebugMenuList))

//ルール使用選択用
static const DEBUG_MENU_LIST DATA_CommRuleTopMenuList[] =
{
	{ DEB_COMM_MENU_CHOICE10, AriFldMenuCallProc_Test },
	{ DEB_COMM_MENU_CHOICE11, AriFldMenuCallProc_Test },
	{ DEB_COMM_MENU_END     , AriCommMenuCallProc_CloseMenu },
};

//メニュー最大数
#define D_RULETOPMENULIST_MAX (NELEMS(DATA_CommRuleTopMenuList))

//======================================================================
//	フィールドデバッグメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　初期化
 * @param	heapID	ヒープID
 * @retval	DEBUG_COMMMENU
 */
//--------------------------------------------------------------
DEBUG_COMMMENU *AriFldMenu_Init( u32 heapID )
{
	DEBUG_COMMMENU *d_menu;
	d_menu = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEBUG_COMMMENU) );
	
	d_menu->heapID = heapID;
	d_menu->bgFrame = DEBUG_BGFRAME_MENU;
	d_menu->msgFrame = DEBUG_BGFRAME_MSG;
	d_menu->subFrame = DEBUG_BGFRAME_SUB;
	
	{	//bmp font いずれメイン側で
		GFL_BMPWIN_Init( d_menu->heapID );
		GFL_BMPWIN_Init( d_menu->heapID );
		GFL_FONTSYS_Init();
	}
	
	{	//BG初期化 いずれメイン側のを利用する形へ
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, 0x6000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_BGCNT_HEADER bgcntTextMsg = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
	
		GFL_BG_BGCNT_HEADER bgcntTextSub = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( d_menu->bgFrame, &bgcntText   , GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( d_menu->msgFrame , &bgcntTextMsg, GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( d_menu->subFrame , &bgcntTextSub, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( d_menu->bgFrame, VISIBLE_ON );
		GFL_BG_SetVisible( d_menu->msgFrame , VISIBLE_ON );
		GFL_BG_SetVisible( d_menu->subFrame , VISIBLE_ON );
		
		GFL_BG_SetPriority( d_menu->bgFrame, 2 );
		GFL_BG_SetPriority( d_menu->msgFrame , 1 );
		GFL_BG_SetPriority( d_menu->subFrame , 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, 3 );

		GFL_ARC_UTIL_TransVramPalette(
			ARCID_D_TAYA, NARC_d_taya_default_nclr,
			PALTYPE_MAIN_BG, DEBUG_FONT_PANO*32, 32, d_menu->heapID );

		GFL_BG_FillCharacter( d_menu->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillCharacter( d_menu->msgFrame , 0x00, 1, 0 );
		GFL_BG_FillCharacter( d_menu->subFrame , 0x00, 1, 0 );
		GFL_BG_FillScreen( d_menu->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_FillScreen( d_menu->msgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_FillScreen( d_menu->subFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_LoadScreenReq( d_menu->bgFrame );
		GFL_BG_LoadScreenReq( d_menu->msgFrame );
		GFL_BG_LoadScreenReq( d_menu->subFrame );
	}
	
	return( d_menu );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　削除
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	nMENU_TYPE_SYSTEMothing
 */
//--------------------------------------------------------------
void AriFldMenu_Delete( DEBUG_COMMMENU *d_menu )
{
	{	//とりあえずこちらで　いずれはメイン側
		GFL_BMPWIN_Exit();
		GFL_BG_FreeCharacterArea( d_menu->bgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( d_menu->bgFrame );
		GFL_BG_FreeCharacterArea( d_menu->msgFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( d_menu->msgFrame );
		GFL_BG_FreeCharacterArea( d_menu->subFrame, 0x00, 0x20 );
		GFL_BG_FreeBGControl( d_menu->subFrame );
	}
	
	GFL_HEAP_FreeMemory( d_menu );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグメニュー　メニュー作成
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	nothing
 */
//--------------------------------------------------------------
void AriFldMenu_Create( DEBUG_COMMMENU *d_menu )
{
	{	//work
		d_menu->seq_no = FMS_INIT_TOPMENU;
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( d_menu->bgFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, d_menu->heapID );
	}
		//ウィンドウ生成時でも可(たぶん
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
			NARC_message_d_comm_menu_dat, d_menu->heapID );
		
		d_menu->strbuf = GFL_STR_CreateBuffer( 2048, d_menu->heapID );
		
		d_menu->arcHandleMsg = GFL_ARC_OpenDataHandle(
				ARCID_D_TAYA, d_menu->heapID );
		
		d_menu->fontHandle = GFL_FONT_CreateHandle(
			d_menu->arcHandleMsg, NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, d_menu->heapID );
		
		PRINTSYS_Init( d_menu->heapID );
		d_menu->printQue = PRINTSYS_QUE_Create( d_menu->heapID );
		d_menu->printQueMsg = PRINTSYS_QUE_Create( d_menu->heapID );
		d_menu->printQueSub = PRINTSYS_QUE_Create( d_menu->heapID );
	
		PRINT_UTIL_Setup( d_menu->printUtil, d_menu->bmpwin );	
	}

	//メッセージウィンドウ用初期化
	{	//window frame
		BmpWinFrame_GraphicSet( d_menu->msgFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, d_menu->heapID );
	}

	//ウィンドウ生成時でも可(たぶん
	{	//bmpwin
		d_menu->msg_bmpwin = GFL_BMPWIN_Create( d_menu->msgFrame,
			D_MSG_CHAR_TOP , D_MSG_CHAR_LEFT , 
			D_MSG_CHARSIZE_X, D_MSG_CHARSIZE_Y,
			DEBUG_FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		d_menu->msg_bmp = GFL_BMPWIN_GetBmp( d_menu->msg_bmpwin );
		
		GFL_BMP_Clear( d_menu->msg_bmp, 0xff );
		GFL_BMPWIN_MakeScreen( d_menu->msg_bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( d_menu->msg_bmpwin );
		GFL_BG_LoadScreenReq( d_menu->bgFrame );
	}

	{	//msg
		PRINT_UTIL_Setup( d_menu->printUtilMsg, d_menu->msg_bmpwin );
	}
	
	//選択肢ウィンドウ用初期化
	{	//window frame
		BmpWinFrame_GraphicSet( d_menu->subFrame, 1,
			DEBUG_MENU_PANO, MENU_TYPE_SYSTEM, d_menu->heapID );
	}

	//ウィンドウ生成時でやる
	/*
	{	//bmpwin
		d_menu->sub_bmpwin = GFL_BMPWIN_Create( d_menu->subFrame,
			5, 5, 
			25, 5,
			DEBUG_FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		d_menu->sub_bmp = GFL_BMPWIN_GetBmp( d_menu->sub_bmpwin );
		
		GFL_BMP_Clear( d_menu->sub_bmp, 0xff );
		GFL_BMPWIN_MakeScreen( d_menu->sub_bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( d_menu->sub_bmpwin );
		GFL_BG_LoadScreenReq( d_menu->subFrame );
	}

	{	//msg
		PRINT_UTIL_Setup( d_menu->printUtilSub, d_menu->sub_bmpwin );
	}
	*/

}

//--------------------------------------------------------------
/**
 * リストメニュー用ヘッダ標準設定関数
 * @param	d_menu	DEBUG_COMMMENU
 * @param	head	BMPMENU_HEADER 設定したいヘッダ
 * @param	itemNum	メニューの項目数
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_SetDefault_BmpMenuHeader( DEBUG_COMMMENU *d_menu, BMPMENU_HEADER *head ,u8 itemNum )
{
	head->x_max = 1;
	head->y_max = itemNum;
	head->line_spc = 4;
	head->c_disp_f = 0;
	head->loop_f = 1;
		
	head->font_size_x = 12;
	head->font_size_y = 12;
	head->msgdata = d_menu->msgdata;
	head->font_handle = d_menu->fontHandle;
}


//--------------------------------------------------------------
/**
 * トップメニュー(対戦方法選択)開く
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_OpenTopMenu( DEBUG_COMMMENU *d_menu )
{
	{	//window frame
		BmpWinFrame_Write( d_menu->bmpwin,
			WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
		BmpWinFrame_Write( d_menu->msg_bmpwin,
			WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
	}
		
	{	//menu create
		u32 i,lmax;
		BMPMENU_HEADER head;
		const DEBUG_MENU_LIST *d_menu_list;
			
		lmax = D_MENULIST_MAX;
		AriCommMenu_SetDefault_BmpMenuHeader( d_menu , &head , lmax );
		head.print_util = d_menu->printUtil;
		head.print_que = d_menu->printQue;
		head.win = d_menu->bmpwin;
		d_menu->menulistdata =
			BmpMenuWork_ListCreate( lmax, d_menu->heapID );
			
		d_menu_list = DATA_DebugMenuList;
			
		for( i = 0; i < lmax; i++ ){
#if 0	//見出しのスキップが巧くできないのでとりあえず保留
			if( d_menu_list[i].callp == NULL ){
			BMPMENULIST_RABEL	BmpMenuWork_ListAddArchiveString(
					d_menu->menulistdata, d_menu->msgdata,
			d_menu_list[i].str_id, BMPMENULIST_RABEL,
					d_menu->heapID );
			}else{
				BmpMenuWork_ListAddArchiveString(
					d_menu->menulistdata, d_menu->msgdata,
					d_menu_list[i].str_id, (u32)d_menu_list[i].callp,
					d_menu->heapID );
			}
#endif
				BmpMenuWork_ListAddArchiveString(
					d_menu->menulistdata, d_menu->msgdata,
					d_menu_list[i].str_id, (u32)d_menu_list[i].callp,
					d_menu->heapID );
		}
		
		head.menu = d_menu->menulistdata;
		
		d_menu->bmpmenu = BmpMenu_Add( &head, 0, d_menu->heapID );
		//初期カーソルを一個下げる
		BmpMenu_MainOutControl( d_menu->bmpmenu , 0 );
		//初期メッセージ
		BmpMenu_SetCursorString( d_menu->bmpmenu, DEB_COMM_MENU_CURSOR );
	}
	
	d_menu->seq_no = FMS_LOOP_TOPMENU;
	AriCommMenu_ChangeExplanation( d_menu , DEB_COMM_MENU_STR_10 );
}

//--------------------------------------------------------------
/**
 * トップメニュー(対戦方法選択)閉じる
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_CloseTopMenu( DEBUG_COMMMENU *d_menu )
{
	//クリア周り
	//終了コマンドの受付後に呼んでたけど、ここに移動
	GFL_BMP_Clear( d_menu->bmp, 0x00 );
	GFL_BMPWIN_MakeScreen( d_menu->bmpwin );
	GFL_BMPWIN_TransVramCharacter( d_menu->bmpwin );
	BmpWinFrame_Clear( d_menu->bmpwin, 0 );
	
	GFL_BMP_Clear( d_menu->msg_bmp, 0x00 );
	GFL_BMPWIN_MakeScreen( d_menu->msg_bmpwin );
	GFL_BMPWIN_TransVramCharacter( d_menu->msg_bmpwin );
	BmpWinFrame_Clear( d_menu->msg_bmpwin, 0 );

	//開放周り
	BmpMenu_Exit( d_menu->bmpmenu, NULL );
	BmpMenuWork_ListSTRBUFDelete( d_menu->menulistdata );
		
	//プリントユーティリティ削除　いらない
	PRINTSYS_QUE_Delete( d_menu->printQue );
		
	FontDataMan_Delete( d_menu->fontHandle );
//	GFL_ARC_CloseDataHandle( d_menu->arcHandleMsg ); //マネージャ側で済み
	GFL_STR_DeleteBuffer( d_menu->strbuf );
	GFL_MSG_Delete( d_menu->msgdata );
	GFL_BMPWIN_Delete( d_menu->bmpwin );
	GFL_BMPWIN_Delete( d_menu->msg_bmpwin );
}

//--------------------------------------------------------------
/**
 * ルール選択トップメニュー(通常か特殊か)開く
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_OpenRuleTopMenu( DEBUG_COMMMENU *d_menu )
{
	{	//bmpwin
		d_menu->sub_bmpwin = GFL_BMPWIN_Create( d_menu->subFrame,
			16, 8, 
			12, 6,
			DEBUG_FONT_PANO, GFL_BMP_CHRAREA_GET_B );
		d_menu->sub_bmp = GFL_BMPWIN_GetBmp( d_menu->sub_bmpwin );
		
		GFL_BMP_Clear( d_menu->sub_bmp, 0xff );
		GFL_BMPWIN_MakeScreen( d_menu->sub_bmpwin );
		
		GFL_BMPWIN_TransVramCharacter( d_menu->sub_bmpwin );
		GFL_BG_LoadScreenReq( d_menu->subFrame );
	}
	PRINT_UTIL_Setup( d_menu->printUtilSub, d_menu->sub_bmpwin );

	{	//window frame
		BmpWinFrame_Write( d_menu->sub_bmpwin,
			WINDOW_TRANS_ON, 1, DEBUG_MENU_PANO );
	}
		
	{	//menu create
		u32 i,lmax;
		BMPMENU_HEADER head;
		const DEBUG_MENU_LIST *d_menu_list;
			
		lmax = D_RULETOPMENULIST_MAX;
		AriCommMenu_SetDefault_BmpMenuHeader( d_menu , &head , lmax );
		head.print_util = d_menu->printUtilSub;
		head.print_que = d_menu->printQueSub;
		head.win = d_menu->sub_bmpwin;

		d_menu->menulistdata_sub =
			BmpMenuWork_ListCreate( lmax, d_menu->heapID );
			
		d_menu_list = DATA_CommRuleTopMenuList;
			
		for( i = 0; i < lmax; i++ ){
				BmpMenuWork_ListAddArchiveString(
					d_menu->menulistdata_sub, d_menu->msgdata,
					d_menu_list[i].str_id, (u32)d_menu_list[i].callp,
					d_menu->heapID );
		}
		
		head.menu = d_menu->menulistdata_sub;
		
		d_menu->bmpmenu_sub = BmpMenu_Add( &head, 0, d_menu->heapID );
		BmpMenu_SetCursorString( d_menu->bmpmenu_sub, DEB_COMM_MENU_CURSOR );
	}
	
	d_menu->seq_no = FMS_LOOP_RULETOP;
	AriCommMenu_ChangeExplanation( d_menu , DEB_COMM_MENU_STR_20 );
}


//--------------------------------------------------------------
/**
 * ルール選択トップメニュー(通常か特殊か)閉じる
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_CloseRuleTopMenu( DEBUG_COMMMENU *d_menu )
{
	//クリア周り
	//終了コマンドの受付後に呼んでたけど、ここに移動
	GFL_BMP_Clear( d_menu->sub_bmp, 0x00 );
	GFL_BMPWIN_MakeScreen( d_menu->sub_bmpwin );
	GFL_BMPWIN_TransVramCharacter( d_menu->sub_bmpwin );
	BmpWinFrame_Clear( d_menu->sub_bmpwin, 0 );
	
	//開放周り
	BmpMenu_Exit( d_menu->bmpmenu_sub, NULL );
	BmpMenuWork_ListSTRBUFDelete( d_menu->menulistdata_sub );
		
	//プリントユーティリティ削除　いらない
	PRINTSYS_QUE_Delete( d_menu->printQueSub );
	GFL_BMPWIN_Delete( d_menu->sub_bmpwin );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　説明文変更
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_ChangeExplanation( DEBUG_COMMMENU *d_menu , u32 msgID)
{
	//文字列をメッセージデータから引き出すためのバッファ
	STRBUF *strTemp;

	//前回文字列の消去
	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(d_menu->msg_bmpwin),
				D_MSG_STR_LEFT, D_MSG_STR_TOP,
				D_MSG_CHARSIZE_X*8, D_MSG_CHARSIZE_Y*8, 0xff );
				
	strTemp = GFL_STR_CreateBuffer( 64, d_menu->heapID );
	GFL_MSG_GetString( d_menu->msgdata , msgID , strTemp );

	PRINT_UTIL_Print(
		d_menu->printUtilMsg, d_menu->printQueMsg,
		D_MSG_STR_LEFT, D_MSG_STR_TOP , (void*)strTemp, d_menu->fontHandle );
	GFL_STR_DeleteBuffer( strTemp );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　説明文メイン
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	void	
 */
//--------------------------------------------------------------
void	AriCommMenu_ExplanationMain( DEBUG_COMMMENU *d_menu )
{
	u32 msgID;
	return;
	
	//次の文字列の設定
	switch( BmpMenu_CursorPosGet(d_menu->bmpmenu) ){
	case 0:
	case 1:
		AriCommMenu_ChangeExplanation( d_menu , DEB_COMM_MENU_STR_10 );
		break;
	default:
		AriCommMenu_ChangeExplanation( d_menu , DEB_COMM_MENU_STR_20 );
		break;
	}
}


//--------------------------------------------------------------
/**
 * デバッグメニュー　メイン
 * @param	d_menu	DEBUG_COMMMENU
 * @retval	BOOL	TRUE=終了
 */
//--------------------------------------------------------------
BOOL AriFldMenu_Main( DEBUG_COMMMENU *d_menu )
{
	switch( d_menu->seq_no ){
	case FMS_INIT_TOPMENU:	//メニュー作成
		AriCommMenu_OpenTopMenu( d_menu );
		break;
	case FMS_LOOP_TOPMENU:	//メニュー処理
		{
			u32 ret;
			
			ret = BmpMenu_Main( d_menu->bmpmenu );
			PRINTSYS_QUE_Main( d_menu->printQue );
			if( PRINT_UTIL_Trans(d_menu->printUtil,d_menu->printQue) ){
			}
			
			PRINTSYS_QUE_Main( d_menu->printQueMsg );
			if( PRINT_UTIL_Trans(d_menu->printUtilMsg,d_menu->printQueMsg) ){
			}
			
			switch( ret ){
			case BMPMENU_NULL:
				break;
			case BMPMENU_CANCEL:
				/* //開放処理のところへ移動
				GFL_BMP_Clear( d_menu->bmp, 0x00 );
				GFL_BMPWIN_MakeScreen( d_menu->bmpwin );
				GFL_BMPWIN_TransVramCharacter( d_menu->bmpwin );
				BmpWinFrame_Clear( d_menu->bmpwin, 0 );
	
				GFL_BMP_Clear( d_menu->msg_bmp, 0x00 );
				GFL_BMPWIN_MakeScreen( d_menu->msg_bmpwin );
				GFL_BMPWIN_TransVramCharacter( d_menu->msg_bmpwin );
				*/
				d_menu->seq_no = FMS_TERM_TOPMENU;
				break;
			default:
				/* //開放処理のところへ移動
				GFL_BMP_Clear( d_menu->bmp, 0x00 );
				GFL_BMPWIN_MakeScreen( d_menu->bmpwin );
				GFL_BMPWIN_TransVramCharacter( d_menu->bmpwin );
				BmpWinFrame_Clear( d_menu->bmpwin, 0 );
				
				GFL_BMP_Clear( d_menu->msg_bmp, 0x00 );
				GFL_BMPWIN_MakeScreen( d_menu->msg_bmpwin );
				GFL_BMPWIN_TransVramCharacter( d_menu->msg_bmpwin );
				BmpWinFrame_Clear( d_menu->msg_bmpwin, 0 );
				*/
				
				d_menu->call_proc = (D_MENU_CALLPROC)ret;
				d_menu->seq_no = FMS_TERM_TOPMENU;
				break;
			}
			
			if(	BmpMenu_MoveSiteGet(d_menu->bmpmenu) == BMPMENU_MOVE_UP || 
				BmpMenu_MoveSiteGet(d_menu->bmpmenu) == BMPMENU_MOVE_DOWN ){

				AriCommMenu_ExplanationMain( d_menu );
			}
				
		
		}
		break;
	case FMS_TERM_TOPMENU:	//削除
		
		if( d_menu->call_proc != NULL ){
			d_menu->call_proc( d_menu );
		}else{
			d_menu->seq_no = FMS_END;
		}
		
		d_menu->call_proc = NULL;
		
		if( d_menu->seq_no == FMS_END ){
			AriCommMenu_CloseTopMenu(d_menu);	
			return( TRUE );
		}
		break;

	//	ルール使用選択	
	case FMS_INIT_RULETOP:	//メニュー作成
		AriCommMenu_OpenRuleTopMenu( d_menu );
		break;
	case FMS_LOOP_RULETOP:	//メニュー処理
		{
			u32 ret;
			
			ret = BmpMenu_Main( d_menu->bmpmenu_sub );

		//	PRINTSYS_QUE_Main( d_menu->printQue );
		//	if( PRINT_UTIL_Trans(d_menu->printUtil,d_menu->printQue) ){
		//	}
			
			PRINTSYS_QUE_Main( d_menu->printQueMsg );
			if( PRINT_UTIL_Trans(d_menu->printUtilMsg,d_menu->printQueMsg) ){
			}
			
			PRINTSYS_QUE_Main( d_menu->printQueSub );
			if( PRINT_UTIL_Trans(d_menu->printUtilSub,d_menu->printQueSub) ){
			}
			
			switch( ret ){
			case BMPMENU_NULL:
				break;
			case BMPMENU_CANCEL:
				d_menu->seq_no = FMS_TERM_RULETOP;
				break;
			default:
				
				d_menu->call_proc = (D_MENU_CALLPROC)ret;
				d_menu->seq_no = FMS_TERM_RULETOP;
				break;
			}
			
			if(	BmpMenu_MoveSiteGet(d_menu->bmpmenu) == BMPMENU_MOVE_UP || 
				BmpMenu_MoveSiteGet(d_menu->bmpmenu) == BMPMENU_MOVE_DOWN ){

				AriCommMenu_ExplanationMain( d_menu );
			}
		}
		break;
	case FMS_TERM_RULETOP:	//削除
		
		if( d_menu->call_proc != NULL ){
			d_menu->call_proc( d_menu );
		}else{
			d_menu->seq_no = FMS_END;
		}
		
		d_menu->call_proc = NULL;
		
		if( d_menu->seq_no == FMS_END ){
			AriCommMenu_CloseRuleTopMenu(d_menu);	
			AriCommMenu_CloseTopMenu(d_menu);	
			return( TRUE );
		}
		break;	
	
	}
	
	return( FALSE );
}

//======================================================================
//	デバッグメニュー呼び出し　テスト
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　テスト
 * @param	wk	DEBUG_COMMMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void AriFldMenuCallProc_Test( void *wk )
{
//	DEBUG_COMMMENU *d_menu = wk;
//	HEAPID DebugHeapID = d_menu->heapID;
	OS_TPrintf("test outpur\n");
}


//======================================================================
//	メニュー項目用コールバック
//======================================================================
//--------------------------------------------------------------
/**
 * TOP→ルール選択用
 * @param	wk	DEBUG_COMMMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void AriCommMenuCallProc_ToSelectRule( void *wk )
{
	DEBUG_COMMMENU *d_menu = wk;
	OS_TPrintf("go rule select\n");
	d_menu->seq_no = FMS_INIT_RULETOP;
}

//--------------------------------------------------------------
/**
 * TOP→閉じる
 * @param	wk	DEBUG_COMMMENU*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void AriCommMenuCallProc_CloseMenu( void *wk )
{
	DEBUG_COMMMENU *d_menu = wk;
	d_menu->seq_no = FMS_END;
}


