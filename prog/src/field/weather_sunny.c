//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_sunny.c
 *	@brief		天気　晴れ
 *	@author		tomoya takahashi
 *	@date		2009.03.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "weather_sunny.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フォグフェード開始オフセット
//=====================================
#define FOG_START_OFFSET	( 0x7FFF )
#define FOG_FADE_SYNC			(160)
#define FOG_FADEOUT_SYNC	(80)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ワーク
//=====================================
typedef struct {
	BOOL fade_init;
} SUNNY_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 


WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SUNNY = {
	//	グラフィック情報
	0,			// アークID
	FALSE,		// OAMを使用するか？
	WEATHER_TASK_3DBG_USE_NONE,		// BGを使用するか？
	0,			// OAM CG
	0,			// OAM PLTT
	0,			// OAM CELL
	0,			// OAM CELLANM
  {
    {
      0,		// BGTEX
      0,		// GXTexSizeS
      0,		// GXTexSizeT
      0,		// GXTexRepeat
      0,		// GXTexFlip
      0,		// GXTexFmt
      0,		// GXTexPlttColor0
    },
    {
      0,		// BGTEX
      0,		// GXTexSizeS
      0,		// GXTexSizeT
      0,		// GXTexRepeat
      0,		// GXTexFlip
      0,		// GXTexFmt
      0,		// GXTexPlttColor0
    },
  },

	// ワークサイズ
	sizeof(SUNNY_WORK),

	// 管理関数
	WEATHER_SUNNY_Init,		// 初期化
	WEATHER_SUNNY_FadeIn,		// フェードイン
	WEATHER_SUNNY_NoFade,		// フェードなし
	WEATHER_SUNNY_Main,		// メイン処理
	WEATHER_SUNNY_InitFadeOut,		// フェードアウト
	WEATHER_SUNNY_FadeOut,		// フェードアウト
	WEATHER_SUNNY_Exit,		// 破棄

	// オブジェ動作関数
	NULL,
};






//-----------------------------------------------------------------------------
/**
 *			晴れ
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	u32 fogoffset, fogslope;
	SUNNY_WORK* p_sunnywork;

	// ローカルワーク取得
	p_sunnywork = WEATHER_TASK_GetWorkData( p_wk );
	p_sunnywork->fade_init = FALSE;

	// フォグの設定
	if( WEATHER_TASK_IsZoneFog( p_wk ) )
	{
		fogoffset = WEATHER_TASK_GetZoneFogOffset( p_wk );
		fogslope	= WEATHER_TASK_GetZoneFogSlope( p_wk );
		WEATHER_TASK_FogSet( p_wk, fogslope, FOG_START_OFFSET, fog_cont );
	}
	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	BOOL result;
	SUNNY_WORK* p_sunnywork;

	// ローカルワーク取得
	p_sunnywork = WEATHER_TASK_GetWorkData( p_wk );
	if( p_sunnywork->fade_init == FALSE )
	{
		u32 fogoffset, fogslope;
		if( WEATHER_TASK_IsZoneFog( p_wk ) )
		{
			fogoffset = WEATHER_TASK_GetZoneFogOffset( p_wk );
			fogslope	= WEATHER_TASK_GetZoneFogSlope( p_wk );

			WEATHER_TASK_FogFadeIn_Init( p_wk,
			fogslope, 
			fogoffset, 
			FOG_FADE_SYNC,
			fog_cont );
		}

    // ライト変更
    if( WEATHER_TASK_IsZoneLight( p_wk ) )
    {
      s32 light;
      light = WEATHER_TASK_GetZoneLight( p_wk );
      WEATHER_TASK_LIGHT_Change( p_wk, FIELD_ZONEFOGLIGHT_ARC_LIGHT, light );
    }

		p_sunnywork->fade_init = TRUE;
	}

	result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
	if( result ){		// フェードリザルトが完了ならばメインへ
		// シーケンス変更
		return WEATHER_TASK_FUNC_RESULT_FINISH;
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	u32 fogoffset, fogslope;

	if( WEATHER_TASK_IsZoneFog( p_wk ) )
	{
		fogoffset = WEATHER_TASK_GetZoneFogOffset( p_wk );
		fogslope	= WEATHER_TASK_GetZoneFogSlope( p_wk );
		// フォグの設定
		WEATHER_TASK_FogSet( p_wk, fogslope, fogoffset, fog_cont );
	}

  // ライト変更
  if( WEATHER_TASK_IsZoneLight( p_wk ) )
  {
    s32 light;
    light = WEATHER_TASK_GetZoneLight( p_wk );
    WEATHER_TASK_LIGHT_Set( p_wk, FIELD_ZONEFOGLIGHT_ARC_LIGHT, light );
  }

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	
	if( WEATHER_TASK_IsZoneFog( p_wk ) )
	{
		WEATHER_TASK_FogFadeOut_Init( p_wk,
				FOG_START_OFFSET, 
				FOG_FADEOUT_SYNC, fog_cont );
	}

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	BOOL result;

	result = WEATHER_TASK_FogFade_IsFade( p_wk );
	if( result ){
		return WEATHER_TASK_FUNC_RESULT_FINISH;
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SUNNY_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	if( WEATHER_TASK_IsZoneFog( p_wk ) )
	{
		// FOG終了
		WEATHER_TASK_FogClear( p_wk, fog_cont );
	}

	if( WEATHER_TASK_IsZoneLight( p_wk ) )
	{
		// ライト元に
		WEATHER_TASK_LIGHT_Back( p_wk );
	}

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}
