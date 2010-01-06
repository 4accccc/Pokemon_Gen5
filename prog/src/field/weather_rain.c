//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_rain.c
 *	@brief		天気雨
 *	@author		tomoya takahashi
 *	@date		2009.03.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc/arc_def.h"
#include "arc/field_weather.naix"
#include "arc/field_weather_light.naix"

#include "weather_rain.h"

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
//	雨管理関数定数
//=====================================
#define	WEATHER_RAIN_TIMING_MIN		(0)							// 雨を出すタイミング最小
#define WEATHER_RAIN_TIMING_MAX		(8)						// 雨を出すタイミング最大
#define WEATHER_RAIN_TIMING_ADD		(4)							// タイミングを減らす数
#define WEATHER_RAIN_ADD_START		(1)							// 最初の同時に雨を登録する数
#define WEATHER_RAIN_ADD_TIMING		(2)							// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_RAIN_ADD			(1)							// 登録する数を増やす数
#define	WEATHER_RAIN_ADD_MAIN		(2)							// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_RAIN_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_RAIN_NOFADE_OBJ_START_DUST_NUM	( 10 )			// 何個単位で変化させるか
#define	WEATHER_RAIN_NOFADE_OBJ_START_DUST_MOVE	( 1 )			// ずらして動作させる値

/*== フォグ ==*/
#define	WEATHER_RAIN_FOG_TIMING		(150)							// に１回フォグテーブルを操作
#define	WEATHER_RAIN_FOG_TIMING_END	(50)							// に１回フォグテーブルを操作
#define WEATHER_RAIN_FOG_START		(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_RAIN_FOG_START_END	(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_RAIN_FOG_OFS		(0x300)

/*== 雨オブジェクト ==*/
#define WEATHER_RAIN_SPEED_X		(-8)						// 横に進むスピード
#define WEATHER_RAIN_SPEED_Y		(16)						// たてに進むスピードベース
#define	WEATHER_RAIN_END_MIN		(1)							// 終了カウンタ最小
#define WEATHER_RAIN_END_MAX		(2)							// 終了カウンタ最大
#define	WEATHER_RAIN_START_X_BASE	(0)							// ベースになるX開始座標
#define	WEATHER_RAIN_MUL_X			(15)						// ベースに雨の種類分足す値
#define	WEATHER_RAIN_START_X_MAX	(270)						// X開始座標乱数の最大値
#define	WEATHER_RAIN_START_Y		(-96)						// Y開始座標
#define	WEATHER_RAIN_SPEED_ERR		(20)						// スピード補正値





//-------------------------------------
//	大雨
//=====================================
#define	WEATHER_STRAIN_TIMING_MIN		(0)					// 雨を出すタイミング最小
#define WEATHER_STRAIN_TIMING_MAX		(15)				// 雨を出すタイミング最大
#define WEATHER_STRAIN_TIMING_ADD		(3)					// タイミングを減らす数
#define WEATHER_STRAIN_ADD_START		(1)					// 最初の同時に雨を登録する数
#define WEATHER_STRAIN_ADD_TIMING		(2)					// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_STRAIN_ADD			(1)						// 登録する数を増やす数
#define WEATHER_STRAIN_ADD_END		(-1)					// 登録する数を増やす数
#define	WEATHER_STRAIN_ADD_MAIN		(1)					// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_STRAIN_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_STRAIN_NOFADE_OBJ_START_DUST_NUM	( 5 )		// 何個ずつ動作数を変更するか
#define	WEATHER_STRAIN_NOFADE_OBJ_START_DUST_MOVE	( 1 )		// ずらす動作数の値

/*== フォグ ==*/
#define	WEATHER_STRAIN_FOG_TIMING		(150)						// に１回フォグテーブルを操作
#define	WEATHER_STRAIN_FOG_TIMING_END	(50)						// に１回フォグテーブルを操作
#define WEATHER_STRAIN_FOG_START		(1)					// このカウント動いてからフォグテーブルを操作
#define WEATHER_STRAIN_FOG_START_END	(1)					// このカウント動いてからフォグテーブルを操作
#define WEATHER_STRAIN_FOG_OFS			(0x200)


/*== 雨オブジェクト ==*/
#define WEATHER_STRAIN_SPEED_X		(-20)						// 横に進むスピード
#define WEATHER_STRAIN_SPEED_Y		(20)							// たてに進むスピードベース
#define	WEATHER_STRAIN_END_MIN		(1)							// 終了カウンタ最小
#define WEATHER_STRAIN_END_MAX		(2)							// 終了カウンタ最大
#define	WEATHER_STRAIN_START_X		(0)							// ベースになるX開始座標
#define	WEATHER_STRAIN_START_X_MAX	(512)						// X開始座標乱数値
#define	WEATHER_STRAIN_START_Y		(-80)						// Y開始座標
#define	WEATHER_STRAIN_START_Y_MAX	(40)						// Y開始座標乱数値

#define WEATHER_STRAIN_OBJ_MUL_NUM	(5)							// オブジェのスピードを変化させる値
#define WEATHER_STRAIN_OBJ_MUL_CHG	(60)						// 変更タイミング



//-------------------------------------
///	雷シーケンス
//=====================================
enum{
	WEATHER_SPARK_START_WAIT,
	WEATHER_SPARK_SETUP,
	WEATHER_SPARK_SPARK,		// 雷
	WEATHER_SPARK_SPARK_WAIT,	// 雷
	WEATHER_SPARK_WAIT,
};

// 雷テーブル数
#define WEATHER_SPARK_TBL_MAX	( 4 )	// 4回まで繰り返すことが出来る

// 初期化ウエイト
#define WEATHER_SPARK_INIT_RAND	(3)

// 雷の色構成青
#define WEATHER_SPARK_COLOR_BLUE	( 8 )	//  これをコメントアウトすると青みを調整できるように
#ifndef WEATHER_SPARK_COLOR_BLUE
static int blue = 8;
#endif

// 雷のタイプ				距離
// 1.軽い雷のみ				遠く
// 2.重い雷のみ				遠く
// 3.軽い雷->重い雷			すごく近く
enum{
	WEATHER_SPARK_TYPE_LIGHT,
	WEATHER_SPARK_TYPE_HARD,
	WEATHER_SPARK_TYPE_LIGHT_HARD,

	WEATHER_SPARK_TYPE_MAX,
};

// 確立
#define WEATHER_SPARK_TYPE_RAND_MAX	( 100 )
static const u8 sc_WEATHER_SPARK_TYPE_RAND[WEATHER_SPARK_TYPE_MAX] = {
	30,
	75,
	100,
};

// 雷がおちた後のウエイト最小
static const u16 sc_WEATHER_SPARK_TYPE_RAND_WAIT_MIN[WEATHER_SPARK_TYPE_MAX] = {
	120,
	240,
	240,
};

// 雷がおちた後のウエイト乱数で変動する値
static const u16 sc_WEATHER_SPARK_TYPE_RAND_WAIT_MAX[WEATHER_SPARK_TYPE_MAX] = {
	240,
	300,
	300,
};

// 雷の強さ	最小
#define WEATHER_SPARK_STRONG_MAX	(31)
static const u8 sc_WEATHER_SPARK_STRONG_RAND_MIN[WEATHER_SPARK_TYPE_MAX] = {
	18,
	24,
	30,
};

// 雷の強さ	乱数で変動する値
static const u8 sc_WEATHER_SPARK_STRONG_RAND_MAX[WEATHER_SPARK_TYPE_MAX] = {
	3,
	3,
	1,
};

// 雷によってのサウンド起動ウエイト
static const s8 sc_WEATHER_SPARK_SND_WAIT[WEATHER_SPARK_TYPE_MAX] = 
{
  28,
  13,
  10,
};

// 雷フラッシュスピード
#define WEATHER_SPARK_FLASHIN_SYNC	( 4 )
static const u8 sc_WEATHER_SPARK_FLASHOUT_SYNC[32] = {
  15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,
	15,15,15,20,20,30,30,35,35,35,
	40,40,
	
};

// 軽い雷の繰り返す数
#define WEATHER_SPARK_LIGHT_NUM_MIN	(1)
#define WEATHER_SPARK_LIGHT_NUM_MAX	(2) //

// 軽い->重い　ウエイト乱数値
#define WEATHER_SPARK_LIGHT_HARD_WAIT_RAND		(4)
#define WEATHER_SPARK_LIGHT_HARD_OUTSYNC		(8)



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雨
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_RAIN_WORK;


//-------------------------------------
///	雷１情報
//=====================================
typedef struct {
	u16 insync;
	u16 outsync;
	u16 spark_power;
	s16 wait;

  s16 snd_wait;
  u16 snd_se;
} SPARK_DATA;


//-------------------------------------
///	雷
//=====================================
typedef struct {
	u16 seq;
	u16 spark_type;
	u16	spark_tbl_num;
	u16 spark_tbl_count;
	
	SPARK_DATA	spark_data[ WEATHER_SPARK_TBL_MAX ];

	s32	wait;
  s16 snd_wait;
  s16 snd_se;
} WEATHER_SPARK_WORK;

//-------------------------------------
///	雷雨
//=====================================
typedef struct {
	s32 work[10];

	WEATHER_SPARK_WORK	spark;
} WEATHER_SPARKRAIN_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	通常の雨
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_RAIN_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_RAIN_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 


//-------------------------------------
///	雷雨
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_SPARKRAIN_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_SPARKRAIN_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 



//-------------------------------------
///	雷
//=====================================
static void WEATHER_PARK_Init( WEATHER_SPARK_WORK* p_wk );
static void WEATHER_PARK_Exit( WEATHER_SPARK_WORK* p_wk );
static void WEATHER_PARK_Main( WEATHER_SPARK_WORK* p_wk, WEATHER_TASK* p_sys );

static void WEATEHR_SPARK_SetUp_Light( WEATHER_SPARK_WORK* p_wk );
static void WEATEHR_SPARK_SetUp_Hard( WEATHER_SPARK_WORK* p_wk );
static void WEATEHR_SPARK_SetUp_LightAndHard( WEATHER_SPARK_WORK* p_wk );

static GXRgb WEATHER_SPARK_GetColor( const WEATHER_SPARK_WORK* cp_wk );



//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 通常の雨
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIN = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_rain_NCGR,			// OAM CG
	NARC_field_weather_rain_NCLR,			// OAM PLTT
	NARC_field_weather_rain_NCER,			// OAM CELL
	NARC_field_weather_rain_NANR,			// OAM CELLANM
	0,		// BGTEX
	0,		// GXTexSizeS
	0,		// GXTexSizeT
	0,		// GXTexRepeat
	0,		// GXTexFlip
	0,		// GXTexFmt
	0,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_RAIN_WORK),

	// 管理関数
	WEATHER_RAIN_Init,		// 初期化
	WEATHER_RAIN_FadeIn,		// フェードイン
	WEATHER_RAIN_NoFade,		// フェードなし
	WEATHER_RAIN_Main,		// メイン処理
	WEATHER_RAIN_InitFadeOut,	// フェードアウト
	WEATHER_RAIN_FadeOut,		// フェードアウト
	WEATHER_RAIN_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_RAIN_OBJ_Move,
};


// 雷雨
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SPARKRAIN = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_rain_st_NCGR,			// OAM CG
	NARC_field_weather_rain_NCLR,			// OAM PLTT
	NARC_field_weather_rain_st_NCER,			// OAM CELL
	NARC_field_weather_rain_st_NANR,			// OAM CELLANM
	0,		// BGTEX
	0,		// GXTexSizeS
	0,		// GXTexSizeT
	0,		// GXTexRepeat
	0,		// GXTexFlip
	0,		// GXTexFmt
	0,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_SPARKRAIN_WORK),

	// 管理関数
	WEATHER_SPARKRAIN_Init,		// 初期化
	WEATHER_SPARKRAIN_FadeIn,		// フェードイン
	WEATHER_SPARKRAIN_NoFade,		// フェードなし
	WEATHER_SPARKRAIN_Main,		// メイン処理
	WEATHER_SPARKRAIN_InitFadeOut,	// フェードアウト
	WEATHER_SPARKRAIN_FadeOut,		// フェードアウト
	WEATHER_SPARKRAIN_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_SPARKRAIN_OBJ_Move,
};





//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_RAIN_ADD_START,
		WEATHER_RAIN_TIMING_MAX,
		WEATHER_RAIN_ADD_MAIN,
		WEATHER_RAIN_TIMING_MIN,
		-WEATHER_RAIN_TIMING_ADD,
		WEATHER_RAIN_ADD_TIMING,
		WEATHER_RAIN_ADD,
		WEATHER_RAIN_OBJ_Add );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );



	p_local_wk->work[0] = WEATHER_RAIN_FOG_START;	// 同じくフォグ用

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
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
			WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIN_FOG_OFS, 
			WEATHER_RAIN_FOG_TIMING,
			fog_cont );

      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat );

      // 音
      WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_RAIN );	
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
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_RAIN_ADD_MAIN,
		WEATHER_RAIN_TIMING_MIN,
		WEATHER_RAIN_ADD_MAIN,
		WEATHER_RAIN_TIMING_MIN,
		-WEATHER_RAIN_TIMING_ADD,
		WEATHER_RAIN_ADD_TIMING,
		WEATHER_RAIN_ADD,
		WEATHER_RAIN_OBJ_Add );


	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIN_FOG_OFS, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_RAIN_OBJ_Add, WEATHER_RAIN_NOFADE_OBJ_START_NUM, WEATHER_RAIN_NOFADE_OBJ_START_DUST_NUM, WEATHER_RAIN_NOFADE_OBJ_START_DUST_MOVE, heapID );

  // 音
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_RAIN );	

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
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
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_RAIN_TIMING_MAX,
			WEATHER_RAIN_TIMING_ADD,
			-WEATHER_RAIN_ADD );
	
	// フォグ
	p_local_wk->work[0] = WEATHER_RAIN_FOG_START_END;	// 同じくフォグ用


	// 音
	WEATHER_TASK_StopLoopSnd( p_wk );	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
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
					WEATHER_RAIN_FOG_TIMING_END, fog_cont );
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
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );

	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雨オブジェ動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_RAIN_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
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
		// 動かす
		mat.x += (obj_w[4]*2);
		mat.y += (obj_w[2]*2);
	
		// 破棄するかチェック
		obj_w[0] += 2;
		if(obj_w[0] > obj_w[1]){
			// アニメーションさせるかチェック
			if(GFUser_GetPublicRand(10) < 7){		// 7/10は破棄
				// 破棄
				obj_w[3] = 2;
			}else{
			
				// 破棄アニメ
				obj_w[3] = 1;		
				obj_w[0] = 4;			// 破棄アニメカウンタ
				GFL_CLACT_WK_SetAnmIndex( p_clwk, 3 );
			}
		}
		// 座標設定
		WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &mat );

		break;
	case 1:		// 破棄アニメ
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
static void WEATHER_RAIN_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	WEATHER_RAIN_WORK*	p_local_wk;	// システムワーク
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
		frame = rand%3;	// 雨の種類
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );

		err = (rand % WEATHER_RAIN_SPEED_ERR);
		obj_w[2] = WEATHER_RAIN_SPEED_Y * (frame+1) + err;
		if(frame == 2){		// 一番前の雨はスピードを１段階上げる
			obj_w[2] += WEATHER_RAIN_SPEED_Y;
		}
		
		err /= WEATHER_RAIN_SPEED_X;
		obj_w[4] = WEATHER_RAIN_SPEED_X * (frame+1) + err;
		if(frame == 2){		// 一番前の雨はスピードを１段階上げる
			obj_w[4] += WEATHER_RAIN_SPEED_X;
		}
		obj_w[3] = 0;			// 破棄アニメフラグ
		
		// 破棄タイミング	全部共通の範囲の値
		obj_w[1] = WEATHER_RAIN_END_MIN  + (rand % WEATHER_RAIN_END_MAX);
		
		// 座標を設定
		mat.x = ( WEATHER_RAIN_START_X_BASE + (frame * WEATHER_RAIN_MUL_X) + (rand % WEATHER_RAIN_START_X_MAX) );
		mat.y = WEATHER_RAIN_START_Y;
		WEATHER_OBJ_WORK_SetPosNoTurn( add_obj, &mat );
	}
}








//----------------------------------------------------------------------------
/**
 *	@brief		雷雨初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// スパーク初期化
	WEATHER_PARK_Init( &p_local_wk->spark );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk, 
		WEATHER_STRAIN_ADD_START,	// obj登録数
		WEATHER_STRAIN_TIMING_MAX,// 登録タイミング
		WEATHER_STRAIN_ADD_MAIN,
		WEATHER_STRAIN_TIMING_MIN,
		-WEATHER_STRAIN_TIMING_ADD,
		WEATHER_STRAIN_ADD_TIMING,
		WEATHER_STRAIN_ADD,
		WEATHER_SPARKRAIN_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	p_local_wk->work[0] = WEATHER_STRAIN_FOG_START;	// 同じくフォグ用
	p_local_wk->work[1] = 0;
	p_local_wk->work[2] = 0;							// 風カウンタ

	p_local_wk->work[4] = 39;							// 音カウンタ

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );


	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		雷雨フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

  if( p_local_wk->work[4] > 0 )
  {
    p_local_wk->work[4]--;
    if( p_local_wk->work[4] == 0 )
    {
      // 音
      WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_HIGHRAIN );	
    }
  }

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){
			WEATHER_TASK_FogFadeIn_Init( p_wk,
			WEATHER_FOG_SLOPE_DEFAULT, 
			WEATHER_FOG_DEPTH_DEFAULT + WEATHER_STRAIN_FOG_OFS, 
			WEATHER_STRAIN_FOG_TIMING,
			fog_cont );

      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat );

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
 *	@brief		雷雨フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );



	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_STRAIN_ADD_MAIN,	// obj登録数
		WEATHER_STRAIN_TIMING_MIN,// 登録タイミング
		WEATHER_STRAIN_ADD_MAIN,
		WEATHER_STRAIN_TIMING_MIN,
		-WEATHER_STRAIN_TIMING_ADD,
		WEATHER_STRAIN_ADD_TIMING,
		WEATHER_STRAIN_ADD,
		WEATHER_SPARKRAIN_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_SPARKRAIN_OBJ_Add, WEATHER_STRAIN_NOFADE_OBJ_START_NUM, WEATHER_STRAIN_NOFADE_OBJ_START_DUST_NUM, WEATHER_STRAIN_NOFADE_OBJ_START_DUST_MOVE, heapID );

	p_local_wk->work[1] = 0;
	p_local_wk->work[2] = 0;							// 風カウンタ

  // 音
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_HIGHRAIN );	

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		雷雨メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	p_local_wk->work[2] = (p_local_wk->work[2] + 1) % (WEATHER_STRAIN_OBJ_MUL_CHG*WEATHER_STRAIN_OBJ_MUL_NUM);		// 雨登録料変更カウンタ
	// カウンタが0いかになったら雨登録
	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );


	// スパークメイン
	WEATHER_PARK_Main( &p_local_wk->spark, p_wk );

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
 *	@brief		雷雨フェードアウト開始
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_STRAIN_TIMING_MAX,
			WEATHER_STRAIN_TIMING_ADD,
			WEATHER_STRAIN_ADD_END );
	
	p_local_wk->work[0] = WEATHER_STRAIN_FOG_START_END;	// 同じくフォグ用


	// 音
	WEATHER_TASK_StopLoopSnd( p_wk );	
	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		雷雨フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
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
					WEATHER_STRAIN_FOG_TIMING_END, fog_cont );
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
 *	@brief		雷雨破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_SPARKRAIN_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_SPARKRAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );

	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk );

	// スパーク初期化
	WEATHER_PARK_Exit( &p_local_wk->spark );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief		雷雨オブジェメイン
 */
//-----------------------------------------------------------------------------
static void WEATHER_SPARKRAIN_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	int i;
	GFL_CLACTPOS mat;
	GFL_CLWK* p_clwk;
	s32* obj_w;

	obj_w = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	WEATHER_OBJ_WORK_GetPos( p_wk, &mat );
	
	// 動作フラグをチェック
	switch(obj_w[3]){
	case 0:		// 動作
		// 動かす
		mat.x += (obj_w[4]*2);
		mat.y += (obj_w[2]*2);
	
		// 破棄するかチェック
		obj_w[0] += 2;
		if(obj_w[0] > obj_w[1]){
			// アニメーションさせるかチェック
			if(GFUser_GetPublicRand(10) < 5){		// 7/10は破棄
				// 破棄
				obj_w[3] = 2;
			}else{
			
				// 破棄アニメ
				obj_w[3] = 1;		
				obj_w[0] = 4;			// 破棄アニメカウンタ
				GFL_CLACT_WK_SetAnmIndex( p_clwk, 3 );
			}
		}
		// 座標設定
		WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &mat );
		break;
	case 1:		// 破棄アニメ
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
 *	@brief		雷雨オブジェ登録
 */
//-----------------------------------------------------------------------------
static void WEATHER_SPARKRAIN_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	WEATHER_SPARKRAIN_WORK*	p_local_wk;	// システムワーク
	int	err;	// 補正値
	s32* obj_w;	// オブジェクトワーク
	int speed_m;	// スピードにかける値テーブルの要素数
	int frame;		// フレーム数
	static const int WEATHER_STRAIN_OBJ_MUL[ WEATHER_STRAIN_OBJ_MUL_NUM ] = {1, 1, 2, 1, 3};
	u32 rand;
	GFL_CLACTPOS mat;	// 設定座標
	GFL_CLWK* p_clwk;

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
		frame = rand%3;	// 雨の種類
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );

		obj_w[4] = WEATHER_STRAIN_SPEED_X * (frame+1);
		obj_w[2] = WEATHER_STRAIN_SPEED_Y * (frame+1);
		obj_w[4] *= WEATHER_STRAIN_OBJ_MUL[p_local_wk->work[2]/WEATHER_STRAIN_OBJ_MUL_CHG];
		obj_w[2] *= WEATHER_STRAIN_OBJ_MUL[p_local_wk->work[2]/WEATHER_STRAIN_OBJ_MUL_CHG];
		
		obj_w[3] = 0;			// 破棄アニメフラグ
		
		// 破棄タイミング	全部共通の範囲の値
		obj_w[1] = WEATHER_STRAIN_END_MIN  + (rand % WEATHER_STRAIN_END_MAX);
		obj_w[1] /= WEATHER_STRAIN_OBJ_MUL[p_local_wk->work[2]/WEATHER_STRAIN_OBJ_MUL_CHG];
		
		// 座標を設定
		{
			mat.x = WEATHER_STRAIN_START_X + (rand % WEATHER_STRAIN_START_X_MAX);
			mat.y = WEATHER_STRAIN_START_Y + (rand % WEATHER_STRAIN_START_Y_MAX);

			WEATHER_OBJ_WORK_SetPosNoTurn( add_obj, &mat );
		}
		
	}

}



//----------------------------------------------------------------------------
/**
 *	@brief	雷処理	初期化
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_PARK_Init( WEATHER_SPARK_WORK* p_wk )
{
	GFL_STD_MemClear( p_wk, sizeof(WEATHER_SPARK_WORK) );

	p_wk->wait = GFUser_GetPublicRand( WEATHER_SPARK_INIT_RAND );
}

//----------------------------------------------------------------------------
/**
 *	@brief	雷処理　破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_PARK_Exit( WEATHER_SPARK_WORK* p_wk )
{
	GFL_STD_MemClear( p_wk, sizeof(WEATHER_SPARK_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	p_wk	ワーク
 *	@param	p_sys	天気システムワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_PARK_Main( WEATHER_SPARK_WORK* p_wk, WEATHER_TASK* p_sys )
{
#ifndef WEATHER_SPARK_COLOR_BLUE
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		blue --;
		OS_TPrintf( "blue %d\n", blue );
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		blue ++;
		OS_TPrintf( "blue %d\n", blue );
	}
#endif

	switch( p_wk->seq ){
	case WEATHER_SPARK_START_WAIT:
		p_wk->wait--;
		if( p_wk->wait < 0 ){
			p_wk->seq = WEATHER_SPARK_SETUP;
		}
		break;

	case WEATHER_SPARK_SETUP:
		{
			u32 rand;
			int i;

			rand = GFUser_GetPublicRand( WEATHER_SPARK_TYPE_RAND_MAX );
				
			// 雷選択
			for( i=0; i<WEATHER_SPARK_TYPE_MAX; i++ ){
				if( sc_WEATHER_SPARK_TYPE_RAND[i] >= rand ){
					break;
				}
			}
			GF_ASSERT( i<WEATHER_SPARK_TYPE_MAX );

//			i = WEATHER_SPARK_TYPE_LIGHT_HARD;
//			OS_TPrintf( "spark_type %d\n", i );

			// それぞれのタイプごとに初期化
			switch( i ){
			case WEATHER_SPARK_TYPE_LIGHT:
				WEATEHR_SPARK_SetUp_Light( p_wk );
				break;
			case WEATHER_SPARK_TYPE_HARD:
				WEATEHR_SPARK_SetUp_Hard( p_wk );
				break;
			case WEATHER_SPARK_TYPE_LIGHT_HARD:
				WEATEHR_SPARK_SetUp_LightAndHard( p_wk );
				break;
			}

			p_wk->spark_tbl_count	= 0;
			p_wk->seq				= WEATHER_SPARK_SPARK;
		}
		break;

	case WEATHER_SPARK_SPARK:	// 雷
		WEATHER_TASK_LIGHT_StartColorFade( p_sys, WEATHER_SPARK_GetColor(p_wk), p_wk->spark_data[ p_wk->spark_tbl_count ].insync, p_wk->spark_data[ p_wk->spark_tbl_count ].outsync );
		// LIGHT->HARDの場合、フェード中に、雷発射してしまう
		if( p_wk->spark_type == WEATHER_SPARK_TYPE_LIGHT_HARD ){
			p_wk->wait = (p_wk->spark_data[ p_wk->spark_tbl_count ].insync/2) + GFUser_GetPublicRand( p_wk->spark_data[ p_wk->spark_tbl_count ].outsync );
		}

    p_wk->snd_wait = p_wk->spark_data[ p_wk->spark_tbl_count ].snd_wait;
    p_wk->snd_se = p_wk->spark_data[ p_wk->spark_tbl_count ].snd_se;

//		OS_TPrintf( "color %d\n", p_wk->spark_data[ p_wk->spark_tbl_count ].spark_power );
		
		p_wk->seq	= WEATHER_SPARK_SPARK_WAIT;
		break;

	case WEATHER_SPARK_SPARK_WAIT:	// 雷

    // SEの再生
    if(p_wk->snd_wait > 0)
    {
      p_wk->snd_wait --;
      if( p_wk->snd_wait == 0 )
      {
        // 雷SE再生
        PMSND_PlaySE( p_wk->snd_se );
      }
    }

		if( !WEATHER_TASK_LIGHT_IsColorFade( p_sys ) ){
			p_wk->seq	= WEATHER_SPARK_WAIT;
		}
		break;

	case WEATHER_SPARK_WAIT:
    // SEの再生
    if(p_wk->snd_wait > 0)
    {
      p_wk->snd_wait --;
      if( p_wk->snd_wait == 0 )
      {
        // 雷SE再生
        PMSND_PlaySE( p_wk->snd_se );
      }
    }
		
		p_wk->spark_data[ p_wk->spark_tbl_count ].wait --;
		if( p_wk->spark_data[ p_wk->spark_tbl_count ].wait <= 0 ){
			// 次のデータがあるなら次のデータへ
			if( p_wk->spark_tbl_num > (p_wk->spark_tbl_count+1) ){
				p_wk->spark_tbl_count ++;
				p_wk->seq = WEATHER_SPARK_SPARK;
			}else{

				// セットアップへ
				p_wk->seq = WEATHER_SPARK_SETUP;
			}
		}
		
		break;

	default:
		break;
	}
}




//----------------------------------------------------------------------------
/**
 *	@brief	軽い雷設定
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATEHR_SPARK_SetUp_Light( WEATHER_SPARK_WORK* p_wk )
{
	int i;
	
	p_wk->spark_type		= WEATHER_SPARK_TYPE_LIGHT;
	p_wk->spark_tbl_num		= WEATHER_SPARK_LIGHT_NUM_MIN + GFUser_GetPublicRand( WEATHER_SPARK_LIGHT_NUM_MAX );

	GF_ASSERT( p_wk->spark_tbl_num < WEATHER_SPARK_TBL_MAX );

	for( i=0; i<p_wk->spark_tbl_num; i++ ){
		p_wk->spark_data[i].spark_power = sc_WEATHER_SPARK_STRONG_RAND_MIN[ WEATHER_SPARK_TYPE_LIGHT ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_STRONG_RAND_MAX[ WEATHER_SPARK_TYPE_LIGHT ] );
		p_wk->spark_data[i].insync		= WEATHER_SPARK_FLASHIN_SYNC;
		p_wk->spark_data[i].outsync		= sc_WEATHER_SPARK_FLASHOUT_SYNC[ p_wk->spark_data[i].spark_power ];
		p_wk->spark_data[i].wait		= sc_WEATHER_SPARK_TYPE_RAND_WAIT_MIN[ WEATHER_SPARK_TYPE_LIGHT ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_TYPE_RAND_WAIT_MAX[ WEATHER_SPARK_TYPE_LIGHT ] );
    p_wk->spark_data[i].snd_wait = sc_WEATHER_SPARK_SND_WAIT[ WEATHER_SPARK_TYPE_LIGHT ];
    p_wk->spark_data[i].snd_se    = WEATHER_SND_SE_SPARK_SML;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	重い雷設定
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATEHR_SPARK_SetUp_Hard( WEATHER_SPARK_WORK* p_wk )
{

	p_wk->spark_type		= WEATHER_SPARK_TYPE_HARD;
	p_wk->spark_tbl_num		= 1;

	p_wk->spark_data[0].spark_power = sc_WEATHER_SPARK_STRONG_RAND_MIN[ WEATHER_SPARK_TYPE_HARD ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_STRONG_RAND_MAX[ WEATHER_SPARK_TYPE_HARD ] );
	p_wk->spark_data[0].insync		= WEATHER_SPARK_FLASHIN_SYNC;
	p_wk->spark_data[0].outsync		= sc_WEATHER_SPARK_FLASHOUT_SYNC[ p_wk->spark_data[0].spark_power ];
	p_wk->spark_data[0].wait		= sc_WEATHER_SPARK_TYPE_RAND_WAIT_MIN[ WEATHER_SPARK_TYPE_HARD ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_TYPE_RAND_WAIT_MAX[ WEATHER_SPARK_TYPE_HARD ] );
  p_wk->spark_data[0].snd_wait = sc_WEATHER_SPARK_SND_WAIT[ WEATHER_SPARK_TYPE_HARD ];
  p_wk->spark_data[0].snd_se    = WEATHER_SND_SE_SPARK;
}

//----------------------------------------------------------------------------
/**
 *	@brief	軽い->重い雷設定
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATEHR_SPARK_SetUp_LightAndHard( WEATHER_SPARK_WORK* p_wk )
{
	int i;
	
	p_wk->spark_type		= WEATHER_SPARK_TYPE_LIGHT_HARD;
	p_wk->spark_tbl_num		= WEATHER_SPARK_LIGHT_NUM_MIN + GFUser_GetPublicRand( WEATHER_SPARK_LIGHT_NUM_MAX );
	p_wk->spark_tbl_num		++;	// 最後の一発

	GF_ASSERT( p_wk->spark_tbl_num < WEATHER_SPARK_TBL_MAX );

	for( i=0; i<p_wk->spark_tbl_num-1; i++ ){
		p_wk->spark_data[i].spark_power = sc_WEATHER_SPARK_STRONG_RAND_MIN[ WEATHER_SPARK_TYPE_LIGHT ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_STRONG_RAND_MAX[ WEATHER_SPARK_TYPE_LIGHT ] );
		p_wk->spark_data[i].insync		= WEATHER_SPARK_FLASHIN_SYNC;
		p_wk->spark_data[i].outsync		= WEATHER_SPARK_LIGHT_HARD_OUTSYNC;
		p_wk->spark_data[i].wait		= GFUser_GetPublicRand( WEATHER_SPARK_LIGHT_HARD_WAIT_RAND );
    p_wk->spark_data[i].snd_wait = sc_WEATHER_SPARK_SND_WAIT[ WEATHER_SPARK_TYPE_LIGHT_HARD ];
    p_wk->spark_data[i].snd_se    = WEATHER_SND_SE_SPARK_SML;
	}

	p_wk->spark_data[i].spark_power = sc_WEATHER_SPARK_STRONG_RAND_MIN[ WEATHER_SPARK_TYPE_LIGHT_HARD ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_STRONG_RAND_MAX[ WEATHER_SPARK_TYPE_LIGHT_HARD ] );
	p_wk->spark_data[i].insync		= WEATHER_SPARK_FLASHIN_SYNC;
	p_wk->spark_data[i].outsync		= sc_WEATHER_SPARK_FLASHOUT_SYNC[ p_wk->spark_data[i].spark_power ];
	p_wk->spark_data[i].wait		= sc_WEATHER_SPARK_TYPE_RAND_WAIT_MIN[ WEATHER_SPARK_TYPE_LIGHT_HARD ] + GFUser_GetPublicRand( sc_WEATHER_SPARK_TYPE_RAND_WAIT_MAX[ WEATHER_SPARK_TYPE_LIGHT_HARD ] );
  p_wk->spark_data[i].snd_wait = sc_WEATHER_SPARK_SND_WAIT[ WEATHER_SPARK_TYPE_LIGHT_HARD ];
  p_wk->spark_data[i].snd_se    = WEATHER_SND_SE_SPARK;
}


//----------------------------------------------------------------------------
/**
 *	@brief	雷色の取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	雷色
 */
//-----------------------------------------------------------------------------
static GXRgb WEATHER_SPARK_GetColor( const WEATHER_SPARK_WORK* cp_wk )
{
	u32 power;

	power = cp_wk->spark_data[ cp_wk->spark_tbl_count ].spark_power;

#ifndef WEATHER_SPARK_COLOR_BLUE
	return GX_RGB( power, power, blue );
#else
	return GX_RGB( power, power, WEATHER_SPARK_COLOR_BLUE );
#endif
}


