//======================================================================
/**
 * @file	sta_act_bg.h
 * @brief	ステージ用　背景(bbdでは大きいサイズが扱えないので独自実装
 * @author	ariizumi
 * @data	09/03/16
 */
//======================================================================
#ifndef ACT_STA_BG_H__
#define ACT_STA_BG_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

typedef struct _STA_BG_SYS STA_BG_SYS;

//======================================================================
//	proto
//======================================================================

STA_BG_SYS* STA_BG_InitSystem( HEAPID heapId , ACTING_WORK* actWork );
void	STA_BG_UpdateSystem( STA_BG_SYS *work );
void	STA_BG_DrawSystem( STA_BG_SYS *work );
void	STA_BG_ExitSystem( STA_BG_SYS *work );

//システム系

void	STA_BG_SetScrollOffset( STA_BG_SYS* work , const u16 bgOfs );
void	STA_BG_CreateBg( STA_BG_SYS* work , const u8 bgNo );
void	STA_BG_DeleteBg( STA_BG_SYS* work );


#endif ACT_STA_BG_H__