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
#define WEATHER_RAIN_SPEED_X		(8)						// 横に進むスピード
#define WEATHER_RAIN_SPEED_Y		(16)						// たてに進むスピードベース
#define	WEATHER_RAIN_END_MIN		(1)							// 終了カウンタ最小
#define WEATHER_RAIN_END_MAX		(2)							// 終了カウンタ最大
#define	WEATHER_RAIN_START_X_BASE	(-72)							// ベースになるX開始座標
#define	WEATHER_RAIN_MUL_X			(15)						// ベースに雨の種類分足す値
#define	WEATHER_RAIN_START_X_MAX	(256)						// X開始座標乱数の最大値
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
#define WEATHER_STRAIN_SPEED_X		(20)						// 横に進むスピード
#define WEATHER_STRAIN_SPEED_Y		(20)							// たてに進むスピードベース
#define	WEATHER_STRAIN_END_MIN		(1)							// 終了カウンタ最小
#define WEATHER_STRAIN_END_MAX		(2)							// 終了カウンタ最大
#define	WEATHER_STRAIN_START_X		(-256)							// ベースになるX開始座標
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
	WEATHER_SPARK_SPARK_DARK,		// 雷
	WEATHER_SPARK_SPARK_DARK_WAIT,		// 雷
	WEATHER_SPARK_SPARK,		// 雷
	WEATHER_SPARK_SPARK_WAIT,	// 雷
	WEATHER_SPARK_WAIT,
};

// 雷モード
enum
{
  WEATHER_SPARK_MODE_NORMAL,
  WEATHER_SPARK_MODE_RAIKAMI,
  WEATHER_SPARK_MODE_MAX,
};

// ライカミ　雷
#define WEATHER_SPARK_DARK_COLOR   ( GX_RGB(6,6,10) )
#define WEATHER_SPARK_DARK_SYNC   ( 4 )
#define WEATHER_SPARK_DARK_WAIT   ( 10 )

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



//-------------------------------------
///	ライカミ　天気
//=====================================
#define	WEATHER_RAIKAMI_TIMING_MIN		(0)					// 雨を出すタイミング最小
#define WEATHER_RAIKAMI_TIMING_MAX		(15)				// 雨を出すタイミング最大
#define WEATHER_RAIKAMI_TIMING_ADD		(3)					// タイミングを減らす数
#define WEATHER_RAIKAMI_ADD_START		(1)					// 最初の同時に雨を登録する数
#define WEATHER_RAIKAMI_ADD_TIMING		(1)					// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_RAIKAMI_ADD			(1)						// 登録する数を増やす数
#define WEATHER_RAIKAMI_ADD_END		(-1)					// 登録する数を増やす数
#define	WEATHER_RAIKAMI_ADD_MAIN		(2)					// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_RAIKAMI_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_RAIKAMI_NOFADE_OBJ_START_DUST_NUM	( 5 )		// 何個ずつ動作数を変更するか
#define	WEATHER_RAIKAMI_NOFADE_OBJ_START_DUST_MOVE	( 1 )		// ずらす動作数の値

/*== フォグ ==*/
#define	WEATHER_RAIKAMI_FOG_TIMING		(80)						// に１回フォグテーブルを操作
#define	WEATHER_RAIKAMI_FOG_TIMING_END	(50)						// に１回フォグテーブルを操作
#define WEATHER_RAIKAMI_FOG_START_END	(1)					// このカウント動いてからフォグテーブルを操作
#define WEATHER_RAIKAMI_FOG_OFS			(0x300)

/*== 雨オブジェクト ==*/
#define WEATHER_RAIKAMI_SPPED_RND (16)
#define WEATHER_RAIKAMI_SPEED_X		(14)						// 横に進むスピード
#define WEATHER_RAIKAMI_SPEED_Y		(14)							// たてに進むスピードベース
#define	WEATHER_RAIKAMI_END_MIN		(10)							// 終了カウンタ最小
#define WEATHER_RAIKAMI_END_MAX		(10)							// 終了カウンタ最大
#define	WEATHER_RAIKAMI_START_X		(-128)							// ベースになるX開始座標
#define	WEATHER_RAIKAMI_START_X_MAX	(384)						// X開始座標乱数値
#define	WEATHER_RAIKAMI_START_Y		(-80)						// Y開始座標
#define	WEATHER_RAIKAMI_START_Y_MAX	(40)						// Y開始座標乱数値

#define WEATHER_RAIKAMI_OBJ_MUL_NUM	(5)							// オブジェのスピードを変化させる値
#define WEATHER_RAIKAMI_OBJ_MUL_CHG	(60)						// 変更タイミング
#define WEATHER_RAIKAMI_OBJ_MUL_ONE ( 10 )

#define WEATHER_RAIKAMI_OBJ_END_Y ( 192 )

//  フェードインの処理
enum
{
  WEATHER_RAIKAMI_FADEIN_SEQ_INIT,
  WEATHER_RAIKAMI_FADEIN_SEQ_LIGHT_DARK,
  WEATHER_RAIKAMI_FADEIN_SEQ_SPARK_00,
  WEATHER_RAIKAMI_FADEIN_SEQ_SPARK_01,
  WEATHER_RAIKAMI_FADEIN_SEQ_SPARK_02,
  WEATHER_RAIKAMI_FADEIN_SEQ_OBJADD,

};

#define WEATHER_RAIKAMI_SPARK_DARK_COLOR   ( GX_RGB(4,4,8) )
#define WEATHER_RAIKAMI_SPARK_DARK_SYNC   ( 8 )
#define WEATHER_RAIKAMI_SPARK_COLOR ( GX_RGB( 30,30,8 ) )
#define WEATHER_RAIKAMI_SPARK_INSYNC ( 4 )
#define WEATHER_RAIKAMI_SPARK_OUTSYNC ( 60 )

#define WEATHER_RAIKAMI_LIGHT_SPARK_WAIT ( 90 )
#define WEATHER_RAIKAMI_LIGHT_DARKSPARK_WAIT ( 16 )

#define WEATHER_RAIKAMI_LIGHT_FULL_RAIN_WAIT ( 48 )





//-------------------------------------
///	カザカミ　天気
//=====================================
// フェードインの流れ
enum
{
  KAZAKAMI_FADEIN_SEQ_INIT,
  KAZAKAMI_FADEIN_SEQ_LIGHT_DARK,
  KAZAKAMI_FADEIN_SEQ_WIND02,
};
#define KAZAKAMI_FADEIN_WIND_WAIT ( WEATHER_RAIKAMI_LIGHT_SPARK_WAIT ) // 暗くなりだしてから風が出てくるまでの間
#define KAZAKAMI_FADEIN_BGSTART_WAIT (8)


#define	WEATHER_KAZAKAMI_TIMING_MIN		(1)					// 雨を出すタイミング最小
#define WEATHER_KAZAKAMI_TIMING_MAX		(4)				// 雨を出すタイミング最大
#define WEATHER_KAZAKAMI_TIMING_ADD		(4)					// タイミングを減らす数
#define WEATHER_KAZAKAMI_ADD_START		(1)					// 最初の同時に雨を登録する数
#define WEATHER_KAZAKAMI_ADD_TIMING		(1)					// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_KAZAKAMI_ADD			(1)						// 登録する数を増やす数
#define WEATHER_KAZAKAMI_ADD_END		(-1)					// 登録する数を増やす数
#define	WEATHER_KAZAKAMI_ADD_MAIN		(1)					// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_KAZAKAMI_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_KAZAKAMI_NOFADE_OBJ_START_DUST_NUM	( 5 )		// 何個ずつ動作数を変更するか
#define	WEATHER_KAZAKAMI_NOFADE_OBJ_START_DUST_MOVE	( 1 )		// ずらす動作数の値

/*== フォグ ==*/
#define	WEATHER_KAZAKAMI_FOG_TIMING		(10)						// に１回フォグテーブルを操作
#define	WEATHER_KAZAKAMI_FOG_TIMING_END	(50)						// に１回フォグテーブルを操作
#define WEATHER_KAZAKAMI_FOG_OFS			(0x180)
#define WEATHER_KAZAKAMI_FOG_OFS_NORMAL			(0x300)

enum
{
  KAZAKAMI_WIND_SCENE_NONE,
  KAZAKAMI_WIND_SCENE_RAIN,
  KAZAKAMI_WIND_SCENE_WINDRAIN,
  
  KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN,
  KAZAKAMI_WIND_SCENE_WINDRAIN_TO_RAIN,

  KAZAKAMI_WIND_SCENE_MAX,
};
#define KAZAKAMI_WIND_WINDRAIN_PARCENT  ( 50 )
#define KAZAKAMI_RAIN_RAIN_TO_WINDRAIN_PARCENT  ( 80 )

#define KAZAKAMI_FIRST_RAIN_TIME  ( 120 )

/*== 雨オブジェクト ==*/
enum
{
  KAZAKAMI_OBJ_WIND,      // 風表現オブジェクト
  KAZAKAMI_OBJ_RAIN,      // 雨表現オブジェクト
  KAZAKAMI_OBJ_WIND_RAIN, // 風雨表現オブジェクト

  KAZAKAMI_OBJ_TYPE_NUM,  // オブジェクトタイプ数
};

// 風スピード　基準値
#define KAZAKAMI_OBJ_WIND_SPEED_DEF ( 8 ) // 8=1
#define KAZAKAMI_OBJ_WIND_SPEED_GET( x ) ((x) / KAZAKAMI_OBJ_WIND_SPEED_DEF)
#define KAZAKAMI_OBJ_WIND_SPEED_SET( x ) ((x) * KAZAKAMI_OBJ_WIND_SPEED_DEF)

// 切り替わり風SE再生タイミング
#define KAZAKAMI_WIND_SCENE_CHANGE_SE_PLAY_TIMING ( 24 )

// 暴風の風ループタイミング
#define KAZAKAMI_WIND_SE_LOOP_TIME_MIN  (50)
#define KAZAKAMI_WIND_SE_LOOP_TIME_DIF  (16)
#define KAZAKAMI_WIND_SE_LOOP_TIME      (KAZAKAMI_WIND_SE_LOOP_TIME_MIN)

#define KAZAKAMI_WIND_SE_LOOP_VOLUME_MIN  ( 50 )
#define KAZAKAMI_WIND_SE_LOOP_VOLUME_DIF  ( 100 - KAZAKAMI_WIND_SE_LOOP_VOLUME_MIN )

// 風移り代わり情報
#define KAZAKAMI_WIND_SCENE_RAIN_TIME_MIN ( 60 )
#define KAZAKAMI_WIND_SCENE_RAIN_TIME_RND ( 200 )

#define KAZAKAMI_WIND_SCENE_WINDRAIN_TIME_MIN ( 60 )
#define KAZAKAMI_WIND_SCENE_WINDRAIN_TIME_RND ( 400 )
#define KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_TIME_MIN ( 16 )

#define KAZAKAMI_WIND_SCENE_RAIN_WIND_SPEED_X (0)
#define KAZAKAMI_WIND_SCENE_RAIN_WIND_SPEED_Y (0)
#define KAZAKAMI_WIND_SCENE_RAIN_WINDON_SPEED_X (KAZAKAMI_OBJ_WIND_SPEED_SET(20))
#define KAZAKAMI_WIND_SCENE_RAIN_WINDON_SPEED_Y (KAZAKAMI_OBJ_WIND_SPEED_SET(2))
#define KAZAKAMI_WIND_SCENE_RAIN_WIND_ON_TIME ( 9 )

#define KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_WIND_SPEED_X (KAZAKAMI_OBJ_WIND_SPEED_SET(20))
#define KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_WIND_SPEED_Y (KAZAKAMI_OBJ_WIND_SPEED_SET(2))
#define KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_WIND_SCALE_X (0x800)

#define KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_TBL_NUM (6)
#define KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_CHANGE_TIME (12 * KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_TBL_NUM) // テーブルすべてを変更する間
static const VecFx32 KAZAKAMI_WIND_SCENE_RAIN_RAINWIND_SPEED_TBL[ KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_TBL_NUM ] = 
{
  { KAZAKAMI_OBJ_WIND_SPEED_SET(26), KAZAKAMI_OBJ_WIND_SPEED_SET(3),}, 
  { KAZAKAMI_OBJ_WIND_SPEED_SET(36), KAZAKAMI_OBJ_WIND_SPEED_SET(2),},
  { KAZAKAMI_OBJ_WIND_SPEED_SET(30), KAZAKAMI_OBJ_WIND_SPEED_SET(3),},
  { KAZAKAMI_OBJ_WIND_SPEED_SET(38), KAZAKAMI_OBJ_WIND_SPEED_SET(3),},
  { KAZAKAMI_OBJ_WIND_SPEED_SET(24), KAZAKAMI_OBJ_WIND_SPEED_SET(3),},
  { KAZAKAMI_OBJ_WIND_SPEED_SET(32), KAZAKAMI_OBJ_WIND_SPEED_SET(2),},
};

// 背景のスケール値
static const fx32 KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_TBL[] = 
{
  0x1100, 
  0x1200, 
  0x1300, 
  0x1300, 
  0x1300, 
  0x1200, 
  0x1100, 
  0x1000, 
  0x1000, 
  0x1200, 
  0x1300, 
  0x1200, 
  0x1200, 
  0x1100, 
};
#define KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_NUM ( NELEMS(KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_TBL) )
#define KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_CHANGE_TIME ( 16*KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_NUM )


// WINDRAINの時には、OBJの登録割合を変更する
#define KAZAKAMI_WIND_SCENE_WIND_ADD_CUT_NUM ( 2 )


// 風表現オブジェ
#define KAZAKAMI_OBJ_WIND_DELETE_X  ( 256 )
#define KAZAKAMI_OBJ_WIND_DELETE_Y  ( 192 )

#define KAZAKAMI_OBJ_WIND_SPEED_X_MIN ( 16 )
#define KAZAKAMI_OBJ_WIND_SPEED_X_RND ( 24 )
#define KAZAKAMI_OBJ_WIND_SPEED_Y_MIN ( 8 )
#define KAZAKAMI_OBJ_WIND_SPEED_Y_RND ( 24 )

#define KAZAKAMI_OBJ_WIND_ADD_X ( -8 )
#define KAZAKAMI_OBJ_WIND_ADD_X_RND ( 8 )
#define KAZAKAMI_OBJ_WIND_ADD_Y_MIN ( -16 )
#define KAZAKAMI_OBJ_WIND_ADD_Y_RND ( 160 )


// 風雨表現オブジェ
#define KAZAKAMI_OBJ_WINDRAIN_SPEED_X_MIN ( 80 )
#define KAZAKAMI_OBJ_WINDRAIN_SPEED_X_RND ( 24 )
#define KAZAKAMI_OBJ_WINDRAIN_SPEED_Y_MIN ( 14 )
#define KAZAKAMI_OBJ_WINDRAIN_SPEED_Y_RND ( 24 )
#define KAZAKAMI_OBJ_WINDRAIN_ADD_X_MIN ( -8 )
#define KAZAKAMI_OBJ_WINDRAIN_ADD_X_RND ( 8 )
#define KAZAKAMI_OBJ_WINDRAIN_ADD_Y_MIN ( -16 )
#define KAZAKAMI_OBJ_WINDRAIN_ADD_Y_RND ( 160 )

// 雨表現オブジェ
#define KAZAKAMI_OBJ_RAIN_SPEED_X_MIN ( 86 ) // 8=1
#define KAZAKAMI_OBJ_RAIN_SPEED_RND ( 43 ) // 8=1
#define KAZAKAMI_OBJ_RAIN_SPEED_Y_MIN ( 258 )
#define KAZAKAMI_OBJ_RAIN_SPEED_Y_RND_MUL ( 2 )
#define KAZAKAMI_OBJ_RAIN_ADD_X_MIN ( -80 )
#define KAZAKAMI_OBJ_RAIN_ADD_X_RND ( 336 )
#define KAZAKAMI_OBJ_RAIN_ADD_Y_MIN ( -80 )
#define KAZAKAMI_OBJ_RAIN_ADD_Y_RND ( 40 )

#define KAZAKAMI_OBJ_RAIN_DELETE_X  ( 256 )
#define KAZAKAMI_OBJ_RAIN_DELETE_Y  ( 160 )


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
	u8 seq;
  u8 mode;
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



//-------------------------------------
///	ライカミ
//=====================================
typedef struct {
  s16 seq;
  s16 wait;

  s32 scroll;
  s32 obj_speed_change;
  

	WEATHER_SPARK_WORK	spark;
} WEATHER_RAIKAMI_WORK;




//-------------------------------------
///	カザカミ
//=====================================
typedef struct {
  s16 seq;
  s16 wait;

  s16 scroll_x;
  s16 scroll_y;
  s16 scroll_back_x;
  s16 scroll_back_y;
  s16 wind_scene_count;
  s16 wind_scene_count_max;
  s16 wind_scene_type;
  s16 wind_se_count;
  s16 wind_se_count_max;

  s16 wind_add_x; // 風つよさ（加速値）
  s16 wind_add_y;
  fx32 wind_bg_back_scale_x;

} WEATHER_KAZAKAMI_WORK;


//-------------------------------------
/// カザカミ　風表現　オブジェ	
//=====================================
typedef struct 
{
  s16 pos_x;
  s16 pos_y;
  s16 speed_x;
  s16 speed_y;
  s16 last_speed_x;
  s16 last_speed_y;
} KAZAKAMI_OBJ_WIND_WK;

//-------------------------------------
/// カザカミ　雨表現　オブジェ	
//=====================================
typedef struct 
{
  s16 pos_x;
  s16 pos_y;
  s16 speed_x;
  s16 speed_y;
} KAZAKAMI_OBJ_RAIN_WK;

//-------------------------------------
/// カザカミ　風雨表現　オブジェ	
//=====================================
typedef struct 
{
  s16 pos_x;
  s16 pos_y;
  s16 speed_x;
  s16 speed_y;
  s16 last_speed_x;
  s16 last_speed_y;
} KAZAKAMI_OBJ_WINDRAIN_WK;

//-------------------------------------
///	カザカミ　オブジェワーク
//=====================================
typedef struct 
{
  u32 type;
  const WEATHER_KAZAKAMI_WORK* cp_parent;

  union
  {
    KAZAKAMI_OBJ_WIND_WK      wind; 
    KAZAKAMI_OBJ_RAIN_WK      rain; 
    KAZAKAMI_OBJ_WINDRAIN_WK  windrain; 
  }objwk;
} KAZAKAMI_OBJ_WK;

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
///	カミシリーズ　ライカミ
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_RAIKAMI_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_RAIKAMI_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 

static void WEATHER_RAIKAMI_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_RAIKAMI_WORK* p_wk );


//-------------------------------------
///	カミシリーズ　カザカミ
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_KAZAKAMI_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_KAZAKAMI_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 

static void WEATHER_KAZAKAMI_OBJ_WIND_Move( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ); 
static void WEATHER_KAZAKAMI_OBJ_WIND_Add( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ); 
static void WEATHER_KAZAKAMI_OBJ_RAIN_Move( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ); 
static void WEATHER_KAZAKAMI_OBJ_RAIN_Add( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ); 
static void WEATHER_KAZAKAMI_OBJ_WINDRAIN_Move( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ); 
static void WEATHER_KAZAKAMI_OBJ_WINDRAIN_Add( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ); 


static void WEATHER_KAZAKAMI_WindStartRain( WEATHER_TASK* p_sys, WEATHER_KAZAKAMI_WORK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, BOOL fadeout );
static void WEATHER_KAZAKAMI_WindControl( WEATHER_TASK* p_sys, WEATHER_KAZAKAMI_WORK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, BOOL fadeout );
static void WEATHER_KAZAKAMI_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_KAZAKAMI_WORK* p_wk );
static void WEATHER_KAZAKAMI_WindSePlay( const WEATHER_KAZAKAMI_WORK* cp_wk, BOOL fog_cont, u16 se_no );
static void WEATHER_KAZAKAMI_WINDRAIN_SCENE_InitWindSePlay( WEATHER_KAZAKAMI_WORK* p_wk );
static void WEATHER_KAZAKAMI_WINDRAIN_SCENE_WindSePlay( WEATHER_KAZAKAMI_WORK* p_wk, BOOL fog_cont );


//-------------------------------------
///	雷
//=====================================
static void WEATHER_PARK_Init( WEATHER_SPARK_WORK* p_wk, u8 mode );
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
	WEATHER_TASK_3DBG_USE_NONE,		// BGを使用するか？
	NARC_field_weather_rain_NCGR,			// OAM CG
	NARC_field_weather_rain_NCLR,			// OAM PLTT
	NARC_field_weather_rain_NCER,			// OAM CELL
	NARC_field_weather_rain_NANR,			// OAM CELLANM
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
	WEATHER_TASK_3DBG_USE_NONE,		// BGを使用するか？
	NARC_field_weather_rain_st_NCGR,			// OAM CG
	NARC_field_weather_rain_NCLR,			// OAM PLTT
	NARC_field_weather_rain_st_NCER,			// OAM CELL
	NARC_field_weather_rain_st_NANR,			// OAM CELLANM
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

// ライカミ
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIKAMI = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	WEATHER_TASK_3DBG_USE_FRONT,		// BGを使用するか？
	NARC_field_weather_rain_raikami_NCGR,			// OAM CG
	NARC_field_weather_rain_NCLR,			// OAM PLTT
	NARC_field_weather_rain_raikami_NCER,			// OAM CELL
	NARC_field_weather_rain_raikami_NANR,			// OAM CELLANM
  {
    {
      NARC_field_weather_rain_bg_nsbtx,		// BGTEX
      GX_TEXSIZE_S32,		// GXTexSizeS
      GX_TEXSIZE_T32,		// GXTexSizeT
      GX_TEXREPEAT_ST,		// GXTexRepeat
      GX_TEXFLIP_NONE,		// GXTexFlip
      GX_TEXFMT_PLTT4,		// GXTexFmt
      GX_TEXPLTTCOLOR0_TRNS,		// GXTexPlttColor0
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
	sizeof(WEATHER_RAIKAMI_WORK),

	// 管理関数
	WEATHER_RAIKAMI_Init,		// 初期化
	WEATHER_RAIKAMI_FadeIn,		// フェードイン
	WEATHER_RAIKAMI_NoFade,		// フェードなし
	WEATHER_RAIKAMI_Main,		// メイン処理
	WEATHER_RAIKAMI_InitFadeOut,	// フェードアウト
	WEATHER_RAIKAMI_FadeOut,		// フェードアウト
	WEATHER_RAIKAMI_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_RAIKAMI_OBJ_Move,
};

// かざカミ
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_KAZAKAMI = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	WEATHER_TASK_3DBG_USE_ALL,		// BGを使用するか？
	NARC_field_weather_kazakami_NCGR,			// OAM CG
	NARC_field_weather_kazakami_NCLR,			// OAM PLTT
	NARC_field_weather_kazakami_NCER,			// OAM CELL
	NARC_field_weather_kazakami_NANR,			// OAM CELLANM
  {
    {
      NARC_field_weather_kazakami_rain_nsbtx,		// BGTEX
      GX_TEXSIZE_S32,		// GXTexSizeS
      GX_TEXSIZE_T32,		// GXTexSizeT
      GX_TEXREPEAT_ST,		// GXTexRepeat
      GX_TEXFLIP_NONE,		// GXTexFlip
      GX_TEXFMT_PLTT4,		// GXTexFmt
      GX_TEXPLTTCOLOR0_TRNS,		// GXTexPlttColor0
    },
    {
	    NARC_field_weather_kazakami_nsbtx,		// BGTEX
      GX_TEXSIZE_S32,		// GXTexSizeS
      GX_TEXSIZE_T32,		// GXTexSizeT
      GX_TEXREPEAT_ST,		// GXTexRepeat
      GX_TEXFLIP_NONE,		// GXTexFlip
      GX_TEXFMT_PLTT4,		// GXTexFmt
      GX_TEXPLTTCOLOR0_TRNS,		// GXTexPlttColor0
    },
  },

	// ワークサイズ
	sizeof(WEATHER_KAZAKAMI_WORK),

	// 管理関数
	WEATHER_KAZAKAMI_Init,		// 初期化
	WEATHER_KAZAKAMI_FadeIn,		// フェードイン
	WEATHER_KAZAKAMI_NoFade,		// フェードなし
	WEATHER_KAZAKAMI_Main,		// メイン処理
	WEATHER_KAZAKAMI_InitFadeOut,	// フェードアウト
	WEATHER_KAZAKAMI_FadeOut,		// フェードアウト
	WEATHER_KAZAKAMI_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_KAZAKAMI_OBJ_Move,
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
	WEATHER_PARK_Init( &p_local_wk->spark, WEATHER_SPARK_MODE_NORMAL );

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
 *	@brief  ライカミ　天気  初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_RAIKAMI_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// スパーク初期化
	WEATHER_PARK_Init( &p_local_wk->spark, WEATHER_SPARK_MODE_RAIKAMI );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk, 
		WEATHER_RAIKAMI_ADD_START,	// obj登録数
		WEATHER_RAIKAMI_TIMING_MAX,// 登録タイミング
		WEATHER_RAIKAMI_ADD_MAIN,
		WEATHER_RAIKAMI_TIMING_MIN,
		-WEATHER_RAIKAMI_TIMING_ADD,
		WEATHER_RAIKAMI_ADD_TIMING,
		WEATHER_RAIKAMI_ADD,
		WEATHER_RAIKAMI_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

  p_local_wk->wait      = 0;
  p_local_wk->seq      = 0;

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );


	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライカミ　フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_RAIKAMI_WORK* p_local_wk;
  BOOL result;
  BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
  
  switch( p_local_wk->seq )
  {
  case WEATHER_RAIKAMI_FADEIN_SEQ_INIT:
    // ライト変更
    WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_raikami_dat );

    p_local_wk->seq++;
    p_local_wk->wait = WEATHER_RAIKAMI_LIGHT_SPARK_WAIT;
    break;

  case WEATHER_RAIKAMI_FADEIN_SEQ_LIGHT_DARK:
    p_local_wk->wait--;
    if(p_local_wk->wait < 0)
    {
      p_local_wk->seq++;
    }
    break;

  case WEATHER_RAIKAMI_FADEIN_SEQ_SPARK_00:
    // ドデカイ雷を鳴らす。（LIGHTを暗くしてから）
		WEATHER_TASK_LIGHT_StartColorFadeOneWay( p_wk, WEATHER_RAIKAMI_SPARK_DARK_COLOR, WEATHER_RAIKAMI_SPARK_DARK_SYNC );
    p_local_wk->wait = WEATHER_RAIKAMI_LIGHT_DARKSPARK_WAIT;
    p_local_wk->seq++;
    break;

  case WEATHER_RAIKAMI_FADEIN_SEQ_SPARK_01:
    p_local_wk->wait --;
    if( p_local_wk->wait < 0 )
    {
      // ドカーンといく
		  WEATHER_TASK_LIGHT_StartColorFade( p_wk, WEATHER_RAIKAMI_SPARK_COLOR, WEATHER_RAIKAMI_SPARK_INSYNC, WEATHER_RAIKAMI_SPARK_OUTSYNC );
      
      // 雷SE再生
      PMSND_PlaySE( WEATHER_SND_SE_SPARK );
      p_local_wk->seq++;
    }
    break;

  case WEATHER_RAIKAMI_FADEIN_SEQ_SPARK_02:
    if( WEATHER_TASK_LIGHT_IsColorFade( p_wk ) == FALSE )
    {
      WEATHER_TASK_FogFadeIn_Init( p_wk,
        WEATHER_FOG_SLOPE_DEFAULT, 
        WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIKAMI_FOG_OFS, 
        WEATHER_RAIKAMI_FOG_TIMING,
        fog_cont );

      p_local_wk->seq++;
      p_local_wk->wait = WEATHER_RAIKAMI_LIGHT_FULL_RAIN_WAIT;
    }
    break;

  case WEATHER_RAIKAMI_FADEIN_SEQ_OBJADD:
    p_local_wk->wait--;
    if( p_local_wk->wait == 0 )
    {
      // 音
      WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_HIGHRAIN );	
      // BGON
      WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE, WEATHER_TASK_3DBG_FRONT );
    }

	  result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行
	  fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );	// 実行
    if( result && fog_result )
    {
			return WEATHER_TASK_FUNC_RESULT_FINISH;
    }
    break;

  default:
    GF_ASSERT(0);
    break;
  }

  // スクロール処理
  WEATHER_RAIKAMI_SCROLL_Main( p_wk, p_local_wk );

  return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードなし初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_RAIKAMI_WORK* p_local_wk;
  BOOL result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_RAIKAMI_ADD_MAIN,	// obj登録数
		WEATHER_RAIKAMI_TIMING_MIN,// 登録タイミング
		WEATHER_RAIKAMI_ADD_MAIN,
		WEATHER_RAIKAMI_TIMING_MIN,
		-WEATHER_RAIKAMI_TIMING_ADD,
		WEATHER_RAIKAMI_ADD_TIMING,
		WEATHER_RAIKAMI_ADD,
		WEATHER_RAIKAMI_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_RAIKAMI_OBJ_Add, WEATHER_RAIKAMI_NOFADE_OBJ_START_NUM, WEATHER_RAIKAMI_NOFADE_OBJ_START_DUST_NUM, WEATHER_RAIKAMI_NOFADE_OBJ_START_DUST_MOVE, heapID );

  // 音
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_HIGHRAIN );	

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_raikami_dat );

  // BGON
  WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE, WEATHER_TASK_3DBG_FRONT );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_RAIKAMI_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	p_local_wk->obj_speed_change = (p_local_wk->obj_speed_change + 1) % (WEATHER_RAIKAMI_OBJ_MUL_CHG*WEATHER_RAIKAMI_OBJ_MUL_NUM);		// 雨登録料変更カウンタ

	// カウンタが0いかになったら雨登録
	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );


	// スパークメイン
	WEATHER_PARK_Main( &p_local_wk->spark, p_wk );

  // スクロール処理
  WEATHER_RAIKAMI_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト処理
 */ 
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_RAIKAMI_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_RAIKAMI_TIMING_MAX,
			WEATHER_RAIKAMI_TIMING_ADD,
			WEATHER_RAIKAMI_ADD_END );
	
	p_local_wk->wait = WEATHER_RAIKAMI_FOG_START_END;	// 同じくフォグ用


	// 音
	WEATHER_TASK_StopLoopSnd( p_wk );	

  // BGOFF
  WEATHER_TASK_3DBG_SetVisible( p_wk, FALSE, WEATHER_TASK_3DBG_FRONT );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_RAIKAMI_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	// フォグ操作
	if(p_local_wk->wait > 0){
		p_local_wk->wait--;			// ワークが０になったらフォグを動かす
		if( p_local_wk->wait == 0 ){

			WEATHER_TASK_FogFadeOut_Init( p_wk,
					WEATHER_FOG_DEPTH_DEFAULT_START, 
					WEATHER_RAIKAMI_FOG_TIMING_END, fog_cont );
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
  WEATHER_RAIKAMI_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了処理
 *
 *	@param	p_wk
 *	@param	fog_cont
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIKAMI_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIKAMI_WORK* p_local_wk;
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
 *	@brief  ライカミ　オブジェ　動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_RAIKAMI_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
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
		mat.x += obj_w[4];
		mat.y += obj_w[2];
	
		// 破棄するかチェック
		if( (mat.y >= WEATHER_RAIKAMI_OBJ_END_Y) ){
      // 破棄
      obj_w[3] = 1;
		}
		// 座標設定
		WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &mat );
		break;
	case 1:		// 破棄
		WEATHER_TASK_DeleteObj( p_wk );
		break;
	}		

}

//----------------------------------------------------------------------------
/**
 *	@brief  ライカミオブジェ　登録
 */
//-----------------------------------------------------------------------------
static void WEATHER_RAIKAMI_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	WEATHER_RAIKAMI_WORK*	p_local_wk;	// システムワーク
	int	err;	// 補正値
	s32* obj_w;	// オブジェクトワーク
	int speed_m;	// スピードにかける値テーブルの要素数
	int frame;		// フレーム数
	static const int WEATHER_RAIKAMI_OBJ_MUL[ WEATHER_RAIKAMI_OBJ_MUL_NUM ] = {10, 10, 13, 10, 16};
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
		frame = rand%3;	// 雨の種類
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );

    GF_ASSERT( (p_local_wk->obj_speed_change /WEATHER_RAIKAMI_OBJ_MUL_CHG) < WEATHER_RAIKAMI_OBJ_MUL_NUM );

		obj_w[4] = WEATHER_RAIKAMI_SPEED_X * (frame + 1) + ((rand % WEATHER_RAIKAMI_SPPED_RND));
		obj_w[2] = WEATHER_RAIKAMI_SPEED_Y * (frame + 1) + ((rand % WEATHER_RAIKAMI_SPPED_RND));
		obj_w[4] = (obj_w[4] * WEATHER_RAIKAMI_OBJ_MUL[p_local_wk->obj_speed_change /WEATHER_RAIKAMI_OBJ_MUL_CHG]) / WEATHER_RAIKAMI_OBJ_MUL_ONE;
		obj_w[2] = (obj_w[2] * WEATHER_RAIKAMI_OBJ_MUL[p_local_wk->obj_speed_change /WEATHER_RAIKAMI_OBJ_MUL_CHG]) / WEATHER_RAIKAMI_OBJ_MUL_ONE;
		
		obj_w[3] = 0;			// 破棄アニメフラグ
		
		// 座標を設定
		{
			mat.x = WEATHER_RAIKAMI_START_X + (rand % WEATHER_RAIKAMI_START_X_MAX);
			mat.y = WEATHER_RAIKAMI_START_Y + (rand % WEATHER_RAIKAMI_START_Y_MAX);

			WEATHER_OBJ_WORK_SetPosNoTurn( add_obj, &mat );
		}
		
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  らいかみ　スクロール処理
 */
//-----------------------------------------------------------------------------
static void WEATHER_RAIKAMI_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_RAIKAMI_WORK* p_wk )
{
	int x, y;
	WEATHER_TASK_GetScrollDist( p_sys, &x, &y );
	WEATHER_TASK_ScrollObj( p_sys, x, y );

	// BG面を斜め上に動かす
	p_wk->scroll = (p_wk->scroll + 9) % 256;
	WEATHER_TASK_3DBG_SetScrollX( p_sys, (-p_wk->scroll) - x, WEATHER_TASK_3DBG_FRONT );
	WEATHER_TASK_3DBG_SetScrollY( p_sys, (-p_wk->scroll) + y, WEATHER_TASK_3DBG_FRONT );
}


//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_KAZAKAMI_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk, 
		WEATHER_KAZAKAMI_ADD_START,	// obj登録数
		WEATHER_KAZAKAMI_TIMING_MAX,// 登録タイミング
		WEATHER_KAZAKAMI_ADD_MAIN,
		WEATHER_KAZAKAMI_TIMING_MIN,
		-WEATHER_KAZAKAMI_TIMING_ADD,
		WEATHER_KAZAKAMI_ADD_TIMING,
		WEATHER_KAZAKAMI_ADD,
		WEATHER_KAZAKAMI_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

  // 風SE再生処理初期化
  WEATHER_KAZAKAMI_WINDRAIN_SCENE_InitWindSePlay( p_local_wk );
  
	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_KAZAKAMI_WORK* p_local_wk;
  BOOL result;
  BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

  switch( p_local_wk->seq )
  {
  case KAZAKAMI_FADEIN_SEQ_INIT:
    // ライト変更
    WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_kazakami_dat );

    p_local_wk->wait = KAZAKAMI_FADEIN_WIND_WAIT;
    p_local_wk->seq++;
    break;
    
  case KAZAKAMI_FADEIN_SEQ_LIGHT_DARK:
    p_local_wk->wait--;
    if(p_local_wk->wait <= 0)
    {
      // 雨も開始する
      WEATHER_KAZAKAMI_WindStartRain( p_wk, p_local_wk, fog_cont, TRUE );

      // ここだけ、雨の時間固定
      p_local_wk->wind_scene_count_max = KAZAKAMI_FIRST_RAIN_TIME;
        
      // BG開始までのウエイト
      p_local_wk->wait = KAZAKAMI_FADEIN_BGSTART_WAIT;
      p_local_wk->seq++;
    }
    break;

  case KAZAKAMI_FADEIN_SEQ_WIND02:
    // 発動監視
    p_local_wk->wait --;
    if( p_local_wk->wait == 0 )
    {
      // 音
      WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_KAZAKAMI_NORMAL_RAIN );	
      // BGON
      WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE, WEATHER_TASK_3DBG_FRONT );
      //FOGON
      WEATHER_TASK_FogFadeIn_Init( p_wk,
        WEATHER_FOG_SLOPE_DEFAULT, 
        WEATHER_FOG_DEPTH_DEFAULT + WEATHER_KAZAKAMI_FOG_OFS_NORMAL, 
        WEATHER_KAZAKAMI_FOG_TIMING,
        fog_cont );
    }
    
	  result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行
	  fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );	// 実行
    if( (result==TRUE) && (fog_result==TRUE) && (p_local_wk->wait <= 0) )
    {
			return WEATHER_TASK_FUNC_RESULT_FINISH;
    }
    break;
    
  default:
    GF_ASSERT(0);
    break;
  }
  
  // 風の管理
  WEATHER_KAZAKAMI_WindControl( p_wk, p_local_wk, fog_cont, FALSE );
  
  // スクロール処理
  WEATHER_KAZAKAMI_SCROLL_Main( p_wk, p_local_wk );

  return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_KAZAKAMI_WORK* p_local_wk;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	// 作業領域の初期化
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_KAZAKAMI_ADD_MAIN,	// obj登録数
		WEATHER_KAZAKAMI_TIMING_MIN,// 登録タイミング
		WEATHER_KAZAKAMI_ADD_MAIN,
		WEATHER_KAZAKAMI_TIMING_MIN,
		-WEATHER_KAZAKAMI_TIMING_ADD,
		WEATHER_KAZAKAMI_ADD_TIMING,
		WEATHER_KAZAKAMI_ADD,
		WEATHER_KAZAKAMI_OBJ_Add );
	

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_KAZAKAMI_OBJ_Add, WEATHER_KAZAKAMI_NOFADE_OBJ_START_NUM, WEATHER_KAZAKAMI_NOFADE_OBJ_START_DUST_NUM, WEATHER_KAZAKAMI_NOFADE_OBJ_START_DUST_MOVE, heapID );

  // 音
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_KAZAKAMI_NORMAL_RAIN );	
  WEATHER_KAZAKAMI_WindStartRain( p_wk, p_local_wk, fog_cont, FALSE );

  // BGON
  WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE, WEATHER_TASK_3DBG_FRONT );

  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_kazakami_dat );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　メイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_KAZAKAMI_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// カウンタが0いかになったら雨登録
	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );

  // 風の管理
  WEATHER_KAZAKAMI_WindControl( p_wk, p_local_wk, fog_cont, FALSE );

  // スクロール処理
  WEATHER_KAZAKAMI_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_KAZAKAMI_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_KAZAKAMI_TIMING_MAX,
			WEATHER_KAZAKAMI_TIMING_ADD,
			WEATHER_KAZAKAMI_ADD_END );
	
	// 音
	WEATHER_TASK_StopLoopSnd( p_wk );	

  WEATHER_TASK_FogFadeOut_Init( p_wk,
      WEATHER_FOG_DEPTH_DEFAULT_START, 
      WEATHER_KAZAKAMI_FOG_TIMING_END, fog_cont );

  // BGOFF
  WEATHER_TASK_3DBG_SetVisible( p_wk, FALSE, WEATHER_TASK_3DBG_FRONT );
  WEATHER_TASK_3DBG_SetVisible( p_wk, FALSE, WEATHER_TASK_3DBG_BACK );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
  WEATHER_KAZAKAMI_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	
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

  // 風の管理
  WEATHER_KAZAKAMI_WindControl( p_wk, p_local_wk, fog_cont, TRUE  );

  // スクロール処理
  WEATHER_KAZAKAMI_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　終了
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_KAZAKAMI_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_KAZAKAMI_WORK* p_local_wk;
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
 *	@brief  カザカミ　オブジェ動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	GFL_CLWK* p_clwk;
  KAZAKAMI_OBJ_WK* p_obj_wk;

  static void (*WEATHER_KAZAKAMI_OBJ_MOVE_TBL[KAZAKAMI_OBJ_TYPE_NUM])( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ) = 
  {
    WEATHER_KAZAKAMI_OBJ_WIND_Move,
    WEATHER_KAZAKAMI_OBJ_RAIN_Move,
    WEATHER_KAZAKAMI_OBJ_WINDRAIN_Move,
  };

	p_obj_wk = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

  WEATHER_KAZAKAMI_OBJ_MOVE_TBL[ p_obj_wk->type ]( p_wk, p_obj_wk->cp_parent, (void*)&p_obj_wk->objwk, p_clwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　オブジェ登録
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	WEATHER_KAZAKAMI_WORK*	p_local_wk;	// システムワーク
  KAZAKAMI_OBJ_WK* p_obj_wk;
	GFL_CLWK* p_clwk;

  static void (*WEATHER_KAZAKAMI_OBJ_ADD_TBL[KAZAKAMI_OBJ_TYPE_NUM])( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk ) = 
  {
    WEATHER_KAZAKAMI_OBJ_WIND_Add,
    WEATHER_KAZAKAMI_OBJ_RAIN_Add,
    WEATHER_KAZAKAMI_OBJ_WINDRAIN_Add,
  };
  u32 rand = GFUser_GetPublicRand(0);

	// ユーザワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

  if( (p_local_wk->wind_scene_type == KAZAKAMI_WIND_SCENE_NONE) )
  {
    return ;
  }

  // 登録率を調整
  if( (p_local_wk->wind_scene_type == KAZAKAMI_WIND_SCENE_WINDRAIN) )
  {
    if( (rand % KAZAKAMI_WIND_SCENE_WIND_ADD_CUT_NUM) != 0 )
    {
      return ;
    }
  }
	
	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		add_obj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(add_obj == NULL){			// 失敗したら終了
			break;
		}
		p_obj_wk = WEATHER_OBJ_WORK_GetWork( add_obj );		// オブジェワーク作成
		p_clwk	= WEATHER_OBJ_WORK_GetClWk( add_obj );

    GF_ASSERT( sizeof(KAZAKAMI_OBJ_WK) < WEATHER_OBJ_WORK_USE_WORKSIZE );

    p_obj_wk->cp_parent = p_local_wk;
    switch( p_local_wk->wind_scene_type )
    {
    case KAZAKAMI_WIND_SCENE_RAIN:
      p_obj_wk->type = KAZAKAMI_OBJ_RAIN;
      break;
    case KAZAKAMI_WIND_SCENE_WINDRAIN:
      if( (rand % 100) <= KAZAKAMI_WIND_WINDRAIN_PARCENT )
      {
        p_obj_wk->type = KAZAKAMI_OBJ_WIND_RAIN;
      }
      else
      {
        p_obj_wk->type = KAZAKAMI_OBJ_WIND;
      }
      break;

    case KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN:
    case KAZAKAMI_WIND_SCENE_WINDRAIN_TO_RAIN:
      if( (rand % 100) <= KAZAKAMI_RAIN_RAIN_TO_WINDRAIN_PARCENT )
      {
        p_obj_wk->type = KAZAKAMI_OBJ_RAIN;
      }
      else
      {
        p_obj_wk->type = KAZAKAMI_OBJ_WIND_RAIN;
      }
      break;

    default:
      GF_ASSERT(0);
      break;
    }

    // 登録作業
    WEATHER_KAZAKAMI_OBJ_ADD_TBL[ p_obj_wk->type ]( add_obj, p_obj_wk->cp_parent, (void*)&p_obj_wk->objwk, p_clwk );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief  風表現オブジェの動き
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_WIND_Move( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk )
{
  GFL_CLACTPOS pos;
  KAZAKAMI_OBJ_WIND_WK* p_objwk = p_obj;
  fx32 last_move_x, last_move_y;
  fx32 move_x, move_y;
  u16 rotate;
  s32 wind_add_x, wind_add_y;

  // もし風がなくなったら、前のスピードで進む
  if( cp_parent->wind_add_x == 0 )
  {
    wind_add_x = p_objwk->last_speed_x;
    wind_add_y = p_objwk->last_speed_y;
  }
  else
  {
    wind_add_x = cp_parent->wind_add_x;
    wind_add_y = cp_parent->wind_add_y;
    p_objwk->last_speed_x = wind_add_x;
    p_objwk->last_speed_y = wind_add_y;
  }

  // 座標取得
	WEATHER_OBJ_WORK_GetPos( p_wk, &pos );

  // 通常スピード＋風
  p_objwk->pos_x += p_objwk->speed_x + wind_add_x;
  p_objwk->pos_y += p_objwk->speed_y + wind_add_y;
  pos.x = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_x);
  pos.y = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_y);

  // 破棄監視
  if( (pos.x >= KAZAKAMI_OBJ_WIND_DELETE_X) || (pos.y >= KAZAKAMI_OBJ_WIND_DELETE_Y) )
  {
    WEATHER_TASK_DeleteObj( p_wk );
    return ;
  }


  WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  風表現オブジェの追加
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_WIND_Add( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk )
{
  GFL_CLACTPOS pos;
  KAZAKAMI_OBJ_WIND_WK* p_objwk = p_obj;
  u32 rand = GFUser_GetPublicRand(0);

  p_objwk->speed_x = KAZAKAMI_OBJ_WIND_SPEED_X_MIN + (rand % KAZAKAMI_OBJ_WIND_SPEED_X_RND);
  p_objwk->speed_y = KAZAKAMI_OBJ_WIND_SPEED_Y_MIN + (rand % KAZAKAMI_OBJ_WIND_SPEED_Y_RND);

  p_objwk->last_speed_x = cp_parent->wind_add_x;
  p_objwk->last_speed_y = cp_parent->wind_add_y;

  // 座標
  pos.x = KAZAKAMI_OBJ_WIND_ADD_X + (rand % KAZAKAMI_OBJ_WIND_ADD_X_RND);
  pos.y = KAZAKAMI_OBJ_WIND_ADD_Y_MIN + (rand % KAZAKAMI_OBJ_WIND_ADD_Y_RND);
  WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &pos );

  p_objwk->pos_x = KAZAKAMI_OBJ_WIND_SPEED_SET( pos.x );
  p_objwk->pos_y = KAZAKAMI_OBJ_WIND_SPEED_SET( pos.y );

  GFL_CLACT_WK_SetAnmSeq( p_clwk, 0 );

  GFL_CLACT_WK_SetAutoAnmFlag( p_clwk, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( p_clwk, 2<<FX32_SHIFT );

}

//----------------------------------------------------------------------------
/**
 *	@brief  雨表現オブジェの動き
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_RAIN_Move( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk )
{
  GFL_CLACTPOS pos;
  KAZAKAMI_OBJ_RAIN_WK* p_objwk = p_obj;

  // 座標取得
	WEATHER_OBJ_WORK_GetPos( p_wk, &pos );

  // 通常スピード＋風
  p_objwk->pos_x += p_objwk->speed_x + cp_parent->wind_add_x;
  p_objwk->pos_y += p_objwk->speed_y + cp_parent->wind_add_y;

    
  pos.x = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_x);
  pos.y = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_y);

  // 破棄監視
  if( (pos.x >= KAZAKAMI_OBJ_RAIN_DELETE_X) || (pos.y >= KAZAKAMI_OBJ_RAIN_DELETE_Y) )
  {
    WEATHER_TASK_DeleteObj( p_wk );
    return ;
  }


  WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  雨表現オブジェの追加
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_RAIN_Add( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk )
{
  GFL_CLACTPOS pos;
  KAZAKAMI_OBJ_RAIN_WK* p_objwk = p_obj;
  u32 rand = GFUser_GetPublicRand(0);

  p_objwk->speed_x = KAZAKAMI_OBJ_RAIN_SPEED_X_MIN + (rand % KAZAKAMI_OBJ_RAIN_SPEED_RND);
  p_objwk->speed_y = KAZAKAMI_OBJ_RAIN_SPEED_Y_MIN + ((rand % KAZAKAMI_OBJ_RAIN_SPEED_RND) * KAZAKAMI_OBJ_RAIN_SPEED_Y_RND_MUL);

  // 座標
  pos.x = KAZAKAMI_OBJ_RAIN_ADD_X_MIN + (rand % KAZAKAMI_OBJ_RAIN_ADD_X_RND);
  pos.y = KAZAKAMI_OBJ_RAIN_ADD_Y_MIN + (rand % KAZAKAMI_OBJ_RAIN_ADD_Y_RND);
  WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &pos );

  p_objwk->pos_x = KAZAKAMI_OBJ_WIND_SPEED_SET( pos.x );
  p_objwk->pos_y = KAZAKAMI_OBJ_WIND_SPEED_SET( pos.y );

  GFL_CLACT_WK_SetAnmSeq( p_clwk, 1 );

  GFL_CLACT_WK_SetAutoAnmFlag( p_clwk, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( p_clwk, 2<<FX32_SHIFT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  風雨表現オブジェの動き
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_WINDRAIN_Move( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk )
{
  GFL_CLACTPOS pos;
  KAZAKAMI_OBJ_WINDRAIN_WK* p_objwk = p_obj;

  // 座標取得
	WEATHER_OBJ_WORK_GetPos( p_wk, &pos );

  // 通常スピード＋風
  if( cp_parent->wind_add_x != 0 )
  {
    p_objwk->pos_x += p_objwk->speed_x + cp_parent->wind_add_x;
    p_objwk->pos_y += p_objwk->speed_y + cp_parent->wind_add_y;
    pos.x = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_x);
    pos.y = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_y);
    
    p_objwk->last_speed_x = p_objwk->speed_x + cp_parent->wind_add_x;
    p_objwk->last_speed_y = p_objwk->speed_y + cp_parent->wind_add_y;
  }
  else
  {
    p_objwk->pos_x += p_objwk->last_speed_x;
    p_objwk->pos_y += p_objwk->last_speed_y;
    pos.x = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_x);
    pos.y = KAZAKAMI_OBJ_WIND_SPEED_GET(p_objwk->pos_y);
  }

  // 破棄監視
  if( (pos.x >= KAZAKAMI_OBJ_WIND_DELETE_X) || (pos.y >= KAZAKAMI_OBJ_WIND_DELETE_Y) )
  {
    WEATHER_TASK_DeleteObj( p_wk );
    return ;
  }

  WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &pos );

}

//----------------------------------------------------------------------------
/**
 *	@brief  風雨表現オブジェの追加
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_OBJ_WINDRAIN_Add( WEATHER_OBJ_WORK* p_wk, const WEATHER_KAZAKAMI_WORK* cp_parent, void* p_obj, GFL_CLWK* p_clwk )
{
  GFL_CLACTPOS pos;
  KAZAKAMI_OBJ_WINDRAIN_WK* p_objwk = p_obj;
  u32 rand = GFUser_GetPublicRand(0);

  p_objwk->speed_x = KAZAKAMI_OBJ_WINDRAIN_SPEED_X_MIN + (rand % KAZAKAMI_OBJ_WINDRAIN_SPEED_X_RND);
  p_objwk->speed_y = KAZAKAMI_OBJ_WINDRAIN_SPEED_Y_MIN + (rand % KAZAKAMI_OBJ_WINDRAIN_SPEED_Y_RND);

  p_objwk->last_speed_x = p_objwk->speed_x + cp_parent->wind_add_x;
  p_objwk->last_speed_y = p_objwk->speed_y + cp_parent->wind_add_y;

  // 座標
  pos.x = KAZAKAMI_OBJ_WINDRAIN_ADD_X_MIN + (rand % KAZAKAMI_OBJ_WINDRAIN_ADD_X_RND);
  pos.y = KAZAKAMI_OBJ_WINDRAIN_ADD_Y_MIN + (rand % KAZAKAMI_OBJ_WINDRAIN_ADD_Y_RND);
  WEATHER_OBJ_WORK_SetPosNoTurn( p_wk, &pos );

  p_objwk->pos_x = KAZAKAMI_OBJ_WIND_SPEED_SET( pos.x );
  p_objwk->pos_y = KAZAKAMI_OBJ_WIND_SPEED_SET( pos.y );

  GFL_CLACT_WK_SetAnmSeq( p_clwk, 2 );

  GFL_CLACT_WK_SetAutoAnmFlag( p_clwk, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( p_clwk, 2<<FX32_SHIFT );
}


//----------------------------------------------------------------------------
/**
 *	@brief  雨つき風の開始
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_WindStartRain( WEATHER_TASK* p_sys, WEATHER_KAZAKAMI_WORK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, BOOL fadeout )
{
  u32 rand = GFUser_GetPublicRand(0);
  // WINDRAINへ
  p_wk->wind_scene_type   = KAZAKAMI_WIND_SCENE_RAIN;
  p_wk->wind_scene_count_max  = KAZAKAMI_WIND_SCENE_WINDRAIN_TIME_MIN + (rand % KAZAKAMI_WIND_SCENE_WINDRAIN_TIME_RND);
  p_wk->wind_scene_count = 0;

  // FOGを動かす
  if( fadeout == FALSE )
  {
    WEATHER_TASK_FogFadeIn_Init( p_sys,
      WEATHER_FOG_SLOPE_DEFAULT, 
      WEATHER_FOG_DEPTH_DEFAULT + WEATHER_KAZAKAMI_FOG_OFS_NORMAL, 
      WEATHER_KAZAKAMI_FOG_TIMING,
      fog_cont );

    WEATHER_TASK_3DBG_SetVisible( p_sys, FALSE, WEATHER_TASK_3DBG_BACK );

    // SEの変更
    //PMSND_PlaySE( WEATHER_SND_SE_KAZAKAMI_NORMAL_MIDDLE_WIND );
    WEATHER_TASK_PlayLoopSnd( p_sys, WEATHER_SND_SE_KAZAKAMI_NORMAL_RAIN );	
  }


}


//----------------------------------------------------------------------------
/**
 *	@brief  風管理
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_WindControl( WEATHER_TASK* p_sys, WEATHER_KAZAKAMI_WORK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, BOOL fadeout )
{
  // scene_typeの変更
  switch( p_wk->wind_scene_type )
  {
  case  KAZAKAMI_WIND_SCENE_RAIN:
    p_wk->wind_scene_count++;
    WEATHER_KAZAKAMI_WindSePlay( p_wk, fog_cont, WEATHER_SND_SE_KAZAKAMI_NORMAL_MIDDLE_WIND );
    if( p_wk->wind_scene_count >= p_wk->wind_scene_count_max )
    {
        // RAIN_TO_WINDRAINへ
        p_wk->wind_scene_type   = KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN;
        p_wk->wind_scene_count_max  = KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_TIME_MIN;
        p_wk->wind_scene_count = 0;

        // FOGを動かす
        if( fadeout == FALSE )
        {
          WEATHER_TASK_FogFadeIn_Init( p_sys,
            WEATHER_FOG_SLOPE_DEFAULT, 
            WEATHER_FOG_DEPTH_DEFAULT + WEATHER_KAZAKAMI_FOG_OFS, 
            WEATHER_KAZAKAMI_FOG_TIMING,
            fog_cont );

          // SEの変更
          //PMSND_PlaySE( WEATHER_SND_SE_KAZAKAMI_NORMAL_MIDDLE_WIND );
          WEATHER_TASK_PlayLoopSnd( p_sys, WEATHER_SND_SE_KAZAKAMI_HIGH_RAIN );	
          
        }
    }
    break;
    
  case  KAZAKAMI_WIND_SCENE_WINDRAIN:
    {
      p_wk->wind_scene_count++;
      //WEATHER_KAZAKAMI_WindSePlay( p_wk, fog_cont, WEATHER_SND_SE_KAZAKAMI_MIDDLE_HIGH_WIND );
      WEATHER_KAZAKAMI_WindSePlay( p_wk, fog_cont, WEATHER_SND_SE_KAZAKAMI_NORMAL_MIDDLE_WIND );
      WEATHER_KAZAKAMI_WINDRAIN_SCENE_WindSePlay( p_wk, fog_cont );
      if( p_wk->wind_scene_count >= p_wk->wind_scene_count_max )
      {
        // WINDRAIN_TO_RAINへ
        p_wk->wind_scene_type   = KAZAKAMI_WIND_SCENE_WINDRAIN_TO_RAIN;
        p_wk->wind_scene_count_max  = KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_TIME_MIN;
        p_wk->wind_scene_count = 0;

        // BGを消す
        if( fadeout == FALSE )
        {
          // 3DBGの管理
          WEATHER_TASK_3DBG_SetVisible( p_sys, FALSE, WEATHER_TASK_3DBG_BACK );
          WEATHER_TASK_3DBG_SetScaleX( p_sys, FX32_ONE, WEATHER_TASK_3DBG_FRONT );
          WEATHER_TASK_3DBG_SetScaleY( p_sys, FX32_ONE, WEATHER_TASK_3DBG_FRONT );

          // SEの変更
          //PMSND_PlaySE( WEATHER_SND_SE_KAZAKAMI_MIDDLE_HIGH_WIND );
          //WEATHER_TASK_PlayLoopSnd( p_sys, WEATHER_SND_SE_KAZAKAMI_MIDDLE_RAIN );	
        
        }

      }
    }
    break;

  case KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN:
    {
      p_wk->wind_scene_count++;
      //WEATHER_KAZAKAMI_WindSePlay( p_wk, fog_cont, WEATHER_SND_SE_KAZAKAMI_MIDDLE_HIGH_WIND );
      if( p_wk->wind_scene_count >= p_wk->wind_scene_count_max )
      {
        u32 rand = GFUser_GetPublicRand(0);
        // WINDRAINへ
        p_wk->wind_scene_type   = KAZAKAMI_WIND_SCENE_WINDRAIN;
        p_wk->wind_scene_count_max  = KAZAKAMI_WIND_SCENE_WINDRAIN_TIME_MIN + (rand % KAZAKAMI_WIND_SCENE_WINDRAIN_TIME_RND);
        p_wk->wind_scene_count = 0;

        // BGを出す
        if( fadeout == FALSE )
        {
          // 3DBGの管理
          WEATHER_TASK_3DBG_SetVisible( p_sys, TRUE, WEATHER_TASK_3DBG_BACK );
          WEATHER_TASK_3DBG_SetScaleX( p_sys, FX32_HALF, WEATHER_TASK_3DBG_FRONT );
          WEATHER_TASK_3DBG_SetScaleY( p_sys, FX32_ONE + FX32_HALF, WEATHER_TASK_3DBG_FRONT );

          // SEの変更
          //PMSND_PlaySE( WEATHER_SND_SE_KAZAKAMI_MIDDLE_HIGH_WIND );
          //WEATHER_TASK_PlayLoopSnd( p_sys, WEATHER_SND_SE_KAZAKAMI_HIGH_RAIN );	
        }
      }
    }
    break;
    
  case KAZAKAMI_WIND_SCENE_WINDRAIN_TO_RAIN:
    {
      p_wk->wind_scene_count++;
      if( p_wk->wind_scene_count >= p_wk->wind_scene_count_max )
      {
        WEATHER_KAZAKAMI_WindStartRain( p_sys, p_wk, fog_cont, fadeout );
      }
    }
    break;

  case KAZAKAMI_WIND_SCENE_NONE:
    break;
    
  default:
    GF_ASSERT(0);
    break;
  }
  
  
  // 風の設定
  switch( p_wk->wind_scene_type )
  {
  case  KAZAKAMI_WIND_SCENE_RAIN:
    if( (p_wk->wind_scene_count_max - p_wk->wind_scene_count) >= KAZAKAMI_WIND_SCENE_RAIN_WIND_ON_TIME )
    {
      p_wk->wind_add_x = KAZAKAMI_WIND_SCENE_RAIN_WIND_SPEED_X;
      p_wk->wind_add_y = KAZAKAMI_WIND_SCENE_RAIN_WIND_SPEED_Y;
    }
    else
    {
      p_wk->wind_add_x = KAZAKAMI_WIND_SCENE_RAIN_WINDON_SPEED_X;
      p_wk->wind_add_y = KAZAKAMI_WIND_SCENE_RAIN_WINDON_SPEED_Y;
    }
    break;
    
  case  KAZAKAMI_WIND_SCENE_WINDRAIN:
    {
      u32 idx;

      idx = ((p_wk->wind_scene_count%KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_CHANGE_TIME) * KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_TBL_NUM) / KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_CHANGE_TIME;
      GF_ASSERT( idx < KAZAKAMI_WIND_SCENE_WINDRAIN_WIND_SPEED_TBL_NUM );
      p_wk->wind_add_x = KAZAKAMI_WIND_SCENE_RAIN_RAINWIND_SPEED_TBL[idx].x;
      p_wk->wind_add_y = KAZAKAMI_WIND_SCENE_RAIN_RAINWIND_SPEED_TBL[idx].y;

      idx = ((p_wk->wind_scene_count%KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_CHANGE_TIME) * KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_NUM) / KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_CHANGE_TIME;
      GF_ASSERT( idx < KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_NUM );
      p_wk->wind_bg_back_scale_x = KAZAKAMI_WIND_SCENE_RAIN_WIND_BG_BACK_SCALE_TBL[idx];
    }
    break;


  case KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN:
  case KAZAKAMI_WIND_SCENE_WINDRAIN_TO_RAIN:
    p_wk->wind_add_x = KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_WIND_SPEED_X;
    p_wk->wind_add_y = KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_WIND_SPEED_Y;
    p_wk->wind_bg_back_scale_x = KAZAKAMI_WIND_SCENE_RAIN_TO_WINDRAIN_WIND_SCALE_X;
    break;

  case KAZAKAMI_WIND_SCENE_NONE:
    break;

  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カザカミ　スクロール処理
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_KAZAKAMI_WORK* p_wk )
{
	int x, y;
	WEATHER_TASK_GetScrollDist( p_sys, &x, &y );
	WEATHER_TASK_ScrollObj( p_sys, x, y );

	// BG面を斜め上に動かす
  if(p_wk->wind_add_x != 0 )
  {
  	p_wk->scroll_x = (p_wk->scroll_x + 9) % 256;
	  p_wk->scroll_y = (p_wk->scroll_y + 3) % 256;
  	p_wk->scroll_back_x = (p_wk->scroll_back_x + 9) % 256;
	  p_wk->scroll_back_y = (p_wk->scroll_back_y + 2) % 256;
  }
  else
  {
  	p_wk->scroll_x = (p_wk->scroll_x + 3) % 256;
	  p_wk->scroll_y = (p_wk->scroll_y + 8) % 256;
  }

	WEATHER_TASK_3DBG_SetScrollX( p_sys, (-p_wk->scroll_x) - x, WEATHER_TASK_3DBG_FRONT );
	WEATHER_TASK_3DBG_SetScrollY( p_sys, (-p_wk->scroll_y) + y, WEATHER_TASK_3DBG_FRONT );

	WEATHER_TASK_3DBG_SetScrollX( p_sys, (-p_wk->scroll_back_x) - x, WEATHER_TASK_3DBG_BACK );
	WEATHER_TASK_3DBG_SetScrollY( p_sys, (-p_wk->scroll_back_y) + y, WEATHER_TASK_3DBG_BACK );

  {
    WEATHER_TASK_3DBG_SetScaleX( p_sys, p_wk->wind_bg_back_scale_x, WEATHER_TASK_3DBG_BACK );
    WEATHER_TASK_3DBG_SetScaleY( p_sys, p_wk->wind_bg_back_scale_x, WEATHER_TASK_3DBG_BACK );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  雨切り替えをごまかすための風SEを再生
 *
 *	@param	cp_wk
 *	@param	fog_cont
 *	@param	se_no 
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_WindSePlay( const WEATHER_KAZAKAMI_WORK* cp_wk, BOOL fog_cont, u16 se_no )
{
  if( fog_cont )
  {
    if( cp_wk->wind_scene_count == (cp_wk->wind_scene_count_max - KAZAKAMI_WIND_SCENE_CHANGE_SE_PLAY_TIMING) )
    {
      PMSND_PlaySE( se_no );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風SE再生処理　初期化
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_WINDRAIN_SCENE_InitWindSePlay( WEATHER_KAZAKAMI_WORK* p_wk )
{
  p_wk->wind_se_count = 0;
  p_wk->wind_se_count_max = KAZAKAMI_WIND_SE_LOOP_TIME_MIN + GFUser_GetPublicRand( KAZAKAMI_WIND_SE_LOOP_TIME_DIF );
}



//----------------------------------------------------------------------------
/**
 *	@brief  暴風時の風再生処理
 *
 *	@param	p_wk
 *	@param	fog_cont
 *	@param	se_no 
 */
//-----------------------------------------------------------------------------
static void WEATHER_KAZAKAMI_WINDRAIN_SCENE_WindSePlay( WEATHER_KAZAKAMI_WORK* p_wk, BOOL fog_cont )
{
  if( fog_cont )
  {
    if( p_wk->wind_scene_count <= (p_wk->wind_scene_count_max - KAZAKAMI_WIND_SE_LOOP_TIME - KAZAKAMI_WIND_SCENE_CHANGE_SE_PLAY_TIMING) )
    {
      p_wk->wind_se_count ++;
      if( p_wk->wind_se_count >= p_wk->wind_se_count_max )
      {
        
        // 鳴っていないときに鳴らす
        if( PMSND_CheckPlaySE_byPlayerID( PMSND_GetSE_DefaultPlayerID(WEATHER_SND_SE_KAZAKAMI_NORMAL_MIDDLE_WIND) ) == FALSE )
        {
          u32 vol = KAZAKAMI_WIND_SE_LOOP_VOLUME_MIN + GFUser_GetPublicRand( KAZAKAMI_WIND_SE_LOOP_VOLUME_DIF );
        
          PMSND_PlaySEVolume( WEATHER_SND_SE_KAZAKAMI_MIDDLE_HIGH_WIND, vol );
          WEATHER_KAZAKAMI_WINDRAIN_SCENE_InitWindSePlay( p_wk );
        }
      }
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
static void WEATHER_PARK_Init( WEATHER_SPARK_WORK* p_wk, u8 mode )
{
	GFL_STD_MemClear( p_wk, sizeof(WEATHER_SPARK_WORK) );

	p_wk->wait = GFUser_GetPublicRand( WEATHER_SPARK_INIT_RAND );

  p_wk->mode = mode;
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
        if( p_wk->mode == WEATHER_SPARK_MODE_NORMAL ){
				  WEATEHR_SPARK_SetUp_Light( p_wk );
        }else{
				  WEATEHR_SPARK_SetUp_Hard( p_wk );
        }
				break;
			case WEATHER_SPARK_TYPE_HARD:
				WEATEHR_SPARK_SetUp_Hard( p_wk );
				break;
			case WEATHER_SPARK_TYPE_LIGHT_HARD:
				WEATEHR_SPARK_SetUp_LightAndHard( p_wk );
				break;
			}

			p_wk->spark_tbl_count	= 0;
			p_wk->seq				= WEATHER_SPARK_SPARK_DARK;
		}
		break;

  case WEATHER_SPARK_SPARK_DARK:
    if( p_wk->mode != WEATHER_SPARK_MODE_RAIKAMI )
    {
			p_wk->seq = WEATHER_SPARK_SPARK;
    }
    else
    {
      // ドデカイ雷を鳴らす。（LIGHTを暗くしてから）
	  	WEATHER_TASK_LIGHT_StartColorFadeOneWay( p_sys, WEATHER_SPARK_DARK_COLOR, WEATHER_SPARK_DARK_SYNC );
      p_wk->wait = WEATHER_SPARK_DARK_WAIT;
			p_wk->seq				= WEATHER_SPARK_SPARK_DARK_WAIT;
    }
    break;

  case WEATHER_SPARK_SPARK_DARK_WAIT:
    p_wk->wait --;
    if( p_wk->wait <= 0 )
    {
			p_wk->seq = WEATHER_SPARK_SPARK;
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

    // 次の雷までのwait
    // 最終雷が鳴り終わったら、ウエイト数を変更する
    // (1/2)にする
    if( p_wk->mode == WEATHER_SPARK_MODE_RAIKAMI )
    {
      if( (p_wk->spark_tbl_count+1) == p_wk->spark_tbl_num )
      {
        p_wk->spark_data[ p_wk->spark_tbl_count ].wait = (p_wk->spark_data[ p_wk->spark_tbl_count ].wait * 1) / 3;
      }
    }
    
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
		
    // モードにより頻度が変わる
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


