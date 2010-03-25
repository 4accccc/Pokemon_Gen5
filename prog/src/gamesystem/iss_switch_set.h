///////////////////////////////////////////////////////////////////////////
/**
 * @brief  ISS スイッチセット
 * @file   iss_switch_set.h
 * @author obata
 * @date   2010.03.24
 */
////////////////////////////////////////////////////////////////////////////
#pragma once 


//==========================================================================
// ■スイッチの不完全型
//==========================================================================
typedef struct _ISS_SWITCH_SET ISS_SWITCH_SET;


//==========================================================================
// ■定数
//==========================================================================
// スイッチ番号
typedef enum {
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
// ■スイッチセット操作関数
//========================================================================== 
// スイッチセットを生成する
extern ISS_SWITCH_SET* ISS_SWITCH_SET_Create( HEAPID heapID );
// スイッチセットを破棄する
extern void ISS_SWITCH_SET_Delete( ISS_SWITCH_SET* set );
// スイッチデータをロードする
extern void ISS_SWITCH_SET_Load( ISS_SWITCH_SET* set, ARCDATID datID, HEAPID heapID );
// スイッチセットのメイン動作
extern void ISS_SWITCH_SET_Main( ISS_SWITCH_SET* set );
// スイッチセットを押す
extern void ISS_SWITCH_SET_SwitchOn( ISS_SWITCH_SET* set, SWITCH_INDEX idx );
// スイッチセットを離す
extern void ISS_SWITCH_SET_SwitchOff( ISS_SWITCH_SET* set, SWITCH_INDEX idx );
// スイッチセットをリセットする
extern void ISS_SWITCH_SET_ResetSwitch( ISS_SWITCH_SET* set );
// 対応するシーケンス番号を取得する
extern u32 ISS_SWITCH_SET_GetSoundIdx( const ISS_SWITCH_SET* set );
// スイッチセットが有効な場所かどうかを判定する
extern BOOL ISS_SWITCH_SET_IsValidAt( const ISS_SWITCH_SET* set, u16 zoneID );
// スイッチが押されているかどうかを判定する
extern BOOL ISS_SWITCH_SET_CheckSwitchOn( const ISS_SWITCH_SET* set, SWITCH_INDEX idx );
// スイッチセットの状態をBGMのボリュームに反映させる
extern void ISS_SWITCH_SET_SetBGMVolume( const ISS_SWITCH_SET* set ); 
