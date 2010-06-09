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
#include "research_common_def.h"
#include "gamesystem/gamesystem.h" // for GAMESYS_WORK
#include "system/palanm.h"         // for PaletteFadeXxxx


//===============================================================================
// ■共通ワークの不完全型
//===============================================================================
typedef struct _RESEARCH_RADAR_COMMON_WORK RRC_WORK;


//===============================================================================
// ■生成・破棄
//===============================================================================
// 生成
extern RRC_WORK* RRC_CreateWork( HEAPID heapID, GAMESYS_WORK* gameSystem );
// 破棄
extern void RRC_DeleteWork( RRC_WORK* work );


//===============================================================================
// ■制御
//===============================================================================
// パレットフェード ( ブラック・アウト ) を開始する
extern void RRC_StartPaletteFadeBlackOut( RRC_WORK* work );
// パレットフェード ( ブラック・イン ) を開始する
extern void RRC_StartPaletteFadeBlackIn( RRC_WORK* work );

// パレットアニメーションを更新する
extern void RRC_UpdatePaletteAnime( RRC_WORK* work );
// パレットアニメーションを開始する
extern void RRC_StartPaletteAnime( 
    RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index );
// パレットアニメーションを停止する ( 個別指定 )
extern void RRC_StopPaletteAnime( 
    RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index );
// パレットアニメーションを停止する ( 全指定 )
extern void RRC_StopAllPaletteAnime( RRC_WORK* work );
// パレットアニメで操作したパレットをリセットする ( 個別指定 )
extern void RRC_ResetPalette(
    RRC_WORK* work, COMMON_PALETTE_ANIME_INDEX index );
// パレットアニメで操作したパレットをリセットする ( 全指定 )
extern void RRC_ResetAllPalette( RRC_WORK* work );


//===============================================================================
// ■取得
//===============================================================================
// ヒープIDを取得する
extern HEAPID RRC_GetHeapID( const RRC_WORK* work );
// ゲームシステムを取得する
extern GAMESYS_WORK* RRC_GetGameSystem( const RRC_WORK* work );
// ゲームデータを取得する
extern GAMEDATA* RRC_GetGameData( const RRC_WORK* work );
// セルアクターユニットを取得する
extern GFL_CLUNIT* RRC_GetClactUnit( 
    const RRC_WORK* work, COMMON_CLUNIT_INDEX unitIdx );
// セルアクターワークを取得する
extern GFL_CLWK* RRC_GetClactWork( 
    const RRC_WORK* work, COMMON_CLWK_INDEX workIdx );
// パレットフェードシステムを取得する
extern PALETTE_FADE_PTR RRC_GetPaletteFadeSystem( const RRC_WORK* work );
// ヒットテーブルを取得する
extern const GFL_UI_TP_HITTBL* RRC_GetHitTable( const RRC_WORK* work );
// 現在の画面を取得する
extern RADAR_SEQ RRC_GetNowSeq( const RRC_WORK* work );
// 直前の画面を取得する
extern RADAR_SEQ RRC_GetPrevSeq( const RRC_WORK* work );
// 画面遷移のトリガを取得する
extern SEQ_CHANGE_TRIG RRC_GetSeqChangeTrig( const RRC_WORK* work );
// 強制終了フラグを取得する
extern BOOL RRC_GetForceReturnFlag( const RRC_WORK* work );

//===============================================================================
// ■設定
//===============================================================================
// 現在の画面を登録する
extern void RRC_SetNowSeq( RRC_WORK* work, RADAR_SEQ seq );
// 画面遷移のトリガを登録する
extern void RRC_SetSeqChangeTrig( RRC_WORK* work, SEQ_CHANGE_TRIG trig );
// 強制終了フラグを立てる
extern void RRC_SetForceReturnFlag( RRC_WORK* work );
