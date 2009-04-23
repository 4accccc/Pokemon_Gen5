//======================================================================
/**
 * @file	musical_system.h
 * @brief	ミュージカル用 汎用定義
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================

#include "musical/musical_system.h"
#include "poke_tool/monsno_def.h"

static const u16 musPokeArr[]=
{
	MONSNO_PIKATYUU,
	MONSNO_RAITYUU,
	MONSNO_PIKUSII,
	MONSNO_EREBUU,
	494,
	495,
	0xFFFF,
};
//ミュージカルの参加資格があるか調べる(仮)
const BOOL	MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( const u16 mons_no )
{
	u16 i=0;
	while( 0xFFFF != musPokeArr[i] )
	{
		if( mons_no == musPokeArr[i] )
		{
			return TRUE;
		}
		i++;
	}
	return FALSE;
}

const BOOL	MUSICAL_SYSTEM_CheckEntryMusical( POKEMON_PARAM *pokePara )
{
	u16	mons_no = PP_Get( pokePara, ID_PARA_monsno,	NULL );
	return MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( mons_no );
}

//ミュージカルでの番号に変換
const u16	MUSICAL_SYSTEM_ChangeMusicalPokeNumber( POKEMON_PARAM *pokePara )
{
	u16 i=0;
	int	mons_no = PP_Get( pokePara, ID_PARA_monsno,	NULL );

	while( 0xFFFF != musPokeArr[i] )
	{
		if( mons_no == musPokeArr[i] )
		{
			return i;
		}
		i++;
	}
	//一応最終番号に変換
	return i-1;
}

//ミュージカル用パラメータの初期化
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPoke( POKEMON_PARAM *pokePara , HEAPID heapId )
{
	int i;
	MUSICAL_POKE_PARAM *musPara;
	musPara = GFL_HEAP_AllocMemory( heapId , sizeof(MUSICAL_POKE_PARAM) );
	musPara->pokePara = pokePara;
	
	for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
	{
		musPara->equip[i].itemNo = MUSICAL_ITEM_INVALID;
		musPara->equip[i].angle = 0;
	}
	
	return musPara;
}