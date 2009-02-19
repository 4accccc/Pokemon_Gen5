//======================================================================
/**
 * @file	mus_poke_data.h
 * @brief	ミュージカル用 ポケモンデータ
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "poke_tool/poke_tool.h"


#include "musical/mus_poke_data.h"

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
struct _MUS_POKE_DATA_WORK
{
	BOOL		isEquip[MUS_POKE_EQUIP_MAX];
	GFL_POINT	equipPos[MUS_POKE_EQUIP_MAX];
};

//======================================================================
//	proto
//======================================================================

//--------------------------------------------------------------
//	データの読み込み
//--------------------------------------------------------------
MUS_POKE_DATA_WORK* MUS_POKE_DATA_LoadMusPokeData( MUSICAL_POKE_PARAM *musPoke , HEAPID heapId )
{
	int i;
	MUS_POKE_DATA_WORK* pokeData = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_POKE_DATA_WORK) );
	
	for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
	{
		pokeData->isEquip[i] = FALSE;
		pokeData->equipPos[i].x = 0;
		pokeData->equipPos[i].y = 0;
	}
	pokeData->isEquip[MUS_POKE_EQU_HEAD] = TRUE;
	pokeData->equipPos[MUS_POKE_EQU_HEAD].x =  2;
	pokeData->equipPos[MUS_POKE_EQU_HEAD].y = -44;

	pokeData->isEquip[MUS_POKE_EQU_BODY] = TRUE;
	pokeData->equipPos[MUS_POKE_EQU_BODY].x =  2;
	pokeData->equipPos[MUS_POKE_EQU_BODY].y = -18;

	pokeData->isEquip[MUS_POKE_EQU_HAND_R] = TRUE;
	pokeData->equipPos[MUS_POKE_EQU_HAND_R].x = -20;
	pokeData->equipPos[MUS_POKE_EQU_HAND_R].y = -28;

	pokeData->isEquip[MUS_POKE_EQU_HAND_L] = TRUE;
	pokeData->equipPos[MUS_POKE_EQU_HAND_L].x = 22;
	pokeData->equipPos[MUS_POKE_EQU_HAND_L].y = -26;

	return pokeData;
}

//--------------------------------------------------------------
//アイテム装備箇所関係
//--------------------------------------------------------------
const BOOL MUS_POKE_DATA_EnableEquipPosition( MUS_POKE_DATA_WORK *pokeData , const MUS_POKE_EQUIP_POS pos )
{
	return pokeData->isEquip[pos];
}
GFL_POINT* MUS_POKE_DATA_GetEquipPosition( MUS_POKE_DATA_WORK *pokeData , const MUS_POKE_EQUIP_POS pos )
{
	return &pokeData->equipPos[pos];
}

//--------------------------------------------------------------
//指定点から近い位置を探す(相対座標にして渡してください
//--------------------------------------------------------------
MUS_POKE_EQUIP_POS MUS_POKE_DATA_SearchEquipPosition( MUS_POKE_DATA_WORK *pokeData , GFL_POINT *pos , u16 *len )
{
	MUS_POKE_EQUIP_POS i;
	u16 minLen = (*len)*(*len);	//あらかじめ最小距離に指定距離を入れておく
	MUS_POKE_EQUIP_POS minPos = MUS_POKE_EQU_INVALID;
	for( i=0; i<MUS_POKE_EQUIP_MAX ;i++ )
	{
		if( pokeData->isEquip[i] == TRUE )
		{
			const int subX = pos->x - pokeData->equipPos[i].x;
			const int subY = pos->y - pokeData->equipPos[i].y;
			const int subLen = (subX*subX)+(subY*subY);
			if( subLen < minLen )
			{
				minLen = subLen;
				minPos = i;
			}
		}
		
	}
	*len = minLen;
	return minPos;
}
