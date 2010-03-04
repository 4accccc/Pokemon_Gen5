//////////////////////////////////////////////////////////////////////////////
/**
 * @brief  1つのパーセント表示を扱う構造体
 * @file   percentage.h
 * @author obata
 * @date   2010.03.01
 */
//////////////////////////////////////////////////////////////////////////////
#include <gflib.h>


//============================================================================
// ■パーセンテージの不完全型
//============================================================================
typedef struct _PERCENTAGE PERCENTAGE;


//============================================================================
// ■表示パラメータ
//============================================================================
typedef struct {
  u32 cgrIndex;      // OBJキャラデータの登録インデックス
  u32 plttIndex;     // OBJパレットデータの登録インデックス
  u32 cellAnimIndex; // OBJセルアニメデータの登録インデックス
  u16 setSerface;    // サーフェイス指定 ( CLSYS_DEFREND_MAIN/ CLSYS_DEFREND_SUB )
  u16 anmseqNumber;  // アニメーションシーケンス ( 数値 )
  u16 anmseqBase;    // アニメーションシーケンス ( 土台 )
} PERCENTAGE_DISP_PARAM;


//============================================================================
// ■生成・破棄
//============================================================================
extern PERCENTAGE* PERCENTAGE_Create( 
    HEAPID heapID, const PERCENTAGE_DISP_PARAM* dispParam, GFL_CLUNIT* clactUnit ); // 生成
extern void PERCENTAGE_Delete( PERCENTAGE* percentage ); // 破棄

//============================================================================
// ■制御
//============================================================================
extern void PERCENTAGE_SetPos( PERCENTAGE* percentage, int x, int y ); // 座標を設定する
extern void PERCENTAGE_SetValue( PERCENTAGE* percentage, int value ); // 数値を設定する
extern void PERCENTAGE_SetDrawEnable( PERCENTAGE* percentage, BOOL enable ); // 描画状態の変更
