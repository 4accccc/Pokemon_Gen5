//======================================================================
/**
 * @file  debugwin_sys.c
 * @brief 汎用的デバッグシステム
 * @author  ariizumi
 * @data  09/05/01
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "print/printsys.h"

#include "debug/debug_str_conv.h"

#include "debug/debugwin_sys.h"
#include "debug/debug_system_group.h"

#if USE_DEBUGWIN_SYSTEM

//======================================================================
//  define
//======================================================================
#pragma mark [> define

#define DEBUGWIN_HEAPSIZE (0x2000)
#define DEBUGWIN_WIDTH  (20)
#define DEBUGWIN_HEIGHT (24)

#define DEBUGWIN_ITEM_NAME_LEN (24)

#define DEBUGWIN_LETTER_COLOR (0x0000)
#define DEBUGWIN_SELECT_COLOR (0x001f)

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
enum
{
  DFP_BACK,
  DFP_NORMAL,
  DFP_SHADOW,
  DFP_ACTIVE,
}DEBWIN_FONT_PLT;

//フラグ用ビット
enum
{
  DWF_IS_ACTIVE = 1,
  DWF_IS_INIT   = 2,
  DWF_IS_UPDATE = 4,
  DWF_MASK      = 0xFFFF,
}DEBUGWIN_FLAG;

//DEBUGWIN_ITEM フラグ用ビット
enum
{
  DIF_UPDATE    = 1,  //文字列に更新があったか
  DIF_GROUP     = 2,  //グループか？
  DIF_MASK      = 0xFFFF,
}DEBUGWIN_ITEM_FLAG;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct

struct _DEBUGWIN_ITEM
{
  STRBUF  *name;
  void    *work;
  DEBUGWIN_UPDATE_FUNC updateFunc;
  DEBUGWIN_DRAW_FUNC drawFunc;
  DEBUGWIN_ITEM *prevItem;
  DEBUGWIN_ITEM *nextItem;
  u16      flg;
};

struct _DEBUGWIN_GROUP
{
  u8  linkNum;
  u8  id;
  DEBUGWIN_ITEM *topLinkData;
  DEBUGWIN_ITEM *lastLinkData;
  DEBUGWIN_ITEM *selfData;
  DEBUGWIN_GROUP  *parentGroup;
};

typedef struct
{
  u16 flg;
  u16 letterColor;
  u16 selectColor;
  u32 frmnum;
  GFL_FONT *fontHandle;
  DEBUGWIN_GROUP  *topGroup;
  DEBUGWIN_GROUP  *activeGroup;
  DEBUGWIN_ITEM   *activeItem;

  u8  *charTempArea;      ///<キャラクタVRAM退避先
  u16 *scrnTempArea;      ///<スクリーンVRAM退避先
  u16 *plttTempArea;      ///<パレットVRAM退避先
  GFL_BMP_DATA *bmp;

  u8  fontColBkup[3];
  u8  priorytyBkup;
  u8  scrollXBkup;
  u8  scrollYBkup;


}DEBUGWIN_SYSTEM_WORK;


//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
static void DEBUGWIN_OpenDebugWindow( void );
static void DEBUGWIN_CloseDebugWindow( void );

static void DEBUGWIN_UpdateDebugWindow( void );
static void DEBUGWIN_DrawDebugWindow( const BOOL forceDraw , const BOOL reset);

static DEBUGWIN_GROUP* DEBUGWIN_CreateGroup( const u8 id , const char *nameStr , const HEAPID heapId );
static void DEBUGWIN_RemoveGroupFunc( DEBUGWIN_GROUP *group );
static DEBUGWIN_GROUP* DEBUGWIN_SearchGroupFromId( const u8 id );
static DEBUGWIN_GROUP* DEBUGWIN_SearchGroupFromIdFunc( const u8 id , DEBUGWIN_GROUP *grp );
static DEBUGWIN_GROUP* DEBUGWIN_AddGroupToGroupFunc( const u8 id , const char *nameStr , DEBUGWIN_GROUP *parentGroup, const HEAPID heapId );

static DEBUGWIN_ITEM* DEBUGWIN_CreateItem( const char *nameStr , void *work , DEBUGWIN_UPDATE_FUNC updateFunc , DEBUGWIN_DRAW_FUNC drawFunc , const HEAPID heapId );
static void DEBUGWIN_AddItemToGroupFunc( DEBUGWIN_GROUP *grp , DEBUGWIN_ITEM *item );
static void DEBUGWIN_RemoveGroupFunc( DEBUGWIN_GROUP *group );
static void DEBUGWIN_RemoveGroupChild( DEBUGWIN_GROUP *group );

static void DEBUGWIN_DeleteItem( DEBUGWIN_ITEM *item );

static DEBUGWIN_SYSTEM_WORK *debWork = NULL;


//--------------------------------------------------------------
//
//--------------------------------------------------------------
void DEBUGWIN_InitSystem( u8* charArea , u16* scrnArea , u16* plttArea )
{

  if( debWork != NULL )
  {
//    OS_TPrintf("Debug system is initialized yet!!\n");
    return;
  }

  GFL_HEAP_CreateHeap( GFL_HEAPID_SYSTEM, GFL_HEAP_LOWID(HEAPID_DEBUGWIN), DEBUGWIN_HEAPSIZE );

  debWork = GFL_HEAP_AllocMemory( HEAPID_DEBUGWIN , sizeof(DEBUGWIN_SYSTEM_WORK) );
  debWork->flg = 0;
  debWork->charTempArea = charArea;
  debWork->scrnTempArea = scrnArea;
  debWork->plttTempArea = plttArea;
  DEBUGWIN_ExitProc();

  debWork->topGroup = DEBUGWIN_CreateGroup( DEBUGWIN_GROUPID_TOPMENU , "TopMenu" , HEAPID_DEBUGWIN );

  DEBUGWIN_AddSystemGroup(HEAPID_DEBUGWIN);

//  OS_TPrintf("Heap[%d]\n", GFL_HEAP_GetHeapFreeSize(HEAPID_DEBUGWIN) );
}

void DEBUGWIN_ExitSystem(void)
{
  if( debWork == NULL )
  {
    OS_TPrintf("Debug system isn't initialized!!\n");
    return;
  }

  DEBUGWIN_RemoveGroupFunc( debWork->topGroup );
  GFL_HEAP_FreeMemory( debWork );
  debWork = NULL;
  GFL_HEAP_DeleteHeap( HEAPID_DEBUGWIN );

}

void DEBUGWIN_UpdateSystem(void)
{
  if( (debWork->flg & DWF_IS_INIT) )
  {
    if( (GFL_UI_KEY_GetCont()& DEBUGWIN_CONT_KEY) &&
        (GFL_UI_KEY_GetTrg() & DEBUGWIN_TRG_KEY ))
    {
      debWork->flg = debWork->flg ^ DWF_IS_ACTIVE;

      if( DEBUGWIN_IsActive() == TRUE )
      {
        DEBUGWIN_OpenDebugWindow();
      }
      else
      {
        DEBUGWIN_CloseDebugWindow();
      }
    }
    if( DEBUGWIN_IsActive() == TRUE )
    {
      DEBUGWIN_UpdateDebugWindow();
      DEBUGWIN_DrawDebugWindow( FALSE , FALSE );
    }
  }
}

BOOL DEBUGWIN_IsActive(void)
{
  //1フレームだけ更新
  if( debWork->flg & DWF_IS_UPDATE )
  {
    debWork->flg = debWork->flg & (DWF_MASK-DWF_IS_UPDATE);

    return FALSE;
  }
  return (debWork->flg & DWF_IS_ACTIVE);
}

void DEBUGWIN_InitProc( const u32 frmnum , GFL_FONT *fontHandle )
{
  debWork->frmnum = frmnum;
  debWork->fontHandle = fontHandle;
  debWork->flg = debWork->flg | DWF_IS_INIT;
  debWork->activeGroup = debWork->topGroup;
  debWork->activeItem = NULL;

  debWork->letterColor = DEBUGWIN_LETTER_COLOR;
  debWork->selectColor = DEBUGWIN_SELECT_COLOR;
}

void DEBUGWIN_ExitProc( void )
{
  debWork->frmnum = 0;
  debWork->fontHandle = NULL;
  debWork->flg = debWork->flg & (DWF_MASK-DWF_IS_INIT);
}

static void DEBUGWIN_OpenDebugWindow( void )
{
  //VRAMのデータを退避
  GFL_STD_MemCopy16(GFL_DISPUT_GetCgxPtr(debWork->frmnum), debWork->charTempArea, DEBUGWIN_CHAR_TEMP_AREA);
  GFL_STD_MemCopy16(GFL_DISPUT_GetScrPtr(debWork->frmnum), debWork->scrnTempArea, DEBUGWIN_SCRN_TEMP_AREA);
  GFL_STD_MemCopy16(GFL_DISPUT_GetPltPtr(debWork->frmnum), debWork->plttTempArea, DEBUGWIN_PLTT_TEMP_AREA);
  //Fontカラーの退避
  GFL_FONTSYS_GetColor( &debWork->fontColBkup[0] ,
                        &debWork->fontColBkup[1] ,
                        &debWork->fontColBkup[2] );

  //もろもろ退避
  debWork->priorytyBkup = GFL_BG_GetPriority(debWork->frmnum);
  debWork->scrollXBkup = GFL_BG_GetScrollX(debWork->frmnum);
  debWork->scrollYBkup = GFL_BG_GetScrollY(debWork->frmnum);

  //上で退避させたものの設定
  GFL_BG_SetPriority( debWork->frmnum , 0 );
  GFL_BG_SetScroll( debWork->frmnum , GFL_BG_SCROLL_X_SET , 0 );
  GFL_BG_SetScroll( debWork->frmnum , GFL_BG_SCROLL_Y_SET , 0 );

  //スクリーンの作成
  {
    u8 x,y;
    u16 *buf = GFL_HEAP_AllocMemory( HEAPID_DEBUGWIN , sizeof( u16 )*32*DEBUGWIN_HEIGHT );
    for( y=0;y<DEBUGWIN_HEIGHT;y++ )
    {
      for( x=0;x<DEBUGWIN_WIDTH;x++ )
      {
        buf[x+y*32] = x+y*DEBUGWIN_WIDTH;
      }
      for( x=DEBUGWIN_WIDTH;x<32;x++ )
      {
        buf[x+y*32] = DEBUGWIN_HEIGHT*DEBUGWIN_WIDTH;
      }
    }
    DC_FlushRange(buf, sizeof( u16 )*32*DEBUGWIN_HEIGHT);
    GFL_STD_MemCopy16(buf, GFL_DISPUT_GetScrPtr(debWork->frmnum), sizeof( u16 )*32*DEBUGWIN_HEIGHT);

    GFL_HEAP_FreeMemory( buf );
  }

  //パレットの作成
  {
    u16 col[4]={ 0x0000 , 0x0000 , 0x7fff , 0x001f };
    col[DFP_NORMAL] = debWork->letterColor;
    col[DFP_ACTIVE] = debWork->selectColor;
    GFL_BG_LoadPalette( debWork->frmnum , col , sizeof(u16)*4 , 0 );
//    GFL_STD_MemCopy16(col, GFL_DISPUT_GetPltPtr(debWork->frmnum), sizeof(u16)*4);
  }

  debWork->bmp = GFL_BMP_CreateInVRAM( GFL_DISPUT_GetCgxPtr(debWork->frmnum) , DEBUGWIN_WIDTH , DEBUGWIN_HEIGHT , GFL_BMP_16_COLOR , HEAPID_DEBUGWIN );
  GFL_STD_MemClear16( GFL_DISPUT_GetCgxPtr(debWork->frmnum) , DEBUGWIN_CHAR_TEMP_AREA );
  //GFL_BMP_Clear( debWork->bmp , 0 );

  DEBUGWIN_DrawDebugWindow( TRUE , FALSE );
}

static void DEBUGWIN_CloseDebugWindow( void )
{
  GFL_BMP_Delete( debWork->bmp );

  //もろもろ復帰
  GFL_BG_SetScroll( debWork->frmnum , GFL_BG_SCROLL_X_SET , debWork->scrollXBkup );
  GFL_BG_SetScroll( debWork->frmnum , GFL_BG_SCROLL_Y_SET , debWork->scrollYBkup );
  GFL_BG_SetPriority( debWork->frmnum , debWork->priorytyBkup );
  //Fontカラーの復帰
  GFL_FONTSYS_SetColor( debWork->fontColBkup[0] ,
                        debWork->fontColBkup[1] ,
                        debWork->fontColBkup[2] );
  GFL_STD_MemCopy16(debWork->charTempArea, GFL_DISPUT_GetCgxPtr(debWork->frmnum), DEBUGWIN_CHAR_TEMP_AREA);
  GFL_STD_MemCopy16(debWork->scrnTempArea, GFL_DISPUT_GetScrPtr(debWork->frmnum), DEBUGWIN_SCRN_TEMP_AREA);
  GFL_STD_MemCopy16(debWork->plttTempArea, GFL_DISPUT_GetPltPtr(debWork->frmnum), DEBUGWIN_PLTT_TEMP_AREA);
}

static void DEBUGWIN_UpdateDebugWindow( void )
{
  if( debWork->activeItem != NULL )
  {
    if( debWork->activeItem->updateFunc != NULL )
    {
      debWork->activeItem->updateFunc( debWork->activeItem->work , (void*)debWork->activeItem );
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      debWork->activeItem->flg |= DIF_UPDATE;
      if( debWork->activeItem->prevItem != NULL )
      {
        debWork->activeItem = debWork->activeItem->prevItem;
      }
      else
      {
        debWork->activeItem = debWork->activeGroup->lastLinkData;
      }
      debWork->activeItem->flg |= DIF_UPDATE;
    }
    else
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      debWork->activeItem->flg |= DIF_UPDATE;
      if( debWork->activeItem->nextItem != NULL )
      {
        debWork->activeItem = debWork->activeItem->nextItem;
      }
      else
      {
        debWork->activeItem = debWork->activeGroup->topLinkData;
      }
      debWork->activeItem->flg |= DIF_UPDATE;
    }
    else
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      if( debWork->activeItem->flg & DIF_GROUP )
      {
        debWork->activeGroup = (DEBUGWIN_GROUP*)debWork->activeItem->work;
        debWork->activeItem = debWork->activeGroup->topLinkData;
        DEBUGWIN_DrawDebugWindow( TRUE , TRUE );
      }
    }
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    if( debWork->activeGroup->parentGroup != NULL )
    {
      debWork->activeGroup = debWork->activeGroup->parentGroup;
      debWork->activeItem = debWork->activeGroup->topLinkData;
      DEBUGWIN_DrawDebugWindow( TRUE , TRUE );
    }
  }
}


static void DEBUGWIN_DrawDebugWindow( const BOOL forceDraw , const BOOL reset)
{
  DEBUGWIN_ITEM *item = debWork->activeGroup->topLinkData;
  u8  line = 1;

  if( reset == TRUE )
  {
    GFL_BMP_Clear( debWork->bmp , 0 );
  }
  //今のグループ
  if( reset == TRUE || forceDraw == TRUE )
  {
    GFL_FONTSYS_SetColor( DFP_NORMAL ,DFP_SHADOW ,DFP_BACK );
    PRINTSYS_Print( debWork->bmp , 0, 0, debWork->activeGroup->selfData->name, debWork->fontHandle );
  }

  //カーソル位置不明だったらTopにする
  if( debWork->activeItem == NULL )
  {
    debWork->activeItem = item;
  }

  while( item != NULL )
  {
    if( (item->flg & DIF_UPDATE) || forceDraw == TRUE )
    {
      if( item->drawFunc != NULL )
      {
        item->drawFunc( item->work , (void*)item );
      }

      if( item == debWork->activeItem )
      {
        GFL_FONTSYS_SetColor( DFP_ACTIVE ,DFP_SHADOW ,DFP_BACK );
      }
      else
      {
        GFL_FONTSYS_SetColor( DFP_NORMAL ,DFP_SHADOW ,DFP_BACK );
      }
      PRINTSYS_Print( debWork->bmp , 8, line*16, item->name, debWork->fontHandle );

      item->flg = item->flg & (DIF_MASK-DIF_UPDATE);
    }
    line++;
    item = item->nextItem;
  }
}

#pragma mark [>Group Func

void DEBUGWIN_AddGroupToTop( const u8 id , const char *nameStr , const HEAPID heapId )
{
  DEBUGWIN_AddGroupToGroupFunc( id , nameStr , debWork->topGroup , heapId );
}

void DEBUGWIN_AddGroupToGroup( const u8 id , const char *nameStr , const u8 parentGroupId, const HEAPID heapId )
{
  DEBUGWIN_GROUP *grp = DEBUGWIN_SearchGroupFromId( parentGroupId );
  if( grp == NULL )
  {
//    OS_TPrintf("DEBUG WINDOW:Group[%d] is not found!!\n",parentGroupId);
  }
  else
  {
    DEBUGWIN_AddGroupToGroupFunc( id , nameStr , grp , heapId );
  }
}

static DEBUGWIN_GROUP* DEBUGWIN_AddGroupToGroupFunc( const u8 id , const char *nameStr , DEBUGWIN_GROUP *parentGroup, const HEAPID heapId )
{
  if( DEBUGWIN_SearchGroupFromId( id ) != NULL )
  {
//    OS_TPrintf("DEBUG WINDOW:Group[%d] is registed yet!!\n",id);
    return NULL;
  }
  else
  {
    DEBUGWIN_GROUP *grp = DEBUGWIN_CreateGroup( id , nameStr , heapId );
    DEBUGWIN_ITEM *item = parentGroup->topLinkData;

    DEBUGWIN_AddItemToGroupFunc( parentGroup , grp->selfData );

    grp->parentGroup = parentGroup;

//    OS_TPrintf("DEBUG WINDOW:Group[%d] is regist.\n",id);
    return grp;
  }
}


void DEBUGWIN_AddItemToGroup( const char *nameStr , DEBUGWIN_UPDATE_FUNC updateFunc , void *work , const u8 parentGroupId , const HEAPID heapId )
{
  DEBUGWIN_GROUP *grp = DEBUGWIN_SearchGroupFromId( parentGroupId );
  if( grp == NULL )
  {
//    OS_TPrintf("DEBUG WINDOW:Group[%d] is not found!!\n",parentGroupId);
  }
  else
  {
    DEBUGWIN_ITEM *item = DEBUGWIN_CreateItem( nameStr , work , updateFunc , NULL , heapId );
    DEBUGWIN_AddItemToGroupFunc( grp , item );
  }
}

void DEBUGWIN_AddItemToGroupEx( DEBUGWIN_UPDATE_FUNC updateFunc , DEBUGWIN_DRAW_FUNC drawFunc , void *work , const u8 parentGroupId , const HEAPID heapId )
{
  DEBUGWIN_GROUP *grp = DEBUGWIN_SearchGroupFromId( parentGroupId );
  if( grp == NULL )
  {
//    OS_TPrintf("DEBUG WINDOW:Group[%d] is not found!!\n",parentGroupId);
  }
  else
  {
    DEBUGWIN_ITEM *item = DEBUGWIN_CreateItem( "" , work , updateFunc , drawFunc , heapId );
    DEBUGWIN_AddItemToGroupFunc( grp , item );
  }
}
static void DEBUGWIN_AddItemToGroupFunc( DEBUGWIN_GROUP *grp , DEBUGWIN_ITEM *item )
{
  if( grp->topLinkData == NULL )
  {
    grp->topLinkData = item;
    grp->lastLinkData = item;
  }
  else
  {
    DEBUGWIN_ITEM *targetItem = grp->topLinkData;
    while( targetItem->nextItem != NULL )
    {
      targetItem = targetItem->nextItem;
    }
    targetItem->nextItem = item;
    item->prevItem = targetItem;

    grp->lastLinkData = item;
  }
  grp->linkNum++;
}

static DEBUGWIN_GROUP* DEBUGWIN_CreateGroup( const u8 id , const char *nameStr , const HEAPID heapId )
{
  DEBUGWIN_GROUP *grp = GFL_HEAP_AllocMemory( heapId , sizeof(DEBUGWIN_GROUP) );

  grp->linkNum = 0;
  grp->id = id;
  grp->topLinkData = NULL;
  grp->parentGroup = NULL;
  grp->selfData = DEBUGWIN_CreateItem( nameStr , grp , NULL , NULL , heapId );
  DEBUGWIN_ITEM_SetNameV( grp->selfData , "[%s]" , nameStr );
  grp->selfData->flg |= DIF_GROUP;

  return grp;
}

static void DEBUGWIN_RemoveGroupFunc( DEBUGWIN_GROUP *group )
{
  //親から自身のリンクを切る
  DEBUGWIN_GROUP *parent = group->parentGroup;
  if( parent != NULL )
  {
    DEBUGWIN_ITEM *selfItem = group->parentGroup->topLinkData;
    while(  selfItem != group->selfData &&
            selfItem != NULL )
    {
      selfItem = selfItem->nextItem;
    }
    if( selfItem != NULL )
    {
      //該当アイテムがあったのでリンクをつなぎなおす
      if( selfItem->nextItem != NULL )
      {
        selfItem->nextItem->prevItem = selfItem->prevItem;
      }
      if( selfItem->prevItem != NULL )
      {
        selfItem->prevItem->nextItem = selfItem->nextItem;
      }

      //トップと末尾だったときの処理
      if( parent->topLinkData == selfItem )
      {
        parent->topLinkData = selfItem->nextItem;
      }
      if( parent->lastLinkData == selfItem )
      {
        parent->lastLinkData = selfItem->prevItem;
      }
    }
  }
  DEBUGWIN_RemoveGroupChild( group );
}

void DEBUGWIN_RemoveGroup( const u8 id )
{
  DEBUGWIN_GROUP *grp = DEBUGWIN_SearchGroupFromId( id );
  if( grp == NULL )
  {
//    OS_TPrintf("DEBUG WINDOW:Group[%d] is not found!!\n",id);
  }
  else
  {
    DEBUGWIN_RemoveGroupFunc( grp );
  }

}

static void DEBUGWIN_RemoveGroupChild( DEBUGWIN_GROUP *group )
{
  DEBUGWIN_ITEM *item = group->topLinkData;
  while( item != NULL )
  {
    DEBUGWIN_ITEM *nextItem = item->nextItem;

    if( item->flg & DIF_GROUP )
    {
      DEBUGWIN_RemoveGroupFunc( (DEBUGWIN_GROUP*)item->work );
    }
    else
    {
      DEBUGWIN_DeleteItem( item );
    }

    item = nextItem;
  }
//  OS_TPrintf("DEBUG WINDOW:Group[%d] is removed.\n",group->id);
  DEBUGWIN_DeleteItem( group->selfData );
  GFL_HEAP_FreeMemory( group );
}

//グループIDからグループを取得
static DEBUGWIN_GROUP* DEBUGWIN_SearchGroupFromId( const u8 id )
{
  return DEBUGWIN_SearchGroupFromIdFunc( id , debWork->topGroup );
}
static DEBUGWIN_GROUP* DEBUGWIN_SearchGroupFromIdFunc( const u8 id , DEBUGWIN_GROUP *grp )
{
  DEBUGWIN_ITEM *item = grp->topLinkData;

  while( item != NULL )
  {
    if( item->flg & DIF_GROUP )
    {
      DEBUGWIN_GROUP *group = (DEBUGWIN_GROUP*)item->work;
      if( group->id == id )
      {
        return group;
      }
      else
      {
        DEBUGWIN_GROUP *retGrp = DEBUGWIN_SearchGroupFromIdFunc( id , group );
        if( retGrp != NULL )
        {
          return retGrp;
        }
      }
    }
    item = item->nextItem;
  }
  return NULL;
}

#pragma mark [>Item Func
static DEBUGWIN_ITEM* DEBUGWIN_CreateItem( const char *nameStr , void *work , DEBUGWIN_UPDATE_FUNC updateFunc , DEBUGWIN_DRAW_FUNC drawFunc , const HEAPID heapId )
{
  DEBUGWIN_ITEM *item = GFL_HEAP_AllocMemory( heapId , sizeof(DEBUGWIN_ITEM) );

  item->name = GFL_STR_CreateBuffer(DEBUGWIN_ITEM_NAME_LEN,heapId);
  item->work = work;
  item->updateFunc = updateFunc;
  item->drawFunc = drawFunc;
  item->prevItem = NULL;
  item->nextItem = NULL;
  item->flg = 0;

  //名前のセット
  if( item->drawFunc == NULL )
  {
    DEBUGWIN_ITEM_SetName( item , nameStr );
  }

  return item;
}

static void DEBUGWIN_DeleteItem( DEBUGWIN_ITEM *item )
{
  GFL_STR_DeleteBuffer( item->name );
  GFL_HEAP_FreeMemory( item );
}

void DEBUGWIN_ITEM_SetName( DEBUGWIN_ITEM* item , const char *nameStr )
{
  int bufLen = DEBUGWIN_ITEM_NAME_LEN-1;
  STRCODE strCode[DEBUGWIN_ITEM_NAME_LEN];

  //STD_ConvertStringSjisToUnicode( strCode , &bufLen , nameStr , NULL , NULL );
  DEB_STR_CONV_SjisToStrcode( nameStr , strCode , bufLen );

  strCode[bufLen] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( item->name , strCode );

  item->flg |= DIF_UPDATE;
}

void DEBUGWIN_ITEM_SetNameV( DEBUGWIN_ITEM* item , char *nameStr , ...)
{
  char str[DEBUGWIN_ITEM_NAME_LEN-1];
  va_list vlist;
  va_start(vlist, nameStr);
  STD_TVSNPrintf( str , DEBUGWIN_ITEM_NAME_LEN-1 , nameStr , vlist );
  va_end(vlist);
  DEBUGWIN_ITEM_SetName( item , str );
}

void DEBUGWIN_ITEM_SetNameU16( DEBUGWIN_ITEM* item , const STRCODE *strcode )
{
  GFL_STR_SetStringCode( item->name , strcode );
  item->flg |= DIF_UPDATE;
}

#pragma mark [>Draw Func
//--------------------------------------------------------------
//  色の変更
//--------------------------------------------------------------
void DEBUGWIN_ChangeLetterColor( const u8 r , const u8 g , const u8 b )
{
  debWork->letterColor = GX_RGB(r, g, b);
}

void DEBUGWIN_ChangeSelectColor( const u8 r , const u8 g , const u8 b )
{
  debWork->selectColor = GX_RGB(r, g, b);
}


//画面全更新
void DEBUGWIN_RefreshScreen( void )
{
  DEBUGWIN_DrawDebugWindow( TRUE , TRUE );
}

void DEBUGWIN_UpdateFrame( void )
{
  debWork->flg |= DWF_IS_UPDATE;
}

/*
void DEBUGWIN_DrawStr( const u8 posX , const u8 posY , const HEAPID heapId , const char* str )
{
  STRBUF *strBuf;
  STRCODE *strCode;
  int len = STD_StrLen( str );

  strBuf = GFL_STR_CreateBuffer( len+1 , heapId );
  strCode = GFL_HEAP_AllocMemory( heapId , sizeof(STRCODE)*(len+1) );

  //STD_ConvertStringSjisToUnicode( strCode , &len , str , NULL , NULL );
  DEB_STR_CONV_SjisToStrcode( str , strCode , len );
  strCode[len] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( strBuf , strCode );

  GFL_FONTSYS_SetColor( DFP_NORMAL ,DFP_SHADOW ,DFP_BACK );
  PRINTSYS_Print( debWork->bmp , posX, posY, strBuf, debWork->fontHandle );

  GFL_HEAP_FreeMemory( strCode );
  GFL_STR_DeleteBuffer( strBuf );
}
*/
#pragma mark [>System Group

#else //USE_DEBUGWIN_SYSTEM


#pragma mark [>dummy
void DEBUGWIN_InitSystem( u8* charArea , u16* scrnArea , u16* plttArea ){}
void DEBUGWIN_UpdateSystem(void){}
void DEBUGWIN_ExitSystem(void){}
BOOL DEBUGWIN_IsActive(void){return FALSE;}
void DEBUGWIN_InitProc( const u32 frmnum , GFL_FONT *fontHandle ){}
void DEBUGWIN_ExitProc( void ){}
void DEBUGWIN_AddGroupToTop( const u8 id , const char *nameStr , const HEAPID heapId ){}
void DEBUGWIN_AddGroupToGroup( const u8 id , const char *nameStr , const u8 parentGroupId, const HEAPID heapId ){}
void DEBUGWIN_RemoveGroup( const u8 id ){}
void DEBUGWIN_AddItemToGroup( const char *nameStr , DEBUGWIN_UPDATE_FUNC updateFunc , void *work , const u8 parentGroupId , const HEAPID heapId ){}
void DEBUGWIN_AddItemToGroupEx( DEBUGWIN_UPDATE_FUNC updateFunc , DEBUGWIN_DRAW_FUNC drawFunc , void *work , const u8 parentGroupId , const HEAPID heapId ){}
void DEBUGWIN_ITEM_SetName( DEBUGWIN_ITEM* item , const char *nameStr ){}
void DEBUGWIN_ITEM_SetNameV( DEBUGWIN_ITEM* item , char *nameStr , ...){}
void DEBUGWIN_RefreshScreen( void ){}
void DEBUGWIN_UpdateFrame( void ){}


#endif //USE_DEBUGWIN_SYSTEM

