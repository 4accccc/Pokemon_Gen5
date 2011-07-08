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
#include "field_gimmick_h01.h"
#include "field_gimmick_h03.h"
#include "field_gimmick_gate.h"
#include "gym_elec.h"
#include "gym_normal.h"
#include "gym_anti.h"
#include "gym_fly.h"
#include "gym_insect.h"
#include "gym_ground.h"
#include "gym_ground_ent.h"
#include "gym_ice.h"
#include "gym_dragon.h"
#include "field_gimmick_d06.h"
#include "field_gimmick_league_front01.h"
#include "field_gimmick_league_front02.h"
#include "field_gimmick_bgate_jet.h"
#include "field_gimmick_bsubway.h"
#include "field_gimmick_r04d03.h"
#include "sp_poke_gimmick.h"
#include "palace_gimmick.h"
#include "palace_map_gimmick.h"
#include "field_gimmick_d12.h"
#include "legend_gmk.h"
#include "field_gimmick_t01.h"
#include "resonance_resort/resort_reception.h"
#include "resonance_resort/resort_field.h"
#include "infinity_dungeon_gimmick.h"
#include "wbt_gimmick.h"
#include "gym_poison.h"
#include "gym_water.h"
#include "fourkings_ghost.h"
#include "fourkings_evil.h"
#include "fourkings_fight.h"
#include "fourkings_esp.h"
#include "field_gimmick_hide_ship_engine.h"
#include "field_gimmick_bgate.h"

FS_EXTERN_OVERLAY(field_gym_insect);
FS_EXTERN_OVERLAY(field_gym_elec);
FS_EXTERN_OVERLAY(field_gym_normal);
FS_EXTERN_OVERLAY(field_gym_fly);
FS_EXTERN_OVERLAY(field_gym_anti);
FS_EXTERN_OVERLAY(field_gimmick_bgate);
FS_EXTERN_OVERLAY(field_gym_ground);
FS_EXTERN_OVERLAY(field_gym_ground_ent);
FS_EXTERN_OVERLAY(field_gym_ice);
FS_EXTERN_OVERLAY(field_gym_dragon);
FS_EXTERN_OVERLAY(field_gimmick_gate);
FS_EXTERN_OVERLAY(field_gimmick_lfront01);
FS_EXTERN_OVERLAY(field_gimmick_lfront02);
FS_EXTERN_OVERLAY(field_gimmick_h01);
FS_EXTERN_OVERLAY(field_gimmick_h03);
FS_EXTERN_OVERLAY(field_gimmick_r04d03);
FS_EXTERN_OVERLAY(sppoke_gimmick);
FS_EXTERN_OVERLAY(palace_gimmick);
FS_EXTERN_OVERLAY(palace_map_gimmick);
FS_EXTERN_OVERLAY(field_gimmick_d12);
FS_EXTERN_OVERLAY(field_gimmick_d06);
FS_EXTERN_OVERLAY(field_gimmick_bsubway);
FS_EXTERN_OVERLAY(legend_gimmick);
FS_EXTERN_OVERLAY(field_gimmick_t01);
FS_EXTERN_OVERLAY(resonance_resort_gimmick);
FS_EXTERN_OVERLAY(infinity_dungeon_gimmick);
FS_EXTERN_OVERLAY(wbt_gimmick);
FS_EXTERN_OVERLAY(field_gym_poison);
FS_EXTERN_OVERLAY(field_gym_water);
FS_EXTERN_OVERLAY(field_fourkings_ghost);
FS_EXTERN_OVERLAY(field_fourkings_evil);
FS_EXTERN_OVERLAY(field_fourkings_fight);
FS_EXTERN_OVERLAY(field_fourkings_esp);
FS_EXTERN_OVERLAY(field_gimmick_hide_ship_engine);


#include "field_gimmick_table.cdat"


//定義シンボル数とテーブル数が違った場合はAssert
SDK_COMPILER_ASSERT( NELEMS(FieldGimmickTable) == FLD_GIMMICK_MAX );


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
	
	//ギミックワーク取得
	work = GAMEDATA_GetGimmickWork(gamedata);
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == GIMMICK_NO_ASSIGN){
		return;					//ギミック無し
	}

  //オーバーレイロード
  if ( FieldGimmickTable[id].ov_id != NULL )
  {
    GFL_OVERLAY_Load( FieldGimmickTable[id].ov_id );
  }
	//ギミックセットアップ
	FieldGimmickTable[id].setup_func(fieldWork);
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
	
  //ギミックワーク取得
	work = GAMEDATA_GetGimmickWork(gamedata);;
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == GIMMICK_NO_ASSIGN){
		return;					//ギミック無し
	}

	//ギミック終了
	if (FieldGimmickTable[id].end_func != NULL){
		FieldGimmickTable[id].end_func(fieldWork);
	}
  //オーバーレイアンロード
  if ( FieldGimmickTable[id].ov_id != NULL )
  {
    GFL_OVERLAY_Unload( FieldGimmickTable[id].ov_id );
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
	
	//ギミックワーク取得
	work = GAMEDATA_GetGimmickWork(gamedata);
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);
	
	if (id == GIMMICK_NO_ASSIGN){
		return;					//ギミック無し
	}

  if (FieldGimmickTable[id].move_func != NULL){
    //ギミックセットアップ
	  FieldGimmickTable[id].move_func(fieldWork);
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
	
	if (id == GIMMICK_NO_ASSIGN){
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

//--------------------------------------------------------------
/**
 * @brief   ギミックコードが指定IDのコードと同一かチェック
 *
 * @param	fieldWork	フィールドワークポインタ
 * @param   gimmick_id		ギミックコード
 *
 * @retval  TRUE:一致。　FALSE:不一致
 *
 */
//--------------------------------------------------------------
BOOL FLDGMK_GimmickCodeCheck(FIELDMAP_WORK *fieldWork, int gimmick_id)
{
  int id;
  GIMMICKWORK *work;

  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
	
	//ギミックワーク取得
	work = GAMEDATA_GetGimmickWork(gamedata);
	//ギミックコードを取得
	id = GIMMICKWORK_GetAssignID(work);

	if(gimmick_id == id){
		return TRUE;
	}
	return FALSE;
}
