//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_snow.c
 *	@brief		天気	雪
 *	@author		tomoya takahashi
 *	@date		2009.03.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc/arc_def.h"
#include "arc/field_weather.naix"
#include "arc/field_weather_light.naix"


#include "weather_snow.h"

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
//	雪管理関数定数
//=====================================
#define	WEATHER_SNOW_TIMING_MIN		(10)				// 雪を出すタイミング最小
#define WEATHER_SNOW_TIMING_MAX		(14)				// 雪を出すタイミング最大
#define WEATHER_SNOW_TIMING_ADD		(5)					// タイミングを減らす数
#define	WEATHER_SNOW_ADD_TIMING		(1)					// １度のオブジェ登録数を増やすタイミング
#define	WEATHER_SNOW_ADD			(0)					// オブジェ登録数を足す値
#define WEATHER_SNOW_MAIN			(1)					// メインではこれだけ登録

/*== フェード無し開始の時 ==*/
#define WEATHER_SNOW_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_SNOW_NOFADE_OBJ_START_DUST_NUM	( 2 )			// 何個ずつずらすか
#define	WEATHER_SNOW_NOFADE_OBJ_START_DUST_MOVE	( 24 )			// ずらして動作させる値

/*== フォグ ==*/
#define	WEATHER_SNOW_FOG_TIMING		(200)							// に１回フォグテーブルを操作
#define	WEATHER_SNOW_FOG_TIMING_END	(150)							// に１回フォグテーブルを操作
#define WEATHER_SNOW_FOG_START		(8)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SNOW_FOG_START_END	(16)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SNOW_FOG_OFS		(0x300)

/*== 雪オブジェクト ==*/
#define	WEATHER_SNOW_ADD_TMG_X_BASE	(2)					// 雪のスピードを足すタイミング
#define	WEATHER_SNOW_ADD_TMG_X_RAN	(2)					// 雪のスピードを足すタイミング乱数値
#define	WEATHER_SNOW_ADD_TMG_Y_BASE	(2)					// 雪のスピードを足すタイミング
#define	WEATHER_SNOW_ADD_TMG_Y_RAN	(1)					// 雪のスピードを足すタイミング乱数値

#define	WEATHER_SSNOWTART_X_BASE	(-32)						// ベースになるX開始座標
#define	WEATHER_SSNOWTART_X_MAX	(414)						// X開始座標乱数の最大値
#define WEATHER_SNOW_OBJ_NUM		(4)					// オブジェ種類

#define WEATHER_SNOW_OBJ_AUTODEST_TIMING_MAX ( 30 )	// 自滅タイム　最大
#define WEATHER_SNOW_OBJ_AUTODEST_TIMING_MIN ( 2 )	// 自滅タイム　最小

/*== 雪グラフィック構成データ ==*/
#define WEATHER_SNOW_GRAPHIC_CELL	( 3 )
#define WEATHER_SNOW_GRAPHIC_BG		( WEATHER_GRAPHIC_NONE )



//-------------------------------------
//	吹雪管理関数定数
//=====================================
#define	WEATHER_SSNOW_TIMING_MIN		(1)							// 雨を出すタイミング最小
#define WEATHER_SSNOW_TIMING_MAX		(6)						// 雨を出すタイミング最大
#define WEATHER_SSNOW_TIMING_ADD		(3)							// タイミングを減らす数
#define WEATHER_SSNOW_ADD_START			(1)							// 最初の同時に雨を登録する数
#define WEATHER_SSNOW_ADD_TIMING		(2)							// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_SSNOW_ADD				(1)							// 登録する数を増やす数
#define WEATHER_SSNOW_ADD_END			(-1)							// 登録する数を増やす数
#define	WEATHER_SSNOW_ADD_MAIN			(1)							// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_SSNOW_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_SSNOW_NOFADE_OBJ_START_DUST_NUM		( 2 )			// 何個ずつずらすか
#define	WEATHER_SSNOW_NOFADE_OBJ_START_DUST_MOVE	( 3 )			// ずらして動作させる値

/*== フォグ ==*/
#define	WEATHER_SSNOW_FOG_TIMING		(400)							// に１回フォグテーブルを操作
#define	WEATHER_SSNOW_FOG_TIMING_END	(300)							// に１回フォグテーブルを操作
#define WEATHER_SSNOW_FOG_START			(8)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SSNOW_FOG_START_END		(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SSNOW_FOG_OFS			(0x100)

/*== 雪オブジェクト ==*/
#define WEATHER_SSNOW_SPEED_X		(-1)						// 横に進むスピード
#define	WEATHER_SSNOW_END_MIN		(2)							// 終了カウンタ最小
#define WEATHER_SSNOW_END_MAX		(23)						// 終了カウンタ最大
#define WEATHER_SSNOW_END_NUM		(WEATHER_SSNOW_END_MAX - WEATHER_SSNOW_END_MIN)	// カウンタの値の数
#define	WEATHER_SSNOW_END_DIV		((WEATHER_SSNOW_END_NUM / 3)+1)	// オブジェクトの種類を計算するとき割る値
#define	WEATHER_SSNOW_START_X_BASE	(-20)						// ベースになるX開始座標
#define	WEATHER_SSNOW_MUL_X			(20)						// ベースに雪の種類分足す値
#define	WEATHER_SSNOW_START_X_MAX	(420)						// X開始座標乱数の最大値
#define	WEATHER_SSNOW_START_Y		(-8)						// Y開始座標

#define	WEATHER_SSNOW_ADD_SP_TMG_NUM (4)						// スピードを変更するタイミングの数
#define	WEATHER_SSNOW_ADD_SP_TMG_TMG (100)						// スピードを変更するタイミングのタイミング

/*== 吹雪グラフィック構成データ ==*/
#define WEATHER_SSNOW_GRAPHIC_CELL	( 1 )
#define WEATHER_SSNOW_GRAPHIC_BG	( WEATHER_GRAPHIC_NONE )

//-----------------------
// 北海道雪
//=======================
#define	WEATHER_SNOW_S_TIMING_MIN		(2)							// 雪を出すタイミング最小
#define WEATHER_SNOW_S_TIMING_MAX		(3)						// 雪を出すタイミング最大
#define WEATHER_SNOW_S_TIMING_ADD		(2)							// タイミングを減らす数
#define WEATHER_SNOW_S_ADD_START		(1)							// 最初の同時に雪を登録する数
#define WEATHER_SNOW_S_ADD_TIMING		(2)							// 雪のタイミングをこれ回変更したら１回増やす
#define WEATHER_SNOW_S_ADD				(1)							// 登録する数を増やす数
#define WEATHER_SNOW_S_ADD_END			(-2)						// 登録する数を増やす数
#define	WEATHER_SNOW_S_ADD_MAIN			(1)						// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_SNOW_S_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_SNOW_S_NOFADE_OBJ_START_DUST_NUM	( 2 )		// １ずつずらして動作させる
#define	WEATHER_SNOW_S_NOFADE_OBJ_START_DUST_MOVE	( 2 )		// １ずつずらして動作させる

/*== フォグ ==*/
#define	WEATHER_SNOW_S_FOG_TIMING		(200)							// に１回フォグテーブルを操作
#define	WEATHER_SNOW_S_FOG_TIMING_END	(150)							// に１回フォグテーブルを操作
#define WEATHER_SNOW_S_FOG_START		(8)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SNOW_S_FOG_START_END	(10)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_SNOW_S_FOG_OFS			(0x100)

/*== BG ==*/
#define WEATHER_SNOW_S_BG_ALP_TIM_S	(6)		// アルファを１上げるタイミング
#define WEATHER_SNOW_S_BG_ALP_TIM_E	(8)		// アルファを１下げるタイミング

// オブジェ 
#define	WEATHER_SNOW_S_END_MIN		(9)						// 終了カウンタ最小
#define WEATHER_SNOW_S_END_MAX		(12)						// 終了カウンタ最大
#define WEATHER_SNOW_S_START_X		(256)						// 横から出すときのX座標
#define WEATHER_SNOW_S_START_X_RAN	(24)						// 横から出すときのX座標
#define	WEATHER_SNOW_S_START_Y_BASE	(-32)							// ベースになるY開始座標
#define	WEATHER_SNOW_S_START_Y_MAX	(168)						// X開始座標乱数の最大値


#define	WEATHER_SNOW_S_ADD_SP_TMG_NUM (4)						// スピードを変更するタイミングの数
#define	WEATHER_SNOW_S_ADD_SP_CHG_TMG (256)						// スピードを変更するタイミングのタイミング

#define WEATHER_SNOW_S_ADD_SP_TMG		(4)						// スピードを変更するタイミング
#define WEATHER_SNOW_S_SP_UP			(8)						// この値以上の横スピードのときは派手にする
static const int WEATHER_SSNOW_SPEED_X_SML[WEATHER_SNOW_S_ADD_SP_TMG_NUM] = {-7,-11,-15,-19};
static const int WEATHER_SSNOW_SPEED_Y_SML[WEATHER_SNOW_S_ADD_SP_TMG_NUM] = {3,6,3,6};	// たてに進むスピード



//-------------------------------------
//	あられ管理関数定数
//=====================================
#define	WEATHER_ARARE_TIMING_MIN		(1)							// 雨を出すタイミング最小
#define WEATHER_ARARE_TIMING_MAX		(8)						// 雨を出すタイミング最大
#define WEATHER_ARARE_TIMING_ADD		(4)							// タイミングを減らす数
#define WEATHER_ARARE_ADD_START		(0)							// 最初の同時に雨を登録する数
#define WEATHER_ARARE_ADD_TIMING		(2)							// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_ARARE_ADD			(1)							// 登録する数を増やす数
#define	WEATHER_ARARE_ADD_MAIN		(1)							// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_ARARE_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_ARARE_NOFADE_OBJ_START_DUST_NUM	( 10 )			// 何個単位で変化させるか
#define	WEATHER_ARARE_NOFADE_OBJ_START_DUST_MOVE	( 1 )			// ずらして動作させる値

/*== フォグ ==*/
#define	WEATHER_ARARE_FOG_TIMING		(150)							// に１回フォグテーブルを操作
#define	WEATHER_ARARE_FOG_TIMING_END	(50)							// に１回フォグテーブルを操作
#define WEATHER_ARARE_FOG_START		(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_ARARE_FOG_START_END	(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_ARARE_FOG_OFS		(0x300)

/*== あられオブジェクト ==*/
#define WEATHER_ARARE_SPEED_X		(-4)						// 横に進むスピード
#define WEATHER_ARARE_SPEED_Y		(10)						// たてに進むスピードベース
#define WEATHER_ARARE_ADDSPEED_X	(2)						// 横に進むスピードに足すあたい
#define WEATHER_ARARE_ADDSPEED_Y	(3)						// たてに進むスピードに足すあたい
#define	WEATHER_ARARE_END_MIN		(1)							// 終了カウンタ最小
#define WEATHER_ARARE_END_MAX		(3)							// 終了カウンタ最大
#define	WEATHER_ARARE_START_X_BASE	(0)							// ベースになるX開始座標
#define	WEATHER_ARARE_MUL_X			(15)						// ベースに雨の種類分足す値
#define	WEATHER_ARARE_START_X_MAX	(270)						// X開始座標乱数の最大値
#define	WEATHER_ARARE_START_Y		(0)						// Y開始座標
#define	WEATHER_ARARE_SPEED_ERR		(23)						// スピード補正値

#define WEATHER_ARARE_GROUND_SPEED_X		(-2)					// 着地後の横に進むスピード
#define WEATHER_ARARE_GROUND_SPEED_Y		(2)						// 着地後の横に進むスピード

// あられ確立
#define WEATHER_ARARE_RAND_MAX		(100)
#define WEATHER_ARARE_RAND_SMALL	(30)
#define WEATHER_ARARE_RAND_MIDDLE	(95)
#define WEATHER_ARARE_RAND_BIG		(100)


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雪ワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_SNOW_WORK;


//-------------------------------------
///	吹雪ワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_SSNOW_WORK;

//-------------------------------------
///	もっと吹雪ワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_SNOW_S_WORK;


//-------------------------------------
///	あられワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_ARARE_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	通常の雪
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_SNOW_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_SNOW_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 





//-------------------------------------
///	吹雪
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_SSNOW_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_SSNOW_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 
static void WEATHER_SSNOW_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_SSNOW_WORK* p_wk );



//-------------------------------------
///	すごい吹雪
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_SNOW_S_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_SNOW_S_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 
static void WEATHER_SNOW_S_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_SNOW_S_WORK* p_wk );


//-------------------------------------
///	あられ
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_ARARE_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_ARARE_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 

//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 通常の雪
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SNOW = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_snow_NCGR,			// OAM CG
	NARC_field_weather_snow_NCLR,			// OAM PLTT
	NARC_field_weather_snow_NCER,			// OAM CELL
	NARC_field_weather_snow_NANR,			// OAM CELLANM
	0,		// BGTEX
	0,		// GXTexSizeS
	0,		// GXTexSizeT
	0,		// GXTexRepeat
	0,		// GXTexFlip
	0,		// GXTexFmt
	0,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_SNOW_WORK),

	// 管理関数
	WEATHER_SNOW_Init,		// 初期化
	WEATHER_SNOW_FadeIn,		// フェードイン
	WEATHER_SNOW_NoFade,		// フェードなし
	WEATHER_SNOW_Main,		// メイン処理
	WEATHER_SNOW_InitFadeOut,	// フェードアウト
	WEATHER_SNOW_FadeOut,		// フェードアウト
	WEATHER_SNOW_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_SNOW_OBJ_Move,
};



// 吹雪
#if 1

WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SNOWSTORM = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	TRUE,		// BGを使用するか？
	NARC_field_weather_snow_s_NCGR,			// OAM CG
	NARC_field_weather_snow_NCLR,			// OAM PLTT
	NARC_field_weather_snow_s_NCER,			// OAM CELL
	NARC_field_weather_snow_s_NANR,			// OAM CELLANM
	NARC_field_weather_snow_storm_nsbtx,		// BGTEX
	GX_TEXSIZE_S32,		// GXTexSizeS
	GX_TEXSIZE_T32,		// GXTexSizeT
	GX_TEXREPEAT_ST,		// GXTexRepeat
	GX_TEXFLIP_NONE,		// GXTexFlip
	GX_TEXFMT_PLTT4,		// GXTexFmt
	GX_TEXPLTTCOLOR0_TRNS,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_SNOW_S_WORK),

	// 管理関数
	WEATHER_SNOW_S_Init,		// 初期化
	WEATHER_SNOW_S_FadeIn,		// フェードイン
	WEATHER_SNOW_S_NoFade,		// フェードなし
	WEATHER_SNOW_S_Main,		// メイン処理
	WEATHER_SNOW_S_InitFadeOut,	// フェードアウト
	WEATHER_SNOW_S_FadeOut,		// フェードアウト
	WEATHER_SNOW_S_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_SNOW_S_OBJ_Move,
};

#else

WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SNOWSTORM = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_snow_s_NCGR,			// OAM CG
	NARC_field_weather_snow_NCLR,			// OAM PLTT
	NARC_field_weather_snow_s_NCER,			// OAM CELL
	NARC_field_weather_snow_s_NANR,			// OAM CELLANM
	0,		// BGTEX
	0,		// GXTexSizeS
	0,		// GXTexSizeT
	0,		// GXTexRepeat
	0,		// GXTexFlip
	0,		// GXTexFmt
	0,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_SNOW_WORK),

	// 管理関数
	WEATHER_SSNOW_Init,		// 初期化
	WEATHER_SSNOW_FadeIn,		// フェードイン
	WEATHER_SSNOW_NoFade,		// フェードなし
	WEATHER_SSNOW_Main,		// メイン処理
	WEATHER_SSNOW_InitFadeOut,	// フェードアウト
	WEATHER_SSNOW_FadeOut,		// フェードアウト
	WEATHER_SSNOW_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_SSNOW_OBJ_Move,
};

#endif


// あられ
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_ARARE = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_arare_NCGR,			// OAM CG
	NARC_field_weather_arare_NCLR,			// OAM PLTT
	NARC_field_weather_arare_NCER,			// OAM CELL
	NARC_field_weather_arare_NANR,			// OAM CELLANM
	0,		// BGTEX
	0,		// GXTexSizeS
	0,		// GXTexSizeT
	0,		// GXTexRepeat
	0,		// GXTexFlip
	0,		// GXTexFmt
	0,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_ARARE_WORK),

	// 管理関数
	WEATHER_ARARE_Init,		// 初期化
	WEATHER_ARARE_FadeIn,		// フェードイン
	WEATHER_ARARE_NoFade,		// フェードなし
	WEATHER_ARARE_Main,		// メイン処理
	WEATHER_ARARE_InitFadeOut,	// フェードアウト
	WEATHER_ARARE_FadeOut,		// フェードアウト
	WEATHER_ARARE_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_ARARE_OBJ_Move,
};




//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雪
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	初期化動作
 *
 *	@param	p_wk			ワーク
 *	@param	fog_cont		フォグが有効か？
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	
	// 雪オブジェの登録フェード初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		1,
		WEATHER_SNOW_TIMING_MAX,
		WEATHER_SNOW_MAIN,
		WEATHER_SNOW_TIMING_MIN,
		-WEATHER_SNOW_TIMING_ADD,
		WEATHER_SNOW_ADD_TIMING,
		WEATHER_SNOW_ADD,
		WEATHER_SNOW_OBJ_Add );
	
	p_local_wk->work[1] = 0;							// 雪が１回下まで行ったかのフラグ
	
	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );
	
	p_local_wk->work[0] = WEATHER_SNOW_FOG_START;	// 同じくフォグ用
	
	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン処理
 *
 *	@param	p_wk		ワーク
 *	@param	fog_cont	フォグ管理フラグ
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	BOOL result;
	BOOL fog_result;
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeIn_Init( p_wk,
					WEATHER_FOG_SLOPE_DEFAULT, 
					WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_FOG_OFS, 
					WEATHER_SNOW_FOG_TIMING, fog_cont );


      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_show_dat, heapID );
		}
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ
			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードなし初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_SNOW_MAIN,
		WEATHER_SNOW_TIMING_MIN,
		WEATHER_SNOW_MAIN,
		WEATHER_SNOW_TIMING_MIN,
		-WEATHER_SNOW_TIMING_ADD,
		WEATHER_SNOW_ADD_TIMING,
		WEATHER_SNOW_ADD,
		WEATHER_SNOW_OBJ_Add );
	
	p_local_wk->work[1] = 0;							// 雪が１回下まで行ったかのフラグ
	p_local_wk->work[2] = 0;							// 自動破棄フラグ
	
	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_FOG_OFS, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_SNOW_OBJ_Add, WEATHER_SNOW_NOFADE_OBJ_START_NUM, WEATHER_SNOW_NOFADE_OBJ_START_DUST_NUM, WEATHER_SNOW_NOFADE_OBJ_START_DUST_MOVE, heapID );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_show_dat, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雪メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト開始
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// obj
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_SNOW_TIMING_MAX,
			WEATHER_SNOW_TIMING_ADD,
			-1 );
	// fog
	p_local_wk->work[0] = WEATHER_SNOW_FOG_START_END;	// 同じくフォグ用

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行
	
	// フォグ操作
	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワークが０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeOut_Init( p_wk,
					WEATHER_FOG_DEPTH_DEFAULT_START, 
					WEATHER_SNOW_FOG_TIMING_END, fog_cont );
		}
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result && result ){

			// オブジェ破棄フラグON
			p_local_wk->work[2] = 1;
			
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );


	
	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雪オブジェ動作
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	GFL_CLACTPOS pos;
	GFL_CLWK* p_clwk;
	s32*	p_local_wk;
	s32*	p_flg;
	s32*	p_exitflg;

	// ユーザワーク取得
	p_local_wk = WEATHER_OBJ_WORK_GetWork( p_wk );

	// アクターワーク取得
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	// 全破棄要請
	p_exitflg = (s32*)p_local_wk[7];
	switch( p_local_wk[8] ){
	case 0:
		// もし　破棄フラグが立っていたら自動破棄動さへ
		if( *p_exitflg ){
			p_local_wk[8] ++;
		}
		break;

	case 1:	// ｶｳﾝﾀが0になったら破棄
		p_local_wk[9] --;
		if( p_local_wk[9] < 0 ){
			WEATHER_TASK_DeleteObj( p_wk );
			return ;
		}
		break;
	}
	

	
	WEATHER_OBJ_WORK_GetPos( p_wk, &pos );

	// 動かす
	// X座標移動
	if(((p_local_wk[5] & 0xffff) >= p_local_wk[2])){
		pos.x	+= p_local_wk[1];
		p_local_wk[4]++;
		p_local_wk[5] &= 0xffff0000;
		// スピードチェンジャー
		if(p_local_wk[4] < 10){
			p_local_wk[2]--;
		}else{
			p_local_wk[2]++;
		}
		if(p_local_wk[4] >= 20){	// これで左右にゆらゆら
			p_local_wk[4] = 0;
			p_local_wk[1]*=-1;
		}//*/
	}
	// Y座標移動
	if(((p_local_wk[5] >> 16) >= p_local_wk[3])){
		pos.y += 1;
		p_local_wk[5] &= 0x0000ffff;
	} 

	// 座標設定
	WEATHER_OBJ_WORK_SetPos( p_wk, &pos );
	
	// カウント設定
	p_local_wk[6] = (p_local_wk[6] + 1) % 100;
	p_local_wk[5]++;
	p_local_wk[5]+=0x10000;

	// 破棄ゾーンにいたら破棄  
	if(((pos.y < -284) && (pos.y > -296)) ||
		((pos.y > 212) && (pos.y < 232))){
		p_flg = (s32*)(p_local_wk[0]);
		*p_flg = 1;
		WEATHER_TASK_DeleteObj( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクト登録処理
 *
 *	@param	p_wk	ワーク
 *	@param	num		数
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	int rand;	// 乱数
	WEATHER_OBJ_WORK* p_addobj;		// 登録オブジェ
	WEATHER_SNOW_WORK*	p_local_wk;	// システムワーク
	s32*	p_obj_wk;	// オブジェクトワーク
	int	frame;		// フレーム
	GFL_CLWK* p_clwk;
	GFL_CLACTPOS pos;	// 座標

	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	if(p_local_wk->work[1] == 1){
		num *= 2;
	}
	
	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		p_addobj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(p_addobj == NULL){			// 失敗したら終了
			break;
		}
		p_obj_wk	= WEATHER_OBJ_WORK_GetWork( p_addobj );
		p_clwk		= WEATHER_OBJ_WORK_GetClWk( p_addobj );
		
		frame = GFUser_GetPublicRand( WEATHER_SNOW_OBJ_NUM );
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );
		
		p_obj_wk[4] = 10;	// 横の動く方向変更カウンタ	
		p_obj_wk[5] = 0;
		p_obj_wk[6] = 0;	// 輝きカウンタ
		p_obj_wk[8] = 0;							// シーケンス
		
		rand = GFUser_GetPublicRand(0);

		p_obj_wk[9] = WEATHER_SNOW_OBJ_AUTODEST_TIMING_MIN + (rand % WEATHER_SNOW_OBJ_AUTODEST_TIMING_MAX);		// 自滅ｶｳﾝﾀ
	
		// スピード
		if((rand % 2) == 0){
			p_obj_wk[1] = 1;
		}else{
			p_obj_wk[1] = -1;
		}
		p_obj_wk[2] = WEATHER_SNOW_ADD_TMG_X_BASE + (GFUser_GetPublicRand(WEATHER_SNOW_ADD_TMG_X_RAN));
		p_obj_wk[3] = WEATHER_SNOW_ADD_TMG_Y_BASE + (GFUser_GetPublicRand(WEATHER_SNOW_ADD_TMG_Y_RAN));

		p_obj_wk[0] = (s32)&p_local_wk->work[1];		// 下まで行ったらフラグを立てるポインタを保存
		p_obj_wk[7] = (s32)&p_local_wk->work[2];		// 破棄フラグポインタON
		
		// 座標を設定
		pos.x = ( WEATHER_SSNOWTART_X_BASE + (GFUser_GetPublicRand(WEATHER_SSNOWTART_X_MAX)) );
		
		if((p_local_wk->work[1] == 1) &&
			(i >= (num /2)) ){	// 半分出す
			pos.y = ( -40 - GFUser_GetPublicRand(20));
		}else{
			pos.y = ( -8 - GFUser_GetPublicRand(20));
		}
		WEATHER_OBJ_WORK_SetPos( p_addobj, &pos );
	}

}




//----------------------------------------------------------------------------
/**
 *	@brief	吹雪初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SSNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_SSNOW_ADD_START,
		WEATHER_SSNOW_TIMING_MAX,
		WEATHER_SSNOW_ADD_MAIN,
		WEATHER_SSNOW_TIMING_MIN,
		-WEATHER_SSNOW_TIMING_ADD,
		WEATHER_SSNOW_ADD_TIMING,
		WEATHER_SSNOW_ADD,
		WEATHER_SSNOW_OBJ_Add );

	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = WEATHER_SNOW_S_FOG_START;	// 同じくフォグ用

	
	p_local_wk->work[1] = 0;					// オブジェ数カウンタ
	p_local_wk->work[2] = 0;					// BGカウンタ


	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );


  // SE
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_SNOWSTORM );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェード
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	BOOL result;
	BOOL fog_result;
	WEATHER_SSNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeIn_Init( p_wk,
					WEATHER_FOG_SLOPE_DEFAULT, 
					WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_S_FOG_OFS,
					WEATHER_SSNOW_FOG_TIMING, fog_cont );
      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_show_dat, heapID );
		}
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ

			// BGON
			WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE );

			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	WEATHER_SSNOW_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SSNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_SSNOW_ADD_MAIN,  
		WEATHER_SSNOW_TIMING_MIN,
		WEATHER_SSNOW_ADD_MAIN,
		WEATHER_SSNOW_TIMING_MIN,
		-WEATHER_SSNOW_TIMING_ADD,
		WEATHER_SSNOW_ADD_TIMING,
		WEATHER_SSNOW_ADD,
		WEATHER_SSNOW_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SSNOW_FOG_OFS, fog_cont );

	
	p_local_wk->work[1] = 0;					// オブジェ数カウンタ
	p_local_wk->work[2] = 0;					// BGカウンタ

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_SSNOW_OBJ_Add, WEATHER_SSNOW_NOFADE_OBJ_START_NUM, WEATHER_SSNOW_NOFADE_OBJ_START_DUST_NUM, WEATHER_SSNOW_NOFADE_OBJ_START_DUST_MOVE, heapID );


	// BGON
	WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_show_dat, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SSNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );

	// スクロール処理
	WEATHER_SSNOW_SCROLL_Main( p_wk, p_local_wk );


	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SSNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_SSNOW_TIMING_MAX,
			WEATHER_SSNOW_TIMING_ADD,
			WEATHER_SSNOW_ADD_END );
	
	// fog
	p_local_wk->work[0] = WEATHER_SSNOW_FOG_START_END;	// 同じくフォグ用

	// スクロール処理
	WEATHER_SSNOW_SCROLL_Main( p_wk, p_local_wk );

	// BGON
	WEATHER_TASK_3DBG_SetVisible( p_wk, FALSE );


  // SE
  WEATHER_TASK_StopLoopSnd( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SSNOW_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	
	// フォグ操作
	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワークが０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeOut_Init( p_wk,
					WEATHER_FOG_DEPTH_DEFAULT_START, 
					WEATHER_SSNOW_FOG_TIMING_END, fog_cont );
		}
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result && result ){

			// オブジェ破棄フラグON
			p_local_wk->work[2] = 1;
			
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	// スクロール処理
	WEATHER_SSNOW_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SSNOW_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SSNOW_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );


	
	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	オブジェ動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_SSNOW_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	s32* obj_w;
	GFL_CLACTPOS mat;	// 設定座標
	GFL_CLWK* p_clwk;

	obj_w = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	GFL_CLACT_WK_GetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
	
	// 動作フラグをチェック
	switch(obj_w[3]){
	case 0:		// 動作

		// 動かす
		mat.x += obj_w[4];
		mat.y += obj_w[2];

		// 破棄するかチェック
		if(obj_w[0]++ > obj_w[1]){
			// 破棄
			obj_w[3] = 1;
		}
		// カウンタで動かす値を変化
		if((obj_w[0] % obj_w[5]) == 0){
			obj_w[4]--;
			
			
			if(obj_w[2] > 1){
				obj_w[2]--;
			}//*/
		}

		GFL_CLACT_WK_SetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
	
		break;
	case 1:		// 破棄
		WEATHER_TASK_DeleteObj( p_wk );
		break;
	}		

}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	オブジェ追加
 */
//-----------------------------------------------------------------------------
static void WEATHER_SSNOW_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	int tbl_num;
	WEATHER_SSNOW_WORK* sys_w;		// システムワーク
	s32* obj_w;			// オブジェクトワーク
	static int WEATHER_SSNOW_ADD_SP_TMG[WEATHER_SSNOW_ADD_SP_TMG_NUM] = {8,16,8,5};
	static int WEATHER_SSNOW_SPEED_Y[WEATHER_SSNOW_ADD_SP_TMG_NUM] = {4,4,4,4};	// たてに進むスピード
	int frame;	// フレーム数
	GFL_CLACTPOS mat;	// 設定座標
	GFL_CLWK* p_clwk;
	
	sys_w = WEATHER_TASK_GetWorkData( p_wk );
	
	

	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		add_obj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(add_obj == NULL){			// 失敗したら終了
			break;
		}

		obj_w = WEATHER_OBJ_WORK_GetWork( add_obj );		// オブジェワーク作成
		p_clwk	= WEATHER_OBJ_WORK_GetClWk( add_obj );

		// 風の値を求める
		sys_w->work[1]++;
		if(sys_w->work[1] >= (WEATHER_SSNOW_ADD_SP_TMG_TMG * WEATHER_SSNOW_ADD_SP_TMG_NUM)){
			sys_w->work[1] = 0;
		}
		tbl_num = (sys_w->work[1] / WEATHER_SSNOW_ADD_SP_TMG_TMG);
		obj_w[5] = WEATHER_SSNOW_ADD_SP_TMG[ tbl_num ];
		
	
		
		// 領域を初期化
		obj_w[0] = 0;			// カウンタ
		obj_w[1] = WEATHER_SSNOW_END_MIN + GFUser_GetPublicRand((WEATHER_SSNOW_END_MAX - WEATHER_SSNOW_END_MIN));	// 終了カウンタ
		frame = (obj_w[1] - WEATHER_SSNOW_END_MIN) / WEATHER_SSNOW_END_DIV;	// 吹雪の種類
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );
		
		obj_w[4] = WEATHER_SSNOW_SPEED_X * (frame+1);
		obj_w[2] = WEATHER_SSNOW_SPEED_Y[tbl_num] * (frame+1);	// たてスピード
		obj_w[3] = 0;			// 破棄アニメフラグ
		
		// 座標を設定
		{
			mat.x = WEATHER_SSNOW_START_X_BASE + (frame * WEATHER_SSNOW_MUL_X) + GFUser_GetPublicRand(WEATHER_SSNOW_START_X_MAX);
			mat.y = WEATHER_SSNOW_START_Y;
			
			GFL_CLACT_WK_SetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
		}
	
	}

}





//----------------------------------------------------------------------------
/**
 *	@brief	スクロール処理
 *
 *	@param	p_sys		システムワーク
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_SSNOW_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_SSNOW_WORK* p_wk )
{
	int x, y;
	WEATHER_TASK_GetScrollDist( p_sys, &x, &y );
	WEATHER_TASK_ScrollObj( p_sys, x, y );

	// BG面を斜め上に動かす
	p_wk->work[2] = (p_wk->work[2] + 6) % 256;
	WEATHER_TASK_3DBG_SetScrollX( p_sys, (p_wk->work[2]*2) - x );
	WEATHER_TASK_3DBG_SetScrollY( p_sys, (-p_wk->work[2]*2) + y );
}







//----------------------------------------------------------------------------
/**
 *	@brief	吹雪初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_S_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_SSNOW_ADD_START,	// obj登録数
		WEATHER_SSNOW_TIMING_MAX,	// 登録タイミング
		WEATHER_SNOW_S_ADD_MAIN,
		WEATHER_SNOW_S_TIMING_MIN,
		-WEATHER_SNOW_S_TIMING_ADD,
		WEATHER_SNOW_S_ADD_TIMING,
		WEATHER_SNOW_S_ADD,
		WEATHER_SNOW_S_OBJ_Add );

	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = WEATHER_SNOW_S_FOG_START;	// 同じくフォグ用

	
	p_local_wk->work[1] = 0;					// オブジェ数カウンタ
	p_local_wk->work[2] = 0;					// BGカウンタ

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );


  // SE
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_SNOWSTORM );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェード
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	BOOL result;
	BOOL fog_result;
	WEATHER_SNOW_S_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeIn_Init( p_wk,
					WEATHER_FOG_SLOPE_DEFAULT, 
					WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_S_FOG_OFS,
					WEATHER_SNOW_S_FOG_TIMING, fog_cont );

      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_show_dat, heapID );
		}
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ

			// BGON
			WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE );

			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	WEATHER_SNOW_S_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_S_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_SNOW_S_ADD_MAIN,	// obj登録数
		WEATHER_SNOW_S_TIMING_MIN,	// 登録タイミング
		WEATHER_SNOW_S_ADD_MAIN,
		WEATHER_SNOW_S_TIMING_MIN,
		-WEATHER_SNOW_S_TIMING_ADD,
		WEATHER_SNOW_S_ADD_TIMING,
		WEATHER_SNOW_S_ADD,
		WEATHER_SNOW_S_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_SNOW_S_FOG_OFS, fog_cont );

	
	p_local_wk->work[1] = 0;					// オブジェ数カウンタ
	p_local_wk->work[2] = 0;					// BGカウンタ

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_SNOW_S_OBJ_Add, WEATHER_SNOW_S_NOFADE_OBJ_START_NUM, WEATHER_SNOW_S_NOFADE_OBJ_START_DUST_NUM, WEATHER_SNOW_S_NOFADE_OBJ_START_DUST_MOVE, heapID );


	// BGON
	WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_show_dat, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_S_WORK* p_local_wk;
	s32 wind;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	if( WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID ) ){

		wind = p_local_wk->work[1] / WEATHER_SNOW_S_ADD_SP_CHG_TMG;
		wind = WEATHER_SSNOW_SPEED_X_SML[wind];
		
		if(wind <= -WEATHER_SNOW_S_SP_UP){
			// 砂嵐登録
			WEATHER_SNOW_S_OBJ_Add( p_wk, WEATHER_SNOW_S_ADD_MAIN, heapID );
		}
	}

	// スクロール処理
	WEATHER_SNOW_S_SCROLL_Main( p_wk, p_local_wk );


	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_S_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_SNOW_S_TIMING_MAX,
			WEATHER_SNOW_S_TIMING_ADD,
			WEATHER_SNOW_S_ADD_END );
	
	// fog
	p_local_wk->work[0] = WEATHER_SNOW_S_FOG_START_END;	// 同じくフォグ用

	// スクロール処理
	WEATHER_SNOW_S_SCROLL_Main( p_wk, p_local_wk );

	// BGON
	WEATHER_TASK_3DBG_SetVisible( p_wk, FALSE );

  // SE
  WEATHER_TASK_StopLoopSnd( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_S_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行
	
	// フォグ操作
	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワークが０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeOut_Init( p_wk,
					WEATHER_FOG_DEPTH_DEFAULT_START, 
					WEATHER_SNOW_S_FOG_TIMING_END, fog_cont );
		}
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result && result ){

			// オブジェ破棄フラグON
			p_local_wk->work[2] = 1;
			
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	// スクロール処理
	WEATHER_SNOW_S_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SNOW_S_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SNOW_S_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );


	
	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	オブジェ動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_S_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	s32* obj_w;
	GFL_CLACTPOS mat;	// 設定座標
	GFL_CLWK* p_clwk;

	obj_w = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	GFL_CLACT_WK_GetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
	
	// 動作フラグをチェック
	switch(obj_w[3]){
	case 0:		// 動作

		// 動かす
		mat.x += obj_w[4];
		mat.y += obj_w[2];

		// 破棄するかチェック
		if(obj_w[0]++ > obj_w[1]){
			// 破棄
			obj_w[3] = 1;
		}
		// カウンタで動かす値を変化
		if((obj_w[0] % WEATHER_SNOW_S_ADD_SP_TMG) == 0){
			obj_w[4] += obj_w[5];
			
			if(obj_w[2] > 1){
				obj_w[2]--;
			}
		}
	
		GFL_CLACT_WK_SetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
		break;
	case 1:		// 破棄
		WEATHER_TASK_DeleteObj( p_wk );
		break;
	}		

}

//----------------------------------------------------------------------------
/**
 *	@brief	吹雪	オブジェ追加
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_S_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	int tbl_num;
	WEATHER_SNOW_S_WORK* sys_w;	// システムワーク
	s32* obj_w;	// オブジェクトワーク
	int		frame;		// フレーム数
	GFL_CLWK* p_clwk;
	GFL_CLACTPOS mat;
	
	sys_w = WEATHER_TASK_GetWorkData( p_wk );

		
	// 風の値を求める
	sys_w->work[1]++;
	if(sys_w->work[1] >= (WEATHER_SNOW_S_ADD_SP_CHG_TMG  * WEATHER_SNOW_S_ADD_SP_TMG_NUM)){
		sys_w->work[1] = 0;
	}
	tbl_num = (sys_w->work[1] / WEATHER_SNOW_S_ADD_SP_CHG_TMG );

	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		add_obj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(add_obj == NULL){			// 失敗したら終了
			break;
		}

		// オブジェワーク
		obj_w = WEATHER_OBJ_WORK_GetWork( add_obj );
		p_clwk	= WEATHER_OBJ_WORK_GetClWk( add_obj );
		
		// 領域を初期化
		obj_w[0] = 0;			// カウンタ
		obj_w[1] = WEATHER_SNOW_S_END_MIN + GFUser_GetPublicRand((WEATHER_SNOW_S_END_MAX - WEATHER_SNOW_S_END_MIN) );	// 終了カウンタ
		frame = GFUser_GetPublicRand(4);	// 吹雪の種類
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );
		
		obj_w[4] = WEATHER_SSNOW_SPEED_X_SML[tbl_num] * ((frame)+1);
		obj_w[2] = WEATHER_SSNOW_SPEED_Y_SML[tbl_num] * ((frame)+1);	// たてスピード
		obj_w[3] = 0;			// 破棄アニメフラグ
		if(frame == 3){
			obj_w[4] += WEATHER_SSNOW_SPEED_X_SML[tbl_num];
			obj_w[2] += WEATHER_SSNOW_SPEED_Y_SML[tbl_num];
		}
		obj_w[5] = WEATHER_SSNOW_SPEED_X_SML[tbl_num];
		
		// 座標を設定
		mat.x = WEATHER_SNOW_S_START_X + GFUser_GetPublicRand(WEATHER_SNOW_S_START_X_RAN);
		mat.y = WEATHER_SNOW_S_START_Y_BASE + GFUser_GetPublicRand(WEATHER_SNOW_S_START_Y_MAX);

		GFL_CLACT_WK_SetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
	}

}





//----------------------------------------------------------------------------
/**
 *	@brief	スクロール処理
 *
 *	@param	p_sys		システムワーク
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_SNOW_S_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_SNOW_S_WORK* p_wk )
{
	int x, y;
	WEATHER_TASK_GetScrollDist( p_sys, &x, &y );
	WEATHER_TASK_ScrollObj( p_sys, x, y );

	// BG面を斜め上に動かす
	p_wk->work[2] = (p_wk->work[2] + 6) % 256;
	WEATHER_TASK_3DBG_SetScrollX( p_sys, (p_wk->work[2]*2) - x );
	WEATHER_TASK_3DBG_SetScrollY( p_sys, (-p_wk->work[2]*2) + y );
}






//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_ARARE_ADD_START,
		WEATHER_ARARE_TIMING_MAX,
		WEATHER_ARARE_ADD_MAIN,
		WEATHER_ARARE_TIMING_MIN,
		-WEATHER_ARARE_TIMING_ADD,
		WEATHER_ARARE_ADD_TIMING,
		WEATHER_ARARE_ADD,
		WEATHER_ARARE_OBJ_Add );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );



	p_local_wk->work[0] = WEATHER_ARARE_FOG_START;	// 同じくフォグ用

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

  // SE
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_ARARE );
	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;


	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){
			WEATHER_TASK_FogFadeIn_Init( p_wk,
			WEATHER_FOG_SLOPE_DEFAULT, 
			WEATHER_FOG_DEPTH_DEFAULT + WEATHER_ARARE_FOG_OFS, 
			WEATHER_ARARE_FOG_TIMING,
			fog_cont );


      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat, heapID );
		}
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ
			// シーケンス変更
			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_ARARE_ADD_MAIN,
		WEATHER_ARARE_TIMING_MIN,
		WEATHER_ARARE_ADD_MAIN,
		WEATHER_ARARE_TIMING_MIN,
		-WEATHER_ARARE_TIMING_ADD,
		WEATHER_ARARE_ADD_TIMING,
		WEATHER_ARARE_ADD,
		WEATHER_ARARE_OBJ_Add );


	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_ARARE_FOG_OFS, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_ARARE_OBJ_Add, WEATHER_ARARE_NOFADE_OBJ_START_NUM, WEATHER_ARARE_NOFADE_OBJ_START_DUST_NUM, WEATHER_ARARE_NOFADE_OBJ_START_DUST_MOVE, heapID );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_ARARE_TIMING_MAX,
			WEATHER_ARARE_TIMING_ADD,
			-WEATHER_ARARE_ADD );
	
	// フォグ
	p_local_wk->work[0] = WEATHER_ARARE_FOG_START_END;	// 同じくフォグ用


  // SE
  WEATHER_TASK_StopLoopSnd( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	// フォグ操作
	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワークが０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeOut_Init( p_wk,
					WEATHER_FOG_DEPTH_DEFAULT_START, 
					WEATHER_ARARE_FOG_TIMING_END, fog_cont );
		}
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result && result ){
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_ARARE_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_ARARE_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );

	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雨オブジェ動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_ARARE_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	int i;
	GFL_CLACTPOS mat;
	GFL_CLWK* p_clwk;
	s32*	obj_w;

	obj_w = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	WEATHER_OBJ_WORK_GetPos( p_wk, &mat );
	
	// 動作フラグをチェック
	switch(obj_w[3]){
	case 0:		// 動作
	
		// 破棄するかチェック
		obj_w[0] += 1;
		if(obj_w[0] > obj_w[1]){
			
			// 破棄アニメ
			obj_w[3] = 1;		
			obj_w[0] = 8 + (obj_w[6] % 4) * 2;			// 破棄アニメカウンタ

			obj_w[4] = WEATHER_ARARE_GROUND_SPEED_X;
			obj_w[2] = WEATHER_ARARE_GROUND_SPEED_Y;

			GFL_CLACT_WK_SetAnmIndex( p_clwk, GFL_CLACT_WK_GetAnmIndex( p_clwk ) + 3 );
		}else{

			// 動かす
			mat.x += (obj_w[4]);
			mat.y += (obj_w[2]);

			// 座標設定
			WEATHER_OBJ_WORK_SetPos( p_wk, &mat );
		}

		break;
	case 1:		// 破棄アニメ
		// 座標設定
		mat.x += obj_w[4];
		if( (obj_w[0] % 2) == 0 ){
			mat.y += obj_w[2];
		}else{
			mat.y -= obj_w[2];
		}

		if( (obj_w[0] % 4) == 0 ){

			if( (obj_w[2] - 1) >= 0 ){
				obj_w[2] --;
			}
			if( (obj_w[4] + 1) <= 0 ){
				obj_w[4] ++;
			}
		}
		WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &mat );

		if(obj_w[0]-- <= 0){
			obj_w[3] = 2;		// 破棄
		}
		break;

	case 2:		// 破棄
		WEATHER_TASK_DeleteObj( p_wk );
		break;
	}		
}

//----------------------------------------------------------------------------
/**
 *	@brief	雨オブジェ登録
 */
//-----------------------------------------------------------------------------
static void WEATHER_ARARE_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	WEATHER_ARARE_WORK*	p_local_wk;	// システムワーク
	int	err;	// 補正値
	s32* obj_w;	// オブジェクトワーク
	int frame;	// 描画フレーム
	GFL_CLACTPOS mat;	// 設定座標
	GFL_CLWK* p_clwk;
	u32 rand;

	// ユーザワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		add_obj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(add_obj == NULL){			// 失敗したら終了
			break;
		}
		obj_w = WEATHER_OBJ_WORK_GetWork( add_obj );		// オブジェワーク作成
		p_clwk	= WEATHER_OBJ_WORK_GetClWk( add_obj );

		// 乱数取得
		rand = GFUser_GetPublicRand(0);
		
		// 領域を初期化
		obj_w[0] = 0;			// カウンタ
		if( (rand % WEATHER_ARARE_RAND_MAX) < WEATHER_ARARE_RAND_SMALL ){
			frame = 0;
		}else if( (rand % WEATHER_ARARE_RAND_MAX) < WEATHER_ARARE_RAND_MIDDLE ){
			frame = 1;
		}else if( (rand % WEATHER_ARARE_RAND_MAX) < WEATHER_ARARE_RAND_BIG ){
			frame = 2;
		}
		GFL_CLACT_WK_SetAnmIndex( p_clwk, 2-frame );

		err = (rand % WEATHER_ARARE_SPEED_ERR);
		obj_w[2] = WEATHER_ARARE_SPEED_Y * (frame+1) + err;
		if(frame == 2){		// 一番前の雨はスピードを１段階上げる
			obj_w[2] += WEATHER_ARARE_ADDSPEED_Y;
		}
		
		err /= WEATHER_ARARE_SPEED_X;
		obj_w[4] = WEATHER_ARARE_SPEED_X * (frame+1) + err;
		if(frame == 2){		// 一番前の雨はスピードを１段階上げる
			obj_w[4] += WEATHER_ARARE_ADDSPEED_X;
		}

		obj_w[3] = 0;			// 破棄アニメフラグ
		
		// 破棄タイミング	全部共通の範囲の値
		obj_w[1] = WEATHER_ARARE_END_MIN  + (rand % WEATHER_ARARE_END_MAX);
		
		// 座標を設定
		mat.x = ( WEATHER_ARARE_START_X_BASE + (frame * WEATHER_ARARE_MUL_X) + (rand % WEATHER_ARARE_START_X_MAX) );
		mat.y = WEATHER_ARARE_START_Y;
		WEATHER_OBJ_WORK_SetPos( add_obj, &mat );

		obj_w[6] = rand;			// 乱数保存
	}
}

