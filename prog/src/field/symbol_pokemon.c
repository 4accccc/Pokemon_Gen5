//==============================================================================
/**
 * @file    symbol_pokemon.c
 * @brief   シンボルポケモン配置処理
 * @author  matsuda --> tamada
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
#include "field/tpoke_data.h"


//==============================================================================
//==============================================================================
///シンボルポケモンのレベル
#define SYMBOL_POKE_LEVEL       (30)


typedef struct {
  TPOKE_DATA * tpdata;
  FIELDMAP_WORK * fieldmap;
  u32 start_no;
}POKEADD_WORK;

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
/// 動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER data_MMdlHeader =
{
	0,	          ///<識別ID  fldmmdl_code.h
	                //  スクリプト上でX番の動作モデルをアニメ、といった判別に使用。
	                //  evファイル上で被らなければよい。
	TPOKE_0001,	///<表示するOBJコード fldmmdl_code.h
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


//==============================================================================
//==============================================================================

//==================================================================
/**
 * @brief
 *
 * @param   fieldmap		
 * @param   start_no  シンボルポケモン配置ナンバーの開始Index
 * @param   sympoke   生成元SYMBOL_POKEMONデータ
 * @param   id        obj_id
 * @param   grid_x		グリッド座標X
 * @param   grid_z		グリッド座標Z
 * @param   fx_y		  Y座標(fx32)
 */
//==================================================================
static void SYMBOLPOKE_AdMMdl(
    const POKEADD_WORK * padd,
    const SYMBOL_POKEMON * sympoke, u16 id, u16 grid_x, u16 grid_z, fx32 fx_y )
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  
  mmdl_sys = FIELDMAP_GetMMdlSys( padd->fieldmap );
  zone_id = FIELDMAP_GetZoneID( padd->fieldmap );
  
  head = data_MMdlHeader;
  head.id = id;
  head.obj_code = TPOKE_DATA_GetObjCode( padd->tpdata,
      sympoke->monsno, sympoke->sex, sympoke->form_no );
  head.move_code = GFUser_GetPublicRand0(2) == 0 ? MV_RND_V : MV_RND_H;
  //動作モデルのパラメータにシンボルポケモンの情報を埋め込んでいる
  head.param0 = padd->start_no + id;
  head.param1 = *((u32*)sympoke) & 0xffff;
  head.param2 = *((u32*)sympoke) >> 16;
  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモン：パラメータの取得
 * @param sympoke パラメータを受け取るポインタ
 * @param mmdl  対象となるポケモン動作モデルへのポインタ
 */
//--------------------------------------------------------------
void SYMBOLPOKE_GetParam( SYMBOL_POKEMON * sympoke, const MMDL * mmdl )
{
  u16 param1 = MMDL_GetParam( mmdl, MMDL_PARAM_1 );
  u16 param2 = MMDL_GetParam( mmdl, MMDL_PARAM_2 );
  *( (u32*)sympoke ) = ( ((u32)param1) | ((u32)param2 << 16) );
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモン：配置ナンバーの取得
 * @param   mmdl  対象となるポケモン動作モデルへのポインタ
 * @return  u32   シンボルポケモンのナンバー
 * SYMBOL_POKE_MAX以上の場合、通信相手のポケモン
 */
//--------------------------------------------------------------
u32 SYMBOLPOKE_GetSymbolNo( const MMDL * mmdl )
{
  return MMDL_GetParam( mmdl, MMDL_PARAM_0 );
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモンパラメータからポケモン生成
 * @param heapID    使用するヒープの指定
 * @param gamedata  ゲームデータ（MyStatus参照用）
 * @param sympoke   生成するシンボルポケモンのパラメータ
 * @return  POKEMON_PARAM 生成したポケモンデータ
 */
//--------------------------------------------------------------
POKEMON_PARAM * SYMBOLPOKE_PP_Create(
    HEAPID heapID, GAMEDATA * gamedata, const SYMBOL_POKEMON * sympoke )
{
  POKEMON_PARAM * pp;
  u32 personal_rnd;
  u32 oya_id;
  oya_id = MyStatus_GetID( GAMEDATA_GetMyStatus( gamedata ) );
  personal_rnd = POKETOOL_CalcPersonalRandEx(
      oya_id, sympoke->monsno, sympoke->form_no, sympoke->sex, 0, FALSE );

  pp = PP_Create( sympoke->monsno, SYMBOL_POKE_LEVEL, PTL_SETUP_ID_AUTO, heapID );
  PP_SetupEx( pp, sympoke->monsno, SYMBOL_POKE_LEVEL, oya_id, PTL_SETUP_ID_AUTO, personal_rnd );
  PP_SetTokusei3( pp, sympoke->monsno, sympoke->form_no );
  if ( sympoke->wazano )
  {
    PP_Put( pp, ID_PARA_waza1, sympoke->wazano );
  }
  return pp;
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモンの動作モデルを配置する
 * @param fieldmap
 * @param start_no  配置ナンバーの開始
 * @param sympoke   配置するシンボルポケモンのデータ配列
 * @param poke_num  配置するシンボルポケモンのデータ数
 */
//--------------------------------------------------------------
void SYMBOLPOKE_Add(
    FIELDMAP_WORK *fieldmap, u32 start_no, const SYMBOL_POKEMON * sympoke, int poke_num )
{
  enum {
    gx = 9,
    gz = 12,
    gx_space = 3,
    gz_space = 1
  };

  POKEADD_WORK padd;
  int x, y;
  int rnd_x, rnd_z;
  int idx = 0;
  
  padd.tpdata = TPOKE_DATA_Create( FIELDMAP_GetHeapID(fieldmap) );
  padd.fieldmap = fieldmap;
  padd.start_no = start_no;

  for(y = 0; y < 7; y++){
    for(x = 0; x < 4; x++){
      if ( sympoke[idx].monsno != 0 )
      {
        rnd_x = GFUser_GetPublicRand0(2);
        rnd_z = GFUser_GetPublicRand0(2);
        SYMBOLPOKE_AdMMdl(&padd, &sympoke[idx], idx,
            gx + gx_space*x + rnd_x, gz + gz_space*y + rnd_z, 0 );
        poke_num --;
      }
      idx ++;
      if (idx >= SYMBOL_MAP_STOCK_MAX ) goto END;
    }
  }
END:

  TPOKE_DATA_Delete( padd.tpdata );
}



