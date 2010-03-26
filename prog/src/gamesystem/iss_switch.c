/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ISS スイッチ
 * @file   iss_switch.c
 * @author obata
 * @date   2010.03.23
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "iss_switch.h"

#include "sound/pm_sndsys.h" // for PMSND_xxxx


//===============================================================================
// ■デバッグ情報
//===============================================================================
//#define DEBUG_MODE       // デバッグスイッチ
#define PRINT_TARGET (1) // デバッグ情報の出力先

#ifdef DEBUG_MODE
static u8 Debug_switchNum = 0;
#endif


//===============================================================================
// ■定数
//===============================================================================
#define MIN_VOLUME   (0) // 最小ボリューム
#define MAX_VOLUME (127) // 最大ボリューム

// スイッチの状態
typedef enum {
  SWITCH_STATE_ON,      // ON
  SWITCH_STATE_OFF,     // OFF
  SWITCH_STATE_FADEIN,  // フェード イン
  SWITCH_STATE_FADEOUT, // フェード アウト
} SWITCH_STATE; 


//===============================================================================
// ■ISS操作スイッチ
//===============================================================================
struct _ISS_SWITCH
{ 
  SWITCH_STATE state;     // 状態
  u16          fadeCount; // フェードカウンタ
  u16          fadeFrame; // フェードフレーム数
  u16          trackBit;  // 操作対象トラックビット

#ifdef DEBUG_MODE
  u8 debugID; // 識別子
#endif
};


//===============================================================================
// ■関数インデックス
//===============================================================================
static ISS_SWITCH* CreateSwitch( HEAPID heapID ); // スイッチを生成する
static void DeleteSwitch( ISS_SWITCH* sw ); // スイッチを破棄する
static void InitSwitch( ISS_SWITCH* sw ); // スイッチを初期化する
static void SetSwitchParam( ISS_SWITCH* sw, u16 fadeFrame, u16 trackBit ); // スイッチのパラメータを設定する
static SWITCH_STATE GetSwitchState( const ISS_SWITCH* sw ); // スイッチの状態を取得する
static void SetSwitchState( ISS_SWITCH* sw, SWITCH_STATE state ); // スイッチの状態を変更する
static int GetVolume( const ISS_SWITCH* sw ); // スイッチの状態からBGMのトラックボリュームを計算する
static void SetBGMVolume( const ISS_SWITCH* sw, int priorVolume ); // スイッチの状態をBGMボリュームに反映させる
static BOOL CheckSwitchFade( const ISS_SWITCH* sw ); // フェード中かどうかを判定する
static BOOL CheckSwitchOn( const ISS_SWITCH* sw ); // スイッチが押されているかどうかを判定する
static void IncFadeCount( ISS_SWITCH* sw ); // フェードカウンタをインクリメントする
static BOOL CheckFadeFinish( const ISS_SWITCH* sw ); // フェードが完了したかどうかをチェックする
static void SwitchMain( ISS_SWITCH* sw ); // スイッチのメイン動作
static void SwitchMain_ON( ISS_SWITCH* sw ); // スイッチのメイン動作 ( SWITCH_STATE_ON )
static void SwitchMain_OFF( ISS_SWITCH* sw ); // スイッチのメイン動作 ( SWITCH_STATE_OFF )
static void SwitchMain_FADEIN( ISS_SWITCH* sw ); // スイッチのメイン動作 ( SWITCH_STATE_FADEIN )
static void SwitchMain_FADEOUT( ISS_SWITCH* sw ); // スイッチのメイン動作 ( SWITCH_STATE_FADEOUT )
static void SwitchOn( ISS_SWITCH* sw ); // スイッチを押す
static void SwitchOff( ISS_SWITCH* sw ); // スイッチを離す
static void SwitchOnDirect( ISS_SWITCH* sw ); // スイッチを即時にONの状態にする
static void SwitchOffDirect( ISS_SWITCH* sw ); // スイッチを即時にOFFの状態にする

#ifdef DEBUG_MODE
static void Debug_PrintSwitch( const ISS_SWITCH* sw ); // スイッチの状態を出力する
#endif


//===============================================================================
// ■public method
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを生成する
 *
 * @param heapID 使用するヒープID
 *
 * @return 生成したスイッチ
 */
//-------------------------------------------------------------------------------
ISS_SWITCH* ISS_SWITCH_Create( HEAPID heapID )
{
  ISS_SWITCH* sw;

  sw = CreateSwitch( heapID );
  InitSwitch( sw ); 

  return sw;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを破棄する
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_Delete( ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  DeleteSwitch( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのパラメータを設定する
 *
 * @param sw
 * @param fadeFrame フェードフレーム数
 * @param trackBit  操作対象トラックを表すビットマスク
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SetSwitchParam( ISS_SWITCH* sw, u16 fadeFrame, u16 trackBit )
{
  GF_ASSERT( sw );

  SetSwitchParam( sw, fadeFrame, trackBit );
}

//-------------------------------------------------------------------------------
/**
 * @brief フェード中かどうかを判定する
 *
 * @param sw
 *
 * @return フェード中なら TRUE, そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_SWITCH_IsFade( const ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  return CheckSwitchFade( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが押されているかどうかを判定する
 *
 * @param sw
 *
 * @return スイッチが押されている場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_SWITCH_IsOn( const ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  return CheckSwitchOn( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを押す
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_On( ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  SwitchOn( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを離す
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_Off( ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  SwitchOff( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを即時にONの状態にする
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_ForceOn( ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  SwitchOnDirect( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを即時にOFFの状態にする
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_ForceOff( ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  SwitchOffDirect( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのメイン動作
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_Main( ISS_SWITCH* sw )
{
  GF_ASSERT( sw );

  SwitchMain( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態をBGMのボリュームに反映させる
 *
 * @param sw
 * @param priorVolume
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SetBGMVolume( const ISS_SWITCH* sw, int priorVolume )
{
  GF_ASSERT( sw );

  SetBGMVolume( sw, priorVolume );
}


//===============================================================================
// ■private method
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを生成する
 *
 * @param heapID
 *
 * @return 生成したスイッチ
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH* CreateSwitch( HEAPID heapID )
{
  ISS_SWITCH* sw;

  sw = GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SWITCH) );

#ifdef DEBUG_MODE
  sw->debugID = Debug_switchNum++;
#endif

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: create switch\n", sw->debugID );
#endif

  return sw;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを破棄する
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void DeleteSwitch( ISS_SWITCH* sw )
{
#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: delete switch\n", sw->debugID );
#endif

  GFL_HEAP_FreeMemory( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを初期化する
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void InitSwitch( ISS_SWITCH* sw )
{
  sw->state     = SWITCH_STATE_OFF;
  sw->fadeCount = 0;
  sw->fadeFrame = 0;
  sw->trackBit  = 0;

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: init switch\n", sw->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのパラメータを設定する
 *
 * @param sw
 * @param fadeFrame フェードフレーム数
 * @param trackBit  操作対象トラックを表すビットマスク
 */
//-------------------------------------------------------------------------------
static void SetSwitchParam( ISS_SWITCH* sw, u16 fadeFrame, u16 trackBit )
{
  sw->fadeFrame = fadeFrame;
  sw->trackBit = trackBit;

#ifdef DEBUG_MODE
  Debug_PrintSwitch( sw );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief フェードカウンタをインクリメントする
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void IncFadeCount( ISS_SWITCH* sw )
{
  sw->fadeCount++;

  // 最大値補正
  if( sw->fadeFrame < sw->fadeCount ) {
    sw->fadeCount = sw->fadeFrame;
  }
} 

//-------------------------------------------------------------------------------
/**
 * @brief フェードが完了したかどうかをチェックする
 *
 * @param sw
 *
 * @return フェードが完了した場合 TRUE
 *         そうでない場合 FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckFadeFinish( const ISS_SWITCH* sw )
{
  if( sw->fadeFrame <= sw->fadeCount ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのメイン動作
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchMain( ISS_SWITCH* sw )
{ 
  switch( GetSwitchState(sw) ) {
  case SWITCH_STATE_ON:      SwitchMain_ON( sw );      break;
  case SWITCH_STATE_OFF:     SwitchMain_OFF( sw );     break;
  case SWITCH_STATE_FADEIN:  SwitchMain_FADEIN( sw );  break;
  case SWITCH_STATE_FADEOUT: SwitchMain_FADEOUT( sw ); break;
  default: GF_ASSERT(0);
  }
} 

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのメイン動作 ( SWITCH_STATE_ON )
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchMain_ON( ISS_SWITCH* sw )
{
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのメイン動作 ( SWITCH_STATE_OFF )
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchMain_OFF( ISS_SWITCH* sw )
{
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのメイン動作 ( SWITCH_STATE_FADEIN )
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchMain_FADEIN( ISS_SWITCH* sw )
{
  IncFadeCount( sw );

  // フェードイン終了
  if( CheckFadeFinish( sw ) == TRUE ) {
    SetSwitchState( sw, SWITCH_STATE_ON );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのメイン動作 ( SWITCH_STATE_FADEOUT )
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchMain_FADEOUT( ISS_SWITCH* sw )
{
  IncFadeCount( sw );

  // フェードアウト終了
  if( CheckFadeFinish( sw ) == TRUE ) {
    SetSwitchState( sw, SWITCH_STATE_OFF );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを押す
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchOn( ISS_SWITCH* sw )
{
  SetSwitchState( sw, SWITCH_STATE_FADEIN ); 

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: on\n", sw->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを離す
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchOff( ISS_SWITCH* sw )
{
  SetSwitchState( sw, SWITCH_STATE_FADEOUT );

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: off\n", sw->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを即時にONの状態にする
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchOnDirect( ISS_SWITCH* sw )
{
  SetSwitchState( sw, SWITCH_STATE_ON );

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: direct on\n", sw->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを即時にOFFの状態にする
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void SwitchOffDirect( ISS_SWITCH* sw )
{
  SetSwitchState( sw, SWITCH_STATE_OFF );

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: direct off\n", sw->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を取得する
 *
 * @param sw
 *
 * @return スイッチの状態
 */
//-------------------------------------------------------------------------------
static SWITCH_STATE GetSwitchState( const ISS_SWITCH* sw )
{
  return sw->state;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を変更する
 *
 * @param sw
 * @param state 設定する状態
 */
//-------------------------------------------------------------------------------
static void SetSwitchState( ISS_SWITCH* sw, SWITCH_STATE state )
{
  sw->state = state;
  sw->fadeCount = 0;

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: set switch state ==> ", sw->debugID );
  switch( state ) {
  case SWITCH_STATE_ON:      OS_TFPrintf( PRINT_TARGET, "ON" );      break;
  case SWITCH_STATE_OFF:     OS_TFPrintf( PRINT_TARGET, "OFF" );     break;
  case SWITCH_STATE_FADEIN:  OS_TFPrintf( PRINT_TARGET, "FADEIN" );  break;
  case SWITCH_STATE_FADEOUT: OS_TFPrintf( PRINT_TARGET, "FADEOUT" ); break;
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態からBGMのトラックボリュームを計算する
 *
 * @param sw
 *
 * @return スイッチの状態に対応するトラックボリューム
 */
//-------------------------------------------------------------------------------
static int GetVolume( const ISS_SWITCH* sw )
{
  SWITCH_STATE state;
  int volume;

  switch( GetSwitchState( sw ) ) {
  case SWITCH_STATE_ON:  return MAX_VOLUME;
  case SWITCH_STATE_OFF: return MIN_VOLUME;
  case SWITCH_STATE_FADEIN:
    volume = MAX_VOLUME * sw->fadeCount / sw->fadeFrame;
    break;
  case SWITCH_STATE_FADEOUT:
    volume = MAX_VOLUME - MAX_VOLUME * sw->fadeCount / sw->fadeFrame;
    break;
  default: 
    GF_ASSERT(0);
  }

  GF_ASSERT( MIN_VOLUME <= volume );
  GF_ASSERT( volume <= MAX_VOLUME );
  return volume;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態をBGMのボリュームに反映させる
 *
 * @param sw
 * @param priorVolume 上位ボリューム値
 */
//-------------------------------------------------------------------------------
static void SetBGMVolume( const ISS_SWITCH* sw, int priorVolume )
{
  int volume;

  // 上位ボリュームを考慮した値を算出
  volume = GetVolume(sw) * priorVolume / MAX_VOLUME;

  GF_ASSERT( MIN_VOLUME <= volume );
  GF_ASSERT( volume <= MAX_VOLUME );

  // BGMに反映
  PMSND_ChangeBGMVolume( sw->trackBit, volume );
}

//-------------------------------------------------------------------------------
/**
 * @brief フェード中かどうかを判定する
 *
 * @param sw
 *
 * @return フェード中なら TRUE, そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckSwitchFade( const ISS_SWITCH* sw )
{
  switch( GetSwitchState( sw ) ) {
  case SWITCH_STATE_ON:      return FALSE;
  case SWITCH_STATE_OFF:     return FALSE;
  case SWITCH_STATE_FADEIN:  return TRUE;
  case SWITCH_STATE_FADEOUT: return TRUE;
  default: GF_ASSERT(0);
  }

  GF_ASSERT(0);
  return FALSE;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが押されているかどうかを判定する
 *
 * @param sw
 *
 * @return スイッチが押されている場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckSwitchOn( const ISS_SWITCH* sw )
{
  switch( GetSwitchState( sw ) ) {
  case SWITCH_STATE_ON:      return TRUE;
  case SWITCH_STATE_OFF:     return FALSE;
  case SWITCH_STATE_FADEIN:  return FALSE;
  case SWITCH_STATE_FADEOUT: return FALSE;
  default: GF_ASSERT(0);
  }

  GF_ASSERT(0);
  return FALSE;
}

#ifdef DEBUG_MODE
//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を出力する
 *
 * @param sw
 */
//-------------------------------------------------------------------------------
static void Debug_PrintSwitch( const ISS_SWITCH* sw )
{
  OS_TFPrintf( PRINT_TARGET, "ISS-SWITCH[%d]: ", sw->debugID );

  // フェード時間
  OS_TFPrintf( PRINT_TARGET, "fadeFrame = %d, ", sw->fadeFrame ); 
  // トラックビット
  OS_TFPrintf( PRINT_TARGET, "trackBit = 0x%x, ", sw->trackBit );

  OS_TFPrintf( PRINT_TARGET, "\n" );
} 
#endif
