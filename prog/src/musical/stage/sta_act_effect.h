//======================================================================
/**
 * @file	sta_act_effect.h
 * @brief	ステージ用　エフェクト
 * @author	ariizumi
 * @data	09/03/03
 */
//======================================================================
#ifndef STA_ACT_EFFECT_H__
#define STA_ACT_EFFECT_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

typedef struct _STA_EFF_SYS STA_EFF_SYS;
typedef struct _STA_EFF_WORK STA_EFF_WORK;

//======================================================================
//	proto
//======================================================================


STA_EFF_SYS* STA_EFF_InitSystem( HEAPID heapId );
void	STA_EFF_UpdateSystem( STA_EFF_SYS *work );
void	STA_EFF_DrawSystem( STA_EFF_SYS *work );
void	STA_EFF_ExitSystem( STA_EFF_SYS *work );

STA_EFF_WORK*	STA_EFF_CreateEffect( STA_EFF_SYS *work , int fileIdx );
void	STA_EFF_CreateEmmitter( STA_EFF_WORK *effWork , u16 emmitNo , VecFx32 *pos );

#endif STA_ACT_EFFECT_H__