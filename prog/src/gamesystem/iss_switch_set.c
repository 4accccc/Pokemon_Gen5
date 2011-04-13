/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ISS スイッチセット
 * @file   iss_switch_set.c
 * @author obata
 * @date   2010.03.24
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "iss_switch.h"
#include "iss_switch_set.h"

#include "arc/arc_def.h" // for ARCID_ISS_SWITCH 


//===============================================================================
// ■デバッグ情報
//===============================================================================
//#define DEBUG_MODE       // デバッグスイッチ
#define PRINT_TARGET (1) // デバッグ情報の出力先

#ifdef DEBUG_MODE
static u8 Debug_switchSetNum = 0;
#endif

#define ASSERT_REPLACE( calc ){ GF_ASSERT_MSG((calc),"") }

//===============================================================================
// ■定数
//===============================================================================
#define MAX_VALID_ZONE_NUM (9) // スイッチ状態を維持するゾーンの最大数

// スイッチセットの状態
//  現在は音量の上げ下げのみ
typedef enum {
  SWITCH_SET_STATE_DEFAULT,      // 通常
  SWITCH_SET_STATE_VOLUME_DOWN,  // 音量ダウン
  SWITCH_SET_STATE_VOLUME_UP,    // 音量アップ
} SWITCH_SET_STATE;


//===============================================================================
// ■ISSスイッチセット
//===============================================================================
struct _ISS_SWITCH_SET
{ 
  SWITCH_SET_STATE  state;             // 状態
  u32 soundIdx;                        // シーケンス番号
  u16 validZone[ MAX_VALID_ZONE_NUM ]; // スイッチが有効な場所配列
  u8  validZoneNum;                    // スイッチが有効な場所の数
  ISS_SWITCH* IssSwitch[ SWITCH_NUM ]; // スイッチ
  int volume;                          // 全てのスイッチの音量
  int tmpVolume;                       // 計算用
  int tmpMaxVolume;                    // 計算用
  u16 fadeCount;                       // フェードカウンタ
  u16 fadeFrame;                       // フェードフレーム数

#ifdef DEBUG_MODE
  u8 debugID; // 識別子
#endif
};


//===============================================================================
// ■関数インデックス
//===============================================================================
static void InitSwitchSet( ISS_SWITCH_SET* set ); // スイッチセットを初期化する
static ISS_SWITCH_SET* CreateSwitchSet( HEAPID heapID ); // スイッチセットを生成する
static void DeleteSwitchSet( ISS_SWITCH_SET* set ); // スイッチセットを破棄する
static void CreateSwitches( ISS_SWITCH_SET* set, HEAPID heapID ); // スイッチを生成する
static void DeleteSwitches( ISS_SWITCH_SET* set ); // スイッチを破棄する
static void LoadSwitchData( ISS_SWITCH_SET* set, ARCDATID datID, HEAPID heapID ); // スイッチデータを読み込む
static u32 GetSoundIdx( const ISS_SWITCH_SET* set ); // スイッチに対応するシーケンス番号を取得する
static u8 GetValidZoneNum( const ISS_SWITCH_SET* set ); // スイッチが有効な場所の数を取得する
static u16 GetValidZoneID( const ISS_SWITCH_SET* set, int idx ); // スイッチが有効な場所を取得する
static BOOL CheckValidZone( const ISS_SWITCH_SET* set, u16 zoneID ); // スイッチが有効な場所かどうかを判定する
static BOOL CheckSwitchOn( const ISS_SWITCH_SET* set, SWITCH_INDEX idx ); // スイッチが押されているかどうかを判定する
static ISS_SWITCH* GetSwitch( const ISS_SWITCH_SET* set, SWITCH_INDEX idx ); // スイッチを取得する
static void SetBGMVolume( const ISS_SWITCH_SET* set ); // スイッチセットの状態をBGMボリュームに反映する
static void SwitchSetMain( ISS_SWITCH_SET* set ); // スイッチセットのメイン動作
static void SwitchOn( ISS_SWITCH_SET* set, SWITCH_INDEX swIdx ); // スイッチを押す
static void SwitchOff( ISS_SWITCH_SET* set, SWITCH_INDEX swIdx ); // スイッチを離す
static void ResetSwitch( ISS_SWITCH_SET* set ); // スイッチをリセットする

static void SwitchSetBGM_VolumeUp( ISS_SWITCH_SET *set );
static void SwitchSetBGM_VolumeDown( ISS_SWITCH_SET *set );
static void IncFadeCount( ISS_SWITCH_SET *set );
static BOOL CheckFadeFinish( ISS_SWITCH_SET *set );

#ifdef DEBUG_MODE
static void Debug_PrintSwitchSet( const ISS_SWITCH_SET* set ); // スイッチの状態を出力する
#endif



//===============================================================================
// ■public method
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを生成する
 *
 * @param heapID
 *
 * @return 生成したスイッチセット
 */
//-------------------------------------------------------------------------------
ISS_SWITCH_SET* ISS_SWITCH_SET_Create( HEAPID heapID )
{
  ISS_SWITCH_SET* set;

  set = CreateSwitchSet( heapID ); // スイッチセットを生成
  InitSwitchSet( set ); // スイッチセットを初期化
  CreateSwitches( set, heapID ); // スイッチを生成
  return set;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを破棄する
 *
 * @param set
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_Delete( ISS_SWITCH_SET* set )
{
  ASSERT_REPLACE( set );

  DeleteSwitches( set ); // スイッチを破棄
  DeleteSwitchSet( set ); // スイッチセットを破棄
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチデータをロードする
 *
 * @param set
 * @param datID  読み込むデータのアーカイブ内インデックス
 * @param heapID 使用するヒープID
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_Load( ISS_SWITCH_SET* set, ARCDATID datID, HEAPID heapID )
{
  ASSERT_REPLACE( set );

  LoadSwitchData( set, datID, heapID );

#ifdef DEBUG_MODE
  Debug_PrintSwitchSet( set );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットのメイン動作
 *
 * @param set
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_Main( ISS_SWITCH_SET* set )
{
  ASSERT_REPLACE( set );

  SwitchSetMain( set );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを押す
 *
 * @param set
 * @param swIdx 押すスイッチを指定
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_SwitchOn( ISS_SWITCH_SET* set, SWITCH_INDEX swIdx )
{
  ASSERT_REPLACE( set );

  SwitchOn( set, swIdx );
}

//----------------------------------------------------------------------------
/**
 * @brief スイッチセットを押す(即時反映)
 *
 * @param set
 * @param swIdx 押すスイッチを指定
 */
//-----------------------------------------------------------------------------
void ISS_SWITCH_SET_SwitchOnForce( ISS_SWITCH_SET* set, SWITCH_INDEX idx )
{
  ISS_SWITCH* sw;

  ASSERT_REPLACE( set );
  ASSERT_REPLACE( idx != SWITCH_00 );
  ASSERT_REPLACE( idx <= SWITCH_MAX );

  sw = GetSwitch( set, idx );
  ISS_SWITCH_ForceOn( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを離す
 *
 * @param set
 * @param swIdx 離すスイッチを指定
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_SwitchOff( ISS_SWITCH_SET* set, SWITCH_INDEX swIdx )
{
  ASSERT_REPLACE( set );

  SwitchOff( set, swIdx );
}
//----------------------------------------------------------------------------
/**
 * @brief スイッチセットを離す(即時反映)
 *
 * @param set
 * @param swIdx 押すスイッチを指定
 */
//-----------------------------------------------------------------------------
void ISS_SWITCH_SET_SwitchOffForce( ISS_SWITCH_SET* set, SWITCH_INDEX idx )
{
  ISS_SWITCH* sw;

  ASSERT_REPLACE( set );
  ASSERT_REPLACE( idx != SWITCH_00 );
  ASSERT_REPLACE( idx <= SWITCH_MAX );

  sw = GetSwitch( set, idx );
  ISS_SWITCH_ForceOff( sw );
}
//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットをリセットする
 *
 * @param set
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_ResetSwitch( ISS_SWITCH_SET* set )
{
  ASSERT_REPLACE( set );

  ResetSwitch( set );
}

//-------------------------------------------------------------------------------
/**
 * @brief 対応するシーケンス番号を取得する
 *
 * @param set
 *
 * @return シーケンス番号
 */
//-------------------------------------------------------------------------------
u32 ISS_SWITCH_SET_GetSoundIdx( const ISS_SWITCH_SET* set )
{
  ASSERT_REPLACE( set );

  return GetSoundIdx( set );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットが有効な場所かどうかを判定する
 *
 * @param set
 * @param zoneID 判定する場所
 *
 * @return 指定したゾーンが, スイッチセットが有効な場所なら TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_SWITCH_SET_IsValidAt( const ISS_SWITCH_SET* set, u16 zoneID )
{ 
  ASSERT_REPLACE( set );

  return CheckValidZone( set, zoneID );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが押されているかどうかを判定する
 *
 * @param set
 *
 * @return スイッチセットが押されている場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_SWITCH_SET_CheckSwitchOn( const ISS_SWITCH_SET* set, SWITCH_INDEX idx )
{
  ASSERT_REPLACE( set );

  return CheckSwitchOn( set, idx );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットの状態をBGMボリュームに反映させる
 *
 * @param set
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SET_SetBGMVolume( const ISS_SWITCH_SET* set )
{
  ASSERT_REPLACE( set );

  SetBGMVolume( set );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スイッチセットに対する音量を設定する
 *	        ここで設定した音量は全てのスイッチに反映される
 *
 *	@param  set
 *	@param  volume    音量（0-127）
 */
//-----------------------------------------------------------------------------
void ISS_SWITCH_SET_SetVlolume( ISS_SWITCH_SET* set, const int volume, const u16 fadeFrame )
{

  // 状態変える
  if( set->volume < volume )     {set->state = SWITCH_SET_STATE_VOLUME_UP;}
  else if( set->volume > volume ){set->state = SWITCH_SET_STATE_VOLUME_DOWN;}
  else 
  {
    // 音量が同じなら変更なし
    set->state = SWITCH_SET_STATE_DEFAULT; 
    return;
  }

  // フェードフレーム数セット
  set->fadeFrame = fadeFrame;
  // 現在の音量との差
  set->tmpVolume = GFL_STD_Abs( set->volume - volume );
  // 現在の音量を最大音量として保存
  set->tmpMaxVolume = set->volume;
}

//===============================================================================
// ■private method
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを初期化する
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void InitSwitchSet( ISS_SWITCH_SET* set )
{
  int i;

  set->state = SWITCH_SET_STATE_DEFAULT;
  set->soundIdx = 0;
  set->validZoneNum = 0; 
  for( i=0; i<MAX_VALID_ZONE_NUM; i++ ) { set->validZone[i] = 0; } 
  for( i=0; i<SWITCH_NUM; i++ ) { set->IssSwitch[i] = NULL; } 
  set->volume = 127;  // 最大ボリューム
  set->tmpVolume = 127;  // 最大ボリューム
  set->tmpMaxVolume = 127;  // 最大ボリューム
  set->fadeCount = 0;
  set->fadeFrame = 0;

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: init switch-set\n", set->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを生成する
 *
 * @param heapID 使用するヒープID
 *
 * @return 生成したスイッチセット
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH_SET* CreateSwitchSet( HEAPID heapID )
{
  ISS_SWITCH_SET* set;

  // スイッチセットを生成
  set = GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SWITCH_SET) ); 

#ifdef DEBUG_MODE
  set->debugID = Debug_switchSetNum++;
#endif

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: create switch-set\n", set->debugID );
#endif

  return set;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを破棄する
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void DeleteSwitchSet( ISS_SWITCH_SET* set )
{
#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: delete switch-set\n", set->debugID );
#endif

  GFL_HEAP_FreeMemory( set ); 
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを生成する
 *
 * @param set
 * @param heapID
 */
//-------------------------------------------------------------------------------
static void CreateSwitches( ISS_SWITCH_SET* set, HEAPID heapID )
{
  int i;

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: ◇create switches\n", set->debugID );
#endif

  for( i=0; i<SWITCH_NUM; i++ )
  {
    ASSERT_REPLACE( set->IssSwitch[i] == NULL );

    set->IssSwitch[i] = ISS_SWITCH_Create( heapID );
  }

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: ◆create switches\n", set->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを破棄する
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void DeleteSwitches( ISS_SWITCH_SET* set )
{
  int i;

  for( i=0; i<SWITCH_NUM; i++ )
  {
    ISS_SWITCH_Delete( set->IssSwitch[i] );
    set->IssSwitch[i] = NULL;
  }

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: delete switches\n", set->debugID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチデータを読み込む
 *
 * @param set
 * @param datID  読み込むデータID
 * @param heapID 使用するヒープID
 */
//-------------------------------------------------------------------------------
static void LoadSwitchData( ISS_SWITCH_SET* set, ARCDATID datID, HEAPID heapID )
{ 
  int i;
  int offset = 0;
  int size = 0;
  u16 trackBit[ SWITCH_NUM ];
  u16 fadeFrame;

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "◇ISS-SWITCH-SET[%d]: load switch data(%d)\n", set->debugID, datID );
#endif

  // シーケンス番号
  size = sizeof(u32);
  GFL_ARC_LoadDataOfs( 
      &(set->soundIdx), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // 各スイッチの操作対象トラックビット
  for( i=0; i<SWITCH_NUM; i++ )
  {
    size = sizeof(u16);
    GFL_ARC_LoadDataOfs( &trackBit[i], ARCID_ISS_SWITCH, datID, offset, size );
    offset += size;
  }

  // フェード時間
  size = sizeof(u16);
  GFL_ARC_LoadDataOfs( &fadeFrame, ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // スイッチが有効な場所の数
  size = sizeof(u8);
  GFL_ARC_LoadDataOfs( 
      &(set->validZoneNum), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // スイッチが有効な場所
  size = sizeof(u16) * set->validZoneNum;
  GFL_ARC_LoadDataOfs( 
      &(set->validZone), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // 各スイッチのパラメータを設定
  for( i=0; i<SWITCH_NUM; i++ )
  {
    ISS_SWITCH_SetSwitchParam( GetSwitch(set, i), fadeFrame, trackBit[i] );
  }

#ifdef DEBUG_MODE
  OS_TFPrintf( PRINT_TARGET, 
      "◆ISS-SWITCH-SET[%d]: load switch data(%d)\n", set->debugID, datID );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットのメイン動作
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void SwitchSetMain( ISS_SWITCH_SET* set )
{
  int i;
  ISS_SWITCH* sw;

  // ボリューム制御
  switch( set->state ){
  case SWITCH_SET_STATE_VOLUME_UP:    SwitchSetBGM_VolumeUp( set );   break;
  case SWITCH_SET_STATE_VOLUME_DOWN:  SwitchSetBGM_VolumeDown( set ); break;
  case SWITCH_SET_STATE_DEFAULT:
    // 何もしない
    break;
  default: ASSERT_REPLACE(0);
  }

  for( i=0; i<SWITCH_NUM; i++ )
  {
    sw = GetSwitch( set, i );
    ISS_SWITCH_Main( sw );
  }
} 

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを押す
 *
 * @param set
 * @param swIdx 押すスイッチを指定
 */
//-------------------------------------------------------------------------------
static void SwitchOn( ISS_SWITCH_SET* set, SWITCH_INDEX swIdx )
{
  ISS_SWITCH* sw;

  ASSERT_REPLACE( swIdx != SWITCH_00 );
  ASSERT_REPLACE( swIdx <= SWITCH_MAX );

  sw = GetSwitch( set, swIdx );
  ISS_SWITCH_On( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを離す
 *
 * @param set
 * @param swIdx 押すスイッチを指定
 */
//-------------------------------------------------------------------------------
static void SwitchOff( ISS_SWITCH_SET* set, SWITCH_INDEX swIdx )
{
  ISS_SWITCH* sw;

  ASSERT_REPLACE( swIdx != SWITCH_00 );
  ASSERT_REPLACE( swIdx <= SWITCH_MAX );

  sw = GetSwitch( set, swIdx );
  ISS_SWITCH_Off( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチをリセットする ( スイッチ0のみが押されている状態にする )
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void ResetSwitch( ISS_SWITCH_SET* set )
{
  int i;
  ISS_SWITCH* sw;
  
  // 状態を初期化
  set->state = SWITCH_SET_STATE_DEFAULT;

  // 音量を最高値にする(初期化)
  set->volume = 127;
  set->tmpVolume = 127;
  set->tmpMaxVolume = 127;
  set->fadeCount = 0;
  set->fadeFrame = 0;

  // スイッチ0をONにする
  sw = GetSwitch( set, SWITCH_00 );
  ISS_SWITCH_ForceOn( sw );

  // それ以外のスイッチをOFFにする
  for( i=SWITCH_01; i<SWITCH_NUM; i++ )
  {
    sw = GetSwitch( set, i ); 
    ISS_SWITCH_ForceOff( sw );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチに対応するシーケンス番号を取得する
 *
 * @param switchSst
 *
 * @return 指定したスイッチに対応するシーケンス番号
 */
//-------------------------------------------------------------------------------
static u32 GetSoundIdx( const ISS_SWITCH_SET* switchSst )
{
  return switchSst->soundIdx;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが有効な場所の数を取得する
 *
 * @param set
 *
 * @return 指定したスイッチが有効な場所の数
 */
//-------------------------------------------------------------------------------
static u8 GetValidZoneNum( const ISS_SWITCH_SET* set )
{
  return set->validZoneNum;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが有効な場所を取得する
 *
 * @param set
 * @param idx       取得する場所のインデックス
 *
 * @return 指定したインデックスにある, スイッチ有効ゾーンのID
 */
//-------------------------------------------------------------------------------
static u16 GetValidZoneID( const ISS_SWITCH_SET* set, int idx )
{
  ASSERT_REPLACE( 0 <= idx );
  ASSERT_REPLACE( idx < GetValidZoneNum(set) );

  return set->validZone[ idx ];
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが有効な場所かどうかを判定する
 *
 * @param set
 * @param zoneID
 *
 * @return 指定したゾーンが, スイッチが有効な場所なら TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckValidZone( const ISS_SWITCH_SET* set, u16 zoneID )
{
  int i;
  int zoneNum;

  zoneNum = GetValidZoneNum( set );

  for( i=0; i < zoneNum; i++ )
  {
    if( GetValidZoneID(set, i) == zoneID ) { return TRUE; }
  } 
  return FALSE;
} 

//-------------------------------------------------------------------------------
/**
 * @brief スイッチが押されているかどうかを判定する
 *
 * @param set
 * @param idx 判定するスイッチを指定
 *
 * @return 指定したスイッチが押されている場合, TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckSwitchOn( const ISS_SWITCH_SET* set, SWITCH_INDEX idx )
{
  ISS_SWITCH* sw;

  ASSERT_REPLACE( idx <= SWITCH_MAX );

  sw = GetSwitch( set, idx );
  return ISS_SWITCH_IsOn( sw );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを取得する
 *
 * @param set
 * @param switchIdx スイッチ番号
 *
 * @return 指定したインデックスのスイッチ
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH* GetSwitch( const ISS_SWITCH_SET* set, SWITCH_INDEX switchIdx )
{
  ASSERT_REPLACE( switchIdx <= SWITCH_MAX );

  return set->IssSwitch[ switchIdx ];
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットの状態をBGMボリュームに反映する
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void SetBGMVolume( const ISS_SWITCH_SET* set )
{
  int i;
  ISS_SWITCH* sw;

  // 各スイッチの状態をBGMボリュームに反映
  for( i=SWITCH_00; i<SWITCH_NUM; i++ )
  {
    sw = GetSwitch( set, i ); 
    ISS_SWITCH_SetBGMVolume( sw, set->volume );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ボリューム上げる処理
 *
 *	@param	set
 */
//-----------------------------------------------------------------------------
static void SwitchSetBGM_VolumeUp( ISS_SWITCH_SET *set )
{
  // フェード値更新
  IncFadeCount( set );

  // ボリューム計算
  set->volume = set->tmpMaxVolume + set->tmpVolume * set->fadeCount / set->fadeFrame;

  // フェード完了チェック
  if( CheckFadeFinish( set ) == TRUE )
  {
    // 状態初期に戻す
    set->state = SWITCH_SET_STATE_DEFAULT;
    ICHI_Printf("volume %d \n", set->volume);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボリューム下げる処理
 *
 *	@param	set
 */
//-----------------------------------------------------------------------------
static void SwitchSetBGM_VolumeDown( ISS_SWITCH_SET *set )
{
  // フェード値更新
  IncFadeCount( set );

  // ボリューム計算
  set->volume = set->tmpMaxVolume - set->tmpVolume * set->fadeCount / set->fadeFrame;

  // フェード完了チェック
  if( CheckFadeFinish( set ) == TRUE )
  {
    // 状態初期に戻す
    set->state = SWITCH_SET_STATE_DEFAULT;
    ICHI_Printf("volume %d \n", set->volume);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードカウンタをインクリメントする
 *
 *	@param	set
 */
//-----------------------------------------------------------------------------
static void IncFadeCount( ISS_SWITCH_SET *set )
{
  set->fadeCount++;
  if( set->fadeFrame < set->fadeCount )
  {
    set->fadeCount = set->fadeFrame;
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
static BOOL CheckFadeFinish( ISS_SWITCH_SET *set )
{
  if( set->fadeFrame <= set->fadeCount )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

#ifdef DEBUG_MODE
//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットの状態を出力する
 *
 * @param set
 */
//-------------------------------------------------------------------------------
static void Debug_PrintSwitchSet( const ISS_SWITCH_SET* set )
{
  int i;

  // BGM番号
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: soundIdx = %d\n", set->debugID, set->soundIdx );

  // 有効ゾーンの数
  OS_TFPrintf( PRINT_TARGET, 
      "ISS-SWITCH-SET[%d]: validZoneNum = %d\n", set->debugID, set->validZoneNum );

  // 有効ゾーン
  for( i=0; i<set->validZoneNum; i++ )
  {
    OS_TFPrintf( PRINT_TARGET, 
        "ISS-SWITCH-SET[%d]: validZone[%d] = %d\n", 
        set->debugID, i, set->validZone[i] );
  }
}
#endif
