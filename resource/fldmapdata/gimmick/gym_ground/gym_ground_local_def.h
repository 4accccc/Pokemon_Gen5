//======================================================================
/**
 * @file	gym_ground_local_def.h
 * @brief	数値定義ファイル
 * @note	このファイルはコンバーターから出力されます
 *
 */
//======================================================================

//床高さ
#define HEIGHT0 (55)	//出口
#define HEIGHT1 (50)	//第1層
#define HEIGHT2 (40)	//第2層
#define HEIGHT3 (30)	//第3層
#define HEIGHT4 (-59)	//第4層
//リフト移動速度
#define LIFT_MOVE_SPEED (4)
#define SP_LIFT_MOVE_SPEED1 (10)
#define SP_LIFT_MOVE_SPEED2 (2)
//隔壁オープン待ち時間
#define WALL_ANM_WAIT (60)
//隔壁オープン開始高さ
#define WALL_ANM_START_HEIHGT (-30)
//リフト振動幅
#define LIFT_SHAKE_VAL (2)
#define SP_LIFT_SHAKE_VAL (4)
//振動回数
#define LIFT_SHAKE_COUNT (4)
#define SP_LIFT_SHAKE_COUNT (4)
//起動時振幅幅
#define LIFT_START_SHAKE_VAL (1)
//起動時振動回数
#define LIFT_START_SHAKE_COUNT (2)
//振動間隔
#define SHAKE_MARGIN (2)
//隔壁位置
#define WALL_Y_POS (-40)
//降下時フォグフェード位置
#define DOWN_FOG_FADE_START (-45)
//上昇時フォグフェード位置
#define UP_FOG_FADE_START (-45)
//移動速度が速いときのフォグフェードシンク
#define FOG_FADE_SPEED_FAST (40)
//移動速度が遅いときのフォグフェードシンク
#define FOG_FADE_SPEED_SLOW (8)
//フォグオフセット
#define FOG_OFFSET (32671)
//フォグスロープ
#define FOG_SLOPE (8)
