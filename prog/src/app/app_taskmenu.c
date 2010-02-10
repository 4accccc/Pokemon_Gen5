//======================================================================
/**
 * @file	bmp_taskmenu.c
 * @brief	タスクバー用 共通メニュー
 * @author	ariizumi toru_nagihashi
 * @data	09/07/27
 *
 * モジュール名：BMP_TASKMENU
 *
 *	taskmenuいう名前ですが、
 *	taskシステムは使っておりません
 *
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "app/app_menu_common.h"
#include "print/wordset.h"
#include "sound/pm_sndsys.h"

#include "arc_def.h"

//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE
//通常時処理
#include "app_menu_common.naix"
#else
//DL子機時処理
#include "app_menu_common_dl.naix"
#endif //MULTI_BOOT_MAKE

#include "app/app_taskmenu.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define APP_TASKMENU_PLATE_LEFT (19)

#define APP_TASKMENU_PLATE_NCG_NUM	(3*8)
#define APP_TASKMENU_PLATE_NCG_LEFTTOP	(0)
#define APP_TASKMENU_PLATE_NCG_TOP			(1)
#define APP_TASKMENU_PLATE_NCG_RIGHTTOP	(2)
#define APP_TASKMENU_PLATE_NCG_LEFT			(3)
#define APP_TASKMENU_PLATE_NCG_CENTER		(4)
#define APP_TASKMENU_PLATE_NCG_RIGHT		(5)
#define APP_TASKMENU_PLATE_NCG_LEFTDOWN	(6)
#define APP_TASKMENU_PLATE_NCG_DOWN			(7)
#define APP_TASKMENU_PLATE_NCG_RIGHTDOWN	(8)
#define APP_TASKMENU_PLATE_NCG_MARK_RET		(9)
#define APP_TASKMENU_PLATE_NCG_WIDTH		(3)

//プレートのアニメ
#define APP_TASKMENU_ANIME_S_R (5)
#define APP_TASKMENU_ANIME_S_G (10)
#define APP_TASKMENU_ANIME_S_B (13)
#define APP_TASKMENU_ANIME_E_R (12)
#define APP_TASKMENU_ANIME_E_G (25)
#define APP_TASKMENU_ANIME_E_B (30)
//プレートのアニメする色
#define APP_TASKMENU_ANIME_COL (0x6)

#define APP_TASKMENU_SND_CURSOR (SEQ_SE_SELECT1)
#define APP_TASKMENU_SND_DECIDE (SEQ_SE_DECIDE1)
#define APP_TASKMENU_SND_CANCEL (SEQ_SE_CANCEL1)
#define APP_TASKMENU_SND_OPENMENU (SEQ_SE_OPEN1)
#define APP_TASKMENU_SND_CLOSEMENU (SEQ_SE_CLOSE1)

#define APP_TASKMENU_SND_CHANGE (SEQ_SE_OPEN1)


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
//リソース用ワーク
struct _APP_TASKMENU_RES
{	
	//リソース関係
	u16	frame;
	u16	plt;
	NNSG2dCharacterData *ncg_data;
  void *ncg_buf; 

	//メッセージ関係
  GFL_MSGDATA *msgHandle;
  GFL_FONT    *fontHandle;
  PRINT_QUE   *printQue;
};

//縦メニュー用ワーク
struct _APP_TASKMENU_WORK
{
  u8 cursorPos;
  u8 anmCnt;
  u16 transAnmCnt;
  GXRgb transCol;
  BOOL isUpdateMsg;
  BOOL isDecide;

  GFL_BMPWIN **menuWin;
  APP_TASKMENU_ITEMWORK *itemWork;
  //メニュー土台
  
  APP_TASKMENU_INITWORK initWork;

	const APP_TASKMENU_RES *res;
};

//縦メニュー用ワーク
struct _APP_TASKMENU_WIN_WORK
{
  GFL_BMPWIN *bmpwin;
	BOOL isActive;
	BOOL isDecide;
	BOOL isUpdateMsg;

	u16	anmCnt;
	u16 dummy;
	u16 transAnmCnt;
  GXRgb transCol;
	const APP_TASKMENU_RES *res;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void APP_TASKMENU_CreateMenuWin( APP_TASKMENU_WORK *work, const APP_TASKMENU_RES *res );
static void APP_TASKMENU_TransFrame( GFL_BMPWIN *bmpwin, const APP_TASKMENU_RES *res, APP_TASKMENU_WIN_TYPE type );
static void APP_TASKMENU_SetActiveItem( GFL_BMPWIN *bmpwin, const APP_TASKMENU_RES *res, const BOOL isActive );
static void APP_TASKMENU_UpdatePalletAnime( u16 *anmCnt , u16 *transBuf , u8 bgFrame , u8 pltNo );
static void APP_TASKMENU_ResetPallet( u16 *transBuf, u8 bgFrame , u8 pltNo );
static void APP_TASKMENU_UpdateKey( APP_TASKMENU_WORK *work );
static void APP_TASKMENU_UpdateTP( APP_TASKMENU_WORK *work );

//--------------------------------------------------------------
//	メニュー開く
//--------------------------------------------------------------
APP_TASKMENU_WORK* APP_TASKMENU_OpenMenu( APP_TASKMENU_INITWORK *initWork, const APP_TASKMENU_RES *res )
{
  APP_TASKMENU_WORK *work = GFL_HEAP_AllocMemory( initWork->heapId , sizeof( APP_TASKMENU_WORK ) );
  PALTYPE palType;

  work->initWork	= *initWork;
	work->res				= res;

  work->menuWin = GFL_HEAP_AllocMemory( work->initWork.heapId , sizeof( GFL_BMPWIN* ) * work->initWork.itemNum );
  work->itemWork = GFL_HEAP_AllocMemory( work->initWork.heapId , sizeof( APP_TASKMENU_ITEMWORK ) * work->initWork.itemNum );
  GFL_STD_MemCopy16( work->initWork.itemWork , work->itemWork , sizeof(APP_TASKMENU_ITEMWORK) * work->initWork.itemNum );

  work->isDecide = FALSE;
  work->cursorPos = 0;
  work->anmCnt = 0;
  work->transAnmCnt = 0;
  

	{	
		work->initWork.w	= initWork->w == 0 ?  APP_TASKMENU_PLATE_WIDTH:  initWork->w;
		work->initWork.h	= initWork->h == 0 ?  APP_TASKMENU_PLATE_HEIGHT: initWork->h;
		APP_TASKMENU_CreateMenuWin( work, res );
	}

  if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY )
  {
    APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, TRUE );
  }
  
  work->isUpdateMsg = TRUE;
  
  return work;
}

//--------------------------------------------------------------
//	メニュー閉じる
//--------------------------------------------------------------
void APP_TASKMENU_CloseMenu( APP_TASKMENU_WORK *work )
{
  u8 i;
  for(i=0;i<work->initWork.itemNum;i++)
  {
    GFL_BMPWIN_ClearScreen( work->menuWin[i] );
    GFL_BMPWIN_Delete( work->menuWin[i] );
  }
  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(work->menuWin[0]) );
  
  GFL_HEAP_FreeMemory( work->menuWin );
  GFL_HEAP_FreeMemory( work->itemWork );
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//	メニュー更新
//--------------------------------------------------------------
void APP_TASKMENU_UpdateMenu( APP_TASKMENU_WORK *work )
{
  u8 i;
  //文字の更新
  if( work->isUpdateMsg == TRUE )
  {
    BOOL isFinish = TRUE;
    for(i=0;i<work->initWork.itemNum;i++)
    {
      if( PRINTSYS_QUE_IsExistTarget( work->res->printQue , GFL_BMPWIN_GetBmp( work->menuWin[i] ) ) == TRUE )
      {
        isFinish = FALSE;
      }
    }
    if( isFinish == TRUE )
    {
      for(i=0;i<work->initWork.itemNum;i++)
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( work->menuWin[i] );
      }
      work->isUpdateMsg = FALSE;
    }
  }
  
  if( work->isDecide == FALSE )
  {
    APP_TASKMENU_UpdateKey( work );
    if( work->isDecide == FALSE )
    {
      APP_TASKMENU_UpdateTP( work );
    }
  }
  else
  {
    //決定時アニメ
    const u8 isBlink = (work->anmCnt/APP_TASKMENU_ANM_INTERVAL)%2;
    if( isBlink == 0 )
    {
			APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, TRUE );
    }
    else
    {
			APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, FALSE );
    }
    work->anmCnt++;
  }
  
  APP_TASKMENU_UpdatePalletAnime( &work->transAnmCnt , &work->transCol , work->res->frame , work->res->plt );
}

//--------------------------------------------------------------
//	終了チェック
//--------------------------------------------------------------
const BOOL APP_TASKMENU_IsFinish( APP_TASKMENU_WORK *work )
{
  if( work->anmCnt < APP_TASKMENU_ANM_CNT )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

//--------------------------------------------------------------
//	カーソル位置の取得
//--------------------------------------------------------------
const u8 APP_TASKMENU_GetCursorPos( APP_TASKMENU_WORK *work )
{
  return work->cursorPos;
}

//--------------------------------------------------------------
//	アクティブの設定
//--------------------------------------------------------------
const void APP_TASKMENU_SetActive( APP_TASKMENU_WORK *work, BOOL isActive )
{ 
  APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, isActive );
}
//--------------------------------------------------------------
//	メニューのBmpWinと文字を作る
//--------------------------------------------------------------
static void APP_TASKMENU_CreateMenuWin( APP_TASKMENU_WORK *work, const APP_TASKMENU_RES *res )
{
  u8 i;
  
  u8 menuTop;
  u8 menuLeft;
  
  if( work->initWork.posType == ATPT_LEFT_UP )
  {
    menuLeft = work->initWork.charPosX;
    menuTop = work->initWork.charPosY;
  }
  else
  {
    menuLeft = work->initWork.charPosX - work->initWork.w;
    menuTop = work->initWork.charPosY - (work->initWork.h*work->initWork.itemNum);
  }
  
  for(i=0;i<work->initWork.itemNum;i++)
  {
    u32 charAdr;
    work->menuWin[i] = GFL_BMPWIN_Create( work->res->frame ,
                        menuLeft , menuTop+(i*work->initWork.h) , 
                        work->initWork.w, work->initWork.h, 
                        work->res->plt+1 , GFL_BMP_CHRAREA_GET_B );

    //プレートの絵を送る
		APP_TASKMENU_TransFrame( work->menuWin[i], res, work->itemWork[i].type );
    PRINTSYS_PrintQueColor( res->printQue , GFL_BMPWIN_GetBmp( work->menuWin[i] ), 
                        8+2 , 4+2 , work->itemWork[i].str , res->fontHandle , work->itemWork[i].msgColor );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->menuWin[i] );
  }
}

//--------------------------------------------------------------
//	項目のアクティブ・非アクティブ切り替え
//--------------------------------------------------------------
static void APP_TASKMENU_SetActiveItem( GFL_BMPWIN *bmpwin, const APP_TASKMENU_RES *res, const BOOL isActive )
{
  if( isActive == TRUE )
  {
    GFL_BMPWIN_SetPalette( bmpwin , res->plt );
  }
  else if( isActive == FALSE )
  {
    GFL_BMPWIN_SetPalette( bmpwin , res->plt+1 );
  }
	GFL_BMPWIN_MakeScreen( bmpwin );
  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(bmpwin) );
}

//--------------------------------------------------------------
//	パレットアニメーションの更新
//--------------------------------------------------------------
static void APP_TASKMENU_UpdatePalletAnime( u16 *anmCnt , u16 *transBuf , u8 bgFrame , u8 pltNo )
{
  //プレートアニメ
  if( *anmCnt + APP_TASKMENU_ANIME_VALUE >= 0x10000 )
  {
    *anmCnt = *anmCnt+APP_TASKMENU_ANIME_VALUE-0x10000;
  }
  else
  {
    *anmCnt += APP_TASKMENU_ANIME_VALUE;
  }
  {
    //1～0に変換
    const fx16 cos = (FX_CosIdx(*anmCnt)+FX16_ONE)/2;
    const u8 r = APP_TASKMENU_ANIME_S_R + (((APP_TASKMENU_ANIME_E_R-APP_TASKMENU_ANIME_S_R)*cos)>>FX16_SHIFT);
    const u8 g = APP_TASKMENU_ANIME_S_G + (((APP_TASKMENU_ANIME_E_G-APP_TASKMENU_ANIME_S_G)*cos)>>FX16_SHIFT);
    const u8 b = APP_TASKMENU_ANIME_S_B + (((APP_TASKMENU_ANIME_E_B-APP_TASKMENU_ANIME_S_B)*cos)>>FX16_SHIFT);
    
    *transBuf = GX_RGB(r, g, b);
    
    if( bgFrame <= GFL_BG_FRAME3_M )
    {
      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                          pltNo * 32 + APP_TASKMENU_ANIME_COL*2 ,
                                          transBuf , 2 );
    }
    else
    {
      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                          pltNo * 32 + APP_TASKMENU_ANIME_COL*2 ,
                                          transBuf , 2 );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレットアニメをした後、リソースを使い続ける際の、パレット初期化
 *
 *	@param	u8 bgFrame  BG面
 *	@param	pltNo       パレットNO
 */
//-----------------------------------------------------------------------------
static void APP_TASKMENU_ResetPallet( u16 *transBuf, u8 bgFrame , u8 pltNo )
{ 
    if( bgFrame <= GFL_BG_FRAME3_M )
    {
      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                          pltNo * 32 + APP_TASKMENU_ANIME_COL*2 ,
                                          transBuf , 2 );
    }
    else
    {
      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                          pltNo * 32 + APP_TASKMENU_ANIME_COL*2 ,
                                          transBuf , 2 );
    }
}

#pragma mark [>main func
//--------------------------------------------------------------
//	キー操作更新
//--------------------------------------------------------------
static void APP_TASKMENU_UpdateKey( APP_TASKMENU_WORK *work )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int repeat = GFL_UI_KEY_GetRepeat();

  if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
  {
    if( trg & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, TRUE );
      GFL_UI_SetTouchOrKey(GFL_APP_KTST_KEY);
    }
  }
  else
  {
    const u8 befPos = work->cursorPos;
    if( repeat & PAD_KEY_UP )
    {
      if( work->cursorPos == 0 )
      {
        work->cursorPos = work->initWork.itemNum-1;
      }
      else
      {
        work->cursorPos--;
      }
      work->transAnmCnt = 0;
      PMSND_PlaySystemSE( APP_TASKMENU_SND_CURSOR );
    }
    else
    if( repeat & PAD_KEY_DOWN )
    {
      if( work->cursorPos >= work->initWork.itemNum-1 )
      {
        work->cursorPos = 0;
      }
      else
      {
        work->cursorPos++;
      }
      work->transAnmCnt = 0;
      PMSND_PlaySystemSE( APP_TASKMENU_SND_CURSOR );
    }
    else
    if( trg & PAD_BUTTON_A )
    {
      work->isDecide = TRUE;
      PMSND_PlaySystemSE( APP_TASKMENU_SND_DECIDE );
    }
    else
    if( trg & PAD_BUTTON_B )
    {
      work->cursorPos = work->initWork.itemNum-1;
      work->isDecide = TRUE;
      PMSND_PlaySystemSE( APP_TASKMENU_SND_CANCEL );
    }
    
    if( befPos != work->cursorPos )
    {
      APP_TASKMENU_SetActiveItem( work->menuWin[befPos], work->res, FALSE );
      APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, TRUE );
    }
  }
}

static void APP_TASKMENU_UpdateTP( APP_TASKMENU_WORK *work )
{
  u8 i;
  int ret;
  GFL_UI_TP_HITTBL hitTbl[9];

  u8 menuTop;
  u8 menuLeft;
  if( work->initWork.posType == ATPT_LEFT_UP )
  {
    menuLeft = work->initWork.charPosX;
    menuTop = work->initWork.charPosY;
  }
  else
  {
    menuLeft = work->initWork.charPosX - work->initWork.w;
    menuTop = work->initWork.charPosY - (work->initWork.h*work->initWork.itemNum);
  }
  
  //テーブルの作成
  for( i=0 ; i<work->initWork.itemNum ; i++ )
  {
    hitTbl[i].rect.top    = menuTop*8 + (work->initWork.h*8*i);
    hitTbl[i].rect.bottom = menuTop*8 + (work->initWork.h*8*(i+1));
    hitTbl[i].rect.left   = menuLeft*8;
    hitTbl[i].rect.right  = (menuLeft+work->initWork.w)*8 - 1;
  }
  hitTbl[i].circle.code = GFL_UI_TP_HIT_END;
  
  ret = GFL_UI_TP_HitTrg( hitTbl );
  
  if( ret != GFL_UI_TP_HIT_NONE )
  {
    GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
		APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, FALSE );
    work->cursorPos = ret;
    work->isDecide = TRUE;
		APP_TASKMENU_SetActiveItem( work->menuWin[work->cursorPos], work->res, TRUE );
    if( work->cursorPos == 0 )
    {
      PMSND_PlaySystemSE( APP_TASKMENU_SND_DECIDE );
    }
    else
    {
			// @todo 後ほど、RETURNマークがついているものだけCANCELにする
      PMSND_PlaySystemSE( APP_TASKMENU_SND_DECIDE );
    }
  }
}

//=============================================================================
/**
 *					RES
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	APPTASKメニュー用リソース読み込み
 *
 *	@param	u8 frame	フレーム
 *	@param	plt				パレット番号
 *	@param	heapID		ヒープID
 *
 *	@return					リソース保持ワーク
 */
//-----------------------------------------------------------------------------
APP_TASKMENU_RES* APP_TASKMENU_RES_Create( u8 frame, u8 plt, GFL_FONT *fontHandle, PRINT_QUE *printQue, HEAPID heapID )
{	
	APP_TASKMENU_RES *wk;
  PALTYPE palType;

	//ワーク作成
	wk	= GFL_HEAP_AllocMemory( heapID ,sizeof(APP_TASKMENU_RES) );
	GFL_STD_MemClear( wk, sizeof(APP_TASKMENU_RES) );
	wk->frame	= frame;
	wk->plt		= plt;
	wk->fontHandle	= fontHandle;
	wk->printQue		= printQue;

	//上下画面
  if( frame <= GFL_BG_FRAME3_M )
  {
    palType = PALTYPE_MAIN_BG;
  }
  else
  {
    palType = PALTYPE_SUB_BG;
  }

	//読み込み
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE
//通常時処理
	wk->ncg_buf = GFL_ARC_UTIL_LoadBGCharacter( APP_COMMON_GetArcId() ,
			NARC_app_menu_common_task_menu_NCGR, FALSE, &wk->ncg_data, heapID );	
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , 
			NARC_app_menu_common_task_menu_NCLR , palType , plt*32 , 32*2 , heapID );	
#else
//DL子機時処理
	wk->ncg_buf = GFL_ARC_UTIL_LoadBGCharacter( APP_COMMON_GetArcId() ,
			NARC_app_menu_common_dl_task_menu_NCGR, FALSE, &wk->ncg_data, heapID );	
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , 
			NARC_app_menu_common_dl_task_menu_NCLR , palType , plt*32 , 32*2 , heapID );	
#endif //MULTI_BOOT_MAKE

	return wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	APPTASKMENU用リソース読み込み
 *
 *	@param	APP_TASKMENU_RES *wk ワーク
 */
//-----------------------------------------------------------------------------
void APP_TASKMENU_RES_Delete( APP_TASKMENU_RES *wk )
{	
	GFL_HEAP_FreeMemory( wk->ncg_buf );
	GFL_HEAP_FreeMemory( wk );
}


//=============================================================================
/**
 *						WIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	単発窓作成
 *
 *	@param	const APP_TASKMENU_WIN_WORK *res	窓のリソース
 *	@param	APP_TASKMENU_ITEMWORK *item	窓情報
 *	@param	x			X座標（キャラ単位）
 *	@param	y			Y座標（キャラ単位）
 *	@param	w			幅（キャラ単位）
 *
 *	@return	単発用ワーク
 */
//-----------------------------------------------------------------------------
APP_TASKMENU_WIN_WORK * APP_TASKMENU_WIN_Create( const APP_TASKMENU_RES *res, const APP_TASKMENU_ITEMWORK *item, u8 x, u8 y, u8 w, HEAPID heapID )
{	
	return APP_TASKMENU_WIN_CreateEx( res, item, x, y, w, APP_TASKMENU_PLATE_HEIGHT, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	単発窓作成高さ	指定できる版
 *
 *	@param	const APP_TASKMENU_WIN_WORK *res	窓のリソース
 *	@param	APP_TASKMENU_ITEMWORK *item	窓情報
 *	@param	x			X座標（キャラ単位）
 *	@param	y			Y座標（キャラ単位）
 *	@param	w			幅（キャラ単位）
 *	@param	h			高さ（キャラ単位）
 *
 *	@return	単発用ワーク
 */
//-----------------------------------------------------------------------------
APP_TASKMENU_WIN_WORK * APP_TASKMENU_WIN_CreateEx( const APP_TASKMENU_RES *res, const APP_TASKMENU_ITEMWORK *item, u8 x, u8 y, u8 w, u8 h, HEAPID heapID )
{	
	APP_TASKMENU_WIN_WORK *wk;

	//ワーク作成
	wk	= GFL_HEAP_AllocMemory( heapID ,sizeof(APP_TASKMENU_WIN_WORK) );
	GFL_STD_MemClear( wk, sizeof(APP_TASKMENU_WIN_WORK) );
	wk->res	= res;
	wk->isUpdateMsg	= TRUE;

	//BMPWIN
	wk->bmpwin	= GFL_BMPWIN_Create( res->frame, x, y , w, h, 
                        res->plt+1, GFL_BMP_CHRAREA_GET_B ); 

	//読み込み	
	APP_TASKMENU_TransFrame( wk->bmpwin, res, item->type );
	PRINTSYS_PrintQueColor( res->printQue , GFL_BMPWIN_GetBmp( wk->bmpwin ), 
                8+2 , 4+2 , item->str , res->fontHandle , item->msgColor );
	GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmpwin );

	return wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	単発ウィンドウ破棄
 *
 *	@param	APP_TASKMENU_WIN_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
void APP_TASKMENU_WIN_Delete( APP_TASKMENU_WIN_WORK *wk )
{	
	//BMPWIN削除
	GFL_BMPWIN_ClearScreen( wk->bmpwin );
	GFL_BMPWIN_Delete( wk->bmpwin );
	//ワーク削除
	GFL_HEAP_FreeMemory( wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	単発ウィンドウアップデート
 *
 *	@param	APP_TASKMENU_WIN_WORK *wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void APP_TASKMENU_WIN_Update( APP_TASKMENU_WIN_WORK *wk )
{	
  //文字の更新
  if( wk->isUpdateMsg )
  {
 		if( PRINTSYS_QUE_IsExistTarget( wk->res->printQue , GFL_BMPWIN_GetBmp( wk->bmpwin ) ) == FALSE )
		{
			GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmpwin );
			wk->isUpdateMsg = FALSE;
		}
  }
  
  if( wk->isDecide )
  {
    //決定時アニメ
    const u8 isBlink = (wk->anmCnt/APP_TASKMENU_ANM_INTERVAL)%2;
    if( isBlink == 0 )
    {
			APP_TASKMENU_SetActiveItem( wk->bmpwin, wk->res, TRUE );
    }
    else
		{
			APP_TASKMENU_SetActiveItem( wk->bmpwin, wk->res, FALSE );
    }
    wk->anmCnt++;
  }
  
  APP_TASKMENU_UpdatePalletAnime( &wk->transAnmCnt , &wk->transCol , wk->res->frame, wk->res->plt );
}
//----------------------------------------------------------------------------
/**
 *	@brief	アクティブON,OFF　（アクティブならば選択中のPLTANM演出　ノンアクティブならばしない）
 *
 *	@param	APP_TASKMENU_WIN_WORK *wk	ワーク
 *	@param	isActive									TRUEアクティブ	FALSEノンアクティブ
 */
//-----------------------------------------------------------------------------
void APP_TASKMENU_WIN_SetActive( APP_TASKMENU_WIN_WORK *wk, BOOL isActive )
{	
  wk->transAnmCnt = 0;
	APP_TASKMENU_SetActiveItem( wk->bmpwin, wk->res, isActive );
}
//----------------------------------------------------------------------------
/**
 *	@brief	決定ON,OFF
 *
 *	@param	APP_TASKMENU_WIN_WORK *wk	ワーク
 *	@param	isDecide	TRUEで決定アニメ	FALSEでなし
 */
//-----------------------------------------------------------------------------
void APP_TASKMENU_WIN_SetDecide( APP_TASKMENU_WIN_WORK *wk, BOOL isDecide )
{	
	wk->isDecide	= isDecide;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  終了チェック
 *
 *	@param	APP_TASKMENU_WIN_WORK *work ワーク
 *
 *	@retval TRUE:終了
 */
//-----------------------------------------------------------------------------
const BOOL APP_TASKMENU_WIN_IsFinish( APP_TASKMENU_WIN_WORK *work )
{
  if( work->anmCnt < APP_TASKMENU_ANM_CNT )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチチェック
 *
 *	@param	APP_TASKMENU_WIN_WORK *wk ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
const BOOL APP_TASKMENU_WIN_IsTrg( const APP_TASKMENU_WIN_WORK *wk )
{ 
  u32 x, y;
  GFL_RECT  rect;

  GFL_UI_TP_GetPointTrg( &x, &y );
  rect.left   = GFL_BMPWIN_GetPosX( wk->bmpwin ) * 8;
  rect.top    = GFL_BMPWIN_GetPosY( wk->bmpwin ) * 8;
  rect.right  = rect.left + GFL_BMPWIN_GetSizeX( wk->bmpwin ) * 8;
  rect.bottom = rect.top + GFL_BMPWIN_GetSizeX( wk->bmpwin ) * 8;

	if( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
			&	((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)))
	{
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フレームをBMPWINに転送
 *
 *	@param	GFL_BMPWIN *bmpwin					BMPWIN
 *	@param	APP_TASKMENU_WIN_WORK *res	リソース
 *	@param	type												窓の種類
 */
//-----------------------------------------------------------------------------
static void APP_TASKMENU_TransFrame( GFL_BMPWIN *bmpwin, const APP_TASKMENU_RES *res, APP_TASKMENU_WIN_TYPE type )
{	
	u32 dstAdr;
	u32 srcAdr;
	u8 w;
	u8 h;


	for( h = 0; h < GFL_BMPWIN_GetSizeY(bmpwin); h++ )
	{	
		for( w = 0; w < GFL_BMPWIN_GetSizeX(bmpwin); w++ )
		{	
			//転送先アドレスは連番
			dstAdr = (u32)(GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp( bmpwin )))
				+ 0x20*(h*GFL_BMPWIN_GetSizeX(bmpwin)+w);

			//転送元は場所により変化
			if( h == 0 && w == 0 )
			{	
				//左上
				srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_LEFTTOP;
			}
			else if( h == 0 && w == GFL_BMPWIN_GetSizeX(bmpwin)-1 )
			{	
				//右上
				srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_RIGHTTOP;
			}
			else if( h == 0 )
			{	
				//上
				srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_TOP;
			}
			else if( h == GFL_BMPWIN_GetSizeY(bmpwin)-1 && w == 0 )
			{	
				//左下
				srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_LEFTDOWN;
			}
			else if( h == GFL_BMPWIN_GetSizeY(bmpwin)-1 && w == GFL_BMPWIN_GetSizeX(bmpwin)-1 )
			{	
				//右下
				srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_RIGHTDOWN;
			}
			else if( h == GFL_BMPWIN_GetSizeY(bmpwin)-1 )
			{	
				//下
				srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_DOWN;
			}
			else
			{	
				//その他
				//場所が記号の場所で記号入力ONならば記号を
				//それ以外は普通に塗る
				if( type != APP_TASKMENU_WIN_TYPE_NORMAL
					&& w == GFL_BMPWIN_GetSizeX(bmpwin)-3 )
				{	
					srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*(APP_TASKMENU_PLATE_NCG_MARK_RET
																									+ (type-1) * APP_TASKMENU_PLATE_NCG_WIDTH);
				}
				else if( type != APP_TASKMENU_WIN_TYPE_NORMAL
					&& w == GFL_BMPWIN_GetSizeX(bmpwin)-2 )
				{	
					srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*(APP_TASKMENU_PLATE_NCG_MARK_RET
																									+ (type-1) * APP_TASKMENU_PLATE_NCG_WIDTH + 1);
				}
				else if( w == 0 )
				{	
					srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_LEFT;
				}
				else if( w == GFL_BMPWIN_GetSizeX(bmpwin)-1 )
				{	
					srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_RIGHT;
				}
				else
				{	
					srcAdr = (u32)(res->ncg_data->pRawData) + 0x20*APP_TASKMENU_PLATE_NCG_CENTER;
				}
			}
		
			//転送
			GFL_STD_MemCopy32( (void*)srcAdr, (void*)dstAdr, 0x20 );
		}
	}

}
