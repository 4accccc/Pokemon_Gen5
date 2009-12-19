//=============================================================================
/**
 *
 *	@file		intro_mcss.c
 *	@brief
 *	@author		hosaka genya
 *	@data		2009.12.18
 *
 */
//=============================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "intro_mcss.h"


//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  MCSS_ID_MAX = 8,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	メインワーク
//==============================================================
struct _INTRO_MCSS_WORK
{
  MCSS_SYS_WORK* mcss;
  MCSS_WORK* mcss_work[ MCSS_ID_MAX ];
};


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================



//=============================================================================
/**
 *								static関数
 */
//=============================================================================


//-----------------------------------------------------------------------------
/**
 *	@brief  MCSS生成
 *
 *	@param	HEAPID heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INTRO_MCSS_WORK* INTRO_MCSS_Create( HEAPID heap_id )
{
  INTRO_MCSS_WORK* wk;

  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof(INTRO_MCSS_WORK) );

  wk->mcss = MCSS_Init( MCSS_ID_MAX, heap_id );

  MCSS_SetTextureTransAdrs( wk->mcss, 0 );
  MCSS_SetOrthoMode( wk->mcss );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  MCSS終了
 *
 *	@param	INTRO_MCSS_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_Exit( INTRO_MCSS_WORK* wk )
{
  int i;

  for( i=0; i<MCSS_ID_MAX; i++ )
  {
    if( wk->mcss_work[i] )
    {
      MCSS_Del( wk->mcss, wk->mcss_work[i] );
    }
  }

  MCSS_Exit( wk->mcss );
  GFL_HEAP_FreeMemory( wk );
}


#define CHECK_KEY_CONT( key ) ( (GFL_UI_KEY_GetCont() & (key) ) == (key) )

static void debug_mcss_camera( MCSS_WORK* mcss_work )
{
  VecFx32 pos;
  static int num = 1;

  GF_ASSERT( mcss_work );
    
  MCSS_GetPosition( mcss_work, &pos );

  if( CHECK_KEY_CONT( PAD_BUTTON_X ) )
  {
    num++;
    HOSAKA_Printf("num=%d \n",num);
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_Y ) )
  {
    num--;
    HOSAKA_Printf("num=%d \n",num);
  }
  else if( CHECK_KEY_CONT( PAD_KEY_UP ) )
  {
    pos.y += num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_KEY_DOWN ) )
  {
    pos.y -= num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }    
  else if( CHECK_KEY_CONT( PAD_KEY_LEFT ) )
  {
    pos.x += num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_KEY_RIGHT ) )
  {
    pos.x -= num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_L ) )
  {
    pos.z += num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_R ) )
  {
    pos.z -= num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  
  MCSS_SetPosition( mcss_work, &pos );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  MCSS主処理
 *
 *	@param	INTRO_MCSS_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_Main( INTRO_MCSS_WORK* wk )
{
  // @TODO debug
#ifdef PM_DEBUG
  {
    static BOOL is_on = FALSE;

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    {
      is_on ^= 1;
    }

    if( is_on && wk->mcss_work[0] )
    {
      debug_mcss_camera( wk->mcss_work[0] );
    }
  }
#endif


  MCSS_Main( wk->mcss );
  MCSS_Draw( wk->mcss );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  MCSSワーク生成
 *
 *	@param	INTRO_MCSS_WORK* wk
 *	@param	px
 *	@param	py
 *	@param	pz
 *	@param	MCSS_ADD_WORK* add
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_Add( INTRO_MCSS_WORK* wk, fx32 px, fx32 py, fx32 pz, const MCSS_ADD_WORK* add, u8 id )
{
  VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};

  GF_ASSERT( wk );
  GF_ASSERT( wk->mcss );
  GF_ASSERT( id < MCSS_ID_MAX );
  GF_ASSERT( wk->mcss_work[id] == NULL );

  wk->mcss_work[id] = MCSS_Add( wk->mcss, px, py, pz, add );

  MCSS_SetScale( wk->mcss_work[id], &scale );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  MCSS指定ワークの表示非表示切替
 *
 *	@param	INTRO_MCSS_WORK* wk
 *	@param	is_visible
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_SetVisible( INTRO_MCSS_WORK* wk, BOOL is_visible, u8 id )
{
  GF_ASSERT( wk );
  GF_ASSERT( wk->mcss );
  GF_ASSERT( id < MCSS_ID_MAX );
  GF_ASSERT( wk->mcss_work[id] );

  if( is_visible )
  {
    MCSS_SetVanishFlag( wk->mcss_work[id] );
  }
  else
  {
    MCSS_ResetVanishFlag( wk->mcss_work[id] );
  }
}




