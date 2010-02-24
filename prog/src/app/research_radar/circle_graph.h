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
  u8 ID;           // ID
  u8 value;        // 値
  u8 outerColorR;  // 外周の色(R) [0, 31]
  u8 outerColorG;  // 外周の色(G) [0, 31]
  u8 outerColorB;  // 外周の色(B) [0, 31]
  u8 centerColorR; // 中心点の色(R) [0, 31]
  u8 centerColorG; // 中心点の色(G) [0, 31]
  u8 centerColorB; // 中心点の色(B) [0, 31]

} GRAPH_COMPONENT_ADD_DATA;


//=========================================================================================
// □グラフの生成・破棄
//=========================================================================================
extern CIRCLE_GRAPH* CIRCLE_GRAPH_Create( HEAPID heapID ); // インスタンスを生成する
extern void CIRCLE_GRAPH_Delete( CIRCLE_GRAPH* graph );    // インスタンスを破棄する


//=========================================================================================
// □構成要素の設定
//=========================================================================================
extern void CIRCLE_GRAPH_SetupComponents(
    CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* data, u8 dataNum ); // 構成要素をセットする

extern void CIRCLE_GRAPH_AddComponent( 
    CIRCLE_GRAPH* graph, const GRAPH_COMPONENT_ADD_DATA* data ); // 構成要素を追加する


//=========================================================================================
// □制御
//=========================================================================================
extern void CIRCLE_GRAPH_Main( CIRCLE_GRAPH* graph ); // メイン動作

extern void CIRCLE_GRAPH_StopGraph( CIRCLE_GRAPH* graph, u32 frames ); // 動作を停止させる

extern void CIRCLE_GRAPH_AnalyzeReq( CIRCLE_GRAPH* graph );   // 解析リクエスト
extern void CIRCLE_GRAPH_AppearReq( CIRCLE_GRAPH* graph );    // 表示リクエスト
extern void CIRCLE_GRAPH_DisappearReq( CIRCLE_GRAPH* graph ); // 消去リクエスト
extern void CIRCLE_GRAPH_UpdateReq( CIRCLE_GRAPH* graph );    // 更新リクエスト

extern void CIRCLE_GRAPH_SetDrawEnable( CIRCLE_GRAPH* graph, BOOL enable ); // 表示・非表示を変更する
extern void CIRCLE_GRAPH_SetCenterPos( CIRCLE_GRAPH* graph, const VecFx16* pos ); // 中心点の座標を設定する 

extern u32 CIRCLE_GRAPH_GetComponentValue( const CIRCLE_GRAPH* graph, u8 componentID ); // 構成要素の値を取得する
extern u8 CIRCLE_GRAPH_GetComponentRank( const CIRCLE_GRAPH* graph, u8 componentID ); // 構成要素が上位何位なのかを取得する
extern u8 CIRCLE_GRAPH_GetComponentPercentage( const CIRCLE_GRAPH* graph, u8 componentID ); // 構成要素が占める, グラフ内の割合を取得する
