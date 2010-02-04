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
#include "app_menu_common.naix"
#include "message.naix"
#include "msg/msg_pokelist.h"
#include "msg/msg_wazaname.h"
#include "app/app_menu_common.h"

#include "print/wordset.h"
#include "system/bmp_winframe.h"
#include "app/app_taskmenu.h"

#include "plist_sys.h"
#include "plist_menu.h"
#include "plist_plate.h"
#include "plist_snd_def.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PLIST_MENU_ITEM_MAX (8)

#define PLIST_MENU_PLATE_LEFT (19)
#define PLIST_MENU_PLATE_WIDTH (13)
#define PLIST_MENU_PLATE_WIDTH_BATTLE (10)
#define PLIST_MENU_PLATE_HEIGHT (3)

//決定時点滅アニメ
#define PLIST_MENU_ANM_CNT (16)
#define PLIST_MENU_ANM_INTERVAL (4)

//プレートのアニメ。sin使うので0～0xFFFFのループ
#define PLIST_MENU_ANIME_VALUE (0x400)
#define PLIST_MENU_ANIME_S_R (5)
#define PLIST_MENU_ANIME_S_G (10)
#define PLIST_MENU_ANIME_S_B (13)
#define PLIST_MENU_ANIME_E_R (12)
#define PLIST_MENU_ANIME_E_G (25)
#define PLIST_MENU_ANIME_E_B (30)
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
  u8  itemNum;
  u16 itemArr[PLIST_MENU_ITEM_MAX];
  APP_TASKMENU_ITEMWORK itemWork[PLIST_MENU_ITEM_MAX];

  APP_TASKMENU_WORK *taskMenuWork;
  APP_TASKMENU_INITWORK *initTaskMenu;

  //メニュー土台
  NNSG2dCharacterData *ncgData;
  void *ncgRes; 
};
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void PLIST_MENU_CreateItem( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , PLIST_MENU_ITEM_TYPE *itemArr );
static STRBUF* PLIST_MENU_CreateMenuStr( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , const PLIST_MENU_ITEM_TYPE type );

static void PLIST_MENU_UpdateKey( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );
static void PLIST_MENU_UpdateTP( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );

static void PLIST_MENU_SetActiveItem( PLIST_MENU_WORK *menuWork , const u8 idx , const BOOL isActive );

//--------------------------------------------------------------
//	メニューシステム初期化
//--------------------------------------------------------------
PLIST_MENU_WORK* PLIST_MENU_CreateSystem( PLIST_WORK *work )
{
  u8 i;
  PLIST_MENU_WORK* menuWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( PLIST_MENU_WORK ) );
  //プレートの土台の絵
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , NARC_app_menu_common_task_menu_NCLR , 
                    PALTYPE_MAIN_BG , PLIST_BG_PLT_MENU_ACTIVE*32 , 32*2 , work->heapId );
  menuWork->ncgRes = GFL_ARC_UTIL_LoadBGCharacter( APP_COMMON_GetArcId() , NARC_app_menu_common_task_menu_NCGR , FALSE , 
                    &menuWork->ncgData , work->heapId );
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
  APP_TASKMENU_INITWORK taskInitWork;
  
  PLIST_MENU_CreateItem( work,menuWork,itemArr );

  taskInitWork.heapId = work->heapId;
  taskInitWork.itemNum = menuWork->itemNum;
  taskInitWork.itemWork = menuWork->itemWork;
  taskInitWork.posType = ATPT_RIGHT_DOWN;
  taskInitWork.charPosX = 32;
  taskInitWork.charPosY = 24;
  taskInitWork.w = APP_TASKMENU_PLATE_WIDTH;
  taskInitWork.h = APP_TASKMENU_PLATE_HEIGHT;
  
  GFL_UI_SetTouchOrKey( work->ktst );
  menuWork->taskMenuWork = APP_TASKMENU_OpenMenu( &taskInitWork, work->taskres );
}

//--------------------------------------------------------------
//	メニュー開く(YesNo
//--------------------------------------------------------------
void PLIST_MENU_OpenMenu_YesNo( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  u8 i;
  APP_TASKMENU_INITWORK taskInitWork;
  PLIST_MENU_ITEM_TYPE itemArr[3] = {PMIT_YES,PMIT_NO,PMIT_END_LIST};
  
  PLIST_MENU_CreateItem( work,menuWork,itemArr );

  taskInitWork.heapId = work->heapId;
  taskInitWork.itemNum = menuWork->itemNum;
  taskInitWork.itemWork = menuWork->itemWork;
  taskInitWork.posType = ATPT_RIGHT_DOWN;
  taskInitWork.charPosX = 32;
  taskInitWork.charPosY = 18;
  taskInitWork.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  taskInitWork.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;
  
  GFL_UI_SetTouchOrKey( work->ktst );
  menuWork->taskMenuWork = APP_TASKMENU_OpenMenu( &taskInitWork, work->taskres );
}

//--------------------------------------------------------------
//	メニュー閉じる
//--------------------------------------------------------------
void PLIST_MENU_CloseMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  u8 i;
  APP_TASKMENU_CloseMenu(menuWork->taskMenuWork);

  for( i=0;i<menuWork->itemNum;i++ )
  {
    GFL_STR_DeleteBuffer( menuWork->itemWork[i].str );
  }
  work->ktst = GFL_UI_CheckTouchOrKey();

}


//--------------------------------------------------------------
//	メニュー更新
//--------------------------------------------------------------
void PLIST_MENU_UpdateMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  u8 i;
  APP_TASKMENU_UpdateMenu(menuWork->taskMenuWork);
}

const PLIST_MENU_ITEM_TYPE PLIST_MENU_IsFinish( PLIST_WORK *work , PLIST_MENU_WORK *menuWork )
{
  if( APP_TASKMENU_IsFinish( menuWork->taskMenuWork ) == FALSE )
  {
    return PMIT_NONE;
  }
  else
  {
    const u8 ret = APP_TASKMENU_GetCursorPos( menuWork->taskMenuWork );
    return menuWork->itemArr[ret];
  }
}

#pragma mark [>init func
//--------------------------------------------------------------
//	メニューの項目を作る
//--------------------------------------------------------------
static void PLIST_MENU_CreateItem(  PLIST_WORK *work , PLIST_MENU_WORK *menuWork , PLIST_MENU_ITEM_TYPE *itemArr )
{
  int i;
  int arrNum = 0;
  
  //個数チェック
  while( itemArr[arrNum] != PMIT_END_LIST )
  {
    arrNum++;
    
    if( arrNum >= PLIST_MENU_ITEM_MAX )
    {
      GF_ASSERT_MSG(NULL,"PLIST_MENU Item num is over!!\n")
      break;
    }
  }
  
  menuWork->itemNum = 0;
  for( i=0;i<arrNum;i++ )
  {
    switch( itemArr[i] )
    {
    case PMIT_STATSU:  //強さを見る
      menuWork->itemArr[menuWork->itemNum] = PMIT_STATSU;
      menuWork->itemNum += 1;
      break;

    case PMIT_HIDEN:   //秘伝技。自動で個数分追加します
      {
        u8 idx;
        for( idx=0;idx<4;idx++ )
        {
          const u32 ret = PLIST_UTIL_CheckFieldWaza( work->selectPokePara , idx );
          if( ret != 0 )
          {
            menuWork->itemArr[menuWork->itemNum] = PMIT_WAZA_1+idx;
            menuWork->itemNum += 1;
          }
        }
      }
      break;
      
    case PMIT_WAZA: //PP回復用技リスト。自動で個数分追加します
      {
        u8 idx;
        for( idx=0;idx<4;idx++ )
        {
          const u32 wazaID = PP_Get( work->selectPokePara , ID_PARA_waza1+idx , NULL );
          if( wazaID != 0 )
          {
            menuWork->itemArr[menuWork->itemNum] = PMIT_WAZA_1+idx;
            menuWork->itemNum += 1;
          }
        }
      }
      break;

    case PMIT_CHANGE:  //入れ替え
      menuWork->itemArr[menuWork->itemNum] = PMIT_CHANGE;
      menuWork->itemNum += 1;
      break;

    case PMIT_ITEM:    //持ち物
      menuWork->itemArr[menuWork->itemNum] = PMIT_ITEM;
      menuWork->itemNum += 1;
      break;
      
    case  PMIT_MAIL:   //メール(持ち物の代わりに出る
      menuWork->itemArr[menuWork->itemNum] = PMIT_MAIL;
      menuWork->itemNum += 1;
      break;

    case PMIT_CLOSE:   //閉じる
      menuWork->itemArr[menuWork->itemNum] = PMIT_CLOSE;
      menuWork->itemNum += 1;
      break;

    case PMIT_LEAVE:   //預ける(育てや
      menuWork->itemArr[menuWork->itemNum] = PMIT_LEAVE;
      menuWork->itemNum += 1;
      break;

    case PMIT_SET_JOIN:    //参加する・参加しない
      //FIXME 取り消し処理チェック
      {
        const battleOrder = PLIST_PLATE_GetBattleOrder( work->plateWork[ work->pokeCursor ] );
        if( battleOrder == PPBO_JOIN_OK )
        {
          menuWork->itemArr[menuWork->itemNum] = PMIT_RET_JOIN;
          menuWork->itemNum += 1;
        }
        else
        if( battleOrder <= PPBO_JOIN_6 )
        {
          menuWork->itemArr[menuWork->itemNum] = PMIT_JOIN_CANCEL;
          menuWork->itemNum += 1;
        }
        //その他は増えない！
      }
      break;
      
    case PMIT_GIVE:    //持たせる
      menuWork->itemArr[menuWork->itemNum] = PMIT_GIVE;
      menuWork->itemNum += 1;
      break;

    case PMIT_TAKE:   //預かる
      menuWork->itemArr[menuWork->itemNum] = PMIT_TAKE;
      menuWork->itemNum += 1;
      break;
      
    case PMIT_ENTER:  //決定
      menuWork->itemArr[menuWork->itemNum] = PMIT_ENTER;
      menuWork->itemNum += 1;
      break;

    case PMIT_MAIL_READ:    //メールを読む
      menuWork->itemArr[menuWork->itemNum] = PMIT_MAIL_READ;
      menuWork->itemNum += 1;
      break;

    case PMIT_MAIL_TAKE:    //メールを取る
      menuWork->itemArr[menuWork->itemNum] = PMIT_MAIL_TAKE;
      menuWork->itemNum += 1;
      break;

    case PMIT_YES:    //はい
      menuWork->itemArr[menuWork->itemNum] = PMIT_YES;
      menuWork->itemNum += 1;
      break;

    case PMIT_NO:   //いいえ
      menuWork->itemArr[menuWork->itemNum] = PMIT_NO;
      menuWork->itemNum += 1;
      break;

    
    default:
      GF_ASSERT_MSG(0,"PLIST_MENU Invalid item type!![%d]\n",itemArr[arrNum-(i+1)] );
      break;
    }
  }

  for( i=0;i<menuWork->itemNum;i++ )
  {
    menuWork->itemWork[i].str = PLIST_MENU_CreateMenuStr( work,menuWork,menuWork->itemArr[i] );
    if( menuWork->itemArr[i] >= PMIT_WAZA_1 && menuWork->itemArr[i] <= PMIT_WAZA_4 &&
        work->plData->mode != PL_MODE_ITEMUSE ) //アイテム使用の時は普通の文字色
    {
      menuWork->itemWork[i].msgColor = PRINTSYS_LSB_Make( PLIST_FONT_MENU_WAZA_LETTER,PLIST_FONT_MENU_SHADOW,PLIST_FONT_MENU_BACK);
    }
    else
    {
      menuWork->itemWork[i].msgColor = PRINTSYS_LSB_Make( PLIST_FONT_MENU_LETTER,PLIST_FONT_MENU_SHADOW,PLIST_FONT_MENU_BACK);
    }
    
    if( menuWork->itemArr[i] == PMIT_CLOSE )
    {
      menuWork->itemWork[i].type = APP_TASKMENU_WIN_TYPE_RETURN;
    }
    else
    {
      menuWork->itemWork[i].type = APP_TASKMENU_WIN_TYPE_NORMAL;
    }
  }
}


//--------------------------------------------------------------
//	アイテムごとの文字の作成
//--------------------------------------------------------------
static STRBUF* PLIST_MENU_CreateMenuStr( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , const PLIST_MENU_ITEM_TYPE type )
{
  static const u32 msgArr[PMIT_MAX] =
  {
    mes_pokelist_05_02 ,  //PMIT_STATSU,  //強さを見る
    0 ,          //PMIT_HIDEN,   //秘伝技。自動で個数分追加します
    0 ,          //PMIT_WAZA,    //PP回復用技リスト。自動で個数分追加します
    mes_pokelist_05_01 ,  //PMIT_CHANGE,  //入れ替え
    mes_pokelist_05_03 ,  //PMIT_ITEM,    //持ち物
    mes_pokelist_05_04 ,  //PMIT_MAIL,    //メール(持ち物の代わりに出る
    mes_pokelist_05_08 ,  //PMIT_CLOSE,   //閉じる
    mes_pokelist_05_07 ,  //PMIT_LEAVE,   //預ける(育てや
    0 ,          //PMIT_SET_JOIN,    //参加する(参加しない)
    mes_pokelist_05_16 ,  //  PMIT_GIVE,    //持たせる
    mes_pokelist_05_17 ,  //  PMIT_TAKE,    //預かる
    mes_pokelist_10_04 ,  //  PMIT_ENTER,    //決定
    mes_pokelist_05_05 ,  //  PMIT_MAIL_READ,    //メールを読む
    mes_pokelist_05_06 ,  //  PMIT_MAIL_TAKE,    //メールを取る
    mes_pokelist_yes ,  //  PMIT_YES,    //はい
    mes_pokelist_no ,   //  PMIT_NO,    //いいえ
    0 ,          //PMIT_WAZA_1,   //技スロット１
    0 ,          //PMIT_WAZA_2,   //技スロット２
    0 ,          //PMIT_WAZA_3,   //技スロット３
    0 ,          //PMIT_WAZA_4,   //技スロット４
    mes_pokelist_05_10 ,  //PMIT_RET_JOIN,      //参加する
    mes_pokelist_05_11 ,  //PMIT_JOIN_CANCEL,   //参加しない
  };
  STRBUF *str;
  
  if( type >= PMIT_WAZA_1 && type <= PMIT_WAZA_4 )
  {
    //技は特殊処理
    const u32 wazaID = PP_Get( work->selectPokePara , ID_PARA_waza1+(type-PMIT_WAZA_1) , NULL );
    GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_wazaname_dat , work->heapId );
    
    str = GFL_MSG_CreateString( msgHandle , wazaID );
    GFL_MSG_Delete( msgHandle );
  }
  else
  {
    GF_ASSERT_MSG( msgArr[type] != 0 , "PLIST_MENU CreateMenuStr type error!![%d]\n",type );
    str = GFL_MSG_CreateString( work->msgHandle , msgArr[type] );
  }
  return str;
}

//--------------------------------------------------------------
//	バトル選択用に座標指定で出せるように
//--------------------------------------------------------------
APP_TASKMENU_WIN_WORK* PLIST_MENU_CreateMenuWin_BattleMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , u32 strId , u8 charX , u8 charY , const BOOL isReturn )
{
  APP_TASKMENU_ITEMWORK itemWork;
  APP_TASKMENU_WIN_WORK *winWork;
  itemWork.str = GFL_MSG_CreateString( work->msgHandle , strId );
  itemWork.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  if( isReturn == FALSE )
  {
    itemWork.type = APP_TASKMENU_WIN_TYPE_NORMAL;
  }
  else
  {
    itemWork.type = APP_TASKMENU_WIN_TYPE_RETURN;
  }
  winWork = APP_TASKMENU_WIN_Create( work->taskres , &itemWork , charX , charY , 
                  PLIST_MENU_PLATE_WIDTH_BATTLE , work->heapId );
  
  GFL_STR_DeleteBuffer( itemWork.str );
  return winWork;
}

//以下NULL例外を入れたラッパー
void PLIST_MENU_DeleteMenuWin_BattleMenu( APP_TASKMENU_WIN_WORK *work )
{
  if( work != NULL )
  {
    APP_TASKMENU_WIN_Delete( work );
  }
}
void PLIST_MENU_UpdateMenuWin_BattleMenu( APP_TASKMENU_WIN_WORK *work )
{
  if( work != NULL )
  {
    APP_TASKMENU_WIN_Update( work );
  }
}
void PLIST_MENU_SetActiveMenuWin_BattleMenu( APP_TASKMENU_WIN_WORK *work , const BOOL flg )
{
  if( work != NULL )
  {
    APP_TASKMENU_WIN_SetActive( work , flg );
  }
}
void PLIST_MENU_SetDecideMenuWin_BattleMenu( APP_TASKMENU_WIN_WORK *work , const BOOL flg )
{
  if( work != NULL )
  {
    APP_TASKMENU_WIN_SetDecide( work , flg );
  }
}

