//============================================================================================
/**
 * @file  oekaki.c
 * @brief お絵かきボード画面処理
 * @author  Akito Mori(移植）
 * @date    09.01.20
 *
 * @todo
 *   ・接続制限の関数をまだUNION_APPのものにしていない(VIRTUAL_CONNECT_LIMIT)
 *   ・同じく接続制限用にビーコン通知の書き換え処理がまだ行われていない（BEACON_CHANGE)
 *
 */
//============================================================================================
#define DEBUGPLAY_ONE ( 0 )
#define VIRTUAL_CONNECT_LIMIT
#define BEACON_CHANGE

#include <gflib.h>
#include <calctool.h>

#include "system/main.h"
#include "print/wordset.h"
#include "font/font.naix"
#include "arc/arc_def.h"
#include "arc/message.naix"
#include "msg/msg_oekaki.h"
#include "system/wipe.h"
#include "system/bmp_winframe.h"
#include "system/gfl_use.h"
#include "gamesystem/msgspeed.h"
#include "print/printsys.h"
#include "sound/pm_sndsys.h"
//#include "field/fieldobj.h"

#include "net_app/oekaki.h"
#include "oekaki_local.h"
#include "comm_command_oekaki.h"

//#include "msgdata/msg_ev_win.h"


#include "arc/oekaki.naix"      // グラフィックアーカイブ定義

//============================================================================================
//  定数定義
//============================================================================================

// 文字列描画用のパレット定義
#define NAME_COL_MINE   ( PRINTSYS_LSB_Make( 3, 4,0)     )  // 自分の名前
#define NAME_COL_NORMAL ( PRINTSYS_LSB_Make(0xe,0xd,0xf) )  // 参加メンバーの名前
#define STRING_COL_END  ( PRINTSYS_LSB_Make(0x7,0x1,0x0) )  // 「やめる」
#define STRING_COL_MSG  ( PRINTSYS_LSB_Make(0x1,0x2,0xf) )  // 「やめる」

#define MESFRAME_PAL      ( 10 )   // メッセージウインドウ
#define MENUFRAME_PAL ( 11 )   // メニューウインドウ

// SE定義
#define OEKAKI_DECIDE_SE     ( SEQ_SE_SELECT1 )
#define OEKAKI_BS_SE         ( SEQ_SE_SELECT4 )
#define OEKAKI_NEWMEMBER_SE  ( SEQ_SE_DECIDE1 )
#define OEKAKI_PEN_CHANGE_SE ( SEQ_SE_DECIDE2 )

//============================================================================================
//  プロトタイプ宣言
//============================================================================================

/*** 関数プロトタイプ ***/
static void VBlankFunc( GFL_TCB *tcb, void * work );
static void VramBankSet(void);
static void BgInit( void );
static void InitWork( OEKAKI_WORK *wk );
static void FreeWork( OEKAKI_WORK *wk );
static void BgExit(void);
static void BgGraphicSet( OEKAKI_WORK * wk, ARCHANDLE* p_handle );
static void InitCellActor(OEKAKI_WORK *wk, ARCHANDLE* p_handle);
static void SetCellActor(OEKAKI_WORK *wk);
static void BmpWinInit(OEKAKI_WORK *wk, GFL_PROC* proc);
static void BmpWinDelete( OEKAKI_WORK *wk );
static void SetCursor_Pos( GFL_CLWK *act, int x, int y );
static void NormalTouchFunc(OEKAKI_WORK *wk);
static int Oekaki_MainNormal( OEKAKI_WORK *wk, int seq );
static void EndSequenceCommonFunc( OEKAKI_WORK *wk );
static int Oekaki_EndSelectPutString( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndSelectWait( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndSelectAnswerWait( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndSelectAnswerOK( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndSelectAnswerNG( OEKAKI_WORK *wk, int seq );
static int  Oekaki_EndChild( OEKAKI_WORK *wk, int seq );
static int  Oekaki_EndChildWait( OEKAKI_WORK *wk, int seq );
static int  Oekaki_EndChildWait2( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndSelectParent( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndSelectParentWait( OEKAKI_WORK *wk, int seq );
static int Oekaki_ForceEnd( OEKAKI_WORK *wk, int seq );
static int Oekaki_ForceEndWait( OEKAKI_WORK *wk, int seq );
static int Oekaki_ForceEndSynchronize( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndParentOnly( OEKAKI_WORK *wk, int seq );
static int Oekaki_EndParentOnlyWait( OEKAKI_WORK *wk, int seq );
static void DrawPoint_to_Line(  GFL_BMPWIN *win,   const u8 *brush,  int px, int py, int *sx, int *sy,   int count, int flag );
static void Stock_OldTouch( TOUCH_INFO *all, OLD_TOUCH_INFO *stock );
static void DrawBrushLine( GFL_BMPWIN *win, TOUCH_INFO *all, OLD_TOUCH_INFO *old, int draw );
static void MoveCommCursor( OEKAKI_WORK *wk );
static void DebugTouchDataTrans( OEKAKI_WORK *wk );
static void CursorColTrans(u16 *CursorCol);
static void NameCheckPrint( GFL_BMPWIN *win[], PRINTSYS_LSB color, OEKAKI_WORK *wk );
static int ConnectCheck( OEKAKI_WORK *wk );
static void LineDataSendRecv( OEKAKI_WORK *wk );
static int MyStatusCheck( OEKAKI_WORK *wk );
static void EndMessagePrint( OEKAKI_WORK *wk, int msgno, int wait );
static int EndMessageWait( PRINT_STREAM *stream );
static void EndMessageWindowOff( OEKAKI_WORK *wk );
static int OnlyParentCheck( OEKAKI_WORK *wk );
static int Oekaki_LogoutChildMes( OEKAKI_WORK *wk, int seq );
static int Oekaki_LogoutChildClose( OEKAKI_WORK *wk, int seq );
static int Oekaki_LogoutChildMesWait( OEKAKI_WORK *wk, int seq );
static int MyStatusGetNum( OEKAKI_WORK *wk );
static int Oekaki_NewMemberWait( OEKAKI_WORK *wk, int seq );
static int Oekaki_NewMember( OEKAKI_WORK *wk, int seq );
static int Oekaki_NewMemberEnd( OEKAKI_WORK *wk, int seq );
static void PalButtonAppearChange( GFL_CLWK *act[], int no);
static void EndButtonAppearChange( GFL_CLWK *act[], BOOL flag );
static void _BmpWinPrint_Rap(
      GFL_BMPWIN * win, void * src,
      int src_x, int src_y, int src_dx, int src_dy,
      int win_x, int win_y, int win_dx, int win_dy );
static void ChangeConnectMax( OEKAKI_WORK *wk, int plus );
static int ConnectNumControl( OEKAKI_WORK *wk );
static int Oekaki_ForceEndWaitNop( OEKAKI_WORK *wk, int seq );
static void SetNextSequence( OEKAKI_WORK *wk, int nextSequence );
static void OekakiSequenceControl( OEKAKI_WORK *wk, int proc_seq );

static BOOL OekakiInitYesNoWin(OEKAKI_WORK *wk, TOUCH_SW_PARAM *param);
static void OekakiResetYesNoWin(OEKAKI_WORK *wk);
static int FakeEndYesNoSelect( OEKAKI_WORK  *wk );
static void SetTouchpanelData( TOUCH_INFO *touchResult, TP_ONE_DATA *tpData, int brush_color, int brush );
static int _get_connect_bit( OEKAKI_WORK *wk );
static int _get_connect_num( OEKAKI_WORK *wk );


typedef struct{
  int (*func)(OEKAKI_WORK *wk, int seq);
  int execOn_SeqLeave;
}OEKAKI_FUNC_TABLE;

static OEKAKI_FUNC_TABLE FuncTable[]={
  {NULL,                      1,},  // OEKAKI_MODE_INIT  = 0, 
  {Oekaki_NewMember,          0,},  // OEKAKI_MODE_NEWMEMBER,
  {Oekaki_NewMemberWait,      0,},  // OEKAKI_MODE_NEWMEMBER_WAIT,
  {Oekaki_NewMemberEnd,       0,},  // OEKAKI_MODE_NEWMEMBER_END,
  {Oekaki_MainNormal,         0,},  // OEKAKI_MODE,
  {Oekaki_EndSelectPutString, 1,},  // OEKAKI_MODE_END_SELECT,
  {Oekaki_EndSelectWait,      1,},  // OEKAKI_MODE_END_SELECT_WAIT,
  {Oekaki_EndSelectAnswerWait,1,},  // OEKAKI_MODE_END_SELECT_ANSWER_WAIT
  {Oekaki_EndSelectAnswerOK,  1,},  // OEKAKI_MODE_END_SELECT_ANSWER_OK
  {Oekaki_EndSelectAnswerNG,  1,},  // OEKAKI_MODE_END_SELECT_ANSWER_NG
  {Oekaki_EndChild,           1,},  // OEKAKI_MODE_END_CHILD
  {Oekaki_EndChildWait,       1,},  // OEKAKI_MODE_END_CHILD_WAIT
  {Oekaki_EndChildWait2,      1,},  // OEKAKI_MODE_END_CHILD_WAIT2
  {Oekaki_EndSelectParent,    1,},  // OEKAKI_MODE_END_SELECT_PARENT
  {Oekaki_EndSelectParentWait,1,},  // OEKAKI_MODE_END_SELECT_PARENT_WAIT
  {Oekaki_ForceEnd,           1,},  // OEKAKI_MODE_FORCE_END
  {Oekaki_ForceEndWait,       1,},  // OEKAKI_MODE_FORCE_END_WAIT
  {Oekaki_ForceEndSynchronize,1,},  // OEKAKI_MODE_FORCE_END_SYNCHRONIZE
  {Oekaki_ForceEndWaitNop,    1,},  // OEKAKI_MODE_FORCE_END_WAIT_NOP
  {Oekaki_EndParentOnly,      1,},  // OEKAKI_MODE_END_PARENT_ONLY
  {Oekaki_EndParentOnlyWait,  1,},  // OEKAKI_MODE_END_PARENT_ONLY_WAIT
  {Oekaki_LogoutChildMes,     1,},  // OEKAKI_MODE_LOGOUT_CHILD
  {Oekaki_LogoutChildMesWait, 1,},  // OEKAKI_MODE_LOGOUT_CHILD_WAIT
  {Oekaki_LogoutChildClose,   1,},  // OEKAKI_MODE_LOGOUT_CHILD_CLOSE
};


#ifdef VIRTUAL_CONNECT_LIMIT
static void CommStateSetLimitNum( OEKAKI_WORK *wk, int num)
{
  return;
}
#endif

//============================================================================================
//  プロセス関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：初期化
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT OekakiProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  OEKAKI_WORK * wk;
  ARCHANDLE* p_handle;

  switch(*seq){
  case 0:
    GFL_DISP_GX_InitVisibleControl();
    GFL_DISP_GXS_InitVisibleControl();

    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OEKAKI, 0x40000 );

    wk = GFL_PROC_AllocWork( proc, sizeof(OEKAKI_WORK), HEAPID_OEKAKI );
    GFL_STD_MemFill( wk, 0, sizeof(OEKAKI_WORK) );
    GFL_BG_Init( HEAPID_OEKAKI );
    
    // 文字列マネージャー生成
    wk->WordSet    = WORDSET_Create( HEAPID_OEKAKI );
    wk->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                     NARC_message_oekaki_dat, HEAPID_OEKAKI );
    wk->font       = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                                      GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_OEKAKI );

    // VRAM バンク設定
    VramBankSet();
    
    // BGシステム初期化
    BgInit();          

    WIPE_SetBrightness( WIPE_DISP_MAIN,WIPE_FADE_BLACK );
    WIPE_SetBrightness( WIPE_DISP_SUB,WIPE_FADE_BLACK );

    // 輝度変更セット
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEIN, WIPE_TYPE_HOLEIN, WIPE_FADE_BLACK, 16, 1, HEAPID_OEKAKI );
    
    // パラメータ取得
    {
      PICTURE_PARENT_WORK *param = (PICTURE_PARENT_WORK*)pwk;
      wk->param = param;
    }

    // ハンドルオープン
    p_handle = GFL_ARC_OpenDataHandle( ARCID_OEKAKI, HEAPID_OEKAKI );

    //BGグラフィックセット
    BgGraphicSet( wk, p_handle );

//    InitTPNoBuff(2);
  
    // VBlank関数セット
    wk->vblankTcb = GFUser_VIntr_CreateTCB( VBlankFunc, wk, 0 );  
  
    // ワーク初期化
    InitWork( wk );

    // CellActorシステム初期化
    InitCellActor(wk, p_handle);
    
    // CellActro表示登録
    SetCellActor(wk);

    // BMPWIN登録・描画
    BmpWinInit(wk,proc);

    // 画面出力を上下入れ替える
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    // 通信コマンドをおえかきボード用に変更
    OEKAKIBOARD_CommandInitialize( wk );

        // お絵かき時には接続切断でエラー扱いしない
//        CommStateSetErrorCheck(FALSE,TRUE);
        
    // 3台まで接続可能に書き換え(開始した時は２人でここにくるのであと一人だけ入れるようにしておく）
    if(GFL_NET_SystemGetCurrentID()==0){
          CommStateSetLimitNum(wk, 3);
    }

    //無線アイコン表示
    GFL_NET_WirelessIconEasy_DefaultLCD();

    // ハンドルクローズ
    GFL_ARC_CloseDataHandle( p_handle );
    (*seq)++;
    break;
  case 1:
    (*seq) = SEQ_IN;
    return GFL_PROC_RES_FINISH;
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}




//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：メイン
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------

GFL_PROC_RESULT OekakiProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  OEKAKI_WORK * wk  = (OEKAKI_WORK *)mywk;

  if(GFL_NET_SystemGetCurrentID() == 0 && wk->ridatu_bit != 0){
    wk->ridatu_bit &= _get_connect_bit(wk);
  }
  
  // ワークに現在のprocシーケンスを保存
  wk->proc_seq = *seq;

  // サブシーケンス遷移制御を行う
  OekakiSequenceControl( wk, *seq );
  
  switch( *seq ){
  case SEQ_IN:
    if( WIPE_SYS_EndCheck() ){
      // ワイプ処理待ち

      // 自分が子機で接続台数が２台以上だった場合はもう絵が描かれている
      if(GFL_NET_SystemGetCurrentID()!=0){
        if( (MyStatusGetNum(wk)>=2) ){
          // 子機乱入リクエスト
          GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(),CO_OEKAKI_CHILD_JOIN, NULL, 0);
          OS_Printf("乱入します\n");
          *seq = SEQ_MAIN;
        }
        break;
      }else{
        //親はメインへ
        *seq = SEQ_MAIN;
      }
    }
    break;

  case SEQ_MAIN:
    // カーソル移動

    // シーケンス毎の動作
    if(FuncTable[wk->seq].func!=NULL){
      *seq = FuncTable[wk->seq].func( wk, *seq );
    }
    // 接続人数を確認して上画面に名前を描画
    NameCheckPrint( wk->TrainerNameWin, NAME_COL_NORMAL ,wk);

    if(GFL_NET_SystemGetCurrentID()==0){        // 親は
      int temp = ConnectNumControl(wk);
      if(*seq == SEQ_MAIN){
        // 既に終了シーケンスなどに入っている場合は書き換えない
        *seq = temp;
      }
    }
    CursorColTrans(&wk->CursorPal);

    break;
  case SEQ_LEAVE:
    if(FuncTable[wk->seq].func!=NULL && FuncTable[wk->seq].execOn_SeqLeave){
      *seq = FuncTable[wk->seq].func( wk, *seq );
    }
    break;

  case SEQ_OUT:
    if( WIPE_SYS_EndCheck() ){
      OS_Printf("ワイプ終了確認\n");
      return GFL_PROC_RES_FINISH;
    }
    break;
  }
  GFL_CLACT_SYS_Main();             // セルアクター常駐関数
  
  return GFL_PROC_RES_CONTINUE;
}

#define DEFAULT_NAME_MAX    18

// ダイヤ・パールで変わるんだろう
#define MALE_NAME_START     0
#define FEMALE_NAME_START   18

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：終了
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT OekakiProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  OEKAKI_WORK  *wk    = (OEKAKI_WORK  *)mywk;
  PICTURE_PARENT_WORK *param = (PICTURE_PARENT_WORK*)pwk;
  int i;

  switch(*seq){
  case 0:
    OS_Printf("おえかきワーク解放処理突入\n");

    // Vblank期間中のBG転送終了
    GFL_TCB_DeleteTask( wk->vblankTcb );

    // セルアクターリソース解放
    GFL_CLGRP_CGR_Release(  wk->resObjTbl[CLACT_RES_M_CHR] );     // メイン面
    GFL_CLGRP_PLTT_Release( wk->resObjTbl[CLACT_RES_M_PLTT] );
    GFL_CLGRP_CELLANIM_Release( wk->resObjTbl[CLACT_RES_M_CELL] );

    GFL_CLGRP_CGR_Release(  wk->resObjTbl[CLACT_RES_S_CHR] );     // サブ面
    GFL_CLGRP_PLTT_Release( wk->resObjTbl[CLACT_RES_S_PLTT] );
    GFL_CLGRP_CELLANIM_Release( wk->resObjTbl[CLACT_RES_S_CELL] );
    
    // セルアクターユニット破棄
    GFL_CLACT_UNIT_Delete( wk->clUnit );

    //OAMレンダラー破棄
    GFL_CLACT_SYS_Delete();
    
    // BMPウィンドウ開放
    BmpWinDelete( wk );

    // BGL削除
    BgExit();

    // メッセージマネージャー・ワードセットマネージャー解放
    GFL_MSG_Delete( wk->MsgManager );
    WORDSET_Delete( wk->WordSet );
    GFL_FONT_Delete( wk->font );

    (*seq)++;
    break;
  case 1:
    // 通信終了
  //  CommStateExitUnion();
//    CommStateSetLimitNum(1);
//    CommStateUnionBconCollectionRestart();
//    UnionRoomView_ObjInit( param->view );

    // 入れ替わっていた上下画面出力を元に戻す
    GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

    // ビーコン書き換え
#ifdef BEACON_CHANGE
//    Union_BeaconChange( UNION_PARENT_MODE_FREE );
#endif

    (*seq)++;
    break;
  case 2:
    if (wk->ireagalJoin){
      OS_Printf("親機だけになるまで待機\n");  
      if (_get_connect_bit(wk) == 1){
        (*seq)++;
      }
    }else{
      (*seq)++;
    }
    break;
  case 3:
    CommStateSetLimitNum(wk,2);
    // ワーク解放
    FreeWork( wk );

    GFL_PROC_FreeWork( proc );        // GFL_PROCワーク開放

    GFL_HEAP_DeleteHeap( HEAPID_OEKAKI );

    CommStateSetLimitNum(wk,2);

    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * VBlank関数
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *tcb, void * work )
{
  // セルアクター
  GFL_CLACT_SYS_VBlankFunc();

  // BG転送  
  GFL_BG_VBlankFunc();
  
}


//--------------------------------------------------------------------------------------------
// VRAM設定data
//--------------------------------------------------------------------------------------------
static const GFL_DISP_VRAM OekakiDispVramDat = {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット

    GX_VRAM_OBJ_128_B,          // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット

    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,       // テクスチャパレットスロット

    GX_OBJVRAMMODE_CHAR_1D_32K, // メイン面OBJVRAMサイズ
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブ面OBJVRAMサイズ
    
  };


//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VramBankSet(void)
{
  GFL_DISP_SetBank( &OekakiDispVramDat );
}

//--------------------------------------------------------------------------------------------
/**
 * BG設定
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgInit( void )
{
  // BG SYSTEM
  { 
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }

  // メイン画面文字版0
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  GFL_BG_FRAME0_M );


  }

  // メイン画面ラクガキ面
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  GFL_BG_FRAME1_M );
  }

  // メイン画面背景
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  GFL_BG_FRAME2_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }


  // サブ画面テキスト面
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  GFL_BG_FRAME0_S );
  }

  // サブ画面背景面
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_OEKAKI );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, HEAPID_OEKAKI );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME1_M, 32, 0, HEAPID_OEKAKI ); ///< 文字面以外もキャラクリアした。080422
  GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 32, 0, HEAPID_OEKAKI );

}


#define TALK_MESSAGE_BUF_NUM  ( 40*2 )

//------------------------------------------------------------------
/**
 * お絵かきワーク初期化
 *
 * @param   wk    OEKAKI_WORK*
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void InitWork( OEKAKI_WORK *wk )
{
  int i;

  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    wk->TrainerName[i] = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, HEAPID_OEKAKI );
    wk->AllTouchResult[i].size = 0;
    wk->OldTouch[i].size = 0;
    wk->TrainerStatus[i][0] = NULL;
    wk->TrainerStatus[i][1] = NULL;

  }
  // 「やめる」文字列バッファ作成
  wk->EndString  = GFL_STR_CreateBuffer( 10, HEAPID_OEKAKI );
  wk->TalkString = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_OEKAKI );

  // ブラシ初期化
  wk->brush_color  = 0;
  wk->brush        = MIDDLE_BRUSH;
  wk->banFlag      = OEKAKI_BAN_OFF;
  wk->yesno_flag   = 0;
  wk->shareNum     = 2;
  wk->shareBit     = _get_connect_bit(wk);
  wk->firstChild   = 0;
  wk->ireagalJoin  = 0;
  wk->seq          = 0;
  wk->bookJoin  = 0;
  wk->joinBit  = 0;

  // 親は通信で絵を受け取る必要が無い
  if( GFL_NET_SystemGetCurrentID()==0 ){
    SetNextSequence( wk, OEKAKI_MODE );
  }else{
    // 絵を受け取らないといけない
    SetNextSequence( wk, OEKAKI_MODE );
  }
  
  // 「やめる」取得
  GFL_MSG_GetString( wk->MsgManager, msg_oekaki_08, wk->EndString );

  // お絵かき画像圧縮データ領域
  wk->lz_buf = GFL_HEAP_AllocMemory( HEAPID_OEKAKI, 30*16*32 );

  // 下画面ウインドウシステム初期化
  wk->TouchSubWindowSys = TOUCH_SW_AllocWork( HEAPID_OEKAKI );

  // 接続人数監視用ワーク初期化
  wk->connectBackup = 0;
}

//------------------------------------------------------------------
/**
 * @brief   ワーク解放
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void FreeWork( OEKAKI_WORK *wk )
{
  int i;

  GFL_HEAP_FreeMemory(  wk->lz_buf );
  TOUCH_SW_FreeWork( wk->TouchSubWindowSys );


  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    GFL_STR_DeleteBuffer( wk->TrainerName[i] );
  }
  GFL_STR_DeleteBuffer( wk->TalkString ); 
  GFL_STR_DeleteBuffer( wk->EndString  ); 

}

//--------------------------------------------------------------------------------------------
/**
 * BG解放
 *
 * @param ini   BGLデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgExit( void )
{

  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

  GFL_BG_Exit();
}


#define TALKFONT_PAL_ROW      ( 13 )
#define TALKFONT_PAL_OFFSET   ( TALKFONT_PAL_ROW*0x20 )
//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param wk    ポケモンリスト画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgGraphicSet( OEKAKI_WORK * wk, ARCHANDLE* p_handle )
{

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_oekaki_oekaki_m_NCLR, PALTYPE_MAIN_BG, 0, 16*2*2,  HEAPID_OEKAKI);
  GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_oekaki_oekaki_s_NCLR, PALTYPE_SUB_BG,  0, 16*2*2,  HEAPID_OEKAKI);
  
  // 会話フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_OEKAKI );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_OEKAKI );

  // メイン画面BG2キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_oekaki_oekaki_m_NCGR, GFL_BG_FRAME2_M, 0, 32*8*0x20, 1, HEAPID_OEKAKI);

  // メイン画面BG2スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_oekaki_oekaki_m_NSCR, GFL_BG_FRAME2_M, 0, 32*24*2, 1, HEAPID_OEKAKI);



  // サブ画面BG1キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_oekaki_oekaki_s_NCGR, GFL_BG_FRAME1_S, 0, 32*8*0x20, 1, HEAPID_OEKAKI);

  // サブ画面BG1スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_oekaki_oekaki_s_NSCR, GFL_BG_FRAME1_S, 0, 32*24*2, 1, HEAPID_OEKAKI);

  // サブ画面会話ウインドウグラフィック転送
  BmpWinFrame_GraphicSet(
        GFL_BG_FRAME0_M, 1, MESFRAME_PAL,  0, HEAPID_OEKAKI );

  BmpWinFrame_GraphicSet(
        GFL_BG_FRAME0_M, 1+TALK_WIN_CGX_SIZ, MENUFRAME_PAL, 0, HEAPID_OEKAKI );

}


//------------------------------------------------------------------
/**
 * レーダー画面用セルアクター初期化
 *
 * @param   wk    レーダー構造体のポインタ
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void InitCellActor(OEKAKI_WORK *wk, ARCHANDLE* p_handle)
{
  int i;
  
  // セルアクター初期化
  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &OekakiDispVramDat, HEAPID_OEKAKI );
  
  
  // セルアクター初期化
  wk->clUnit = GFL_CLACT_UNIT_Create( 50+3, 1,  HEAPID_OEKAKI );
  
//  CLACT_U_SetSubSurfaceMatrix( &wk->renddata, 0, NAMEIN_SUB_ACTOR_DISTANCE );

  
  //---------上画面用-------------------

  //chara読み込み
  wk->resObjTbl[CLACT_RES_M_CHR] = GFL_CLGRP_CGR_Register( p_handle, NARC_oekaki_oekaki_m_obj_NCGR, 0, 
                                                           CLSYS_DRAW_MAIN, HEAPID_OEKAKI );

  //pal読み込み
  wk->resObjTbl[CLACT_RES_M_PLTT] = GFL_CLGRP_PLTT_Register( p_handle, NARC_oekaki_oekaki_m_NCLR, 
                                                             CLSYS_DRAW_MAIN, 0, HEAPID_OEKAKI );

  //cell読み込み
  wk->resObjTbl[CLACT_RES_M_CELL] = GFL_CLGRP_CELLANIM_Register( p_handle, 
                                                                 NARC_oekaki_oekaki_m_obj_NCER, 
                                                                 NARC_oekaki_oekaki_m_obj_NANR, 
                                                                 HEAPID_OEKAKI );

  //---------下画面用-------------------

  //chara読み込み
  wk->resObjTbl[CLACT_RES_S_CHR] = GFL_CLGRP_CGR_Register( p_handle, NARC_oekaki_oekaki_m_obj_NCGR, 0, 
                                                           CLSYS_DRAW_SUB, HEAPID_OEKAKI );

  //pal読み込み
  wk->resObjTbl[CLACT_RES_S_PLTT] = GFL_CLGRP_PLTT_Register( p_handle, NARC_oekaki_oekaki_m_NCLR, 
                                                             CLSYS_DRAW_SUB, 0, HEAPID_OEKAKI );

  //cell読み込み
  wk->resObjTbl[CLACT_RES_S_CELL] = GFL_CLGRP_CELLANIM_Register( p_handle, 
                                                                 NARC_oekaki_oekaki_m_obj_NCER, 
                                                                 NARC_oekaki_oekaki_m_obj_NANR, 
                                                                 HEAPID_OEKAKI );

}

#define TRAINER_NAME_POS_X    ( 24 )
#define TRAINER_NAME_POS_Y    ( 32 )
#define TRAINER_NAME_POS_SPAN ( 32 )

static const u16 pal_button_oam_table[][3]={
  {13     , 171, 5},    // 黒
  {13+24*1, 171, 7},    // 白
  {13+24*2, 171, 9},    // 赤
  {13+24*3, 171,11},    // 紫
  {13+24*4, 171,13},    // 青
  {13+24*5, 171,15},    // 水色
  {13+24*6, 171,17},    // 緑
  {13+24*7, 171,19},    // 黄色
  {13+24*8+20, 171,21}, // 「やめる」

  // ペン先アイコン
  {20,    9,   29, },
  {36,      9,   32, }, // 初期値で決定状態になっている
  {52,      9,   33, },
};

//------------------------------------------------------------------
/**
 * セルアクター登録
 *
 * @param   wk      OEKAKI_WORK*
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void SetCellActor(OEKAKI_WORK *wk)
{
  int i;
  // セルアクターヘッダ作成

  {
    //登録情報格納
    GFL_CLWK_DATA add;

    add.pos_x = 0;
    add.pos_y = 0;
    add.anmseq    = 0;
    add.softpri   = 1;
    add.bgpri     = 1;

    //セルアクター表示開始

    // メイン画面用(矢印の登録）
    for(i=0;i<5;i++){
      add.pos_x = TRAINER_NAME_POS_X + i * 40;
      add.pos_y = TRAINER_NAME_POS_Y + TRAINER_NAME_POS_SPAN;
      wk->MainActWork[i] = GFL_CLACT_WK_Create( wk->clUnit,
        wk->resObjTbl[CLACT_RES_M_CHR],
        wk->resObjTbl[CLACT_RES_M_PLTT],
        wk->resObjTbl[CLACT_RES_M_CELL],
        &add, CLSYS_DEFREND_MAIN, HEAPID_OEKAKI );

      GFL_CLACT_WK_SetAutoAnmFlag( wk->MainActWork[i], 1 );
      GFL_CLACT_WK_SetAnmSeq(      wk->MainActWork[i], i );
      GFL_CLACT_WK_SetDrawEnable(  wk->MainActWork[i], 0 );

    }

    // メイン画面パレット・「やめる」ボタンの登録
    for(i=0;i<9+3;i++){
      add.pos_x = pal_button_oam_table[i][0];
      add.pos_y = pal_button_oam_table[i][1];
      wk->ButtonActWork[i] = GFL_CLACT_WK_Create( wk->clUnit,
        wk->resObjTbl[CLACT_RES_M_CHR ],
        wk->resObjTbl[CLACT_RES_M_PLTT],
        wk->resObjTbl[CLACT_RES_M_CELL],
        &add, CLSYS_DEFREND_MAIN, HEAPID_OEKAKI );

      GFL_CLACT_WK_SetAutoAnmFlag( wk->ButtonActWork[i], 1 );
      GFL_CLACT_WK_SetAnmSeq( wk->ButtonActWork[i], pal_button_oam_table[i][2] );
      if(i >= 8){
        GFL_CLACT_WK_SetBgPri( wk->ButtonActWork[i], 2 );
      }
    }

    // 黒色パレットはへこませておく
    GFL_CLACT_WK_SetAnmSeq( wk->ButtonActWork[0], 6 );


    // サブ画面用(矢印の登録）
    for(i=0;i<5;i++){
      add.pos_x = TRAINER_NAME_POS_X;
      add.pos_y = (TRAINER_NAME_POS_Y+TRAINER_NAME_POS_SPAN*i)+NAMEIN_SUB_ACTOR_DISTANCE;
      wk->SubActWork[i] = GFL_CLACT_WK_Create( wk->clUnit,
        wk->resObjTbl[CLACT_RES_S_CHR],
        wk->resObjTbl[CLACT_RES_S_PLTT],
        wk->resObjTbl[CLACT_RES_S_CELL],
        &add, CLSYS_DEFREND_SUB, HEAPID_OEKAKI );


      GFL_CLACT_WK_SetAutoAnmFlag( wk->SubActWork[i], 1 );
      GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[i],      i );
      GFL_CLACT_WK_SetSoftPri( wk->SubActWork[i],     1 );  // それぞれのアクターのプライオリティ設定
      GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[i],  0 );
      
    }
    
  } 
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON ); //メイン画面OBJ面ＯＮ
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON ); //サブ画面OBJ面ＯＮ
  
}

// おえかきボードBMP（下画面）
#define OEKAKI_BOARD_POSX  ( 1 )
#define OEKAKI_BOARD_POSY  ( 2 )
#define OEKAKI_BOARD_W   ( 30 )
#define OEKAKI_BOARD_H   ( 15 )


// 名前表示BMP（上画面）
#define OEKAKI_NAME_BMP_W  ( 10 )
#define OEKAKI_NAME_BMP_H  (  2 )
#define OEKAKI_NAME_BMP_SIZE (OEKAKI_NAME_BMP_W * OEKAKI_NAME_BMP_H)


// 「やめる」文字列BMP（下画面）
#define OEKAKI_END_BMP_X  ( 26 )
#define OEKAKI_END_BMP_Y  ( 21 )
#define OEKAKI_END_BMP_W  ( 6  )
#define OEKAKI_END_BMP_H  ( 2  )


// 会話ウインドウ表示位置定義
#define OEKAKI_TALK_X   (  2 )
#define OEKAKI_TALK_Y   (  1 )

#define MSG_WIN_W      ( 27 )
#define MSG_WIN_H      (  4 )
//------------------------------------------------------------------
/**
 * BMPWIN処理（文字パネルにフォント描画）
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void BmpWinInit( OEKAKI_WORK *wk, GFL_PROC* proc )
{
  // ---------- メイン画面 ------------------

  // BMPWINシステム開始
  GFL_BMPWIN_Init( HEAPID_OEKAKI );

  // BG0面BMP（会話ウインドウ）ウインドウ確保
  wk->MsgWin = GFL_BMPWIN_Create( GFL_BG_FRAME0_M,
                                  OEKAKI_TALK_X, OEKAKI_TALK_Y, 
                                  MSG_WIN_W, MSG_WIN_H, 13, GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin), 0x0f0f );

  // BG1面用BMP（お絵かき画像）ウインドウ確保
  wk->OekakiBoard = GFL_BMPWIN_Create( GFL_BG_FRAME1_M,
                                   OEKAKI_BOARD_POSX, OEKAKI_BOARD_POSY, 
                                   OEKAKI_BOARD_W, OEKAKI_BOARD_H, 0, GFL_BMP_CHRAREA_GET_B);
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->OekakiBoard), 0x0202 );

  // BG1面BMP（やめる）ウインドウ確保・描画
  wk->EndWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M,
                              OEKAKI_END_BMP_X, OEKAKI_END_BMP_Y, 
                              OEKAKI_END_BMP_W, OEKAKI_END_BMP_H, 13,  GFL_BMP_CHRAREA_GET_B );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->EndWin), 0x0000 );
  
  PRINT_UTIL_Setup( &wk->printUtil[OEKAKI_PRINT_UTIL_END], wk->EndWin );
//  GF_STR_PrintColor( wk->EndWin, FONT_TALK, wk->EndString, 0, 0, MSG_ALLPUT, STRING_COL_END ,NULL);
  PRINT_UTIL_PrintColor( &wk->printUtil[OEKAKI_PRINT_UTIL_END], wk->printQue, 
                         0, 0, wk->EndString, wk->font, STRING_COL_END );

  // メッセージ表示システム用初期化 
  wk->pMsgTcblSys = GFL_TCBL_Init( HEAPID_OEKAKI, HEAPID_OEKAKI, 32 , 32 );
  wk->printQue    = PRINTSYS_QUE_Create( HEAPID_OEKAKI );
  
  // ----------- サブ画面名前表示BMP確保 ------------------
  {
    int i;
    for(i=0;i<OEKAKI_MEMBER_MAX;i++){
      wk->TrainerNameWin[i] = GFL_BMPWIN_Create( GFL_BG_FRAME0_S,  
                                             TRAINER_NAME_POS_X/8+2, TRAINER_NAME_POS_Y/8+i*4-1, 
                                             10, 2, 13,  GFL_BMP_CHRAREA_GET_B);
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->TrainerNameWin[i] ), 0 );
    }

    //最初に見えている面なので文字パネル描画と転送も行う
    NameCheckPrint( wk->TrainerNameWin, NAME_COL_NORMAL, wk );
  }

} 

// はい・いいえ用定義（下画面）
#define YESNO_CHARA_OFFSET  (1 + TALK_WIN_CGX_SIZ + MENU_WIN_CGX_SIZ + MSG_WIN_W*MSG_WIN_H )
#define YESNO_CHARA_W   ( 8 )
#define YESNO_CHARA_H   ( 4 )





//------------------------------------------------------------------
/**
 * @brief   確保したBMPWINを解放
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void BmpWinDelete( OEKAKI_WORK *wk )
{
  int i;

  // メッセージ表示用システム解放
  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_TCBL_Exit( wk->pMsgTcblSys );

  
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    GFL_BMPWIN_Delete( wk->TrainerNameWin[i] );
  }
  GFL_BMPWIN_Delete( wk->EndWin );
  GFL_BMPWIN_Delete( wk->OekakiBoard );
  GFL_BMPWIN_Delete( wk->MsgWin );

  // BMPWINシステム終了
  GFL_BMPWIN_Exit();
}








//------------------------------------------------------------------
/**
 * @brief   カーソル位置を変更する
 *
 * @param   act   アクターのポインタ
 * @param   x   
 * @param   y   
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void SetCursor_Pos( GFL_CLWK *act, int x, int y )
{
  GFL_CLACTPOS pos;

  pos.x = x;
  pos.y = y-8;
  GFL_CLACT_WK_SetWldPos( act, &pos );

}


// ---------------------------------------------------------
// 通常時タッチ検出テーブル
// ---------------------------------------------------------

#define PAL_BUTTON_X  (  0  )
#define PAL_BUTTON_Y  ( 150 )
#define PAL_BUTTON_W  ( 3*8 )
#define PAL_BUTTON_H  ( 5*8 )
#define PAL_BUTTON_RECT ( 3*8 -1 )
#define PAL_BUTTON_RECT ( 3*8 -1 )
#define END_BUTTON_RECT ( 8*8 -1 )

#define PAL_BUTTON0_X ( PAL_BUTTON_X+PAL_BUTTON_W*0 )
#define PAL_BUTTON1_X ( PAL_BUTTON_X+PAL_BUTTON_W*1 )
#define PAL_BUTTON2_X ( PAL_BUTTON_X+PAL_BUTTON_W*2 )
#define PAL_BUTTON3_X ( PAL_BUTTON_X+PAL_BUTTON_W*3 )
#define PAL_BUTTON4_X ( PAL_BUTTON_X+PAL_BUTTON_W*4 )
#define PAL_BUTTON5_X ( PAL_BUTTON_X+PAL_BUTTON_W*5 )
#define PAL_BUTTON6_X ( PAL_BUTTON_X+PAL_BUTTON_W*6 )
#define PAL_BUTTON7_X ( PAL_BUTTON_X+PAL_BUTTON_W*7 )
#define END_BUTTON_X  ( PAL_BUTTON_X+PAL_BUTTON_W*8 )

#define BRUSH_BUTTON0_X ( 14 ) // 16,5 23,12
#define BRUSH_BUTTON1_X ( 30 ) // 16,5 23,12
#define BRUSH_BUTTON2_X ( 46 ) // 16,5 23,12
#define BRUSH_BUTTON_Y  (  1 ) // 16,5 23,12
#define BRUSH_BUTTON_W  ( 12 ) // 16,5 23,12
#define BRUSH_BUTTON_H  ( 12 ) // 16,5 23,12


static const GFL_UI_TP_HITTBL sub_button_hittbl[]={
// パレット
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON0_X,PAL_BUTTON0_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON1_X,PAL_BUTTON1_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON2_X,PAL_BUTTON2_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON3_X,PAL_BUTTON3_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON4_X,PAL_BUTTON4_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON5_X,PAL_BUTTON5_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON6_X,PAL_BUTTON6_X+PAL_BUTTON_RECT},
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,PAL_BUTTON7_X,PAL_BUTTON7_X+PAL_BUTTON_RECT},

// 終了ボタン
{PAL_BUTTON_Y,PAL_BUTTON_Y+PAL_BUTTON_H-1,END_BUTTON_X,END_BUTTON_X+END_BUTTON_RECT},

// ペン先ボタン
{BRUSH_BUTTON_Y,BRUSH_BUTTON_Y+BRUSH_BUTTON_H, BRUSH_BUTTON0_X,BRUSH_BUTTON0_X+BRUSH_BUTTON_W},
{BRUSH_BUTTON_Y,BRUSH_BUTTON_Y+BRUSH_BUTTON_H, BRUSH_BUTTON1_X,BRUSH_BUTTON1_X+BRUSH_BUTTON_W},
{BRUSH_BUTTON_Y,BRUSH_BUTTON_Y+BRUSH_BUTTON_H, BRUSH_BUTTON2_X,BRUSH_BUTTON2_X+BRUSH_BUTTON_W},

{GFL_UI_TP_HIT_END,0,0,0},   // 終了データ
};


// ---------------------------------------------------------
// 離脱選択禁止中の際のYESNOボタン検出テーブル
// ---------------------------------------------------------
#define OEKAKI_YESNO_BUTTON_X ( 25 )
#define OEKAKI_YESNO_BUTTON_Y ( 6  )

#define FAKE_YESNO_X  ( OEKAKI_YESNO_BUTTON_X*8 )
#define FAKE_YESNO_Y  ( OEKAKI_YESNO_BUTTON_Y*8 )
#define FAKE_YESNO_W  ( 6*8 )
#define FAKE_YESNO_H  ( 8*8 )

static const GFL_UI_TP_HITTBL fake_yesno_hittbl[]={
  {FAKE_YESNO_Y, FAKE_YESNO_Y+FAKE_YESNO_H, FAKE_YESNO_X, FAKE_YESNO_X+FAKE_YESNO_W},
  {GFL_UI_TP_HIT_END,0,0,0},
};

// ---------------------------------------------------------
// 描画領域検出用テーブル
// ---------------------------------------------------------
#define DRAW_AREA_X (  8  )
#define DRAW_AREA_Y ( 16  )
#define DRAW_AREA_W ( OEKAKI_BOARD_W*8 )
#define DRAW_AREA_H ( OEKAKI_BOARD_H*8)

static const GFL_UI_TP_HITTBL sub_canvas_touchtbl[]={
  {DRAW_AREA_Y,DRAW_AREA_Y+DRAW_AREA_H,DRAW_AREA_X,DRAW_AREA_X+DRAW_AREA_W},
  {GFL_UI_TP_HIT_END,0,0,0},   // 終了データ
};




//------------------------------------------------------------------
/**
 * タッチパネルで押したボタンがへこむ
 *
 * @param   wk    OEKAKI_WORKのポインタ
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void NormalTouchFunc(OEKAKI_WORK *wk)
{
  int button=-1,touch;
  int decide = FALSE;
  
  // 入力処理

  // カラー切り替え
  button=GFL_UI_TP_HitTrg( sub_button_hittbl );
  if( button != GFL_UI_TP_HIT_NONE ){
    switch(button){
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
      // 色変更
      if(wk->brush_color!=button){
        wk->brush_color = button;
        PalButtonAppearChange( wk->ButtonActWork, button);
        PMSND_PlaySE( OEKAKI_DECIDE_SE );
      }
      break;
    case 8:
      // 「やめる」を押したらウインドウ描画開始
      if(wk->seq==OEKAKI_MODE){
        // 親機の場合は接続拒否
        if(GFL_NET_SystemGetCurrentID()==0){
          //コネクト数と表示人数が異なる場合は、やめるボタンを無反応にする
          if (wk->shareBit != _get_connect_bit(wk)){
            PMSND_PlaySE(OEKAKI_BS_SE);
            break;
          }
          CommStateSetLimitNum(wk,_get_connect_num(wk));
          wk->banFlag = OEKAKI_BAN_ON;
          // 「おえかきをやめますか？」
          EndMessagePrint( wk, msg_oekaki_02, 1 );
          SetNextSequence( wk, OEKAKI_MODE_END_SELECT );
          EndButtonAppearChange( wk->ButtonActWork, TRUE );
          decide = TRUE;
          PMSND_PlaySE(OEKAKI_DECIDE_SE);
        }else{
          if(wk->AllTouchResult[0].banFlag == OEKAKI_BAN_ON ){
            // 親機に禁止されているときはＳＥのみ
            PMSND_PlaySE(OEKAKI_BS_SE);
          }else{
            // 「おえかきをやめますか？」
            EndMessagePrint( wk, msg_oekaki_02, 1 );
            SetNextSequence( wk, OEKAKI_MODE_END_SELECT );
            EndButtonAppearChange( wk->ButtonActWork, TRUE );
            decide = TRUE;
            PMSND_PlaySE(OEKAKI_DECIDE_SE);
          }
        }

      }
      break;
    case 9: case 10: case 11:
      // ペン先変更
      {
        int i;
        for(i=0;i<3;i++){
          if((button-9)==i){
            GFL_CLACT_WK_SetAnmSeq( wk->ButtonActWork[CELL_BRUSH_NO+i], 29+2*i+1 );   // ON
          }else{
            GFL_CLACT_WK_SetAnmSeq( wk->ButtonActWork[CELL_BRUSH_NO+i], 29+2*i ); // OFF
          }
        }
        if(wk->brush!=(SMALL_BRUSH + button-9)){
          wk->brush = SMALL_BRUSH + button-9;
          PMSND_PlaySE(OEKAKI_PEN_CHANGE_SE);
        }
      }
      break;
    }
  }

  // カーソル位置変更
  touch = GFL_UI_TP_HitCont( sub_canvas_touchtbl );

  GFL_CLACT_WK_SetDrawEnable( wk->MainActWork[GFL_NET_SystemGetCurrentID()], 0 );
  if(touch!=GFL_UI_TP_HIT_NONE){
    // 自分のカーソルはタッチパネルの座標を反映させる
    u32 x,y;
    GFL_UI_TP_GetPointCont( &x, &y );
    SetCursor_Pos( wk->MainActWork[GFL_NET_SystemGetCurrentID()], x, y );
    GFL_CLACT_WK_SetDrawEnable( wk->MainActWork[GFL_NET_SystemGetCurrentID()], 1 );
  }

  // サンプリング情報を取得して格納
  {
    TP_ONE_DATA tpData;
    int i;
//  if(GFL_UI_TP_Main( &tpData, TP_BUFFERING_JUST, 64 )==TP_OK){
    if(GFL_UI_TP_GetCont()==TRUE){
      u32 x,y;
      GFL_UI_TP_GetPointCont( &x, &y );
      tpData.Size = 1;
      tpData.TPDataTbl[0].x = x;
      tpData.TPDataTbl[0].y = y;
      SetTouchpanelData( &wk->MyTouchResult, &tpData, wk->brush_color, wk->brush );

      if(decide == TRUE){
        wk->MyTouchResult.size = 0;
      }
    }
  }


}


//------------------------------------------------------------------
/**
 * @brief   パレットとやめるボタンのセルアクター制御
 *
 * @param   act   アクターのポインタ
 * @param   button    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void PalButtonAppearChange( GFL_CLWK *act[], int no )
{
  int i;
  
  for(i=0;i<8;i++){
    // 押されたボタンはへこむ。ほかのボタンは戻る
    if(i==no){
      GFL_CLACT_WK_SetAnmSeq( act[i], pal_button_oam_table[i][2]+1 );
    }else{
      GFL_CLACT_WK_SetAnmSeq( act[i], pal_button_oam_table[i][2] );
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief   やめるボタンのオン・オフ
 *
 * @param   act   アクターのポインタ
 * @param   flag    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void EndButtonAppearChange( GFL_CLWK *act[], BOOL flag )
{
  if(flag==TRUE){
    GFL_CLACT_WK_SetAnmSeq( act[8], pal_button_oam_table[8][2]+1 );
  }else{
    GFL_CLACT_WK_SetAnmSeq( act[8], pal_button_oam_table[8][2] );
  }
}

//------------------------------------------------------------------
/**
 * @brief   お絵かきボード通常処理
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_MainNormal( OEKAKI_WORK *wk, int seq )
{
  NormalTouchFunc(wk);      //  タッチパネル処理


  
  if(GFL_NET_SystemGetCurrentID()==0){      // 親機の時
    if(OnlyParentCheck(wk)!=1){     // 一人じゃないか？
      LineDataSendRecv( wk );
    }
  }else{
      LineDataSendRecv( wk );
  }
  MoveCommCursor( wk );
  DrawBrushLine( wk->OekakiBoard, wk->AllTouchResult, wk->OldTouch, 1 );

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   終了選択処理の共通処理関数(後ろで描画等）
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void EndSequenceCommonFunc( OEKAKI_WORK *wk )
{
  MoveCommCursor( wk );
  DrawBrushLine( wk->OekakiBoard, wk->AllTouchResult, wk->OldTouch, 0 );
  
}


//------------------------------------------------------------------
/**
 * @brief   新しい子機がきたので親機が画像を送信しはじめる
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static int Oekaki_NewMember( OEKAKI_WORK *wk, int seq )
{
  // ●●●さんがはいってきました
  OS_Printf("newMemberは%d\n",wk->newMemberId);
  if(GFL_NET_SystemGetCurrentID()==wk->newMemberId){
    EndMessagePrint(wk, msg_oekaki_14, 0);
  }else{
    EndMessagePrint(wk, msg_oekaki_01, 0);
  }
  SetNextSequence( wk, OEKAKI_MODE_NEWMEMBER_WAIT );

  PMSND_PlaySE(OEKAKI_NEWMEMBER_SE);

  // 画像転送状態になったら輝度ダウン
  G2_SetBlendBrightness(  GX_BLEND_PLANEMASK_BG1|
              GX_BLEND_PLANEMASK_BG2|
              GX_BLEND_PLANEMASK_BG3,  -6);

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  
  return seq;

}

//------------------------------------------------------------------
/**
 * @brief   新しい子機用の画像受信待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static int Oekaki_NewMemberWait( OEKAKI_WORK *wk, int seq )
{
  int i;

  // 座標データが入っている時はカーソル座標を反映させる
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->MainActWork[i], 0 );
  }
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   子機乱入受信終了
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_NewMemberEnd( OEKAKI_WORK *wk, int seq )
{
  // 輝度ダウン解除
  G2_BlendNone();

  ChangeConnectMax(wk, 1);

  SetNextSequence( wk, OEKAKI_MODE );
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理

  //乱入済み
  wk->bookJoin = 0;
  OS_Printf("乱入成功\n");
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   お絵かきボード「やめる」を選択した時
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectPutString( OEKAKI_WORK *wk, int seq )
{
  if( EndMessageWait( wk->printStream ) ){
    TOUCH_SW_PARAM param;
    BOOL rc;

    // YES NO ウィンドウボタンの表示
    MI_CpuClear8(&param,sizeof(TOUCH_SW_PARAM));
    param.bg_frame  = GFL_BG_FRAME0_M;
    param.char_offs = YESNO_CHARA_OFFSET+YESNO_CHARA_W*YESNO_CHARA_H;
    param.pltt_offs = 8;
    param.x     = OEKAKI_YESNO_BUTTON_X; /*25*/
    param.y     = OEKAKI_YESNO_BUTTON_Y; /* 6*/
    param.type    = TOUCH_SW_TYPE_S;

    rc = OekakiInitYesNoWin(wk, &param);
    GF_ASSERT(rc);

    SetNextSequence( wk, OEKAKI_MODE_END_SELECT_WAIT );
  }
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   「やめますか？」はい・いいえ選択待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectWait( OEKAKI_WORK *wk, int seq )
{
  int result;

  // 誤送信を防ぐ
  wk->MyTouchResult.size = 0;

  if(wk->AllTouchResult[0].banFlag==OEKAKI_BAN_ON && GFL_NET_SystemGetCurrentID()!=0){
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理

    if(FakeEndYesNoSelect(wk)){
      // 親機に禁止されているときはＳＥのみ
      PMSND_PlaySE(OEKAKI_BS_SE);
    }

    return seq;
  }

  if(GFL_NET_SystemGetCurrentID() == 0 && wk->ridatu_bit != 0){
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }
  
  if(MyStatusGetNum(wk) != _get_connect_num(wk)){
    //一致していないなら「やめる」許可しない(子も通るここは親しか更新されないshareNumは見ない)
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }

  result = TOUCH_SW_Main( wk->TouchSubWindowSys );
  switch(result){       //やめますか？
  case TOUCH_SW_RET_YES:            //はい
    if(GFL_NET_SystemGetCurrentID()==0){    
      SetNextSequence( wk, OEKAKI_MODE_END_SELECT_PARENT );
      EndMessagePrint( wk, msg_oekaki_05, 1 );    // リーダーがやめると…
    }else{
      COMM_OEKAKI_END_CHILD_WORK coec;
      
      MI_CpuClear8(&coec, sizeof(COMM_OEKAKI_END_CHILD_WORK));
      coec.request = COEC_REQ_RIDATU_CHECK;
      coec.ridatu_id = GFL_NET_SystemGetCurrentID();
      
      wk->status_end = TRUE;
      wk->ridatu_wait = 0;

      SetNextSequence( wk, OEKAKI_MODE_END_SELECT_ANSWER_WAIT );
      GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(),CO_OEKAKI_END_CHILD, 
                        sizeof(COMM_OEKAKI_END_CHILD_WORK), &coec );
      BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_OFF );
    }
    OekakiResetYesNoWin(wk);

    //おえかき再描画
    GFL_BMPWIN_MakeTransWindow( wk->OekakiBoard );
  
    break;
  case TOUCH_SW_RET_NO:           //いいえ
    SetNextSequence( wk, OEKAKI_MODE );
    EndButtonAppearChange( wk->ButtonActWork, FALSE );
    BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_OFF );
    OekakiResetYesNoWin(wk);

    //おえかき再描画
    GFL_BMPWIN_MakeTransWindow( wk->OekakiBoard );
    
    // 親機は接続拒否を解除
    if(GFL_NET_SystemGetCurrentID()==0){
      CommStateSetLimitNum(wk, _get_connect_num(wk)+1);
      wk->banFlag = OEKAKI_BAN_OFF;
    }

    break;
  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   「やめますか？」＞「はい」、で親から離脱許可待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectAnswerWait( OEKAKI_WORK *wk, int seq )
{
  // 誤送信を防ぐ
  wk->MyTouchResult.size = 0;
  
  wk->ridatu_wait = 0;
  
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   「やめますか？」＞「はい」、で親から離脱許可待ち＞OK!
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectAnswerOK( OEKAKI_WORK *wk, int seq )
{
  // 誤送信を防ぐ
  wk->MyTouchResult.size = 0;

  if((wk->oya_share_num != _get_connect_num(wk))
      || (wk->oya_share_num != MyStatusGetNum(wk))){
    OS_TPrintf("share_nuM = %d, Comm = %d, My = %d, Bit = %d\n", wk->oya_share_num, _get_connect_num(wk), MyStatusGetNum(wk), _get_connect_bit(wk));
    wk->ridatu_wait = 0;
    SetNextSequence( wk, OEKAKI_MODE_END_SELECT_ANSWER_NG );
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }
  
  wk->ridatu_wait++;
  OS_TPrintf("ridatu_wait = %d\n", wk->ridatu_wait);
  if(wk->ridatu_wait > 30){
    COMM_OEKAKI_END_CHILD_WORK coec;
    
    MI_CpuClear8(&coec, sizeof(COMM_OEKAKI_END_CHILD_WORK));
    coec.request = COEC_REQ_RIDATU_EXE;
    coec.ridatu_id = GFL_NET_SystemGetCurrentID();

    GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), CO_OEKAKI_END_CHILD, 
                      sizeof(COMM_OEKAKI_END_CHILD_WORK), &coec );

    wk->ridatu_wait = 0;
    SetNextSequence( wk, OEKAKI_MODE_END_CHILD );
  }


  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   「やめますか？」＞「はい」、で親から離脱許可待ち＞NG!
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectAnswerNG( OEKAKI_WORK *wk, int seq )
{
  // 誤送信を防ぐ
  wk->MyTouchResult.size = 0;

  wk->status_end = FALSE;
  SetNextSequence( wk, OEKAKI_MODE );
  EndButtonAppearChange( wk->ButtonActWork, FALSE );
  
  OS_TPrintf("==========離脱強制キャンセル！===========\n");
  
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}

// OEKAKI_MODE_END_CHILD
//------------------------------------------------------------------
/**
 * @brief   子機終了メッセージ開始
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int       
 */
//------------------------------------------------------------------
static int  Oekaki_EndChild( OEKAKI_WORK *wk, int seq )
{
  // おえかきをやめました
  EndMessagePrint( wk, msg_oekaki_07, 1 );  

  SetNextSequence( wk, OEKAKI_MODE_END_CHILD_WAIT );

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}
        

// OEKAKI_MODE_END_CHILD_WAIT
//------------------------------------------------------------------
/**
 * @brief   子機終了メッセージ表示終了待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int       
 */
//------------------------------------------------------------------
static int  Oekaki_EndChildWait( OEKAKI_WORK *wk, int seq )
{     

  if( EndMessageWait( wk->printStream ) ){
    wk->wait = 0;
    SetNextSequence( wk, OEKAKI_MODE_END_CHILD_WAIT2 );
  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   メッセージ表示終了後少し待つ
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int       
 */
//------------------------------------------------------------------
static int  Oekaki_EndChildWait2( OEKAKI_WORK *wk, int seq )
{     

  if( ++wk->wait > OEKAKI_MESSAGE_END_WAIT ){
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, WIPE_FADE_BLACK, 16, 1, HEAPID_OEKAKI );
    seq = SEQ_OUT;            //終了シーケンスへ
  }

  EndSequenceCommonFunc( wk );      //終了選択時の共通処理
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   親機が終了するときはもう一度「リーダーがやめると…」と質問する
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectParent( OEKAKI_WORK *wk, int seq )
{
  if( EndMessageWait( wk->printStream ) ){
    TOUCH_SW_PARAM param;
    BOOL rc;

    // YES NO ウィンドウボタンの表示
    param.bg_frame  = GFL_BG_FRAME0_M;
    param.char_offs = YESNO_CHARA_OFFSET+YESNO_CHARA_W*YESNO_CHARA_H;
    param.pltt_offs = 8;
    param.x     = 25;
    param.y     = 6;
    param.type    = TOUCH_SW_TYPE_S;

    rc = OekakiInitYesNoWin(wk, &param);
    GF_ASSERT(rc);

    SetNextSequence( wk, OEKAKI_MODE_END_SELECT_PARENT_WAIT );

  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   親機の２回目の「はい・いいえ」
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndSelectParentWait( OEKAKI_WORK *wk, int seq )
{
  int result = TOUCH_SW_Main( wk->TouchSubWindowSys );

  if(wk->shareNum != MyStatusGetNum(wk) //一致していないなら「やめる」許可しない
      || wk->ridatu_bit != 0){  //離脱しようとしている子がいるなら許可しない
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }

  switch(result){       //やめますか？
  case TOUCH_SW_RET_YES:            //はい
    SetNextSequence( wk, OEKAKI_MODE_FORCE_END );
    GFL_NET_SendData( GFL_NET_GetNetHandle( GFL_NET_NETID_SERVER), 
                      CO_OEKAKI_END, NULL, 0 );  //終了通知
    // 親機（自分）の名前をWORDSET
    WORDSET_RegisterPlayerName( wk->WordSet, 0, 
                                Union_App_GetMystatus(wk->param->uniapp, 0)); 
    seq = SEQ_LEAVE;
    OS_Printf("OEKAKI_MODE_FORCE_ENDにかきかえ\n");
    OekakiResetYesNoWin(wk);
    break;
  case TOUCH_SW_RET_NO:           //いいえ
    SetNextSequence( wk, OEKAKI_MODE );
    EndButtonAppearChange( wk->ButtonActWork, FALSE );
    BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_OFF );
    OekakiResetYesNoWin(wk);

    // 親機は接続拒否を解除
    if(GFL_NET_SystemGetCurrentID()==0){
//      CommStateSetEntryChildEnable(TRUE);
      CommStateSetLimitNum( wk, _get_connect_num(wk)+1);
      wk->banFlag = OEKAKI_BAN_OFF;
    }
    break;
  }


  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   親機が終わりと言ったので終わる
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_ForceEnd( OEKAKI_WORK *wk, int seq )
{
  // 親機（自分）の名前をWORDSET
  WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(wk->param->uniapp, 0) ); 
  
  EndMessagePrint( wk, msg_oekaki_04, 1 );        // リーダーが抜けたので解散します。
  SetNextSequence( wk, OEKAKI_MODE_FORCE_END_WAIT );

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;

}


//------------------------------------------------------------------
/**
 * @brief   親機が終わりと言ったので終わる
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_ForceEndWait( OEKAKI_WORK *wk, int seq )
{
  if( EndMessageWait( wk->printStream ) ){
    SetNextSequence( wk, OEKAKI_MODE_FORCE_END_SYNCHRONIZE );
    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),
                                  OEKAKI_SYNCHRONIZE_END, WB_NET_GURUGURU);
    OS_Printf("同期開始\n");
    
  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;

}

//------------------------------------------------------------------
/**
 * @brief   全員の通信同期を待つ
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_ForceEndSynchronize( OEKAKI_WORK *wk, int seq )
{
  GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();
  if(GFL_NET_HANDLE_IsTimeSync( pNet,OEKAKI_SYNCHRONIZE_END, WB_NET_GURUGURU) || _get_connect_num(wk) == 1){
    OS_Printf("終了時同期成功  seq = %d\n", seq);
    OS_Printf("コネクト人数%d\n",_get_connect_num(wk));
//    wk->seq = OEKAKI_MODE_FORCE_END_WAIT_NOP;
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, WIPE_FADE_BLACK, 16, 1, HEAPID_OEKAKI );
    seq = SEQ_OUT;            //終了シーケンスへ
  }
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
  
}

//------------------------------------------------------------------
/**
 * @brief   何もしない
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_ForceEndWaitNop( OEKAKI_WORK *wk, int seq )
{
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   親一人になったので終了
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndParentOnly( OEKAKI_WORK *wk, int seq )
{
  EndMessagePrint( wk, msg_oekaki_06, 1 );        // リーダーが抜けたので解散します。
  SetNextSequence( wk, OEKAKI_MODE_END_PARENT_ONLY_WAIT );

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   一人になった文章を表示して終了シーケンスへ
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_EndParentOnlyWait( OEKAKI_WORK *wk, int seq )
{
  if( EndMessageWait( wk->printStream ) ){
    SetNextSequence( wk, OEKAKI_MODE_END_CHILD_WAIT2 );
  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;

}

//------------------------------------------------------------------
/**
 * @brief   ●●●さんがかえりました
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_LogoutChildMes( OEKAKI_WORK *wk, int seq )
{
  // ●●●さんがかえりました
  if( EndMessageWait( wk->printStream ) ){
    //表示中のメッセージがある場合は強制停止
    //GF_STR_PrintForceStop(wk->MsgIndex);
     PRINTSYS_PrintStreamDelete( wk->printStream );

  }

  EndMessagePrint( wk, msg_oekaki_03, 1 );  
  SetNextSequence( wk, OEKAKI_MODE_LOGOUT_CHILD_WAIT );
  PMSND_PlaySE(OEKAKI_NEWMEMBER_SE);

  // 接続可能人数を一旦現在の接続人数に落とす
  if(GFL_NET_SystemGetCurrentID()==0){
    ChangeConnectMax( wk, 0 );
  }
  wk->err_num = _get_connect_num(wk);

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   子機か抜けた文章終了待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Oekaki_LogoutChildMesWait( OEKAKI_WORK *wk, int seq )
{
  // 接続人数が１減るかチェック
  if(wk->err_num != 0 && _get_connect_num(wk) != wk->err_num){
    wk->err_num = 0;
  }

  if( EndMessageWait( wk->printStream ) ){
    SetNextSequence( wk, OEKAKI_MODE_LOGOUT_CHILD_CLOSE );
    wk->wait = 0;
  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;

}

//------------------------------------------------------------------
/**
 * @brief   表示終了ウェイト
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int       
 */
//------------------------------------------------------------------
static int  Oekaki_LogoutChildClose( OEKAKI_WORK *wk, int seq )
{     
  // 接続人数が１減るまでは待つ
  if(wk->err_num != 0 && _get_connect_num(wk) != wk->err_num){
    wk->err_num = 0;
  }

  if( ++wk->wait > OEKAKI_MESSAGE_END_WAIT && wk->err_num == 0 ){
    EndMessageWindowOff( wk );
    SetNextSequence( wk, OEKAKI_MODE );
    if(GFL_NET_SystemGetCurrentID() == 0){
      wk->banFlag = OEKAKI_BAN_OFF;
      ChangeConnectMax(wk, 1);
    }
  }

  EndSequenceCommonFunc( wk );      //終了選択時の共通処理
  return seq;
  
}




//==============================================================================
/**
 * @brief   どんな状態であっても強制的にメインシーケンスチェンジ
 *
 * @param   wk    
 * @param   seq   
 * @param   id    
 *
 * @retval  none    
 */
//==============================================================================
void OekakiBoard_MainSeqForceChange( OEKAKI_WORK *wk, int seq, u8 id  )
{
  switch(seq){
  case OEKAKI_MODE_NEWMEMBER: 
    if(wk->seq==OEKAKI_MODE_END_SELECT_WAIT || wk->seq==OEKAKI_MODE_END_SELECT_PARENT_WAIT){
      OekakiResetYesNoWin(wk);
    }
    EndButtonAppearChange( wk->ButtonActWork, FALSE );
    // 指定の子機の名前をWORDSETに登録（離脱・乱入時)
    WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(wk->param->uniapp, id) );  
    wk->newMemberId = id;
    wk->ridatu_bit = 0;
    OS_Printf("新しい人のID %d\n",id);
    break;
  case OEKAKI_MODE_NEWMEMBER_END:
    EndMessageWindowOff( wk );
    break;
  case OEKAKI_MODE_LOGOUT_CHILD:
    if(wk->status_end == TRUE){
      return; //自分自身が離脱処理中
    }
    WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(wk->param->uniapp, id) );  
    if(id==GFL_NET_SystemGetCurrentID()){
      // 自分が離脱する子機だった場合は「子機がいなくなたよ」とは言わない
      return;
    }

    if(GFL_NET_SystemGetCurrentID() == 0){
      wk->ridatu_bit &= 0xffff ^ id;
    }
    if(wk->seq==OEKAKI_MODE_END_SELECT_WAIT || wk->seq==OEKAKI_MODE_END_SELECT_PARENT_WAIT){
      OekakiResetYesNoWin(wk);

      //おえかき再描画
      GFL_BMPWIN_MakeTransWindow( wk->OekakiBoard );
    }
    EndButtonAppearChange( wk->ButtonActWork, FALSE );
    break;
  case OEKAKI_MODE_FORCE_END:
    break;
  case OEKAKI_MODE_END_SELECT_ANSWER_OK:
    break;
  case OEKAKI_MODE_END_SELECT_ANSWER_NG:
    break;
  default:
    GF_ASSERT( "指定以外のシーケンスチェンジが来た" );
    return;
  }
  SetNextSequence( wk, seq );

}

//==============================================================================
/**
 * @brief   何か処理をしてからメインループ内シーケンスチェンジ(通常状態であること）
 *
 * @param   wk    
 * @param   seq   このメインシーケンスにチェンジしたい
 *
 * @retval  none    
 */
//==============================================================================
void OekakiBoard_MainSeqCheckChange( OEKAKI_WORK *wk, int seq, u8 id  )
{
  // 通常状態なら
  if(wk->seq == OEKAKI_MODE){
    OS_Printf("子機%dの名前を登録\n",id);
    switch(seq){
    case OEKAKI_MODE_LOGOUT_CHILD:
      WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(wk->param->uniapp, id) );  
      if(id==GFL_NET_SystemGetCurrentID()){
        // 自分が離脱する子機だった場合は「子機がいなくなたよ」とは言わない
        return;
      }
      if(GFL_NET_SystemGetCurrentID() == 0){
        wk->ridatu_bit &= 0xffff ^ id;
      }
      SetNextSequence( wk, seq );
      break;
    
    default:
      GF_ASSERT( 0&&"指定以外のシーケンスチェンジが来た" );
      return;
    }
  }

}



//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//11520
//---------------------------------------------------------
// ブラシパターン
//---------------------------------------------------------

// どうしてもパレットデータを3bitに縮めてしまいたいので、透明色を8にして
// 色指定自体は0-7に当てている。0は透明色だがBD面を黒にして黒く見えるようにしている

// BMPデータは最低横8dot分必要なので、4x4のドットデータを作りたい時は
// ２バイトごとに参照されないデータがもう２バイト必要
static const u8 oekaki_brush[3][8][24]={

{ /*  0  */              /*  1  */               /*  2  */              /*  3  */
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x11,  0x00,0x00,  0x00,0x11, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x22,  0x00,0x00,  0x00,0x22, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x33,  0x00,0x00,  0x00,0x33, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x44,  0x00,0x00,  0x00,0x44, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x55,  0x00,0x00,  0x00,0x55, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x66,  0x00,0x00,  0x00,0x66, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x77,  0x00,0x00,  0x00,0x77, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,  0x00,0x88,  0x00,0x00,  0x00,0x88, 0x00,0x00,  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
},
{ /*  0  */              /*  1  */               /*  2  */              /*  3  */
  {0x00,0x00, 0x00,0x00, 0x00,0x11, 0x00,0x00,  0x10,0x11,  0x01,0x00,  0x10,0x11, 0x01,0x00,  0x00,0x11, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x22, 0x00,0x00,  0x20,0x22,  0x02,0x00,  0x20,0x22, 0x02,0x00,  0x00,0x22, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x33, 0x00,0x00,  0x30,0x33,  0x03,0x00,  0x30,0x33, 0x03,0x00,  0x00,0x33, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x44, 0x00,0x00,  0x40,0x44,  0x04,0x00,  0x40,0x44, 0x04,0x00,  0x00,0x44, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x55, 0x00,0x00,  0x50,0x55,  0x05,0x00,  0x50,0x55, 0x05,0x00,  0x00,0x55, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x66, 0x00,0x00,  0x60,0x66,  0x06,0x00,  0x60,0x66, 0x06,0x00,  0x00,0x66, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x77, 0x00,0x00,  0x70,0x77,  0x07,0x00,  0x70,0x77, 0x07,0x00,  0x00,0x77, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
  {0x00,0x00, 0x00,0x00, 0x00,0x88, 0x00,0x00,  0x80,0x88,  0x08,0x00,  0x80,0x88, 0x08,0x00,  0x00,0x88, 0x00,0x00, 0x00,0x00, 0x00,0x00,},
},                                                                     
{   /*  0  */              /*  1  */               /*  2  */              /*  3  */
  {0x10,0x11, 0x01,0x00, 0x11,0x11, 0x11,0x00,  0x11,0x11,  0x11,0x00,  0x11,0x11, 0x11,0x00,  0x11,0x11, 0x11,0x00, 0x10,0x11, 0x01,0x00,},
  {0x20,0x22, 0x02,0x00, 0x22,0x22, 0x22,0x00,  0x22,0x22,  0x22,0x00,  0x22,0x22, 0x22,0x00,  0x22,0x22, 0x22,0x00, 0x20,0x22, 0x02,0x00,},
  {0x30,0x33, 0x03,0x00, 0x33,0x33, 0x33,0x00,  0x33,0x33,  0x33,0x00,  0x33,0x33, 0x33,0x00,  0x33,0x33, 0x33,0x00, 0x30,0x33, 0x03,0x00,},
  {0x40,0x44, 0x04,0x00, 0x44,0x44, 0x44,0x00,  0x44,0x44,  0x44,0x00,  0x44,0x44, 0x44,0x00,  0x44,0x44, 0x44,0x00, 0x40,0x44, 0x04,0x00,},
  {0x50,0x55, 0x05,0x00, 0x55,0x55, 0x55,0x00,  0x55,0x55,  0x55,0x00,  0x55,0x55, 0x55,0x00,  0x55,0x55, 0x55,0x00, 0x50,0x55, 0x05,0x00,},
  {0x60,0x66, 0x06,0x00, 0x66,0x66, 0x66,0x00,  0x66,0x66,  0x66,0x00,  0x66,0x66, 0x66,0x00,  0x66,0x66, 0x66,0x00, 0x60,0x66, 0x06,0x00,},
  {0x70,0x77, 0x07,0x00, 0x77,0x77, 0x77,0x00,  0x77,0x77,  0x77,0x00,  0x77,0x77, 0x77,0x00,  0x77,0x77, 0x77,0x00, 0x70,0x77, 0x07,0x00,},
  {0x80,0x88, 0x08,0x00, 0x88,0x88, 0x88,0x00,  0x88,0x88,  0x88,0x00,  0x88,0x88, 0x88,0x00,  0x88,0x88, 0x88,0x00, 0x80,0x88, 0x08,0x00,},
},
};

/*


















*/



//==============================================================================
/**
 * @brief   描画開始位置がマイナス方向にあっても描画できるBmpWinPrintラッパー
 * @retval  none    
 */
//==============================================================================
static void _BmpWinPrint_Rap(
      GFL_BMPWIN * win, void * src,
      int src_x, int src_y, int src_dx, int src_dy,
      int win_x, int win_y, int win_dx, int win_dy )
{
  GFL_BMP_DATA *src_win;
  // X描画開始ポイントがマイナスか
  if(win_x < 0){
    int diff;
    diff = win_x*-1;
    if(diff>win_dx){  // そのマイナス分は転送幅を超えてないか
      diff = win_dx;  // 超えてたら転送幅と同じにする
    }

    // 転送参照開始ポイントと、転送幅を引く
    win_x   = 0;
    src_x  += diff;
    src_dx -= diff;
    win_dx -= diff;
  }

  // Y描画開始ポイントがマイナスか
  if(win_y < 0){
    int diff;
    diff = win_y*-1;
    if(diff>win_dy){  // そのマイナス分は転送幅を超えてないか
      diff = win_dy;  // 超えてたら転送幅と同じにする
    }

    // 転送参照開始ポイントと、転送幅を引く
    win_y   = 0;
    src_y  += diff;
    src_dy -= diff;
    win_dy -= diff;
  }

//  GFL_BMPWINPrint( win, src, src_x, src_y, src_dx, src_dy, win_x, win_y, win_dx, win_dy );
  src_win =  GFL_BMP_CreateWithData( src, 8, 8, 0, HEAPID_OEKAKI );
  GFL_BMP_Print( src, GFL_BMPWIN_GetBmp(win), src_x, src_y, win_x, win_y, src_dx, src_dy, 0 );
  GFL_BMP_Delete( src_win );
}


#define POINT_W ( 6 )
#define POINT_H ( 6 )
//------------------------------------------------------------------
/**
 * @brief   ライン描画
 *
 * @param   win   
 * @param   brush   
 * @param   px    
 * @param   py    
 * @param   sx    
 * @param   sy    
 * @param   count   
 * @param   flag    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void DrawPoint_to_Line( 
  GFL_BMPWIN *win, 
  const u8 *brush, 
  int px, int py, int *sx, int *sy, 
  int count, int flag )
{
  int dx, dy, s, step;
  int x1 = *sx;
  int y1 = *sy;
  int x2 = px;
  int y2 = py;

  // 初回は原点保存のみ
  if(count==0 && flag == 0){
    *sx = px;   *sy = py;
    return;
  }
  

  dx = MATH_IAbs(x2 - x1);  dy = MATH_IAbs(y2 - y1);
  if (dx > dy) {
    if (x1 > x2) {
      step = (y1 > y2) ? 1 : -1;
      s = x1;  x1 = x2;  x2 = s;  y1 = y2;
    } else step = (y1 < y2) ? 1: -1;
    _BmpWinPrint_Rap( win, (void*)brush,  0, 0, POINT_W, POINT_H, x1, y1, POINT_W, POINT_H );
    s = dx >> 1;
    while (++x1 <= x2) {
      if ((s -= dy) < 0) {
        s += dx;  y1 += step;
      };
      _BmpWinPrint_Rap( win, (void*)brush,  0, 0, POINT_W, POINT_H, x1, y1, POINT_W, POINT_H );
    }
  } else {
    if (y1 > y2) {
      step = (x1 > x2) ? 1 : -1;
      s = y1;  y1 = y2;  y2 = s;  x1 = x2;
    } else step = (x1 < x2) ? 1 : -1;
    _BmpWinPrint_Rap( win, (void*)brush,  0, 0, POINT_W, POINT_H, x1, y1, POINT_W, POINT_H );
    s = dy >> 1;
    while (++y1 <= y2) {
      if ((s -= dx) < 0) {
        s += dy;  x1 += step;
      }
      _BmpWinPrint_Rap( win, (void*)brush,  0, 0, POINT_W, POINT_H, x1, y1, POINT_W, POINT_H );
    }
  }
  
  
  *sx = px;     *sy = py;

}

static void Stock_OldTouch( TOUCH_INFO *all, OLD_TOUCH_INFO *stock )
{
  int i;
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    stock[i].size = all[i].size;
    if(all[i].size!=0){
      stock[i].x = all[i].x[all[i].size-1];
      stock[i].y = all[i].y[all[i].size-1];
    }
  }
}


static int debug_count;
//------------------------------------------------------------------
/**
 * @brief  通信で受信したタッチパネルの結果データを下に描画する
 *
 * @param   win   
 * @param   all   
 * @param   draw  メモリ上で行ったCGX変更を転送するか？(0:しない  1:する）
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void DrawBrushLine( GFL_BMPWIN *win, TOUCH_INFO *all, OLD_TOUCH_INFO *old, int draw )
{
  int px,py,i,r,flag=0, sx, sy;

//  OS_Printf("id0=%d,id1=%d,id2=%d,id3=%d,id4=%d\n",all[0].size,all[1].size,all[2].size,all[3].size,all[4].size);

  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    if(all[i].size!=0){
      if(old[i].size){
        sx = old[i].x-9;
        sy = old[i].y-17;
      }
      for(r=0;r<all[i].size;r++){
        px = all[i].x[r] - 9;
        py = all[i].y[r] - 17;
        // BG1面用BMP（お絵かき画像）ウインドウ確保
        DrawPoint_to_Line(win, oekaki_brush[all[i].brush][all[i].color], px, py, &sx, &sy, r, old[i].size);
        flag = 1;
      }
      
    }
  }
  if(flag && draw){
    
//    OS_Printf("write board %d times\n",debug_count++);
    GFL_BMPWIN_MakeTransWindow( win );
  }
  
  // 今回の最終座標のバックアップを取る   
    Stock_OldTouch(all, old);
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    all[i].size = 0;    // 一度描画したら座標情報は捨てる
  }
  
}

//------------------------------------------------------------------
/**
 * @brief   通信データからカーソル位置を移動させる
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void MoveCommCursor( OEKAKI_WORK *wk )
{
  int i;
  TOUCH_INFO *all = wk->AllTouchResult;
  
  // 座標データが入っている時はカーソル座標を反映させる
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    if(i!=GFL_NET_SystemGetCurrentID()){    // 自分のカーソルはタッチパネルから直接とる
      if(all[i].size!=0 && Union_App_GetMystatus(wk->param->uniapp,i)!=NULL){
        GFL_CLACT_WK_SetDrawEnable( wk->MainActWork[i], 1 );
        SetCursor_Pos( wk->MainActWork[i], all[i].x[all[i].size-1],  all[i].y[all[i].size-1]);
      }else{
        GFL_CLACT_WK_SetDrawEnable( wk->MainActWork[i], 0 );
      }
    }
  }
  
}
//------------------------------------------------------------------
/**
 * @brief   デバッグ用に自分で取得した情報を受信バッファにコピーする
 *
 * @param   wk    
 *
 * @retval  static    
 */
//------------------------------------------------------------------
static void DebugTouchDataTrans( OEKAKI_WORK *wk )
{
  wk->AllTouchResult[0] = wk->MyTouchResult;
}


//------------------------------------------------------------------
/**
 * カーソルのパレット変更（点滅）
 *
 * @param   CursorCol sinに渡すパラメータ（360まで）
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void CursorColTrans(u16 *CursorCol)
{
  fx32  sin;
  GXRgb tmp;
  int   r,g,b;

  *CursorCol+=20;
  if(*CursorCol>360){
    *CursorCol = 0;
  }

  sin = GFL_CALC_Sin360R(*CursorCol);
  g   = 15 +( sin * 10 ) / FX32_ONE;
  tmp = GX_RGB(29,g,0);


  GX_LoadOBJPltt((u16*)&tmp, ( 12 )*2, 2);
}

//------------------------------------------------------------------
/**
 * @brief   現在何人接続しているか？
 *
 * @param   none    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int OnlyParentCheck( OEKAKI_WORK *wk )
{
  int i,result;
  const MYSTATUS *status;

  result = 0;
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    status = Union_App_GetMystatus(wk->param->uniapp,i);
    if(status!=NULL){
      result++;
    }
  }

  return result;
}


//------------------------------------------------------------------
/**
 * @brief   オンライン状況を確認して上画面に名前を表示する
 *
 * @param   win   
 * @param   frame   
 * @param   color   
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void NameCheckPrint( GFL_BMPWIN *win[], PRINTSYS_LSB color, OEKAKI_WORK *wk )
{
  int i,id = GFL_NET_SystemGetCurrentID();
  int num;

  // 名前取得の状況に変化が無い場合は書き換えない
  if(!MyStatusCheck(wk)){
    return;
  }

  // ログイン状況に応じて上画面の矢印をON・OFFする
  for(i=0;i<5;i++){
    if(wk->TrainerStatus[i][0]==NULL){
      GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[i], 0 );
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[i], 1 );
    }
  }

  // それぞれの文字パネルの背景色でクリア
  for(i=0;i<5;i++){
    GFL_BMP_Fill( GFL_BMPWIN_GetBmp(win[i]), 0,0,OEKAKI_NAME_BMP_W*8, OEKAKI_NAME_BMP_H*8, 0 );
  }

  // 描画
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    if(wk->TrainerStatus[i][0]!=NULL){
      MyStatus_CopyNameString( wk->TrainerStatus[i][0], wk->TrainerName[i] );
      if(id==i){
//        GF_STR_PrintColor(  &win[i], FONT_TALK, wk->TrainerName[i], 0, 0, MSG_NO_PUT, 
//                            NAME_COL_MINE,NULL);
          PRINT_UTIL_PrintColor( &wk->printUtil[OEKAKI_PRINT_UTIL_NAME_WIN0+i], wk->printQue, 
                                  0, 0, wk->TrainerName[i], wk->font, NAME_COL_MINE );

      }else{
//        GF_STR_PrintColor(  &win[i], FONT_TALK, wk->TrainerName[i], 0, 0, MSG_NO_PUT, color,NULL);
          PRINT_UTIL_PrintColor( &wk->printUtil[OEKAKI_PRINT_UTIL_NAME_WIN0+i], wk->printQue, 
                                  0, 0, wk->TrainerName[i], wk->font, color );

      }
    }
    GFL_BMPWIN_MakeTransWindow( win[i] );
  }
  OS_Printf("名前かきかえしますよ\n");

}









#define PLATE_CHARA_OFFSET1 ( 12 )
#define PLATE_CHARA_OFFSET2 ( 16 )

static const u8 plate_num[4]={2,2,3,2};
static const u8 plate_table[4][3]={
  {8*8,17*8,0,},
  {8*8,17*8,0,},
  {5*8,12*8,18*8,},
  {8*8,17*8,0,},

};
static const u8 plate_chara_no[][5]={
  {PLATE_CHARA_OFFSET1,PLATE_CHARA_OFFSET1,},
  {PLATE_CHARA_OFFSET1,PLATE_CHARA_OFFSET1,},
  {PLATE_CHARA_OFFSET1,PLATE_CHARA_OFFSET1,PLATE_CHARA_OFFSET1,},
  {PLATE_CHARA_OFFSET1,PLATE_CHARA_OFFSET1,},
};

//------------------------------------------------------------------
/**
 * @brief   接続が確認された場合は名前を取得する
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static int ConnectCheck( OEKAKI_WORK *wk )
{
  int i,result=0;
  const MYSTATUS *status;
  STRCODE  *namecode;

  // 接続チェック
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    wk->ConnectCheck[i][0] = wk->ConnectCheck[i][1];

    if(Union_App_GetMemberNetBit(wk->param->uniapp)&(1<<i)){
      wk->ConnectCheck[i][0] = 1;
    }
//    wk->ConnectCheck[i][0] = CommIsConnect(i);

  }

  // 接続がいたら名前を反映させる
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    if(wk->ConnectCheck[i][0]){       // 接続しているか？

      status = Union_App_GetMystatus(wk->param->uniapp,i);
      if(status!=NULL){         // MYSTATUSは取得できているか？
        namecode = (STRCODE*)MyStatus_GetMyName(status);
        GFL_STR_SetStringCode( wk->TrainerName[i], namecode );
      }

    }
  }
  return 0;
}


//------------------------------------------------------------------
/**
 * @brief   タッチパネル情報の送受信を行う
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void LineDataSendRecv( OEKAKI_WORK *wk )
{
  if( GFL_NET_SystemGetCurrentID()==0 ){
    GFL_NETHANDLE *pNet = GFL_NET_InitHandle(GFL_NET_NETID_SERVER);

    // 親機は自分のタッチパネル情報を追加して送信する
    if(GFL_NET_IsEmptySendData(pNet)){  // パケットが空いてるなら
      wk->MyTouchResult.banFlag    = wk->banFlag;
      wk->ParentTouchResult[0] = wk->MyTouchResult;
      GFL_NET_SendData( GFL_NET_GetNetHandle( GFL_NET_NETID_SERVER), CO_OEKAKI_LINEPOS_SERVER, 
                        COMM_SEND_5TH_PACKET_MAX*OEKAKI_MEMBER_MAX, wk->ParentTouchResult );
    }
  }else{
    GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();
    if(GFL_NET_IsEmptySendData(pNet)){
      // 子機は自分のタッチパネル情報を親機に送信する
      GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), CO_OEKAKI_LINEPOS, 
                        COMM_SEND_5TH_PACKET_MAX, &wk->MyTouchResult );
    }
  }
}


//------------------------------------------------------------------
/**
 * @brief   現在のオンライン数を取得
 *
 * @param   none    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int MyStatusGetNum( OEKAKI_WORK *wk )
{
  return Union_App_GetMemberNum( wk->param->uniapp );
}


//------------------------------------------------------------------
/**
 * @brief   MYSTATUSの取得状況に変化があったか？
 *
 * @param   wk    
 *
 * @retval  int   あったら1,  無い場合は0
 */
//------------------------------------------------------------------
static int MyStatusCheck( OEKAKI_WORK *wk )
{
  int i,result=0;
  // 接続がいたら名前を反映させる
  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    wk->TrainerStatus[i][1] = wk->TrainerStatus[i][0];
    wk->TrainerStatus[i][0] = Union_App_GetMystatus(wk->param->uniapp,i);
  }

  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    if(wk->TrainerStatus[i][1] != wk->TrainerStatus[i][0]){
      result = 1;
    }
  }

  return result;
}


//------------------------------------------------------------------
/**
 * @brief   会話ウインドウ表示
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void EndMessagePrint( OEKAKI_WORK *wk, int msgno, int wait )
{
  // 文字列取得
  STRBUF *tempbuf;
  
  tempbuf = GFL_STR_CreateBuffer(TALK_MESSAGE_BUF_NUM,HEAPID_OEKAKI);
  GFL_MSG_GetString(  wk->MsgManager, msgno, tempbuf );
  WORDSET_ExpandStr( wk->WordSet, wk->TalkString, tempbuf );
  GFL_STR_DeleteBuffer(tempbuf);

  // 会話ウインドウ枠描画
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin),  0x0f0f );
  BmpWinFrame_Write( wk->MsgWin, WINDOW_TRANS_ON, 1, MESFRAME_PAL );


  // メッセージスピードを指定
  if(wait==0){
    PRINT_UTIL_PrintColor( &wk->printUtil[OEKAKI_PRINT_UTIL_MSG], wk->printQue, 
                           0, 0, wk->TalkString, wk->font, STRING_COL_MSG );
  }else{
    wk->printStream = PRINTSYS_PrintStream( wk->MsgWin, 0, 0, wk->TalkString, wk->font,
                                            MSGSPEED_GetWait(), wk->pMsgTcblSys, 
                                            1, HEAPID_OEKAKI, 0x0f0f );
  }

  // 文字列描画開始
//  wk->MsgIndex = GF_STR_PrintSimple( &wk->MsgWin, FONT_TALK, wk->TalkString, 0, 0, wait, NULL);

}

//------------------------------------------------------------------
/**
 * @brief   会話表示ウインドウ終了待ち
 *
 * @param   msg_index   
 *
 * @retval  int   0:表示中  1:終了
 */
//------------------------------------------------------------------
static int EndMessageWait( PRINT_STREAM *stream )
{
  PRINTSTREAM_STATE state;
  state = PRINTSYS_PrintStreamGetState( stream );
  if(state==PRINTSTREAM_STATE_PAUSE){
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
      PRINTSYS_PrintStreamReleasePause( stream );
    }
    return 0;
  }else if(state==PRINTSTREAM_STATE_DONE){
    PRINTSYS_PrintStreamDelete( stream );
    return 1;
  }
  return 0;
}


//------------------------------------------------------------------
/**
 * @brief   
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void EndMessageWindowOff( OEKAKI_WORK *wk )
{
  BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
}


//------------------------------------------------------------------
/**
 * @brief   接続人数がその時接続人数＋１になるように再設定する
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void ChangeConnectMax( OEKAKI_WORK *wk, int plus )
{
  if(GFL_NET_SystemGetCurrentID()==0){
    int num = _get_connect_num(wk)+plus;
    if(num>5){
      num = 5;
    }
    CommStateSetLimitNum(wk, num);
    OS_Printf("接続人数を %d人に変更\n",num);
  }

}

//------------------------------------------------------------------
/**
 * @brief  接続人数を監視して制御を変える 
 *
 * @param   wk    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int ConnectNumControl( OEKAKI_WORK *wk )
{
  int num;
  // ビーコンを書き換える
  num = MyStatusGetNum(wk);

  switch(num){
  case 1:
    if(wk->seq<=OEKAKI_MODE_END_SELECT_PARENT_WAIT){
      if (_get_connect_bit(wk) != 1){
        OS_Printf("一人ではなくなりました。\n");
        OS_Printf("bit:%d\n",_get_connect_bit(wk));
        wk->ireagalJoin = 1;
      }
      OS_Printf("OEKAKI_MODE_END_PARENT_ONLYにかきかえ\n");
//      wk->seq = OEKAKI_MODE_END_PARENT_ONLY;
      wk->next_seq = OEKAKI_MODE_END_PARENT_ONLY; //予約
      OS_Printf("接続制限を１にする");
          CommStateSetLimitNum(wk,1);
      // やめる選択中だったら強制リセット
      if(wk->yesno_flag){
        OekakiResetYesNoWin(wk);
        GFL_CLACT_WK_SetAnmSeq( wk->ButtonActWork[8], pal_button_oam_table[8][2] );
      }
      return SEQ_LEAVE;
    }
    break;
  case 2:case 3:case 4:
    // まだ入れるよ
#ifdef BEACON_CHANGE
//    Union_BeaconChange( UNION_PARENT_MODE_OEKAKI_FREE );
#endif

    // 接続人数が減った場合は接続最大人数も減らす
    if(num<wk->connectBackup){
      if(wk->banFlag==OEKAKI_BAN_ON){
        ChangeConnectMax( wk, 0 );
      }else{
        ChangeConnectMax( wk, 1 );
      }
    }
    break;
  case 5:

#ifdef BEACON_CHANGE
    // いっぱいです
//    Union_BeaconChange( UNION_PARENT_MODE_OEKAKINOW );
#endif
    break;
  }

  // 画像共有人数が減ったときは更新
  if(num<wk->connectBackup){
    wk->shareNum = _get_connect_num(wk);
    wk->shareBit = _get_connect_bit(wk);
    OS_TPrintf("接続人数が減ったのでshareNumを%d人に変更\n", _get_connect_num(wk));
    /*乱入中フラグ成立している状態で、人数が減った*/
    if(wk->bookJoin){/*乱入予定ビットと比較し、それが落ちていた場合は、乱入者が電源を切ったとみなす*/
      if (!(wk->shareBit&wk->joinBit)){
        //リミット制限再設定
        ChangeConnectMax( wk, 1 );
        //離脱解除
        wk->banFlag = OEKAKI_BAN_OFF;
        //乱入予約キャンセル
        wk->bookJoin = 0;
        wk->joinBit = 0;
        OS_Printf("乱入キャンセルされました\n");
      }
    }
  }

  // 接続人数を保存
  wk->connectBackup = MyStatusGetNum(wk);

  // 通信接続人数が画像共有人数よりも多くなった場合は離脱禁止フラグを立てる
  if(wk->shareNum < _get_connect_num(wk)){
    OS_Printf("ban_flag_on:%d,%d\n",wk->shareNum, _get_connect_num(wk));
    OS_Printf("乱入予定\n");
    wk->banFlag = OEKAKI_BAN_ON;
    //乱入を期待する
    wk->bookJoin = 1;
    //乱入しようとしている人のビットを取得
    wk->joinBit = wk->shareBit^_get_connect_bit(wk);
    GF_ASSERT( (wk->joinBit == 2)||
          (wk->joinBit == 4)||
          (wk->joinBit == 8)||
          (wk->joinBit == 16) );
    
  }

  return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   次に移行させたいシーケンス番号を入れる
 *
 * @param   wk    
 * @param   nextSequence    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void SetNextSequence( OEKAKI_WORK *wk, int nextSequence )
{
  wk->next_seq = nextSequence;
}


//------------------------------------------------------------------
/**
 * @brief   wk->seqのコントロールを行う
 *
 * @param   wk    
 *
 * @retval  void      
 */
//------------------------------------------------------------------
static void OekakiSequenceControl( OEKAKI_WORK *wk, int proc_seq )
{
  // シーケンスが違うなら
  if(wk->seq!=wk->next_seq){
    // 終了シーケンスじゃないことを確認して
    if(proc_seq!=SEQ_LEAVE){ 
      wk->seq = wk->next_seq;
    }else{
      // 一部実行制限のシーケンスじゃないことを確認して
      if(FuncTable[wk->next_seq].execOn_SeqLeave){
        OS_Printf("now_next:%d,%d\n",wk->seq,wk->next_seq);
        // シーケンス書き換え
        wk->seq = wk->next_seq;
      }
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief   タッチパネルのはい・いいえウィンドウの作成関数
 *
 * @param   wk    
 *
 * @retval  BOOL    TRUE：成功  FALSE：失敗       
 */
//------------------------------------------------------------------
static BOOL OekakiInitYesNoWin(OEKAKI_WORK *wk, TOUCH_SW_PARAM *param)
{
  if(!wk->yesno_flag){
    TOUCH_SW_Init( wk->TouchSubWindowSys, param );
    wk->yesno_flag = 1;
    return TRUE;
  }else{
    return FALSE;
  }
}

//------------------------------------------------------------------
/**
 * @brief   タッチパネルのはい・いいえウィンドウの消去関数
 *
 * @param   wk    
 *
 * @retval  void      
 */
//------------------------------------------------------------------
static void OekakiResetYesNoWin(OEKAKI_WORK *wk)
{
  if(wk->yesno_flag){
    TOUCH_SW_Reset( wk->TouchSubWindowSys );
    wk->yesno_flag = 0;
  }
}


//------------------------------------------------------------------
/**
 * @brief   YESNOボタンを禁止している間、嘘の検出を行う
 *
 * @param   wk    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int FakeEndYesNoSelect( OEKAKI_WORK  *wk )
{

  int button=GFL_UI_TP_HitTrg( fake_yesno_hittbl );
  if( button != GFL_UI_TP_HIT_NONE ){
    return 1;
  }
  return 0;
}


//------------------------------------------------------------------
/**
 * @brief   タッチパネルデータの格納処理
 *
 * @param   touchResult   
 * @param   tpData    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void SetTouchpanelData( TOUCH_INFO *touchResult, TP_ONE_DATA *tpData, int brush_color, int brush )
{
  int i,n;

  // 最初にサンプリングされた点を格納
  if(tpData->Size!=0){
    touchResult->x[0] = tpData->TPDataTbl[0].x;
    touchResult->y[0] = tpData->TPDataTbl[0].y;

    // 最後にサンプリングされた点を格納
    n = tpData->Size-1;
    touchResult->x[1] = tpData->TPDataTbl[n].x;
    touchResult->y[1] = tpData->TPDataTbl[n].y;

#if PLFIX_T1624
    // タッチ座標が0,0なら設定しない
    for( i=0; i<2; i++ ){
      if( (touchResult->x[i] + touchResult->y[i]) == 0 ){
        tpData->Size = 0; // なかったことにする
      }
    }
#endif
  }

  // 最大４回サンプリングされるはずだが、それでも送信データは２と格納する
  if(tpData->Size>=2){
    touchResult->size  = 2;
  }else{
    touchResult->size  = tpData->Size;
  }
  
  // 色・大きさ設定
  touchResult->color = brush_color;
  touchResult->brush = brush;

}


//----------------------------------------------------------------------------------
/**
 * @brief 接続人数取得
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_connect_num( OEKAKI_WORK *wk )
{
  return Union_App_GetMemberNum( wk->param->uniapp );
}

//----------------------------------------------------------------------------------
/**
 * @brief 接続状態をBITMAPで表現
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_connect_bit( OEKAKI_WORK *wk )
{
  return Union_App_GetMemberNetBit( wk->param->uniapp );
}
