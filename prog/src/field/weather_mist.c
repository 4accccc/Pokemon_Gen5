//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_mist.c
 *	@brief  天気：霧
 *	@author	tomoya takahashi
 *	@date		2010.02.11
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc/arc_def.h"
#include "arc/field_weather.naix"
#include "arc/field_weather_light.naix"


#include "weather_mist.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

/*== 霧 ==*/
#define	WEATHER_MIST_FOG_TIMING		(90)							// に１回フォグテーブルを操作
#define	WEATHER_MIST_FOG_TIMING_END	(60)							// に１回フォグテーブルを操作
#define WEATHER_MIST_FOG_OFS			(-8)



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	吹雪ワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_MIST_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	霧
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_MIST_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_MIST_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 


//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 霧
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_MIST = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
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
	sizeof(WEATHER_MIST_WORK),

	// 管理関数
	WEATHER_MIST_Init,		// 初期化
	WEATHER_MIST_FadeIn,		// フェードイン
	WEATHER_MIST_NoFade,		// フェードなし
	WEATHER_MIST_Main,		// メイン処理
	WEATHER_MIST_InitFadeOut,	// フェードアウト
	WEATHER_MIST_FadeOut,		// フェードアウト
	WEATHER_MIST_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_MIST_OBJ_Move,
};



//----------------------------------------------------------------------------
/**
 *	@brief  霧  初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = 0;	// 同じくフォグ用

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	BOOL fog_result;
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
  if( p_local_wk->work[0] == 0 ){

    WEATHER_TASK_FogFadeIn_Init( p_wk,
        WEATHER_FOG_SLOPE_DEFAULT, 
        WEATHER_FOG_DEPTH_DEFAULT + WEATHER_MIST_FOG_OFS, 
        WEATHER_MIST_FOG_TIMING, fog_cont );

      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( WEATHER_NO_MIST ) );

    p_local_wk->work[0]--;
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result ){		// フェードリザルトが完了ならばメインへ
			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_MIST_FOG_OFS, fog_cont );

  // ライト変更
  WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( WEATHER_NO_MIST ) );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// fog
	p_local_wk->work[0] = 0;	// 同じくフォグ用

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIST_WORK* p_local_wk;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// フォグ操作
  if( p_local_wk->work[0] == 0 ){
    p_local_wk->work[0] --;

    WEATHER_TASK_FogFadeOut_Init( p_wk,
        WEATHER_FOG_DEPTH_DEFAULT_START, 
        WEATHER_MIST_FOG_TIMING_END, fog_cont );
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result ){
			
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );

	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_MIST_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
}


