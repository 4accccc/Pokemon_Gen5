//------------------------------------------------------------------------------
/**
 *  @file mailview.c
 *  @brief  メール画面描画ルーチン
 *  @author Akito Mori / Miyuki Iwasawa
 *  @date 09.11.10
 */
//------------------------------------------------------------------------------


#include <gflib.h>

#include "system/main.h"
#include "system/palanm.h"
#include "system/gfl_use.h"
#include "system/pms_draw.h"
#include "arc/arc_def.h"
#include "arc/winframe.naix"
#include "arc/message.naix"
#include "system/bmp_winframe.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"
#include "item/item.h"
#include "print/printsys.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "gamesystem/game_data.h"
#include "gamesystem/msgspeed.h"
#include "app/mailtool.h"
#include "savedata/mail_util.h"
#include "savedata/config.h"
#include "sound/pm_sndsys.h"
#include "mail_param.h"
#include "mailview.h"
#include "app/pms_input.h"
#include "app/app_taskmenu.h"
#include "mail_snd_def.h"
#include "buflen.h"
#include "system/wipe.h"

#include "pokeicon/pokeicon.h"
#include "poke_tool/monsno_def.h"

#include "savedata/mail.h"
#include "mail_param.h"
#include "mailview.h"
#include "mailview_def.h"
#include "arc/mail_gra.naix"
#include "msg/msg_mailview.h"
#include "mail_snd_def.h"


#define MAILD_CHAR_S  (NARC_mail_gra_mail_000_lz_ncgr)
#define MAILD_SCRN_S  (NARC_mail_gra_mail_000_lz_nscr)
#define MAILD_PLTT_S  (NARC_mail_gra_mail_000_nclr)
#define MAILD_SCRN_FIL  (NARC_mail_gra_mail_scr_lz_nscr)

#define WINCLR_COL(col) (((col)<<4)|(col))

enum{
 MAILVIEW_BLACKSET,
 MAILVIEW_RESINIT,
 MAILVIEW_FADEIN,
 MAILVIEW_MAIN,
 MAILVIEW_FADEOUT,
 MAILVIEW_RESRELEASE,
};

enum{
 KEYIN_VIEW,
 KEYIN_CREATE,
 KEYIN_NOMSG,
 KEYIN_CANCEL
}KEYIN;

enum{
 WIN_SPACE_M,
 WIN_M01,
 WIN_M02,
 WIN_M03,
 WIN_NAME,
 WIN_TALK,
 WIN_MAX,
};
enum{
 VIEW_SIDE_DECIDE,
 VIEW_SIDE_CANCEL,
};

#define VIEW_LINE_MAX (5)
#define VIEW_BUFLEN_MSG (19*2*2)
#define VIEW_TALK_FCOL  (PRINTSYS_LSB_Make( 1,2,15))
#define VIEW_NRM_FCOL   (PRINTSYS_LSB_Make( 1,2, 0))

#define APP_KTST_TOUCH  ( 0 )
#define APP_KTST_KEY    ( 1 )
#define APP_END_TOUCH   ( 0 )
#define APP_END_KEY     ( 1 )

//@TODO wk->win_typeにまだコンフィグで指定するウインドウの種類が素材として
//　　　存在していないので、後から対応する必要が有る

typedef struct _MAIL_VIEW_DATA  MAIL_VIEW_DAT;

///サブプロセス定義用関数型
typedef int (*MailKeyIn)(MAIL_VIEW_DAT* wk);

///メール描画メインワーク
struct _MAIL_VIEW_DATA{
  int heapID;
  int seq;    ///<メインシーケンスNo
  u16 sub_seq;  ///<サブシーケンスNo
  u8  key_mode; ///<操作モード
  u8  win_type; ///<ウィンドウタイプ
  u8  mode; ///<動作タイプ
  u8  inMode; ///<動作タイプ
  u8  cntNo;
  u8  msgIdx;

  u8  msg_spd;
  u8  line; ///<選択ライン
  u8  side; ///<サイド選択
  u8  canm_f; ///<アニメフックフラグ
  u8  colEvy;
  u8  colDir;
  u8  nowCol;
  u8  oldCol;

  MAIL_TMP_DATA *dat;
  GFL_MSGDATA*  pMsgData;
  GFL_FONT      *font;
  STRBUF  *pMsg[MAILDAT_MSGMAX];
  PALETTE_FADE_PTR  palAnm;

  MailKeyIn to_key_func;  ///<キーに切替
  MailKeyIn to_touch_func;  ///<タッチに切替

  ///グラフィックリソース
  GFL_BMPWIN    *win[WIN_MAX];
  PRINT_UTIL    printUtil[WIN_MAX];
  PRINT_QUE     *printQue;
  PRINT_STREAM  *printStream;


  // セルアクター
  GFL_CLUNIT   *clUnit;
  GFL_CLWK     *clwk[MAILDAT_ICONMAX];
  u32          clres[4][MAILDAT_ICONMAX];  // セルアクターリソース用インデックス
  
  void*        *TcbWorkArea;
  GFL_TCBSYS   *TcbSys;
  GFL_TCB      *palAnmTcb;
  GFL_TCBLSYS  *pMsgTcblSys;
  GFL_TCB      *vBlankTcb;

  // メニュー共通リソース
  APP_TASKMENU_RES  *menures;
  // はい・いいえメニュー
  APP_TASKMENU_WORK *yn_menuwork;
  APP_TASKMENU_ITEMWORK yn_menuitem[2];
  // 「けってい・やめる」メニュー
  APP_TASKMENU_WIN_WORK *menuwork[2];
  APP_TASKMENU_ITEMWORK menuitem[2];

  PMS_DRAW_WORK *pms_draw_work;
  GFL_CLUNIT    *pmsClunit;
  PRINT_QUE     *pmsPrintque;

};


//=================================================
//プロトタイプ
//=================================================
static int MailViewMain(MAIL_VIEW_DAT* wk);

static int inchg2touch_create(MAIL_VIEW_DAT* wk);
static int inchg2key_create(MAIL_VIEW_DAT* wk);
static void bg_visible_init( void );
static int MailView_KeyInView(MAIL_VIEW_DAT* wk);
static int MailView_KeyInCreate(MAIL_VIEW_DAT* wk);
static int MailView_KeyInCancel(MAIL_VIEW_DAT* wk);
static int MailView_KeyInMsg(MAIL_VIEW_DAT* wk);

static void MailView_PltAnime( GFL_TCB *tcb, void* work );
static int MailViewResInit(MAIL_VIEW_DAT* wk);
static int MailViewResRelease(MAIL_VIEW_DAT* wk);

static void MailVBlank( GFL_TCB *tcb, void * work );
static void MailView_VBankSet(void);
static void MailView_BGLInit(MAIL_VIEW_DAT* wk);
static void MailView_BGLRelease(MAIL_VIEW_DAT* wk);
static void MailView_2DGraInit(MAIL_VIEW_DAT* wk);
static void MailView_2DGraRelease(MAIL_VIEW_DAT* wk);
static void MailView_BmpWinInit(MAIL_VIEW_DAT* wk);
static void MailView_BmpWinRelease(MAIL_VIEW_DAT* wk);
static void MailView_MsgWrite(MAIL_VIEW_DAT* wk);
static void MailView_MenuMake( MAIL_VIEW_DAT* wk );

static void MailView_PokeIconInit(MAIL_VIEW_DAT* wk);
static void MailView_PokeIconRelease(MAIL_VIEW_DAT* wk);

static void MailView_PmsInit( MAIL_VIEW_DAT *wk );
static void MailView_PmsExit( MAIL_VIEW_DAT *wk );
static void MailView_PalAnmInit( MAIL_VIEW_DAT *wk );


//====================================================================
//プロセス定義
//====================================================================

//=============================================================================================
/**
 * @brief   メール描画呼び出し　初期化
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT MailViewProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  MAIL_TMP_DATA* tp;
  MAIL_VIEW_DAT* wk;

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MAILVIEW, 0x20000 );
  wk = GFL_PROC_AllocWork( proc, sizeof(MAIL_VIEW_DAT), HEAPID_MAILVIEW );
  MI_CpuClearFast( wk, sizeof(MAIL_VIEW_DAT) );

  //ヒープID保存
  wk->heapID = HEAPID_MAILVIEW;

  //パラメータ引継ぎ
  wk->dat    = pwk;
  wk->inMode = wk->mode = wk->dat->val; //描画タイプ
  wk->line   = wk->dat->cntNo;
  wk->side   = wk->dat->flags;
  wk->oldCol = wk->nowCol = wk->line;
  wk->dat->val = VIEW_END_CANCEL;

  wk->msg_spd  = MSGSPEED_GetWait();

  wk->font = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                                    GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapID );

  wk->pMsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );

  //現在のキーモードを取得
//  wk->key_mode = KeyTouchStatus_CheckTouchOrKey(wk->dat->kt_status);
  wk->key_mode   = APP_END_KEY;

  GFL_BG_Init( wk->heapID );

  return GFL_PROC_RES_FINISH;
}


//=============================================================================================
/**
 * @brief   メール描画呼び出し　メイン
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT MailViewProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  MAIL_VIEW_DAT* wk = mywk;

  if(MailViewMain(wk)){
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================================
/**
 * @brief   メール描画呼び出し  終了　
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT MailViewProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  MAIL_VIEW_DAT* wk = mywk;
  int heap;

  GFL_BG_Exit( );
  
  GFL_TCBL_Exit( wk->pMsgTcblSys );
  GFL_FONT_Delete( wk->font );

  //終了パラメータ格納
  heap = wk->heapID;
  
  //現在のキーモードを書き出し
//  KeyTouchStatus_SetTouchOrKey(wk->dat->kt_status,wk->key_mode);

  //ワークエリア解放
  GFL_PROC_FreeWork( proc );
  
  if(GFL_HEAP_CheckHeapSafe( heap )){
    GF_ASSERT( "heap errror!!!\n" );
  }

  GFL_HEAP_DeleteHeap( heap );


  return GFL_PROC_RES_FINISH;
}


//====================================================================
//ローカル関数定義
//====================================================================



//----------------------------------------------------------------------------------
/**
 * @brief メール描画メイン
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailViewMain(MAIL_VIEW_DAT* wk)
{
  int result = FALSE;

  switch(wk->seq){
  case MAILVIEW_BLACKSET:
    //Blank関数リセット

    bg_visible_init();
    
    WIPE_SetBrightness( WIPE_DISP_MAIN, WIPE_FADE_BLACK);
    WIPE_SetBrightness( WIPE_DISP_SUB,  WIPE_FADE_BLACK);
    WIPE_ResetWndMask(  WIPE_DISP_MAIN);
    WIPE_ResetWndMask(  WIPE_DISP_SUB);
    wk->seq++;
    break;
  case MAILVIEW_RESINIT:
    if(MailViewResInit(wk)){
      wk->vBlankTcb = GFUser_VIntr_CreateTCB( MailVBlank, wk, 16 );

      GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON);
  
      //ブレンド設定
      WIPE_ResetBrightness(WIPE_DISP_MAIN);
      G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG3,ALPHA_SECOND,ALPHA_FIRST);

      PaletteFadeReq( wk->palAnm, PF_BIT_MAIN_ALL, 0xFFFF,-1,16,0,0x0000, GFUser_VIntr_GetTCBSYS());
      wk->seq++;
    }
    break;
  case MAILVIEW_FADEIN:
    if( PaletteFadeCheck(wk->palAnm)==0){
      wk->seq++;
    }
    PMS_DRAW_Main( wk->pms_draw_work );
    break;
  case MAILVIEW_MAIN:
    {
      static const MailKeyIn keyin[] = {
        MailView_KeyInView,
        MailView_KeyInCreate,
        MailView_KeyInMsg,
        MailView_KeyInCancel,
      };
  
      if(keyin[wk->mode](wk)==TRUE){
        wk->seq++;
        PaletteFadeReq( wk->palAnm, PF_BIT_MAIN_ALL, 0xFFFF,3,0,16,0x0000, GFUser_VIntr_GetTCBSYS());
      }
    }
    PMS_DRAW_Main( wk->pms_draw_work );
    if(wk->menuwork[0]!=NULL){
      APP_TASKMENU_WIN_Update( wk->menuwork[0] );
      APP_TASKMENU_WIN_Update( wk->menuwork[1] );
    }
    break;
  case MAILVIEW_FADEOUT:
    if( PaletteFadeCheck(wk->palAnm)==0){
      WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
      WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
      GFL_TCB_DeleteTask( wk->vBlankTcb );
      bg_visible_init();
      wk->seq++;
    }
    PMS_DRAW_Main( wk->pms_draw_work );
    break;
  case MAILVIEW_RESRELEASE:
    if(MailViewResRelease(wk)){
      result = TRUE;
    }
  }

  if(wk->clUnit != NULL){
    GFL_CLACT_SYS_Main( );
  }
  if(wk->TcbSys!=NULL){
    GFL_TCB_Main( wk->TcbSys );
  }
  if(wk->pMsgTcblSys){
    GFL_TCBL_Main( wk->pMsgTcblSys );
  }
  if(wk->printQue){
    PRINTSYS_QUE_Main( wk->printQue );
  }
  if(wk->pmsPrintque){
    PRINTSYS_QUE_Main( wk->pmsPrintque );
  }
  
  return result;
}

//----------------------------------------------------------------------------------
/**
 * @brief   会話文が全文空かどうかチェック
 *
 * @param   wk    
 *
 * @retval  BOOL  TRUE:全文空 FALSE:入力有り
 */
//----------------------------------------------------------------------------------
static BOOL MailView_IsWordNull(MAIL_VIEW_DAT* wk)
{
  int i = 0;

  for(i = 0;i < MAILDAT_MSGMAX;i++){
    if(PMSDAT_IsEnabled(&wk->dat->msg[i])){
      return FALSE;
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief   パレットアニメを元の色に戻す
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void palanm_reset(MAIL_VIEW_DAT* wk)
{
  //元の色に戻す
  SoftFadePfd(wk->palAnm, FADE_MAIN_BG, PALANM_STARTPAL+wk->oldCol,1,0,0x7FFF);
  wk->colDir = 0;
  wk->colEvy = 0;
}


//----------------------------------------------------------------------------------
/**
 * @brief   Createモードでキーからタッチに切り替える処理
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int inchg2touch_create(MAIL_VIEW_DAT* wk)
{
  palanm_reset(wk);
  return 0;
}

//----------------------------------------------------------------------------------
/**
 * @brief   Createモードでタッチからキーに切り替える処理
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int inchg2key_create(MAIL_VIEW_DAT* wk)
{
  return 0;
}


//----------------------------------------------------------------------------------
/**
 * @brief キー＆タッチ切替関数
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int input_change(MAIL_VIEW_DAT* wk)
{
  if(wk->key_mode == APP_KTST_TOUCH){
    if(GFL_UI_TP_GetCont() != 0){ //
      return 0;
    }
    if(GFL_UI_KEY_GetCont() != 0){
      if(wk->to_key_func != NULL){
        (wk->to_key_func)(wk);
      }
      wk->key_mode = APP_KTST_KEY;
      return 1;
    }
  }else{
    if(GFL_UI_KEY_GetCont() != 0){
      return 0;
    }
    if(GFL_UI_TP_GetCont() != 0){
      if(wk->to_touch_func != NULL){
        (wk->to_touch_func)(wk);
      }
      wk->key_mode = APP_KTST_TOUCH;
      return 0;
    }
  }
  return 0;
}


//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　キー取得(Viewモード)
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailView_KeyInView(MAIL_VIEW_DAT* wk)
{
  if(GFL_UI_KEY_GetTrg()){
    wk->key_mode = APP_KTST_KEY;
  }else if(GFL_UI_TP_GetCont()){
    wk->key_mode = APP_KTST_TOUCH;
  }else{
    return 0;
  }
  GFL_SOUND_PlaySE(SND_MAIL_FINISH);
  return 1;
}

//----------------------------------------------------------------------------------
/**
 * @brief   キー取得(Createモード)
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int input_key_create(MAIL_VIEW_DAT* wk)
{
  u8 se_play = 0;
  
  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE)){
    if(wk->line == VIEW_END_DECIDE){
      //会話文が空かどうかチェック
      if(MailView_IsWordNull(wk)){
        GFL_SOUND_PlaySE(SND_MAIL_CANCEL);
        wk->mode = KEYIN_NOMSG;
        return FALSE;
      }else{
        wk->dat->val = VIEW_END_DECIDE;
        GFL_SOUND_PlaySE(SND_MAIL_FINISH);
        wk->dat->cntNo = 0;
        wk->dat->flags = 0;
        return TRUE;
      }
    }else if(wk->line == 4){
      GFL_SOUND_PlaySE(SND_MAIL_CANCEL);
      wk->mode = KEYIN_CANCEL;
      return FALSE;
    }else{
      wk->dat->val = wk->dat->cntNo = wk->line;
      wk->dat->flags = wk->side;
      GFL_SOUND_PlaySE(SND_MAIL_DECIDE);
    }
    return TRUE;
  }else if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL)){
    GFL_SOUND_PlaySE(SND_MAIL_CANCEL);
    wk->mode = KEYIN_CANCEL;
    return FALSE;
  }

  //選択項目変更
  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_START)){
    //一発で「決定」へ
    wk->line = VIEW_END_DECIDE;
    wk->side = VIEW_SIDE_DECIDE;
    se_play = 1;
  }else if(GFL_UI_KEY_GetTrg() & (PAD_KEY_DOWN)){
    wk->line = (wk->line + 1)%VIEW_LINE_MAX;
    se_play = 1;
  }else if(GFL_UI_KEY_GetTrg() & (PAD_KEY_UP)){
    wk->line = (wk->line+VIEW_LINE_MAX-1)%VIEW_LINE_MAX;
    se_play = 1;
  }else{
    return FALSE;
  }

  if(!se_play){
    return FALSE;
  }
  //選択ライン描画変更
  GFL_SOUND_PlaySE(SND_MAIL_SELECT);

  if(wk->line<3)
  {
    APP_TASKMENU_WIN_SetActive( wk->menuwork[0], FALSE );
    APP_TASKMENU_WIN_SetActive( wk->menuwork[1], FALSE );
  }
  else if(wk->line==VIEW_END_DECIDE)
  {
    APP_TASKMENU_WIN_SetActive( wk->menuwork[0], TRUE );
    APP_TASKMENU_WIN_SetActive( wk->menuwork[1], FALSE );
  }
  else if(wk->line==4)
  {
    APP_TASKMENU_WIN_SetActive( wk->menuwork[0], FALSE );
    APP_TASKMENU_WIN_SetActive( wk->menuwork[1], TRUE );
  }

  // カラー変更位置を変更する
  wk->nowCol = wk->line;
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief   タッチ取得(Createモード)
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int input_touch_create(MAIL_VIEW_DAT* wk)
{
  int ret;
  u16 pat;
  static const GFL_UI_TP_HITTBL Btn_TpRect[] = {
//    {0,191,0,255}, ty,by,lx,rx
    {TPMSG_DPY01,TPMSG_DPY01+TPMSG_DSY,TPMSG_DPX,TPMSG_DPX+TPMSG_DSX},
    {TPMSG_DPY02,TPMSG_DPY02+TPMSG_DSY,TPMSG_DPX,TPMSG_DPX+TPMSG_DSX},
    {TPMSG_DPY03,TPMSG_DPY03+TPMSG_DSY,TPMSG_DPX,TPMSG_DPX+TPMSG_DSX},
    {TPSW_CANCEL_Y,TPSW_CANCEL_Y+TPSW_CANCEL_H,TPSW_CANCEL_X,TPSW_CANCEL_X+TPSW_CANCEL_W},
    {TPSW_DECIDE_Y,TPSW_DECIDE_Y+TPSW_DECIDE_H,TPSW_DECIDE_X,TPSW_DECIDE_X+TPSW_DECIDE_W},
    {GFL_UI_TP_HIT_END,0,0,0}
  };
  ret = GFL_UI_TP_HitTrg( Btn_TpRect );
  if( ret == GFL_UI_TP_HIT_NONE){
    return FALSE;
  }
//  pat = 0x0001; //0000 0000 0000 0001 無効パレットbit(onになっている色Noは反応させない)
//  if(GFL_BG_DotCheck(wk->bgl,GFL_BG_FRAME2_M,sys.tp_x,sys.tp_y,&pat) == TRUE){
//    return FALSE;
//  }

  if(ret == 3){ //キャンセル
    GFL_SOUND_PlaySE(SND_MAIL_CANCEL);
    wk->mode = KEYIN_CANCEL;
    return FALSE;
  }
  if(ret < 3){  //文字入力
    wk->dat->val = wk->dat->cntNo = ret;
    wk->dat->flags = wk->side;
    GFL_SOUND_PlaySE(SND_MAIL_DECIDE);
    return TRUE;
  }
  //決定のとき

  //会話文が空かどうかチェック
  if(MailView_IsWordNull(wk)){
    GFL_SOUND_PlaySE(SND_MAIL_CANCEL);
    wk->mode = KEYIN_NOMSG;
    return FALSE;
  }else{
    wk->dat->val = VIEW_END_DECIDE;
    GFL_SOUND_PlaySE(SND_MAIL_FINISH);
    wk->dat->cntNo = 0;
    wk->dat->flags = 0;
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　キー取得(Createモード)
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailView_KeyInCreate(MAIL_VIEW_DAT* wk)
{
  if(input_change(wk)){
    return FALSE;
  }

  if(wk->key_mode == MAIL_KEY_IN){    // キー入力モードのとき
    return input_key_create(wk);
  }else{
    return input_touch_create(wk);
  }
  
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 会話ウインドウメッセージ描画開始
 *
 * @param   wk    
 * @param   msgId   
 */
//----------------------------------------------------------------------------------
static void print_talk_msg( MAIL_VIEW_DAT *wk, int msgId )
{
  STRBUF* str = NULL;

  TalkWinFrame_Write( wk->win[WIN_TALK], WINDOW_TRANS_OFF, BMPL_TALK_WIN_CGX, BMPL_TALK_WIN_PAL );
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[WIN_TALK]), WINCLR_COL(15));

  str = GFL_MSG_CreateString( wk->pMsgData, msgId  );
  wk->printStream = PRINTSYS_PrintStream(
    wk->win[WIN_TALK], 0, 0, str, wk->font,
    wk->msg_spd, wk->pMsgTcblSys, 4, wk->heapID, 15 );
  
  GFL_BMPWIN_MakeTransWindow( wk->win[WIN_TALK] );

//  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, 0, 0, str, wk->font, VIEW_TALK_FCOL );

/*
  GF_STR_PrintColor(
    &wk->win[WIN_TALK],FONT_TALK,str,
    0,0,MSG_ALLPUT,VIEW_TALK_FCOL,NULL);
*/
  GFL_STR_DeleteBuffer(str);
  
}  

//----------------------------------------------------------------------------------
/**
 * @brief メール描画キー取得　メッセージ待ちモード
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailView_KeyInMsg(MAIL_VIEW_DAT* wk)
{
  int ret = 0;
  STRBUF* str = NULL;
  
  switch(wk->sub_seq){
  case 0:
    //空はダメメッセージ描画開始
    print_talk_msg( wk, msg_mail_nozero );
    break;
  case 1:
    //キー待ち
    if((GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL))){
      ret = TRUE;
      wk->key_mode = APP_KTST_KEY;
    }else if(GFL_UI_TP_GetTrg()){
      ret = TRUE;
      wk->key_mode = APP_KTST_TOUCH;
      palanm_reset(wk);
    }
    if(!ret){
      return FALSE;
    }
    TalkWinFrame_Clear( wk->win[WIN_TALK],WINDOW_TRANS_OFF);
    GFL_BMPWIN_ClearTransWindow( wk->win[WIN_TALK]);

    wk->sub_seq = 0;
    wk->mode = wk->inMode;  //モードを元に戻す 
    return FALSE;
  }
  wk->sub_seq++;
  return FALSE;
}



//----------------------------------------------------------------------------------
/**
 * @brief はい・いいえ開始
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void yesNoInit(MAIL_VIEW_DAT* wk)
{
  APP_TASKMENU_INITWORK init;

  wk->yn_menuitem[0].str      = GFL_MSG_CreateString( wk->pMsgData, msg_mail_yesno_y ); //メニューに表示する文字列
  wk->yn_menuitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
  wk->yn_menuitem[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示

  wk->yn_menuitem[1].str      = GFL_MSG_CreateString( wk->pMsgData, msg_mail_yesno_n ); //メニューに表示する文字列
  wk->yn_menuitem[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
  wk->yn_menuitem[1].type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示


  init.heapId   = wk->heapID;
  init.itemNum  = 2;
  init.itemWork = wk->yn_menuitem;
  init.posType  = ATPT_LEFT_UP;
  init.charPosX = 23; //ウィンドウ開始位置(キャラ単位
  init.charPosY =  6;
  init.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;  //キャラ単位
  init.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;  //キャラ単位
  
  
  wk->yn_menuwork = APP_TASKMENU_OpenMenu( &init, wk->menures );
}


//----------------------------------------------------------------------------------
/**
 * @brief はい・いいえ選択待ち
 *
 * @param   wk    
 *
 * @retval  int   0:はい  1:いいえ  -1:未選択
 */
//----------------------------------------------------------------------------------
static int yesNoWait( MAIL_VIEW_DAT *wk )
{
  int result = -1;
  
  // 終了判定
  if(APP_TASKMENU_IsFinish(wk->yn_menuwork)){
    result = APP_TASKMENU_GetCursorPos(wk->yn_menuwork); // カーソル位置取得
    APP_TASKMENU_CloseMenu(wk->yn_menuwork);             // メニュー終了
    GFL_STR_DeleteBuffer( wk->yn_menuitem[0].str );
    GFL_STR_DeleteBuffer( wk->yn_menuitem[1].str );
    OS_Printf( "yesno=%d\n", result );
  }

  return result;
}

//----------------------------------------------------------------------------------
/**
 * @brief メール表示キャンセル待ち
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailView_KeyInCancel(MAIL_VIEW_DAT* wk)
{
  int ret;
  STRBUF* str = NULL;
  
  switch(wk->sub_seq){
  case 0:
    wk->canm_f = 1;
    //やめますかメッセージ描画開始
    print_talk_msg( wk, msg_mail_cmsg );

    //カラーアニメを一旦もとの色に戻す
    palanm_reset(wk);
    wk->oldCol = wk->nowCol;
    break;
  case 1:
    //描画終了待ち
    if(PRINTSYS_PrintStreamGetState( wk->printStream )==PRINTSTREAM_STATE_DONE)
    {
      PRINTSYS_PrintStreamDelete( wk->printStream );
      //YNウィンドウ初期化
      yesNoInit(wk);
      wk->sub_seq = 2;
    }
    return FALSE;
    break;
  case 2:
    //選択待ち
    APP_TASKMENU_UpdateMenu( wk->yn_menuwork);

    ret = yesNoWait(wk);
    if(ret < 0){
      return FALSE;
    }
    TalkWinFrame_Clear( wk->win[WIN_TALK],WINDOW_TRANS_OFF);
    GFL_BMPWIN_ClearTransWindow( wk->win[WIN_TALK]);
    wk->sub_seq = 0;

    if(ret==0){
      //モジュール終了
      wk->dat->val = VIEW_END_CANCEL;
      wk->mode     = wk->inMode;
      return TRUE;
    }else{
      wk->mode = wk->inMode;  //モードを元に戻す 
      wk->canm_f = 0;
      return FALSE;
    }
  }
  wk->sub_seq++;
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief BG面の一斉非表示設定
 *
 * @param   none    
 */
//----------------------------------------------------------------------------------
static void bg_visible_init( void )
{
    GFL_DISP_GX_InitVisibleControl();
    GFL_DISP_GXS_InitVisibleControl();
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF);
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF);
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF);
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF);
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF);
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF);

}

//----------------------------------------------------------------------------------
/**
 * @brief メール画面VBlank
 *
 * @param   tcb   
 * @param   work    
 */
//----------------------------------------------------------------------------------
static void MailVBlank( GFL_TCB *tcb, void * work )
{
  MAIL_VIEW_DAT* wk = (MAIL_VIEW_DAT*)work;

  //パレット転送
  if( wk->palAnm != NULL ){
    PaletteFadeTrans( wk->palAnm );
  }


  GFL_CLACT_SYS_VBlankFunc();
  GFL_BG_VBlankFunc( );
  OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画パレットアニメ【タスク関数】
 *
 * @param   tcb   
 * @param   work    
 */
//----------------------------------------------------------------------------------
static void MailView_PltAnime( GFL_TCB *tcb, void* work )
{
  MAIL_VIEW_DAT* wk = (MAIL_VIEW_DAT*)work;

//  //パレットアニメが削除されたらタスクも削除
//  if(wk->palAnm == NULL){
//    TCB_Delete(tcb);
//    return;
//  }
  //アニメフックフラグが立っている時は一時停止
  if(wk->canm_f || wk->key_mode == MAIL_TOUCH_IN){
    return;
  }

  if(wk->oldCol != wk->nowCol){
    //元の色に戻す
    palanm_reset(wk);
    wk->oldCol = wk->nowCol;
  }
  SoftFadePfd(wk->palAnm,FADE_MAIN_BG,PALANM_STARTPAL+wk->nowCol,1,wk->colEvy,0x7FFF);
  if(wk->colDir){
    if(wk->colEvy-- == 1){
      wk->colDir ^= 1;
    }
  }else{
    if(wk->colEvy++ == 12){
      wk->colDir ^= 1;
    }
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief   メール描画リソース初期化
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailViewResInit(MAIL_VIEW_DAT* wk)
{
  switch(wk->sub_seq){
  case 0:
    // メッセージデータオープン
    wk->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                 NARC_message_mailview_dat, wk->heapID );
    MailView_BGLInit(wk);
    break;
  case 1:
    MailView_2DGraInit(wk);
    break;
  case 2:
    MailView_BmpWinInit(wk);
    break;
  case 3:
    MailView_PokeIconInit(wk);
    wk->menures = APP_TASKMENU_RES_Create( GFL_BG_FRAME0_M, 8, wk->font, wk->printQue, wk->heapID );
    wk->sub_seq = 0;
    MailView_PmsInit(wk);
    MailView_MsgWrite(wk);
    MailView_MenuMake( wk );
    MailView_PalAnmInit( wk );
    return TRUE;
  }
  wk->sub_seq++;
  return FALSE;
}


#define PAL_TCB_MAX ( 3 )

//----------------------------------------------------------------------------------
/**
 * @brief パレットフェードを初期状態（黒くフェード）にする
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_PalAnmInit( MAIL_VIEW_DAT *wk )
{
  //初期パレットを塗りつぶす
  SoftFadePfd(wk->palAnm,FADE_MAIN_BG, 0,16*MAILVIEW_PALMAX,16,0x0000);
  SoftFadePfd(wk->palAnm,FADE_MAIN_OBJ,0,16*3,16,0x0000);
  PaletteTrans_AutoSet(wk->palAnm,TRUE);
  PaletteFadeTrans( wk->palAnm );

  //パレットアニメ設定  
  wk->colEvy = 0;
  wk->colDir = 0;

  wk->TcbWorkArea = GFL_HEAP_AllocMemory( wk->heapID, GFL_TCB_CalcSystemWorkSize( PAL_TCB_MAX ) );
  wk->TcbSys      = GFL_TCB_Init(  PAL_TCB_MAX , wk->TcbWorkArea );
  wk->palAnmTcb   = GFL_TCB_AddTask( wk->TcbSys, MailView_PltAnime, wk, 0);
 
}

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画リソース解放
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int MailViewResRelease(MAIL_VIEW_DAT* wk)
{
  MailView_PmsExit(wk);
  MailView_PokeIconRelease(wk);
  if(wk->pMsg != NULL){
    GFL_MSG_Delete( wk->pMsgData );
  }
  APP_TASKMENU_RES_Delete( wk->menures );

  MailView_BmpWinRelease(wk);
  MailView_2DGraRelease(wk);
  MailView_BGLRelease(wk);
  return TRUE;
}

#define PMS_WORD_CLACT_MAX    ( 6 )

//----------------------------------------------------------------------------------
/**
 * @brief 簡易会話描画システム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_PmsInit( MAIL_VIEW_DAT *wk )
{
  
  wk->pmsPrintque = PRINTSYS_QUE_Create( wk->heapID );
  wk->pmsClunit   = GFL_CLACT_UNIT_Create( PMS_WORD_CLACT_MAX, 1, wk->heapID );
//  GFL_CLACT_UNIT_SetDefaultRend( wk->clUnit );

  wk->pms_draw_work = PMS_DRAW_Init( wk->pmsClunit, CLSYS_DRAW_MAIN, 
                                     wk->pmsPrintque, wk->font, 0, 3, wk->heapID );
  // PMS_DRAW_OBJ_PLTT_NUM
}

//----------------------------------------------------------------------------------
/**
 * @brief 簡易会話描画システム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_PmsExit( MAIL_VIEW_DAT *wk )
{
  PMS_DRAW_Exit( wk->pms_draw_work );
  PRINTSYS_QUE_Delete( wk->pmsPrintque );
  GFL_CLACT_UNIT_Delete( wk->pmsClunit );
}


static const GFL_DISP_VRAM vramSetTable = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,         // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K,
    GX_OBJVRAMMODE_CHAR_1D_32K,
  
};

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画Vramバンク初期化
 *
 * @param   none    
 */
//----------------------------------------------------------------------------------
static void MailView_VBankSet(void)
{
  GFL_DISP_SetBank( &vramSetTable );
}


//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　BGL初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_BGLInit(MAIL_VIEW_DAT* wk)
{
  int i,frame;
  
  //VramBankセット
  MailView_VBankSet();

  { //BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D
    };
    GFL_BG_SetBGMode(&BGsys_data);
  }
  //上画面をサブ,下画面をメインに切り替え
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
  
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
    { //MAIN BG0
      0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800,GX_BG_CHARBASE_0x08000,GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01,0,0,0,FALSE},
    { //MAIN BG1
      0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000,GX_BG_CHARBASE_0x10000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 1,0,0,FALSE},
    { //MAIN BG2
      0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800,GX_BG_CHARBASE_0x00000,GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 2,0,0,FALSE},
    { //MAIN BG3
      0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000,GX_BG_CHARBASE_0x00000,GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 3,0,0,FALSE},
    { //SUB BG0
      0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800,GX_BG_CHARBASE_0x00000,GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 0,0,0,FALSE},
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M,&(TextBgCntDat[0]),GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME1_M,&(TextBgCntDat[1]),GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME2_M,&(TextBgCntDat[2]),GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME3_M,&(TextBgCntDat[3]),GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S,&(TextBgCntDat[4]),GFL_BG_MODE_TEXT );
  }
  GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
  GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
  GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
  GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
  GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M,32,0,wk->heapID);
  GFL_BG_SetClearCharacter( GFL_BG_FRAME1_M,32,0,wk->heapID);
  GFL_BG_SetClearCharacter( GFL_BG_FRAME2_M,32,0,wk->heapID);
  GFL_BG_SetClearCharacter( GFL_BG_FRAME3_M,32,0,wk->heapID);
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S,32,0,wk->heapID);
}



//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　BGL解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_BGLRelease(MAIL_VIEW_DAT* wk)
{
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );  
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );  
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );  
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );  
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
  
  //上画面をメイン,下画面をサブに切り替え
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
}


//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　BGリソース初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_2DGraInit(MAIL_VIEW_DAT* wk)
{
  void* tmp;
  u32 size;
  void* pSrc;
  NNSG2dCharacterData* pChar;
  NNSG2dPaletteData*  pPal;
  int charID,scrnID,plttID;
  ARCHANDLE* handle;
  
  //デザインNoからグラフィックリソースID取得
  charID = MAILD_CHAR_S+wk->dat->design;
  scrnID = MAILD_SCRN_S+wk->dat->design;
  plttID = MAILD_PLTT_S+wk->dat->design;

  OS_Printf("mail design = %d\n", wk->dat->design);
  OS_Printf("mail char = %d\n", charID);
  OS_Printf("mail pltt = %d\n", plttID);
  OS_Printf("mail scrn = %d\n", scrnID);

  //アーカイブのハンドルを取得
  handle = GFL_ARC_OpenDataHandle( ARCID_MAIL_GRA, wk->heapID );


  // メニューウィンドウセット
  BmpWinFrame_GraphicSet( BMPL_WIN_FRM, BMPL_MENU_WIN_CGX, BMPL_MENU_WIN_PAL,
                          MENU_TYPE_SYSTEM, wk->heapID );

  // 会話ウィンドウセット
  TalkWinFrame_GraphicSet( BMPL_WIN_FRM, BMPL_TALK_WIN_CGX, BMPL_TALK_WIN_PAL, 
                           wk->win_type, wk->heapID);


  // BG転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, charID, GFL_BG_FRAME3_M, 0,0,1, wk->heapID );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, charID, GFL_BG_FRAME0_S, 0,0,1, wk->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( handle,scrnID, GFL_BG_FRAME3_M, 0, 0x800,1,wk->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( handle,scrnID, GFL_BG_FRAME0_S, 0, 0x800,1,wk->heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, plttID, PALTYPE_SUB_BG, 0,0, wk->heapID);
//  GFL_ARCHDL_UTIL_TransVramPalette( handle, plttID, PALTYPE_MAIN_BG, 0,0, wk->heapID);
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 5*32,32, wk->heapID);

/*  
  size = ArchiveDataSizeGetByHandle(handle,charID);
  pSrc = sys_AllocMemoryLo(wk->heapID,size);
  ArchiveDataLoadByHandle(handle,charID,(void*)pSrc);
  
  NNS_G2dGetUnpackedCharacterData(pSrc,&pChar); 
  GFL_BG_LoadCharacter(wk->bgl,GFL_BG_FRAME1_M,
      pChar->pRawData,pChar->szByte,0);
  GFL_BG_LoadCharacter(wk->bgl,GFL_BG_FRAME0_S,
      pChar->pRawData,pChar->szByte,0);
  GFL_HEAP_FreeMemory(pSrc);
  //パレット転送
  size = ArchiveDataSizeGetByHandle(handle,plttID);
  pSrc = sys_AllocMemoryLo(wk->heapID,size);
  ArchiveDataLoadByHandle(handle,plttID,(void*)pSrc);

  NNS_G2dGetUnpackedPaletteData(pSrc,&pPal);
//  GFL_BG_PaletteSet(GFL_BG_FRAME0_M,pPal->pRawData,pPal->szByte,0);
  GFL_BG_PaletteSet(GFL_BG_FRAME0_S,pPal->pRawData,pPal->szByte,0);
*/

  //パレットアニメコントローラー確保
  wk->palAnm = PaletteFadeInit(wk->heapID);
  //リクエストデータをmallocしてセット
  // MAIN BGパレット6列確保
  PaletteFadeWorkAllocSet( wk->palAnm, FADE_MAIN_BG,  FADE_PAL_ONE_SIZE*MAILVIEW_PALMAX, wk->heapID);
  // MAIN OBJパレット3列確保
  PaletteFadeWorkAllocSet( wk->palAnm, FADE_MAIN_OBJ, FADE_PAL_ONE_SIZE*3,wk->heapID);
  // MAIN OBJパレット3列確保
  PaletteWorkSet_ArcHandle( wk->palAnm, handle, plttID, wk->heapID, FADE_MAIN_BG,FADE_PAL_ONE_SIZE*3,0 );

//  if(wk->mode == MAIL_MODE_CREATE){
//    //ウィンドウフォントパレットを入れ替え(3列目のデータを1列目にコピー）
//    PaletteWorkCopy( wk->palAnm, FADE_MAIN_BG, 16*3, FADE_MAIN_BG, 16, 32 );
//  }
  // ポケモンアイコン用パレット転送( 0列目から3列転送
  PaletteWorkSet_Arc(wk->palAnm,ARCID_POKEICON,0,wk->heapID, FADE_MAIN_OBJ, FADE_PAL_ONE_SIZE*3,0);

  //フォント＆ウィンドウ用パレットセット
  // 3列目にシステムウインドウパレット転送
  PaletteWorkSet_Arc(wk->palAnm,ARCID_FONT,NARC_font_systemwin_nclr,wk->heapID, 
    FADE_MAIN_BG,FADE_PAL_ONE_SIZE,16*MENU_FONT_PAL);
  // 5列目に会話フォントパレット転送
  PaletteWorkSet_Arc(wk->palAnm,ARCID_FONT,NARC_font_default_nclr,wk->heapID, 
    FADE_MAIN_BG,FADE_PAL_ONE_SIZE,16*TALK_FONT_PAL);
  // 4列目にメニューウインドウパレット転送
  PaletteWorkSet_Arc(wk->palAnm,ARCID_FLDMAP_WINFRAME,NARC_winframe_system_NCLR,wk->heapID, 
    FADE_MAIN_BG,FADE_PAL_ONE_SIZE,16*BMPL_MENU_WIN_PAL);
  // 6列目に会話ウインドウパレット転送
  PaletteWorkSet_Arc(wk->palAnm,ARCID_FLDMAP_WINFRAME,NARC_winframe_talk_win00_NCLR + wk->win_type,wk->heapID, 
    FADE_MAIN_BG,FADE_PAL_ONE_SIZE,16*BMPL_TALK_WIN_PAL);



  //ハンドル解放
  GFL_ARC_CloseDataHandle( handle );

  //初期スクリーン転送
  GFL_BG_FillScreen( GFL_BG_FRAME0_S,0x2001,0,0,32,32,GFL_BG_SCRWRT_PALIN );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S);

  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, TRUE );
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, TRUE );
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, TRUE );

  // 閲覧モードならここまでで終了
  if(wk->mode == MAIL_MODE_VIEW){
    return;
  }
  // BG2面スクリーン転送
  handle = GFL_ARC_OpenDataHandle( ARCID_MAIL_GRA, wk->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( handle,MAILD_SCRN_FIL, GFL_BG_FRAME2_M, 0, 0x800,1,wk->heapID );
  GFL_ARC_CloseDataHandle( handle );
  
  //キータッチ切替のコールバックを登録
  wk->to_key_func   = inchg2key_create;
  wk->to_touch_func = inchg2touch_create;

  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2, TRUE );


}


//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　BGリソース解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_2DGraRelease(MAIL_VIEW_DAT* wk)
{

  // パレットフェード開放
  PaletteFadeWorkAllocFree( wk->palAnm, FADE_MAIN_OBJ );
  PaletteFadeWorkAllocFree( wk->palAnm, FADE_MAIN_BG );

  //パレットフェードシステム開放
  PaletteFadeFree( wk->palAnm );
  wk->palAnm = NULL;


  GFL_TCB_DeleteTask( wk->palAnmTcb );
  GFL_TCB_Exit( wk->TcbSys );
  GFL_HEAP_FreeMemory( wk->TcbWorkArea );
  
  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,GX_BLEND_PLANEMASK_NONE,31,0);
}

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　BMPWin初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_BmpWinInit(MAIL_VIEW_DAT* wk)
{
  int i = 0;

  GFL_BMPWIN_Init( wk->heapID );

  // 0キャラ目を潰す
  wk->win[WIN_SPACE_M] = GFL_BMPWIN_Create( BMPL_MSG_FRM, 0, 0, 
                                        1, 1, BMPL_TALK_PAL,
                                        GFL_BMP_CHRAREA_GET_F );
  // 簡易会話０
  wk->win[WIN_M01] = GFL_BMPWIN_Create( BMPL_MSG_FRM, BMPL_MSG_PX, BMPL_M01_PY, 
                                       BMPL_MSG_SX, BMPL_MSG_SY, BMPL_MSG_PAL,
                                       GFL_BMP_CHRAREA_GET_F );

  // 簡易会話１
  wk->win[WIN_M02] = GFL_BMPWIN_Create( BMPL_MSG_FRM, BMPL_MSG_PX, BMPL_M02_PY, 
                                       BMPL_MSG_SX, BMPL_MSG_SY, BMPL_MSG_PAL,
                                       GFL_BMP_CHRAREA_GET_F );

  // 簡易会話２
  wk->win[WIN_M03] = GFL_BMPWIN_Create( BMPL_MSG_FRM, BMPL_MSG_PX, BMPL_M03_PY, 
                                       BMPL_MSG_SX, BMPL_MSG_SY, BMPL_MSG_PAL,
                                       GFL_BMP_CHRAREA_GET_F );

  // メール持ち主
  wk->win[WIN_NAME] = GFL_BMPWIN_Create( BMPL_MSG_FRM, BMPL_YN_PX, BMPL_YN02_PY,
                                        BMPL_YN_SX, BMPL_YN_SY, BMPL_YN_PAL,
                                        GFL_BMP_CHRAREA_GET_F );

  // 会話ウインドウ
  wk->win[WIN_TALK] = GFL_BMPWIN_Create( BMPL_TALK_FRM, BMPL_TALK_PX, BMPL_TALK_PY, 
                                        BMPL_TALK_SX, BMPL_TALK_SY, BMPL_TALK_PAL,
                                        GFL_BMP_CHRAREA_GET_B );

  // 会話ウインドウ関連付け
  for(i=0;i<WIN_MAX-1;i++){
    PRINT_UTIL_Setup( &wk->printUtil[i], wk->win[i] );
  }
  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

  // 領域クリア & 表示ＯＮ
  for(i = 0;i < WIN_MAX;i++){
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[i]), 0 );
    if(i < WIN_NAME){
      GFL_BMPWIN_MakeTransWindow( wk->win[i] );
    }
  }


}

//----------------------------------------------------------------------------------
/**
 * @brief 「けってい・やめる」メニュー作成
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_MenuMake( MAIL_VIEW_DAT* wk )
{
  // メール作成時は「けってい」「やめる」メニュー表示
  if(wk->mode == MAIL_MODE_CREATE){
    wk->menuitem[0].str      = GFL_MSG_CreateString( wk->pMsgData, msg_mail_decide ); //メニューに表示する文字列
    wk->menuitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->menuitem[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示
  
    wk->menuitem[1].str      = GFL_MSG_CreateString( wk->pMsgData, msg_mail_cancel ); //メニューに表示する文字列
    wk->menuitem[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->menuitem[1].type     = APP_TASKMENU_WIN_TYPE_RETURN; //戻るマークの表示


    // 下メニュー作成
    wk->menuwork[0] = APP_TASKMENU_WIN_Create( wk->menures, &wk->menuitem[0], 
                                                  BMPL_YN_PX, BMPL_YN01_PY, BPML_YN_W, wk->heapID );
    wk->menuwork[1] = APP_TASKMENU_WIN_Create( wk->menures, &wk->menuitem[1], 
                                                  BMPL_YN_PX, BMPL_YN02_PY, BPML_YN_W, wk->heapID );
    
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画  BmpWin解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_BmpWinRelease(MAIL_VIEW_DAT* wk)
{
  int i = 0;

  if(wk->mode == MAIL_MODE_CREATE){
    // 下メニュー解放
    APP_TASKMENU_WIN_Delete( wk->menuwork[0] );
    APP_TASKMENU_WIN_Delete( wk->menuwork[1] );
    GFL_STR_DeleteBuffer( wk->menuitem[0].str );
    GFL_STR_DeleteBuffer( wk->menuitem[1].str );
  }

  PRINTSYS_QUE_Delete( wk->printQue );

  for(i = 0;i < WIN_MAX;i++){
    GFL_BMPWIN_ClearTransWindow( wk->win[i] );
    GFL_BMPWIN_Delete( wk->win[i] );
  }

  GFL_BMPWIN_Exit( );
}

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　簡易文描画
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_MsgWrite(MAIL_VIEW_DAT* wk)
{
  int i = 0;
  STRBUF* str;
  
  
  //メッセージ描画
  for(i = 0;i < MAILDAT_MSGMAX;i++){
    if(!PMSDAT_IsEnabled(&wk->dat->msg[i])){
      continue;
    }
//  @TODO 新簡易会話ライブラリを使用して描画する
    PMS_DRAW_Print( wk->pms_draw_work, wk->win[WIN_M01+i], &wk->dat->msg[i], i );

  }

  //メール描画
  if(wk->mode == MAIL_MODE_VIEW){ //メール閲覧
    //ライター名表示
    PRINT_UTIL_PrintColor( &wk->printUtil[WIN_NAME], wk->printQue, 0,2, wk->dat->name, wk->font, VIEW_NRM_FCOL );
    GFL_BMPWIN_MakeTransWindow( wk->win[WIN_NAME] );
  }
}


enum{
  OBJ_POKEICON_PAL=0,
};
enum{
  OBJ_POKEICON0_CGR=0,
  OBJ_POKEICON1_CGR,
  OBJ_POKEICON2_CGR,
  OBJ_PMSOBJ_CGR,
  OBJ_POKEICON_MAX,
};
enum{
  OBJ_POKEICON_CEL=0,
};

enum{
  RES_CGR=0,
  RES_PAL,
  RES_CELANM,
};

//----------------------------------------------------------------------------------
/**
 * @brief メール描画　ポケモンアイコン追加
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_PokeIconInit(MAIL_VIEW_DAT* wk)
{
  int i = 0;
  ARCHANDLE *i_handle;
  //-----------セルアクター初期化----------------

  //通信中なら通信アイコン復帰
  GFL_NET_WirelessIconEasy_HoldLCD( TRUE, wk->heapID); //通信アイコン
  
  {
    static const GFL_CLSYS_INIT clsys_init  =
    {
      0, 0,   //メインサーフェースの左上X,Y座標
      0, 512, //サブサーフェースの左上X,Y座標
      4, 40,  //メインOAM管理開始～終了 （通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
      4, 40,  //差bOAM管理開始～終了  （通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
      0,      //セルVram転送管理数
      OBJ_POKEICON_MAX, OBJ_POKEICON_MAX, OBJ_POKEICON_MAX, 32,
      16, 16,       //メイン、サブのCGRVRAM管理領域開始オフセット（通信アイコン用に16以上にしてください）
    };
  
  
    //システム作成
    GFL_CLACT_SYS_Create( &clsys_init, &vramSetTable, wk->heapID );
    wk->clUnit = GFL_CLACT_UNIT_Create( MAILDAT_ICONMAX, 0, wk->heapID );
    GFL_CLACT_UNIT_SetDefaultRend( wk->clUnit );
  
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  }

  //----------リソース追加-------------

  // ポケモンアイコン用アーカイブハンドルオープン
  i_handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, wk->heapID );


  // Pltt
  wk->clres[RES_PAL][OBJ_POKEICON_PAL] = GFL_CLGRP_PLTT_RegisterComp(
                                i_handle, POKEICON_GetPalArcIndex(),
                                CLSYS_DRAW_MAIN, 0, wk->heapID );
  // CellAnm
  wk->clres[RES_CELANM][OBJ_POKEICON_CEL] = GFL_CLGRP_CELLANIM_Register(
                                      i_handle, POKEICON_GetCellArcIndex(),
                                      POKEICON_GetAnmArcIndex(), wk->heapID );
  // Char
  for(i = 0;i < MAILDAT_ICONMAX;i++){
    if(wk->dat->icon[i].dat == MAIL_ICON_NULL)
    { // アイコン無い場合は登録しない
      break;
    }
    wk->clres[RES_CGR][OBJ_POKEICON0_CGR+i] = GFL_CLGRP_CGR_Register( i_handle,
                                    wk->dat->icon[i].cgxID, 0,
                                    CLSYS_DRAW_MAIN, wk->heapID );

  }
  GFL_ARC_CloseDataHandle( i_handle );

  // ----------セルアクター登録-------------

  for(i=0;i<MAILDAT_ICONMAX;i++){
    GFL_CLWK_DATA dat;
    if(wk->dat->icon[i].dat == MAIL_ICON_NULL){ // アイコン無い場合は登録しない
      break;
    }
    dat.pos_x   = ICONVIEW_PX-(ICONVIEW_WIDTH*i); ///< [ X ] 座標
    dat.pos_y   = ICONVIEW_PY;                    ///< [ Y ] 座標
    dat.anmseq  = 0;                  ///< アニメ番号
    dat.softpri = 0;
    dat.bgpri   = MAILVIEW_ICON_PRI;

    wk->clwk[i] = GFL_CLACT_WK_Create( wk->clUnit, wk->clres[RES_CGR][OBJ_POKEICON0_CGR+i], 
                                               wk->clres[RES_PAL][OBJ_POKEICON_PAL],
                                               wk->clres[RES_CELANM][OBJ_POKEICON_CEL], &dat, 
                                               CLSYS_DEFREND_MAIN, wk->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], FALSE);


    // メール作成モードではポケモンアイコンを表示させない
    // (CLACT初期化共通化のため設定はしている)
    if(wk->mode == MAIL_MODE_VIEW){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], TRUE );
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
    }
  }


}

//----------------------------------------------------------------------------------
/**
 * @brief   メール描画　ポケモンアイコン解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void MailView_PokeIconRelease(MAIL_VIEW_DAT* wk)
{
  int i;

  // 閲覧モードのみ表示しているので
  for(i=0; i<MAILDAT_ICONMAX; i++){
      if(wk->clwk[i] != NULL){
        GFL_CLACT_WK_Remove( wk->clwk[i] );
        GFL_CLGRP_CGR_Release( wk->clres[RES_CGR][OBJ_POKEICON0_CGR+i] );
      }
  }
  GFL_CLGRP_CELLANIM_Release( wk->clres[RES_PAL][OBJ_POKEICON_PAL]);
  GFL_CLGRP_PLTT_Release( wk->clres[RES_CELANM][OBJ_POKEICON_CEL] );
 
  //システム破棄
  GFL_CLACT_UNIT_Delete( wk->clUnit );
  GFL_CLACT_SYS_Delete();

}

