//////////////////////////////////////////////////////////////////////////////
/**
 * @brief  1つのパーセント表示を扱う構造体
 * @file   percentage.c
 * @author obata
 * @date   2010.03.01
 */
//////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "percentage.h"


//============================================================================
// ■定数
//============================================================================ 
#define PRINT_TARGET (2)  // デバッグ情報の出力先
#define OFFSET_X_100 (10)  // ベースからのx座標オフセット ( 100の位 )
#define OFFSET_Y_100 (-8) // ベースからのy座標オフセット ( 100の位 )
#define OFFSET_X_10  (15) // ベースからのx座標オフセット ( 10の位 )
#define OFFSET_Y_10  (-8) // ベースからのy座標オフセット ( 10の位 )
#define OFFSET_X_1   (20) // ベースからのx座標オフセット ( 1の位 )
#define OFFSET_Y_1   (-8) // ベースからのy座標オフセット ( 1の位 )

// セルアクターワークのインデックス
typedef enum {
  CLWK_BASE,      // 土台
  CLWK_VALUE_100, // 100の位
  CLWK_VALUE_10,  // 10の位
  CLWK_VALUE_1,   // 1の位
  CLWK_NUM,       // 総数
} CLWK_INDEX;


//============================================================================
// ■パーセンテージ
//============================================================================ 
struct _PERCENTAGE
{
  HEAPID heapID;

  u8 value; // 数値
  int x;    // x座標
  int y;    // y座標

  GFL_CLUNIT* clactUnit;             // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ]; // セルアクターワーク
  PERCENTAGE_DISP_PARAM dispParam;   // 表示パラメータ
};


//============================================================================
// ■非公開関数
//============================================================================ 

//----------------------------------------------------------------------------
// □表示制御
//----------------------------------------------------------------------------
void SetDrawEnable( PERCENTAGE* percentage, BOOL enable ); // 描画状態を設定する

//----------------------------------------------------------------------------
// □設定
//----------------------------------------------------------------------------
// layer 1
void SetupClactWorks( PERCENTAGE* percentage ); // セルアクターワークをセットアップする
// layer 0
void SetValue( PERCENTAGE* percentage, u8 value ); // 数値を設定する
void SetPos( PERCENTAGE* percentage, int x, int y ); // 座標を設定する

//----------------------------------------------------------------------------
// □取得
//----------------------------------------------------------------------------
u8 GetValue_1( const PERCENTAGE* percentage ); // 1の位の数字を取得する
u8 GetValue_10( const PERCENTAGE* percentage ); // 10の位の数字を取得する
u8 GetValue_100( const PERCENTAGE* percentage ); // 100の位の数字を取得する
void GetPos_1( const PERCENTAGE* percentage, s16* destX, s16* destY ); // 1の位の数字の座標を取得する
void GetPos_10( const PERCENTAGE* percentage, s16* destX, s16* destY ); // 10の位の数字の座標を取得する
void GetPos_100( const PERCENTAGE* percentage, s16* destX, s16* destY ); // 100の位の数字の座標を取得する

//----------------------------------------------------------------------------
// □生成・初期化・破棄
//----------------------------------------------------------------------------
// layer 1
PERCENTAGE* CreatePercentage( HEAPID heapID ); // パーセンテージを生成する
void DeletePercentage( PERCENTAGE* percentage ); // パーセンテージを破棄する
void InitPercentage( PERCENTAGE* percentage, HEAPID heapID ); // パーセンテージを初期化する
void SetupPercentage( PERCENTAGE* percentage,
    const PERCENTAGE_DISP_PARAM* dispParam, GFL_CLUNIT* clactUnit ); // パーセンテージをセットアップする
void CleanUpPercentage( PERCENTAGE* percentage ); // パーセンテージをクリーンアップする
// layer 0
void SetClactUnit( PERCENTAGE* percentage, GFL_CLUNIT* clactUnit ); // セルアクターユニットをセットする
void CreateClactWorks( PERCENTAGE* percentage ); // セルアクターワークを生成する
void DeleteClactWorks( PERCENTAGE* percentage ); // セルアクターワークを破棄する
void SetDispParams( PERCENTAGE* percentage, 
    const PERCENTAGE_DISP_PARAM* dispParam ); // 表示パラメータをセットする



//============================================================================
// ■生成・破棄
//============================================================================

//----------------------------------------------------------------------------
/**
 * @brief 生成
 *
 * @param percentage
 * @parma dispParam 表示パラメータ
 * @parma clactUnit セルアクターユニット
 *
 * @return 生成したインスタンス
 */
//----------------------------------------------------------------------------
PERCENTAGE* PERCENTAGE_Create( 
    HEAPID heapID, const PERCENTAGE_DISP_PARAM* dispParam, GFL_CLUNIT* clactUnit )
{
  PERCENTAGE* percentage;
  percentage = CreatePercentage( heapID ); // 生成
  InitPercentage( percentage, heapID ); // 初期化
  SetupPercentage( percentage, dispParam, clactUnit ); // セットアップ
  return percentage;
}

//----------------------------------------------------------------------------
/**
 * @brief 破棄
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void PERCENTAGE_Delete( PERCENTAGE* percentage )
{
  CleanUpPercentage( percentage ); // クリーンアップ
  DeletePercentage( percentage ); // 破棄
}


//============================================================================
// ■制御
//============================================================================

//----------------------------------------------------------------------------
/**
 * @brief 座標を設定する
 *
 * @param percentage
 * @param x ベース部のx座標
 * @param y ベース部のy座標
 */
//----------------------------------------------------------------------------
void PERCENTAGE_SetPos( PERCENTAGE* percentage, int x, int y )
{
  SetPos( percentage, x, y );
  SetupClactWorks( percentage );
}

//----------------------------------------------------------------------------
/**
 * @brief 数値を設定する
 *
 * @param percentage
 * @param value 数値
 */
//----------------------------------------------------------------------------
void PERCENTAGE_SetValue( PERCENTAGE* percentage, int value )
{
  SetValue( percentage, value );
  SetupClactWorks( percentage );
}

//----------------------------------------------------------------------------
/**
 * @brief 描画状態の変更
 *
 * @param percentage
 * @param enable 描画するかどうか
 */
//----------------------------------------------------------------------------
void PERCENTAGE_SetDrawEnable( PERCENTAGE* percentage, BOOL enable )
{
  SetDrawEnable( percentage, enable );
}



//============================================================================ 
// ■表示制御
//============================================================================ 

//----------------------------------------------------------------------------
/**
 * @brief 描画状態を設定する
 *
 * @param percentage
 * @param enable 描画するかどうか
 */
//----------------------------------------------------------------------------
void SetDrawEnable( PERCENTAGE* percentage, BOOL enable )
{
  // 土台
  GFL_CLACT_WK_SetDrawEnable( percentage->clactWork[ CLWK_BASE ], enable );

  // 100の位
  if( GetValue_100(percentage) != 0 ) {
    GFL_CLACT_WK_SetDrawEnable( percentage->clactWork[ CLWK_VALUE_100 ], enable );
  }

  // 10の位
  if( (GetValue_100(percentage) != 0) || (GetValue_10(percentage) != 0) ) {
    GFL_CLACT_WK_SetDrawEnable( percentage->clactWork[ CLWK_VALUE_10 ], enable );
  }

  // 1の位
  GFL_CLACT_WK_SetDrawEnable( percentage->clactWork[ CLWK_VALUE_1 ], enable );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: set draw enable ==> %d\n", enable );
}




//============================================================================ 
// ■設定
//============================================================================ 

//----------------------------------------------------------------------------
/**
 * @brief セルアクターワークをセットアップする
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void SetupClactWorks( PERCENTAGE* percentage )
{ 
  GFL_CLWK* clwk;
  const PERCENTAGE_DISP_PARAM* dispParam;
  GFL_CLACTPOS pos;

  dispParam = &( percentage->dispParam );

  // 土台
  clwk = percentage->clactWork[ CLWK_BASE ];
  pos.x = percentage->x;
  pos.y = percentage->y;
  GFL_CLACT_WK_SetPos( clwk, &pos, dispParam->setSerface );
  GFL_CLACT_WK_SetAnmSeq( clwk, dispParam->anmseqBase );
  GFL_CLACT_WK_SetAnmFrame( clwk, 0 );
  GFL_CLACT_WK_StopAnm( clwk );
  GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );

  // 100の位
  clwk = percentage->clactWork[ CLWK_VALUE_100 ];
  GetPos_100( percentage, &pos.x, &pos.y );
  GFL_CLACT_WK_SetPos( clwk, &pos, dispParam->setSerface );
  GFL_CLACT_WK_SetAnmSeq( clwk, dispParam->anmseqNumber );
  GFL_CLACT_WK_SetAnmFrame( clwk, GetValue_100(percentage) << FX32_SHIFT );
  GFL_CLACT_WK_StopAnm( clwk );
  GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );

  // 10の位
  clwk = percentage->clactWork[ CLWK_VALUE_10 ];
  GetPos_10( percentage, &pos.x, &pos.y );
  GFL_CLACT_WK_SetPos( clwk, &pos, dispParam->setSerface );
  GFL_CLACT_WK_SetAnmSeq( clwk, dispParam->anmseqNumber );
  GFL_CLACT_WK_SetAnmFrame( clwk, GetValue_10(percentage) << FX32_SHIFT );
  GFL_CLACT_WK_StopAnm( clwk );
  GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );

  // 1の位
  clwk = percentage->clactWork[ CLWK_VALUE_1 ];
  GetPos_1( percentage, &pos.x, &pos.y );
  GFL_CLACT_WK_SetPos( clwk, &pos, dispParam->setSerface );
  GFL_CLACT_WK_SetAnmSeq( clwk, dispParam->anmseqNumber );
  GFL_CLACT_WK_SetAnmFrame( clwk, GetValue_1(percentage) << FX32_SHIFT );
  GFL_CLACT_WK_StopAnm( clwk );
  GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: setup clact works\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief 数値を設定する
 *
 * @param percentage
 * @param value      設定する値
 */
//----------------------------------------------------------------------------
void SetValue( PERCENTAGE* percentage, u8 value )
{
  percentage->value = value;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: set value ==> %d\n", value );
}

//----------------------------------------------------------------------------
/**
 * @brief 座標を設定する
 *
 * @param percentage
 * @param x x座標
 * @param y y座標
 */
//----------------------------------------------------------------------------
void SetPos( PERCENTAGE* percentage, int x, int y )
{
  percentage->x = x;
  percentage->y = y;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: set pos (%d, %d)\n", x, y );
}



//============================================================================ 
// ■取得
//============================================================================ 

//----------------------------------------------------------------------------
/**
 * @brief 1の位の数字を取得する
 *
 * @param percentage
 *
 * @return 1の位の数値
 */
//----------------------------------------------------------------------------
u8 GetValue_1( const PERCENTAGE* percentage )
{
  return percentage->value % 10;
}

//----------------------------------------------------------------------------
/**
 * @brief 10の位の数字を取得する
 *
 * @param percentage
 *
 * @return 10の位の数値
 */
//----------------------------------------------------------------------------
u8 GetValue_10( const PERCENTAGE* percentage )
{
  return ( percentage->value % 100 ) / 10;
}

//----------------------------------------------------------------------------
/**
 * @brief 100の位の数字を取得する
 *
 * @param percentage
 *
 * @return 100の位の数値
 */
//----------------------------------------------------------------------------
u8 GetValue_100( const PERCENTAGE* percentage )
{
  return percentage->value / 100;
}

//----------------------------------------------------------------------------
/**
 * @brief 1の位の数字の座標を取得する
 *
 * @param percentage
 * @param destX 求めたx座標の格納先 
 * @param destY 求めたy座標の格納先
 *
 * @return 1の位の数値の座標
 */
//----------------------------------------------------------------------------
void GetPos_1( const PERCENTAGE* percentage, s16* destX, s16* destY )
{
  *destX = percentage->x + OFFSET_X_1;
  *destY = percentage->y + OFFSET_Y_1;
}

//----------------------------------------------------------------------------
/**
 * @brief 10の位の数字の座標を取得する
 *
 * @param percentage
 * @param destX 求めたx座標の格納先 
 * @param destY 求めたy座標の格納先
 *
 * @return 10の位の数値の座標
 */
//----------------------------------------------------------------------------
void GetPos_10( const PERCENTAGE* percentage, s16* destX, s16* destY )
{
  *destX = percentage->x + OFFSET_X_10;
  *destY = percentage->y + OFFSET_Y_10;
}

//----------------------------------------------------------------------------
/**
 * @brief 100の位の数字の座標を取得する
 *
 * @param percentage
 * @param destX 求めたx座標の格納先 
 * @param destY 求めたy座標の格納先
 *
 * @return 100の位の数値の座標
 */
//----------------------------------------------------------------------------
void GetPos_100( const PERCENTAGE* percentage, s16* destX, s16* destY )
{
  *destX = percentage->x + OFFSET_X_100;
  *destY = percentage->y + OFFSET_Y_100;
}



//============================================================================ 
// ■生成・初期化・破棄
//============================================================================ 

//----------------------------------------------------------------------------
/**
 * @brief パーセンテージを生成する
 *
 * @param heapID
 *
 * @return 生成したオブジェクト
 */
//----------------------------------------------------------------------------
PERCENTAGE* CreatePercentage( HEAPID heapID )
{
  PERCENTAGE* percentage;
  percentage = GFL_HEAP_AllocMemory( heapID, sizeof(PERCENTAGE) );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: create percentage\n" );

  return percentage;
}

//----------------------------------------------------------------------------
/**
 * @brief パーセンテージを破棄する
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void DeletePercentage( PERCENTAGE* percentage )
{
  GFL_HEAP_FreeMemory( percentage );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: delete percentage\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief パーセンテージを初期化する
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void InitPercentage( PERCENTAGE* percentage, HEAPID heapID )
{
  int i;

  percentage->heapID = heapID;
  percentage->value = 0;
  percentage->clactUnit = NULL;

  for( i=0; i < CLWK_NUM; i++ )
  {
    percentage->clactWork[i] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: init percentage\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief パーセンテージをセットアップする
 *
 * @param percentage
 * @param dispParam  表示パラメータ
 * @param clactUnit  セルアクターユニット
 */
//----------------------------------------------------------------------------
void SetupPercentage( 
    PERCENTAGE* percentage, const PERCENTAGE_DISP_PARAM* dispParam, GFL_CLUNIT* clactUnit )
{
  SetClactUnit( percentage, clactUnit );  // セルアクターユニットをセット
  SetDispParams( percentage, dispParam ); // 表示パラメータをセット
  CreateClactWorks( percentage );         // セルアクターワークを生成

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: setup percentage\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief パーセンテージをクリーンアップする
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void CleanUpPercentage( PERCENTAGE* percentage )
{
  DeleteClactWorks( percentage ); // セルアクターワークを破棄

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: clean up percentage\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief セルアクターユニットをセットする
 *
 * @param percentage
 * @param clactUnit
 */
//----------------------------------------------------------------------------
void SetClactUnit( PERCENTAGE* percentage, GFL_CLUNIT* clactUnit )
{
  GF_ASSERT( percentage->clactUnit == NULL );

  percentage->clactUnit = clactUnit;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: set clact unit\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief セルアクターワークを生成する
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void CreateClactWorks( PERCENTAGE* percentage )
{ 
  int idx;
  const PERCENTAGE_DISP_PARAM* dispParam;
  GFL_CLWK_DATA data;

  dispParam = &(percentage->dispParam);
  data.pos_x   = 0;
  data.pos_y   = 0;
  data.anmseq  = 0;
  data.softpri = 0;
  data.bgpri   = 0;

  for(idx=0; idx < CLWK_NUM; idx++ )
  {
    GF_ASSERT( percentage->clactWork[ idx ] == NULL );

    percentage->clactWork[ idx ] =
      GFL_CLACT_WK_Create( percentage->clactUnit, 
                           dispParam->cgrIndex, 
                           dispParam->plttIndex, 
                           dispParam->cellAnimIndex, 
                           &data, 
                           dispParam->setSerface, 
                           percentage->heapID );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: create clact works\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void DeleteClactWorks( PERCENTAGE* percentage )
{
  int idx;

  for(idx=0; idx < CLWK_NUM; idx++ )
  {
    GF_ASSERT( percentage->clactWork[ idx ] );

    GFL_CLACT_WK_Remove( percentage->clactWork[ idx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: delete clact works\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief 表示パラメータをセットする
 *
 * @param percentage
 */
//----------------------------------------------------------------------------
void SetDispParams( PERCENTAGE* percentage, const PERCENTAGE_DISP_PARAM* dispParam )
{
  percentage->dispParam = *dispParam;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PERCENTAGE: set disp params\n" );
} 
