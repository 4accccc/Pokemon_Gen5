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

#include "symbol_map.h"
#include "symbol_pokemon.h"
#include "field/tpoke_data.h"

#include "symbol_lv_tbl.naix"


//==============================================================================
//==============================================================================
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
  {0},            ///<ポジションバッファ
};


//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  u16 move_code;
  u8 limx, limz;
}SYMPOKE_MV_TABLE;

//--------------------------------------------------------------
//--------------------------------------------------------------
static const SYMPOKE_MV_TABLE symPokeMoveCode[] = {
#if 1
  { MV_DIR_RND, 0, 0 }, // 固定（４方向ランダム）
  { MV_RND,     1, 1 }, // ランダム移動（狭い）
  { MV_RND,     2, 2 }, // ランダム移動（広い）
  { MV_RND_V,   0, 2 }, // 上下移動
  { MV_RND_H,   2, 0 }, // 左右移動
  { MV_RND,     2, 0 }, // 左右移動（キョロキョロ）
  { MV_SPIN_R,  0, 0 }, // 右回転移動
  { MV_SPIN_L,  0, 0 }, // 左回転移動
#else
  { MV_DOWN, 0, 0 }, // 固定（４方向ランダム）
  { MV_DOWN,     1, 1 }, // ランダム移動（狭い）
  { MV_DOWN,     2, 2 }, // ランダム移動（広い）
  { MV_DOWN,   0, 2 }, // 上下移動
  { MV_DOWN,   2, 0 }, // 左右移動
  { MV_DOWN,     2, 0 }, // 左右移動（キョロキョロ）
  { MV_DOWN,  0, 0 }, // 右回転移動
  { MV_DOWN,  0, 0 }, // 左回転移動
#endif
};

//--------------------------------------------------------------
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
//--------------------------------------------------------------
static void SYMBOLPOKE_AdMMdl(
    const POKEADD_WORK * padd,
    const SYMBOL_POKEMON * sympoke, u16 id, u16 grid_x, u16 grid_z, fx32 fx_y )
{
  MMDL_HEADER head;
  MMDL_HEADER_GRIDPOS *grid_pos;
  MMDLSYS *mmdl_sys;
  ZONEID zone_id;
  const SYMPOKE_MV_TABLE * move_data;
  
  move_data = &symPokeMoveCode[ sympoke->move_type ];
  
  mmdl_sys = FIELDMAP_GetMMdlSys( padd->fieldmap );
  zone_id = FIELDMAP_GetZoneID( padd->fieldmap );
  
  head = data_MMdlHeader;
  head.id = id;
  // 見た目反映
  head.obj_code = TPOKE_DATA_GetObjCode( padd->tpdata,
      sympoke->monsno, sympoke->sex, sympoke->form_no );
  // 移動タイプ反映
  head.move_code    = move_data->move_code;
  head.move_limit_x = move_data->limx;
  head.move_limit_z = move_data->limz;
  //動作モデルのパラメータにシンボルポケモンの情報を埋め込んでいる
  head.param0 = padd->start_no + id;
  head.param1 = *((u32*)sympoke) & 0xffff;
  head.param2 = *((u32*)sympoke) >> 16;

  grid_pos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
  grid_pos->gx = grid_x;
  grid_pos->gz = grid_z;
  grid_pos->y = fx_y;
  
  MMDLSYS_AddMMdl(mmdl_sys, &head, zone_id);
  TAMADA_Printf("idx=%02d (%2d,%2d)\n", id, grid_x, grid_z );
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
  u8 *level_tbl;
  u8 level;
  
  level_tbl = GFL_ARC_UTIL_Load(ARCID_SYMBOL_LV, NARC_symbol_lv_tbl_lv_tbl_bin, FALSE, heapID);
  level = level_tbl[sympoke->monsno];
  GFL_HEAP_FreeMemory(level_tbl);

  oya_id = MyStatus_GetID( GAMEDATA_GetMyStatus( gamedata ) );
  personal_rnd = POKETOOL_CalcPersonalRandSpec(
      oya_id, sympoke->monsno, sympoke->form_no, sympoke->sex, 0, FALSE );
  TAMADA_Printf("symbol rand = %ld\n", personal_rnd );

  pp = PP_Create( sympoke->monsno, level, PTL_SETUP_ID_AUTO, heapID );
  PP_SetupEx( pp, sympoke->monsno, level, oya_id, PTL_SETUP_ID_AUTO, personal_rnd );
  PP_SetTokusei3( pp, sympoke->monsno, sympoke->form_no );
  PP_ChangeFormNo( pp, sympoke->form_no );
  if ( sympoke->wazano )
  {
    if ( PP_SetWaza( pp, sympoke->wazano ) == PTL_WAZASET_FAIL )
    {
      PP_SetWazaPush( pp, sympoke->wazano );
    }
  }
  return pp;
}


//--------------------------------------------------------------
//--------------------------------------------------------------
#include "../../../resource/fldmapdata/symbol_map/symbol_map_pos.cdat"
//--------------------------------------------------------------
/**
 * @brief シンボルポケモンの動作モデルを配置する
 * @param fieldmap
 * @param start_no  配置ナンバーの開始
 * @param sympoke   配置するシンボルポケモンのデータ配列
 * @param poke_num  配置するシンボルポケモンのデータ数
 *
 * @note
 * KEEPゾーンの場合、前半１０は大きいポケモン、後半１０は小さいポケモンの
 * 領域で、間にポケモンが入っていない条件のデータが来ることもあるので注意。
 *
 */
//--------------------------------------------------------------
void SYMBOLPOKE_Add(
    FIELDMAP_WORK *fieldmap, u32 start_no, const SYMBOL_POKEMON * sympoke, int poke_num,
    SYMBOL_MAP_ID symmap_id )
{
  POKEADD_WORK padd;
  int idx = 0;
  u16 jx, jz;
  MMDL * jiki;

  const u8 * map_pos;

  if ( SYMBOLMAP_IsKeepzoneID( symmap_id ) )
  {
    map_pos = keepMapPos;
  }
  else if ( SYMBOLMAP_IsLargePokeID( symmap_id ) )
  {
    map_pos = largeMapPos;
  }
  else
  {
    map_pos = littleMapPos;
  }

#if 0
  switch ( type )
  {
  case SYMBOL_ZONE_TYPE_KEEP_LARGE:
  case SYMBOL_ZONE_TYPE_KEEP_SMALL:
    map_pos = keepMapPos;
    break;
  case SYMBOL_ZONE_TYPE_FREE_LARGE:
    map_pos = largeMapPos;
    break;
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case SYMBOL_ZONE_TYPE_FREE_SMALL:
    map_pos = littleMapPos;
    break;
  }
#endif
  
  padd.tpdata = TPOKE_DATA_Create( FIELDMAP_GetHeapID(fieldmap) );
  padd.fieldmap = fieldmap;
  padd.start_no = start_no;

  jiki = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldmap ) );
  jx = MMDL_GetGridPosX( jiki );
  jz = MMDL_GetGridPosZ( jiki );

  for ( idx = 0; idx < SYMBOL_MAP_STOCK_MAX; idx ++ )
  {
    if ( sympoke[idx].monsno != 0 )
    {
      int x, z;
      int rnd_x, rnd_z;
      rnd_x = GFUser_GetPublicRand0(2) - 1;
      rnd_z = GFUser_GetPublicRand0(2) - 1;
      x = map_pos[ idx * 2 + 0 ] + rnd_x;
      z = map_pos[ idx * 2 + 1 ] + rnd_z;
      if ( jx == x && jz == z )
      { //自機との重なりを回避する
        if (rnd_x) x -= rnd_x;
        else if (rnd_z) z -= rnd_z;
        else x += 1;
      }
      SYMBOLPOKE_AdMMdl(&padd, &sympoke[idx], idx, x, z, 0 );
      poke_num --;
    }
  }

  TPOKE_DATA_Delete( padd.tpdata );
}



