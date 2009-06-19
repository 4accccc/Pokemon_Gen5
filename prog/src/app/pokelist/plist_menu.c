//======================================================================
/**
 * @file	plist_menu.c
 * @brief	ポケモンリスト メニュー
 * @author	ariizumi
 * @data	09/06/18
 *
 * モジュール名：PLIST_MENU
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "pokelist_gra.naix"
#include "msg/msg_pokelist.h"

#include "print/wordset.h"
#include "system/bmp_winframe.h"

#include "plist_menu.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PLIST_MENU_PLATE_LEFT (19)
#define PLIST_MENU_PLATE_WIDTH (13)
#define PLIST_MENU_PLATE_HEIGHT (3)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _PLIST_MENU_WORK
{
  u8 itemNum;
  BOOL isUpdateMsg;

  GFL_BMPWIN *menuWin[8];

  //メニュー土台
  NNSG2dCharacterData *ncgData;
  void *ncgRes; 

};
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto


//--------------------------------------------------------------
//	メニューシステム初期化
//--------------------------------------------------------------
PLIST_MENU_WORK* PLIST_MENU_CreateSystem( PLIST_WORK *work )
{
  u8 i;
  PLIST_MENU_WORK* menuWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( PLIST_MENU_WORK ) );

  //プレートの土台の絵
  menuWork->ncgRes = GFL_ARC_UTIL_LoadBGCharacter( ARCID_POKELIST , NARC_pokelist_gra_list_sel_NCGR , FALSE , 
                    &menuWork->ncgData , work->heapId );
  
  menuWork->isUpdateMsg = FALSE;
  
  for(i=0;i<8;i++)
  {
    menuWork->menuWin[i] = NULL;
  }

  return menuWork;
}

//--------------------------------------------------------------
//	メニューシステム開放
//--------------------------------------------------------------
void PLIST_MENU_DeleteSystem( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  GFL_HEAP_FreeMemory( menuWork->ncgRes );
  GFL_HEAP_FreeMemory( menuWork );
}

//--------------------------------------------------------------
//	メニュー開く
//--------------------------------------------------------------
void PLIST_MENU_OpenMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , PLIST_MENU_ITEM_TYPE *itemArr )
{
  u8 i;
  
  for(i=0;i<8;i++)
  {
    STRBUF *str = GFL_MSG_CreateString( work->msgHandle , mes_pokelist_05_01+i );
    menuWork->menuWin[i] = GFL_BMPWIN_Create( PLIST_BG_MENU ,
                        PLIST_MENU_PLATE_LEFT , 24-((i+1)*PLIST_MENU_PLATE_HEIGHT) , 
                        PLIST_MENU_PLATE_WIDTH , PLIST_MENU_PLATE_HEIGHT , 
                        PLIST_BG_PLT_MENU_NORMAL , GFL_BMP_CHRAREA_GET_B );
    //プレートの絵を送る
    GFL_STD_MemCopy32( menuWork->ncgData->pRawData , GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp( menuWork->menuWin[i] )) ,
                     0x20*PLIST_MENU_PLATE_WIDTH*PLIST_MENU_PLATE_HEIGHT );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( menuWork->menuWin[i] ), 
                        8 , 4 , str , work->fontHandle , 
                        PRINTSYS_LSB_Make( PLIST_FONT_MENU_LETTER,PLIST_FONT_MENU_HIDEN_LETTER,PLIST_FONT_MENU_BACK) );
    GFL_STR_DeleteBuffer( str );
  }
  menuWork->itemNum = 8;
  menuWork->isUpdateMsg = TRUE;
}

//--------------------------------------------------------------
//	メニュー閉じる
//--------------------------------------------------------------
void PLIST_MENU_CloseMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  u8 i;
  for(i=0;i<menuWork->itemNum;i++)
  {
    GFL_BMPWIN_Delete( menuWork->menuWin[i] );
    menuWork->menuWin[i] = NULL;
  }
}


//--------------------------------------------------------------
//	メニュー更新
//--------------------------------------------------------------
void PLIST_MENU_UpdateMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  u8 i;
  
  //文字の更新
  if( menuWork->isUpdateMsg == TRUE )
  {
    BOOL isFinish = TRUE;
    for(i=0;i<menuWork->itemNum;i++)
    {
      if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( menuWork->menuWin[i] ) ) == TRUE )
      {
        isFinish = FALSE;
      }
    }
    if( isFinish == TRUE )
    {
      for(i=0;i<menuWork->itemNum;i++)
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( menuWork->menuWin[i] );
      }
      menuWork->isUpdateMsg = FALSE;
    }
  }
  for(i=0;i<menuWork->itemNum;i++)
  {
    
  }
}

