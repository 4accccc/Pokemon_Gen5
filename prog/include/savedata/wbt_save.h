//============================================================================================
/**
 * @file  wbt_save.h
 * @brief   WBT用セーブデータ関連
 * @author  tamada@gamefreak.co.jp
 * @date  2011.04.05
 */
//============================================================================================
#pragma once

//======================================================================
//======================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  /// WBT優勝回数カウントの上限値
  WBTWIN_MAX = 9999,
};

//----------------------------------------------------------
///各トーナメントでの優勝回数カウント指定
//----------------------------------------------------------
typedef enum {
  WBTWIN_COUNT_POKETYPE_NORMAL   = 0,
  WBTWIN_COUNT_POKETYPE_KAKUTOU  = 1,
  WBTWIN_COUNT_POKETYPE_HIKOU    = 2,
  WBTWIN_COUNT_POKETYPE_DOKU     = 3,
  WBTWIN_COUNT_POKETYPE_JIMEN    = 4,
  WBTWIN_COUNT_POKETYPE_IWA      = 5,
  WBTWIN_COUNT_POKETYPE_MUSHI    = 6,
  WBTWIN_COUNT_POKETYPE_GHOST    = 7,
  WBTWIN_COUNT_POKETYPE_HAGANE   = 8,
  WBTWIN_COUNT_POKETYPE_HONOO    = 9,
  WBTWIN_COUNT_POKETYPE_MIZU     = 10,
  WBTWIN_COUNT_POKETYPE_KUSA     = 11,
  WBTWIN_COUNT_POKETYPE_DENKI    = 12,
  WBTWIN_COUNT_POKETYPE_ESPER    = 13,
  WBTWIN_COUNT_POKETYPE_KOORI    = 14,
  WBTWIN_COUNT_POKETYPE_DRAGON   = 15,
  WBTWIN_COUNT_POKETYPE_AKU      = 16,

  WBTWIN_COUNT_ALLTOURNAMENT     = 17,
  WBTWIN_COUNT_CHAMPION          = 18,

  WBTWIN_COUNT_MAX,
}WBTWIN_COUNT;

//----------------------------------------------------------
/// WBT用セーブ：フラグ操作関連
//----------------------------------------------------------
typedef enum {
  WBTSAVE_FLAG_ENABLE_TYPE_TOURNAMENT = 0,  ///<タイプトーナメントへ参加できるか？
  WBTSAVE_FLAG_ENABLE_CHAMPION_TOURNAMENT,  ///<チャンピオントーナメントへ参加できるか？
  WBTSAVE_FLAG_WIN_CHAMPION_TOURNAMENT,     ///<チャンピオントーナメントで優勝したか？

  WBTSAVE_FLAG_MAX = 32,
}WBTSAVE_FLAG;

//======================================================================
//	typedef struct
//======================================================================
typedef struct _WBTSAVE WBTSAVE;

//======================================================================
//======================================================================
//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern u32 WBTSAVE_GetWorkSize(void);
extern void WBTSAVE_Init( WBTSAVE * wbtsave );

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
extern void WBTSAVE_UpdateChampionKey( WBTSAVE * save );

extern u16 WBTSAVE_GetWinCount( const WBTSAVE * wbtsave, WBTWIN_COUNT id );
extern void WBTSAVE_IncWinCount( WBTSAVE * wbtsave, WBTWIN_COUNT id );

extern BOOL WBTSAVE_GetFlag( const WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id );
extern void WBTSAVE_SetFlag( WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id );
extern void WBTSAVE_ResetFlag( WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id );

