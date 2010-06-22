////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ISSスイッチシステム
 * @file   iss_switch_sys.h
 * @author obata
 * @date   2009.11.17
 */
////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <gflib.h>
#include "iss_switch_set.h"


//==========================================================================
// ■システムの不完全型宣言
//==========================================================================
typedef struct _ISS_SWITCH_SYS ISS_SWITCH_SYS; 


//==========================================================================
// ■作成・破棄
//==========================================================================
extern ISS_SWITCH_SYS* ISS_SWITCH_SYS_Create( HEAPID heapID );
extern void ISS_SWITCH_SYS_Delete( ISS_SWITCH_SYS* system );

//==========================================================================
// ■システム動作
//==========================================================================
extern void ISS_SWITCH_SYS_Update( ISS_SWITCH_SYS* system );

//==========================================================================
// ■システム制御
//==========================================================================
// 起動/停止
extern void ISS_SWITCH_SYS_On( ISS_SWITCH_SYS* system );
extern void ISS_SWITCH_SYS_Off( ISS_SWITCH_SYS* system );
// ゾーン変更通知
extern void ISS_SWITCH_SYS_ZoneChange( ISS_SWITCH_SYS* system, u16 zoneID );

//==========================================================================
// ■スイッチ制御
//==========================================================================
// スイッチ ON/OFF
extern void ISS_SWITCH_SYS_SwitchOn( ISS_SWITCH_SYS* system, SWITCH_INDEX switchIdx );
extern void ISS_SWITCH_SYS_SwitchOff( ISS_SWITCH_SYS* system, SWITCH_INDEX switchIdx );
// スイッチの ON/OFF 取得
extern BOOL ISS_SWITCH_SYS_IsSwitchOn( const ISS_SWITCH_SYS* system, SWITCH_INDEX switchIdx );
// スイッチのリセット
extern void ISS_SWITCH_SYS_ResetSwitch( ISS_SWITCH_SYS* system );
