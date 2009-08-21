
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
#include "waza_tool/wazadata.h"
#include "btlv_effvm_def.h"

#pragma once

#ifdef PM_DEBUG
//デバッグ用パーティクルデータ構造体定義
typedef struct
{
	int	particle_count;
	u8	adrs[1];
}DEBUG_PARTICLE_DATA;
#endif

extern	VMHANDLE	*BTLV_EFFVM_Init( GFL_TCBSYS *tcbsys, HEAPID heapID );
extern	BOOL  		BTLV_EFFVM_Main( VMHANDLE *vmh );
extern	void  		BTLV_EFFVM_Exit( VMHANDLE *core );
extern	void  		BTLV_EFFVM_Start( VMHANDLE *core, BtlvMcssPos from, BtlvMcssPos to, WazaID waza );
extern  void			BTLV_EFFVM_Stop( VMHANDLE *vmh );

#ifdef PM_DEBUG
extern	void		BTLV_EFFVM_StartDebug( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, const VM_CODE *start, const DEBUG_PARTICLE_DATA *dpd );
extern	void		BTLV_EFFVM_DebugParticlePlay( VMHANDLE *vmh, GFL_PTC_PTR ptc, int index, int src, int dst, fx32 ofs_y, fx32 angle );
extern  void    BTLV_EFFVM_DebugSeEffect( VMHANDLE *vmh, int player, int type, int param, int start, int end, 
                                          int start_wait, int frame, int wait, int count );
extern  void    BTLV_EFFVM_ClearUnpackInfo( VMHANDLE *vmh );
#endif

