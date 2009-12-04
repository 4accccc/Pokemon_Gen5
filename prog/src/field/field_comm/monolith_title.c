//==============================================================================
/**
 * @file    monolith_title.c
 * @brief   モノリス：ミッション説明
 * @author  matsuda
 * @date    2009.11.20(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "intrude_mission.h"
#include "msg/msg_monolith.h"


//==============================================================================
//  定数定義
//==============================================================================
///パネル数
enum{
  TITLE_PANEL_MISSION,
  TITLE_PANEL_STATUS,
  TITLE_PANEL_RECORD,
  TITLE_PANEL_POWER,
  TITLE_PANEL_CANCEL,
  
  TITLE_PANEL_MAX,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  PANEL_ACTOR panel[TITLE_PANEL_MAX];
  s8 cursor_pos;      ///<カーソル位置
  u8 padding[3];
}MONOLITH_TITLE_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithTitleProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithTitleProc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithTitleProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Title_PanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw);
static void _Title_PanelDelete(MONOLITH_TITLE_WORK *mtw);
static void _Title_PanelUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_Title = {
  MonolithTitleProc_Init,
  MonolithTitleProc_Main,
  MonolithTitleProc_End,
};

//--------------------------------------------------------------
//  パネル
//--------------------------------------------------------------
///パネルY座標
enum{
  TITLE_PANEL_Y_MISSION = 8*2 + 4,
  TITLE_PANEL_Y_STATUS = 8*6 + 4,
  TITLE_PANEL_Y_RECORD = 8*10 + 4,
  TITLE_PANEL_Y_POWER = 8*14 + 4,
  TITLE_PANEL_Y_CANCEL = 8*21 + 4,
};

///パネルY座標テーブル
static const u32 TitlePanelTblY[] = {
  TITLE_PANEL_Y_MISSION,
  TITLE_PANEL_Y_STATUS,
  TITLE_PANEL_Y_RECORD,
  TITLE_PANEL_Y_POWER,
  TITLE_PANEL_Y_CANCEL,
};

///パネルタッチデータ   ※TITLE_PANEL_MISSION等と並びを同じにしておくこと！
static const GFL_UI_TP_HITTBL TitlePanelRect[] = {
  {//TITLE_PANEL_Y_MISSION
    TITLE_PANEL_Y_MISSION - PANEL_CHARSIZE_Y/2*8,
    TITLE_PANEL_Y_MISSION + PANEL_CHARSIZE_Y/2*8,
    PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8,
    PANEL_POS_X + PANEL_SMALL_CHARSIZE_X/2*8,
  },
  {//TITLE_PANEL_Y_STATUS
    TITLE_PANEL_Y_STATUS - PANEL_CHARSIZE_Y/2*8,
    TITLE_PANEL_Y_STATUS + PANEL_CHARSIZE_Y/2*8,
    PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8,
    PANEL_POS_X + PANEL_SMALL_CHARSIZE_X/2*8,
  },
  {//TITLE_PANEL_Y_RECORD
    TITLE_PANEL_Y_RECORD - PANEL_CHARSIZE_Y/2*8,
    TITLE_PANEL_Y_RECORD + PANEL_CHARSIZE_Y/2*8,
    PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8,
    PANEL_POS_X + PANEL_SMALL_CHARSIZE_X/2*8,
  },
  {//TITLE_PANEL_Y_POWER
    TITLE_PANEL_Y_POWER - PANEL_CHARSIZE_Y/2*8,
    TITLE_PANEL_Y_POWER + PANEL_CHARSIZE_Y/2*8,
    PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8,
    PANEL_POS_X + PANEL_SMALL_CHARSIZE_X/2*8,
  },
  {//TITLE_PANEL_Y_CANCEL
    TITLE_PANEL_Y_CANCEL - PANEL_CHARSIZE_Y/2*8,
    TITLE_PANEL_Y_CANCEL + PANEL_CHARSIZE_Y/2*8,
    PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8,
    PANEL_POS_X + PANEL_SMALL_CHARSIZE_X/2*8,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};
SDK_COMPILER_ASSERT(TITLE_PANEL_MAX == NELEMS(TitlePanelTblY));
SDK_COMPILER_ASSERT(TITLE_PANEL_MAX == NELEMS(TitlePanelRect)-1); //-1=HIT_END


//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithTitleProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_TITLE_WORK *mtw = mywk;
	ARCHANDLE *hdl;
  
  mtw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_TITLE_WORK), HEAPID_MONOLITH);
  GFL_STD_MemClear(mtw, sizeof(MONOLITH_TITLE_WORK));
  
  _Title_PanelCreate(appwk, mtw);
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithTitleProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_TITLE_WORK *mtw = mywk;
  int tp_ret;
  
  _Title_PanelUpdate(appwk, mtw);

  if(appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }

  switch(*seq){
  case 0:
    tp_ret = GFL_UI_TP_HitTrg(TitlePanelRect);
    if(tp_ret != GFL_UI_TP_HIT_NONE){
      OS_TPrintf("タッチ有効 %d\n", tp_ret);
      mtw->cursor_pos = tp_ret;
      (*seq)++;
      break;
    }
    else{
      int repeat = GFL_UI_KEY_GetRepeat();
      int trg = GFL_UI_KEY_GetTrg();
      
      if(trg & PAD_BUTTON_DECIDE){
        (*seq)++;
      }
      else if(trg & PAD_BUTTON_CANCEL){
        mtw->cursor_pos = TITLE_PANEL_CANCEL;
        (*seq)++;
      }
      else if(repeat & PAD_KEY_DOWN){
        mtw->cursor_pos++;
        if(mtw->cursor_pos >= TITLE_PANEL_MAX){
          mtw->cursor_pos = 0;
        }
        MonolithTool_Panel_Focus(
          appwk, mtw->panel, TITLE_PANEL_MAX, mtw->cursor_pos, FADE_SUB_OBJ);
      }
      else if(repeat & PAD_KEY_UP){
        mtw->cursor_pos--;
        if(mtw->cursor_pos < 0){
          mtw->cursor_pos = TITLE_PANEL_MAX - 1;
        }
        MonolithTool_Panel_Focus(
          appwk, mtw->panel, TITLE_PANEL_MAX, mtw->cursor_pos, FADE_SUB_OBJ);
      }
    }
    break;
  case 1:
    if(mtw->cursor_pos == TITLE_PANEL_MISSION && appwk->parent->list_occ == FALSE){
      OS_TPrintf("ミッション選択候補リストが未受信\n");
      (*seq)--;
      break;
    }
    MonolithTool_Panel_Flash(appwk, mtw->panel, TITLE_PANEL_MAX, mtw->cursor_pos, FADE_SUB_OBJ);
    (*seq)++;
    break;
  case 2:
    if(MonolithTool_PanelColor_GetMode(appwk) == PANEL_COLORMODE_NONE){
      appwk->next_menu_index = MONOLITH_MENU_MISSION + mtw->cursor_pos;
      return GFL_PROC_RES_FINISH;
    }
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithTitleProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_TITLE_WORK *mtw = mywk;

  _Title_PanelDelete(mtw);
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * パネル生成
 *
 * @param   appwk		
 * @param   mtw		
 */
//==================================================================
static void _Title_PanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw)
{
  int i;
  
  for(i = 0; i < TITLE_PANEL_MAX; i++){
    MonolithTool_Panel_Create(appwk->setup, &mtw->panel[i], 
      COMMON_RESOURCE_INDEX_DOWN, PANEL_SIZE_SMALL, TitlePanelTblY[i], msg_mono_title_001 + i,
      NULL);
  }
  
  MonolithTool_Panel_Focus(appwk, mtw->panel, TITLE_PANEL_MAX, 0, FADE_SUB_OBJ);
}

//==================================================================
/**
 * パネル削除
 *
 * @param   mtw		
 */
//==================================================================
static void _Title_PanelDelete(MONOLITH_TITLE_WORK *mtw)
{
  int i;
  
  for(i = 0; i < TITLE_PANEL_MAX; i++){
    MonolithTool_Panel_Delete(&mtw->panel[i]);
  }
}

//==================================================================
/**
 * パネル更新処理
 *
 * @param   mtw		
 */
//==================================================================
static void _Title_PanelUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw)
{
  int i;
  
  MonolithTool_Panel_ColorUpdate(appwk, FADE_SUB_OBJ);
  for(i = 0; i < TITLE_PANEL_MAX; i++){
    MonolithTool_Panel_TransUpdate(appwk->setup, &mtw->panel[i]);
  }
}
