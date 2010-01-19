//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_mirage.c
 *	@brief		蜃気楼
 *	@author		tomoya takahshi
 *	@date		2009.04.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"
#include "calctool.h"

#include "arc/arc_def.h"
#include "arc/field_weather.naix"

#include "system/laster.h"

#include "weather_mirage.h"

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
///	蜃気楼BG面
//=====================================
#define WEATRHER_MIRAGE_BG_FRAME	( GFL_BG_FRAME2_M )
#define WEATRHER_MIRAGE_BG_PLTT		( 0x5 )


//-------------------------------------
///	FOG
//=====================================
#define	WEATHER_MIRAGE_FOG_TIMING		(300)							// に１回フォグテーブルを操作
#define	WEATHER_MIRAGE_FOG_TIMING_END	(100)							// に１回フォグテーブルを操作
#define WEATHER_MIRAGE_FOG_START		(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_MIRAGE_FOG_START_END	(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_MIRAGE_FOG_OFS			(0xf00)
#define WEATHER_MIRAGE_FOG_OFS_START	(0x1000)

#define WEATHER_MIRAGE_FOG_ALPHA		( 0 )
#define WEATHER_MIRAGE_FOG_SLOPE		( 10 )

//-------------------------------------
///	蜃気楼ラスター
//=====================================
#define WEATHER_MIRAGE_LASTER_ADDR	( GFL_CALC_GET_ROTA_NUM(8) )
#define WEATHER_MIRAGE_LASTER_R_W	( 1.5*FX32_ONE )
#define WEATHER_MIRAGE_LASTER_SCRL	( 1 )
#define WEATHER_MIRAGE_LASTER_BG	( LASTER_SCROLL_MBG2 )
#define WEATHER_MIRAGE_LASTER_INIT	( 0 )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	蜃気楼ワーク
//=====================================
typedef struct {
	BOOL fog_init;

	GFL_TCB*			p_vblank;
	GFL_TCB*			p_hblank;

	s16 scroll[192];
	s16 scroll_index;
	
} WEATHER_MIRAGE_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
/// 蜃気楼	
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 

static void WEATHER_MIRAGE_VBlank( GFL_TCB* p_tcb, void* p_work );
static void WEATHER_MIRAGE_HBlank( GFL_TCB* p_tcb, void* p_work );





//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 蜃気楼
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_MIRAGE = {
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
	sizeof(WEATHER_MIRAGE_WORK),

	// 管理関数
	WEATHER_MIRAGE_Init,		// 初期化
	WEATHER_MIRAGE_FadeIn,		// フェードイン
	WEATHER_MIRAGE_NoFade,		// フェードなし
	WEATHER_MIRAGE_Main,		// メイン処理
	WEATHER_MIRAGE_InitFadeOut,	// フェードアウト
	WEATHER_MIRAGE_FadeOut,		// フェードアウト
	WEATHER_MIRAGE_Exit,		// 破棄

	// オブジェ動作関数
	NULL,
};







//-----------------------------------------------------------------------------
/**
 *			プライベート関数郡
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIRAGE_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// BG面の生成
	{
		static const GFL_BG_BGCNT_HEADER header_main2 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x08000,0x8000,
			GX_BG_EXTPLTT_01, 3, 1, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};

		GFL_BG_FreeBGControl( WEATRHER_MIRAGE_BG_FRAME );
		GFL_BG_SetBGControl( WEATRHER_MIRAGE_BG_FRAME, &header_main2, GFL_BG_MODE_TEXT );
		GFL_BG_ClearFrame( WEATRHER_MIRAGE_BG_FRAME );
		GFL_BG_SetVisible( WEATRHER_MIRAGE_BG_FRAME, VISIBLE_ON );
	}

	// グラフィック展開
	{
		ARCHANDLE* p_handle;

		p_handle = GFL_ARC_OpenDataHandle( ARCID_FIELD_WEATHER, heapID );
		
		// パレット情報を転送
		GFL_ARCHDL_UTIL_TransVramPalette(
			p_handle, NARC_field_weather_mirage_NCLR,
			PALTYPE_MAIN_BG, WEATRHER_MIRAGE_BG_PLTT*32, 32, heapID );

		// キャラクタ情報転送
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_field_weather_mirage_NCGR, WEATRHER_MIRAGE_BG_FRAME, 0, 0x4000, FALSE, heapID );

		// スクリーン情報転送
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_field_weather_mirage_NSCR, WEATRHER_MIRAGE_BG_FRAME, 0, 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}
	
	// ラスターシステム生成
	{
		LASTER_ScrollMakeSinTbl( p_local_wk->scroll, 192, WEATHER_MIRAGE_LASTER_ADDR, WEATHER_MIRAGE_LASTER_R_W );
		
		p_local_wk->p_hblank = GFUser_HIntr_CreateTCB( WEATHER_MIRAGE_HBlank, p_local_wk, 0 );
		p_local_wk->p_vblank = GFUser_VIntr_CreateTCB( WEATHER_MIRAGE_VBlank, p_local_wk, 10 );
	}

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_MIRAGE_FOG_OFS_START, fog_cont );

	//
	WEATHER_TASK_Fog_SetBlendMode( p_wk, FIELD_FOG_BLEND_ALPHA );
	WEATHER_TASK_Fog_SetAlpha( p_wk, WEATHER_MIRAGE_FOG_ALPHA );

	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0, GX_BLEND_PLANEMASK_BG2, 0, 31 );

	p_local_wk->fog_init = FALSE;

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼　フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIRAGE_WORK* p_local_wk;
	BOOL result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	if( p_local_wk->fog_init == FALSE ){
		WEATHER_TASK_FogFadeIn_Init( p_wk,
				WEATHER_FOG_SLOPE_DEFAULT, 
				WEATHER_FOG_DEPTH_DEFAULT + WEATHER_MIRAGE_FOG_OFS, 
				WEATHER_MIRAGE_FOG_TIMING, fog_cont );
		p_local_wk->fog_init = TRUE;
	}

	result = WEATHER_TASK_FogFade_IsFade( p_wk );

	// タイミングが最小になったらメインへ
	if( result ){		// フェードリザルトが完了ならばメインへ
		return WEATHER_TASK_FUNC_RESULT_FINISH;
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼　フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_MIRAGE_FOG_OFS, fog_cont );


	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼　メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIRAGE_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼　フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIRAGE_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	WEATHER_TASK_FogFadeOut_Init( p_wk,
			WEATHER_FOG_DEPTH_DEFAULT + WEATHER_MIRAGE_FOG_OFS_START, 
			WEATHER_MIRAGE_FOG_TIMING_END, fog_cont );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼　フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIRAGE_WORK* p_local_wk;
	BOOL result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	result = WEATHER_TASK_FogFade_IsFade( p_wk );

	// タイミングが最小になったらメインへ
	if( result ){		// フェードリザルトが完了ならばメインへ
		return WEATHER_TASK_FUNC_RESULT_FINISH;
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief		蜃気楼　破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIRAGE_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_MIRAGE_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// ラスターシステム破棄
	{
		GFL_TCB_DeleteTask( p_local_wk->p_hblank );
		GFL_TCB_DeleteTask( p_local_wk->p_vblank );
	}

	GFL_BG_FreeBGControl( WEATRHER_MIRAGE_BG_FRAME );

	GFL_BG_SetVisible( WEATRHER_MIRAGE_BG_FRAME, VISIBLE_OFF );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}


//----------------------------------------------------------------------------
/**
 *	@brief	蜃気楼　VBLANK
 */
//-----------------------------------------------------------------------------
static void WEATHER_MIRAGE_VBlank( GFL_TCB* p_tcb, void* p_work )
{
	WEATHER_MIRAGE_WORK* p_wk;
	p_wk = p_work;

	p_wk->scroll_index += WEATHER_MIRAGE_LASTER_SCRL;
	if( p_wk->scroll_index < 0 ){
		p_wk->scroll_index += 192;
	}else{
		p_wk->scroll_index %= 192;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	蜃気楼　HBLANK
 */
//-----------------------------------------------------------------------------
static void WEATHER_MIRAGE_HBlank( GFL_TCB* p_tcb, void* p_work )
{
	WEATHER_MIRAGE_WORK* p_wk;
	int v_c;
	p_wk = p_work;

	v_c = GX_GetVCount();

	v_c += (p_wk->scroll_index + 1);
	v_c %= 192;

	if( GX_IsHBlank() ){
		G2_SetBG2Offset( p_wk->scroll[v_c], 0 );
	}
}


