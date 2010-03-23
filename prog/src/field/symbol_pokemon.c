//==============================================================================
/**
 * @file    symbol_pokemon.c
 * @brief   シンボルポケモン配置処理
 * @author  matsuda
 * @date    2009.10.25(日)
 *
 * 2010.03.21 tamada  ミニモノリス用を変更
 *
 * 動作モデルのパラメータにシンボルポケモンの情報を埋め込んでいる
 */
//==============================================================================
#include <gflib.h>
#include "field/fieldmap.h"
#include "field/fldmmdl.h"
#include "savedata/mystatus.h"

#include "field/field_const.h"
//#include "intrude_work.h"
//#include "intrude_main.h"
#include "../../../resource/fldmapdata/script/symbol_encount_scr_def.h"

#include "symbol_pokemon.h"

#include "waza_tool/wazano_def.h"  //WAZANO_

///シンボルポケモンのレベル
#define SYMBOL_POKE_LEVEL       (30)

//--------------------------------------------------------------
/// ミニモノリス動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER data_MMdlHeader =
{
	0,	          ///<識別ID  fldmmdl_code.h
	                //  スクリプト上でX番の動作モデルをアニメ、といった判別に使用。
	                //  evファイル上で被らなければよい。
	BLACKMONOLITH,	///<表示するOBJコード fldmmdl_code.h
	MV_DOWN,	      ///<動作コード    
	EV_TYPE_NORMAL,	///<イベントタイプ
	0,	            ///<イベントフラグ  タイプ毎に使用されるフラグ。外側から取得する
	SCRID_SYMBOL_POKEMON,///<イベントID 話しかけた時に起動するスクリプトID(*.ev の _EVENT_DATAの番号)
	DIR_DOWN,	      ///<指定方向  グリッド移動ならばDIR系 ノングリッドの場合は違う値
	0,	            ///<指定パラメタ 0  タイプ毎に使用されるパラメタ。外側から取得する
	0,            	///<指定パラメタ 1
	0,	            ///<指定パラメタ 2
	MOVE_LIMIT_NOT,	///<X方向移動制限
	MOVE_LIMIT_NOT,	///<Z方向移動制限
  MMDL_HEADER_POSTYPE_GRID,
  {0},            ///<ポジションバッファ ※check　グリッドマップの場合はここはMMDL_HEADER_GRIDPOS
};


#ifdef PM_DEBUG
//==================================================================
/**
 * ミニモノリスの動作モデルをAdd
 *
 * @param   fieldmap		
 * @param   grid_x		グリッド座標X
 * @param   grid_z		グリッド座標Z
 * @param   fx_y		  Y座標(fx32)
 */
//==================================================================
static void SYMBOLPOKE_AdMMdl(
    FIELDMAP_WORK *fieldmap, const SYMBOL_POKEMON * sympoke, u16 id, u16 grid_x, u16 grid_z, fx32 fx_y )
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  
  mmdl_sys = FIELDMAP_GetMMdlSys( fieldmap );
  zone_id = FIELDMAP_GetZoneID( fieldmap );
  
  head = data_MMdlHeader;
  head.id = id;
  head.obj_code = TPOKE_0001 + sympoke->monsno;
  head.move_code = GFUser_GetPublicRand0(2) == 0 ? MV_RND_V : MV_RND_H;
  //動作モデルのパラメータにシンボルポケモンの情報を埋め込んでいる
  head.param0 = sympoke->monsno;
  head.param1 = sympoke->wazano;
  head.param2 = (sympoke->form_no << 2 ) || sympoke->sex;
  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MMDL * getPokeMMdl( FIELDMAP_WORK * fieldmap, u16 obj_id )
{
  MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  GF_ASSERT( mmdl != NULL );
  return mmdl;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 getMonsno( MMDL * poke )
{
  if ( poke )
  {
    return MMDL_GetParam( poke, MMDL_PARAM_0 );
  }
  return 0;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 getFormno( MMDL * poke )
{
  if ( poke )
  {
    u16 para = MMDL_GetParam( poke, MMDL_PARAM_2 );
    return ( para >> 2 );
  }
  return 0;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 getSex( MMDL * poke )
{
  if ( poke )
  {
    u16 para = MMDL_GetParam( poke, MMDL_PARAM_2 );
    return ( para & 0x3 );
  }
  return PTL_SEX_MALE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 getWaza( MMDL * poke )
{
  if ( poke )
  {
    return MMDL_GetParam( poke, MMDL_PARAM_1 );
  }
  return WAZANO_HATAKU;  //エラー対処、とりあえず
}

//--------------------------------------------------------------
//--------------------------------------------------------------
u16 SYMBOLPOKE_GetMonsno( FIELDMAP_WORK * fieldmap, u16 obj_id )
{
  MMDL * poke = getPokeMMdl( fieldmap, obj_id );
  return getMonsno( poke );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
u16 SYMBOLPOKE_GetFormno( FIELDMAP_WORK * fieldmap, u16 obj_id )
{
  MMDL * poke = getPokeMMdl( fieldmap, obj_id );
  return getFormno( poke );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
u16 SYMBOLPOKE_GetSex( FIELDMAP_WORK * fieldmap, u16 obj_id )
{
  MMDL * poke = getPokeMMdl( fieldmap, obj_id );
  return getSex( poke );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
u16 SYMBOLPOKE_GetWaza( FIELDMAP_WORK * fieldmap, u16 obj_id )
{
  MMDL * poke = getPokeMMdl( fieldmap, obj_id );
  return getWaza( poke );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static POKEMON_PARAM * SYMBOLPOKE_PP_Create(
    HEAPID heapID, u32 oya_id, u16 monsno, u16 formno, int sex, u16 wazano )
{
  POKEMON_PARAM * pp;
  u32 personal_rnd;
  personal_rnd = POKETOOL_CalcPersonalRandEx( oya_id, monsno, formno, sex, 0, FALSE );

  pp = PP_Create( monsno, SYMBOL_POKE_LEVEL, PTL_SETUP_ID_AUTO, heapID );
  PP_SetupEx( pp, monsno, SYMBOL_POKE_LEVEL, oya_id, PTL_SETUP_ID_AUTO, personal_rnd );
  PP_SetTokusei3( pp, monsno, formno );
  if ( wazano )
  {
    PP_Put( pp, ID_PARA_waza1, wazano );
  }
  return pp;
}

//--------------------------------------------------------------
/**
 * @brief
 * @param heapID    使用するヒープ指定
 * @param fieldmap
 * @param obj_id    対象となるポケモンのOBJID
 */
//--------------------------------------------------------------
POKEMON_PARAM * SYMBOLPOKE_PP_CreateByObjID( HEAPID heapID, GAMESYS_WORK * gsys, u16 obj_id )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  MMDL * poke = getPokeMMdl( fieldmap, obj_id );
  if ( poke )
  {
    POKEMON_PARAM * pp;
    u32 personal_rnd, id;
    u16 monsno, formno, sex, wazano;
    monsno = getMonsno( poke );
    formno = getFormno( poke );
    sex = getSex( poke );
    wazano = getWaza( poke );
    id = MyStatus_GetID( GAMEDATA_GetMyStatus( gamedata ) );
    return SYMBOLPOKE_PP_Create( heapID, id, monsno, formno, sex, wazano );
  }
  return NULL;
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモンの動作モデルを配置する
 * @param fieldmap
 * @param sympoke   配置するシンボルポケモンのデータ配列
 * @param poke_num  配置するシンボルポケモンのデータ数
 */
//--------------------------------------------------------------
void SYMBOLPOKE_Add(FIELDMAP_WORK *fieldmap, const SYMBOL_POKEMON * sympoke, int poke_num )
{
  u32 gx = 9, gz = 12;
  u32 gx_space = 3, gz_space = 1;
  int x, y;
  int rnd_x, rnd_z;
  int idx = 0;
  
  for(y = 0; y < 7; y++){
    for(x = 0; x < 4; x++){
      if (idx < poke_num )
      {
        rnd_x = GFUser_GetPublicRand0(2);
        rnd_z = GFUser_GetPublicRand0(2);
        SYMBOLPOKE_AdMMdl(fieldmap, &sympoke[idx], idx,
            gx + gx_space*x + rnd_x, gz + gz_space*y + rnd_z, 0 );
        idx ++;
      }
    }
  }
}
#if 0
//==================================================================
/**
 * 
 *
 * @param   fieldmap		
 */
//==================================================================
void DEBUG_INTRUDE_Pokemon_Add(FIELDMAP_WORK *fieldmap)
{
  static u16 gx = 9, gz = 12;
  static x_count = 0, y_count = 0;
  u32 gx_space = 3, gz_space = 1;
  int rnd_x, rnd_z;
  int monsno;
  
  //monsno = GFUser_GetPublicRand0(MONSNO_ARUSEUSU) + 1;
  if((GFL_UI_KEY_GetCont() & PAD_BUTTON_R)){
    monsno = TPOKE_0348;
  }
  else{
    monsno = TPOKE_0006 + GFUser_GetPublicRand0(200);
  }
  
  rnd_x = GFUser_GetPublicRand0(2);
  rnd_z = GFUser_GetPublicRand0(2);
  SYMBOLPOKE_AdMMdl(fieldmap, gx + gx_space*x_count + rnd_x, 
    gz + gz_space*y_count + rnd_z, 0,
    monsno);
  x_count++;
  if(x_count >= 4){
    x_count = 0;
    y_count++;
  }
}
#endif


#endif  //-------- PM_DEBUG --------

