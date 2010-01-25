////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ISSスイッチシステム
 * @file   iss_switch_sys.h
 * @author obata
 * @date   2009.11.17
 */
////////////////////////////////////////////////////////////////////////////
#pragma once 


//==========================================================================
// ■システムの不完全型宣言
//==========================================================================
typedef struct _ISS_SWITCH_SYS ISS_SWITCH_SYS;


//==========================================================================
// ■定数
//==========================================================================
// スイッチ番号
typedef enum{
  SWITCH_00,  // スイッチ0 ==> デフォルトでON
  SWITCH_01,  // スイッチ1
  SWITCH_02,  // スイッチ2
  SWITCH_03,  // スイッチ3
  SWITCH_04,  // スイッチ4
  SWITCH_05,  // スイッチ5
  SWITCH_06,  // スイッチ6
  SWITCH_07,  // スイッチ7
  SWITCH_08,  // スイッチ8
  SWITCH_NUM,
  SWITCH_MAX = SWITCH_NUM - 1
} SWITCH_INDEX;


//==========================================================================
// ■作成・破棄
//==========================================================================
extern ISS_SWITCH_SYS* ISS_SWITCH_SYS_Create( HEAPID heapID );
extern void            ISS_SWITCH_SYS_Delete( ISS_SWITCH_SYS* sys );

//==========================================================================
// ■システム動作
//==========================================================================
extern void ISS_SWITCH_SYS_Update( ISS_SWITCH_SYS* sys );

//==========================================================================
// ■システム制御
//==========================================================================
// 起動/停止
extern void ISS_SWITCH_SYS_On( ISS_SWITCH_SYS* sys );
extern void ISS_SWITCH_SYS_Off( ISS_SWITCH_SYS* sys );
// ゾーン変更通知
extern void ISS_SWITCH_SYS_ZoneChange( ISS_SWITCH_SYS* sys, u16 zoneID );

//==========================================================================
// ■スイッチ制御
//==========================================================================
// スイッチ ON/OFF
extern void ISS_SWITCH_SYS_SwitchOn( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
extern void ISS_SWITCH_SYS_SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
// スイッチの ON/OFF 取得
extern BOOL ISS_SWITCH_SYS_IsSwitchOn( const ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
