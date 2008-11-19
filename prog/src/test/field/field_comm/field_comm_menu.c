//======================================================================
/**
 * @file	field_comm_menu.c
 * @brief	フィールド通信　メニュー部分
 * @author	ariizumi
 * @data	08/11/11
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/bmp_menu.h"
#include "system/bmp_winframe.h"

#include "test/ariizumi/ari_debug.h"
#include "field_comm_menu.h"

#include "arc_def.h"
#include "message.naix"
#include "test_graphic/d_taya.naix"
#include "msg/msg_d_field.h"

//======================================================================
//	define
//======================================================================
#define F_COMM_MENU_PALETTE	(14)
#define F_COMM_FONT_PALETTE	(15)

//======================================================================
//	enum
//======================================================================
//BGエリア
#define F_COMM_BG_AREA (GFL_BMP_CHRAREA_GET_B)

//はい・いいえウィンドウサイズ
#define F_COMM_YNWIN_TOP	(13)
#define F_COMM_YNWIN_LEFT	(25)
#define F_COMM_YNWIN_WIDTH	( 6)	
#define F_COMM_YNWIN_HEIGHT	( 4)	

//メッセージウィンドウサイズ
#define F_COMM_MSGWIN_TOP		(19)
#define F_COMM_MSGWIN_LEFT		( 1)
#define F_COMM_MSGWIN_WIDTH		(30)	
#define F_COMM_MSGWIN_HEIGHT	( 4)	

//======================================================================
//	typedef struct
//======================================================================
struct _FIELD_COMM_MENU
{
	HEAPID heapID_;

	GFL_MSGDATA	*msgData_;
	GFL_FONT	*fontHandle_;

	//はい・いいえ用
//	u8				ynMenuPlane_;
	GFL_BMP_DATA	*ynMenuBmp_;
	GFL_BMPWIN		*ynMenuBmpWin_;
	BMP_MENULIST_DATA	*ynMenuList_;
	BMPMENU_WORK	*ynMenuWork_;
	PRINT_UTIL		ynMenuPrintUtil_[1];
	PRINT_QUE		*ynMenuPrintQue_;

	//メッセージウィンドウ用
	BOOL			isInitMsgWin_;
	GFL_BMP_DATA	*msgWinBmp_;
	GFL_BMPWIN		*msgWinBmpWin_;
	PRINT_UTIL		msgWinPrintUtil_[1];
	PRINT_QUE		*msgWinPrintQue_;

	//デバッグウィンドウ(Msg
#if DEB_ARI
	BOOL			isOpenDebugWin_;
#endif
};

//メニュー最大数
#define D_YESNO_MENULIST_MAX (2)


//======================================================================
//	proto
//======================================================================
FIELD_COMM_MENU* FIELD_COMM_MENU_InitCommMenu( HEAPID heapID );
void	FIELD_COMM_MENU_TermCommMenu( FIELD_COMM_MENU *commMenu );

void	FIELD_COMM_MENU_OpenYesNoMenu( u8 bgPlane , FIELD_COMM_MENU *commMenu );
const u8 FIELD_COMM_MENU_UpdateYesNoMenu( FIELD_COMM_MENU *commMenu );
void	FIELD_COMM_MENU_CloseYesNoMenu( FIELD_COMM_MENU *commMenu );

void	FIELD_COMM_MENU_OpenMessageWindow( u8 bgPlane , FIELD_COMM_MENU *commMenu );
void	FIELD_COMM_MENU_CloseMessageWindow( FIELD_COMM_MENU *commMenu );
void	FIELD_COMM_MENU_SetMessage( u16 msgID , FIELD_COMM_MENU *commMenu );
void	FIELD_COMM_MENU_UpdateMessageWindow( FIELD_COMM_MENU *commMenu );

#if DEB_ARI
void	FIELD_COMM_MENU_SwitchDebugWindow( u8 bgPlane );
void	FIELD_COMM_MENU_UpdateDebugWindow( void );
FIELD_COMM_MENU *debCommMenu = NULL;
#endif
//--------------------------------------------------------------
//	通信用メニュー作業領域　初期化
//--------------------------------------------------------------
FIELD_COMM_MENU* FIELD_COMM_MENU_InitCommMenu( HEAPID heapID )
{
	FIELD_COMM_MENU *commMenu;
	ARCHANDLE *arcHandle;
	
	commMenu = GFL_HEAP_AllocMemory( heapID , sizeof(FIELD_COMM_MENU) );
	commMenu->heapID_ = heapID;
	commMenu->isInitMsgWin_ = FALSE;

	commMenu->msgData_ = GFL_MSG_Create( 
			GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE ,
			NARC_message_d_field_dat , commMenu->heapID_ );
	commMenu->fontHandle_ = GFL_FONT_Create( ARCID_D_TAYA ,
			NARC_d_taya_lc12_2bit_nftr , GFL_FONT_LOADTYPE_FILE ,
			FALSE , commMenu->heapID_ );
#if DEB_ARI
	commMenu->isOpenDebugWin_ = FALSE;
#endif
	return commMenu;
}

//--------------------------------------------------------------
//	通信用メニュー作業領域　開放
//--------------------------------------------------------------
void	FIELD_COMM_MENU_TermCommMenu( FIELD_COMM_MENU *commMenu )
{
	GFL_MSG_Delete( commMenu->msgData_ );
	GFL_FONT_Delete( commMenu->fontHandle_ );
	GFL_HEAP_FreeMemory( commMenu );
	commMenu = NULL;
}

//--------------------------------------------------------------
//	はい・いいえ汎用　開く
//--------------------------------------------------------------
void	FIELD_COMM_MENU_OpenYesNoMenu( u8 bgPlane , FIELD_COMM_MENU *commMenu )
{
	{	//bmpwin
		commMenu->ynMenuBmpWin_ = GFL_BMPWIN_Create( bgPlane,
			F_COMM_YNWIN_LEFT, F_COMM_YNWIN_TOP, F_COMM_YNWIN_WIDTH, F_COMM_YNWIN_HEIGHT,
			F_COMM_FONT_PALETTE, F_COMM_BG_AREA );
		commMenu->ynMenuBmp_ = GFL_BMPWIN_GetBmp( commMenu->ynMenuBmpWin_ );
		
		GFL_BMP_Clear( commMenu->ynMenuBmp_, 0xff );
		GFL_BMPWIN_MakeScreen( commMenu->ynMenuBmpWin_ );
		
		GFL_BMPWIN_TransVramCharacter( commMenu->ynMenuBmpWin_ );
		GFL_BG_LoadScreenReq( bgPlane );
	}
	commMenu->ynMenuPrintQue_ = PRINTSYS_QUE_Create( commMenu->heapID_ );
	PRINT_UTIL_Setup( commMenu->ynMenuPrintUtil_, commMenu->ynMenuBmpWin_ );

	{	//window frame
		BmpWinFrame_GraphicSet( bgPlane , 1 , F_COMM_MENU_PALETTE ,
				MENU_TYPE_SYSTEM , commMenu->heapID_ );
		BmpWinFrame_Write( commMenu->ynMenuBmpWin_,
			WINDOW_TRANS_ON, 1, F_COMM_MENU_PALETTE );
	}
		
	{	//menu create
		u32 i;
		BMPMENU_HEADER head;
		const u16 lmax = D_YESNO_MENULIST_MAX;

		head.x_max	= 1;
		head.y_max	= lmax;
		head.line_spc	= 4;
		head.c_disp_f	= 0;
		head.loop_f		= 1;
		head.font_size_x = 12;
		head.font_size_y = 12;
		head.msgdata		= commMenu->msgData_;
		head.font_handle	= commMenu->fontHandle_;
		head.print_util		= commMenu->ynMenuPrintUtil_;
		head.print_que		= commMenu->ynMenuPrintQue_;
		head.win			= commMenu->ynMenuBmpWin_;

		commMenu->ynMenuList_ =
			BmpMenuWork_ListCreate( lmax, commMenu->heapID_ );
			
		BmpMenuWork_ListAddArchiveString(
			commMenu->ynMenuList_, commMenu->msgData_,
			DEBUG_FIELD_C_CHOICE02, NULL, commMenu->heapID_ );
		BmpMenuWork_ListAddArchiveString(
			commMenu->ynMenuList_, commMenu->msgData_,
			DEBUG_FIELD_C_CHOICE03, NULL, commMenu->heapID_ );
			
		head.menu = commMenu->ynMenuList_;
		
		commMenu->ynMenuWork_ = BmpMenu_Add( &head, 0, commMenu->heapID_ );
		BmpMenu_SetCursorString( commMenu->ynMenuWork_ , DEBUG_FIELD_STR00 );
	}
}

//--------------------------------------------------------------
//	はい・いいえ汎用閉じる
//--------------------------------------------------------------
void	FIELD_COMM_MENU_CloseYesNoMenu( FIELD_COMM_MENU *commMenu )
{
	PRINTSYS_QUE_Delete( commMenu->ynMenuPrintQue_ );
	//クリア周り
	//終了コマンドの受付後に呼んでたけど、ここに移動
	GFL_BMP_Clear( commMenu->ynMenuBmp_, 0x00 );
	GFL_BMPWIN_MakeScreen( commMenu->ynMenuBmpWin_ );
	GFL_BMPWIN_TransVramCharacter( commMenu->ynMenuBmpWin_ );
	BmpWinFrame_Clear( commMenu->ynMenuBmpWin_, 0 );
	
	//開放周り
	BmpMenu_Exit( commMenu->ynMenuWork_ , NULL );
	BmpMenuWork_ListDelete( commMenu->ynMenuList_ );
		
	GFL_BMPWIN_Delete( commMenu->ynMenuBmpWin_ );
}

//--------------------------------------------------------------
//	はい・いいえ汎用　更新
//--------------------------------------------------------------
const u8	FIELD_COMM_MENU_UpdateYesNoMenu( FIELD_COMM_MENU *commMenu )
{
	const u32 ret = BmpMenu_Main( commMenu->ynMenuWork_ );

	PRINTSYS_QUE_Main( commMenu->ynMenuPrintQue_ );
	if( PRINT_UTIL_Trans(commMenu->ynMenuPrintUtil_ ,commMenu->ynMenuPrintQue_ ) ){
	}
	
	switch( ret )
	{
	case BMPMENU_NULL:
		break;
	case BMPMENU_CANCEL:
		return YNR_NO;
		break;
	default:
		switch( BmpMenu_CursorPosGet(commMenu->ynMenuWork_) )
		{
		case 0:
			return YNR_YES;
			break;
		case 1:
			return YNR_NO;
			break;
		default:
			GF_ASSERT("Invalid return value!\n");
			break;
		}
	}

	return YNR_WAIT;
}

//--------------------------------------------------------------
//	メッセージウィンドウ　開く
//		初期メッセージの設定は各自で。
//--------------------------------------------------------------
void	FIELD_COMM_MENU_OpenMessageWindow( u8 bgPlane , FIELD_COMM_MENU *commMenu )
{
#if DEB_ARI
	if( debCommMenu != NULL )
	{
		FIELD_COMM_MENU_SwitchDebugWindow( bgPlane );
	}
#endif //DEB_ARI
	{	//bmpwin
		commMenu->msgWinBmpWin_ = GFL_BMPWIN_Create( bgPlane,
			F_COMM_MSGWIN_LEFT, F_COMM_MSGWIN_TOP, F_COMM_MSGWIN_WIDTH, F_COMM_MSGWIN_HEIGHT,
			F_COMM_FONT_PALETTE, F_COMM_BG_AREA );
		commMenu->msgWinBmp_ = GFL_BMPWIN_GetBmp( commMenu->msgWinBmpWin_ );
		
		GFL_BMP_Clear( commMenu->msgWinBmp_, 0xff );
		GFL_BMPWIN_MakeScreen( commMenu->msgWinBmpWin_ );
		
		GFL_BMPWIN_TransVramCharacter( commMenu->msgWinBmpWin_ );
		GFL_BG_LoadScreenReq( bgPlane );
	}
	commMenu->msgWinPrintQue_ = PRINTSYS_QUE_Create( commMenu->heapID_ );
	PRINT_UTIL_Setup( commMenu->msgWinPrintUtil_, commMenu->msgWinBmpWin_ );

	{	//window frame
		BmpWinFrame_GraphicSet( bgPlane , 1 , F_COMM_MENU_PALETTE ,
				MENU_TYPE_SYSTEM , commMenu->heapID_ );
		BmpWinFrame_Write( commMenu->msgWinBmpWin_,
			WINDOW_TRANS_ON, 1, F_COMM_MENU_PALETTE );
	}

	commMenu->isInitMsgWin_ = TRUE;
}

//--------------------------------------------------------------
//	メッセージウィンドウ　閉じる
//--------------------------------------------------------------
void	FIELD_COMM_MENU_CloseMessageWindow( FIELD_COMM_MENU *commMenu )
{
	PRINTSYS_QUE_Delete( commMenu->msgWinPrintQue_ );
	//クリア周り
	//終了コマンドの受付後に呼んでたけど、ここに移動
	GFL_BMP_Clear( commMenu->msgWinBmp_, 0x00 );
	GFL_BMPWIN_MakeScreen( commMenu->msgWinBmpWin_ );
	GFL_BMPWIN_TransVramCharacter( commMenu->msgWinBmpWin_ );
	BmpWinFrame_Clear( commMenu->msgWinBmpWin_, 0 );
	
	//開放周り
	GFL_BMPWIN_Delete( commMenu->msgWinBmpWin_ );

	commMenu->isInitMsgWin_ = FALSE;
}

//--------------------------------------------------------------
//	メッセージウィンドウ　メッセージ設定
//--------------------------------------------------------------
void	FIELD_COMM_MENU_SetMessage( u16 msgID , FIELD_COMM_MENU *commMenu )
{
	//文字列をメッセージデータから引き出すためのバッファ
	STRBUF *strTemp;

	//前回文字列の消去
	GFL_BMP_Fill( commMenu->msgWinBmp_,
				0, 0,
				F_COMM_MSGWIN_WIDTH*8, F_COMM_MSGWIN_HEIGHT*8, 0xff );
				
	strTemp = GFL_STR_CreateBuffer( 128, commMenu->heapID_ );
	GFL_MSG_GetString( commMenu->msgData_ , msgID , strTemp );

	PRINT_UTIL_Print(
		commMenu->msgWinPrintUtil_, commMenu->msgWinPrintQue_,
		0, 0, (void*)strTemp, commMenu->fontHandle_ );
	GFL_STR_DeleteBuffer( strTemp );
}

//--------------------------------------------------------------
//	メッセージウィンドウ　更新	
//--------------------------------------------------------------
void	FIELD_COMM_MENU_UpdateMessageWindow( FIELD_COMM_MENU *commMenu )
{
	if( commMenu->isInitMsgWin_ == TRUE )
	{
		PRINTSYS_QUE_Main( commMenu->msgWinPrintQue_ );
		if( PRINT_UTIL_Trans(commMenu->msgWinPrintUtil_ ,commMenu->msgWinPrintQue_ ) ){
		}
	}
}

//--------------------------------------------------------------
//	デバッグ用メッセージウィンドウ	
//--------------------------------------------------------------
#if DEB_ARI
#include "field_comm_main.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "gamesystem/playerwork.h"
#define F_COMM_DEBWIN_TOP		(F_COMM_MSGWIN_TOP)
#define F_COMM_DEBWIN_LEFT		(F_COMM_MSGWIN_LEFT)
#define F_COMM_DEBWIN_WIDTH		(F_COMM_MSGWIN_WIDTH)	
#define F_COMM_DEBWIN_HEIGHT	(F_COMM_MSGWIN_HEIGHT)	
void	FIELD_COMM_MENU_SwitchDebugWindow( u8 bgPlane )
{
	FIELD_COMM_MENU *commMenu = debCommMenu;
	if( debCommMenu == NULL )
	{
		//もう１面追加します Ari1113
		GFL_BG_BGCNT_HEADER msgBgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		const u8 msgBgFrame = bgPlane;
		GFL_BMPWIN_Init( GFL_HEAPID_APP );
		GFL_FONTSYS_Init();
		
		GFL_BG_SetBGControl(
			msgBgFrame, &msgBgcntText, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( msgBgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( msgBgFrame, 1 );

		GFL_BG_FillCharacter( msgBgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( msgBgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_LoadScreenReq( msgBgFrame );
		commMenu = FIELD_COMM_MENU_InitCommMenu( GFL_HEAPID_APP );
		commMenu->isOpenDebugWin_ = TRUE;
		FIELD_COMM_MENU_OpenMessageWindow( msgBgFrame , commMenu );
#if 0
		//bmpwinConvertStringSjisToUnicode
		commMenu->msgWinBmpWin_ = GFL_BMPWIN_Create( bgPlane,
			F_COMM_DEBWIN_LEFT, F_COMM_DEBWIN_TOP, F_COMM_DEBWIN_WIDTH, F_COMM_DEBWIN_HEIGHT,
			F_COMM_FONT_PALETTE, F_COMM_BG_AREA );
		commMenu->msgWinBmp_ = GFL_BMPWIN_GetBmp( commMenu->msgWinBmpWin_ );
		
		GFL_BMP_Clear( commMenu->msgWinBmp_, 0xff );
		GFL_BMPWIN_MakeScreen( commMenu->msgWinBmpWin_ );
		
		GFL_BMPWIN_TransVramCharacter( commMenu->msgWinBmpWin_ );
		GFL_BG_LoadScreenReq( bgPlane );
	
		commMenu->msgWinPrintQue_ = PRINTSYS_QUE_Create( commMenu->heapID_ );
		PRINT_UTIL_Setup( commMenu->msgWinPrintUtil_, commMenu->msgWinBmpWin_ );

		//window frame
		BmpWinFrame_GraphicSet( bgPlane , 1 , F_COMM_MENU_PALETTE ,
				MENU_TYPE_SYSTEM , commMenu->heapID_ );
		BmpWinFrame_Write( commMenu->msgWinBmpWin_,
			WINDOW_TRANS_ON, 1, F_COMM_MENU_PALETTE );
#endif
	}
	else if( PRINTSYS_QUE_IsFinished( commMenu->msgWinPrintQue_ ) )
	{
		commMenu->isOpenDebugWin_ = FALSE;
		FIELD_COMM_MENU_CloseMessageWindow( commMenu );
#if 0
		PRINTSYS_QUE_Delete( commMenu->msgWinPrintQue_ );
		//クリア周り
		//終了コマンドの受付後に呼んでたけど、ここに移動
		GFL_BMP_Clear( commMenu->msgWinBmp_, 0x00 );
		GFL_BMPWIN_MakeScreen( commMenu->msgWinBmpWin_ );
		GFL_BMPWIN_TransVramCharacter( commMenu->msgWinBmpWin_ );
		BmpWinFrame_Clear( commMenu->msgWinBmpWin_, 0 );
	
		//開放周り
		GFL_BMPWIN_Delete( commMenu->msgWinBmpWin_ );
#endif
		FIELD_COMM_MENU_TermCommMenu( commMenu );
		
		GFL_BMPWIN_Exit();
		GFL_BG_FreeCharacterArea( bgPlane, 0x00, 0x20 );
		GFL_BG_FreeBGControl( bgPlane );

		commMenu = NULL;
	}
	debCommMenu = commMenu;
}
void	FIELD_COMM_MENU_UpdateDebugWindow( )
{
	FIELD_COMM_MENU *commMenu = debCommMenu;
	if( commMenu == NULL )
		return;
	if( commMenu->isOpenDebugWin_ == TRUE )
	{
		//文字列バッファ
		STRBUF *strTemp;
		u16 codeTemp[128];
		char	strBase[128];
		int	codeTempLen;
		u8 i;
		//if( GFL_UI_KEY_GetTrg() ==PAD_BUTTON_Y )
		if( PRINTSYS_QUE_IsFinished( commMenu->msgWinPrintQue_ ) )
		{
		
			//前回文字列の消去
			GFL_BMP_Fill( commMenu->msgWinBmp_,
						0, 0,
						F_COMM_DEBWIN_WIDTH*8, F_COMM_DEBWIN_HEIGHT*8, 0xff );
			
			for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
			{	
				PLAYER_WORK *plWork = NULL;
				strTemp = GFL_STR_CreateBuffer( 128, commMenu->heapID_ );
				if( i == FIELD_COMM_FUNC_GetSelfIndex(NULL) )
					plWork = FIELD_COMM_DATA_GetSelfData_PlayerWork();
				if( FIELD_COMM_DATA_GetCharaData_IsExist(i) == TRUE )
					plWork = FIELD_COMM_DATA_GetCharaData_PlayerWork(i);
				if( plWork != NULL )
				{
					codeTempLen = 128;
					sprintf(strBase,"P%d:%3d:%3d:%d  \0",i+1,(int)F32_CONST(plWork->position.x),(int)F32_CONST(plWork->position.z),(int)F32_CONST(plWork->position.y));
					//sprintf(strBase,"P%d:%3d:%3d  \0",i+1,plWork->position.x,plWork->position.z);
					STD_ConvertStringSjisToUnicode( codeTemp ,&codeTempLen, strBase , NULL , NULL);
					GFL_STR_SetStringCodeOrderLength( strTemp , codeTemp , codeTempLen);
			
					PRINT_UTIL_Print(
						commMenu->msgWinPrintUtil_, commMenu->msgWinPrintQue_,
						(i%2)*64, (i/2)*16, (void*)strTemp, commMenu->fontHandle_ );
					
				}
				GFL_STR_DeleteBuffer( strTemp );
			}
		}	
		PRINTSYS_QUE_Main( commMenu->msgWinPrintQue_ );
		PRINT_UTIL_Trans(commMenu->msgWinPrintUtil_ ,commMenu->msgWinPrintQue_ );
		
	}
	debCommMenu = commMenu;
}
#endif	//DEB_ARI

