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

#include "monolith.naix"


//==============================================================================
//  定数定義
//==============================================================================
///BMPWIN
enum{
  BMPWIN_TITLE,           ///<タイトル
  BMPWIN_LEVEL,           ///<モノリスレベル
  BMPWIN_NICKNAME,        ///<モノリスの通称
  BMPWIN_POWER_BALANCE,   ///<パワーバランス
  BMPWIN_USE_POWER,       ///<使えるパワーの数
  BMPWIN_TIME,            ///<滞在時間
  
  BMPWIN_MAX,
};

///BMPWINのサイズ定義など
enum{
  BMPWIN_POS_TITLE_X = 2,
  BMPWIN_POS_TITLE_Y = 1,
  BMPWIN_POS_TITLE_SIZE_X = 32 - BMPWIN_POS_TITLE_X,
  BMPWIN_POS_TITLE_SIZE_Y = 2,

  BMPWIN_POS_LEVEL_X = 2,
  BMPWIN_POS_LEVEL_Y = 4,
  BMPWIN_POS_LEVEL_SIZE_X = 32 - BMPWIN_POS_LEVEL_X,
  BMPWIN_POS_LEVEL_SIZE_Y = 2,
  
  BMPWIN_POS_NICKNAME_X = 2,
  BMPWIN_POS_NICKNAME_Y = BMPWIN_POS_LEVEL_Y + 2,
  BMPWIN_POS_NICKNAME_SIZE_X = 32 - BMPWIN_POS_NICKNAME_X,
  BMPWIN_POS_NICKNAME_SIZE_Y = 2,
  
  BMPWIN_POS_POWER_BALANCE_X = 4,
  BMPWIN_POS_POWER_BALANCE_Y = BMPWIN_POS_NICKNAME_Y + 4,
  BMPWIN_POS_POWER_BALANCE_SIZE_X = 32 - BMPWIN_POS_POWER_BALANCE_X,
  BMPWIN_POS_POWER_BALANCE_SIZE_Y = 4,
  
  BMPWIN_POS_USE_POWER_X = BMPWIN_POS_POWER_BALANCE_X,
  BMPWIN_POS_USE_POWER_Y = 15,
  BMPWIN_POS_USE_POWER_SIZE_X = 32 - BMPWIN_POS_USE_POWER_X,
  BMPWIN_POS_USE_POWER_SIZE_Y = 2,
  
  BMPWIN_POS_TIME_X = BMPWIN_POS_POWER_BALANCE_X,
  BMPWIN_POS_TIME_Y = BMPWIN_POS_USE_POWER_Y + 2,
  BMPWIN_POS_TIME_SIZE_X = 32 - BMPWIN_POS_TIME_X,
  BMPWIN_POS_TIME_SIZE_Y = 2,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin[BMPWIN_MAX];
  PRINT_UTIL print_util[BMPWIN_MAX];
  GFL_CLWK *act_cancel;
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
static GFL_PROC_RESULT MonolithStatusProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_STATUS_WORK *msw = mywk;
  int tp_ret;
  int i;

  if(appwk->up_proc_finish == TRUE || appwk->force_finish == TRUE){
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
        (*seq)++;
      }
    }
    break;
  case 1:
    appwk->next_menu_index = MONOLITH_MENU_TITLE;
    return GFL_PROC_RES_FINISH;
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
	GFL_BG_SetVisible(GFL_BG_FRAME0_S, VISIBLE_ON);
	GFL_BG_SetVisible(GFL_BG_FRAME2_S, VISIBLE_ON);
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
  msw->bmpwin[BMPWIN_NICKNAME] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_NICKNAME_X, BMPWIN_POS_NICKNAME_Y, 
    BMPWIN_POS_NICKNAME_SIZE_X, BMPWIN_POS_NICKNAME_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_NICKNAME] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_NICKNAME] );

  //パワーバランス
  msw->bmpwin[BMPWIN_POWER_BALANCE] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_POWER_BALANCE_X, BMPWIN_POS_POWER_BALANCE_Y, 
    BMPWIN_POS_POWER_BALANCE_SIZE_X, BMPWIN_POS_POWER_BALANCE_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_POWER_BALANCE] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_POWER_BALANCE] );

  //使えるパワーの数
  msw->bmpwin[BMPWIN_USE_POWER] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_USE_POWER_X, BMPWIN_POS_USE_POWER_Y, 
    BMPWIN_POS_USE_POWER_SIZE_X, BMPWIN_POS_USE_POWER_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_USE_POWER] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_USE_POWER] );

  //滞在時間
  msw->bmpwin[BMPWIN_TIME] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_TIME_X, BMPWIN_POS_TIME_Y, 
    BMPWIN_POS_TIME_SIZE_X, BMPWIN_POS_TIME_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_TIME] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_TIME] );

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
  MYSTATUS *myst = Intrude_GetMyStatus(appwk->parent->intcomm, appwk->parent->palace_area);
  
  occupy = Intrude_GetOccupyInfo(appwk->parent->intcomm, appwk->parent->palace_area);
  
  strbuf = GFL_STR_CreateBuffer(256, HEAPID_MONOLITH);
  expand_strbuf = GFL_STR_CreateBuffer( 256, HEAPID_MONOLITH );

  //タイトル
	GFL_STR_SetStringCodeOrderLength(
	  strbuf, MyStatus_GetMyName(myst), PERSON_NAME_SIZE + EOM_SIZE);
  WORDSET_RegisterWord(setup->wordset, 0, strbuf, MyStatus_GetMySex(myst), TRUE, PM_LANG);
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_title_000, strbuf);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TITLE], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  //モノリスレベル
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_000, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->white_level, 3, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT); // level + 1 = 表示を1origin化
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_LEVEL], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  //通称
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_001, strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_NICKNAME], setup->printQue, 
    0, 0, strbuf, setup->font_handle);
  
  //パワーバランス
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_002, strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_POWER_BALANCE], setup->printQue, 
    0, 8, strbuf, setup->font_handle);
  //パワーバランス：WHITE
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_003, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, 560, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_POWER_BALANCE], setup->printQue, 
    14*8, 0, expand_strbuf, setup->font_handle);
  //パワーバランス：BLACK
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_004, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, 70, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_POWER_BALANCE], setup->printQue, 
    14*8, 16, expand_strbuf, setup->font_handle);
  
  //使えるパワーの数
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_005, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, 99, 3, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_USE_POWER], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  //滞在時間
  GFL_MSG_GetString(setup->mm_monolith, msg_mono_st_006, strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, 99, 3, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(setup->wordset, 1, 99, 2, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TIME], setup->printQue, 
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
  msw->act_cancel = MonolithTool_CancelIcon_Create(appwk->setup);
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
  MonolithTool_CancelIcon_Delete(msw->act_cancel);
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
  MonolithTool_CancelIcon_Update(msw->act_cancel);
}
