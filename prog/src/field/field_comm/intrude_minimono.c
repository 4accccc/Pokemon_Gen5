//==============================================================================
/**
 * @file    intrude_minimono.c
 * @brief   ミニモノリス処理  ※fieldオーバーレイに配置
 * @author  matsuda
 * @date    2009.10.25(日)
 */
//==============================================================================
#include <gflib.h>
#include "field/fieldmap.h"
#include "field/fldmmdl.h"
#include "intrude_minimono.h"
#include "field/field_const.h"
#include "intrude_work.h"
#include "intrude_main.h"
#include "../../../resource/fldmapdata/script/plc04_def.h"  //SCRID_～
#include "../../../resource/fldmapdata/script/palace02_def.h"  //SCRID_～


//--------------------------------------------------------------
/// ミニモノリス動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER data_MMdlHeader =
{
	100,	          ///<識別ID  fldmmdl_code.h
	                //  スクリプト上でX番の動作モデルをアニメ、といった判別に使用。
	                //  evファイル上で被らなければよい。
	BLACKMONOLITH,	///<表示するOBJコード fldmmdl_code.h
	MV_DOWN,	      ///<動作コード    
	EV_TYPE_NORMAL,	///<イベントタイプ
	0,	            ///<イベントフラグ  タイプ毎に使用されるフラグ。外側から取得する
	SCRID_PLC04_MINIMONO,///<イベントID 話しかけた時に起動するスクリプトID(*.ev の _EVENT_DATAの番号)
	DIR_DOWN,	      ///<指定方向  グリッド移動ならばDIR系 ノングリッドの場合は違う値
	0,	            ///<指定パラメタ 0  タイプ毎に使用されるパラメタ。外側から取得する
	0,            	///<指定パラメタ 1
	0,	            ///<指定パラメタ 2
	MOVE_LIMIT_NOT,	///<X方向移動制限
	MOVE_LIMIT_NOT,	///<Z方向移動制限
  MMDL_HEADER_POSTYPE_GRID,
  {0},            ///<ポジションバッファ ※check　グリッドマップの場合はここはMMDL_HEADER_GRIDPOS
};


//==================================================================
/**
 * ミニモノリスの動作モデルをAdd
 *
 * @param   fieldWork		
 * @param   grid_x		グリッド座標X
 * @param   grid_z		グリッド座標Z
 * @param   fx_y		  Y座標(fx32)
 */
//==================================================================
void MINIMONO_AddMMdl(FIELDMAP_WORK *fieldWork, u16 grid_x, u16 grid_z, fx32 fx_y)
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  
  mmdl_sys = FIELDMAP_GetMMdlSys( fieldWork );
  zone_id = FIELDMAP_GetZoneID( fieldWork );
  
  head = data_MMdlHeader;
  if(GFUser_GetPublicRand(2) == 0){
    head.obj_code = WHITEMONOLITH;
  }
  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
  OS_TPrintf("minimono add\n");
}

//==================================================================
/**
 * ミニモノリスの動作モデルを位置ランダムでAdd
 *
 * @param   fieldWork		
 */
//==================================================================
void MINIMONO_AddPosRand(GAME_COMM_SYS_PTR game_comm, FIELDMAP_WORK *fieldWork)
{
  VecFx32 vec;
  int town_tblno;
  
  town_tblno = Intrude_GetWarpTown(game_comm);
  if(town_tblno == PALACE_TOWN_DATA_NULL || town_tblno == PALACE_TOWN_DATA_PALACE){
    return;
  }
  
  Intrude_GetPalaceTownRandPos(town_tblno, &vec);
  vec.z -= GRID_TO_FX32( 1 );  //自機のワープ場所と被らないように1grid上にする
  MINIMONO_AddMMdl(fieldWork, FX32_TO_GRID( vec.x ), FX32_TO_GRID( vec.z ), vec.y);
}

#ifdef PM_DEBUG
//==================================================================
/**
 * ミニモノリスの動作モデルをAdd
 *
 * @param   fieldWork		
 * @param   grid_x		グリッド座標X
 * @param   grid_z		グリッド座標Z
 * @param   fx_y		  Y座標(fx32)
 */
//==================================================================
static void DEBUG_INTRUDE_Pokemon_AddMMdl(FIELDMAP_WORK *fieldWork, u16 grid_x, u16 grid_z, fx32 fx_y, u32 monsno)
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  
  mmdl_sys = FIELDMAP_GetMMdlSys( fieldWork );
  zone_id = FIELDMAP_GetZoneID( fieldWork );
  
  head = data_MMdlHeader;
  head.obj_code = TPOKE_0001 + monsno;
  head.event_id = SCRID_PALACE02_SYMBOL_POKE;
  head.move_code = GFUser_GetPublicRand0(2) == 0 ? MV_RND_V : MV_RND_H;
  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
}

//==================================================================
/**
 * 
 *
 * @param   fieldWork		
 */
//==================================================================
void DEBUG_INTRUDE_Pokemon_Add(FIELDMAP_WORK *fieldWork)
{
  static u16 gx = 20;
  static u16 gz = 30;
  
  DEBUG_INTRUDE_Pokemon_AddMMdl(fieldWork, gx, gz, 0, GFUser_GetPublicRand0(MONSNO_ARUSEUSU) + 1);
  gx += 1;
  if(gx % 10 == 0){
    gz += 2;
    gx -= 10;
  }
}

void DEBUG_INTRUDE_BingoPokeSet(FIELDMAP_WORK *fieldWork)
{
  u32 gx = 18, gz = 17;
  u32 gx_space = 6, gz_space = 4;
  int x, y;
  int rnd_x, rnd_z;
  
  for(y = 0; y < 4; y++){
    for(x = 0; x < 4; x++){
      rnd_x = GFUser_GetPublicRand0(2);
      rnd_z = GFUser_GetPublicRand0(2);
      DEBUG_INTRUDE_Pokemon_AddMMdl(fieldWork, gx + gx_space*x + rnd_x, gz + gz_space*y + rnd_z, 0,
        GFUser_GetPublicRand0(MONSNO_ARUSEUSU) + 1);
    }
  }
}

#endif  //-------- PM_DEBUG --------

