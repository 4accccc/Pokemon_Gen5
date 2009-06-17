//======================================================================
/**
 * @file  plist_local_def.h
 * @brief ポケモンリスト ローカル定義
 * @author  ariizumi
 * @data  09/06/10
 */
//======================================================================
#pragma once

#include "app/pokelist.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "debug/debugwin_sys.h"

#define PLIST_LIST_MAX (6)

//BG面定義(アルファ設定のところは定義が違うので変えたら変更すること
#define PLIST_BG_MAIN_BG (GFL_BG_FRAME3_M)
#define PLIST_BG_PLATE   (GFL_BG_FRAME2_M)
#define PLIST_BG_PARAM   (GFL_BG_FRAME1_M)
#define PLIST_BG_MENU    (GFL_BG_FRAME0_M)
#define PLIST_BG_3D      (GFL_BG_FRAME0_M)

#define PLIST_BG_SUB_BG (GFL_BG_FRAME3_S)

//BGパレット
#define PLIST_BG_PLT_FONT (0x0e)

//文字色
#define PLIST_FONT_PARAM_LETTER (0xF)
#define PLIST_FONT_PARAM_SHADOW (0x1)

//OBJリソースIdx
typedef enum
{
  PCR_PLT_OBJ_COMMON,
  PCR_PLT_ITEM_ICON,
  PCR_PLT_CONDITION,
  PCR_PLT_HP_BASE,
  PCR_PLT_POKEICON,
  
  PCR_NCG_CURSOR,
  PCR_NCG_BALL,
  PCR_NCG_BAR_ICON,
  PCR_NCG_ITEM_ICON,
  PCR_NCG_CONDITION,
  PCR_NCG_HP_BASE,
  //ポケアイコンのNCGは各プレートで持つ
  
  PCR_ANM_CURSOR,
  PCR_ANM_BALL,
  PCR_ANM_BAR_ICON,
  PCR_ANM_ITEM_ICON,
  PCR_ANM_CONDITION,
  PCR_ANM_HP_BASE,
  PCR_ANM_POKEICON,
  
  PCR_PLT_START = PCR_PLT_OBJ_COMMON,
  PCR_PLT_END = PCR_PLT_POKEICON,
  PCR_NCG_START = PCR_NCG_CURSOR,
  PCR_NCG_END = PCR_NCG_HP_BASE,
  PCR_ANM_START = PCR_ANM_CURSOR,
  PCR_ANM_END = PCR_ANM_POKEICON,
  
  PCR_MAX,
  
}PLIST_CEL_RESOURCE;

//OBJパレットの開始位置
#define PLIST_OBJPLT_COMMON (0)     //3本
#define PLIST_OBJPLT_ITEM_ICON (3)  //1本
#define PLIST_OBJPLT_CONDITION (4)  //1本
#define PLIST_OBJPLT_HP_BASE   (5)  //1本
#define PLIST_OBJPLT_POKEICON  (6)  //3本

//カーソルのアニメ
enum PLIST_CURCOR_ANIME
{
  PCA_NORMAL_A, //通常0番用
  PCA_NORMAL_B, //通常
  PCA_CHANGE_A, //入れ替え0番用
  PCA_CHANGE_B, //入れ替え
  PCA_SELECT_A, //選択0番用
  PCA_SELECT_B, //選択
  PCA_MENU,   //メニュー用

};


typedef struct _PLIST_PLATE_WORK PLIST_PLATE_WORK;
typedef struct _PLIST_DEBUG_WORK PLIST_DEBUG_WORK;

typedef struct
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;

  GFL_MSGDATA *msgHandle;
  GFL_FONT *fontHandle;
  GFL_FONT *sysFontHandle;
  PRINT_QUE *printQue;
  
  //プレートScr
  void  *plateScrRes;
  NNSG2dScreenData *plateScrData;

  //プレートデータ
  PLIST_PLATE_WORK  *plateWork[PLIST_LIST_MAX];
  
  //Cell系
  u32 cellRes[PCR_MAX];
  GFL_CLUNIT  *cellUnit;
  GFL_CLWK    *clwkCursor[2];
  

  PLIST_DATA *plData;
#if USE_DEBUGWIN_SYSTEM
  PLIST_DEBUG_WORK *debWork;
#endif
}PLIST_WORK;

