//======================================================================
/**
 * @file	debug_system_group.c
 * @brief	デバッグシステムのデフォルトグループ登録
 * @author	ariizumi
 * @data	09/05/08
 */
//======================================================================
#include <gflib.h>

#include "debug/debugwin_sys.h"
#include "debug/debug_system_group.h"
#include "test/performance.h"
#include "test/debug_pause.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  RTCTime rtcTime;
  RTCDate rtcDate;
}DEBUG_SYS_GROUP_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void DEBWIN_Update_PMeterOn( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_PMeterOff( void* userWork , DEBUGWIN_ITEM* item );

static void DEBWIN_Update_Pause( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_Pause( void* userWork , DEBUGWIN_ITEM* item );

static void DEBWIN_Update_Kanji( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_Kanji( void* userWork , DEBUGWIN_ITEM* item );

static void DEBWIN_Update_RTC_year( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_month( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_day( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_hour( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_minute( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_sec( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_apply( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_RTC_get( void* userWork , DEBUGWIN_ITEM* item );

static void DEBWIN_Draw_RTC_year( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_RTC_month( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_RTC_day( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_RTC_hour( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_RTC_minute( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_RTC_sec( void* userWork , DEBUGWIN_ITEM* item );

static void DEBWIN_UTIL_UpdateU32( u32 *value , const u32 min, const u32 max );

DEBUG_SYS_GROUP_WORK *sysGroupWork = NULL;
//--------------------------------------------------------------
//	
//--------------------------------------------------------------

void DEBUGWIN_AddSystemGroup( const HEAPID heapId )
{
  sysGroupWork = GFL_HEAP_AllocMemory( heapId , sizeof(DEBUG_SYS_GROUP_WORK) );
  
  GFL_RTC_GetDateTime( &sysGroupWork->rtcDate , &sysGroupWork->rtcTime );
  
  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUPID_SYSTEM , "System" , heapId );
  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUPID_RTC , "RTC" , DEBUGWIN_GROUPID_SYSTEM , heapId );
  DEBUGWIN_AddItemToGroup( "パフォーマンスメーターON",DEBWIN_Update_PMeterOn , NULL , DEBUGWIN_GROUPID_SYSTEM , heapId );
  DEBUGWIN_AddItemToGroup( "パフォーマンスメーターOFF",DEBWIN_Update_PMeterOff , NULL , DEBUGWIN_GROUPID_SYSTEM , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_Pause   ,DEBWIN_Draw_Pause   , (void*)sysGroupWork , DEBUGWIN_GROUPID_SYSTEM , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_Kanji   ,DEBWIN_Draw_Kanji   , (void*)sysGroupWork , DEBUGWIN_GROUPID_SYSTEM , heapId );

  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_RTC_year   ,DEBWIN_Draw_RTC_year   , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_RTC_month  ,DEBWIN_Draw_RTC_month  , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_RTC_day    ,DEBWIN_Draw_RTC_day    , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_RTC_hour   ,DEBWIN_Draw_RTC_hour   , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_RTC_minute ,DEBWIN_Draw_RTC_minute , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_RTC_sec    ,DEBWIN_Draw_RTC_sec    , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroup( "適用",DEBWIN_Update_RTC_apply , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
  DEBUGWIN_AddItemToGroup( "現在時刻取得",DEBWIN_Update_RTC_get , (void*)sysGroupWork , DEBUGWIN_GROUPID_RTC , heapId );
}

void DEBUGWIN_RemoveSystemGroup(void)
{
  DEBUGWIN_RemoveGroup( 254 );
}

//パフォーマンスメーター
static void DEBWIN_Update_PMeterOn(  void* userWork , DEBUGWIN_ITEM* item )
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    DEBUG_PerformanceSetActive( TRUE );
  }
}
static void DEBWIN_Update_PMeterOff(  void* userWork , DEBUGWIN_ITEM* item )
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    DEBUG_PerformanceSetActive( FALSE );
  }
}

//一時停止機能
static void DEBWIN_Update_Pause( void* userWork , DEBUGWIN_ITEM* item )
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    const BOOL flg = DEBUG_PAUSE_GetEnable();
    DEBUG_PAUSE_SetEnable( !flg );
    DEBUGWIN_RefreshScreen();
  }
}

static void DEBWIN_Draw_Pause( void* userWork , DEBUGWIN_ITEM* item )
{
  const BOOL flg = DEBUG_PAUSE_GetEnable();
  if( flg == TRUE )
  {
    DEBUGWIN_ITEM_SetNameV( item , "ポーズ機能[現在ON]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetNameV( item , "ポーズ機能[現在OFF]" );
  }
}

//漢字
static void DEBWIN_Update_Kanji( void* userWork , DEBUGWIN_ITEM* item )
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    const u8 langId = GFL_MSGSYS_GetLangID();
    if( langId == 0 )
    {
      GFL_MSGSYS_SetLangID( 1 );
    }
    else
    {
      GFL_MSGSYS_SetLangID( 0 );
    }
    DEBUGWIN_RefreshScreen();
  }
}

static void DEBWIN_Draw_Kanji( void* userWork , DEBUGWIN_ITEM* item )
{
  const u8 langId = GFL_MSGSYS_GetLangID();
  if( langId == 0 )
  {
    DEBUGWIN_ITEM_SetNameV( item , "漢字モード[現在OFF]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetNameV( item , "漢字モード[現在ON]" );
  }
}

//RTC
static void DEBWIN_Update_RTC_year( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBWIN_UTIL_UpdateU32( &work->rtcDate.year , 0 , 99 );
}
static void DEBWIN_Update_RTC_month( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBWIN_UTIL_UpdateU32( &work->rtcDate.month , 1 , 12 );
}
static void DEBWIN_Update_RTC_day( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBWIN_UTIL_UpdateU32( &work->rtcDate.day , 1 , 31 );
}
static void DEBWIN_Update_RTC_hour( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBWIN_UTIL_UpdateU32( &work->rtcTime.hour , 0 , 23 );
}
static void DEBWIN_Update_RTC_minute( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBWIN_UTIL_UpdateU32( &work->rtcTime.minute , 0 , 59 );
}
static void DEBWIN_Update_RTC_sec( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBWIN_UTIL_UpdateU32( &work->rtcTime.second , 0 , 59 );
}
static void DEBWIN_Update_RTC_apply( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    RTC_SetDateTime( &work->rtcDate , &work->rtcTime );
  }
}
static void DEBWIN_Update_RTC_get( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    GFL_RTC_GetDateTime( &work->rtcDate , &work->rtcTime );
    DEBUGWIN_RefreshScreen();
  }
}

static void DEBWIN_Draw_RTC_year( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "年[%4d]",work->rtcDate.year+2000 );
}
static void DEBWIN_Draw_RTC_month( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "月[%2d]",work->rtcDate.month );
}
static void DEBWIN_Draw_RTC_day( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "日[%2d]",work->rtcDate.day );
}
static void DEBWIN_Draw_RTC_hour( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "時[%2d]",work->rtcTime.hour );
}
static void DEBWIN_Draw_RTC_minute( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "分[%2d]",work->rtcTime.minute );
}
static void DEBWIN_Draw_RTC_sec( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUG_SYS_GROUP_WORK *work = (DEBUG_SYS_GROUP_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "秒[%2d]",work->rtcTime.second );
}

static void DEBWIN_UTIL_UpdateU32( u32 *value , const u32 min, const u32 max )
{
  u32 addValue = 1;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    addValue = 5;
  }
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    if( *value + addValue <= max )
    {
      *value += addValue;
    }
    else
    {
      *value = min;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  {
    if( *value  < min + addValue )
    {
      *value = max;
    }
    else
    {
      *value -= addValue;
    }
    DEBUGWIN_RefreshScreen();
  }
}

