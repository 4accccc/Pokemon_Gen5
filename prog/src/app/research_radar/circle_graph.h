///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  円グラフ
 * @file   circle_graph.h
 * @author obata
 * @date   2010.2.21
 */
/////////////////////////////////////////////////////////////////////////////////////////// 
#pragma once 
#include <gflib.h>


//=========================================================================================
// □円グラフの不完全形
//=========================================================================================
typedef struct _CIRCLE_GRAPH CIRCLE_GRAPH;


//=========================================================================================
// □円グラフの構成要素追加データ
//=========================================================================================
typedef struct
{
  u8 value;        // 値
  u8 outerColorR;  // 外周の色(R) [0, 31]
  u8 outerColorG;  // 外周の色(G) [0, 31]
  u8 outerColorB;  // 外周の色(B) [0, 31]
  u8 centerColorR; // 中心点の色(R) [0, 31]
  u8 centerColorG; // 中心点の色(G) [0, 31]
  u8 centerColorB; // 中心点の色(B) [0, 31]

} GRAPH_COMPONENT_ADD_DATA;


//=========================================================================================
// □生成・破棄
//=========================================================================================
extern CIRCLE_GRAPH* CIRCLE_GRAPH_Create( HEAPID heapID ); // 生成
extern void CIRCLE_GRAPH_Delete( CIRCLE_GRAPH* graph );    // 破棄

extern void CIRCLE_GRAPH_AddComponent( 
    CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* data ); // 構成要素追加

//=========================================================================================
// □制御
//=========================================================================================
extern void CIRCLE_GRAPH_Main( CIRCLE_GRAPH* graph ); // メイン動作

extern void CIRCLE_GRAPH_AnalyzeReq( CIRCLE_GRAPH* graph ); // 解析リクエスト
extern void CIRCLE_GRAPH_ShowReq( CIRCLE_GRAPH* graph );    // 表示リクエスト
extern void CIRCLE_GRAPH_HideReq( CIRCLE_GRAPH* graph );    // 消去リクエスト
extern void CIRCLE_GRAPH_UpdateReq( CIRCLE_GRAPH* graph );  // 更新リクエスト
