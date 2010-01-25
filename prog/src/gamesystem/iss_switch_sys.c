/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ISSスイッチシステム
 * @file iss_switch_sys.c
 * @author obata
 * @date 2009.11.17
 */
/////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_switch_sys.h"
#include "sound/pm_sndsys.h"
#include "arc/arc_def.h"  // for ARCID_ISS_SWITCH


//===============================================================================
// ■定数
//===============================================================================
#define PRINT_DEST       (1)       // デバッグ情報の出力先
#define TRACK_NUM        (16)      // トラック数
#define TRACKBIT_ALL     (0xffff)  // 全トラック指定
#define MAX_VOLUME       (127)     // ボリューム最大値
#define SWITCH_DATA_NONE (0xff)    // 参照データの無効インデックス

// スイッチの状態
typedef enum{
  SWITCH_STATE_ON,       // ON
  SWITCH_STATE_OFF,      // OFF
  SWITCH_STATE_FADE_IN,  // フェード イン
  SWITCH_STATE_FADE_OUT, // フェード アウト
} SWITCH_STATE; 


//===============================================================================
// ■スイッチデータ
//===============================================================================
typedef struct
{ 
  u32  soundIdx;               // シーケンス番号
  u16  trackBit[ SWITCH_NUM ]; // 各スイッチの操作対象トラックビット
  u16  fadeFrame;              // フェード時間
  u16  validZoneNum;           // スイッチが有効な場所の数
  u16* validZone;              // スイッチが有効な場所

} SWITCH_DATA;


//===============================================================================
// ■ISS-Sシステム
//===============================================================================
struct _ISS_SWITCH_SYS
{ 
  HEAPID heapID;
  BOOL   boot;    // 起動しているかどうか

  // スイッチ状態
  SWITCH_STATE switchState[ SWITCH_NUM ]; // 各スイッチの状態
  u16          fadeCount  [ SWITCH_NUM ]; // フェードカウンタ

  // スイッチデータ
  SWITCH_DATA* switchData;     // スイッチデータ配列
  u8           switchDataNum;  // スイッチデータの数
  u8           switchDataIdx;  // 参照データのインデックス
};


//===============================================================================
// ■システム制御
//===============================================================================
// システム動作
static void SystemMain( ISS_SWITCH_SYS* sys );
static void SwitchMain( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
static void SwitchMain_FADE_IN( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
static void SwitchMain_FADE_OUT( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
// システム操作
static void BootSystem( ISS_SWITCH_SYS* sys );
static void StopSystem( ISS_SWITCH_SYS* sys ); 
static void NotifyZoneChange( ISS_SWITCH_SYS* sys, u16 nextZoneID );
// スイッチ操作
static void SwitchOn ( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
static void SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );

//===============================================================================
// ■データ操作
//===============================================================================
// スイッチデータ
static u8 SearchSwitchDataIndex( const ISS_SWITCH_SYS* sys, u32 soundIdx ); 
// 参照中スイッチデータ
static const SWITCH_DATA* GetCurrentSwitchData   ( const ISS_SWITCH_SYS* sys );
static BOOL               ChangeCurrentSwitchData( ISS_SWITCH_SYS* sys );
static BOOL               CheckZoneIsValid       ( const ISS_SWITCH_SYS* sys, u16 zoneID );
// スイッチ状態
static SWITCH_STATE GetSwitchState( const ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx );
static void         SetSwitchState( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx, SWITCH_STATE switchState );
static void         InitSwitchState ( ISS_SWITCH_SYS* sys );
static void         ResetSwitchState( ISS_SWITCH_SYS* sys ); 
// BGM音量操作
static void SetBGMVolume( const ISS_SWITCH_SYS* sys );

//===============================================================================
// ■作成/破棄
//===============================================================================
// スイッチデータ
static void InitSwitchData     ( ISS_SWITCH_SYS* sys );
static void LoadAllSwitchData  ( ISS_SWITCH_SYS* sys );
static void UnloadAllSwitchData( ISS_SWITCH_SYS* sys );
static void LoadSwitchData  ( SWITCH_DATA* switchData, ARCDATID datID, HEAPID heapID );
static void UnloadSwitchData( SWITCH_DATA* switchData );

//===============================================================================
// ■デバッグ出力
//===============================================================================
static void DebugPrint_SwitchData( const ISS_SWITCH_SYS* sys );


//===============================================================================
// ■作成・破棄
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief ISSスイッチシステムを作成する
 * 
 * @param heapID 使用するヒープID
 *
 * @return 作成したシステム
 */
//-------------------------------------------------------------------------------
ISS_SWITCH_SYS* ISS_SWITCH_SYS_Create( HEAPID heapID )
{
  ISS_SWITCH_SYS* sys;

  // 生成
  sys = GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SWITCH_SYS) );

  // 初期化
  sys->heapID = heapID;
  sys->boot   = FALSE;
  InitSwitchState( sys );
  InitSwitchData( sys );

  // 初期設定
  LoadAllSwitchData( sys );
  DebugPrint_SwitchData( sys );

  return sys;
}

//-------------------------------------------------------------------------------
/**
 * @brief ISSスイッチシステムを破棄する
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Delete( ISS_SWITCH_SYS* sys )
{ 
  UnloadAllSwitchData( sys );
  GFL_HEAP_FreeMemory( sys );
}


//===============================================================================
// ■動作
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システム・メイン動作
 *
 * @param sys 動かすシステム
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Update( ISS_SWITCH_SYS* sys )
{
  SystemMain( sys );
}


//===============================================================================
// ■システム制御
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動するシステム
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_On( ISS_SWITCH_SYS* sys )
{
  BootSystem( sys );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys 停止するシステム
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Off( ISS_SWITCH_SYS* sys )
{
  StopSystem( sys );
}

//-------------------------------------------------------------------------------
/**
 * @brief ゾーンの変更を通知する
 * 
 * @param sys
 * @param zoneID 変更後のゾーンID
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_ZoneChange( ISS_SWITCH_SYS* sys, u16 zoneID )
{
  NotifyZoneChange( sys, zoneID );
}


//===============================================================================
// ■スイッチ制御
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを ON にする
 *
 * @param sys スイッチを押すシステム
 * @param idx 押すスイッチのインデックス
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchOn( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  SwitchOn( sys, idx );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを OFF にする
 *
 * @param sys スイッチを押すシステム
 * @param idx 放すスイッチのインデックス
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  SwitchOff( sys, idx );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの ON/OFF を取得する
 *
 * @param sys 取得対象システム
 * @param idx 判定するスイッチ番号
 *
 * @return スイッチidxが押されている場合 TRUE, そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_SWITCH_SYS_IsSwitchOn( const ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  if( SWITCH_MAX < idx )
  {
    OS_TFPrintf( PRINT_DEST, "ISS-S: switch index range over\n" );
    return FALSE;
  }
  return (sys->switchState[idx] == SWITCH_STATE_ON);
}


//===============================================================================
// ■非公開関数
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 指定したスイッチデータを読み込む
 *
 * @param switchData 読み込んだデータの格納先
 * @param datID      読み込むデータを指定
 * @param heapID     使用するヒープID
 */
//-------------------------------------------------------------------------------
static void LoadSwitchData( SWITCH_DATA* switchData, ARCDATID datID, HEAPID heapID )
{
  int offset = 0;
  int size = 0;

  // シーケンス番号
  size = sizeof(u32);
  GFL_ARC_LoadDataOfs( &(switchData->soundIdx), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // 各スイッチの操作対象トラックビット
  size = sizeof(u16) * SWITCH_NUM;
  GFL_ARC_LoadDataOfs( &(switchData->trackBit), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // フェード時間
  size = sizeof(u16);
  GFL_ARC_LoadDataOfs( &(switchData->fadeFrame), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // スイッチが有効な場所の数
  size = sizeof(u16);
  GFL_ARC_LoadDataOfs( &(switchData->validZoneNum), ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;

  // スイッチが有効な場所
  size = sizeof(u16) * switchData->validZoneNum;
  switchData->validZone = GFL_HEAP_AllocMemory( heapID, size );
  GFL_ARC_LoadDataOfs( switchData->validZone, ARCID_ISS_SWITCH, datID, offset, size );
  offset += size;
}

//-------------------------------------------------------------------------------
/**
 * @brief 読み込んだスイッチデータを破棄する
 *
 * @param switchData 破棄するデータ
 */
//-------------------------------------------------------------------------------
static void UnloadSwitchData( SWITCH_DATA* switchData )
{
  if( switchData == NULL ){ return; }

  if( switchData->validZone )
  {
    GFL_HEAP_FreeMemory( switchData->validZone );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在参照中のスイッチデータを取得する
 *
 * @param sys
 *
 * @return 参照中のスイッチデータ
 *         参照中のデータがない場合 NULL
 */
//-------------------------------------------------------------------------------
static const SWITCH_DATA* GetCurrentSwitchData( const ISS_SWITCH_SYS* sys )
{
  // 参照中のデータがない
  if( sys->switchDataIdx == SWITCH_DATA_NONE ){ return NULL; }

  // 参照中のデータを返す
  return &( sys->switchData[ sys->switchDataIdx ] );
}

//-------------------------------------------------------------------------------
/**
 * @brief 参照するスイッチデータを更新する
 *
 * @param sys
 *
 * @return 参照データが変化したら TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL ChangeCurrentSwitchData( ISS_SWITCH_SYS* sys )
{
  u32 soundIdx;
  u8 nextDataIdx;

  // 再生中のBGMのスイッチデータに変更
  soundIdx    = PMSND_GetBGMsoundNo(); 
  nextDataIdx = SearchSwitchDataIndex( sys, soundIdx );

  if( nextDataIdx == SWITCH_DATA_NONE ){ return FALSE; } // データが存在しない
  if( nextDataIdx == sys->switchDataIdx ){ return FALSE; } // 変化なし

  // 更新
  sys->switchDataIdx = nextDataIdx;

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, 
               "ISS-S: change switch data index ==> %d\n", nextDataIdx );

  return TRUE;
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンがスイッチ状態維持の対象かどうかを調べる
 *
 * @param sys
 * @param zoneID 調べるゾーン
 *
 * @return スイッチ状態維持の対象なら TRUE
 *         そうでないなら FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckZoneIsValid ( const ISS_SWITCH_SYS* sys, u16 zoneID )
{
  int i;
  const SWITCH_DATA* switchData;

  // 参照中のスイッチデータ取得
  switchData = GetCurrentSwitchData( sys );

  if( switchData == NULL ){ return NULL; } // 参照中のデータが存在しない

  // データを検索
  for( i=0; i < switchData->validZoneNum; i++ )
  {
    // 発見
    if( switchData->validZone[i] == zoneID ){ return TRUE; }
  }
  return FALSE;
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したBGMに対応するスイッチデータのインデックスを検索する
 *
 * @param sys
 * @param soundIdx データのインデックスを取得するBGM
 *
 * @return 指定したBGMのスイッチデータのインデックス
 *         スイッチデータが存在しない場合 SWITCH_DATA_NONE
 */
//-------------------------------------------------------------------------------
static u8 SearchSwitchDataIndex( const ISS_SWITCH_SYS* sys, u32 soundIdx )
{
  SWITCH_DATA* switchData;
  u8 dataNum;
  u8 dataIdx;

  switchData = sys->switchData;
  dataNum    = sys->switchDataNum;

  // 検索
  for( dataIdx=0; dataIdx < dataNum; dataIdx++ )
  {
    // 発見
    if( switchData[ dataIdx ].soundIdx == soundIdx )
    {
      return dataIdx;
    }
  }

  // データがない
  return SWITCH_DATA_NONE;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチデータを初期化する
 * 
 * @param sys
 */
//-------------------------------------------------------------------------------
static void InitSwitchData( ISS_SWITCH_SYS* sys )
{
  sys->switchData    = NULL;
  sys->switchDataNum = 0;
  sys->switchDataIdx = SWITCH_DATA_NONE;

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: init switch data\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief すべてのスイッチデータを読み込む
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void LoadAllSwitchData( ISS_SWITCH_SYS* sys )
{
  int dataIdx;
  u8 dataNum;
  ARCHANDLE* handle;

  // アーカイブハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_ISS_SWITCH, sys->heapID );

  // バッファを確保
  dataNum = GFL_ARC_GetDataFileCntByHandle( handle );
  sys->switchDataNum = dataNum;
  sys->switchData    = GFL_HEAP_AllocMemory( sys->heapID, sizeof(SWITCH_DATA) * dataNum );

  // アーカイブハンドルクローズ
  GFL_ARC_CloseDataHandle( handle );

  // 全てのデータを読み込む
  for( dataIdx=0; dataIdx<dataNum; dataIdx++ )
  {
    LoadSwitchData( &(sys->switchData[ dataIdx ]), dataIdx, sys->heapID );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: load all switch data\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 読み込んだ全てのスイッチデータを破棄する
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void UnloadAllSwitchData( ISS_SWITCH_SYS* sys )
{
  int dataIdx;
  int dataNum;
  SWITCH_DATA* data;

  dataNum = sys->switchDataNum;
  data    = sys->switchData;

  // 配列要素を破棄
  for( dataIdx=0; dataIdx < dataNum; dataIdx++ )
  {
    UnloadSwitchData( &(data[ dataIdx ]) );
  }
  // 配列本体を破棄
  GFL_HEAP_FreeMemory( data );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: unload all switch data\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を取得する
 *
 * @param sys
 * @param switchIdx 状態を取得するスイッチを指定
 *
 * @return 指定したスイッチの状態
 */
//-------------------------------------------------------------------------------
static SWITCH_STATE GetSwitchState( const ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx )
{
  // インデックス エラー
  if( SWITCH_NUM < switchIdx )
  {
    OS_Printf( "ISS-S: switch index error\n" );
    GF_ASSERT(0);
    return SWITCH_STATE_OFF;
  }

  return sys->switchState[ switchIdx ];
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を設定する
 *
 * @param sys
 * @param switchIdx   状態を取得するスイッチを指定
 * @param switchState 設定する状態
 */
//-------------------------------------------------------------------------------
static void SetSwitchState( ISS_SWITCH_SYS* sys, 
                            SWITCH_INDEX switchIdx, SWITCH_STATE switchState )
{
  // インデックス エラー
  if( SWITCH_NUM < switchIdx )
  {
    OS_Printf( "ISS-S: switch index error\n" );
    GF_ASSERT(0);
    return;
  }

  sys->switchState[ switchIdx ] = switchState;
  sys->fadeCount  [ switchIdx ] = 0;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチ状態を初期化する
 *
 * @param sys 初期化対象システム
 */
//-------------------------------------------------------------------------------
static void InitSwitchState( ISS_SWITCH_SYS* sys )
{
  SWITCH_INDEX switchIdx;

  // 全スイッチを初期化
  for( switchIdx=SWITCH_00; switchIdx < SWITCH_NUM; switchIdx++ )
  {
    SetSwitchState( sys, switchIdx, SWITCH_STATE_OFF );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: init switch state\n" );
} 

//-------------------------------------------------------------------------------
/**
 * @brief スイッチ0のみが押されている状態にリセットする
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void ResetSwitchState( ISS_SWITCH_SYS* sys )
{
  InitSwitchState( sys );
  SetSwitchState( sys, SWITCH_00, SWITCH_STATE_ON );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: reset switch state\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief システム起動
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void BootSystem( ISS_SWITCH_SYS* sys )
{
  if( sys->boot ){ return; } // すでに起動している

  // 起動
  sys->boot = TRUE;

  // 参照するスイッチデータが変わったら, リセットする
  if( ChangeCurrentSwitchData( sys ) ){ ResetSwitchState( sys ); }

  // BGM音量を設定
  SetBGMVolume( sys );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: boot system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys 停止させるシステム
 */
//-------------------------------------------------------------------------------
static void StopSystem( ISS_SWITCH_SYS* sys )
{
  if( sys->boot == FALSE ){ return; } // すでに停止している

  // 停止
  sys->boot = FALSE;

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: stop system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief ゾーンの変化を通知する
 *
 * @param sys
 * @param nextZoneID 変更後のゾーンID
 */
//-------------------------------------------------------------------------------
static void NotifyZoneChange( ISS_SWITCH_SYS* sys, u16 nextZoneID )
{
  // スイッチ状態維持 有効ゾーン
  if( CheckZoneIsValid( sys, nextZoneID ) )
  {
    // DEBUG:
    OS_TFPrintf( PRINT_DEST, "ISS-S: detect zone change ==> valid zone\n" );
    return;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: detect zone change ==> invalid zone\n" );

  // リセット ( BGM 音量操作なし )
  ResetSwitchState( sys ); 
}

//-------------------------------------------------------------------------------
/**
 * @beirf スイッチを押す
 *
 * @param sys 
 * @param switchIdx 押すスイッチのインデックス
 */
//-------------------------------------------------------------------------------
static void SwitchOn( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx )
{
  // 起動していない
  if( !sys->boot ){ return; }
  // スイッチが押せる状態じゃない
  if( GetSwitchState( sys, switchIdx ) != SWITCH_STATE_OFF ){ return; }

  // スイッチON ( フェードイン開始 )
  SetSwitchState( sys, switchIdx, SWITCH_STATE_FADE_IN );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: switch %d on\n", switchIdx );
}

//-------------------------------------------------------------------------------
/**
 * @beirf スイッチを離す
 *
 * @param sys
 * @param switchIdx 離すスイッチのインデックス
 */
//-------------------------------------------------------------------------------
static void SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx )
{
  // 起動していない
  if( !sys->boot ){ return; }
  // スイッチが離せる状態じゃない
  if( GetSwitchState( sys, switchIdx ) != SWITCH_STATE_ON ){ return; }

  // スイッチOFF ( フェードアウト開始 )
  SetSwitchState( sys, switchIdx, SWITCH_STATE_FADE_OUT );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-S: switch %d off\n", switchIdx );
}

//-------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void SystemMain( ISS_SWITCH_SYS* sys )
{
  SWITCH_INDEX switchIdx;
  
  if( sys->boot == FALSE ){ return; } // 起動していない
  if( GetCurrentSwitchData( sys ) == NULL ){ return; } // 参照データがない

  // スイッチ動作
  for( switchIdx=SWITCH_00; switchIdx < SWITCH_NUM; switchIdx++ )
  {
    SwitchMain( sys, switchIdx );
  }

  // スイッチの状態をBGMに反映
  SetBGMVolume( sys );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチ動作
 *
 * @param sys 
 * @param switchIdx 更新するスイッチを指定
 */
//-------------------------------------------------------------------------------
static void SwitchMain( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx )
{
  switch( sys->switchState[ switchIdx ] )
  {
  case SWITCH_STATE_ON:
  case SWITCH_STATE_OFF:
    // 変化しない
    break;
  case SWITCH_STATE_FADE_IN:   SwitchMain_FADE_IN( sys, switchIdx );   break;
  case SWITCH_STATE_FADE_OUT:  SwitchMain_FADE_OUT( sys, switchIdx );  break;
  default:
    // エラー
    OS_TFPrintf( PRINT_DEST, "ISS-S: switch state error\n" ); 
    GF_ASSERT(0);
    break;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチ動作 ( FADE_IN )
 *
 * @param sys
 * @param switchIdx 動作するスイッチを指定
 */
//-------------------------------------------------------------------------------
static void SwitchMain_FADE_IN( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx )
{
  const SWITCH_DATA* switchData;

  // 参照データ取得
  switchData = GetCurrentSwitchData( sys );

  // 参照データが存在しない
  if( switchData == NULL )
  {
    OS_Printf( "ISS-S: switch data not found\n" );
    GF_ASSERT(0);
    return;
  }

  // フェードカウント更新
  sys->fadeCount[ switchIdx ]++;

  // フェードイン終了
  if( switchData->fadeFrame <= sys->fadeCount[ switchIdx ] )
  {
    // ON
    SetSwitchState( sys, switchIdx, SWITCH_STATE_ON );

    // DEBUG:
    OS_TFPrintf( PRINT_DEST, "ISS-S: switch %d fade in finish\n", switchIdx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチ動作 ( FADE_OUT )
 *
 * @param sys
 * @param switchIdx 動作するスイッチを指定
 */
//-------------------------------------------------------------------------------
static void SwitchMain_FADE_OUT( ISS_SWITCH_SYS* sys, SWITCH_INDEX switchIdx )
{
  const SWITCH_DATA* switchData;

  // 参照データ
  switchData = GetCurrentSwitchData( sys ); 

  // 参照データが存在しない
  if( switchData == NULL )
  {
    OS_Printf( "ISS-S: switch data not found\n" );
    GF_ASSERT(0);
    return;
  }

  // フェードカウント更新
  sys->fadeCount[ switchIdx ]++;

  // フェードアウト終了
  if( switchData->fadeFrame <= sys->fadeCount[ switchIdx ] )
  {
    // OFF
    SetSwitchState( sys, switchIdx, SWITCH_STATE_OFF );

    // DEBUG:
    OS_TFPrintf( PRINT_DEST, "ISS-S: switch %d fade out finish\n", switchIdx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在のスイッチの状態をBGMの音量に反映する
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void SetBGMVolume( const ISS_SWITCH_SYS* sys )
{
  int switchIdx;
  const SWITCH_DATA* switchData;

  // 参照データ
  switchData = GetCurrentSwitchData( sys );

  // 各スイッチの状態を反映する
  for( switchIdx=0; switchIdx < SWITCH_NUM; switchIdx++ )
  {
    switch( sys->switchState[ switchIdx ] )
    {
    case SWITCH_STATE_ON:
        PMSND_ChangeBGMVolume( switchData->trackBit[ switchIdx ], MAX_VOLUME );
        break;
    case SWITCH_STATE_OFF:
        PMSND_ChangeBGMVolume( switchData->trackBit[ switchIdx ], 0 );
        break;
    case SWITCH_STATE_FADE_IN:
      { // フェードイン中
        float now = (float)sys->fadeCount[ switchIdx ];
        float end = (float)switchData->fadeFrame;
        int   vol = MAX_VOLUME * (now / end);
        PMSND_ChangeBGMVolume( switchData->trackBit[ switchIdx ], vol );
      }
      break;
    case SWITCH_STATE_FADE_OUT:
      { // フェードアウト中
        float now = (float)sys->fadeCount[ switchIdx ];
        float end = (float)switchData->fadeFrame;
        int   vol = MAX_VOLUME * ( 1.0f - (now / end) );
        PMSND_ChangeBGMVolume( switchData->trackBit[ switchIdx ], vol );
      }
      break;
    }
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチデータを出力する
 *
 * @param sys
 */
//-------------------------------------------------------------------------------
static void DebugPrint_SwitchData( const ISS_SWITCH_SYS* sys )
{
  int dataIdx;
  int bitIdx;
  int switchIdx;
  int validZoneIdx;

  for( dataIdx=0; dataIdx < sys->switchDataNum; dataIdx++ )
  {
    SWITCH_DATA* switchData = &sys->switchData[ dataIdx ];

    // シーケンス番号
    OS_TFPrintf( PRINT_DEST, 
                 "- switchData[%d].soundIdx = %d\n", dataIdx, switchData->soundIdx );

    // 各スイッチのビットマスク
    for( switchIdx=0; switchIdx < SWITCH_NUM; switchIdx++ )
    {
      OS_TFPrintf( PRINT_DEST, 
                   "- switchData[%d].trackBit[%d] = ", dataIdx, switchIdx );

      for( bitIdx=0; bitIdx < TRACK_NUM; bitIdx++ )
      {
        if( switchData->trackBit[switchIdx] & (1<<(TRACK_NUM-1-bitIdx)) )
        {
          OS_TFPrintf( PRINT_DEST, "■" ); 
        }
        else
        { 
          OS_TFPrintf( PRINT_DEST, "□" ); 
        }
      }
      OS_TFPrintf( PRINT_DEST, "\n" );
    }

    // フェード フレーム数
    OS_TFPrintf( PRINT_DEST, 
                "- switchData[%d].fadeFrame = %d\n", dataIdx, switchData->fadeFrame );

    // スイッチが有効な場所
    OS_TFPrintf( PRINT_DEST, 
                 "- switchData[%d].validZoneNum = %d\n", dataIdx, switchData->validZoneNum ); 
    for( validZoneIdx=0; validZoneIdx<switchData->validZoneNum; validZoneIdx++ )
    {
      OS_TFPrintf( PRINT_DEST, 
                   "- switchData[%d].validZone[%d] = %d\n", 
                   dataIdx, validZoneIdx, switchData->validZone[ validZoneIdx ] ); 
    }
  }
}
