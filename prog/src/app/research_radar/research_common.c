/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 共通定義
 * @file   research_common.c
 * @author obata
 * @date   2010.02.13
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "palette_anime.h"
#include "research_common.h"
#include "research_common_def.h"
#include "research_common_data.cdat"

#include "gamesystem/gamesystem.h" // for GAMESYS_WORK
#include "system/palanm.h"         // for PaletteFadeXxxx

#include "arc/arc_def.h"             // for ARCID_xxxx
#include "arc/app_menu_common.naix"  // for NARC_research_radar_xxxx


//===============================================================================
// ■調査レーダー 全画面共通ワーク
//===============================================================================
struct _RESEARCH_RADAR_COMMON_WORK {

  HEAPID        heapID;
  GAMESYS_WORK* gameSystem;
  GAMEDATA*     gameData;

  // タッチ範囲
  GFL_UI_TP_HITTBL touchHitTable[ COMMON_TOUCH_AREA_NUM ]; 
  
  // OBJ
  u32         OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_NUM ]; // 共通リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ COMMON_CLUNIT_NUM ];               // セルアクターユニット
  GFL_CLWK*   clactWork[ COMMON_CLWK_NUM ];                 // セルアクターワーク

  // カラーパレット
  PALETTE_FADE_PTR paletteFadeSystem; // カラーパレット フェードシステム

  // パレットアニメーション
  PALETTE_ANIME* paletteAnime[ COMMON_PALETTE_ANIME_NUM ];

  // 画面遷移
  RADAR_SEQ prevSeq; // 直前の画面
  RADAR_SEQ nowSeq;  // 現在の画面
  SEQ_CHANGE_TRIG seqTrig; // 画面遷移の引き金となったトリガ

  // 強制終了フラグ
  BOOL forceReturnFlag;
};

//-------------------------------------------------------------------------------
// ■初期化・生成・破棄
//-------------------------------------------------------------------------------
// 全画面共通ワーク
static void InitCommonWork( RRC_WORK* work ); // 全画面共通ワークを初期化する
static RRC_WORK* CreateCommonWork( HEAPID heapID ); // 全画面共通ワークを生成する
static void DeleteCommonWork( RRC_WORK* work ); // 全画面共通ワークを破棄する
static void SetupCommonWork( RRC_WORK* work, GAMESYS_WORK* gameSystem, HEAPID heapID ); // 全画面共通ワークをセットアップする
static void CleanUpCommonWork( RRC_WORK* work ); // 全画面共通ワークをクリーンアップする
// セルアクターシステム
static void CreateClactSystem( RRC_WORK* work ); // アクターシステムを初期化する
static void DeleteClactSystem( RRC_WORK* work ); // アクターシステムを破棄する
// OBJ リソース
static void InitOBJResources( RRC_WORK* work ); // 共通OBJのリソースを初期化する
static void RegisterOBJResources( RRC_WORK* work ); // 共通OBJのリソースを登録する
static void ReleaseOBJResources( RRC_WORK* work ); // 共通OBJのリソースを解放する
// セルアクターユニット
static void InitClactUnits( RRC_WORK* work ); // 共通OBJのセルアクターユニットを初期化する
static void CreateClactUnits( RRC_WORK* work ); // 共通OBJのセルアクターユニットを生成する
static void DeleteClactUnits( RRC_WORK* work ); // 共通OBJのセルアクターユニットを破棄する
// セルアクターワーク
static void InitClactWorks( RRC_WORK* work ); // 共通OBJのセルアクターワークを初期化する
static void CreateClactWorks( RRC_WORK* work ); // 共通OBJのセルアクターワークを生成する
static void DeleteClactWorks( RRC_WORK* work ); // 共通OBJのセルアクターワークを破棄する
// パレットフェードシステム
static void InitPaletteFadeSystem( RRC_WORK* work ); // パレットフェードシステムを初期化する
static void CreatePaletteFadeSystem( RRC_WORK* work ); // パレットフェードシステムを生成する
static void DeletePaletteFadeSystem( RRC_WORK* work ); // パレットフェードシステムを破棄する
static void SetupPaletteFadeSystem( RRC_WORK* work ); // パレットフェードシステムをセットアップする
static void CleanUpPaletteFadeSystem( RRC_WORK* work ); // パレットフェードシステムをクリーンアップする
// パレットアニメーション
static void InitPaletteAnime( RRC_WORK* work ); // パレットアニメーションワークを初期化する
static void CreatePaletteAnime( RRC_WORK* work ); // パレットアニメーションワークを生成する
static void DeletePaletteAnime( RRC_WORK* work ); // パレットアニメーションワークを破棄する
static void SetupPaletteAnime( RRC_WORK* work ); // パレットアニメーションワークをセットアップする
static void CleanUpPaletteAnime( RRC_WORK* work ); // パレットアニメーションワークをクリーンアップする
static void StartPaletteAnime( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index ); // パレットアニメーションを開始する
static void StopPaletteAnime( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index ); // パレットアニメーションを停止する
static void ResetPalette( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index  ); // パレットをアニメ開始時の状態にリセットする
static void UpdatePaletteAnime( RRC_WORK* work ); // パレットアニメーションを更新する
// タッチ範囲
static void SetupTouchArea( RRC_WORK* work ); // タッチ範囲をセットアップする
//-------------------------------------------------------------------------------
// ■取得
//-------------------------------------------------------------------------------
static u32 GetOBJResRegisterIndex( const RRC_WORK* work, COMMON_OBJ_RESOURCE_ID resID ); // 共通OBJリソースの登録インデックス
static GFL_CLUNIT* GetClactUnit( const RRC_WORK* work, COMMON_CLUNIT_INDEX unitIdx ); // 共通OBJのセルアクターユニットを取得する
static GFL_CLWK* GetClactWork( const RRC_WORK* work, COMMON_CLWK_INDEX workIdx ); // 共通OBJのセルアクターワークを取得する



//===============================================================================
// ■外部公開関数
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 共通ワークを生成する
 *
 * @param heapID
 * @param gameSystem
 */
//-------------------------------------------------------------------------------
RRC_WORK* RRC_CreateWork( HEAPID heapID, GAMESYS_WORK* gameSystem )
{
  RRC_WORK* work;

  work = CreateCommonWork( heapID );           // 生成
  InitCommonWork( work );                      // 初期化
  SetupCommonWork( work, gameSystem, heapID ); // セットアップ

  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通ワークを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RRC_DeleteWork( RRC_WORK* work )
{
  CleanUpCommonWork( work ); // クリーンアップ
  DeleteCommonWork( work );  // 破棄
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェード ( ブラック・アウト ) を開始する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RRC_StartPaletteFadeBlackOut( RRC_WORK* work )
{
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェード ( ブラック・イン ) を開始する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RRC_StartPaletteFadeBlackIn( RRC_WORK* work )
{
}

//-------------------------------------------------------------------------------
/**
 * @brief ヒープIDを取得する
 *
 * @param work
 *
 * @return ヒープID
 */
//-------------------------------------------------------------------------------
HEAPID RRC_GetHeapID( const RRC_WORK* work )
{
  return work->heapID;
}

//-------------------------------------------------------------------------------
/**
 * @brief ゲームシステムを取得する
 *
 * @param work
 *
 * @return ゲームシステム
 */
//-------------------------------------------------------------------------------
GAMESYS_WORK* RRC_GetGameSystem( const RRC_WORK* work )
{
  return work->gameSystem;
}

//-------------------------------------------------------------------------------
/**
 * @brief ゲームデータを取得する
 *
 * @param work
 *
 * @return ゲームデータ
 */
//-------------------------------------------------------------------------------
GAMEDATA* RRC_GetGameData( const RRC_WORK* work )
{
  return work->gameData;
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを取得する
 *
 * @param work
 * @param unitIdx セルアクターユニットのインデックス
 *
 * @return 指定したインデックスのセルアクターユニット
 */
//-------------------------------------------------------------------------------
GFL_CLUNIT* RRC_GetClactUnit( const RRC_WORK* work, COMMON_CLUNIT_INDEX unitIdx )
{
  return GetClactUnit( work, unitIdx );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを取得する
 *
 * @param work
 * @param workIdx セルアクターワークのインデックス
 *
 * @return 指定したインデックスのセルアクターワーク
 */
//-------------------------------------------------------------------------------
GFL_CLWK* RRC_GetClactWork( const RRC_WORK* work, COMMON_CLWK_INDEX workIdx )
{
  return GetClactWork( work, workIdx );
}

//-------------------------------------------------------------------------------
/** * @brief 共通パレットフェードシステムを取得する
 *
 * @param work
 *
 * @return 共通パレットフェードシステム
 */
//-------------------------------------------------------------------------------
PALETTE_FADE_PTR RRC_GetPaletteFadeSystem( const RRC_WORK* work )
{
  return work->paletteFadeSystem;
} 

//-------------------------------------------------------------------------------
/**
 * @brief ヒットテーブルを取得する
 *
 * @param work
 *
 * @return 共通タッチ範囲のヒットテーブル
 */
//-------------------------------------------------------------------------------
const GFL_UI_TP_HITTBL* RRC_GetHitTable( const RRC_WORK* work )
{
  return work->touchHitTable;
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在の画面を登録する
 *
 * @param work
 * @param seq  登録する画面の識別子
 */
//-------------------------------------------------------------------------------
void RRC_SetNowSeq( RRC_WORK* work, RADAR_SEQ seq )
{
  // 直前の画面を更新
  work->prevSeq = work->nowSeq;

  // 現在の画面を更新
  work->nowSeq = seq;
}

//-------------------------------------------------------------------------------
/**
 * @brief 画面遷移のトリガを登録する
 *
 * @param work
 * @param trig
 */
//-------------------------------------------------------------------------------
void RRC_SetSeqChangeTrig( RRC_WORK* work, SEQ_CHANGE_TRIG trig )
{
  work->seqTrig = trig;
}

//-------------------------------------------------------------------------------
/**
 * @brief 現在の画面を取得する
 *
 * @param work
 *
 * @return 現在の画面を表す識別子
 */
//-------------------------------------------------------------------------------
RADAR_SEQ RRC_GetNowSeq( const RRC_WORK* work )
{
  return work->nowSeq;
}

//-------------------------------------------------------------------------------
/**
 * @brief 直前の画面を取得する
 *
 * @param work
 *
 * @return 直前の画面を表す識別子
 */
//-------------------------------------------------------------------------------
RADAR_SEQ RRC_GetPrevSeq( const RRC_WORK* work )
{
  return work->prevSeq;
}

//-------------------------------------------------------------------------------
/**
 * @brief 画面遷移のトリガを取得する
 *
 * @paramw work
 *
 * @return 画面遷移の引き金となったトリガ
 */
//-------------------------------------------------------------------------------
SEQ_CHANGE_TRIG RRC_GetSeqChangeTrig( const RRC_WORK* work )
{
  return work->seqTrig;
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RRC_UpdatePaletteAnime( RRC_WORK* work )
{
  UpdatePaletteAnime( work );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 * @param index 開始するアニメーション
 */
//-------------------------------------------------------------------------------
void RRC_StartPaletteAnime( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  StartPaletteAnime( work, index );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを停止する ( 個別指定 ) 
 *
 * @param work
 * @param index 停止するアニメーション
 */
//-------------------------------------------------------------------------------
void RRC_StopPaletteAnime( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{ 
  StopPaletteAnime( work, index );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを停止する ( 全指定 ) 
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RRC_StopAllPaletteAnime( RRC_WORK* work )
{ 
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  { 
    StopPaletteAnime( work, idx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメで操作したパレットをリセットする ( 個別指定 )
 *
 * @param work
 * @param index リセット対象のアニメーション
 */
//-------------------------------------------------------------------------------
void RRC_ResetPalette(
    RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  ResetPalette( work, index );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメで操作したパレットをリセットする ( 全指定 )
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RRC_ResetAllPalette( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  { 
    ResetPalette( work, idx );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 強制終了フラグを取得する
 */
//-------------------------------------------------------------------------------
BOOL RRC_GetForceReturnFlag( const RRC_WORK* work )
{
  return work->forceReturnFlag;
}

//-------------------------------------------------------------------------------
/**
 * @brief 強制終了フラグを立てる
 */
//-------------------------------------------------------------------------------
void RRC_SetForceReturnFlag( RRC_WORK* work )
{
  work->forceReturnFlag = TRUE;
}



//===============================================================================
// ■初期化・生成・破棄
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitCommonWork( RRC_WORK* work )
{
  // 初期化
  GFL_STD_MemClear( work, sizeof(RRC_WORK) );

  work->prevSeq = RADAR_SEQ_NULL;
  work->nowSeq = RADAR_SEQ_NULL;

  InitOBJResources( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitPaletteFadeSystem( work );
  InitPaletteAnime( work );
}

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する全画面共通ワークを生成する
 *
 * @param heapID 使用するヒープID
 *
 * @return 生成した全画面共通ワーク
 */
//-------------------------------------------------------------------------------
static RRC_WORK* CreateCommonWork( HEAPID heapID )
{
  RRC_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(RRC_WORK) );

  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する全画面共通ワークを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteCommonWork( RRC_WORK* work )
{
  GFL_HEAP_FreeMemory( work );
}

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する全画面共通ワークをセットアップする
 *
 * @param work
 * @param gameSystem
 * @param heapID
 */
//-------------------------------------------------------------------------------
static void SetupCommonWork( RRC_WORK* work, GAMESYS_WORK* gameSystem, HEAPID heapID )
{
  work->heapID     = heapID;
  work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem );

  CreateClactSystem( work );       // セルアクターシステムを作成
  RegisterOBJResources( work );    // OBJリソースを登録
  CreateClactUnits( work );        // セルアクターユニットを生成
  CreateClactWorks( work );        // セルアクターワークを生成
  CreatePaletteFadeSystem( work ); // パレット管理システムをk生成
  SetupPaletteFadeSystem( work );  // パレット管理システムをセットアップ
  SetupTouchArea( work );          // タッチ範囲をセットアップ
  CreatePaletteAnime( work );      // パレットアニメーションワークを生成
  SetupPaletteAnime( work );       // パレットアニメーションワークをセットアップ
}

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する全画面共通ワークをクリーンアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpCommonWork( RRC_WORK* work )
{
  CleanUpPaletteAnime( work );      // パレットアニメーションワークをクリーンアップ
  DeletePaletteAnime( work );       // パレットアニメーションワークを破棄
  CleanUpPaletteFadeSystem( work ); // パレット管理システムをクリーンアップ
  DeletePaletteFadeSystem( work );  // パレット管理システムを破棄
  DeleteClactWorks( work );         // セルアクターワークを破棄
  DeleteClactUnits( work );         // セルアクターユニットを破棄
  ReleaseOBJResources( work );      // OBJリソースを解放
  DeleteClactSystem( work );        // アクターシステムを破棄
}

//-------------------------------------------------------------------------------
/**
 * @brief アクターシステムを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreateClactSystem( RRC_WORK* work )
{
  // システム作成
  GFL_CLACT_SYS_Create( &CommonClactSystemInitData, &VRAMBankSettings, work->heapID );
}

//-------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteClactSystem( RRC_WORK* work )
{ 
  // システム破棄
  GFL_CLACT_SYS_Delete();
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのリソースを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitOBJResources( RRC_WORK* work )
{
  int i;

  for( i=0; i<COMMON_OBJ_RESOURCE_NUM; i++ )
  {
    work->OBJResRegisterIdx[i] = 0; 
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのリソースを登録する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void RegisterOBJResources( RRC_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, iconPalette, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, GetHeapLowID(heapID) );

  // リソースを登録
  character = GFL_CLGRP_CGR_Register( arcHandle, 
                                      NARC_app_menu_common_bar_button_128k_NCGR, 
                                      FALSE, CLSYS_DRAW_MAIN, heapID ); 

  // 共通素材 ( アイコン ) のパレット
  iconPalette = GFL_CLGRP_PLTT_RegisterEx( arcHandle, 
                                       NARC_app_menu_common_bar_button_NCLR,
                                       CLSYS_DRAW_MAIN, 
                                       ONE_PALETTE_SIZE*0, 0, 4, 
                                       heapID );

  cellAnime = GFL_CLGRP_CELLANIM_Register( arcHandle,
                                           NARC_app_menu_common_bar_button_32k_NCER,
                                           NARC_app_menu_common_bar_button_32k_NANR,
                                           heapID ); 
  // 登録インデックスを記憶
  work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CHARACTER ]      = character;
  work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_PALETTE_ICON ]   = iconPalette;
  work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CELL_ANIME ]     = cellAnime;

  GFL_ARC_CloseDataHandle( arcHandle );
}

//-------------------------------------------------------------------------------
/**
 * @breif 共通OBJのリソースを解放する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void ReleaseOBJResources( RRC_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_PALETTE_ICON ] );
  GFL_CLGRP_CELLANIM_Release( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CELL_ANIME ] );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitClactUnits( RRC_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < COMMON_CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを生成する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreateClactUnits( RRC_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < COMMON_CLUNIT_NUM; unitIdx++ )
  {
    u16 workNum;
    u8 priority;

    // 多重生成
    GF_ASSERT( work->clactUnit[ unitIdx ] == NULL );

    workNum  = CommonClactUnitWorkSize[ unitIdx ];
    priority = CommonClactUnitPriority[ unitIdx ];
    work->clactUnit[ unitIdx ] = GFL_CLACT_UNIT_Create( workNum, priority, work->heapID );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteClactUnits( RRC_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < COMMON_CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitClactWorks( RRC_WORK* work )
{
  int workIdx;

  // 初期化
  for( workIdx=0; workIdx < COMMON_CLWK_NUM; workIdx++ )
  {
    work->clactWork[ workIdx ] = NULL;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを生成する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreateClactWorks( RRC_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < COMMON_CLWK_NUM; wkIdx++ )
  {
    GFL_CLUNIT* unit;
    GFL_CLWK_DATA wkData;
    u32 charaIdx, paletteIdx, cellAnimeIdx;
    u16 surface;

    // 多重生成
    GF_ASSERT( work->clactWork[ wkIdx ] == NULL );

    // 生成パラメータ選択
    wkData.pos_x   = CommonClactWorkInitData[ wkIdx ].posX;
    wkData.pos_y   = CommonClactWorkInitData[ wkIdx ].posY;
    wkData.anmseq  = CommonClactWorkInitData[ wkIdx ].animeSeq;
    wkData.softpri = CommonClactWorkInitData[ wkIdx ].softPriority; 
    wkData.bgpri   = CommonClactWorkInitData[ wkIdx ].BGPriority; 
    unit           = GetClactUnit( work, CommonClactWorkInitData[ wkIdx ].unitIdx );
    charaIdx       = GetOBJResRegisterIndex( work, CommonClactWorkInitData[ wkIdx ].characterResID );
    paletteIdx     = GetOBJResRegisterIndex( work, CommonClactWorkInitData[ wkIdx ].paletteResID );
    cellAnimeIdx   = GetOBJResRegisterIndex( work, CommonClactWorkInitData[ wkIdx ].cellAnimeResID );
    surface        = CommonClactWorkInitData[ wkIdx ].setSurface;

    // 生成
    work->clactWork[ wkIdx ] = GFL_CLACT_WK_Create( 
        unit, charaIdx, paletteIdx, cellAnimeIdx, &wkData, surface, work->heapID );

    GFL_CLACT_WK_SetDrawEnable( work->clactWork[ wkIdx ], TRUE );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteClactWorks( RRC_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < COMMON_CLWK_NUM; wkIdx++ )
  {
    // 生成されていない
    GF_ASSERT( work->clactWork[ wkIdx ] );

    // 破棄
    GFL_CLACT_WK_Remove( work->clactWork[ wkIdx ] );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitPaletteFadeSystem( RRC_WORK* work )
{
  work->paletteFadeSystem = NULL;
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを生成する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreatePaletteFadeSystem( RRC_WORK* work )
{
  GF_ASSERT( work->paletteFadeSystem == NULL );

  work->paletteFadeSystem = PaletteFadeInit( work->heapID );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeletePaletteFadeSystem( RRC_WORK* work )
{
  GF_ASSERT( work->paletteFadeSystem );

  PaletteFadeFree( work->paletteFadeSystem );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムをセットアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupPaletteFadeSystem( RRC_WORK* work )
{
  PALETTE_FADE_PTR fadeSystem;
  
  fadeSystem = work->paletteFadeSystem;

  // ワークを確保
  PaletteFadeWorkAllocSet( fadeSystem, FADE_MAIN_BG,  FULL_PALETTE_SIZE, work->heapID );
  PaletteFadeWorkAllocSet( fadeSystem, FADE_MAIN_OBJ, FULL_PALETTE_SIZE, work->heapID );
  PaletteFadeWorkAllocSet( fadeSystem, FADE_SUB_BG,   FULL_PALETTE_SIZE, work->heapID );
  PaletteFadeWorkAllocSet( fadeSystem, FADE_SUB_OBJ,  FULL_PALETTE_SIZE, work->heapID );

  // ワークを初期化
  PaletteWorkSet_VramCopy( fadeSystem, FADE_MAIN_BG,  0, FULL_PALETTE_SIZE );
  PaletteWorkSet_VramCopy( fadeSystem, FADE_MAIN_OBJ, 0, FULL_PALETTE_SIZE );
  PaletteWorkSet_VramCopy( fadeSystem, FADE_SUB_BG,   0, FULL_PALETTE_SIZE );
  PaletteWorkSet_VramCopy( fadeSystem, FADE_SUB_OBJ,  0, FULL_PALETTE_SIZE );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムをクリーンアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpPaletteFadeSystem( RRC_WORK* work )
{
  PALETTE_FADE_PTR fadeSystem;
  
  fadeSystem = work->paletteFadeSystem;

  // ワークを破棄
  PaletteFadeWorkAllocFree( fadeSystem, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( fadeSystem, FADE_MAIN_OBJ );
  PaletteFadeWorkAllocFree( fadeSystem, FADE_SUB_BG );
  PaletteFadeWorkAllocFree( fadeSystem, FADE_SUB_OBJ );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitPaletteAnime( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    work->paletteAnime[ idx ] = NULL;
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreatePaletteAnime( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] == NULL ); // 多重生成

    work->paletteAnime[ idx ] = PALETTE_ANIME_Create( work->heapID );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeletePaletteAnime( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Delete( work->paletteAnime[ idx ] );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupPaletteAnime( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Setup( work->paletteAnime[ idx ],
                         CommonPaletteAnimeData[ idx ].destAdrs,
                         CommonPaletteAnimeData[ idx ].srcAdrs,
                         CommonPaletteAnimeData[ idx ].colorNum);
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをクリーンアップする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpPaletteAnime( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    // 操作していたパレットを元に戻す
    PALETTE_ANIME_Reset( work->paletteAnime[ idx ] );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 * @param index 開始するアニメーション
 */
//----------------------------------------------------------------------------------------------
static void StartPaletteAnime( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Start( work->paletteAnime[ index ], 
                       CommonPaletteAnimeData[ index ].animeType,
                       CommonPaletteAnimeData[ index ].fadeColor );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを停止する
 *
 * @param work
 * @param index 停止するアニメーション
 */
//----------------------------------------------------------------------------------------------
static void StopPaletteAnime( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Stop( work->paletteAnime[ index ] );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットをアニメ開始時の状態にリセットする
 *
 * @param work
 * @param index リセット対象のアニメーション
 */
//----------------------------------------------------------------------------------------------
static void ResetPalette( RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{ 
  PALETTE_ANIME_Reset( work->paletteAnime[ index ] );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdatePaletteAnime( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Update( work->paletteAnime[ idx ] );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief タッチ範囲をセットアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupTouchArea( RRC_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_TOUCH_AREA_NUM; idx++ )
  {
    work->touchHitTable[ idx ].rect.left   = CommonTouchAreaInitData[ idx ].left;
    work->touchHitTable[ idx ].rect.right  = CommonTouchAreaInitData[ idx ].right;
    work->touchHitTable[ idx ].rect.top    = CommonTouchAreaInitData[ idx ].top;
    work->touchHitTable[ idx ].rect.bottom = CommonTouchAreaInitData[ idx ].bottom;
  }
}




//===============================================================================
// ■取得
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief OBJ リソースの登録インデックスを取得する
 *
 * @param work
 * @param resID リソースID
 *
 * @return 指定したリソースの登録インデックス
 */
//-------------------------------------------------------------------------------
static u32 GetOBJResRegisterIndex( const RRC_WORK* work, COMMON_OBJ_RESOURCE_ID resID )
{
  return work->OBJResRegisterIdx[ resID ];
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを取得する
 *
 * @param work
 * @param unitIdx セルアクターユニットのインデックス
 *  
 * @return 指定したセルアクターユニット
 */
//-------------------------------------------------------------------------------
static GFL_CLUNIT* GetClactUnit( const RRC_WORK* work, COMMON_CLUNIT_INDEX unitIdx )
{
  return work->clactUnit[ unitIdx ];
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを取得する
 *
 * @param work
 * @param workIdx セルアクターワークのインデックス
 *  
 * @return 指定したセルアクターワーク
 */
//-------------------------------------------------------------------------------
static GFL_CLWK* GetClactWork( const RRC_WORK* work, COMMON_CLWK_INDEX workIdx )
{
  return work->clactWork[ workIdx ];
}
