//======================================================================
/**
 * @file	mus_poke_data.h
 * @brief	ミュージカル用 ポケモンデータ
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUS_POKE_DATA_H__
#define MUS_POKE_DATA_H__

#include "musical/musical_system.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//描画１匹分
typedef struct _MUS_POKE_DATA_WORK MUS_POKE_DATA_WORK;

//======================================================================
//	proto
//======================================================================

//MUS_POKE_DRAW_WORKの作成で内部で作成されているので、描画を一緒に使う場合はそっちから取得してください
MUS_POKE_DATA_WORK* MUS_POKE_DATA_LoadMusPokeData( MUSICAL_POKE_PARAM *musPoke , HEAPID heapId );

//アイテム装備箇所関係
const BOOL MUS_POKE_DATA_EnableEquipPosition( MUS_POKE_DATA_WORK *pokeData , const MUS_POKE_EQUIP_POS pos );
GFL_POINT* MUS_POKE_DATA_GetEquipPosition( MUS_POKE_DATA_WORK *pokeData , const MUS_POKE_EQUIP_POS pos );
//指定点から近い位置を探す(相対座標にして渡してください
//lenは判定距離を渡す。戻り値で実際の距離の"二乗"が帰ってくる
MUS_POKE_EQUIP_POS MUS_POKE_DATA_SearchEquipPosition( MUS_POKE_DATA_WORK *pokeData , GFL_POINT *pos , u16 *len );


#endif MUS_POKE_DATA_H__