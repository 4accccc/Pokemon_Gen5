//======================================================================
/**
 * @file  gym_fly.c
 * @brief  飛行ジム
 * @author  Saito
 */
//======================================================================
#include "fieldmap.h"
#include "gym_fly_sv.h"
#include "gym_fly.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"
#include "system/ica_anime.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_fly.naix"

#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c06gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_fly/gym_fly_local_def.cdat"

#define GYM_FLY_UNIT_IDX (0)
#define GYM_FLY_TMP_ASSIGN_ID  (1)

#define FRAME_POS_SIZE  (4*3*150)   //座標ＸＹＺ各4バイトｘ150フレーム
#define HEADER_SIZE  (8)   //フレームサイズ4バイト+格納情報3バイトのヘッダー情報があるためアライメントを加味して8バイト確保する

#define CANNON_NUM_MAX  (10)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define CANNON_ANM_NUM  (2) //大砲1つのアニメの種類（データ依存）

/**
//砲台グリッド座標
#define CAN1_GX (15)
#define CAN1_GY (0)
#define CAN1_GZ (28)
#define CAN2_GX (15)
#define CAN2_GY (0)
#define CAN2_GZ (9)
#define CAN3_GX (23)
#define CAN3_GY (0)
#define CAN3_GZ (17)
#define CAN4_GX (20)
#define CAN4_GY (0)
#define CAN4_GZ (9)
#define CAN5_GX (11)
#define CAN5_GY (0)
#define CAN5_GZ (13)
#define CAN6_GX (8)
#define CAN6_GY (0)
#define CAN6_GZ (4)
#define CAN7_GX (28)
#define CAN7_GY (0)
#define CAN7_GZ (4)
#define CAN8_GX (26)
#define CAN8_GY (0)
#define CAN8_GZ (22)
#define CAN9_GX (6)
#define CAN9_GY (0)
#define CAN9_GZ (20)
#define CAN10_GX (2)
#define CAN10_GY (0)
#define CAN10_GZ (13)

//扉開くフレーム
#define   CAN_UP_OP_FRM     (15)
#define   CAN_DOWN_OP_FRM   (15)
#define   CAN_LEFT_OP_FRM   (30)
#define   CAN_RIGHT_OP_FRM  (30)
*/
//砲台座標
#define CAN1_X  (CAN1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN1_Y  (CAN1_GY*FIELD_CONST_GRID_FX32_SIZE)
#define CAN1_Z  (CAN1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN2_X  (CAN2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN2_Y  (CAN2_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN2_Z  (CAN2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN3_X  (CAN3_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN3_Y  (CAN3_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN3_Z  (CAN3_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN4_X  (CAN4_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN4_Y  (CAN4_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN4_Z  (CAN4_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN5_X  (CAN5_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN5_Y  (CAN5_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN5_Z  (CAN5_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN6_X  (CAN6_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN6_Y  (CAN6_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN6_Z  (CAN6_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN7_X  (CAN7_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN7_Y  (CAN7_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN7_Z  (CAN7_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN8_X  (CAN8_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN8_Y  (CAN8_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN8_Z  (CAN8_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN9_X  (CAN9_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN9_Y  (CAN9_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN9_Z  (CAN9_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CAN10_X  (CAN10_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN10_Y  (CAN10_GY*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CAN10_Z  (CAN10_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

/**
//着地座標
#define STAND1_GX (15)
#define STAND1_GY (2)
#define STAND1_GZ (19)
*/

#define ANM_BIN_DATA_NONE (0xffffffff)

//ジム内部中の一時ワーク
typedef struct GYM_FLY_TMP_tag
{
  u8 ShotIdx;
  u8 ShotDir;
  u8 FrameSetStart;
  u8 dummy;

  u8 FramePosDat[FRAME_POS_SIZE+HEADER_SIZE];
  ICA_ANIME* IcaAnmPtr;
  u16 NowIcaAnmFrmIdx;
  u16 MaxIcaAnmFrm;
}GYM_FLY_TMP;

//リソースの並び順番
enum {
  RES_ID_CAN_MDL = 0,
  RES_ID_CAN_ANM1_ICA,
  RES_ID_CAN_ANM1_IMA,
  RES_ID_CAN_ANM2_ICA,
  RES_ID_CAN_ANM2_IMA,
  RES_ID_CAN_ANM3_ICA,
  RES_ID_CAN_ANM3_IMA,
  RES_ID_CAN_ANM4_ICA,
  RES_ID_CAN_ANM4_IMA,
};

//ＯＢＪインデックス
enum {
  OBJ_CAN_1 = 0,
  OBJ_CAN_2,
  OBJ_CAN_3,
  OBJ_CAN_4,
  OBJ_CAN_5,
  OBJ_CAN_6,
  OBJ_CAN_7,
  OBJ_CAN_8,
  OBJ_CAN_9,
  OBJ_CAN_10,
};

//アニメの順番
enum{
  ANM_CAN_ANM1 = 0,    //基準の下から
  ANM_CAN_ANM2,    //基準の右から
  ANM_CAN_ANM3,    //基準の上から
  ANM_CAN_ANM4,    //基準の左から
};

//大砲ごとの基準からの回転
static u8 CannoneRot[CANNON_NUM_MAX] = {
  0,    //回転なし      上向き
  0,    //回転なし      上向き
  1,    //半時計90度    左向き
  3,    //反時計270度   右向き
  1,    //反時計90度    左向き
  2,    //反時計180度   下向き
  3,    //反時計270度   右向き
  0,    //回転なし      上向き
  1,    //反時計90度    左向き
  2,    //反時計180度   下向き
};

//大砲位置
static const u16 CanPos[CANNON_NUM_MAX][3] = {
  {CAN1_GX, CAN1_GY, CAN1_GZ},
  {CAN2_GX, CAN2_GY, CAN2_GZ},
  {CAN3_GX, CAN3_GY, CAN3_GZ},
  {CAN4_GX, CAN4_GY, CAN4_GZ},
  {CAN5_GX, CAN5_GY, CAN5_GZ},
  {CAN6_GX, CAN6_GY, CAN6_GZ},
  {CAN7_GX, CAN7_GY, CAN7_GZ},
  {CAN8_GX, CAN8_GY, CAN8_GZ},
  {CAN9_GX, CAN9_GY, CAN9_GZ},
  {CAN10_GX, CAN10_GY, CAN10_GZ},
};

//方向ごとの大砲扉が開くフレーム
static u16 CanOpenFrame[4] = {
  CAN_UP_OP_FRM,
  CAN_DOWN_OP_FRM,
  CAN_RIGHT_OP_FRM,
  CAN_LEFT_OP_FRM,
};

//方向ごとの大砲扉が閉まりきるフレーム
static u16 CanClosedFrame[4] = {
  CAN_UP_CL_FRM,
  CAN_DOWN_CL_FRM,
  CAN_RIGHT_CL_FRM,
  CAN_LEFT_CL_FRM,
};

//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {  
	{ ARCID_GYM_FLY, NARC_gym_fly_cannon_01_00_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　大砲本体
  
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_01_00_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　大砲アニメ1
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_01_00_nsbma, GFL_G3D_UTIL_RESARC }, //IMA　大砲アニメ1
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_02_00_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　大砲アニメ2
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_02_00_nsbma, GFL_G3D_UTIL_RESARC }, //IMA　大砲アニメ2
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_03_00_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　大砲アニメ3
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_03_00_nsbma, GFL_G3D_UTIL_RESARC }, //IMA　大砲アニメ3
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_04_00_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　大砲アニメ4
  { ARCID_GYM_FLY, NARC_gym_fly_cannon_04_00_nsbma, GFL_G3D_UTIL_RESARC }, //IMA　大砲アニメ4
};

//3Dアニメ　大砲
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl[] = {
	{ RES_ID_CAN_ANM1_ICA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM1_IMA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM2_ICA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM2_IMA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM3_ICA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM3_IMA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM4_ICA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_CAN_ANM4_IMA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //大砲1
	{
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲2
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲3
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲4
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲5
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲6
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲7
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲8
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲9
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //大砲10
  {
		RES_ID_CAN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CAN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

typedef struct POINT_tag
{
  int x;
  int y;
  int z;
}POINT;

//打ち上げデータ
typedef struct SHOT_DATA_tag
{
  int ArcID;
  POINT Point;
}SHOT_DATA;

static const SHOT_DATA ShotData[CANNON_NUM_MAX][4] = {
  //1
  {
    { NARC_gym_fly_null_01_00_bin, {STAND1_GX,STAND1_GY,STAND1_GZ} },   //上
    { NARC_gym_fly_null_01_01_bin, {STAND2_GX,STAND2_GY,STAND2_GZ} },   //下
    { ANM_BIN_DATA_NONE, {0,0,0} },   //右
    { ANM_BIN_DATA_NONE, {0,0,0} },   //左
  },
  //2
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { NARC_gym_fly_null_02_00_bin, {STAND3_GX,STAND3_GY,STAND3_GZ} },   //右
    { NARC_gym_fly_null_02_01_bin, {STAND4_GX,STAND4_GY,STAND4_GZ} },   //左
  },
  //3
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { ANM_BIN_DATA_NONE, {0,0,0} },   //右
    { NARC_gym_fly_null_03_00_bin, {STAND5_GX,STAND5_GY,STAND5_GZ} },   //左
  },
  //4
  {
    { NARC_gym_fly_null_04_00_bin, {STAND6_GX,STAND6_GY,STAND6_GZ} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { NARC_gym_fly_null_04_01_bin, {STAND7_GX,STAND7_GY,STAND7_GZ} },   //右
    { ANM_BIN_DATA_NONE, {0,0,0} },   //左
  },
  //5
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { NARC_gym_fly_null_05_00_bin, {STAND8_GX,STAND8_GY,STAND8_GZ} },   //右
    { NARC_gym_fly_null_05_01_bin, {STAND9_GX,STAND9_GY,STAND9_GZ} },   //左
  },
  //6
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { NARC_gym_fly_null_06_00_bin, {STAND10_GX,STAND10_GY,STAND10_GZ} },   //下
    { ANM_BIN_DATA_NONE, {0,0,0} },   //右
    { ANM_BIN_DATA_NONE, {0,0,0} },   //左
  },
  //7
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { NARC_gym_fly_null_07_00_bin, {STAND11_GX,STAND11_GY,STAND11_GZ} },   //右
    { ANM_BIN_DATA_NONE, {0,0,0} },   //左
  },
  //8
  {
    { NARC_gym_fly_null_08_00_bin, {STAND12_GX,STAND12_GY,STAND12_GZ} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { ANM_BIN_DATA_NONE, {0,0,0} },   //右
    { ANM_BIN_DATA_NONE, {0,0,0} },   //左
  },
  //9
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { ANM_BIN_DATA_NONE, {0,0,0} },   //下
    { ANM_BIN_DATA_NONE, {0,0,0} },   //右
    { NARC_gym_fly_null_09_00_bin, {STAND13_GX,STAND13_GY,STAND13_GZ} },   //左
  },
  //10
  {
    { ANM_BIN_DATA_NONE, {0,0,0} },   //上
    { NARC_gym_fly_null_10_00_bin, {STAND14_GX,STAND14_GY,STAND14_GZ} },   //下
    { ANM_BIN_DATA_NONE, {0,0,0} },   //右
    { ANM_BIN_DATA_NONE, {0,0,0} },   //左
  },
};

static GMEVENT_RESULT ShotEvt( GMEVENT* event, int* seq, void* work );
static u8 GetCanIdx(const int inX, const int inZ);

static u8 GetCannonAnmOfs(u16 inDir);
static u8 GetDirIdxFromDir(u16 inDir);

#ifdef PM_DEBUG
BOOL test_GYM_FLY_Shot(GAMESYS_WORK *gsys, const u8 inShotIdx, const u8 inShotDir);
#endif

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork)
{
  u8 i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_FLY_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_FLY_TMP));

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_FLY_UNIT_IDX );

  for (i=0;i<CANNON_NUM_MAX;i++){
    //座標セット　大砲
    {
      VecFx32 pos[CANNON_NUM_MAX] = {
        {CAN1_X, CAN1_Y, CAN1_Z},{CAN2_X, CAN2_Y, CAN2_Z},{CAN3_X, CAN3_Y, CAN3_Z},{CAN4_X, CAN4_Y, CAN4_Z},
        {CAN5_X, CAN5_Y, CAN5_Z},{CAN6_X, CAN6_Y, CAN6_Z},{CAN7_X, CAN7_Y, CAN7_Z},{CAN8_X, CAN8_Y, CAN8_Z},
        {CAN9_X, CAN9_Y, CAN9_Z},{CAN10_X, CAN10_Y, CAN10_Z},
      };
      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_FLY_UNIT_IDX, OBJ_CAN_1+i);
      status->trans = pos[i];
      //回転セット
      ;
    }
    //1回再生設定
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_FLY_UNIT_IDX, OBJ_CAN_1+i, 0);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_FLY_UNIT_IDX, OBJ_CAN_1+i, 1);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
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
void GYM_FLY_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_FLY_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_FLY_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
#ifdef PM_DEBUG
  //テスト
  {
    GAMESYS_WORK *gsys  = FIELDMAP_GetGameSysWork( fieldWork );
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
      //自機の向きを取得
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      u16 dir = FIELD_PLAYER_GetDir( fld_player );
      test_GYM_FLY_Shot(gsys, 0, dir);
    }
  } 
#endif
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

}

//--------------------------------------------------------------
/**
 * 打ち出しイベント作成
 * @param	
 * @return
 */
//--------------------------------------------------------------
GMEVENT *GYM_FLY_CreateShotEvt(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  GYM_FLY_TMP *tmp;

  u8 shot_idx;
  u16 dir;

  GYM_FLY_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_FLY );
  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);

  {
    int x,z;
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    x = pos.x / FIELD_CONST_GRID_FX32_SIZE;
    z = pos.z / FIELD_CONST_GRID_FX32_SIZE;
    //自機の向きによって分岐
    dir = FIELD_PLAYER_GetDir( fld_player );
    switch( dir ){
    case DIR_UP:
      z-=2;
      break;
    case DIR_DOWN:
      z+=2;
      break;
    case DIR_LEFT:
      x-=2;
      break;
    case DIR_RIGHT:
      x+=2;
      break;
    }
    //自機の位置から、大砲番号を調べる
    shot_idx = GetCanIdx(x,z);
  }

  //大砲が見つからない場合は終了
  if (shot_idx >= CANNON_NUM_MAX){
    return NULL;
  }

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, ShotEvt, 0 );
    
  //打ち出しインデックスをセット
  tmp->ShotIdx = shot_idx;
  //打ち出し方向をセット
  tmp->ShotDir = dir;

  return event;
}
#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * 実験イベント起動
 * @param	
 * @return
 */
//--------------------------------------------------------------
BOOL test_GYM_FLY_Shot(GAMESYS_WORK *gsys, const u8 inShotIdx, const u8 inShotDir)
{
  u8 shot_idx;
  
  GYM_FLY_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_FLY_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_FLY );

  {
    int x,z;
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    x = pos.x / FIELD_CONST_GRID_FX32_SIZE;
    z = pos.z / FIELD_CONST_GRID_FX32_SIZE;
    switch( inShotDir ){
    case DIR_UP:
      z-=2;
      break;
    case DIR_DOWN:
      z+=2;
      break;
    case DIR_LEFT:
      x-=2;
      break;
    case DIR_RIGHT:
      x+=2;
      break;
    }
    //自機の位置から、大砲番号を調べる
    shot_idx = GetCanIdx(x,z);
  }

  //大砲が見つからない場合は終了
  if (shot_idx >= CANNON_NUM_MAX){
    return NULL;
  }

  {
    //イベントセット
    GMEVENT * event = GMEVENT_Create( gsys, NULL, ShotEvt, 0/*sizeof(ENCEFF_WORK)*/ );
    GAMESYSTEM_SetEvent(gsys, event);
    //打ち出しインデックスをセット
    tmp->ShotIdx = inShotIdx;
    //打ち出し方向をセット
    tmp->ShotDir = inShotDir;
  }
  return TRUE;
  
}
#endif  //PM_DEBUG

//--------------------------------------------------------------
/**
 * 発射イベント
 * @param	
 * @return
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ShotEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_FLY_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_FLY_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_FLY );
  }

  switch(*seq){
  case 0:
    {
      int arc_idx;
      {
        u8 dir_idx;
        dir_idx = GetDirIdxFromDir(tmp->ShotDir);
        arc_idx = ShotData[tmp->ShotIdx][dir_idx].ArcID;
        if (arc_idx == ANM_BIN_DATA_NONE){
          GF_ASSERT(0);
          return GMEVENT_RES_FINISH;
        }
      }

      tmp->IcaAnmPtr = ICA_ANIME_Create( 
          GFL_HEAP_LOWID(HEAPID_FIELDMAP), ARCID_GYM_FLY, arc_idx,
          tmp->FramePosDat, FRAME_POS_SIZE );
      GF_ASSERT(tmp->IcaAnmPtr != NULL);
      tmp->MaxIcaAnmFrm = ICA_ANIME_GetMaxFrame(tmp->IcaAnmPtr);
    }

    //移動データロード
    ;
    //自機の向きにより分岐
    //大砲ドアオープンフレームまで再生開始
    ;
    {
      u8 anm_ofs;
      u8 obj_idx;
      obj_idx = tmp->ShotIdx;
      //大砲アニメを自機の向きから決定する
      anm_ofs = GetCannonAnmOfs(tmp->ShotDir);
      //アニメを開始
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_FLY_UNIT_IDX, obj_idx, anm_ofs, TRUE);
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_FLY_UNIT_IDX, obj_idx, anm_ofs+1, TRUE);
      //頭出し
      FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_FLY_UNIT_IDX, obj_idx, anm_ofs, 0 );
      FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_FLY_UNIT_IDX, obj_idx, anm_ofs+1, 0 );  
    }
    (*seq)++;
    break;
  case 1:
    {
      u8 obj_idx;
      u8 dir_idx;
      fx32 frm;
      obj_idx = tmp->ShotIdx;
      frm = FLD_EXP_OBJ_GetObjAnmFrm(ptr, GYM_FLY_UNIT_IDX, obj_idx, 0);
      dir_idx = GetDirIdxFromDir(tmp->ShotDir);
      //オープンフレーム到達監視
      if (frm >= CanOpenFrame[dir_idx]*FX32_ONE){
        //大砲アニメ停止
        {
          EXP_OBJ_ANM_CNT_PTR anm;
          anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_FLY_UNIT_IDX, obj_idx, 0);
          FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
          anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_FLY_UNIT_IDX, obj_idx, 1);
          FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        }
        //自機、大砲に入る
        OS_Printf("スクリプトコール\n");
        SCRIPT_CallScript( event, SCRID_PRG_C06GYM0101_PLAYER_IN,
          NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
        (*seq)++;
      }
    }
    break;
  case 2:
    //自機アニメ終了待ち
    if(1){
      {
        MMDL * mmdl;
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        mmdl = FIELD_PLAYER_GetMMdl( fld_player );
        //自機を非表示
        MMDL_SetStatusBitVanish(mmdl, TRUE);
        //自機の高さ取得を禁止
        MMDL_OnStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }
      //大砲アニメ再開
      {
        u8 obj_idx;
        EXP_OBJ_ANM_CNT_PTR anm;
        obj_idx = tmp->ShotIdx;
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_FLY_UNIT_IDX, obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_FLY_UNIT_IDX, obj_idx, 1);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      }
      (*seq)++;
    }
    break;
  case 3:
    {
      u8 obj_idx;
      u8 dir_idx;
      fx32 frm;
      obj_idx = tmp->ShotIdx;
      frm = FLD_EXP_OBJ_GetObjAnmFrm(ptr, GYM_FLY_UNIT_IDX, obj_idx, 0);
      dir_idx = GetDirIdxFromDir(tmp->ShotDir);
      //発射フレーム到達チェック
      if (frm >= CanClosedFrame[dir_idx]*FX32_ONE){
        //フレーム読み取り開始
        tmp->FrameSetStart = 1;
        tmp->NowIcaAnmFrmIdx = 0;
        (*seq)++;
      }
    }
    break;
  case 4:
    //大砲アニメ終了チェック＆フレーム読み取り終了チェック
    if (!tmp->FrameSetStart){
      //主人公最終位置セット
      {
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        //主人公の座標セット
        {
          VecFx32 pos = {0,0,0};
          u8 dir_idx;
          const POINT *point;
          dir_idx = GetDirIdxFromDir(tmp->ShotDir);
          point = &ShotData[tmp->ShotIdx][dir_idx].Point;

          pos.x = point->x * FIELD_CONST_GRID_FX32_SIZE;
          pos.x += GRID_HALF_SIZE;
          pos.y = point->y * FIELD_CONST_GRID_FX32_SIZE;
          pos.z = point->z * FIELD_CONST_GRID_FX32_SIZE;
          pos.z += GRID_HALF_SIZE;
          FIELD_PLAYER_SetPos( fld_player, &pos );
        }        
      }      
      (*seq)++;
    }
    break;
  case 5:
    {
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      //自機の高さ取得を開始
      MMDL_OffStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
    }
    //フレーム読み取り解放
    ICA_ANIME_Delete( tmp->IcaAnmPtr );

    //大砲アニメを切り離し
    {
      u8 anm_ofs;
      u8 obj_idx;
      obj_idx = tmp->ShotIdx;
      //大砲アニメを自機の向きから決定する
      anm_ofs = GetCannonAnmOfs(tmp->ShotDir);
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_FLY_UNIT_IDX, obj_idx, anm_ofs, FALSE);
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_FLY_UNIT_IDX, obj_idx, anm_ofs+1, FALSE);
    }
    return GMEVENT_RES_FINISH;
  }

  //フレーム読み取り処理部分
  if (tmp->FrameSetStart){      //読み取りフラグが立っていれば、実行
    u8 obj_idx;
    u8 anm_idx;
    VecFx32 dst_vec = {0,0,0};

    obj_idx = tmp->ShotIdx;

    if (tmp->NowIcaAnmFrmIdx == SHOT_APP_FRM){
      //自機表示
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      MMDL_SetStatusBitVanish(mmdl, FALSE);
    }

    //座標取得
    if ( ICA_ANIME_GetTranslateAt( tmp->IcaAnmPtr, &dst_vec, tmp->NowIcaAnmFrmIdx ) ){
      OS_Printf("%x %x %x\n",dst_vec.x,dst_vec.y,dst_vec.z);
      dst_vec.x += (CanPos[obj_idx][0]*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE);
      dst_vec.y += (CanPos[obj_idx][1]*FIELD_CONST_GRID_FX32_SIZE);
      dst_vec.z += (CanPos[obj_idx][2]*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE);
      //自機座標セット
      FIELD_PLAYER_SetPos( FIELDMAP_GetFieldPlayer( fieldWork ), &dst_vec );
    }

    //アニメフレーム更新
    tmp->NowIcaAnmFrmIdx++;
    
    //終了チェック
    if ( tmp->NowIcaAnmFrmIdx >= tmp->MaxIcaAnmFrm ){
      tmp->FrameSetStart = 0;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//指定グリッド座標に大砲があるかを調べる
static u8 GetCanIdx(const int inX, const int inZ)
{
  u8 i;

  for(i=0;i<CANNON_NUM_MAX;i++){
    if ( (CanPos[i][0] == inX)&&(CanPos[i][2] == inZ) ){
      return i;
    }
  }
  return i;
}

static u8 GetCannonAnmOfs(u16 inDir)
{
  u8 anm_ofs;
  switch(inDir){
  case DIR_UP:
    anm_ofs = 0;
    break;
  case DIR_DOWN:
    anm_ofs = 1*CANNON_ANM_NUM;
    break;
  case DIR_LEFT:
    anm_ofs = 2*CANNON_ANM_NUM;
    break;
  case DIR_RIGHT:
    anm_ofs = 3*CANNON_ANM_NUM;
    break;
  default:
    GF_ASSERT(0);
    anm_ofs = 0;
  }
  return anm_ofs;
}

static u8 GetDirIdxFromDir(u16 inDir)
{
  switch(inDir){
  case DIR_UP:
    return 0;
  case DIR_DOWN:
    return 1;
  case DIR_RIGHT:
    return 2;
  case DIR_LEFT:  
    return 3;
  default:
    GF_ASSERT(0);
    return 0;
  }
}

