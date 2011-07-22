//============================================================================================
/**
 * @file  wbt_save.h
 * @brief   WBT用セーブデータ関連
 * @author  tamada@gamefreak.co.jp
 * @date  2011.04.05
 */
//============================================================================================
#pragma once

#include "savedata/save_control.h"  //SAVE_CONTROL_WORK
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
  WBTWIN_COUNT_POKETYPE_NORMAL   = 0,   ///<タイプトーナメント優勝回数：ノーマル
  WBTWIN_COUNT_POKETYPE_KAKUTOU  = 1,   ///<タイプトーナメント優勝回数：格闘
  WBTWIN_COUNT_POKETYPE_HIKOU    = 2,   ///<タイプトーナメント優勝回数：飛行
  WBTWIN_COUNT_POKETYPE_DOKU     = 3,   ///<タイプトーナメント優勝回数：毒
  WBTWIN_COUNT_POKETYPE_JIMEN    = 4,   ///<タイプトーナメント優勝回数：地面
  WBTWIN_COUNT_POKETYPE_IWA      = 5,   ///<タイプトーナメント優勝回数：岩
  WBTWIN_COUNT_POKETYPE_MUSHI    = 6,   ///<タイプトーナメント優勝回数：虫
  WBTWIN_COUNT_POKETYPE_GHOST    = 7,   ///<タイプトーナメント優勝回数：ゴースト
  WBTWIN_COUNT_POKETYPE_HAGANE   = 8,   ///<タイプトーナメント優勝回数：はがね
  WBTWIN_COUNT_POKETYPE_HONOO    = 9,   ///<タイプトーナメント優勝回数：炎
  WBTWIN_COUNT_POKETYPE_MIZU     = 10,  ///<タイプトーナメント優勝回数：水
  WBTWIN_COUNT_POKETYPE_KUSA     = 11,  ///<タイプトーナメント優勝回数：草
  WBTWIN_COUNT_POKETYPE_DENKI    = 12,  ///<タイプトーナメント優勝回数：電気
  WBTWIN_COUNT_POKETYPE_ESPER    = 13,  ///<タイプトーナメント優勝回数：エスパー
  WBTWIN_COUNT_POKETYPE_KOORI    = 14,  ///<タイプトーナメント優勝回数：氷
  WBTWIN_COUNT_POKETYPE_DRAGON   = 15,  ///<タイプトーナメント優勝回数：ドラゴン
  WBTWIN_COUNT_POKETYPE_AKU      = 16,  ///<タイプトーナメント優勝回数：悪

  WBTWIN_COUNT_CHAMPION          = 17,  ///<チャンピオントーナメント優勝回数

  WBTWIN_COUNT_HODOMOE           = 18,
  WBTWIN_COUNT_ISSYU             = 19,
  WBTWIN_COUNT_KANTO             = 20,
  WBTWIN_COUNT_JOUTO             = 21,
  WBTWIN_COUNT_HOUEN             = 22,
  WBTWIN_COUNT_SINOU             = 23,
  WBTWIN_COUNT_WORLD             = 24,
  WBTWIN_COUNT_WCS               = 25,
  WBTWIN_COUNT_RENTAL            = 26,
  WBTWIN_COUNT_RENTALMASTER      = 27,
  WBTWIN_COUNT_SHUFFLE           = 28,
  WBTWIN_COUNT_SHUFFLEMASTER     = 29,

  WBTWIN_COUNT_MAX,
}WBTWIN_COUNT;

//----------------------------------------------------------
/// WBT用セーブ：フラグ操作関連
//----------------------------------------------------------
typedef enum {
  WBTSAVE_FLAG_MAX = 64,
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

extern WBTSAVE * SAVEDATA_GetWBTSave( SAVE_CONTROL_WORK * sv );
//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
extern void WBTSAVE_UpdateChampionKey( WBTSAVE * save );

extern u16 WBTSAVE_GetWinCount( const WBTSAVE * wbtsave, WBTWIN_COUNT id );
extern void WBTSAVE_SetWinCount( WBTSAVE * wbtsave, WBTWIN_COUNT id, u16 value );
extern void WBTSAVE_IncWinCount( WBTSAVE * wbtsave, WBTWIN_COUNT id );

extern BOOL WBTSAVE_GetFlag( const WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id );
extern void WBTSAVE_SetFlag( WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id );
extern void WBTSAVE_ResetFlag( WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id );

