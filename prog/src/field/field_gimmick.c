//============================================================================================
/**
 * @file	field_gimmick.c
 * @brief	マップ固有の仕掛けを発動するかどうかを判定するソース
 * @author	saito
 * @date	2006.02.09
 *
 */
//============================================================================================

#include "include/savedata/gimmickwork.h"
#include "field_gimmick_def.h"
#include "field_gimmick.h"
#include "fieldmap.h"

extern void GYM_SetupTest(FIELDMAP_WORK *fieldWork);
extern void GYM_EndTest(FIELDMAP_WORK *fieldWork);
extern void GYM_MoveTest(FIELDMAP_WORK *fieldWork);

const static FLD_GMK_SETUP_FUNC FldGimmickSetupFunc[FLD_GIMMICK_MAX] = {
	NULL,					//0:無し
  GYM_SetupTest,
/* 
	GYM_SetupGhostGym,		//1:ゴーストジム
	GYM_SetupCombatGym,		//2:格闘ジム
	GYM_SetupElecGym,		//3:電気ジム		カントー
	GYM_SetupSkyGym,		//4:飛行ジム
	GYM_SetupInsectGym,		//5:虫ジム
	GYM_SetupDragonGym,		//6:ドラゴンジム
	GYM_SetupPoisonGym,		//7:毒ジム			カントー
	GYM_SetupAlmGym,		//8:万能ジム		カントー
	ArceusGmk_Setup,		//9:シント遺跡ギミック
*/
};

const static FLD_GMK_END_FUNC FldGimmickEndFunc[FLD_GIMMICK_MAX] = {
	NULL,					//0:無し
  GYM_EndTest,
/*
	GYM_EndGhostGym,		//1:ゴーストジム
	GYM_EndCombatGym,		//2:格闘ジム
	GYM_EndElecGym,			//3:電気ジム		カントー
	NULL,					//4:飛行ジム
	GYM_EndInsectGym,		//5:虫ジム
	GYM_EndDragonGym,		//6:ドラゴンジム
	GYM_EndPoisonGym,		//7:毒ジム			カントー
	GYM_EndAlmGym,			//8:万能ジム		カントー
	ArceusGmk_End,			//9:シント遺跡ギミック
*/
};

const static FLD_GMK_END_FUNC FldGimmickMoveFunc[FLD_GIMMICK_MAX] = {
	NULL,					//0:無し
  GYM_MoveTest,
/*
	GYM_EndGhostGym,		//1:ゴーストジム
	GYM_EndCombatGym,		//2:格闘ジム
	GYM_EndElecGym,			//3:電気ジム		カントー
	NULL,					//4:飛行ジム
	GYM_EndInsectGym,		//5:虫ジム
	GYM_EndDragonGym,		//6:ドラゴンジム
	GYM_EndPoisonGym,		//7:毒ジム			カントー
	GYM_EndAlmGym,			//8:万能ジム		カントー
	ArceusGmk_End,			//9:シント遺跡ギミック
*/
};


const static FLD_GMK_HIT_CHECK FldGimmickHitCheck[FLD_GIMMICK_MAX] = {
	NULL,					//0:無し
/*
	NULL,					//1:ゴーストジム
	NULL,					//2:格闘ジム
	NULL,					//3:電気ジム		カントー
	NULL,					//4:飛行ジム
	NULL,					//5:虫ジム
	GYM_HitCheckDragonGym,	//6:ドラゴンジム
	NULL,					//7:毒ジム			カントー
	NULL,					//8:万能ジム		カントー
	NULL,					//9:シント遺跡ギミック
*/
};

//---------------------------------------------------------------------------
/**
 * @brief	ギミックのセットアップ関数
 * 
 * @param	fieldWork	フィールドワークポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void FLDGMK_SetUpFieldGimmick(FIELDMAP_WORK *fieldWork)
{
	int id;
  GIMMICKWORK *work;

  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv= GAMEDATA_GetSaveControlWork(gamedata);
	
	//ギミックワーク取得
	work = SaveData_GetGimmickWork(sv);
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == FLD_GIMMICK_NONE){
		return;					//ギミック無し
	}
	//ギミックセットアップ
	FldGimmickSetupFunc[id](fieldWork);
}

//---------------------------------------------------------------------------
/**
 * @brief	ギミックの終了関数
 * 
 * @param	fieldWork	フィールドワークポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void FLDGMK_EndFieldGimmick(FIELDMAP_WORK *fieldWork)
{
	int id;
	GIMMICKWORK *work;

  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv= GAMEDATA_GetSaveControlWork(gamedata);
	
  //ギミックワーク取得
	work = SaveData_GetGimmickWork(sv);
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == FLD_GIMMICK_NONE){
		return;					//ギミック無し
	}
	//ギミック終了
	if (FldGimmickEndFunc[id] != NULL){
		FldGimmickEndFunc[id](fieldWork);
	}
}

//---------------------------------------------------------------------------
/**
 * @brief	ギミックの動作関数
 * 
 * @param	fieldWork	フィールドワークポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void FLDGMK_MoveFieldGimmick(FIELDMAP_WORK *fieldWork)
{
  int id;
  GIMMICKWORK *work;

  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv= GAMEDATA_GetSaveControlWork(gamedata);
	
	//ギミックワーク取得
	work = SaveData_GetGimmickWork(sv);
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == FLD_GIMMICK_NONE){
		return;					//ギミック無し
	}

  if (FldGimmickMoveFunc[id] != NULL){
    //ギミックセットアップ
	  FldGimmickMoveFunc[id](fieldWork);
  }
}


#if 0
//---------------------------------------------------------------------------
/**
 * @brief	ギミック用特殊当たり判定処理
 * 
 * @param	fsys	フィールドシステムポインタ
 * @param	inGridX		グリッドＸ座標
 * @param	inGirdZ		グリッドＺ座標
 * @param	inHeight	現在高さ
 * @param	outHit		TRUE:通常当たり判定を行なわない	FALSE：通常当たり判定を行う
 * 
 * @return	BOOL		TRUE:HIT		FALSE:NOHIT
 */
//---------------------------------------------------------------------------
BOOL FLDGMK_FieldGimmickHitCheck(	FIELDSYS_WORK *fsys,
									const int inGridX, const int inGridZ,
									const fx32 inHeight,
									BOOL *outHit)
{
	int id;
	GIMMICKWORK *work;
	//ギミックワーク取得
	work = SaveData_GetGimmickWork(GameSystem_GetSaveData(fsys));
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == FLD_GIMMICK_NONE){
		return FALSE;					//ギミック無し
	}
	//ギミック終了
	if (FldGimmickHitCheck[id] != NULL){
		BOOL rc;
		rc = FldGimmickHitCheck[id](fsys, inGridX, inGridZ, inHeight, outHit);
		return rc;
	}
	
	return FALSE;						//処理無し
}
#endif
