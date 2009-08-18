//======================================================================
/**
 * @file	bmp_taskmenu.c
 * @brief	タスクバー用 共通メニュー
 * @author	ariizumi
 * @data	09/07/27
 *
 * モジュール名：BMP_TASKMENU
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "app/app_menu_common.h"
#include "print/wordset.h"
#include "sound/pm_sndsys.h"

#include "arc_def.h"
#include "app_menu_common.naix"

#include "app/app_taskmenu.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define APP_TASKMENU_PLATE_LEFT (19)
#define APP_TASKMENU_PLATE_WIDTH (13)
#define APP_TASKMENU_PLATE_HEIGHT (3)

#define APP_TASKMENU_PLATE_NCG_SIZE (0x20*APP_TASKMENU_PLATE_WIDTH*APP_TASKMENU_PLATE_HEIGHT)

//プレートのアニメ。sin使うので0～0xFFFFのループ
#define APP_TASKMENU_ANIME_VALUE (0x400)
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
  NNSG2dCharacterData *ncgData;
  void *ncgRes; 
  
  APP_TASKMENU_INITWORK initWork;

};
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void APP_TASKMENU_CreateMenuWin( APP_TASKMENU_WORK *work );
static void APP_TASKMENU_SetActiveItem( APP_TASKMENU_WORK *work , const u8 idx , const BOOL isActive );
static void APP_TASKMENU_UpdateKey( APP_TASKMENU_WORK *work );
static void APP_TASKMENU_UpdateTP( APP_TASKMENU_WORK *work );

//--------------------------------------------------------------
//	メニュー開く
//--------------------------------------------------------------
APP_TASKMENU_WORK* APP_TASKMENU_OpenMenu( APP_TASKMENU_INITWORK *initWork )
{
  APP_TASKMENU_WORK *work = GFL_HEAP_AllocMemory( initWork->heapId , sizeof( APP_TASKMENU_WORK ) );
  PALTYPE palType;

  work->initWork = *initWork;

  if( work->initWork.bgFrame <= GFL_BG_FRAME3_M )
  {
    palType = PALTYPE_MAIN_BG;
  }
  else
  {
    palType = PALTYPE_SUB_BG;
  }

  //プレートの土台の絵
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , NARC_app_menu_common_task_menu_NCLR , 
                    palType , work->initWork.palNo*32 , 32*2 , work->initWork.heapId );
  work->ncgRes = GFL_ARC_UTIL_LoadBGCharacter( APP_COMMON_GetArcId() , NARC_app_menu_common_task_menu_NCGR , FALSE , 
                    &work->ncgData , work->initWork.heapId );
  
  work->menuWin = GFL_HEAP_AllocMemory( work->initWork.heapId , sizeof( GFL_BMPWIN* ) * work->initWork.itemNum );
  work->itemWork = GFL_HEAP_AllocMemory( work->initWork.heapId , sizeof( APP_TASKMENU_ITEMWORK ) * work->initWork.itemNum );
  GFL_STD_MemCopy16( work->initWork.itemWork , work->itemWork , sizeof(APP_TASKMENU_ITEMWORK) * work->initWork.itemNum );

  work->isDecide = FALSE;
  work->cursorPos = 0;
  work->anmCnt = 0;
  work->transAnmCnt = 0;
  
  APP_TASKMENU_CreateMenuWin( work );

  if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY )
  {
    APP_TASKMENU_SetActiveItem( work , work->cursorPos , TRUE );
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
  GFL_BG_LoadScreenV_Req( work->initWork.bgFrame );
  
  GFL_HEAP_FreeMemory( work->menuWin );
  GFL_HEAP_FreeMemory( work->itemWork );
  GFL_HEAP_FreeMemory( work->ncgRes );
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
      if( PRINTSYS_QUE_IsExistTarget( work->initWork.printQue , GFL_BMPWIN_GetBmp( work->menuWin[i] ) ) == TRUE )
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
      APP_TASKMENU_SetActiveItem( work , work->cursorPos , TRUE );
    }
    else
    {
      APP_TASKMENU_SetActiveItem( work , work->cursorPos , FALSE );
    }
    work->anmCnt++;
  }
  
  APP_TASKMENU_UpdatePalletAnime( &work->transAnmCnt , &work->transCol , work->initWork.palNo );
  
  //プレートアニメ
  /*
  if( work->transAnmCnt + APP_TASKMENU_ANIME_VALUE >= 0x10000 )
  {
    work->transAnmCnt = work->transAnmCnt+APP_TASKMENU_ANIME_VALUE-0x10000;
  }
  else
  {
    work->transAnmCnt += APP_TASKMENU_ANIME_VALUE;
  }
  {
    //1～0に変換
    const fx16 cos = (FX_CosIdx(work->transAnmCnt)+FX16_ONE)/2;
    const u8 r = APP_TASKMENU_ANIME_S_R + (((APP_TASKMENU_ANIME_E_R-APP_TASKMENU_ANIME_S_R)*cos)>>FX16_SHIFT);
    const u8 g = APP_TASKMENU_ANIME_S_G + (((APP_TASKMENU_ANIME_E_G-APP_TASKMENU_ANIME_S_G)*cos)>>FX16_SHIFT);
    const u8 b = APP_TASKMENU_ANIME_S_B + (((APP_TASKMENU_ANIME_E_B-APP_TASKMENU_ANIME_S_B)*cos)>>FX16_SHIFT);
    
    work->transCol = GX_RGB(r, g, b);
    
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                        work->initWork.palNo * 32 + APP_TASKMENU_ANIME_COL*2 ,
                                        &work->transCol , 2 );
  }
  */
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
//	メニューのBmpWinと文字を作る
//--------------------------------------------------------------
static void APP_TASKMENU_CreateMenuWin( APP_TASKMENU_WORK *work )
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
    menuLeft = work->initWork.charPosX - APP_TASKMENU_PLATE_WIDTH;
    menuTop = work->initWork.charPosY - (APP_TASKMENU_PLATE_HEIGHT*work->initWork.itemNum);
  }
  
  for(i=0;i<work->initWork.itemNum;i++)
  {
    u32 charAdr;
    work->menuWin[i] = GFL_BMPWIN_Create( work->initWork.bgFrame ,
                        menuLeft , menuTop+(i*APP_TASKMENU_PLATE_HEIGHT) , 
                        APP_TASKMENU_PLATE_WIDTH , APP_TASKMENU_PLATE_HEIGHT , 
                        work->initWork.palNo+1 , GFL_BMP_CHRAREA_GET_B );
    //プレートの絵を送る
    if( work->itemWork[i].isReturn == TRUE )
    {
      charAdr = (u32)(work->ncgData->pRawData) + APP_TASKMENU_PLATE_NCG_SIZE;
    }
    else
    {
      charAdr = (u32)(work->ncgData->pRawData);
    }
    GFL_STD_MemCopy32( (void*)charAdr , GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp( work->menuWin[i] )) ,
                     APP_TASKMENU_PLATE_NCG_SIZE );

    PRINTSYS_PrintQueColor( work->initWork.printQue , GFL_BMPWIN_GetBmp( work->menuWin[i] ), 
                        8+2 , 4+2 , work->itemWork[i].str , work->initWork.fontHandle , work->itemWork[i].msgColor );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->menuWin[i] );
  }
}

//--------------------------------------------------------------
//	項目のアクティブ・非アクティブ切り替え
//--------------------------------------------------------------
static void APP_TASKMENU_SetActiveItem( APP_TASKMENU_WORK *work , const u8 idx , const BOOL isActive )
{
  if( isActive == TRUE )
  {
    GFL_BMPWIN_SetPalette( work->menuWin[idx] , work->initWork.palNo );
  }
  else if( isActive == FALSE )
  {
    GFL_BMPWIN_SetPalette( work->menuWin[idx] , work->initWork.palNo+1 );
  }
	GFL_BMPWIN_MakeScreen( work->menuWin[idx] );
  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(work->menuWin[idx]) );
}

//--------------------------------------------------------------
//	パレットアニメーションの更新
//--------------------------------------------------------------
void APP_TASKMENU_UpdatePalletAnime( u16 *anmCnt , u16 *transBuf , u8 pltNo )
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
    
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
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
    if( trg != 0 )
    {
      APP_TASKMENU_SetActiveItem( work , work->cursorPos , TRUE );
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
      APP_TASKMENU_SetActiveItem( work , befPos , FALSE );
      APP_TASKMENU_SetActiveItem( work , work->cursorPos , TRUE );
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
    menuLeft = work->initWork.charPosX - APP_TASKMENU_PLATE_WIDTH;
    menuTop = work->initWork.charPosY - (APP_TASKMENU_PLATE_HEIGHT*work->initWork.itemNum);
  }
  
  //テーブルの作成
  for( i=0 ; i<work->initWork.itemNum ; i++ )
  {
    hitTbl[i].rect.top    = menuTop*8 + (APP_TASKMENU_PLATE_HEIGHT*8*i);
    hitTbl[i].rect.bottom = menuTop*8 + (APP_TASKMENU_PLATE_HEIGHT*8*(i+1));
    hitTbl[i].rect.left   = menuLeft*8;
    hitTbl[i].rect.right  = (menuLeft+APP_TASKMENU_PLATE_WIDTH)*8 - 1;
  }
  hitTbl[i].circle.code = GFL_UI_TP_HIT_END;
  
  ret = GFL_UI_TP_HitTrg( hitTbl );
  
  if( ret != GFL_UI_TP_HIT_NONE )
  {
    GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
    APP_TASKMENU_SetActiveItem( work , work->cursorPos , FALSE );
    work->cursorPos = ret;
    work->isDecide = TRUE;
    APP_TASKMENU_SetActiveItem( work , work->cursorPos , TRUE );
    if( work->cursorPos == 0 )
    {
      PMSND_PlaySystemSE( APP_TASKMENU_SND_DECIDE );
    }
    else
    {
      PMSND_PlaySystemSE( APP_TASKMENU_SND_CANCEL );
    }
  }
}

