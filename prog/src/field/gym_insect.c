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
//#include "script.h"     //for SCRIPT_CallScript
//#include "../../../resource/fldmapdata/script/c04gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_insect/gym_insect_local_def.cdat"

//#include "sound/pm_sndsys.h"

//#include "gym_onsect_se_def.h"

#define GYM_INSECT_UNIT_IDX (0)
#define GYM_INSECT_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)


#define SW_ANM_NUM  (2)
#define PL_ANM_NUM  (2)

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
  {PL9_X,0,PL7_Z},
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

//ジム内部中の一時ワーク
typedef struct GYM_INSECT_TMP_tag
{
  u16 SwIdx;
  u16 PlIdx;
  u16 TrapX;
  u16 TrapZ;
}GYM_INSECT_TMP;


//リソースの並び順番
enum {
  RES_ID_PL_LEFT_MDL = 0,
  RES_ID_PL_RIGHT_MDL,
  RES_ID_EFFECT_MDL,
  RES_ID_SW_MDL,
  RES_ID_WALL_MDL,

  RES_ID_PL_LEFT_MOV1,
  RES_ID_PL_LEFT_MOV2,
  RES_ID_PL_RIGHT_MOV1,
  RES_ID_PL_RIGHT_MOV2,
  RES_ID_EFF_MOV1,
  RES_ID_EFF_MOV2,
  RES_ID_SW_MOV1,
  RES_ID_SW_MOV2,
  RES_ID_WALL1_MOV1,
  RES_ID_WALL1_MOV2,
  RES_ID_WALL2_MOV1,
  RES_ID_WALL2_MOV2,
  RES_ID_WALL3_MOV1,
  RES_ID_WALL3_MOV2,
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
};



//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_INSECT, NARC_gym_insect_c03_pole1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole2_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_effect1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_switch1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD

  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole1_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole2_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_pole2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_effect1_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_effect1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_switch1_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_switch1_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall1_2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall2_2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall3_1_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_INSECT, NARC_gym_insect_c03_wall3_2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
};

//3Dアニメ　壁
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_wall[] = {
	{ RES_ID_WALL1_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL1_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL3_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL3_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
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
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};


static GMEVENT_RESULT SwitchEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PoleEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT TrTrapEvt( GMEVENT* event, int* seq, void* work );

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

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_INSECT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_INSECT_TMP));
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_INSECT_UNIT_IDX );

  //座標セット
  for (i=0;i<INSECT_SW_MAX;i++)
  {
    int idx = OBJ_SW1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, idx);
    status->trans = SwPos[i];
  }
  for (i=0;i<INSECT_PL_MAX;i++)
  {
    int idx = OBJ_PL1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, idx);
    status->trans = PolePos[i];
  }
  for (i=0;i<INSECT_WALL_MAX;i++)
  {
    int idx = OBJ_WALL1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_INSECT_UNIT_IDX, idx);
    status->trans = WallPos[i];
  }


  //スイッチのセットアップ
  //ウォールバンドのセットアップ
  //ポールのセットアップ

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

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * バンド押し込み後イベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_PushWall(FIELDMAP_WORK *fieldWork)
{
  //イベント作成
  ;
  //イベントコール
  ;

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
GMEVENT* GYM_INSECT_CreateTrTrapEvt(GAMESYS_WORK *gsys, const int inTrapX, const int inTrapZ)
{
  //イベント作成
  GMEVENT* event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  //トラップ位置セット
  tmp->TrapX = inTrapX;
  tmp->TrapZ = inTrapZ;
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
        u8 check_idx;
        //ポールフラグオン
        gmk_sv_work->Pl[tmp->PlIdx] = TRUE;
        //ウォールバンド弱体化チェック
        check_idx = WallIdxCheck[ tmp->PlIdx ];
        if ( gmk_sv_work->WallSt[tmp->PlIdx] == WALL_ST_NORMAL )
        {
          //弱体化アニメ開始
          ;
          (*seq)++;
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
    //弱体化アニメ待ち
    if (1)
    {
      //ウォールステートを弱体状態に変更
      gmk_sv_work->WallSt[tmp->PlIdx] = WALL_ST_WEAKNESS;
      //イベント終了
      return GMEVENT_RES_FINISH;
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
    (*seq)++;
    break;
  case 1: //トレーナー出現アニメスクリプトコール
    (*seq)++;
    break;
  case 2:
    //出現アニメ終了待ち
    if (1){
      //トレーナー戦スクリプトチェンジ
      ;
    }
  }
  return GMEVENT_RES_CONTINUE;
}
