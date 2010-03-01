///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  矢印
 * @file   arrow.c
 * @author obata
 * @date   2010.02.22
 */
///////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "arrow.h"


//=====================================================================================
// ■定数
//=====================================================================================
#define PRINT_TARGET   (2)  // デバッグ情報の出力先
#define MAX_CELL_NUM   (40) // 最大セル数
#define ANIME_SPEED    (18.0f) // アニメーション再生速度
#define ANIME_FRAMES   (36) // アニメーションフレーム数
#define STRETCH_FRAMES (ANIME_FRAMES/ANIME_SPEED) // 伸長アニメのフレーム数
#define CELL_WIDTH     (8)  // セルの幅 ( ドット数 )
#define CELL_HEIGHT    (8)  // セルの高さ ( ドット数 )

// 矢印の状態
typedef enum { 
  ARROW_STATE_WAIT,     // 待機 ( 非表示 )
  ARROW_STATE_STRETCH,  // 伸長
  ARROW_STATE_STAY,     // 待機 ( 表示 )
} ARROW_STATE;

// 矢印のセルタイプ
typedef enum {
  ARROW_CELL_H,      // 横へ伸びるセル
  ARROW_CELL_V,      // 縦に伸びるセル
  ARROW_CELL_CORNER, // 角のセル
  ARROW_CELL_END,    // 終点のセル
} ARROW_CELL_TYPE;


//=====================================================================================
// ■矢印を構成するセル
//=====================================================================================
typedef struct {
  BOOL            bootFlag;  // 起動しているかどうか
  u16             bootFrame; // 起動するフレーム数
  u8              left;      // 左上のx座標
  u8              top;       // 左上のy座標
  ARROW_CELL_TYPE type;      // セルタイプ
  GFL_CLWK*       clactWork; // セルアクターワーク
} ARROW_CELL;


//=====================================================================================
// ■矢印
//=====================================================================================
struct _ARROW
{
  HEAPID           heapID;
  GFL_CLUNIT*      clactUnit; // セルアクターユニット
  ARROW_DISP_PARAM dispParam; // 表示パラメータ

  ARROW_STATE state;      // 現在の状態
  u32         stateCount; // 状態カウンタ

 ARROW_CELL cell[ MAX_CELL_NUM ];// 矢印を構成するセル
  u8         cellNum;             // 使用するセルの数
};




//=====================================================================================
// ■
//=====================================================================================

//-------------------------------------------------------------------------------------
// 動作
//-------------------------------------------------------------------------------------
static void ArrowMain( ARROW* arrow ); // メイン動作

static void ArrowAct_WAIT   ( ARROW* arrow ); // 状態メイン動作 ( ARROW_STATE_WAIT )
static void ArrowAct_STRETCH( ARROW* arrow ); // 状態メイン動作 ( ARROW_STATE_STRETCH )
static void ArrowAct_STAY   ( ARROW* arrow ); // 状態メイン動作 ( ARROW_STATE_STAY )

static void ChangeState( ARROW* arrow, ARROW_STATE nextState ); // 状態を変更する

static BOOL CheckAllCellBoot( const ARROW* arrow ); // 全てのセルが起動しているかどうかを判定する
static void BootCell( ARROW* arrow, u8 cellIdx ); // セルを起動する
static void Vanish( ARROW* arrow ); // 全てのセルを非表示にする

//-------------------------------------------------------------------------------------
// 設定
//------------------------------------------------------------------------------------- 
// セルの動作パラメータ
static void SetupCellAct( ARROW* arrow, int startX, int startY, int endX, int endY ); // セルの動作をセットアップする
static void SetCellParams( ARROW_CELL* cell, u8 left, u8 top, u16 frame, ARROW_CELL_TYPE type ); // セルのパラメータを設定する

// セルの表示パラメータ
static void SetDispParams( ARROW* arrow, const ARROW_DISP_PARAM* param ); // 表示パラメータを設定する

//-------------------------------------------------------------------------------------
// 初期化・生成・破棄・準備
//-------------------------------------------------------------------------------------
static void InitArrow( ARROW* arrow, HEAPID heapID ); // 矢印 初期化
static ARROW* CreateArrow( HEAPID heapID );           // 矢印 生成
static void DeleteArrow( ARROW* arrow );              // 矢印 破棄
static void CreateClactUnit( ARROW* arrow ); // セルアクターユニット 生成
static void DeleteClactUnit( ARROW* arrow ); // セルアクターユニット 破棄
static void CreateClactWorks( ARROW* arrow ); // セルアクターワーク 生成
static void DeleteClactWorks( ARROW* arrow ); // セルアクターワーク 破棄

//-------------------------------------------------------------------------------------
// デバッグ
//-------------------------------------------------------------------------------------
static void DebugPrint( const ARROW* arrow );




//=====================================================================================
// ■外部公開関数
//===================================================================================== 

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印を生成する
 *
 * @param heapID    使用するヒープID
 * @param dispParam 表示パラメータ
 *
 * @return 作成した矢印オブジェクト
 */
//-------------------------------------------------------------------------------------
ARROW* ARROW_Create( HEAPID heapID, const ARROW_DISP_PARAM* dispParam )
{
  ARROW* arrow;
  arrow = CreateArrow( heapID );
  InitArrow( arrow, heapID );
  SetDispParams( arrow, dispParam );
  CreateClactUnit( arrow );
  CreateClactWorks( arrow );
  return arrow;
}

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印を破棄する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
void ARROW_Delete( ARROW* arrow )
{
  DeleteClactWorks( arrow );
  DeleteClactUnit( arrow );
  DeleteArrow( arrow );
}

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印のパラメータをセットアップする
 *
 * @param arrow
 * @param startX  始点x座標
 * @param startY  始点y座標
 * @param endX    終点x座標
 * @param endY    終点y座標
 */
//-------------------------------------------------------------------------------------
void ARROW_Setup( ARROW* arrow, int startX, int startY, int endX, int endY ) 
{
  SetupCellAct( arrow, startX, startY, endX, endY );
  DebugPrint( arrow );
}

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印メイン動作処理
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
void ARROW_Main( ARROW* arrow )
{
  ArrowMain( arrow );
}

//-------------------------------------------------------------------------------------
/**
 * @brief アニメーション開始
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
void ARROW_StartAnime( ARROW* arrow )
{
  ChangeState( arrow, ARROW_STATE_STRETCH );
}

//-------------------------------------------------------------------------------------
/**
 * @brief 消去する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
void ARROW_Vanish( ARROW* arrow )
{
  Vanish( arrow );
}


//=====================================================================================
// ■動作
//=====================================================================================

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印メイン動作
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void ArrowMain( ARROW* arrow )
{
  // 状態ごとの処理
  switch( arrow->state ) {
  case ARROW_STATE_WAIT:    ArrowAct_WAIT   ( arrow ); break;
  case ARROW_STATE_STRETCH: ArrowAct_STRETCH( arrow ); break;
  case ARROW_STATE_STAY:    ArrowAct_STAY   ( arrow ); break;
  default: GF_ASSERT(0);
  }

  // 状態カウンタ更新
  arrow->stateCount++;
}

//-------------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( ARROW_STATE_WAIT )
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void ArrowAct_WAIT( ARROW* arrow )
{
}

//-------------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( ARROW_STATE_STRETCH )
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void ArrowAct_STRETCH( ARROW* arrow )
{
  int cellIdx;

  // セルの起動
  for( cellIdx=0; cellIdx < arrow->cellNum; cellIdx++ )
  {
    if( (arrow->cell[ cellIdx ].bootFlag == FALSE) &&
        (arrow->cell[ cellIdx ].bootFrame < arrow->stateCount) )
    {
      BootCell( arrow, cellIdx );
    }
  }

  // すべてのセルが起動
  if( CheckAllCellBoot( arrow ) )
  {
    ChangeState( arrow, ARROW_STATE_STAY );
  }
}

//-------------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( ARROW_STATE_STAY )
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void ArrowAct_STAY( ARROW* arrow )
{
}

//-------------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void ChangeState( ARROW* arrow, ARROW_STATE nextState )
{
  // 状態を更新
  arrow->state = nextState;
  arrow->stateCount = 0;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: change state ==> " );
  switch( nextState ) {
  case ARROW_STATE_WAIT:    OS_TFPrintf( PRINT_TARGET, "WAIT");     break;
  case ARROW_STATE_STRETCH: OS_TFPrintf( PRINT_TARGET, "STRETCH");  break;
  case ARROW_STATE_STAY:    OS_TFPrintf( PRINT_TARGET, "STAY");     break;
  default:                  OS_TFPrintf( PRINT_TARGET, "UNKNOWN "); break;
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief 全てのセルが起動しているかどうかを判定する
 *
 * @param arrow
 *
 * @return すべてのセルが起動している場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------------
static BOOL CheckAllCellBoot( const ARROW* arrow )
{
  int cellIdx;
  int cellNum;

  cellNum = arrow->cellNum;

  for( cellIdx=0; cellIdx < cellNum; cellIdx++ )
  {
    // 起動していないセルを発見
    if( arrow->cell[ cellIdx ].bootFlag == FALSE )
    {
      return FALSE;
    }
  }
  return TRUE;
}

//-------------------------------------------------------------------------------------
/**
 * @brief セルを起動する
 *
 * @param arrow
 * @param cellIdx 起動するセルのインデックス
 */
//-------------------------------------------------------------------------------------
static void BootCell( ARROW* arrow, u8 cellIdx )
{
  ARROW_CELL* cell;
  GFL_CLWK* clwk;
  GFL_CLACTPOS pos;
  u16 anmseq;

  // インデックスエラー
  GF_ASSERT( cellIdx < MAX_CELL_NUM );

  cell  = &( arrow->cell[ cellIdx ] );
  clwk  = cell->clactWork;
  pos.x = cell->left;
  pos.y = cell->top;

  // 起動
  cell->bootFlag = TRUE;

  // アニメーション開始
  switch( cell->type ) {
  case ARROW_CELL_H:      anmseq = arrow->dispParam.anmseqH; break;
  case ARROW_CELL_V:      anmseq = arrow->dispParam.anmseqV; break;
  case ARROW_CELL_CORNER: anmseq = arrow->dispParam.anmseqCorner; break;
  case ARROW_CELL_END:    anmseq = arrow->dispParam.anmseqEnd; break;
  default: GF_ASSERT(0);
  }
  GFL_CLACT_WK_SetPos( clwk, &pos, arrow->dispParam.setSerface );
  GFL_CLACT_WK_SetAnmSeq( clwk, anmseq );
  GFL_CLACT_WK_SetAnmFrame( clwk, 0 );
  GFL_CLACT_WK_SetAnmMode( clwk, CLSYS_ANMPM_FORWARD );
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( clwk, FX32_CONST(ANIME_SPEED) );
  GFL_CLACT_WK_StartAnm( clwk );
  GFL_CLACT_WK_SetDrawEnable( clwk, TRUE );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: boot cell[%d]\n", cellIdx );
}

//-------------------------------------------------------------------------------------
/**
 * @breif すべてのセルを非表示にする
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void Vanish( ARROW* arrow )
{
  int idx;
  int cellNum;

  cellNum = arrow->cellNum;

  // すべてのセルを非表示にする
  for(idx=0; idx < cellNum; idx++ )
  {
    GFL_CLACT_WK_SetDrawEnable( arrow->cell[ idx ].clactWork, FALSE );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: vanish\n" );
}


//-------------------------------------------------------------------------------------
/**
 * @brief セルのパラメータを設定する
 *
 * @param arrow
 * @param startX 始点x座標
 * @param startY 始点y座標
 * @param endX   終点x座標
 * @param endY   終点y座標
 */
//-------------------------------------------------------------------------------------
static void SetupCellAct( ARROW* arrow, int startX, int startY, int endX, int endY )
{
  int cellIdx, count;
  float frame;
  int left, top;
  int length;
  int hCellNum, vCellNum;

  GF_ASSERT( endX <= startX ); // 左にしか伸びない
  GF_ASSERT( startY <= endY ); // 下にしか伸びない

  frame   = 0;
  cellIdx = 0;

  // 横に伸びるセル
  length   = startX - endX;
  length  -= CELL_WIDTH/2;
  hCellNum = length / CELL_WIDTH;
  left     = startX - CELL_WIDTH;
  top      = startY - CELL_HEIGHT/2;
  for( count=0; count < hCellNum; count++ )
  {
    SetCellParams( &( arrow->cell[ cellIdx ] ), left, top, frame, ARROW_CELL_H );
    left  -= CELL_WIDTH;
    frame += STRETCH_FRAMES - 1;
    cellIdx++;
  } 

  // 横に伸びるセル ( 調整 )
  if( (length % CELL_WIDTH) != 0 )
  {
    int diff;
    left   = endX + CELL_WIDTH/2;
    diff   = length - (hCellNum * CELL_WIDTH);
    frame -= STRETCH_FRAMES - (STRETCH_FRAMES * diff / CELL_WIDTH);
    SetCellParams( &( arrow->cell[ cellIdx ] ), left, top, frame, ARROW_CELL_H );
    frame += STRETCH_FRAMES - 1;
    cellIdx++;
  }

  // コーナーのセル
  left = endX - CELL_WIDTH/2;
  top  = startY - CELL_HEIGHT/2;
  SetCellParams( &( arrow->cell[ cellIdx ] ), left, top, frame, ARROW_CELL_CORNER );
  top   += CELL_HEIGHT;
  frame += STRETCH_FRAMES - 1;
  cellIdx++;

  // 縦に伸びるセル
  length   = endY - startY;
  length  -= CELL_HEIGHT/2;
  vCellNum = length / CELL_HEIGHT;
  left     = endX - CELL_WIDTH/2;
  top      = startY + CELL_HEIGHT/2;
  for( count=0; count < vCellNum; count++ )
  {
    SetCellParams( &( arrow->cell[ cellIdx ] ), left, top, frame, ARROW_CELL_V );
    top   += CELL_HEIGHT;
    frame += STRETCH_FRAMES - 1;
    cellIdx++;
  } 

  // 縦に伸びるセル ( 調整 )
  if( (length % CELL_WIDTH) != 0 )
  {
    int diff;
    top    = endY - CELL_HEIGHT;
    diff   = length - (vCellNum * CELL_HEIGHT);
    frame -= STRETCH_FRAMES - (STRETCH_FRAMES * diff / CELL_HEIGHT);
    SetCellParams( &( arrow->cell[ cellIdx ] ), left, top, frame, ARROW_CELL_V );
    frame += STRETCH_FRAMES - 1;
    cellIdx++;
  }

  // 終点
  left = endX - CELL_WIDTH/2;
  top  = endY - CELL_HEIGHT/2;
  SetCellParams( &( arrow->cell[ cellIdx ] ), left, top, frame, ARROW_CELL_END );
  cellIdx++;

  // セル数
  arrow->cellNum = cellIdx;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: setup cells\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief セルのパラメータを設定する
 *
 * @param cell  設定するセル
 * @param left  左上x座標
 * @param top   左上y座標
 * @param frame 起動フレーム数
 * @param type  セルタイプ
 */
//-------------------------------------------------------------------------------------
static void SetCellParams( ARROW_CELL* cell, u8 left, u8 top, u16 frame, ARROW_CELL_TYPE type )
{
  cell->left      = left;
  cell->top       = top;
  cell->bootFrame = frame;
  cell->bootFlag  = FALSE;
  cell->type      = type;
}




//=====================================================================================
// ■初期化・生成・破棄・準備
//=====================================================================================

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印を生成する
 *
 * @param heapID 使用するヒープID
 */
//-------------------------------------------------------------------------------------
static ARROW* CreateArrow( HEAPID heapID )
{
  ARROW* arrow; 

  // 矢印を生成
  arrow = GFL_HEAP_AllocMemory( heapID, sizeof(ARROW) ); 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: create arrow\n" );
  return arrow;
}

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印を初期化する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void InitArrow( ARROW* arrow, HEAPID heapID )
{
  int i;

  // 本体の初期化
  arrow->heapID         = heapID;
  arrow->state          = ARROW_STATE_WAIT;
  arrow->stateCount     = 0;
  arrow->cellNum        = 0;
  arrow->clactUnit      = NULL;

  // セルの初期化
  for( i=0; i < MAX_CELL_NUM; i++ )
  {
    arrow->cell[i].bootFlag  = FALSE;
    arrow->cell[i].bootFrame = 0;
    arrow->cell[i].left      = 0;
    arrow->cell[i].top       = 0;
    arrow->cell[i].clactWork = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: init arrow\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief 矢印を破棄する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void DeleteArrow( ARROW* arrow )
{
  GF_ASSERT( arrow ); // 多重破棄

  // 矢印を破棄
  GFL_HEAP_FreeMemory( arrow );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: delete arrow\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを生成する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void CreateClactUnit( ARROW* arrow )
{
  GF_ASSERT( arrow->clactUnit == NULL ); // 多重生成

  arrow->clactUnit = GFL_CLACT_UNIT_Create( MAX_CELL_NUM, 0, arrow->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: create clact unit\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void DeleteClactUnit( ARROW* arrow )
{
  GF_ASSERT( arrow->clactUnit );

  GFL_CLACT_UNIT_Delete( arrow->clactUnit );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: delete clact unit\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを生成する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void CreateClactWorks( ARROW* arrow )
{
  int idx;
  const ARROW_DISP_PARAM* dispParam;
  GFL_CLWK_DATA data;

  dispParam = &( arrow->dispParam );
  data.pos_x   = 0;
  data.pos_y   = 0;
  data.anmseq  = 0;
  data.softpri = 0;
  data.bgpri   = 0;

  // セルアクターユニットが生成されていない
  GF_ASSERT( arrow->clactUnit );

  // セルアクターワークを生成
  for( idx=0; idx < MAX_CELL_NUM; idx++ )
  {
    arrow->cell[ idx ].clactWork = 
      GFL_CLACT_WK_Create( arrow->clactUnit, 
                           dispParam->cgrIndex, dispParam->plttIndex, 
                           dispParam->cellAnimIndex, &data, 
                           dispParam->setSerface, arrow->heapID ); 
  }
  
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: create clact works\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを生成する
 *
 * @param arrow
 */
//-------------------------------------------------------------------------------------
static void DeleteClactWorks( ARROW* arrow )
{
  int idx;

  // セルアクターワークを破棄
  for( idx=0; idx < MAX_CELL_NUM; idx++ )
  {
    GFL_CLACT_WK_Remove( arrow->cell[ idx ].clactWork );
  }
  
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: delete clact works\n" );
}

//-------------------------------------------------------------------------------------
/**
 * @param 表示パラメータを設定する
 *
 * @param arrow
 * @param param 表示パラメータ
 */
//-------------------------------------------------------------------------------------
static void SetDispParams( ARROW* arrow, const ARROW_DISP_PARAM* param )
{ 
  arrow->dispParam = *param;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "ARROW: set disp params\n" );
}

//-------------------------------------------------------------------------------------
// ■パラメータを出力する
//-------------------------------------------------------------------------------------
static void DebugPrint( const ARROW* arrow )
{
  int i;

  for( i=0; i<arrow->cellNum; i++ )
  {
    OS_TFPrintf( PRINT_TARGET, "[%d]: x=%3d, y=%3d, f=%3d\n", 
        i, arrow->cell[i].left, arrow->cell[i].top, arrow->cell[i].bootFrame );
  }
}
