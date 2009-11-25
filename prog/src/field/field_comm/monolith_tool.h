//==============================================================================
/**
 * @file    monolith_tool.h
 * @brief   モノリス画面用ツール
 * @author  matsuda
 * @date    2009.11.24(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  定数定義
//==============================================================================
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


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void MonolithTool_Panel_Create(MONOLITH_SETUP *setup, PANEL_ACTOR *dest, COMMON_RESOURCE_INDEX res_index, PANEL_SIZE size, int y, u32 msg_id);
extern void MonolithTool_Panel_Delete(PANEL_ACTOR *panel);
extern BOOL MonolithTool_Panel_TransUpdate(MONOLITH_SETUP *setup, PANEL_ACTOR *panel);

