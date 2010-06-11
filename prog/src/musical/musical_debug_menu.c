//======================================================================
/**
 * @file	musical_debug.c
 * @brief	ミュージカルデバッグ用
 * @author	ariizumi
 * @data	100322
 *
 * モジュール名：MUSICAL_DEBUG
 */
//======================================================================

#include "musical_entry.cdat"
#include "musical_debug_menu.h"
#include "poke_tool/poke_tool.h"
#include "debug/debugwin_sys.h"

#if PM_DEBUG

static void MUSICAL_DEBUG_InitMenu( MUSICAL_DEBUG_MENU_WORK *work );

//デバッグメニュー
static MUSICAL_DEBUG_MENU_WORK *musicalDebugWork = NULL;

#define MUSICAL_DEBUG_GROUP_TOP  (80)

void MUSICAL_DEBUG_InitDebugMenu( MUSICAL_SCRIPT_WORK *work , const HEAPID heapId )
{
  if( musicalDebugWork == NULL )
  {
    musicalDebugWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(MUSICAL_DEBUG_MENU_WORK) );
    musicalDebugWork->heapId = heapId;
    musicalDebugWork->scriptWork = work;
    musicalDebugWork->arr[0] = 0;
    musicalDebugWork->arr[1] = 1;
    musicalDebugWork->arr[2] = 2;
    musicalDebugWork->arr[3] = 3;
    musicalDebugWork->npc[0] = 0;
    musicalDebugWork->npc[1] = 1;
    musicalDebugWork->npc[2] = 2;
    MUSICAL_DEBUG_InitMenu( musicalDebugWork );

  }
}
void MUSICAL_DEBUG_TermDebugMenu( void )
{
  if( musicalDebugWork != NULL )
  {
    DEBUGWIN_RemoveGroup( MUSICAL_DEBUG_GROUP_TOP );
    GFL_HEAP_FreeMemory( musicalDebugWork );
    musicalDebugWork = NULL;
  }
}

MUSICAL_DEBUG_MENU_WORK* MUSICAL_DEBUG_GetWork( void )
{
  return musicalDebugWork;
}

static const BOOL MUSICAL_DEBUG_UpdateValue_u16( u16 *val , const u16 max );
static const BOOL MUSICAL_DEBUG_UpdateValue_BOOL( BOOL *val );

static void MD_U_EnableArr( void* userWork , DEBUGWIN_ITEM* item );
static void MD_D_EnableArr( void* userWork , DEBUGWIN_ITEM* item );
static void MD_U_Arr( void* userWork , DEBUGWIN_ITEM* item );
static void MD_D_Arr( void* userWork , DEBUGWIN_ITEM* item );
static void MD_U_EnableNpc( void* userWork , DEBUGWIN_ITEM* item );
static void MD_D_EnableNpc( void* userWork , DEBUGWIN_ITEM* item );
static void MD_U_Npc( void* userWork , DEBUGWIN_ITEM* item );
static void MD_D_Npc( void* userWork , DEBUGWIN_ITEM* item );
static void MD_U_EnableMax( void* userWork , DEBUGWIN_ITEM* item );
static void MD_D_EnableMax( void* userWork , DEBUGWIN_ITEM* item );

static const char StrArrOnOff[2][4] = {"OFF","ON"};

static void MUSICAL_DEBUG_InitMenu( MUSICAL_DEBUG_MENU_WORK *work )
{
  DEBUGWIN_AddGroupToTop( MUSICAL_DEBUG_GROUP_TOP , "MUSICAL" , work->heapId );

  DEBUGWIN_AddItemToGroupEx( MD_U_EnableArr ,MD_D_EnableArr , (void*)work , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Arr ,MD_D_Arr , (void*)0 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Arr ,MD_D_Arr , (void*)1 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Arr ,MD_D_Arr , (void*)2 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Arr ,MD_D_Arr , (void*)3 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_EnableNpc ,MD_D_EnableNpc , (void*)work , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Npc ,MD_D_Npc , (void*)0 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Npc ,MD_D_Npc , (void*)1 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_Npc ,MD_D_Npc , (void*)2 , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MD_U_EnableMax ,MD_D_EnableMax , (void*)work , MUSICAL_DEBUG_GROUP_TOP , work->heapId );
}


static const BOOL MUSICAL_DEBUG_UpdateValue_u16( u16 *val , const u16 max )
{
  u16 value;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){ value = 10; }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){ value = 100; }
  else{ value = 1; }

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( *val + value > max )
    {
      *val = 0;
    }
    else
    {
      *val += value;
    }
    return TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    if( *val < value )
    {
      *val = max;
    }
    else
    {
      *val -= value;
    }
    return TRUE;
  }
  return FALSE;
}
static const BOOL MUSICAL_DEBUG_UpdateValue_BOOL( BOOL *val )
{
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ||
      GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ||
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    *val = !*val;
    return TRUE;
  }
  return FALSE;
}

static void MD_U_EnableArr( void* userWork , DEBUGWIN_ITEM* item )
{
  MUSICAL_DEBUG_MENU_WORK *work = userWork;
  if( MUSICAL_DEBUG_UpdateValue_BOOL(&work->enableArr) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
 
static void MD_D_EnableArr( void* userWork , DEBUGWIN_ITEM* item )
{
  MUSICAL_DEBUG_MENU_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ならびじゅん[%s]",StrArrOnOff[work->enableArr] );
}

static void MD_U_Arr( void* userWork , DEBUGWIN_ITEM* item )
{
  int pos = (int)userWork;
  if( MUSICAL_DEBUG_UpdateValue_u16(&musicalDebugWork->arr[pos],3) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void MD_D_Arr( void* userWork , DEBUGWIN_ITEM* item )
{
  int pos = (int)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "キャラ%d[%dばん]",pos+1,musicalDebugWork->arr[pos]+1 );
}

static void MD_U_EnableNpc( void* userWork , DEBUGWIN_ITEM* item )
{
  MUSICAL_DEBUG_MENU_WORK *work = userWork;
  if( MUSICAL_DEBUG_UpdateValue_BOOL(&work->forceNpc) )
  {
    DEBUGWIN_RefreshScreen();
  }
}

static void MD_D_EnableNpc( void* userWork , DEBUGWIN_ITEM* item )
{
  MUSICAL_DEBUG_MENU_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "NPCこてい[%s]",StrArrOnOff[work->forceNpc] );
}

static void MD_U_Npc( void* userWork , DEBUGWIN_ITEM* item )
{
  int pos = (int)userWork;
  if( MUSICAL_DEBUG_UpdateValue_u16(&musicalDebugWork->npc[pos],5) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void MD_D_Npc( void* userWork , DEBUGWIN_ITEM* item )
{
  int pos = (int)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "NPC%d[%d]",pos+1,musicalDebugWork->npc[pos]+1 );
}

static void MD_U_EnableMax( void* userWork , DEBUGWIN_ITEM* item )
{
  MUSICAL_DEBUG_MENU_WORK *work = userWork;
  if( MUSICAL_DEBUG_UpdateValue_BOOL(&work->memMaxMode) )
  {
    DEBUGWIN_RefreshScreen();
  }
}

static void MD_D_EnableMax( void* userWork , DEBUGWIN_ITEM* item )
{
  MUSICAL_DEBUG_MENU_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "MaxMemory[%s]",StrArrOnOff[work->memMaxMode] );
}

#endif
