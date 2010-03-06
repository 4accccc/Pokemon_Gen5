//////////////////////////////////////////////////////////////////////////////
/**
 * @brief  円グラフ
 * @file   circle_graph.c
 * @author obata
 * @date   2010.2.21
 */
///////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <gflib.h>
#include "circle_graph.h"
#include "queue.h"


//==============================================================================
// ■定数
//==============================================================================
#define PI                   (3.1415926535f) // 円周率
#define PRINT_TARGET         (2)   // デバッグ情報の出力先
#define STATE_QUEUE_SIZE     (10)  // 状態キューのサイズ
#define CIRCLE_DIV_COUNT     (100) // 円グラフの分割数 ( 外周頂点数 )
#define CIRCLE_POINT_COUNT   (CIRCLE_DIV_COUNT) // 円の外周頂点数
#define CIRCLE_POLYGON_COUNT (CIRCLE_DIV_COUNT) // 円を構成する三角形ポリゴンの数
#define CIRCLE_VERTEX_COUNT  (CIRCLE_POINT_COUNT * 3) // 円の描画に使用する頂点の数
#define DIV_PERCENTAGE       (100.0f / CIRCLE_DIV_COUNT) // ポリゴンあたりが占める割合
#define MAX_COMPONENT_NUM    (20)  // 円グラフの最大構成要素数
#define CIRCLE_CENTER_X      (FX16_CONST(-0.707f)) // 円グラフ中心点の x 座標
#define CIRCLE_CENTER_Y      (FX16_CONST(-0.041f)) // 円グラフ中心点の y 座標
#define CIRCLE_RADIUS        (FX32_CONST(0.491f)) // 円グラフの半径
#define COMPONENT_POINT_RADIUS (0.42f) // 構成要素を指し示す場所を決定する円の半径
#define Z_STRIDE             (FX16_CONST(0.05f)) // 構成要素ごとの z 値の間隔
#define BOARDER_Z            (FX16_CONST(4.00f)) // 境界線の z 値
#define BOARDER_COLOR_R      (0) // 境界線の色(R)[0, 31]
#define BOARDER_COLOR_B      (0) // 境界線の色(G)[0, 31]
#define BOARDER_COLOR_G      (0) // 境界線の色(B)[0, 31]

#define ANALYZE_FRAMES   (120) // 解析状態の動作フレーム数
#define APPEAR_FRAMES    (30)  // 出現状態の動作フレーム数
#define DISAPPEAR_FRAMES (20)  // 消去状態の動作フレーム数
#define UPDATE_FRAMES    (60)  // 消去状態の動作フレーム数

// 円グラフの状態
typedef enum {
  GRAPH_STATE_HIDE,      // 非表示
  GRAPH_STATE_ANALYZE,   // 解析
  GRAPH_STATE_APPEAR,    // 出現
  GRAPH_STATE_DISAPPEAR, // 消去
  GRAPH_STATE_STAY,      // 表示
  GRAPH_STATE_UPDATE,    // 更新
} GRAPH_STATE;


//==============================================================================
// ■描画用頂点データ
//==============================================================================
typedef struct {
  VecFx16 pos;       // 座標
  GXRgb   color;     // 頂点カラー
  u8      polygonID; // ポリゴンID 
} VERTEX;


//==============================================================================
// ■グラフの構成要素単位のデータ
//==============================================================================
typedef struct {
  u8  ID;              // 構成要素ID
  u32 value;           // 値
  u8  percentage;      // 全構成要素中の割合[%]
  u8  startPercentage; // 割合が占める範囲 ( 先頭位置[%] )
  u8  endPercentage;   // 割合が占める範囲 ( 末尾位置[%] )
  u8  outerColorR;     // 外周の色(R) [0, 31]
  u8  outerColorG;     // 外周の色(G) [0, 31]
  u8  outerColorB;     // 外周の色(B) [0, 31]
  u8  centerColorR;    // 中心点の色(R) [0, 31]
  u8  centerColorG;    // 中心点の色(G) [0, 31]
  u8  centerColorB;    // 中心点の色(B) [0, 31] 
} GRAPH_COMPONENT;


//==============================================================================
// ■円グラフ
//==============================================================================
struct _CIRCLE_GRAPH
{
  HEAPID heapID;

  BOOL        drawFlag;   // 描画許可フラグ
  GRAPH_STATE state;      // 状態
  QUEUE*      stateQueue; // 状態キュー
  u32         stateCount; // 状態カウンタ
  BOOL        stopFlag;   // 停止フラグ
  u32         stopCount;  // 停止カウンタ

  GRAPH_COMPONENT components[ MAX_COMPONENT_NUM ]; // グラフの構成要素
  u8              componentNum;                    // 有効な構成要素の数

  fx32    radius;    // 円の半径
  VecFx16 centerPos; // 中心点の座標
  VecFx16 circlePoints[ CIRCLE_POINT_COUNT ]; // 外周頂点の座標 ( 中心点からのオフセット )

  VERTEX vertices[ CIRCLE_VERTEX_COUNT ]; // 描画に使用する頂点リスト ( 三角形リスト ) 
};


//==============================================================================
// ◆関数インデックス
//==============================================================================

//------------------------------------------------------------------------------
// ◇描画
//------------------------------------------------------------------------------
static void DrawGraph( const CIRCLE_GRAPH* graph ); // 円グラフを描画する
static void DrawBoarder( const CIRCLE_GRAPH* graph ); // 境界線を描画する
static void SetMatrix( const CIRCLE_GRAPH* graph ); // 行列を設定する
//------------------------------------------------------------------------------
// ◇動作
//------------------------------------------------------------------------------
static void GraphMain( CIRCLE_GRAPH* graph ); // メイン関数
// 状態開始処理
static void GraphStart_HIDE     ( CIRCLE_GRAPH* graph ); // 状態開始処理 ( GRAPH_STATE_HIDE )
static void GraphStart_ANALYZE  ( CIRCLE_GRAPH* graph ); // 状態開始処理 ( GRAPH_STATE_ANALYZE )
static void GraphStart_APPEAR   ( CIRCLE_GRAPH* graph ); // 状態開始処理 ( GRAPH_STATE_APPEAR )
static void GraphStart_DISAPPEAR( CIRCLE_GRAPH* graph ); // 状態開始処理 ( GRAPH_STATE_DISAPPEAR )
static void GraphStart_STAY     ( CIRCLE_GRAPH* graph ); // 状態開始処理 ( GRAPH_STATE_STAY )
static void GraphStart_UPDATE   ( CIRCLE_GRAPH* graph ); // 状態開始処理 ( GRAPH_STATE_UPDATE )
// 状態ごとの処理
static void GraphAct_HIDE     ( CIRCLE_GRAPH* graph ); // 状態メイン動作 ( GRAPH_STATE_HIDE )
static void GraphAct_ANALYZE  ( CIRCLE_GRAPH* graph ); // 状態メイン動作 ( GRAPH_STATE_ANALYZE )
static void GraphAct_APPEAR   ( CIRCLE_GRAPH* graph ); // 状態メイン動作 ( GRAPH_STATE_APPEAR )
static void GraphAct_DISAPPEAR( CIRCLE_GRAPH* graph ); // 状態メイン動作 ( GRAPH_STATE_DISAPPEAR )
static void GraphAct_STAY     ( CIRCLE_GRAPH* graph ); // 状態メイン動作 ( GRAPH_STATE_STAY )
static void GraphAct_UPDATE   ( CIRCLE_GRAPH* graph ); // 状態メイン動作 ( GRAPH_STATE_UPDATE )
// 状終了処理
static void GraphFinish_HIDE     ( CIRCLE_GRAPH* graph ); // 状態終了処理 ( GRAPH_STATE_HIDE )
static void GraphFinish_ANALYZE  ( CIRCLE_GRAPH* graph ); // 状態終了処理 ( GRAPH_STATE_ANALYZE )
static void GraphFinish_APPEAR   ( CIRCLE_GRAPH* graph ); // 状態終了処理 ( GRAPH_STATE_APPEAR )
static void GraphFinish_DISAPPEAR( CIRCLE_GRAPH* graph ); // 状態終了処理 ( GRAPH_STATE_DISAPPEAR )
static void GraphFinish_STAY     ( CIRCLE_GRAPH* graph ); // 状態終了処理 ( GRAPH_STATE_STAY )
static void GraphFinish_UPDATE   ( CIRCLE_GRAPH* graph ); // 状態終了処理 ( GRAPH_STATE_UPDATE )
// 状態
static void SetNextState( CIRCLE_GRAPH* graph, GRAPH_STATE nextState ); // 次の状態をキューに登録する
static void SwitchState( CIRCLE_GRAPH* graph ); // 状態を切り替える
static void ChangeState( CIRCLE_GRAPH* graph, GRAPH_STATE nextState ); // 状態を変更する
// カウンタ
static void CountUpStateCount ( CIRCLE_GRAPH* graph ); // 状態カウンタを更新する
static void CountDownStopCount( CIRCLE_GRAPH* graph ); // 停止カウンタを更新する
//------------------------------------------------------------------------------
// ◇個別操作
//------------------------------------------------------------------------------
// 構成要素
static void ResetComponents( CIRCLE_GRAPH* graph ); // 構成要素をリセットする
static void AddComponent( CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* newComponent ); // 構成要素を追加する
static void LowerSortComponents( CIRCLE_GRAPH* graph ); // 構成要素を降順ソートする
static void UpdateComponentsPercentage( CIRCLE_GRAPH* graph ); // 各構成要素の割合を更新する
static void UpdateComponentsScope( CIRCLE_GRAPH* graph ); // 各構成要素が占める割合の範囲を更新する
// 描画用頂点リスト
static void UpdateDrawVertices( CIRCLE_GRAPH* graph ); // 描画に使用する頂点リストを更新する
// 描画許可フラグ
static void SetDrawEnable( CIRCLE_GRAPH* graph, BOOL enable ); // 描画の許可状態を設定する
// 動作停止フラグ
static void StopGraph( CIRCLE_GRAPH* graph, u32 frames ); // 動作を停止させる
// 座標
static void SetCenterPos( CIRCLE_GRAPH* graph, const VecFx16* pos ); // 中心点の座標を設定する
//------------------------------------------------------------------------------
// ◇取得・判定
//------------------------------------------------------------------------------
const u8 GetComponentRank( const CIRCLE_GRAPH* graph, u8 componentID ); // 構成要素のランク
const GRAPH_COMPONENT* GetComponentByID( const CIRCLE_GRAPH* graph, u8 componentID ); // 構成要素
const GRAPH_COMPONENT* GetComponentByRank( const CIRCLE_GRAPH* graph, int compoentRank ); // 構成要素
static u32 GetComponentsTotalValue( const CIRCLE_GRAPH* graph ); // 全構成要素の合計値
static u32 GetComponentsTotalPercentage( const CIRCLE_GRAPH* graph ); // 全構成要素が占める割合の合計値
static void GetComponentCirclePointIndex( const GRAPH_COMPONENT* component, u8* destHeadIdx, u8* destTailIdx ); // 構成要素に該当する外周頂点のインデックス
static GXRgb GetComponentOuterColor( const GRAPH_COMPONENT* component ); // 構成要素の外周の色
static GXRgb GetComponentCenterColor( const GRAPH_COMPONENT* component ); // 構成要素の中心の色
static BOOL GetDrawEnable( const CIRCLE_GRAPH* graph ); // 描画が許可されているかどうか
static BOOL CheckAnime( const CIRCLE_GRAPH* graph ); // アニメーション中かどうか
static void GetComponentPointPos( const CIRCLE_GRAPH* graph, const GRAPH_COMPONENT* component, VecFx16* dest ); // 矢印の指すべき座標

//------------------------------------------------------------------------------
// ◇初期化・生成・破棄・準備
//------------------------------------------------------------------------------
// layer 1
static void SetupGraph( CIRCLE_GRAPH* graph );   // 円グラフ セットアップ
static void CleanUpGraph( CIRCLE_GRAPH* graph ); // 円グラフ クリーンアップ
// layer 0
static void InitGraph( CIRCLE_GRAPH* graph, HEAPID heapID ); // 円グラフ 初期化
static CIRCLE_GRAPH* CreateGraph( HEAPID heapID );           // 円グラフ 生成
static void DeleteGraph( CIRCLE_GRAPH* graph );              // 円グラフ 破棄
static void CreateStateQueue( CIRCLE_GRAPH* graph ); // 状態キュー 生成
static void DeleteStateQueue( CIRCLE_GRAPH* graph ); // 状態キュー 破棄
static void SetupCirclePoints( CIRCLE_GRAPH* graph ); // 外周頂点の座標 準備

//------------------------------------------------------------------------------
// 計算
//------------------------------------------------------------------------------
static void CalcScreenPos( const VecFx16* pos, int* destX, int* destY ); // スクリーン座標を計算する

//------------------------------------------------------------------------------
// デバッグ
//------------------------------------------------------------------------------
static void DebugPrint_stateQueue( const CIRCLE_GRAPH* graph ); // 状態キュー
static void DebugPrint_components( const CIRCLE_GRAPH* graph ); // 構成要素





//===============================================================================
// ■外部公開関数
//===============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを作成する
 *
 * @param heapID 使用するヒープID
 *
 * @return 作成した円グラフ
 */
//------------------------------------------------------------------------------
CIRCLE_GRAPH* CIRCLE_GRAPH_Create( HEAPID heapID )
{
  CIRCLE_GRAPH* graph;

  graph = CreateGraph( heapID ); // 生成
  SetupGraph( graph );           // セットアップ

  return graph;
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを破棄する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_Delete( CIRCLE_GRAPH* graph )
{
  CleanUpGraph( graph ); // クリーンアップ
  DeleteGraph( graph );  // 破棄
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素をセットする
 *
 * @param graph
 * @param data    構成要素のデータ配列
 * @param dataNum 構成要素のデータ数
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_SetupComponents(
    CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* data, u8 componentNum )
{
  int idx;

  // 構成要素をリセット
  ResetComponents( graph );

  // 構成要素を追加
  for( idx=0; idx < componentNum; idx++ )
  {
    AddComponent( graph, &(data[ idx ]) );
  }

  // セットアップ処理
  LowerSortComponents( graph );        // ソート
  UpdateComponentsPercentage( graph ); // 割合を更新
  UpdateComponentsScope( graph );      // 割合の範囲を更新
  UpdateDrawVertices( graph );         // 描画に使用する頂点

  // DEBUG:
  DebugPrint_components( graph );
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素を追加する
 *
 * @param graph
 * @param data  追加する構成要素のデータ
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_AddComponent( CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* data )
{
  AddComponent( graph, data );         // 追加
  LowerSortComponents( graph );        // ソート
  UpdateComponentsPercentage( graph ); // 割合を更新
  UpdateComponentsScope( graph );      // 割合の範囲を更新
  UpdateDrawVertices( graph );         // 描画に使用する頂点

  // DEBUG:
  DebugPrint_components( graph );
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフのメイン動作
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_Main( CIRCLE_GRAPH* graph )
{
  GraphMain( graph ); 
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを描画する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_Draw( const CIRCLE_GRAPH* graph )
{
  if( (GetDrawEnable(graph) == TRUE) && (graph->state != GRAPH_STATE_HIDE) )
  {
    SetMatrix( graph );   // 行列を設定する
    DrawGraph( graph );   // 円グラフを描画する
    DrawBoarder( graph ); // 境界線を描画する
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 解析リクエストを発行する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_AnalyzeReq( CIRCLE_GRAPH* graph )
{
  SetNextState( graph, GRAPH_STATE_ANALYZE );
}

//------------------------------------------------------------------------------
/**
 * @brief 表示リクエストを発行する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_AppearReq( CIRCLE_GRAPH* graph )
{
  SetNextState( graph, GRAPH_STATE_APPEAR );
}

//------------------------------------------------------------------------------
/**
 * @brief 消去リクエストを発行する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_DisappearReq( CIRCLE_GRAPH* graph )
{
  SetNextState( graph, GRAPH_STATE_DISAPPEAR );
}

//------------------------------------------------------------------------------
/**
 * @brief 更新リクエストを発行する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_UpdateReq( CIRCLE_GRAPH* graph )
{
  SetNextState( graph, GRAPH_STATE_UPDATE );
}


//------------------------------------------------------------------------------
/**
 * @brief 描画の許可状態を設定する
 *
 * @param graph
 * @param enable 描画を許可するなら TRUE
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_SetDrawEnable( CIRCLE_GRAPH* graph, BOOL enable )
{
  SetDrawEnable( graph, enable );
}

//------------------------------------------------------------------------------
/**
 * @brief グラフの動作を停止させる
 *
 * @param graph
 * @param frames 停止フレーム数
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_StopGraph( CIRCLE_GRAPH* graph, u32 frames )
{
  StopGraph( graph, frames );
}

//------------------------------------------------------------------------------
/**
 * @brief グラフの中心点の座標を設定する
 *
 * @param graph
 * @param pos    中心点の座標
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_SetCenterPos( CIRCLE_GRAPH* graph, const VecFx16* pos )
{
  SetCenterPos( graph, pos );  // 中心点を設定
  UpdateDrawVertices( graph ); // 描画に使用する頂点座標を更新
} 

//------------------------------------------------------------------------------
/**
 * @brief グラフの中心点の座標を設定する
 *
 * @param graph
 * @param z    中心点のz座標
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_SetCenterZ( CIRCLE_GRAPH* graph, fx16 z )
{
  VecFx16 pos;

  // z座標のみを変更した中心点の位置ベクトルを作成
  VEC_Fx16Set( &pos, graph->centerPos.x, graph->centerPos.y, z );

  // 中心点を設定
  CIRCLE_GRAPH_SetCenterPos( graph, &pos );
} 

//------------------------------------------------------------------------------
/**
 * @brief 構成要素の数を取得する  
 *
 * @param graph
 *
 * @return 指定した円グラフの構成要素の数
 */
//------------------------------------------------------------------------------
u8 CIRCLE_GRAPH_GetComponentNum( const CIRCLE_GRAPH* graph )
{
  return graph->componentNum;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素の値を取得する ( 構成要素IDを指定 )
 *
 * @param graph 
 * @param componentID 構成要素ID
 *
 * @return 指定した構成要素の値
 */
//------------------------------------------------------------------------------
u32 CIRCLE_GRAPH_GetComponentValue_byID( const CIRCLE_GRAPH* graph, u8 componentID )
{
  const GRAPH_COMPONENT* component;

  component = GetComponentByID( graph, componentID );

  return component->value;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素の値を取得する ( ランクを指定 )
 *
 * @param graph 
 * @param rank 構成要素のランク
 *
 * @return 指定した構成要素の値
 */
//------------------------------------------------------------------------------
u32 CIRCLE_GRAPH_GetComponentValue_byRank( const CIRCLE_GRAPH* graph, u8 rank )
{
  const GRAPH_COMPONENT* component;

  component = GetComponentByRank( graph, rank );

  return component->value;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素が上位何位なのかを取得する ( 構成要素IDを指定 )
 *
 * @param graph
 * @param componentID 構成要素ID
 *
 * @return 指定した構成要素が上位何位なのか
 */
//------------------------------------------------------------------------------
u8 CIRCLE_GRAPH_GetComponentRank_byID( const CIRCLE_GRAPH* graph, u8 componentID )
{
  return GetComponentRank( graph, componentID );
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素が上位何位なのかを取得する ( ランクを指定 )
 *
 * @param graph
 * @param rank 構成要素のランク
 *
 * @return 指定したランクの構成要素のID
 */
//------------------------------------------------------------------------------
u8 CIRCLE_GRAPH_GetComponentID_byRank( const CIRCLE_GRAPH* graph, u8 rank )
{
  const GRAPH_COMPONENT* component;

  component = GetComponentByRank( graph, rank );

  return component->ID;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素が占める, グラフ内の割合を取得する ( 構成要素IDを指定 )
 *
 * @param graph 
 * @param componentID 構成要素ID
 *
 * @return 構成要素が占める, グラフ内の割合
 */
//------------------------------------------------------------------------------
u8 CIRCLE_GRAPH_GetComponentPercentage_byID( const CIRCLE_GRAPH* graph, u8 componentID )
{
  const GRAPH_COMPONENT* component;

  component = GetComponentByID( graph, componentID );

  return component->percentage;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素が占める, グラフ内の割合を取得する ( ランクを指定 )
 *
 * @param graph 
 * @param rank 構成要素のランク
 *
 * @return 構成要素が占める, グラフ内の割合
 */
//------------------------------------------------------------------------------
u8 CIRCLE_GRAPH_GetComponentPercentage_byRank( const CIRCLE_GRAPH* graph, u8 rank )
{
  const GRAPH_COMPONENT* component;

  component = GetComponentByRank( graph, rank );

  return component->percentage;
}

//------------------------------------------------------------------------------
/**
 * @breif アニメーション中かどうかを判定する
 *
 * @param graph
 *
 * @return アニメーション中なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------
BOOL CIRCLE_GRAPH_IsAnime( const CIRCLE_GRAPH* graph )
{
  return CheckAnime( graph );
}

//------------------------------------------------------------------------------
/**
 * @brief 矢印が指し示すべき座標を取得する ( 構成要素IDを指定 )
 *
 * @param graph
 * @param componentID 構成要素ID
 * @param destX       スクリーンx座標の格納先
 * @param destY       スクリーンy座標の格納先
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_GetComponentPointPos_byID( const CIRCLE_GRAPH* graph, u8 componentID, int* destX, int* destY )
{
  const GRAPH_COMPONENT* component;
  VecFx16 worldPos;

  component = GetComponentByID( graph, componentID );
  GetComponentPointPos( graph, component, &worldPos );
  CalcScreenPos( &worldPos, destX, destY );
}

//------------------------------------------------------------------------------
/**
 * @brief 矢印が指し示すべき座標を取得する ( ランクを指定 )
 *
 * @param graph
 * @param rank  構成要素のランク
 * @param destX スクリーンx座標の格納先
 * @param destY スクリーンy座標の格納先
 */
//------------------------------------------------------------------------------
void CIRCLE_GRAPH_GetComponentPointPos_byRank( const CIRCLE_GRAPH* graph, u8 rank, int* destX, int* destY )
{
  const GRAPH_COMPONENT* component;
  VecFx16 worldPos;

  component = GetComponentByRank( graph, rank );
  GetComponentPointPos( graph, component, &worldPos );
  CalcScreenPos( &worldPos, destX, destY );
}


//==============================================================================
// 描画
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを描画する
 *
 * @param
 */
//------------------------------------------------------------------------------
static void DrawGraph( const CIRCLE_GRAPH* graph )
{
  int drawPolygonNum;
  int drawPolygonCount;
  int firstPolygonIdx;
  int vertexIdx;
  int polygonID;

  // 描画ポリゴン範囲を決定
  switch( graph->state )
  {
  case GRAPH_STATE_HIDE:
    drawPolygonNum = 0;  // 描画しない
    break;
  case GRAPH_STATE_ANALYZE: 
    drawPolygonNum  = CIRCLE_POLYGON_COUNT * graph->stateCount / ANALYZE_FRAMES;  // ポリゴン数
    firstPolygonIdx = CIRCLE_POLYGON_COUNT - drawPolygonNum;                      // 先頭ポリゴンインデックス
    break;
  case GRAPH_STATE_APPEAR:    
    drawPolygonNum  = CIRCLE_POLYGON_COUNT * graph->stateCount / APPEAR_FRAMES;    // ポリゴン数
    firstPolygonIdx = CIRCLE_POLYGON_COUNT - drawPolygonNum;                     // 先頭ポリゴンインデックス
    break;
  case GRAPH_STATE_DISAPPEAR:    
    drawPolygonNum  = CIRCLE_POLYGON_COUNT * (DISAPPEAR_FRAMES - graph->stateCount) / DISAPPEAR_FRAMES; // ポリゴン数
    firstPolygonIdx = CIRCLE_POLYGON_COUNT - drawPolygonNum;                                  // 先頭ポリゴンインデックス
    break;
  case GRAPH_STATE_STAY:    
    drawPolygonNum  = CIRCLE_POLYGON_COUNT;                      // ポリゴン数
    firstPolygonIdx = CIRCLE_POLYGON_COUNT - drawPolygonNum;     // 先頭ポリゴンインデックス
    break;
  case GRAPH_STATE_UPDATE:  
    drawPolygonNum  = CIRCLE_POLYGON_COUNT * graph->stateCount / UPDATE_FRAMES;  // ポリゴン数
    firstPolygonIdx = CIRCLE_POLYGON_COUNT - drawPolygonNum;                     // 先頭ポリゴンインデックス
    break;
  default: 
    GF_ASSERT(0);
  }

  // 先頭頂点インデックスを算出
  vertexIdx = firstPolygonIdx * 3;

  // 描画
  for( drawPolygonCount=0; drawPolygonCount < drawPolygonNum; drawPolygonCount++ )
  { 
    // ポリゴンIDは頂点1に依存
    polygonID = graph->vertices[ vertexIdx ].polygonID;

    // ポリゴン関連属性値を設定
    G3_PolygonAttr( 
        GX_LIGHTMASK_NONE,        // no lights
        GX_POLYGONMODE_MODULATE,  // modulation mode
        GX_CULL_NONE,             // cull none
        polygonID,                // polygon ID(0 - 63)
        31,                       // alpha(0 - 31)
        0                         // OR of GXPolygonAttrMisc's value
        );

    G3_Begin( GX_BEGIN_TRIANGLES );

    // 頂点1
    G3_Color( graph->vertices[ vertexIdx ].color );
    G3_Vtx( graph->vertices[ vertexIdx ].pos.x,
            graph->vertices[ vertexIdx ].pos.y,
            graph->vertices[ vertexIdx ].pos.z );
    vertexIdx = ( vertexIdx + 1 ) % CIRCLE_VERTEX_COUNT;

    // 頂点2
    G3_Color( graph->vertices[ vertexIdx ].color );
    G3_Vtx( graph->vertices[ vertexIdx ].pos.x,
            graph->vertices[ vertexIdx ].pos.y,
            graph->vertices[ vertexIdx ].pos.z );
    vertexIdx = ( vertexIdx + 1 ) % CIRCLE_VERTEX_COUNT;

    // 頂点3
    G3_Color( graph->vertices[ vertexIdx ].color );
    G3_Vtx( graph->vertices[ vertexIdx ].pos.x,
            graph->vertices[ vertexIdx ].pos.y,
            graph->vertices[ vertexIdx ].pos.z );
    vertexIdx = ( vertexIdx + 1 ) % CIRCLE_VERTEX_COUNT;

    G3_End();
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 境界線を描画する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void DrawBoarder( const CIRCLE_GRAPH* graph )
{
  int componentIdx;
  int componentNum; // 構成要素の数
  GXRgb boarderColor; // 境界線の色

  // アニメーション中は描画しない
  if( CheckAnime(graph) ) { return; }

  // ポリゴン関連属性値を設定
  G3_PolygonAttr( 
      GX_LIGHTMASK_NONE,        // no lights
      GX_POLYGONMODE_MODULATE,  // modulation mode
      GX_CULL_NONE,             // cull none
      0,                        // polygon ID(0 - 63)
      31,                       // alpha(0 - 31)
      0                         // OR of GXPolygonAttrMisc's value
      );

  componentNum = graph->componentNum;
  boarderColor = GX_RGB( BOARDER_COLOR_R, BOARDER_COLOR_G, BOARDER_COLOR_B );

  // すべての構成要素について, 描画を行う
  for( componentIdx=0; componentIdx < componentNum; componentIdx++ )
  {
    const GRAPH_COMPONENT* component;
    u8 headPointIdx, tailPointIdx;
    VecFx16 lineStartPos, lineEndPos;

    // 構成要素のデータを取得
    component = GetComponentByRank( graph, componentIdx );
    GetComponentCirclePointIndex( component, &headPointIdx, &tailPointIdx ); // 構成要素が含む外周頂点の範囲

    // 占める割合が一定値以下の場合, 描画をスキップする
    if( component->percentage < 3 ) { continue; } 

    // ライン始点の座標を設定
    lineStartPos.x = graph->centerPos.x;
    lineStartPos.y = graph->centerPos.y;
    lineStartPos.z = BOARDER_Z;
    // ライン終点の座標を計算
    lineEndPos.x = graph->centerPos.x + graph->circlePoints[ headPointIdx ].x;
    lineEndPos.y = graph->centerPos.y + graph->circlePoints[ headPointIdx ].y;
    lineEndPos.z = BOARDER_Z;

    // 構成要素の先頭側にラインを引く
    G3_Begin( GX_BEGIN_TRIANGLES ); 
    G3_Color( boarderColor );
    G3_Vtx( lineStartPos.x, lineStartPos.y, lineStartPos.z ); // 頂点1
    G3_Vtx( lineStartPos.x, lineStartPos.y, lineStartPos.z ); // 頂点2
    G3_Vtx( lineEndPos.x, lineEndPos.y, lineEndPos.z );       // 頂点3 
    G3_End();
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 行列をセットする
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void SetMatrix( const CIRCLE_GRAPH* graph )
{
  VecFx32 camPos, camUp, target;

  VEC_Set( &camPos, 0, 0, FX32_ONE );
  VEC_Set( &camUp, 0, FX32_ONE, 0 );
  VEC_Set( &target, 0, 0, 0 );

  G3_LookAt( &camPos, &camUp, &target, NULL ); 
  G3_Ortho( FX_F32_TO_FX16(1.0f),
            FX_F32_TO_FX16(-1.0f),
            FX_F32_TO_FX16(-1.333f),
            FX_F32_TO_FX16(1.333f),
            FX_F32_TO_FX16(0.1f),
            FX_F32_TO_FX16(5.0f),
            NULL ); 

  NNS_G3dGlbInit(); 
  NNS_G3dGlbLookAt( &camPos, &camUp, &target ); 
  NNS_G3dGlbOrtho( FX32_CONST(1.0f),
                   FX32_CONST(-1.0f),
                   FX32_CONST(-1.333f),
                   FX32_CONST(1.333f),
                   FX32_CONST(0.1f),
                   FX32_CONST(5.0f) ); 
  NNS_G3dGlbFlushP();
}



//==============================================================================
// ■動作
//===============================================================================

//------------------------------------------------------------------------------
/**
 * @brief メイン関数
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphMain( CIRCLE_GRAPH* graph )
{
  if( graph->stopFlag == FALSE ) {
    // 状態ごとのメイン動作
    switch( graph->state ) {
    case GRAPH_STATE_HIDE:      GraphAct_HIDE( graph );      break;
    case GRAPH_STATE_ANALYZE:   GraphAct_ANALYZE( graph );   break;
    case GRAPH_STATE_APPEAR:    GraphAct_APPEAR( graph );    break;
    case GRAPH_STATE_DISAPPEAR: GraphAct_DISAPPEAR( graph ); break;
    case GRAPH_STATE_STAY:      GraphAct_STAY( graph );      break;
    case GRAPH_STATE_UPDATE:    GraphAct_UPDATE( graph );    break;
    default: GF_ASSERT(0);
    }
  }

  // 状態カウンタ更新
  CountUpStateCount( graph );

  // 停止カウンタ更新
  CountDownStopCount( graph );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( GRAPH_STATE_HIDE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphAct_HIDE( CIRCLE_GRAPH* graph )
{
  // キューが空でなければ状態遷移する
  if( QUEUE_IsEmpty( graph->stateQueue ) == FALSE )
  {
    SwitchState( graph );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( GRAPH_STATE_ANALYZE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphAct_ANALYZE( CIRCLE_GRAPH* graph )
{
  // 一定時間経過で状態遷移する
  if( ANALYZE_FRAMES <= graph->stateCount )
  {
    ChangeState( graph, GRAPH_STATE_STAY );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( GRAPH_STATE_APPEAR )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphAct_APPEAR( CIRCLE_GRAPH* graph )
{
  // 一定時間経過で状態遷移する
  if( APPEAR_FRAMES <= graph->stateCount )
  {
    ChangeState( graph, GRAPH_STATE_STAY );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( GRAPH_STATE_DISAPPEAR )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphAct_DISAPPEAR( CIRCLE_GRAPH* graph )
{
  // 一定時間経過で状態遷移する
  if( DISAPPEAR_FRAMES <= graph->stateCount )
  {
    ChangeState( graph, GRAPH_STATE_HIDE );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( GRAPH_STATE_STAY )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphAct_STAY( CIRCLE_GRAPH* graph )
{
  // キューが空でなければ状態遷移する
  if( QUEUE_IsEmpty( graph->stateQueue ) == FALSE )
  {
    SwitchState( graph );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 状態メイン動作 ( GRAPH_STATE_UPDATE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphAct_UPDATE( CIRCLE_GRAPH* graph )
{
  // 一定時間経過で状態遷移する
  if( UPDATE_FRAMES <= graph->stateCount )
  {
    ChangeState( graph, GRAPH_STATE_STAY );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 状態カウンタを更新する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void CountUpStateCount( CIRCLE_GRAPH* graph )
{
  u32 maxCount;

  // インクリメント
  graph->stateCount++;

  // 最大値を決定
  switch( graph->state ) {
  case GRAPH_STATE_HIDE:      maxCount = 0xffffffff;       break;
  case GRAPH_STATE_ANALYZE:   maxCount = ANALYZE_FRAMES;   break;
  case GRAPH_STATE_APPEAR:    maxCount = APPEAR_FRAMES;    break;
  case GRAPH_STATE_DISAPPEAR: maxCount = DISAPPEAR_FRAMES; break;
  case GRAPH_STATE_STAY:      maxCount = 0xffffffff;       break;
  case GRAPH_STATE_UPDATE:    maxCount = UPDATE_FRAMES;    break;
  default: GF_ASSERT(0);
  }

  // 最大値補正
  if( maxCount < graph->stateCount )
  { 
    graph->stateCount = maxCount;
  }
}


//------------------------------------------------------------------------------
/**
 * @brief 停止状態のカウントダウン処理
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void CountDownStopCount( CIRCLE_GRAPH* graph )
{
  // 停止していない
  if( graph->stopFlag == FALSE ) { return; }

  if( 0 < graph->stopCount ) {
    // カウントダウン
    graph->stopCount--; 

    // カウントダウン終了
    if( graph->stopCount == 0 ) { graph->stopFlag = FALSE; }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 次の状態をキューに登録する
 *
 * @param graph
 * @param nextState キューに登録する状態
 */
//------------------------------------------------------------------------------
static void SetNextState( CIRCLE_GRAPH* graph, GRAPH_STATE nextState )
{
  // エンキュー
  QUEUE_EnQueue( graph->stateQueue, nextState );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: set next state\n" );
  DebugPrint_stateQueue( graph );  // キューの状態を出力
}

//------------------------------------------------------------------------------
/**
 * @brief 状態を切り替える
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void SwitchState( CIRCLE_GRAPH* graph )
{
  GRAPH_STATE nextState;

  // 状態を更新
  nextState = QUEUE_DeQueue( graph->stateQueue );
  ChangeState( graph, nextState );

  // DEBUG:
  DebugPrint_stateQueue( graph );  // キューの状態を出力
}

//------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param graph
 * @param nextState
 */
//------------------------------------------------------------------------------
static void ChangeState( CIRCLE_GRAPH* graph, GRAPH_STATE nextState )
{
  // 現在の状態の終了処理
  switch( graph->state ) {
  case GRAPH_STATE_HIDE:      GraphFinish_HIDE( graph );      break;
  case GRAPH_STATE_ANALYZE:   GraphFinish_ANALYZE( graph );   break;
  case GRAPH_STATE_APPEAR:    GraphFinish_APPEAR( graph );    break;
  case GRAPH_STATE_DISAPPEAR: GraphFinish_DISAPPEAR( graph ); break;
  case GRAPH_STATE_STAY:      GraphFinish_STAY( graph );      break;
  case GRAPH_STATE_UPDATE:    GraphFinish_UPDATE( graph );    break;
  default: GF_ASSERT(0);
  }

  // 状態を更新
  graph->state = nextState;
  graph->stateCount = 0;

  // 次の状態の状態の開始処理
  switch( nextState ) {
  case GRAPH_STATE_HIDE:      GraphStart_HIDE( graph );      break;
  case GRAPH_STATE_ANALYZE:   GraphStart_ANALYZE( graph );   break;
  case GRAPH_STATE_APPEAR:    GraphStart_APPEAR( graph );    break;
  case GRAPH_STATE_DISAPPEAR: GraphStart_DISAPPEAR( graph ); break;
  case GRAPH_STATE_STAY:      GraphStart_STAY( graph );      break;
  case GRAPH_STATE_UPDATE:    GraphStart_UPDATE( graph );    break;
  default: GF_ASSERT(0);
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: change state ==> " );
  switch( nextState ) {
  case GRAPH_STATE_HIDE:      OS_TFPrintf( PRINT_TARGET, "HIDE" );      break;
  case GRAPH_STATE_ANALYZE:   OS_TFPrintf( PRINT_TARGET, "ANALYZE" );   break;
  case GRAPH_STATE_APPEAR:    OS_TFPrintf( PRINT_TARGET, "APPEAR" );    break;
  case GRAPH_STATE_DISAPPEAR: OS_TFPrintf( PRINT_TARGET, "DISAPPEAR" ); break;
  case GRAPH_STATE_STAY:      OS_TFPrintf( PRINT_TARGET, "STAY" );      break;
  case GRAPH_STATE_UPDATE:    OS_TFPrintf( PRINT_TARGET, "UPDATE" );    break;
  default: GF_ASSERT(0);
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態開始処理 ( GRAPH_STATE_HIDE )
 *
 * @param graph
 */
//-------------------------------------------------------------------------------
static void GraphStart_HIDE( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start state HIDE\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態開始処理 ( GRAPH_STATE_ANALYZE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphStart_ANALYZE( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start state ANALYZE\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態開始処理 ( GRAPH_STATE_APPEAR )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphStart_APPEAR( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start state APPEAR\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態開始処理 ( GRAPH_STATE_DISAPPEAR )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphStart_DISAPPEAR( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start state DISAPPEAR\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態開始処理 ( GRAPH_STATE_STAY )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphStart_STAY( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start state STAY\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態開始処理 ( GRAPH_STATE_UPDATE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphStart_UPDATE( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start state UPDATE\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態終了処理 ( GRAPH_STATE_HIDE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------

static void GraphFinish_HIDE( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: finish state HIDE\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態終了処理 ( GRAPH_STATE_ANALYZE )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphFinish_ANALYZE( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: finish state ANALYZE\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態終了処理 ( GRAPH_STATE_APPEAR )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphFinish_APPEAR( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: finish state APPEAR\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態終了処理 ( GRAPH_STATE_DISAPPEAR )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphFinish_DISAPPEAR( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: finish state DISAPPEAR\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態終了処理 ( GRAPH_STATE_STAY )
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphFinish_STAY( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: finish state STAY\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態終了処理 ( GRAPH_STATE_UPDATE ) 
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void GraphFinish_UPDATE( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: finish state UPDATE\n" );
} 




//==============================================================================
// ■個別操作
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 構成要素をリセットする
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void ResetComponents( CIRCLE_GRAPH* graph )
{
  graph->componentNum = 0;                // 有効な構成要素の数をリセット
  QUEUE_Clear( graph->stateQueue );       // 状態キューをクリア
  ChangeState( graph, GRAPH_STATE_HIDE ); // 状態を変更する
}

//------------------------------------------------------------------------------
/**
 * @brief グラフの構成要素を追加する
 *
 * @param graph
 * @param newComponent 追加する構成要素のデータ
 */
//------------------------------------------------------------------------------
static void AddComponent( CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* newComponent )
{
  // 構成要素数オーバーフロー
  GF_ASSERT( graph->componentNum < MAX_COMPONENT_NUM );

  // 追加
  graph->components[ graph->componentNum ].ID              = newComponent->ID;
  graph->components[ graph->componentNum ].value           = newComponent->value;
  graph->components[ graph->componentNum ].percentage      = 0;
  graph->components[ graph->componentNum ].startPercentage = 0;
  graph->components[ graph->componentNum ].endPercentage   = 0;
  graph->components[ graph->componentNum ].outerColorR     = newComponent->outerColorR;
  graph->components[ graph->componentNum ].outerColorG     = newComponent->outerColorG;
  graph->components[ graph->componentNum ].outerColorB     = newComponent->outerColorB;
  graph->components[ graph->componentNum ].centerColorR    = newComponent->centerColorR;
  graph->components[ graph->componentNum ].centerColorG    = newComponent->centerColorG;
  graph->components[ graph->componentNum ].centerColorB    = newComponent->centerColorB;
  graph->componentNum++;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: add new component(%d)\n", graph->componentNum );
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素を降順ソートする
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void LowerSortComponents( CIRCLE_GRAPH* graph )
{
  int idx;
  int sortedNum;
  int unSortedTailPos;
  int componentNum;

  componentNum = graph->componentNum;

  // バブルソート
  for( sortedNum = 0; sortedNum < componentNum-1; sortedNum++ )
  {
    // 未ソートの終端要素インデックスを計算
    unSortedTailPos = componentNum - 1 - sortedNum;

    for( idx=0; idx < unSortedTailPos; idx++ )
    {
      if( graph->components[ idx ].value < graph->components[ idx + 1 ].value )
      {
        // 入れ替える
        GRAPH_COMPONENT temp         = graph->components[ idx ];
        graph->components[ idx ]     = graph->components[ idx + 1 ];
        graph->components[ idx + 1 ] = temp;
      }
    }
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: lower sort components\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 各構成要素が占める割合を更新する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void UpdateComponentsPercentage( CIRCLE_GRAPH* graph )
{
  int totalValue;
  int idx;
  int componentNum;
  int loss;

  totalValue   = GetComponentsTotalValue( graph );
  componentNum = graph->componentNum;

  // 全ての構成要素を更新する
  for( idx=0; idx < componentNum; idx++ )
  {
    GRAPH_COMPONENT* component;

    component = &( graph->components[ idx ] );

    // 割合を計算
    component->percentage = component->value * 100 / totalValue;
  }

  // 計算誤差による不足分を求める
  loss = 100 - GetComponentsTotalPercentage( graph );
  
  // 不足分を補正する
  idx = 0;
  while( 0 < loss )
  {
    graph->components[ idx ].percentage++;
    loss--;
    idx = ( idx + 1 ) % componentNum;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: update components percentage\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 各構成要素が占める割合の範囲を更新する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void UpdateComponentsScope( CIRCLE_GRAPH* graph )
{
  int idx;
  int componentNum;
  int percentageOffset;

  componentNum = graph->componentNum;
  percentageOffset = 0;

  // 全ての構成要素を更新する
  for( idx=0; idx < componentNum; idx++ )
  {
    GRAPH_COMPONENT* component;

    component = &( graph->components[ idx ] );

    // 範囲を設定
    component->startPercentage = percentageOffset;
    component->endPercentage   = percentageOffset + component->percentage - 1;

    // オフセット加算
    percentageOffset += component->percentage;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: update components scope\n" );
} 

//------------------------------------------------------------------------------
/**
 * @brief 描画に使用する頂点リストを更新する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void UpdateDrawVertices( CIRCLE_GRAPH* graph )
{
  int vertexIdx;
  int drawPolygonNum;
  int componentNum;
  int componentIdx;

  vertexIdx = 0;
  componentNum = graph->componentNum;

  // すべての構成要素の頂点を設定する
  for( componentIdx=0; componentIdx < componentNum; componentIdx++ )
  {
    const GRAPH_COMPONENT* component;
    GXRgb centerColor, outerColor;
    u8 headPointIdx, tailPointIdx;
    int circlePointIdx;
    VecFx16 centerPos;

    // 構成要素のデータを取得
    component   = GetComponentByRank( graph, componentIdx );
    centerColor = GetComponentCenterColor( component ); // 中心の色
    outerColor  = GetComponentOuterColor( component );  // 外周の色
    GetComponentCirclePointIndex( component, &headPointIdx, &tailPointIdx ); // 構成要素が含む外周頂点の範囲
    
    // 中心点の座標を決定
    centerPos.x = graph->centerPos.x;
    centerPos.y = graph->centerPos.y;
    centerPos.z = graph->centerPos.z - Z_STRIDE * componentIdx; // 構成要素ごとに異なる z値 で描画する

    // 構成要素が占める割合の範囲内にある, すべての外周頂点が構成するポリゴンを追加する
    for( circlePointIdx=headPointIdx; circlePointIdx <= tailPointIdx; circlePointIdx++ )
    {
      // 頂点データ追加
      // 頂点1
      graph->vertices[ vertexIdx ].color     = centerColor;
      graph->vertices[ vertexIdx ].pos       = centerPos;
      graph->vertices[ vertexIdx ].polygonID = componentIdx; // ポリゴンID = 構成要素のインデックス
      vertexIdx++;

      // 頂点2
      graph->vertices[ vertexIdx ].color = outerColor;
      VEC_Fx16Add( &centerPos, 
                   &graph->circlePoints[ circlePointIdx ], 
                   &graph->vertices[ vertexIdx ].pos );
      graph->vertices[ vertexIdx ].polygonID = componentIdx; // ポリゴンID = 構成要素のインデックス
      vertexIdx++;

      // 頂点3
      graph->vertices[ vertexIdx ].color = outerColor;
      VEC_Fx16Add( &centerPos, 
                   &graph->circlePoints[ (circlePointIdx + 1) % CIRCLE_POINT_COUNT ], 
                   &graph->vertices[ vertexIdx ].pos );
      graph->vertices[ vertexIdx ].polygonID = componentIdx; // ポリゴンID = 構成要素のインデックス
      vertexIdx++;
    }
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: update draw vertices\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 描画の許可状態を設定する
 *
 * @param graph
 * @param enable 描画を許可するなら TRUE
 */
//------------------------------------------------------------------------------
static void SetDrawEnable( CIRCLE_GRAPH* graph, BOOL enable )
{
  graph->drawFlag = enable;
}

//------------------------------------------------------------------------------
/**
 * @brief グラフの動作を停止させる
 *
 * @param graph
 * @param frames 停止フレーム数
 */
//------------------------------------------------------------------------------
static void StopGraph( CIRCLE_GRAPH* graph, u32 frames )
{
  graph->stopFlag  = TRUE;
  graph->stopCount = frames;
}

//------------------------------------------------------------------------------
/**
 * @brief 中心点の座標を設定する
 *
 * @param graph
 * @param pos
 */
//------------------------------------------------------------------------------
static void SetCenterPos( CIRCLE_GRAPH* graph, const VecFx16* pos )
{
  VEC_Fx16Set( &(graph->centerPos), pos->x, pos->y, pos->z );
}


//==============================================================================
// ■取得
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 構成要素のランクを取得する
 *
 * @param graph
 * @param componentID 構成要素ID
 *
 * @return 指定したIDの構成要素が上位何位なのか [0, 要素数-1]
 */
//------------------------------------------------------------------------------
const u8 GetComponentRank( const CIRCLE_GRAPH* graph, u8 componentID )
{
  int rank;
  int componentNum;

  componentNum = graph->componentNum;

  for( rank=0; rank < componentNum; rank++ )
  {
    const GRAPH_COMPONENT* component = &( graph->components[ rank ] );

    // 発見
    if( component->ID == componentID ) { return rank; }
  }

  // 指定されたIDを持つ構成要素が存在しない
  GF_ASSERT(0);
  return 0;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素を取得する
 *
 * @param graph
 * @param rank 何番目の構成要素を取得するか
 *
 * @return 指定したインデックスの構成要素
 */
//------------------------------------------------------------------------------
const GRAPH_COMPONENT* GetComponentByRank( const CIRCLE_GRAPH* graph, int rank )
{
  // インデックス エラー
  GF_ASSERT( rank < graph->componentNum );

  return &( graph->components[ rank ] );
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素を取得する
 *
 * @param graph
 * @param componentID 構成要素ID
 *
 * @param 指定したIDを持つ構成要素
 */
//------------------------------------------------------------------------------
const GRAPH_COMPONENT* GetComponentByID( const CIRCLE_GRAPH* graph, u8 componentID )
{ 
  int rank;

  rank = GetComponentRank( graph, componentID );
  return GetComponentByRank( graph, rank );
}

//------------------------------------------------------------------------------
/**
 * @brief 全構成要素の合計値を計算する
 *
 * @param graph
 *
 * @return 全構成要素が持つデータの合計値
 */
//------------------------------------------------------------------------------
static u32 GetComponentsTotalValue( const CIRCLE_GRAPH* graph )
{
  int idx;
  int num;
  int sum;

  num = graph->componentNum;
  sum = 0;

  // 合計を計算
  for( idx=0; idx < num; idx++ )
  {
    sum += graph->components[ idx ].value;
  } 

  return sum;
}

//------------------------------------------------------------------------------
/**
 * @brief 全構成要素が占める割合の合計値を計算する
 *
 * @param graph
 *
 * @return 全構成要素が占める割合の合計値
 */
//------------------------------------------------------------------------------
static u32 GetComponentsTotalPercentage( const CIRCLE_GRAPH* graph )
{
  int idx;
  int num;
  int sum;

  num = graph->componentNum;
  sum = 0;

  // 合計を計算
  for( idx=0; idx < num; idx++ )
  {
    sum += graph->components[ idx ].percentage;
  } 

  return sum;
}

//------------------------------------------------------------------------------
/**
 * @brief 指定した構成要素に該当する, 外周頂点のインデックスを取得する
 *
 * @param component    インデックスを取得する構成要素
 * @param destHeadIdx  取得した先頭インデックスの格納先
 * @param destTailIdx  取得した末尾インデックスの格納先
 */
//------------------------------------------------------------------------------
static void GetComponentCirclePointIndex( const GRAPH_COMPONENT* component, u8* destHeadIdx, u8* destTailIdx )
{
  u8 startPercentage, endPercentage;
  int headIdx, tailIdx;

  // 先頭・末尾の頂点インデックスを計算
  headIdx = component->startPercentage / DIV_PERCENTAGE;
  tailIdx = component->endPercentage / DIV_PERCENTAGE;

  *destHeadIdx = headIdx;
  *destTailIdx = tailIdx;
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素の外周の色を取得する
 *
 * @param component 色を取得する構成要素
 *
 * @return 指定した構成要素の表示カラー
 */
//------------------------------------------------------------------------------
static GXRgb GetComponentOuterColor( const GRAPH_COMPONENT* component )
{
  return GX_RGB( component->outerColorR, component->outerColorG, component->outerColorB );
}

//------------------------------------------------------------------------------
/**
 * @brief 構成要素の中心の色を取得する
 *
 * @param component 色を取得する構成要素
 *
 * @return 指定した構成要素の表示カラー
 */
//------------------------------------------------------------------------------
static GXRgb GetComponentCenterColor( const GRAPH_COMPONENT* component )
{
  return GX_RGB( component->centerColorR, component->centerColorG, component->centerColorB );
}

//------------------------------------------------------------------------------
/**
 * @brief 指定した構成要素について, 矢印が指すべき座標を取得する
 * 
 * @param graph
 * @param component 構成要素
 * @param dest      計算した座標の格納先
 */
//------------------------------------------------------------------------------
static void GetComponentPointPos( const CIRCLE_GRAPH* graph, const GRAPH_COMPONENT* component, VecFx16* dest )
{
  u8 headIdx, tailIdx, centerIdx;
  VecFx16 outerPos, vecToOuterPos;
  float vx, vy, vz;
  float cx, cy, cz;

  // 対象となる外周点の座標を取得
  GetComponentCirclePointIndex( component, &headIdx, &tailIdx ); // 構成要素の両端の外周頂点のインデックスを取得
  centerIdx = (headIdx + tailIdx) / 2; // 外周両頂点の中間地点にある頂点インデックスを算出
  outerPos = graph->circlePoints[ centerIdx ];

  // 指し示す座標を決定
  VEC_Fx16Normalize( &outerPos, &vecToOuterPos ); // 中心点→外周点 方向ベクトル ( 単位ベクトル )
  vx = FX_FX16_TO_F32(vecToOuterPos.x) * COMPONENT_POINT_RADIUS; // 中心点から外周点方向ベクトル
  vy = FX_FX16_TO_F32(vecToOuterPos.y) * COMPONENT_POINT_RADIUS; // 
  vz = FX_FX16_TO_F32(vecToOuterPos.z) * COMPONENT_POINT_RADIUS; // 
  cx = FX_FX16_TO_F32(graph->centerPos.x); // 中心点の位置ベクトル
  cy = FX_FX16_TO_F32(graph->centerPos.y); //
  cz = FX_FX16_TO_F32(graph->centerPos.z); //
  dest->x = FX16_CONST(cx + vx);
  dest->y = FX16_CONST(cy + vy);
  dest->z = FX16_CONST(cz + vz);
}

//------------------------------------------------------------------------------
/**
 * @brief 描画が許可されているかどうか
 *
 * @param graph
 *
 * @return 描画が許可されているなら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------
static BOOL GetDrawEnable( const CIRCLE_GRAPH* graph )
{
  return graph->drawFlag;
}

//------------------------------------------------------------------------------
/**
 * @breif アニメーション中かどうか
 *
 * @param graph
 *
 * @return アニメーション中なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------
static BOOL CheckAnime( const CIRCLE_GRAPH* graph )
{
  switch( graph->state ) {
    case GRAPH_STATE_HIDE:      return FALSE;
    case GRAPH_STATE_ANALYZE:   return TRUE;
    case GRAPH_STATE_APPEAR:    return TRUE;
    case GRAPH_STATE_DISAPPEAR: return TRUE;
    case GRAPH_STATE_STAY:      return FALSE;
    case GRAPH_STATE_UPDATE:    return TRUE;
    default: GF_ASSERT(0);
  }
  return FALSE;
}



//==============================================================================
// ■初期化・生成・破棄・準備
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 円グラフをセットアップする
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void SetupGraph( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start setup graph\n" );

  // セットアップ処理
  CreateStateQueue( graph );  // 状態キューを作成
  SetupCirclePoints( graph ); // 外周オフセット座標を算出

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: end setup graph\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフをクリーンアップする
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void CleanUpGraph( CIRCLE_GRAPH* graph )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: start clean up graph\n" );

  // クリーンアップ処理
  DeleteStateQueue( graph ); // 状態キューを破棄

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: end clean up graph\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを初期化する
 *
 * @param graph
 * @param heapID 使用するヒープID
 */
//------------------------------------------------------------------------------
static void InitGraph( CIRCLE_GRAPH* graph, HEAPID heapID )
{
  graph->heapID       = heapID;
  graph->drawFlag     = FALSE;
  graph->state        = GRAPH_STATE_HIDE;
  graph->stateQueue   = NULL;
  graph->stopFlag     = FALSE;
  graph->stopCount    = 0;
  graph->radius       = CIRCLE_RADIUS;
  graph->componentNum = 0;

  VEC_Fx16Set( &(graph->centerPos), CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: init graph\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを生成する
 *
 * @param graph
 * @param heapID 使用するヒープID
 */
//------------------------------------------------------------------------------
static CIRCLE_GRAPH* CreateGraph( HEAPID heapID )
{
  CIRCLE_GRAPH* graph;

  // 生成
  graph = GFL_HEAP_AllocMemory( heapID, sizeof(CIRCLE_GRAPH) );

  // 初期化
  InitGraph( graph, heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: create graph\n" );

  return graph;
}

//------------------------------------------------------------------------------
/**
 * @brief 円グラフを破棄する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void DeleteGraph( CIRCLE_GRAPH* graph )
{
  GFL_HEAP_FreeMemory( graph );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: delete graph\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態キューを生成する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void CreateStateQueue( CIRCLE_GRAPH* graph )
{
  // 多重生成
  GF_ASSERT( graph->stateQueue == NULL );

  graph->stateQueue = QUEUE_Create( STATE_QUEUE_SIZE, graph->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: create state queueh\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 状態キューを破棄する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void DeleteStateQueue( CIRCLE_GRAPH* graph )
{
  // 未生成
  GF_ASSERT( graph->stateQueue );

  QUEUE_Delete( graph->stateQueue );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: delete state queue\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief 外周頂点の座標を準備する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void SetupCirclePoints( CIRCLE_GRAPH* graph )
{
  int pointIdx;
  float radius;

  radius = FX_FX32_TO_F32( graph->radius );

  for( pointIdx=0; pointIdx < CIRCLE_POINT_COUNT; pointIdx++ )
  {
    float radian;
    float x, y, z;

    radian = 2.0f * PI * pointIdx / (float)CIRCLE_POINT_COUNT;
    radian = 0.5f * PI - radian; // 位相を調整
    x = radius * cosf(radian);
    y = radius * sinf(radian);
    z = 0;

    VEC_Fx16Set( &(graph->circlePoints[ pointIdx ]), 
                 FX_F32_TO_FX16(x), FX_F32_TO_FX16(y), FX_F32_TO_FX16(z)  );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: setup circle points\n" );
}


//==============================================================================
// ■計算
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @breif スクリーン座標を求める
 *
 * @param worldPos 変換するワールド座標
 * @param destX    求めたスクリーンx座標の格納先
 * @param destY    求めたスクリーンy座標の格納先Y
 */
//------------------------------------------------------------------------------
static void CalcScreenPos( const VecFx16* worldPos, int* destX, int* destY )
{
  VecFx32 posFx32;
  int ret;

  // ベクトルを変換
  posFx32.x = FX32_CONST( FX_FX16_TO_F32(worldPos->x) );
  posFx32.y = FX32_CONST( FX_FX16_TO_F32(worldPos->y) );
  posFx32.z = FX32_CONST( FX_FX16_TO_F32(worldPos->z) );

  ret = NNS_G3dWorldPosToScrPos( &posFx32, destX, destY );
}


//==============================================================================
// ■デバッグ
//===============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 状態キューの内容を出力する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void DebugPrint_stateQueue( const CIRCLE_GRAPH* graph )
{
  int i;
  int data;
  int dataNum;
  const QUEUE* queue;

  queue   = graph->stateQueue;
  dataNum = QUEUE_GetDataNum( queue );

  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: state queue = " );
  for( i=0; i < dataNum; i++ )
  {
    data = QUEUE_PeekData( graph->stateQueue, i );

    switch( data ) {
    case GRAPH_STATE_HIDE:      OS_TFPrintf( PRINT_TARGET, "HIDE ");      break;
    case GRAPH_STATE_ANALYZE:   OS_TFPrintf( PRINT_TARGET, "ANALYZE ");   break;
    case GRAPH_STATE_APPEAR:    OS_TFPrintf( PRINT_TARGET, "APPEAR ");    break;
    case GRAPH_STATE_DISAPPEAR: OS_TFPrintf( PRINT_TARGET, "DISAPPEAR "); break;
    case GRAPH_STATE_STAY:      OS_TFPrintf( PRINT_TARGET, "STAY ");      break;
    case GRAPH_STATE_UPDATE:    OS_TFPrintf( PRINT_TARGET, "UPDATE ");    break;
    default:                    OS_TFPrintf( PRINT_TARGET, "UNKNOWN ");   break;
    }
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}

//------------------------------------------------------------------------------
/**
 * @brief すべての構成要素を出力する
 *
 * @param graph
 */
//------------------------------------------------------------------------------
static void DebugPrint_components( const CIRCLE_GRAPH* graph )
{
  int idx;
  int componentNum;
  int totalValue;
  int totalPercentage;

  totalValue      = 0;
  totalPercentage = 0;
  componentNum    = graph->componentNum;

  // 個別データ出力
  for( idx=0; idx < componentNum; idx++ )
  {
    const GRAPH_COMPONENT* component;

    component = GetComponentByRank( graph, idx );

    // データ出力
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].ID              = %d\n", idx, component->ID );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].value           = %d\n", idx, component->value );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].percentage      = %d\n", idx, component->percentage );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].startPercentage = %d\n", idx, component->startPercentage );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].endPercentage   = %d\n", idx, component->endPercentage );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].outerColorR     = %d\n", idx, component->outerColorR );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].outerColorG     = %d\n", idx, component->outerColorG );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].outerColorB     = %d\n", idx, component->outerColorB );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].centerColorR    = %d\n", idx, component->centerColorR );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].centerColorG    = %d\n", idx, component->centerColorG );
    OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: component[%d].centerColorB    = %d\n", idx, component->centerColorB );

    // 総合データ集計
    totalValue      += component->value;
    totalPercentage += component->percentage;
  }

  // 総合データ出力
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: components num   = %d\n", componentNum );
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: total value      = %d\n", totalValue );
  OS_TFPrintf( PRINT_TARGET, "CIRCLE-GRAPH: total percentage = %d\n", totalPercentage );
} 
