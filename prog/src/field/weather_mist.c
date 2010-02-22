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

#if 0 // 通常FOGでも、いろんなカメラアングルに対応する必要が出たら。
static const u8 sc_MIST_FOG_SLOPE[] = {
  WEATHER_FOG_SLOPE_DEFAULT,    // 通常
  WEATHER_FOG_SLOPE_DEFAULT,    // H01
  WEATHER_FOG_SLOPE_DEFAULT,    // C03
  WEATHER_FOG_SLOPE_DEFAULT,    // ポケセン
  WEATHER_FOG_SLOPE_DEFAULT,    // H01ゲート前
  WEATHER_FOG_SLOPE_DEFAULT,    // ポケモンリーグ全景
  WEATHER_FOG_SLOPE_DEFAULT,    // C03路地（裏路地）
  WEATHER_FOG_SLOPE_DEFAULT,    // C03中央広場
  WEATHER_FOG_SLOPE_DEFAULT,    // C03路地（ジム）
  WEATHER_FOG_SLOPE_DEFAULT,    // H02
  WEATHER_FOG_SLOPE_DEFAULT,    // H03
  WEATHER_FOG_SLOPE_DEFAULT,    // チャンピオンロード
  WEATHER_FOG_SLOPE_DEFAULT,    // C05地面ジム
  WEATHER_FOG_SLOPE_DEFAULT,    // C03路地（真ん中）
  WEATHER_FOG_SLOPE_DEFAULT,    // C03路地（ポケセン）
  WEATHER_FOG_SLOPE_DEFAULT,    // C03路地（アトリエ）
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（ゴースト）
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（あく）
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（格闘）
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（エスパー）
  WEATHER_FOG_SLOPE_DEFAULT,    // ポケリーグ内部（リフト上）
  WEATHER_FOG_SLOPE_DEFAULT,    // ポケリーグ内部（リフト下）
  WEATHER_FOG_SLOPE_DEFAULT,    // WFBC高さ１
  WEATHER_FOG_SLOPE_DEFAULT,    // WFBC高さ2
  WEATHER_FOG_SLOPE_DEFAULT,    // WFBC高さ3
  WEATHER_FOG_SLOPE_DEFAULT,    // C03ベイサイド
  WEATHER_FOG_SLOPE_DEFAULT,    // H05
  WEATHER_FOG_SLOPE_DEFAULT,    // C07氷ジム
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（ゴースト）頂上
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（あく）頂上
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（格闘）頂上
  WEATHER_FOG_SLOPE_DEFAULT,    // 四天王部屋（エスパー）頂上
  WEATHER_FOG_SLOPE_DEFAULT,    // C08ドラゴンジム
  WEATHER_FOG_SLOPE_DEFAULT,    // タワーオブヘブン１F～４F
};

static const u32 sc_MIST_FOG_OFFS[] = {
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 通常
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // H01
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // ポケセン
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // H01ゲート前
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // ポケモンリーグ全景
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03路地（裏路地）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03中央広場
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03路地（ジム）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // H02
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // H03
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // チャンピオンロード
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C05地面ジム
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03路地（真ん中）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03路地（ポケセン）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03路地（アトリエ）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（ゴースト）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（あく）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（格闘）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（エスパー）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // ポケリーグ内部（リフト上）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // ポケリーグ内部（リフト下）
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // WFBC高さ１
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // WFBC高さ2
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // WFBC高さ3
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C03ベイサイド
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // H05
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C07氷ジム
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（ゴースト）頂上
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（あく）頂上
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（格闘）頂上
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // 四天王部屋（エスパー）頂上
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // C08ドラゴンジム
  WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS,    // タワーオブヘブン１F～４F
};
#endif


/*== 霧 ==*/
#define	WEATHER_PALACE_MIST_FOG_TIMING		(90)							// に１回フォグテーブルを操作
#define	WEATHER_PALACE_MIST_FOG_TIMING_END	(60)							// に１回フォグテーブルを操作
#define WEATHER_PALACE_MIST_FOG_OFS			(32546)
#define WEATHER_PALACE_MIST_FOG_SLOPE		(8)

static const u8 sc_PALACE_MIST_FOG_SLOPE[] = {
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 通常
  WEATHER_PALACE_MIST_FOG_SLOPE,    // H01
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03
  WEATHER_PALACE_MIST_FOG_SLOPE,    // ポケセン
  WEATHER_PALACE_MIST_FOG_SLOPE,    // H01ゲート前
  WEATHER_PALACE_MIST_FOG_SLOPE,    // ポケモンリーグ全景
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03路地（裏路地）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03中央広場
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03路地（ジム）
  7,    // H02
  3,    // H03
  WEATHER_PALACE_MIST_FOG_SLOPE,    // チャンピオンロード
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C05地面ジム
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03路地（真ん中）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03路地（ポケセン）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03路地（アトリエ）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（ゴースト）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（あく）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（格闘）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（エスパー）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // ポケリーグ内部（リフト上）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // ポケリーグ内部（リフト下）
  WEATHER_PALACE_MIST_FOG_SLOPE,    // WFBC高さ１
  WEATHER_PALACE_MIST_FOG_SLOPE,    // WFBC高さ2
  3,    // WFBC高さ3
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C03ベイサイド
  8,    // H05
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C07氷ジム
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（ゴースト）頂上
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（あく）頂上
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（格闘）頂上
  WEATHER_PALACE_MIST_FOG_SLOPE,    // 四天王部屋（エスパー）頂上
  WEATHER_PALACE_MIST_FOG_SLOPE,    // C08ドラゴンジム
  WEATHER_PALACE_MIST_FOG_SLOPE,    // タワーオブヘブン１F～４F
};

static const u32 sc_PALACE_MIST_FOG_OFFS[] = {
  WEATHER_PALACE_MIST_FOG_OFS,    // 通常
  WEATHER_PALACE_MIST_FOG_OFS,    // H01
  WEATHER_PALACE_MIST_FOG_OFS,    // C03
  WEATHER_PALACE_MIST_FOG_OFS,    // ポケセン
  WEATHER_PALACE_MIST_FOG_OFS,    // H01ゲート前
  WEATHER_PALACE_MIST_FOG_OFS,    // ポケモンリーグ全景
  WEATHER_PALACE_MIST_FOG_OFS,    // C03路地（裏路地）
  WEATHER_PALACE_MIST_FOG_OFS,    // C03中央広場
  WEATHER_PALACE_MIST_FOG_OFS,    // C03路地（ジム）
  32460,    // H02
  0,    // H03
  WEATHER_PALACE_MIST_FOG_OFS,    // チャンピオンロード
  WEATHER_PALACE_MIST_FOG_OFS,    // C05地面ジム
  WEATHER_PALACE_MIST_FOG_OFS,    // C03路地（真ん中）
  WEATHER_PALACE_MIST_FOG_OFS,    // C03路地（ポケセン）
  WEATHER_PALACE_MIST_FOG_OFS,    // C03路地（アトリエ）
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（ゴースト）
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（あく）
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（格闘）
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（エスパー）
  WEATHER_PALACE_MIST_FOG_OFS,    // ポケリーグ内部（リフト上）
  WEATHER_PALACE_MIST_FOG_OFS,    // ポケリーグ内部（リフト下）
  WEATHER_PALACE_MIST_FOG_OFS,    // WFBC高さ１
  WEATHER_PALACE_MIST_FOG_OFS,    // WFBC高さ2
  0,    // WFBC高さ3
  WEATHER_PALACE_MIST_FOG_OFS,    // C03ベイサイド
  32591,    // H05
  WEATHER_PALACE_MIST_FOG_OFS,    // C07氷ジム
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（ゴースト）頂上
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（あく）頂上
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（格闘）頂上
  WEATHER_PALACE_MIST_FOG_OFS,    // 四天王部屋（エスパー）頂上
  WEATHER_PALACE_MIST_FOG_OFS,    // C08ドラゴンジム
  WEATHER_PALACE_MIST_FOG_OFS,    // タワーオブヘブン１F～４F
};



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

  u32 slope;
  u32 offs;
} WEATHER_MIST_WORK;

//-------------------------------------
///	吹雪ワーク
//=====================================
typedef struct {
  s32 weather_no;
	s32 work[10];

  u32 slope;
  u32 offs;
} WEATHER_PALACE_MIST_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	霧
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static void WEATHER_MIST_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_MIST_OBJ_Add( WEATHER_TASK* p_wk, int num, HEAPID heapID ); 



//-------------------------------------
///	パレス霧
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_WHITE_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_BLACK_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID ); 
static void WEATHER_PALACE_MIST_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_PALACE_MIST_OBJ_Add( WEATHER_TASK* p_wk, int num, HEAPID heapID ); 


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

// パレス霧
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_PALACE_WHITE_MIST = {
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
	sizeof(WEATHER_PALACE_MIST_WORK),

	// 管理関数
	WEATHER_PALACE_WHITE_MIST_Init,		// 初期化
	WEATHER_PALACE_MIST_FadeIn,		// フェードイン
	WEATHER_PALACE_MIST_NoFade,		// フェードなし
	WEATHER_PALACE_MIST_Main,		// メイン処理
	WEATHER_PALACE_MIST_InitFadeOut,	// フェードアウト
	WEATHER_PALACE_MIST_FadeOut,		// フェードアウト
	WEATHER_PALACE_MIST_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_PALACE_MIST_OBJ_Move,
};

WEATHER_TASK_DATA c_WEATHER_TASK_DATA_PALACE_BLACK_MIST = {
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
	sizeof(WEATHER_PALACE_MIST_WORK),

	// 管理関数
	WEATHER_PALACE_BLACK_MIST_Init,		// 初期化
	WEATHER_PALACE_MIST_FadeIn,		// フェードイン
	WEATHER_PALACE_MIST_NoFade,		// フェードなし
	WEATHER_PALACE_MIST_Main,		// メイン処理
	WEATHER_PALACE_MIST_InitFadeOut,	// フェードアウト
	WEATHER_PALACE_MIST_FadeOut,		// フェードアウト
	WEATHER_PALACE_MIST_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_PALACE_MIST_OBJ_Move,
};





//----------------------------------------------------------------------------
/**
 *	@brief  霧  初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = 0;	// 同じくフォグ用

  p_local_wk->slope = WEATHER_FOG_SLOPE_DEFAULT;
  p_local_wk->offs = WEATHER_FOG_DEPTH_DEFAULT_START + WEATHER_MIST_FOG_OFS;

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	BOOL fog_result;
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
  if( p_local_wk->work[0] == 0 ){

    WEATHER_TASK_FogFadeIn_Init( p_wk,
        p_local_wk->slope, 
        p_local_wk->offs, 
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
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, p_local_wk->slope, p_local_wk->offs, fog_cont );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( WEATHER_NO_MIST ) );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
  return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
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
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
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
static WEATHER_TASK_FUNC_RESULT WEATHER_MIST_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
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








//----------------------------------------------------------------------------
/**
 *	@brief  霧  初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_WHITE_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_PALACE_MIST_WORK* p_local_wk;
  u32 camera_type = WEATHER_TASK_CAMERA_GetType( p_wk );

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = 0;	// 同じくフォグ用

  p_local_wk->weather_no = WEATHER_NO_PALACE_WHITE_MIST;


  GF_ASSERT( camera_type < NELEMS(sc_PALACE_MIST_FOG_SLOPE) );
  GF_ASSERT( camera_type < NELEMS(sc_PALACE_MIST_FOG_OFFS) );

  // slope と offsを求める
  p_local_wk->slope = sc_PALACE_MIST_FOG_SLOPE[ camera_type ];
  p_local_wk->offs  = sc_PALACE_MIST_FOG_OFFS[ camera_type ];

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_BLACK_MIST_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_PALACE_MIST_WORK* p_local_wk;
  u32 camera_type = WEATHER_TASK_CAMERA_GetType( p_wk );

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = 0;	// 同じくフォグ用

  p_local_wk->weather_no = WEATHER_NO_PALACE_BLACK_MIST;

  GF_ASSERT( camera_type < NELEMS(sc_PALACE_MIST_FOG_SLOPE) );
  GF_ASSERT( camera_type < NELEMS(sc_PALACE_MIST_FOG_OFFS) );

  // slope と offsを求める
  p_local_wk->slope = sc_PALACE_MIST_FOG_SLOPE[ camera_type ];
  p_local_wk->offs  = sc_PALACE_MIST_FOG_OFFS[ camera_type ];

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	BOOL fog_result;
	WEATHER_PALACE_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
  if( p_local_wk->work[0] == 0 ){

    WEATHER_TASK_FogFadeIn_Init( p_wk,
        p_local_wk->slope, 
        p_local_wk->offs, 
        WEATHER_PALACE_MIST_FOG_TIMING, fog_cont );

      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( p_local_wk->weather_no ) );

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
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_PALACE_MIST_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, p_local_wk->slope, p_local_wk->offs, fog_cont );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( p_local_wk->weather_no ) );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
  return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧  フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_PALACE_MIST_WORK* p_local_wk;

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
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
{
	WEATHER_PALACE_MIST_WORK* p_local_wk;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// フォグ操作
  if( p_local_wk->work[0] == 0 ){
    p_local_wk->work[0] --;

    WEATHER_TASK_FogFadeOut_Init( p_wk,
        WEATHER_FOG_DEPTH_DEFAULT_START, 
        WEATHER_PALACE_MIST_FOG_TIMING_END, fog_cont );
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
static WEATHER_TASK_FUNC_RESULT WEATHER_PALACE_MIST_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, HEAPID heapID )
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
static void WEATHER_PALACE_MIST_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
}


