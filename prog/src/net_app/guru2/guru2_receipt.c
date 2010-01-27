//******************************************************************************
/**
 * 
 * @file    guru2_receipt.c
 * @brief   ぐるぐる交換　受付
 * @author  Akito Mori(移植） / kagaya 
 * @data    2010.01.20
 *
 */
//******************************************************************************
#include <gflib.h>
#include "system/main.h"
#include "arc/arc_def.h"
#include "font/font.naix"
#include "gamesystem/msgspeed.h"
#include "arc/worldtrade.naix"
#include "print/wordset.h"
#include "arc/message.naix"
#include "msg/msg_guru2_receipt.h"
#include "system/wipe.h"
#include "system/bmp_winframe.h"
#include "print/printsys.h"
#include "poke_tool/monsno_def.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "net_app/union/union_beacon_tool.h"

#include "guru2_local.h"
#include "guru2_receipt.h"
//#include "comm_command_record.h"

// SE用定義
#define RECORD_MOVE_SE    (SEQ_SE_DP_SELECT)
#define RECORD_DECIDE_SE  (SEQ_SE_DP_SELECT)
#define RECORD_BS_SE    (SEQ_SE_DP_SELECT)

#define FIELDOBJ_PAL_START  ( 7 )

#define RECORD_1SEC_WAIT  ( 45 )

#define MESFRAME_PAL_INDEX  ( 10 )  // ウインドウフレームのパレットの行
#define MENUFRAME_PAL_INDEX ( 11 )

#include "guru2_2d.naix" //専用2Dリソースアーカイブ追加
#include "c_gear.naix"

//============================================================================================
//  定数定義
//============================================================================================
enum {
  SEQ_IN = 0,
  SEQ_MAIN,
  SEQ_LEAVE,
  SEQ_OUT,
};

enum{
  LIMIT_MODE_NONE,  ///<制限なし
  LIMIT_MODE_TRUE,  ///<制限発生中
  LIMIT_MODE_ONLY,  ///<制限発生中(一人のみ)
};

//==============================================================================
//  プロトタイプ
//==============================================================================
static void VBlankFunc( GFL_TCB *tcb, void * work );
static void VramBankSet(void);
static void BgInit( void );
static void InitWork( GURU2RC_WORK *wk, ARCHANDLE* p_handle );
static void FreeWork( GURU2RC_WORK *wk );
static void BgExit( void );
static void BgGraphicSet( GURU2RC_WORK * wk, ARCHANDLE* p_handle );
static void InitCellActor(GURU2RC_WORK *wk, ARCHANDLE* p_handle);
static void SetCellActor(GURU2RC_WORK *wk);
static void BmpWinInit(GURU2RC_WORK *wk );
static void BmpWinDelete( GURU2RC_WORK *wk );
static void SetCursor_Pos( GFL_CLWK *act, int x, int y );
static void EndSequenceCommonFunc( GURU2RC_WORK *wk );
static void CursorColTrans(u16 *CursorCol);
static BOOL NameCheckPrint( GFL_BMPWIN *win, int frame, PRINTSYS_LSB color, GURU2RC_WORK *wk );
static int  ConnectCheck( GURU2RC_WORK *wk );
static BOOL  MyStatusCheck( GURU2RC_WORK *wk );
static void RecordMessagePrint( GURU2RC_WORK *wk, int msgno, int all_put );
static int  EndMessageWait( PRINT_STREAM *stream );
static void EndMessageWindowOff( GURU2RC_WORK *wk );
static int  OnlyParentCheck( GURU2RC_WORK *wk );
static int  MyStatusGetNum( GURU2RC_WORK *wk );
static u32 MyStatusGetNumBit( GURU2RC_WORK *wk );
static void RecordDataSendRecv( GURU2RC_WORK *wk );
static void CenteringPrint(GFL_BMPWIN *win, STRBUF *strbuf, GFL_FONT *font);
static void SequenceChange_MesWait( GURU2RC_WORK *wk, int next );
static void TrainerObjFunc( GURU2RC_WORK *wk );
static void LoadFieldObjData( GURU2RC_WORK *wk, ARCHANDLE* p_handle );
static void FreeFieldObjData( GURU2RC_WORK *wk );
static void TransFieldObjData( NNSG2dCharacterData *CharaData[2], NNSG2dPaletteData *PalData[2], int id, int view, int sex );
static void TransPal( GFL_TCB* tcb, void *work );
static int GetTalkSpeed( GURU2RC_WORK *wk );
static int  RecordCorner_BeaconControl( GURU2RC_WORK *wk, int plus );
static void PadControl( GURU2RC_WORK *wk );
static int _get_key_trg( void );

// FuncTableからシーケンス遷移で呼ばれる関数
static int Record_MainInit( GURU2RC_WORK *wk, int seq );
static int Record_MainNormal( GURU2RC_WORK *wk, int seq );
static int Record_StartRecordCommand( GURU2RC_WORK *wk, int seq );
static int Record_RecordSendData( GURU2RC_WORK *wk, int seq );
static int Record_StartSelectWait( GURU2RC_WORK *wk, int seq );
static int Record_StartSelect( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectPutString( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectWait( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectAnswerWait( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectAnswerOK( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectAnswerNG( GURU2RC_WORK *wk, int seq );
static int Record_EndChild( GURU2RC_WORK *wk, int seq );
static int Record_EndChildWait( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectParent( GURU2RC_WORK *wk, int seq );
static int Record_EndSelectParentWait( GURU2RC_WORK *wk, int seq );
static int Record_ForceEnd( GURU2RC_WORK *wk, int seq );
static int Record_ForceEndWait( GURU2RC_WORK *wk, int seq );
static int Record_ForceEndSynchronize( GURU2RC_WORK *wk, int seq );
static int Record_EndParentOnly( GURU2RC_WORK *wk, int seq );
static int Record_EndParentOnlyWait( GURU2RC_WORK *wk, int seq );
static int Record_MessageWaitSeq( GURU2RC_WORK *wk, int seq );
static int Record_LogoutChildMes( GURU2RC_WORK *wk, int seq );
static int Record_LogoutChildClose( GURU2RC_WORK *wk, int seq );
static int Record_LogoutChildMesWait( GURU2RC_WORK *wk, int seq );
static int Record_NewMember( GURU2RC_WORK *wk, int seq );
static int Record_NewMemberEnd( GURU2RC_WORK *wk, int seq );
static int Record_FroceEndMesWait( GURU2RC_WORK *wk, int seq );

static int  Record_Guru2PokeSelStart( GURU2RC_WORK *wk, int seq );



// レコードコーナーメインシーケンス用関数配列定義
static int (* const FuncTable[])(GURU2RC_WORK *wk, int seq)={
  Record_MainInit,            //  RECORD_MODE_INIT  = 0, 
  Record_NewMember,           //  RECORD_MODE_NEWMEMBER,
  Record_NewMemberEnd,        //  RECORD_MODE_NEWMEMBER_END,
  Record_MainNormal,          //  RECORD_MODE,
  Record_EndSelectPutString,  //  RECORD_MODE_END_SELECT,
  Record_EndSelectWait,       //  RECORD_MODE_END_SELECT_WAIT,
  Record_EndSelectAnswerWait, //  RECORD_MODE_END_SELECT_ANSWER_WAIT
  Record_EndSelectAnswerOK,   //  RECORD_MODE_END_SELECT_ANSWER_OK
  Record_EndSelectAnswerNG,   //  RECORD_MODE_END_SELECT_ANSWER_NG
  Record_EndChild,            //  RECORD_MODE_END_CHILD,
  Record_EndChildWait,        //  RECORD_MODE_END_CHILD_WAIT,
  Record_EndSelectParent,     //  RECORD_MODE_END_SELECT_PARENT,
  Record_EndSelectParentWait, //  RECORD_MODE_END_SELECT_PARENT_WAIT,
  Record_ForceEnd,            //  RECORD_MODE_FORCE_END,
  Record_FroceEndMesWait,     //  RECORD_MODE_FORCE_END_MES_WAIT,
  Record_ForceEndWait,        //  RECORD_MODE_FORCE_END_WAIT,
  Record_ForceEndSynchronize, //  RECORD_MODE_FORCE_END_SYNCHRONIZE,
  Record_EndParentOnly,       //  RECORD_MODE_END_PARENT_ONLY,
  Record_EndParentOnlyWait,   //  RECORD_MODE_END_PARENT_ONLY_WAIT,
  Record_LogoutChildMes,      //  RECORD_MODE_LOGOUT_CHILD,
  Record_LogoutChildMesWait,  //  RECORD_MODE_LOGOUT_CHILD_WAIT,
  Record_LogoutChildClose,    //  RECORD_MODE_LOGOUT_CHILD_CLOSE,
  Record_StartSelect,         //  RECORD_MODE_START_SELECT,
  Record_StartSelectWait,     //  RECORD_MODE_START_SELECT_WAIT
  Record_StartRecordCommand,  //  RECORD_MODE_START_RECORD_COMMAND
  Record_RecordSendData,      //  RECORD_MODE_RECORD_SEND_DATA ここがレコードデータ受信チェック

  Record_MessageWaitSeq,      //  RECORD_MODE_MESSAGE_WAIT,
  
  //ぐるぐる
  Record_Guru2PokeSelStart,
};

//==============================================================================
//  ぐるぐる受付
//==============================================================================
//--------------------------------------------------------------
/**
 * ぐるぐる受付　初期化
 * @param proc  GFL_PROC *
 * @param seq   seq
 * @retval  GFL_PROC_RESULT GFL_PROC_RES_CONTINUE,GFL_PROC_RES_FINISH
 */
//--------------------------------------------------------------
GFL_PROC_RESULT Guru2Receipt_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  GURU2RC_WORK *wk;
  ARCHANDLE *p_handle;
  GURU2PROC_WORK *g2p = (GURU2PROC_WORK *)pwk;
  
  switch( *seq ){
  case 0:
    GFL_DISP_GX_InitVisibleControl();
    GFL_DISP_GXS_InitVisibleControl();
    GX_SetVisiblePlane( 0 );
    GXS_SetVisiblePlane( 0 );
  
    //ヒープ作成
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GURU2, GURU2_HEAPSIZE );
  
    p_handle = GFL_ARC_OpenDataHandle( ARCID_GURU2, HEAPID_GURU2 );
    GF_ASSERT( p_handle );
    
    //受付用ワーク確保
    wk = GFL_PROC_AllocWork( proc, sizeof(GURU2RC_WORK), HEAPID_GURU2 );
    GFL_STD_MemFill( wk, 0, sizeof(GURU2RC_WORK) );
    
    g2p->g2r = wk;
    wk->g2p = g2p;
    wk->g2c = g2p->g2c;
    
    GFL_BG_Init( HEAPID_GURU2 );
  
    // 文字列マネージャー生成
    wk->WordSet    = WORDSET_Create( HEAPID_GURU2 );
    wk->MsgManager = GFL_MSG_Create(  GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                     NARC_message_guru2_receipt_dat, HEAPID_GURU2 );
    wk->font       = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                              GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_GURU2 );
   
    // VRAM バンク設定
    VramBankSet();
      
    // BGLレジスタ設定
    BgInit();          
    
  /* WIPEリセットにより画面が見えてしまうバグ*/
  //  WIPE_ResetBrightness( WIPE_DISP_MAIN );
  //  WIPE_ResetBrightness( WIPE_DISP_SUB );
  
    // ワイプフェード開始
    WIPE_SYS_Start(
      WIPE_PATTERN_WMS, WIPE_TYPE_HOLEIN,
      WIPE_TYPE_HOLEIN, WIPE_FADE_BLACK, 16, 1, HEAPID_GURU2 );
  
    //BGグラフィックセット
    BgGraphicSet( wk, p_handle );
  
    // VBlank関数セット
    wk->vintr_tcb = GFUser_VIntr_CreateTCB( VBlankFunc, wk, 1 );
    // メッセージストリーム処理用タスクシステム生成
    wk->pMsgTcblSys = GFL_TCBL_Init( HEAPID_GURU2, HEAPID_GURU2, 32 , 32 );
    wk->printQue    = PRINTSYS_QUE_Create( HEAPID_GURU2 );
  
    // ワーク初期化
    InitWork( wk, p_handle );
  
  
    // CellActorシステム初期化
    InitCellActor(wk, p_handle);
      
    // CellActro表示登録
    SetCellActor(wk);
  
    // BMPWIN登録・描画
    BmpWinInit(wk);
    
    // 通信コマンドを交換リスト用に変更
    Guru2Comm_CommandInit( g2p->g2c );
    
    // レコードコーナーモードに変更
    // CommStateUnionGuru2Change();
    
      // 3台まで接続可能に書き換え
      // CommStateSetLimitNum(3);
    
    // 親だったら「レコード通信募集中」にビーコン書き換え
    //if(GFL_NET_SystemGetCurrentID()==0){
    //  Union_BeaconChange( UNION_PARENT_MODE_GURU2_FREE );
    //}
    
    GFL_NET_WirelessIconEasyFunc();
  
  //  MakeSendData( wk->g2p->param.sv, &wk->g2c->send_data );
  
    GFL_ARC_CloseDataHandle( p_handle );

    (*seq)++;
    break;
  case 1:
    (*seq) = 0;
    return( GFL_PROC_RES_FINISH );
  }
  
  return( GFL_PROC_RES_CONTINUE );
}

static UNION_APP_PTR _get_unionwork(GURU2RC_WORK *wk)
{
  return wk->g2p->param.guru2union->uniapp;
}

//--------------------------------------------------------------
/**
 * ぐるぐる交換受付　メイン
 * @param proc  GFL_PROC *
 * @param seq   seq
 * @retval  GFL_PROC_RESULT GFL_PROC_RES_CONTINUE,GFL_PROC_RES_FINISH
 */
//--------------------------------------------------------------
GFL_PROC_RESULT Guru2Receipt_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  GURU2RC_WORK *wk = (GURU2RC_WORK *)mywk;

  if(GFL_NET_SystemGetCurrentID() == 0 && wk->g2c->ridatu_bit != 0){
    wk->g2c->ridatu_bit &= Union_App_GetMemberNetBit(_get_unionwork(wk));
  }
  
  switch( wk->proc_seq ){
  case SEQ_IN:  //　ワイプ処理待ち
    if( WIPE_SYS_EndCheck() ){
      wk->proc_seq = SEQ_MAIN;
      
      // 自分が子機で接続台数が２台以上だった場合はもう絵が描かれている
      if(GFL_NET_SystemGetCurrentID()!=0){
        if(MyStatusGetNum(wk)>2){
          Guru2Comm_SendData(wk->g2c,G2COMM_RC_CHILD_JOIN, NULL, 0);
        }
      }
    }
    break;
  case SEQ_MAIN:    // カーソル移動
    // シーケンス毎の動作
    if(FuncTable[wk->seq]!=NULL){
      wk->proc_seq = (*FuncTable[wk->seq])( wk, wk->proc_seq );
    }
    
    if(wk->g2c->record_execute == FALSE){ //レコード混ぜ中の時は更新しない
      NameCheckPrint(
        wk->TrainerNameWin, 0, PRINTSYS_LSB_Make(1, 3, 0), wk );
    }
    
    TrainerObjFunc(wk);

    // 変化した人数に応じてビーコンを変える
    if(GFL_NET_SystemGetCurrentID()==0){
      int temp = RecordCorner_BeaconControl( wk, 1 );
      if(wk->proc_seq==SEQ_MAIN){
        // 終了シーケンスなどに流れていなければ変更する
        wk->proc_seq = temp;
      }
    }

    break;
  case SEQ_LEAVE:
    if(FuncTable[wk->seq]!=NULL){
      wk->proc_seq = (*FuncTable[wk->seq])( wk, wk->proc_seq );
    }
    break;
  case SEQ_OUT:
    if( WIPE_SYS_EndCheck() ){
      return GFL_PROC_RES_FINISH;
    }
    break;
  }
  
  GFL_CLACT_SYS_Main();         // セルアクター常駐関数

  return GFL_PROC_RES_CONTINUE;
}

#define DEFAULT_NAME_MAX    18

// ダイヤ・パールで変わるんだろう
#define MALE_NAME_START     0
#define FEMALE_NAME_START   18

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：終了
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT Guru2Receipt_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  int i;
  GURU2RC_WORK *wk = (GURU2RC_WORK *)mywk;
  
  // メッセージ表示用システム解放
  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_TCBL_Exit( wk->pMsgTcblSys );

  // セルアクターリソース解放

  // キャラ破棄
  GFL_CLGRP_CGR_Release( wk->resObjTbl[GURU2_CLACT_RES_CHR] );

  // パレット破棄
  GFL_CLGRP_PLTT_Release( wk->resObjTbl[GURU2_CLACT_RES_PLTT] );
  
  // セル・アニメ破棄
  GFL_CLGRP_CELLANIM_Release( wk->resObjTbl[GURU2_CLACT_RES_CELL] );
  
  // キャラ・パレット・セル・セルアニメのリソースマネージャー破棄
  //for(i=0;i<CLACT_RESOURCE_NUM;i++){
  //  CLACT_U_ResManagerDelete(wk->resMan[i]);
  //}
  
  // セルアクターセット破棄
  GFL_CLACT_UNIT_Delete( wk->clUnit );

  //OAMレンダラー破棄
  GFL_CLACT_SYS_Delete();

  // BMPウィンドウ開放
  BmpWinDelete( wk );

  // BGL削除
  BgExit( );

  // 通信終了
  
  // メッセージマネージャー・ワードセットマネージャー解放
  GFL_MSG_Delete( wk->MsgManager );
  WORDSET_Delete( wk->WordSet );

  // 入れ替わっていた上下画面出力を元に戻す
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

  GFL_TCB_DeleteTask( wk->vintr_tcb );  // VBlank関数解除
  
  //ワーク反映
  wk->g2p->receipt_ret = wk->end_next_flag;
  wk->g2p->receipt_num = MyStatusGetNum(wk);
  wk->g2p->receipt_bit = MyStatusGetNumBit(wk);
  
  //ワーク解放
  FreeWork( wk );
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_GURU2 );
  return GFL_PROC_RES_FINISH;
}

static const u8 palanimetable[][2]={
  { 8,2,},
  { 8,3,},
  { 8,4,},
  {24,0,},
  { 0,0xff,},
};

//------------------------------------------------------------------
/**
 * @brief   パレット転送タスク
 *
 * @param   tcb   
 * @param   work    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void TransPal( GFL_TCB* tcb, void *work )
{
  GURU2RC_WORK     *wk  = (GURU2RC_WORK *)work;
  VTRANS_PAL_WORK *pal = &wk->palwork;
  
  // パレットアニメ
  if(pal->sw){
    if(pal->wait>palanimetable[pal->seq][0]){
      u16 *src;
      pal->wait=0;
      pal->seq++;
      if(palanimetable[pal->seq][1]==0xff){
        pal->seq = 0;
      }
      src = (u16*)pal->palbuf->pRawData;
      GX_LoadOBJPltt(&src[16*palanimetable[pal->seq][1]], 0, 32);
//      OS_Printf("パレット転送 seq = %d \n",pal->seq);
    }else{
      wk->palwork.wait++;
    }
    
  }


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
  
  // パレット転送
  TransPal( tcb, work );
  
  GFL_CLACT_SYS_VBlankFunc();
  
  GFL_BG_VBlankFunc();
}


static const GFL_DISP_VRAM Guru2DispVramDat = {

    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット

    GX_VRAM_OBJ_128_B,          // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット

    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,        // テクスチャパレットスロット
  
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

  GFL_DISP_SetBank( &Guru2DispVramDat );
}

//--------------------------------------------------------------------------------------------
/**
 * BG設定
 *
 * @param ini   BGLデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgInit()
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
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME0_S );


  }

  // メイン画面1
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
  }

  // メイン画面背景
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x08000, GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }


  // サブ画面テキスト面
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  GFL_BG_FRAME0_M );
  }

  // サブ画面背景面
  { 
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_GURU2 );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, HEAPID_GURU2 );

}


#define TALK_MESSAGE_BUF_NUM  ( 90*2 )
#define TITLE_MESSAGE_BUF_NUM ( 20*2 )

//------------------------------------------------------------------
/**
 * お絵かきワーク初期化
 *
 * @param   wk    GURU2RC_WORK*
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void InitWork( GURU2RC_WORK *wk, ARCHANDLE* p_handle )
{
  int i;

  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    wk->TrainerName[i] = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, HEAPID_GURU2 );
    wk->TrainerStatus[i][0] = NULL;
    wk->TrainerStatus[i][1] = NULL;
    wk->TrainerReq[i]       = RECORD_EXIST_NO;

  }
  // 文字列バッファ作成
  wk->TalkString  = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_GURU2 );
  wk->TitleString = GFL_STR_CreateBuffer( TITLE_MESSAGE_BUF_NUM, HEAPID_GURU2 );

  wk->seq = RECORD_MODE_INIT;
  
  // レコードこうかんぼしゅう中！文字列取得
  GFL_MSG_GetString(  wk->MsgManager, msg_guru2_receipt_title_01, wk->TitleString );

  // フィールドOBJ画像読み込み
  LoadFieldObjData( wk, p_handle );

  wk->ObjPaletteTable = UnionView_PalleteTableAlloc( HEAPID_GURU2 );


  // パレットアニメ用ワーク初期化
  wk->palwork.sw       = 0;
  wk->palwork.wait     = 0;
  wk->palwork.seq      = 0;
  wk->palwork.paldata  = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_guru2_2d_record_s_obj_nclr, &wk->palwork.palbuf, HEAPID_GURU2 );
  wk->connectBackup    = 0;
  wk->YesNoMenuWork    = NULL;
  wk->g2c->shareNum    = 2;
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
static void FreeWork( GURU2RC_WORK *wk )
{
  int i;

  // フィールドOBJ画像解放
  FreeFieldObjData( wk );

  // 機械のパレットアニメ用データ解放
  GFL_HEAP_FreeMemory( wk->palwork.paldata );

  // ユニオンOBJのパレットデータ解放
  GFL_HEAP_FreeMemory( wk->ObjPaletteTable );

  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    GFL_STR_DeleteBuffer( wk->TrainerName[i] );
  }
  GFL_STR_DeleteBuffer( wk->TitleString ); 
  GFL_STR_DeleteBuffer( wk->TalkString ); 

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

  GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

  GFL_BG_Exit();
}


//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param wk    ポケモンリスト画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgGraphicSet( GURU2RC_WORK * wk, ARCHANDLE* p_handle )
{
  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle,  NARC_guru2_2d_record_s_nclr, PALTYPE_MAIN_BG, 0, 16*16*2, HEAPID_GURU2 );
  GFL_ARC_UTIL_TransVramPalette( ARCID_C_GEAR, NARC_c_gear_c_gear_NCLR,  PALTYPE_SUB_BG,  0, 16*5,   HEAPID_GURU2 );
  
  // 会話フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 13*0x20, 32, HEAPID_GURU2 );

  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_C_GEAR, NARC_c_gear_c_gear_NCGR,   GFL_BG_FRAME2_S, 0, 0, 0, HEAPID_GURU2);
  GFL_ARC_UTIL_TransVramScreen(      ARCID_C_GEAR, NARC_c_gear_c_gear00_NSCR, GFL_BG_FRAME2_S, 0, 0, 0, HEAPID_GURU2);

  // メイン画面BG1キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_guru2_2d_subbg_lz_ncgr,  GFL_BG_FRAME1_M, 0, 32*8*0x20, 1, HEAPID_GURU2);

  // メイン画面BG1スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_guru2_2d_subbg_lz_nscr,  GFL_BG_FRAME1_M, 0, 32*24*2, 1, HEAPID_GURU2);

  // メイン画面会話ウインドウグラフィック転送
  BmpWinFrame_GraphicSet(
     GFL_BG_FRAME0_M, 1, MESFRAME_PAL_INDEX,
     0, HEAPID_GURU2 );

  BmpWinFrame_GraphicSet(
         GFL_BG_FRAME0_M, 1+TALK_WIN_CGX_SIZ, MENUFRAME_PAL_INDEX, 0, HEAPID_GURU2 );

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
static void InitCellActor(GURU2RC_WORK *wk, ARCHANDLE* p_handle)
{
  int i;
  
  
  // OAMマネージャーの初期化
  NNS_G2dInitOamManagerModule();

  // 共有OAMマネージャ作成
  // レンダラ用OAMマネージャ作成
  // ここで作成したOAMマネージャをみんなで共有する
  
  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &Guru2DispVramDat, HEAPID_GURU2 );
  
  // セルアクター初期化
  wk->clUnit = GFL_CLACT_UNIT_Create( 30, 0, HEAPID_GURU2 );
  GF_ASSERT( wk->clUnit );
  
  //---------上画面人物OBJ読み込み-------------------

  //chara読み込み
  wk->resObjTbl[GURU2_CLACT_RES_CHR] = GFL_CLGRP_CGR_Register( p_handle, NARC_guru2_2d_obj_s_lz_ncgr, 1, 
                                                               CLSYS_DRAW_MAIN, HEAPID_GURU2 );
  //pal読み込み
  wk->resObjTbl[GURU2_CLACT_RES_PLTT] = GFL_CLGRP_PLTT_Register( p_handle, NARC_guru2_2d_record_s_obj_nclr,
                                                               CLSYS_DRAW_MAIN, 0, HEAPID_GURU2 );
//  CLACT_U_ResManagerResAddArcPltt_ArcHandle(wk->resMan[CLACT_U_PLTT_RES],
//              p_handle, NARC_guru2_2d_record_s_obj_nclr, 0, 2, NNS_G2D_VRAM_TYPE_2DMAIN, 15, HEAPID_GURU2);

  //cell読み込み
  wk->resObjTbl[GURU2_CLACT_RES_CELL] = GFL_CLGRP_CELLANIM_Register( p_handle, 
                                            NARC_guru2_2d_obj_s_lz_ncer, 
                                            NARC_guru2_2d_obj_s_lz_nanr, 
                                            HEAPID_GURU2 );

//  CLACT_U_ResManagerResAddArcKindCell_ArcHandle(wk->resMan[CLACT_U_CELL_RES],
//              p_handle, NARC_guru2_2d_obj_s_lz_ncer, 1, 2, CLACT_U_CELL_RES,HEAPID_GURU2);

  //同じ関数でanim読み込み
//  wk->resObjTbl[CHARA_RES][CLACT_U_CELLANM_RES] = CLACT_U_ResManagerResAddArcKindCell_ArcHandle(wk->resMan[CLACT_U_CELLANM_RES],
//              p_handle, NARC_guru2_2d_obj_s_lz_nanr, 1, 2, CLACT_U_CELLANM_RES,HEAPID_GURU2);


  // リソースマネージャーから転送
}

#define TRAINER_NAME_POS_X    ( 24 )
#define TRAINER_NAME_POS_Y    ( 32 )
#define TRAINER_NAME_POS_SPAN ( 32 )

#define TRAINER_NAME_WIN_X    (  2 )
#define TRAINER1_NAME_WIN_Y   (  6 )
#define TRAINER2_NAME_WIN_Y   (  7 )

static const u16 obj_pos_tbl[][2]={
  { 204,  84   },
  { 204,  61-8 },
  { 233,  80-8 },
  { 223, 111-8 },
  { 184, 111-8 },
  { 174,  80-8 },
};

//------------------------------------------------------------------
/**
 * セルアクター登録
 *
 * @param   wk      GURU2RC_WORK*
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void SetCellActor(GURU2RC_WORK *wk)
{
  int i;

  {
    //登録情報格納
    GFL_CLWK_DATA add;
  
    add.bgpri   = 1;
    add.softpri = 0;

    //セルアクター表示開始

    // メイン画面用(人物の登録）
    for(i=0;i<5;i++){
      add.pos_x = obj_pos_tbl[i+1][0];
      add.pos_y = obj_pos_tbl[i+1][1];
      wk->MainActWork[i+1] = GFL_CLACT_WK_Create( wk->clUnit, 
                                               wk->resObjTbl[GURU2_CLACT_RES_CHR],
                                               wk->resObjTbl[GURU2_CLACT_RES_PLTT],
                                               wk->resObjTbl[GURU2_CLACT_RES_CELL],
                                               &add, CLSYS_DEFREND_MAIN, HEAPID_GURU2);
      GFL_CLACT_WK_SetAutoAnmFlag( wk->MainActWork[i+1],1 );
      GFL_CLACT_WK_SetAnmSeq( wk->MainActWork[i+1], 27+(i-1)*2 );
      GFL_CLACT_WK_SetDrawEnable( wk->MainActWork[i+1], 0 );
    }

  } 
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON );   // メイン画面OBJ面ＯＮ
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF );  // サブ画面OBJ面OFF
  
}

// おえかきボードBMP（下画面）
#define OEKAKI_BOARD_POSX  ( 1 )
#define OEKAKI_BOARD_POSY  ( 2 )
#define OEKAKI_BOARD_W   ( 30 )
#define OEKAKI_BOARD_H   ( 15 )


// 名前表示BMP（上画面）
#define RECORD_NAME_BMP_W  ( 16 )
#define RECORD_NAME_BMP_H  ( 11 )
#define RECORD_NAME_BMP_SIZE (RECORD_NAME_BMP_W * RECORD_NAME_BMP_H)


// 「やめる」文字列BMP（下画面）
#define OEKAKI_END_BMP_X  ( 26 )
#define OEKAKI_END_BMP_Y  ( 20 )
#define OEKAKI_END_BMP_W  ( 6  )
#define OEKAKI_END_BMP_H  ( 2  )


// 会話ウインドウ表示位置定義
#define RECORD_TALK_X   (  2 )
#define RECORD_TALK_Y   (  19 )

#define RECORD_TITLE_X    (   3  )
#define RECORD_TITLE_Y    (   1  )
#define RECORD_TITLE_W    (  26  )
#define RECORD_TITLE_H    (   2  )

#define MESSAGE_WINDOW_W  ( 27 )
#define MESSAGE_WINDOW_H  (  4 )

#define RECORD_MSG_WIN_OFFSET     ( 1 + TALK_WIN_CGX_SIZ   + MENU_WIN_CGX_SIZ)
#define RECORD_YESNO_WIN_OFFSET   ( RECORD_MSG_WIN_OFFSET  + RECORD_NAME_BMP_SIZE )

//------------------------------------------------------------------
/**
 * BMPWIN処理（文字パネルにフォント描画）
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void BmpWinInit(GURU2RC_WORK *wk )
{
  // ---------- メイン画面 ------------------

  // BG1面BMP（やめる）ウインドウ確保・描画
  wk->EndWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_S,
                                  OEKAKI_END_BMP_X, OEKAKI_END_BMP_Y, 
                                  OEKAKI_END_BMP_W, OEKAKI_END_BMP_H, 13,  
                                  GFL_BMP_CHRAREA_GET_B );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->EndWin), 0 );

  // ----------- サブ画面名前表示BMP確保 ------------------
  // BG0面BMP（会話ウインドウ）確保
  wk->MsgWin = GFL_BMPWIN_Create( GFL_BG_FRAME0_M,
    RECORD_TALK_X, RECORD_TALK_Y, MESSAGE_WINDOW_W, MESSAGE_WINDOW_H, 13,  GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin), 0x0f0f );

  // BG0面BMP（タイトルウインドウ）確保
  wk->TitleWin = GFL_BMPWIN_Create( GFL_BG_FRAME0_M,
                                    RECORD_TITLE_X, RECORD_TITLE_Y, 
                                    RECORD_TITLE_W, RECORD_TITLE_H, 15,  
                                    GFL_BMP_CHRAREA_GET_B );
  CenteringPrint(wk->TitleWin, wk->TitleString, wk->font );

  // 名前表示ウインドウ
  {
    int i;
    wk->TrainerNameWin = GFL_BMPWIN_Create( GFL_BG_FRAME0_M,  
                                               TRAINER_NAME_WIN_X, TRAINER1_NAME_WIN_Y, 
                                               RECORD_NAME_BMP_W, RECORD_NAME_BMP_H, 15,  
                                               GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TrainerNameWin), 0 );

    //最初に見えている面なので文字パネル描画と転送も行う
    NameCheckPrint( wk->TrainerNameWin, 0, PRINTSYS_LSB_Make(1, 3, 0), wk );


  }

  // BMPとPRINT_UTILの関連付け
  PRINT_UTIL_Setup( &wk->printUtil, wk->TitleWin );

  OS_Printf("YESNO win offset = %d\n",RECORD_YESNO_WIN_OFFSET);

/*
3713 2D4A 5694 089D 5EBF 7DC0 76EF 0000
0000 0000 0000 7FFF 62FF 6F7B 7F52 7FFF

*/



} 

//------------------------------------------------------------------
/**
 * @brief   センタリングしてプリント
 *
 * @param   win   GFL_BMPWIN
 * @param   strbuf  
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void CenteringPrint(GFL_BMPWIN *win, STRBUF *strbuf, GFL_FONT *font)
{
  int length = PRINTSYS_GetStrWidth( strbuf, font, 0 );
  int x      = (RECORD_TITLE_W*8-length)/2;
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win), 0x0000 );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(win), x, 1, strbuf, font, PRINTSYS_LSB_Make(1, 4, 0) );
  GFL_BMPWIN_MakeTransWindow( win );
}


// はい・いいえBMP（下画面）
#define YESNO_WIN_FRAME_CHAR  ( 1 + TALK_WIN_CGX_SIZ )
#define YESNO_CHARA_OFFSET    ( 1 + TALK_WIN_CGX_SIZ + MENU_WIN_CGX_SIZ + MESSAGE_WINDOW_W*MESSAGE_WINDOW_H )
#define YESNO_WINDOW_X      ( 22 )
#define YESNO_WINDOW_Y1     (  7 )
#define YESNO_WINDOW_Y2     ( 13 )
#define YESNO_CHARA_W     (  8 )
#define YESNO_CHARA_H     (  4 )


//------------------------------------------------------------------
/**
 * @brief   確保したBMPWINを解放
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void BmpWinDelete( GURU2RC_WORK *wk )
{

  GFL_BMPWIN_Delete( wk->TrainerNameWin );
  GFL_BMPWIN_Delete( wk->TitleWin );
  GFL_BMPWIN_Delete( wk->EndWin );
  GFL_BMPWIN_Delete( wk->MsgWin );


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
  pos.y = y;
  
  GFL_CLACT_WK_SetPos( act, &pos, CLSYS_DEFREND_MAIN );

}

//==============================================================================
//  受付　メインシーケンス
//==============================================================================
//------------------------------------------------------------------
/**
 * @brief   開始時のメッセージ
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static int Record_MainInit( GURU2RC_WORK *wk, int seq )
{
  GFL_NET_SetAutoErrorCheck( FALSE ); // 子機の出入りが確定するまではエラー扱いしない
  // 親の時はAボタンで開始メッセージ。子機は開始待ちメッセージ
  if(GFL_NET_SystemGetCurrentID()==0){

/* レコードコーナーで3人以上の時、０．５秒ぐらいのタイミングで子機が抜けて親を一人にすると
 「つごうがつかないメンバーが…」というメッセージが２重に表示されてウインドウ内で  
  表示が壊れてしまうバグを対処 */
#if AFTER_MASTER_070424_RECORDCONER_FIX
    // 接続人数が2人より多い場合は「Aボタンでかいし」を表示
    if(Union_App_GetMemberNum(_get_unionwork(wk))>=2){
      RecordMessagePrint(wk, msg_guru2_receipt_01_01, 0 );
    }
#else
    RecordMessagePrint(wk, msg_guru2_receipt_01_01, 0 );

#endif
  }else{
    RecordMessagePrint(wk, msg_guru2_receipt_01_07, 0 );
  }
  
  SequenceChange_MesWait(wk, RECORD_MODE);
  
//  wk->seq = RECORD_MODE;

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   レコードコーナー通常処理
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_MainNormal( GURU2RC_WORK *wk, int seq )
{
  // 全体操作禁止でなければキー入力
  PadControl( wk );

  //TrainerObjFunc(wk);
  
  if(GFL_NET_SystemGetCurrentID()==0){        // 親機の時
    if(OnlyParentCheck(wk)!=1){     // 一人じゃないか？
      RecordDataSendRecv( wk );
    }
  }else{
      RecordDataSendRecv( wk );
  }

  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   キー入力管理
 *
 * @param   wk    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static void PadControl( GURU2RC_WORK *wk )
{
  // レコード交換を開始しますか？（親のみ）
  if(_get_key_trg() & PAD_BUTTON_A){
    if(GFL_NET_SystemGetCurrentID()==0){
      if(MyStatusGetNum(wk)==wk->g2c->shareNum && wk->g2c->ridatu_bit == 0){
        u8 flag = GURU2COMM_BAN_ON;
        RecordMessagePrint( wk, msg_guru2_receipt_01_02, 0 );
        SequenceChange_MesWait(wk,RECORD_MODE_START_SELECT);

        // 離脱禁止通達
        Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );
        
        // 接続人数制限ON
        //ChangeConnectMax( wk, 0 );
      }
      else{
        PMSND_PlaySE(SEQ_SE_SELECT1);
      }
    }

  }else if(_get_key_trg()&PAD_BUTTON_B){
    // やめますか？
    if(GFL_NET_SystemGetCurrentID()){
      // 子機は親機から禁止が来ている場合は終了できない
      if(wk->g2c->banFlag==0){
        RecordMessagePrint( wk, msg_guru2_receipt_01_03, 0 );
        SequenceChange_MesWait(wk,RECORD_MODE_END_SELECT);
      }
      else{
        PMSND_PlaySE(SEQ_SE_SELECT1);
      }
    }else{
//      if( MyStatusGetNum(wk)==wk->g2c->shareNum ){
      if( Union_App_GetMemberNum(_get_unionwork(wk))==wk->g2c->shareNum && wk->g2c->ridatu_bit == 0){
        u8 flag = GURU2COMM_BAN_ON;
        // 親機は終了メニューへ
        RecordMessagePrint( wk, msg_guru2_receipt_01_03, 0 );
        SequenceChange_MesWait(wk,RECORD_MODE_END_SELECT);
        // 離脱禁止通達
        Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );

        // 接続人数制限ON
        //ChangeConnectMax( wk, 0 );
      }
      else{
        PMSND_PlaySE(SEQ_SE_SELECT1);
      }
    }
  }
  else{
    if(wk->beacon_flag == GURU2COMM_BAN_NONE){
      if(GFL_NET_SystemGetCurrentID() == 0 && Union_App_GetMemberNum(_get_unionwork(wk))==wk->g2c->shareNum){
        u8 flag = GURU2COMM_BAN_NONE;
        // 離脱禁止解除通達
        Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );
      }
    }
  }

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
static void EndSequenceCommonFunc( GURU2RC_WORK *wk )
{
  
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
static int Record_NewMember( GURU2RC_WORK *wk, int seq )
{
  // ●●●さんがはいってきました
//  RecordMessagePrint(wk, msg_oekaki_01);
//  wk->seq = RECORD_MODE_NEWMEMBER_WAIT;
  SequenceChange_MesWait(wk, RECORD_MODE_NEWMEMBER_END );

  // 画像転送状態になったら輝度ダウン
//  G2_SetBlendBrightness(  GX_BLEND_PLANEMASK_BG1|
//              GX_BLEND_PLANEMASK_BG2|
//              GX_BLEND_PLANEMASK_BG3|
//              GX_BLEND_PLANEMASK_OBJ
//                ,  -6);

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;

}

//------------------------------------------------------------------
/**
 * @brief   メッセージ終了を待って次のシーケンスに行く関数
 *
 * @param   wk    
 * @param   next    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void SequenceChange_MesWait( GURU2RC_WORK *wk, int next )
{
  wk->seq     = RECORD_MODE_MESSAGE_WAIT;
  wk->nextseq = next;
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
static int Record_MessageWaitSeq( GURU2RC_WORK *wk, int seq )
{
  if( EndMessageWait( wk->printStream ) ){
    wk->seq = wk->nextseq;
  }
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  
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
static int Record_NewMemberEnd( GURU2RC_WORK *wk, int seq )
{
  // 輝度ダウン解除
//  G2_BlendNone();

  if(GFL_NET_SystemGetCurrentID()==0){
    int flag = GURU2COMM_BAN_NONE;
    // 離脱禁止解除通達
    Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );
  }

  wk->seq = RECORD_MODE;
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
  
}


// はい・いいえ
#define BMP_YESNO_PX  ( 23 )
#define BMP_YESNO_PY  ( 13 )
#define BMP_YESNO_SX  ( 7 )
#define BMP_YESNO_SY  ( 4 )
#define BMP_YESNO_PAL ( 13 )
#define BMP_YESNO_CGX ( RECORD_YESNO_WIN_OFFSET )


// はい・いいえ
static const BMPWIN_YESNO_DAT YesNoBmpWin = {
  GFL_BG_FRAME0_M, 
  BMP_YESNO_PX, BMP_YESNO_PY,
  BMP_YESNO_PAL, BMP_YESNO_CGX
};

//------------------------------------------------------------------
/**
 * @brief   レコードコーナー「やめる」を選択した時
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_EndSelectPutString( GURU2RC_WORK *wk, int seq )
{
  // はい・いいえ表示
  wk->YesNoMenuWork = BmpMenu_YesNoSelectInit( &YesNoBmpWin, YESNO_WIN_FRAME_CHAR,
                                               MENUFRAME_PAL_INDEX, 0, HEAPID_GURU2 );


  wk->seq = RECORD_MODE_END_SELECT_WAIT;
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
static int Record_EndSelectWait( GURU2RC_WORK *wk, int seq )
{
  int result;
  u32 ret;
  
  // 子機は親機から操作禁止がきている場合は操作できない
  if(GFL_NET_SystemGetCurrentID()){
    if(wk->g2c->banFlag){
      if(_get_key_trg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL | PAD_KEY_UP | PAD_KEY_DOWN)){
        PMSND_PlaySE(SEQ_SE_SELECT1);
      }
      EndSequenceCommonFunc( wk );    //終了選択時の共通処理
      return seq;
    }
  }
  else{ //親の場合
    if(wk->g2c->ridatu_bit != 0){     //離脱しようとしている子がいるなら許可しない
      if(_get_key_trg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL | PAD_KEY_UP | PAD_KEY_DOWN)){
        PMSND_PlaySE(SEQ_SE_SELECT1);
      }
      EndSequenceCommonFunc( wk );    //終了選択時の共通処理
      return seq;
    }
  }

//  if(wk->g2c->shareNum != MyStatusGetNum(wk)){  //一致していないなら「やめる」許可しない
  if(MyStatusGetNum(wk) != Union_App_GetMemberNum(_get_unionwork(wk))){
    //一致していないなら「やめる」許可しない(子も通るここは親しか更新されないshareNumは見ない)
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }

  // メニュー入力
  ret = BmpMenu_YesNoSelectMain( wk->YesNoMenuWork );

  if(ret!=BMPMENU_NULL){
    if(ret==BMPMENU_CANCEL){
      if(GFL_NET_SystemGetCurrentID()==0){
        int flag = GURU2COMM_BAN_NONE;
        // 離脱禁止解除通達
        Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );

        // 接続人数制限OFF
        //ChangeConnectMax( wk, 1 );
      }
      
      SequenceChange_MesWait( wk, RECORD_MODE_INIT );
    }else{
      if(GFL_NET_SystemGetCurrentID()==0){    
        SequenceChange_MesWait( wk, RECORD_MODE_END_SELECT_PARENT );
        RecordMessagePrint( wk, msg_guru2_receipt_01_13, 0 );   // リーダーがやめると…
      }else{
        GURU2COMM_END_CHILD_WORK crec;
        
        MI_CpuClear8(&crec, sizeof(GURU2COMM_END_CHILD_WORK));
        crec.request = CREC_REQ_RIDATU_CHECK;
        crec.ridatu_id = GFL_NET_SystemGetCurrentID();
        
        wk->status_end = TRUE;
        wk->ridatu_wait = 0;
        //wk->seq = RECORD_MODE_END_CHILD;
        wk->seq = RECORD_MODE_END_SELECT_ANSWER_WAIT;
        Guru2Comm_SendData(wk->g2c, G2COMM_RC_END_CHILD, &crec, sizeof(GURU2COMM_END_CHILD_WORK) );
      }
    }
    wk->YesNoMenuWork = NULL;
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
static int Record_EndSelectAnswerWait( GURU2RC_WORK *wk, int seq )
{
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
static int Record_EndSelectAnswerOK( GURU2RC_WORK *wk, int seq )
{
  if((wk->g2c->oya_share_num != Union_App_GetMemberNum(_get_unionwork(wk)))
      || (wk->g2c->oya_share_num != MyStatusGetNum(wk))){
    OS_TPrintf("share_nuM = %d, Comm = %d, My = %d, Bit = %d\n", wk->g2c->oya_share_num, Union_App_GetMemberNum(_get_unionwork(wk)), MyStatusGetNum(wk), Union_App_GetMemberNetBit(_get_unionwork(wk)));
    wk->ridatu_wait = 0;
    wk->seq = RECORD_MODE_END_SELECT_ANSWER_NG;
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }
  
  wk->ridatu_wait++;
  OS_TPrintf("ridatu_wait = %d\n", wk->ridatu_wait);
  if(wk->ridatu_wait > 30){
    GURU2COMM_END_CHILD_WORK crec;
    
    MI_CpuClear8(&crec, sizeof(GURU2COMM_END_CHILD_WORK));
    crec.request = CREC_REQ_RIDATU_EXE;
    crec.ridatu_id = GFL_NET_SystemGetCurrentID();

    Guru2Comm_SendData(wk->g2c, G2COMM_RC_END_CHILD, &crec, sizeof(GURU2COMM_END_CHILD_WORK) );

    wk->ridatu_wait = 0;
    wk->seq = RECORD_MODE_END_CHILD;
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
static int Record_EndSelectAnswerNG( GURU2RC_WORK *wk, int seq )
{
  wk->status_end = FALSE;
  SequenceChange_MesWait( wk, RECORD_MODE_INIT );
  
  OS_TPrintf("==========離脱強制キャンセル！===========\n");
  
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   このメンバーでレコードを開始しますか？
 *
 * @param   wk    
 * @param   seq   
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_StartSelect( GURU2RC_WORK *wk, int seq )
{
  // はい・いいえ表示
//  wk->YesNoMenuWork = BmpYesNoSelectInit( &YesNoBmpWin, YESNO_WIN_FRAME_CHAR, MENUFRAME_PAL_INDEX, HEAPID_GURU2 );
  wk->YesNoMenuWork = BmpMenu_YesNoSelectInit( &YesNoBmpWin, YESNO_WIN_FRAME_CHAR,
                                               MENUFRAME_PAL_INDEX, 0, HEAPID_GURU2 );


  wk->seq = RECORD_MODE_START_SELECT_WAIT;
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   「開始しますか？」はい・いいえ選択待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_StartSelectWait( GURU2RC_WORK *wk, int seq )
{
  int result;
  u32 ret;

  if(MyStatusGetNum(wk) != wk->g2c->shareNum    //一致していないなら許可しない
      || wk->g2c->ridatu_bit != 0){     //離脱しようとしている子がいるなら許可しない
    if(_get_key_trg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL | PAD_KEY_UP | PAD_KEY_DOWN)){
      PMSND_PlaySE(SEQ_SE_SELECT1);
    }
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }
    
  ret = BmpMenu_YesNoSelectMain( wk->YesNoMenuWork );


  if(ret!=BMPMENU_NULL){
    if(ret==BMPMENU_CANCEL){
      int flag = GURU2COMM_BAN_NONE;
      // 離脱禁止解除通達
      Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );

      // ビーコン状態変更
      ////ChangeConnectMax( wk, 1 );
      SequenceChange_MesWait( wk, RECORD_MODE_INIT );
    }else{
      // 絶対ここにくるのは親だけど
      if(GFL_NET_SystemGetCurrentID()==0){
        /*
        SequenceChange_MesWait(
          wk, RECORD_MODE_STAET_RECORD_COMMAND );
        //まぜています
        RecordMessagePrint( wk, msg_guru2_receipt_01_11, 1 );
        */
        
                // 接続禁止に書き換え
        wk->seq = RECORD_MODE_START_RECORD_COMMAND;
        wk->start_num = MyStatusGetNum(wk);
//        Union_BeaconChange( UNION_PARENT_MODE_GURU2 );
      }else{
        GF_ASSERT(0);
      }
    }
    wk->YesNoMenuWork = NULL;
  }


  EndSequenceCommonFunc( wk );    //終了選択時の共通処理

  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_StartRecordCommand( GURU2RC_WORK *wk, int seq )
{
  if(wk->record_send == 0){
    if(Guru2Comm_SendData(wk->g2c, G2COMM_RC_START, NULL, 0 ) == TRUE){
//      wk->record_send = TRUE;
      wk->seq = RECORD_MODE_GURU2_POKESEL_START;
    }
  }
  
//  SequenceChange_MesWait( wk, RECORD_MODE_RECORD_SEND_DATA );
//  RecordMessagePrint( wk, msg_guru2_receipt_01_11, 1 );   // まぜています

  return seq;

}


//------------------------------------------------------------------
/**
 * @brief   データ送信・受信待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_RecordSendData( GURU2RC_WORK *wk, int seq )
{
  int i,result;

  result = 0;
//  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
//    result += CommIsSpritDataRecv(i);
//  }

  if( GFL_NET_SystemGetCurrentID() == 0 && Union_App_GetMemberNum(_get_unionwork(wk)) != wk->start_num){
    //開始した時の人数と現在の人数が変わっているなら通信エラーにする
//    OS_TPrintf("人数が変わっているので強制的に通信エラーにします　開始時＝%d, 現在＝%d\n", wk->start_num, Union_App_GetMemberNum(_get_unionwork(wk)));
//    CommStateSetError();
  }
  
  // 受信が終了した人数と接続人数が同じになったら終了
  if( Union_App_GetMemberNum(_get_unionwork(wk))==wk->g2c->recv_count ){

    // データ混ぜる処理へ
    wk->seq = RECORD_MODE_GURU2_POKESEL_START;
    
#if 0
    for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
      OS_TPrintf("受信データ id=%d, seed=%d, xor=%08x\n",i,wk->recv_data[i].seed, wk->recv_data[i]._xor);
    }
#endif
  }

  return seq;
}




// RECORD_MODE_END_CHILD
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
static int  Record_EndChild( GURU2RC_WORK *wk, int seq )
{
  u8 temp;
  
  // レコードコーナーをぬけました
  RecordMessagePrint( wk, msg_guru2_receipt_01_04, 0 ); 

  // 終了通達
  temp = 0;
//  Guru2Comm_SendData(wk->g2c, G2COMM_RC_END_CHILD, &temp, 1 );

  SequenceChange_MesWait( wk, RECORD_MODE_END_CHILD_WAIT );

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;
}
        

// RECORD_MODE_END_CHILD_WAIT
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
static int  Record_EndChildWait( GURU2RC_WORK *wk, int seq )
{     
  if( ++wk->wait > RECORD_CORNER_MESSAGE_END_WAIT ){
    // ワイプフェード開始
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, WIPE_FADE_BLACK, 16, 1, HEAPID_GURU2 );
    seq = SEQ_OUT;            //終了シーケンスへ
  }

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
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
static int Record_EndSelectParent( GURU2RC_WORK *wk, int seq )
{

  // はい・いいえ表示
//  wk->YesNoMenuWork = BmpYesNoSelectInit( &YesNoBmpWin, YESNO_WIN_FRAME_CHAR, MENUFRAME_PAL_INDEX, HEAPID_GURU2 );
  wk->YesNoMenuWork = BmpMenu_YesNoSelectInit( &YesNoBmpWin, YESNO_WIN_FRAME_CHAR,
                                               MENUFRAME_PAL_INDEX, 0, HEAPID_GURU2 );

  wk->seq = RECORD_MODE_END_SELECT_PARENT_WAIT;

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
static int Record_EndSelectParentWait( GURU2RC_WORK *wk, int seq )
{
  int ret;

  if(wk->g2c->shareNum != MyStatusGetNum(wk)      //一致していないなら「やめる」許可しない
      || wk->g2c->ridatu_bit != 0){     //離脱しようとしている子がいるなら許可しない
    if(_get_key_trg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL | PAD_KEY_UP | PAD_KEY_DOWN)){
      PMSND_PlaySE( SEQ_SE_DECIDE1 );
    }
    EndSequenceCommonFunc( wk );    //終了選択時の共通処理
    return seq;
  }

  ret = BmpMenu_YesNoSelectMain( wk->YesNoMenuWork );

  if(ret!=BMPMENU_NULL){
    if(ret==BMPMENU_CANCEL){
      int flag = GURU2COMM_BAN_NONE;
      
      wk->seq = RECORD_MODE_INIT;

      // 離脱禁止解除通達
      Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );

      // 接続人数制限OFF
      //ChangeConnectMax( wk, 1 );

    }else{
      wk->seq = RECORD_MODE_FORCE_END;
      Guru2Comm_SendData(wk->g2c, G2COMM_RC_END, NULL, 0 ); //終了通知
      WORDSET_RegisterPlayerName(   // 親機（自分）の名前をWORDSET
        wk->WordSet, 0, Union_App_GetMystatus(_get_unionwork(wk),0) );
    }
    wk->YesNoMenuWork = NULL;
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
static int Record_ForceEnd( GURU2RC_WORK *wk, int seq )
{
  if(GFL_NET_SystemGetCurrentID()==0){
    // それでは　レコード通信をちゅうし　します
    RecordMessagePrint( wk, msg_guru2_receipt_01_04, 0 );
  }else{
    // つごうがわるくなったメンバーがいるのでかいさんします
    WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(_get_unionwork(wk),0) ); // 親機（自分）の名前をWORDSET
    RecordMessagePrint( wk, msg_guru2_receipt_01_15, 0 );
  }
  SequenceChange_MesWait(wk,RECORD_MODE_FORCE_END_MES_WAIT);
  wk->wait = 0;

  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
  return seq;

}


//------------------------------------------------------------------
/**
 * @brief   強制終了メッセージ１秒待ち
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int Record_FroceEndMesWait( GURU2RC_WORK *wk, int seq )
{
  wk->wait++;
  if(wk->wait > RECORD_1SEC_WAIT){
    wk->seq = RECORD_MODE_FORCE_END_WAIT;
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
static int Record_ForceEndWait( GURU2RC_WORK *wk, int seq )
{
  // 通信同期待ち
//  CommTimingSyncStart(COMM_GURU2_TIMINGSYNC_NO);
  GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();
  GFL_NET_TimingSyncStart(pNet, COMM_GURU2_TIMINGSYNC_NO);

  wk->seq = RECORD_MODE_FORCE_END_SYNCHRONIZE;

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
static int Record_ForceEndSynchronize( GURU2RC_WORK *wk, int seq )
{
  GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(GFL_NET_IsTimingSync(pNet, COMM_GURU2_TIMINGSYNC_NO)){
    GFL_NET_SetAutoErrorCheck(FALSE);
    OS_Printf("終了時同期成功\n");
    // ワイプフェード開始
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, WIPE_FADE_BLACK, 16, 1, HEAPID_GURU2);

    seq = SEQ_OUT;            //終了シーケンスへ
  }
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
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
static int Record_EndParentOnly( GURU2RC_WORK *wk, int seq )
{
/* レコードコーナーで3人以上の時、０．５秒ぐらいのタイミングで子機が抜けて親を一人にすると
 「つごうがつかないメンバーが…」というメッセージが２重に表示されてウインドウ内で  
  表示が壊れてしまうバグを対処 */
#if AFTER_MASTER_070424_RECORDCONER_FIX
  // メッセージ表示中は呼び出さないようにする
  if( EndMessageWait( wk->printStream ) ){
    RecordMessagePrint( wk, msg_guru2_receipt_01_08, 0 ); // リーダーが抜けたので解散します。
  }
#else
    RecordMessagePrint( wk, msg_guru2_receipt_01_08, 0 ); // リーダーが抜けたので解散します。
#endif
  wk->seq = RECORD_MODE_END_PARENT_ONLY_WAIT;

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
static int Record_EndParentOnlyWait( GURU2RC_WORK *wk, int seq )
{
  if( EndMessageWait( wk->printStream ) ){
    wk->seq = RECORD_MODE_END_CHILD_WAIT;
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
static int Record_LogoutChildMes( GURU2RC_WORK *wk, int seq )
{
  // ●●●さんがかえりました
  if( EndMessageWait( wk->printStream ) == 0){
    //表示中のメッセージがある場合は強制停止
//    GF_STR_PrintForceStop(wk->printStream);
    PRINTSYS_PrintStreamStop( wk->printStream );
  }
  RecordMessagePrint(wk, msg_guru2_receipt_01_14, 1 );
  wk->seq = RECORD_MODE_LOGOUT_CHILD_WAIT;

  // 接続可能人数を一旦現在の接続人数に落とす
  if(GFL_NET_SystemGetCurrentID()==0){
    //ChangeConnectMax( wk, 0 );
  }


  OS_TPrintf("人数チェック＝logout child %d\n", Union_App_GetMemberNum(_get_unionwork(wk)));
  wk->err_num = Union_App_GetMemberNum(_get_unionwork(wk));
    wk->err_num_timeout = ERRNUM_TIMEOUT;
  
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
static int Record_LogoutChildMesWait( GURU2RC_WORK *wk, int seq )
{
  // 接続人数が１減るかチェック
  if(wk->err_num != 0 && Union_App_GetMemberNum(_get_unionwork(wk)) != wk->err_num){
    wk->err_num = 0;
  }

  wk->seq  = RECORD_MODE_LOGOUT_CHILD_CLOSE;
  wk->wait = 0;

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
static int  Record_LogoutChildClose( GURU2RC_WORK *wk, int seq )
{     

  // 接続人数が１減るまでは待つ
  if(wk->err_num != 0 && Union_App_GetMemberNum(_get_unionwork(wk)) != wk->err_num){
    wk->err_num = 0;
  }
    wk->err_num_timeout--;
    if(wk->err_num_timeout<0){
        wk->err_num = 0;
    }
  
  if( ++wk->wait > RECORD_CORNER_MESSAGE_END_WAIT && wk->err_num == 0){
    wk->seq = RECORD_MODE_INIT;//RECORD_MODE;       //終了シーケンスへ

    // 離脱者がいなくなったので、＋１状態で一人は入れるようにする
    if(GFL_NET_SystemGetCurrentID()==0){
      //ChangeConnectMax( wk, 1 );
    }
  }

  EndSequenceCommonFunc( wk );      //終了選択時の共通処理
  return seq;
  
}

#if 0
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
static int Record_LogoutChildMes( GURU2RC_WORK *wk, int seq )
{
  // ●●●さんがかえりました
  wk->seq = RECORD_MODE_LOGOUT_CHILD_WAIT;

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
static int Record_LogoutChildMesWait( GURU2RC_WORK *wk, int seq )
{
    wk->seq  = RECORD_MODE_LOGOUT_CHILD_CLOSE;
    wk->wait = 0;

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
static int  Record_LogoutChildClose( GURU2RC_WORK *wk, int seq )
{     

  if( ++wk->wait > RECORD_CORNER_MESSAGE_END_WAIT ){
    wk->seq = RECORD_MODE;        //終了シーケンスへ
  }

  EndSequenceCommonFunc( wk );      //終了選択時の共通処理
  return seq;
  
}
#endif

//==============================================================================
//  ぐるぐる交換用
//==============================================================================
//------------------------------------------------------------------
/**
 * @brief   ポケモンセレクトへ
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int       
 */
//------------------------------------------------------------------
static int  Record_Guru2PokeSelStart( GURU2RC_WORK *wk, int seq )
{
  OS_Printf( "ぐるぐる　ポケモンセレクトへ\n" );
  
//  Union_BeaconChange( UNION_PARENT_MODE_GURU2 );
  
  // ワイプフェード開始
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT,
    WIPE_FADE_BLACK, 16, 1, HEAPID_GURU2);
  
  wk->end_next_flag = TRUE;
  seq = SEQ_OUT;
  EndSequenceCommonFunc( wk );    //終了選択時の共通処理
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
void Guru2Rc_MainSeqForceChange( GURU2RC_WORK *wk, int seq, u8 id  )
{
  switch(seq){
  case RECORD_MODE_NEWMEMBER_END:
    EndMessageWindowOff( wk );
    break;
  case RECORD_MODE_FORCE_END:
    if(wk->YesNoMenuWork!=NULL){
      BmpMenu_YesNoMenuExit( wk->YesNoMenuWork );
      wk->YesNoMenuWork = NULL;
    }
    break;
  case RECORD_MODE_RECORD_SEND_DATA:
    if(EndMessageWait(wk->printStream) == 0){
      //表示中のメッセージがある場合は強制停止
//      GF_STR_PrintForceStop(wk->printStream);
    PRINTSYS_PrintStreamStop( wk->printStream );
    }
    RecordMessagePrint( wk, msg_guru2_receipt_01_11, 0 );   // 「まぜています」表示
    if(wk->YesNoMenuWork!=NULL){
      BmpMenu_YesNoMenuExit( wk->YesNoMenuWork );
      wk->YesNoMenuWork = NULL;
    }
    break;
  case RECORD_MODE_LOGOUT_CHILD:
    if(wk->status_end == TRUE){
      return; //自分自身が離脱処理中
    }
    WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(_get_unionwork(wk),id) );  
    if(id==GFL_NET_SystemGetCurrentID()){
      // 自分が離脱する子機だった場合は「子機がいなくなたよ」とは言わない
      return;
    }
    if(wk->YesNoMenuWork!=NULL){
      BmpMenu_YesNoMenuExit( wk->YesNoMenuWork );
      wk->YesNoMenuWork = NULL;
    }
    if(GFL_NET_SystemGetCurrentID() == 0){
      wk->g2c->ridatu_bit &= 0xffff ^ id;
    }
    break;
  case RECORD_MODE_END_SELECT_ANSWER_OK:
    break;
  case RECORD_MODE_END_SELECT_ANSWER_NG:
    break;
  case RECORD_MODE_GURU2_POKESEL_START:
    if(EndMessageWait(wk->printStream) == 0){
      //表示中のメッセージがある場合は強制停止
//      GF_STR_PrintForceStop(wk->printStream);
      PRINTSYS_PrintStreamStop( wk->printStream );
  }
    
    if(wk->YesNoMenuWork!=NULL){
      BmpMenu_YesNoMenuExit( wk->YesNoMenuWork );
      wk->YesNoMenuWork = NULL;
    }
    break;
  default:
    GF_ASSERT( "指定以外のシーケンスチェンジが来た" );
    return;
  }
  wk->seq = seq;

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
void Guru2Rc_MainSeqCheckChange( GURU2RC_WORK *wk, int seq, u8 id  )
{
  // 通常状態なら
  if(wk->seq == RECORD_MODE){
    OS_Printf("子機%dの名前を登録\n",id);
    switch(seq){
    case RECORD_MODE_NEWMEMBER: 
      // 指定の子機の名前をWORDSETに登録（離脱・乱入時)
      WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(_get_unionwork(wk),id) );  
      //ChangeConnectMax( wk, 1 );
      wk->seq      = seq;
      wk->g2c->shareNum = Union_App_GetMemberNum(_get_unionwork(wk));
      wk->g2c->ridatu_bit = 0;
      if(GFL_NET_SystemGetCurrentID()==0){
        int flag = GURU2COMM_BAN_ON;//NONE;
        // 離脱禁止解除通達
        Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );
        
      }
      break;
      // ↓↓↓ 
    case RECORD_MODE_LOGOUT_CHILD:
      WORDSET_RegisterPlayerName( wk->WordSet, 0, Union_App_GetMystatus(_get_unionwork(wk),id) );  
      if(id==GFL_NET_SystemGetCurrentID()){
        // 自分が離脱する子機だった場合は「子機がいなくなたよ」とは言わない
        return;
      }
      if(GFL_NET_SystemGetCurrentID() == 0){
        wk->g2c->ridatu_bit &= 0xffff ^ id;
      }
      wk->seq = seq;
      break;
    default:
      GF_ASSERT( 0&&"指定以外のシーケンスチェンジが来た" );
      return;
    }


  }
  else if(seq == RECORD_MODE_NEWMEMBER){
    wk->g2c->ridatu_bit = 0;
    wk->g2c->shareNum = Union_App_GetMemberNum(_get_unionwork(wk));
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



//------------------------------------------------------------------
/**
 * @brief   現在何人接続しているか？
 *
 * @param   none    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int OnlyParentCheck( GURU2RC_WORK *wk )
{
  int i,result;
  const MYSTATUS *status;

  result = 0;
  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    status = Union_App_GetMystatus(_get_unionwork(wk),i);
    if(status!=NULL){
      result++;
    }
  }

  return result;
}

#define NAME_PRINT_HABA ( 18 )
#define ID_STRING_NUM (2+1+5+1)
//------------------------------------------------------------------
/**
 * @brief   オンライン状況を確認して上画面に名前とIDを表示する
 *
 * @param   win   
 * @param   frame   
 * @param   color   
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static BOOL NameCheckPrint( GFL_BMPWIN *win, int frame, PRINTSYS_LSB color, GURU2RC_WORK *wk )
{
  int i,id = GFL_NET_SystemGetCurrentID();
  STRBUF *id_str  = NULL;
  STRBUF *tmp_str = NULL;


  // 名前取得の状況に変化が無い場合は書き換えない
  if(!MyStatusCheck(wk)){
    return FALSE;
  }


  // それぞれの文字パネルの背景色でクリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win),0x0000);

  // 描画
  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    if(wk->TrainerStatus[i][0]!=NULL){
      // トレーナーIDの取得
      u16 tid = MyStatus_GetID_Low( wk->TrainerStatus[i][0] );
      id_str  = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_GURU2 );

      // 自分の名前取得
      MyStatus_CopyNameString( wk->TrainerStatus[i][0], wk->TrainerName[i] );

      // トレーナーIDを埋め込んだ文字列「ID 12345」を取得
      WORDSET_RegisterNumber( wk->WordSet, 0, tid, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU );
      tmp_str = GFL_MSG_CreateString( wk->MsgManager, msg_guru2_receipt_name_02 );
      WORDSET_ExpandStr( wk->WordSet, id_str, tmp_str );
      
      if(id==i){  // 自分の名前の時は描画色を赤色に
        color = PRINTSYS_LSB_Make(2,3,0);
      }
      // 名前とID描画
      PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(win), 5, 1+i*NAME_PRINT_HABA, wk->TrainerName[i], 
                           wk->font, PRINTSYS_LSB_Make(2,3,0) );

      PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(win), 5+13*5, 1+i*NAME_PRINT_HABA, id_str, 
                            wk->font, PRINTSYS_LSB_Make(2,3,0) );
      // ID文字列を解放
      GFL_STR_DeleteBuffer(id_str);
    }
  }
  GFL_BMPWIN_MakeTransWindow( win );

  return TRUE;
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
static int ConnectCheck( GURU2RC_WORK *wk )
{
  int i,result=0;
  const MYSTATUS *status;
  STRCODE  *namecode;
  u32 bit = Union_App_GetMemberNetBit(_get_unionwork(wk));

  // 接続チェック
  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    wk->ConnectCheck[i][0] = wk->ConnectCheck[i][1];
    if(bit&(1<<i)){
      wk->ConnectCheck[i][0] = 1;
    }else{
      wk->ConnectCheck[i][0] = 0;
    }

  }

  // 接続がいたら名前を反映させる
  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    if(wk->ConnectCheck[i][0]){       // 接続しているか？
      // 接続している

      status = Union_App_GetMystatus(_get_unionwork(wk),i);
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
static void RecordDataSendRecv( GURU2RC_WORK *wk )
{
  
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
static int MyStatusGetNum( GURU2RC_WORK *wk )
{
  int i,result;
  for(result=0,i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    if(Union_App_GetMystatus(_get_unionwork(wk),i)!=NULL){
      result++;
    }
  }
  return result;
}

//------------------------------------------------------------------
/**
 * @brief   現在のオンライン数をビットで取得
 *
 * @param   none    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static u32 MyStatusGetNumBit( GURU2RC_WORK *wk )
{
  int i;
  u32 result;
  for(result=0,i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    if(Union_App_GetMystatus(_get_unionwork(wk),i)!=NULL){
      result |= 1 << i;
    }
  }
  return result;
}


//  RECORD_EXIST_NO=0,
//  RECORD_EXIST_APPEAR_REQ,
//  RECORD_EXIST_APPEAR,
//  RECORD_EXIST_BYE_REQ,


//------------------------------------------------------------------
/**
 * @brief   MYSTATUSの取得状況に変化があったか？
 *
 * @param   wk    
 *
 * @retval  int   あったら1,  無い場合は0
 */
//------------------------------------------------------------------
static BOOL MyStatusCheck( GURU2RC_WORK *wk )
{
  int i,result=FALSE;
  
  // 接続がいたら名前を反映させる
  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    wk->TrainerStatus[i][1] = wk->TrainerStatus[i][0];
    wk->TrainerStatus[i][0] = Union_App_GetMystatus(_get_unionwork(wk),i);

    // 080626 tomoya 瞬間的に人が入れ替わったときのことを考え、トレーナidを保存
    wk->trainer_id[i][1]  = wk->trainer_id[i][0];
    if( wk->TrainerStatus[i][0] != NULL ){
      wk->trainer_id[i][0].data.tr_id = MyStatus_GetID( wk->TrainerStatus[i][0] );
      wk->trainer_id[i][0].data.in  = TRUE;

    }else{
      // 全OFF
      wk->trainer_id[i][0].check  = 0;
    }

  }

  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){

    if(wk->TrainerStatus[i][1] != wk->TrainerStatus[i][0]){

      // 変化あり
      result = TRUE;

      // 登場・さよならリクエストの発行
      if(wk->TrainerStatus[i][0]==NULL){
        wk->TrainerReq[i] = RECORD_EXIST_BYE_REQ;
      }else{
        wk->TrainerReq[i] = RECORD_EXIST_APPEAR_REQ;
      }
    }else{

      // 080626 tomoya 瞬間的に人が入れ替わったときのことを考え
      // トレーナIDが変わってないかチェック
      if( wk->trainer_id[i][0].check != wk->trainer_id[i][1].check ){

        // 変化あり
        result = TRUE;

        // 瞬間的にi番目の人が入れ替わった、
        // 瞬間的に入場させる
        if( wk->trainer_id[i][0].data.in == FALSE ){  // おそらくFALSEでここに来ることはない
          wk->TrainerReq[i] = RECORD_EXIST_BYE_REQ;
        }else{
          wk->TrainerReq[i] = RECORD_EXIST_APPEAR_REQ;
        }
      }
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
static void RecordMessagePrint( GURU2RC_WORK *wk, int msgno, int all_put )
{
  // 文字列取得
  STRBUF *tempbuf;
  
  tempbuf = GFL_STR_CreateBuffer(TALK_MESSAGE_BUF_NUM,HEAPID_GURU2);
  GFL_MSG_GetString(  wk->MsgManager, msgno, tempbuf );
  WORDSET_ExpandStr( wk->WordSet, wk->TalkString, tempbuf );
  GFL_STR_DeleteBuffer(tempbuf);

  // 会話ウインドウ枠描画
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin),  0x0f0f );
  BmpWinFrame_Write( wk->MsgWin, WINDOW_TRANS_ON, 1, MESFRAME_PAL_INDEX );

  // 文字列描画開始
  if(all_put == 0){
//    wk->printStream = GF_STR_PrintSimple( wk->MsgWin, FONT_TALK, wk->TalkString, 0, 0, GetTalkSpeed(wk), NULL);
    wk->printStream = PRINTSYS_PrintStream( wk->MsgWin, 0,0, wk->TalkString,
                                            wk->font, MSGSPEED_GetWait(), wk->pMsgTcblSys,
                                            0,HEAPID_GURU2, 15 );
  }
  else{
    //一括表示の場合はMsgIndexが0xffになるので注意！
//    GF_STR_PrintSimple( wk->MsgWin, FONT_TALK, wk->TalkString, 0, 0, MSG_ALLPUT, NULL);
    PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, 0,0, wk->TalkString, 
                           wk->font, PRINTSYS_LSB_Make(2,3,0) );


  }

}

//------------------------------------------------------------------
/**
 * @brief   会話表示ウインドウ終了待ち
 *
 * @param   msg_index   
 *
 * @retval  int   
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
static void EndMessageWindowOff( GURU2RC_WORK *wk )
{
  BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
}


//------------------------------------------------------------------
/**
 * @brief   トレーナーOBJの表示制御
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void TrainerObjFunc( GURU2RC_WORK *wk )
{
  int i,seflag=0;
  
  for(i=0;i<RECORD_CORNER_MEMBER_MAX;i++){
    switch(wk->TrainerReq[i]){
    case RECORD_EXIST_NO:
      break;
    case RECORD_EXIST_APPEAR_REQ:
      {
        int view,sex;
        GF_ASSERT(wk->TrainerStatus[i][0]!=NULL);
        
        sex = MyStatus_GetMySex( wk->TrainerStatus[i][0] );
        view  = MyStatus_GetTrainerView( wk->TrainerStatus[i][0] );
        if(GFL_NET_SystemGetCurrentID()==i){
          // 主人公OBJ用のアニメ
          GFL_CLACT_WK_SetAnmSeq( wk->MainActWork[i+1], 38+sex*2 );
        }else{
          // フィールドOBJ用のアニメ
          TransFieldObjData( wk->FieldObjCharaData, wk->FieldObjPalData,  i, view, sex );
          GFL_CLACT_WK_SetAnmSeq( wk->MainActWork[i+1], 27+i*2 );
        }

      }
      GFL_CLACT_WK_SetDrawEnable(wk->MainActWork[i+1], 1);
      wk->TrainerReq[i] = RECORD_EXIST_APPEAR;
      seflag = 1;
      break;
    case RECORD_EXIST_APPEAR:
      break;
    case RECORD_EXIST_BYE_REQ:
      if(GFL_NET_SystemGetCurrentID()==i){
        int sex = MyStatus_GetMySex( wk->TrainerStatus[i][0] );
        GFL_CLACT_WK_SetAnmSeq( wk->MainActWork[i+1], 38+sex*2+1 );
      }else{
        GFL_CLACT_WK_SetAnmSeq( wk->MainActWork[i+1], 27+i*2+1 );
      }
      wk->TrainerReq[i] = RECORD_EXIST_NO;
      break;
    }
  }
  
  // 登場SEが必要な場合は鳴らす
  if(seflag){
    PMSND_PlaySE( SEQ_SE_SELECT2 );  // SE_GTC_APPEAR
  }
}


//------------------------------------------------------------------
/**
 * @brief   フィールドOBJ画像読み込み（ユニオンと自機）
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void LoadFieldObjData( GURU2RC_WORK *wk, ARCHANDLE* p_handle )
{
  // パレット読み込み
  wk->FieldObjPalBuf[0] = GFL_ARC_UTIL_LoadPalette(    ARCID_WORLDTRADE_GRA, NARC_worldtrade_hero_nclr, &(wk->FieldObjPalData[0]), HEAPID_GURU2 );
  wk->FieldObjPalBuf[1] = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_guru2_2d_union_chara_nclr, &(wk->FieldObjPalData[1]), HEAPID_GURU2 );

  // 画像読み込み
  wk->FieldObjCharaBuf[0] = GFL_ARC_UTIL_LoadOBJCharacter( ARCID_WORLDTRADE_GRA, NARC_worldtrade_hero_lz_ncgr, 1, &(wk->FieldObjCharaData[0]), HEAPID_GURU2 );
  wk->FieldObjCharaBuf[1] = GFL_ARCHDL_UTIL_LoadOBJCharacter( p_handle, NARC_guru2_2d_union_chara_lz_ncgr,  1, &(wk->FieldObjCharaData[1]), HEAPID_GURU2 );

}

#define OBJ_TRANS_SIZE  ( 3 * 4*4 )
static const u16 obj_offset[]={
  ( 13                    )*0x20,
  ( 13 + OBJ_TRANS_SIZE*1 )*0x20,
  ( 13 + OBJ_TRANS_SIZE*2 )*0x20,
  ( 13 + OBJ_TRANS_SIZE*3 )*0x20,
  ( 13 + OBJ_TRANS_SIZE*4 )*0x20,
};


static int _obj_no = 0;
static int _pal_no = 0;

//------------------------------------------------------------------
/**
 * @brief   渡されたユニオン見た目コードからフィールドOBJのキャラを転送する
 *
 * @param   id    
 * @param   view    
 * @param   sex   
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void TransFieldObjData( NNSG2dCharacterData *CharaData[2], NNSG2dPaletteData *PalData[2], int id, int view, int sex )
{
  int pos;
  u8 *chara, *pal;
  
  // ユニオンキャラを転送

  pos   = UnionView_GetCharaNo( sex, view );

  chara = (u8*)CharaData[1]->pRawData;
  pal   = (u8*)PalData[1]->pRawData;

  GX_LoadOBJ( &chara[OBJ_TRANS_SIZE*pos*0x20], obj_offset[id], OBJ_TRANS_SIZE*0x20 );
  GX_LoadOBJPltt( &pal[pos*32], (id+FIELDOBJ_PAL_START)*32, 32 );

  OS_Printf("ID=%d のユニオン見た目は %d アイコン番号は %d\n", id, view, pos);

  
  
}

//------------------------------------------------------------------
/**
 * @brief   フィールドOBJ画像解放
 *
 * @param   wk    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void FreeFieldObjData( GURU2RC_WORK *wk )
{

  GFL_HEAP_FreeMemory( wk->FieldObjPalBuf[0]  );
  GFL_HEAP_FreeMemory( wk->FieldObjPalBuf[1]  );
                      
  GFL_HEAP_FreeMemory( wk->FieldObjCharaBuf[0] );
  GFL_HEAP_FreeMemory( wk->FieldObjCharaBuf[1] );
}

//------------------------------------------------------------------
/**
 * @brief   会話スピードを取得
 *
 * @param   wk    
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int GetTalkSpeed( GURU2RC_WORK *wk )
{
  return 1;
//  return CONFIG_GetMsgPrintSpeed(wk->g2p->param.config);
}


//------------------------------------------------------------------
/**
 * @brief   募集人数とビーコン情報を設定する
 *
 * @param   wk    
 * @param   plus  1なら募集したい時、0なら締め切りたいとき
 *
 * @retval  none
 */
//------------------------------------------------------------------
static int RecordCorner_BeaconControl( GURU2RC_WORK *wk, int plus )
{
  int num;
  // ビーコンを書き換える
  num = MyStatusGetNum(wk);

  if(num>wk->g2c->shareNum){
    u8 flag = GURU2COMM_BAN_ON;
    // 離脱禁止解除通達
    Guru2Comm_SendData(wk->g2c, G2COMM_RC_BAN, &flag, 1 );
    wk->beacon_flag = GURU2COMM_BAN_ON;
  }
  else{
    wk->beacon_flag = GURU2COMM_BAN_NONE;
  }

  // 接続人数とレコード交換可能人数が一致するまでは操作禁止
  if(num==wk->connectBackup){
    return SEQ_MAIN;
  }

  OS_TPrintf("人数が変わった\n");

  // 上限じゃない、かつ締め切りでもないなら募集続行
  switch(num){
  case 1:
    // ひとりになったので終了
    OS_TPrintf("ONLY!! Comm = %d, My = %d, Bit = %d\n", Union_App_GetMemberNum(_get_unionwork(wk)), MyStatusGetNum(wk), Union_App_GetMemberNetBit(_get_unionwork(wk)));
    if(  Union_App_GetMemberNum(_get_unionwork(wk)) > 1 
      || Union_App_GetMemberNetBit(_get_unionwork(wk)) > 1){
      return SEQ_MAIN;  //完全に一人にはなっていないのでまだ何もしない
    }
    wk->seq = RECORD_MODE_END_PARENT_ONLY;
    //ChangeConnectMax( wk, -1 );
    if(wk->YesNoMenuWork!=NULL){
      BmpMenu_YesNoMenuExit( wk->YesNoMenuWork );
      wk->YesNoMenuWork = NULL;
    }
    return SEQ_LEAVE;
    break;
  case 2: case 3:case 4:
    // まだ入れるよ
//    Union_BeaconChange( UNION_PARENT_MODE_GURU2_FREE );

    // 接続人数が減った場合は接続最大人数も減らす
    if(num<wk->connectBackup){
      switch(wk->limit_mode){
      case LIMIT_MODE_NONE:
        //ChangeConnectMax( wk, plus );
        break;
      case LIMIT_MODE_TRUE:
        //ChangeConnectMax(wk, 0);
        break;
      }
      wk->g2c->shareNum = Union_App_GetMemberNum(_get_unionwork(wk));
    }
    break;
  case 5:
    // いっぱいです
//    Union_BeaconChange( UNION_PARENT_MODE_GURU2 );
    //ChangeConnectMax( wk, plus );
    break;
  }

  
  // 接続人数を保存
  wk->connectBackup = MyStatusGetNum(wk);

  return SEQ_MAIN;
}



//----------------------------------------------------------------------------------
/**
 * @brief タッチもAボタンとみなすキー取得ルーチン
 *
 * @param   none    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_key_trg( void )
{
  // キー情報取得
  int key = GFL_UI_KEY_GetTrg();

  // タッチTRGでもAボタン情報を追加
  if( GFL_UI_TP_GetTrg() ){
    key |= PAD_BUTTON_A;
  }

  return key;
}