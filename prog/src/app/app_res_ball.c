//======================================================================
/**
 * @file	app_res_ball.c
 * @brief	共通素材関係ソース
 * @author	genya_hosaka
 * @data	09/10/07
 *
 * モジュール名：APP_RES_BALL
 */
//======================================================================


#include <gflib.h>

#include "item/item.h"
#include "app/app_res_ball.h"

#include "arc_def.h"
#include "app_ball.naix"



//=============================================================================
/**
 *	ボールアイコン
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  ARCIDの取得
 *
 *	@param	void 
 *
 *	@retval ARCID アーカイブインデックス
 */
//-----------------------------------------------------------------------------
const u32 APP_RES_BALL_GetArcId( void )
{
  return ARCID_APP_BALL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *
 *	@return	パレットアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_RES_BALL_GetBallPltArcIdx( const BALL_ID ball )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_ball_ball00_NCLR + ball - 1;	//BALLIDは1オリジン
}
//----------------------------------------------------------------------------
/**
 *	@brief	キャラリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *
 *	@return	キャラアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_RES_BALL_GetBallCharArcIdx( const BALL_ID ball )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_ball_ball00_NCGR + ball - 1;	//BALLIDは１オリジン
}
//----------------------------------------------------------------------------
/**
 *	@brief	セルリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *	@param	APP_RES_MAPPING mapping	マッピングモード
 *
 *	@return	セルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_RES_BALL_GetBallCellArcIdx( const BALL_ID ball, const APP_RES_MAPPING mapping )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_ball_ball_32k_NCER + mapping;
}
//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *	@param	APP_RES_MAPPING mapping	マッピングモード
 *
 *	@return	パレットアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_RES_BALL_GetBallAnimeArcIdx( const BALL_ID ball, const APP_RES_MAPPING mapping )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_ball_ball_32k_NANR + mapping;
}

