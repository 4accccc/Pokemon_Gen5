//======================================================================
/**
 * @file  gym_insect.c
 * @brief  虫ジム
 * @author  Saito
 * @date  09.11.05
 */
//======================================================================

#include "fieldmap.h"
#include "gym_insect_sv.h"
#include "gym_insect.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_insect.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c03gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_insect/gym_insect_local_def.h"

#include "sound/pm_sndsys.h"
#include "gym_insect_se_def.h"

#include "event_fldmmdl_control.h"  //for EVENT_ObjAnime

#define GYM_INSECT_UNIT_IDX (0)
#define GYM_INSECT_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)


#define SW_ANM_NUM  (2)
#define PL_ANM_NUM  (2)
#define EFF_ANM_NUM  (2)

#define WALL_BRK_ANM_NUM   (4)

#define WALL_CHG_ANM_NUM   (4)

#define WALL_ANM_NUM   (4+WALL_CHG_ANM_NUM)

#define TRAP_NUM  (2)

#define BAND_WIDTH  (32)

#define CONT_TIME   (2)   //押し込み値が加算されるベタ押しの時間

#define PAIR_NONE (-1)

//ポール座標
#define PL1_X (PL1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL1_Z (PL1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL2_X (PL2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL2_Z (PL2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL3_X (PL3_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL3_Z (PL3_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL4_X (PL4_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL4_Z (PL4_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL5_X (PL5_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL5_Z (PL5_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL6_X (PL6_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL6_Z (PL6_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL7_X (PL7_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL7_Z (PL7_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL8_X (PL8_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL8_Z (PL8_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL9_X (PL9_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL9_Z (PL9_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL10_X (PL10_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define PL10_Z (PL10_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

//スイッチ座標
#define SW1_X (SW1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW1_Z (SW1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW2_X (SW2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW2_Z (SW2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW3_X (SW3_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW3_Z (SW3_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW4_X (SW4_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW4_Z (SW4_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW5_X (SW5_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW5_Z (SW5_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW6_X (SW6_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW6_Z (SW6_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW7_X (SW7_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW7_Z (SW7_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW8_X (SW8_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW8_Z (SW8_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

//バンド座標
#define BD1_X (BD1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD1_Z (BD1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD2_X (BD2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD2_Z (BD2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD3_X (BD3_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD3_Z (BD3_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD4_X (BD4_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD4_Z (BD4_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD5_X (BD5_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD5_Z (BD5_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD6_X (BD6_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD6_Z (BD6_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD7_X (BD7_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define BD7_Z (BD7_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

static const VecFx32 SwPos[INSECT_SW_MAX] = {
  {SW1_X,0,SW1_Z},
  {SW2_X,0,SW2_Z},
  {SW3_X,0,SW3_Z},
  {SW4_X,0,SW4_Z},
  {SW5_X,0,SW5_Z},
  {SW6_X,0,SW6_Z},
  {SW7_X,0,SW7_Z},
  {SW8_X,0,SW8_Z},
};

static const VecFx32 PolePos[INSECT_PL_MAX] = {
  {PL1_X,0,PL1_Z},
  {PL2_X,0,PL2_Z},
  {PL3_X,0,PL3_Z},
  {PL4_X,0,PL4_Z},
  {PL5_X,0,PL5_Z},
  {PL6_X,0,PL6_Z},
  {PL7_X,0,PL7_Z},
  {PL8_X,0,PL8_Z},
  {PL9_X,0,PL9_Z},
  {PL10_X,0,PL10_Z},
};

static const VecFx32 WallPos[INSECT_WALL_MAX] = {
  {BD1_X,0,BD1_Z},
  {BD2_X,0,BD2_Z},
  {BD3_X,0,BD3_Z},
  {BD4_X,0,BD4_Z},
  {BD5_X,0,BD5_Z},
  {BD6_X,0,BD6_Z},
  {BD7_X,0,BD7_Z},
};

//ポールごとの関連壁インデックス一覧
static const u8 WallIdxCheck[INSECT_PL_MAX] = {
  0,0,
  1,1,
  2,2,
  3,
  4,
  5,5
};    //※ウォールインデックス6はポールとの接続がない

//ポールごとの対関連インデックス一覧
static const s8 PolePairIdx[INSECT_PL_MAX] = {
  1,  //0
  0,  //1
  3,  //2
  2,  //3
  5,  //4
  4,  //5
  PAIR_NONE,  //6 対なし
  PAIR_NONE,  //7 対なし
  9,   //8
  8,  //9
};

//トラップの位置
static const VecFx32 TrapPos[TRAP_NUM] = 
{
  {
    TRAP1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE,
    0,
    TRAP1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE
  },
  {
    TRAP2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE,
    0,
    TRAP2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE
  },
};

//ジム内部中の一時ワーク
typedef struct GYM_INSECT_TMP_tag
{
  u8 SwIdx;
  u8 PlIdx;
  u8 TrEvtIdx;
  u8 WallIdx;
  u8 BrkWallIdx;
  u8 Val;
  u8 Dir;
  u8 Cont;
  VecFx32 BasePos;
  GFL_TCB *PushAnmTcb;
  BOOL PushEnd;
  BOOL PushStepStart;
  BOOL SeFlg;
}GYM_INSECT_TMP;

//リソースの並び順番
enum {
  RES_ID_PL_LEFT_MDL = 0,
  RES_ID_PL_RIGHT_MDL,
  RES_ID_EFFECT_MDL,
  RES_ID_SW_MDL,
  RES_ID_WALL_MDL,
  RES_ID_WALL_BRK_MDL,
  RES_ID_WALL_LOW_MDL,
  RES_ID_WALL_BRK_LOW_MDL,

  RES_ID_PL_LEFT_MOV1,
  RES_ID_PL_LEFT_MOV2,
  RES_ID_PL_RIGHT_MOV1,
  RES_ID_PL_RIGHT_MOV2,
  RES_ID_EFF_MOV1,
  RES_ID_EFF_MOV2,
  RES_ID_SW_MOV1,
  RES_ID_SW_MOV2,
  RES_ID_WALL1_MOV_N,
  RES_ID_WALL1_MOV_S,
  RES_ID_WALL2_MOV_N,
  RES_ID_WALL2_MOV_S,
  RES_ID_WALL3_MOV1,
  RES_ID_WALL3_MOV2,
  RES_ID_WALL3_MOV3,
  RES_ID_WALL3_MOV4,

  RES_ID_WALL1_BRK_MOV_N,
  RES_ID_WALL1_BRK_MOV_S,
  RES_ID_WALL2_BRK_MOV_N,
  RES_ID_WALL2_BRK_MOV_S,
};

//ＯＢＪインデックス
enum {
  OBJ_SW1= 0,
  OBJ_SW2,
  OBJ_SW3,
  OBJ_SW4,
  OBJ_SW5,
  OBJ_SW6,
  OBJ_SW7,
  OBJ_SW8,

  OBJ_PL1,
  OBJ_PL2,
  OBJ_PL3,
  OBJ_PL4,
  OBJ_PL5,
  OBJ_PL6,
  OBJ_PL7,
  OBJ_PL8,
  OBJ_PL9,
  OBJ_PL10,

  OBJ_WALL1,
  OBJ_WALL2,
  OBJ_WALL3,
  OBJ_WALL4,
  OBJ_WALL5,
  OBJ_WALL6,
  OBJ_WALL7,

  OBJ_WALL_BRK1,
  OBJ_WALL_BRK2,
  OBJ_WALL_BRK3,
  OBJ_WALL_BRK4,
  OBJ_WALL_BRK5,
  OBJ_WALL_BRK6,
  OBJ_WALL_BRK7,

  OBJ_EFF,
  OBJ_WALL_DETAIL,
  OBJ_WALL_BRK_DETAIL,
};

//壁アニメの順番
enum{
  ANM_WALL_N = 0,   //北向き突破
  ANM_WALL_S,       //南向き突破
  ANM_WALL_SIDE_N,  //北向きサイド
  ANM_WALL_SIDE_S,  //南向きサイド
  ANM_WALL_CHG1,    //壁変更1(壊れない壁のみ)
  ANM_WALL_CHG2,    //壁変更2(壊れない壁のみ)
  ANM_WALL_CHG3,    //壁変更3(壊れない壁のみ)
  ANM_WALL_CHG4,    //壁変更4(壊れない壁のみ)
};




//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_INSECT, NARC_gym_insect_c03_pole1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole2_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_effect1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_switch1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1s_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2s_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD


  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole1_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole2_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_effect1_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_effect1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_switch1_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_switch1_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP

  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_1a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_1a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_2a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_2b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_3_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_3_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_3_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_3_nsbma, GFL_G3D_UTIL_RESARC }, //IMA

  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_1a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_1b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_2a_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_2b_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
};

//3Dアニメ　壁
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_wall[] = {
	{ RES_ID_WALL1_MOV_N,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL1_MOV_S,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_MOV_N,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_MOV_S,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL3_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL3_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL3_MOV3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL3_MOV4,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　壊れる壁
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_wall_brk[] = {
	{ RES_ID_WALL1_BRK_MOV_N,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL1_BRK_MOV_S,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_BRK_MOV_N,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_BRK_MOV_S,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw[] = {
  { RES_ID_SW_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　ポール1
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_plL[] = {
  { RES_ID_PL_LEFT_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_PL_LEFT_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　ポール2
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_plR[] = {
  { RES_ID_PL_RIGHT_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_PL_RIGHT_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　トラップエフェクト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_eff[] = {
  { RES_ID_EFF_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_EFF_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};



//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //スイッチ1
	{
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ2
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ3
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ4
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ5
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ6
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ7
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ8
  {
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},

  //ポール1   左
  {
		RES_ID_PL_LEFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_LEFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plL,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plL),	//アニメリソース数
	},
  //ポール2   右
  {
		RES_ID_PL_RIGHT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_RIGHT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plR,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plR),	//アニメリソース数
	},
  //ポール3   左
  {
		RES_ID_PL_LEFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_LEFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plL,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plL),	//アニメリソース数
	},
  //ポール4   右
  {
		RES_ID_PL_RIGHT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_RIGHT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plR,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plR),	//アニメリソース数
	},
  //ポール5   左
  {
		RES_ID_PL_LEFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_LEFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plL,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plL),	//アニメリソース数
	},
  //ポール6   右
  {
		RES_ID_PL_RIGHT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_RIGHT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plR,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plR),	//アニメリソース数
	},
  //ポール7   右
  {
		RES_ID_PL_RIGHT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_RIGHT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plR,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plR),	//アニメリソース数
	},
  //ポール8   右
  {
		RES_ID_PL_RIGHT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_RIGHT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plR,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plR),	//アニメリソース数
	},
  //ポール9   左
  {
		RES_ID_PL_LEFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_LEFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plL,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plL),	//アニメリソース数
	},
  //ポール10    右
  {
		RES_ID_PL_RIGHT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_PL_RIGHT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_plR,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_plR),	//アニメリソース数
	},

  //壁1
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壁2
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壁3
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壁4
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壁5
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壁6
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壁7
  {
		RES_ID_WALL_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},

  //壊せる壁1
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壊せる壁2
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壊せる壁3
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壊せる壁4
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壊せる壁5
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壊せる壁6
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
  //壊せる壁7
  {
		RES_ID_WALL_BRK_LOW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_LOW_MDL, 	//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
#if 0
  //壁1
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //壁2
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //壁3
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //壁4
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //壁5
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //壁6
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //壁7
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},

  //壊せる壁1
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
  //壊せる壁2
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
  //壊せる壁3
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
  //壊せる壁4
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
  //壊せる壁5
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
  //壊せる壁6
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
  //壊せる壁7
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},
#endif  
  //トラップエフェクト
  {
		RES_ID_EFFECT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_EFFECT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_eff,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_eff),	//アニメリソース数
	},
  //アニメ付き壁
  {
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //アニメ付き壊せる壁
  {
		RES_ID_WALL_BRK_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_BRK_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall_brk,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall_brk),	//アニメリソース数
	},

};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

static const MMDL_ACMD_LIST anime_up_table[] = {
  AC_WALK_U_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_down_table[] = {
  AC_WALK_D_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_up_push_table[] = {
  AC_STAY_WALK_U_32F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_down_push_table[] = {
  AC_STAY_WALK_D_32F, 1,
  ACMD_END, 0
};

static GMEVENT_RESULT SwitchEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PoleEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT TrTrapEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PushWallEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WallReverseEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PushWallSideEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PushWallNoBreakEvt( GMEVENT* event, int* seq, void* work );
static int GetWallInfo( GYM_INSECT_SV_WORK *gmk_sv_work,
                        const int inX, const int inZ,
                        GYM_INSECT_WALL_ST *outSt, BOOL *outCenter  );

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GYM_INSECT_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  {
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("bfore_size = %x\n",size);
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_INSECT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_INSECT_TMP));
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_INSECT_UNIT_IDX );
{
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("bfore_size = %x\n",size);
  }

  //座標セット
  for (i=0;i<INSECT_SW_MAX;i++)
  {
    int j;
    int idx = OBJ_SW1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, idx);
    status->trans = SwPos[i];
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_INSECT_UNIT_IDX, idx, TRUE);

    for (j=0;j<SW_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ有効
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, idx, j, TRUE);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
      if ( gmk_sv_work->Sw[i] ){
        fx32 last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
        //ラストフレーム
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, idx, j, last );  
      }
    }
  }
  for (i=0;i<INSECT_PL_MAX;i++)
  {
    int j;
    int idx = OBJ_PL1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, idx);
    status->trans = PolePos[i];
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_INSECT_UNIT_IDX, idx, TRUE);
    for (j=0;j<PL_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ有効
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, idx, j, TRUE);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
      if ( gmk_sv_work->Pl[i] ){
        fx32 last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
        //ラストフレーム
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, idx, j, last );  
      }      
    }
  }
  for (i=0;i<INSECT_WALL_MAX;i++)
  {
    GFL_G3D_OBJSTATUS *status;
    int j;
    int nrm_idx = OBJ_WALL1 + i;
    int brk_idx = OBJ_WALL_BRK1 + i;
    status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, nrm_idx);
    status->trans = WallPos[i];
    status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, brk_idx);
    status->trans = WallPos[i];

    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_INSECT_UNIT_IDX, nrm_idx, TRUE);
    FLD_EXP_OBJ_SetCulling(ptr, GYM_INSECT_UNIT_IDX, brk_idx, TRUE);
    
    //壁の状態のより分岐
    switch(gmk_sv_work->WallSt[i]){
    case WALL_ST_NORMAL:
      //壊せる壁を消す
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, brk_idx, TRUE );
      break;
    case WALL_ST_WEAKNESS:
      //通常壁を消す
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, nrm_idx, TRUE );
      break;
    default:
      GF_ASSERT(0);
    }
  }
  
  {
    int j;
    int eff_idx = OBJ_EFF;
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_INSECT_UNIT_IDX, eff_idx, TRUE);
    for (j=0;j<EFF_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, eff_idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }
  }

  //アニメ付き壁
  {
    int j;
    int nrm_idx = OBJ_WALL_DETAIL;
    int brk_idx = OBJ_WALL_BRK_DETAIL;
    for (j=0;j<WALL_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, nrm_idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //無効化
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, nrm_idx, j, FALSE);
    }
    for (j=0;j<WALL_BRK_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, brk_idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    }
    FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, nrm_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, brk_idx, TRUE );
  }
  //スイッチのセットアップ
  //ポールのセットアップ
  {
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("after_size = %x\n",size);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_INSECT_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_INSECT_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  //フィールドの残りヒープをプリント
  if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("size = %x\n",size);
  }

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

}

//--------------------------------------------------------------
/**
 * 壁情報取得
 * @param
 * @param
 * @param
 * @param
 * @param
 * @return    
 */
//--------------------------------------------------------------
static int GetWallInfo( GYM_INSECT_SV_WORK *gmk_sv_work,
                        const int inX, const int inZ,
                        GYM_INSECT_WALL_ST *outSt, BOOL *outCenter  )
{
  int i;
  for(i=0;i<INSECT_WALL_MAX;i++)
  {
    BOOL hit;
    int x,z;
    x = WallPos[i].x / FIELD_CONST_GRID_FX32_SIZE;
    z = WallPos[i].z / FIELD_CONST_GRID_FX32_SIZE;
    hit = FALSE;
    if ( (x == inX) && (z == inZ) )
    {
      //ど真ん中
      *outCenter = TRUE;
      hit = TRUE;
    }
    else if ( (x-1 == inX) && (z == inZ) )
    {
      //サイド
      *outCenter = FALSE;
      hit = TRUE;
    }
    else if( (x+1 == inX) && (z == inZ) )
    {
      //サイド
      *outCenter = FALSE;
      hit = TRUE;
    }

    if (hit){
      *outSt = gmk_sv_work->WallSt[i];
      return i;
    }
  }

  //見つからなかったとき
  *outCenter = FALSE;
  *outSt = WALL_ST_NORMAL;    //<<不定値防止
  return INSECT_WALL_MAX;
}

//--------------------------------------------------------------
/**
 * 壁押し込みイベント作成
 * @param	      gsys     ゲームシステムポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT* GYM_INSECT_CreatePushWallEvt(GAMESYS_WORK *gsys, const int inX, const int inZ)
{
  GMEVENT* event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  int idx;
  GYM_INSECT_WALL_ST wall_st;
  BOOL weak;
  u16 dir;

  GYM_INSECT_SV_WORK *gmk_sv_work;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  dir = FIELD_PLAYER_GetDir( fld_player );

  if ( (dir!=DIR_UP)&&(dir!=DIR_DOWN) ) return NULL;

  //獲得座標は壊せる壁のどの部分かを調べる
  idx = GetWallInfo(gmk_sv_work, inX,inZ, &wall_st, &weak);
  if (idx < INSECT_WALL_MAX)
  {
    if (weak)
    {
      if (wall_st == WALL_ST_WEAKNESS)
      {
        //中央弱っている壁
        event = GMEVENT_Create( gsys, NULL, PushWallEvt, 0 );
      }
      else
      {
        //中央弱っていない壁
        event = GMEVENT_Create( gsys, NULL, PushWallNoBreakEvt, 0 );
      }
    }
    else
    {
      //壁サイド
      event = GMEVENT_Create( gsys, NULL, PushWallSideEvt, 0 );
    }
  }
  else
  {
    //壊せる壁にヒットしていない
    return NULL;
  }

  //イベント開始時のセットアップ
  {
    tmp->Val = 0;
    tmp->Cont = 0;
    //自機の向き取得
    tmp->Dir = dir;
    //壁インデックス
    tmp->WallIdx = idx;
    //ＴＣＢ初期化
    tmp->PushAnmTcb = NULL;
    tmp->PushEnd = FALSE;
    tmp->PushStepStart = FALSE;
  }

  return event;
}


//--------------------------------------------------------------
/**
 * バンド押し込み後イベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PushWallEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  u8 obj_idx;
  u8 detail_obj_idx;
  u8 anm_idx;
  EXP_OBJ_ANM_CNT_PTR anm;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  obj_idx = OBJ_WALL_BRK1+tmp->WallIdx;
  detail_obj_idx = OBJ_WALL_BRK_DETAIL;
  anm_idx = ANM_WALL_N;
  //向きでアニメインデックスにオフセットをつける
  if (tmp->Dir == DIR_DOWN){
    anm_idx += 1;
  }
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx);

  switch(*seq){
  case 0:
    {
      MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      //自機アニメ終了待ち
      if ( MMDL_CheckEndAcmd( mmdl ) && MMDL_CheckPossibleAcmd( mmdl ) )
      {
        //足踏みスタート
        tmp->PushStepStart = TRUE;
        FIELD_PLAYER_GetPos( fld_player, &tmp->BasePos );

        //アニメついていない壁を非表示にする
        FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, TRUE );
        //アニメする壁を表示
        FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, FALSE );
        //座標セット
        {
          GFL_G3D_OBJSTATUS *status;
          GFL_G3D_OBJSTATUS *status_detail;
          status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, obj_idx);
          status_detail = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx);
          status_detail->trans = status->trans;
        }
        //アニメ有効化
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, TRUE);
        //頭出し
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, 0 );
        //アニメ停止
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);

        //ＳＥ再生フラグ初期化
        tmp->SeFlg = FALSE;
        (*seq)++;
      }
    }
    break;
  case 1:
    {
      int check_key;
      int accel;
      //キー入力検出
      if ( tmp->Dir == DIR_UP ){
        check_key = PAD_KEY_UP;
        accel = -1;
      }else{
        check_key = PAD_KEY_DOWN;
        accel = 1;
      }

      if ( GFL_UI_KEY_GetCont() & check_key )
      {
        VecFx32 pos;
        tmp->Cont++;
        if (tmp->Cont>=CONT_TIME)
        {
          int frm;
          tmp->Cont = 0;
          tmp->Val++;
          FIELD_PLAYER_GetPos( fld_player, &pos );
          pos.z += (accel*FX32_ONE);
          FIELD_PLAYER_SetPos( fld_player, &pos );
          frm = tmp->Val/2;
          if ( (!tmp->SeFlg) && (frm != 0 ))
          {
            //壁押し込みSE
            PMSND_PlaySE(GYM_INSECT_SE_WALL_PUSH);
            //ＳＥ再生フラグＯＮ
            tmp->SeFlg = TRUE;
          }
          //アニメフレーム進める
          FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX,
                                    detail_obj_idx, anm_idx, frm*FX32_ONE );

          if ( tmp->Val >= BAND_WIDTH ) (*seq)++;
          
        } //end if (tmp->Cont>=CONT_TIME)
      } //end if ( GFL_UI_KEY_GetCont() & check_key )
      else
      {
        GMEVENT *next_evt;
        //キー入力やめた
        next_evt = GMEVENT_Create(gsys, NULL, WallReverseEvt, 0);
        GMEVENT_ChangeEvent(event, next_evt);
        return GMEVENT_RES_CONTINUE;
      }
    }
    break;
  case 2:
    if ( tmp->PushAnmTcb != NULL )
    {
      if( MMDL_CheckEndAcmdList(tmp->PushAnmTcb) == TRUE )
      {
        MMDL_EndAcmdList( tmp->PushAnmTcb );
		    tmp->PushAnmTcb = NULL;
        tmp->PushEnd = TRUE;
        (*seq)++;
      }
    }else{
      tmp->PushEnd = TRUE;
      (*seq)++;
    }

    if (tmp->PushEnd){
      //残りアニメ再生
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      //アニメコマンド発行
      {
        GMEVENT * child;
        void *tbl;
        if ( tmp->Dir == DIR_UP ){
          tbl = (void*)anime_up_table;
        }else{
          tbl = (void*)anime_down_table;
        }
        child = EVENT_ObjAnime( gsys, fieldWork, MMDL_ID_PLAYER, tbl );
        GMEVENT_CallEvent(event, child);
      }
      //ＳＥ停止
      PMSND_StopSE();
      //壁つきぬけSE
      PMSND_PlaySE(GYM_INSECT_SE_WALL_THROUGH);
    }
    break;
  case 3:
    //アニメ終了待ち
    {
      fx32 frm;
      frm = FLD_EXP_OBJ_GetObjAnmFrm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx );
      OS_Printf("%x :: %d\n", frm, frm/FX32_ONE);
    }
    if ( FLD_EXP_OBJ_ChkAnmEnd(anm) ){
      //アニメ無効化
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, FALSE);
      //アニメする壁を非表示
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, TRUE);
      //アニメしない壁を表示
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, FALSE);
      return GMEVENT_RES_FINISH;
    }
  }

  //押し込みアニメ
  if ( (tmp->PushStepStart) &&(!tmp->PushEnd) )
  {
    MMDL *fmmdl;
    fmmdl = FIELD_PLAYER_GetMMdl( fld_player );

    if ( tmp->PushAnmTcb == NULL)
    {
      void *tbl;
      if ( tmp->Dir == DIR_UP ) tbl = (void*)anime_up_push_table;
      else tbl = (void*)anime_down_push_table;
      tmp->PushAnmTcb = MMDL_SetAcmdList( fmmdl, tbl );
    }
    else{
      if( MMDL_CheckEndAcmdList(tmp->PushAnmTcb) == TRUE )
      {
        MMDL_EndAcmdList( tmp->PushAnmTcb );
			  tmp->PushAnmTcb = NULL;
      }
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 弾かれイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WallReverseEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  u8 obj_idx = OBJ_WALL_BRK1+tmp->WallIdx;
  u8 detail_obj_idx = OBJ_WALL_BRK_DETAIL;
  u8 anm_idx = ANM_WALL_N;
  //向きでアニメインデックスにオフセットをつける
  if (tmp->Dir == DIR_DOWN){
    anm_idx += 1;
  }

  switch(*seq){
  case 0:
    //ＳＥ停止
    PMSND_StopSE();
    (*seq)++;
    //NONE BREAK
  case 1:
    //アニメ終了待ち
    if ( tmp->PushAnmTcb != NULL)
    {
      if( MMDL_CheckEndAcmdList(tmp->PushAnmTcb) == TRUE )
      {
        MMDL_EndAcmdList( tmp->PushAnmTcb );
			  tmp->PushAnmTcb = NULL;
        tmp->Val = 0;
        (*seq)++;
      }
    }
    else
    {
      tmp->Val = 0;
      (*seq)++;
    }

    //シーケンスシフトが発生したら
    if (*seq == 2) {
      //SE再生フラグ初期化
      tmp->SeFlg = FALSE;
    }
    break;
  case 2:
    {
      BOOL arrive;
      VecFx32 pos;
      fx32 diff;
      int accel;
      fx32 frm;
      FIELD_PLAYER_GetPos( fld_player, &pos );
      //戻る値セット
      tmp->Val++;
      arrive = FALSE;
      if ( tmp->Dir == DIR_UP ){
        accel = 1;
        pos.z += (accel*tmp->Val*FX32_ONE);
        //基点地点まで戻ったか？
        if ( pos.z >= tmp->BasePos.z )
        {
          pos.z = tmp->BasePos.z;
          arrive = TRUE; //到達
        }
        diff = tmp->BasePos.z - pos.z;
      }else{
        accel = -1;
        pos.z += (accel*tmp->Val*FX32_ONE);
        //基点地点まで戻ったか？
        if ( pos.z <= tmp->BasePos.z )
        {
          pos.z = tmp->BasePos.z;
          arrive = TRUE; //到達
        }
        diff = pos.z - tmp->BasePos.z;
      }

      frm = diff/2;

      if ( (!tmp->SeFlg) && (frm != 0) )
      {
        //弾かれSE
        PMSND_PlaySE(GYM_INSECT_SE_WALL_RETURN);
        //ＳＥ再生フラグＯＮ
        tmp->SeFlg = TRUE;
      }

      FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, frm );
      FIELD_PLAYER_SetPos( fld_player, &pos );

      if (arrive)
      {
        //アニメ無効化
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, FALSE);
        //アニメする壁を非表示
        FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, TRUE);
        //アニメしない壁を表示
        FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, FALSE);
        
        return GMEVENT_RES_FINISH;
      }
    }
  }

  return GMEVENT_RES_CONTINUE;
}


//--------------------------------------------------------------
/**
 * バンドサイド押し込みイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PushWallSideEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  u8 obj_idx;
  u8 detail_obj_idx;
  u8 anm_idx;
  EXP_OBJ_ANM_CNT_PTR anm;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  //突破できる壁かで分岐
  if(gmk_sv_work->WallSt[tmp->WallIdx] == WALL_ST_NORMAL){
    obj_idx = OBJ_WALL1+tmp->WallIdx;
    detail_obj_idx = OBJ_WALL_DETAIL;
  }else{
    obj_idx = OBJ_WALL_BRK1+tmp->WallIdx;
    detail_obj_idx = OBJ_WALL_BRK_DETAIL;
  }

  anm_idx = ANM_WALL_SIDE_N;
  //向きでアニメインデックスにオフセットをつける
  if (tmp->Dir == DIR_DOWN){
    anm_idx += 1;
  }

  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx);

  switch(*seq){
  case 0:
    //アニメついていない壁を非表示にする
    FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, TRUE );
    //アニメする壁を表示
    FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, FALSE );
    //座標セット
    {
       GFL_G3D_OBJSTATUS *status;
       GFL_G3D_OBJSTATUS *status_detail;
       status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, obj_idx);
       status_detail = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx);
       status_detail->trans = status->trans;
    }
    //壁が動くアニメ（サイドなので、突破できない）
    //アニメを開始
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, TRUE);
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //頭出し
    FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, 0 );
    //壁に触れたSE
    PMSND_PlaySE(GYM_INSECT_SE_WALL_TOUCH);
    
    (*seq)++;
    break;
  case 1:
    //アニメ終了待ち
    if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
    {
      //アニメし終わったら無効化しておく
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, FALSE);
      //アニメする壁を非表示
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, TRUE);
      //アニメしない壁を表示
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, FALSE);
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 突破できないバンド押し込みイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PushWallNoBreakEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  u8 obj_idx;
  u8 detail_obj_idx;
  u8 anm_idx;
  EXP_OBJ_ANM_CNT_PTR anm;

  obj_idx = OBJ_WALL1+tmp->WallIdx;
  detail_obj_idx = OBJ_WALL_DETAIL;

  anm_idx = ANM_WALL_N;
  //向きでアニメインデックスにオフセットをつける
  if (tmp->Dir == DIR_DOWN){
    anm_idx += 1;
  }

  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx);

  switch(*seq){
  case 0:
    //アニメついていない壁を非表示にする
    FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, TRUE );
    //アニメする壁を表示
    FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, FALSE );
    //座標セット
    {
       GFL_G3D_OBJSTATUS *status;
       GFL_G3D_OBJSTATUS *status_detail;
       status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, obj_idx);
       status_detail = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx);
       status_detail->trans = status->trans;
    }
    //壁が動くアニメ
    //アニメを開始
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, TRUE);
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //頭出し
    FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, 0 );
    //壁に触れたSE
    PMSND_PlaySE(GYM_INSECT_SE_WALL_TOUCH);
    (*seq)++;
    break;
  case 1:
    //アニメ終了待ち
    if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
    {
      //アニメし終わったら無効化しておく
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, anm_idx, FALSE);
      //アニメする壁を非表示
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_obj_idx, TRUE);
      //アニメしない壁を表示
      FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, obj_idx, FALSE);
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スイッチ押下イベント
 * @param	      gsys     ゲームシステムポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT* GYM_INSECT_CreateSwitchEvt(GAMESYS_WORK *gsys, const int inSwIdx)
{
  GMEVENT* event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  if (inSwIdx >= INSECT_SW_MAX){
    return NULL;
  }

  //動作予定スイッチインデックスをセット
  tmp->SwIdx = inSwIdx;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, SwitchEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * ポールアニメイベント
 * @param	      gsys     ゲームシステムポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT* GYM_INSECT_CreatePoleEvt(GAMESYS_WORK *gsys, const int inPoleIdx)
{
  GMEVENT* event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  if (inPoleIdx >= INSECT_PL_MAX){
    return NULL;
  }

  //動作予定ポールインデックスをセット
  tmp->PlIdx = inPoleIdx;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, PoleEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * トレーナートラップイベント開始
 * @param	      gsys     ゲームシステムポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT* GYM_INSECT_CreateTrTrapEvt(GAMESYS_WORK *gsys,
                                    const int inTrEvtIdx)
{
  //イベント作成
  GMEVENT* event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  //トラップイベントインデックスセット
  tmp->TrEvtIdx = inTrEvtIdx;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, TrTrapEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * スイッチイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SwitchEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  switch(*seq){
  case 0:  //スイッチアニメ開始
    {
      int i;
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 sw_obj_idx;
      sw_obj_idx = OBJ_SW1 + tmp->SwIdx;
      //アニメを開始
      for (i=0;i<SW_ANM_NUM;i++){
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, sw_obj_idx, i, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, sw_obj_idx, i);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      }
    }
    (*seq)++;
    break;
  case 1:
    {
      u8 sw_obj_idx;
      EXP_OBJ_ANM_CNT_PTR sw_anm;
      sw_obj_idx = OBJ_SW1 + tmp->SwIdx;
      //↓アニメ終了判定はくっついている始めアニメで行うことにする
      sw_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, sw_obj_idx, 0);
      //スイッチアニメ終了待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(sw_anm) )
      {
        //スイッチ押下フラグオン
        gmk_sv_work->Sw[tmp->SwIdx] = TRUE;
        //イベント終了
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ポールイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PoleEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  switch(*seq){
  case 0:  //ポールアニメ開始
    {
      int i;
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 pl_obj_idx;
      pl_obj_idx = OBJ_PL1 + tmp->PlIdx;
      //アニメを開始
      for (i=0;i<PL_ANM_NUM;i++){
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, pl_obj_idx, i, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, pl_obj_idx, i);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      }
    }
    (*seq)++;
    break;
  case 1:
    {
      u8 pl_obj_idx;
      EXP_OBJ_ANM_CNT_PTR pl_anm;
      pl_obj_idx = OBJ_PL1 + tmp->PlIdx;
      //↓アニメ終了判定はくっついている始めアニメで行うことにする
      pl_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, pl_obj_idx, 0);
      //ポールアニメ終了待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(pl_anm) )
      {
        s8 pair;
        u8 check_idx;
        BOOL open;
        open = FALSE; //ポール解放フラグ初期化
        //ポールフラグオン
        gmk_sv_work->Pl[tmp->PlIdx] = TRUE;
        //対になるポールのフラグ状態を調べる
        pair = PolePairIdx[tmp->PlIdx];
        if (pair != PAIR_NONE){
          if (gmk_sv_work->Pl[pair] ) open = TRUE;
        }else{
          open = TRUE;
        }

        if (open)
        {
          //ウォールバンド弱体化チェック
          check_idx = WallIdxCheck[ tmp->PlIdx ];
          if ( gmk_sv_work->WallSt[check_idx] == WALL_ST_NORMAL )
          {
            int i;
            EXP_OBJ_ANM_CNT_PTR anm;
            u8 wall_obj_idx;
            u8 detail_wall_obj_idx;
            u8 wall_anm_idx;
            wall_obj_idx = OBJ_WALL1 + check_idx;
            detail_wall_obj_idx = OBJ_WALL_DETAIL;
            //壊れる壁のインデックスを保存
            tmp->BrkWallIdx = check_idx;
            //アニメついていない壁を非表示にする
            FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, wall_obj_idx, TRUE );
            //アニメする壁を表示
            FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, FALSE );
            //座標セット
            {
              GFL_G3D_OBJSTATUS *status;
              GFL_G3D_OBJSTATUS *status_detail;
              status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, wall_obj_idx);
              status_detail = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx);
              status_detail->trans = status->trans;
            }

            for (i=0;i<WALL_CHG_ANM_NUM;i++)
            {
              wall_anm_idx = ANM_WALL_CHG1+i;
              //弱体化アニメ開始
              FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, wall_anm_idx, TRUE);
              anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, wall_anm_idx);
              FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
              //頭出し
              FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, wall_anm_idx, 0 );
            }
            //柵降りるSE
            PMSND_PlaySE(GYM_INSECT_SE_WALL_OPEN);
            (*seq)++;
          }
          else
          {
            //ポールが完全解放されていないので、イベント終了
            return GMEVENT_RES_FINISH;
          }
        }
        else
        {
          //壁の弱体化がないので、イベント終了
          return GMEVENT_RES_FINISH;
        }
      }
    }
    break;
  case 2:
    {
      u8 wall_obj_idx;
      u8 detail_wall_obj_idx;
      u8 anm_idx;
      EXP_OBJ_ANM_CNT_PTR anm;
      wall_obj_idx = OBJ_WALL1 + tmp->BrkWallIdx;
      detail_wall_obj_idx = OBJ_WALL_DETAIL;
      anm_idx = ANM_WALL_CHG1;   //アニメ終了判定用
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, anm_idx);
      //弱体化アニメ待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        int i;
        //アニメし終わったら無効化しておく
        for (i=0;i<WALL_CHG_ANM_NUM;i++)
        {
          u8 wall_anm_idx = ANM_WALL_CHG1+i;
          FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, wall_anm_idx, FALSE);
        }
        //ウォールステートを弱体状態に変更
        gmk_sv_work->WallSt[tmp->BrkWallIdx] = WALL_ST_WEAKNESS;
        //ＯＢＪ表示切替
        {
          int nrm_idx = OBJ_WALL1 + tmp->BrkWallIdx;
          int brk_idx = OBJ_WALL_BRK1 + tmp->BrkWallIdx;
          //通常壁を消す
          FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, nrm_idx, TRUE );
          //壊せる壁を表示
          FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, brk_idx, FALSE );
          //アニメする壁を非表示
          FLD_EXP_OBJ_SetVanish( ptr, GYM_INSECT_UNIT_IDX, detail_wall_obj_idx, TRUE);
        }
        //柵移動終了ＳＥ
        PMSND_PlaySE(GYM_INSECT_SE_WALL_OPEN_END);
        //イベント終了
        return GMEVENT_RES_FINISH;
      }
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トレーナートラップイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT TrTrapEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  switch(*seq){
  case 0:  //トレーナー出現アニメ開始
    {
      int i;
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 obj_idx;
      obj_idx = OBJ_EFF;
      //アニメを開始
      for (i=0;i<EFF_ANM_NUM;i++){
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_INSECT_UNIT_IDX, obj_idx, i, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, obj_idx, i);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //頭だし
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, obj_idx, i, 0 );
      }
      //位置セット
      {
        GFL_G3D_OBJSTATUS *status;
        status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, obj_idx);
        status->trans = TrapPos[tmp->TrEvtIdx];
      }
    }
    (*seq)++;
    break;
  case 1: //トレーナー出現アニメスクリプトコール
    {      
      int scr_id;
      if ( tmp->TrEvtIdx ){
        scr_id = SCRID_PRG_C03GYM0101_TR2_APPEAR;
      }else{
        scr_id = SCRID_PRG_C03GYM0101_TR1_APPEAR;
      }
      SCRIPT_CallScript( event, scr_id,
          NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );      
    }
    (*seq)++;
    break;
  case 2:
    {
      u8 obj_idx;
      EXP_OBJ_ANM_CNT_PTR anm;
      obj_idx = OBJ_EFF;
      //↓アニメ終了判定はくっついている始めアニメで行うことにする
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, obj_idx, 0);
      //出現アニメ終了待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        int scr_id;
        if ( tmp->TrEvtIdx ){
          scr_id = SCRID_PRG_C03GYM0101_TR2_BTL;
        }else{
          scr_id = SCRID_PRG_C03GYM0101_TR1_BTL;
        }
        //トレーナー戦スクリプトチェンジ
        SCRIPT_ChangeScript( event, scr_id,
          NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
        (*seq)++;
      }
    }
    break;
  case 3:
    GF_ASSERT(0);   //イベントチェンジしているので、ここには来ない
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
