#include "fieldmap.h"
#include "arc/fieldmap/buildmodel_outdoor.naix"

#define TEST_UNIT_IDX (1)

//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	//1P
	{ ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_trailer_01_nsbmd, GFL_G3D_UTIL_RESARC },
///	{ ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_h01_ship_nsbmd, GFL_G3D_UTIL_RESARC },
};


//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	//1P
	{
		0, 			//モデルリソースID
		0, 							//モデルデータID(リソース内部INDEX)
		0, 			//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
/*  
  {
		1, 			//モデルリソースID
		0, 							//モデルデータID(リソース内部INDEX)
		1, 			//テクスチャリソースID
		NULL,			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
*/	
};


static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

void GYM_SetupTest(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr;
  ptr = FIELDMAP_GetExpObjCntPtr(fieldWork);

  FLD_EXP_OBJ_AddUnit(ptr, &Setup, TEST_UNIT_IDX );
}

void GYM_EndTest(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr;
  ptr = FIELDMAP_GetExpObjCntPtr(fieldWork);
  FLD_EXP_OBJ_DelUnit( ptr, TEST_UNIT_IDX );
}

void GYM_MoveTest(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr;
  VecFx32 pos;

  ptr = FIELDMAP_GetExpObjCntPtr(fieldWork);

	// プレイヤーの位置判定
	FIELD_PLAYER_GetPos( FIELDMAP_GetFieldPlayer( fieldWork ), &pos );
  //座標セット
  {
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, TEST_UNIT_IDX, 0);
    status->trans = pos;
  }

  //アニメーションの再生
  ;
}
