//==============================================================================
/**
 * @file    monolith_fmission.c
 * @brief   モノリス：フェスミッション選択画面
 * @author  matsuda->iwasawa
 * @date    2011.02.23(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/net_err.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "intrude_mission.h"
#include "intrude_comm_command.h"
#include "msg/msg_monolith.h"
#include "msg/msg_fest_mission.h"
#include "monolith.naix"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"
#include "message.naix"
#include "gamesystem/g_power.h"
#include "app_menu_common.naix"
#include "app/app_menu_common.h"
#include "gamesystem/game_comm.h"
#include "monolith_snd_def.h"
#include "intrude_work.h"
#include "field/monolith_tool_notwifi.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_nw.h"
#include "gamesystem/fest_mission_data.h"
#include "field/field_subscreen.h"


//==============================================================================
//  定数定義
//==============================================================================
///BMP枠のパレット番号
#define BMPWIN_FRAME_PALNO        (MONOLITH_MENUBAR_PALNO - 1)
///BMP枠のキャラクタ開始位置
#define BMPWIN_FRAME_START_CGX    (1)

enum{
  _BMPWIN_TALKWIN,    ///<会話ウィンドウ
  _BMPWIN_TITLE,      ///<タイトル
  
  _BMPWIN_MAX,
};

enum{
  _PRINTUTIL_TITLE,
  
  _PRINTUTIL_MAX,
};


//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///Gパワー名用に確保するBMPのX(キャラクタ単位)
#define GPOWER_NAME_BMP_LEN_X     (23)
///Gパワー名用に確保するBMPのY(キャラクタ単位)
#define GPOWER_NAME_BMP_LEN_Y     (2)

///1フレームに描画するGパワー名の数 (キューに貯めるには数が多すぎるので自分でフレームを分けて描画)
#define GPOWER_NAME_WRITE_NUM     (3)

///一度に画面に表示されるパワー項目数
#define POWER_ITEM_DISP_NUM   (7)
///十時キーで移動させる時に、上下に余裕を持たせてスクロールさせる為のパネル数
#define POWER_ITEM_KEY_SCROLL_SPACE   (2)

enum{
  SOFTPRI_BMPOAM_NAME = 10,
};

enum{
  BGPRI_BMPOAM_HAVE_POINT = 1,
  BGPRI_BMPOAM_NAME = 3,
};

enum{
  _PANEL_LEFT_POS = 3,      ///<パネル左端X位置(キャラクタ単位)
  _PANEL_RIGHT_POS = 0x1c,  ///<パネル右端X位置(キャラクタ単位)
  _PANEL_SIZE_Y = 4,        ///<パネルYサイズ(キャラクタ単位)
};

///パワー名のリスト間の幅(ドット数)
#define POWER_LIST_SPACE    (_PANEL_SIZE_Y*8)

///BMPOAMのパワー名 X座標
#define _BMPOAM_POWER_NAME_X    (4*8)
///BMPOAMのパワー名 開始Y座標オフセット
#define _BMPOAM_POWER_NAME_OFFSET_Y    (8)

///スクロールの減速力(下位8ビット小数)
#define _SCROLL_SLOW_DOWN   (0x00c0)

enum{
  _TOUCH_DECIDE_RANGE = 4,    ///<決定チェック：最初にタッチパネルを押した状態から決定と判定する許容のずれ
  _TOUCH_DECIDE_NONE = 0xffff,  ///<決定チェック：最初のタッチからずれすぎた為、無効を示す値
};

///無効な項目No
#define _LIST_NO_NONE     (0xffff)
///無効なカーソル位置
#define _CURSOR_POS_NONE  (0xffff)

typedef enum{
  _SET_CURSOR_MODE_INIT,  ///<初期化用
  _SET_CURSOR_MODE_KEY,   ///<キーで操作
  _SET_CURSOR_MODE_TP,    ///<タッチパネルで操作
}_SET_CURSOR_MODE;

enum{
  _DUMMY_PANEL_UP_NUM = 1,    ///<上に入るダミーパネル数
  _DUMMY_PANEL_DOWN_NUM = 1,  ///<下に入るダミーパネル数
  _DUMMY_PANEL_NUM = 2,       ///<上下に入るダミーパネル数
};
///タイトルバーの直下からパネルが開始するようにする為のリストY上限値
#define _LIST_Y_TOP_OFFSET         (8 << 8)
///メニューバーの真上でパネルが停止するようにする為のリストY下限値
#define _LIST_Y_BOTTOM_OFFSET      (8 << 8)
///タイトルバーのY長
#define _TITLE_BAR_Y_LEN          (8*3)
///メニューバーのY長
#define _MENU_BAR_Y_LEN           (8*3)

///パネル「ミッション開始」X座標
#define _PANEL_DECIDE_X   (64)
///パネル「ミッション開始」Y座標
#define _PANEL_DECIDE_Y   (192 - _MENU_BAR_Y_LEN/2)

enum{
  _SCROLL_ARROW_X = 128,
  _SCROLL_ARROW_UP_Y = _TITLE_BAR_Y_LEN + 4,
  _SCROLL_ARROW_DOWN_Y = 192 - _MENU_BAR_Y_LEN - 4,
};


//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_TCB *vintr_tcb;
 
  GAMEDATA* gdata;
  LINK_FESTIVAL*  fest_sys; 
  FMISSION_DATA_MAN*  fmd_man;
  GAME_COMM_SYS_PTR gcsp;

  u32 alloc_menubar_pos;
  int wait;

  PANEL_ACTOR panel;
  GFL_CLWK *act_arrow_up;       ///<上矢印
  GFL_CLWK *act_arrow_down;     ///<下矢印
  
  GFL_MSGDATA *mm_fmission;     ///<フェスミッション名gmm
  GFL_BMP_DATA *bmp_power_name[GPOWER_ID_MAX + _DUMMY_PANEL_NUM];
  GPOWER_ID use_gpower_id[GPOWER_ID_MAX + _DUMMY_PANEL_NUM];      ///<bmp_power_nameに対応したGPOWER_IDが入っている
  MONO_USE_POWER use_power[GPOWER_ID_MAX + _DUMMY_PANEL_NUM];         ///<use_gpower_idの習得状況
  u8 power_list_num;        ///<パワーリスト数
  u8 power_list_write_num;  ///<BMPに書き込んだ名前の数
  
  GFL_BMPWIN *bmpwin[_BMPWIN_MAX];
  PRINT_UTIL print_util[_PRINTUTIL_MAX];
  STRBUF *strbuf_stream;
  PRINT_STREAM *print_stream;
  MONOLITH_CANCEL_ICON cancel_icon;   ///<キャンセルアイコンアクターへのポインタ

  BMPOAM_ACT_PTR bmpoam_power[POWER_ITEM_DISP_NUM];
  GFL_BMP_DATA *bmpoam_power_bmp[POWER_ITEM_DISP_NUM];
  
  s32 list_y;    ///<下位8ビット小数
  
  u32 tp_cont_frame;     ///<TRUE:タッチパネル押しっぱなしにしているフレーム数
  s32 speed;              ///<スクロール速度(下位8ビット小数。マイナス有)
  s32 one_speed;          ///<スクロール速度(下位8ビット小数。マイナス有) 1フレームで消滅
  u32 tp_backup_y;        ///<1フレーム前のTP_Y値
  u32 tp_first_hit_y;     ///<最初にタッチした時のY値
  
  BOOL scroll_update;     ///<TRUE:スクロール座標に整数レベルで変化があった
  
  s32 cursor_pos;         ///<カーソル位置(項目No)
  s32 backup_cursor_pos;  ///<1つ前のカーソル位置
  s32 backup_tp_decide_pos; ///<タッチパネル決定判定用の1つ前の選択していたカーソル位置
  s32 decide_cursor_pos;  ///<決定時のカーソル位置
  BOOL cursor_pos_update;   ///<TRUE:カーソル位置に更新があった
  BOOL  order_end;          ///<ミッションを受けた
  _SET_CURSOR_MODE cursor_mode; ///<カーソルをキーとタッチパネルどちらでいじったか
  
  APP_TASKMENU_RES *app_menu_res;
  APP_TASKMENU_WORK *app_menu_work;
}MONOLITH_FMSELECT_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithFMissionSelectProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithFMissionSelectProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithFMissionSelectProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_DataSetting( MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw );
static void _Setup_DataExit( MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw );
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup);
static void _Setup_BGGraphicUnload(MONOLITH_FMSELECT_WORK *mfw);
static void _BmpOamCreate(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup);
static void _BmpOamDelete(MONOLITH_FMSELECT_WORK *mfw);
static void _DecidePanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw);
static void _DecidePanelDelete(MONOLITH_FMSELECT_WORK *mfw);
static void _DecidePanelUpdate(MONOLITH_SETUP *setup, MONOLITH_FMSELECT_WORK *mfw);
static void _CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw);
static void _CancelIconDelete(MONOLITH_FMSELECT_WORK *mfw);
static void _CancelIconUpdate(MONOLITH_FMSELECT_WORK *mfw);
static void _ScrollArrowCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw);
static void _ScrollArrowDelete(MONOLITH_FMSELECT_WORK *mfw);
static void _ScrollArrowUpdate(MONOLITH_FMSELECT_WORK *mfw);
static void _Setup_BmpWinCreate(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup);
static void _Setup_BmpWinDelete(MONOLITH_FMSELECT_WORK *mfw);
static void _Set_TitlePrint(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw);
static void _Set_MsgStream(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup, u16 msg_id);
static void _Set_MsgStreamExpand(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup, u16 msg_id);
static BOOL _Wait_MsgStream(MONOLITH_SETUP *setup, MONOLITH_FMSELECT_WORK *mfw);
static void _Clear_MsgStream(MONOLITH_FMSELECT_WORK *mfw);
static void _Setup_ScreenClear(MONOLITH_FMSELECT_WORK *mfw);
static void _Setup_PowerNameBMPCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup);
static void _Setup_PowerNameBMPDelete(MONOLITH_FMSELECT_WORK *mfw);
static BOOL _MissionNameBMPDraw(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup);
static BOOL _ScrollSpeedUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw);
static void _ScrollBeforeUpdate(MONOLITH_FMSELECT_WORK *mfw);
static BOOL _SetCursorPos(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos, _SET_CURSOR_MODE mode);
static void _ScrollAfterUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw);
static BOOL _ScrollPosUpdate(MONOLITH_FMSELECT_WORK *mfw, s32 add_y);
static void _ScrollPos_BGReflection(MONOLITH_FMSELECT_WORK *mfw);
static void _ScrollPos_NameOamReflection(MONOLITH_FMSELECT_WORK *mfw);
static void _PowerSelect_VBlank(GFL_TCB *tcb, void *work);
static s32 _GetTouchListNo(MONOLITH_FMSELECT_WORK *mfw, u32 tp_y);
static void _CursorPos_PanelScreenChange(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos, int palno);
static void _CursorPos_PanelFocus(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos);
static void _CursorPos_NotFocus(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_FMissionSelect = {
  MonolithFMissionSelectProc_Init,
  MonolithFMissionSelectProc_Main,
  MonolithFMissionSelectProc_End,
};

///タッチ範囲テーブル
static const GFL_UI_TP_HITTBL TouchRect[] = {
  {////TOUCH_CANCEL キャンセルアイコン
    CANCEL_POS_Y - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_Y + CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X + CANCEL_TOUCH_RANGE_HALF,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};

enum{
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
static GFL_PROC_RESULT MonolithFMissionSelectProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_FMSELECT_WORK *mfw = mywk;
	ARCHANDLE *hdl;
  
  switch(*seq){
  case 0:
    mfw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_FMSELECT_WORK), HEAPID_MONOLITH);
    GFL_STD_MemClear(mfw, sizeof(MONOLITH_FMSELECT_WORK));
    mfw->list_y = _LIST_Y_TOP_OFFSET;
    mfw->decide_cursor_pos = _CURSOR_POS_NONE;
    mfw->backup_cursor_pos = _CURSOR_POS_NONE;
    mfw->backup_tp_decide_pos = _CURSOR_POS_NONE;

    MonolithTool_Panel_Init(appwk);
    
    //データ初期化
    _Setup_DataSetting( appwk, mfw );

    //BG
    _Setup_BGFrameSetting();
    _Setup_BGGraphicLoad(mfw, appwk->setup);
    _Setup_BmpWinCreate(mfw, appwk->setup);
    _ScrollPos_BGReflection(mfw);
    //OBJ
    _CancelIconCreate(appwk, mfw);
    _BmpOamCreate(mfw, appwk->setup);
    _DecidePanelCreate(appwk, mfw);
    _ScrollArrowCreate(appwk, mfw);

    _Set_TitlePrint(appwk, mfw);

    mfw->app_menu_res = MonolithTool_TaskMenuCreate(
      appwk->setup, GFL_BG_FRAME0_S, HEAPID_MONOLITH);

    appwk->common->power_select_no = GPOWER_ID_NULL;
    
    mfw->vintr_tcb = GFUser_VIntr_CreateTCB(
      _PowerSelect_VBlank, mfw, MONOLITH_VINTR_TCB_PRI_POWER);

    OS_TPrintf("NameDraw start\n");
    _Setup_PowerNameBMPCreate(appwk, mfw, appwk->setup);
    _Setup_ScreenClear(mfw);
    (*seq)++;
    break;
  case 1:
    if(_MissionNameBMPDraw(mfw, appwk->setup) == TRUE){
      OS_TPrintf("NameDraw end\n");
      //名前描画
      _ScrollPos_NameOamReflection(mfw);
      mfw->scroll_update = TRUE;

      if(GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY){
        _SetCursorPos(mfw, _DUMMY_PANEL_UP_NUM, _SET_CURSOR_MODE_INIT);
      }
      else{
        _SetCursorPos(mfw, _CURSOR_POS_NONE, _SET_CURSOR_MODE_INIT);
      }
      (*seq)++;
    }
    break;
  case 2:
    {
      int i;
      BOOL no_trans = FALSE;

      for(i = 0; i < _PRINTUTIL_MAX; i++){
        if(PRINT_UTIL_Trans(&mfw->print_util[i], appwk->setup->printQue) == FALSE){
          no_trans = TRUE;
        }
      }
      if(no_trans == FALSE){
      	GFL_BG_SetVisible(GFL_BG_FRAME0_S, VISIBLE_ON);
      	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_ON);
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
static GFL_PROC_RESULT MonolithFMissionSelectProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_FMSELECT_WORK *mfw = mywk;
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(appwk->parent->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  int i;
  enum{
    SEQ_INIT,
    SEQ_FIRST_STREAM,
    SEQ_FIRST_STREAM_WAIT,
    SEQ_FIRST_STREAM_TP_RELEASE,
    SEQ_TOP,
    SEQ_YESNO_STREAM,
    SEQ_YESNO_STREAM_WAIT,
    SEQ_YESNO_SELECT_WAIT,
    SEQ_DECIDE_STREAM,
    SEQ_DECIDE_STREAM_WAIT,
    SEQ_DECIDE_STREAM_KEY_WAIT,
    SEQ_TP_RELEASE_WAIT,
    SEQ_FINISH_WAIT,
    SEQ_COMM_EXIT_WAIT,
    SEQ_COMM_RESTART,
    SEQ_FINISH,
  };
  
  MonolithTool_Panel_ColorUpdate(appwk, FADE_SUB_BG);
  MonolithTool_Panel_ColorUpdate(appwk, FADE_SUB_OBJ);
  _CancelIconUpdate(mfw);
  _DecidePanelUpdate(appwk->setup, mfw);
  for(i = 0; i < _PRINTUTIL_MAX; i++){
    PRINT_UTIL_Trans(&mfw->print_util[i], appwk->setup->printQue);
  }

  if(appwk->force_finish == TRUE && (*seq) == SEQ_TOP){
    return GFL_PROC_RES_FINISH;
  }
  
  switch(*seq){
  case SEQ_INIT:
    *seq = SEQ_FIRST_STREAM;
    break;

  case SEQ_FIRST_STREAM:
    _Set_MsgStream(mfw, appwk->setup, msg_mono_fmission_001);
    (*seq)++;
    break;
  case SEQ_FIRST_STREAM_WAIT:
    if(_Wait_MsgStream(appwk->setup, mfw) == TRUE){
      if(GFL_UI_TP_GetTrg() || (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
        _Clear_MsgStream(mfw);
        PMSND_PlaySE(MONOLITH_SE_MSG);
        if(GFL_UI_TP_GetTrg() == FALSE){
          _SetCursorPos(mfw, _DUMMY_PANEL_UP_NUM, _SET_CURSOR_MODE_INIT);
        }
        else{
          _SetCursorPos(mfw, _CURSOR_POS_NONE, _SET_CURSOR_MODE_INIT);
        }
        (*seq)++;
      }
    }
    break;
  case SEQ_FIRST_STREAM_TP_RELEASE: //文字送りに使用したキーとタッチ両方を離すのを待つ
    if(GFL_UI_TP_GetCont() == FALSE 
        && (GFL_UI_KEY_GetCont() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL)) == 0){
      *seq = SEQ_TOP;
    }
    break;
    
  case SEQ_TOP:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int tp_ret = GFL_UI_TP_HitTrg(TouchRect);

      if(tp_ret == TOUCH_CANCEL || (trg & PAD_BUTTON_CANCEL)){
        OS_TPrintf("キャンセル選択\n");
        MonolithTool_CancelIcon_FlashReq(&mfw->cancel_icon);
        if(tp_ret == TOUCH_CANCEL){
          GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        }
        else{
          GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        }
        PMSND_PlaySE(MONOLITH_SE_CANCEL);
        (*seq) = SEQ_FINISH_WAIT;
        break;
      }
      
      _ScrollSpeedUpdate(appwk, mfw);
      _ScrollBeforeUpdate(mfw);
      if(_ScrollPosUpdate(mfw, mfw->speed + mfw->one_speed) == TRUE){
        _ScrollPos_BGReflection(mfw);
        _ScrollPos_NameOamReflection(mfw);
        mfw->scroll_update = TRUE;
      }
      _ScrollAfterUpdate(appwk, mfw);
    }

    if(mfw->cursor_pos == _CURSOR_POS_NONE){
      appwk->common->power_select_no = GPOWER_ID_NULL;
      MonolithTool_PanelOBJ_SetEnable(&mfw->panel, FALSE);
    }
    else{
      appwk->common->power_select_no = mfw->use_gpower_id[mfw->cursor_pos];
      appwk->common->power_mono_use = mfw->use_power[mfw->cursor_pos];
      if(mfw->use_power[mfw->cursor_pos] == MONO_USE_POWER_OK){
        MonolithTool_PanelOBJ_SetEnable(&mfw->panel, TRUE);
      }
      else{
        MonolithTool_PanelOBJ_SetEnable(&mfw->panel, FALSE);
      }
    }
    
    if(mfw->decide_cursor_pos != _CURSOR_POS_NONE){
      PMSND_PlaySE( MONOLITH_SE_DECIDE );
      *seq = SEQ_YESNO_STREAM;
    }
    break;
  
  case SEQ_YESNO_STREAM:
    if(MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_BG) != PANEL_COLORMODE_FLASH
        && MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_OBJ) != PANEL_COLORMODE_FLASH){
      _Set_MsgStream(mfw, appwk->setup, msg_mono_fmission_003);
      (*seq)++;
    }
    break;
  case SEQ_YESNO_STREAM_WAIT:
    if(_Wait_MsgStream(appwk->setup, mfw) == TRUE){
      GF_ASSERT_HEAVY(mfw->app_menu_work == NULL);
      mfw->app_menu_work = MonolithTool_TaskMenu_YesNoInit(
        appwk->setup, mfw->app_menu_res, HEAPID_MONOLITH);
      (*seq)++;
    }
    break;
  case SEQ_YESNO_SELECT_WAIT:
    {
      BOOL ret_yesno;
      if(MonolithTool_TaskMenu_Update(appwk->setup, GFL_BG_FRAME0_S, 
          mfw->app_menu_work, &ret_yesno) == TRUE){
        MonolithTool_TaskMenu_YesNoExit(mfw->app_menu_work);
        mfw->app_menu_work = NULL;
        _Clear_MsgStream(mfw);
        if(ret_yesno == TRUE){
          *seq = SEQ_DECIDE_STREAM;
        }
        else{
          mfw->decide_cursor_pos = _CURSOR_POS_NONE;
          MonolithTool_PanelBG_Focus(appwk, TRUE, FADE_SUB_BG);
          MonolithTool_PanelOBJ_Focus(appwk, &mfw->panel, 1, PANEL_NO_FOCUS, FADE_SUB_OBJ);
          *seq = SEQ_TOP;
        }
      }
    }
    break;
    
  case SEQ_DECIDE_STREAM:
    if(MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_BG) != PANEL_COLORMODE_FLASH
        && MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_OBJ) != PANEL_COLORMODE_FLASH){
#if 0
      WORDSET_RegisterGPowerName( 
        appwk->setup->wordset, 0, mfw->use_gpower_id[mfw->decide_cursor_pos] );
#endif
      _Set_MsgStreamExpand(mfw, appwk->setup, msg_mono_fmission_004);
      mfw->order_end = TRUE;
      (*seq)++;
    }
    break;
  case SEQ_DECIDE_STREAM_WAIT:
    if(_Wait_MsgStream(appwk->setup, mfw) == TRUE){
      mfw->wait = 60;
      (*seq)++;
    }
    break;
  case SEQ_DECIDE_STREAM_KEY_WAIT:
    if( mfw->wait ){
      --mfw->wait;
    }
    if( mfw->wait == 0 ||
        GFL_UI_TP_GetTrg() ||
        (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL)))
    {
      PMSND_PlaySE(MONOLITH_SE_MSG);
      _Clear_MsgStream(mfw);
      {
        GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(appwk->parent->gsys);
        GameCommInfo_MessageEntry_FMissionStartWait(game_comm);
      }
      MonolithTool_PanelBG_Focus(appwk, TRUE, FADE_SUB_BG);
      MonolithTool_PanelOBJ_Focus(appwk, &mfw->panel, 1, PANEL_NO_FOCUS, FADE_SUB_OBJ);
      if(GFL_UI_TP_GetTrg()){
        *seq = SEQ_TP_RELEASE_WAIT;
      }
      else{
        *seq = SEQ_FINISH_WAIT;
      }
    }
    break;
  case SEQ_TP_RELEASE_WAIT:
    if(GFL_UI_TP_GetCont() == 0){
      *seq = SEQ_FINISH_WAIT;
    }
    break;
    
  case SEQ_FINISH_WAIT:
    if(MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_BG) != PANEL_COLORMODE_FLASH
        && MonolithTool_PanelColor_GetMode(appwk, FADE_SUB_OBJ) != PANEL_COLORMODE_FLASH
        && MonolithTool_CancelIcon_FlashCheck(&mfw->cancel_icon) == FALSE)
    {
      u16 mission_no = mfw->decide_cursor_pos-1;
      mfw->decide_cursor_pos = _CURSOR_POS_NONE;

      if( mfw->order_end ){
        FEST_MISSION  mission;
        GAME_COMM_NO comm_no = GameCommSys_BootCheck( mfw->gcsp );

        //ミッション開始
        IWASAWA_Printf("mission no %d->%d\n", mission_no,mission_no%4);
        FMISSION_DATA_GetMissionParam( mfw->fmd_man, mission_no%4, &mission);
        LINK_FESTIVAL_MissionStart( mfw->fest_sys, &mission, FALSE );

//        GAMEDATA_SetSubScreenMode(GAMESYSTEM_GetGameData(appwk->parent->gsys), FIELD_SUBSCREEN_FMISSION_VIEW );

        if( comm_no == GAME_COMM_NO_FIELD_BEACON_SEARCH ){
          *seq = SEQ_FINISH;
        }else if( comm_no != GAME_COMM_NO_NULL ){
          GameCommSys_ExitReq( mfw->gcsp );
          OS_TPrintf("パレス通信終了\n");
          *seq = SEQ_COMM_EXIT_WAIT;
        }
        break;
      }
      *seq = SEQ_FINISH;
    }
    break;
  case SEQ_COMM_EXIT_WAIT:
    //通信切断待ち
    if( GameCommSys_BootCheck( mfw->gcsp ) != GAME_COMM_NO_NULL ){
      break;
    }
    if( NetErr_App_CheckError() == NET_ERR_CHECK_NONE ){
      GameCommSys_ClearLastStatus( mfw->gcsp ); //自分が最初の退出者の場合ステータスが残っているのでクリア
    }
    (*seq)++;
    break;
  case SEQ_COMM_RESTART:
    GAMESYSTEM_CommBootAlways( appwk->parent->gsys );
    (*seq)++;
    break;
  case SEQ_FINISH:
    if( mfw->order_end ){
      appwk->next_menu_index = MONOLITH_MENU_END;
    }else{
      appwk->next_menu_index = MONOLITH_MENU_TITLE;
    }
    return GFL_PROC_RES_FINISH;
  }

  _ScrollArrowUpdate(mfw);
  
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
static GFL_PROC_RESULT MonolithFMissionSelectProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_FMSELECT_WORK *mfw = mywk;

  if(PRINTSYS_QUE_IsFinished(appwk->setup->printQue) == FALSE){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_DISP_GXS_SetVisibleControlDirect(GX_PLANEMASK_BG3);

  GFL_TCB_DeleteTask( mfw->vintr_tcb );
  
  if(mfw->print_stream != NULL){
    PRINTSYS_PrintStreamDelete(mfw->print_stream);
  }
  if(mfw->strbuf_stream != NULL){
    GFL_STR_DeleteBuffer(mfw->strbuf_stream);
  }

  _Setup_PowerNameBMPDelete(mfw);

  if(mfw->app_menu_work != NULL){
    MonolithTool_TaskMenu_YesNoExit(mfw->app_menu_work);
  }
  MonolithTool_TaskMenuDelete(mfw->app_menu_res);

  //OBJ
  _DecidePanelDelete(mfw);
  _BmpOamDelete(mfw);
  _CancelIconDelete(mfw);
  _ScrollArrowDelete(mfw);
  //BG
  _Setup_BGGraphicUnload(mfw);
  _Setup_BmpWinDelete(mfw);
  _Setup_BGFrameExit();
  //DATA
  _Setup_DataExit( appwk, mfw );
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   データ設定
 */
//--------------------------------------------------------------
static void _Setup_DataSetting( MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw )
{
  mfw->gdata = GAMESYSTEM_GetGameData(appwk->parent->gsys);
  mfw->gcsp = GAMESYSTEM_GetGameCommSysPtr(appwk->parent->gsys);
  mfw->fest_sys = GAMESYSTEM_GetLinkFestivalWork( appwk->parent->gsys );

  mfw->fmd_man = FMISSION_DATA_Init( HEAPID_MONOLITH );

  mfw->mm_fmission = GFL_MSG_Create(
    GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_fest_mission_dat, HEAPID_MONOLITH);
}

//--------------------------------------------------------------
/**
 * @brief   データ設定開放
 */
//--------------------------------------------------------------
static void _Setup_DataExit( MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw )
{
  FMISSION_DATA_Release( mfw->fmd_man );
  GFL_MSG_Delete(mfw->mm_fmission);
}


//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 */
//--------------------------------------------------------------
static void _Setup_BGFrameSetting(void)
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {
		{//GFL_BG_FRAME0_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		},
		{//GFL_BG_FRAME1_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		},
		{//GFL_BG_FRAME2_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME1_S,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_S,   &bgcnt_frame[2],   GFL_BG_MODE_TEXT );

	GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME1_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
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
	GFL_BG_SetVisible(GFL_BG_FRAME1_S, VISIBLE_OFF);
	GFL_BG_SetVisible(GFL_BG_FRAME2_S, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
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
static void _Setup_BGGraphicLoad(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup)
{
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgd_power_lz_NSCR, 
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_MONOLITH);
	
	GFL_ARCHDL_UTIL_TransVramBgCharacter(setup->hdl, NARC_monolith_mono_bgd_lz_NCGR, 
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_MONOLITH);  //キャラベースを分けているので
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgd_title_lz_NSCR, 
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_MONOLITH);

	//メニューバー  下にパネルを通す為、commonだけでなくこのBG面にもメニューバーをセット
  {
    ARCHANDLE *app_handle = GFL_ARC_OpenDataHandle(ARCID_APP_MENU_COMMON, HEAPID_MONOLITH);
  
    mfw->alloc_menubar_pos = GFL_BG_AllocCharacterArea(
      GFL_BG_FRAME1_S, MENUBAR_CGX_SIZE, GFL_BG_CHRAREA_GET_B );
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
      app_handle, NARC_app_menu_common_menu_bar_NCGR, GFL_BG_FRAME1_S, mfw->alloc_menubar_pos, 
      MENUBAR_CGX_SIZE, FALSE, HEAPID_MONOLITH);
    
    APP_COMMON_MenuBarScrn_Fusion(app_handle, GFL_BG_FRAME1_S, HEAPID_MONOLITH, 
      mfw->alloc_menubar_pos, MONOLITH_MENUBAR_PALNO);
  
  	GFL_ARC_CloseDataHandle(app_handle);
  	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
  }
	
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
}

//--------------------------------------------------------------
/**
 * BGグラフィック解放
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _Setup_BGGraphicUnload(MONOLITH_FMSELECT_WORK *mfw)
{
  GFL_BG_FreeCharacterArea( GFL_BG_FRAME1_S, mfw->alloc_menubar_pos, MENUBAR_CGX_SIZE );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   mfw		
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_BmpWinCreate(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup)
{
  GFL_BMP_DATA *bmp;

  BmpWinFrame_CgxSet( GFL_BG_FRAME0_S, BMPWIN_FRAME_START_CGX, MENU_TYPE_SYSTEM, HEAPID_MONOLITH );
  PaletteWorkSetEx_Arc(setup->pfd, ARCID_FLDMAP_WINFRAME, BmpWinFrame_WinPalArcGet(MENU_TYPE_SYSTEM), 
    HEAPID_MONOLITH, FADE_SUB_BG, 0x20, BMPWIN_FRAME_PALNO * 16, 0);
  
	mfw->bmpwin[_BMPWIN_TALKWIN] = GFL_BMPWIN_Create( GFL_BG_FRAME0_S,
		1,19,30,4, MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B );

	bmp = GFL_BMPWIN_GetBmp( mfw->bmpwin[_BMPWIN_TALKWIN] );
	
	GFL_BMP_Clear( bmp, 0xff );
//	GFL_BMPWIN_TransVramCharacter( mfw->bmpwin[_BMPWIN_TALKWIN] );
//	GFL_BMPWIN_MakeScreen( mfw->bmpwin[_BMPWIN_TALKWIN] );
//	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	
	mfw->bmpwin[_BMPWIN_TITLE] = GFL_BMPWIN_Create( GFL_BG_FRAME0_S,
		0,0,32,3, MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_MakeScreen( mfw->bmpwin[_BMPWIN_TITLE] );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	PRINT_UTIL_Setup(&mfw->print_util[_PRINTUTIL_TITLE], mfw->bmpwin[_BMPWIN_TITLE]);
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _Setup_BmpWinDelete(MONOLITH_FMSELECT_WORK *mfw)
{
  int i;
  for(i = 0; i < _BMPWIN_MAX; i++){
    GFL_BMPWIN_Delete(mfw->bmpwin[i]);
  }
}

//--------------------------------------------------------------
/**
 * タイトル描画
 *
 * @param   appwk		
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _Set_TitlePrint(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw)
{
  STRBUF *str_msg;
  
  str_msg = GFL_MSG_CreateString(appwk->setup->mm_monolith, msg_mono_fmission_000);

  PRINT_UTIL_Print(&mfw->print_util[_PRINTUTIL_TITLE], appwk->setup->printQue, 
    24, 4, str_msg, appwk->setup->font_handle);
  
  GFL_STR_DeleteBuffer(str_msg);
}

//--------------------------------------------------------------
/**
 * メッセージ出力開始
 *
 * @param   mfw		
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Set_MsgStream(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup, u16 msg_id)
{
  GF_ASSERT_HEAVY(mfw->print_stream == NULL);
  
  GFL_FONTSYS_SetColor( 1, 2, 15 );

  mfw->strbuf_stream = GFL_MSG_CreateString(setup->mm_monolith, msg_id);
  
  mfw->print_stream = PRINTSYS_PrintStream(
    mfw->bmpwin[_BMPWIN_TALKWIN], 0, 0, mfw->strbuf_stream, setup->font_handle,
    MSGSPEED_GetWait(), setup->tcbl_sys, 10, HEAPID_MONOLITH, 0xf);

	GFL_BMPWIN_MakeScreen( mfw->bmpwin[_BMPWIN_TALKWIN] );
	BmpWinFrame_Write( mfw->bmpwin[_BMPWIN_TALKWIN], 
	  WINDOW_TRANS_ON_V, BMPWIN_FRAME_START_CGX, BMPWIN_FRAME_PALNO );
}

//--------------------------------------------------------------
/**
 * メッセージ出力開始(WORDSETされたものをEXPANDして出力)
 *
 * @param   mfw		
 * @param   setup		
 * @param   u16     msg_id
 *
 * WORDSET用の単語は外側でセットしておくこと
 */
//--------------------------------------------------------------
static void _Set_MsgStreamExpand(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup, u16 msg_id)
{
  STRBUF *str_temp;
  
  GF_ASSERT_HEAVY(mfw->print_stream == NULL);
  
  GFL_FONTSYS_SetColor( 1, 2, 15 );

  str_temp = GFL_MSG_CreateString(setup->mm_monolith, msg_id);
  mfw->strbuf_stream = GFL_STR_CreateBuffer(256, HEAPID_MONOLITH);
  
  WORDSET_ExpandStr( setup->wordset, mfw->strbuf_stream, str_temp );
  GFL_STR_DeleteBuffer(str_temp);
  
  mfw->print_stream = PRINTSYS_PrintStream(
    mfw->bmpwin[_BMPWIN_TALKWIN], 0, 0, mfw->strbuf_stream, setup->font_handle,
    MSGSPEED_GetWait(), setup->tcbl_sys, 10, HEAPID_MONOLITH, 0xf);

	GFL_BMPWIN_MakeScreen( mfw->bmpwin[_BMPWIN_TALKWIN] );
	BmpWinFrame_Write( mfw->bmpwin[_BMPWIN_TALKWIN], 
	  WINDOW_TRANS_ON_V, BMPWIN_FRAME_START_CGX, BMPWIN_FRAME_PALNO );
}

//--------------------------------------------------------------
/**
 * メッセージ出力完了待ち
 *
 * @param   mfw		
 *
 * @retval  BOOL		TRUE:出力完了
 */
//--------------------------------------------------------------
static BOOL _Wait_MsgStream(MONOLITH_SETUP *setup, MONOLITH_FMSELECT_WORK *mfw)
{
  return APP_PRINTSYS_COMMON_PrintStreamFunc( &setup->app_printsys, mfw->print_stream );
}

//--------------------------------------------------------------
/**
 * メッセージクリア
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _Clear_MsgStream(MONOLITH_FMSELECT_WORK *mfw)
{
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( mfw->bmpwin[_BMPWIN_TALKWIN] );

  GF_ASSERT_HEAVY(mfw->print_stream != NULL);
  PRINTSYS_PrintStreamDelete(mfw->print_stream);
  mfw->print_stream = NULL;
  
  GFL_STR_DeleteBuffer(mfw->strbuf_stream);
  mfw->strbuf_stream = NULL;

  BmpWinFrame_Clear( mfw->bmpwin[_BMPWIN_TALKWIN], WINDOW_TRANS_ON );
  
  GFL_BMP_Clear(bmp, 0xff);
	GFL_BMPWIN_TransVramCharacter( mfw->bmpwin[_BMPWIN_TALKWIN] );

  GFL_FONTSYS_SetColor( 
    MONOLITH_FONT_DEFCOLOR_LETTER, MONOLITH_FONT_DEFCOLOR_SHADOW, MONOLITH_FONT_DEFCOLOR_BACK );
}

//--------------------------------------------------------------
/**
 * リストが画面内に収まっている場合、不必要なスクリーンをクリアする
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _Setup_ScreenClear(MONOLITH_FMSELECT_WORK *mfw)
{
  int start_y;
  
  if(mfw->power_list_num - _DUMMY_PANEL_NUM >= POWER_ITEM_DISP_NUM){
    return;
  }
  
  start_y = (mfw->power_list_num - _DUMMY_PANEL_DOWN_NUM) * POWER_LIST_SPACE / 8;
  GFL_BG_FillScreen( GFL_BG_FRAME2_S, 0, 0, start_y, 32, 32 - start_y, GFL_BG_SCRWRT_PALIN );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
}

//--------------------------------------------------------------
/**
 * Gパワー名BMP作成
 *
 * @param   mfw		
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_PowerNameBMPCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup)
{
  GPOWER_ID gpower_id;
  MONO_USE_POWER use_power;
  int use_count = 0;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(appwk->parent->gsys);
  const OCCUPY_INFO *occupy = MonolithTool_GetOccupyInfo(appwk);
  
  mfw->use_gpower_id[use_count] = 0;  //画面上部の見えないパネル
  mfw->use_power[use_count] = MONO_USE_POWER_SOMEMORE;
  mfw->bmp_power_name[use_count] = GFL_BMP_Create( 
    GPOWER_NAME_BMP_LEN_X, GPOWER_NAME_BMP_LEN_Y, GFL_BMP_16_COLOR, HEAPID_MONOLITH );
  use_count++;
  for(gpower_id = 0; gpower_id < GPOWER_ID_MAX; gpower_id++){
    use_power = MONO_USE_POWER_OK;  //MonolithToolEx_CheckUsePower(setup->powerdata, gpower_id, occupy, appwk->parent->monolith_status.gpower_distribution_bit);
    if(use_power == MONO_USE_POWER_NONE){
      continue;
    }
    
    mfw->use_gpower_id[use_count] = gpower_id;
    mfw->use_power[use_count] = use_power;
    mfw->bmp_power_name[use_count] = GFL_BMP_Create( 
      GPOWER_NAME_BMP_LEN_X, GPOWER_NAME_BMP_LEN_Y, GFL_BMP_16_COLOR, HEAPID_MONOLITH );
    
    use_count++;
  }
  mfw->use_gpower_id[use_count] = 0;  //画面下部の見えないパネル
  mfw->use_power[use_count] = MONO_USE_POWER_SOMEMORE;
  mfw->bmp_power_name[use_count] = GFL_BMP_Create( 
    GPOWER_NAME_BMP_LEN_X, GPOWER_NAME_BMP_LEN_Y, GFL_BMP_16_COLOR, HEAPID_MONOLITH );
  use_count++;

  mfw->power_list_num = use_count;
}

//--------------------------------------------------------------
/**
 * Gパワー名BMP削除
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _Setup_PowerNameBMPDelete(MONOLITH_FMSELECT_WORK *mfw)
{
  int i;
  
  for(i = 0; i < mfw->power_list_num; i++){
    GFL_BMP_Delete(mfw->bmp_power_name[i]);
  }
}

//--------------------------------------------------------------
/**
 * Gパワー名をBMP描画
 *
 * @param   mfw		  
 *
 * @retval  BOOL		TRUE:全てのGパワー名の描画が完了した
 * @retval  BOOL    FALSE:まだ途中
 *
 * TRUEが返るまで毎フレーム呼び続けてください
 */
//--------------------------------------------------------------
static BOOL _MissionNameBMPDraw(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup)
{
  int i, count = 0;
  STRBUF *str;
  FEST_MISSION  mission;

  static const PRINTSYS_LSB print_color[] = {
    PRINTSYS_MACRO_LSB(0xf, 2, 0),
    PRINTSYS_MACRO_LSB(2, 1, 0),
  };
  
  str = GFL_STR_CreateBuffer(32, HEAPID_MONOLITH);
  for(i = mfw->power_list_write_num; i < mfw->power_list_num; i++){
    if(i >= _DUMMY_PANEL_UP_NUM && i < mfw->power_list_num - _DUMMY_PANEL_DOWN_NUM){
      GF_ASSERT(mfw->use_power[i] < NELEMS(print_color));
     
      FMISSION_DATA_GetMissionParam( mfw->fmd_man, mfw->use_gpower_id[i]%4, &mission );
      FEST_MISSION_GetTitleStr( mfw->mm_fmission, str, &mission, GFL_HEAP_LOWID(HEAPID_MONOLITH) );

//      GFL_MSG_GetString(mfw->mm_fmission, /*mfw->use_gpower_id[i]*/, str);
      PRINTSYS_PrintColor( mfw->bmp_power_name[i], 0, 0, str, 
        setup->font_handle, print_color[mfw->use_power[i]] );
    }
    
    count++;
    if(count >= GPOWER_NAME_WRITE_NUM){
      break;
    }
  }
  GFL_STR_DeleteBuffer(str);
  
  mfw->power_list_write_num = i;
  if(mfw->power_list_write_num < mfw->power_list_num){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * BMPOAMを作成
 *
 * @param   mfw		
 * @param   setup		
 */
//--------------------------------------------------------------
static void _BmpOamCreate(MONOLITH_FMSELECT_WORK *mfw, MONOLITH_SETUP *setup)
{
  int i;
  BMPOAM_ACT_DATA head = {
    NULL,
    _BMPOAM_POWER_NAME_X,
    0,
    0,
    0,
    SOFTPRI_BMPOAM_NAME,
    BGPRI_BMPOAM_NAME,
    CLSYS_DEFREND_SUB,
    CLSYS_DRAW_SUB,
  };
  
  head.pltt_index = setup->common_res[COMMON_RESOURCE_INDEX_DOWN].pltt_bmpfont_index;
  for(i = 0; i < POWER_ITEM_DISP_NUM; i++){
    mfw->bmpoam_power_bmp[i] = GFL_BMP_Create( 
      GPOWER_NAME_BMP_LEN_X, GPOWER_NAME_BMP_LEN_Y, GFL_BMP_16_COLOR, HEAPID_MONOLITH );
    head.bmp = mfw->bmpoam_power_bmp[i];
    mfw->bmpoam_power[i] = BmpOam_ActorAdd(setup->bmpoam_sys, &head);
  }
}

//--------------------------------------------------------------
/**
 * BMPOAMを削除
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _BmpOamDelete(MONOLITH_FMSELECT_WORK *mfw)
{
  int i;
  
  for(i = 0; i < POWER_ITEM_DISP_NUM; i++){
    BmpOam_ActorDel(mfw->bmpoam_power[i]);
    GFL_BMP_Delete(mfw->bmpoam_power_bmp[i]);
  }
}

//==================================================================
/**
 * 「ミッション開始」パネル生成
 *
 * @param   appwk		
 * @param   mfw		
 */
//==================================================================
static void _DecidePanelCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_PanelOBJ_Create(appwk->setup, &mfw->panel, 
    COMMON_RESOURCE_INDEX_DOWN, PANEL_SIZE_DECIDE, 
    _PANEL_DECIDE_X, _PANEL_DECIDE_Y, msg_mono_fmission_002, NULL);
  
  if(GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH){
    MonolithTool_PanelOBJ_SetEnable(&mfw->panel, FALSE);
  }
}

//==================================================================
/**
 * 「もらう」パネル削除
 *
 * @param   mfw		
 */
//==================================================================
static void _DecidePanelDelete(MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_PanelOBJ_Delete(&mfw->panel);
}

//==================================================================
/**
 * 「もらう」パネル更新処理
 *
 * @param   mfw		
 */
//==================================================================
static void _DecidePanelUpdate(MONOLITH_SETUP *setup, MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_PanelOBJ_TransUpdate(setup, &mfw->panel);
}

//==================================================================
/**
 * キャンセルアイコン生成
 *
 * @param   appwk		
 * @param   mfw		
 */
//==================================================================
static void _CancelIconCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_CancelIcon_Create(appwk->setup, &mfw->cancel_icon);
}

//==================================================================
/**
 * キャンセルアイコン削除
 *
 * @param   mfw		
 */
//==================================================================
static void _CancelIconDelete(MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_CancelIcon_Delete(&mfw->cancel_icon);
}

//==================================================================
/**
 * キャンセルアイコン更新処理
 *
 * @param   mfw		
 */
//==================================================================
static void _CancelIconUpdate(MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_CancelIcon_Update(&mfw->cancel_icon);
}

//==================================================================
/**
 * 矢印上下アクター生成
 *
 * @param   appwk		
 * @param   mfw		
 */
//==================================================================
static void _ScrollArrowCreate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw)
{
  mfw->act_arrow_up = MonolithTool_Arrow_Create(
    appwk->setup, _SCROLL_ARROW_X, _SCROLL_ARROW_UP_Y, COMMON_ANMSEQ_ARROW_UP);
  mfw->act_arrow_down = MonolithTool_Arrow_Create(
    appwk->setup, _SCROLL_ARROW_X, _SCROLL_ARROW_DOWN_Y, COMMON_ANMSEQ_ARROW_DOWN);
  GFL_CLACT_WK_SetDrawEnable( mfw->act_arrow_up, FALSE );
  GFL_CLACT_WK_SetDrawEnable( mfw->act_arrow_down, FALSE );
}

//==================================================================
/**
 * 矢印上下アクター削除
 *
 * @param   mfw		
 */
//==================================================================
static void _ScrollArrowDelete(MONOLITH_FMSELECT_WORK *mfw)
{
  MonolithTool_ArrowIcon_Delete(mfw->act_arrow_up);
  MonolithTool_ArrowIcon_Delete(mfw->act_arrow_down);
}

//==================================================================
/**
 * 矢印上下アクター更新処理
 *
 * @param   mfw		
 */
//==================================================================
static void _ScrollArrowUpdate(MONOLITH_FMSELECT_WORK *mfw)
{
  if(mfw->list_y <= _LIST_Y_TOP_OFFSET){
    GFL_CLACT_WK_SetDrawEnable( mfw->act_arrow_up, FALSE );
  }
  else{
    GFL_CLACT_WK_SetDrawEnable( mfw->act_arrow_up, TRUE );
  }
//  if((mfw->list_y >> 8) >= POWER_LIST_SPACE * mfw->power_list_num - _MENU_BAR_Y_LEN - 192){
  if((mfw->list_y >> 8) >= mfw->power_list_num * POWER_LIST_SPACE - (_LIST_Y_BOTTOM_OFFSET >> 8) - 192){
    GFL_CLACT_WK_SetDrawEnable( mfw->act_arrow_down, FALSE );
  }
  else{
    GFL_CLACT_WK_SetDrawEnable( mfw->act_arrow_down, TRUE );
  }
  
  MonolithTool_ArrowIcon_Update(mfw->act_arrow_up);
  MonolithTool_ArrowIcon_Update(mfw->act_arrow_down);
}

//--------------------------------------------------------------
/**
 * スクロール速度を算出する
 *
 * @param   mfw		
 *
 * @retval  BOOL		TRUE:決定キーを押した　FALSE:決定キーを押していない
 */
//--------------------------------------------------------------
static BOOL _ScrollSpeedUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw)
{
  BOOL tp_cont;
  u32 tp_x, tp_y;
  BOOL decide_on = FALSE;
  s32 cursor_pos = _CURSOR_POS_NONE;
  
  mfw->one_speed = 0;
  
  tp_cont = GFL_UI_TP_GetPointCont( &tp_x, &tp_y );
  if(mfw->tp_cont_frame == 0 && (tp_x < 24 || tp_x > 224)){
    tp_cont = FALSE;
  }
  
  if(GFL_UI_KEY_GetCont() & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL)){
    s32 list_top_no = ((mfw->list_y >> 8) + _TITLE_BAR_Y_LEN) / POWER_LIST_SPACE;
    s32 list_space_pos = ((mfw->list_y >> 8) + _TITLE_BAR_Y_LEN) % POWER_LIST_SPACE;
    //慣性力で進んでいる場合に新たな入力があれば停止する
    mfw->speed = 0;
    mfw->tp_cont_frame = 0;
    tp_cont = FALSE;
    if(mfw->cursor_pos == _CURSOR_POS_NONE){
      s32 set_pos = list_top_no;
      if(list_space_pos != 0){
        set_pos++;
      }
      if(set_pos < _DUMMY_PANEL_UP_NUM){
        set_pos = _DUMMY_PANEL_UP_NUM;
      }
      if(set_pos >= mfw->power_list_num - 1 - _DUMMY_PANEL_DOWN_NUM){
        _SetCursorPos(mfw, mfw->power_list_num - 1 - _DUMMY_PANEL_DOWN_NUM, _SET_CURSOR_MODE_KEY);
      }
      else{
        _SetCursorPos(mfw, set_pos, _SET_CURSOR_MODE_KEY);
      }
      PMSND_PlaySE(MONOLITH_SE_SELECT);
      return FALSE;
    }
    else{
      if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
        _SetCursorPos(mfw, mfw->cursor_pos, _SET_CURSOR_MODE_KEY);
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        if(mfw->use_power[mfw->cursor_pos] == MONO_USE_POWER_OK){
          PMSND_PlaySE(MONOLITH_SE_DECIDE);
        }
        return TRUE;
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
        s32 check_topno = list_top_no;
        if(list_space_pos != 0){
          check_topno++;
        }
        if(_SetCursorPos(mfw, mfw->cursor_pos - 1, _SET_CURSOR_MODE_KEY) == TRUE){
          PMSND_PlaySE(MONOLITH_SE_SELECT);
        }
        if(mfw->cursor_pos < check_topno){
          mfw->one_speed = -(POWER_LIST_SPACE << 8);
        }
        return FALSE;
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
        s32 check_bottomno = ((mfw->list_y>>8) + 192-_MENU_BAR_Y_LEN) / POWER_LIST_SPACE;
        s32 bottom_list_space = ((mfw->list_y>>8) + 192-_MENU_BAR_Y_LEN) % POWER_LIST_SPACE;
        if(bottom_list_space != 0){
          //check_bottomno--;
        }
        if(_SetCursorPos(mfw, mfw->cursor_pos + 1, _SET_CURSOR_MODE_KEY) == TRUE){
          PMSND_PlaySE(MONOLITH_SE_SELECT);
        }
        if(mfw->cursor_pos >= check_bottomno){
          mfw->one_speed = POWER_LIST_SPACE << 8;
        }
        return FALSE;
      }
    }
  }
  
  if(mfw->tp_cont_frame == 0){
  #if 0
    //慣性力で進んでいる場合に新たな入力があれば停止する
    if(mfw->speed != 0){
      if(tp_cont == TRUE 
          ||(GFL_UI_KEY_GetTrg() & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL))){
        mfw->speed = 0;
      }
    }
  #endif

    if(tp_cont == TRUE){
      if(tp_y > _TITLE_BAR_Y_LEN && tp_y < 192-_MENU_BAR_Y_LEN){
        mfw->tp_cont_frame = 1;
        mfw->tp_backup_y = tp_y;
        mfw->tp_first_hit_y = tp_y;
        _SetCursorPos(mfw, _CURSOR_POS_NONE, _SET_CURSOR_MODE_INIT);
      }
      //「もらう」タッチチェック
      else if(GFL_UI_TP_GetTrg() == TRUE && mfw->cursor_pos != _CURSOR_POS_NONE
          && tp_x >= _PANEL_DECIDE_X-PANEL_DECIDE_CHARSIZE_X/2*8 
          && tp_x <= _PANEL_DECIDE_X+PANEL_DECIDE_CHARSIZE_X/2*8 
          && tp_y >= _PANEL_DECIDE_Y-PANEL_CHARSIZE_Y/2*8
          && tp_y <= _PANEL_DECIDE_Y+PANEL_CHARSIZE_Y/2*8){
        decide_on = TRUE;
        cursor_pos = mfw->cursor_pos;
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      }
    }
    else if(mfw->speed != 0){ //減速
      if(mfw->speed > 0){
        mfw->speed -= _SCROLL_SLOW_DOWN;
        if(mfw->speed < 0){
          mfw->speed = 0;
        }
      }
      else{
        mfw->speed += _SCROLL_SLOW_DOWN;
        if(mfw->speed > 0){
          mfw->speed = 0;
        }
      }
    }
  }
  else{ //タッチパネル押しっぱなしの場合の続き
    if(tp_cont == FALSE){
      mfw->tp_cont_frame = 0;
      if(mfw->tp_first_hit_y != _TOUCH_DECIDE_NONE){
        if(MATH_ABS((s32)mfw->tp_backup_y - (s32)mfw->tp_first_hit_y) < _TOUCH_DECIDE_RANGE){
          s32 list_no = _GetTouchListNo(mfw, mfw->tp_backup_y);
          if(list_no != _LIST_NO_NONE){
            decide_on = TRUE;
            cursor_pos = list_no;
            PMSND_PlaySE(MONOLITH_SE_SELECT);
          }
        }
      }
    }
    else{
      mfw->speed = (mfw->tp_backup_y - (s32)tp_y) * 0x100;  //マイナス有の為SHIFTではなく掛け算
      mfw->tp_backup_y = tp_y;
      mfw->tp_cont_frame++;
      if(mfw->tp_first_hit_y != _TOUCH_DECIDE_NONE){
        if(MATH_ABS((s32)tp_y - (s32)mfw->tp_first_hit_y) > _TOUCH_DECIDE_RANGE){
          mfw->tp_first_hit_y = _TOUCH_DECIDE_NONE;
        }
      }
    }
  }
  
  if(decide_on == TRUE){
    mfw->speed = 0;
    _SetCursorPos(mfw, cursor_pos, _SET_CURSOR_MODE_TP);
  }
  
  return decide_on;
}

//--------------------------------------------------------------
/**
 * カーソル位置変更処理
 *
 * @param   mfw		
 * @param   cursor_pos		カーソル位置(項目番号) ※未選択にするには _CURSOR_POS_NONE
 *
 * @retval  TRUE:カーソル位置を変更した　FALSE:カーソル位置を変更しなかった
 */
//--------------------------------------------------------------
static BOOL _SetCursorPos(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos, _SET_CURSOR_MODE mode)
{
  if(cursor_pos != _CURSOR_POS_NONE 
      && (cursor_pos >= mfw->power_list_num - _DUMMY_PANEL_DOWN_NUM 
      || cursor_pos < _DUMMY_PANEL_UP_NUM)){
    return FALSE;
  }

  mfw->backup_cursor_pos = mfw->cursor_pos;
#if 0 //ダブルタッチで決定は無しにした
  if(mfw->cursor_pos != _CURSOR_POS_NONE){
    mfw->backup_tp_decide_pos = mfw->cursor_pos;
  }
#else
  mfw->backup_tp_decide_pos = mfw->cursor_pos;
#endif
  mfw->cursor_pos = cursor_pos;
  mfw->cursor_pos_update = TRUE;
  mfw->cursor_mode = mode;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * カーソル位置のパネルのスクリーンを指定パレットNoに変更する
 *
 * @param   cursor_pos		
 * @param   palno		
 */
//--------------------------------------------------------------
static void _CursorPos_PanelScreenChange(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos, int palno)
{
  s32 list_top_no, list_space_pos, scr_y, item_offset, target_bg_y;
  
  if(cursor_pos == _CURSOR_POS_NONE){
    return;
  }
  
  //先頭の項目番号
  list_top_no = (mfw->list_y >> 8) / POWER_LIST_SPACE;
  list_space_pos = (mfw->list_y >> 8) % POWER_LIST_SPACE;
  scr_y = (mfw->list_y >> 8) % (256-192);
  item_offset = cursor_pos - list_top_no;
  
  if(item_offset < 0 || item_offset >= list_top_no + POWER_ITEM_DISP_NUM){
    return; //画面外
  }
  
  target_bg_y = (scr_y + POWER_LIST_SPACE * item_offset - list_space_pos ) % 256;
  GFL_BG_ChangeScreenPalette( GFL_BG_FRAME2_S, _PANEL_LEFT_POS, target_bg_y/8, 
    _PANEL_RIGHT_POS - _PANEL_LEFT_POS, _PANEL_SIZE_Y, palno );
}

//--------------------------------------------------------------
/**
 * カーソル位置のパネルをフォーカス状態にする
 *
 * @param   cursor_pos		
 */
//--------------------------------------------------------------
static void _CursorPos_PanelFocus(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos)
{
  _CursorPos_PanelScreenChange(mfw, cursor_pos, COMMON_PALBG_PANEL_FOCUS);
}

//--------------------------------------------------------------
/**
 * カーソル位置のパネルをフォーカス無し状態にする
 *
 * @param   cursor_pos		
 */
//--------------------------------------------------------------
static void _CursorPos_NotFocus(MONOLITH_FMSELECT_WORK *mfw, s32 cursor_pos)
{
  _CursorPos_PanelScreenChange(mfw, cursor_pos, COMMON_PALBG_PANEL);
}

//--------------------------------------------------------------
/**
 * BGスクロール開始前に実行する必要がある処理
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _ScrollBeforeUpdate(MONOLITH_FMSELECT_WORK *mfw)
{
  if(mfw->cursor_pos_update == TRUE){
    //昔のカーソル位置のBGスクリーンをフェード無しのカラーNoに変更する
    _CursorPos_NotFocus(mfw, mfw->backup_cursor_pos);
  }
}

//--------------------------------------------------------------
/**
 * BGスクロール後に実行する必要がある処理
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _ScrollAfterUpdate(MONOLITH_APP_PARENT *appwk, MONOLITH_FMSELECT_WORK *mfw)
{
  if(mfw->cursor_pos_update == TRUE){
    //今のカーソル位置のBGスクリーンをフェード有のカラーNoに変更する
    _CursorPos_PanelFocus(mfw, mfw->cursor_pos);
  
    if(mfw->cursor_pos == _CURSOR_POS_NONE){
      MonolithTool_PanelBG_Focus(appwk, FALSE, FADE_SUB_BG);
    }
    else if((mfw->cursor_mode == _SET_CURSOR_MODE_TP 
            && mfw->cursor_pos == mfw->backup_tp_decide_pos)
        || (mfw->cursor_mode == _SET_CURSOR_MODE_KEY 
            && mfw->cursor_pos == mfw->backup_cursor_pos)){
      if(mfw->use_power[mfw->cursor_pos] == MONO_USE_POWER_OK){
        mfw->decide_cursor_pos = mfw->cursor_pos;
        if(mfw->cursor_mode == _SET_CURSOR_MODE_KEY){
          MonolithTool_PanelBG_Flash(appwk, FADE_SUB_BG);
        }
        else{
          MonolithTool_PanelOBJ_Flash(appwk, &mfw->panel, 1, 0, FADE_SUB_OBJ);
        }
      }
      else{
        if(mfw->cursor_mode == _SET_CURSOR_MODE_TP){
          MonolithTool_PanelBG_Focus(appwk, TRUE, FADE_SUB_BG);
        }
      }
    }
    else{
      MonolithTool_PanelBG_Focus(appwk, TRUE, FADE_SUB_BG);
    }
    GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
    
    mfw->cursor_pos_update = FALSE;
  }
}

//--------------------------------------------------------------
/**
 * リスト座標更新処理
 *
 * @param   mfw		
 * @param   add_y		下位8ビット小数(マイナス有)
 * 
 * @retval  TRUE:座標の変化が整数レベルで発生した
 */
//--------------------------------------------------------------
static BOOL _ScrollPosUpdate(MONOLITH_FMSELECT_WORK *mfw, s32 add_y)
{
  s32 max_y, backup_y;
  
  backup_y = mfw->list_y;
  
  max_y = mfw->power_list_num * POWER_LIST_SPACE - (_LIST_Y_BOTTOM_OFFSET >> 8);
  if(max_y <= 192){
    return FALSE; //スクロールさせる必要無し
  }
  
  mfw->list_y += add_y;
  if(mfw->list_y < _LIST_Y_TOP_OFFSET){
    mfw->list_y = _LIST_Y_TOP_OFFSET;
  }
  else if(mfw->list_y > ((max_y - 192) << 8)){
    mfw->list_y = (max_y - 192) << 8;
  }
  
  if((mfw->list_y >> 8) != (backup_y >> 8)){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * リスト座標をBGの座標へ反映する
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _ScrollPos_BGReflection(MONOLITH_FMSELECT_WORK *mfw)
{
  s32 scr_y;
  
  scr_y = (mfw->list_y >> 8) % (256-192);
  GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, scr_y );
}

//--------------------------------------------------------------
/**
 * リスト座標をパワー名BMPOAMへ反映する
 *
 * @param   mfw		
 */
//--------------------------------------------------------------
static void _ScrollPos_NameOamReflection(MONOLITH_FMSELECT_WORK *mfw)
{
  s32 list_space_pos;
  s32 oam_top_y, list_top_no;
  int i;
  
  //リスト座標から先頭になるOAMのY座標を算出する
  list_space_pos = (mfw->list_y >> 8) % POWER_LIST_SPACE;
  oam_top_y = _BMPOAM_POWER_NAME_OFFSET_Y - list_space_pos;
  
  //リスト座標から先頭になる項目番号を算出する
  list_top_no = (mfw->list_y >> 8) / POWER_LIST_SPACE;
  
  //先頭から順に名前BMPを上書きコピーする
  for(i = 0; i < POWER_ITEM_DISP_NUM; i++){
    BmpOam_ActorSetPos(mfw->bmpoam_power[i], 
      _BMPOAM_POWER_NAME_X, oam_top_y + POWER_LIST_SPACE * i);
    if(mfw->bmp_power_name[list_top_no + i] != NULL){
      GFL_BMP_Copy( mfw->bmp_power_name[list_top_no + i], mfw->bmpoam_power_bmp[i] );
    }
    //転送はVブランクで行う
  }
}

//--------------------------------------------------------------
/**
 * パワー選択画面：VブランクTCB
 *
 * @param   tcb		
 * @param   work		
 */
//--------------------------------------------------------------
static void _PowerSelect_VBlank(GFL_TCB *tcb, void *work)
{
  MONOLITH_FMSELECT_WORK *mfw = work;
  int i;
  
  if(mfw->scroll_update == TRUE){
    for(i = 0; i < POWER_ITEM_DISP_NUM; i++){
      BmpOam_ActorBmpTrans(mfw->bmpoam_power[i]);
    }
    mfw->scroll_update = FALSE;
  }
}

//--------------------------------------------------------------
/**
 * タッチ座標とリスト座標からタッチした項目Noを取得する
 *
 * @param   mfw		
 * @param   tp_y		タッチ座標Y
 *
 * @retval  s32		項目No(無効な場合は_LIST_NO_NONE)
 */
//--------------------------------------------------------------
static s32 _GetTouchListNo(MONOLITH_FMSELECT_WORK *mfw, u32 tp_y)
{
  s32 list_no;
  
  list_no = ((mfw->list_y >> 8) + tp_y) / POWER_LIST_SPACE;
  if(list_no >= mfw->power_list_num){
    return _LIST_NO_NONE;
  }
  return list_no;
}
