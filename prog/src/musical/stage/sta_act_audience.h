//======================================================================
/**
 * @file	sta_act_audience.h
 * @brief	ステージ用　観客
 * @author	ariizumi
 * @data	09/05/26
 */
//======================================================================

#include "sta_acting.h"
#include "sta_act_poke.h"

#pragma once

//======================================================================
//	define
//======================================================================
//ライトが誰もターゲットしてない
#define STA_AUDI_NO_TARGET (0xFF)
//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

typedef struct _STA_AUDI_SYS STA_AUDI_SYS;

//======================================================================
//	proto
//======================================================================

extern STA_AUDI_SYS* STA_AUDI_InitSystem( HEAPID heapId , ACTING_WORK *actWork , STAGE_INIT_WORK *initWork );
extern void	STA_AUDI_UpdateSystem( STA_AUDI_SYS *work );
extern void	STA_AUDI_ExitSystem( STA_AUDI_SYS *work );

extern void	STA_AUDI_SetAttentionPoke( STA_AUDI_SYS *work , const u8 trgPoke , const BOOL flg );
extern void	STA_AUDI_SetAttentionLight( STA_AUDI_SYS *work , const u8 trgLight );

extern void	STA_SUDI_SetScrollOffset( STA_AUDI_SYS *work , const u16 scrOfs );
