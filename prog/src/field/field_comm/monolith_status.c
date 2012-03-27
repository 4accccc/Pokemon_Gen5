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
#include "monolith_snd_def.h"
#include "field/monolith_tool_notwifi.h"
#include "savedata/fmission_save.h"

#include "monolith.naix"

//==============================================================================
//  定数定義
//==============================================================================
///BMPWIN
enum{
  BMPWIN_TITLE,               ///<タイトル
  BMPWIN_OPEN,                ///<開催したミッション
  BMPWIN_JOIN,                ///<参加したミッション
  BMPWIN_FINISH,              ///<達成したミッション
  BMPWIN_MEMBER,              ///<最高参加者数
  BMPWIN_TOPSCORE,            ///<トップスコア
  
  BMPWIN_MAX,
};

///BMPWINのサイズ定義など
enum{
  // タイトル
  BMPWIN_POS_TITLE_X = 2,
  BMPWIN_POS_TITLE_Y = 0,
  BMPWIN_POS_TITLE_SIZE_X = 32 - BMPWIN_POS_TITLE_X,
  BMPWIN_POS_TITLE_SIZE_Y = 3,

  //開催したミッション
  BMPWIN_OPEN_X = 2,
  BMPWIN_OPEN_Y = 5,
  BMPWIN_OPEN_SIZE_X = 32 - BMPWIN_OPEN_X,
  BMPWIN_OPEN_SIZE_Y = 2,
  
  //参加したミッション
  BMPWIN_JOIN_X = 2,
  BMPWIN_JOIN_Y = BMPWIN_OPEN_Y + 2,
  BMPWIN_JOIN_SIZE_X = 32 - BMPWIN_JOIN_X,
  BMPWIN_JOIN_SIZE_Y = 2,
  
  //達成したミッション
  BMPWIN_FINISH_X = 2,
  BMPWIN_FINISH_Y = BMPWIN_JOIN_Y + 2,
  BMPWIN_FINISH_SIZE_X = 32 - BMPWIN_FINISH_X,
  BMPWIN_FINISH_SIZE_Y = 4,
  
  //最高人数
  BMPWIN_MEMBER_X = BMPWIN_FINISH_X,
  BMPWIN_MEMBER_Y = 12 + 2,
  BMPWIN_MEMBER_SIZE_X = 32 - BMPWIN_MEMBER_X,
  BMPWIN_MEMBER_SIZE_Y = 2,
  
  //トップスコア
  BMPWIN_TOPSCORE_X = BMPWIN_FINISH_X,
  BMPWIN_TOPSCORE_Y = BMPWIN_MEMBER_Y + 2,
  BMPWIN_TOPSCORE_SIZE_X = 32 - BMPWIN_TOPSCORE_X,
  BMPWIN_TOPSCORE_SIZE_Y = 2,
  //
  MES_OFFSET = 16,     // サイズ調整
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin[BMPWIN_MAX];        ///< bmpwin
  PRINT_UTIL print_util[BMPWIN_MAX];     ///< printutil
  MONOLITH_CANCEL_ICON cancel_icon;      ///< キャンセルアイコン用
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
//
static u16 ms_get_strsize( const STRBUF * buf, GFL_FONT * font, u32 mode );

//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_Status = {
  MonolithStatusProc_Init,
  MonolithStatusProc_Main,
  MonolithStatusProc_End,
};

///タッチ範囲テーブル(キャンセルアイコン用)
static const GFL_UI_TP_HITTBL StatusTouchRect[] = {
  {
    CANCEL_POS_Y - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_Y + CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X + CANCEL_TOUCH_RANGE_HALF,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};

// メッセージID
static const s16 status_mestbl[] = {
  msg_mono_title_008,	// ミッションのきろく
  msg_mono_st_009,		// かいさいしたミッション
  msg_mono_st_010,		// さんかしたミッション
  msg_mono_st_007,		// たっせいしたミッション
  msg_mono_st_011,		// さいこうさんかしゃすう
  msg_mono_st_012,		// トップスコア
  //
  msg_mono_st_014,		// 開催した回数
  msg_mono_st_015,		// 参加した回数
  msg_mono_st_013,		// 達成した回数
  msg_mono_st_016,		// 最高参加者数
  msg_mono_st_017,		// トップスコア数
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

  if(appwk->force_finish == TRUE){
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
      appwk->common->title_curpos = MONOLITH_RET_STATUS;
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
 * @param   MONOLITH_SETUP			*setup
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
 * @param   MONOLITH_SETUP			*setup
 * @param   MONOLITH_STATUS_WORK	*msw
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_STATUS_WORK *msw)
{
  int i;
  
  //タイトル
  msw->bmpwin[BMPWIN_TITLE] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_POS_TITLE_X, BMPWIN_POS_TITLE_Y, 
    BMPWIN_POS_TITLE_SIZE_X, BMPWIN_POS_TITLE_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_TITLE] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_TITLE] );

  //開催したミッション
  msw->bmpwin[BMPWIN_OPEN] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_OPEN_X, BMPWIN_OPEN_Y, 
    BMPWIN_OPEN_SIZE_X, BMPWIN_OPEN_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_OPEN] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_OPEN] );

  //参加したミッション
  msw->bmpwin[BMPWIN_JOIN] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_JOIN_X, BMPWIN_JOIN_Y, 
    BMPWIN_JOIN_SIZE_X, BMPWIN_JOIN_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_JOIN] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_JOIN] );

  //達成したミッション
  msw->bmpwin[BMPWIN_FINISH] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_FINISH_X, BMPWIN_FINISH_Y, 
    BMPWIN_FINISH_SIZE_X, BMPWIN_FINISH_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_FINISH] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_FINISH] );

  //最高参加者数
  msw->bmpwin[BMPWIN_MEMBER] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_MEMBER_X, BMPWIN_MEMBER_Y, 
    BMPWIN_MEMBER_SIZE_X, BMPWIN_MEMBER_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_MEMBER] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_MEMBER] );

  //トップスコア
  msw->bmpwin[BMPWIN_TOPSCORE] = GFL_BMPWIN_Create(GFL_BG_FRAME0_S, 
    BMPWIN_TOPSCORE_X, BMPWIN_TOPSCORE_Y, 
    BMPWIN_TOPSCORE_SIZE_X, BMPWIN_TOPSCORE_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_TOPSCORE] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_TOPSCORE] );

  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Setup(&msw->print_util[i], msw->bmpwin[i]);
  }
  
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   MONOLITH_STATUS_WORK	*msw
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
 * 各種情報表示
 *
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_SETUP			*setup
 * @param   MONOLITH_STATUS_WORK	*msw
 */
//--------------------------------------------------------------
static void _Write_Status(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_STATUS_WORK *msw)
{
  STRBUF *strbuf, *expand_strbuf;
  const OCCUPY_INFO *occupy;
  MYSTATUS *myst = MonolithTool_GetMystatus(appwk);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(appwk->parent->gsys);
  SAVE_CONTROL_WORK *work = GAMEDATA_GetSaveControlWork( gamedata );
  FMISSION_SAVE *fsave = SAVEDATA_GetFMissionSave( work );
  u16 size;
  u32 num;
  occupy = MonolithTool_GetOccupyInfo(appwk);
  
  strbuf = GFL_STR_CreateBuffer(256, HEAPID_MONOLITH);
  expand_strbuf = GFL_STR_CreateBuffer( 256, HEAPID_MONOLITH );
  
  //タイトル
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[0], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TITLE], setup->printQue, 
    0, 4, strbuf, setup->font_handle);
  
  //-------------------
  // 開催したミッション
  //-------------------
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[1], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_OPEN], setup->printQue, 
    0, 0, strbuf, setup->font_handle);
  
  //開催したミッションの回数
  num = FMISSION_SAVE_GetMissionOpenNum( fsave );
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[6], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, num, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  size = ms_get_strsize( expand_strbuf, setup->font_handle, 1 );	// 右
//  OS_Printf("%d\n",size);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_OPEN], setup->printQue, 
    BMPWIN_OPEN_SIZE_X*8-MES_OFFSET-size, 0, expand_strbuf, setup->font_handle);
  
  //--------------------
  // 参加したミッション
  //--------------------
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[2], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_JOIN], setup->printQue, 
    0, 0, strbuf, setup->font_handle);
  
  //参加したミッションの回数
  num = FMISSION_SAVE_GetMissionEntryNum( fsave );
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[7], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, num, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  size = ms_get_strsize( expand_strbuf, setup->font_handle, 1 );	// 右
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_JOIN], setup->printQue, 
    BMPWIN_JOIN_SIZE_X*8-MES_OFFSET-size, 0, expand_strbuf, setup->font_handle);
  
  //--------------------
  // 達成したミッション
  //--------------------
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[3], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_FINISH], setup->printQue, 
    0, 0, strbuf, setup->font_handle);
  
  //達成したミッションの回数
  num = FMISSION_SAVE_GetMissionClearNum( fsave );
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[8], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, num, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  size = ms_get_strsize( expand_strbuf, setup->font_handle, 1 );	// 右
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_FINISH], setup->printQue, 
    BMPWIN_FINISH_SIZE_X*8-MES_OFFSET-size, 0, expand_strbuf, setup->font_handle);
  
  //--------------
  // 最高参加者数
  //--------------
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[4], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_MEMBER], setup->printQue, 
    0, 0, strbuf, setup->font_handle);
  
  // 参加者数
  num = FMISSION_SAVE_GetMissionMaxJoinNum( fsave );
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[9], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, num, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  size = ms_get_strsize( expand_strbuf, setup->font_handle, 1 );	// 右
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_MEMBER], setup->printQue, 
    BMPWIN_MEMBER_SIZE_X*8-MES_OFFSET-size, 0, expand_strbuf, setup->font_handle);
  
  //--------------
  // トップスコア
  //--------------
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[5], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TOPSCORE], setup->printQue, 
    0, 0, strbuf, setup->font_handle);
  
  // トップスコア数
  num = FMISSION_SAVE_GetMissionTopScoreNum( fsave );
  GFL_MSG_GetString(setup->mm_monolith, status_mestbl[10], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, num, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  size = ms_get_strsize( expand_strbuf, setup->font_handle, 1 );	// 右
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TOPSCORE], setup->printQue, 
    BMPWIN_TOPSCORE_SIZE_X*8-MES_OFFSET-size, 0, expand_strbuf, setup->font_handle);
  
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
 * @param   MONOLITH_STATUS_WORK	*msw
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
 * @param   MONOLITH_STATUS_WORK	*msw
 */
//==================================================================
static void _Status_CancelIconUpdate(MONOLITH_STATUS_WORK *msw)
{
  MonolithTool_CancelIcon_Update(&msw->cancel_icon);
}

//--------------------------------------------------------------------------------------------
/**
 * 文字列表示サイズから各モードの補正値を取得
 * (ui/printtoolから持ってきた)
 *
 * @param   const STRBUF	*buf(文字列)
 * @param   GFL_FONT		*font(フォント)
 * @param	u32				mode(表示モード(0-2))
 *
 * @return	補正値
 */
//--------------------------------------------------------------------------------------------
static u16 ms_get_strsize( const STRBUF * buf, GFL_FONT * font, u32 mode )
{
  // (0:左、1:右、2:中央)
  if( mode == 1 ){			// 右(サイズを取得できる)
    return PRINTSYS_GetStrWidth( buf, font, 0 );
  }else if( mode == 2 ){	// 中央(サイズの半分)
    return ( PRINTSYS_GetStrWidth( buf, font, 0 ) / 2 );
  }
  return 0;
}

