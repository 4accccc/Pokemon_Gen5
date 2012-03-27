//==============================================================================
/**
 * @file    monolith_title.c
 * @brief   モノリス：タイトル
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
#include "msg/msg_monolith.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"
#include "monolith_snd_def.h"
#include "field/eventwork.h"
#include "../../../../resource/fldmapdata/flagwork/flag_define.h" //SYS_FLAG_
//
#include "field/script_def.h"
#include "../../../../resource/fldmapdata/flagwork/work_define.h" //WK_SYS_FES_TUTORIAL

//==============================================================================
//  定数定義
//==============================================================================
/// カーソル位置と戻り先を合わせる
enum{
  TITLE_PANEL_FMISSION,	// フェスミッション
  TITLE_PANEL_POWER,	// Gパワーをうけとる
  TITLE_PANEL_STATUS,	// 記録を見る
  
  TITLE_PANEL_MAX,
  TITLE_PANEL_CANCEL = TITLE_PANEL_MAX,
};

///BMP枠のパレット番号
#define BMPWIN_FRAME_PALNO        (MONOLITH_MENUBAR_PALNO - 1)
///BMP枠のキャラクタ開始位置
#define BMPWIN_FRAME_START_CGX    (1)


// 情報いろいろ(整理)
enum{
  TPANEL_X   = 128,		// パネルX座標(monolith_tool.hのPANEL_POS_Xと同じ)
  //
  TPANEL_Y0  =  44,		// パネルY座標
  TPANEL_Y1  =  84,
  TPANEL_Y2  = 124,
  //
  TPANEL_MAX = 3,		// パネル最大数
};

// モード(Mode)(フラグで分岐)
enum{
  MODE_NORMAL = 0,		// 通常
  MODE_TUTORIAL,		// チュートリアル
  MODE_GPOWER,			// チュートリアルGPOWER
};

// 戻り
enum{
  RET_NORMAL = 0,		// 通常
  RET_DECIDE,			// 決定
  RET_DISPMES,			// メッセージ表示(決定不可)
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  PANEL_ACTOR panel[TITLE_PANEL_MAX];
  s8 cursor_pos;      ///<カーソル位置
  s8 select_panel;    ///<選択したパネル(TITLE_PANEL_xxx)
  u8 title_mode;      ///<タイトルの表示モード(TITLE_MODE_xxx)
  u8 no_focus;        ///<TRUE:カーソルが何もフォーカスしていない状態
  s32   Mode;         ///<モード(0:通常、1:チュートリアル、2:GPOWERチュートリアル)
  MONOLITH_CANCEL_ICON cancel_icon;   ///<キャンセルアイコンアクターへのポインタ
  GFL_BMPWIN *bmpwin;           ///<bmpwin
  PRINT_STREAM *print_stream;   ///<stream(未使用)
  STRBUF *strbuf_stream;        ///<文字列(未使用)
}MONOLITH_TITLE_WORK;

// objのパラメータ
typedef struct{
  s16  index;		///< index(-1:終了)
  s16  x;			///< x
  s16  y;			///< y
  s16  msgid;		///< msgid
}MONO_TITLE_OBJ_DATA;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithTitleProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithTitleProc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithTitleProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Title_PanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw);
static void _Title_PanelDelete(MONOLITH_TITLE_WORK *mtw);
static void _Title_PanelUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw);
static void _Title_CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw);
static void _Title_CancelIconDelete(MONOLITH_TITLE_WORK *mtw);
static void _Title_CancelIconUpdate(MONOLITH_TITLE_WORK *mtw);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BmpWinCreate(MONOLITH_TITLE_WORK *mtw, MONOLITH_SETUP *setup);
static void _Setup_BmpWinDelete(MONOLITH_TITLE_WORK *mtw);
static void _Set_MsgStream(MONOLITH_TITLE_WORK *mtw, MONOLITH_SETUP *setup, u16 msg_id);
static BOOL _Wait_MsgStream(MONOLITH_SETUP *setup, MONOLITH_TITLE_WORK *mtw);
static void _Clear_MsgStream(MONOLITH_TITLE_WORK *mtw);

//
static s32  mt_chk_input( MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw );
static void mt_init( MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw );

//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_Title = {
  MonolithTitleProc_Init,
  MonolithTitleProc_Main,
  MonolithTitleProc_End,
};

//-----------------
// objデータまとめ
//-----------------
static const MONO_TITLE_OBJ_DATA mtitle_obj_tbl[] = {
  { PANEL_SIZE_SMALL, TPANEL_X, TPANEL_Y0, msg_mono_title_005 },	// フェスミッション
  { PANEL_SIZE_SMALL, TPANEL_X, TPANEL_Y1, msg_mono_title_002 },	// Gパワーをうけとる
  { PANEL_SIZE_SMALL, TPANEL_X, TPANEL_Y2, msg_mono_title_007 },	// 記録を見る
  //
  { -1, 0, 0, 0 },	// end
};

//-----------------
// あたり領域まとめ
// (サイズ分補正)
//-----------------
static const GFL_UI_TP_HITTBL title_rect_tbl[] = {
  { 44-8, 44+8, 128-72, 128+72, },		// フェスミッション
  { 84-8, 84+8, 128-72, 128+72, },		// Gパワーをうけとる
  {124-8,124+8, 128-72, 128+72, },		// 記録を見る
  { CANCEL_POS_Y - 8, CANCEL_POS_Y + 8, CANCEL_POS_X - 8, CANCEL_POS_X + 8, }, // キャンセル
  //
  { GFL_UI_TP_HIT_END, 0, 0, 0 },		// end
};

//--------------------------
// カーソル選択の戻り先
// (monolith_common.hで定義)
//--------------------------
static const s16 title_ret_mode[] = {
  MONOLITH_MENU_FMISSION,   ///<フェスミッション
  MONOLITH_MENU_POWER,      ///<Gパワーをうけとる
  MONOLITH_MENU_STATUS,     ///<記録を見る
  //
  MONOLITH_MENU_END,        ///<終了
};

//----------------------------------
// フラグ処理(0:押せない、1:押せる)
// (フェス、デルパワー、記録を見る、戻る)
//----------------------------------
static const s16 title_sel_tbl[] = {
  1, 1, 1, 1,		// 通常時
  1, 0, 0, 1,		// チュートリアル時
  0, 1, 0, 1,		// デルパワーチュートリアル時
};

//----------------------------------
// メッセージ
//----------------------------------
static const s16 title_mes_tbl[] = {
  msg_mono_title_info_001,		// 通常(dummy)
  msg_mono_title_info_001,		// フェスミッション
  msg_mono_title_info_002,		// デルパワー
  msg_mono_title_info_002,		// pad(dummy)
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
static GFL_PROC_RESULT MonolithTitleProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_TITLE_WORK *mtw = mywk;
	ARCHANDLE *hdl;
	GAMEDATA *gamedata = GAMESYSTEM_GetGameData(appwk->parent->gsys);
  
  mtw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_TITLE_WORK), HEAPID_MONOLITH);
  GFL_STD_MemClear(mtw, sizeof(MONOLITH_TITLE_WORK));
  
  _Setup_BGFrameSetting();
  _Setup_BmpWinCreate(mtw, appwk->setup);

  // 初期化
  mt_init( appwk, mtw );

  _Title_PanelCreate(appwk, mtw);
  _Title_CancelIconCreate(appwk, mtw);
  
	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
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
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(appwk->parent->gsys);
  int tp_ret;
  s32 ret;
  
  enum{
    _SEQ_MAIN,
    _SEQ_DECIDE,
    _SEQ_DECIDE_WAIT,
    _SEQ_STREAM_WAIT,
  };
  
  _Title_PanelUpdate(appwk, mtw);
  _Title_CancelIconUpdate(mtw);

  if(appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }

  switch(*seq){
  // 入力処理
  case _SEQ_MAIN:
    // 入力処理をまとめておく
    ret = mt_chk_input( appwk, mtw );
    // 決定
    if(ret == RET_DECIDE){
      (*seq)++;
    }
#if 0
    // メッセージ表示(決定不可)
    else if(ret == RET_DISPMES){
      _Set_MsgStream( mtw, appwk->setup, title_mes_tbl[mtw->Mode] );
      *seq = _SEQ_STREAM_WAIT;
    }
#endif
    break;
  // 決定時
  case _SEQ_DECIDE:
    if(mtw->select_panel != TITLE_PANEL_CANCEL){
      MonolithTool_PanelOBJ_Flash(appwk, mtw->panel, 
        TPANEL_MAX, mtw->cursor_pos, FADE_SUB_OBJ);
      PMSND_PlaySE(MONOLITH_SE_DECIDE);
    }
    else{
      MonolithTool_CancelIcon_FlashReq(&mtw->cancel_icon);
      PMSND_PlaySE(MONOLITH_SE_CANCEL);
    }
    (*seq)++;
    break;
  case _SEQ_DECIDE_WAIT:
    if(mtw->select_panel != TITLE_PANEL_CANCEL){
      if(MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_OBJ) != PANEL_COLORMODE_FLASH){
        // 戻り先セット
        appwk->next_menu_index = title_ret_mode[mtw->select_panel];
        return GFL_PROC_RES_FINISH;
      }
    }
    else{
      if(MonolithTool_CancelIcon_FlashCheck(&mtw->cancel_icon) == FALSE){
        // 戻り先セット
        appwk->next_menu_index = title_ret_mode[mtw->select_panel];
        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  // メッセージ待ち
  case _SEQ_STREAM_WAIT:
    if(_Wait_MsgStream(appwk->setup, mtw) == TRUE){
      if(GFL_UI_TP_GetTrg() || (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
        _Clear_MsgStream(mtw);
        PMSND_PlaySE(MONOLITH_SE_MSG);
        *seq = _SEQ_MAIN;
      }
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

  if(PRINTSYS_QUE_IsFinished(appwk->setup->printQue) == FALSE){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_DISP_GXS_SetVisibleControlDirect(GX_PLANEMASK_BG3);

  if(mtw->print_stream != NULL){
    PRINTSYS_PrintStreamDelete(mtw->print_stream);
  }
  if(mtw->strbuf_stream != NULL){
    GFL_STR_DeleteBuffer(mtw->strbuf_stream);
  }

  _Title_PanelDelete(mtw);
  _Title_CancelIconDelete(mtw);

  _Setup_BmpWinDelete(mtw);
  _Setup_BGFrameExit();
  
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
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//==================================================================
static void _Title_PanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw)
{
  int i;
  s32 j;
  const MONO_TITLE_OBJ_DATA *adr = &mtitle_obj_tbl[0];
  STRBUF *strbuf;
  static const PRINTSYS_LSB print_color[] = {
    PRINTSYS_MACRO_LSB(2, 1, 0),		// 色変え
    PRINTSYS_MACRO_LSB(0xf, 2, 0),		// 通常
  };
  
  //mtw->Modeで判断
  
  MonolithTool_Panel_Init(appwk);
  // objのセット
  for(i=0;i<TPANEL_MAX;i++){
    // 終了判定
    if(adr->index == -1){
      break;
    }
    // 押せるか判定(0 or 1)
    j = title_sel_tbl[4*mtw->Mode+i];
    //
    // 色指定表示
    MonolithTool_PanelOBJ_Create_Color(appwk->setup, &mtw->panel[i], 
      COMMON_RESOURCE_INDEX_DOWN, adr->index, adr->x, adr->y, adr->msgid, NULL, print_color[j]);
    adr++;
  }
  //
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY ){
    MonolithTool_PanelOBJ_Focus(appwk, mtw->panel, TPANEL_MAX, mtw->cursor_pos, FADE_SUB_OBJ);
    mtw->no_focus = FALSE;
  }
  else{
    mtw->no_focus = TRUE;
  }
}

//==================================================================
/**
 * パネル削除
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//==================================================================
static void _Title_PanelDelete(MONOLITH_TITLE_WORK *mtw)
{
  int i;
  
  for(i = 0; i < TPANEL_MAX; i++){
    MonolithTool_PanelOBJ_Delete(&mtw->panel[i]);
  }
}

//==================================================================
/**
 * パネル更新処理
 *
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//==================================================================
static void _Title_PanelUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw)
{
  int i;
  
  MonolithTool_Panel_ColorUpdate(appwk, FADE_SUB_OBJ);
  for(i = 0; i < TPANEL_MAX; i++){
    MonolithTool_PanelOBJ_TransUpdate(appwk->setup, &mtw->panel[i]);
  }
}

//==================================================================
/**
 * キャンセルアイコン生成
 *
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//==================================================================
static void _Title_CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw)
{
  MonolithTool_CancelIcon_Create(appwk->setup, &mtw->cancel_icon);
}

//==================================================================
/**
 * キャンセルアイコン削除
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//==================================================================
static void _Title_CancelIconDelete(MONOLITH_TITLE_WORK *mtw)
{
  MonolithTool_CancelIcon_Delete(&mtw->cancel_icon);
}

//==================================================================
/**
 * キャンセルアイコン更新処理
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//==================================================================
static void _Title_CancelIconUpdate(MONOLITH_TITLE_WORK *mtw)
{
  MonolithTool_CancelIcon_Update(&mtw->cancel_icon);
}

//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 */
//--------------------------------------------------------------
static void _Setup_BGFrameSetting(void)
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {
		{//GFL_BG_FRAME1_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME1_S,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );

	GFL_BG_FillScreen( GFL_BG_FRAME1_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 */
//--------------------------------------------------------------
static void _Setup_BGFrameExit(void)
{
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 * @param   MONOLITH_SETUP			*setup
 */
//--------------------------------------------------------------
static void _Setup_BmpWinCreate(MONOLITH_TITLE_WORK *mtw, MONOLITH_SETUP *setup)
{
  GFL_BMP_DATA *bmp;

  BmpWinFrame_CgxSet( GFL_BG_FRAME1_S, BMPWIN_FRAME_START_CGX, MENU_TYPE_SYSTEM, HEAPID_MONOLITH );
  PaletteWorkSetEx_Arc(setup->pfd, ARCID_FLDMAP_WINFRAME, 
    BmpWinFrame_WinPalArcGet(MENU_TYPE_SYSTEM), 
    HEAPID_MONOLITH, FADE_SUB_BG, 0x20, BMPWIN_FRAME_PALNO * 16, 0);
  
	mtw->bmpwin = GFL_BMPWIN_Create( GFL_BG_FRAME1_S,
		1,19,30,4, MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B );

	bmp = GFL_BMPWIN_GetBmp( mtw->bmpwin );
	
	GFL_BMP_Clear( bmp, 0xff );
//	GFL_BMPWIN_TransVramCharacter( mtw->bmpwin );
	GFL_BMPWIN_MakeScreen( mtw->bmpwin );
//	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
	
	BmpWinFrame_Write( mtw->bmpwin, WINDOW_TRANS_ON, BMPWIN_FRAME_START_CGX, BMPWIN_FRAME_PALNO );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//--------------------------------------------------------------
static void _Setup_BmpWinDelete(MONOLITH_TITLE_WORK *mtw)
{
  GFL_BMPWIN_Delete(mtw->bmpwin);
}

//--------------------------------------------------------------
/**
 * メッセージ出力開始(stream)
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 * @param   MONOLITH_SETUP			*setup
 * @param   u16						msg_id
 */
//--------------------------------------------------------------
static void _Set_MsgStream(MONOLITH_TITLE_WORK *mtw, MONOLITH_SETUP *setup, u16 msg_id)
{
  GF_ASSERT_HEAVY(mtw->print_stream == NULL);
  
  GFL_FONTSYS_SetColor( 1, 2, 15 );

  mtw->strbuf_stream = GFL_MSG_CreateString(setup->mm_monolith, msg_id);
  
  mtw->print_stream = PRINTSYS_PrintStream(
    mtw->bmpwin, 0, 0, mtw->strbuf_stream, setup->font_handle,
    MSGSPEED_GetWait(), setup->tcbl_sys, 10, HEAPID_MONOLITH, 0xf);

	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * メッセージ出力完了待ち(stream)
 *
 * @param   MONOLITH_SETUP			*setup
 * @param   MONOLITH_TITLE_WORK		*mtw
 *
 * @retval  BOOL					(TRUE:出力完了、FALSE:まだ)
 */
//--------------------------------------------------------------
static BOOL _Wait_MsgStream(MONOLITH_SETUP *setup, MONOLITH_TITLE_WORK *mtw)
{
  return APP_PRINTSYS_COMMON_PrintStreamFunc( &setup->app_printsys, mtw->print_stream );
}

//--------------------------------------------------------------
/**
 * メッセージクリア(stream)
 *
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//--------------------------------------------------------------
static void _Clear_MsgStream(MONOLITH_TITLE_WORK *mtw)
{
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( mtw->bmpwin );

  GF_ASSERT_HEAVY(mtw->print_stream != NULL);
  PRINTSYS_PrintStreamDelete(mtw->print_stream);
  mtw->print_stream = NULL;
  
  GFL_STR_DeleteBuffer(mtw->strbuf_stream);
  mtw->strbuf_stream = NULL;
  
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
  GFL_BMP_Clear(bmp, 0xff);
	GFL_BMPWIN_TransVramCharacter( mtw->bmpwin );

  GFL_FONTSYS_SetColor( 
    MONOLITH_FONT_DEFCOLOR_LETTER, MONOLITH_FONT_DEFCOLOR_SHADOW, MONOLITH_FONT_DEFCOLOR_BACK );
}

//--------------------------------------------------------------
/**
 * キー入力処理(まとめ)
 *
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_TITLE_WORK		*mtw
 *
 * @retval  s32						(0:なにもしない、1:DECIDE、2:メッセージ表示)
 */
//--------------------------------------------------------------
static s32 mt_chk_input( MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw )
{
  int tp_ret;
  int repeat;
  int trg;
  int ret=RET_NORMAL;
  
  tp_ret = GFL_UI_TP_HitTrg( title_rect_tbl );
  if(tp_ret != GFL_UI_TP_HIT_NONE){
    mtw->cursor_pos = tp_ret;
    mtw->select_panel = tp_ret;	// そのままセット
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
    mtw->no_focus = FALSE;
    if(title_sel_tbl[4*mtw->Mode+tp_ret] == 1){
      // 抜ける
      ret = RET_DECIDE;
    }else{
      // パネルの更新はしておく
      MonolithTool_PanelOBJ_Focus(appwk, mtw->panel, TPANEL_MAX, 
        mtw->cursor_pos, FADE_SUB_OBJ);
      PMSND_PlaySE(MONOLITH_SE_BEEP);
      // メッセージ表示
//      ret = RET_DISPMES;
    }
//    OS_Printf("decide tp:%d\n",tp_ret);
  }
  else{
    repeat = GFL_UI_KEY_GetRepeat();
    trg = GFL_UI_KEY_GetTrg();
    
    if(trg > 0 && mtw->no_focus == TRUE){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      mtw->no_focus = FALSE;
      MonolithTool_PanelOBJ_Focus(appwk, mtw->panel, TPANEL_MAX, 
        mtw->cursor_pos, FADE_SUB_OBJ);
      PMSND_PlaySE(MONOLITH_SE_SELECT);
    }
    else if(trg & PAD_BUTTON_DECIDE){
      if(title_sel_tbl[4*mtw->Mode+mtw->cursor_pos] == 1){
        mtw->select_panel = mtw->cursor_pos;	// そのままセット
        // 抜ける
        ret = RET_DECIDE;
      }else{
//        ret = RET_DISPMES;
        PMSND_PlaySE(MONOLITH_SE_BEEP);
      }
//      OS_Printf("decide\n");
    }
    else if(trg & PAD_BUTTON_CANCEL){
      mtw->select_panel = TITLE_PANEL_CANCEL;
      // 抜ける
      ret = RET_DECIDE;
    }
    else if(repeat & PAD_KEY_DOWN){
      mtw->cursor_pos++;
      if(mtw->cursor_pos >= TPANEL_MAX){
        mtw->cursor_pos = 0;
      }
      MonolithTool_PanelOBJ_Focus(appwk, mtw->panel, 
        TPANEL_MAX, mtw->cursor_pos, FADE_SUB_OBJ);
      PMSND_PlaySE(MONOLITH_SE_SELECT);
    }
    else if(repeat & PAD_KEY_UP){
      mtw->cursor_pos--;
      if(mtw->cursor_pos < 0){
        mtw->cursor_pos = TPANEL_MAX - 1;
      }
      MonolithTool_PanelOBJ_Focus(
        appwk, mtw->panel, TPANEL_MAX, 
        mtw->cursor_pos, FADE_SUB_OBJ);
      PMSND_PlaySE(MONOLITH_SE_SELECT);
    }
  }
  return( ret );
}


//--------------------------------------------------------------
/**
 * 初期化(主にフラグの判定)
 *
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_TITLE_WORK		*mtw
 */
//--------------------------------------------------------------
static void mt_init( MONOLITH_APP_PARENT *appwk, MONOLITH_TITLE_WORK *mtw )
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( appwk->parent->gsys );
  EVENTWORK	*ev = GAMEDATA_GetEventWork( gamedata );
  BOOL	bflg;
  u16	*adr;
  
  // チュートリアル中かどうか
  bflg = EVENTWORK_CheckEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR );
  if(bflg == FALSE){
    mtw->Mode = MODE_TUTORIAL;
  }
  // デルパワーチュートリアル中かどうか
  adr = EVENTWORK_GetEventWorkAdrs( ev,  WK_SYS_FES_TUTORIAL );
  if(*adr == SCR_FES_TUTORIAL_GPOWER_1ST_GET){
    mtw->Mode = MODE_GPOWER;
  }
  
  // 初期カーソル位置セット
  mtw->cursor_pos   = appwk->common->title_curpos;
  mtw->select_panel = mtw->cursor_pos;
  appwk->common->title_curpos = MONOLITH_RET_FMISSION;
  
  // 初期化をまとめて行う
//#ifdef PM_DEBUG
#if 0	// 未使用
  // Lを押しながら..
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){
      mtw->Mode = MODE_TUTORIAL;
    }
    else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
      mtw->Mode = MODE_GPOWER;
    }
  }
#endif
}

