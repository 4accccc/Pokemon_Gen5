//======================================================================
/**
 * @file	sta_act_poke.h
 * @brief	ステージ用　ポケモン
 * @author	ariizumi
 * @data	09/03/06
 */
//======================================================================
#ifndef STA_ACT_POKE_H__
#define STA_ACT_POKE_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

typedef struct _STA_POKE_SYS STA_POKE_SYS;
typedef struct _STA_POKE_WORK STA_POKE_WORK;

//======================================================================
//	proto
//======================================================================

STA_POKE_SYS* STA_POKE_InitSystem( HEAPID heapId , MUS_POKE_DRAW_SYSTEM* drawSys , MUS_ITEM_DRAW_SYSTEM *itemDrawSys );
//ポケモンの描画の後にアイテムの更新をする必要があるので別関数化
void	STA_POKE_UpdateSystem( STA_POKE_SYS *work );
void	STA_POKE_UpdateSystem_Item( STA_POKE_SYS *work );
void	STA_POKE_DrawSystem( STA_POKE_SYS *work );
void	STA_POKE_ExitSystem( STA_POKE_SYS *work );

//システム系
void	STA_POKE_System_SetScrollOffset( STA_POKE_SYS *work , const u16 scrOfs );


//ポケ単体系
STA_POKE_WORK *STA_POKE_AddPoke( STA_POKE_SYS *work , MUSICAL_POKE_PARAM *musPoke );
void STA_POKE_SetPosition( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const VecFx32 *pos );

#endif STA_ACT_POKE_H__