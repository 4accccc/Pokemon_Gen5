//======================================================================
/**
 * @file  gym_elec.c
 * @brief  電気ジム
 * @author  Saito
 * @date  09.09.02
 */
//======================================================================

#include "fieldmap.h"
#include "gym_elec_sv.h"
#include "gym_elec.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"
#include "system/ica_anime.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_elec.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c04gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_denki/gym_elec_local_def.h"

#include "sound/pm_sndsys.h"

#include "gym_elec_se_def.h"

#define GYM_ELEC_UNIT_IDX (0)
#define GYM_ELEC_TMP_ASSIGN_ID  (1)

#define STOP_NUM_MAX  (2)
#define RALE_NUM_MAX  (8)


#define PLATFORM_NUM_MAX (9)

#define PLATFORM_NONE (0xff)
#define RIDE_NONE   (0xff)  //カプセルに乗っていない

#define GYM_ELEC_GMK_OBJ_NUM  (10)
#define RALE_PLATFORM_NUM_MAX (2)   //1レールで止まるプラットホームの数

#define STOP_TIME (90)
#define CAP_OPEN_FRAME  (15*FX32_ONE)   //蓋が開ききるフレーム（データ依存）

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)


//カプセル初期座標
#define CAP1_X  (22*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP1_Z  (37*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP2_X  (41*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP2_Z  (38*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP3_X  (34*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP3_Z  (6*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP4_X  (5*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAP4_Z  (37*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
//スイッチ座標
#define SW1_X (29*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW1_Z (33*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW2_X (13*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW2_Z (16*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW3_X (46*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW3_Z (41*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW4_X (25*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW4_Z (9*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
//OBJのＹ座標
#define OBJ3D_Y  (2*FIELD_CONST_GRID_FX32_SIZE)


#define UP_SE_FRM1  (782336)
#define TOP_SE_STOP_FRM1  (962560)
#define DOWN_SE_FRM1  (1044480)
#define UP_SE_FRM2  (1081344)
#define TOP_SE_STOP_FRM2  (1249280)
#define DOWN_SE_FRM2  (1343488)

#define LOOP_SE_FRM1  (724992)
#define LOOP_SE_FRM2  (720896)


#define SPD_LV5 (FX32_ONE * 8 * 4)
#define SPD_LV4 (FX32_ONE * 8 * 3)
#define SPD_LV3 (FX32_ONE * 8 * 2)
#define SPD_LV2 (FX32_ONE * 8 * 1)

#define SE_VOL  (127)
#define SE_RECT_MAX (2)
#define SE_LEN  (6 * 16 * FX32_ONE)

static const VecFx32 CapPos[CAPSULE_NUM_MAX] = {
  {CAP1_X,OBJ3D_Y,CAP1_Z},
  {CAP2_X,OBJ3D_Y,CAP2_Z},
  {CAP3_X,OBJ3D_Y,CAP3_Z},
  {CAP4_X,OBJ3D_Y,CAP4_Z},
};

static const int RaleAnm[RALE_NUM_MAX] = {
  NARC_gym_elec_gym04_rail_02a_bin,
  NARC_gym_elec_gym04_rail_02b_bin,
  NARC_gym_elec_gym04_rail_03a_bin,
  NARC_gym_elec_gym04_rail_03b_bin,
  NARC_gym_elec_gym04_rail_04a_bin,
  NARC_gym_elec_gym04_rail_04b_bin,
  NARC_gym_elec_gym04_rail_01a_bin,
  NARC_gym_elec_gym04_rail_01b_bin,
};



#define FRAME_POS_SIZE  (4*3*360)   //座標ＸＹＺ各4バイトｘ360フレーム
#define HEADER_SIZE  (8)   //フレームサイズ4バイト+格納情報3バイトのヘッダー情報があるためアライメントを加味して8バイト確保する

typedef struct ELEC_GYM_TASK_WK_tag
{
  u32 Timer;
}ELEC_GYM_TASK_WK;

//ジム内部中の一時ワーク
typedef struct GYM_ELEC_TMP_tag
{
  GFL_TCB * CapStopTcbPtr;   //カプセル停止制御ＴＣＢ
  u8 RadeRaleIdx;          //自機が走行しているレールインデックス
  s8 LeverIdx;              //操作しようとしているレバーインデックス(イベント中以外は-1)
  u8 ChgingSw;            //イベント中のみ使用
  s8 RideEvt;            //乗降イベント中カプセル番号
  ELEC_GYM_TASK_WK  TaskWk[CAPSULE_NUM_MAX];
  ICA_ANIME* IcaAnmPtr;
  u8 FramePosDat[FRAME_POS_SIZE*2+HEADER_SIZE];
  BOOL AltoMove;

  u8 RaleChgReq[CAPSULE_NUM_MAX];
  u8 NowRaleIdx[CAPSULE_NUM_MAX];     //現在走行しているレールのインデックス(スイッチ非依存)
  s8 StopPlatformIdx[CAPSULE_NUM_MAX];  //PLATFORM_NO_STOP (-1)　を使用するのでマイナス値を使用できるように。
/**
  ICA_ANIME* IcaAnmPtr2[RALE_NUM_MAX];
  u8 FramePosDat2[RALE_NUM_MAX][FRAME_POS_SIZE*2+HEADER_SIZE];
*/
}GYM_ELEC_TMP;

typedef struct STOP_DAT_tag
{
  int PlatformIdx;      //停車プラットフォーム
  int StopStart;        //停車開始フレーム
  int StopEnd;          //停車終了フレーム
}STOP_DAT;

typedef struct SE_RECT_tag
{
  int CapIdx;
  u16 X;
  u16 Z;
  u16 SizeW;
  u16 SizeH;
}SE_RECT;

SE_RECT SeRect[SE_RECT_MAX] = {
  {1, 21,10,15,2},
  {3, 10,15,1,16},
};

//レールごとの停車プラットホーム
static const s8 RaleStopPlatform[RALE_NUM_MAX][STOP_NUM_MAX] = {
  {1, 0},
  {1, 2},
  {4, -1},
  {4, 3},
  {6, -1},
  {6, 5},
  {8, -1},
  {8, 7}
};

//プラットホーム座標リスト(グリッド座標)
static const u16 PlatformXZ[PLATFORM_NUM_MAX][2] = {
  {14,39},
  {22,39},
  {16,27},
  {16,15},
  {41,40},
  {49,40},
  {34,8},
  {22,8},
  {5,39},
};

//プラットホーム別停車カプセルインデックスリスト
static const u8 PlatStpCapIdx[PLATFORM_NUM_MAX] = {
  0,0,0,1,1,2,2,3,3
};
#if 0
//1つのレバーで切り替えるレール間で共通で停車するプラットホーム
static const u8 CommPlatFormIdx[CAPSULE_NUM_MAX] = {
  1,4,6,8
};
#endif
//レールごとの停車アニメフレーム（1レール2つずつ）
//1箇所しか止まらないレールは2データとも同じ値を入れる。ICAデータ依存
static const fx32 StopFrame[RALE_NUM_MAX][RALE_PLATFORM_NUM_MAX] = {
  {0, FX32_ONE*R_STOP_FRM1},     //レール1
  {0, FX32_ONE*R_STOP_FRM2},   //レール2
  {0, FX32_ONE*0},     //レール3
  {0, FX32_ONE*R_STOP_FRM4},     //レール4
  {0, FX32_ONE*0},     //レール5
  {0, FX32_ONE*R_STOP_FRM6},     //レール6
  {0, FX32_ONE*0},     //レール7
  {0, FX32_ONE*R_STOP_FRM8},     //レール8
};

//カプセルごとのレール移動アニメ共通部分フレーム（アニメデータ依存）
static const u16 CommFrame[CAPSULE_NUM_MAX] = {
  COMM_FRM1,COMM_FRM2,COMM_FRM3,COMM_FRM4
};

//リソースの並び順番
enum {
  RES_ID_CAP_MDL = 0,
  RES_ID_SW_MDL,
  RES_ID_RL1_MDL,
  RES_ID_RL2_MDL,
  RES_ID_RL3_MDL,
  RES_ID_RL4_MDL,

  RES_ID_CAP_OPCL,

  RES_ID_CAP_MOV1,
  RES_ID_CAP_MOV2,
  RES_ID_CAP_MOV3,
  RES_ID_CAP_MOV4,
  RES_ID_CAP_MOV5,
  RES_ID_CAP_MOV6,
  RES_ID_CAP_MOV7,
  RES_ID_CAP_MOV8,

  RES_ID_SW1_A,
  RES_ID_SW1_B,
  RES_ID_SW2_A,
  RES_ID_SW2_B,
  RES_ID_SW3_A,
  RES_ID_SW3_B,
  RES_ID_SW4_A,
  RES_ID_SW4_B,

  RES_ID_RL1_A,
  RES_ID_RL1_B,
  RES_ID_RL2_A,
  RES_ID_RL2_B,
  RES_ID_RL3_A,
  RES_ID_RL3_B,
  RES_ID_RL4_A,
  RES_ID_RL4_B,

};

//ＯＢＪインデックス
enum {
  OBJ_CAP_1 = 0,
  OBJ_CAP_2,
  OBJ_CAP_3,
  OBJ_CAP_4,
  OBJ_SW_1,
  OBJ_SW_2,
  OBJ_SW_3,
  OBJ_SW_4,
  OBJ_RL_1,
  OBJ_RL_2,
  OBJ_RL_3,
  OBJ_RL_4,
};


//カプセルアニメの順番
enum{
  ANM_CAP_OPCL = 0,    //開く、閉じる

  ANM_CAP_MOV1,        //レール1
  ANM_CAP_MOV2,        //レール2
  ANM_CAP_MOV3,        //レール3
  ANM_CAP_MOV4,        //レール4
  ANM_CAP_MOV5,        //レール5
  ANM_CAP_MOV6,        //レール6
  ANM_CAP_MOV7,        //レール7
  ANM_CAP_MOV8,        //レール8
  
//  ANM_SW2_A,          //スイッチ2
//  ANM_SW2_B,          //スイッチ2
//  ANM_SW3_A,          //スイッチ3
//  ANM_SW3_B,          //スイッチ3
//  ANM_SW4_A,          //スイッチ4
//  ANM_SW4_B,          //スイッチ4
};

//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_ELEC, NARC_gym_elec_gym04_cart_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　カプセル本体
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_switch_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　スイッチ本体
  
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_02_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　レール1本体
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_03_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　レール2本体
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_04_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　レール3本体
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　レール4本体

  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_cart_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　カプセル開く閉じる

  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_02a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動1
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_02b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動2
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_03a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動3
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_03b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動4
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_04a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動5
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_04b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動6
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_01a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動7
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_01b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カプセル移動8

  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_ao_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ1
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_mizu_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ1
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_kimi_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ2
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_kiiro_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ2
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_mura_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ3
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_ore_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ3
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_aka_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ4
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_s_pin_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ4

  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_02a_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール1
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_02b_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール1
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_03a_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール2
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_03b_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール2
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_04a_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール3
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_04b_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール3
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_01a_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール4
  { ARCID_GYM_ELEC, NARC_gym_elec_gym04_rail_01b_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　レール4

};

//3Dアニメ　カプセル
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_cap[] = {
	{ RES_ID_CAP_OPCL,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)

  { RES_ID_CAP_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV4,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV5,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV6,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV7,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAP_MOV8,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)

};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw1[] = {
  { RES_ID_SW1_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW1_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw2[] = {
  { RES_ID_SW2_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW2_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw3[] = {
  { RES_ID_SW3_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW3_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw4[] = {
  { RES_ID_SW4_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW4_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　レール
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_rl1[] = {
  { RES_ID_RL1_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_RL1_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　レール
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_rl2[] = {
  { RES_ID_RL2_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_RL2_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　レール
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_rl3[] = {
  { RES_ID_RL3_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_RL3_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　レール
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_rl4[] = {
  { RES_ID_RL4_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_RL4_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //カプセル1
	{
		RES_ID_CAP_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAP_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_cap,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_cap),	//アニメリソース数
	},
  //カプセル2
  {
		RES_ID_CAP_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAP_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_cap,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_cap),	//アニメリソース数
	},
  //カプセル3
  {
		RES_ID_CAP_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAP_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_cap,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_cap),	//アニメリソース数
	},
  //カプセル4
  {
		RES_ID_CAP_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAP_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_cap,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_cap),	//アニメリソース数
	},

  //スイッチ1
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw1),	//アニメリソース数
	},
  //スイッチ2
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw2),	//アニメリソース数
	},
  //スイッチ3
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw3,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw3),	//アニメリソース数
	},
  //スイッチ4
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw4,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw4),	//アニメリソース数
	},

  //レール1
  {
		RES_ID_RL1_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_RL1_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_rl1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_rl1),	//アニメリソース数
	},
  //レール2
  {
		RES_ID_RL2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_RL2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_rl2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_rl2),	//アニメリソース数
	},
  //レール3
  {
		RES_ID_RL3_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_RL3_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_rl3,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_rl3),	//アニメリソース数
	},
  //レール4
  {
		RES_ID_RL4_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_RL4_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_rl4,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_rl4),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

static u8 GetPlatformIdx(const int inX, const int inZ);
static u8 GetRaleIdxByCapIdx(GYM_ELEC_TMP *tmp, const u8 inCapIdx);
static u8 GetRaleIdxByPlatformIdx(GYM_ELEC_TMP *tmp, const u8 inPlatformIdx);
static u8 GetCapIdxByPlatformIdx(const u8 inPlatformIdx);
static BOOL IsStopCapsuleToPlatForm(GYM_ELEC_TMP *tmp, const u8 inPlatformIdx);
static fx32 GetAnimeFrame(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inObjIdx, const u16 inAnmIdx);
static void SetAnimeFrame
         (FLD_EXP_OBJ_CNT_PTR ptr, const u16 inObjIdx, const u16 inAnmIdx, const int inFrame);
static GMEVENT_RESULT ChangePointEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT CapMoveEvt(GMEVENT* event, int* seq, void* work);
static GMEVENT_RESULT TrEncEvt(GMEVENT* event, int* seq, void* work);

static void ChgRale(FIELDMAP_WORK *fieldWork, FLD_EXP_OBJ_CNT_PTR ptr);
static void ChgRaleAnm(FLD_EXP_OBJ_CNT_PTR ptr, const u8 inSw, const u8 inCapIdx);
static BOOL CheckCapStopFrame(const u8 inRaleIdx, const fx32 inFrm,  u8 *outStopIdx);
static BOOL CheckChangableRaleAtOnce(FLD_EXP_OBJ_CNT_PTR ptr, const u8 inCapIdx, const u8 inRaleIdx);

static void CapStopTcbFunc(GFL_TCB* tcb, void* work);


static BOOL CheckCapTrEnc(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx);
static void SetCapTrEncFlg(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx);

static void StateSeFunc(FIELDMAP_WORK *fieldWork, GYM_ELEC_TMP *tmp);

#ifdef PM_DEBUG
BOOL test_GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx);
BOOL test_GYM_ELEC_CallMoveEvt(GAMESYS_WORK *gsys);
static void DbgChgRale(FIELDMAP_WORK *fieldWork, FLD_EXP_OBJ_CNT_PTR ptr);

static u8 dbg_frame_slow = 0;
static u8 dbg_count = 0;
#endif  //PM_DEBUG

static void
getJntSRTAnmResult_(const NNSG3dResJntAnm* pJntAnm, 
                    u32                    dataIdx, 
                    fx32                   Frame,
                    VecFx32 *outVec);
static void
getTransData_(fx32* pVal,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm);

static u8 CheckGetSeRect(FIELDMAP_WORK *fieldWork);

//--------------------------------------------------------------
/**
 * 現在座標からプラットホームインデックスを取得する
 * @param	    inX     Ｘ座標
 * @param     inZ     Ｚ座標
 * @return    u8      プラットフォームインデックス    存在しない場合はPLATFORM_NONE
 */
//--------------------------------------------------------------
static u8 GetPlatformIdx(const int inX, const int inZ)
{
  u8 i;
  for (i=0;i<PLATFORM_NUM_MAX;i++){
    if ( (PlatformXZ[i][0] == inX) &&
         (PlatformXZ[i][1] == inZ) ){
      return i;
    }
  }
  return PLATFORM_NONE;
}

//--------------------------------------------------------------
/**
 * カプセルのインデックスを渡し、セーブデータから走行中のレールインデックスを取得する
 * @param       tmp   電気ジムテンポラリワークポインタ
 * @param       inCapIdx      カプセルインデックス
 * @return      u8            レールインデックス
 */
//--------------------------------------------------------------
static u8 GetRaleIdxByCapIdx(GYM_ELEC_TMP *tmp, const u8 inCapIdx)
{
  //レールインデックス取得
  return tmp->NowRaleIdx[inCapIdx];
}

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GYM_ELEC_TMP *tmp;
  FIELD_SOUND* fs;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
    fs = GAMEDATA_GetFieldSound( gamedata );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ELEC_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ELEC_TMP));
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_ELEC_UNIT_IDX );  
  //座標セット　カプセル
  {
    VecFx32 pos = {CAP1_X, OBJ3D_Y, CAP1_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_CAP_1);
    status->trans = pos;
  }
  {
    VecFx32 pos = {CAP2_X, OBJ3D_Y, CAP2_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_CAP_2);
    status->trans = pos;
  }
  {
    VecFx32 pos = {CAP3_X, OBJ3D_Y, CAP3_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_CAP_3);
    status->trans = pos;
  }
  {
    VecFx32 pos = {CAP4_X, OBJ3D_Y, CAP4_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_CAP_4);
    status->trans = pos;
  }
  //座標セット　スイッチ
  {
    VecFx32 pos = {SW1_X, OBJ3D_Y, SW1_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_SW_1);
    status->trans = pos;
  }
  {
    VecFx32 pos = {SW2_X, OBJ3D_Y, SW2_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_SW_2);
    status->trans = pos;
  }
  {
    VecFx32 pos = {SW3_X, OBJ3D_Y, SW3_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_SW_3);
    status->trans = pos;
  }
  {
    VecFx32 pos = {SW4_X, OBJ3D_Y, SW4_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_SW_4);
    status->trans = pos;
  }

  //座標セット　レール
  {
    VecFx32 pos = {CAP1_X, OBJ3D_Y, CAP1_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_RL_1);
    status->trans = pos;
  }
  {
    VecFx32 pos = {CAP2_X, OBJ3D_Y, CAP2_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_RL_2);
    status->trans = pos;
  }
  {
    VecFx32 pos = {CAP3_X, OBJ3D_Y, CAP3_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_RL_3);
    status->trans = pos;
  }
  {
    VecFx32 pos = {CAP4_X, OBJ3D_Y, CAP4_Z};
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ELEC_UNIT_IDX, OBJ_RL_4);
    status->trans = pos;
  }

  //カプセル・スイッチ初期アニメ決定
  {
    u8 i;
    for (i=0;i<CAPSULE_NUM_MAX;i++){
      u8 obj_idx;
      u8 anm_idx;

      obj_idx = OBJ_CAP_1+i;
      if ( gmk_sv_work->LeverSw[i] ){
        anm_idx = ANM_CAP_MOV1+(i*2)+1; //切り替え後
        OS_Printf("%d 切り替え後\n",i);
      }else{
        anm_idx = ANM_CAP_MOV1+(i*2); //デフォルト
      }
      //レール移動の決定
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, TRUE);
      //蓋アニメをセットして止めておく
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL, TRUE);
      {
        EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //1回再生設定
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
        //フレームセット
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, gmk_sv_work->CapDat[i].AnmFrame );
      }
      //スイッチアニメの適用
      obj_idx = OBJ_SW_1+i;
      anm_idx = gmk_sv_work->LeverSw[i];
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, TRUE);
      {
        //1回再生設定
        EXP_OBJ_ANM_CNT_PTR anm;
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, 1);
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
        //ラストフレームセット
        {
          EXP_OBJ_ANM_CNT_PTR valid_anm;
          fx32 last;
          valid_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
          last = FLD_EXP_OBJ_GetAnimeLastFrame(valid_anm);
          FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, last );
        }
      }
      //レールアニメの適用
      obj_idx = OBJ_RL_1+i;
      anm_idx = gmk_sv_work->LeverSw[i];
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, TRUE);
      {
        //1回再生設定
        EXP_OBJ_ANM_CNT_PTR anm;
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, 1);
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
        //ラストフレームセット
        {
          EXP_OBJ_ANM_CNT_PTR valid_anm;
          fx32 last;
          valid_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
          last = FLD_EXP_OBJ_GetAnimeLastFrame(valid_anm);
          FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, last );
        }
      }
    }
  }
  
  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  //カプセル停止制御ＴＣＢ追加
  {
    GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldWork );
    // TCBを追加
    tmp->CapStopTcbPtr = GFL_TCB_AddTask( tcbsys, CapStopTcbFunc, fieldWork, 0 );
  }

  //メンバ初期化
  {
    int i;
    for (i=0;i<CAPSULE_NUM_MAX;i++)
    {
      tmp->RaleChgReq[i] = 0;
      tmp->NowRaleIdx[i] = (i*2)+gmk_sv_work->LeverSw[i];
      tmp->StopPlatformIdx[i] = PLATFORM_NO_STOP;
    }
    tmp->LeverIdx = -1;   //初期化
  }

  //イベント未起動状態セット
  tmp->RideEvt = -1;
  tmp->RadeRaleIdx = RIDE_NONE;

  //ＳＥ鳴らしっぱなし
  {
    SEPLAYER_ID player_id;
    player_id = PMSND_GetSE_DefaultPlayerID( GYM_ELEC_SE_SPEED );
    FSND_PlayEnvSE( fs, GYM_ELEC_SE_DRIVE );
    //始めはボリューム0
    FSND_SetEnvSEVol( fs, GYM_ELEC_SE_DRIVE, 0 );
    NNS_SndPlayerSetVolume( PMSND_GetSE_SndHandle( player_id ), 0 );
  }
}

//--------------------------------------------------------------
/**
 * カプセル監視ＴＣＢ
 * @param	    tcb       TCBポインタ
 * @param     work      ワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
static void CapStopTcbFunc(GFL_TCB* tcb, void* work)
{
  u8 i;
  FIELDMAP_WORK *fieldWork;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR obj_cnt_ptr;
  GYM_ELEC_TMP *tmp;
  FIELD_SOUND* fs;
  
  fieldWork = (FIELDMAP_WORK *)(work);
  obj_cnt_ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
    fs = GAMEDATA_GetFieldSound( gamedata );
  }

  //----リクエスト消化部----
  for (i=0;i<CAPSULE_NUM_MAX;i++){
    //リクエストチェック
    if ( tmp->RaleChgReq[i] ){
      //現在のスイッチの逆がチェックすべきスイッチ
      u8 old_sw = (gmk_sv_work->LeverSw[i] +1 ) % 2;
      u8 obj_idx = OBJ_CAP_1+i;
      u8 anm_idx = ANM_CAP_MOV1 + (i * 2) + old_sw;
      fx32 frm = GetAnimeFrame(obj_cnt_ptr, obj_idx, anm_idx);
      //アニメ切り替えをして良いプラットフォームに停車中か？
      if( frm == 0 ){
        //アニメ切り替え
        ChgRaleAnm(obj_cnt_ptr, old_sw, i);
        //走行レール保存
        tmp->NowRaleIdx[i] = (i*2)+gmk_sv_work->LeverSw[i];
        //リクエストフラグを落とす
        tmp->RaleChgReq[i] = 0;
      }
    }
  }
  //----カプセル停止制御部----
  for(i=0;i<CAPSULE_NUM_MAX;i++){
    u8 obj_idx = OBJ_CAP_1+i;
    u8 rale_idx = tmp->NowRaleIdx[i];
    u8 anm_idx = ANM_CAP_MOV1 + rale_idx;
    EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);

    //タイマーがセットされている場合は時間消化
    if (tmp->TaskWk[i].Timer){
      //乗降イベント中はタイマーカウントをフック
      if ( tmp->RideEvt == i ){
        continue;
      }
      //時間消化
      tmp->TaskWk[i].Timer--;
      //時間消化しきったか？
      if ( tmp->TaskWk[i].Timer == 0 ){
        //アニメストップを解除
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //自分が乗っているカプセルのときはループＳＥ再生
        if ( (tmp->RadeRaleIdx != RIDE_NONE)&&(i == tmp->RadeRaleIdx / 2) ){
          SEPLAYER_ID player_id;
          //ループＳＥスタート
          FSND_PlayEnvSE( fs, GYM_ELEC_SE_DRIVE );
          PMSND_PlaySE(GYM_ELEC_SE_SPEED);
          player_id = PMSND_GetSE_DefaultPlayerID( GYM_ELEC_SE_SPEED );
          //鳴らし始めはボリューム0にしとく
          FSND_SetEnvSEVol( fs, GYM_ELEC_SE_DRIVE, 0 );
          NNS_SndPlayerSetVolume( PMSND_GetSE_SndHandle( player_id ), 0 );

        }
        tmp->StopPlatformIdx[i] = PLATFORM_NO_STOP;
      }
    }else{
      u8 stop_idx;
      fx32 frame = FLD_EXP_OBJ_GetObjAnmFrm( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx );
      //停止すべきフレームかを調べる
      if ( CheckCapStopFrame(rale_idx, frame, &stop_idx) ){
        //アニメをストップ
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //自分の乗っているカプセルの場合はループＳＥをストップ
        if ( (tmp->RadeRaleIdx != RIDE_NONE)&&(i == tmp->RadeRaleIdx / 2) ){
          //ループＳＥストップ
          PMSND_StopSE();
        }
        //指定時間とめるためタイマーをセット
        tmp->TaskWk[i].Timer = STOP_TIME;
        tmp->StopPlatformIdx[i] = RaleStopPlatform[rale_idx][stop_idx];
      }
    }
//    OS_Printf("%d::stop_plat %d\n",i, gmk_sv_work->StopPlatformIdx[i]);
  } //end for
  
  //--自機自動移動部--
  if (tmp->AltoMove){
    fx32 frame;
    u8 cap_idx;
    u8 anm_idx;
    int frm_idx;
    VecFx32 dst_vec = {0,OBJ3D_Y,0};

    GF_ASSERT(tmp->RadeRaleIdx != RIDE_NONE);
    //レールインデックス/2でカプセルインデックスになる
    cap_idx = tmp->RadeRaleIdx / 2;
    anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
    //アニメフレーム取得
    frame = FLD_EXP_OBJ_GetObjAnmFrm( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, cap_idx, anm_idx );
    frm_idx = frame / FX32_ONE;
    
    //座標取得
    if ( ICA_ANIME_GetTranslateAt( tmp->IcaAnmPtr, &dst_vec, frm_idx ) ){
      dst_vec.x += CapPos[cap_idx].x;
      dst_vec.y += CapPos[cap_idx].y;
      dst_vec.z += CapPos[cap_idx].z;
      //自機座標セット
      FIELD_PLAYER_SetPos( FIELDMAP_GetFieldPlayer( fieldWork ), &dst_vec );
    }
  }
  else    //カプセルに乗っていないときボリューム変更
  {
    StateSeFunc(fieldWork, tmp);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_End(FIELDMAP_WORK *fieldWork)
{
  u8 i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  //ＳＥストップ
  PMSND_StopSE();
/**
  for(i=0;i<RALE_NUM_MAX;i++){
    ICA_ANIME_Delete( tmp->IcaAnmPtr2[i] );
  }
*/
  //監視ＴＣＢ削除
  GFL_TCB_DeleteTask( tmp->CapStopTcbPtr );
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_ELEC_UNIT_IDX );

}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }
#ifdef PM_DEBUG
  //テスト
  {
#if 1    
    if (GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT ){
      FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
      GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
      if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
        tmp->LeverIdx++;
        if (tmp->LeverIdx>=4){
          tmp->LeverIdx = 0;
        }
        OS_Printf("now_idx = %d\n",tmp->LeverIdx);
      }
      DbgChgRale(fieldWork, ptr);
    }
#endif
    if ( GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG ){
      dbg_frame_slow = 1;
    }else{
      dbg_frame_slow = 0;
    }
  }
#endif  //PM_DEBUG


#if 1
#ifdef PM_DEBUG
  if (dbg_frame_slow){
    dbg_count++;
    if (dbg_count >= 30){
      dbg_count = 0;
      //アニメーション再生
      FLD_EXP_OBJ_PlayAnime( ptr );
      //今時分の追っているレールのフレームを取得して表示
      for (i=0;i<CAPSULE_NUM_MAX;i++){
        u8 obj_idx;
        u8 anm_idx;
        obj_idx = OBJ_CAP_1 + i;
        anm_idx = ANM_CAP_MOV1 + tmp->NowRaleIdx[i];
        OS_Printf("now_rale = %d\n", tmp->NowRaleIdx[i]);
        OS_Printf("%d now_frm = %d\n", i,GetAnimeFrame(ptr, obj_idx, anm_idx));
      }
    }
  }else{
    //アニメーション再生
    FLD_EXP_OBJ_PlayAnime( ptr );
  }
#endif //PM_DEBUG
#else
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
#endif

  //アニメフレームをセーブデータに保存
  for (i=0;i<CAPSULE_NUM_MAX;i++){
    u8 obj_idx;
    u8 anm_idx;
    obj_idx = OBJ_CAP_1 + i;
    anm_idx = ANM_CAP_MOV1 + tmp->NowRaleIdx[i];
    gmk_sv_work->CapDat[i].AnmFrame = GetAnimeFrame(ptr, obj_idx, anm_idx);
  }
}

//--------------------------------------------------------------
/**
 * 指定プラットホームの現在のレールインデックスを取得
 * @param	      tmp     電気ジムテンポラリワークポインタ
 * @param       inPlatformIdx   プラットフォームインデックス
 * @return      u8              レールインデックス
 */
//--------------------------------------------------------------
static u8 GetRaleIdxByPlatformIdx(GYM_ELEC_TMP *tmp, const u8 inPlatformIdx)
{
  u8 cap_idx;
  u8 rale_idx;
  //指定プラットフォームに停車するカプセルを取得
  cap_idx = GetCapIdxByPlatformIdx(inPlatformIdx);
  //取得したカプセルが走行しているレールインデックスを取得
  rale_idx = GetRaleIdxByCapIdx(tmp, cap_idx);

  return rale_idx;
}

//--------------------------------------------------------------
/**
 * プラットフォームインデックスからカプセルインデックスを取得
 * @param	        inPlatformIdx     プラットフォームインデックス
 * @return        u8                カプセルインデックス
 */
//--------------------------------------------------------------
static u8 GetCapIdxByPlatformIdx(const u8 inPlatformIdx)
{
  u8 cap_idx;
  //指定プラットフォームに停車するカプセルを取得
  cap_idx = PlatStpCapIdx[inPlatformIdx];
  return cap_idx;
}

//--------------------------------------------------------------
/**
 * 指定プラットホームにカプセルが停車しているかを調べる
 * @param	      tmp     電気ジムテンポラリワークポインタ
 * @param       inPlatformIdx   プラットフォームインデックス
 * @return      BOOL    TRUEで停止
 */
//--------------------------------------------------------------
static BOOL IsStopCapsuleToPlatForm(GYM_ELEC_TMP *tmp, const u8 inPlatformIdx)
{
  u8 i;
  u8 cap_idx;
  int platform_idx;

  //指定プラットホームのカプセルインデックスを取得
  cap_idx = GetCapIdxByPlatformIdx(inPlatformIdx);
  //取得カプセルインデックスの停車プラットホームを取得
  platform_idx = tmp->StopPlatformIdx[cap_idx];
  OS_Printf("%d %d %d\n",platform_idx, cap_idx, tmp->StopPlatformIdx[cap_idx]);
  if (platform_idx == PLATFORM_NO_STOP){
    //停車していない
    return FALSE;
  }
  //プラットホームインデックス比較
  if (platform_idx == inPlatformIdx){
    //指定プラットホームに停車
    return TRUE;
  }

  //指定プラットホームに停車していない
  return FALSE;
  
}

//--------------------------------------------------------------
/**
 * 指定レールの現在アニメフレームを取得
 * @param   ptr       拡張ＯＢＪ管理ポインタ
 * @param   inObjIdx  ＯＢＪインデックス
 * @param   inAnmIdx  アニメインデックス
 * @return  fx32      フレーム
 */
//--------------------------------------------------------------
static fx32 GetAnimeFrame(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inObjIdx, const u16 inAnmIdx)
{
  BOOL rc;
  int frame = 0;
  GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, GYM_ELEC_UNIT_IDX, inObjIdx);
  rc = GFL_G3D_OBJECT_GetAnimeFrame( g3Dobj, inAnmIdx, &frame );
  GF_ASSERT(rc);
  return frame;
}

//--------------------------------------------------------------
/**
 * 指定レールの現在アニメフレームを取得
 * @param   ptr	        拡張ＯＢＪコントロールポインタ
 * @param   inObjIdx    OBJインデックス
 * @param   inAnmIdx    アニメインデックス
 * @param   inFrame     アニメフレーム　int型ですが、内部ではfx32型にキャストされます
 *
 * @return  none
 */
//--------------------------------------------------------------
static void SetAnimeFrame(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inObjIdx, const u16 inAnmIdx, const int inFrame)
{
  BOOL rc;
  GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, GYM_ELEC_UNIT_IDX, inObjIdx);
  rc = GFL_G3D_OBJECT_SetAnimeFrame( g3Dobj, inAnmIdx, &inFrame );
  GF_ASSERT(rc);
}

//--------------------------------------------------------------
/**
 * レバーの切り替えを行うイベント起動
 * @param         gsys        ゲームシステムポインタ
 * @param         inLeverIdx  スイッチインデックス
 * @return        event       イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx)
{
  GMEVENT * event;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );

  if (inLeverIdx >= LEVER_NUM_MAX){
    return NULL;
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ChangePointEvt, 0 );
    
  //操作予定レバーインデックスをセット
  tmp->LeverIdx = inLeverIdx;

  return event;
}

//--------------------------------------------------------------
/**
 * レバーの切り替えイベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ChangePointEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }
  
  switch(*seq){
  case 0:  //レバーアニメ切り替え
    {      
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 sw_obj_idx;
      u8 rl_obj_idx;
      tmp->ChgingSw = gmk_sv_work->LeverSw[tmp->LeverIdx];
      sw_obj_idx = OBJ_SW_1 + tmp->LeverIdx;
      rl_obj_idx = OBJ_RL_1 + tmp->LeverIdx;
    //操作使用としているレバーインデックスから現在の倒れ方向を見て、逆のアニメを再生する
      if (tmp->ChgingSw==0){
        //アニメを無効
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 0, FALSE);
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 0, FALSE);
        //アニメを開始
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 1, TRUE);
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 1, TRUE);

        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 1);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 1);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //頭出し
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 1, 0 );
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 1, 0 );
        tmp->ChgingSw = 1;
      }else{
        //アニメを無効
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 1, FALSE);
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 1, FALSE);
        //アニメを開始
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 0, TRUE);
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 0, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //頭出し
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, 0, 0 );
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, 0, 0 );
        tmp->ChgingSw = 0;
      }

      //スイッチ押下ＳＥ再生
      PMSND_PlaySE(GYM_ELEC_SE_PUSH_SW);
    }    
    (*seq)++;
    break;
  case 1: //アニメ待ち
    {
      
      u8 sw_obj_idx;
      u8 rl_obj_idx;
      EXP_OBJ_ANM_CNT_PTR sw_anm;
      EXP_OBJ_ANM_CNT_PTR rl_anm;
      sw_obj_idx = OBJ_SW_1 + tmp->LeverIdx;
      rl_obj_idx = OBJ_RL_1 + tmp->LeverIdx;
      sw_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, sw_obj_idx, tmp->ChgingSw);
      rl_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, rl_obj_idx, tmp->ChgingSw);
      if ( FLD_EXP_OBJ_ChkAnmEnd(sw_anm)&&FLD_EXP_OBJ_ChkAnmEnd(rl_anm) ){
        OS_Printf("アニメ終了\n");
        //路線ポイント切り替え（アニメ切り替え）
        ChgRale(fieldWork, ptr);
        tmp->LeverIdx = -1;   //操作終了
        return GMEVENT_RES_FINISH;
      }
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * プラットホーム間を移動している間のイベント
 * @param	      gsys        ゲームシステムポインタ
 * @return      event       イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_ELEC_CreateMoveEvt(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  GYM_ELEC_TMP *tmp_work;
  u8 platform_idx;
  
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  tmp_work = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  {
    int x,z;
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    //自機が上向きでない場合は終了
    if ( FIELD_PLAYER_GetDir( fld_player ) != DIR_UP ){
      return NULL;
    }

    FIELD_PLAYER_GetPos( fld_player, &pos );
    x = pos.x / FIELD_CONST_GRID_FX32_SIZE;
    z = pos.z / FIELD_CONST_GRID_FX32_SIZE;
    //自機の位置から、プラットフォーム番号を調べる
    platform_idx = GetPlatformIdx(x,z);
    OS_Printf("platform = %d\n",platform_idx);
  }

  //プラットホームが見つからない場合は終了
  if (platform_idx == PLATFORM_NONE){
    return NULL;
  }
  
  //カプセルがプラットフォームに停止しているか？
  if ( IsStopCapsuleToPlatForm(tmp_work, platform_idx) ){ //停止している
    u8 cap_idx = GetCapIdxByPlatformIdx(platform_idx);
    {
      //自機が走行するレールインデックスを保存
      tmp_work->RadeRaleIdx = GetRaleIdxByPlatformIdx(tmp_work, platform_idx);
      OS_Printf("RideRaleIdx %d\n",tmp_work->RadeRaleIdx);
      //カプセル移動アニメを停止
      {
        FLD_EXP_OBJ_CNT_PTR obj_cnt_ptr;
        EXP_OBJ_ANM_CNT_PTR anm;
        u8 obj_idx = OBJ_CAP_1+cap_idx;
        u8 anm_idx = ANM_CAP_MOV1 + tmp_work->RadeRaleIdx;
        obj_cnt_ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
        anm = FLD_EXP_OBJ_GetAnmCnt( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //乗降開始
        tmp_work->RideEvt = cap_idx;
      }
    }
    //カプセル内トレーナーエンカウントチェック
    if ( CheckCapTrEnc(gmk_sv_work, cap_idx) ){
      //カプセル内トレーナーとのエンカウントイベント作成
      event = GMEVENT_Create( gsys, NULL, TrEncEvt, 0 );
    }else{
      //次のプラットフォームまでの移動イベントを作成する
      event = GMEVENT_Create( gsys, NULL, CapMoveEvt, 0 );
    }

    return event;
  }
  return NULL;
}

//--------------------------------------------------------------
/**
 * トレーナー戦終了フラグセット
 * @param	      gsys            ゲームシステムポインタ
 * @param       inCapIdx        カプセルインデックス
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_SetTrEncFlg(GAMESYS_WORK *gsys, const int inCapIdx)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }
  
  SetCapTrEncFlg(gmk_sv_work, inCapIdx);
}

//--------------------------------------------------------------
/**
 * トレーナー戦チェック
 * @param	      gmk_sv_work     電気ジムセーブワークポインタ
 * @param       inCapIdx        カプセルインデックス	
 * @return      BOOL            TRUEで未戦闘
 */
//--------------------------------------------------------------
static BOOL CheckCapTrEnc(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx)
{
  u8 check = 0;   //チェックフラグ 1でチェック処理する
  u8 evt_idx;
  //第2、第3カプセル（インデックス　1,2のカプセル）のときチェックする
  if (inCapIdx == 1){
    check = 1;
    evt_idx = 0;
  }else if(inCapIdx == 2){
    check =1;
    evt_idx = 1;
  }

  if (check){
    if ( !gmk_sv_work->EvtFlg[evt_idx] ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * トレーナー戦終了フラグセット
 * @param	      gmk_sv_work     電気ジムセーブワークポインタ
 * @param       inCapIdx        カプセルインデックス
 * @return      none
 */
//--------------------------------------------------------------
static void SetCapTrEncFlg(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx)
{
  u8 set = 0;   //チェックフラグ 1でセット処理する
  u8 evt_idx;
  //第2、第3カプセル（インデックス　1,2のカプセル）のときセットする
  if (inCapIdx == 1){
    set = 1;
    evt_idx = 0;
  }else if(inCapIdx == 2){
    set =1;
    evt_idx = 1;
  }else{
    GF_ASSERT(0);
  }

  if (set){
    gmk_sv_work->EvtFlg[evt_idx] = 1;
  }
}

//--------------------------------------------------------------
/**
 * カプセル移動イベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CapMoveEvt(GMEVENT* event, int* seq, void* work)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  FIELD_SOUND* fs;
  u8 cap_idx;
  u8 obj_idx;
  EXP_OBJ_ANM_CNT_PTR anm;
  int volume;
  SEPLAYER_ID player_id;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    fs = GAMEDATA_GetFieldSound( gamedata );
  }

  player_id = PMSND_GetSE_DefaultPlayerID( GYM_ELEC_SE_SPEED );

  GF_ASSERT(tmp->RadeRaleIdx != RIDE_NONE);

  //レールインデックス/2でカプセルインデックスになる
  cap_idx = tmp->RadeRaleIdx / 2;
  obj_idx = OBJ_CAP_1+cap_idx;

  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

  switch(*seq){
  case 0:
    {
      u16 arc_idx;
      //自機移動データロード
      OS_Printf("ロードレール%d\n",tmp->RadeRaleIdx);
      arc_idx = RaleAnm[tmp->RadeRaleIdx];
      tmp->IcaAnmPtr = ICA_ANIME_Create( 
          GFL_HEAP_LOWID(HEAPID_FIELDMAP), ARCID_GYM_ELEC, arc_idx, tmp->FramePosDat, FRAME_POS_SIZE*2
          );

      GF_ASSERT(tmp->IcaAnmPtr != NULL);

      //カプセル開くアニメスタート
      {
        //フレーム頭だし
        SetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL, 0);
        //再生
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //SE再生
        PMSND_PlaySE(GYM_ELEC_SE_CAP_OC);
      }
    }
    (*seq)++;
    break;
  case 1:
    {
      fx32 frame;
      frame = GetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL);
      //カプセル開くアニメ待ち
      if (frame >= CAP_OPEN_FRAME){
        //カプセル開くアニメ止める
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //自機がカプセルに乗るスクリプトをコール
        SCRIPT_CallScript( event, SCRID_PRG_C04GYM0101_CAPSULE_IN,
          NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
        (*seq)++;
      }
    }
    break;
  case 2:
    //カプセル閉まるアニメスタート
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //SE再生
    PMSND_PlaySE(GYM_ELEC_SE_CAP_OC);
    (*seq)++;
    break;
  case 3:
    //カプセル閉まるアニメ待ち
    if( FLD_EXP_OBJ_ChkAnmEnd(anm) ){
      EXP_OBJ_ANM_CNT_PTR cap_anm;
      u8 anm_idx;
      {
        MMDL * mmdl;
        mmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
        //主人公消す
        MMDL_SetStatusBitVanish(mmdl, TRUE);
      }
      //カプセル蓋アニメ止める
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
      //カプセル移動アニメ再開
      anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
      cap_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
      FLD_EXP_OBJ_ChgAnmStopFlg(cap_anm, 0);

      //タイマークリア
      tmp->TaskWk[cap_idx].Timer = 0;
      tmp->StopPlatformIdx[cap_idx] = PLATFORM_NO_STOP;

      //乗降終了
      tmp->RideEvt = -1;

      //ループＳＥスタート
      FSND_PlayEnvSE( fs, GYM_ELEC_SE_DRIVE );
      PMSND_PlaySE(GYM_ELEC_SE_SPEED);
      //鳴らし始めはボリューム0にしとく
      NNS_SndPlayerSetVolume( PMSND_GetSE_SndHandle( player_id ), 0 );
      //ボリュームセット
      FSND_SetEnvSEVol( fs, GYM_ELEC_SE_DRIVE, SE_VOL );
      //自機自動移動開始
      tmp->AltoMove = TRUE;

      (*seq)++;
    }
    break;
  case 4:
    //プラットホーム到着待ち
    if( tmp->StopPlatformIdx[cap_idx] != PLATFORM_NO_STOP ){
      //プレーヤ入力待ち
      if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){   //入力あり
        (*seq)++;
      }
    }
    break;
  case 5:
    {
      EXP_OBJ_ANM_CNT_PTR cap_anm;
      u8 anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
      cap_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);

      //乗降開始
      tmp->RideEvt = cap_idx;
      //カプセル移動アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(cap_anm, 1);
      //カプセル開くアニメスタート
      {
        //フレーム頭だし
        SetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL, 0);
        //再生
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //SE再生
        PMSND_PlaySE(GYM_ELEC_SE_CAP_OC);
        {
          MMDL * mmdl;
          FIELD_PLAYER *fld_player;
          fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
          mmdl = FIELD_PLAYER_GetMMdl( fld_player );
          //下方向をセット
          MMDL_SetDirDisp(mmdl,DIR_DOWN);
          //主人公表示
          MMDL_SetStatusBitVanish(mmdl, FALSE);
        }
      }
    }
    (*seq)++;
    break;
  case 6:
    {
      fx32 frame;
      frame = GetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL);
      //カプセル開くアニメ待ち
      if (frame >= CAP_OPEN_FRAME){
        //カプセル開くアニメ止める
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        {
          FIELD_PLAYER *fld_player;
          fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
          //主人公のカプセル出る座標セット
          {
            u8 platform_idx;
            VecFx32 pos = {0,OBJ3D_Y,0};
            platform_idx = tmp->StopPlatformIdx[cap_idx];
            pos.x = PlatformXZ[platform_idx][0] * FIELD_CONST_GRID_FX32_SIZE;
            pos.x += GRID_HALF_SIZE;
            pos.z = (PlatformXZ[platform_idx][1]-2) * FIELD_CONST_GRID_FX32_SIZE;
            pos.z += GRID_HALF_SIZE;
            FIELD_PLAYER_SetPos( fld_player, &pos );
          }
        }
        //自機自動移動終了
        tmp->AltoMove = FALSE;
        (*seq)++;
      }
    }
    break;
  case 7:
    //自機がカプセルを降りるスクリプトをコール
    SCRIPT_CallScript( event, SCRID_PRG_C04GYM0101_CAPSULE_OUT,
        NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    (*seq)++;
    break;
  case 8:
    //カプセル閉まるアニメスタート
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //SE再生
    PMSND_PlaySE(GYM_ELEC_SE_CAP_OC);
    (*seq)++;
    break;
  case 9:
    //カプセル閉まるアニメ待ち
    if( FLD_EXP_OBJ_ChkAnmEnd(anm) ){
      EXP_OBJ_ANM_CNT_PTR cap_anm;
      u8 anm_idx;
      //カプセル蓋アニメ止める
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
      //カプセル移動アニメ再開
      anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
      cap_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
      FLD_EXP_OBJ_ChgAnmStopFlg(cap_anm, 0);

      //タイマークリア
      tmp->TaskWk[cap_idx].Timer = 0;
      tmp->StopPlatformIdx[cap_idx] = PLATFORM_NO_STOP;

      ICA_ANIME_Delete( tmp->IcaAnmPtr );
      
      //乗降終了
      tmp->RideEvt = -1;
      tmp->RadeRaleIdx = RIDE_NONE;
      
      {
        SEPLAYER_ID player_id;
        //カプセル近接ＳＥのためGYM_ELEC_SE_DRIVEは鳴らし続ける
        FSND_PlayEnvSE( fs, GYM_ELEC_SE_DRIVE );
        //始めはボリューム0
        FSND_SetEnvSEVol( fs, GYM_ELEC_SE_DRIVE, 0 );
        //加速音はストップする
        player_id = PMSND_GetSE_DefaultPlayerID( GYM_ELEC_SE_SPEED );
        PMSND_StopSE_byPlayerID( player_id );
      }
      
      return GMEVENT_RES_FINISH;
    }
  }

  //ＳＥ制御
  if ( (tmp->RadeRaleIdx == 2)||(tmp->RadeRaleIdx == 3)||(tmp->RadeRaleIdx == 4)||(tmp->RadeRaleIdx == 5) )
  {
    u8 anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
    fx32 frm = GetAnimeFrame(ptr, obj_idx, anm_idx);

    switch( tmp->RadeRaleIdx ){
    case 2:
      if (frm == UP_SE_FRM1){
        PMSND_StopSE();
        //昇りＳＥスタート
        PMSND_PlaySE(GYM_ELEC_SE_RISE);
      }else if(frm == TOP_SE_STOP_FRM1){
        //SEストップ
        PMSND_StopSE();
      }else if(frm == DOWN_SE_FRM1){
        //下りＳＥスタート
        PMSND_PlaySE(GYM_ELEC_SE_DROP);
      }
      break;
    case 3:
      if (frm == UP_SE_FRM2){
        PMSND_StopSE();
        //昇りＳＥスタート
        PMSND_PlaySE(GYM_ELEC_SE_RISE);
      }else if(frm == TOP_SE_STOP_FRM2){
        //SEストップ
        PMSND_StopSE();
      }else if(frm == DOWN_SE_FRM2){
        //下りＳＥスタート
        PMSND_PlaySE(GYM_ELEC_SE_DROP);
      };
      break;
    case 4:
      if (frm == LOOP_SE_FRM1){
        PMSND_StopSE();
        //宙返りＳＥスタート
        PMSND_PlaySE(GYM_ELEC_SE_LOOP);
      }
      break;
    case 5:
      if (frm == LOOP_SE_FRM2){
        PMSND_StopSE();
        //宙返りＳＥスタート
        PMSND_PlaySE(GYM_ELEC_SE_LOOP);
      }
      break;
    }
  }

  //アニメデータ取得
  {
    fx32 now_frm,next_frm;
    VecFx32 now,next, dst;
    fx32 len;
    NNSG3dAnmObj* anm_obj_ptr;
    GFL_G3D_ANM*  gfl_anm;
    GFL_G3D_OBJ* g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, GYM_ELEC_UNIT_IDX, obj_idx);
    u8 anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
    EXP_OBJ_ANM_CNT_PTR anm_ptr = FLD_EXP_OBJ_GetAnmCnt(ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
    fx32 last_frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm_ptr );

    now_frm = GetAnimeFrame(ptr, obj_idx, anm_idx);
    next_frm = now_frm+FX32_ONE;
    if ( next_frm >= last_frm) next_frm = 0;

//    OS_Printf("%x, %x, %x\n",now_frm, next_frm, last_frm);

    gfl_anm = GFL_G3D_OBJECT_GetG3Danm( g3Dobj, anm_idx );
    anm_obj_ptr = GFL_G3D_ANIME_GetAnmObj( gfl_anm );
    getJntSRTAnmResult_(anm_obj_ptr->resAnm,
                        0,
                        now_frm,
                        &now);
    getJntSRTAnmResult_(anm_obj_ptr->resAnm,
                        0,
                        next_frm,
                        &next);

    VEC_Subtract( &now, &next ,&dst );
    len = VEC_Mag( &dst );
//    OS_Printf("len = %x\n",len);
    if ( len >=SPD_LV5 ){
//      OS_Printf("11111\n");
      volume = 127;
    }else if ( len >=SPD_LV4 ){
//      OS_Printf("1111\n");
      volume = 90;
    }else if ( len >=SPD_LV3 ){
//      OS_Printf("111\n");
      volume = 60;
    }else if( len >= SPD_LV2 ){
//      OS_Printf("11\n");
      volume = 30;
    }else{
//      OS_Printf("1\n");
      volume = 0;
    }
  }

  NNS_SndPlayerSetVolume( PMSND_GetSE_SndHandle( player_id ), volume );

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トレーナー戦イベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT TrEncEvt(GMEVENT* event, int* seq, void* work)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  u8 cap_idx;
  u8 obj_idx;
  EXP_OBJ_ANM_CNT_PTR anm;

  GF_ASSERT(tmp->RadeRaleIdx != RIDE_NONE);

  //レールインデックス/2でカプセルインデックスになる
  cap_idx = (tmp->RadeRaleIdx) / 2;
  obj_idx = OBJ_CAP_1+cap_idx;

  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

  switch(*seq){
  case 0:
    //トレーナー出現スクリプトコール
    SCRIPT_CallScript( event, SCRID_PRG_C04GYM0101_TR_DISP_ON,
        NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    (*seq)++;
    break;
  case 1:
    //カプセル開くアニメスタート
      {
        //フレーム頭だし
        SetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL, 0);
        //再生
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //SE再生
        PMSND_PlaySE(GYM_ELEC_SE_CAP_OC); 
      }
      (*seq)++;
    break;
  case 2:
    {
      fx32 frame;
      frame = GetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL);
      //カプセル開くアニメ待ち
      if (frame >= CAP_OPEN_FRAME){
        //カプセル開くアニメ止める
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //アニメスクリプトコール
        SCRIPT_CallScript( event, SCRID_PRG_C04GYM0101_SCR01,
          NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
        (*seq)++;
      }      
    }
    break;
  case 3:
    //カプセル閉まるアニメスタート
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //SE再生
    PMSND_PlaySE(GYM_ELEC_SE_CAP_OC);
    (*seq)++;
    break;
  case 4:
    //カプセル閉まるアニメ待ち
    if( FLD_EXP_OBJ_ChkAnmEnd(anm) ){
      EXP_OBJ_ANM_CNT_PTR cap_anm;
      u8 anm_idx;
      //カプセル蓋アニメ止める
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
      //カプセル移動アニメ再開
      anm_idx = ANM_CAP_MOV1 + tmp->RadeRaleIdx;
      cap_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
      FLD_EXP_OBJ_ChgAnmStopFlg(cap_anm, 0);

      //タイマークリア
      tmp->TaskWk[cap_idx].Timer = 0;
      tmp->StopPlatformIdx[cap_idx] = PLATFORM_NO_STOP;

      //スクリプトチェンジ
      SCRIPT_ChangeScript( event, SCRID_PRG_C04GYM0101_SCR02,
          NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
      (*seq)++;
    }
    break;
  case 5:
    GF_ASSERT(0);   //イベントチェンジしているので、ここには来ない
    //※トレーナーバトル負けを考慮し、イベントコールからイベントチェンジに変更したので、ここには来なくなりました。
#if 0
    //乗降終了
    {
      //↓ここにくるときにセットアップ関数を通るので、明示的な代入は不要だが念のため。
      tmp->RideEvt = -1;
      tmp->RadeRaleIdx = RIDE_NONE;
    }
    SetCapTrEncFlg(gmk_sv_work, cap_idx);
#endif
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * レール変更
 * @param   fieldWork         フィールドワークポインタ
 * @param   ptr               拡張ＯＢＪ管理ポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void ChgRale(FIELDMAP_WORK *fieldWork, FLD_EXP_OBJ_CNT_PTR ptr)
{
  u8 cap_idx;
  u8 obj;
  u8 sw;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );

  if(tmp->LeverIdx == -1)
  {
    GF_ASSERT(0);
    return;
  }

  //操作しようとしているレバーのスイッチがどちらに入っているかを調べる  
  sw = gmk_sv_work->LeverSw[tmp->LeverIdx];
  obj = OBJ_CAP_1+tmp->LeverIdx;
  cap_idx = tmp->LeverIdx;    //スイッチインデックスがカプセルインデックスに対応

  //セーブデータのスイッチ切り替え
  gmk_sv_work->LeverSw[tmp->LeverIdx] = (gmk_sv_work->LeverSw[tmp->LeverIdx]+1)%2;
  //すぐにレールアニメを切り替えられるならば変える
  if ( CheckChangableRaleAtOnce(ptr, cap_idx, tmp->NowRaleIdx[tmp->LeverIdx]) ){
    ChgRaleAnm(ptr, sw, tmp->LeverIdx);
    //走行レール保存
    tmp->NowRaleIdx[tmp->LeverIdx] =
      (tmp->LeverIdx*2)+gmk_sv_work->LeverSw[tmp->LeverIdx];
  }else{
    //変えられない場合はリクエストを出す
    if (tmp->RaleChgReq[tmp->LeverIdx]){
      //既にリクエストされているならば、リクエストをキャンセル
      tmp->RaleChgReq[tmp->LeverIdx] = 0;
    }else{
      //リクエストされていないならば、リクエストする
      tmp->RaleChgReq[tmp->LeverIdx] = 1;
    }
  }
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * レール変更
 * @param   fieldWork         フィールドワークポインタ
 * @param   ptr               拡張ＯＢＪ管理ポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void DbgChgRale(FIELDMAP_WORK *fieldWork, FLD_EXP_OBJ_CNT_PTR ptr)
{
  u8 cap_idx;
  u8 obj;
  u8 sw;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );

  if(tmp->LeverIdx == -1)
  {
    return;
  }

  //操作しようとしているレバーのスイッチがどちらに入っているかを調べる  
  sw = gmk_sv_work->LeverSw[tmp->LeverIdx];
  obj = OBJ_CAP_1+tmp->LeverIdx;
  cap_idx = tmp->LeverIdx;    //スイッチインデックスがカプセルインデックスに対応

  //セーブデータのスイッチ切り替え
  gmk_sv_work->LeverSw[tmp->LeverIdx] = (gmk_sv_work->LeverSw[tmp->LeverIdx]+1)%2;
  //すぐにレールアニメを切り替えられるならば変える
  if ( CheckChangableRaleAtOnce(ptr, cap_idx, tmp->NowRaleIdx[tmp->LeverIdx]) ){
    ChgRaleAnm(ptr, sw, tmp->LeverIdx);
    //走行レール保存
    tmp->NowRaleIdx[tmp->LeverIdx] =
      (tmp->LeverIdx*2)+gmk_sv_work->LeverSw[tmp->LeverIdx];
  }else{
    //変えられない場合はリクエストを出す
    if (tmp->RaleChgReq[tmp->LeverIdx]){
      //既にリクエストされているならば、リクエストをキャンセル
      tmp->RaleChgReq[tmp->LeverIdx] = 0;
    }else{
      //リクエストされていないならば、リクエストする
      tmp->RaleChgReq[tmp->LeverIdx] = 1;
    }
  }
}

#endif

//--------------------------------------------------------------
/**
 * レールアニメ変更
 * @param   ptr         拡張ＯＢＪ管理ポインタ
 * @param   inSw        スイッチフラグ
 * @param   inCapIdx    カプセルインデックス
 * @return  none
 */
//--------------------------------------------------------------
static void ChgRaleAnm(FLD_EXP_OBJ_CNT_PTR ptr, const u8 inSw, const u8 inCapIdx)
{
  u8 obj_idx;
  u8 rale_anm_before;
  u8 rale_anm_next;
  fx32 frame;
  if (inSw==0){   //現在、デフォルト
    rale_anm_before = ANM_CAP_MOV1+(inCapIdx*2);
    rale_anm_next = rale_anm_before+1;
  }else{        //現在、非デフォルト
    rale_anm_next = ANM_CAP_MOV1+(inCapIdx*2);
    rale_anm_before = rale_anm_next+1;
  }
  obj_idx = OBJ_CAP_1+inCapIdx;
  frame = GetAnimeFrame(ptr, obj_idx, rale_anm_before);
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, rale_anm_before, FALSE);
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, rale_anm_next, TRUE);
  //フレームの引継ぎ
  SetAnimeFrame(ptr, obj_idx, rale_anm_next, frame);
  //ストップフラグ引継ぎ
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    u8 stop;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, rale_anm_before);
    stop = FLD_EXP_OBJ_GetAnmStopFlg(anm);
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, rale_anm_next);
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, stop);
  }

}

//--------------------------------------------------------------
/**
 * 停止すべきフレームか？
 * @param	    inRaleInx       レールインデックス
 * @param     inFrm           指定フレーム
 * @param     outStopIdx      停止するプラットフォームインデックス格納バッファ
 * @return    BOOL            停止すべきフレームのときTRUE
 */
//--------------------------------------------------------------
static BOOL CheckCapStopFrame(const u8 inRaleIdx, const fx32 inFrm, u8 *outStopIdx)
{
  u8 i;
  for (i=0;i<RALE_PLATFORM_NUM_MAX;i++){
    if ( inFrm == StopFrame[inRaleIdx][i] ){
      *outStopIdx = i;
      return TRUE;
    }
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * 即レールアニメ切り替えできるか？
 * @param	    ptr             拡張ＯＢＪ管理ポインタ
 * @param     inCapIdx        カプセルインデックス
 * @param     inRraleIdx      レールインデックス
 * @return    BOOL  TRUEで可
 */
//--------------------------------------------------------------
static BOOL CheckChangableRaleAtOnce(FLD_EXP_OBJ_CNT_PTR ptr, const u8 inCapIdx, const u8 inRaleIdx)
{
  fx32 frame;
  //指定レールインデックスのレールアニメの現在アニメフレームを取得
  {
    u8 obj_idx;
    u16 anm_idx;
    //レバー番号がカプセル番号
    obj_idx = OBJ_CAP_1+inCapIdx;
    anm_idx = ANM_CAP_MOV1 + inRaleIdx;
    //指定レールのアニメフレームを取得
    frame = GetAnimeFrame(ptr, obj_idx, anm_idx);
    OS_Printf("frame=%x,%d\n",frame, frame/FX32_ONE);
  }
  
  if (frame <= CommFrame[inCapIdx]*FX32_ONE){
    return TRUE;
  }
  return FALSE;
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * レバーの切り替えを行うイベント起動
 * @param	      gsys    ゲームシステムポインタ
 * @param       inLeverIdx  レバーインデックス
 * @return      BOOL TRUE 
 */
//--------------------------------------------------------------
BOOL test_GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );

  {
    //イベントセット
    GMEVENT * event = GMEVENT_Create( gsys, NULL, ChangePointEvt, 0/*sizeof(ENCEFF_WORK)*/ );
    GAMESYSTEM_SetEvent(gsys, event);
    //操作予定レバーインデックスをセット
    tmp->LeverIdx = inLeverIdx;
    //GMEVENT_CallEvent(GMEVENT * parent, GMEVENT * child);
  }
  return TRUE;
  
}

//--------------------------------------------------------------
/**
 * プラットホーム間を移動している間のイベント
 * @param   gsys	ゲームシステムポインタ
 * @return  BOOL    イベントセットした場合TRUE
 */
//--------------------------------------------------------------
BOOL test_GYM_ELEC_CallMoveEvt(GAMESYS_WORK *gsys)
{
  GYM_ELEC_TMP *tmp_work;
  u8 platform_idx;
  int x,z;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  tmp_work = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  platform_idx = 8;

  //カプセルがプラットフォームに停止しているか？
  if ( IsStopCapsuleToPlatForm(tmp_work, platform_idx) ){ //停止している
    //次のプラットフォームまでの移動イベントを作成する
    GMEVENT * event = GMEVENT_Create( gsys, NULL, CapMoveEvt, 0 );
    GAMESYSTEM_SetEvent(gsys, event);

    //自機が走行するレールインデックスを保存
    tmp_work->RadeRaleIdx = GetRaleIdxByPlatformIdx(tmp_work, platform_idx);
    //カプセル移動アニメを停止
    {
      FLD_EXP_OBJ_CNT_PTR obj_cnt_ptr;
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 cap_idx = GetCapIdxByPlatformIdx(platform_idx);
      u8 obj_idx = OBJ_CAP_1+cap_idx;
      u8 anm_idx = ANM_CAP_MOV1 + tmp_work->RadeRaleIdx;
      obj_cnt_ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
      anm = FLD_EXP_OBJ_GetAnmCnt( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }

    return TRUE;
  }
  return FALSE;
}

#endif  //PM_DEBUG


/*---------------------------------------------------------------------------*
    getJntSRTAnmResult_

    pResultにrotation, translation, flagをセット
    pS_invSにスケールとスケールの逆数をセット
 *---------------------------------------------------------------------------*/
static void
getJntSRTAnmResult_(const NNSG3dResJntAnm* pJntAnm, 
                    u32                    dataIdx, 
                    fx32                   Frame,
                    VecFx32 *outVec)
{
  NNSG3dResJntAnmSRTTag* pAnmSRTTag;
  NNSG3dJntAnmSRTTag     tag;
  const u32*             pData;
  fx32                   pS_invS[6]; // ポインタ引数からローカル配列に変更

  {
    const u16* ofsTag = (const u16*)((u8*) pJntAnm + sizeof(NNSG3dResJntAnm));
    // アニメリソース内のdataIdx番目のデータのSRTタグを取得
    pAnmSRTTag = (NNSG3dResJntAnmSRTTag*)((u8*) pJntAnm + ofsTag[dataIdx]);

    tag = (NNSG3dJntAnmSRTTag)pAnmSRTTag->tag;
  }

  // pAnmSRTTagに続く領域にpAnmSRTTagの値に従った可変長のデータが格納されている
  pData = (const u32*)((const u8*)pAnmSRTTag + sizeof(NNSG3dResJntAnmSRTTag));

  // res_struct.hのNNSG3dResJntAnmSRTについてのコメントを参照のこと
    
  // Translationの取得
  if (!(tag & (NNS_G3D_JNTANM_SRTINFO_IDENTITY_T | NNS_G3D_JNTANM_SRTINFO_BASE_T)))
  {
    // TX
    if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_TX))
    {
      // TXを取り出す関数呼び出し
      // データのオフセットはpAnmSRTTagからのものになる
      fx32 x;
      getTransData_(&x, Frame, pData, pJntAnm);
//      OS_Printf("X= %d::%x\n",x,x);
      outVec->x = x;

      // NNSG3dJntAnmTInfoと配列へのオフセットデータの分
      pData += 2;
    }
    // TY
    if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_TY))
    {
      // TYを取り出す関数呼び出し
      // データのオフセットはpAnmSRTTagからのものになる
      fx32 y;
      getTransData_(&y, Frame, pData, pJntAnm);
//      OS_Printf("Y= %d::%x\n",y,y);
      outVec->y = y;
          
      // NNSG3dJntAnmTInfoと配列へのオフセットデータの分
      pData += 2;
    }
    // TZ
    if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_TZ))
    {
      // TZを取り出す関数呼び出し
      // データのオフセットはpAnmSRTTagからのものになる
      fx32 z;
      getTransData_(&z, Frame, pData, pJntAnm);
//      OS_Printf("Z= %d::%x\n",z,z);
      outVec->z = z;

      // NNSG3dJntAnmTInfoと配列へのオフセットデータの分
      pData += 2;
    }
  }
}

/*---------------------------------------------------------------------------*
    getTransData_

    translationデータを取得し、*pValに入れる
 *---------------------------------------------------------------------------*/
static void
getTransData_(fx32* pVal,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm)
{
    u32 frame = (u32)FX_Whole(Frame);
    const void* pArray = (const void*)((const u8*)pJntAnm + *(pData + 1));
    NNSG3dJntAnmTInfo info = (NNSG3dJntAnmTInfo)*pData;
    u32 last_interp;
    u32 idx;
    u32 idx_sub;

    NNS_G3D_NULL_ASSERT(pVal);
    NNS_G3D_NULL_ASSERT(pArray);

    if (!(info & NNS_G3D_JNTANM_TINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_TINFO_STEP1が選択されている
        idx = frame;
        goto TRANS_NONINTERP;
    }

    // last_interp以降は1コマずつデータが入っている
    // last_interpは2の倍数か4の倍数である。
    last_interp = ((u32)info & NNS_G3D_JNTANM_TINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_TINFO_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_JNTANM_TINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_TINFO_STEP_2が選択されている
        if (frame & 1)
        {
            if (frame > last_interp)
            {
                // 最終フレーム以外ありえない
                idx = (last_interp >> 1) + 1;
                goto TRANS_NONINTERP;
            }
            else
            {
                // 奇数で最終フレームでないので50:50の補間処理がいる。
                idx = frame >> 1;
                goto TRANS_INTERP_2;
            }
        }
        else
        {
            // 偶数フレームなので補間処理はいらない
            idx = frame >> 1;
            goto TRANS_NONINTERP;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_TINFO_STEP_4が選択されている
        if (frame & 3)
        {
            if (frame > last_interp)
            {
                idx = (last_interp >> 2) + (frame & 3);
                goto TRANS_NONINTERP;
            }

            // 補間処理あり
            if (frame & 1)
            {
                fx32 v, v_sub;
                if (frame & 2)
                {
                    // 3:1の位置で補間
                    idx_sub = (frame >> 2);
                    idx = idx_sub + 1;
                }
                else
                {
                    // 1:3の位置で補間
                    idx = (frame >> 2);
                    idx_sub = idx + 1;
                }
                
                // 1:3, 3:1の場合の補間
                if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
                {
                    const fx16* p_fx16 = (const fx16*)pArray;

                    v = *(p_fx16 + idx);
                    v_sub = *(p_fx16 + idx_sub);
                    *pVal = (v + v + v + v_sub) >> 2; // v:v_subを3:1でブレンド
                }
                else
                {
                    const fx32* p_fx32 = (const fx32*)pArray;

                    v = *(p_fx32 + idx);
                    v_sub = *(p_fx32 + idx_sub);
                    // v:v_subを3:1でブレンド。オーバーフローを避けるためfx64で計算
                    *pVal = (fx32)(((fx64)v + v + v + v_sub) >> 2);
                }
                return;
            }
            else
            {
                // 50:50の補間になる
                idx = frame >> 2;
                goto TRANS_INTERP_2;
            }
        }
        else
        {
            // フレームは丁度4の倍数になっている。
            idx = frame >> 2;
            goto TRANS_NONINTERP;
        }
    }
    NNS_G3D_ASSERT(0);
TRANS_INTERP_2:
    if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
    {
        const fx16* p_fx16 = (const fx16*)pArray;

        *pVal = (*(p_fx16 + idx) + *(p_fx16 + idx + 1)) >> 1;
    }
    else
    {
        const fx32* p_fx32 = (const fx32*)pArray;

        fx32 v1 = *(p_fx32 + idx) >> 1;
        fx32 v2 = *(p_fx32 + idx + 1) >> 1;
        *pVal = v1 + v2;
    }
    return;
TRANS_NONINTERP:
    if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
    {
        *pVal = *((const fx16*)pArray + idx);
    }
    else
    {
        *pVal = *((const fx32*)pArray + idx);
    }
    return;
}

//--------------------------------------------------------------
/**
 * ＳＥ音量変更する矩形内チェック
 * @param	    fieldWork       フィールドワークポインタ
 * @return    u8    矩形インデックス  SE_RECT_MAXで未発見
 */
//--------------------------------------------------------------
static u8 CheckGetSeRect(FIELDMAP_WORK *fieldWork)
{
  u8 i;
  u16 x,z;
  //自機の座標取得
  const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
	
  x = MMDL_GetGridPosX( fmmdl );
  z = MMDL_GetGridPosZ( fmmdl );

  for (i=0;i<SE_RECT_MAX;i++)
  {
    //矩形チェック
    if ( (SeRect[i].X<=x)&&(x<=SeRect[i].X+SeRect[i].SizeW-1)&&
         (SeRect[i].Z<=z)&&(z<=SeRect[i].Z+SeRect[i].SizeH-1) ){
      break;
    }
  }
  return i;
}

static void StateSeFunc(FIELDMAP_WORK *fieldWork, GYM_ELEC_TMP *tmp)
{
  int volume;
  fx32 min_len;
  int min_obj_idx, min_anm_idx;
  BOOL first;
  int i;
  FIELD_SOUND* fs;
  FLD_EXP_OBJ_CNT_PTR obj_cnt_ptr;
  obj_cnt_ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    fs = GAMEDATA_GetFieldSound( gamedata );
  }

  first = TRUE;
  for(i=0;i<CAPSULE_NUM_MAX;i++)
  {
    //アニメデータ取得
    fx32 now_frm;
    VecFx32 now, player_pos, dst;
    fx32 len;
    NNSG3dAnmObj* anm_obj_ptr;
    GFL_G3D_ANM*  gfl_anm;
    u8 cap_idx = i;
    u8 obj_idx = OBJ_CAP_1 + cap_idx;
    GFL_G3D_OBJ* g3Dobj = FLD_EXP_OBJ_GetUnitObj(obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx);
    u8 anm_idx = ANM_CAP_MOV1 + tmp->NowRaleIdx[cap_idx];
    EXP_OBJ_ANM_CNT_PTR anm_ptr = FLD_EXP_OBJ_GetAnmCnt(obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);

    now_frm = GetAnimeFrame(obj_cnt_ptr, obj_idx, anm_idx);

    gfl_anm = GFL_G3D_OBJECT_GetG3Danm( g3Dobj, anm_idx );
    anm_obj_ptr = GFL_G3D_ANIME_GetAnmObj( gfl_anm );
    getJntSRTAnmResult_(anm_obj_ptr->resAnm,
                        0,
                        now_frm,
                        &now);
    VEC_Add( &now, &CapPos[cap_idx] ,&now );
    FIELD_PLAYER_GetPos( FIELDMAP_GetFieldPlayer( fieldWork ), &player_pos );
    VEC_Subtract( &now, &player_pos ,&dst );
    len = VEC_Mag( &dst );
    if ( (min_len >len)|| first )
    {
      min_len = len;
      min_obj_idx = obj_idx;
      min_anm_idx = ANM_CAP_MOV1 + tmp->NowRaleIdx[cap_idx];
      first = FALSE;
    }
  }

  if ( SE_LEN <= min_len ) volume = 0;
  else
  {
    fx32 now_frm,next_frm;
    VecFx32 now,next, dst;
    fx32 len;
    NNSG3dAnmObj* anm_obj_ptr;
    GFL_G3D_ANM*  gfl_anm;
    GFL_G3D_OBJ* g3Dobj = FLD_EXP_OBJ_GetUnitObj(obj_cnt_ptr, GYM_ELEC_UNIT_IDX, min_obj_idx);
    EXP_OBJ_ANM_CNT_PTR anm_ptr = FLD_EXP_OBJ_GetAnmCnt(obj_cnt_ptr, GYM_ELEC_UNIT_IDX, min_obj_idx, min_anm_idx);
    fx32 last_frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm_ptr );

    now_frm = GetAnimeFrame(obj_cnt_ptr, min_obj_idx, min_anm_idx);
    next_frm = now_frm+FX32_ONE;
    if ( next_frm >= last_frm) next_frm = 0;

    gfl_anm = GFL_G3D_OBJECT_GetG3Danm( g3Dobj, min_anm_idx );
    anm_obj_ptr = GFL_G3D_ANIME_GetAnmObj( gfl_anm );
    getJntSRTAnmResult_(anm_obj_ptr->resAnm,
                        0,
                        now_frm,
                        &now);
    getJntSRTAnmResult_(anm_obj_ptr->resAnm,
                        0,
                        next_frm,
                        &next);

    VEC_Subtract( &now, &next ,&dst );
    len = VEC_Mag( &dst );
    if ( len >=SPD_LV5 ){
      volume = 127;
    }else if ( len >=SPD_LV4 ){
      volume = 120;
    }else if ( len >=SPD_LV3 ){
      volume = 90;
    }else if( len >= SPD_LV2 ){
      volume = 60;
    }else{
      volume = 0;
    }
  }
  
  FSND_SetEnvSEVol( fs, GYM_ELEC_SE_DRIVE, volume );
}
