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
struct _RESEARCH_COMMON_WORK
{
  HEAPID        heapID;
  GAMESYS_WORK* gameSystem;
  GAMEDATA*     gameData;

  // タッチ範囲
  GFL_UI_TP_HITTBL touchHitTable[ COMMON_TOUCH_AREA_NUM ]; 
  
  // パレットアニメーション
  PALETTE_ANIME* paletteAnime[ COMMON_PALETTE_ANIME_NUM ];

  // OBJ
  u32         OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_NUM ]; // 共通リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ COMMON_CLUNIT_NUM ];               // セルアクターユニット
  GFL_CLWK*   clactWork[ COMMON_CLWK_NUM ];                 // セルアクターワーク

  // カラーパレット
  PALETTE_FADE_PTR paletteFadeSystem; // カラーパレット フェードシステム
};

//-------------------------------------------------------------------------------
// ■初期化・生成・破棄
//-------------------------------------------------------------------------------
// 全画面共通ワーク
static void InitCommonWork( RESEARCH_COMMON_WORK* work ); // 全画面共通ワークを初期化する
static RESEARCH_COMMON_WORK* CreateCommonWork( HEAPID heapID ); // 全画面共通ワークを生成する
static void DeleteCommonWork( RESEARCH_COMMON_WORK* work ); // 全画面共通ワークを破棄する
static void SetupCommonWork( RESEARCH_COMMON_WORK* work, GAMESYS_WORK* gameSystem, HEAPID heapID ); // 全画面共通ワークをセットアップする
static void CleanUpCommonWork( RESEARCH_COMMON_WORK* work ); // 全画面共通ワークをクリーンアップする
// セルアクターシステム
static void CreateClactSystem( RESEARCH_COMMON_WORK* work ); // アクターシステムを初期化する
static void DeleteClactSystem( RESEARCH_COMMON_WORK* work ); // アクターシステムを破棄する
// OBJ リソース
static void InitOBJResources( RESEARCH_COMMON_WORK* work ); // 共通OBJのリソースを初期化する
static void RegisterOBJResources( RESEARCH_COMMON_WORK* work ); // 共通OBJのリソースを登録する
static void ReleaseOBJResources( RESEARCH_COMMON_WORK* work ); // 共通OBJのリソースを解放する
// セルアクターユニット
static void InitClactUnits( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターユニットを初期化する
static void CreateClactUnits( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターユニットを生成する
static void DeleteClactUnits( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターユニットを破棄する
// セルアクターワーク
static void InitClactWorks( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターワークを初期化する
static void CreateClactWorks( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターワークを生成する
static void DeleteClactWorks( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターワークを破棄する
// パレットフェードシステム
static void InitPaletteFadeSystem( RESEARCH_COMMON_WORK* work ); // パレットフェードシステムを初期化する
static void CreatePaletteFadeSystem( RESEARCH_COMMON_WORK* work ); // パレットフェードシステムを生成する
static void DeletePaletteFadeSystem( RESEARCH_COMMON_WORK* work ); // パレットフェードシステムを破棄する
static void SetupPaletteFadeSystem( RESEARCH_COMMON_WORK* work ); // パレットフェードシステムをセットアップする
static void CleanUpPaletteFadeSystem( RESEARCH_COMMON_WORK* work ); // パレットフェードシステムをクリーンアップする
// パレットアニメーション
static void InitPaletteAnime( RESEARCH_COMMON_WORK* work ); // パレットアニメーションワークを初期化する
static void CreatePaletteAnime( RESEARCH_COMMON_WORK* work ); // パレットアニメーションワークを生成する
static void DeletePaletteAnime( RESEARCH_COMMON_WORK* work ); // パレットアニメーションワークを破棄する
static void SetupPaletteAnime( RESEARCH_COMMON_WORK* work ); // パレットアニメーションワークをセットアップする
static void CleanUpPaletteAnime( RESEARCH_COMMON_WORK* work ); // パレットアニメーションワークをクリーンアップする
static void StartPaletteAnime( RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index ); // パレットアニメーションを開始する
static void StopPaletteAnime( RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index ); // パレットアニメーションを停止する
static void ResetPalette( RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index  ); // パレットをアニメ開始時の状態にリセットする
static void UpdatePaletteAnime( RESEARCH_COMMON_WORK* work ); // パレットアニメーションを更新する
// タッチ範囲
static void SetupTouchArea( RESEARCH_COMMON_WORK* work ); // タッチ範囲をセットアップする

//-------------------------------------------------------------------------------
// ■取得
//-------------------------------------------------------------------------------
static u32 GetOBJResRegisterIndex( const RESEARCH_COMMON_WORK* work, COMMON_OBJ_RESOURCE_ID resID ); // 共通OBJリソースの登録インデックス
static GFL_CLUNIT* GetClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx ); // 共通OBJのセルアクターユニットを取得する
static GFL_CLWK* GetClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx ); // 共通OBJのセルアクターワークを取得する





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
RESEARCH_COMMON_WORK* RESEARCH_COMMON_CreateWork( HEAPID heapID, GAMESYS_WORK* gameSystem )
{
  RESEARCH_COMMON_WORK* work;

  work = CreateCommonWork( heapID );           // 生成
  InitCommonWork( work );                      // 初期化
  SetupCommonWork( work, gameSystem, heapID ); // セットアップ

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create work\n" );

  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通ワークを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RESEARCH_COMMON_DeleteWork( RESEARCH_COMMON_WORK* work )
{
  CleanUpCommonWork( work ); // クリーンアップ
  DeleteCommonWork( work );  // 破棄

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete work\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェード ( ブラック・アウト ) を開始する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RESEARCH_COMMON_StartPaletteFadeBlackOut( RESEARCH_COMMON_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: start palette fade black out\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェード ( ブラック・イン ) を開始する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RESEARCH_COMMON_StartPaletteFadeBlackIn( RESEARCH_COMMON_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: start palette fade black in\n" );
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
HEAPID RESEARCH_COMMON_GetHeapID( const RESEARCH_COMMON_WORK* work )
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
GAMESYS_WORK* RESEARCH_COMMON_GetGameSystem( const RESEARCH_COMMON_WORK* work )
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
GAMEDATA* RESEARCH_COMMON_GetGameData( const RESEARCH_COMMON_WORK* work )
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
GFL_CLUNIT* RESEARCH_COMMON_GetClactUnit( 
    const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx )
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
GFL_CLWK* RESEARCH_COMMON_GetClactWork( 
    const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx )
{
  return GetClactWork( work, workIdx );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通パレットフェードシステムを取得する
 *
 * @param work
 *
 * @return 共通パレットフェードシステム
 */
//-------------------------------------------------------------------------------
PALETTE_FADE_PTR RESEARCH_COMMON_GetPaletteFadeSystem( const RESEARCH_COMMON_WORK* work )
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
const GFL_UI_TP_HITTBL* RESEARCH_COMMON_GetHitTable( const RESEARCH_COMMON_WORK* work )
{
  return work->touchHitTable;
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RESEARCH_COMMON_UpdatePaletteAnime( RESEARCH_COMMON_WORK* work )
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
void RESEARCH_COMMON_StartPaletteAnime( 
    RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
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
void RESEARCH_COMMON_StopPaletteAnime( 
    RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
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
void RESEARCH_COMMON_StopAllPaletteAnime( RESEARCH_COMMON_WORK* work )
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
void RESEARCH_COMMON_ResetPalette(
    RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
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
void RESEARCH_COMMON_ResetAllPalette( RESEARCH_COMMON_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  { 
    ResetPalette( work, idx );
  }
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
static void InitCommonWork( RESEARCH_COMMON_WORK* work )
{
  // 初期化
  GFL_STD_MemClear( work, sizeof(RESEARCH_COMMON_WORK) );

  InitOBJResources( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitPaletteFadeSystem( work );
  InitPaletteAnime( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: init common work\n" );
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
static RESEARCH_COMMON_WORK* CreateCommonWork( HEAPID heapID )
{
  RESEARCH_COMMON_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_COMMON_WORK) );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create common work\n" );

  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する全画面共通ワークを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteCommonWork( RESEARCH_COMMON_WORK* work )
{
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete common work\n" );
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
static void SetupCommonWork( RESEARCH_COMMON_WORK* work, GAMESYS_WORK* gameSystem, HEAPID heapID )
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

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: setup common work\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを初期化する全画面共通ワークをクリーンアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpCommonWork( RESEARCH_COMMON_WORK* work )
{
  CleanUpPaletteAnime( work );      // パレットアニメーションワークをクリーンアップ
  DeletePaletteAnime( work );       // パレットアニメーションワークを破棄
  CleanUpPaletteFadeSystem( work ); // パレット管理システムをクリーンアップ
  DeletePaletteFadeSystem( work );  // パレット管理システムを破棄
  DeleteClactWorks( work );         // セルアクターワークを破棄
  DeleteClactUnits( work );         // セルアクターユニットを破棄
  ReleaseOBJResources( work );      // OBJリソースを解放
  DeleteClactSystem( work );        // アクターシステムを破棄

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: clean up common work\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief アクターシステムを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreateClactSystem( RESEARCH_COMMON_WORK* work )
{
  // システム作成
  GFL_CLACT_SYS_Create( &CommonClactSystemInitData, &VRAMBankSettings, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create clact system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteClactSystem( RESEARCH_COMMON_WORK* work )
{ 
  // システム破棄
  GFL_CLACT_SYS_Delete();

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete clact system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのリソースを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitOBJResources( RESEARCH_COMMON_WORK* work )
{
  int i;

  for( i=0; i<COMMON_OBJ_RESOURCE_NUM; i++ )
  {
    work->OBJResRegisterIdx[i] = 0; 
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: init OBJ resources\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのリソースを登録する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void RegisterOBJResources( RESEARCH_COMMON_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, iconPalette, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, heapID );

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

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: register OBJ resources\n" );
}

//-------------------------------------------------------------------------------
/**
 * @breif 共通OBJのリソースを解放する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void ReleaseOBJResources( RESEARCH_COMMON_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_PALETTE_ICON ] );
  GFL_CLGRP_CELLANIM_Release( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: release OBJ resources\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitClactUnits( RESEARCH_COMMON_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < COMMON_CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: init clact units\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを生成する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreateClactUnits( RESEARCH_COMMON_WORK* work )
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

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMON: create clact units\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターユニットを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteClactUnits( RESEARCH_COMMON_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < COMMON_CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete clact units\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitClactWorks( RESEARCH_COMMON_WORK* work )
{
  int workIdx;

  // 初期化
  for( workIdx=0; workIdx < COMMON_CLWK_NUM; workIdx++ )
  {
    work->clactWork[ workIdx ] = NULL;
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: init clact works\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを生成する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreateClactWorks( RESEARCH_COMMON_WORK* work )
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

    // 非表示に設定
    //GFL_CLACT_WK_SetDrawEnable( work->clactWork[ wkIdx ], FALSE );
    GFL_CLACT_WK_SetDrawEnable( work->clactWork[ wkIdx ], TRUE );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create clact works\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJのセルアクターワークを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeleteClactWorks( RESEARCH_COMMON_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < COMMON_CLWK_NUM; wkIdx++ )
  {
    // 生成されていない
    GF_ASSERT( work->clactWork[ wkIdx ] );

    // 破棄
    GFL_CLACT_WK_Remove( work->clactWork[ wkIdx ] );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete clact works\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを初期化する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void InitPaletteFadeSystem( RESEARCH_COMMON_WORK* work )
{
  work->paletteFadeSystem = NULL;

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: init palette fade system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを生成する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CreatePaletteFadeSystem( RESEARCH_COMMON_WORK* work )
{
  GF_ASSERT( work->paletteFadeSystem == NULL );

  work->paletteFadeSystem = PaletteFadeInit( work->heapID );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create palette fade system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを破棄する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void DeletePaletteFadeSystem( RESEARCH_COMMON_WORK* work )
{
  GF_ASSERT( work->paletteFadeSystem );

  PaletteFadeFree( work->paletteFadeSystem );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete palette fade system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムをセットアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupPaletteFadeSystem( RESEARCH_COMMON_WORK* work )
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

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: setup palette fade system\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムをクリーンアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpPaletteFadeSystem( RESEARCH_COMMON_WORK* work )
{
  PALETTE_FADE_PTR fadeSystem;
  
  fadeSystem = work->paletteFadeSystem;

  // ワークを破棄
  PaletteFadeWorkAllocFree( fadeSystem, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( fadeSystem, FADE_MAIN_OBJ );
  PaletteFadeWorkAllocFree( fadeSystem, FADE_SUB_BG );
  PaletteFadeWorkAllocFree( fadeSystem, FADE_SUB_OBJ );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: clean up palette fade system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitPaletteAnime( RESEARCH_COMMON_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    work->paletteAnime[ idx ] = NULL;
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: init palette anime\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreatePaletteAnime( RESEARCH_COMMON_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] == NULL ); // 多重生成

    work->paletteAnime[ idx ] = PALETTE_ANIME_Create( work->heapID );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create palette anime\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeletePaletteAnime( RESEARCH_COMMON_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Delete( work->paletteAnime[ idx ] );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete palette anime\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupPaletteAnime( RESEARCH_COMMON_WORK* work )
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

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: setup palette anime\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをクリーンアップする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpPaletteAnime( RESEARCH_COMMON_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    // 操作していたパレットを元に戻す
    PALETTE_ANIME_Reset( work->paletteAnime[ idx ] );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: clean up palette anime\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 * @param index 開始するアニメーション
 */
//----------------------------------------------------------------------------------------------
static void StartPaletteAnime( RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Start( work->paletteAnime[ index ], 
                       CommonPaletteAnimeData[ index ].animeType,
                       CommonPaletteAnimeData[ index ].fadeColor );
  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: start palette anime [%d]\n", index );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを停止する
 *
 * @param work
 * @param index 停止するアニメーション
 */
//----------------------------------------------------------------------------------------------
static void StopPaletteAnime( RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Stop( work->paletteAnime[ index ] );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: stop palette anime [%d]\n", index );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットをアニメ開始時の状態にリセットする
 *
 * @param work
 * @param index リセット対象のアニメーション
 */
//----------------------------------------------------------------------------------------------
static void ResetPalette( RESEARCH_COMMON_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{ 
  PALETTE_ANIME_Reset( work->paletteAnime[ index ] );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: reset palette [%d]\n", index );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdatePaletteAnime( RESEARCH_COMMON_WORK* work )
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
static void SetupTouchArea( RESEARCH_COMMON_WORK* work )
{
  int idx;

  for( idx=0; idx < COMMON_TOUCH_AREA_NUM; idx++ )
  {
    work->touchHitTable[ idx ].rect.left   = CommonTouchAreaInitData[ idx ].left;
    work->touchHitTable[ idx ].rect.right  = CommonTouchAreaInitData[ idx ].right;
    work->touchHitTable[ idx ].rect.top    = CommonTouchAreaInitData[ idx ].top;
    work->touchHitTable[ idx ].rect.bottom = CommonTouchAreaInitData[ idx ].bottom;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: setup touch area\n" );
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
static u32 GetOBJResRegisterIndex( const RESEARCH_COMMON_WORK* work, COMMON_OBJ_RESOURCE_ID resID )
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
static GFL_CLUNIT* GetClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx )
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
static GFL_CLWK* GetClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx )
{
  return work->clactWork[ workIdx ];
}
