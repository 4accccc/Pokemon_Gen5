/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 共通定義
 * @file   research_common.c
 * @author obata
 * @date   2010.02.13
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "research_common.h"
#include "research_common_def.h"
#include "research_common_data.cdat"

#include "gamesystem/gamesystem.h"       // for GAMESYS_WORK

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

  u32         OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_NUM ]; // 共通リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ COMMON_CLUNIT_NUM ];               // セルアクターユニット
  GFL_CLWK*   clactWork[ COMMON_CLWK_NUM ];                 // セルアクターワーク
};

//-------------------------------------------------------------------------------
// 初期化・生成・破棄
//-------------------------------------------------------------------------------
// OBJ
static void CreateClactSystem( RESEARCH_COMMON_WORK* work ); // アクターシステムを初期化する
static void DeleteClactSystem( RESEARCH_COMMON_WORK* work ); // アクターシステムを破棄する
static void InitCommonOBJResources( RESEARCH_COMMON_WORK* work ); // 共通OBJのリソースを初期化する
static void RegisterCommonOBJResources( RESEARCH_COMMON_WORK* work ); // 共通OBJのリソースを登録する
static void ReleaseCommonOBJResources( RESEARCH_COMMON_WORK* work ); // 共通OBJのリソースを解放する
static void InitCommonClactUnits( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターユニットを初期化する
static void CreateCommonClactUnits( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターユニットを生成する
static void DeleteCommonClactUnits( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターユニットを破棄する
static void InitCommonClactWorks( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターワークを初期化する
static void CreateCommonClactWorks( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターワークを生成する
static void DeleteCommonClactWorks( RESEARCH_COMMON_WORK* work ); // 共通OBJのセルアクターワークを破棄する

//-------------------------------------------------------------------------------
// 取得
//-------------------------------------------------------------------------------
static u32 GetCommonOBJResRegisterIndex( const RESEARCH_COMMON_WORK* work, COMMON_OBJ_RESOURCE_ID resID ); // 共通OBJリソースの登録インデックス
static GFL_CLUNIT* GetCommonClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx ); // 共通OBJのセルアクターユニットを取得する
static GFL_CLWK* GetCommonClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx ); // 共通OBJのセルアクターワークを取得する


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

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_COMMON_WORK) );

  // 初期化
  work->heapID     = heapID;
  work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem );
  InitCommonOBJResources( work );
  InitCommonClactUnits( work );
  InitCommonClactWorks( work );

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
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: delete work\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJをセットアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RESEARCH_COMMON_SetupCommonOBJ( RESEARCH_COMMON_WORK* work )
{
  CreateClactSystem( work );

  RegisterCommonOBJResources( work );
  CreateCommonClactUnits( work );
  CreateCommonClactWorks( work );
}

//-------------------------------------------------------------------------------
/**
 * @brief 共通OBJをクリーンアップする
 *
 * @param work
 */
//-------------------------------------------------------------------------------
void RESEARCH_COMMON_CleanUpCommonOBJ( RESEARCH_COMMON_WORK* work )
{
  // 破棄
  DeleteCommonClactWorks( work );
  DeleteCommonClactUnits( work );
  ReleaseCommonOBJResources( work );

  // 初期化
  InitCommonOBJResources( work );
  InitCommonClactUnits( work );
  InitCommonClactWorks( work );

  // アクターシステム破棄
  DeleteClactSystem( work );
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
GFL_CLUNIT* RESEARCH_COMMON_GetClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx )
{
  return GetCommonClactUnit( work, unitIdx );
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
GFL_CLWK* RESEARCH_COMMON_GetClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx )
{
  return GetCommonClactWork( work, workIdx );
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
  GFL_CLACT_SYS_Create( &ClactSystemInitData, &VRAMBankSettings, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-COMMON: create clact system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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
static void InitCommonOBJResources( RESEARCH_COMMON_WORK* work )
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
static void RegisterCommonOBJResources( RESEARCH_COMMON_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, palette, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, heapID );

  // リソースを登録
  character = GFL_CLGRP_CGR_Register( arcHandle, 
                                      NARC_app_menu_common_bar_button_128k_NCGR, 
                                      FALSE, CLSYS_DRAW_MAIN, heapID ); 

  palette = GFL_CLGRP_PLTT_Register( arcHandle, 
                                     NARC_app_menu_common_bar_button_NCLR,
                                     CLSYS_DRAW_MAIN, 0, heapID );

  cellAnime = GFL_CLGRP_CELLANIM_Register( arcHandle,
                                           NARC_app_menu_common_bar_button_32k_NCER,
                                           NARC_app_menu_common_bar_button_32k_NANR,
                                           heapID ); 
  // 登録インデックスを記憶
  work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CHARACTER ]  = character;
  work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_PALETTE ]    = palette;
  work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CELL_ANIME ] = cellAnime;

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
static void ReleaseCommonOBJResources( RESEARCH_COMMON_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->OBJResRegisterIdx[ COMMON_OBJ_RESOURCE_MAIN_PALETTE ] );
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
static void InitCommonClactUnits( RESEARCH_COMMON_WORK* work )
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
static void CreateCommonClactUnits( RESEARCH_COMMON_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < COMMON_CLUNIT_NUM; unitIdx++ )
  {
    u16 workNum;
    u8 priority;

    // 多重生成
    GF_ASSERT( work->clactUnit[ unitIdx ] == NULL );

    workNum  = ClactUnitWorkSize[ unitIdx ];
    priority = ClactUnitPriority[ unitIdx ];
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
static void DeleteCommonClactUnits( RESEARCH_COMMON_WORK* work )
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
static void InitCommonClactWorks( RESEARCH_COMMON_WORK* work )
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
static void CreateCommonClactWorks( RESEARCH_COMMON_WORK* work )
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
    wkData.pos_x   = ClactWorkInitData[ wkIdx ].posX;
    wkData.pos_y   = ClactWorkInitData[ wkIdx ].posY;
    wkData.anmseq  = ClactWorkInitData[ wkIdx ].animeSeq;
    wkData.softpri = ClactWorkInitData[ wkIdx ].softPriority; 
    wkData.bgpri   = ClactWorkInitData[ wkIdx ].BGPriority; 
    unit           = GetCommonClactUnit( work, ClactWorkInitData[ wkIdx ].unitIdx );
    charaIdx       = GetCommonOBJResRegisterIndex( work, ClactWorkInitData[ wkIdx ].characterResID );
    paletteIdx     = GetCommonOBJResRegisterIndex( work, ClactWorkInitData[ wkIdx ].paletteResID );
    cellAnimeIdx   = GetCommonOBJResRegisterIndex( work, ClactWorkInitData[ wkIdx ].cellAnimeResID );
    surface        = ClactWorkInitData[ wkIdx ].setSurface;

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
static void DeleteCommonClactWorks( RESEARCH_COMMON_WORK* work )
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

//----------------------------------------------------------------------------------------------
/**
 * @brief OBJ リソースの登録インデックスを取得する
 *
 * @param work
 * @param resID リソースID
 *
 * @return 指定したリソースの登録インデックス
 */
//----------------------------------------------------------------------------------------------
static u32 GetCommonOBJResRegisterIndex( const RESEARCH_COMMON_WORK* work, COMMON_OBJ_RESOURCE_ID resID )
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
static GFL_CLUNIT* GetCommonClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx )
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
static GFL_CLWK* GetCommonClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx )
{
  return work->clactWork[ workIdx ];
}
