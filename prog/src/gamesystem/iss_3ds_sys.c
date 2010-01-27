/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  橋ISSシステム ( 3Dサウンドシステム )
 * @file   iss_3d_sys.c
 * @author obata
 * @date   2009.09.08
 */
/////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_3ds_sys.h"
#include "sound/pm_sndsys.h"
#include "arc/arc_def.h"


//===============================================================================
// ■定数
//===============================================================================
// デバッグ情報の出力先
#define PRINT_TARGET (1)

// 参照中データインデックスの無効値
#define CURRENT_DATA_NONE (0xff)

// 操作対象トラックの初期化ボリューム
#define INITIAL_VOLUME (0)

// 最大ボリューム
#define MAX_VOLUME (127)

// マスターボリュームの変更速度
#define MASTER_VOLUME_FADE_SPEED (2)

// 絶対値取得マクロ
#define ABS(n) (n>0 ? n : -n)


//===============================================================================
// ■1つの音源を扱う構造体 ( ユニット )
//===============================================================================
typedef struct
{ 
  BOOL    active;          // 有効かどうか
  VecFx32 pos;             // 音源の位置
  fx32    effectiveRange;  // 音が届く距離
  int     maxVolume;       // 最大音量(0～127)
  u16     trackBit;        // 操作トラックマスク

} ISS_3DS_UNIT;


//===============================================================================
// ■システムデータ
//===============================================================================
typedef struct
{
  u32 soundIdx;     // BGM No.
  u16 holdTrackBit; // システム起動時に抑えるトラックのビットマスク
  u16 padding;

} ISS_BRIDGE_DATA;


//===============================================================================
// ■システムワーク
//===============================================================================
struct _ISS_3DS_SYS
{
  HEAPID heapID;
  BOOL   boot;    // 起動しているかどうか

  u8 masterVolume;           // マスターボリューム ( 設定値 )
  u8 practicalMasterVolume;  // マスターボリューム ( 実行値 )
  u8 masterVolumeFadeSpeed;  // マスターボリュームの変更速度

  VecFx32 observerPos;        // 観測者の座標
  VecFx32 observerTargetPos;  // 観測者の注視点座標

  ISS_3DS_UNIT unit[ ISS3DS_UNIT_NUM ];  // ユニット

  ISS_BRIDGE_DATA* systemData;         // システムデータ
  u8               systemDataNum;      // システムデータの数
  u8               currentSysDataIdx;  // 参照中データのインデックス
};


//===============================================================================
// ■非公開関数
//=============================================================================== 
// システムデータ
static void InitSystemData          ( ISS_3DS_SYS* system );
static void LoadSystemData          ( ISS_3DS_SYS* system );
static void UnloadSystemData        ( ISS_3DS_SYS* system );
static void ChangeCurrentSystemData ( ISS_3DS_SYS* system, u8 nextDataIdx );
static u8   SearchSystemData        ( const ISS_3DS_SYS* system, u32 soundIdx );
// ユニット
static void InitAllUnit ( ISS_3DS_SYS* system ); 
static void RegisterUnit( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, 
                          fx32 effectiveRange, int maxVolume );
static void SetUnitPos  ( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, const VecFx32* pos );
// 観測者
static void SetObserverPos( ISS_3DS_SYS* system, const VecFx32* pos, const VecFx32* targetPos );
// システム制御
static void BootSystem( ISS_3DS_SYS* system );
static void StopSystem( ISS_3DS_SYS* system );
static void SystemMain( ISS_3DS_SYS* system );
static void ZoneChange( ISS_3DS_SYS* system );
// ボリューム / パン
static void InitTrackVolume  ( const ISS_3DS_SYS* system );
static void UpdateTrackVolume( const ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx );
static void UpdateTrackPan   ( const ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx );
static int  CalcTrackVolume  ( const ISS_3DS_UNIT* unit, const VecFx32* observerPos );
static int  CalcTrackPan     ( const ISS_3DS_UNIT* unit, const VecFx32* observerPos, 
                               const VecFx32* observerTargetPos );
// マスターボリューム
static void FadePracticalMasterVolume( ISS_3DS_SYS* system );
static u8   AdjustByMasterVolume     ( const ISS_3DS_SYS* system, u8 volume );
// デバッグ
static void DebugPrint_unit( const ISS_3DS_UNIT* unit );
static void DebugPrint_systemData( const ISS_3DS_SYS* system );


//===============================================================================
// ■システム作成/破棄/動作
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムを作成する
 *
 * @param heapID 使用するヒープID
 *
 * @return 作成したシステム
 */
//-------------------------------------------------------------------------------
ISS_3DS_SYS* ISS_3DS_SYS_Create( HEAPID heapID )
{
  ISS_3DS_SYS* system;

  // 生成
  system = GFL_HEAP_AllocMemory( heapID, sizeof(ISS_3DS_SYS) );

  // 初期化
  system->heapID = heapID;
  system->boot   = FALSE;
  system->masterVolume = MAX_VOLUME;
  system->practicalMasterVolume = MAX_VOLUME;
  system->masterVolumeFadeSpeed = MASTER_VOLUME_FADE_SPEED;
  VEC_Set( &(system->observerPos), 0, 0, 0 );
  VEC_Set( &(system->observerTargetPos), 0, 0, 0 );
  InitAllUnit( system );
  InitSystemData( system );

  // システムデータ読み込み
  LoadSystemData( system );

  return system;
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを破棄する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_Delete( ISS_3DS_SYS* system )
{
  UnloadSystemData( system );
  GFL_HEAP_FreeMemory( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムの動作処理
 *
 * @param system 動かすシステム
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_Main( ISS_3DS_SYS* system )
{
  SystemMain( system );
}


//===============================================================================
// ■システム制御
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_On( ISS_3DS_SYS* system )
{
  BootSystem( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_Off( ISS_3DS_SYS* system )
{
  StopSystem( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief ゾーンの切り替えを通知する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_ZoneChange( ISS_3DS_SYS* system )
{
  ZoneChange( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief マスターボリュームを設定する
 *
 * @param system
 * @param volume 設定するボリューム値 [0, 127]
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_SetMasterVolume( ISS_3DS_SYS* system, u8 volume )
{
  // ボリュームを補正
  if( MAX_VOLUME < volume ){ volume = MAX_VOLUME; }

  // 設定
  system->masterVolume( volume );
}


//===============================================================================
// ■ユニット操作
//=============================================================================== 

//-------------------------------------------------------------------------------
/**
 * @brief ユニットを登録する
 *
 * @param system
 * @param unitIdx        登録するユニットを指定
 * @param effectiveRange 音が聞こえる距離
 * @param maxVolume      最大ボリューム
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_RegisterUnit( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, 
                               fx32 effectiveRange, int maxVolume )
{
  RegisterUnit( system, unitIdx, effectiveRange, maxVolume );
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニットが登録済みかどうかを取得する
 *
 * @param system
 * @param unitIdx 判定するユニットを指定
 *
 * @return 指定したユニットが登録済みなら TRUE
 *         そうでないなら                 FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_3DS_SYS_IsUnitRegistered( const ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx )
{
  return system->unit[ unitIdx ].active;
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニットの座標を設定する
 *
 * @param system
 * @param unitIdx ユニットを指定
 * @param pos     設定する座標
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_SetUnitPos( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, const VecFx32* pos )
{
  SetUnitPos( system, unitIdx, pos );
}


//===============================================================================
// ■観測者 操作
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 観測者の位置を設定する
 *
 * @param system
 * @param pos       観測者の座標
 * @param targetPos 観測者の注視点座標
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_SetObserverPos( ISS_3DS_SYS* system, const VecFx32* pos, const VecFx32* targetPos )
{
  SetObserverPos( system, pos, targetPos );
}


//===============================================================================
// ■非公開関数
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムデータを初期化する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void InitSystemData( ISS_3DS_SYS* system )
{
  system->systemData        = NULL;
  system->systemDataNum     = 0;
  system->currentSysDataIdx = CURRENT_DATA_NONE;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: init system data\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムデータを読み込む
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void LoadSystemData( ISS_3DS_SYS* system )
{
  HEAPID heapID;
  ARCHANDLE* handle;
  u8 dataNum;
  u32 datID;

  GF_ASSERT( system->systemData == NULL );

  heapID  = system->heapID;
  handle  = GFL_ARC_OpenDataHandle( ARCID_ISS_BRIDGE, heapID );
  dataNum = GFL_ARC_GetDataFileCntByHandle( handle );

  // ワークを確保
  system->systemDataNum = dataNum;
  system->systemData    = GFL_HEAP_AllocMemory( heapID, sizeof(ISS_BRIDGE_DATA) * dataNum );

  // ロード
  for( datID=0; datID < dataNum; datID++ )
  {
    GFL_ARC_LoadDataByHandle( handle, datID, &(system->systemData[ datID ]) );
  } 
  GFL_ARC_CloseDataHandle( handle );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: load system data\n" );
  DebugPrint_systemData( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムデータを破棄する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void UnloadSystemData( ISS_3DS_SYS* system )
{
  GF_ASSERT( system->systemData );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: unload system data\n" );

  // 破棄 / 初期化
  GFL_HEAP_FreeMemory( system->systemData );
  InitSystemData( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief 参照するシステムデータを変更する
 *
 * @param system
 * @param nextDataIdx 新しい参照データのインデックス
 */
//-------------------------------------------------------------------------------
static void ChangeCurrentSystemData( ISS_3DS_SYS* system, u8 nextDataIdx )
{
  // 変化なし
  if( system->currentSysDataIdx == nextDataIdx ){ return; }

  // 変更
  system->currentSysDataIdx = nextDataIdx;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: update current system data index ==> %d\n", nextDataIdx ); 
}

//-------------------------------------------------------------------------------
/**
 * @brief システムデータを検索する
 *
 * @param system
 * @param soundIdx 検索するデータのキーとなるBGM No.
 *
 * @return 指定したBGMNo.に対応するシステムデータのインデックス
 *         対応するデータが無い場合 CURRENT_DATA_NONE
 */
//-------------------------------------------------------------------------------
static u8 SearchSystemData( const ISS_3DS_SYS* system, u32 soundIdx )
{
  u8 dataIdx;
  u8 dataNum;
  const ISS_BRIDGE_DATA* dataArray;
  const ISS_BRIDGE_DATA* data;

  // データが読み込まれていない
  if( system->systemData == NULL )
  {
    OS_Printf( "ISS-B: don't have system data\n" );
    GF_ASSERT(0);
    return CURRENT_DATA_NONE;
  }

  dataNum   = system->systemDataNum;
  dataArray = system->systemData;

  // 検索
  for( dataIdx=0; dataIdx < dataNum; dataIdx++ )
  {
    // 発見
    if( dataArray[ dataIdx ].soundIdx == soundIdx ){ return dataIdx; } 
  } 

  // 発見できず
  return CURRENT_DATA_NONE;
}

//-------------------------------------------------------------------------------
/**
 * @brief すべてのユニットを初期化する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void InitAllUnit( ISS_3DS_SYS* system )
{
  ISS3DS_UNIT_INDEX unitIdx;

  // 初期化
  for( unitIdx=ISS3DS_UNIT_TRACK01; unitIdx < ISS3DS_UNIT_NUM; unitIdx++ )
  {
    system->unit[ unitIdx ].active    = FALSE;
    system->unit[ unitIdx ].maxVolume = MAX_VOLUME;
    system->unit[ unitIdx ].trackBit  = ( 1 << unitIdx );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: init all unit\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニットを登録する
 *
 * @param system
 * @param unitIdx        登録するユニットを指定
 * @param effectiveRange 音が聞こえる距離
 * @param maxVolume      最大ボリューム
 */
//-------------------------------------------------------------------------------
void RegisterUnit( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, 
                   fx32 effectiveRange, int maxVolume )
{
  ISS_3DS_UNIT* unit;

  unit = &( system->unit[ unitIdx ] );

  // すでに登録済み
  if( unit->active == TRUE )
  {
    OS_Printf( "ISS-B: unit %d is already registered\n", unitIdx );
    GF_ASSERT(0);
    return;
  }

  // 登録
  unit->active         = TRUE;
  unit->effectiveRange = effectiveRange;
  unit->maxVolume      = maxVolume;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: register unit no.%d\n", unitIdx );
  DebugPrint_unit( unit );
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニットの座標を設定する
 *
 * @param system
 * @param unitIdx ユニットを指定
 * @param pos     設定する座標
 */
//-------------------------------------------------------------------------------
static void SetUnitPos( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, const VecFx32* pos )
{
  ISS_3DS_UNIT* unit;

  unit = &( system->unit[ unitIdx ] );

  // ユニットが登録されていない
  if( unit->active == FALSE )
  {
    OS_Printf( "ISS-B: unit no.%d is not registered\n", unitIdx );
    GF_ASSERT(0);
    return;
  }

  // 座標を更新
  VEC_Set( &(unit->pos), pos->x, pos->y, pos->z );
}

//-------------------------------------------------------------------------------
/**
 * @brief 観測者の位置を設定する
 *
 * @param system
 * @param pos       観測者の座標
 * @param targetPos 観測者の注視点座標
 */
//-------------------------------------------------------------------------------
static void SetObserverPos( ISS_3DS_SYS* system, const VecFx32* pos, const VecFx32* targetPos )
{
  VEC_Set( &(system->observerPos), pos->x, pos->y, pos->z );
  VEC_Set( &(system->observerTargetPos), targetPos->x, targetPos->y, targetPos->z );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @parma system
 */
//-------------------------------------------------------------------------------
static void BootSystem( ISS_3DS_SYS* system )
{
  // 起動済み
  if( system->boot ){ return; }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: boot system\n" );

  // 起動
  system->boot = TRUE; 

  // 参照するシステムデータを更新
  {
    u32 soundIdx;
    u8 nextDataIdx;

    soundIdx    = PMSND_GetBGMsoundNo();
    nextDataIdx = SearchSystemData( system, soundIdx );
    ChangeCurrentSystemData( system, nextDataIdx );
  }

  // ボリュームを初期化
  InitTrackVolume( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @parma system
 */
//-------------------------------------------------------------------------------
static void StopSystem( ISS_3DS_SYS* system )
{ 
  // 停止済み
  if( system->boot == FALSE ){ return; }

  // 停止
  system->boot = FALSE;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ISS-B: stop system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void SystemMain( ISS_3DS_SYS* system )
{
  ISS3DS_UNIT_INDEX unitIdx;

  // 起動していない
  if( system->boot == FALSE ){ return; }

  // マスターボリューム更新
  FadePracticalMasterVolume( system );

  // ボリュームとパンを更新
  for( unitIdx=ISS3DS_UNIT_TRACK01; unitIdx < ISS3DS_UNIT_NUM; unitIdx++ )
  {
    UpdateTrackVolume( system, unitIdx );
    UpdateTrackPan( system, unitIdx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief ゾーンチェンジ時の動作
 */
//-------------------------------------------------------------------------------
static void ZoneChange( ISS_3DS_SYS* system )
{
  // 全ユニット初期化
  InitAllUnit( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief BGMボリュームを初期化する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void InitTrackVolume( const ISS_3DS_SYS* system )
{
  const ISS_BRIDGE_DATA* systemData;

  // 参照するデータが無い
  if( system->currentSysDataIdx == CURRENT_DATA_NONE )
  {
    OS_Printf( "ISS-B: not found system data\n" );;
    GF_ASSERT(0);
    return;
  } 

  // ボリューム初期化
  systemData = &( system->systemData[ system->currentSysDataIdx ] );
  PMSND_ChangeBGMVolume( systemData->holdTrackBit, INITIAL_VOLUME );
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したユニットのトラックボリュームを更新する
 *
 * @param system
 * @param unitIdx ユニットを指定
 */
//-------------------------------------------------------------------------------
static void UpdateTrackVolume( const ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx )
{ 
  const ISS_3DS_UNIT* unit;
  int volume;

  unit = &( system->unit[ unitIdx ] );

  // ユニットが有効でない
  if( unit->active == FALSE ){ return; }

  // ボリューム更新
  volume = CalcTrackVolume( unit, &(system->observerPos) ); 
  volume = AdjustByMasterVolume( system, volume );
  PMSND_ChangeBGMVolume( unit->trackBit, volume );

  // DEBUG:
#if 0
  OS_TFPrintf( PRINT_TARGET, 
               "ISS-B: update track volume (unit[%d] ==> %d)\n", unitIdx, volume );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したユニットのトラックパンを更新する
 *
 * @param system
 * @param unitIdx ユニットを指定
 */
//-------------------------------------------------------------------------------
static void UpdateTrackPan( const ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx )
{
  const ISS_3DS_UNIT* unit;
  int pan;

  unit = &( system->unit[ unitIdx ] );

  // ユニットが有効でない
  if( unit->active == FALSE ){ return; }

  // パンを設定
  pan = CalcTrackPan( unit, &(system->observerPos), &(system->observerTargetPos) ); 
  PMSND_SetStatusBGM_EX( unit->trackBit, 256, 0, pan );

  // DEBUG:
#if 0
  OS_TFPrintf( PRINT_TARGET, 
               "ISS-B: update track pan (unit[%d] ==> %d)\n", unitIdx, pan );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief ボリュームを計算する
 *
 * @param unit        計算対象のユニット
 * @param observerPos 観測者の位置
 *
 * @return 指定したユニットが操作するトラックのボリューム
 */
//-------------------------------------------------------------------------------
static int CalcTrackVolume( const ISS_3DS_UNIT* unit, const VecFx32* observerPos )
{
  int volume;

  GF_ASSERT( unit );
  GF_ASSERT( unit->active );

  // ボリュームを計算
  {
    VecFx32 vecToUnit;
    fx32 dist, range;

    // 距離を計算
    VEC_Subtract( &(unit->pos), observerPos, &vecToUnit );
    dist  = VEC_Mag( &vecToUnit );
    range = unit->effectiveRange;

    // ボリューム決定
    if( range < dist )  
    { // 音が届かない距離
      volume = 0;
    }
    else                
    { // 音が届く距離
      float rate = FX_FX32_TO_F32( FX_Div( range - dist, range ) ); 
      volume     = unit->maxVolume * rate;
    }
  } 

  return volume;
}

//-------------------------------------------------------------------------------
/**
 * @brief パンを計算する
 *
 * @param unit              計算対象のユニット
 * @param observerPos       観測者の座標
 * @param observerTargetPos 観測者の注視点座標
 *
 * @return 指定したユニットが操作するトラックのパン
 */
//-------------------------------------------------------------------------------
static int CalcTrackPan( const ISS_3DS_UNIT* unit, const VecFx32* observerPos, const VecFx32* observerTargetPos )
{
  int pan;

  GF_ASSERT( unit );
  GF_ASSERT( unit->active );

  // パンを計算
  {
    VecFx32 v0, v1, v2, v3, vup, vp;
    VecFx32 cx, cy, cz;
    VecFx32 camPos, targetPos, unitPos;
    fx32 dot;

    // カメラ・注視点・音源それぞれの位置
    camPos    = *observerPos;
    targetPos = *observerTargetPos;
    unitPos   = unit->pos;

    // カメラの向きベクトルv0
    VEC_Subtract( &targetPos, &camPos, &v0 );

    // カメラ→音源ベクトルv1
    VEC_Subtract( &unitPos, &camPos, &v1 );

    // カメラ上方ベクトルvup
    VEC_Set( &vup, 0, FX32_ONE, 0 );

    // カメラ座標軸cx, cy, cz
    VEC_Normalize( &v0, &cz );
    VEC_CrossProduct( &cz, &vup, &cx );   VEC_Normalize( &cx, &cx );
    VEC_CrossProduct( &cx, &cz, &cy );    VEC_Normalize( &cy, &cy );

    // v1をカメラ座標系のxz平面へ射影したベクトルv2
    VEC_Normalize( &cy, &cy );
    dot = VEC_DotProduct( &v1, &cy );
    VEC_Set( &vp, FX_Mul(cy.x, dot), FX_Mul(cy.y, dot), FX_Mul(cy.z, dot) );
    VEC_Subtract( &v1, &vp, &v2 );  // v1のcy成分を除去
    VEC_Normalize( &v2, &v2 );

    // パン = cx・v2
    dot = VEC_DotProduct( &cx, &v2 );
    pan = 128 * FX_FX32_TO_F32(dot);

    // パンの値を補正
    if( pan < -128 ) pan = -128;
    if( 127 < pan  ) pan =  127;
  }
  
  return pan;
} 

//-------------------------------------------------------------------------------
/**
 * @brief マスターボリュームの実行値を更新する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void FadePracticalMasterVolume( ISS_3DS_SYS* system )
{
  int diff;
  int add;

  // 実行値 == 設定値
  if( system->practicalMasterVolume == system->masterVolume ){ return; }

  // 設定値までの差分
  diff = system->masterVolume - system->practicalMasterVolume;

  // 加算値を決定
  if( ABS(diff) < system->masterVolumeFadeSpeed )
  {
    add = diff;
  }
  else if( diff < 0 )
  {
    add = -( system->masterVolumeFadeSpeed );
  }
  else
  {
    add = system->masterVolumeFadeSpeed;
  }

  // マスターボリューム ( 実行値 ) 更新
  system->practicalMasterVolume += add;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, 
               "ISS-B: update master volume ==> %d\n", system->practicalMasterVolume );
}

//-------------------------------------------------------------------------------
/**
 * @brief マスターボリュームによる補正をかける
 *
 * @param system
 * @param volume 補正をかける前の音量
 *
 * @return 補正をかけた後の音量
 */
//-------------------------------------------------------------------------------
static u8 AdjustByMasterVolume( const ISS_3DS_SYS* system, u8 volume )
{
  u32 v;

  v = system->practicalMasterVolume * volume;
  v = v / MAX_VOLUME;

  GF_ASSERT( MAX_VOLUME < v );

  return v;
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニットのパラメータを出力する
 *
 * @param unit
 */
//-------------------------------------------------------------------------------
static void DebugPrint_unit( const ISS_3DS_UNIT* unit )
{
  // 有効かどうか
  OS_TFPrintf( PRINT_TARGET, "ISS-B-UNIT: active = %d\n", unit->active );

  // 座標
  OS_TFPrintf( PRINT_TARGET, 
              "ISS-B-UNIT: pos = (%d, %d, %d)\n", 
              FX_Whole(unit->pos.x), FX_Whole(unit->pos.y), FX_Whole(unit->pos.z) );

  // 音が届く距離
  OS_TFPrintf( PRINT_TARGET, 
               "ISS-B-UNIT: effectiveRange = %d\n", FX_Whole(unit->effectiveRange) );

  // 最大音量
  OS_TFPrintf( PRINT_TARGET, "ISS-B-UNIT: maxVolume = %d\n", unit->maxVolume );

  // 操作トラックマスク
  OS_TFPrintf( PRINT_TARGET, "ISS-B-UNIT: trackBit = %x\n", unit->trackBit );
} 

//-------------------------------------------------------------------------------
/**
 * @brief システムデータを表示する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void DebugPrint_systemData( const ISS_3DS_SYS* system )
{
  u8 dataNum;
  u8 dataIdx;
  u8 bitIdx;
  const ISS_BRIDGE_DATA* data;

  dataNum = system->systemDataNum;

  for( dataIdx=0; dataIdx < dataNum; dataIdx++ )
  {
    data = &( system->systemData[ dataIdx ] );

    // BGM No.
    OS_TFPrintf( PRINT_TARGET, "ISS-B-SYSTEM-DATA[%d]: soundIdx = %d\n", dataIdx, data->soundIdx );

    // ビットマスク
    OS_TFPrintf( PRINT_TARGET, "ISS-B-SYSTEM-DATA[%d]: holdTrackBit = ", dataIdx );
    for( bitIdx=0; bitIdx<16; bitIdx++ )
    {
      u16 mask = ( 1 << (15 - bitIdx) );
      if( mask & data->holdTrackBit ){ OS_TFPrintf( PRINT_TARGET, "■" ); }
      else{ OS_TFPrintf( PRINT_TARGET, "□" ); }
    }
    OS_TFPrintf( PRINT_TARGET, "\n" );
  }
}
