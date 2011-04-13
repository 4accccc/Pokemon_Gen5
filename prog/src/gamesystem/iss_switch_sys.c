/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ISSスイッチシステム
 * @file iss_switch_sys.c
 * @author obata
 * @date 2009.11.17
 */
/////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_switch_set.h"
#include "iss_switch_sys.h"

#include "sound/pm_sndsys.h" // for PMSND_xxxx
#include "arc/arc_def.h"     // for ARCID_ISS_SWITCH


//===============================================================================
// ■定数
//===============================================================================
//#define DEBUG_PRINT_ON           // デバッグ出力スイッチ
#define PRINT_DEST           (1) // デバッグ情報の出力先
#define TRACK_NUM           (16) // トラック数
#define TRACKBIT_ALL    (0xffff) // 全トラック指定
#define RESERVE_MAX     (3)  //予約数

//===============================================================================
// ■ISS-S予約
//===============================================================================
typedef struct
{
  BOOL          is_use;
  BOOL          is_on;
  u32           bgm;
  SWITCH_INDEX  switchIdx;
}ISS_SWITCH_RESERVE;

//===============================================================================
// ■ISS-Sシステム
//===============================================================================
struct _ISS_SWITCH_SYS
{ 
  BOOL             bootFlag;     // 起動しているかどうか
  u8               switchSetNum; // スイッチセットの数
  ISS_SWITCH_SET** switchSet;    // スイッチセット
  ISS_SWITCH_SET*  curSwitchSet; // 操作中のスイッチセット

  ISS_SWITCH_RESERVE  reserveData[RESERVE_MAX]; //予約情報
};


//===============================================================================
// ■関数インデックス
//===============================================================================
static void InitializeSystem( ISS_SWITCH_SYS* system ); // システムを初期化する
static ISS_SWITCH_SYS* CreateSystem( HEAPID heapID ); // システムを生成する
static void DeleteSystem( ISS_SWITCH_SYS* system ); // システムを破棄する
static void CreateSwitchSet( ISS_SWITCH_SYS* system, HEAPID heapID ); // スイッチセットを生成する
static void DeleteSwitchSet( ISS_SWITCH_SYS* system ); // スイッチセットを破棄する
static u8 GetSwitchSetNum( const ISS_SWITCH_SYS* system ); // スイッチの数を取得する
static ISS_SWITCH_SET* GetSwitchSet( const ISS_SWITCH_SYS* system, u8 setIdx ); // スイッチセットを取得する
static ISS_SWITCH_SET* GetCurrentSwitchSet( const ISS_SWITCH_SYS* system ); // 操作中のスイッチセットを取得する
static BOOL CheckCurrentSwitchSetExist( const ISS_SWITCH_SYS* system ); // 操作中のスイッチセットがあるかどうかを判定する
static BOOL CheckCurrentSwitchSetValid( const ISS_SWITCH_SYS* system, u16 zoneID ); // 現在のスイッチセットが指定したゾーンに対応しているかどうかをチェックする
static BOOL CheckSwitchSetExist( const ISS_SWITCH_SYS* system, u32 soundIdx ); // 指定したBGMに対応するスイッチセットがあるかどうかを判定する
static ISS_SWITCH_SET* SearchSwitchSet( const ISS_SWITCH_SYS* system, u32 soundIdx ); // 指定したBGMに対応するスイッチセットを検索する
static BOOL ChangeSwitchSet( ISS_SWITCH_SYS* system, u32 soundIdx ); // 指定したBGMに対応するスイッチセットに変更する
static void SetBGMVolume( const ISS_SWITCH_SYS* system ); // 現在のスイッチセットをBGMボリュームに反映させる
static u32 GetCurrentBGM( void ); // 現在のBGMを取得する
static BOOL CheckSystemBoot( const ISS_SWITCH_SYS* system ); // システムが起動しているかどうかをチェックする
static void SystemMain( ISS_SWITCH_SYS* system ); // システムメイン動作
static void BootSystem( ISS_SWITCH_SYS* system ); // システムを起動する
static void StopSystem( ISS_SWITCH_SYS* system ); // システムを終了する
static void ZoneChange( ISS_SWITCH_SYS* system, u16 nextZoneID ); // ゾーン変更処理
static void ResetCurrentSwitchSet( ISS_SWITCH_SYS* system ); // 現在のスイッチセットをリセットする
static void ReserveSwitchSet( ISS_SWITCH_SYS* system, SWITCH_INDEX switchIdx, BOOL is_on, u16 bgm );//スイッチを予約する

static void ISS_SWITCH_RESERVE_Set( ISS_SWITCH_RESERVE *wk, SWITCH_INDEX switchIdx, BOOL is_on, u16 bgm );
static void ISS_SWITCH_RESERVE_Clear( ISS_SWITCH_RESERVE *wk );
static void ISS_SWITCH_RESERVE_Main( ISS_SWITCH_RESERVE *wk, ISS_SWITCH_SYS* system );
static BOOL ISS_SWITCH_RESERVE_IsUse( const ISS_SWITCH_RESERVE *wk );

//===============================================================================
// ■public method
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
  ISS_SWITCH_SYS* system;

  system = CreateSystem( heapID ); // システムを生成
  InitializeSystem( system );      // システムを初期化
  CreateSwitchSet( system, heapID ); // スイッチセットを生成

  return system;
}

//-------------------------------------------------------------------------------
/**
 * @brief ISSスイッチシステムを破棄する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Delete( ISS_SWITCH_SYS* system )
{ 
  DeleteSwitchSet( system );
  DeleteSystem( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システム・メイン動作
 *
 * @param system 動かすシステム
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Update( ISS_SWITCH_SYS* system )
{
  // 停止中
  if( CheckSystemBoot(system) == FALSE ) { return; }
  
  {
    int i;
    for( i= 0; i < RESERVE_MAX; i++ )
    {
      ISS_SWITCH_RESERVE *wk  = &system->reserveData[ i ];
      if( ISS_SWITCH_RESERVE_IsUse( wk ) )
      {
        ISS_SWITCH_RESERVE_Main( wk, system );
      }
    }
  }

  SystemMain( system );
  SetBGMVolume( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system 起動するシステム
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_On( ISS_SWITCH_SYS* system )
{
  BootSystem( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param system 停止するシステム
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_Off( ISS_SWITCH_SYS* system )
{
  StopSystem( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief ゾーンの変更を通知する
 * 
 * @param system
 * @param zoneID 変更後のゾーンID
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_ZoneChange( ISS_SWITCH_SYS* system, u16 zoneID )
{
  ZoneChange( system, zoneID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スイッチセットの音量を設定する
 *
 *	@param	system
 *	@param  volume  音量 0-127
 */
//-----------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchSetVolume( ISS_SWITCH_SYS* system, const int volume, const u16 fadeFrame )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  if( set ) { 
    ISS_SWITCH_SET_SetVlolume( set, volume, fadeFrame );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを ON にする
 *
 * @param system スイッチを押すシステム
 * @param idx    押すボタンインデックス
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchOn( ISS_SWITCH_SYS* system, SWITCH_INDEX idx )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  if( set ) { 
    ISS_SWITCH_SET_SwitchOn( set, idx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチを OFF にする
 *
 * @param system スイッチを押すシステム
 * @param idx 放すスイッチのインデックス
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_SwitchOff( ISS_SWITCH_SYS* system, SWITCH_INDEX idx )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  if( set ) { 
    ISS_SWITCH_SET_SwitchOff( set, idx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチの ON/OFF を取得する
 *
 * @param system 取得対象システム
 * @param idx    判定するスイッチ番号
 *
 * @return スイッチidxが押されている場合 TRUE, そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
BOOL ISS_SWITCH_SYS_IsSwitchOn( const ISS_SWITCH_SYS* system, SWITCH_INDEX idx )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  if( set ) { 
    return ISS_SWITCH_SET_CheckSwitchOn( set, idx );
  }
  else {
    return FALSE;
  }
} 


//-------------------------------------------------------------------------------
/**
 * @brief スイッチの状態をリセットする
 *
 * @param system
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_ResetSwitch( ISS_SWITCH_SYS* system )
{ 
  ResetCurrentSwitchSet( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチのOn,OFFを予約する
 *
 * @param system
 * @param switchIdx 予約するスイッチ番号
 * @param is_on     TRUEでONを予約  FALSEでOFFを予約
 * @param bgm       予約先のBGM番号
 */
//-------------------------------------------------------------------------------
void ISS_SWITCH_SYS_ReserveSwitch( ISS_SWITCH_SYS* system, SWITCH_INDEX switchIdx, BOOL is_on, u16 bgm )
{
  ReserveSwitchSet( system, switchIdx, is_on, bgm );
}

//----------------------------------------------------------------------------
/**
 *	@brief  予約設定をクリア
 *
 *	@param	ISS_SWITCH_SYS* system 
 */
//-----------------------------------------------------------------------------
void ISS_SWITCH_SYS_ClearReserve( ISS_SWITCH_SYS* system )
{
  int i;
  for( i = 0; i < RESERVE_MAX; i++ )
  {
    ISS_SWITCH_RESERVE *wk  = &system->reserveData[ i ];
    ISS_SWITCH_RESERVE_Clear( wk );
  }
}

//===============================================================================
// ■private method
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムを初期化する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void InitializeSystem( ISS_SWITCH_SYS* system )
{
  system->bootFlag      = FALSE;
  system->switchSetNum  = 0;
  system->switchSet     = NULL;
  system->curSwitchSet  = NULL;

  {
    int i;
    for( i = 0; i < RESERVE_MAX; i++ )
    {
      ISS_SWITCH_RESERVE_Clear( &system->reserveData[i] );
    }
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: init system\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを生成する
 *
 * @param heapID
 *
 * @return 生成したシステム
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH_SYS* CreateSystem( HEAPID heapID )
{
  ISS_SWITCH_SYS* system;

  system = GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SWITCH_SYS) );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: create system\n" );
#endif

  return system;
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを破棄する*
 * @param system
 */
//-------------------------------------------------------------------------------
static void DeleteSystem( ISS_SWITCH_SYS* system )
{
  GF_ASSERT( system );

  GFL_HEAP_FreeMemory( system );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: delete system\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを生成する
 *
 * @param system
 * @param heapID
 */
//-------------------------------------------------------------------------------
static void CreateSwitchSet( ISS_SWITCH_SYS* system, HEAPID heapID )
{
  int datID;
  int num;
  ISS_SWITCH_SET* set;

  GF_ASSERT( system->switchSet == NULL );

  // データの数を取得
  num = GFL_ARC_GetDataFileCnt( ARCID_ISS_SWITCH );
  system->switchSetNum = num;

  // ワークを確保
  system->switchSet = 
    GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SWITCH_SET*)*num );

  // スイッチセットを生成
  for( datID=0; datID < num; datID++ )
  {
    set = ISS_SWITCH_SET_Create( heapID );
    ISS_SWITCH_SET_Load( set, datID, heapID );
    system->switchSet[ datID ] = set;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: create switch-set\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを破棄する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void DeleteSwitchSet( ISS_SWITCH_SYS* system )
{
  int i;
  int num;

  GF_ASSERT( system );
  GF_ASSERT( system->switchSet );

  num = system->switchSetNum;

  // 全スイッチを破棄
  for( i=0; i<num; i++ )
  {
    ISS_SWITCH_SET_Delete( system->switchSet[i] );
  } 
  // ワークを破棄
  GFL_HEAP_FreeMemory( system->switchSet );

  // クリア
  system->switchSetNum  = 0;
  system->switchSet     = NULL;
  system->curSwitchSet  = NULL;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: delete switch-set\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief システムメイン動作
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void SystemMain( ISS_SWITCH_SYS* system )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  if( set ) {
    ISS_SWITCH_SET_Main( set );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void BootSystem( ISS_SWITCH_SYS* system )
{
  // 起動済み
  if( CheckSystemBoot(system) == TRUE ) { return; }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "◇ISS-S: boot system\n" );
#endif

  // 起動する
  system->bootFlag = TRUE;

  // スイッチが変化したら, スイッチをリセットする
  if( ChangeSwitchSet( system, GetCurrentBGM() ) == TRUE )
  {
    ResetCurrentSwitchSet( system );
    SetBGMVolume( system );
  } 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "◆ISS-S: boot system\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを終了する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void StopSystem( ISS_SWITCH_SYS* system )
{
  // 停止済み
  if( CheckSystemBoot(system) == FALSE ) { return; }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "◇ISS-S: down system\n" );
#endif

  // 停止する
  system->bootFlag = FALSE;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "◆ISS-S: down system\n" );
#endif
}

//-------------------------------------------------------------------------------
/**
 * @brief ゾーン変更処理
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-------------------------------------------------------------------------------
static void ZoneChange( ISS_SWITCH_SYS* system, u16 nextZoneID )
{
  // 停止中
  if( CheckSystemBoot(system) == FALSE ) { return; }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: ◇zone change\n" );
#endif

  // スイッチ維持ゾーンからはずれた
  if( CheckCurrentSwitchSetValid( system, nextZoneID ) == FALSE ) { 
    ResetCurrentSwitchSet( system );
    StopSystem( system );
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-S: ◆zone change\n" );
#endif
} 

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットの数を取得する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static u8 GetSwitchSetNum( const ISS_SWITCH_SYS* system )
{
  return system->switchSetNum;
}

//-------------------------------------------------------------------------------
/**
 * @brief スイッチセットを取得する
 *
 * @param system 
 * @param setIdx スイッチセットのインデックス
 *
 * @return 指定したインデックスのスイッチセット
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH_SET* GetSwitchSet( const ISS_SWITCH_SYS* system, u8 setIdx )
{ 
  GF_ASSERT( setIdx < system->switchSetNum );

  return system->switchSet[ setIdx ];
}

//-------------------------------------------------------------------------------
/**
 * @brief 操作中のスイッチセットを取得する
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH_SET* GetCurrentSwitchSet( const ISS_SWITCH_SYS* system )
{
  return system->curSwitchSet;
} 

//-------------------------------------------------------------------------------
/**
 * @brief 操作中のスイッチセットがあるかどうかを判定する
 *
 * @param system
 *
 * @return 操作中のスイッチがある場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckCurrentSwitchSetExist( const ISS_SWITCH_SYS* system )
{
  if( GetCurrentSwitchSet(system) ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在のスイッチセットが, 
 *        指定したゾーンに対応しているかどうかをチェックする
 *
 * @param system
 * @param zoneID 判定するゾーンID
 *
 * @return 指定したゾーンが, 現在のスイッチセットに対応している場合, TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckCurrentSwitchSetValid( const ISS_SWITCH_SYS* system, u16 zoneID )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  // スイッチがない
  if( set == FALSE ) { return FALSE; }

  return ISS_SWITCH_SET_IsValidAt( set, zoneID );
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したBGMに対応するスイッチセットがあるかどうかを判定する
 *
 * @param system
 * @param soundIdx 検索キーとなるBGM
 *
 * @return 指定したBGMに対応するスイッチセットがある場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckSwitchSetExist( const ISS_SWITCH_SYS* system, u32 soundIdx )
{
  if( SearchSwitchSet( system, soundIdx ) == NULL ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したBGMに対応するスイッチセットを検索する
 *
 * @param system
 * @param soundIdx 検索キーとなるBGM
 *
 * @return 指定したBGMに対応するスイッチセットがある場合, 
 *         そのスイッチセットを返す.
 *         そうでなければ, NULL
 */
//-------------------------------------------------------------------------------
static ISS_SWITCH_SET* SearchSwitchSet( const ISS_SWITCH_SYS* system, u32 soundIdx )
{
  int idx;
  int setNum;
  ISS_SWITCH_SET* set;

  setNum = GetSwitchSetNum( system );

  // 全てのスイッチセットをチェック
  for( idx=0; idx < setNum; idx++ )
  {
    set = GetSwitchSet( system, idx );

    // 指定されたBGMに対応するスイッチを発見
    if( ISS_SWITCH_SET_GetSoundIdx(set) == soundIdx ) { return set; }
  } 

  // 該当するスイッチセットは持っていない
  return NULL;
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定したBGMに対応するスイッチセットに変更する
 *
 * @param system
 * @param soundIdx BGMを指定
 *
 * @return スイッチが変化した場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------
static BOOL ChangeSwitchSet( ISS_SWITCH_SYS* system, u32 soundIdx )
{
  ISS_SWITCH_SET* oldSet;
  ISS_SWITCH_SET* newSet;

  oldSet = GetCurrentSwitchSet( system );
  newSet = SearchSwitchSet( system, soundIdx );

  // スイッチセットに変化なし
  if( oldSet == newSet ) { return FALSE; }

  // スイッチセットを変更
  system->curSwitchSet = newSet;
  return TRUE;
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在のスイッチセットをBGMボリュームに反映させる
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void SetBGMVolume( const ISS_SWITCH_SYS* system )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );
  
  if( set ) {
    ISS_SWITCH_SET_SetBGMVolume( set );
  } 
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在のBGMを取得する
 *
 * @return 再生中のBGM番号
 */
//-------------------------------------------------------------------------------
static u32 GetCurrentBGM( void )
{
  return PMSND_GetBGMsoundNo();
}

//-------------------------------------------------------------------------------
/**
 * @brief システムが起動しているかどうかをチェックする
 *
 * @param system
 *
 * @return システムが起動中なら TRUE
 *         そうでないなら FALSE
 */
//-------------------------------------------------------------------------------
static BOOL CheckSystemBoot( const ISS_SWITCH_SYS* system )
{
  return system->bootFlag;
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在のスイッチセットをリセットする
 *
 * @param system
 */
//-------------------------------------------------------------------------------
static void ResetCurrentSwitchSet( ISS_SWITCH_SYS* system )
{
  ISS_SWITCH_SET* set;

  set = GetCurrentSwitchSet( system );

  if( set ) {
    ISS_SWITCH_SET_ResetSwitch( set );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  スイッチの予約
 *	        BOOTしていないときや、BGM変更前のときにON,OFFを事前設定する
 *
 *  @param system
 *  @param switchIdx 予約するスイッチ番号
 *  @param is_on     TRUEでONを予約  FALSEでOFFを予約
 *  @param bgm       予約先のBGM番号
 */
//-----------------------------------------------------------------------------
static void ReserveSwitchSet( ISS_SWITCH_SYS* system, SWITCH_INDEX switchIdx, BOOL is_on, u16 bgm )
{
  int i;
  for( i = 0; i < RESERVE_MAX; i++ )
  {
    ISS_SWITCH_RESERVE *wk  = &system->reserveData[ i ];
    if( ISS_SWITCH_RESERVE_IsUse( wk ) == FALSE )
    {
      ISS_SWITCH_RESERVE_Set( wk, switchIdx, is_on, bgm );
      return;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  予約設定
 *
 *	@param  wk          予約ワーク	
 *	@param  switchIdx   予約するスイッチ
 *  @param  is_on       予約する操作
 *  @param  bgm         BGM
 */
//-----------------------------------------------------------------------------
static void ISS_SWITCH_RESERVE_Set( ISS_SWITCH_RESERVE *wk, SWITCH_INDEX switchIdx, BOOL is_on, u16 bgm )
{
  ISS_SWITCH_RESERVE_Clear( wk );
  wk->switchIdx = switchIdx;
  wk->is_on     = is_on;
  wk->bgm       = bgm;
  wk->is_use    = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  予約ワークをクリア
 *
 *	@param	ISS_SWITCH_RESERVE *wk 予約
 */
//-----------------------------------------------------------------------------
static void ISS_SWITCH_RESERVE_Clear( ISS_SWITCH_RESERVE *wk )
{
  GFL_STD_MemClear( wk, sizeof(ISS_SWITCH_RESERVE) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  予約メイン
 *
 *	@param	ISS_SWITCH_RESERVE *wk  予約ワーク
 *	@param	system                  システムワーク
 */
//-----------------------------------------------------------------------------
static void ISS_SWITCH_RESERVE_Main( ISS_SWITCH_RESERVE *wk, ISS_SWITCH_SYS* system )
{
  ISS_SWITCH_SET* set;

  if( wk->is_use == FALSE )
  {
    return;
  }

  set = GetCurrentSwitchSet( system );
  if( set )
  {
    if( wk->bgm == GetCurrentBGM() )
    {
      ISS_SWITCH_SET* set = GetCurrentSwitchSet( system );
      if( wk->is_on )
      {
        if( set ) { 
          ISS_SWITCH_SET_SwitchOnForce( set, wk->switchIdx  );
        }
      }
      else
      {
        if( set ) { 
          ISS_SWITCH_SET_SwitchOffForce( set, wk->switchIdx  );
        }
      }
      ISS_SWITCH_RESERVE_Clear( wk );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  使用フラグを設定
 *
 *	@param	const ISS_SWITCH_RESERVE *wk ワーク
 *
 *	@return TRUE使用  FALSE不使用
 */
//-----------------------------------------------------------------------------
static BOOL ISS_SWITCH_RESERVE_IsUse( const ISS_SWITCH_RESERVE *wk )
{
  return wk->is_use;
}
