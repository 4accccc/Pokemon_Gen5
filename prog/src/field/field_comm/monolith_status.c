//==============================================================================
/**
 * @file    monolith_status.c
 * @brief   モノリス：状態を見る
 * @author  matsuda
 * @date    2009.12.04(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "msg/msg_monolith.h"
#include "mission_types.h"
#include "intrude_mission.h"
#include "intrude_work.h"
#include "monolith_snd_def.h"

#include "monolith.naix"


//==============================================================================
//  定数定義
//==============================================================================
///BMPWIN
enum{
  BMPWIN_TITLE,               ///<タイトル
  BMPWIN_LEVEL,               ///<モノリスレベル
  BMPWIN_WBLV,                ///<ホワイト＆ブラックのレベル
  BMPWIN_TIME,                ///<滞在時間
  BMPWIN_CLEAR_MISSION,       ///<クリアしたミッション数
  BMPWIN_USE_GPOWER,          ///<使えるGパワー数
  
  BMPWIN_MAX,
};

///BMPWINのサイズ定義など
enum{
  BMPWIN_POS_TITLE_X = 2,
  BMPWIN_POS_TITLE_Y = 0,
  BMPWIN_POS_TITLE_SIZE_X = 32 - BMPWIN_POS_TITLE_X,
  BMPWIN_POS_TITLE_SIZE_Y = 3,

  //合計レベル
  BMPWIN_POS_LEVEL_X = 2,
  BMPWIN_POS_LEVEL_Y = 5,
  BMPWIN_POS_LEVEL_SIZE_X = 32 - BMPWIN_POS_LEVEL_X,
  BMPWIN_POS_LEVEL_SIZE_Y = 2,
  
  //ホワイト＆ブラックのレベル
  BMPWIN_POS_WBLV_X = 2,
  BMPWIN_POS_WBLV_Y = BMPWIN_POS_LEVEL_Y + 3,
  BMPWIN_POS_WBLV_SIZE_X = 32 - BMPWIN_POS_WBLV_X,
  BMPWIN_POS_WBLV_SIZE_Y = 2,
  
  //滞在時間
  BMPWIN_POS_TIME_X = 2,
  BMPWIN_POS_TIME_Y = 14,
  BMPWIN_POS_TIME_SIZE_X = 32 - BMPWIN_POS_TIME_X,
  BMPWIN_POS_TIME_SIZE_Y = 4,
  
  //クリアしたミッション
  BMPWIN_POS_CLEAR_MISSION_X = BMPWIN_POS_TIME_X,
  BMPWIN_POS_CLEAR_MISSION_Y = BMPWIN_POS_TIME_Y + 2,
  BMPWIN_POS_CLEAR_MISSION_SIZE_X = 32 - BMPWIN_POS_CLEAR_MISSION_X,
  BMPWIN_POS_CLEAR_MISSION_SIZE_Y = 2,
  
  //使えるGパワーの数
  BMPWIN_POS_USE_GPOWER_X = BMPWIN_POS_TIME_X,
  BMPWIN_POS_USE_GPOWER_Y = BMPWIN_POS_CLEAR_MISSION_Y + 2,
  BMPWIN_POS_USE_GPOWER_SIZE_X = 32 - BMPWIN_POS_USE_GPOWER_X,
  BMPWIN_POS_USE_GPOWER_SIZE_Y = 2,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin[BMPWIN_MAX];
  PRINT_UTIL print_util[BMPWIN_MAX];
  MONOLITH_CANCEL_ICON cancel_icon;
}MONOLITH_STATUS_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithStatusProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithStatusProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithStatusProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_SETUP *setup);
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_STATUS_WORK *msw);
static void _Setup_BmpWin_Exit(MONOLITH_STATUS_WORK *msw);
static void _Write_Status(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_STATUS_WORK *msw);
static void _Status_CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_STATUS_WORK *msw);
static void _Status_CancelIconDelete(MONOLITH_STATUS_WORK *msw);
static void _Status_CancelIconUpdate(MONOLITH_STATUS_WORK *msw);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_Status = {
  MonolithStatusProc_Init,
  MonolithStatusProc_Main,
  MonolithStatusProc_End,
};

///タッチ範囲テーブル
static const GFL_UI_TP_HITTBL StatusTouchRect[] = {
  {
    CANCEL_POS_Y - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_Y + CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X + CANCEL_TOUCH_RANGE_HALF,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};



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
static GFL_PROC_RESULT MonolithStatusProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_STATUS_WORK *msw = mywk;
	ARCHANDLE *hdl;
  
  switch(*seq){
  case 0:
    msw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_STATUS_WORK), HEAPID_MONOLITH);
    GFL_STD_MemClear(msw, sizeof(MONOLITH_STATUS_WORK));
    
    //BG
    _Setup_BGFrameSetting();
    _Setup_BGGraphicLoad(appwk->setup);
    _Setup_BmpWin_Create(appwk->setup, msw);
    //OBJ
    _Status_CancelIconCreate(appwk, msw);
    
    //メッセージ描画
    _Write_Status(appwk, appwk->setup, msw);
    
    (*seq)++;
    break;
  case 1:
    {
      int i, no_trans = FALSE;
      for(i = 0; i < BMPWIN_MAX; i++){
        if(PRINT_UTIL_Trans(&msw->print_util[i], appwk->setup->printQue) == FALSE){
          no_trans = TRUE;
        }
      }
      if(no_trans == FALSE){
      	GFL_BG_SetVisible(GFL_BG_FRAME0_S, VISIBLE_ON);
      	GFL_BG_SetVisible(GFL_BG_FRAME2_S, VISIBLE_ON);
      	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
      	return GFL_PROC_RES_FINISH;
      }
    }
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
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
static GFL_PROC_RESULT MonolithStatusProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_STATUS_WORK *msw = mywk;
  int tp_ret;
  int i;

  if(appwk->up_proc_finish == TRUE){// || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Trans(&msw->print_util[i], appwk->setup->printQue);
  }
  _Status_CancelIconUpdate(msw);
  
  switch(*seq){
  case 0:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int tp_ret = GFL_UI_TP_HitTrg(StatusTouchRect);

      if(tp_ret != GFL_UI_TP_HIT_NONE || (trg & PAD_BUTTON_CANCEL)){
        OS_TPrintf("キャンセル選択\n");
        MonolithTool_CancelIcon_FlashReq(&msw->cancel_icon);
        if(trg & PAD_BUTTON_CANCEL){
          GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        }
        else{
          GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        }
        PMSND_PlaySE(MONOLITH_SE_CANCEL);
        (*seq)++;
      }
    }
    break;
  case 1:
    if(MonolithTool_CancelIcon_FlashCheck(&msw->cancel_icon) == FALSE){
      appwk->next_menu_index = MONOLITH_MENU_TITLE;
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
static GFL_PROC_RESULT MonolithStatusProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_STATUS_WORK *msw = mywk;

  if(PRINTSYS_QUE_IsFinished(appwk->setup->printQue) == FALSE){
    return GFL_PROC_RES_CONTINUE;
  }
  
  GFL_DISP_GXS_SetVisibleControlDirect(GX_PLANEMASK_BG3);

  //OBJ
  _Status_CancelIconDelete(msw);
  
  //BG
  _Setup_BmpWin_Exit(msw);
  _Setup_BGFrameExit();
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 */
//--------------------------------------------------------------
static void _Setup_BGFrameSetting(void)
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {
		{//GFL_BG_FRAME0_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		},
		{//GFL_BG_FRAME2_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_S,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );

	GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME2_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 */
//--------------------------------------------------------------
static void _Setup_BGFrameExit(void)
{
	GFL_BG_SetVisible(GFL_BG_FRAME0_S, VISIBLE_OFF);
	GFL_BG_SetVisible(GFL_BG_FRAME2_S, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME2_S);
}

//--------------------------------------------------------------
/**
 * @brief   BGグラフィックをVRAMへ転送
 *
 * @param   hdl		アーカイブハンドル
 */
//--------------------------------------------------------------
static void _Setup_BGGraphicLoad(MONOLITH_SETUP *setup)
{
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgd_joutai_lz_NSCR, 
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_MONOLITH);

	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   setup		
 * @param   msw		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_STATUS_WORK *msw)
{
  int i;
  
  //モノリスタイトル
  msw->bmpwin[BMPWIN_TITLE] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_TITLE_X, BMPWIN_POS_TITLE_Y, 
    BMPWIN_POS_TITLE_SIZE_X, BMPWIN_POS_TITLE_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_TITLE] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_TITLE] );

  //モノリスレベル
  msw->bmpwin[BMPWIN_LEVEL] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_LEVEL_X, BMPWIN_POS_LEVEL_Y, 
    BMPWIN_POS_LEVEL_SIZE_X, BMPWIN_POS_LEVEL_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_LEVEL] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_LEVEL] );

  //モノリスの通称
  msw->bmpwin[BMPWIN_WBLV] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_WBLV_X, BMPWIN_POS_WBLV_Y, 
    BMPWIN_POS_WBLV_SIZE_X, BMPWIN_POS_WBLV_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_WBLV] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_WBLV] );

  //パワーバランス
  msw->bmpwin[BMPWIN_TIME] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_TIME_X, BMPWIN_POS_TIME_Y, 
    BMPWIN_POS_TIME_SIZE_X, BMPWIN_POS_TIME_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_TIME] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_TIME] );

  //使えるパワーの数
  msw->bmpwin[BMPWIN_CLEAR_MISSION] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_CLEAR_MISSION_X, BMPWIN_POS_CLEAR_MISSION_Y, 
    BMPWIN_POS_CLEAR_MISSION_SIZE_X, BMPWIN_POS_CLEAR_MISSION_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_CLEAR_MISSION] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_CLEAR_MISSION] );

  //滞在時間
  msw->bmpwin[BMPWIN_USE_GPOWER] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_USE_GPOWER_X, BMPWIN_POS_USE_GPOWER_Y, 
    BMPWIN_POS_USE_GPOWER_SIZE_X, BMPWIN_POS_USE_GPOWER_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_USE_GPOWER] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_USE_GPOWER] );

  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Setup(&msw->print_util[i], msw->bmpwin[i]);
  }
  
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   msw		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Exit(MONOLITH_STATUS_WORK *msw)
{
  int i;
  
  for(i = 0; i < BMPWIN_MAX; i++){
    GFL_BMPWIN_Delete(msw->bmpwin[i]);
  }
}

//--------------------------------------------------------------
/**
 * ミッション説明描画
 *
 * @param   appwk		
 * @param   setup		
 * @param   msw		
 * @param   select_town		
 */
//--------------------------------------------------------------
static void _Write_Status(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_STATUS_WORK *msw)
{
  STRBUF *strbuf, *expand_strbuf;
  const OCCUPY_INFO *occupy;
  MYSTATUS *myst = MonolithTool_GetMystatus(appwk);
  
  occupy = MonolithTool_GetOccupyInfo(appwk);
  
  strbuf = GFL_STR_CreateBuffer(256, HEAPID_MONOLITH);
  expand_strbuf = GFL_STR_CreateBuffer( 256, HEAPID_MONOLITH );

  //タイトル
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_008, strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TITLE], setup->printQue, 
    0, 4, strbuf, setup->font_handle);
  
  //ハイルツリーのレベル
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_000, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->black_level + occupy->white_level, 5, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT); // level + 1 = 表示を1origin化
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_LEVEL], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  //ブラックのレベル
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_004, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->black_level, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_WBLV], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  //ホワイトのレベル
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_003, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->white_level, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_WBLV], setup->printQue, 
    BMPWIN_POS_WBLV_SIZE_X / 2 * 8, 0, expand_strbuf, setup->font_handle);
  
  //滞在時間
  {
    s64 src_seq;
    u32 hour, minute;
    src_seq = appwk->parent->monolith_status.palace_sojourn_time;
    hour = src_seq / 60 / 60;
    if(hour > 999){
      hour = 999;
      minute = 59;
    }
    else{
      src_seq -= hour * 60 * 60;
      minute = src_seq / 60;
    }

    GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_006, strbuf);
    
    WORDSET_RegisterNumber(setup->wordset, 0, hour, 3, 
      STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
    WORDSET_RegisterNumber(setup->wordset, 1, minute, 2, 
      STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
    WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
    PRINT_UTIL_Print(&msw->print_util[BMPWIN_TIME], setup->printQue, 
      0, 0, expand_strbuf, setup->font_handle);
  }
  
  //クリアしたミッションの数
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_007, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, 
    appwk->parent->monolith_status.clear_mission_count, 5, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_CLEAR_MISSION], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  //使えるGパワーの数
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_005, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, 
    MonolithTool_CountUsePower(
    setup, MonolithTool_GetOccupyInfo(appwk), &appwk->parent->monolith_status), 
    4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_USE_GPOWER], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  GFL_STR_DeleteBuffer(strbuf);
  GFL_STR_DeleteBuffer(expand_strbuf);
}

//==================================================================
/**
 * キャンセルアイコン生成
 *
 * @param   appwk		
 * @param   msw		
 */
//==================================================================
static void _Status_CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_STATUS_WORK *msw)
{
  MonolithTool_CancelIcon_Create(appwk->setup, &msw->cancel_icon);
}

//==================================================================
/**
 * キャンセルアイコン削除
 *
 * @param   msw		
 */
//==================================================================
static void _Status_CancelIconDelete(MONOLITH_STATUS_WORK *msw)
{
  MonolithTool_CancelIcon_Delete(&msw->cancel_icon);
}

//==================================================================
/**
 * キャンセルアイコン更新処理
 *
 * @param   msw		
 */
//==================================================================
static void _Status_CancelIconUpdate(MONOLITH_STATUS_WORK *msw)
{
  MonolithTool_CancelIcon_Update(&msw->cancel_icon);
}

