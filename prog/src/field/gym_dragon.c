//======================================================================
/**
 * @file  gym_dragon.c
 * @brief  ドラゴンジム
 * @author  Saito
 * @date  09.12.04
 */
//======================================================================
#include "fieldmap.h"
#include "gym_dragon_sv.h"
#include "gym_dragon.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_dragon.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
//#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_local_def.h"

#include "sound/pm_sndsys.h"
#include "gym_dragon_se_def.h"

//#include "field/fieldmap_proc.h"    //for FLDMAP_CTRLTYPE
//#include "fieldmap_ctrl_hybrid.h" //for FIELDMAP_CTRL_HYBRID
#include "../../../resource/fldmapdata/eventdata/define/total_header.h" //for OBJID
#include "rail_line/c08gym0101.h" //for RAIL

#define GYM_DRAGON_UNIT_IDX (0)
#define GYM_DRAGON_TMP_ASSIGN_ID  (1)

#define DRAGON_PARTS_SET  (5)   //頭・左腕・右腕・左ボタン・右ボタンの4つ
#define ANM_PLAY_MAX  (1)
#define DRAGON_ANM_NUM  (8)
#define ARM_ANM_NUM  (2)
#define BTN_ANM_NUM  (1)

#define JUMP_COUNT  (8)
#define JUMP_MOVE_X  (FX32_ONE*2)

#define CHECK_RAIL_NUM  (6)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define FLOOR_VISIBLE_HEIGHT    (4*FIELD_CONST_GRID_FX32_SIZE)
#define FLOOR_VANISH_X          (16*FIELD_CONST_GRID_FX32_SIZE)
#define FLOOR_VANISH_Z          (30*FIELD_CONST_GRID_FX32_SIZE)


#define DRAGON1_X (12*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define DRAGON1_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define DRAGON1_Z (29*FIELD_CONST_GRID_FX32_SIZE)
#define DRAGON_HEAD1_Z (31*FIELD_CONST_GRID_FX32_SIZE)

#define DRAGON2_X (42*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define DRAGON2_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define DRAGON2_Z (23*FIELD_CONST_GRID_FX32_SIZE)
#define DRAGON_HEAD2_Z (21*FIELD_CONST_GRID_FX32_SIZE)

#define DRAGON3_X (27*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define DRAGON3_Y (6*FIELD_CONST_GRID_FX32_SIZE)
#define DRAGON3_Z (28*FIELD_CONST_GRID_FX32_SIZE)
#define DRAGON_HEAD3_Z (26*FIELD_CONST_GRID_FX32_SIZE)

#define FLOOR_X (32*FIELD_CONST_GRID_FX32_SIZE)
#define FLOOR_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define FLOOR_Z (32*FIELD_CONST_GRID_FX32_SIZE)

#define BUTTON1_X (17*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define BUTTON1_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define BUTTON1_Z (29*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define BUTTON2_X (7*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define BUTTON2_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define BUTTON2_Z (29*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define BUTTON3_X (37*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define BUTTON3_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define BUTTON3_Z (22*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define BUTTON4_X (47*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define BUTTON4_Y (0*FIELD_CONST_GRID_FX32_SIZE)
#define BUTTON4_Z (22*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define BUTTON5_X (22*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define BUTTON5_Y (6*FIELD_CONST_GRID_FX32_SIZE)
#define BUTTON5_Z (27*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define BUTTON6_X (32*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define BUTTON6_Y (6*FIELD_CONST_GRID_FX32_SIZE)
#define BUTTON6_Z (27*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

static const VecFx32 DragonPos[DRAGON_NUM_MAX] = {
  { DRAGON1_X, DRAGON1_Y, DRAGON_HEAD1_Z },
  { DRAGON2_X, DRAGON2_Y, DRAGON_HEAD2_Z },
  { DRAGON3_X, DRAGON3_Y, DRAGON_HEAD3_Z },
};

static const VecFx32 ArmPos[DRAGON_NUM_MAX] = {
  { DRAGON1_X, DRAGON1_Y, DRAGON1_Z },
  { DRAGON2_X, DRAGON2_Y, DRAGON2_Z },
  { DRAGON3_X, DRAGON3_Y, DRAGON3_Z },
};

static const VecFx32 ButtonPos[DRAGON_NUM_MAX*2] = {
  { BUTTON1_X, BUTTON1_Y, BUTTON1_Z },
  { BUTTON2_X, BUTTON2_Y, BUTTON2_Z },
  { BUTTON3_X, BUTTON3_Y, BUTTON3_Z },
  { BUTTON4_X, BUTTON4_Y, BUTTON4_Z },
  { BUTTON5_X, BUTTON5_Y, BUTTON5_Z },
  { BUTTON6_X, BUTTON6_Y, BUTTON6_Z },
};


static const u16 DragonRad[DRAGON_NUM_MAX] = {
  0x8000,
  0,
  0
};

typedef struct ANM_PLAY_WORK_tag
{
  u8 ObjIdx;
  u8 AnmNum;
  u8 AllAnmNum;
  u8 JumpCount;
  fx32 JumpDownX;
  fx32 JumpDownBaseY;
  int AnmOfs[ANM_PLAY_MAX];
  int SeNo[2];
}ANM_PLAY_WORK;

//ジム内部中の一時ワーク
typedef struct GYM_DRAGON_TMP_tag
{
  int TrgtHead;
  DRA_ARM TrgtArm;
  DRAGON_WORK *DraWk;
  ANM_PLAY_WORK AnmPlayWk;
  BOOL FloorVanish;
}GYM_DRAGON_TMP;

//リソースの並び順番
enum {
  RES_ID_HEAD_MDL = 0,
  RES_ID_L_ARM_MDL,
  RES_ID_R_ARM_MDL,
  RES_ID_FLOOR_MDL,
  RES_ID_BUTTON_MDL,
  RES_ID_HEAD_ANM_UR,
  RES_ID_HEAD_ANM_RU,
  RES_ID_HEAD_ANM_UL,
  RES_ID_HEAD_ANM_LU,
  RES_ID_HEAD_ANM_DR,
  RES_ID_HEAD_ANM_RD,
  RES_ID_HEAD_ANM_DL,
  RES_ID_HEAD_ANM_LD,
  RES_ID_ARM_ANM1,
  RES_ID_ARM_ANM2,
  RES_ID_ARM_ANM3,
  RES_ID_ARM_ANM4,
  RES_ID_BUTTON_ANM,
  RES_ID_FLOOR_ANM,
};

//ＯＢＪインデックス
enum {
  OBJ_HEAD_1 = 0,
  OBJ_L_ARM_1,
  OBJ_R_ARM_1,
  OBJ_BUTTON_L_1,
  OBJ_BUTTON_R_1,
  OBJ_HEAD_2,
  OBJ_L_ARM_2,
  OBJ_R_ARM_2,
  OBJ_BUTTON_L_2,
  OBJ_BUTTON_R_2,
  OBJ_HEAD_3,
  OBJ_L_ARM_3,
  OBJ_R_ARM_3,
  OBJ_BUTTON_L_3,
  OBJ_BUTTON_R_3,
  OBJ_FLOOR,
};



//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_dragon_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_DRAGON, NARC_gym_dragon_in31_l_arm_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_DRAGON, NARC_gym_dragon_in31_r_arm_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_DRAGON, NARC_gym_dragon_m_gym0802f_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_DRAGON, NARC_gym_dragon_in31_botton_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_DRAGON, NARC_gym_dragon_in31_ue_rneck_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_ue_rneck2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_ue_lneck_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_ue_lneck2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_si_rneck_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_si_rneck2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_si_lneck_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_si_lneck2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_DRAGON, NARC_gym_dragon_in31_l_ue_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_l_sita_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_r_ue_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
	{ ARCID_GYM_DRAGON, NARC_gym_dragon_in31_r_sita_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_DRAGON, NARC_gym_dragon_in31_botton_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_GYM_DRAGON, NARC_gym_dragon_m_gym0802f_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
};

//3Dアニメ　頭
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_head[] = {
  { RES_ID_HEAD_ANM_UR,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_RU,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_UL,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_LU,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_DR,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_RD,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_DL,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_HEAD_ANM_LD,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　左腕
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_l_arm[] = {
  { RES_ID_ARM_ANM1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)  腕上
  { RES_ID_ARM_ANM2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)　腕下
};
//3Dアニメ　右腕
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_r_arm[] = {
  { RES_ID_ARM_ANM3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)  腕上
  { RES_ID_ARM_ANM4,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)　腕下
};

//3Dアニメ　ボタン
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_btn[] = {
  { RES_ID_BUTTON_ANM,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　床
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_floor[] = {
  { RES_ID_FLOOR_ANM,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};


//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //頭
	{
		RES_ID_HEAD_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_HEAD_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_head,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_head),	//アニメリソース数
	},
  //左腕
	{
		RES_ID_L_ARM_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_L_ARM_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_l_arm,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_l_arm),	//アニメリソース数
	},
  //右腕
	{
		RES_ID_R_ARM_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_R_ARM_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_r_arm,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_r_arm),	//アニメリソース数
	},
  //ボタン
	{
		RES_ID_BUTTON_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BUTTON_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_btn,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_btn),	//アニメリソース数
	},
  //ボタン
	{
		RES_ID_BUTTON_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BUTTON_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_btn,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_btn),	//アニメリソース数
	},

  //頭
	{
		RES_ID_HEAD_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_HEAD_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_head,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_head),	//アニメリソース数
	},
  //左腕
	{
		RES_ID_L_ARM_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_L_ARM_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_l_arm,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_l_arm),	//アニメリソース数
	},
  //右腕
	{
		RES_ID_R_ARM_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_R_ARM_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_r_arm,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_r_arm),	//アニメリソース数
	},
  //ボタン
	{
		RES_ID_BUTTON_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BUTTON_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_btn,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_btn),	//アニメリソース数
	},
  //ボタン
	{
		RES_ID_BUTTON_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BUTTON_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_btn,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_btn),	//アニメリソース数
	},

  //頭
	{
		RES_ID_HEAD_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_HEAD_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_head,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_head),	//アニメリソース数
	},
  //左腕
	{
		RES_ID_L_ARM_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_L_ARM_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_l_arm,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_l_arm),	//アニメリソース数
	},
  //右腕
	{
		RES_ID_R_ARM_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_R_ARM_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_r_arm,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_r_arm),	//アニメリソース数
	},
  //ボタン
	{
		RES_ID_BUTTON_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BUTTON_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_btn,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_btn),	//アニメリソース数
	},
  //ボタン
	{
		RES_ID_BUTTON_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BUTTON_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_btn,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_btn),	//アニメリソース数
	},

  //床
	{
		RES_ID_FLOOR_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_FLOOR_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_floor,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_floor),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

//首の状態別の適用アニメテーブル
static const int NeckAnmTbl[HEAD_DIR_MAX][HEAD_DIR_MAX] =
{
  //UP,DOWN,LEFT,RIGHT  (NEXT)

  {-1, -1, RES_ID_HEAD_ANM_UL, RES_ID_HEAD_ANM_UR},     //NOW UP
  {-1, -1, RES_ID_HEAD_ANM_DL, RES_ID_HEAD_ANM_DR},     //NOW DOWN
  {RES_ID_HEAD_ANM_LU, RES_ID_HEAD_ANM_LD, -1, -1},     //NOW LEFT
  {RES_ID_HEAD_ANM_RU, RES_ID_HEAD_ANM_RD, -1, -1},     //NOW RIGHT
};

//腕の状態別の適用アニメテーブル
static const int ArmAnmTbl[3/*DRA_ARM_MAX*/][ARM_DIR_MAX] =
{
  //UP,DOWN
  {0, 1},     //LEFT ARM
  {2, 3},     //RIGHT ARM
  {0,0},
};

static const int JumpY[JUMP_COUNT] = { 8,16,8,0,-8,-16,-24,-32 };

//レール進入不可テーブル
static const int RailSt[DRAGON_NUM_MAX][HEAD_DIR_MAX][CHECK_RAIL_NUM] = {
  //ON, ON, OFF, OFF, OFF, OFF
  {
    { RE_LINE_38, RE_LINE_32, RE_LINE_33, RE_LINE_33_new, RE_LINE_34, RE_LINE_34_new }, //首の向き上
    { RE_LINE_38, RE_LINE_32, RE_LINE_33, RE_LINE_33_new, RE_LINE_34, RE_LINE_34_new }, //首の向き下   実際はどの方向にもいけない
    { RE_LINE_34, RE_LINE_34_new, RE_LINE_33, RE_LINE_33_new, RE_LINE_38, RE_LINE_32 }, //首の向き左
    { RE_LINE_33, RE_LINE_33_new, RE_LINE_34, RE_LINE_34_new, RE_LINE_38, RE_LINE_32 }, //首の向き右
  },
  {
    { RE_LINE_11, RE_LINE_14, RE_LINE_12, RE_LINE_12_new, RE_LINE_13, RE_LINE_13_new }, //首の向き上
    { RE_LINE_11, RE_LINE_14, RE_LINE_12, RE_LINE_12_new, RE_LINE_13, RE_LINE_13_new }, //首の向き下  実際はどの方向にもいけない
    { RE_LINE_13, RE_LINE_13_new, RE_LINE_11, RE_LINE_14, RE_LINE_12, RE_LINE_12_new }, //首の向き左
    { RE_LINE_12, RE_LINE_12_new, RE_LINE_11, RE_LINE_14, RE_LINE_13, RE_LINE_13_new }, //首の向き右
  },
  {
    { RE_LINE_53, RE_LINE_54, RE_LINE_55, RE_LINE_55_new, RE_LINE_56, RE_LINE_56_new }, //首の向き上
    { RE_LINE_53, RE_LINE_54, RE_LINE_55, RE_LINE_55_new, RE_LINE_56, RE_LINE_56_new }, //首の向き下  実際はどの方向にもいけない
    { RE_LINE_56, RE_LINE_56_new, RE_LINE_53, RE_LINE_54, RE_LINE_55, RE_LINE_55_new }, //首の向き左
    { RE_LINE_55, RE_LINE_55_new, RE_LINE_53, RE_LINE_54, RE_LINE_56, RE_LINE_56_new }, //首の向き右
  },
};


static void SetupMdl(FLD_EXP_OBJ_CNT_PTR ptr,
    const int inIdx, const VecFx32 *inPos, const u16 *inRad,
    const int inAnmNum, const BOOL inCulling);
static void SetupAnm(GYM_DRAGON_SV_WORK *gmk_sv_work, FLD_EXP_OBJ_CNT_PTR ptr, const int inIdx);

static GMEVENT_RESULT SwitchMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT ArmMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT HeadMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT AnmEvt( GMEVENT* event, int* seq, void* work );
static  HEAD_DIR GetHeadDirByArm(DRAGON_WORK *wk);
static int GetHeadAnmIdx(DRAGON_WORK *wk, const HEAD_DIR inNextDir);
static GMEVENT_RESULT JumpDownEvt( GMEVENT* event, int* seq, void* work );

static void VanishFloor(FLD_EXP_OBJ_CNT_PTR ptr, FIELDMAP_WORK *fieldWork, const BOOL inVanish);
static MMDL *SeatchMmdl(FIELDMAP_WORK *fieldWork, const int inObjID);
static void SetObjVanish(FIELDMAP_WORK *fieldWork, const int inObjID, BOOL inVanish);
static void SetRailSt(FIELDMAP_WORK *fieldWork, const int inHeadIdx, const HEAD_DIR inDir);
static BOOL CheckFloorVanish(const VecFx32 *inVec);

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_DRAGON_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GYM_DRAGON_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  //汎用ワーク確保
  tmp = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_DRAGON_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_DRAGON_TMP));

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_DRAGON_UNIT_IDX );

  //座標セット
  for (i=0;i<DRAGON_NUM_MAX;i++)
  {
    int idx;
    u16 rad = 0;
    rad = DragonRad[i];
    //頭
    idx = OBJ_HEAD_1 + (i*DRAGON_PARTS_SET);
    SetupMdl(ptr, idx, &DragonPos[i], &rad, DRAGON_ANM_NUM, FALSE);

    //左腕
    idx = OBJ_L_ARM_1 + (i*DRAGON_PARTS_SET);
    SetupMdl(ptr, idx, &ArmPos[i], &rad, ARM_ANM_NUM, FALSE);
   
    //右腕
    idx = OBJ_R_ARM_1 + (i*DRAGON_PARTS_SET);
    SetupMdl(ptr, idx, &ArmPos[i], &rad, ARM_ANM_NUM, FALSE);

    //ボタン
    idx = OBJ_BUTTON_L_1 + (i*DRAGON_PARTS_SET);
    SetupMdl(ptr, idx, &ButtonPos[i*2], NULL, BTN_ANM_NUM, TRUE);
    
    //ボタン
    idx = OBJ_BUTTON_R_1 + (i*DRAGON_PARTS_SET);
    SetupMdl(ptr, idx, &ButtonPos[i*2+1], NULL, BTN_ANM_NUM, TRUE);

    //セーブデータを見て、アニメ状態をセット
    SetupAnm(gmk_sv_work, ptr, i);
  }

  //床座標セット
  {
    int idx = OBJ_FLOOR;
    VecFx32 pos = {FLOOR_X,FLOOR_Y,FLOOR_Z};
    SetupMdl(ptr, idx, &pos, NULL, 0, FALSE);
    //アニメ無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, OBJ_FLOOR, 0, TRUE);
  }

  //セットアップ時2階表示判定
  {
    VecFx32 pos;
    BOOL vanish;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    if ( CheckFloorVanish(&pos) ) vanish = TRUE;     //2階の床と竜を非表示
    else vanish = FALSE;       //2階の床と竜を表示

    VanishFloor(ptr, fieldWork, vanish);

    tmp->FloorVanish = vanish;
  }

  //レールの進入不可設定
  for(i=0;i<DRAGON_NUM_MAX;i++){
    DRAGON_WORK *wk = &gmk_sv_work->DraWk[i];
    SetRailSt(fieldWork, i, wk->HeadDir);
  }
}

//--------------------------------------------------------------
/**
 * モデルのセットアップ関数
 * @param	      ptr       ＯＢＪポインタ
 * @param       inIdx     ＯＢＪインデックス
 * @param       inPos     表示座標
 * @param       inRad     回転値ポインタ（ＮＵＬＬのときは計算しない）
 * @param       inAnmNum  アニメ数
 * @param       inCulling カリング有無
 * @return      none
 */
//--------------------------------------------------------------
static void SetupMdl(FLD_EXP_OBJ_CNT_PTR ptr,
    const int inIdx, const VecFx32 *inPos, const u16 *inRad, const int inAnmNum, const BOOL inCulling)
{
  int i;
  GFL_G3D_OBJSTATUS *status; 
  status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_DRAGON_UNIT_IDX, inIdx);
  status->trans = *inPos;
  if (inRad!=NULL) MTX_RotY33(&status->rotate, FX_SinIdx(*inRad), FX_CosIdx(*inRad));
  //カリング設定
  FLD_EXP_OBJ_SetCulling(ptr, GYM_DRAGON_UNIT_IDX, inIdx, inCulling);
  //アニメ設定
  for (i=0;i<inAnmNum;i++)
  {
    //1回再生設定
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, inIdx, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, inIdx, i, FALSE);
  }
/**
  //アルファ設定テスト
  {
    GFL_G3D_UTIL *util = FLD_EXP_OBJ_GetUtil(ptr);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( util, inIdx );
    GFL_G3D_RND* rnd = GFL_G3D_OBJECT_GetG3Drnd( obj );
    GFL_G3D_RES* res = GFL_G3D_RENDER_GetG3DresMdl( rnd );
    NNSG3dResFileHeader* res_head = GFL_G3D_GetResourceFileHeader( res );
    NNSG3dResMdlSet* ms = NNS_G3dGetMdlSet(res_head);
		NNSG3dResMdl* mdl = NNS_G3dGetMdlByIdx(ms, 0);
    NNS_G3dMdlSetMdlAlphaAll((NNSG3dResMdl*)mdl, 24);
  }
*/  
}

//--------------------------------------------------------------
/**
 * アニメのセットアップ関数
 * @param	      ptr       ＯＢＪポインタ
 * @param       inIdx     ドラゴンのインデックス
 * @return      none
 */
//--------------------------------------------------------------
static void SetupAnm(GYM_DRAGON_SV_WORK *gmk_sv_work, FLD_EXP_OBJ_CNT_PTR ptr, const int inIdx)
{
  int anm_ofs;
  int res_base;
  int idx;
  fx32 last;
  EXP_OBJ_ANM_CNT_PTR anm;

  DRAGON_WORK *wk = &gmk_sv_work->DraWk[inIdx];

  res_base = RES_ID_HEAD_ANM_UR;
  //頭
  switch(wk->HeadDir){
  case HEAD_DIR_UP:
    //右＞上のアニメのラストフレームをセットしておく
    anm_ofs = RES_ID_HEAD_ANM_RU - res_base;
    break;
  case HEAD_DIR_DOWN:
    //右＞下のアニメのラストフレームをセットしておく
    anm_ofs = RES_ID_HEAD_ANM_RD - res_base;
    break;
  case HEAD_DIR_RIGHT:
    //上＞右のアニメのラストフレームをセットしておく
    anm_ofs = RES_ID_HEAD_ANM_UR - res_base;
    break;
  case HEAD_DIR_LEFT:
    //上＞左のアニメのラストフレームをセットしておく
    anm_ofs = RES_ID_HEAD_ANM_UL - res_base;
    break;
  default:
    GF_ASSERT(0);
    //右＞上のアニメのラストフレームをセットしておく
    anm_ofs = RES_ID_HEAD_ANM_RU - res_base;
  }

  {
    idx = OBJ_HEAD_1 + (inIdx*DRAGON_PARTS_SET);
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs);
    //アニメ停止解除
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs, TRUE);
    //最終フレームをセット
    last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
    //ラストフレーム
    FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs, last );
  }

  //左腕
  {
    idx = OBJ_L_ARM_1 + (inIdx*DRAGON_PARTS_SET);
    if (wk->ArmDir[DRA_ARM_LEFT] == ARM_DIR_UP) anm_ofs = ARM_DIR_UP;
    else anm_ofs = ARM_DIR_DOWN;

    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs);
    //アニメ停止解除
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs, TRUE);
    //最終フレームをセット
    last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
    //ラストフレーム
    FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs, last );
  }

  //右腕
  {
    idx = OBJ_R_ARM_1 + (inIdx*DRAGON_PARTS_SET);
    if (wk->ArmDir[DRA_ARM_RIGHT] == ARM_DIR_UP) anm_ofs = ARM_DIR_UP;
    else anm_ofs = ARM_DIR_DOWN;

    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs);
    //アニメ停止解除
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs, TRUE);
    //最終フレームをセット
    last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
    //ラストフレーム
    FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_DRAGON_UNIT_IDX, idx, anm_ofs, last );
  }
}


//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_DRAGON_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_DRAGON_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_DRAGON_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

  //自機の高さを監視
  {
    VecFx32 pos;
    BOOL vanish;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    if ( CheckFloorVanish(&pos) ) vanish = TRUE;     //2階の床と竜を非表示
    else vanish = FALSE;       //2階の床と竜を表示

    if (tmp->FloorVanish != vanish)
    {
      VanishFloor(ptr, fieldWork, vanish);
      tmp->FloorVanish = vanish;
    }
  }

  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG){
    GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
    GMEVENT *event;
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      event = GYM_DRAGON_CreateGmkEvt(gsys, 1,0);
      GAMESYSTEM_SetEvent(gsys, event);
    }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
      event = GYM_DRAGON_CreateGmkEvt(gsys, 1,1);
      GAMESYSTEM_SetEvent(gsys, event);
    }
  }
}

//--------------------------------------------------------------
/**
 * ギミック発動イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inDragonIdx   対象ドラゴンインデックス0～2
 * @param       inArmIdx   対象腕インデックス0～1　0：左　1：右
 * 
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_DRAGON_CreateGmkEvt(GAMESYS_WORK *gsys, const int inDragonIdx, const int inArmIdx)
{
  GMEVENT * event;

  GYM_DRAGON_TMP *tmp;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  }

  tmp->TrgtHead = inDragonIdx;
  tmp->TrgtArm = inArmIdx;
  tmp->DraWk = &gmk_sv_work->DraWk[inDragonIdx];

  OS_Printf("head=%d arm=%d\n",inDragonIdx, inArmIdx);

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, SwitchMoveEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 腕ギミック発動イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inDragonIdx   対象ドラゴンインデックス0～2
 * @param       inArmIdx   対象腕インデックス0～1　0：左　1：右
 * 
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_DRAGON_CreateArmMoveEvt(GAMESYS_WORK *gsys, const int inDragonIdx, const int inArmIdx)
{
  GMEVENT * event;

  GYM_DRAGON_TMP *tmp;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  }

  tmp->TrgtHead = inDragonIdx;
  tmp->TrgtArm = inArmIdx;
  tmp->DraWk = &gmk_sv_work->DraWk[inDragonIdx];

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ArmMoveEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 首ギミック発動イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inDragonIdx   対象ドラゴンインデックス0～2
 * 
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_DRAGON_CreateHeadMoveEvt(GAMESYS_WORK *gsys, const int inDragonIdx)
{
  GMEVENT * event;

  GYM_DRAGON_TMP *tmp;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  }

  tmp->TrgtHead = inDragonIdx;
  tmp->DraWk = &gmk_sv_work->DraWk[inDragonIdx];

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, HeadMoveEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 自機アニメイベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inDir       自機の向き
 * 
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_DRAGON_CreateJumpDownEvt(GAMESYS_WORK *gsys, const int inDir)
{
  GMEVENT * event;

  GYM_DRAGON_TMP *tmp;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, JumpDownEvt, 0 );
  {
    ANM_PLAY_WORK *play_work = &tmp->AnmPlayWk;
    play_work->JumpCount = 0;
    play_work->JumpDownX = JUMP_MOVE_X;
    {
      VecFx32 pos;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      play_work->JumpDownBaseY = pos.y;
    }
    if (inDir == DIR_LEFT) play_work->JumpDownX *= -1;
  }

  return event;

}

//--------------------------------------------------------------
/**
 * ドラゴンの腕の状態から次の首の位置を決定する
 * @param	      wk        ドラゴンのデータワーク
 * @return      HEAD_DIR    頭の向き
 */
//--------------------------------------------------------------
static  HEAD_DIR GetHeadDirByArm(DRAGON_WORK *wk)
{
  HEAD_DIR dir;
  if (wk->ArmDir[DRA_ARM_LEFT] == ARM_DIR_UP)
  {
    if (wk->ArmDir[DRA_ARM_RIGHT] == ARM_DIR_UP) dir = HEAD_DIR_UP;   //左腕:上　右腕:上  ⇒　首:上
    else dir = HEAD_DIR_RIGHT;    //左腕:上　右腕:下  ⇒　首:右
  }
  else{
    if (wk->ArmDir[DRA_ARM_RIGHT] == ARM_DIR_UP) dir = HEAD_DIR_LEFT; //左腕:下　右腕:上  ⇒　首:左
    else dir = HEAD_DIR_DOWN;     //左腕:下　右腕:下  ⇒　首:下
  }
  return dir;
}

//--------------------------------------------------------------
/**
 * 現在の首の位置と次の首の位置から、再生する首のアニメーションを決定する
 * @param	      wk        ドラゴンのデータワーク
 * @param       inNextDir    頭の向き
 * @return       int       アニメインデックス
 */
//--------------------------------------------------------------
static int GetHeadAnmIdx(DRAGON_WORK *wk, const HEAD_DIR inNextDir)
{
  int idx = NeckAnmTbl[wk->HeadDir][inNextDir]-RES_ID_HEAD_ANM_UR;
#ifdef PM_DEBUG  
  switch(wk->HeadDir){
  case HEAD_DIR_UP:
    OS_Printf("現在上\n");
    break;
  case HEAD_DIR_DOWN:
    OS_Printf("現在下\n");
    break;
  case HEAD_DIR_LEFT:
    OS_Printf("現在左\n");
    break;
  case HEAD_DIR_RIGHT:
    OS_Printf("現在右\n");
    break;  
  }
#endif 
  GF_ASSERT_MSG(idx>=0, "now=%d next=%d",wk->HeadDir, inNextDir);
  return idx;
}

//--------------------------------------------------------------
/**
 * ドラゴンの腕の状態から次の腕の位置を決定する
 * @param	      wk        ドラゴンのデータワーク
 * @param       inArm     腕インデックス
 * @return       ARM_DIR   腕の向き
 */
//--------------------------------------------------------------
static  ARM_DIR GetArmDir(DRAGON_WORK *wk, const DRA_ARM inArm)
{
  ARM_DIR dir, now_dir;

  now_dir = wk->ArmDir[inArm];
  if (now_dir == ARM_DIR_UP) dir = ARM_DIR_DOWN;
  else dir = ARM_DIR_UP;
  return dir;
}

//--------------------------------------------------------------
/**
 * 指定の腕の移動をするアニメのインデックスを返す
 * @param       inNextDir     移動後の腕の方向
 * @param       inArm         腕のインデックス
 * @return       int           アニメインデックス
 */
//--------------------------------------------------------------
static int GetArmAnmIdx(const ARM_DIR inNextDir, const DRA_ARM inArm)
{
  return ArmAnmTbl[inArm][inNextDir];
}

//--------------------------------------------------------------
/**
 * ギミックアニメイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SwitchMoveEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  switch(*seq){
  case 0:
    {
      GMEVENT * call_event;
      int obj_idx;
      if ( tmp->TrgtArm == DRA_ARM_LEFT ) obj_idx = OBJ_BUTTON_L_1;
      else obj_idx = OBJ_BUTTON_R_1;
      obj_idx += (tmp->TrgtHead*DRAGON_PARTS_SET);
      //スイッチのＯＢＪとアニメをセット
      tmp->AnmPlayWk.ObjIdx = obj_idx;
      tmp->AnmPlayWk.AnmNum = 1;
      tmp->AnmPlayWk.AnmOfs[0] = 0;
      tmp->AnmPlayWk.AllAnmNum = BTN_ANM_NUM;
      tmp->AnmPlayWk.SeNo[0] = GYM_DRAGON_SE_SW;
      tmp->AnmPlayWk.SeNo[1] = NONE_SE;

      call_event = GMEVENT_Create(gsys, NULL, AnmEvt, 0);
      //スイッチ押下アニメイベントコール
      GMEVENT_CallEvent(event, call_event);
      
    }
    (*seq)++;
    break;
  case 1:
    //終了
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ギミックアニメイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ArmMoveEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  switch(*seq){
  case 0:
    {
      GMEVENT * call_event;
      int anm_idx;
      int obj_idx;
      ARM_DIR next_dir;
      //現在状況から動かす腕アニメを決定
      if ( tmp->DraWk->ArmDir[tmp->TrgtArm] == ARM_DIR_UP )
      {
        anm_idx = 1;  //下に動かす
        next_dir = ARM_DIR_DOWN;
        OS_Printf("腕を下に動かす\n");
      }
      else
      {
        anm_idx = 0;  //上に動かす
        next_dir = ARM_DIR_UP;
        OS_Printf("腕を上に動かす\n");
      }

      if ( tmp->TrgtArm == DRA_ARM_LEFT ) obj_idx = OBJ_L_ARM_1;
      else obj_idx = OBJ_R_ARM_1;

      OS_Printf("動かす腕は%d obj= %d\n",tmp->TrgtArm,obj_idx);

      obj_idx += (tmp->TrgtHead*DRAGON_PARTS_SET);

      //腕のＯＢＪとアニメをセット
      tmp->AnmPlayWk.ObjIdx = obj_idx;
      tmp->AnmPlayWk.AnmNum = 1;
      tmp->AnmPlayWk.AnmOfs[0] = anm_idx;
      tmp->AnmPlayWk.AllAnmNum = ARM_ANM_NUM;
      tmp->AnmPlayWk.SeNo[0] = GYM_DRAGON_SE_ARM_MOVE;
      tmp->AnmPlayWk.SeNo[1] = GYM_DRAGON_SE_ARM_STOP;
      
      call_event = GMEVENT_Create(gsys, NULL, AnmEvt, 0);
      //腕アニメイベントコール
      GMEVENT_CallEvent(event, call_event);
      
      //腕の情報を更新
      tmp->DraWk->ArmDir[tmp->TrgtArm] = next_dir;
    }
    (*seq)++;
    break;
  case 1:
    //終了
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}


//--------------------------------------------------------------
/**
 * 首ギミックアニメイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT HeadMoveEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  switch(*seq){
  case 0:
    {
      GMEVENT * call_event;
      HEAD_DIR next_dir;
      int anm_idx;
      int obj_idx;
      
      //腕の状態から、首アニメを決定
      next_dir = GetHeadDirByArm(tmp->DraWk);
      anm_idx = GetHeadAnmIdx(tmp->DraWk, next_dir);

      OS_Printf("次の首の動きは　%d anm=%d\n",next_dir, anm_idx);

      obj_idx = OBJ_HEAD_1+(tmp->TrgtHead*DRAGON_PARTS_SET);

      //首のＯＢＪとアニメをセット
      tmp->AnmPlayWk.ObjIdx = obj_idx;
      tmp->AnmPlayWk.AnmNum = 1;
      tmp->AnmPlayWk.AnmOfs[0] = anm_idx;
      tmp->AnmPlayWk.AllAnmNum = DRAGON_ANM_NUM;
      tmp->AnmPlayWk.SeNo[0] = GYM_DRAGON_SE_HEAD_MOVE;
      tmp->AnmPlayWk.SeNo[1] = GYM_DRAGON_SE_HEAD_STOP;

#ifdef PM_DEBUG
      {
        switch(anm_idx){
        case 0:
          OS_Printf("上＞右\n");
          break;
        case 1:
          OS_Printf("右＞上\n");
          break;
        case 2:
          OS_Printf("上＞左\n");
          break;
        case 3:
          OS_Printf("左＞上\n");
          break;
        case 4:
          OS_Printf("下＞右\n");
          break;
        case 5:
          OS_Printf("右＞下\n");
          break;
        case 6:
          OS_Printf("下＞左\n");
          break;
        case 7:
          OS_Printf("左＞下\n");
          break;
        }
      }
#endif
      
      call_event = GMEVENT_Create(gsys, NULL, AnmEvt, 0);
      //首アニメイベントコール
      GMEVENT_CallEvent(event, call_event);
      //首の情報更新
      tmp->DraWk->HeadDir = next_dir;
    }
    (*seq)++;
    break;
  case 1:
    {
      //首の向きと操作した首のインデックッスからレールの進行状態を変更する
      SetRailSt(fieldWork, tmp->TrgtHead, tmp->DraWk->HeadDir);

      //終了
      return GMEVENT_RES_FINISH;
    }
  }
  
  return GMEVENT_RES_CONTINUE;
}


//--------------------------------------------------------------
/**
 * アニメイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT AnmEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  ANM_PLAY_WORK *play_work = &tmp->AnmPlayWk;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  switch(*seq){
  case 0:
    {
      int i;
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 obj_idx = play_work->ObjIdx;
      OS_Printf("obj_id = %d\n",obj_idx);
      for (i=0;i<play_work->AllAnmNum;i++)
      {
        u8 anm_ofs = i;
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, obj_idx, i);
        //アニメ停止
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //適用されているであろうアニメを無効化（適用していないアニメも無効フラグをセットしにいく）
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, obj_idx, anm_ofs, FALSE);
        OS_Printf("%dを無効\n",anm_ofs);
      }

      for (i=0;i<play_work->AnmNum;i++)
      {
        u8 anm_ofs = play_work->AnmOfs[i];
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, obj_idx, anm_ofs);
        //アニメ停止解除
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //アニメ無効を解除
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_DRAGON_UNIT_IDX, obj_idx, anm_ofs, TRUE);
        //頭出し
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_DRAGON_UNIT_IDX, obj_idx, anm_ofs, 0 );
        OS_Printf("%dを再生\n",anm_ofs);

        //ＳＥ再生
        PMSND_PlaySE(play_work->SeNo[0]);
      }
    }
    (*seq)++;
    break;
  case 1:
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      int obj_idx;
      int anm_idx;
      obj_idx = play_work->ObjIdx;
      anm_idx = play_work->AnmOfs[0];//先頭のアニメで監視
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_DRAGON_UNIT_IDX, obj_idx, anm_idx );
      //アニメ待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        OS_Printf("アニメ終了\n");
        PMSND_StopSE();
        if (play_work->SeNo[1] != NONE_SE) PMSND_PlaySE(play_work->SeNo[1]);

        return GMEVENT_RES_FINISH;
      }
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の飛び降りアニメイベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT JumpDownEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_DRAGON_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  ANM_PLAY_WORK *play_work = &tmp->AnmPlayWk;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  switch(*seq){
  case 0:
    //ジャンプSE再生
    PMSND_PlaySE( GYM_DRAAGON_SE_JUMP );
    (*seq)++;
    break;
  case 1:
    {
      VecFx32 pos;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      pos.x += play_work->JumpDownX;
      pos.y = play_work->JumpDownBaseY + (JumpY[play_work->JumpCount]*FX32_ONE);
      FIELD_PLAYER_SetPos( fld_player, &pos );
      play_work->JumpCount++;
      if ( play_work->JumpCount >= JUMP_COUNT)
      {
        //着地ＳＥ(スイッチＳＥ)
        PMSND_PlaySE( GYM_DRAGON_SE_SW );
        //終了
        return GMEVENT_RES_FINISH;
      }
    }
  }
  
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * フロアの表示・非表示
 * @param	  ptr         拡張ＯＢＪ管理ポインタ
 * @param   fieldWork   フィールドワークポインタ
 * @param   inVanish    バニッシュフラグ　　TRUEで非表示
 * @return  none
 */
//--------------------------------------------------------------
static void VanishFloor(FLD_EXP_OBJ_CNT_PTR ptr, FIELDMAP_WORK *fieldWork, const BOOL inVanish)
{
  FLD_EXP_OBJ_SetVanish( ptr, GYM_DRAGON_UNIT_IDX, OBJ_FLOOR, inVanish );
  FLD_EXP_OBJ_SetVanish( ptr, GYM_DRAGON_UNIT_IDX, OBJ_HEAD_3, inVanish );
  FLD_EXP_OBJ_SetVanish( ptr, GYM_DRAGON_UNIT_IDX, OBJ_L_ARM_3, inVanish );
  FLD_EXP_OBJ_SetVanish( ptr, GYM_DRAGON_UNIT_IDX, OBJ_R_ARM_3, inVanish );
  FLD_EXP_OBJ_SetVanish( ptr, GYM_DRAGON_UNIT_IDX, OBJ_BUTTON_L_3, inVanish );
  FLD_EXP_OBJ_SetVanish( ptr, GYM_DRAGON_UNIT_IDX, OBJ_BUTTON_R_3, inVanish );
  
  SetObjVanish(fieldWork, C08GYM0101_WKOBJCODE00_01, inVanish);
  SetObjVanish(fieldWork, TR_C08GYM0101_F2_01, inVanish);
//  SetObjVanish(fieldWork, TR_C08GYM0101_F2_02, inVanish); <<抹消　20100120
  SetObjVanish(fieldWork, TR_C08GYM0101_F2_03, inVanish);
  SetObjVanish(fieldWork, TR_C08GYM0101_F2_04, inVanish);
}

//--------------------------------------------------------------
/**
 * フィールドモデルの検索
 * @param   fieldWork   フィールドワークポインタ
 * @param   inObjID     ＯＢＪＩＤ
 * @return  MMDL        モデルポインタ
 */
//--------------------------------------------------------------
static MMDL *SeatchMmdl(FIELDMAP_WORK *fieldWork, const int inObjID)
{
  u32 no = 0;
	MMDL *mmdl;
  MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
	while( MMDLSYS_SearchUseMMdl(mmdlsys,&mmdl,&no) == TRUE ){
		if( MMDL_GetOBJID(mmdl) == inObjID ){
			return( mmdl );
		}
	}
  return NULL;
}

//--------------------------------------------------------------
/**
 * ＯＢＪの表示・非表示
 * @param   fieldWork   フィールドワークポインタ
 * @param   inObjID     ＯＢＪＩＤ
 * @param   inVanish    バニッシュフラグ　　TRUEで非表示
 * @return  none
 */
//--------------------------------------------------------------
static void SetObjVanish(FIELDMAP_WORK *fieldWork, const int inObjID, BOOL inVanish)
{
  MMDL *mmdl;
  mmdl = SeatchMmdl(fieldWork, inObjID);
  if (mmdl != NULL) MMDL_SetStatusBitVanish(mmdl, inVanish);
}

//--------------------------------------------------------------
/**
 * レールの進入不可を制御
 * @param   fieldWork   フィールドワークポインタ
 * @param   inHeadIdx   竜の首インデックス　0～2
 * @param   inDir       首の向き
 * @return  none
 */
//--------------------------------------------------------------
static void SetRailSt(FIELDMAP_WORK *fieldWork, const int inHeadIdx, const HEAD_DIR inDir)
{
  int i;
  u32 line_idx[CHECK_RAIL_NUM];
  BOOL valid[CHECK_RAIL_NUM];
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
  
  for (i=0;i<CHECK_RAIL_NUM;i++)
  {
    line_idx[i] = RailSt[inHeadIdx][inDir][i];
    valid[i] = FALSE;
  }

  if (inDir != HEAD_DIR_DOWN)
  {
    valid[0] = TRUE;
    valid[1] = TRUE;
  }

  for (i=0;i<CHECK_RAIL_NUM;i++)
  {
    FLDNOGRID_MAPPER_SetLineActive( p_mapper, line_idx[i], valid[i] );
    NOZOMU_Printf("line%d = %d  Valid=%d\n",i,line_idx[i],valid[i]);
  }
}

//--------------------------------------------------------------
/**
 * 2Fを表示するかのチェック
 * @param   inVec       チェック座標
 * @return  BOOL        TRUEで非表示
 */
//--------------------------------------------------------------
static BOOL CheckFloorVanish(const VecFx32 *inVec)
{
  if ( inVec->y > FLOOR_VISIBLE_HEIGHT ) return FALSE;
  else
  {
    if ( (inVec->x >= FLOOR_VANISH_X)&&(inVec->z >= FLOOR_VANISH_Z) ) return TRUE;
  }

  return FALSE;
}
