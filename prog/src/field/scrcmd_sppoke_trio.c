//======================================================================
/**
 * @file  scrcmd_sppoke_trio.c
 * @brief  ダルタニス技覚え関連スクリプト関数
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"
#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_sppoke_trio.h"

#include "poke_tool/poke_memo.h"
#include "waza_tool/wazano_def.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "fldmmdl.h"  //for MMDLSYS

#include "../../../resource/fldmapdata/gimmick/merodhia/gmk_timing_def.h"

#define ROUND_RANGE (MERODHIA_ROTATE_RANGE*FIELD_CONST_GRID_FX32_SIZE)
#define ROUND_OUT_SYNC  (MERODHIA_ROUND_OUT_SYNC)  //拡散に必要なシンク数
#define ROUND_OUT_NUM   (MERODHIA_ROUND_OUT_NUM)   //回転回数

#define ROUND_SYNC  (MERODHIA_ROUND_SYNC)  //大回り回転に必要なシンク数
#define ROUND_NUM   (MERODHIA_ROUND_NUM)   //回転回数

#define ROUND_IN_SYNC  (MERODHIA_ROUND_IN_SYNC)  //収縮に必要なシンク数
#define ROUND_IN_NUM   (MERODHIA_ROUND_IN_NUM)   //回転回数

typedef struct OBJ_RISE_WORK_tag
{
  fx32 Height;
  u16 Sync;
  u16 NowSync;
  MMDL *TrgtMmdl;
  VecFx32 BasePos;
}OBJ_RISE_WORK;

typedef struct OBJ_ROTATE_WORK_tag
{
  u32 NowSync;
  MMDL *TrgtMmdl;
  VecFx32 BasePos;
}OBJ_ROTATE_WORK;


static GMEVENT_RESULT ObjRotateEvt(GMEVENT * event, int * seq, void * work);
static GMEVENT_RESULT ObjRiseEvt(GMEVENT * event, int * seq, void * work);

//--------------------------------------------------------------
/**
 * 指定ＯＢＪを指定シンクかけて指定値分上昇させる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 * @note    条件を満たすダルタニスがいない場合は retにFALSEがセットされる
 * @note　　条件は配布されたダルタニスでかつ、固有技を覚えていないこと
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdRiseObj( VMHANDLE *core, void *wk )
{
  GMEVENT* event;
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  u16 y_half_grid = SCRCMD_GetVMWorkValue( core, work );
 	u16 sync = SCRCMD_GetVMWorkValue( core, work );
  OBJ_RISE_WORK *evt_work;

  event = GMEVENT_Create(gsys, NULL, ObjRiseEvt, sizeof(OBJ_RISE_WORK));
  evt_work = GMEVENT_GetEventWork(event);
	evt_work->Height = y_half_grid * (FIELD_CONST_GRID_FX32_SIZE/2);
  evt_work->Sync = sync;
  evt_work->NowSync = 0;
  //基点座標取得
  {
    FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
    MMDLSYS *fos = FIELDMAP_GetMMdlSys( fieldmap );
    MMDL *mmdl = MMDLSYS_SearchOBJID( fos, obj_id );
    MMDL_GetVectorPos( mmdl, &evt_work->BasePos );
    evt_work->TrgtMmdl = mmdl;
    // 影を消す
    MMDL_OnMoveBit( mmdl, MMDL_MOVEBIT_SHADOW_VANISH );
  }
  
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 指定ＯＢＪを指定シンクかけて上昇させるイベント
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ObjRiseEvt(GMEVENT * event, int * seq, void * work)
{
	OBJ_RISE_WORK *evt_work = work;

  //座標更新
  evt_work->NowSync++;
  {
    VecFx32 vec;
    vec = evt_work->BasePos;
    vec.y += (evt_work->Height * evt_work->NowSync) / evt_work->Sync;
    MMDL_SetVectorPos( evt_work->TrgtMmdl, &vec );
  }
  
  if ( evt_work->NowSync >= evt_work->Sync )
  {   //終了
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 指定ＯＢＪをイベント回転させる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 * @note    条件を満たすダルタニスがいない場合は retにFALSEがセットされる
 * @note　　条件は配布されたダルタニスでかつ、固有技を覚えていないこと
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEventRotateObj( VMHANDLE *core, void *wk )
{
  GMEVENT* event;
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  OBJ_ROTATE_WORK *evt_work;

  event = GMEVENT_Create(gsys, NULL, ObjRotateEvt, sizeof(OBJ_ROTATE_WORK));
  evt_work = GMEVENT_GetEventWork(event);
  evt_work->NowSync = 0;
  //基点座標取得
  {
    FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
    MMDLSYS *fos = FIELDMAP_GetMMdlSys( fieldmap );
    MMDL *mmdl = MMDLSYS_SearchOBJID( fos, obj_id );
    MMDL_GetVectorPos( mmdl, &evt_work->BasePos );
    evt_work->TrgtMmdl = mmdl;
  }
  
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 指定ＯＢＪを指定シンクかけて回転させるイベント
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ObjRotateEvt(GMEVENT * event, int * seq, void * work)
{
  VecFx32 vec;
	OBJ_ROTATE_WORK *evt_work = work;

  vec = evt_work->BasePos;
  //座標更新
  evt_work->NowSync++;
  switch(*seq){
  case 0:
    {
      fx32 rad;
      fx32 r;
      fx32 x, z;
    
      rad = (0x10000*ROUND_OUT_NUM*evt_work->NowSync) / ROUND_OUT_SYNC;
      if (rad >= 0x10000) rad = rad % 0x10000;
      r = (ROUND_RANGE*evt_work->NowSync) / ROUND_OUT_SYNC;
      x = FX_Mul( FX_CosIdx(rad), r );
      z = FX_Mul( FX_SinIdx(rad), r );

      vec.x += x;
      vec.z += z;
      MMDL_SetVectorPos( evt_work->TrgtMmdl, &vec );

      if ( evt_work->NowSync >= ROUND_OUT_SYNC )
      {
        evt_work->NowSync = 0;
        (*seq) = 1;
      }
    }
    break;
  case 1:
    {
      fx32 rad;
      fx32 r;
      fx32 x, z;
    
      rad = (0x10000*ROUND_NUM*evt_work->NowSync) / ROUND_SYNC;
      if (rad >= 0x10000) rad = rad % 0x10000;
      r = ROUND_RANGE;
      x = FX_Mul( FX_CosIdx(rad), r );
      z = FX_Mul( FX_SinIdx(rad), r );

      vec.x += x;
      vec.z += z;
      MMDL_SetVectorPos( evt_work->TrgtMmdl, &vec );

      if ( evt_work->NowSync >= ROUND_SYNC )
      {
        evt_work->NowSync = 0;
        (*seq) = 2;
      }
    }
    break;
  case 2:
    {
      fx32 rad;
      fx32 r;
      fx32 x, z;
    
      rad = (0x10000*ROUND_IN_NUM*evt_work->NowSync) / ROUND_IN_SYNC;
      if (rad >= 0x10000) rad = rad % 0x10000;
      r = (ROUND_RANGE*(ROUND_IN_SYNC-evt_work->NowSync)) / ROUND_IN_SYNC;
      x = FX_Mul( FX_CosIdx(rad), r );
      z = FX_Mul( FX_SinIdx(rad), r );
      vec.x += x;
      vec.z += z;
      MMDL_SetVectorPos( evt_work->TrgtMmdl, &vec );

      if ( evt_work->NowSync >= ROUND_IN_SYNC )
      {
        //終了
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }
  
	return GMEVENT_RES_CONTINUE;
}


