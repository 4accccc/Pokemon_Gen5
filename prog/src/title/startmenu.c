//======================================================================
/**
 * @file	startmenu.c
 * @brief	最初から・続きからを行うトップメニュー
 * @author	ariizumi
 * @data	09/01/07
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "font/font.naix"

#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "message.naix"
#include "startmenu.naix"
#include "msg/msg_startmenu.h"

#include "net/dwc_raputil.h"
#include "savedata/save_control.h"
#include "savedata/mystatus.h"
#include "app/name_input.h"
#include "title/startmenu.h"
#include "title/title.h"
#include "title/game_start.h"
#include "test/ariizumi/ari_debug.h"
//======================================================================
//	define
//======================================================================
#define BG_PLANE_MENU GFL_BG_FRAME1_M
#define BG_PLANE_BACK_GROUND GFL_BG_FRAME3_M

//BGキャラアドレス
#define START_MENU_FRAMECHR1	1
#define START_MENU_FRAMECHR2	(START_MENU_FRAMECHR1 + TALK_WIN_CGX_SIZ)

//パレット
#define START_MENU_PLT_FONT		0
#define START_MENU_PLT_SEL		3
#define START_MENU_PLT_NOSEL	4

#define START_MENU_FONT_TOP  2
#define START_MENU_FONT_LEFT 2

//======================================================================
//	enum
//======================================================================
typedef enum 
{
	SMS_FADE_IN,
	SMS_SELECT,
	SMS_FADE_OUT,

	SMS_MAX,
}START_MENU_STATE;

//スタートメニューの項目
enum START_MENU_ITEM
{
	SMI_CONTINUE,		//続きから
	SMI_NEWGAME,		//最初から
	SMI_MYSTERY_GIFT,	//不思議な贈り物
	SMI_WIFI_SETTING,	//Wifi設定
	SMI_EMAIL_SETTING,	//E-Mail設定
	
	SMI_MAX,
	
	SMI_RETURN_TITLE = 0xFF,	//Bで戻る
};

//======================================================================
//	typedef struct
//======================================================================

//項目情報
typedef struct
{
	BOOL enable_;
	u8	top_;
	GFL_BMPWIN *win_;
}START_MENU_ITEM_WORK;

//ワーク
typedef struct
{
	HEAPID heapId_;
	START_MENU_STATE state_;
		
	u8		selectItem_;	//選択中項目
	u8		selectButtom_;	//選択中位置(下端
	int		dispPos_;		//表示中位置(dot単位
	int		targetPos_;		//表示目標位置(dot単位
	u8		length_;		//メニューの長さ(キャラ単位
	
	MYSTATUS	*mystatus_;
	
	GFL_FONT *fontHandle_;
	GFL_MSGDATA *msgMng_;	//メニュー作成のところでだけ有効
	GFL_TCB		*vblankFuncTcb_;

	GFL_CLUNIT	*cellUnit_;
	GFL_CLWK	*cellCursor_[2];
	NNSG2dImagePaletteProxy	cursorPltProxy_;
	NNSG2dCellDataBank	*cursorCellData_;
	NNSG2dAnimBankData	*cursorAnmData_;
	void	*cursorCellRes_;
	void	*cursorAnmRes_;

	START_MENU_ITEM_WORK itemWork_[SMI_MAX];
}START_MENU_WORK;

//項目設定情報
//項目を開くか？
typedef BOOL (*START_MENU_ITEM_CheckFunc)(START_MENU_WORK *work);
//決定時動作
typedef BOOL (*START_MENU_ITEM_SelectFunc)(START_MENU_WORK *work);
//項目内描画
typedef void (*START_MENU_ITEM_DrawFunc)(START_MENU_WORK *work,GFL_BMPWIN *win, const u8 idx );
typedef struct
{
	u8	height_;	//高さ(キャラ単位
	//各種動作
	START_MENU_ITEM_CheckFunc	checkFunc_;
	START_MENU_ITEM_SelectFunc	selectFunc_;
	START_MENU_ITEM_DrawFunc	drawFunc_;
}START_MENU_ITEM_SETTING;


//======================================================================
//	proto
//======================================================================

static void START_MENU_VBlankFunc(GFL_TCB *tcb,void *work);

static void	START_MENU_InitGraphic( START_MENU_WORK *work );
static void	START_MENU_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );

static void	START_MENU_CreateMenuItem( START_MENU_WORK *work );

static BOOL START_MENU_MoveSelectItem( START_MENU_WORK *work , BOOL isDown );
static void START_MENU_ChangeActiveItem( START_MENU_WORK *work , const u8 newItem , const u8 oldItem );

//メニュー項目用機能関数
static BOOL START_MENU_ITEM_TempCheck( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_TempCheckFalse( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_CheckContinue( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_CommonSelect( START_MENU_WORK *work );
static void START_MENU_ITEM_CommonDraw( START_MENU_WORK *work , GFL_BMPWIN *win, const u8 idx );
static void START_MENU_ITEM_ContinueDraw( START_MENU_WORK *work , GFL_BMPWIN *win, const u8 idx );

static void START_MENU_ITEM_MsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId );
static void START_MENU_ITEM_WordMsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId , WORDSET *word);

//Procデータ
static GFL_PROC_RESULT START_MENU_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT START_MENU_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT START_MENU_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

const GFL_PROC_DATA StartMenuProcData = {
	START_MENU_ProcInit,
	START_MENU_ProcMain,
	START_MENU_ProcEnd,
};

//メニュー項目設定データ配列
static const START_MENU_ITEM_SETTING ItemSettingData[SMI_MAX] =
{
	{ 10 , START_MENU_ITEM_CheckContinue , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_ContinueDraw },
	{  2 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
	{  2 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
	{  2 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
	{  2 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
};



//--------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	u8 i;
	
	START_MENU_WORK *work;
		//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_STARTMENU, 0x80000 );
	work = GFL_PROC_AllocWork( proc, sizeof(START_MENU_WORK), HEAPID_STARTMENU );
	GFL_STD_MemClear(work, sizeof(START_MENU_WORK));

	work->heapId_ = HEAPID_STARTMENU;
	for( i=0;i<SMI_MAX;i++ )
	{
		work->itemWork_[i].enable_ = FALSE;
	}
	work->mystatus_ = SaveData_GetMyStatus( SaveControl_GetPointer() );
	work->state_ = SMS_FADE_IN;
	START_MENU_InitGraphic( work );

	//フォント用パレット
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , START_MENU_PLT_FONT * 32, 16*2, work->heapId_ );
	work->fontHandle_ = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId_ );

	//WinFrame用グラフィック設定
	BmpWinFrame_GraphicSet( BG_PLANE_MENU , START_MENU_FRAMECHR1 , START_MENU_PLT_SEL 	, 0, work->heapId_);
	BmpWinFrame_GraphicSet( BG_PLANE_MENU , START_MENU_FRAMECHR2 , START_MENU_PLT_NOSEL , 1, work->heapId_);
	//背景色
	*((u16 *)HW_BG_PLTT) = 0x7d8c;//RGB(12, 12, 31);
	
	START_MENU_CreateMenuItem( work );
	
	work->vblankFuncTcb_ = GFUser_VIntr_CreateTCB( START_MENU_VBlankFunc , (void*)work , 0 );

	GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 16 , 0 , ARI_FADE_SPD );
	
	return GFL_PROC_RES_FINISH;
}

FS_EXTERN_OVERLAY(mystery);
extern const GFL_PROC_DATA MysteryGiftProcData;
//--------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	START_MENU_WORK *work = mywk;

	//選択による分岐処理
	{
		switch( work->selectItem_ )
		{
		case SMI_CONTINUE:		//続きから
			//Procの変更を中でやってる
			GameStart_Continue();
			break;
		case SMI_NEWGAME:		//最初から
			//Procの変更を中でやってる
			GameStart_Beginning();
			//名前入力へ
			//GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &NameInputProcData,(void*)NAMEIN_MYNAME);
			break;
			
		case SMI_MYSTERY_GIFT:	//不思議な贈り物
			GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(mystery), &MysteryGiftProcData, NULL);
			break;

        case SMI_WIFI_SETTING: //WIFI設定
			GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(wifi_util), &WifiUtilProcData, NULL);
            break;
            
		case SMI_RETURN_TITLE:
			GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
			break;
		default:
			//Procの変更を中でやってる
			GameStart_Beginning();
			break;
		}
	}

	//開放処理
	{
		u8 i;
		GFL_TCB_DeleteTask( work->vblankFuncTcb_ );

		GFL_CLACT_WK_Remove( work->cellCursor_[0] );
		GFL_CLACT_WK_Remove( work->cellCursor_[1] );
		GFL_HEAP_FreeMemory( work->cursorCellRes_ );
		GFL_HEAP_FreeMemory( work->cursorAnmRes_ );
		GFL_CLACT_UNIT_Delete( work->cellUnit_ );
		GFL_CLACT_SYS_Delete();

		for( i=0;i<SMI_MAX;i++ )
		{
			if( work->itemWork_[i].enable_ == TRUE )
			{
				GFL_BMPWIN_Delete( work->itemWork_[i].win_ );
			}
		}

		GFL_FONT_Delete( work->fontHandle_ );
		GFL_BMPWIN_Exit();

		GFL_BG_FreeBGControl(BG_PLANE_MENU);
		GFL_BG_FreeBGControl(BG_PLANE_BACK_GROUND);
		GFL_BG_Exit();
		
		GFL_PROC_FreeWork( proc );

		GFL_HEAP_DeleteHeap( HEAPID_STARTMENU );
	}

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	START_MENU_WORK *work = mywk;
	const int keyTrg = GFL_UI_KEY_GetTrg();
	
	switch( work->state_ )
	{
	case SMS_FADE_IN:
		if( GFL_FADE_CheckFade() == FALSE )
		{
			work->state_ = SMS_SELECT;
		}
		break;
		
	case SMS_SELECT:
		if( keyTrg & PAD_KEY_UP )
		{
			START_MENU_MoveSelectItem( work , FALSE );
		}
		else
		if( keyTrg & PAD_KEY_DOWN )
		{
			START_MENU_MoveSelectItem( work , TRUE );
		}
		else
		if( keyTrg & PAD_BUTTON_A )
		{
			const BOOL ret = ItemSettingData[work->selectItem_].selectFunc_( work );
			if( ret == TRUE )
			{
				GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 0 , 16 , ARI_FADE_SPD );
				work->state_ = SMS_FADE_OUT;
			}
		}
		else
		if( keyTrg & PAD_BUTTON_B )
		{
			GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 0 , 16 , ARI_FADE_SPD );
			work->selectItem_ = SMI_RETURN_TITLE;
			work->state_ = SMS_FADE_OUT;
		}
		break;
		
	case SMS_FADE_OUT:
		if( GFL_FADE_CheckFade() == FALSE )
		{
			return GFL_PROC_RES_FINISH;
		}
		break;
	}

	
	//スクロール処理(現状特にオーバー処理など入れてないので8の約数で
	if( work->targetPos_ > work->dispPos_ )
	{
		work->dispPos_ += 2;
		GFL_BG_SetScroll( BG_PLANE_MENU , GFL_BG_SCROLL_Y_SET , work->dispPos_ );
	}
	else	//メニューの頭の位置なので計算が長い・・・
	if( work->targetPos_ < work->dispPos_ )
	{
		work->dispPos_ -= 2;
		GFL_BG_SetScroll( BG_PLANE_MENU , GFL_BG_SCROLL_Y_SET , work->dispPos_ );
	}

	//OBJの更新
	GFL_CLACT_SYS_Main();

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	VBLank Function
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
static void START_MENU_VBlankFunc(GFL_TCB *tcb,void *work)
{
	GFL_CLACT_SYS_VBlankFunc();
}

//--------------------------------------------------------------------------
//	グラフィック周り初期化
//--------------------------------------------------------------------------
static void	START_MENU_InitGraphic( START_MENU_WORK *work )
{
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_32K,		// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};

	static const GFL_BG_SYS_HEADER sysHeader = {
		GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
	};
	
	static const GFL_BG_BGCNT_HEADER bgCont_Menu = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};

	static const GFL_BG_BGCNT_HEADER bgCont_BackGround = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};
	
	GX_SetMasterBrightness(-16);	
	GXS_SetMasterBrightness(-16);
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);

	GFL_DISP_SetBank( &vramBank );
	GFL_BG_Init( work->heapId_ );
	GFL_BG_SetBGMode( &sysHeader );	

	START_MENU_InitBgFunc( &bgCont_Menu , BG_PLANE_MENU );
	START_MENU_InitBgFunc( &bgCont_BackGround , BG_PLANE_BACK_GROUND );
	
	GFL_BMPWIN_Init( work->heapId_ );
	
	//OBJ系
	{
		NNSG2dImageProxy	imgProxy;
		GFL_CLWK_RES	cellRes;
		GFL_CLWK_DATA	cellInitData;

		GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
		cellSysInitData.oamst_main = 0x10;	//デバッグメータの分
		cellSysInitData.oamnum_main = 128-0x10;
		GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId_ );
		work->cellUnit_  = GFL_CLACT_UNIT_Create( 2 , 0, work->heapId_ );
		GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit_ );

		NNS_G2dInitImagePaletteProxy( &work->cursorPltProxy_ );
		NNS_G2dInitImageProxy( &imgProxy );
		//各種素材の読み込み
		GFL_ARC_UTIL_TransVramPaletteMakeProxy( ARCID_STARTMENU , NARC_startmenu_title_cursor_NCLR , 
				NNS_G2D_VRAM_TYPE_2DMAIN , 0 , work->heapId_ , &work->cursorPltProxy_ );
		
		work->cursorCellRes_ = GFL_ARC_UTIL_LoadCellBank( ARCID_STARTMENU , NARC_startmenu_title_cursor_NCER , 
					FALSE , &work->cursorCellData_ , work->heapId_ );
	
		work->cursorAnmRes_ = GFL_ARC_UTIL_LoadAnimeBank( ARCID_STARTMENU , NARC_startmenu_title_cursor_NANR ,
					FALSE , &work->cursorAnmData_ , work->heapId_ );
		
		GFL_ARC_UTIL_TransVramCharacterMakeProxy( ARCID_STARTMENU , NARC_startmenu_title_cursor_NCGR , 
					FALSE , 0 , 0 , NNS_G2D_VRAM_TYPE_2DMAIN , 0 , work->heapId_ , &imgProxy );

		//セルの生成
		GFL_CLACT_WK_SetCellResData( &cellRes , &imgProxy , &work->cursorPltProxy_ ,
					work->cursorCellData_ , work->cursorAnmData_ );

		cellInitData.pos_x = 128;
		cellInitData.pos_y =  8;
		cellInitData.anmseq = 0;
		cellInitData.softpri = 0;
		cellInitData.bgpri = 0;
		//↑矢印
		work->cellCursor_[0] = GFL_CLACT_WK_Add( work->cellUnit_ , &cellInitData ,
					&cellRes , CLSYS_DEFREND_MAIN , work->heapId_ );
		GFL_CLACT_WK_SetAutoAnmSpeed( work->cellCursor_[0], FX32_ONE );
		GFL_CLACT_WK_SetAutoAnmFlag( work->cellCursor_[0], TRUE );
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[0], FALSE );
		
		//↓矢印
		cellInitData.pos_y =  192-8;
		cellInitData.anmseq = 1;
		work->cellCursor_[1] = GFL_CLACT_WK_Add( work->cellUnit_ , &cellInitData ,
					&cellRes , CLSYS_DEFREND_MAIN , work->heapId_ );
		GFL_CLACT_WK_SetAutoAnmSpeed( work->cellCursor_[1], FX32_ONE );
		GFL_CLACT_WK_SetAutoAnmFlag( work->cellCursor_[1], TRUE );
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[1], FALSE );
				
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
	}

}

//--------------------------------------------------------------------------
//	Bg初期化 機能部
//--------------------------------------------------------------------------
static void	START_MENU_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
	GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
	GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
	GFL_BG_ClearFrame( bgPlane );
	GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//	メニューの作成
//--------------------------------------------------------------------------
static void	START_MENU_CreateMenuItem( START_MENU_WORK *work )
{
	u8	i;
	u8	bgTopPos = 1;	//アイテムの表示位置(キャラ単位

	work->msgMng_ = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_startmenu_dat, work->heapId_);
	work->selectItem_ = 0xFF;	//最初にアクティブなのは一番上にするための判定用
	for( i=0;i<SMI_MAX;i++ )
	{
		if( ItemSettingData[i].checkFunc_(work) == TRUE )
		{
			work->itemWork_[i].enable_ = TRUE;
			work->itemWork_[i].win_ = GFL_BMPWIN_Create( 
						BG_PLANE_MENU , 
						3 , bgTopPos ,
						26 , ItemSettingData[i].height_ ,
						START_MENU_PLT_FONT , 
						GFL_BMP_CHRAREA_GET_B );
			GFL_BMPWIN_MakeScreen( work->itemWork_[i].win_ );
			GFL_BMPWIN_TransVramCharacter( work->itemWork_[i].win_ );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->itemWork_[i].win_), 0xf );

			ItemSettingData[i].drawFunc_(work,work->itemWork_[i].win_,i);
			
			GFL_BMPWIN_TransVramCharacter( work->itemWork_[i].win_ );
			BmpWinFrame_Write(work->itemWork_[i].win_, WINDOW_TRANS_ON, 
							START_MENU_FRAMECHR1, START_MENU_PLT_NOSEL);
			
			bgTopPos += ItemSettingData[i].height_ + 2;
			if( work->selectItem_ == 0xFF )
			{
				//一番上の項目
				work->selectItem_ = i;
			}
		}
	}
	GFL_MSG_Delete( work->msgMng_ );
	work->msgMng_ = NULL;
	
	GF_ASSERT( work->selectItem_ != 0xFF );
	work->selectButtom_ = ItemSettingData[work->selectItem_].height_ + 2;
	work->length_ = bgTopPos-1;	//次のメニューの枠の分も足しているので1引く
	START_MENU_ChangeActiveItem( work , work->selectItem_ , 0xFF );
	
	//最初の下カーソルの表示チェック
	if( work->length_ > 24 )
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[1], TRUE );

}

//--------------------------------------------------------------------------
//	項目移動
//	@return 移動できたか？
//	@value  BOOL isDown　FALSE:↑移動 TRUE:↓移動
//--------------------------------------------------------------------------
static BOOL START_MENU_MoveSelectItem( START_MENU_WORK *work , BOOL isDown )
{
	const int moveValue = (isDown==TRUE ? 1 : -1 );
	const int endValue = (isDown==TRUE ? SMI_MAX : -1 );
	int	i;
	BOOL isShow;
	//開始位置は１つ分動かしておく
	for( i = work->selectItem_ + moveValue; i != endValue ; i += moveValue )
	{
		if( work->itemWork_[i].enable_ == TRUE )
		{
			break;
		}
	}
	if( i == endValue )
	{
		//動けなかった
		return FALSE;
	}
	
	START_MENU_ChangeActiveItem( work , i , work->selectItem_ );
	if( isDown == TRUE )
	{
		work->selectButtom_ += ItemSettingData[i].height_ + 2;	//枠で+2
	}
	else
	{
		work->selectButtom_ -= ItemSettingData[work->selectItem_].height_ + 2;	//枠で+2
	}
	
	work->selectItem_ = i;
	
	//表示位置計算
	if( work->selectButtom_*8 > work->targetPos_ + 192 )
	{
		//下のチェック
		work->targetPos_ = work->selectButtom_*8 -192;
	}
	else	//メニューの頭の位置なので計算が長い・・・
	if( ( work->selectButtom_ - ItemSettingData[work->selectItem_].height_ - 2 )*8 < work->targetPos_ )
	{
		//上のチェック
		work->targetPos_ = ( work->selectButtom_ - ItemSettingData[work->selectItem_].height_ - 2 )*8;
	}

	//カーソル表示のチェック
	//上
	if( work->targetPos_ > 0 )
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[0], TRUE );
	else
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[0], FALSE );

	//下
	if( work->targetPos_ + 192 < work->length_*8 )
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[1], TRUE );
	else
		GFL_CLACT_WK_SetDrawEnable( work->cellCursor_[1], FALSE );
	
	return TRUE;
}

//--------------------------------------------------------------------------
//	選択メニューの切り替え
//--------------------------------------------------------------------------
static void START_MENU_ChangeActiveItem( START_MENU_WORK *work , const u8 newItem , const u8 oldItem )
{
	if( newItem != 0xFF )
	{
		BmpWinFrame_Write(work->itemWork_[newItem].win_, WINDOW_TRANS_ON, 
						START_MENU_FRAMECHR2, START_MENU_PLT_SEL);
	}
	if( oldItem != 0xFF )
	{
		BmpWinFrame_Write(work->itemWork_[oldItem].win_, WINDOW_TRANS_ON, 
						START_MENU_FRAMECHR1, START_MENU_PLT_NOSEL);
	}
}

#pragma mark MenuSelectFunc

//--------------------------------------------------------------------------
//	メニューを有効にするか？
//	@return TRUE=有効
//--------------------------------------------------------------------------
static BOOL START_MENU_ITEM_TempCheck( START_MENU_WORK *work )
{
	return TRUE;
}
static BOOL START_MENU_ITEM_TempCheckFalse( START_MENU_WORK *work )
{
	return FALSE;
}
static BOOL START_MENU_ITEM_CheckContinue( START_MENU_WORK *work )
{
	return SaveData_GetExistFlag( SaveControl_GetPointer() );
}

//--------------------------------------------------------------------------
//	決定時の動作
//	@return TRUE=スタートメニュの終了処理へ
//--------------------------------------------------------------------------
static BOOL START_MENU_ITEM_CommonSelect( START_MENU_WORK *work )
{
	//今のところ選択された
	return TRUE;
}
//メニュー内描画共通(idxでmsgIdを読み分ける
static void START_MENU_ITEM_CommonDraw( START_MENU_WORK *work , GFL_BMPWIN *win , const u8 idx )
{
	static const msgIdArr[SMI_MAX] =
	{
		START_MENU_STR_ITEM_01_01,	//実際はこない(ダミー
		START_MENU_STR_ITEM_02,
		START_MENU_STR_ITEM_03,
		START_MENU_STR_ITEM_04,
		START_MENU_STR_ITEM_05,
	};
	
	START_MENU_ITEM_MsgDrawFunc( work , win , 0,0, msgIdArr[idx] );
}

//続きからメニュー内描画
static void START_MENU_ITEM_ContinueDraw( START_MENU_WORK *work , GFL_BMPWIN *win , const u8 idx )
{
	u8 lCol,sCol,bCol;
	WORDSET *word;
	STRBUF *nameStr;
	const STRCODE *myname = MyStatus_GetMyName( work->mystatus_ );

	word = WORDSET_Create( work->heapId_ );
	//Font色を戻すため取っておく
	GFL_FONTSYS_GetColor( &lCol,&sCol,&bCol );

	START_MENU_ITEM_MsgDrawFunc( work , win ,  0, 0, START_MENU_STR_ITEM_01_01 );

	GFL_FONTSYS_SetColor( 5,sCol,bCol );
	START_MENU_ITEM_MsgDrawFunc( work , win , 32,16, START_MENU_STR_ITEM_01_02 );
	START_MENU_ITEM_MsgDrawFunc( work , win , 32,32, START_MENU_STR_ITEM_01_03 );
	START_MENU_ITEM_MsgDrawFunc( work , win , 32,48, START_MENU_STR_ITEM_01_04 );
	START_MENU_ITEM_MsgDrawFunc( work , win , 32,64, START_MENU_STR_ITEM_01_05 );

	//名前	
	nameStr = 	GFL_STR_CreateBuffer( 16, work->heapId_ );
	if( MyStatus_CheckNameClear( work->mystatus_ ) == FALSE )
	{
		GFL_STR_SetStringCode( nameStr , myname );
	}
	else
	{
		//TODO 念のため名前が入ってないときに落ちないようにしておく
		u16 tempName[7] = { L'N',L'o',L'N',L'a',L'm',L'e',0xFFFF };
		GFL_STR_SetStringCode( nameStr , tempName );
	}
	WORDSET_RegisterWord( word, 0, nameStr, 0, TRUE , 0 );
	START_MENU_ITEM_WordMsgDrawFunc( work , win ,192,16, START_MENU_STR_ITEM_01_06 , word);

	//プレイ時間
	WORDSET_RegisterNumber( word, 0, 9, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
	WORDSET_RegisterNumber( word, 1, 2, 2, STR_NUM_DISP_ZERO , STR_NUM_CODE_HANKAKU );
	START_MENU_ITEM_WordMsgDrawFunc( work , win ,192,32, START_MENU_STR_ITEM_01_07 , word);
	//ポケモン図鑑
	WORDSET_RegisterNumber( word, 0, 55, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
	START_MENU_ITEM_WordMsgDrawFunc( work , win ,192,48, START_MENU_STR_ITEM_01_08 , word);
	//バッジ
	WORDSET_RegisterNumber( word, 0, MyStatus_GetBadgeCount(work->mystatus_), 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
	START_MENU_ITEM_WordMsgDrawFunc( work , win ,192,64, START_MENU_STR_ITEM_01_09 , word);

	//Font色を戻す
	GFL_FONTSYS_SetColor( lCol,sCol,bCol );
	
	GFL_STR_DeleteBuffer( nameStr );
	WORDSET_Delete( word );
}

static void START_MENU_ITEM_MsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId )
{
	STRBUF  *str = GFL_STR_CreateBuffer( 96 , work->heapId_ );
	GFL_MSG_GetString( work->msgMng_ , msgId , str );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp(win),
					posX+START_MENU_FONT_LEFT,
					posY+START_MENU_FONT_TOP,
					str,
					work->fontHandle_);
	
	GFL_STR_DeleteBuffer( str );
}

static void START_MENU_ITEM_WordMsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId , WORDSET *word)
{
	u8 strLen;
	STRBUF  *str = GFL_STR_CreateBuffer( 96 , work->heapId_ );
	STRBUF  *baseStr = GFL_STR_CreateBuffer( 96 , work->heapId_ );
	GFL_MSG_GetString( work->msgMng_ , msgId , baseStr );
	WORDSET_ExpandStr( word , str , baseStr );

	strLen = PRINTSYS_GetStrWidth( str , work->fontHandle_ , 0 );
	
	PRINTSYS_Print( GFL_BMPWIN_GetBmp(win),
					posX+START_MENU_FONT_LEFT - strLen,
					posY+START_MENU_FONT_TOP,
					str,
					work->fontHandle_);
	
	GFL_STR_DeleteBuffer( baseStr );
	GFL_STR_DeleteBuffer( str );
	
}
