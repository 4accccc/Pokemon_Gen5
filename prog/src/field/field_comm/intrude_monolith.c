//==============================================================================
/**
 * @file    intrude_monolith.c
 * @brief   モノリス処理  ※fieldオーバーレイに配置
 * @author  matsuda
 * @date    2009.10.29(木)
 */
//==============================================================================
#include <gflib.h>
#include "field/fieldmap.h"
#include "field/fldmmdl.h"
#include "intrude_minimono.h"
#include "field/field_const.h"
#include "intrude_work.h"
#include "intrude_main.h"
#include "intrude_field.h"
#include "intrude_monolith.h"
#include "field/zonedata.h"
#include "../../../resource/fldmapdata/script/palace01_def.h"  //SCRID_～


//--------------------------------------------------------------
/// モノリス動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER data_MMdlHeader =
{
	99, 	          ///<識別ID  fldmmdl_code.h
	                //  スクリプト上でX番の動作モデルをアニメ、といった判別に使用。
	                //  evファイル上で被らなければよい。
	NONDRAW,	      ///<表示するOBJコード fldmmdl_code.h
	MV_DOWN,	      ///<動作コード    
	EV_TYPE_NORMAL,	///<イベントタイプ
	0,	            ///<イベントフラグ  タイプ毎に使用されるフラグ。外側から取得する
	SCRID_PALACE01_MONOLITH,///<イベントID 話しかけた時に起動するスクリプトID(*.ev の _EVENT_DATAの番号)
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
 * モノリスの動作モデルをAdd
 *
 * @param   fieldWork		
 * @param   grid_x		グリッド座標X
 * @param   grid_z		グリッド座標Z
 * @param   fx_y		  Y座標(fx32)
 */
//==================================================================
void MONOLITH_AddMMdl(FIELDMAP_WORK *fieldWork, u16 grid_x, u16 grid_z, fx32 fx_y)
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  const MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  
  mmdl_sys = FIELDMAP_GetMMdlSys( fieldWork );
  zone_id = FIELDMAP_GetZoneID( fieldWork );
  
  head = data_MMdlHeader;
  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
  OS_TPrintf("monolith add\n");
}

//==================================================================
/**
 * モノリスの動作モデルを位置ランダムでAdd
 *
 * @param   fieldWork		
 */
//==================================================================
void MONOLITH_AddConnectAllMap(FIELDMAP_WORK *fieldWork)
{
  VecFx32 vec;
  int i, net_id;
  
  if(ZONEDATA_IsPalace( FIELDMAP_GetZoneID( fieldWork ) ) == FALSE){
    return;
  }
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    for(i = 0; i < PALACE_MAP_MONOLITH_NUM; i++){
      vec.x = PALACE_MAP_MONOLITH_X + FIELD_CONST_GRID_FX32_SIZE * i + PALACE_MAP_LEN * net_id;//(net_id+1);
      vec.y = PALACE_MAP_MONOLITH_Y;
      vec.z = PALACE_MAP_MONOLITH_Z;
      MONOLITH_AddMMdl(fieldWork, FX32_TO_GRID( vec.x ), FX32_TO_GRID( vec.z ), vec.y);
    }
  }
  OS_TPrintf("モノリス登録\n");
}
