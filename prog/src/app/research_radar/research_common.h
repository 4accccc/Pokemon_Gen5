/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 共通ヘッダ
 * @file   research_common.h
 * @author obata
 * @date   2010.02.06
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>
#include "research_common_index.h"
#include "gamesystem/gamesystem.h"  // for GAMESYS_WORK


//=============================================================================== 
// ■定数
//=============================================================================== 
#define PRINT_TARGET (2)  // デバッグ出力先

#define DOT_PER_CHARA     (8)                      // 1キャラ = 8ドット
#define ONE_CHARA_SIZE    (0x20)                   // 1キャラクターデータのサイズ [Byte]
#define ONE_PALETTE_SIZE  (0x20)                   // 1パレット(16色分)のサイズ [Byte]
#define FULL_PALETTE_SIZE (ONE_PALETTE_SIZE * 16)  // 16パレットのサイズ [Byte]

//----------
// □SUB-BG
//----------
// 背景面
#define SUB_BG_BACK (GFL_BG_FRAME0_S)  // BG フレーム
#define SUB_BG_BACK_PRIORITY      (3)  // 表示優先順位
#define SUB_BG_BACK_FIRST_PLT_IDX (0)  // 先頭パレット番号
#define SUB_BG_BACK_PLT_NUM       (1)  // 使用パレット数
// レーダー面
#define SUB_BG_RADAR (GFL_BG_FRAME1_S)    // BG フレーム
#define SUB_BG_RADAR_PRIORITY        (2)  // 表示優先順位
#define SUB_BG_RADAR_FIRST_PLT_IDX   (0)  // 先頭パレット番号
#define SUB_BG_RADAR_PLT_NUM         (6)  // 使用パレット数
#define SUB_BG_RADAR_PLT_ANIME_FRAME (10) // パレットアニメーションの更新間隔[frame]

//-----------
// □MAIN-BG
//-----------
// 背景面
#define MAIN_BG_BACK (GFL_BG_FRAME1_M)  // 背景BG面
#define MAIN_BG_BACK_PRIORITY      (3)  // 背景BG面の表示優先順位
#define MAIN_BG_BACK_FIRST_PLT_IDX (0)  // 背景BG面の先頭パレット番号
#define MAIN_BG_BACK_PLT_NUM       (1)  // 背景BG面のパレット数


// VRAM-Bank 設定
extern const GFL_DISP_VRAM VRAMBankSettings;

// BGモード設定
extern const GFL_BG_SYS_HEADER BGSysHeader2D;
extern const GFL_BG_SYS_HEADER BGSysHeader3D;

// セルアクターシステム
extern const GFL_CLSYS_INIT ClactSystemInitData;

// 共通ワーク
typedef struct _RESEARCH_COMMON_WORK RESEARCH_COMMON_WORK;

// 共通ワーク 生成・破棄
RESEARCH_COMMON_WORK* RESEARCH_COMMON_CreateWork( HEAPID heapID, GAMESYS_WORK* gameSystem );
void RESEARCH_COMMON_DeleteWork( RESEARCH_COMMON_WORK* work ); 

// 共通ワーク 共通OBJ 準備・後片付け
void RESEARCH_COMMON_SetupCommonOBJ( RESEARCH_COMMON_WORK* work );
void RESEARCH_COMMON_CleanUpCommonOBJ( RESEARCH_COMMON_WORK* work );

// 共通ワーク 取得
HEAPID RESEARCH_COMMON_GetHeapID( const RESEARCH_COMMON_WORK* work );
GAMESYS_WORK* RESEARCH_COMMON_GetGameSystem( const RESEARCH_COMMON_WORK* work );
GAMEDATA* RESEARCH_COMMON_GetGameData( const RESEARCH_COMMON_WORK* work );
GFL_CLUNIT* RESEARCH_COMMON_GetClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx );
GFL_CLWK* RESEARCH_COMMON_GetClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx );
