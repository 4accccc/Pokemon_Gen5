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
#include "app/app_taskmenu.h"


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
  PANEL_SIZE_DECIDE,        ///<決定パネル
}PANEL_SIZE;

///パネル座標X
#define PANEL_POS_X     (128)
///パネル(大)のBMPFONT描画可能サイズ(キャラクタ単位)
#define PANEL_LARGE_CHARSIZE_X  (22)
///パネル(小)のBMPFONT描画可能サイズ(キャラクタ単位)
#define PANEL_SMALL_CHARSIZE_X  (18)
///パネル(決定)のBMPFONT描画可能サイズ(キャラクタ単位)
#define PANEL_DECIDE_CHARSIZE_X  (12)
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
  CANCEL_POS_Y = 192-12,
  
  CANCEL_TOUCH_RANGE_HALF = 8,  ///<キャンセルアイコンのヒット範囲半径
};

//--------------------------------------------------------------
//  バランスゲージ
//--------------------------------------------------------------
typedef enum{
  BALANCE_GAUGE_UP,       ///<上画面への表示
  BALANCE_GAUGE_DOWN,     ///<下画面への表示
}BALANCE_GAUGE_DISP;


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
extern void MonolithTool_PanelOBJ_Create(MONOLITH_SETUP *setup, PANEL_ACTOR *dest, COMMON_RESOURCE_INDEX res_index, PANEL_SIZE size, int x, int y, u32 msg_id, WORDSET *wordset);
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
extern PANEL_COLORMODE MonolithTool_PanelColor_GetMode(MONOLITH_APP_PARENT *appwk, FADEREQ req);
extern void MonolithTool_Panel_ColorUpdate(MONOLITH_APP_PARENT *appwk, FADEREQ req);

//--------------------------------------------------------------
//  BMPOAM
//--------------------------------------------------------------
extern void MonolithTool_Bmpoam_Create(MONOLITH_SETUP *setup, MONOLITH_BMPSTR *dest, COMMON_RESOURCE_INDEX res_index, int act_x, int act_y, int bmp_size_x, int bmp_size_y, u32 msg_id, WORDSET *wordset);
extern void MonolithTool_Bmpoam_Delete(MONOLITH_BMPSTR *bmpstr);
extern BOOL MonolithTool_Bmpoam_TransUpdate(MONOLITH_SETUP *setup, MONOLITH_BMPSTR *bmpstr);
extern void MonolithTool_Bmpoam_BGPriSet(MONOLITH_SETUP *setup, MONOLITH_BMPSTR *bmpstr, int bg_pri);

extern MYSTATUS * MonolithTool_GetMystatus(MONOLITH_APP_PARENT *appwk);
extern OCCUPY_INFO * MonolithTool_GetOccupyInfo(MONOLITH_APP_PARENT *appwk);

extern GFL_CLWK * MonolithTool_TownIcon_Create(MONOLITH_SETUP *setup, 
  const OCCUPY_INFO *occupy, int town_no, const GFL_POINT *pos, COMMON_RESOURCE_INDEX res_index);
extern void MonolithTool_TownIcon_Delete(GFL_CLWK *cap);
extern void MonolithTool_TownIcon_Update(GFL_CLWK *cap, const OCCUPY_INFO *occupy, int town_no);

extern GFL_CLWK * MonolithTool_TownCursor_Create(MONOLITH_SETUP *setup, const GFL_POINT *pos, COMMON_RESOURCE_INDEX res_index);
extern void MonolithTool_TownCursor_Delete(GFL_CLWK *cap);

extern void MonolithTool_CancelIcon_Create(MONOLITH_SETUP *setup, MONOLITH_CANCEL_ICON *cancel);
extern void MonolithTool_CancelIcon_Delete(MONOLITH_CANCEL_ICON *cancel);
extern void MonolithTool_CancelIcon_Update(MONOLITH_CANCEL_ICON *cancel);
extern void MonolithTool_CancelIcon_FlashReq(MONOLITH_CANCEL_ICON *cancel);
extern BOOL MonolithTool_CancelIcon_FlashCheck(MONOLITH_CANCEL_ICON *cancel);

extern GFL_CLWK * MonolithTool_Arrow_Create(MONOLITH_SETUP *setup, int x, int y, int anmseq);
extern void MonolithTool_ArrowIcon_Delete(GFL_CLWK *cap);
extern void MonolithTool_ArrowIcon_Update(GFL_CLWK *cap);

extern APP_TASKMENU_RES * MonolithTool_TaskMenuCreate(MONOLITH_SETUP *setup, int frame_no, HEAPID heap_id);
extern void MonolithTool_TaskMenuDelete(APP_TASKMENU_RES *app_task_res);
extern APP_TASKMENU_WORK * MonolithTool_TaskMenu_YesNoInit(MONOLITH_SETUP *setup, APP_TASKMENU_RES *app_task_res, HEAPID heap_id);
extern void MonolithTool_TaskMenu_YesNoExit(APP_TASKMENU_WORK *app_menu);
extern BOOL MonolithTool_TaskMenu_Update(MONOLITH_SETUP *setup, int frame_no, APP_TASKMENU_WORK *app_menu, BOOL *ret_yesno);

extern void MonolithTool_BalanceGaugeRewrite(MONOLITH_APP_PARENT *appwk, BALANCE_GAUGE_DISP up_down, int frame_no);


//==============================================================================
//  外部データ
//==============================================================================
extern const u32 TownNo_to_Type[];
