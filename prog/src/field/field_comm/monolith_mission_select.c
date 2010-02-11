//==============================================================================
/**
 * @file    monolith_mission_select.c
 * @brief   モノリス：ミッション受ける
 * @author  matsuda
 * @date    2009.11.25(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "intrude_mission.h"
#include "intrude_comm_command.h"
#include "msg/msg_monolith.h"
#include "monolith.naix"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"


//==============================================================================
//  定数定義
//==============================================================================
///パネル「受ける」Y座標
#define PANEL_RECEIVE_Y   (0x12*8)

///パネルアクターID
enum{
  _PANEL_ORDER,           ///<ミッションを受ける
  _PANEL_ENFORCEMENT,     ///<実施中
  
  _PANEL_MAX,
};

///表示モード
enum{
  VIEW_ORDER,           ///<ミッション受注画面
  VIEW_ENFORCEMENT,     ///<ミッション実施中画面
};

///BMP枠のパレット番号
#define BMPWIN_FRAME_PALNO        (MONOLITH_BG_DOWN_FONT_PALNO - 1)
///BMP枠のキャラクタ開始位置
#define BMPWIN_FRAME_START_CGX    (1)

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin;
  STRBUF *strbuf_stream;
  PRINT_STREAM *print_stream;
  GFL_CLWK *act_town[INTRUDE_TOWN_MAX]; ///<街アイコンアクターへのポインタ
  GFL_CLWK *act_cancel;   ///<キャンセルアイコンアクターへのポインタ
  PANEL_ACTOR panel[_PANEL_MAX];
  u8 no_select;           ///<ミッション受注済み。選択はもう出来ない
  u8 order_end;           ///<TRUE:ミッションを受注して終了
}MONOLITH_MSSELECT_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithMissionSelectProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithMissionSelectProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithMissionSelectProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_SETUP *setup);
static void _TownIcon_AllCreate(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_APP_PARENT *appwk);
static void _TownIcon_AllDelete(MONOLITH_MSSELECT_WORK *mmw);
static void _TownIcon_AllUpdate(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_APP_PARENT *appwk);
static void _Msselect_PanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw);
static void _Msselect_PanelDelete(MONOLITH_MSSELECT_WORK *mmw);
static void _Msselect_PanelUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw);
static void _Msselect_CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw);
static void _Msselect_CancelIconDelete(MONOLITH_MSSELECT_WORK *mmw);
static void _Msselect_CancelIconUpdate(MONOLITH_MSSELECT_WORK *mmw);
static void _Msselect_ViewChange(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw, int view_mode);
static void _Setup_BmpWinCreate(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_SETUP *setup);
static void _Setup_BmpWinDelete(MONOLITH_MSSELECT_WORK *mmw);
static void _Set_MsgStream(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_SETUP *setup, u16 msg_id);
static BOOL _Wait_MsgStream(MONOLITH_MSSELECT_WORK *mmw);
static void _Clear_MsgStream(MONOLITH_MSSELECT_WORK *mmw);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_MissionSelect = {
  MonolithMissionSelectProc_Init,
  MonolithMissionSelectProc_Main,
  MonolithMissionSelectProc_End,
};

//--------------------------------------------------------------
//  街アイコン
//--------------------------------------------------------------
///街アイコン表示座標
enum{
  TOWN_0_X = 11*8,
  TOWN_0_Y = 3*8,
  TOWN_1_X = 16*8,
  TOWN_1_Y = 3*8,
  TOWN_2_X = 21*8,
  TOWN_2_Y = 3*8,
  
  TOWN_3_X = 8*8,
  TOWN_3_Y = 7*8,
  TOWN_4_X = 0x18*8,
  TOWN_4_Y = 7*8,
  
  TOWN_5_X = 11*8,
  TOWN_5_Y = 11*8,
  TOWN_6_X = 16*8,
  TOWN_6_Y = 11*8,
  TOWN_7_X = 21*8,
  TOWN_7_Y = 11*8,
};

///街アイコン表示座標テーブル
static const GFL_POINT TownIconPosTbl[] = {
  {TOWN_0_X, TOWN_0_Y},
  {TOWN_1_X, TOWN_1_Y},
  {TOWN_2_X, TOWN_2_Y},
  
  {TOWN_3_X, TOWN_3_Y},
  {TOWN_4_X, TOWN_4_Y},
  
  {TOWN_5_X, TOWN_5_Y},
  {TOWN_6_X, TOWN_6_Y},
  {TOWN_7_X, TOWN_7_Y},
};
SDK_COMPILER_ASSERT(NELEMS(TownIconPosTbl) == INTRUDE_TOWN_MAX);

///街アイコンタッチ範囲：半径
#define TOWN_TOUCH_RANGE  (8)

///街アイコンタッチ範囲テーブル
static const GFL_UI_TP_HITTBL TownTouchRect[] = {
  {//TOUCH_TOWN0
    TOWN_0_Y - TOWN_TOUCH_RANGE,
    TOWN_0_Y + TOWN_TOUCH_RANGE,
    TOWN_0_X - TOWN_TOUCH_RANGE,
    TOWN_0_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN1
    TOWN_1_Y - TOWN_TOUCH_RANGE,
    TOWN_1_Y + TOWN_TOUCH_RANGE,
    TOWN_1_X - TOWN_TOUCH_RANGE,
    TOWN_1_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN2
    TOWN_2_Y - TOWN_TOUCH_RANGE,
    TOWN_2_Y + TOWN_TOUCH_RANGE,
    TOWN_2_X - TOWN_TOUCH_RANGE,
    TOWN_2_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN3
    TOWN_3_Y - TOWN_TOUCH_RANGE,
    TOWN_3_Y + TOWN_TOUCH_RANGE,
    TOWN_3_X - TOWN_TOUCH_RANGE,
    TOWN_3_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN4
    TOWN_4_Y - TOWN_TOUCH_RANGE,
    TOWN_4_Y + TOWN_TOUCH_RANGE,
    TOWN_4_X - TOWN_TOUCH_RANGE,
    TOWN_4_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN5
    TOWN_5_Y - TOWN_TOUCH_RANGE,
    TOWN_5_Y + TOWN_TOUCH_RANGE,
    TOWN_5_X - TOWN_TOUCH_RANGE,
    TOWN_5_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN6
    TOWN_6_Y - TOWN_TOUCH_RANGE,
    TOWN_6_Y + TOWN_TOUCH_RANGE,
    TOWN_6_X - TOWN_TOUCH_RANGE,
    TOWN_6_X + TOWN_TOUCH_RANGE,
  },
  {//TOUCH_TOWN7
    TOWN_7_Y - TOWN_TOUCH_RANGE,
    TOWN_7_Y + TOWN_TOUCH_RANGE,
    TOWN_7_X - TOWN_TOUCH_RANGE,
    TOWN_7_X + TOWN_TOUCH_RANGE,
  },
  {////TOUCH_RECEIVE パネル：ミッションを受ける
    PANEL_RECEIVE_Y - PANEL_CHARSIZE_Y/2*8,
    PANEL_RECEIVE_Y + PANEL_CHARSIZE_Y/2*8,
    PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8,
    PANEL_POS_X + PANEL_SMALL_CHARSIZE_X/2*8,
  },
  {////TOUCH_CANCEL キャンセルアイコン
    CANCEL_POS_Y - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_Y + CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X + CANCEL_TOUCH_RANGE_HALF,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};
SDK_COMPILER_ASSERT(NELEMS(TownTouchRect) == INTRUDE_TOWN_MAX + 3);

enum{
  TOUCH_TOWN0,
  TOUCH_TOWN1,
  TOUCH_TOWN2,
  TOUCH_TOWN3,
  TOUCH_TOWN4,
  TOUCH_TOWN5,
  TOUCH_TOWN6,
  TOUCH_TOWN7,
  TOUCH_RECEIVE,
  TOUCH_CANCEL,
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
static GFL_PROC_RESULT MonolithMissionSelectProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_MSSELECT_WORK *mmw = mywk;
	ARCHANDLE *hdl;
  
  mmw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_MSSELECT_WORK), HEAPID_MONOLITH);
  GFL_STD_MemClear(mmw, sizeof(MONOLITH_MSSELECT_WORK));
  
  //BG
  _Setup_BGFrameSetting();
  _Setup_BGGraphicLoad(appwk->setup);
  _Setup_BmpWinCreate(mmw, appwk->setup);
  //OBJ
  _TownIcon_AllCreate(mmw, appwk);
  _Msselect_PanelCreate(appwk, mmw);
  _Msselect_CancelIconCreate(appwk, mmw);

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
static GFL_PROC_RESULT MonolithMissionSelectProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_MSSELECT_WORK *mmw = mywk;
  enum{
    SEQ_INIT,
    SEQ_TOP,
    SEQ_NO_SELECT,
    SEQ_ORDER,
    SEQ_ORDER_WAIT,
    SEQ_ORDER_OK_STREAM_WAIT,
    SEQ_ORDER_NG_STREAM_WAIT,
    SEQ_FINISH,
  };
  
  _TownIcon_AllUpdate(mmw, appwk);
  _Msselect_PanelUpdate(appwk, mmw);
  _Msselect_CancelIconUpdate(mmw);

  if(appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  switch(*seq){
  case SEQ_INIT:
    if(MISSION_RecvCheck(&appwk->parent->intcomm->mission) == TRUE){  //ミッション受注済み
      _Msselect_ViewChange(appwk, mmw, VIEW_ENFORCEMENT);
      *seq = SEQ_NO_SELECT;
    }
    else{
      *seq = SEQ_TOP;
    }
    break;
    
  case SEQ_TOP:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int tp_ret = GFL_UI_TP_HitTrg(TownTouchRect);

      if(MISSION_RecvCheck(&appwk->parent->intcomm->mission) == TRUE){
        OS_TPrintf("誰かがミッションを受注した\n");
        _Msselect_ViewChange(appwk, mmw, VIEW_ENFORCEMENT);
        mmw->no_select = TRUE;
        *seq = SEQ_NO_SELECT;
        break;
      }

      if(tp_ret >= TOUCH_TOWN0 && tp_ret < TOUCH_TOWN0 + INTRUDE_TOWN_MAX){
        OS_TPrintf("街選択 %d\n", tp_ret);
        appwk->common->mission_select_town = tp_ret;
      }
      else if(tp_ret == TOUCH_RECEIVE || (trg & PAD_BUTTON_DECIDE)){
        OS_TPrintf("「受ける」選択\n");
        MonolithTool_PanelOBJ_Flash(appwk, &mmw->panel[_PANEL_ORDER], 1, 0, FADE_SUB_OBJ);
        *seq = SEQ_ORDER;
      }
      else if(tp_ret == TOUCH_CANCEL || (trg & PAD_BUTTON_CANCEL)){
        OS_TPrintf("キャンセル選択\n");
        (*seq) = SEQ_FINISH;
      }
    }
    break;

  case SEQ_NO_SELECT:   //既に受注済み
    {
      int trg = GFL_UI_KEY_GetTrg();
      int tp_ret = GFL_UI_TP_HitTrg(TownTouchRect);

      if(tp_ret == TOUCH_CANCEL || (trg & PAD_BUTTON_CANCEL)){
        OS_TPrintf("キャンセル選択\n");
        (*seq) = SEQ_FINISH;
      }
    }
    break;

  case SEQ_ORDER:   //親機に「ミッション受注確認」を送信
    {
      const MISSION_DATA *mdata;

      mdata = &appwk->parent->list.md[TownNo_to_Type[appwk->common->mission_select_town]];
      if(IntrudeSend_MissionOrderConfirm(appwk->parent->intcomm, mdata) == TRUE){
        *seq = SEQ_ORDER_WAIT;
      }
    }
    break;
  case SEQ_ORDER_WAIT:    //親機からの返事を待つ
    {
      MISSION_ENTRY_RESULT result;
      
      result = MISSION_GetRecvEntryAnswer(&appwk->parent->intcomm->mission);
      if(result == MISSION_ENTRY_RESULT_OK){
        _Set_MsgStream(mmw, appwk->setup, msg_mono_mis_002);
        *seq = SEQ_ORDER_OK_STREAM_WAIT;
      }
      else if(result == MISSION_ENTRY_RESULT_NG){
        _Set_MsgStream(mmw, appwk->setup, msg_mono_mis_002);
        *seq = SEQ_ORDER_NG_STREAM_WAIT;
      }
    }
    break;
  case SEQ_ORDER_OK_STREAM_WAIT:
  case SEQ_ORDER_NG_STREAM_WAIT:
    if(_Wait_MsgStream(mmw) == TRUE){
      if(GFL_UI_TP_GetTrg() || (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
        _Clear_MsgStream(mmw);
        if((*seq) == SEQ_ORDER_OK_STREAM_WAIT){
          MISSION_SetMissionEntry(&appwk->parent->intcomm->mission);  //受注者の為、参加セット
          mmw->order_end = TRUE;
          *seq = SEQ_FINISH;
        }
        else{
          *seq = SEQ_TOP;
        }
      }
    }
    break;

  case SEQ_FINISH:
    if(MonolithTool_PanelColor_GetMode(appwk) != PANEL_COLORMODE_FLASH){
      if(mmw->order_end == TRUE){
        appwk->next_menu_index = MONOLITH_MENU_END;
      }
      else{
        appwk->next_menu_index = MONOLITH_MENU_TITLE;
      }
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
static GFL_PROC_RESULT MonolithMissionSelectProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_MSSELECT_WORK *mmw = mywk;
  
  if(mmw->print_stream != NULL){
    PRINTSYS_PrintStreamDelete(mmw->print_stream);
  }
  if(mmw->strbuf_stream != NULL){
    GFL_STR_DeleteBuffer(mmw->strbuf_stream);
  }

  //OBJ
  _Msselect_CancelIconDelete(mmw);
  _Msselect_PanelDelete(mmw);
  _TownIcon_AllDelete(mmw);
  //BG
  _Setup_BmpWinDelete(mmw);
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
		{//GFL_BG_FRAME1_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		},
		{//GFL_BG_FRAME2_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME1_S,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_S,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );

	GFL_BG_FillScreen( GFL_BG_FRAME1_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME2_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
	GFL_BG_SetVisible(GFL_BG_FRAME2_S, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 */
//--------------------------------------------------------------
static void _Setup_BGFrameExit(void)
{
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
	GFL_BG_SetVisible(GFL_BG_FRAME2_S, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
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
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgd_map_lz_NSCR, 
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_MONOLITH);

	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   mmw		
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_BmpWinCreate(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_SETUP *setup)
{
  GFL_BMP_DATA *bmp;

  BmpWinFrame_CgxSet( GFL_BG_FRAME1_S, BMPWIN_FRAME_START_CGX, MENU_TYPE_SYSTEM, HEAPID_MONOLITH );
  PaletteWorkSetEx_Arc(setup->pfd, ARCID_FLDMAP_WINFRAME, BmpWinFrame_WinPalArcGet(), 
    HEAPID_MONOLITH, FADE_SUB_BG, 0x20, BMPWIN_FRAME_PALNO * 16, 0);
  
	mmw->bmpwin = GFL_BMPWIN_Create( GFL_BG_FRAME1_S,
		1,19,30,4, MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B );

	bmp = GFL_BMPWIN_GetBmp( mmw->bmpwin );
	
	GFL_BMP_Clear( bmp, 0xff );
//	GFL_BMPWIN_TransVramCharacter( mmw->bmpwin );
	GFL_BMPWIN_MakeScreen( mmw->bmpwin );
//	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
	
	BmpWinFrame_Write( mmw->bmpwin, WINDOW_TRANS_ON, BMPWIN_FRAME_START_CGX, BMPWIN_FRAME_PALNO );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   mmw		
 */
//--------------------------------------------------------------
static void _Setup_BmpWinDelete(MONOLITH_MSSELECT_WORK *mmw)
{
  GFL_BMPWIN_Delete(mmw->bmpwin);
}

//--------------------------------------------------------------
/**
 * メッセージ出力開始
 *
 * @param   mmw		
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Set_MsgStream(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_SETUP *setup, u16 msg_id)
{
  GF_ASSERT(mmw->print_stream == NULL);
  
  GFL_FONTSYS_SetColor( 1, 2, 15 );

  mmw->strbuf_stream = GFL_MSG_CreateString(setup->mm_monolith, msg_id);
  
  mmw->print_stream = PRINTSYS_PrintStream(
    mmw->bmpwin, 0, 0, mmw->strbuf_stream, setup->font_handle,
    MSGSPEED_GetWait(), setup->tcbl_sys, 10, HEAPID_MONOLITH, 0xf);

	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * メッセージ出力完了待ち
 *
 * @param   mmw		
 *
 * @retval  BOOL		TRUE:出力完了
 */
//--------------------------------------------------------------
static BOOL _Wait_MsgStream(MONOLITH_MSSELECT_WORK *mmw)
{
  if(PRINTSYS_PrintStreamGetState( mmw->print_stream ) == PRINTSTREAM_STATE_DONE ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * メッセージクリア
 *
 * @param   mmw		
 */
//--------------------------------------------------------------
static void _Clear_MsgStream(MONOLITH_MSSELECT_WORK *mmw)
{
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( mmw->bmpwin );

  GF_ASSERT(mmw->print_stream != NULL);
  PRINTSYS_PrintStreamDelete(mmw->print_stream);
  mmw->print_stream = NULL;
  
  GFL_STR_DeleteBuffer(mmw->strbuf_stream);
  mmw->strbuf_stream = NULL;
  
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
  GFL_BMP_Clear(bmp, 0xff);
	GFL_BMPWIN_TransVramCharacter( mmw->bmpwin );

  GFL_FONTSYS_SetColor( 
    MONOLITH_FONT_DEFCOLOR_LETTER, MONOLITH_FONT_DEFCOLOR_SHADOW, MONOLITH_FONT_DEFCOLOR_BACK );
}

//--------------------------------------------------------------
/**
 * @brief   街アイコンアクター生成
 */
//--------------------------------------------------------------
static void _TownIcon_AllCreate(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_APP_PARENT *appwk)
{
  int i;
  OCCUPY_INFO *occupy;
  
  occupy = MonolithTool_GetOccupyInfo(appwk);
  
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    mmw->act_town[i] = MonolithTool_TownIcon_Create(
      appwk->setup, occupy, i, &TownIconPosTbl[i], COMMON_RESOURCE_INDEX_UP);
  }
}

//--------------------------------------------------------------
/**
 * @brief   街アイコンアクター削除
 */
//--------------------------------------------------------------
static void _TownIcon_AllDelete(MONOLITH_MSSELECT_WORK *mmw)
{
  int i;
  
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    MonolithTool_TownIcon_Delete(mmw->act_town[i]);
  }
}

//--------------------------------------------------------------
/**
 * 街アイコン全更新
 *
 * @param   mmw		
 */
//--------------------------------------------------------------
static void _TownIcon_AllUpdate(MONOLITH_MSSELECT_WORK *mmw, MONOLITH_APP_PARENT *appwk)
{
  int i;
  OCCUPY_INFO *occupy;
  
  occupy = MonolithTool_GetOccupyInfo(appwk);
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    MonolithTool_TownIcon_Update(mmw->act_town[i], occupy, i);
  }
}

//==================================================================
/**
 * パネル生成
 *
 * @param   appwk		
 * @param   mmw		
 */
//==================================================================
static void _Msselect_PanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw)
{
  MonolithTool_Panel_Init(appwk);
  
  MonolithTool_PanelOBJ_Create(appwk->setup, &mmw->panel[_PANEL_ORDER], 
    COMMON_RESOURCE_INDEX_DOWN, PANEL_SIZE_SMALL, PANEL_RECEIVE_Y, msg_mono_mis_000, NULL);
  MonolithTool_PanelOBJ_Focus(appwk, &mmw->panel[_PANEL_ORDER], 1, 0, FADE_SUB_OBJ);

  MonolithTool_PanelOBJ_Create(appwk->setup, &mmw->panel[_PANEL_ENFORCEMENT], 
    COMMON_RESOURCE_INDEX_DOWN, PANEL_SIZE_SMALL, PANEL_RECEIVE_Y, msg_mono_mis_001, NULL);
  MonolithTool_PanelOBJ_Focus(appwk, &mmw->panel[_PANEL_ENFORCEMENT], 1, 0, FADE_SUB_OBJ);
  MonolithTool_PanelOBJ_SetEnable(&mmw->panel[_PANEL_ENFORCEMENT], FALSE);
}

//==================================================================
/**
 * パネル削除
 *
 * @param   mmw		
 */
//==================================================================
static void _Msselect_PanelDelete(MONOLITH_MSSELECT_WORK *mmw)
{
  int i;
  
  for(i = 0; i < _PANEL_MAX; i++){
    MonolithTool_PanelOBJ_Delete(&mmw->panel[i]);
  }
}

//==================================================================
/**
 * パネル更新処理
 *
 * @param   mmw		
 */
//==================================================================
static void _Msselect_PanelUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw)
{
  int i;
  
  MonolithTool_Panel_ColorUpdate(appwk, FADE_SUB_OBJ);
  for(i = 0; i < _PANEL_MAX; i++){
    MonolithTool_PanelOBJ_TransUpdate(appwk->setup, &mmw->panel[i]);
  }
}

//==================================================================
/**
 * キャンセルアイコン生成
 *
 * @param   appwk		
 * @param   mmw		
 */
//==================================================================
static void _Msselect_CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw)
{
  mmw->act_cancel = MonolithTool_CancelIcon_Create(appwk->setup);
}

//==================================================================
/**
 * キャンセルアイコン削除
 *
 * @param   mmw		
 */
//==================================================================
static void _Msselect_CancelIconDelete(MONOLITH_MSSELECT_WORK *mmw)
{
  MonolithTool_CancelIcon_Delete(mmw->act_cancel);
}

//==================================================================
/**
 * キャンセルアイコン更新処理
 *
 * @param   mmw		
 */
//==================================================================
static void _Msselect_CancelIconUpdate(MONOLITH_MSSELECT_WORK *mmw)
{
  MonolithTool_CancelIcon_Update(mmw->act_cancel);
}

//--------------------------------------------------------------
/**
 * 表示モード変更
 *
 * @param   appwk		
 * @param   mmw		
 * @param   view_mode		VIEW_???
 */
//--------------------------------------------------------------
static void _Msselect_ViewChange(MONOLITH_APP_PARENT *appwk, MONOLITH_MSSELECT_WORK *mmw, int view_mode)
{
  if(view_mode == VIEW_ORDER){
    MonolithTool_PanelOBJ_SetEnable(&mmw->panel[_PANEL_ORDER], TRUE);
    MonolithTool_PanelOBJ_SetEnable(&mmw->panel[_PANEL_ENFORCEMENT], FALSE);
  }
  else{
    appwk->common->mission_select_town = SELECT_TOWN_ENFORCEMENT;
    MonolithTool_PanelOBJ_SetEnable(&mmw->panel[_PANEL_ORDER], FALSE);
    MonolithTool_PanelOBJ_SetEnable(&mmw->panel[_PANEL_ENFORCEMENT], TRUE);
  }
}
