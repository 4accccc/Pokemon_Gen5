//============================================================================
/**
 *
 *	@file		debug_hudson.h
 *	@brief  hudson用ユーティリティ
 *	@author		hosaka genya
 *	@data		2009.11.25
 *
 */
//============================================================================
#pragma once

//ハドソンデバッグを使う用にする
//（本来ならばDEBUG_ONLY_FOR_hudsonを別定義にしたいところなのだが
//相当いろんなソースに書いてあるので別定義にはしない）
#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_nagihashi_toru) || defined(DEBUG_ONLY_FOR_shimoyamada)
#define DEBUG_HUDSON_INPUT_AUTO_TEST  //L+Bとかで強制デバッグ実行できる
#define DEBUG_ONLY_FOR_hudson
#endif  //if defined
#endif  //PMDEBUG




#ifdef DEBUG_ONLY_FOR_hudson

#include <gflib.h>
#include <tcbl.h>
#include "system/main.h"    //HEAPIDの参照用
#include "gamesystem/gamesystem.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
#define HUDSON_TESTCODE_MAP_JUMP ("1")
#define HUDSON_TESTCODE_ALL_CONNECT ("2")
#define HUDSON_TESTCODE_ALL_WAZA ("3")
#define HUDSON_TESTCODE_ALL_WAZA2 ("4")
#define HUDSON_TESTCODE_ALL_POKE ("5")
#define HUDSON_TESTCODE_ALL_WAZA_CAM ("6")  ///< 全技カメラチェック
#define HUDSON_TESTCODE_POKE_CHANGE ("7")  
#define HUDSON_TESTCODE_JA_SHOP_CHECK ("8")   ///< JA施設チェック

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================
//ハドソンデバッグ用処理
extern void HUDSON_Init( GAMESYS_WORK* gsys, HEAPID heapID );
extern void HUDSON_Exit( GAMESYS_WORK* gsys );
extern void HUDSON_Main( GAMESYS_WORK* gsys );


//ツール類
extern BOOL HUDSON_IsSkipTitle( void );
extern BOOL HUDSON_IsTestCode( const char* str );


//=============================================================================
/**
 *								オートテスト用
 */
//=============================================================================
//-------------------------------------
///	オートテスト
//=====================================
typedef struct
{
  void (*Init)( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID );
  GFL_TCBLFUNC  *Update;     //タスク関数
  u32 (*GetSize)( void ); //ワークサイズ取得関数
} HUDSON_AUTOTEST_WORK;

//これはタスクと他の挙動をつなぐためのフラグです
extern void HUDSON_AUTOTEST_SetFlag( int flag, BOOL is_on );
extern BOOL HUDSON_AUTOTEST_GetFlag( int flag );

//---------------------------------
// HUDSONのみ実体を定義
//---------------------------------

// HUDSON用アサート
#define HUDSON_ASSERT(exp)  GF_ASSERT(exp)
// PRINT関数
#define HUDSON_Printf(...)  OS_Printf(__VA_ARGS__)
// PRINT関数(軽量版)
#define HUDSON_TPrintf(...) OS_TPrintf(__VA_ARGS__)

#else //DEBUG_ONLY_FOR_hudson

//---------------------------------
// 無効化
//---------------------------------
#define HUDSON_ASSERT(exp) ((void)0)
#define HUDSON_TPrintf(...) ((void)0)
#define HUDSON_Printf(...) ((void)0)

#endif //DEBUG_ONLY_FOR_hudson
