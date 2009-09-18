//======================================================================
/**
 * @file  gym_elec.c
 * @bfief  電気ジム
 * @author  Saito
 * @date  09.09.02
 */
//======================================================================

#include "fieldmap.h"
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

#define GYM_ELEC_UNIT_IDX (0)
#define GYM_ELEC_TMP_ASSIGN_ID  (1)

#define STOP_NUM_MAX  (2)
#define RALE_NUM_MAX  (8)
#define LEVER_NUM_MAX (4)
#define CAPSULE_NUM_MAX (4)
#define PLATFORM_NUM_MAX (9)

#define PLATFORM_NONE (0xff)
#define PLATFORM_NO_STOP (-1)

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

typedef struct CAPSULE_DAT_tag
{
  int RaleIdx;      //現在走行中のレールインデックス
  fx32 AnmFrame;    //レールの現在アニメフレーム
}CAPSULE_DAT;

typedef struct ELEC_GYM_TASK_WK_tag
{
  u32 Timer;
}ELEC_GYM_TASK_WK;

//ジム内部中の一時ワーク
typedef struct GYM_ELEC_TMP_tag
{
  GFL_TCB * MoveTcbPtr;     //カプセル追従ＴＣＢ
  GFL_TCB * CapStopTcbPtr;   //カプセル停止制御ＴＣＢ
  u8 RadeRaleIdx;          //自機が走行しているレールインデックス
  u8 LeverIdx;              //操作しようとしているレバーインデックス
  u8 ChgingSw;
  u8 RideEvt;            //乗降イベント中カプセル番号
  ELEC_GYM_TASK_WK  TaskWk[CAPSULE_NUM_MAX];
  ICA_ANIME* IcaAnmPtr;
  u8 FramePosDat[FRAME_POS_SIZE+HEADER_SIZE];
  BOOL AltoMove;

  GFL_TCB *PlayerBack;
  GFL_TCB *TrGo;
}GYM_ELEC_TMP;

//電気ジムセーブワーク
typedef struct GYM_ELEC_SV_WORK_tag
{
  CAPSULE_DAT CapDat[CAPSULE_NUM_MAX];
  u8 LeverSw[LEVER_NUM_MAX];    //0or1
  u8 RaleChgReq[CAPSULE_NUM_MAX];
  u8 NowRaleIdx[CAPSULE_NUM_MAX];     //現在走行しているレールのインデックス(スイッチ非依存)
  u8 RideFlg[CAPSULE_NUM_MAX];      //カプセルに乗ったことがあるか？
  u8 StopPlatformIdx[CAPSULE_NUM_MAX];
}GYM_ELEC_SV_WORK;

typedef struct LEVER_SW_tag
{
  u16 OffOnRaleIdx[2];    //添え字0がレバーオフ　1がレバーオン
}LEVER_SW;

typedef struct STOP_DAT_tag
{
  int PlatformIdx;      //停車プラットフォーム
  int StopStart;        //停車開始フレーム
  int StopEnd;          //停車終了フレーム
}STOP_DAT;

typedef struct RALE_DAT_tag
{
  STOP_DAT StopDat[STOP_NUM_MAX];

}RALE_DAT;

//トレーナーエンカウント前アニメ
static const MMDL_ACMD_LIST AcmdList[] = {
  {AC_WALK_D_8F, 2},
  {ACMD_END, 0},
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

//1つのレバーで切り替えるレール間で共通で停車するプラットホーム
static const u8 CommPlatFormIdx[CAPSULE_NUM_MAX] = {
  1,4,6,8
};

//レールごとの停車アニメフレーム（1レール2つずつ）
//1箇所しか止まらないレールは2データとも同じ値を入れる。ICAデータ依存
static const fx32 StopFrame[RALE_NUM_MAX][RALE_PLATFORM_NUM_MAX] = {
  {0, FX32_ONE*15},     //レール1
  {0, FX32_ONE*33},   //レール2
  {0, FX32_ONE*0},     //レール3
  {0, FX32_ONE*125},     //レール4
  {0, FX32_ONE*0},     //レール5
  {0, FX32_ONE*57},     //レール6
  {0, FX32_ONE*0},     //レール7
  {0, FX32_ONE*113},     //レール8
};

//カプセルごとのレール移動アニメ共通部分フレーム（アニメデータ依存）
static const u16 CommFrame[CAPSULE_NUM_MAX] = {
  4,120,40,70
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
static u8 GetRaleIdxByCapIdx(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx);
static u8 GetRaleIdxByPlatformIdx(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inPlatformIdx);
static u8 GetCapIdxByPlatformIdx(const u8 inPlatformIdx);
static BOOL IsStopCapsuleToPlatForm(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inPlatformIdx);
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

BOOL test_GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx);
BOOL test_GYM_ELEC_CallMoveEvt(GAMESYS_WORK *gsys);

//--------------------------------------------------------------
/**
 * 現在座標からプラットホームインデックスを取得する
 * @param	
 * @return
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
 * @param	
 * @return
 */
//--------------------------------------------------------------
static u8 GetRaleIdxByCapIdx(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx)
{
  //レールインデックス取得
  return gmk_sv_work->NowRaleIdx[inCapIdx];
}


//--------------------------------------------------------------
/**
 * レールアニメーション切り替えＴＣＢ
 * @param	
 * @return
 */
//--------------------------------------------------------------
static BOOL RaleChgTcb()
{
  u8 i;
  //全部のカプセルのレールアニメ切り替えリクエストを調べる
  for(i=0;i<LEVER_NUM_MAX;i++){
    if (0){//リクエストがかかっているか？
      if (0){ //リクエストに答えられるプラットホームに停車中か？
        //アニメ変更
        ;
        //リクエストオフ
        ;
      }
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ELEC_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GYM_ELEC_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ELEC_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ELEC_TMP));
  //ギミックワークにアクセス
  ;  
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




  //セーブされているデータで、アニメーションの頭出し
#if 0
  {
    //レバー1回再生設定
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, 4, 0);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, 4, 1);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, 4, 1, FALSE);
  }
#endif  
  //カプセル・スイッチ初期アニメ決定
  {
    u8 i;
    for (i=0;i<CAPSULE_NUM_MAX;i++){
      u8 obj_idx;
      u8 anm_idx;

      obj_idx = OBJ_CAP_1+i;
      anm_idx = ANM_CAP_MOV1+(i*2);
      //レール移動の決定(偶数インデックスを有効)
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx, TRUE);
      //蓋アニメをセットして止めておく
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL, TRUE);
      {
        EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //1回再生設定
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
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
      }
    }
  }
  
  //レールアニメリ変更クエスト消化ＴＣＢ作成
  ;
  //リクエストチェック後ＴＣＢ実行
  ;
  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  //カプセル停止制御ＴＣＢ追加
  {
    GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldWork );
    // TCBを追加
    tmp->CapStopTcbPtr = GFL_TCB_AddTask( tcbsys, CapStopTcbFunc, fieldWork, 0 );
  }

  //イベント未起動状態セット
  tmp->RideEvt = -1;


  //イニット関数に入れる処理　今だけここにおく
  {
    gmk_sv_work->NowRaleIdx[0] = 0;
    gmk_sv_work->NowRaleIdx[1] = 2;
    gmk_sv_work->NowRaleIdx[2] = 4;
    gmk_sv_work->NowRaleIdx[3] = 6;

    gmk_sv_work->StopPlatformIdx[0] = PLATFORM_NO_STOP;
    gmk_sv_work->StopPlatformIdx[1] = PLATFORM_NO_STOP;
    gmk_sv_work->StopPlatformIdx[2] = PLATFORM_NO_STOP;
    gmk_sv_work->StopPlatformIdx[3] = PLATFORM_NO_STOP;
  }
}

static void CapStopTcbFunc(GFL_TCB* tcb, void* work)
{
  u8 i;
  FIELDMAP_WORK *fieldWork;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR obj_cnt_ptr;
  GYM_ELEC_TMP *tmp;
  
  fieldWork = (FIELDMAP_WORK *)(work);
  obj_cnt_ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

  //----リクエスト消化部----
  for (i=0;i<CAPSULE_NUM_MAX;i++){
    //リクエストチェック
    if ( gmk_sv_work->RaleChgReq[i] ){
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
        gmk_sv_work->NowRaleIdx[i] = (i*2)+gmk_sv_work->LeverSw[i];
        //リクエストフラグを落とす
        gmk_sv_work->RaleChgReq[i] = 0;
      }
    }
  }

  //----カプセル停止制御部----
  for(i=0;i<CAPSULE_NUM_MAX;i++){
    u8 obj_idx = OBJ_CAP_1+i;
    u8 rale_idx = gmk_sv_work->NowRaleIdx[i];
    u8 anm_idx = ANM_CAP_MOV1 + rale_idx;
    EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx);

///    if (i!=3){continue;}//テスト
    
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
        gmk_sv_work->StopPlatformIdx[i] = PLATFORM_NO_STOP;
      }
    }else{
      u8 stop_idx;
      fx32 frame = FLD_EXP_OBJ_GetObjAnmFrm( obj_cnt_ptr, GYM_ELEC_UNIT_IDX, obj_idx, anm_idx );
      //停止すべきフレームかを調べる
      if ( CheckCapStopFrame(rale_idx, frame, &stop_idx) ){
        //アニメをストップ
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //指定時間とめるためタイマーをセット
        tmp->TaskWk[i].Timer = STOP_TIME;
        gmk_sv_work->StopPlatformIdx[i] = RaleStopPlatform[rale_idx][stop_idx];
      }
    }
    OS_Printf("%d::stop_plat %d\n",i, gmk_sv_work->StopPlatformIdx[i]);
  } //end for

  //--自機自動移動部--
  if (tmp->AltoMove){
    fx32 frame;
    u8 cap_idx;
    u8 anm_idx;
    int frm_idx;
    VecFx32 dst_vec = {0,0,0};
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
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ELEC_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

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
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ELEC_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

  //テスト
  {
    GAMESYS_WORK *gsys  = FIELDMAP_GetGameSysWork( fieldWork );
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
      test_GYM_ELEC_ChangePoint(gsys, 3);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
      test_GYM_ELEC_CallMoveEvt(gsys);
    }
    if (GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT ){
      FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
      GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
      tmp->LeverIdx = 2;
      ChgRale(fieldWork, ptr);
    }
  }

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

  //アニメフレームをセーブデータに保存
  for (i=0;i<CAPSULE_NUM_MAX;i++){
    u8 obj_idx;
    u8 anm_idx;
    obj_idx = OBJ_CAP_1 + i;
    anm_idx = ANM_CAP_MOV1 + gmk_sv_work->CapDat[i].RaleIdx;
    gmk_sv_work->CapDat[i].AnmFrame = GetAnimeFrame(ptr, obj_idx, anm_idx);
  }
}

//--------------------------------------------------------------
/**
 * 指定プラットホームの現在のレールインデックスを取得
 * @param	
 * @return
 */
//--------------------------------------------------------------
static u8 GetRaleIdxByPlatformIdx(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inPlatformIdx)
{
  u8 cap_idx;
  u8 rale_idx;
  //指定プラットフォームに停車するカプセルを取得
  cap_idx = GetCapIdxByPlatformIdx(inPlatformIdx);
  //取得したカプセルが走行しているレールインデックスを取得
  rale_idx = GetRaleIdxByCapIdx(gmk_sv_work, cap_idx);

  return rale_idx;
}

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
 * @param	
 * @return
 */
//--------------------------------------------------------------
static BOOL IsStopCapsuleToPlatForm(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inPlatformIdx)
{
  u8 i;
  u8 cap_idx;
  int platform_idx;

  //指定プラットホームのカプセルインデックスを取得
  cap_idx = GetCapIdxByPlatformIdx(inPlatformIdx);
  //取得カプセルインデックスの停車プラットホームを取得
  platform_idx = gmk_sv_work->StopPlatformIdx[cap_idx];
  OS_Printf("%d %d %d\n",platform_idx, cap_idx, gmk_sv_work->StopPlatformIdx[cap_idx]);
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
 * @param	
 * @return
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
 * 移動カプセルに自機がついてくるように制御するＴＣＢ
 * @param	
 * @return
 */
//--------------------------------------------------------------
static BOOL CapsuleTraceTcb()
{
  return FALSE;
}

//--------------------------------------------------------------
/**
 * レバーの切り替えを行うイベント起動
 * @param	
 * @return
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
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, ChangePointEvt, 0 );
    
  //操作予定レバーインデックスをセット
  tmp->LeverIdx = inLeverIdx;

  return event;
}

//--------------------------------------------------------------
/**
 * レバーの切り替えを行うイベント起動
 * @param	
 * @return
 */
//--------------------------------------------------------------
BOOL test_GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

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
 * レバーの切り替えイベント
 * @param	
 * @return
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
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
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
    }    
    (*seq)++;
    break;
  case 1: //アニメ待ち
#if 1    
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
        return GMEVENT_RES_FINISH;
      }
    }
#else
    {
      //路線ポイント切り替え（アニメ切り替え）
      ChgRale(fieldWork, ptr);
      return GMEVENT_RES_FINISH;
    }
#endif
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * プラットホーム間を移動している間のイベント
 * @param	
 * @return
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
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
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
  if ( IsStopCapsuleToPlatForm(gmk_sv_work, platform_idx) ){ //停止している
    u8 cap_idx = GetCapIdxByPlatformIdx(platform_idx);
    //カプセル内トレーナーエンカウントチェック
    if ( CheckCapTrEnc(gmk_sv_work, cap_idx) ){
      //カプセル内トレーナーとのエンカウントイベント作成
      event = GMEVENT_Create( gsys, NULL, TrEncEvt, 0 );
    }else{
      //次のプラットフォームまでの移動イベントを作成する
      event = GMEVENT_Create( gsys, NULL, CapMoveEvt, 0 );
    }

    {
      //自機が走行するレールインデックスを保存
      tmp_work->RadeRaleIdx = GetRaleIdxByPlatformIdx(gmk_sv_work, platform_idx);
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

    //乗ったフラグオン
    gmk_sv_work->RideFlg[cap_idx] = 1;

    return event;
  }
  return NULL;
}

static BOOL CheckCapTrEnc(GYM_ELEC_SV_WORK *gmk_sv_work, const u8 inCapIdx)
{
  u8 check = 0;
  if (inCapIdx == 1){
    check = 1;
  }else if(inCapIdx == 2){
    check =1;
  }

  if (check){
    if ( !gmk_sv_work->RideFlg[inCapIdx] ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * プラットホーム間を移動している間のイベント
 * @param	
 * @return
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
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  tmp_work = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  platform_idx = 8;

  //カプセルがプラットフォームに停止しているか？
  if ( IsStopCapsuleToPlatForm(gmk_sv_work, platform_idx) ){ //停止している
    //次のプラットフォームまでの移動イベントを作成する
    GMEVENT * event = GMEVENT_Create( gsys, NULL, CapMoveEvt, 0 );
    GAMESYSTEM_SetEvent(gsys, event);

    //自機が走行するレールインデックスを保存
    tmp_work->RadeRaleIdx = GetRaleIdxByPlatformIdx(gmk_sv_work, platform_idx);
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

static GMEVENT_RESULT CapMoveEvt(GMEVENT* event, int* seq, void* work)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  //レールインデックス/2でカプセルインデックスになる
  u8 cap_idx = tmp->RadeRaleIdx / 2;
  u8 obj_idx = OBJ_CAP_1+cap_idx;

  EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
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
          GFL_HEAP_LOWID(HEAPID_FIELDMAP), ARCID_GYM_ELEC, arc_idx, tmp->FramePosDat, FRAME_POS_SIZE
          );

      GF_ASSERT(tmp->IcaAnmPtr != NULL);

      //カプセル開くアニメスタート
      {
        //フレーム頭だし
        SetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL, 0);
        //再生
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
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
        //主人公カプセルに入るアニメ(上に移動)
        {
          MMDL * mmdl;
          mmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
          MMDL_SetAcmd( mmdl, AC_WALK_U_8F );
        }
        (*seq)++;
      }
    }
    break;
  case 2:
    {
      MMDL * mmdl;
      mmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
      //主人公アニメ終了待ち
      if( MMDL_CheckEndAcmd(mmdl) ){
        //主人公消す
        MMDL_SetStatusBitVanish(mmdl, TRUE);

        //カプセル閉まるアニメスタート
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        (*seq)++;
      }
    }
    break;
  case 3:
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
      gmk_sv_work->StopPlatformIdx[cap_idx] = PLATFORM_NO_STOP;

      //乗降終了
      tmp->RideEvt = -1;

      //自機自動移動開始
      tmp->AltoMove = TRUE;

      (*seq)++;
    }
    break;
  case 4:
    //プラットホーム到着待ち
    if( gmk_sv_work->StopPlatformIdx[cap_idx] != PLATFORM_NO_STOP ){
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
            platform_idx = gmk_sv_work->StopPlatformIdx[cap_idx];
            pos.x = PlatformXZ[platform_idx][0] * FIELD_CONST_GRID_FX32_SIZE;
            pos.x += GRID_HALF_SIZE;
            pos.z = (PlatformXZ[platform_idx][1]-1) * FIELD_CONST_GRID_FX32_SIZE;
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
    {
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      //主人公表示
      MMDL_SetStatusBitVanish(mmdl, FALSE);
      //主人公カプセルからでるアニメ
      MMDL_SetAcmd( mmdl, AC_WALK_D_8F );
    }
    (*seq)++;
    break;
  case 8:
    {
      MMDL * mmdl;
      mmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
      //主人公アニメ終了待ち
      if( MMDL_CheckEndAcmd(mmdl) ){
        //カプセル閉まるアニメスタート
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        (*seq)++;
      }
    }
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
      gmk_sv_work->StopPlatformIdx[cap_idx] = PLATFORM_NO_STOP;

      ICA_ANIME_Delete( tmp->IcaAnmPtr );
      
      //乗降終了
      tmp->RideEvt = -1;
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//トレーナー戦イベント
static GMEVENT_RESULT TrEncEvt(GMEVENT* event, int* seq, void* work)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);

  //レールインデックス/2でカプセルインデックスになる
  u8 cap_idx = tmp->RadeRaleIdx / 2;
  u8 obj_idx = OBJ_CAP_1+cap_idx;

  EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ELEC_UNIT_IDX, obj_idx, ANM_CAP_OPCL);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

  switch(*seq){
  case 0:
    //カプセル開くアニメスタート
      {
        //フレーム頭だし
        SetAnimeFrame(ptr, obj_idx, ANM_CAP_OPCL, 0);
        //再生
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
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
        //アニメスクリプトコール
        OS_Printf("スクリプトコール\n");
        SCRIPT_CallScript( event, SCRID_GIMMICK_C04GYM0101_SCR01,
          NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
        (*seq)++;
      }      
    }
    break;
  case 2:
    //カプセル閉まるアニメスタート
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    (*seq)++;
    break;
  case 3:
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
      gmk_sv_work->StopPlatformIdx[cap_idx] = PLATFORM_NO_STOP;

      //乗降終了
      tmp->RideEvt = -1;

      //スクリプトコール
      OS_Printf("スクリプトコール\n");
      SCRIPT_CallScript( event, SCRID_GIMMICK_C04GYM0101_SCR02,
          NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
      (*seq)++;
    }
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

static void ChgRale(FIELDMAP_WORK *fieldWork, FLD_EXP_OBJ_CNT_PTR ptr)
{
  u8 cap_idx;
  u8 obj;
  u8 sw;
  GYM_ELEC_SV_WORK *gmk_sv_work;
  GYM_ELEC_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ELEC_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  //操作しようとしているレバーのスイッチがどちらに入っているかを調べる  
  sw = gmk_sv_work->LeverSw[tmp->LeverIdx];
  obj = OBJ_CAP_1+tmp->LeverIdx;
  cap_idx = tmp->LeverIdx;    //スイッチインデックスがカプセルインデックスに対応

  //セーブデータのスイッチ切り替え
  gmk_sv_work->LeverSw[tmp->LeverIdx] = (gmk_sv_work->LeverSw[tmp->LeverIdx]+1)%2;
  //すぐにレールアニメを切り替えられるならば変える
  if ( CheckChangableRaleAtOnce(ptr, cap_idx, gmk_sv_work->NowRaleIdx[tmp->LeverIdx]) ){
    ChgRaleAnm(ptr, sw, tmp->LeverIdx);
    //走行レール保存
    gmk_sv_work->NowRaleIdx[tmp->LeverIdx] =
      (tmp->LeverIdx*2)+gmk_sv_work->LeverSw[tmp->LeverIdx];
  }else{
    //変えられない場合はリクエストを出す
    if (gmk_sv_work->RaleChgReq[tmp->LeverIdx]){
      //既にリクエストされているならば、リクエストをキャンセル
      gmk_sv_work->RaleChgReq[tmp->LeverIdx] = 0;
    }else{
      //リクエストされていないならば、リクエストする
      gmk_sv_work->RaleChgReq[tmp->LeverIdx] = 1;
    }
  }
}

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

//即レールアニメ切り替えできるか？
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
