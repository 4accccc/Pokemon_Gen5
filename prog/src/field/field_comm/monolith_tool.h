//==============================================================================
/**
 * @file    monolith_tool.h
 * @brief   モノリス画面用ツール
 * @author  matsuda
 * @date    2009.11.24(火)
 */
//==============================================================================
#pragma once

#include "savedata/intrude_save.h"  //OCCUPY_INFO


//==============================================================================
//  定数定義
//==============================================================================
//--------------------------------------------------------------
//  パネル
//--------------------------------------------------------------
///パネルのサイズ
typedef enum{
  PANEL_SIZE_LARGE,         ///<大きいパネル
  PANEL_SIZE_SMALL,         ///<小さいパネル
}PANEL_SIZE;

///パネル座標X
#define PANEL_POS_X     (128)
///パネル(大)のBMPFONT描画可能サイズ(キャラクタ単位)
#define PANEL_LARGE_CHARSIZE_X  (22)
///パネル(小)のBMPFONT描画可能サイズ(キャラクタ単位)
#define PANEL_SMALL_CHARSIZE_X  (18)
///パネルのBMPFONT描画可能サイズ(キャラクタ単位) : 大小兼用
#define PANEL_CHARSIZE_Y        (2)

///パネルの何もフォーカスしない場合の指定
#define PANEL_NO_FOCUS        (0xff)

//--------------------------------------------------------------
//  キャンセルアイコン
//--------------------------------------------------------------
///キャンセルアイコンの座標
enum{
  CANCEL_POS_X = 256-16,
  CANCEL_POS_Y = 192-16,
  
  CANCEL_TOUCH_RANGE_HALF = 8,  ///<キャンセルアイコンのヒット範囲半径
};

//==============================================================================
//  構造体定義
//==============================================================================
///パネルアクター構造体
typedef struct{
  GFL_CLWK *cap;
  BMPOAM_ACT_PTR bmpoam;
  GFL_BMP_DATA *bmp;
  u8 trans_flag;
  u8 padding[3];
}PANEL_ACTOR;

///モノリスBMPOAM構造体
typedef struct{
  GFL_BMP_DATA *bmp;
  BMPOAM_ACT_PTR bmpoam;
  u8 trans_flag;
  u8 padding[3];
}MONOLITH_BMPSTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  パネルOBJ
//--------------------------------------------------------------
extern void MonolithTool_PanelOBJ_Create(MONOLITH_SETUP *setup, PANEL_ACTOR *dest, COMMON_RESOURCE_INDEX res_index, PANEL_SIZE size, int y, u32 msg_id, WORDSET *wordset);
extern void MonolithTool_PanelOBJ_Delete(PANEL_ACTOR *panel);
extern void MonolithTool_PanelOBJ_SetEnable(PANEL_ACTOR *panel, BOOL on_off);
extern BOOL MonolithTool_PanelOBJ_TransUpdate(MONOLITH_SETUP *setup, PANEL_ACTOR *panel);
extern void MonolithTool_PanelOBJ_Focus(MONOLITH_APP_PARENT *appwk, PANEL_ACTOR panel[], 
  int panel_max, int focus_no, FADEREQ req);
extern void MonolithTool_PanelOBJ_Flash(MONOLITH_APP_PARENT *appwk, PANEL_ACTOR panel[], 
  int panel_max, int focus_no, FADEREQ req);

//--------------------------------------------------------------
//  パネルBG
//--------------------------------------------------------------
extern void MonolithTool_PanelBG_Focus(MONOLITH_APP_PARENT *appwk, BOOL focus_use, FADEREQ req);
extern void MonolithTool_PanelBG_Flash(MONOLITH_APP_PARENT *appwk, FADEREQ req);

//--------------------------------------------------------------
//  パネル共通処理
//--------------------------------------------------------------
extern void MonolithTool_Panel_Init(MONOLITH_APP_PARENT *appwk);
extern PANEL_COLORMODE MonolithTool_PanelColor_GetMode(MONOLITH_APP_PARENT *appwk);
extern void MonolithTool_Panel_ColorUpdate(MONOLITH_APP_PARENT *appwk, FADEREQ req);

//--------------------------------------------------------------
//  BMPOAM
//--------------------------------------------------------------
extern void MonolithTool_Bmpoam_Create(MONOLITH_SETUP *setup, MONOLITH_BMPSTR *dest, COMMON_RESOURCE_INDEX res_index, int act_x, int act_y, int bmp_size_x, int bmp_size_y, u32 msg_id, WORDSET *wordset);
extern void MonolithTool_Bmpoam_Delete(MONOLITH_BMPSTR *bmpstr);
extern BOOL MonolithTool_Bmpoam_TransUpdate(MONOLITH_SETUP *setup, MONOLITH_BMPSTR *bmpstr);

extern OCCUPY_INFO * MonolithTool_GetOccupyInfo(MONOLITH_APP_PARENT *appwk);

extern GFL_CLWK * MonolithTool_TownIcon_Create(MONOLITH_SETUP *setup, 
  const OCCUPY_INFO *occupy, int town_no, const GFL_POINT *pos, COMMON_RESOURCE_INDEX res_index);
extern void MonolithTool_TownIcon_Delete(GFL_CLWK *cap);
extern void MonolithTool_TownIcon_Update(GFL_CLWK *cap, const OCCUPY_INFO *occupy, int town_no);

extern GFL_CLWK * MonolithTool_CancelIcon_Create(MONOLITH_SETUP *setup);
extern void MonolithTool_CancelIcon_Delete(GFL_CLWK *cap);
extern void MonolithTool_CancelIcon_Update(GFL_CLWK *cap);


//==============================================================================
//  外部データ
//==============================================================================
extern const u32 TownNo_to_Type[];
