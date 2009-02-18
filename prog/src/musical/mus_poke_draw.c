//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカル用 ポケモン描画
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/mcss.h"
#include "system/mcss_tool.h"

#include "arc_def.h"
#include "poke_tool/poke_tool.h"

#include "battle/poke_mcss.h"

#include "musical/mus_poke_draw.h"

//======================================================================
//	define
//======================================================================
#define MUS_POKE_DRAW_MAX 4	//描画最大数

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//描画１匹分
struct _MUS_POKE_DRAW_WORK
{
	BOOL enable;
	MCSS_WORK	*mcss;
	
	MUS_POKE_DATA_WORK *pokeData;
};

//描画システム
struct _MUS_POKE_DRAW_SYSTEM
{
	HEAPID heapId;
	MCSS_SYS_WORK *mcssSys;
	MUS_POKE_DRAW_WORK musMcss[MUS_POKE_DRAW_MAX];
};

//======================================================================
//	proto
//======================================================================


//--------------------------------------------------------------
//	
//--------------------------------------------------------------
//システムの初期化と開放
MUS_POKE_DRAW_SYSTEM*	MUS_POKE_DRAW_InitSystem( HEAPID heapId )
{
	int i;
	MUS_POKE_DRAW_SYSTEM *work = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_POKE_DRAW_SYSTEM) );
	
	work->heapId = heapId;
	work->mcssSys = MCSS_Init( MUS_POKE_DRAW_MAX , heapId );
	for( i=0;i<MUS_POKE_DRAW_MAX;i++ )
	{
		work->musMcss[i].enable = FALSE;
	}
	
	MCSS_SetOrthoMode( work->mcssSys );
	return work;
}
void MUS_POKE_DRAW_TermSystem( MUS_POKE_DRAW_SYSTEM* work )
{
	int i;
	for( i=0;i<MUS_POKE_DRAW_MAX;i++ )
	{
		if( work->musMcss[i].enable == TRUE )
		{
			MUS_POKE_DRAW_Del( work , &work->musMcss[i] );
		}
	}
	MCSS_Exit( work->mcssSys );
	GFL_HEAP_FreeMemory( work );
}

//更新
void MUS_POKE_DRAW_UpdateSystem( MUS_POKE_DRAW_SYSTEM* work )
{
	MCSS_Main( work->mcssSys );
}
void MUS_POKE_DRAW_DrawSystem( MUS_POKE_DRAW_SYSTEM* work )
{
	MCSS_Draw( work->mcssSys );
}

MUS_POKE_DRAW_WORK* MUS_POKE_DRAW_Add( MUS_POKE_DRAW_SYSTEM* work , MUSICAL_POKE_PARAM *musPoke )
{
	int i;
	VecFx32 scale;
	MCSS_ADD_WORK maw;
	
	MCSS_TOOL_MakeMAWPP( musPoke->pokePara , &maw , MCSS_DIR_FRONT );
	
	//空きスペースの検索
	
	for( i=0;i<MUS_POKE_DRAW_MAX;i++ )
	{
		if( work->musMcss[i].enable == FALSE )
		{
			break;
		}
	}
	GF_ASSERT_MSG( i < MUS_POKE_DRAW_MAX , "MusicalPokemon draw max over!!\n");
	
	
	work->musMcss[i].mcss = MCSS_Add( work->mcssSys , 0,0,0,&maw );
	VEC_Set( &scale, 
			 FX32_ONE*16, 
			 FX32_ONE*16,
			 FX32_ONE );

	MUS_POKE_DRAW_SetScale( &work->musMcss[i], &scale );
	
	//装備箇所データなど読み込み
	work->musMcss[i].pokeData = MUS_POKE_DATA_LoadMusPokeData( musPoke , work->heapId );
	
	return &work->musMcss[i];
}

void MUS_POKE_DRAW_Del( MUS_POKE_DRAW_SYSTEM* work , MUS_POKE_DRAW_WORK *drawWork )
{
	MCSS_Del( work->mcssSys , drawWork->mcss );
	drawWork->enable = FALSE;
}

void MUS_POKE_DRAW_SetPosition( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *pos )
{
	MCSS_SetPosition( drawWork->mcss , pos );
}

void MUS_POKE_DRAW_GetPosition( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *pos )
{
	MCSS_GetPosition( drawWork->mcss , pos );
}

void MUS_POKE_DRAW_SetScale( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *scale )
{
	MCSS_SetScale( drawWork->mcss , scale );
}

void MUS_POKE_DRAW_GetScale( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *scale )
{
	MCSS_GetScale( drawWork->mcss , scale );
}

MUS_POKE_DATA_WORK*	MUS_POKE_DRAW_GetPokeData( MUS_POKE_DRAW_WORK *drawWork )
{
	return drawWork->pokeData;
}
