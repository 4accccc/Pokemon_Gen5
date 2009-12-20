//=============================================================================
/**
 *
 *	@file		intro_mcss.c
 *	@brief  イントロデモ MCSSラッパー
 *	@author	hosaka genya
 *	@data		2009.12.18
 *
 */
//=============================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// MCSS ポケモン
#include "system/mcss_tool.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

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
  HEAPID          heap_id;
  MCSS_SYS_WORK*  mcss;
  MCSS_WORK*      mcss_work[ MCSS_ID_MAX ];
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

  wk->mcss    = MCSS_Init( MCSS_ID_MAX, heap_id );
  wk->heap_id = heap_id;

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
 *	@brief  MCSSワーク生成（ポケモン)
 *
 *	@param	INTRO_MCSS_WORK* wk
 *	@param	px
 *	@param	py
 *	@param	pz
 *	@param	monsno
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_AddPoke( INTRO_MCSS_WORK* wk, fx32 px, fx32 py, fx32 pz, int monsno, u8 id )
{
  MCSS_ADD_WORK   add;
  POKEMON_PARAM*  pp;
  VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
  
  GF_ASSERT( wk );
  GF_ASSERT( wk->mcss );
  GF_ASSERT( id < MCSS_ID_MAX );
  GF_ASSERT( wk->mcss_work[id] == NULL );

  pp = PP_Create( MONSNO_HITOKAGE, 0, 0, wk->heap_id );
      
  MCSS_TOOL_MakeMAWPP( pp, &add, MCSS_DIR_FRONT );

  GFL_HEAP_FreeMemory( pp );

  wk->mcss_work[id] = MCSS_Add( wk->mcss, px, py, pz, &add );
  MCSS_SetScale( wk->mcss_work[id], &scale );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  MCSS指定ワークの表示/非表示切替
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

  if( is_visible == TRUE )
  {
    MCSS_ResetVanishFlag( wk->mcss_work[id] );
  }
  else
  {
    MCSS_SetVanishFlag( wk->mcss_work[id] );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	INTRO_MCSS_WORK* wk
 *	@param	id
 *	@param	anm_idx 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_SetAnimeIndex( INTRO_MCSS_WORK* wk, u8 id, int anm_idx )
{
  GF_ASSERT( wk );
  GF_ASSERT( wk->mcss );
  GF_ASSERT( id < MCSS_ID_MAX );
  GF_ASSERT( wk->mcss_work[id] );

  MCSS_SetAnimeIndex( wk->mcss_work[id], anm_idx );
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	INTRO_MCSS_WORK* wk
 *	@param	id
 *	@param	VecFx32* scale 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_SetScale( INTRO_MCSS_WORK* wk, u8 id, VecFx32* scale )
{ 
  GF_ASSERT( wk );
  GF_ASSERT( wk->mcss );
  GF_ASSERT( id < MCSS_ID_MAX );
  GF_ASSERT( wk->mcss_work[id] );

  MCSS_SetScale( wk->mcss_work[id], scale );
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	INTRO_MCSS_WORK* wk
 *	@param	id
 *	@param	alpha 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MCSS_SetAlpha( INTRO_MCSS_WORK* wk, u8 id, u8 alpha )
{
  GF_ASSERT( wk );
  GF_ASSERT( wk->mcss );
  GF_ASSERT( id < MCSS_ID_MAX );
  GF_ASSERT( wk->mcss_work[id] );

  MCSS_SetAlpha( wk->mcss_work[id], alpha );
}


