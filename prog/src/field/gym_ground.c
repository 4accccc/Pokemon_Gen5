//======================================================================
/**
 * @file  gym_ground.c
 * @brief  地面ジム
 * @author  Saito
 * @date  09.11.11
 */
//======================================================================

#include "fieldmap.h"
#include "gym_ground_sv.h"
#include "gym_ground.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_ground.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP

#include "../../../resource/fldmapdata/gimmick/gym_ground/gym_ground_local_def.h"

#include "sound/pm_sndsys.h"

#include "gym_ground_se_def.h"

#include "height_ex.h"    //for ExHeight

#define GYM_GROUND_UNIT_IDX (0)
#define GYM_GROUND_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define FLOOR_RECT_NUM  (8)
#define LIFT_ANM_NUM    (2)

#define EXIT_LIFT_MOVE_SPD (2*FX32_ONE)

#define LIFT_MOVE_SPD (LIFT_MOVE_SPEED*FX32_ONE)
#define SP_LIFT_MOVE_SPD1 (SP_LIFT_MOVE_SPEED1*FX32_ONE)
#define SP_LIFT_MOVE_SPD2 (SP_LIFT_MOVE_SPEED2*FX32_ONE)

#define SP_LIFT_IDX (5)     //メインリフトインデックス
#define EXIT_LIFT_IDX (0xff)     //出口リフトインデックス（特別扱い）

#define LIFT_ON_MAX (5) //リフトに乗れる最大人数（透明人間を含む）

typedef struct SHAKE_WORK_tag
{
  u8 Seq;
  u8 AddCount;
  u8 ShakeCount;
  u8 ShakeMargin;
  fx32 AddVal;
  FIELD_CAMERA *Camera;
  const VecFx32 *Watch;
  VecFx32 OrgOffset;
}SHAKE_WORK;

//ジム内部中の一時ワーク
typedef struct GYM_GROUND_TMP_tag
{
  int TargetLiftIdx;
  fx32 NowHeight;
  fx32 DstHeight;
  fx32 AddVal;
  const VecFx32 *Watch;
  BOOL Exit;
  BOOL LiftMvStop;
  u16 StopTime;
  u16 MainLiftSeq;
  SHAKE_WORK ShakeWork;
  BOOL FogFadeFlg;
  s32 FogBaseOffset;
  s32 FogBaseSlope;
  int WallAnmWatch;
  MMDL *LiftMmdl[LIFT_ON_MAX];
}GYM_GROUND_TMP;

typedef struct FLOOR_RECT_tag
{
  int X;
  int Z;
  int W;
  int H;
  fx32 Height;
}FLOOR_RECT;

typedef struct LIFT_RECT_tag
{
  int X;
  int Z;
  int W;
  int H;
  fx32 Height[2];
}LIFT_RECT;

#define F0_HEIGHT ( HEIGHT0*FIELD_CONST_GRID_FX32_SIZE )
#define F1_HEIGHT ( HEIGHT1*FIELD_CONST_GRID_FX32_SIZE )
#define F2_HEIGHT ( HEIGHT2*FIELD_CONST_GRID_FX32_SIZE )
#define F3_HEIGHT ( HEIGHT3*FIELD_CONST_GRID_FX32_SIZE )
#define F4_HEIGHT ( HEIGHT4*FIELD_CONST_GRID_FX32_SIZE )

#define WALL_HEIGHT ( WALL_Y_POS*FIELD_CONST_GRID_FX32_SIZE )
#define WALL_OPEN_HEIGHT ( WALL_ANM_START_HEIHGT*FIELD_CONST_GRID_FX32_SIZE )

#define F1_1_X  (7)
#define F1_1_Z  (19)
#define F1_1_W  (17)
#define F1_1_H  (1)
#define F1_2_X  (7)
#define F1_2_Z  (23)
#define F1_2_W  (17)
#define F1_2_H  (1)
#define F1_3_X  (13)
#define F1_3_Z  (24)
#define F1_3_W  (5)
#define F1_3_H  (3)

#define F2_1_X  (7)
#define F2_1_Z  (19)
#define F2_1_W  (17)
#define F2_1_H  (1)
#define F2_2_X  (7)
#define F2_2_Z  (20)
#define F2_2_W  (1)
#define F2_2_H  (3)
#define F2_3_X  (7)
#define F2_3_Z  (23)
#define F2_3_W  (17)
#define F2_3_H  (1)

#define F3_1_X  (7)
#define F3_1_Z  (19)
#define F3_1_W  (6)
#define F3_1_H  (1)
#define F3_2_X  (7)
#define F3_2_Z  (23)
#define F3_2_W  (11)
#define F3_2_H  (1)

#define LIFT_WIDTH (5)
#define LIFT_HEIGHT (3)
#define SP_LIFT_WIDTH (3)
#define SP_LIFT_HEIGHT (3)

#define LIFT_1_X  (18)
#define LIFT_1_Z  (24)

#define LIFT_2_X  (18)
#define LIFT_2_Z  (20)

#define LIFT_3_X  (8)
#define LIFT_3_Z  (24)

#define LIFT_4_X  (13)
#define LIFT_4_Z  (24)

#define LIFT_5_X  (8)
#define LIFT_5_Z  (20)

#define LIFT_6_X  (14)
#define LIFT_6_Z  (20)

//配置座標
#define LIFT_0_GX (15*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_0_GY (F1_HEIGHT)
#define LIFT_0_GZ (25*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define LIFT_1_GX (20*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_1_GY (F1_HEIGHT)
#define LIFT_1_GZ (25*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_2_GX (20*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_2_GY (F2_HEIGHT)
#define LIFT_2_GZ (21*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_3_GX (10*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_3_GY (F2_HEIGHT)
#define LIFT_3_GZ (25*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_4_GX (15*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_4_GY (F3_HEIGHT)
#define LIFT_4_GZ (25*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_5_GX (10*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_5_GY (F3_HEIGHT)
#define LIFT_5_GZ (21*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_6_GX (15*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_6_GY (F1_HEIGHT)
#define LIFT_6_GZ (21*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define WALL_GX (15*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define WALL_GY (WALL_HEIGHT)
#define WALL_GZ (24*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

static const FLOOR_RECT FloorRect[FLOOR_RECT_NUM] = {
  {F1_1_X, F1_1_Z, F1_1_W, F1_1_H, F1_HEIGHT},
  {F1_2_X, F1_2_Z, F1_2_W, F1_2_H, F1_HEIGHT},
  {F1_3_X, F1_3_Z, F1_3_W, F1_3_H, F1_HEIGHT},

  {F2_1_X, F2_1_Z, F2_1_W, F2_1_H, F2_HEIGHT},
  {F2_2_X, F2_2_Z, F2_2_W, F2_2_H, F2_HEIGHT},
  {F2_3_X, F2_3_Z, F2_3_W, F2_3_H, F2_HEIGHT},

  {F3_1_X, F3_1_Z, F3_1_W, F3_1_H, F3_HEIGHT},
  {F3_2_X, F3_2_Z, F3_2_W, F3_2_H, F3_HEIGHT},
};

static const LIFT_RECT LiftRect[LIFT_NUM_MAX] = {
  {LIFT_1_X, LIFT_1_Z, LIFT_WIDTH, LIFT_HEIGHT, { F1_HEIGHT, F2_HEIGHT} },
  {LIFT_2_X, LIFT_2_Z, LIFT_WIDTH, LIFT_HEIGHT, { F2_HEIGHT, F1_HEIGHT} },
  {LIFT_3_X, LIFT_3_Z, LIFT_WIDTH, LIFT_HEIGHT, { F2_HEIGHT, F3_HEIGHT} },
  {LIFT_4_X, LIFT_4_Z, LIFT_WIDTH, LIFT_HEIGHT, { F3_HEIGHT, F2_HEIGHT} },
  {LIFT_5_X, LIFT_5_Z, LIFT_WIDTH, LIFT_HEIGHT, { F3_HEIGHT, F1_HEIGHT} },
  {LIFT_6_X, LIFT_6_Z, SP_LIFT_WIDTH, SP_LIFT_HEIGHT, { F1_HEIGHT, F4_HEIGHT} },
};

static const VecFx32 LiftBasePos[LIFT_NUM_MAX] = {
  { LIFT_1_GX, LIFT_1_GY, LIFT_1_GZ },
  { LIFT_2_GX, LIFT_2_GY, LIFT_2_GZ },
  { LIFT_3_GX, LIFT_3_GY, LIFT_3_GZ },
  { LIFT_4_GX, LIFT_4_GY, LIFT_4_GZ },
  { LIFT_5_GX, LIFT_5_GY, LIFT_5_GZ },
  { LIFT_6_GX, LIFT_6_GY, LIFT_6_GZ },
};

//リフトごとの基準位置が上下どちらなのかを管理するリスト 上の場合はTRUE
static const BOOL LiftBaseUP[LIFT_NUM_MAX] = {
  TRUE,
  FALSE,
  TRUE,
  FALSE,
  FALSE,
  TRUE,
};

//リソースの並び順番
enum {
  RES_ID_LIFT_MDL = 0,
  RES_ID_SP_LIFT_MDL,
  RES_ID_WALL_MDL,
  RES_ID_LIFT_ANM_A,
  RES_ID_LIFT_ANM_B,
  RES_ID_SP_LIFT_ANM_A,
  RES_ID_SP_LIFT_ANM_B,
  RES_ID_WALL_ANM,
};

//ＯＢＪインデックス
enum {
  OBJ_LIFT_1 = 0,
  OBJ_LIFT_2,
  OBJ_LIFT_3,
  OBJ_LIFT_4,
  OBJ_LIFT_5,
  OBJ_SP_LIFT,
  OBJ_WALL,
  OBJ_LIFT_0,
};

//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_GROUND, NARC_gym_ground_rif_anim01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_GROUND, NARC_gym_ground_mainrif_anim01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_GROUND, NARC_gym_ground_kakuheki_anim01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD

  { ARCID_GYM_GROUND, NARC_gym_ground_rif_anim01_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_GROUND, NARC_gym_ground_rif_anim02_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_GROUND, NARC_gym_ground_mainrif_anim01_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_GROUND, NARC_gym_ground_mainrif_anim02_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_GROUND, NARC_gym_ground_kakuheki_anim01_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
};

//3Dアニメ　リフト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_lift[] = {
	{ RES_ID_LIFT_ANM_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_LIFT_ANM_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_splift[] = {
	{ RES_ID_SP_LIFT_ANM_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SP_LIFT_ANM_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　隔壁
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_wall[] = {
  { RES_ID_WALL_ANM,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //リフト1
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
  //リフト2
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
  //リフト3
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
  //リフト4
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
  //リフト5
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
  //メインリフト
	{
		RES_ID_SP_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SP_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_splift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_splift),	//アニメリソース数
	},
  //隔壁
	{
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall),	//アニメリソース数
	},
  //リフト0
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

static GMEVENT_RESULT UpDownEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT ExitLiftEvt( GMEVENT* event, int* seq, void* work );
static BOOL CheckArrive(GYM_GROUND_TMP *tmp);
static BOOL CheckLiftHit(u8 inLiftIdx, const int inX, const int inZ);
static void SetupLiftAnm( GYM_GROUND_SV_WORK *gmk_sv_work,
                          FLD_EXP_OBJ_CNT_PTR ptr,
                          const int inLiftIdx);
static int GetWatchLiftAnmIdx(GYM_GROUND_SV_WORK *gmk_sv_work, const int inLiftIdx);
static void FuncMainLiftOnly(GAMESYS_WORK *gsys);
static void SetExitLiftDefaultSt(FLD_EXP_OBJ_CNT_PTR ptr);

static void InitLiftShake(  const u8 inLiftIdx,
                            const fx32 inAddVal,
                            const BOOL inStart,
                            FIELD_CAMERA *camera,
                            SHAKE_WORK *outWork );
static BOOL ShakeLift(SHAKE_WORK *work);

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  EHL_PTR exHeightPtr;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  GYM_GROUND_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  FIELD_FOG_WORK* fog = FIELDMAP_GetFieldFog( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }

  //汎用ワーク確保
  tmp = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_GROUND_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_GROUND_TMP));

  {
    FLDMAPPER * mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
    exHeightPtr = FLDMAPPER_GetExHegihtPtr( mapper );
  }

  //床高さセット
  for(i=0;i<FLOOR_RECT_NUM;i++)
  {
    EXH_SetUpHeightData( i,
              FloorRect[i].X, FloorRect[i].Z, FloorRect[i].W, FloorRect[i].H,
							FloorRect[i].Height,
							exHeightPtr );
  }

  //リフト高さセット
  for(i=0;i<LIFT_NUM_MAX;i++)
  {
    u8 height_idx = FLOOR_RECT_NUM+i;
    u8 idx;
    //セーブデータで分岐
    if ( gmk_sv_work->LiftMoved[i] ) idx = 1;
    else idx = 0;
      
    EXH_SetUpHeightData( height_idx,
              LiftRect[i].X, LiftRect[i].Z, LiftRect[i].W, LiftRect[i].H,
							LiftRect[i].Height[idx],
							exHeightPtr );
  }


  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_GROUND_UNIT_IDX );

  //座標セット
  for (i=0;i<LIFT_NUM_MAX;i++)
  {
    int j;
    int idx = OBJ_LIFT_1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_UNIT_IDX, idx);
    status->trans = LiftBasePos[i];
    //セーブデータで分岐してリフトのＹ座標をセット
    {
      u8 height;
      if ( gmk_sv_work->LiftMoved[i] ) height = 1;
      else height = 0;
      status->trans.y = LiftRect[i].Height[height];
    }

    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_GROUND_UNIT_IDX, idx, TRUE);

    for (j=0;j<LIFT_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }
    
    SetupLiftAnm(gmk_sv_work, ptr, i);
  }

  {
    int j;
    EXP_OBJ_ANM_CNT_PTR anm;
    VecFx32 pos = {LIFT_0_GX, LIFT_0_GY, LIFT_0_GZ};
    int obj_idx = OBJ_LIFT_0;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_UNIT_IDX, obj_idx);
    status->trans = pos;
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_GROUND_UNIT_IDX, obj_idx, TRUE);
    for (j=0;j<LIFT_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }
    SetExitLiftDefaultSt(ptr);
  }
  
  //隔壁座標セット
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    VecFx32 wall_pos = {WALL_GX, WALL_GY, WALL_GZ};
    int obj_idx = OBJ_WALL;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_UNIT_IDX, obj_idx);
    status->trans = wall_pos;
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_GROUND_UNIT_IDX, obj_idx, TRUE);

    //アニメ有効
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE);
    
    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //既に隔壁開いていたら、最終フレームをセット
    if ( gmk_sv_work->WallOpen ){
      fx32 last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
      //ラストフレーム
      FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0, last );
    }
  }

  //デフォルトフォグ設置を保存
  {
    FIELD_ZONEFOGLIGHT * zone_fog = FIELDMAP_GetFieldZoneFog( fieldWork );
    tmp->FogBaseOffset = FIELD_ZONEFOGLIGHT_GetOffset( zone_fog );
    tmp->FogBaseSlope = FIELD_ZONEFOGLIGHT_GetSlope( zone_fog );
    NOZOMU_Printf("fog::%d,%d\n",tmp->FogBaseOffset, tmp->FogBaseSlope);
  }
  //自機が隔壁より下にいる場合はフォグフェード後の設定に書き換え
  {
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    if (pos.y <= WALL_HEIGHT)
    {
      FIELD_FOG_SetOffset( fog, FOG_OFFSET );
      FIELD_FOG_SetSlope( fog, FOG_SLOPE );
    }
  }
  
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
void GYM_GROUND_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_GROUND_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }
#ifdef PM_DEBUG
  //フィールドの残りヒープをプリント
  if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("size = %x\n",size);
  }
  
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GMEVENT *event;
      {
        u8 i;
        s16 gx,gz;
        const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
        gx = MMDL_GetGridPosX( fmmdl );
        gz = MMDL_GetGridPosZ( fmmdl );
        for (i=0;i<LIFT_NUM_MAX;i++){
          if ( CheckLiftHit(i, gx, gz) ) break;
        }
        if (i != LIFT_NUM_MAX)
        {
          event = GYM_GROUND_CreateLiftMoveEvt(gsys, i);
          GAMESYSTEM_SetEvent(gsys, event);
        }
      }
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GMEVENT *event;
      event = GYM_GROUND_CreateExitLiftMoveEvt(gsys, TRUE);
      GAMESYSTEM_SetEvent(gsys, event);
    }
  }
  else if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L)
  {
    //自機の高さを変更
    {
      VecFx32 pos;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      pos.y -=  10*FIELD_CONST_GRID_FX32_SIZE;
      FIELD_PLAYER_SetPos( fld_player, &pos );
    }
  }
#endif  //PM_DEBUG

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * 昇降イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inLiftIdx   リフトインデックス
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_GROUND_CreateLiftMoveEvt(GAMESYS_WORK *gsys, const int inLiftIdx)
{
  fx32 speed;
  GMEVENT * event;
  GYM_GROUND_TMP *tmp;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  }

  if ( inLiftIdx >= LIFT_NUM_MAX ){
    GF_ASSERT_MSG(0, "LIFT_INDEX_OVER %d",inLiftIdx);
    return NULL;
  }

  tmp->TargetLiftIdx = inLiftIdx;
  //対象リフトの状態を取得
  if ( gmk_sv_work->LiftMoved[ inLiftIdx ] )
  {
    //移動後位置→基準位置
    tmp->NowHeight = LiftRect[inLiftIdx].Height[1];
    tmp->DstHeight = LiftRect[inLiftIdx].Height[0];
  }
  else
  {
    //基準位置→移動後位置
    tmp->NowHeight = LiftRect[inLiftIdx].Height[0];
    tmp->DstHeight = LiftRect[inLiftIdx].Height[1];
  }

  if (inLiftIdx == SP_LIFT_IDX){
    speed = SP_LIFT_MOVE_SPD1;
    //フォグフェードリクエストする
    tmp->FogFadeFlg = TRUE;
  }else{
    speed = LIFT_MOVE_SPD;
    //フォグフェードリクエストしない
    tmp->FogFadeFlg = FALSE;
  }
  if ( tmp->DstHeight - tmp->NowHeight < 0 ) tmp->AddVal = -speed;
  else tmp->AddVal = speed;

  
  //隔壁アニメ監視フラグオフ
  tmp->WallAnmWatch = 0;

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, UpDownEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 出口昇降イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inExit      ジムから出るとき TRUE
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_GROUND_CreateExitLiftMoveEvt(GAMESYS_WORK *gsys, const BOOL inExit)
{
  GMEVENT * event;
  GYM_GROUND_TMP *tmp;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  }

  tmp->Exit = inExit;

  //対象リフトの状態を取得
  if ( inExit )
  {
    //基準位置→移動後位置
    tmp->NowHeight = F1_HEIGHT;
    tmp->DstHeight = F0_HEIGHT;
  }
  else
  {
    //移動後位置→基準位置
    tmp->NowHeight = F0_HEIGHT;
    tmp->DstHeight = F1_HEIGHT;
    {
      int obj_idx = OBJ_LIFT_0;
      FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, 1, TRUE);
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0, FALSE);
    }
  }
  if ( tmp->DstHeight - tmp->NowHeight < 0 ) tmp->AddVal = -EXIT_LIFT_MOVE_SPD;
  else tmp->AddVal = EXIT_LIFT_MOVE_SPD;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ExitLiftEvt, 0 );
  return event;
}

//--------------------------------------------------------------
/**
 * 昇降イベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT UpDownEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_GROUND_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }

  switch(*seq){
  case 0:
    //フィールドＯＢＪ全検索
    {
      VecFx32 player_pos;
      int i;
      int count = 0;
      u32 no = 0;
      MMDL *mmdl;
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );

      for (i=0;i<LIFT_ON_MAX;i++)
      {
        tmp->LiftMmdl[i] = NULL;
      }
      {
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        FIELD_PLAYER_GetPos( fld_player, &player_pos );
      }
      while( MMDLSYS_SearchUseMMdl(mmdlsys, &mmdl, &no) == TRUE )
      {
        //グリッド座標を取得
        s16 x = MMDL_GetGridPosX( mmdl );
        s16 z = MMDL_GetGridPosZ( mmdl );
        //内外判定
        if ( CheckLiftHit(tmp->TargetLiftIdx, x, z) )
        {
          VecFx32 pos;
          MMDL_GetVectorPos( mmdl, &pos );

          //自機と同じ高さのＯＢＪをエントリ（多少のずれを考慮し上下1グリッドの猶予をもつことにする）
          if ( (player_pos.y-FIELD_CONST_GRID_FX32_SIZE < pos.y) &&
              (pos.y < player_pos.y+FIELD_CONST_GRID_FX32_SIZE)  )
          {
            if ( count >= LIFT_ON_MAX ){
              GF_ASSERT(0);
            }else{
              tmp->LiftMmdl[count++] = mmdl;
            }
          }
        }
      }
    }
    InitLiftShake(tmp->TargetLiftIdx, tmp->AddVal, TRUE, camera, &tmp->ShakeWork);
    (*seq)++;
    break;
  case 1:
    {
      SHAKE_WORK *shake = &tmp->ShakeWork;
      if ( ShakeLift(shake) ) (*seq)++;
    }
    break;
  case 2:
    //カメラトレースを切る
    FIELD_CAMERA_StopTraceRequest(camera);
    (*seq)++;
    break;
  case 3:
    {
      //カメラトレースが生きている間は処理を進めない
      if ( FIELD_CAMERA_CheckTrace(camera) ) break;
      PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE);
      //トレース終了
      (*seq)++;
    }
    //NO BREAK
  case 4:
    //変動後の高さに書き換え
    if (!tmp->LiftMvStop){
      tmp->NowHeight += tmp->AddVal;
    }
    //目的の高さに到達したか？
    if(  CheckArrive(tmp) )
    {
      //目的高さで上書き
      tmp->NowHeight = tmp->DstHeight;
      //リフト振動セットアップ
      InitLiftShake(tmp->TargetLiftIdx, tmp->AddVal, FALSE, camera, &tmp->ShakeWork);
      //リフト停止音
      PMSND_PlaySE(GYM_GROUND_SE_LIFT_STOP);
      //次のシーケンスへ
      (*seq)++;
    }

    //対象リフト高さ更新
    {
      int obj_idx = OBJ_LIFT_1 + tmp->TargetLiftIdx;
      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_UNIT_IDX, obj_idx);
      status->trans.y = tmp->NowHeight;
    }

    {
      int i;
      for (i=0;i<LIFT_ON_MAX;i++){
        if ( tmp->LiftMmdl[i] != NULL){
          //高さ書き換え
          MMDL *mmdl;
          const VecFx32 *mmdl_pos;
          VecFx32 pos;
          int gy;
          mmdl = tmp->LiftMmdl[i];
          //座標更新
          mmdl_pos = MMDL_GetVectorPosAddress( mmdl );
          pos = *mmdl_pos;
          pos.y = tmp->NowHeight;
          gy = SIZE_GRID_FX32( pos.y );
          MMDL_SetGridPosY( mmdl, gy );
          MMDL_SetVectorPos( mmdl, &pos );
        }
      }
    }
#if 0    
    //フィールドＯＢＪ全検索
    {
      u32 no = 0;
      MMDL *mmdl;
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
      while( MMDLSYS_SearchUseMMdl(mmdlsys, &mmdl, &no) == TRUE )
      {
        //グリッド座標を取得
        s16 x = MMDL_GetGridPosX( mmdl );
        s16 z = MMDL_GetGridPosZ( mmdl );
        //内外判定
        if ( CheckLiftHit(tmp->TargetLiftIdx, x, z) )
        {
          const VecFx32 *mmdl_pos;
          VecFx32 pos;
          int gy;
          //座標更新
          mmdl_pos = MMDL_GetVectorPosAddress( mmdl );
          pos = *mmdl_pos;
          pos.y = tmp->NowHeight;
          gy = SIZE_GRID_FX32( pos.y );
          MMDL_SetGridPosY( mmdl, gy );
          MMDL_SetVectorPos( mmdl, &pos );
        }
      }
    }
#endif    
    break;
  case 5:
    {
      SHAKE_WORK *shake = &tmp->ShakeWork;
      if ( ShakeLift(shake) ) (*seq)++;
    }
    break;
  case 6:
    //カメラトレース再開
    FIELD_CAMERA_RestartTrace(camera);
    //アニメ開始
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      int obj_idx;
      int anm_idx;
      obj_idx = OBJ_LIFT_1+tmp->TargetLiftIdx;
      anm_idx = GetWatchLiftAnmIdx(gmk_sv_work, tmp->TargetLiftIdx);
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx );
      //アニメ停止解除
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    }
    (*seq)++;
    break;
  case 7:
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      int obj_idx;
      int anm_idx;
      obj_idx = OBJ_LIFT_1+tmp->TargetLiftIdx;
      anm_idx = GetWatchLiftAnmIdx(gmk_sv_work, tmp->TargetLiftIdx);
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx );
      //アニメ待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        u8 idx;
        //対象リフトのギミックデータ書き換え
        if ( gmk_sv_work->LiftMoved[ tmp->TargetLiftIdx ] )
        {
          gmk_sv_work->LiftMoved[ tmp->TargetLiftIdx ] = FALSE;
          idx = 0;
        }
        else
        {
          gmk_sv_work->LiftMoved[ tmp->TargetLiftIdx ] = TRUE;
          idx = 1;
        }
        //対象リフトの拡張高さを書き換え
        {
          u8 height_idx;
          EHL_PTR exHeightPtr;
          {
            FLDMAPPER * mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
            exHeightPtr = FLDMAPPER_GetExHegihtPtr( mapper );
          }
          height_idx = FLOOR_RECT_NUM + tmp->TargetLiftIdx;
          EXH_SetHeight( height_idx,
              					 LiftRect[tmp->TargetLiftIdx].Height[idx],
                         exHeightPtr );
        }

        //次回に備え、アニメ切り替え
        SetupLiftAnm(gmk_sv_work, ptr, tmp->TargetLiftIdx);
        //OBJのポーズを解除
        ;
        //メインリフトのときのみフォグフェード待ちシーケンスへ移行
        if (tmp->TargetLiftIdx != SP_LIFT_IDX) return GMEVENT_RES_FINISH;
        else (*seq)++;
      }
    }
    break;
  case 8:
    {
      FIELD_FOG_WORK* fog = FIELDMAP_GetFieldFog( fieldWork );
      //メインリフトのときのみフォグフェード待ち
      if ( FIELD_FOG_FADE_IsFade( fog ) ) break;    //フォグフェード中

      return GMEVENT_RES_FINISH;
    }
  }

  //メインリフトのときのみの処理
  FuncMainLiftOnly( gsys );

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 出口昇降イベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ExitLiftEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_GROUND_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }

  switch(*seq){
  case 0:
    {
      //現在ウォッチターゲットを保存
      tmp->Watch = FIELD_CAMERA_GetWatchTarget(camera);
      //カメラのバインドを切る
      FIELD_CAMERA_FreeTarget(camera);
    }
    //リフト移動ＳＥ
    if ( !tmp->Exit ) PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE_IN);
    else PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE);
    (*seq)++;
    break;
  case 1:
    {
      //変動後の高さに書き換え
      tmp->NowHeight += tmp->AddVal;
      //目的の高さに到達したか？
      if(  CheckArrive(tmp) )
      {
        //目的高さで上書き
        tmp->NowHeight = tmp->DstHeight;
        //リフト振動セットアップ
        if (!tmp->Exit)
        {
          InitLiftShake(EXIT_LIFT_IDX, tmp->AddVal, FALSE, camera, &tmp->ShakeWork);
          //リフト停止ＳＥ
          PMSND_PlaySE(GYM_GROUND_SE_LIFT_STOP);
        }
        //次のシーケンスへ
        (*seq)++;
      }
      //対象リフト高さ更新
      {
        int obj_idx = OBJ_LIFT_0;
        GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_UNIT_IDX, obj_idx);
        status->trans.y = tmp->NowHeight;
      }
      //自機の高さを更新
      {
        VecFx32 pos;
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        FIELD_PLAYER_GetPos( fld_player, &pos );
        pos.y = tmp->NowHeight;
        FIELD_PLAYER_SetPos( fld_player, &pos );
      }
    }
    break;
  case 2:
    if (tmp->Exit) return GMEVENT_RES_FINISH;
    else
    {
      //振動終わるまで処理をフック
      {
        SHAKE_WORK *shake = &tmp->ShakeWork;
        if ( !ShakeLift(shake) ) break;
      }

      //カメラを再バインド
      if ( (tmp->Watch != NULL) ){
        FIELD_CAMERA_BindTarget(camera, tmp->Watch);
      }
      //アニメ開始
      {
        EXP_OBJ_ANM_CNT_PTR anm;
        int obj_idx;
        int anm_idx;
        obj_idx = OBJ_LIFT_0;
        anm_idx = 1;
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx );
        //アニメ停止解除
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      }
      //次のシーケンスへ
      (*seq)++;
    }
    break;
  case 3:
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      int obj_idx;
      int anm_idx;
      obj_idx = OBJ_LIFT_0;
      anm_idx = 1;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx );
      //アニメ待ち
      if( FLD_EXP_OBJ_ChkAnmEnd(anm) ){
        SetExitLiftDefaultSt(ptr);
        return GMEVENT_RES_FINISH;
      }
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * リフト到達チェック
 * @param	  GYM_GROUND_TMP *tmp
 * @return  BOOL      TRUEで到達
 */
//--------------------------------------------------------------
static BOOL CheckArrive(GYM_GROUND_TMP *tmp)
{
  if (tmp->AddVal < 0){
    if ( tmp->DstHeight >= tmp->NowHeight ) return TRUE;
  }else{
    if ( tmp->DstHeight <= tmp->NowHeight ) return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * リフト矩形内チェック
 * @param	  inLiftIdx リフトインデックス
 * @param   inX       対象Ｘグリッド座標
 * @param   inZ       対象Zグリッド座標
 * @return  BOOL      TRUEで矩形内
 */
//--------------------------------------------------------------
static BOOL CheckLiftHit(u8 inLiftIdx, const int inX, const int inZ)
{
  int x = LiftRect[inLiftIdx].X;
  int z = LiftRect[inLiftIdx].Z;
  int width = LiftRect[inLiftIdx].W;
  int height = LiftRect[inLiftIdx].H;

  if ( (x<=inX)&&(inX<=x+width-1)&&
       (z<=inZ)&&(inZ<=z+height-1) ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * リフトアニメ準備
 * @param   gmk_sv_work   ギミックセーブワーク
 * @param   ptr       拡張ＯＢＪコントロールポインタ
 * @param	  inLiftIdx リフトインデックス
 * @return  none
 */
//--------------------------------------------------------------
static void SetupLiftAnm(GYM_GROUND_SV_WORK *gmk_sv_work, FLD_EXP_OBJ_CNT_PTR ptr, const int inLiftIdx)
{
  EXP_OBJ_ANM_CNT_PTR anm;
  int valid_anm_idx, invalid_anm_idx;
  int obj_idx;

  obj_idx = OBJ_LIFT_1 + inLiftIdx;
  if ( gmk_sv_work->LiftMoved[inLiftIdx] )
  {
    if ( LiftBaseUP[inLiftIdx] )
    {
      //下→上アニメ有効
      valid_anm_idx = 0;
      invalid_anm_idx = 1;
    }
    else
    {
      //上→下アニメ有効
      valid_anm_idx = 1;
      invalid_anm_idx = 0;
    }
  }
  else
  {
    if ( LiftBaseUP[inLiftIdx] )
    {
      //上→下アニメ有効
      valid_anm_idx = 1;
      invalid_anm_idx = 0;
    }
    else
    {
      //下→上アニメ有効
      valid_anm_idx = 0;
      invalid_anm_idx = 1;
    }
  }
  
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, valid_anm_idx, TRUE);
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, invalid_anm_idx, FALSE);
  //頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_GROUND_UNIT_IDX, obj_idx, valid_anm_idx, 0 );
  //停止にしておく
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, valid_anm_idx);
  FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
}

//--------------------------------------------------------------
/**
 * 終了監視するリフトアニメインデックスを取得
 * @param   gmk_sv_work   ギミックセーブワーク
 * @param	  inLiftIdx リフトインデックス
 * @return  int     終了監視するアニメインデックス　0～1
 */
//--------------------------------------------------------------
static int GetWatchLiftAnmIdx(GYM_GROUND_SV_WORK *gmk_sv_work, const int inLiftIdx)
{
  int valid_anm_idx;

  if ( gmk_sv_work->LiftMoved[inLiftIdx] )
  {
    if ( LiftBaseUP[inLiftIdx] )
    {
      //下→上アニメ有効
      valid_anm_idx = 0;
    }
    else
    {
      //上→下アニメ有効
      valid_anm_idx = 1;
    }
  }
  else
  {
    if ( LiftBaseUP[inLiftIdx] )
    {
      //上→下アニメ有効
      valid_anm_idx = 1;
    }
    else
    {
      //下→上アニメ有効
      valid_anm_idx = 0;
    }
  }

  return valid_anm_idx;
}

//--------------------------------------------------------------
/**
 * メインリフトのみの処理
 * @param   gsys      ゲームシステムポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void FuncMainLiftOnly(GAMESYS_WORK *gsys)
{
  GYM_GROUND_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }
  
  //メインリフトのみの処理
  if (tmp->TargetLiftIdx != SP_LIFT_IDX) return;
  
  //リフトの高さが隔壁オープン開始する高さに達したか？
  if (!gmk_sv_work->WallOpen && tmp->NowHeight<=WALL_OPEN_HEIGHT){
    EXP_OBJ_ANM_CNT_PTR anm;
    int obj_idx = OBJ_WALL;
    gmk_sv_work->WallOpen = TRUE;
    {
      //隔壁アニメーション開始
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0);
      //アニメ開始
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      //隔壁開く音
      PMSND_PlaySE(GYM_GROUND_SE_WALL_OPEN);
    }
    tmp->NowHeight = WALL_OPEN_HEIGHT;
    tmp->LiftMvStop = TRUE;
    tmp->StopTime = 0;
    tmp->MainLiftSeq = 0;

    //隔壁アニメ終了監視フラグオン
    tmp->WallAnmWatch = 1;
  }

  if ( tmp->LiftMvStop ){
    switch(tmp->MainLiftSeq){
    case 0:
      InitLiftShake(tmp->TargetLiftIdx, tmp->AddVal, FALSE, camera, &tmp->ShakeWork);
      //リフト停止ＳＥ
      PMSND_PlaySE(GYM_GROUND_SE_LIFT_STOP);
      tmp->MainLiftSeq++;
      break;
    case 1:
      {
        SHAKE_WORK *shake = &tmp->ShakeWork;
        if ( ShakeLift(shake) )
        {
          //停止タイマーカウント
          tmp->StopTime++;
          if (tmp->StopTime >= WALL_ANM_WAIT)
          {
            tmp->AddVal = -SP_LIFT_MOVE_SPD2;   //スピード変更
            tmp->LiftMvStop = FALSE;            //ストップ解除
            //リフト移動開始再開
            PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE);
          }
        }
      }
    }   //end switch
  }

  //フォグ操作
  if (tmp->FogFadeFlg){
    //フォグフェード開始位置に到達したか？
    BOOL fog_start = FALSE;
    s32 fog_offset, fog_slope;

    //進行方向で分岐
    if ( tmp->AddVal>=0 )   //上昇
    {
      if (tmp->NowHeight >= UP_FOG_FADE_START*FIELD_CONST_GRID_FX32_SIZE)
      {
        fog_start = TRUE;
        fog_offset = tmp->FogBaseOffset;
        fog_slope = tmp->FogBaseSlope;
      }
    }
    else                    //下降
    {
      if (tmp->NowHeight <= DOWN_FOG_FADE_START*FIELD_CONST_GRID_FX32_SIZE)
      {
        fog_start = TRUE;
        fog_offset = FOG_OFFSET;
        fog_slope = FOG_SLOPE;
      }
    }

    if (fog_start)
    {
      u16 fog_sync;
      FIELD_FOG_WORK* fog = FIELDMAP_GetFieldFog( fieldWork );
      //リフトの移動速度で分岐
      if (tmp->AddVal == -SP_LIFT_MOVE_SPD2) fog_sync = FOG_FADE_SPEED_SLOW;
      else fog_sync = FOG_FADE_SPEED_FAST;

      FIELD_FOG_FADE_Init( fog, fog_offset, fog_slope, fog_sync );
      tmp->FogFadeFlg = FALSE;
    }
  }

  //隔壁アニメ監視
  if (tmp->WallAnmWatch)
  {
    if ( tmp->WallAnmWatch == 1 ) tmp->WallAnmWatch = 2;    //フラグを立てたシンクは処理しない
    else
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      int obj_idx = OBJ_WALL;
      //隔壁アニメーション終了監視
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0);
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        PMSND_StopSE_byPlayerID( PMSND_GetSE_DefaultPlayerID( GYM_GROUND_SE_WALL_OPEN ) );
        //隔壁アニメ監視フラグオフ
        tmp->WallAnmWatch = 0;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 出口リフトアニメ初期状態セット
 * @param	  FLD_EXP_OBJ_CNT_PTR ptr   拡張ＯＢＪコントローラポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void SetExitLiftDefaultSt(FLD_EXP_OBJ_CNT_PTR ptr)
{
  EXP_OBJ_ANM_CNT_PTR anm;
  int obj_idx;
  int anm_idx;
  obj_idx = OBJ_LIFT_0;
  anm_idx = 0;

  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE);
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, 1, FALSE);
  //頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx, 0 );
  //停止にしておく
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx);
  FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
}

//--------------------------------------------------------------
/**
 * リフト振動初期化
 * @param	  inLiftIdx   リフトインデックス
 * @param   inAddVal       昇降速度
 * @param   inStart     起動時 OR 到着時 TRUEで起動時
 * @param   camera      フィールドカメラポインタ
 * @param   outWork     格納バッファ
 * @return  none
 */
//--------------------------------------------------------------
static void InitLiftShake(  const u8 inLiftIdx,
                            const fx32 inAddVal,
                            const BOOL inStart,
                            FIELD_CAMERA *camera,
                            SHAKE_WORK *outWork )
{
  SHAKE_WORK *work;

  work = outWork;
  work->Seq = 0;
  work->AddCount = 0;
  work->Camera = camera;
  //現在ウォッチターゲットを保存
  work->Watch = FIELD_CAMERA_GetWatchTarget(camera);
  //復帰オフセット値保存
  FIELD_CAMERA_GetTargetOffset( camera, &work->OrgOffset );

  if (inStart)
  {
    work->AddVal = LIFT_START_SHAKE_VAL*FX32_ONE;
    work->ShakeCount = LIFT_START_SHAKE_COUNT;
  }
  else
  {
    //リフト別に振動幅分岐
    if (inLiftIdx == SP_LIFT_IDX)
    {
      work->AddVal = SP_LIFT_SHAKE_VAL*FX32_ONE;
      work->ShakeCount = SP_LIFT_SHAKE_COUNT;
    }
    else if ( inLiftIdx == EXIT_LIFT_IDX )
    {
      work->AddVal = LIFT_SHAKE_VAL*FX32_ONE;
      work->ShakeCount = LIFT_SHAKE_COUNT;
    }
    else
    {
      work->AddVal = LIFT_SHAKE_VAL*FX32_ONE;
      work->ShakeCount = LIFT_SHAKE_COUNT;
    }
  }
  //進行方向で加算値変更
  if (inAddVal < 0) work->AddVal *= -1;
}

//--------------------------------------------------------------
/**
 * リフト振動関数
 * @param	  work    振動ワークポインタ
 * @return  BOOL    TRUEで振動終了
 */
//--------------------------------------------------------------
static BOOL ShakeLift(SHAKE_WORK *work)
{
  switch(work->Seq){
  case 0:
    //カメラのバインドを切る
    FIELD_CAMERA_FreeTarget(work->Camera);
    
    work->Seq++;
    //none break
  case 1:
    if (work->ShakeMargin % SHAKE_MARGIN == 0)
    {
      //値加算
      {
        VecFx32 offset;
        FIELD_CAMERA_GetTargetOffset( work->Camera, &offset );
        offset.y += work->AddVal;
        FIELD_CAMERA_SetTargetOffset( work->Camera, &offset );
      }
      //値逆転
      work->AddVal *= -1;
      //加算回数インクリメント
      work->AddCount++;
      //終了判定
      if ( work->AddCount >= work->ShakeCount*2 ){
        //カメラを再バインド
        if ( (work->Watch != NULL) ) FIELD_CAMERA_BindTarget(work->Camera, work->Watch);

        //オフセット戻し
        FIELD_CAMERA_SetTargetOffset( work->Camera, &work->OrgOffset );

        return TRUE;
      }
    }
    work->ShakeMargin = (work->ShakeMargin+1)%SHAKE_MARGIN;
  }

  return FALSE;
}

