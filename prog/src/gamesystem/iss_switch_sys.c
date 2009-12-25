////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ISSスイッチシステム
 * @file iss_switch_sys.c
 * @author obata
 * @date 2009.11.17
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_switch_sys.h"
#include "sound/pm_sndsys.h"
#include "arc/arc_def.h"  // for ARCID_ISS_SWITCH


//==========================================================================================
// ■定数
//==========================================================================================
#define TRACK_NUM          (16)      // トラック数
#define TRACKBIT_ALL       (0xffff)  // 全トラック指定
#define MAX_VOLUME         (127)     // ボリューム最大値
#define INVALID_DATA_INDEX (0xff)    // 参照データの無効インデックス

// スイッチの状態
typedef enum{
  SWITCH_STATE_ON,       // ON
  SWITCH_STATE_OFF,      // OFF
  SWITCH_STATE_FADE_IN,  // フェード・イン
  SWITCH_STATE_FADE_OUT, // フェード・アウト
} SWITCH_STATE; 


//==========================================================================================
// ■スイッチデータ
//========================================================================================== 
typedef struct
{ 
  u16 seqNo;                // シーケンス番号
  u16 padding1;             // padding
  u16 trackBit[SWITCH_NUM]; // 各スイッチの操作対象トラックビット
  u16 padding2;             // padding
  u16 fadeFrame;            // フェード時間

} SWITCH_DATA;


//========================================================================================== 
// ■ISS-Sシステム
//========================================================================================== 
struct _ISS_SWITCH_SYS
{ 
  u8        switchDataNum;  // スイッチデータの数
  u8        switchDataIdx;  // 参照データのインデックス
  SWITCH_DATA* switchData;  // スイッチデータ配列

  BOOL         boot;                    // 起動しているかどうか
  SWITCH_STATE switchState[SWITCH_NUM]; // 各スイッチの状態
  u16          fadeCount[SWITCH_NUM];   // フェードカウンタ
};


//========================================================================================== 
// ■非公開関数のプロトタイプ宣言
//========================================================================================== 
static void LoadSwitchData( ISS_SWITCH_SYS* sys, HEAPID heap_id );
static void InitSwitch( ISS_SWITCH_SYS* sys );
static void UpdateSwitchDataIdx( ISS_SWITCH_SYS* sys );
static void BootSystem( ISS_SWITCH_SYS* sys );
static void StopSystem( ISS_SWITCH_SYS* sys );
static void SwitchOn( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx );
static void SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx );
static void SwitchUpdate( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx );
static void SwitchUpdate_FADE_IN( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx );
static void SwitchUpdate_FADE_OUT( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx );
static void AllSwitchUpdate( ISS_SWITCH_SYS* sys );
static void CheckSystemDown( ISS_SWITCH_SYS* sys );


//==========================================================================================
// ■作成・破棄
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ISSスイッチシステムを作成する
 * 
 * @param heap_id 使用するヒープID
 *
 * @return 作成したシステム
 */
//------------------------------------------------------------------------------------------
ISS_SWITCH_SYS* ISS_SWITCH_SYS_Create( HEAPID heap_id )
{
  ISS_SWITCH_SYS* sys;

  // 生成
  sys = GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_SWITCH_SYS) );
  sys->switchDataNum = 0;
  sys->switchDataIdx = INVALID_DATA_INDEX;
  sys->switchData    = NULL;
  sys->boot          = FALSE;
  // 初期化
  InitSwitch( sys );
  LoadSwitchData( sys, heap_id );

  return sys;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ISSスイッチシステムを破棄する
 *
 * @param sys 破棄するシステム
 */
//------------------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Delete( ISS_SWITCH_SYS* sys )
{ 
  GFL_HEAP_FreeMemory( sys->switchData ); // スイッチデータ
  GFL_HEAP_FreeMemory( sys );             // 本体
}


//==========================================================================================
// ■動作
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief システム・メイン動作
 *
 * @param sys 動かすシステム
 */
//------------------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Update( ISS_SWITCH_SYS* sys )
{
  // 起動してない
  if( !sys->boot ){ return; }

  // 全てのスイッチ状態を更新する
  AllSwitchUpdate( sys );
}


//==========================================================================================
// ■システム制御
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動するシステム
 */
//------------------------------------------------------------------------------------------
void ISS_SWITCH_SYS_On( ISS_SWITCH_SYS* sys )
{
  BootSystem( sys );           // 起動
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys 停止するシステム
 */
//------------------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Off( ISS_SWITCH_SYS* sys )
{
  StopSystem( sys );
}


//==========================================================================================
// ■スイッチ制御
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチを ON にする
 *
 * @param sys スイッチを押すシステム
 * @param idx 押すスイッチのインデックス
 */
//------------------------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchOn( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  SwitchOn( sys, idx );
}

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチを OFF にする
 *
 * @param sys スイッチを押すシステム
 * @param idx 放すスイッチのインデックス
 */
//------------------------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  SwitchOff( sys, idx );
}

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチの ON/OFF を取得する
 *
 * @param sys 取得対象システム
 * @param idx 判定するスイッチ番号
 *
 * @return スイッチidxが押されている場合 TRUE, そうでなければ FALSE
 */
//------------------------------------------------------------------------------------------
BOOL ISS_SWITCH_SYS_IsSwitchOn( const ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  if( SWITCH_MAX < idx )
  {
    OBATA_Printf( "==============================\n" );
    OBATA_Printf( "ISS-S: switch index range over\n" );
    OBATA_Printf( "==============================\n" );
    return FALSE;
  }
  return (sys->switchState[idx] == SWITCH_STATE_ON);
}


//========================================================================================== 
// ■非公開関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチデータを読み込む
 *
 * @param sys     読み込んだデータを保持するシステム
 * @param heap_id 使用するヒープID
 */
//------------------------------------------------------------------------------------------
static void LoadSwitchData( ISS_SWITCH_SYS* sys, HEAPID heap_id )
{
  int data_idx;
  u8 num;
  ARCHANDLE* handle;

  // アーカイブハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_ISS_SWITCH, heap_id );

  // バッファを確保
  num = GFL_ARC_GetDataFileCntByHandle( handle );
  sys->switchDataNum = num;
  sys->switchData = GFL_HEAP_AllocMemory( heap_id, sizeof(SWITCH_DATA) * num );

  // 全てのデータを取得
  for( data_idx=0; data_idx<num; data_idx++ )
  {
    GFL_ARC_LoadDataOfsByHandle( 
        handle, data_idx, 0, sizeof(SWITCH_DATA), &sys->switchData[data_idx] );
  }

  // アーカイブハンドルクローズ
  GFL_ARC_CloseDataHandle( handle );

  // DEBUG:
  OBATA_Printf( "ISS-S: load switch data\n" );
  for( data_idx=0; data_idx<num; data_idx++ )
  {
    int bit_idx;
    int swt_idx;
    SWITCH_DATA* data = &sys->switchData[data_idx];
    OBATA_Printf( "- switchData[%d].seqNo = %d\n", data_idx, data->seqNo );
    for( swt_idx=0; swt_idx<SWITCH_NUM; swt_idx++ )
    {
      OBATA_Printf( "- switchData[%d].trackBit[%d] = ", data_idx, swt_idx );
      for( bit_idx=0; bit_idx<TRACK_NUM; bit_idx++ )
      {
        if( data->trackBit[swt_idx] & (1<<(TRACK_NUM-1-bit_idx)) ) OBATA_Printf( "■" );
        else                                                       OBATA_Printf( "□" );
      }
      OBATA_Printf( "\n" );
    }
    OBATA_Printf( "- switchData[%d].fadeFrame = %d\n", data_idx, data->fadeFrame );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチを初期化する
 *
 * @param sys 初期化対象システム
 */
//------------------------------------------------------------------------------------------
static void InitSwitch( ISS_SWITCH_SYS* sys )
{
  int i;
  for( i=0; i<SWITCH_NUM; i++ )
  {
    sys->switchState[i] = SWITCH_STATE_OFF;
    sys->fadeCount[i]   = 0;
  }
  // DEBUG:
  OBATA_Printf( "ISS-S: init switch\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 参照するスイッチデータを更新する
 *
 * @param sys 更新するシステム
 */
//------------------------------------------------------------------------------------------
static void UpdateSwitchDataIdx( ISS_SWITCH_SYS* sys )
{
  u32 bgm;
  int data_idx;

  // 現在のBGMに対するスイッチデータを検索
  bgm = PMSND_GetBGMsoundNo();
  for( data_idx=0; data_idx<sys->switchDataNum; data_idx++ )
  {
    // 発見
    if( sys->switchData[data_idx].seqNo == bgm )
    {
      sys->switchDataIdx = data_idx;
      // DEBUG:
      OBATA_Printf( "ISS-S: update switch data index --> %d\n", data_idx );
      return;
    }
  }

  // 該当するデータを持っていない
  sys->switchDataIdx = INVALID_DATA_INDEX;
  OBATA_Printf( "ISS-S: don't have switch data of bgm seq %d\n", bgm );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動させるシステム
 */
//------------------------------------------------------------------------------------------
static void BootSystem( ISS_SWITCH_SYS* sys )
{
  // すでに起動している
  if( sys->boot ){ return; }

  // 起動
  sys->boot = TRUE;

  // スイッチ0のみが押されている状態に初期化
  InitSwitch( sys );
  UpdateSwitchDataIdx( sys );
  PMSND_ChangeBGMVolume( TRACKBIT_ALL, 0 );
  PMSND_ChangeBGMVolume( sys->switchData[sys->switchDataIdx].trackBit[0], MAX_VOLUME );
  sys->switchState[0] = SWITCH_STATE_ON;

  // DEBUG:
  OBATA_Printf( "ISS-S: boot\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys 停止させるシステム
 */
//------------------------------------------------------------------------------------------
static void StopSystem( ISS_SWITCH_SYS* sys )
{
  // すでに停止している
  if( !sys->boot ){ return; }

  // 停止
  sys->boot = FALSE;

  // DEBUG:
  OBATA_Printf( "ISS-S: stop\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @beirf スイッチを押す
 *
 * @param sys スイッチを押すシステム
 * @param idx 押すスイッチのインデックス
 */
//------------------------------------------------------------------------------------------
static void SwitchOn( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  // 起動していない
  if( !sys->boot ){ return; }
  // スイッチが押せる状態じゃない
  if( sys->switchState[idx] != SWITCH_STATE_OFF ){ return; }

  // スイッチON(フェードイン開始)
  sys->switchState[idx] = SWITCH_STATE_FADE_IN;
  sys->fadeCount[idx]   = 0;

  // DEBUG:
  OBATA_Printf( "ISS-S: switch %d on\n", idx );
}

//------------------------------------------------------------------------------------------
/**
 * @beirf スイッチを離す
 *
 * @param sys スイッチを押すシステム
 * @param idx 離すスイッチのインデックス
 */
//------------------------------------------------------------------------------------------
static void SwitchOff( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  // 起動していない
  if( !sys->boot ){ return; }
  // スイッチが離せる状態じゃない
  if( sys->switchState[idx] != SWITCH_STATE_ON ){ return; }

  // スイッチOFF(フェードアウト開始)
  sys->switchState[idx] = SWITCH_STATE_FADE_OUT;
  sys->fadeCount[idx]   = 0;

  // DEBUG:
  OBATA_Printf( "ISS-S: switch %d off\n", idx );
}

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を更新する
 *
 * @param sys 更新するシステム
 * @param idx 更新するスイッチを指定
 */
//------------------------------------------------------------------------------------------
static void SwitchUpdate( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  switch( sys->switchState[idx] )
  {
  case SWITCH_STATE_FADE_IN:   SwitchUpdate_FADE_IN( sys, idx );   break;
  case SWITCH_STATE_FADE_OUT:  SwitchUpdate_FADE_OUT( sys, idx );  break;
  case SWITCH_STATE_ON:
  case SWITCH_STATE_OFF:
    // 変化しない
    break;
  default:
    // エラー
    OBATA_Printf( "==================\n" );
    OBATA_Printf( "ISS-S: state error\n" ); 
    OBATA_Printf( "==================\n" );
    GF_ASSERT(0);
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を更新する(FADE_IN)
 *
 * @param sys 更新するシステム
 * @param idx 更新するスイッチを指定
 */
//------------------------------------------------------------------------------------------
static void SwitchUpdate_FADE_IN( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  SWITCH_DATA* data;

  // 参照データを取得
  data = &sys->switchData[sys->switchDataIdx];  

  // 更新
  sys->fadeCount[idx]++;
  {
    float now = (float)sys->fadeCount[idx];
    float end = (float)data->fadeFrame;
    int   vol = MAX_VOLUME * (now / end);
    PMSND_ChangeBGMVolume( data->trackBit[idx], vol );
    // DEBUG:
    OBATA_Printf( "ISS-S: fade in volume switch[%d] ==> %d\n", idx, vol );
  }
  // フェードインが終了したら ON 状態へ
  if( data->fadeFrame <= sys->fadeCount[idx] )
  {
    sys->switchState[idx] = SWITCH_STATE_ON;
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief スイッチの状態を更新する(FADE_OUT)
 *
 * @param sys 更新するシステム
 * @param idx 更新するスイッチを指定
 */
//------------------------------------------------------------------------------------------
static void SwitchUpdate_FADE_OUT( ISS_SWITCH_SYS* sys, SWITCH_INDEX idx )
{
  SWITCH_DATA* data;

  // 参照データを取得
  data = &sys->switchData[sys->switchDataIdx];  

  // 更新
  sys->fadeCount[idx]++;
  {
    float now = (float)sys->fadeCount[idx];
    float end = (float)data->fadeFrame;
    int   vol = MAX_VOLUME * ( 1.0f - (now / end) );
    PMSND_ChangeBGMVolume( data->trackBit[idx], vol );
    // DEBUG:
    OBATA_Printf( "ISS-S: fade out volume switch[%d] ==> %d\n", idx, vol );
  }
  // フェードアウトが終了したら OFF 状態へ
  if( data->fadeFrame <= sys->fadeCount[idx] )
  {
    sys->switchState[idx] = SWITCH_STATE_OFF;
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 全てのスイッチ状態を更新する
 *
 * @param sys 更新するスイッチ
 */
//------------------------------------------------------------------------------------------
static void AllSwitchUpdate( ISS_SWITCH_SYS* sys )
{
  int swt_idx;

  // 参照データが存在しない
  if( sys->switchDataNum <= sys->switchDataIdx ){ return; }

  // 全てのスイッチ状態を更新
  for( swt_idx=0; swt_idx<SWITCH_NUM; swt_idx++ )
  {
    SwitchUpdate( sys, swt_idx );
  }
}
