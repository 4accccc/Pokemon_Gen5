//======================================================================
/**
 * @file	debug_flg.c
 * @brief	グローバルに参照できるフラグの管理
 * @author	ariizumi
 * @data	09/10/27
 *
 * モジュール名：DEBUG_FLG
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "debug/debug_flg.h"
#include "debug/debugwin_sys.h"
#include "debug/debug_system_group.h"  //ぐるーぷID参照

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define DEBUG_FLG_ARR_NUM ((DEBUG_FLG_MAX+7)/8)
#define DEBUG_FLG_PAGE_NUM (10)
#define DEBUG_FLG_PAGE_MAX ((DEBUG_FLG_MAX+DEBUG_FLG_PAGE_NUM-1)/DEBUG_FLG_PAGE_NUM)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void DEBUG_FLG_InitFlg( void );
static void DEBWIN_PageChangeFunc( void ); 
static void DEBWIN_Update_Page( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_Page( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Update_Flg( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_Flg( void* userWork , DEBUGWIN_ITEM* item );

#define DEBUG_FLG_DEF(flgName,dispName) dispName ,
static const char * const dispName[DEBUG_FLG_MAX] =
{
#include "debug/debug_flg.cdat"
};
#undef DEBUG_FLG_DEF

static u8 debugFlgArr[DEBUG_FLG_ARR_NUM];
static u8 debugFlgPage;

//--------------------------------------------------------------
//	フラグ初期設定
//--------------------------------------------------------------
static void DEBUG_FLG_InitFlg( void )
{
  //ここにはデフォルトでONにしたい物を設定してください。
  
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko) /*| defined(DEBUG_ONLY_FOR_iwao_kazumasa)*/
  //DEBUG_FLG_FlgOn( DEBUG_FLG_MusicalEventSkip );
#endif

}



//--------------------------------------------------------------
//	フラグ操作
//--------------------------------------------------------------
void DEBUG_FLG_FlgOn( const u16 flgIdx )
{
  const u8 bitIdx = flgIdx%8;
  const u8 arrIdx = flgIdx/8;
  GF_ASSERT_MSG( flgIdx < DEBUG_FLG_MAX , "invalid flg index[%d]\n",flgIdx );
  OS_TPrintf("DEBUG_FLG [%s] -> ON\n",dispName[flgIdx]);
  
  debugFlgArr[arrIdx] |= 1<<bitIdx;
}
void DEBUG_FLG_FlgOff( const u16 flgIdx )
{
  const u8 bitIdx = flgIdx%8;
  const u8 arrIdx = flgIdx/8;
  GF_ASSERT_MSG( flgIdx < DEBUG_FLG_MAX , "invalid flg index[%d]\n",flgIdx );
  OS_TPrintf("DEBUG_FLG [%s] -> OFF\n",dispName[flgIdx]);
  
  debugFlgArr[arrIdx] &= (0xFF-(1<<bitIdx));
}
void DEBUG_FLG_FlgFlip( const u16 flgIdx )
{
  const u8 bitIdx = flgIdx%8;
  const u8 arrIdx = flgIdx/8;
  GF_ASSERT_MSG( flgIdx < DEBUG_FLG_MAX , "invalid flg index[%d]\n",flgIdx );

  debugFlgArr[arrIdx] ^= (1<<bitIdx);
  if( debugFlgArr[arrIdx] & (1<<bitIdx) )
  {
    OS_TPrintf("DEBUG_FLG [%s] -> ON\n",dispName[flgIdx]);
  }
  else
  {
    OS_TPrintf("DEBUG_FLG [%s] -> OFF\n",dispName[flgIdx]);
  }
  
}

const BOOL DEBUG_FLG_GetFlg( const u16 flgIdx )
{
  const u8 bitIdx = flgIdx%8;
  const u8 arrIdx = flgIdx/8;
  GF_ASSERT_MSG( flgIdx < DEBUG_FLG_MAX , "invalid flg index[%d]\n",flgIdx );
  
  if( debugFlgArr[arrIdx] & (1<<bitIdx) )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//--------------------------------------------------------------
//	デバッググループに登録・解除
//--------------------------------------------------------------
void DEBUG_FLG_CreateDebugGoupe( const HEAPID heapId )
{
  u8 i;
  debugFlgPage = 0;
  for( i=0;i<DEBUG_FLG_ARR_NUM;i++ )
  {
    debugFlgArr[i] = 0;
  }
  DEBUG_FLG_InitFlg();

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUPID_DEBUG_FLG , "DebugFlg" , heapId );

  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_Page ,DEBWIN_Draw_Page , NULL , DEBUGWIN_GROUPID_DEBUG_FLG , heapId );
  for( i=0;i<DEBUG_FLG_PAGE_NUM;i++ )
  {
    DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_Flg ,DEBWIN_Draw_Flg , (void*)i , DEBUGWIN_GROUPID_DEBUG_FLG , heapId );
  }

}

void DEBUG_FLG_DeleteDebugGoupe( void )
{
  DEBUGWIN_RemoveGroup(DEBUGWIN_GROUPID_DEBUG_FLG);
}

//--------------------------------------------------------------
//	ページ操作
//--------------------------------------------------------------
static void DEBWIN_Update_Page( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBWIN_PageChangeFunc();
}

static void DEBWIN_Draw_Page( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "ページ[%2d]",debugFlgPage );
}

static void DEBWIN_PageChangeFunc( void )
{
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( debugFlgPage < DEBUG_FLG_PAGE_MAX-1 )
    {
      debugFlgPage++;
    }
    else
    {
      debugFlgPage = 0;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    if( debugFlgPage > 0 )
    {
      debugFlgPage--;
    }
    else
    {
      debugFlgPage = DEBUG_FLG_PAGE_MAX-1;
    }
    DEBUGWIN_RefreshScreen();
  }  
}

static void DEBWIN_Update_Flg( void* userWork , DEBUGWIN_ITEM* item )
{
  const u32 idx = ((u32)userWork) + debugFlgPage*DEBUG_FLG_PAGE_NUM;
  DEBWIN_PageChangeFunc();
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    DEBUG_FLG_FlgFlip( idx );
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    DEBUG_FLG_FlgOn( idx );
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
  {
    DEBUG_FLG_FlgOff( idx );
    DEBUGWIN_RefreshScreen();
  }
}

static void DEBWIN_Draw_Flg( void* userWork , DEBUGWIN_ITEM* item )
{
  const u32 idx = ((u32)userWork) + debugFlgPage*DEBUG_FLG_PAGE_NUM;
  if( idx < DEBUG_FLG_MAX )
  {
    const BOOL flg = DEBUG_FLG_GetFlg( idx );
    DEBUGWIN_ITEM_SetNameV( item , "%3s[%s]", flg==TRUE?"ON":"OFF" ,dispName[idx] );
  }
  else
  {
    DEBUGWIN_ITEM_SetNameV( item , "---------------------" );
  }
}


