//======================================================================
/**
 * @file	gym_insect_local_def.h
 * @brief	数値定義ファイル
 * @note	このファイルはコンバーターから出力されます
 *
 */
//======================================================================

//フォグパラメータ
#define FOG_COLOR_A (31)
#define FOG_SLOPE (6)
#define FOG_OFFSET (32495)
//アニメウェイト
#define COCCON_OPEN_ANM_WAIT (70)	//繭オープンアニメウェイト
//カメラズーム
#define CAMERA_ZOOM_DISTANCE (200)	//ズーム距離(Unit)
#define CAMERA_ZOOM_IN_FRAME (20)	//ズームインウェイト
#define CAMERA_ZOOM_OUT_FRAME (10)	//ズームアウトフレーム
//
#define GATE_IN_FADE_FRM_KEY (20)
//繭ID
#define COCCON_ID_1C (0)	//繭ID 1C
#define COCCON_ID_3B (1)	//繭ID 3B
//ゲートID0-1
#define GATE_ID_1A_2A (0)	//ゲートIDは配列順に依存しているので、ずらす時はプログラマに相談
#define GATE_ID_2A_1A (1)
//ゲートID2-3
#define GATE_ID_1A_2B (2)
#define GATE_ID_2B_1A (3)
//ゲートID4-5
#define GATE_ID_1B_3B (4)
#define GATE_ID_3B_1B (5)
//ゲートID6-7
#define GATE_ID_1B_3C (6)
#define GATE_ID_3C_1B (7)
//ゲートID8-9
#define GATE_ID_1C_4B (8)
#define GATE_ID_4B_1C (9)
//ゲートID10-11
#define GATE_ID_2C_3A (10)
#define GATE_ID_3A_2C (11)
//ゲートID12-13
#define GATE_ID_3C_3D (12)
#define GATE_ID_3D_3C (13)
//ゲートID14-15
#define GATE_ID_3D_4A (14)
#define GATE_ID_4A_3D (15)
