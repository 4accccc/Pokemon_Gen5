//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ステージ 演技メイン
 * @author	ariizumi
 * @data	09/03/02
 */
//======================================================================
#ifndef STA_ACTING_H__
#define STA_ACTING_H__

#include "musical/musical_system.h"
#include "musical/musical_stage_sys.h"
#include "musical/mus_item_data.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
typedef enum
{
	ACT_RET_CONTINUE,
	ACT_RET_GO_END,
}ACTING_RETURN;

typedef enum
{
  ANIT_START,
  ANIT_ACTION,
  ANIT_DISTURB,
}ACTING_NPC_ITEMUSE_TYPE;
//======================================================================
//	typedef struct
//======================================================================

typedef struct _ACTING_WORK ACTING_WORK;


//======================================================================
//	proto
//======================================================================

extern ACTING_WORK*	STA_ACT_InitActing( STAGE_INIT_WORK *initWork , HEAPID heapId );
extern void	STA_ACT_TermActing( ACTING_WORK *work );
extern ACTING_RETURN	STA_ACT_LoopActing( ACTING_WORK *work );

extern u16		STA_ACT_GetStageScroll( ACTING_WORK *work );
extern void	STA_ACT_SetStageScroll( ACTING_WORK *work , const u16 scroll );

extern MUS_ITEM_DATA_SYS* STA_ACT_GetItemDataSys( ACTING_WORK *work );

extern void STA_ACT_UseItemRequest( ACTING_WORK *work , MUS_POKE_EQUIP_POS ePos );
extern void STA_ACT_UseItem( ACTING_WORK *work , u8 pokeIdx , MUS_POKE_EQUIP_POS ePos );
extern void STA_ACT_CalcRank( ACTING_WORK *work );

extern const u8 STA_ACT_GetUseItemAttentionPoke( ACTING_WORK *work );
extern const BOOL STA_ACT_IsUsingItemSelf( ACTING_WORK *work );
extern void STA_ACT_CheckUseItemNpc( ACTING_WORK *work , const ACTING_NPC_ITEMUSE_TYPE type , const u8 selfIdx );
extern const u8 STA_ACT_GetPokeTempPoint( ACTING_WORK *work , const u8 pokeNo );
extern const u32 STA_ACT_GetDelayVCnt( ACTING_WORK *work );

//エディタ用

extern void	STA_ACT_EDITOR_SetEditorMode( ACTING_WORK *work );
extern void	STA_ACT_EDITOR_SetScript( ACTING_WORK *work , void* data );
extern void	STA_ACT_EDITOR_StartScript( ACTING_WORK *work );

#endif STA_ACTING_H__