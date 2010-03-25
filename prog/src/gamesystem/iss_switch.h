///////////////////////////////////////////////////////////////////////////
/**
 * @brief  ISS スイッチ
 * @file   iss_switch.h
 * @author obata
 * @date   2010.03.23
 */
////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <gflib.h>

//==========================================================================
// ■スイッチの不完全型
//==========================================================================
typedef struct _ISS_SWITCH ISS_SWITCH;

//==========================================================================
// ■スイッチ操作関数
//==========================================================================
extern ISS_SWITCH* ISS_SWITCH_Create( HEAPID heapID );
extern void ISS_SWITCH_Delete( ISS_SWITCH* sw );
extern void ISS_SWITCH_SetSwitchParam( ISS_SWITCH* sw, u16 fadeFrame, u16 trackBit );
extern BOOL ISS_SWITCH_IsFade( const ISS_SWITCH* sw );
extern BOOL ISS_SWITCH_IsOn( const ISS_SWITCH* sw );
extern void ISS_SWITCH_On( ISS_SWITCH* sw );
extern void ISS_SWITCH_Off( ISS_SWITCH* sw );
extern void ISS_SWITCH_ForceOn( ISS_SWITCH* sw );
extern void ISS_SWITCH_ForceOff( ISS_SWITCH* sw );
extern void ISS_SWITCH_Main( ISS_SWITCH* sw );
extern void ISS_SWITCH_SetBGMVolume( const ISS_SWITCH* sw, int priorVolume );
