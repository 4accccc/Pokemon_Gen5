/*
 *  @file   beacon_view_common.c
 *  @brief  すれ違いログ＆フェスミッションログ画面　共有サブルーチン群
 *  @author Miyuki Iwasawa
 *  @date   10.10.20
 */

#include <gflib.h>
#include "system/main.h"
#include "system/bmp_oam.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_accessor.h"
#include "field/field_subscreen.h"
#include "sound/pm_sndsys.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "arc_def.h"
#include "font/font.naix"
#include "field/beacon_view.h"
#include "print\printsys.h"
#include "message.naix"

#include "field/research_team_def.h"
//#include "beacon_status.naix"
//#include "wifi_unionobj.naix"
//#include "beacon_view_local.h"
#include "beacon_view_common.h"
#include "msg/msg_beacon_view_common.h"
#include "msg/msg_beacon_status.h"

//#include "arc/wifi_unionobj_plt.cdat"

//タスクメニュー位置
#define TMENU_PY (21)
#define TMENU_W  (8)
#define TMENU_H  (APP_TASKMENU_PLATE_HEIGHT)

#define TMENU_ITEM3_PX (8)
#define TMENU_ITEM2_PX (16)

typedef struct TMENU_INIT_PARAM{
 u8 max;
 u8 px;
 u8 str_id[TMENU_ITEM_MAX];
}TMENU_INIT_PARAM;

//=====================================================================
//三角形の当たり判定
//=====================================================================
/*
 *  @brief  外積計算
 */
static int calc_Cross( POINT* a, POINT* b ) 
{
  return (a->x*b->y - a->y*b->x);
}

/*
 *  @brief  POINTの減算
 */
static void calc_PointMinus( POINT* a, POINT* b, POINT* out )
{
  out->x = a->x-b->x;
  out->y = a->y-b->y;
}

/*
 *  @brief  三角形あたり判定
 *
 *  三角形は反時計回り
 * 1-0 p-a, b-a
 * 2-1 p-b, c-b
 * 0-2 p-c, a-c
 */
BOOL BVCCalc_PointInTriangle( POINT* p, TRIANGLE* tri )
{
  int i;
  POINT d1,d2;

  for( i = 0;i < 3;i++ ){
    calc_PointMinus( p, &tri->p[i], &d1 );
    calc_PointMinus( &tri->p[(1+i)%3], &tri->p[i], &d2 );
    if( calc_Cross( &d1, &d2 ) < 0 ){
      return FALSE;
    }
  }
  return TRUE;
}

/*
 *  @brief  SE再生
 */
void BVCSound_PlaySE( u16 se_no )
{
  PMSND_PlaySE( se_no );
}

/*
 *  @brief  アクター　座標セット
 */
void BVCAct_SetPosition( GFL_CLWK* act, s16 x, s16 y )
{
  GFL_CLACTPOS pos;
  pos.x = x;
  pos.y = y;
  
  GFL_CLACT_WK_SetPos( act, &pos, ACT_RENDER_ID);
}

/*
 *  @brief  アクター アニメスタート
 */
void BVCAct_AnmStart( GFL_CLWK* act, u8 anm_no )
{
  GFL_CLACT_WK_SetAnmSeq( act, anm_no );
  GFL_CLACT_WK_ResetAnm( act );
}

/*
 *  @brief  プリントキュー　処理スピードアップリクエスト
 *
 *  処理分割数を減らし、処理負荷を上げる代わりに、
 *  プリント処理終了までのスピードをアップする
 */
void BVC_PrintQueLimmitUpSet( PRINT_QUE *printQue, BOOL flag )
{
  if( flag ){ //スピードアップ
    PRINTSYS_QUE_SetLimitTick( printQue, PRINT_QUE_LIMMIT_UP );
  }else{  //デフォルトに戻す
    PRINTSYS_QUE_SetLimitTick( printQue, PRINT_QUE_LIMMIT_DEFAULT );
  }
}



//========================================================================
//選択メニュー処理系
//========================================================================

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu リソース初期化
 */
//---------------------------------------------------------------
BVC_TMENU_WORK* BVCTMenu_Init( u8 frame, u8 plt, GFL_FONT *fontHandle, PRINT_QUE *printQue, HEAPID heapID )
{
  int i;
  GFL_MSGDATA* msg;

  BVC_TMENU_WORK* wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(BVC_TMENU_WORK));

  wk->heapID = heapID;
	
  msg = GFL_MSG_Create(
		GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_beacon_view_common_dat, GFL_HEAP_LOWID(heapID) );

  wk->menuRes = APP_TASKMENU_RES_Create( frame, plt, fontHandle, printQue, heapID );

  for( i = 0; i < TMENU_STR_MAX;i++){
    wk->str[i] = GFL_MSG_CreateString( msg, msg_bvc_tmenu_01+i ); //メニューに表示する文字列
  
  }
  //アイテム初期化
  for( i = 0;i < TMENU_ITEM_MAX;i++){
    wk->tmenu[i].item.str      = NULL;
    wk->tmenu[i].item.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->tmenu[i].item.type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示
    wk->tmenu[i].work = NULL;
  }
  GFL_MSG_Delete( msg );
 
  return wk;
}

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu リソース開放
 */
//---------------------------------------------------------------
void BVCTMenu_Release( BVC_TMENU_WORK* wk )
{
  int i;

  for( i = 0;i < TMENU_ITEM_MAX; i++){
    if( wk->tmenu[i].work != NULL ){
      APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
    }
  }
  for( i = 0;i < TMENU_STR_MAX;i++){
    GFL_STR_DeleteBuffer( wk->str[i] );
  }
  APP_TASKMENU_RES_Delete( wk->menuRes );
  
  GFL_HEAP_FreeMemory( wk );
}

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu YesNo生成
 */
//---------------------------------------------------------------
void BVCTMenu_Create( BVC_TMENU_WORK* wk, u8 menu_id )
{
  int i;

  static const TMENU_INIT_PARAM prm[TMENU_ID_MAX] = {
    { 3, TMENU_ITEM3_PX, { TMENU_STR_CHECK, TMENU_STR_YES, TMENU_STR_NO } }, //CYN
    { 2, TMENU_ITEM2_PX, { TMENU_STR_YES, TMENU_STR_NO } }, //YN
    { 2, TMENU_ITEM2_PX, { TMENU_STR_CHECK, TMENU_STR_CLOSE } }, //CHECK
  };

  wk->c_id = menu_id;
  wk->c_max = prm[ wk->c_id ].max;

  for( i = 0;i < wk->c_max; i++ ){
    wk->tmenu[i].item.str = wk->str[ prm[wk->c_id].str_id[i] ];
    wk->tmenu[i].work = APP_TASKMENU_WIN_CreateEx( wk->menuRes, &wk->tmenu[i].item, 
                        prm[wk->c_id].px+TMENU_W*i, TMENU_PY, TMENU_W, TMENU_H, TRUE, FALSE , wk->heapID );
  }
}

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu ウィンドウワークを取得する
 */
//---------------------------------------------------------------
APP_TASKMENU_WIN_WORK* BVCTMenu_GetWinWork( BVC_TMENU_WORK* wk, u8 select )
{
  return wk->tmenu[select].work;
}

/*
 *  @brief  TaskMenu アップデート
 */
int BVCTMenu_Update( BVC_TMENU_WORK* wk )
{
  int i,ret = -1;

  for( i = 0; i < wk->c_max; i++ ){
    //まだ書き込み中
    if( APP_TASKMENU_WIN_IsUpdateMsg( wk->tmenu[i].work ) == TRUE){
      APP_TASKMENU_WIN_Update( wk->tmenu[i].work );
      return ret;
    }
  }
  for( i = 0; i < wk->c_max; i++ ){
    if( APP_TASKMENU_WIN_IsTrg( wk->tmenu[i].work )){
      if(i < (wk->c_max-1)){
        BVCSound_PlaySE( SEQ_SE_DECIDE1 );
      }else{
        APP_TASKMENU_WIN_SetDecide( wk->tmenu[i].work, TRUE );
        BVCSound_PlaySE( SEQ_SE_DECIDE1 );
      }
      ret = i;
    }
    APP_TASKMENU_WIN_Update( wk->tmenu[i].work );
  }
  return ret;
}

/*
 *  @brief  TaskMenu 終了アニメ待ち
 */
BOOL BVCTMenu_EndWait( BVC_TMENU_WORK* wk, u8 select )
{
  int i;

  APP_TASKMENU_WIN_Update( wk->tmenu[select].work );

  if( APP_TASKMENU_WIN_IsFinish( wk->tmenu[select].work )){
    for( i = 0;i < wk->c_max; i++){
      APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
      wk->tmenu[i].work = NULL;
    }
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  TaskMenu 終了
 */
void BVCTMenu_End( BVC_TMENU_WORK* wk )
{
  int i;

  for( i = 0;i < wk->c_max; i++){
    APP_TASKMENU_WIN_Hide( wk->tmenu[i].work );
    APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
    wk->tmenu[i].work = NULL;
    wk->tmenu[i].item.str = NULL;
  }
  wk->c_id = 0;
  wk->c_max = 0;
}

//------------------------------------------------------------
/*
 *  @brief  ポップアップ　タイムウェイトチェック
 */
//------------------------------------------------------------
BOOL BVCPrint_PopupWindowTimeWaitCheck( u8* p_wait, u8 frame )
{
  u32 x,y;
  u16 pat = 0xFFFE; 

  if( (*p_wait)-- == 0 ){ //タイムアウト
    *p_wait = 0;
    return TRUE; 
  }
 
  //タッチチェック
  if( GFL_UI_TP_GetPointTrg( &x, &y )){
    //スクロールのため、スクリーンが実座標+64dotの位置にあるので、ちょっと位置補正してチェック
    if( GFL_BG_CheckDot( frame, x, y+64, &pat )){
      *p_wait = 0;
      return TRUE; 
    }
  }
  return FALSE;
}

//------------------------------------------------------------
/**
 * ビーコン情報から表示するメッセージIDを取得する
 *
 * @param   info		対象のビーコン情報へのポインタ
 */
//------------------------------------------------------------
u32 BVCPrint_GetBeaconMsgID(const GAMEBEACON_INFO *info)
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo(info);

  if(action == GAMEBEACON_ACTION_NULL || action >= GAMEBEACON_ACTION_MAX){
    GF_ASSERT_MSG(0, "action_no = %d\n", action);
    return msg_beacon_001;
  }
  return msg_beacon_001 + action - GAMEBEACON_ACTION_SEARCH;
}

//------------------------------------------------------------
/**
 * FontOam構造体の初期化
 *
 * @param FONT_OAM*
 * @param BMPOAM_ACT_DATA*
 * @param bsp BMPOAM_SYS_PTR
 * @param printQue  PRINT_QUE*
 * @param fontHandle  GFL_FONT*
 * @param pTcbSys  GFL_TCBLSYS*
 */
//------------------------------------------------------------
void BVCPrint_FontOamAdd( FONT_OAM* foam, BMPOAM_ACT_DATA* finit, BMPOAM_SYS_PTR bsp, PRINT_QUE* que, GFL_FONT* font, GFL_TCBLSYS* tcbsys )
{
  foam->bmp = finit->bmp;
  foam->printQue = que;
  foam->fontHandle = font;
  foam->pTcbSys = tcbsys;
  foam->oam = BmpOam_ActorAdd( bsp, finit );
}

//------------------------------------------------------------
/**
 * FontOam構造体の削除
 *
 * @param FONT_OAM*
 */
//------------------------------------------------------------
void BVCPrint_FontOamDel( FONT_OAM* foam )
{
  BmpOam_ActorSetDrawEnable( foam->oam, FALSE );
  BmpOam_ActorDel( foam->oam );
	GFL_BMP_Delete( foam->bmp );

  MI_CpuClear8( foam, sizeof(FONT_OAM));
}


/////////////////////////////////////////////////////////////////////
/*
 *  @brief  BMPウィンドウメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_BMPWIN_PRINT{
  u8  seq;
  BMP_WIN* win;
  
  int* task_ct;
}TASKWK_BMPWIN_PRINT;

static void taskAdd_BmpwinPrint( BMP_WIN* win, int* task_ct );
static void tcb_BmpwinPrint( GFL_TCBL *tcb , void* work);

void BVCPrintReq_BmpwinPrintEx( BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct, BOOL direct_f, BOOL clear_f )
{
  if( clear_f ){
    GFL_BMP_Clear( win->bmp, clear );
  }
  if( direct_f ){
	  PRINTSYS_PrintColor( win->bmp, x, y, str, win->fontHandle, color );
    GFL_BMPWIN_MakeTransWindow( win->win );
  }else{
    PRINT_UTIL_PrintColor( &win->putil, win->printQue, x, y, str, win->fontHandle, color );
    taskAdd_BmpwinPrint( win, task_ct );
  }
}

void BVCPrintReq_BmpwinPrint( BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct, BOOL direct_f )
{
  BVCPrintReq_BmpwinPrintEx( win, str, x, y, clear, color, task_ct, direct_f, TRUE);
}

/*
 *  @brief  メッセージウィンドウ パネルフラッシュタスク登録
 */
static void taskAdd_BmpwinPrint( BMP_WIN* win, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_BMPWIN_PRINT* twk;

  tcb = GFL_TCBL_Create( win->pTcbSys, tcb_BmpwinPrint, sizeof(TASKWK_BMPWIN_PRINT), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_BMPWIN_PRINT ));

  twk->win = win;

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_BmpwinPrint( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_BMPWIN_PRINT* twk = (TASKWK_BMPWIN_PRINT*)tcb_wk;

  if( !PRINT_UTIL_Trans( &twk->win->putil, twk->win->printQue )){
    return;
  }
  GFL_BMPWIN_MakeScreen( twk->win->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( twk->win->win ) );

  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  FontOam BMPウィンドウメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_FOAM_PRINT{
  u8  seq;
  FONT_OAM* foam;

  int* task_ct;
}TASKWK_FOAM_PRINT;

static void taskAdd_FOamPrint( FONT_OAM* foam, int* task_ct );
static void tcb_FOamPrint( GFL_TCBL *tcb , void* work);

void BVCPrintReq_FOamPrintEx( FONT_OAM* foam, STRBUF* str, PRINTSYS_LSB color, int* task_ct, BOOL direct_f, BOOL clear_f )
{
  if( clear_f ){
    GFL_BMP_Clear( foam->bmp, 0 );
  }
  if( direct_f ){
	  PRINTSYS_PrintColor( foam->bmp, 0, 0, str, foam->fontHandle, color );
 	  BmpOam_ActorBmpTrans( foam->oam );
  }else{
    PRINTSYS_PrintQueColor( foam->printQue, foam->bmp, 0, 0, str, foam->fontHandle, color );
    taskAdd_FOamPrint( foam, task_ct );
  }
}

void BVCPrintReq_FOamPrint( FONT_OAM* foam, STRBUF* str, PRINTSYS_LSB color, int* task_ct, BOOL direct_f )
{
  BVCPrintReq_FOamPrintEx( foam, str, color, task_ct, direct_f, TRUE);
}

/*
 *  @brief  メッセージウィンドウ パネルフラッシュタスク登録
 */
static void taskAdd_FOamPrint( FONT_OAM* foam,int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_FOAM_PRINT* twk;

  tcb = GFL_TCBL_Create( foam->pTcbSys, tcb_FOamPrint, sizeof(TASKWK_FOAM_PRINT), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_FOAM_PRINT ));

  twk->foam = foam;

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_FOamPrint( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_FOAM_PRINT* twk = (TASKWK_FOAM_PRINT*)tcb_wk;
  
  if( PRINTSYS_QUE_IsExistTarget( twk->foam->printQue, twk->foam->bmp ) ){
    return;
  }
	BmpOam_ActorBmpTrans( twk->foam->oam );

  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ アップダウン 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_MSG_UPDOWN{
  u8  dir;
  u8  frame;
  u8  wait;
  u8  ct;
  s8  y;
  s8  diff;

  int* task_ct;
}TASKWK_MSG_UPDOWN;

static void tcb_MsgUpdown( GFL_TCBL *tcb , void* work);

/*
 *  @brief  メッセージウィンドウ アップダウンタスク登録
 */
void BVCTaskAdd_MsgUpdown(   GFL_TCBLSYS*  pTcbSys, u8 frame, u8 dir, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_MSG_UPDOWN* twk;

  tcb = GFL_TCBL_Create( pTcbSys, tcb_MsgUpdown, sizeof(TASKWK_MSG_UPDOWN), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_MSG_UPDOWN ));

  twk->frame = frame;
  twk->dir = dir;
  if( twk->dir == SCROLL_UP ){
    twk->y = 0;
    twk->diff = POPUP_DIFF;
  }else{
    twk->y = POPUP_HEIGHT;
    twk->diff = -POPUP_DIFF;
  }
  twk->wait = POPUP_COUNT;

//  IWASAWA_Printf("MsgWin UpDown Req %d\n", twk->dir );
  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_MsgUpdown( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_MSG_UPDOWN* twk = (TASKWK_MSG_UPDOWN*)tcb_wk;
 
  if( twk->ct++ < twk->wait ){
    twk->y += twk->diff;
    GFL_BG_SetScroll( twk->frame, GFL_BG_SCROLL_Y_SET, twk->y );
    return;
  }
//  IWASAWA_Printf("MsgWin UpDown End %d\n", twk->dir );

  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}


