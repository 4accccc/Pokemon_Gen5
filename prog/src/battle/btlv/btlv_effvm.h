
//======================================================================
/**
 * @file	btlv_effvm.h
 * @brief	戦闘エフェクト
 * @author	HisashiSogabe
 * @date	2009.02.12
 */
//======================================================================

#define __C_NO_DEF_

#include "system/vm.h"
#include "btlv_effvm_def.h"

#ifndef __BTLV_EFFVM_H_
#define __BTLV_EFFVM_H_

extern	VMHANDLE	*BTLV_EFFVM_Init( HEAPID heapID );
extern	void		BTLV_EFFVM_Exit( VMHANDLE *core );

#endif //__BTLV_EFFECT_DEF_H_
