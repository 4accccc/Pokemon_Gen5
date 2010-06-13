//============================================================================================
/**
 * @file    trainer_card.c
 * @brief   トレーナーカード
 * @author  Saito Nozomu
 * @date    05.11.15
 */
//============================================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"
#include "savedata/trainercard_data.h"
#include "sound/pm_sndsys.h"
#include "app/app_menu_common.h"
#include "arc_def.h"
#include "font/font.naix"

#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "message.naix"
#include "trainer_case.naix"
#include "msg/msg_trainercard.h"

#include "trcard_sys.h"

#include "trcard_snd_def.h"
#include "trcard_bmp.h"
#include "trcard_obj.h"
#include "trcard_cgx_def.h"
#include "trainer_case.naix"
#include "trc_union.naix"

#include "savedata/config.h"
#include "msg/msg_trainercard.h"
#include "message.naix"
#include "system/wipe.h"
#include "system/pms_draw.h"
#include "test/ariizumi/ari_debug.h"


#define MIN_SCRUCH  (3)
#define MAX_SCRUCH  (40)
#define REV_SPEED (FX32_SHIFT - wk->RevSpeed)

#define CGX_WIDE    (32)
#define BUTTON_CGX_POS  (4*CGX_WIDE)
#define MAIN_BUTTON_CGX_SIZE  (4)
#define MAIN_BUTTON_POS_Y (19)
#define MAIN_BUTTON_POS_X (14)

#define NOBODY_CGX_POS  (5*CGX_WIDE+26)
#define GYM_READER_CGX_SIZE (5)
#define COVER_CGX_WIDTH (16)

#define BADGE_BAD_PLTT_NO (3)
#define UNI_TRAINER_PLTT_NO (4)

#define TRC_BG_TRAINER  (GFL_BG_FRAME3_S)
#define TRC_BG_FONT     (GFL_BG_FRAME3_S)
#define TRC_BG_SIGN     (GFL_BG_FRAME3_S)
#define TRC_BG_MSG      (GFL_BG_FRAME0_S)
#define TRC_BG_BACK     (GFL_BG_FRAME1_S)
#define TRC_BG_CARD     (GFL_BG_FRAME2_S)
#define TRC_BG_BADGE_CASE (GFL_BG_FRAME2_M)
#define TRC_BG_BADGE_BACK (GFL_BG_FRAME3_M)

#define UNION_TR_MAX  (16)

enum {
  SEQ_IN,
  SEQ_MAIN,
  SEQ_RETURN_FADEOUT,
  SEQ_END_FADEOUT,
  SEQ_OUT,
  SEQ_REVERSE,
  SEQ_PMSINPUT,
  SEQ_BADGE_VIEW_CALL,
  SEQ_COVER,
  SEQ_SCALING,
};

typedef enum {
  TRC_KEY_REQ_NONE = 0,
  TRC_KEY_REQ_TP_BEAT,
  TRC_KEY_REQ_TP_BRUSH,
  TRC_KEY_REQ_REV_BUTTON,
  TRC_KEY_REQ_RETURN_BUTTON,
  TRC_KEY_REQ_END_BUTTON,
  TRC_KEY_REQ_TRAINER_TYPE,
  TRC_KEY_REQ_PERSONALITY,
  TRC_KEY_REQ_SCALE_BUTTON,
  TRC_KEY_REQ_PMS_CALL,
  TRC_KEY_REQ_BADGE_CALL,
}TRC_KEY_REQ;

typedef enum {
  COVER_CLOSE = 0,
  COVER_OPEN = 1,
}COVER_STATE;

enum{
  SCREEN_SIGN_LEFT=0,
  SCREEN_SIGN_RIGHT,
  SCREEN_SIGN_ALL,
};


// 戻るボタン座標
#define TP_RETURN_X   (  228 )
#define TP_RETURN_Y   ( 21*8 )
#define TP_RETURN_W   (  3*8 )
#define TP_RETURN_H   (  3*8 )

// 終了ボタン座標
#define TP_END_X  (  204 )
#define TP_END_Y  ( 21*8 )
#define TP_END_W  (  3*8 )
#define TP_END_H  (  3*8 )

// カードひっくり返しボタン
#define TP_CARD_X  (  100 )
#define TP_CARD_Y  ( 21*8 )
#define TP_CARD_W  (  8*8 )
#define TP_CARD_H  (  3*8 )

// バッジ画面ボタン(カード戻るボタンでもあり、アニメ開始・停止ボタンでもある)
#define TP_BADGE_X  (    4 )
#define TP_BADGE_Y  ( 21*8 )
#define TP_BADGE_W  (  3*8 )
#define TP_BADGE_H  (  3*8 )

// 精密書き込みボタン
#define TP_LOUPE_X  (   36 )
#define TP_LOUPE_Y  ( 21*8 )
#define TP_LOUPE_W  (  3*8 )
#define TP_LOUPE_H  (  3*8 )

// ペン先ボタン
#define TP_PEN_X  (   68 )
#define TP_PEN_Y  ( 21*8 )
#define TP_PEN_W  (  3*8 )
#define TP_PEN_H  (  3*8 )

// ブックマークボタン
#define TP_BOOKMARK_X  (  180 )
#define TP_BOOKMARK_Y  ( 21*8 )
#define TP_BOOKMARK_W  (  3*8 )
#define TP_BOOKMARK_H  (  3*8 )

// トレーナータイプ
#define TP_TRTYPE_X  (   24 )
#define TP_TRTYPE_Y  (   48 )
#define TP_TRTYPE_W  (  136 )
#define TP_TRTYPE_H  (  2*8 )

// せいかく
#define TP_PERSONAL_X  (   24 )
#define TP_PERSONAL_Y  (   64 )
#define TP_PERSONAL_W  (  136 )
#define TP_PERSONAL_H  (  2*8 )

// 簡易会話
#define TP_PMS_X  (   16 )
#define TP_PMS_Y  (  120 )
#define TP_PMS_W  (  223 )
#define TP_PMS_H  (   31 )

// 右へ移動（拡大時）
#define TP_MOVERIGHT_X  ( 255-24 )
#define TP_MOVERIGHT_Y  (      0 )
#define TP_MOVERIGHT_W  (     24 )
#define TP_MOVERIGHT_H  ( 192-24 )

// 左へ移動（拡大時）
#define TP_MOVELEFT_X  (      0 )
#define TP_MOVELEFT_Y  (      0 )
#define TP_MOVELEFT_W  (     24 )
#define TP_MOVELEFT_H  ( 192-24 )


// 左へ移動（拡大時）

//============================================
// 結構広いタッチ範囲定義
//============================================

// スコアスクロール領域
#define TP_SCORE_SCROL_X  (   16 )
#define TP_SCORE_SCROL_Y  (   16 )
#define TP_SCORE_SCROL_W  (  224 )
#define TP_SCORE_SCROL_H  (   64 )

// 落書き領域（簡易版）
#define TP_PAINTS_X  (   24 )
#define TP_PAINTS_Y  (   88 )
#define TP_PAINTS_W  (  208 )
#define TP_PAINTS_H  (   64 )

// 落書き領域（精密版）
#define TP_PAINTL_X  (   24 )
#define TP_PAINTL_Y  (   24 )
#define TP_PAINTL_W  (  208 )
#define TP_PAINTL_H  (  128 )

// カードアフィン座標初期値
#define TRCARD_CENTER_POSX  ( 128 )
#define TRCARD_CENTER_POSY  (  96 )
#define TRCARD_OFFSET_POSX  (   0 )
#define TRCARD_OFFSET_POSY  (   0 )
#define TRCARD_LEFT_SCALE_CENTER_POSX   (  32 ) // 左側拡大時
#define TRCARD_RIGHT_SCALE_CENTER_POSX  ( 224 ) // 右側拡大時
#define TRCARD_SCALE_CENTER_POSY        ( 152 ) // 拡大時Y座標共通

#define SCALE_SIDE_LEFT_MAX_X (96)  // 拡大、左側時で書ける上限
#define SCALE_SIDE_RIGHT_MIN_X (95) // 拡大、右側時で書ける上限

//ユニオンルームトレーナー表示テーブル
static const int UniTrTable[UNION_TR_MAX][2] =
{
  //男
  { NARC_trc_union_01_NCGR, NARC_trc_union_01_NCLR },
  { NARC_trc_union_02_NCGR, NARC_trc_union_02_NCLR },
  { NARC_trc_union_03_NCGR, NARC_trc_union_03_NCLR },
  { NARC_trc_union_04_NCGR, NARC_trc_union_04_NCLR },
  { NARC_trc_union_05_NCGR, NARC_trc_union_05_NCLR },
  { NARC_trc_union_06_NCGR, NARC_trc_union_06_NCLR },
  { NARC_trc_union_07_NCGR, NARC_trc_union_07_NCLR },
  { NARC_trc_union_08_NCGR, NARC_trc_union_08_NCLR },
  //女              
  { NARC_trc_union_09_NCGR, NARC_trc_union_09_NCLR },
  { NARC_trc_union_10_NCGR, NARC_trc_union_10_NCLR },
  { NARC_trc_union_11_NCGR, NARC_trc_union_11_NCLR },
  { NARC_trc_union_12_NCGR, NARC_trc_union_12_NCLR },
  { NARC_trc_union_13_NCGR, NARC_trc_union_13_NCLR },
  { NARC_trc_union_14_NCGR, NARC_trc_union_14_NCLR },
  { NARC_trc_union_15_NCGR, NARC_trc_union_15_NCLR },
  { NARC_trc_union_16_NCGR, NARC_trc_union_16_NCLR },
};

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_128_D,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,  // メインOBJマッピングモード
  GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
};

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void AllocStrBuf( TR_CARD_WORK * wk );
static void FreeStrBuf( TR_CARD_WORK * wk );

static void SetTrCardBg( void );
static void SetTrCardBgGraphic( TR_CARD_WORK * wk );

static void TrCardBgExit( void );

static void CardDesignDraw(TR_CARD_WORK* wk);
static void CardDataDraw(TR_CARD_WORK* wk);
static BOOL CardRev(TR_CARD_WORK * wk );
static void CardRevAffineSet(TR_CARD_WORK* wk);
static int  CheckInput(TR_CARD_WORK *wk);
static void JumpInputResult( TR_CARD_WORK *wk, int req, int *seq );
static void SetCardPalette(TR_CARD_WORK *wk ,u8 inCardRank, const u8 inPokeBookHold);
static void SetUniTrainerPalette(TR_CARD_WORK *wk ,const u8 inTrainerNo);

static void VBlankFunc( GFL_TCB *tcb, void *work );
static void DispTrainer(TR_CARD_WORK *wk);
static void ClearTrainer(TR_CARD_WORK *wk);

static void ResetAffinePlane(void);

static void DecodeSignData(const u8 *inRawData, u8 *outData);
static void EncodeSignData( const u8 *inBmpData, u8 *outData );

static void TransSignData(const int inFrame, const u8 *inSignData, int flag);
static void DispTouchBarObj( TR_CARD_WORK *wk, int is_back );
static void UpdateSignAnime( TR_CARD_WORK *wk );
static void Trans_SignScreen( const int inFrame, const int flag );
static BOOL CardScaling( TR_CARD_WORK *wk );
static void Change_SignAnimeButton( TR_CARD_WORK *wk, int flag, int OnOff );
static void Start_SignAnimeButton( TR_CARD_WORK *wk, int flag );
static void UpdateTextBlink( TR_CARD_WORK *wk );


static void tp_sign_end_check( TR_CARD_WORK *wk );
static void Stock_TouchPoint( TR_CARD_WORK *wk, int scale_mode );
static void _BmpWinPrint_Rap(     GFL_BMPWIN * win, void * src,
      int src_x, int src_y, int src_dx, int src_dy,
      int win_x, int win_y, int win_dx, int win_dy );
static int DrawPoint_to_Line(  GFL_BMPWIN *win, 
  const u8 *brush, 
  int px, int py, int *sx, int *sy, 
  int count, int flag, int scale_mode, int scale_side );
static void Stock_OldTouch( TOUCH_INFO *all, TOUCH_INFO *stock );
static  int DrawBrushLine( GFL_BMPWIN *win, TOUCH_INFO *all, TOUCH_INFO *old, int draw, u8 *SignData, u8 sign_mode, u8 scale_side );
static void SetBookMark( TR_CARD_WORK *wk);


//============================================================================================
//  グローバル変数
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
GFL_PROC_RESULT TrCardProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_WORK * wk;

  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
  WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  WIPE_ResetWndMask(WIPE_DISP_MAIN);
  WIPE_ResetWndMask(WIPE_DISP_SUB);

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TR_CARD, 0x50000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(TR_CARD_WORK), HEAPID_TR_CARD );
  GFL_STD_MemFill( wk, 0, sizeof(TR_CARD_WORK) );

  wk->heapId = HEAPID_TR_CARD;
  wk->ObjWork.heapId = HEAPID_TR_CARD;
  //引継ぎパラメータ取得
  wk->tcp = pwk;
  wk->TrCardData = wk->tcp->TrCardData;
  wk->key_mode = GFL_UI_CheckTouchOrKey();

  wk->msg_spd = MSGSPEED_GetWait();
  wk->win_type = 0;

  if(wk->TrCardData->Clear_m != 0){ //月が0月でなければ、クリアしたとみなす
    wk->isClear = TRUE;
  }else{
    wk->isClear = FALSE;
  }
  {
    int i;
    u16 flag  = 1;
    for(i=0;i<TR_BADGE_NUM_MAX;i++){
      if(wk->TrCardData->BadgeFlag & flag){
        wk->badge[i] = 1;
      }else{
        wk->badge[i] = 0;
      }
      flag <<= 1;
    }
  }

  //通信中かどうか？
  if(GFL_NET_GetConnectNum() > 0){
    wk->isComm = TRUE;
  }

  GFL_BG_Init( wk->heapId );

  AllocStrBuf(wk);

  GFL_DISP_SetBank( &vramBank );
  SetTrCardBg();
  SetTrCardBgGraphic( wk );

  InitTRCardCellActor( &wk->ObjWork , &vramBank );

  // 裏面表示項目設定
  TRCBmp_MakeScoreList( wk, wk->tcp->TrCardData );

  // 下画面セルアクター登録
  SetTrCardActorSub( &wk->ObjWork);

  //Bmpウィンドウ初期化
  TRCBmp_AddTrCardBmp( wk );

  // 簡易会話システム初期化
  wk->PmsDrawWork = PMS_DRAW_Init( wk->ObjWork.cellUnit, CLSYS_DRAW_SUB, wk->printQue, 
                                   wk->fontHandle, 8, 1, HEAPID_TR_CARD );

  //コロン描く
  TRCBmp_WriteSec(wk,wk->win[TRC_BMPWIN_PLAY_TIME], TRUE, wk->SecBuf);

  // 表・裏設定（ショートカットで裏面指定の際のみ裏に）
  if(wk->tcp->mode==TRCARD_SHORTCUT_BACK){
    wk->tcp->mode = TRCARD_SHORTCUT_NONE; // 一回反映させたらクリア
    wk->is_back = TRUE;
  }else{
    wk->is_back = FALSE;
  }

  // パラメータ初期化
  wk->tcp->value = FALSE;
  wk->IsOpen = COVER_CLOSE;     //ケースは閉じた状態からスタート

  wk->ButtonPushFlg = FALSE;    //ボタン押されてない
  if(wk->TrCardData->SignAnimeOn){  // サインアニメしているか？
    wk->SignAnimePat = SCREEN_SIGN_LEFT;    
  }else{
    wk->SignAnimePat = SCREEN_SIGN_ALL;    
  }
  wk->CardCenterX = TRCARD_CENTER_POSX;
  wk->CardCenterY = TRCARD_CENTER_POSY;
  wk->CardOffsetX = TRCARD_OFFSET_POSX;
  wk->CardOffsetY = TRCARD_OFFSET_POSY;

  
  //拡縮面をリセット
  ResetAffinePlane();

  //カードの初期状態表示
  CardDataDraw(wk);

  wk->vblankFunc = GFUser_VIntr_CreateTCB(VBlankFunc, wk , 1);  // VBlankセット
  wk->vblankTcblSys = GFL_TCBL_Init( wk->heapId , wk->heapId , 3 , 0 );
  //通信アイコンセット
  GFL_NET_ReloadIconTopOrBottom( TRUE, wk->heapId );

  WIPE_SYS_Start( WIPE_PATTERN_FMAS, WIPE_TYPE_SHUTTERIN_DOWN,
          WIPE_TYPE_SHUTTERIN_DOWN, WIPE_FADE_BLACK,
          WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );

  return GFL_PROC_RES_FINISH;
}

#if 0
//----------------------------------------------------------------------------------
/**
 * @brief カードの拡大縮小ポイントのあたりを付けるためのデバッグルーチン
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void debug_scale( TR_CARD_WORK *wk )
{
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
    wk->CardScaleX -= 0x100;
    wk->CardScaleY -= 0x100;
    OS_Printf("scaleX=%d, scaleY=%d, cx=%d, cy=%d, ox=%d, oy=%d\n", 
               wk->CardScaleX, wk->CardScaleY,wk->CardCenterX, wk->CardCenterY,wk->CardOffsetX, wk->CardOffsetY);
    //アフィン変換実行リクエスト
    wk->aff_req = TRUE;
  }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
    wk->CardScaleX += 0x100;
    wk->CardScaleY += 0x100;
    OS_Printf("scaleX=%d, scaleY=%d, cx=%d, cy=%d, ox=%d, oy=%d\n", 
               wk->CardScaleX, wk->CardScaleY,wk->CardCenterX, wk->CardCenterY,wk->CardOffsetX, wk->CardOffsetY);
    //アフィン変換実行リクエスト
    wk->aff_req = TRUE;
  }else if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      wk->CardOffsetY -= 1;
    }else{
      wk->CardCenterY -= 1;
    }
    OS_Printf("scaleX=%d, scaleY=%d, cx=%d, cy=%d, ox=%d, oy=%d\n", 
               wk->CardScaleX, wk->CardScaleY,wk->CardCenterX, wk->CardCenterY,wk->CardOffsetX, wk->CardOffsetY);
    //アフィン変換実行リクエスト
    wk->aff_req = TRUE;
  }else if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      wk->CardOffsetY += 1;
    }else{
      wk->CardCenterY += 1;
    }
    OS_Printf("scaleX=%d, scaleY=%d, cx=%d, cy=%d, ox=%d, oy=%d\n", 
               wk->CardScaleX, wk->CardScaleY,wk->CardCenterX, wk->CardCenterY,wk->CardOffsetX, wk->CardOffsetY);
    //アフィン変換実行リクエスト
    wk->aff_req = TRUE;
  }else if(GFL_UI_KEY_GetCont()&PAD_KEY_LEFT){
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      wk->CardOffsetX -= 1;
    }else{
      wk->CardCenterX -= 1;
    }
    OS_Printf("scaleX=%d, scaleY=%d, cx=%d, cy=%d, ox=%d, oy=%d\n", 
               wk->CardScaleX, wk->CardScaleY,wk->CardCenterX, wk->CardCenterY,wk->CardOffsetX, wk->CardOffsetY);
    //アフィン変換実行リクエスト
    wk->aff_req = TRUE;
  }else if(GFL_UI_KEY_GetCont()&PAD_KEY_RIGHT){
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      wk->CardOffsetX += 1;
    }else{
      wk->CardCenterX += 1;
    }
    OS_Printf("scaleX=%d, scaleY=%d, cx=%d, cy=%d, ox=%d, oy=%d\n", 
               wk->CardScaleX, wk->CardScaleY,wk->CardCenterX, wk->CardCenterY,wk->CardOffsetX, wk->CardOffsetY);
    //アフィン変換実行リクエスト
    wk->aff_req = TRUE;
  }
}
#endif

#define TRAINER_TYPE_GENDER_MAX   ( 8 )

//----------------------------------------------------------------------------------
/**
 * @brief 男は(0-7) 女は(8-15)間のトレーナータイプを足し込む
 *
 * @param   trCard    
 */
//----------------------------------------------------------------------------------
static void _add_UnionTrNo( TR_CARD_DATA *trCard )
{
  if(++trCard->UnionTrNo > (TRAINER_TYPE_GENDER_MAX*(trCard->TrSex+1))-1){
    trCard->UnionTrNo = TRAINER_TYPE_GENDER_MAX*trCard->TrSex;
  }

}


#define SIGN_SE_LINES_LIMIT   ( 5 )

static void _sign_se( TR_CARD_WORK *wk, int lines )
{
  //OS_Printf("lines=%d, sewait=%d\n", lines, wk->sign_se_wait);
  if(wk->sign_se_wait!=0){
    wk->sign_se_wait--;
  }else{
    if(lines>SIGN_SE_LINES_LIMIT){
      wk->sign_se_wait=5;
      if(wk->pen==0){
        PMSND_PlaySE(SND_TRCARD_SIGN);
      }else{
        PMSND_PlaySE(SND_TRCARD_KESHI);
      }
    }
  }
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
GFL_PROC_RESULT TrCardProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_WORK * wk  = mywk;
  
  switch( *seq ){
  case SEQ_IN:
    if( WIPE_SYS_EndCheck() ){
      *seq = SEQ_MAIN;
    }
    break;

  case SEQ_MAIN:
    {
      int req;
      req = CheckInput(wk);           // 入力チェック
      JumpInputResult(wk, req, seq);  // 入力で分岐
      
      UpdateSignAnime(wk);
//      UpdateTextBlink(wk);

      if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
        int line = DrawBrushLine( (GFL_BMPWIN*)wk->TrSignData, &wk->AllTouchResult, 
                                   &wk->OldTouch, 1, wk->TrSignData, wk->ScaleMode, wk->ScaleSide );
        _sign_se(wk, line);
      }
    }
    break;

  case SEQ_RETURN_FADEOUT:
    if(GFL_CLACT_WK_CheckAnmActive(wk->ObjWork.ClActWorkS[ACTS_BTN_BACK])==FALSE){
      WIPE_SYS_Start( WIPE_PATTERN_FSAM, WIPE_TYPE_SHUTTEROUT_UP,
              WIPE_TYPE_SHUTTEROUT_UP, WIPE_FADE_BLACK,
              WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );
      *seq = SEQ_OUT;
    }
    break;
  case SEQ_END_FADEOUT:
    if(GFL_CLACT_WK_CheckAnmActive(wk->ObjWork.ClActWorkS[ACTS_BTN_END])==FALSE){
      WIPE_SYS_Start( WIPE_PATTERN_FSAM, WIPE_TYPE_SHUTTEROUT_UP,
              WIPE_TYPE_SHUTTEROUT_UP, WIPE_FADE_BLACK,
              WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );
      *seq = SEQ_OUT;
    }
    break;
  case SEQ_OUT:
    if( WIPE_SYS_EndCheck() ){
      return GFL_PROC_RES_FINISH;
    }
    break;

  case SEQ_REVERSE: //リバース処理
    if ( CardRev(wk) ){
      *seq = SEQ_MAIN;
    }
    break;
  case SEQ_SCALING:
    if( CardScaling(wk)){
      *seq = SEQ_MAIN;
    }
    break;
  case SEQ_PMSINPUT:  //簡易会話処理を呼ぶ
    WIPE_SYS_Start( WIPE_PATTERN_FSAM, WIPE_TYPE_SHUTTEROUT_UP,
            WIPE_TYPE_SHUTTEROUT_UP, WIPE_FADE_BLACK,
            WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );
    wk->tcp->value = CALL_PMSINPUT;
    *seq = SEQ_OUT;
    break;
  case SEQ_BADGE_VIEW_CALL: // バッジ画面遷移
    if(GFL_CLACT_WK_CheckAnmActive(wk->ObjWork.ClActWorkS[ACTS_BTN_CHANGE])==FALSE){
      WIPE_SYS_Start( WIPE_PATTERN_FSAM, WIPE_TYPE_SHUTTEROUT_UP,
              WIPE_TYPE_SHUTTEROUT_UP, WIPE_FADE_BLACK,
              WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );
      wk->tcp->value = CALL_BADGE;
      *seq = SEQ_OUT;
    }
    break;

  }
  
  if(++wk->scrl_ct >= 256){
    wk->scrl_ct = 0;
  }
  GFL_CLACT_SYS_Main();
  PMS_DRAW_Main( wk->PmsDrawWork );
  PRINTSYS_QUE_Main( wk->printQue );
  if( wk->vblankTcblSys != NULL ){
    GFL_TCBL_Main( wk->vblankTcblSys );
  }
  
  return GFL_PROC_RES_CONTINUE;
}

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
GFL_PROC_RESULT TrCardProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_WORK * wk  = mywk;
  TRCARD_CALL_PARAM *param = pwk;

  // 自分のサインを見ている時はサインデータを反映させる
  if(wk->TrCardData->SignDisenable==0){
    //サイン圧縮
    EncodeSignData( wk->TrSignData, wk->TrCardData->SignRawData);
  
    {
      u8 *SignBuf;
      // サイン書き出しバッファポインタ取得
      SignBuf = (u8*)TRCSave_GetSignDataPtr(GAMEDATA_GetTrainerCardPtr(wk->tcp->gameData));
      MI_CpuCopy8( wk->TrCardData->SignRawData, SignBuf, SIGN_SIZE_X*SIGN_SIZE_Y*8);
    }
  }
  // 編集可能だった場合はユニオン見た目と性格を変更する
  if(param->edit_possible){
    MYSTATUS *my = GAMEDATA_GetMyStatus(wk->tcp->gameData);
    TR_CARD_SV_PTR tr = GAMEDATA_GetTrainerCardPtr(wk->tcp->gameData);
    OS_Printf("見た目を%dに、性格を%dに変更\n", wk->TrCardData->UnionTrNo,wk->TrCardData->Personality);
    MyStatus_SetTrainerView( my, wk->TrCardData->UnionTrNo );
    TRCSave_SetPersonarity(  tr, wk->TrCardData->Personality );
    TRCSave_SetSignAnime(    tr, wk->TrCardData->SignAnimeOn );
  }


  PMS_DRAW_Exit( wk->PmsDrawWork );

  //使用した拡縮面を元に戻す
  ResetAffinePlane();

  FreeStrBuf( wk );         //文字列解放

  RereaseCellObject(&wk->ObjWork);    //2Dオブジェクト関連領域解放

  GFL_HEAP_FreeMemory( wk->TrArcData ); //トレーナーキャラアーカイブデータ解放

  GFL_HEAP_FreeMemory( wk->TrScrnArcData );//トレーナースクリーン解放

  // 文字点滅パレット解放
  GFL_HEAP_FreeMemory( wk->pPalBuf );

  TRCBmp_ExitTrCardBmpWin( wk );      // BMPウィンドウ開放
  TrCardBgExit();     // BGL削除
//  StopTP();             // タッチパネル終了
  GFL_TCB_DeleteTask( wk->vblankFunc );   // VBlankセット
  GFL_TCBL_Exit( wk->vblankTcblSys );
  //動作モード書き戻し
  GFL_UI_SetTouchOrKey(wk->key_mode);

  GFL_PROC_FreeWork( proc );        // ワーク開放

  GFL_HEAP_DeleteHeap( HEAPID_TR_CARD );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * 文字列バッファの確保
 *
 * @param wk    画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void AllocStrBuf( TR_CARD_WORK * wk )
{
  int i;

  //フォント読み込み
  wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapId );

  wk->msgMan = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_trainercard_dat, wk->heapId);

  //固定のバッファ作成
  wk->PlayTimeBuf = GFL_STR_CreateBuffer(TIME_H_DIGIT+1, wk->heapId);
  wk->TmpBuf = GFL_STR_CreateBuffer(TR_STRING_LEN, wk->heapId);
  wk->DigitBuf = GFL_STR_CreateBuffer(TR_DIGIT_LEN, wk->heapId);
  wk->SecBuf =  GFL_STR_CreateBuffer(5, wk->heapId);

  //固定文字列取得
  GFL_MSG_GetString(wk->msgMan, MSG_TCARD_12, wk->SecBuf);

  for(i = 0;i < 3;i++){
    wk->ExpBuf[i] = GFL_MSG_CreateString(wk->msgMan, MSG_TCARD_BACK+i);
  }
  for(i = 0;i < 2;i++){
    wk->SignBuf[i] = GFL_MSG_CreateString(wk->msgMan, MSG_TCARD_SIGN01+i);
  }
  for(i = 0;i < TR_CPRM_STR_MAX;i++){
    wk->CPrmBuf[i] = GFL_MSG_CreateString(wk->msgMan, MSG_TCARD_01+i);
  }
  for(i=0;i<TR_CREDIT_STR_MAX;i++){
    wk->CreditBuf[i] = GFL_MSG_CreateString(wk->msgMan, MSG_TCARD_NUM_KAI+i);
  }
  
  // WordSet初期化
  wk->wordset = WORDSET_Create( wk->heapId );
}

//--------------------------------------------------------------------------------------------
/**
 * 文字列バッファの解放
 *
 * @param wk    画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void FreeStrBuf( TR_CARD_WORK * wk )
{
  int i;
  // WordSet解放
  WORDSET_Delete( wk->wordset );

  for(i=0;i<TR_CREDIT_STR_MAX;i++){
    GFL_STR_DeleteBuffer( wk->CreditBuf[i] );
  }

  for(i = 0;i < TR_CPRM_STR_MAX;i++){
    GFL_STR_DeleteBuffer( wk->CPrmBuf[i] );
  }
  GFL_STR_DeleteBuffer( wk->SignBuf[1] );
  GFL_STR_DeleteBuffer( wk->SignBuf[0] );

  GFL_STR_DeleteBuffer( wk->ExpBuf[2] );
  GFL_STR_DeleteBuffer( wk->ExpBuf[1] );
  GFL_STR_DeleteBuffer( wk->ExpBuf[0] );

  GFL_STR_DeleteBuffer( wk->SecBuf );
  GFL_STR_DeleteBuffer( wk->DigitBuf );
  GFL_STR_DeleteBuffer( wk->TmpBuf );
  GFL_STR_DeleteBuffer( wk->PlayTimeBuf );

  GFL_MSG_Delete( wk->msgMan );

  GFL_FONT_Delete(wk->fontHandle);
}

// カード面パレットをＷＢで替えるためのテーブル
static int card_palette_table[][2]={
  {    NARC_trainer_case_card_w0_NCLR, NARC_trainer_case_card_0_NCLR, },
  {    NARC_trainer_case_card_w1_NCLR, NARC_trainer_case_card_1_NCLR, },
  {    NARC_trainer_case_card_w2_NCLR, NARC_trainer_case_card_2_NCLR, },
  {    NARC_trainer_case_card_w3_NCLR, NARC_trainer_case_card_3_NCLR, },
  {    NARC_trainer_case_card_w4_NCLR, NARC_trainer_case_card_4_NCLR, },
  {    NARC_trainer_case_card_w5_NCLR, NARC_trainer_case_card_5_NCLR, },
  {    NARC_trainer_case_card_6_NCLR,  NARC_trainer_case_card_6_NCLR, },  // バージョン違い
};

//----------------------------------------------------------------------------------
/**
 * @brief カードのパレット転送をランク・WBに対応して行う
 *
 * @param   wk    
 * @param   inCardRank    
 * @param   inVersion
 */
//----------------------------------------------------------------------------------
static void SetCardPalette(TR_CARD_WORK *wk ,u8 inCardRank, const u8 inPokeBookHold)
{
  u32 palette_index;
  int Version = 1;

  // ＷＢのバージョンにあわせてバレット切り替え（WB以外は全て青）
  if(wk->TrCardData->Version==VERSION_WHITE){
    Version = 0;
  }else if(wk->TrCardData->Version==VERSION_BLACK){
    Version = 1;
  }else{
    inCardRank = 6;
  }
  
  OS_Printf("rank=%d, version=%d", inCardRank, Version);

  // ＷＢにあわせてパレット読み込み
  palette_index = card_palette_table[inCardRank][Version];

  // パレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_TRAINERCARD, palette_index,
                                 PALTYPE_SUB_BG, 0, 2*16*16 ,wk->heapId );
  
  // 点滅用にパレット1列目を保存しておく
  wk->pPalBuf = GFL_ARC_UTIL_LoadPalette( ARCID_TRAINERCARD, palette_index, 
                                          &wk->pPalData, wk->heapId );
}


//--------------------------------------------------------------------------------------------
/**
 * ユニオントレーナーパレットセット
 *
 * @param inTrainerNo
 *
 * @return  なし
 */
//--------------------------------------------------------------------------------------------
static void SetUniTrainerPalette(TR_CARD_WORK *wk ,const u8 inTrainerNo)
{
  // ユニオンルーム用のグラフィックは０行目のパレットを転送する
  // が、グラフィックそのもののパレット行は４行目が参照されているのでそこに転送する
  GFL_ARC_UTIL_TransVramPalette( ARCID_TRC_UNION, UniTrTable[inTrainerNo][1], 
                                 PALTYPE_SUB_BG, 32*UNI_TRAINER_PLTT_NO, 32, wk->heapId );
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
static void SetTrCardBg( void )
{

  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

  { // BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_5, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }

  { // FONT (BMP) BG3
    GFL_BG_BGCNT_HEADER  ExAffineBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256, 
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x10000, 0x10000, GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( TRC_BG_FONT, &ExAffineBgCntDat, GFL_BG_MODE_256X16 );
    GFL_BG_ClearFrame( TRC_BG_FONT );
    GFL_BG_SetVisible( TRC_BG_FONT, VISIBLE_ON );
  }

  { // BG (CASE CHAR,MSG_CHAR) BG0 BG1
    GFL_BG_BGCNT_HEADER  TextBgCntDat[] = {
     {  0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, 0x6000, GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE},
     {  0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x04000, 0x6000, GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE},
    };
    GFL_BG_SetBGControl( TRC_BG_MSG,  &TextBgCntDat[0], GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( TRC_BG_MSG );
    GFL_BG_SetVisible( TRC_BG_MSG, VISIBLE_ON );
    GFL_BG_SetBGControl( TRC_BG_BACK, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( TRC_BG_BACK );
    GFL_BG_SetVisible( TRC_BG_BACK, VISIBLE_ON );
  }

  { // BG (CARD CHAR)
    GFL_BG_BGCNT_HEADER  AffineBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x08000, 0x8000, GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( TRC_BG_CARD, &AffineBgCntDat, GFL_BG_MODE_256X16 );
    GFL_BG_ClearFrame( TRC_BG_CARD );
    GFL_BG_SetVisible( TRC_BG_CARD, VISIBLE_ON );
  }

  { // BG (BADGE_CASE CHAR)
    GFL_BG_BGCNT_HEADER  TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( TRC_BG_BADGE_CASE, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( TRC_BG_BADGE_CASE );
    GFL_BG_SetVisible( TRC_BG_BADGE_CASE, VISIBLE_OFF );
  }

  { // BG (BADGE_BACK CHAR)
    GFL_BG_BGCNT_HEADER  TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000, GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( TRC_BG_BADGE_BACK, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( TRC_BG_BADGE_BACK );
    GFL_BG_SetVisible( TRC_BG_BADGE_BACK, VISIBLE_ON );
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief カードグラフィックをVERSIONで読み分けるためのID変更処理
 *
 * @param   version   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_card_ncgr( int version )
{
  if(version==VERSION_WHITE){
    return NARC_trainer_case_card_w_NCGR;
  }else{
    return NARC_trainer_case_card_NCGR;
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief カードスクリーン面(表）をVERSIONで読み分けるためのID変更処理
 *
 * @param   version   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_card_face_nscr( int version )
{
  if(version==VERSION_WHITE){
    return NARC_trainer_case_card_faca_w_NSCR;
  }else{
    return NARC_trainer_case_card_faca_NSCR;
  }

}

//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param wk    画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SetTrCardBgGraphic( TR_CARD_WORK * wk )
{
  // TRAINER_PALETTE(UP_DISPLAY)
  // CARD PALETTE
#ifdef PM_DEBUG
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
    wk->TrCardData->PokeBookFlg = 0;
  }
#endif

  SetCardPalette(wk,wk->TrCardData->CardRank, wk->TrCardData->PokeBookFlg);

  //TRAINER
  if (wk->TrCardData->OtherTrCard==FALSE){
    wk->TrArcData = GFL_ARC_UTIL_LoadBGCharacter( ARCID_TRAINERCARD, NARC_trainer_case_card_trainer_NCGR,
                    FALSE, &wk->TrCharData, wk->heapId);
    if (wk->TrCardData->TrSex == PM_MALE){
      //男
      wk->TrScrnArcData = GFL_ARC_UTIL_LoadScreen( ARCID_TRAINERCARD, NARC_trainer_case_card_trainer01_NSCR,
                          0, &wk->ScrnData, wk->heapId);
    }else{
      //女
      wk->TrScrnArcData = GFL_ARC_UTIL_LoadScreen( ARCID_TRAINERCARD, NARC_trainer_case_card_trainer02_NSCR,
                          0, &wk->ScrnData, wk->heapId);
    }
  }else{
    //ユニオンルームで他の人のデータを見る時
    {
      wk->TrArcData = GFL_ARC_UTIL_LoadBGCharacter( ARCID_TRC_UNION, UniTrTable[wk->TrCardData->UnionTrNo][0],
                    FALSE, &wk->TrCharData, wk->heapId);

      wk->TrScrnArcData = GFL_ARC_UTIL_LoadScreen( ARCID_TRC_UNION, NARC_trc_union_card_test256_NSCR,
                          0, &wk->ScrnData, wk->heapId);
      //トレーナーパレット変更
      SetUniTrainerPalette(wk,wk->TrCardData->UnionTrNo);
    }
  }
  DispTrainer(wk);

  //CARD
  GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_TRAINERCARD, _get_card_ncgr( wk->TrCardData->Version ), TRC_BG_CARD, 0, 0, 0, wk->heapId );
  GFL_ARC_UTIL_TransVramScreen(
      ARCID_TRAINERCARD, _get_card_face_nscr( wk->TrCardData->Version), TRC_BG_CARD, 0, 0, 0, wk->heapId );

  //CARD BACKGROUND
  GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_NCGR, TRC_BG_BACK, 0, 0, 0, wk->heapId );
  GFL_ARC_UTIL_TransVramScreen(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_NSCR, TRC_BG_BACK, 0, 0, 0, wk->heapId );

  //UPLCD BACKGROUND
  GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_NCGR, TRC_BG_BADGE_BACK, 0, 0, 0, wk->heapId );
  GFL_ARC_UTIL_TransVramScreen(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_NSCR, TRC_BG_BADGE_BACK, 0, 0, 0, wk->heapId );
    GFL_ARC_UTIL_TransVramPalette( ARCID_TRAINERCARD, NARC_trainer_case_card_w0_NCLR,
          PALTYPE_MAIN_BG , 0 , 16*2 ,wk->heapId );


//  GFL_ARC_UTIL_TransVramScreen( ARCID_TRAINERCARD, NARC_trainer_case_card_case2bg_NSCR,
//    TRC_BG_BADGE_BACK, 0, 0, 0, wk->heapId );

  //サイン展開
  DecodeSignData( wk->TrCardData->SignRawData, wk->TrSignData );

  // タッチバー準備
  GFL_ARC_UTIL_TransVramBgCharacter( APP_COMMON_GetArcId(),
                                     APP_COMMON_GetBarCharArcIdx(),
                                     TRC_BG_MSG, 0, 0, 0, wk->heapId );
  GFL_ARC_UTIL_TransVramScreen( APP_COMMON_GetArcId(),
                                APP_COMMON_GetBarScrnArcIdx(),
                                TRC_BG_MSG,0,0,0,wk->heapId);
                                
  GFL_BG_ChangeScreenPalette( TRC_BG_MSG, 0, 21, 32, 3, 12 );
  GFL_BG_LoadScreenReq( TRC_BG_MSG );
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
static void TrCardBgExit( void )
{

  
  GFL_DISP_GX_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 |
    GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ, VISIBLE_OFF );
  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 |
    GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ, VISIBLE_OFF );

  GFL_BG_SetScroll( TRC_BG_BACK, GFL_BG_SCROLL_X_SET, 0);
  GFL_BG_SetScroll( TRC_BG_BACK, GFL_BG_SCROLL_Y_SET, 0);
  GFL_BG_SetScroll( TRC_BG_BADGE_BACK, GFL_BG_SCROLL_X_SET, 0);
  GFL_BG_SetScroll( TRC_BG_BADGE_BACK, GFL_BG_SCROLL_Y_SET, 0);

  GFL_BG_FreeBGControl( TRC_BG_FONT );
  GFL_BG_FreeBGControl( TRC_BG_CARD );
  GFL_BG_FreeBGControl( TRC_BG_MSG );
  GFL_BG_FreeBGControl( TRC_BG_BACK );
  GFL_BG_FreeBGControl( TRC_BG_BADGE_CASE );
  GFL_BG_FreeBGControl( TRC_BG_BADGE_BACK );

  GFL_BG_Exit();
}


//----------------------------------------------------------------------------------
/**
 * @brief ハードウェアウインドウの設定
 *
 * @param   flag    
 */
//----------------------------------------------------------------------------------
static void HardWareWindow_Set( int flag )
{
  if(flag){
    G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_OBJ,0);
    G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|GX_WND_PLANEMASK_OBJ,0);
    G2S_SetWnd0Position( 24, 0, 232, 16);
    GXS_SetVisibleWnd(GX_WNDMASK_W0);
  }else{
    GXS_SetVisibleWnd(GX_WNDMASK_NONE);
    
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief 裏面スクリーンを替える
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void Trans_CardBackScreen( TR_CARD_WORK *wk, int version )
{
  // アニメONかOFFかで転送するスクリーンを替える
  const int card_back_screen[][2]={
    { NARC_trainer_case_card_back_w_NSCR, NARC_trainer_case_card_back_NSCR,},    // サインアニメOFF
    { NARC_trainer_case_card_back2_w_NSCR, NARC_trainer_case_card_back2_NSCR,}, // サインアニメON
  };
  if(version==VERSION_WHITE){
    version = 0;
  }else{
    version = 1;
  }

  GFL_ARC_UTIL_TransVramScreen( ARCID_TRAINERCARD, 
                                card_back_screen[wk->TrCardData->SignAnimeOn][version],
                                TRC_BG_CARD, 0, 0, 0, wk->heapId );
/*
  if(wk->TrCardData->SignAnimeOn){
    GFL_ARC_UTIL_TransVramScreen(
      ARCID_TRAINERCARD, NARC_trainer_case_card_back2_NSCR, TRC_BG_CARD, 0, 0, 0, wk->heapId );
  }else{
    GFL_ARC_UTIL_TransVramScreen(
      ARCID_TRAINERCARD, NARC_trainer_case_card_back_NSCR, TRC_BG_CARD, 0, 0, 0, wk->heapId );
  }
*/
}


//--------------------------------------------------------------------------------------------
/**
 *  @brief  wk->is_back変数の状態によって、カードの表or裏のViewをクリア＆書き込み
 */
//--------------------------------------------------------------------------------------------
static void CardDesignDraw(TR_CARD_WORK* wk)
{
  if (wk->is_back == FALSE){
    GFL_ARC_UTIL_TransVramScreen(
        ARCID_TRAINERCARD, _get_card_face_nscr( wk->TrCardData->Version), TRC_BG_CARD, 0, 0, 0, wk->heapId );
    //スクリーンクリア
    GFL_BG_ClearScreen( TRC_BG_TRAINER );
//    HardWareWindow_Set( 0 );
  }else{
    // 裏面カードスクリーン転送
    Trans_CardBackScreen(wk, wk->TrCardData->Version);
    //トレーナー消す
    ClearTrainer(wk);
    //スクリーンクリア
    GFL_BG_ClearScreen( TRC_BG_SIGN );
//    HardWareWindow_Set( 1 );
  }
}


//--------------------------------------------------------------------------------------------
/**
 *  @brief  wk->is_back変数の状態によって、カードの表or裏のViewを作成
 */
//--------------------------------------------------------------------------------------------
static void CardDataDraw(TR_CARD_WORK* wk)
{
  //背景デザイン書き込み
  CardDesignDraw(wk);
  TRCBmp_WriteTrWinInfo(wk, wk->win, wk->TrCardData );
  PMS_DRAW_Main( wk->PmsDrawWork );

  if (wk->is_back == FALSE){  //表
    TRCBmp_WriteScoreListWin( wk, wk->scrol_point, 0, 0 );
    //トレーナー表示
    DispTrainer(wk);
  }else{  //裏面表示
    TRCBmp_WriteTrWinInfoRev(wk, wk->win, wk->TrCardData );
    TransSignData(TRC_BG_SIGN, wk->TrSignData, wk->SignAnimePat);
  }
  
  // タッチバーOBJ周りの再描画
  DispTouchBarObj( wk, wk->is_back );

  TRCBmp_TransTrWinInfo(wk,wk->win);
}


//----------------------------------------------------------------------------------
/**
 * @brief 自分のカードを編集可能状態で見るときのボタン表示・非表示制御
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void EditOK_MineCardAppear( TR_CARD_WORK *wk, int is_back )
{
  BOOL flag = FALSE;
 // 表
  if(is_back==0){  
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,  APP_COMMON_BARICON_RETURN,TRUE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, ANMS_LOUPE_L,             FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_PEN,   ANMS_BLACK_PEN_L,         FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE,ANMS_BADGE_L,             TRUE);

    // ショートカット登録しているか
    flag = GAMEDATA_GetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_FRONT );
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF+flag, TRUE);

  // 裏
  }else{          
    // 通常表示
    if(wk->ScaleMode==0){
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,  APP_COMMON_BARICON_RETURN,TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END,   APP_COMMON_BARICON_EXIT,  TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN,  ANMS_TURN_L,              TRUE);

      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, ANMS_LOUPE_L,           wk->TrCardData->SignAnimeOn^1);
      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_WHITE_PEN_L-wk->pen*2, wk->TrCardData->SignAnimeOn^1);

      // サインアニメ中か
      Change_SignAnimeButton( wk, wk->TrCardData->SignAnimeOn, TRUE);

      // ショートカット登録しているか
      flag = GAMEDATA_GetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_BACK );
      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF+flag, TRUE);

    // 拡大表示
    }else{
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,   APP_COMMON_BARICON_RETURN_OFF, TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END,    APP_COMMON_BARICON_EXIT_OFF,   TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE,  ANMS_SIMPLE_L,                 TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_PEN,    ANMS_WHITE_PEN_L-wk->pen*2,    TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN,   ANMS_TURN_L,                   FALSE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE, ANMS_ANIME_L,                  FALSE);
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * @brief 自分のカードを編集可能状態で見るときのボタン表示・非表示制御
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void EditNG_MineCardAppear( TR_CARD_WORK *wk, int is_back )
{
  BOOL flag = FALSE;
 // 表
  if(is_back==0){  
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,  APP_COMMON_BARICON_RETURN,TRUE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, ANMS_LOUPE_L,             FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_PEN,   ANMS_BLACK_PEN_L,         FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE,ANMS_BADGE_L,             TRUE);

    // ショートカット登録しているか
    flag = GAMEDATA_GetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_FRONT );
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF+flag, TRUE);

  // 裏
  }else{          
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,  APP_COMMON_BARICON_RETURN,TRUE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END,   APP_COMMON_BARICON_EXIT,  TRUE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, ANMS_LOUPE_L,             FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN,  ANMS_TURN_L,              TRUE);

    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_WHITE_PEN_L-wk->pen*2, FALSE);

    // サインアニメ中か
    Change_SignAnimeButton( wk, wk->TrCardData->SignAnimeOn, FALSE);

    // ショートカット登録しているか
    flag = GAMEDATA_GetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_BACK );
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF+flag, TRUE);
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 他人のカードを編集可能状態で見るときのボタン表裏表示・非表示制御
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void OtherCardAppear( TR_CARD_WORK *wk, int is_back )
{
 // 表
  if(is_back==0){  
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,  APP_COMMON_BARICON_RETURN,     TRUE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END,   APP_COMMON_BARICON_EXIT_OFF,   FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE,ANMS_BADGE_L,                  FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_PEN,   ANMS_BLACK_PEN_L,              FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BOOKMARK,APP_COMMON_BARICON_CHECK_OFF,FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, 0,FALSE);

  // 裏
  }else{          
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK,  APP_COMMON_BARICON_RETURN,TRUE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END,   APP_COMMON_BARICON_EXIT,  FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, ANMS_LOUPE_L,             FALSE);
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN,  ANMS_TURN_L,              TRUE);
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_WHITE_PEN_L-wk->pen*2, FALSE);
    Change_SignAnimeButton( wk, wk->TrCardData->SignAnimeOn, FALSE);
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF, FALSE);
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief 表裏の切り替わり時にタッチバーのOBJ表示状態を変更する
 *
 * @param   wk       TR_CARD_WORK
 * @param   is_back  表・裏
 */
//----------------------------------------------------------------------------------
static void DispTouchBarObj( TR_CARD_WORK *wk, int is_back )
{

  // 人のカードを見ている
  if(wk->TrCardData->OtherTrCard==TRUE){
    OtherCardAppear( wk, is_back );
  // 自分のカードだが編集不可状態
  }else if(wk->tcp->edit_possible==FALSE){
    EditNG_MineCardAppear(wk, is_back);
  // 編集可能な自分のカード
  }else{
    EditOK_MineCardAppear(wk, is_back);
  }
  
 
}


#define ROTATE_SPEED (0x400)    //90度のカウントアップ
#define ROTATE_MAX (0x4000)   //360度 = 0x10000　90度 = 0x4000 = 16384
#define START_SCALE ( FX32_ONE + FX32_CONST( 0.015f ) ) //左右を2ピクセル拡大させる倍率
#define CARD_SCALE_MIN  ( 8.0f )  //最低限表示される厚み(ピクセル)
#define CARD_SCALE_MIN_FX ( FX32_CONST(CARD_SCALE_MIN/256.0f) ) //最低限表示される厚み


//--------------------------------------------------------------------------------------------
/**
 * カードをひっくり返す
 *
 * @param wk    画面のワーク
 *
 * @return  BOOL  TRUE:終了 FALSE：処理中
 */
//--------------------------------------------------------------------------------------------
static BOOL CardRev( TR_CARD_WORK *wk )
{
  BOOL rc;
  MtxFx22 mtx;
    fx32 rScale_x,rScale_y;
  rc = FALSE;
  switch(wk->sub_seq){
  case 0:
    wk->RotateCount = 0;
    //情報を消す
    //ちょっと拡大して
    wk->CardScaleX = START_SCALE;
    wk->CardScaleY = START_SCALE;
    PMSND_PlaySE( SND_TRCARD_REV );   //ひっくり返す音
    if(wk->is_back==0){
      PMS_DRAW_SetPmsObjVisible( wk->PmsDrawWork, 0, FALSE );
    }
    wk->sub_seq++;
    break;
  case 1:
    //幅を縮める
    wk->CardScaleX = FX_Mul(FX_CosIdx(wk->RotateCount),START_SCALE);
    if (wk->CardScaleX <= CARD_SCALE_MIN_FX){//カードスクリーン変更
      wk->CardScaleX = CARD_SCALE_MIN_FX; //バグ表示防止(値はカードの厚みを維持できる位の目分量)
      wk->sub_seq++;
    }

    wk->RotateCount += ROTATE_SPEED;
    if( wk->RotateCount >= ROTATE_MAX )
    {
      wk->RotateCount = ROTATE_MAX;
    }
    break;
  case 2:
    wk->is_back ^= 1;
    CardDesignDraw(wk);
    DispTouchBarObj( wk, wk->is_back ); // タッチバーOBJ周りの再描画

    wk->sub_seq++;
    break;
  case 3:
    if (wk->is_back == FALSE){
      //トレーナー表示
      DispTrainer(wk);
    }else{
      TransSignData(TRC_BG_SIGN, wk->TrSignData, wk->SignAnimePat);
    }
    wk->sub_seq++;
    break;
  case 4:
    TRCBmp_TransTrWinInfo(wk,wk->win);
    wk->sub_seq++;
    break;
  case 5:
    //幅を広げる
    wk->RotateCount -= ROTATE_SPEED;
    if( wk->RotateCount <= 0 )
      wk->RotateCount = 0;

    wk->CardScaleX = FX_Mul(FX_CosIdx(wk->RotateCount),START_SCALE);
    if (wk->CardScaleX <= CARD_SCALE_MIN_FX){//カードスクリーン変更
      wk->CardScaleX = CARD_SCALE_MIN_FX; //バグ表示防止(値はカードの厚みを維持できる位の目分量)
    }
    if( wk->RotateCount == 0 )
    {
      //元のサイズへ
      wk->CardScaleX = 1 << FX32_SHIFT;
      wk->CardScaleY = 1 << FX32_SHIFT;
      rc = TRUE;
      if(wk->is_back==0){
        PMS_DRAW_VisibleSet( wk->PmsDrawWork, 0, TRUE );
        PMS_DRAW_SetPmsObjVisible( wk->PmsDrawWork, 0, TRUE );
      }
    }
    break;
  }
  //アフィン変換実行リクエスト
  wk->aff_req = TRUE;
  return rc;
}

//----------------------------------------------------------------------------------
/**
 * @brief ハードウェアウインドウと半透明を設定する
 *
 * @param   side    
 */
//----------------------------------------------------------------------------------
static void _blend_win_set( int side )
{
  // 半透明とウインドウ設定OFF
  if(side<0){
    G2S_BlendNone();
    GXS_SetVisibleWnd(GX_WNDMASK_NONE);
  // 右側をウインドウ設定
  }else if(side==0){
    // 輝度ダウン
    G2S_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3, -8);
  
    // ウインドウの設定
    G2S_SetWnd0Position( 256-32, 0, 256, 192-24);
    G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|
                            GX_WND_PLANEMASK_OBJ, 1 );
    G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|
                            GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|
                            GX_WND_PLANEMASK_OBJ,0 );
    GXS_SetVisibleWnd(GX_WNDMASK_W0);

  // 左側をウインドウ設定
  }else if(side==1){
    // 輝度ダウン
    G2S_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3, -8);
  
    // ウインドウの設定
    G2S_SetWnd0Position( 0, 0, 32, 192-24);
    G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|
                            GX_WND_PLANEMASK_OBJ, 1 );
    G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|
                            GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|
                            GX_WND_PLANEMASK_OBJ,0 );
    GXS_SetVisibleWnd(GX_WNDMASK_W0);
  
  }
}


enum{
  CARDSCALE_WIDE_START=0,
  CARDSCALE_WIDE_CALC,
  CARDSCALE_WIDE_END,
  CARDSCALE_SMALL_START,
  CARDSCALE_SMALL_CALC,
  CARDSCALE_SMALL_END,
  CARDSCALE_MOVE_LEFT_START,
  CARDSCALE_MOVE_LEFT_CALC,
  CARDSCALE_MOVE_LEFT_END,
  CARDSCALE_MOVE_RIGHT_START,
  CARDSCALE_MOVE_RIGHT_CALC,
  CARDSCALE_MOVE_RIGHT_END,
};

//----------------------------------------------------------------------------------
/**
 * @brief 拡大サイン画面に変更する(行く・戻る）
 *
 * @param   wk    
 *
 * @retval  BOOL    TRUE:終了
 */
//----------------------------------------------------------------------------------
static BOOL CardScaling( TR_CARD_WORK *wk )
{
  switch(wk->sub_seq){

  // 拡大画面へ移行
  case CARDSCALE_WIDE_START:
    if(wk->ScaleSide==0){
      wk->CardCenterX = TRCARD_LEFT_SCALE_CENTER_POSX;
      wk->CardCenterY = TRCARD_SCALE_CENTER_POSY;
    }else{
      wk->CardCenterX = TRCARD_RIGHT_SCALE_CENTER_POSX;
      wk->CardCenterY = TRCARD_SCALE_CENTER_POSY;
    }
    wk->sub_seq = 1;
    break;
  case CARDSCALE_WIDE_CALC:
    wk->CardScaleX+=0x100;
    wk->CardScaleY+=0x100;
    if(wk->CardScaleX >= 0x2000){
      wk->CardScaleX = 0x2000;
      wk->CardScaleY = 0x2000;
      wk->sub_seq = 2;
    }
    break;
  case CARDSCALE_WIDE_END:
    wk->ScaleMode = 1;
    DispTouchBarObj( wk, wk->is_back ); // タッチバーOBJ周りの再描画
    _blend_win_set(wk->ScaleSide);
    return TRUE;
    break;

  // 通常画面へ移行
  case CARDSCALE_SMALL_START:
    wk->sub_seq = 4;
    break;
  case CARDSCALE_SMALL_CALC:
    wk->CardScaleX-=0x100;
    wk->CardScaleY-=0x100;
    if(wk->CardScaleX <= 0x1000){
      wk->CardScaleX = 0x1000;
      wk->CardScaleY = 0x1000;
      wk->sub_seq = 5;
    }
    break;
  case CARDSCALE_SMALL_END:
    wk->CardCenterX = TRCARD_CENTER_POSX;
    wk->CardCenterY = TRCARD_CENTER_POSY;
    wk->ScaleMode = 0;
    DispTouchBarObj( wk, wk->is_back ); // タッチバーOBJ周りの再描画
    _blend_win_set(-1);
    return TRUE;
    break;

  // 拡大状態の際に左側の拡大面に移動する処理
  case CARDSCALE_MOVE_LEFT_START:
    wk->sub_seq = CARDSCALE_MOVE_LEFT_CALC;
    break;
  case CARDSCALE_MOVE_LEFT_CALC:
    wk->CardCenterX-=8;
    if( wk->CardCenterX < TRCARD_LEFT_SCALE_CENTER_POSX){
      wk->CardCenterX = TRCARD_LEFT_SCALE_CENTER_POSX;
      wk->sub_seq = CARDSCALE_MOVE_LEFT_END;
    }
    break;
  case CARDSCALE_MOVE_LEFT_END:
    wk->ScaleSide ^= 1;
    _blend_win_set(wk->ScaleSide);
    return TRUE;
    break;

  // 拡大状態の際に右側の拡大面に移動する処理
  case CARDSCALE_MOVE_RIGHT_START:
    wk->sub_seq = CARDSCALE_MOVE_RIGHT_CALC;
    break;
  case CARDSCALE_MOVE_RIGHT_CALC:
    wk->CardCenterX+=8;
    if( wk->CardCenterX > TRCARD_RIGHT_SCALE_CENTER_POSX){
      wk->CardCenterX = TRCARD_RIGHT_SCALE_CENTER_POSX;
      wk->sub_seq = CARDSCALE_MOVE_RIGHT_END;
    }
    break;
  case CARDSCALE_MOVE_RIGHT_END:
    wk->ScaleSide ^= 1;
    _blend_win_set(wk->ScaleSide);
    return TRUE;
    break;

  }
  
  //アフィン変換実行リクエスト
  wk->aff_req = TRUE;
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief BGアフィン変換リクエスト
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void CardRevAffineSet(TR_CARD_WORK* wk)
{
  MtxFx22 mtx;

  mtx._00 = FX_Inv(wk->CardScaleX);
  mtx._01 = 0;
  mtx._10 = 0;
  mtx._11 = FX_Inv(wk->CardScaleY);
//  SVC_WaitVBlankIntr();          // Waiting the end of VBlank interrup
  G2S_SetBG2Affine(&mtx,          // a matrix for rotation and scaling
          wk->CardCenterX, wk->CardCenterY,      // the center of rotation
          wk->CardOffsetX, wk->CardOffsetY       // the reference point before rotation and scaling applied
      );
  G2S_SetBG3Affine(&mtx,          // a matrix for rotation and scaling
          wk->CardCenterX, wk->CardCenterY,      // the center of rotation
          wk->CardOffsetX, wk->CardOffsetY       // the reference point before rotation and scaling applied
      );
  wk->aff_req = FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * キーとタッチの切り替え判定
 *
 * @param wk    画面のワーク
 *
 * @return  int   リクエスト
 */
//--------------------------------------------------------------------------------------------
static int CheckKTStatus(TR_CARD_WORK* wk)
{
  if(wk->key_mode == GFL_APP_KTST_TOUCH){
    if(GFL_UI_TP_GetCont() != 0){ //
      return 0;
    }
    if(GFL_UI_KEY_GetCont() != 0){
      wk->key_mode = GFL_APP_KTST_KEY;
      return 1;
    }
  }else{
    if(GFL_UI_KEY_GetCont() != 0){
      return 0;
    }
    if(GFL_UI_TP_GetCont() != 0){
      wk->key_mode = GFL_APP_KTST_TOUCH;
      return 0;
    }
  }
  return 0;
}

//----------------------------------------------------------------------------------
/**
 * @brief キー処理
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int CheckKey(TR_CARD_WORK* wk)
{
  const int keyTrg = GFL_UI_KEY_GetTrg();
  if( keyTrg & PAD_BUTTON_CANCEL )
  {
    if(wk->ScaleMode==0){
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK, 9, TRUE);
      PMSND_PlaySE( SND_TRCARD_CANCEL );   //終了音
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      return TRC_KEY_REQ_RETURN_BUTTON;
    }
  }
  else if(keyTrg & PAD_BUTTON_X )
  {
    if(wk->ScaleMode==0 && wk->TrCardData->OtherTrCard==FALSE){
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END, 8, TRUE);
      PMSND_PlaySE( SND_TRCARD_END );   //終了音
      return TRC_KEY_REQ_END_BUTTON;
    }
    
  }
  else if(keyTrg & (PAD_KEY_LEFT|PAD_KEY_RIGHT))
  {
    // 拡大していない
    if(wk->ScaleMode==0){
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN, ANMS_TURN_G ,TRUE);
      return TRC_KEY_REQ_REV_BUTTON;
    }
  }
  else if(keyTrg & PAD_BUTTON_Y)
  {
    // ブックマーク（Yボタンメニュー）登録
    if(wk->TrCardData->OtherTrCard==FALSE){ // 自分のカードであればバッジ画面へ
      SetBookMark( wk );
    }
  }
  else if(keyTrg & PAD_BUTTON_DECIDE)
  {
    // 自分のカードで表で拡大モードでなければあればバッジ画面へ
    if(wk->TrCardData->OtherTrCard==FALSE && wk->is_back==0){ 
      PMSND_PlaySE( SND_TRCARD_DECIDE );
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE, ANMS_BADGE_G, TRUE);
      return TRC_KEY_REQ_BADGE_CALL;
    }
  }

  return TRC_KEY_REQ_NONE;
}

//----------------------------------------------------------------------------------
/**
 * @brief トレーナーカードのブックマーク処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void SetBookMark( TR_CARD_WORK *wk)
{
  PMSND_PlaySE( SND_TRCARD_BOOKMARK );

  if(wk->is_back==0){   // 表
    BOOL flag = GAMEDATA_GetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_FRONT );
    flag ^=1;
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF+flag, TRUE);
    GAMEDATA_SetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_FRONT, flag );
  }else{                // 裏
    BOOL flag = GAMEDATA_GetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_BACK );
    flag ^=1;
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BOOKMARK, APP_COMMON_BARICON_CHECK_OFF+flag, TRUE);
    GAMEDATA_SetShortCut( wk->tcp->gameData, SHORTCUT_ID_TRCARD_BACK, flag );
  
  }

}


//----------------------------------------------------------------------------------
/**
 * @brief アニメボタンの表示を切り替える
 *
 * @param   wk    
 *
 * @retval  static    
 */
//----------------------------------------------------------------------------------
static void Change_SignAnimeButton( TR_CARD_WORK *wk, int flag, int OnOff )
{
  if(flag==0){  // 再生ボタン
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_CHANGE, ANMS_ANIME_L, OnOff);
  }else{
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_CHANGE, ANMS_STOP_L,  OnOff);
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief アニメボタンの表示を切り替える
 *
 * @param   wk    
 *
 * @retval  none
 */
//----------------------------------------------------------------------------------
static void Start_SignAnimeButton( TR_CARD_WORK *wk, int flag )
{
  if(flag==0){  // 再生ボタン有効
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_CHANGE, ANMS_ANIME_G, TRUE);
    GFL_CLACT_WK_SetDrawEnable( wk->ObjWork.ClActWorkS[ACTS_BTN_LOUPE], FALSE );
    GFL_CLACT_WK_SetDrawEnable( wk->ObjWork.ClActWorkS[ACTS_BTN_PEN], FALSE );
  }else{        // 停止ボタン有効
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_CHANGE, ANMS_STOP_G,  TRUE);
    GFL_CLACT_WK_SetDrawEnable( wk->ObjWork.ClActWorkS[ACTS_BTN_LOUPE], TRUE );
    GFL_CLACT_WK_SetDrawEnable( wk->ObjWork.ClActWorkS[ACTS_BTN_PEN], TRUE );
  }

}


//----------------------------------------------------------------------------------
/**
 * @brief サインアニメ初期化処理
 *
 * @param   wk    
 * @param   flag    
 */
//----------------------------------------------------------------------------------
static void Change_SignAnime( TR_CARD_WORK *wk, int flag )
{
  if(flag){
    wk->SignAnimePat  = SCREEN_SIGN_LEFT;
    wk->SignAnimeWait = 0;
  }else{
    wk->SignAnimePat  = SCREEN_SIGN_ALL;
  }
  Trans_SignScreen( TRC_BG_SIGN, wk->SignAnimePat );
}


//----------------------------------------------------------------------------------
/**
 * @brief タッチされたボタンは機能するか？
 *
 * @param   wk    TR_CARD_WORK
 * @param   hitNo ボタンナンバー
 *
 * @retval  BOOL  TRUE:起動させる・FALSE:させない
 */
//----------------------------------------------------------------------------------
static BOOL _sign_eneble_check( TR_CARD_WORK *wk, int hitNo )
{
  // 操作できないモードか？
  if(wk->TrCardData->SignDisenable){
    // できないモードの時はエディット関連のボタンは無効
    if( hitNo>=3 && hitNo<=9 ){
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief 通常時タッチ処理
 *
 * @param   wk    
 * @param   hitNo   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int normal_touch_func( TR_CARD_WORK *wk, int hitNo )
{
  // サイン不可モードの時はエディット関連は機能させない
  if(_sign_eneble_check(wk, hitNo)==TRUE){
    return TRC_KEY_REQ_NONE;
  }

  switch(hitNo){
  case 0:     // 戻る
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_BACK, 9, TRUE);
    PMSND_PlaySE( SND_TRCARD_CANCEL );
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
    return TRC_KEY_REQ_RETURN_BUTTON;
    break;
  case 1:     // 終了
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_END, 8, TRUE);
    PMSND_PlaySE( SND_TRCARD_END );
    return TRC_KEY_REQ_END_BUTTON;
    break;
  case 2:     // カード裏返しボタン
    GFL_UI_TP_GetPointCont( &wk->tp_x , &wk->tp_y );
    SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN, ANMS_TURN_G ,TRUE);
    return TRC_KEY_REQ_REV_BUTTON;
    break;
  case 3:     // バッジ画面ボタン・アニメON/OFFボタン
    if(wk->is_back){
      // アニメON／OFF
      if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
        Start_SignAnimeButton( wk, wk->TrCardData->SignAnimeOn );
        wk->TrCardData->SignAnimeOn ^=1;
        Trans_CardBackScreen(wk, wk->TrCardData->Version);
        Change_SignAnime( wk, wk->TrCardData->SignAnimeOn );
        OS_Printf("SignAnime = %d\n", wk->TrCardData->SignAnimeOn);
        PMSND_PlaySE( SND_TRCARD_ANIME );
      }
    }else{  
      // バッジ画面へ
      PMSND_PlaySE( SND_TRCARD_DECIDE );
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE, ANMS_BADGE_G, TRUE);
      return TRC_KEY_REQ_BADGE_CALL;
    }
    break;
  case 4:     // 精密描画ボタン
    if(wk->is_back && wk->TrCardData->SignAnimeOn==0){
      if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
          if(wk->ScaleMode==0){
            SetSActDrawSt(&wk->ObjWork,ACTS_BTN_LOUPE, ANMS_LOUPE_G, TRUE);
            PMSND_PlaySE( SND_TRCARD_SCALEUP );
            wk->sub_seq = 0;
          }else{
            SetSActDrawSt(&wk->ObjWork,ACTS_BTN_LOUPE, ANMS_SIMPLE_G, TRUE);
            PMSND_PlaySE( SND_TRCARD_SCALEDOWN );
            wk->sub_seq = 3;
          }
          return TRC_KEY_REQ_SCALE_BUTTON;
      }
    }
    break;
  case 5:     // ペン先ボタン
    if(wk->is_back){
      // 編集可能 & アニメ中じゃなかったら
      if(wk->tcp->TrCardData->EditPossible && wk->TrCardData->SignAnimeOn==0){    
        PMSND_PlaySE( SND_TRCARD_PEN );
        SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_WHITE_PEN_L-wk->pen*2+1, TRUE);
        wk->pen ^=1;
        OS_Printf("pen touch\n");
      }
    }
    break;
  case 6:     // ブックマークボタン
    SetBookMark( wk );
    break;
  case 7:     // トレーナータイプ
    if(wk->is_back==0){
      return TRC_KEY_REQ_TRAINER_TYPE;
    }
    break;
  case 8:     // 性格
    if(wk->is_back==0){
      return TRC_KEY_REQ_PERSONALITY;
    }
    break;
  case 9:     // 簡易会話
    if(wk->is_back==0){
      return TRC_KEY_REQ_PMS_CALL;
    }
    break;
  }
  return TRC_KEY_REQ_NONE;
}

//----------------------------------------------------------------------------------
/**
 * @brief 拡大モード時タッチ処理(かなりの処理が無視される）
 *
 * @param   wk    
 * @param   hitNo   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int large_touch_func( TR_CARD_WORK *wk, int hitNo )
{
  switch(hitNo){
  case 0:     // 戻る
  case 1:     // 終了
  case 2:     // カード裏返しボタン
  case 3:     // バッジ画面ボタン・アニメON/OFFボタン
  case 7:     // トレーナータイプ
  case 8:     // 性格
    break;
  case 4:     // 精密描画ボタン
    if(wk->is_back){
      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_LOUPE, ANMS_SIMPLE_G, TRUE);
      PMSND_PlaySE( SND_TRCARD_SCALEDOWN );
      wk->sub_seq = 3;
      return TRC_KEY_REQ_SCALE_BUTTON;
    }
    break;
  case 5:     // ペン先ボタン
    if(wk->is_back){
      PMSND_PlaySE( SND_TRCARD_PEN );
      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_WHITE_PEN_L-wk->pen*2+1, TRUE);
      wk->pen ^=1;
      OS_Printf("pen touch\n");
    }
    break;
  case 6:     // ブックマークボタン
    SetBookMark( wk );
    break;
  case 10:     // 右へ移動
    if(wk->ScaleSide==0){
      PMSND_PlaySE( SND_TRCARD_ANIME );
      wk->sub_seq = CARDSCALE_MOVE_RIGHT_START;
      return TRC_KEY_REQ_SCALE_BUTTON;
    }
    break;
  case 11:    // 左へ移動
    if(wk->ScaleSide==1){
      PMSND_PlaySE( SND_TRCARD_ANIME );
      wk->sub_seq = CARDSCALE_MOVE_LEFT_START;
      return TRC_KEY_REQ_SCALE_BUTTON;
    }
    break;
  }
  return TRC_KEY_REQ_NONE;
}


// スクロール
static const GFL_UI_TP_HITTBL sc_Scrol_TpRect[] = {
  { TP_SCORE_SCROL_Y, TP_SCORE_SCROL_Y+TP_SCORE_SCROL_H, TP_SCORE_SCROL_X, TP_SCORE_SCROL_X+TP_SCORE_SCROL_W }, // スクロール
  {GFL_UI_TP_HIT_END,0,0,0}
};

//-----------------------------------------------------------------------------
/**
 *  @brief  カード裏面：タッチスクロール終了判定＆処理
 *
 *  @param  TR_CARD_WORK *wk 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void tp_scroll_end_check( TR_CARD_WORK *wk )
{
  if( wk->is_back && GFL_UI_TP_HitCont( sc_Scrol_TpRect ) != 0 )
  {
    if(wk->card_list_col==1)
    {
      TRCBmp_WriteScoreListWin( wk, wk->scrol_point, 1, 0 );
      wk->card_list_col = 0;
      wk->scroll_se_wait = 0;
      wk->b_touch_scroll = FALSE;
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  カード裏面：タッチスクロール処理
 *
 *  @param  TR_CARD_WORK *wk 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void tp_scroll( TR_CARD_WORK *wk )
{
  u32 x,y;
  
  if(GFL_UI_TP_HitTrg( sc_Scrol_TpRect )==0){
    wk->b_touch_scroll = TRUE;
  }
  
  if( wk->b_touch_scroll && GFL_UI_TP_HitCont( sc_Scrol_TpRect )==0){
    if(GFL_UI_TP_GetTrg()){
      GFL_UI_TP_GetPointTrg( &x, &y );
      wk->touch_sy    = y;
      wk->scrol_start = wk->scrol_point;
    }
    if(GFL_UI_TP_GetCont()){
      GFL_UI_TP_GetPointCont( &x, &y );
      wk->scrol_point = wk->scrol_start - (wk->touch_sy-y);
//      OS_Printf("sy=%d,y=%d,start=%d,point=%d\n",wk->touch_sy,y,wk->scrol_start,wk->scrol_point);
    }
    // 範囲外チェック
    if(wk->scrol_point<-(wk->score_max-4)*16){
      wk->scrol_point = -(wk->score_max-4)*16;
    }else if(wk->scrol_point>0){
      wk->scrol_point = 0;
    }

    // スクロール値の変更がある場合はレコードリスト再描画
    if(wk->old_scrol_point!=wk->scrol_point){
      wk->card_list_col = 1;
      TRCBmp_WriteScoreListWin( wk, wk->scrol_point, 1, 1 );
      PMSND_PlaySE(SND_TRCARD_SLIDE);
    }
    wk->old_scrol_point=wk->scrol_point;
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief タッチ判定
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int CheckTouch(TR_CARD_WORK* wk)
{
  int ret;
  u16 pat;
  int hitNo;
  // タッチバー
  static const GFL_UI_TP_HITTBL Btn_TpRect[] = {
    { TP_RETURN_Y,   TP_RETURN_Y+TP_RETURN_H,    TP_RETURN_X,   TP_RETURN_X+TP_RETURN_W }, // 戻る
    { TP_END_Y,      TP_END_Y+TP_END_H,          TP_END_X,      TP_END_X+TP_END_W },       // 終了
    { TP_CARD_Y,     TP_CARD_Y+TP_CARD_H,        TP_CARD_X,     TP_CARD_X+TP_CARD_W },     // カード裏返しボタン
    { TP_BADGE_Y,    TP_BADGE_Y+TP_BADGE_H,      TP_BADGE_X,    TP_BADGE_X+TP_BADGE_W },   // バッジ画面ボタン
    { TP_LOUPE_Y,    TP_LOUPE_Y+TP_LOUPE_H,      TP_LOUPE_X,    TP_LOUPE_X+TP_LOUPE_W },   // 精密描画ボタン
    { TP_PEN_Y,      TP_PEN_Y+TP_PEN_H,          TP_PEN_X,      TP_PEN_X+TP_PEN_W },       // ペン先ボタン
    { TP_BOOKMARK_Y, TP_BOOKMARK_Y+TP_BOOKMARK_H,TP_BOOKMARK_X, TP_BOOKMARK_X+TP_BOOKMARK_W },    //ブックマークボタン
    { TP_TRTYPE_Y, TP_TRTYPE_Y+TP_TRTYPE_H,TP_TRTYPE_X, TP_TRTYPE_X+TP_TRTYPE_W },                //トレーナータイプ
    { TP_PERSONAL_Y, TP_PERSONAL_Y+TP_PERSONAL_H,TP_PERSONAL_X, TP_PERSONAL_X+TP_PERSONAL_W },    //性格
    { TP_PMS_Y,      TP_PMS_Y+TP_PMS_H,          TP_PMS_X,      TP_PMS_X+TP_PMS_W },              //簡易会話
    { TP_MOVERIGHT_Y,TP_MOVERIGHT_Y+TP_MOVERIGHT_H ,TP_MOVERIGHT_X,TP_MOVERIGHT_X+TP_MOVERIGHT_W},//右へ移動(拡大時）
    { TP_MOVELEFT_Y,TP_MOVELEFT_Y+TP_MOVELEFT_H ,TP_MOVELEFT_X,TP_MOVELEFT_X+TP_MOVELEFT_W},      //左へ移動(拡大時）
    { GFL_UI_TP_HIT_END, 0, 0, 0 },
  };
  static const GFL_UI_TP_HITTBL Btn_TpRectComm[] = {
    { TP_RETURN_Y,   TP_RETURN_Y+TP_RETURN_H,    TP_RETURN_X,   TP_RETURN_X+TP_RETURN_W }, // 戻る
    { TP_RETURN_Y,   TP_RETURN_Y+TP_RETURN_H,    TP_RETURN_X,   TP_RETURN_X+TP_RETURN_W }, // 戻る
    { TP_CARD_Y,     TP_CARD_Y+TP_CARD_H,        TP_CARD_X,     TP_CARD_X+TP_CARD_W },     // カード裏返しボタン
    { GFL_UI_TP_HIT_END,0,0,0}
  };


  // タッチ検出
  if(wk->TrCardData->OtherTrCard==FALSE){
    hitNo = GFL_UI_TP_HitTrg( Btn_TpRect );
  }else{
    hitNo = GFL_UI_TP_HitTrg( Btn_TpRectComm );
  }
  // 拡大状態かどうか
  if(wk->ScaleMode==0){
    ret = normal_touch_func( wk, hitNo );
  }else{
    ret = large_touch_func( wk, hitNo );
  }
  if(ret!=TRC_KEY_REQ_NONE){
    return ret;
  }

  // 裏面用処理
  if(wk->is_back){
    // 拡大してなくてアニメしてない
    if(wk->ScaleMode==0){
      // スクロール・サイン処理
      tp_scroll(wk);
      Stock_TouchPoint( wk, 0 );
    }else{
      // サイン面情報取得処理
      Stock_TouchPoint( wk, 1 );
    }

  }

  return TRC_KEY_REQ_NONE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 入力検出（キー・タッチ）
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int CheckInput(TR_CARD_WORK *wk)
{
  int key_req;
  BOOL tp_key_flg;
  tp_key_flg = FALSE;
  key_req = TRC_KEY_REQ_NONE;

//  wk->touch = RECT_HIT_NONE;

  //キーとタッチの切替監視
  CheckKTStatus(wk);

  //タッチスクロール終了判定
  tp_scroll_end_check(wk);
  //サイン終了判定
  tp_sign_end_check(wk);

  if(wk->key_mode == GFL_APP_KTST_TOUCH){
    return CheckTouch(wk);
  }else{
    return CheckKey(wk);
  }
  return key_req;
}


//----------------------------------------------------------------------------------
/**
 * @brief 入力内容によってシーケンスを分岐させる
 *
 * @param   wk    
 * @param   req   入力結果のリクエスト内容
 */
//----------------------------------------------------------------------------------
static void JumpInputResult( TR_CARD_WORK *wk, int req, int *seq )
{
  switch( req ){
  //カードひっくり返す
  case TRC_KEY_REQ_REV_BUTTON:
    wk->sub_seq = 0;    //サブシーケンス初期化
    *seq = SEQ_REVERSE; //カード回転
    break;
  // 拡大縮小
  case TRC_KEY_REQ_SCALE_BUTTON:
    if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
      *seq = SEQ_SCALING;
    }
    break;
  // 簡易会話呼び出し
  case TRC_KEY_REQ_PMS_CALL:
    if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
      PMSND_PlaySE( SND_TRCARD_PMS );
      wk->sub_seq = 0;
      *seq = SEQ_PMSINPUT;
    }
    break;
  // 通常終了
  case TRC_KEY_REQ_RETURN_BUTTON:
    *seq = SEQ_RETURN_FADEOUT;
    wk->tcp->value = CALL_NONE;
    wk->tcp->next_proc = TRAINERCARD_NEXTPROC_RETURN;
    break;
  // フィールド直接終了
  case TRC_KEY_REQ_END_BUTTON:
    *seq = SEQ_END_FADEOUT;
    wk->tcp->value = CALL_NONE;
    wk->tcp->next_proc = TRAINERCARD_NEXTPROC_EXIT;
    break;
  // トレーナータイプ切り替え
  case TRC_KEY_REQ_TRAINER_TYPE:
    if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
      TR_CARD_SV_PTR trsave = GAMEDATA_GetTrainerCardPtr(wk->tcp->gameData);

      TRCSave_SetTrainerViewChange( trsave );   //変更したフラグを立てる
      PMSND_PlaySE( SND_TRCARD_TRTYPE );
      _add_UnionTrNo( wk->TrCardData );
      TRCBmp_PrintTrainerType( wk, wk->TrCardData->UnionTrNo, 1 );
    }
    break;
  // 性格切り替え
  case TRC_KEY_REQ_PERSONALITY:
    if(wk->tcp->TrCardData->EditPossible){    // 編集可能なら
      PMSND_PlaySE( SND_TRCARD_PERSONALITY );
      if(++wk->TrCardData->Personality>(TRCARD_PERSONARITY_MAX-1)){
        wk->TrCardData->Personality = 0;
      }
      TRCBmp_PrintPersonality( wk, wk->TrCardData->Personality, 1 );
    }
    break;
  // バッジ閲覧画面へ
  case TRC_KEY_REQ_BADGE_CALL:
    wk->sub_seq = 0;
    *seq = SEQ_BADGE_VIEW_CALL;
    break;
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
static void VBlankFunc( GFL_TCB *tcb, void *work )
{
  TR_CARD_WORK* wk = (TR_CARD_WORK*)work;
  int scr;

  // スクロールテキスト点滅
  UpdateTextBlink( wk );


  //背景スクロール
  scr = -(wk->scrl_ct/2);
  GFL_BG_SetScroll( TRC_BG_BACK, GFL_BG_SCROLL_X_SET,scr );
  GFL_BG_SetScroll( TRC_BG_BACK, GFL_BG_SCROLL_Y_SET,scr );
  GFL_BG_SetScroll( TRC_BG_BADGE_BACK, GFL_BG_SCROLL_X_SET, scr);
  GFL_BG_SetScroll( TRC_BG_BADGE_BACK, GFL_BG_SCROLL_Y_SET, scr);

  if(wk->aff_req){
    CardRevAffineSet(wk);
  }
  GFL_BG_VBlankFunc();

  GFL_CLACT_SYS_VBlankFunc();


//  OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * 拡縮面リセット
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ResetAffinePlane(void)
{
  MtxFx22 mtx;
    fx32 rScale_x,rScale_y;

  rScale_x = FX_Inv((1 << FX32_SHIFT));
  rScale_y = FX_Inv((1 << FX32_SHIFT));
  mtx._00 = rScale_x;
  mtx._01 = 0;
  mtx._10 = 0;
  mtx._11 = rScale_y;
  SVC_WaitVBlankIntr();          // Waiting the end of VBlank interrup
  G2S_SetBG2Affine(&mtx,          // a matrix for rotation and scaling
            128, 96,      // the center of rotation
            0, 0           // the reference point before rotation and scaling applied
      );

  G2S_SetBG3Affine(&mtx,          // a matrix for rotation and scaling
            128, 96,      // the center of rotation
            0, 0           // the reference point before rotation and scaling applied
      );
  SVC_WaitVBlankIntr();          // Waiting the end of VBlank interrup
  G2_SetBG3Affine(&mtx,          // a matrix for rotation and scaling
            128, 0,      // the center of rotation
            0, 0           // the reference point before rotation and scaling applied
      );
}

#define TRAINER_SCREEN_PASTE_X  ( 21 )
#define TRAINER_SCREEN_PASTE_Y  (  2 )
#define TRAINER_SCREEN_PASTE_W  ( 10 )
#define TRAINER_SCREEN_PASTE_H  ( 11 )
#define TRAINER_SCREEN_SRC_X    ( 21 )
#define TRAINER_SCREEN_SRC_Y    (  2 )

//--------------------------------------------------------------------------------------------
/**
 * トレーナースクリーン転送
 *
 * @param wk      画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void DispTrainer(TR_CARD_WORK *wk)
{
  u32 transSize;

  transSize = wk->ScrnData->szByte;

  //TRAINER
  GFL_BG_LoadCharacter(TRC_BG_TRAINER, wk->TrCharData->pRawData, wk->TrCharData->szByte, 0);

  GFL_BG_WriteScreenExpand(TRC_BG_TRAINER,
    TRAINER_SCREEN_PASTE_X,TRAINER_SCREEN_PASTE_Y,
    TRAINER_SCREEN_PASTE_W,TRAINER_SCREEN_PASTE_H,
    wk->ScrnData->rawData,
    TRAINER_SCREEN_SRC_X,
    TRAINER_SCREEN_SRC_Y,
    wk->ScrnData->screenWidth/8,wk->ScrnData->screenHeight/8);
  GFL_BG_LoadScreenReq( TRC_BG_TRAINER );

}

//--------------------------------------------------------------------------------------------
/**
 * トレーナースクリーンクリア
 *
 * @param wk      画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ClearTrainer(TR_CARD_WORK *wk)
{
  GFL_BG_FillScreen(TRC_BG_TRAINER, 0, 20, 6, 6, 9, GFL_BG_SCRWRT_PALNL);
}


//--------------------------------------------------------------------------------------------
/**
 * サインデータ展開（bitデータをBMP256データに展開）
 *
 * @param *inRawData  ビット単位で持っているサインデータ
 * @param *outData  サインデータ格納場所
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void DecodeSignData( const u8 *inRawData, u8 *outData )
{

  int dot,raw_dot;
  u8 raw_line;  //0～7
  u8 shift;   //0～7
  for(dot=0;dot<SIGN_SIZE_X*SIGN_SIZE_Y*64;dot++){
    raw_dot = dot/64;
    raw_line = (dot/8)%8;
    shift = (dot%8);
    outData[dot] = ( 0x01 & (inRawData[(raw_dot*8)+raw_line]>>shift) );
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief サインデータ圧縮（BMP256データを1bitデータに圧縮）
 *
 * @param   inBmpData   
 * @param   outData   
 */
//----------------------------------------------------------------------------------
static void EncodeSignData( const u8 *inBmpData, u8 *outData )
{
  int i,r,tmp;

  // 1bit単位で格納するので1/8のループですむはず
  for(i=0;i<SIGN_SIZE_X*SIGN_SIZE_Y*8;i++){
    tmp = 0;
    for(r=0;r<8;r++){
      tmp |= (inBmpData[i*8+r]<<r);
    }
    outData[i] = tmp;
  }
}

#define SIGN_HALF_SIZE    ( SIGN_SX/2 )

//----------------------------------------------------------------------------------
/**
 * @brief サイン画面のスクリーンデータ加工・転送を行う
 *
 * @param   inFrame   指定スクリーン
 * @param   flag      SCREEN_SIGN_ALL: SCREEN_SIGN_LEFT:  SCREEN_SIGN_RIGHT
 */
//----------------------------------------------------------------------------------
static void Trans_SignScreen( const int inFrame, const int flag )
{
  u16 *buf;
  u8 x,y;
  u16 count;

  // スクリーンデータの加工
  buf   = (u16 *)GFL_BG_GetScreenBufferAdrs( inFrame );
  
  // 全クリア 
  for( y=0; y<SIGN_SY; y++ ){
    for( x=0; x<SIGN_SX; x++ ){
      buf[ (SIGN_PY+y)*TRC_SCREEN_WIDTH + x+SIGN_PX ]=0;
    }
  }

  // モードにあわせてスクリーンを加工  
  count = 0;
  switch(flag){
  case SCREEN_SIGN_ALL:   // 全面
    for( y=0; y<SIGN_SY; y++ ){
      for( x=0; x<SIGN_SX; x++ ){
        buf[ (SIGN_PY+y)*TRC_SCREEN_WIDTH + x+SIGN_PX ] = (SIGN_CGX+count);
        count++;
      }
    }
    break;
  case SCREEN_SIGN_LEFT:  // 左半分
    for( y=0; y<SIGN_ANIME_SY; y++ ){
      for( x=0; x<SIGN_ANIME_SX; x++ ){
        buf[ (SIGN_ANIME_PY+y)*TRC_SCREEN_WIDTH + x+SIGN_ANIME_PX ] = (SIGN_CGX+count);
        count++;
      }
      count+=SIGN_HALF_SIZE;
    }
    break;
  case SCREEN_SIGN_RIGHT: // 右半分
    count+=SIGN_HALF_SIZE;
    for( y=0; y<SIGN_ANIME_SY; y++ ){
      for( x=0; x<SIGN_ANIME_SX; x++ ){
        buf[ (SIGN_ANIME_PY+y)*TRC_SCREEN_WIDTH + x+SIGN_ANIME_PX ] = (SIGN_CGX+count);
        count++;
      }
      count+=SIGN_HALF_SIZE;
    }
    break;
  }
  
  // スクリーン転送
  GFL_BG_LoadScreenV_Req( inFrame );

}

//--------------------------------------------------------------------------------------------
/**
 * サインデータ転送
 *
 * @param bgl       bgl
 * @param inFrame     対象フレーム
 * @param inSignData    サインデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void TransSignData(const int inFrame, const u8 *inSignData, const int flag)
{

  // サイングラフィックデータをVRAMに転送
  GFL_BG_LoadCharacter( inFrame, inSignData, SIGN_BYTE_SIZE, SIGN_CGX );

  // スクリーンデータの加工・転送を行う
  Trans_SignScreen( inFrame, flag );
}


#define SIGN_ANIME_SWITCH_FRAME   ( 20 )


//----------------------------------------------------------------------------------
/**
 * @brief サイン画面アニメの再生処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void UpdateSignAnime( TR_CARD_WORK *wk )
{
  // 裏面でアニメフラグが立っているならサインアニメを行う
  if(wk->is_back && wk->TrCardData->SignAnimeOn ){
    wk->SignAnimeWait++;
    if(wk->SignAnimeWait>SIGN_ANIME_SWITCH_FRAME){
      wk->SignAnimePat^=1;
      Trans_SignScreen( TRC_BG_SIGN, wk->SignAnimePat );
      wk->SignAnimeWait = 0;
    }
  }
}

#define BLINK_COUNT_MAX       ( 8 )  ///< 点滅カウンタの最大値
#define BLINK_PAL_DES_OFFSET  (  3 )  ///< 転送先オフセット
#define BLINK_PAL_SRC_OFFSET  (  3 )  ///< 転送元オフセット

//----------------------------------------------------------------------------------
/**
 * @brief レコードデータスクロール時パレットの点滅処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void UpdateTextBlink( TR_CARD_WORK *wk )
{
  u16 *pal = wk->pPalData->pRawData;
  if(++wk->blink_count > BLINK_COUNT_MAX){
    wk->blink_count = 0;
  }
  if(wk->blink_count==0){
    GXS_LoadBGPltt( &pal[BLINK_PAL_SRC_OFFSET], BLINK_PAL_DES_OFFSET*2, 4 );
  }else if(wk->blink_count==(BLINK_COUNT_MAX/2)){
    GXS_LoadBGPltt( &pal[BLINK_PAL_SRC_OFFSET+2], BLINK_PAL_DES_OFFSET*2, 4 );
  }
}


//---------------------------------------------------------
// ブラシパターン
//---------------------------------------------------------

// どうしてもパレットデータを3bitに縮めてしまいたいので、透明色を8にして
// 色指定自体は0-7に当てている。0は透明色だがBD面を黒にして黒く見えるようにしている

// BMPデータは最低横8dot分必要なので、4x4のドットデータを作りたい時は
// ２バイトごとに参照されないデータがもう２バイト必要
static const u8 sign_brush[2][2][16]={
  {
    {
      0,1,1,0, 
      1,1,1,1,  
      1,1,1,1,  
      0,1,1,0,  
    },
    {
      0,2,2,0, 
      2,2,2,2, 
      2,2,2,2,
      0,2,2,0,  
    },
  },
  {
    {
      0,0,0,0, 
      0,1,1,0,  
      0,1,1,0,  
      0,0,0,0,  
    },
    {
      0,0,0,0, 
      0,2,2,0, 
      0,2,2,0,
      0,0,0,0,  
    },
  },    
};                                                                    

// おえかきボードBMP（下画面）
#define OEKAKI_BOARD_POSX  (  4 )
#define OEKAKI_BOARD_POSY  ( 11 )
#define OEKAKI_BOARD_W     ( 24 )
#define OEKAKI_BOARD_H     (  8 )

#define LARGE_BOARD_POSX  (  4 )
#define LARGE_BOARD_POSY  (  3 )
#define LARGE_BOARD_W     ( 24 )
#define LARGE_BOARD_H     ( 16 )

static const GFL_UI_TP_HITTBL sign_tbl[]={
  {OEKAKI_BOARD_POSY*8,(OEKAKI_BOARD_POSY+OEKAKI_BOARD_H)*8, OEKAKI_BOARD_POSX*8, (OEKAKI_BOARD_POSX+OEKAKI_BOARD_W)*8+1},
  {GFL_UI_TP_HIT_END,0,0,0},
};

static const GFL_UI_TP_HITTBL large_sign_tbl[]={
  {LARGE_BOARD_POSY*8,(LARGE_BOARD_POSY+LARGE_BOARD_H)*8, LARGE_BOARD_POSX*8, (LARGE_BOARD_POSX+LARGE_BOARD_W)*8},
  {GFL_UI_TP_HIT_END,0,0,0},
};

// ↓サイン終了判定用 端を塗りつぶす時の為に一回り大きい
static const GFL_UI_TP_HITTBL sign_release_tbl[]={
  {OEKAKI_BOARD_POSY*8-16,(OEKAKI_BOARD_POSY+OEKAKI_BOARD_H)*8+16, OEKAKI_BOARD_POSX*8-16, (OEKAKI_BOARD_POSX+OEKAKI_BOARD_W)*8+16},
  {GFL_UI_TP_HIT_END,0,0,0},
};

static const GFL_UI_TP_HITTBL large_sign_release_tbl[]={
  {LARGE_BOARD_POSY*8-16,(LARGE_BOARD_POSY+LARGE_BOARD_H)*8+16, LARGE_BOARD_POSX*8-16, (LARGE_BOARD_POSX+LARGE_BOARD_W)*8+16},
  {GFL_UI_TP_HIT_END,0,0,0},
};

//-----------------------------------------------------------------------------
/**
 *  @brief  サイン終了判定 範囲外に出たらまた新たにタッチしなおさないと書けない
 *
 *  @param  TR_CARD_WORK *wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void tp_sign_end_check( TR_CARD_WORK *wk )
{
  if( ( wk->is_back ) &&
      ( wk->ScaleMode == 0 && GFL_UI_TP_HitCont( sign_release_tbl ) != 0 ) ||
      ( wk->ScaleMode == 1 && GFL_UI_TP_HitCont( large_sign_release_tbl ) != 0 ) )
  {
      wk->b_touch_sign = FALSE;
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief サイン面のタッチパネル情報取得
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void Stock_TouchPoint( TR_CARD_WORK *wk, int scale_mode )
{
  //  サンプリング情報を取得して格納
  u32 x,y;

  // サイン書き換え不可(編集×かサインアニメ中）だったら終了
  if(wk->TrCardData->SignDisenable || wk->TrCardData->SignAnimeOn==1){
    return;
  }

  if(GFL_UI_TP_GetPointCont( &x, &y )){

    // 通常
    if(scale_mode==0){
      if( GFL_UI_TP_HitTrg( sign_tbl ) == 0 ){
        wk->b_touch_sign = TRUE;
      }
      
      if( wk->b_touch_sign && GFL_UI_TP_HitSelf( sign_tbl, x, y )==0){
        wk->MyTouchResult.x = x;
        wk->MyTouchResult.y = y;
        wk->MyTouchResult.brush = wk->pen;
        wk->MyTouchResult.on    = 1;
  
        // 拡大する際の左右を決める
        if(x<128){
          wk->ScaleSide=0;
        }else{
          wk->ScaleSide=1;
        }
      }else{
        wk->MyTouchResult.on    = 0;
      }

    // 精密
    }else{
      if( GFL_UI_TP_HitTrg( large_sign_tbl ) == 0 ){
        wk->b_touch_sign = TRUE;
      }

      if( wk->b_touch_sign && GFL_UI_TP_HitSelf( large_sign_tbl, x, y )==0){
        wk->MyTouchResult.x = OEKAKI_BOARD_POSX*8+(x-LARGE_BOARD_POSX*8+wk->ScaleSide*(24*8))/2;
        wk->MyTouchResult.y = OEKAKI_BOARD_POSY*8+(y-LARGE_BOARD_POSY*8)/2;
        wk->MyTouchResult.brush = wk->pen;
        wk->MyTouchResult.on    = 1;
      }else{
        wk->MyTouchResult.on    = 0;
      }
    }
  }else{
    wk->MyTouchResult.on    = 0;
  }

  wk->AllTouchResult = wk->MyTouchResult;

}

//----------------------------------------------------------------------------------
/**
 * @brief 256色のBMPWINに直接点を書き込むルーチン
 *
 * @param   sign    サイン面の画像配列
 * @param   brush   ブラシデータ
 * @param   x   サイン面に書き込む点X
 * @param   y   サイン面に書き込む点Y
 * @param   ww  サイン面の横幅
 * @param   hh  サイン面の縦幅
 */
//----------------------------------------------------------------------------------
static void draw_pen( u8 *sign, u8* brush, int x, int y, int ww, int hh, int scale_mode, int scale_side )
{
  int bx,by,px,py,wx,wy;
  int i,offset,w,h,count=0;
  
  // 書きこむのは4x4に限定
  for(h=0;h<4;h++){
    wy = y+h;
    by = wy / 8;
    py = wy % 8;
    for(w=0;w<4;w++){
      wx = x+w;
      if(wx<0 || wy<0){
//        HOSAKA_Printf("count=%d wx=%d wy=%d \n",count,wx,wy);
        count++;
        continue;
      }
      if(wx >= ww || wy >= hh){
        continue;
      }
      // 拡大時、中央への書き込みが境界を越えて反対側に書いてしまうので規定値で絞込み。
      if( scale_mode == 1 )
      {
        if( ( scale_side == 0 && wx >= SCALE_SIDE_LEFT_MAX_X ) ||
            ( scale_side == 1 && wx <= SCALE_SIDE_RIGHT_MIN_X ) )
        {
          count++;
          continue;
        }
      }

      bx = wx / 8;
      px = wx % 8;
      offset = (by*OEKAKI_BOARD_W+bx)*64 + py*8+px;
//      HOSAKA_Printf("w=%d h=%d bx=%d by=%d px=%d py=%d wx=%d wy=%d offset=%d\n", w,h,bx,by,px,py,wx,wy,offset );
      if(brush[count]==1){
        sign[offset] = 1;
      }else if(brush[count]==2){
        sign[offset] = 0;
      }
#if 0
      else if( ( bx == 0 && px == 0 ) || ( by == 0 && py == 0 ) ){
        // 端ならブラシの中央で塗る
        if(brush[5]==1){
          sign[offset] = 1;
        }else if(brush[5]==2){
          sign[offset] = 0;
        }
      }
#endif
      count++;
    }
  }
}

//==============================================================================
/**
 * @brief   描画開始位置がマイナス方向にあっても描画できるBmpWinPrintラッパー
 * @retval  none    
 */
//==============================================================================
#if 0
static void _BmpWinPrint_Rap(
      GFL_BMPWIN * win, void * src,
      int src_x, int src_y, int src_dx, int src_dy,
      int win_x, int win_y, int win_dx, int win_dy )
{

  if(win_x < 0){
    int diff;
    diff = win_x*-1;
    if(diff>win_dx){
      diff = win_dx;
    }
    win_x   = 0;
    src_x  += diff;
    src_dx -= diff;
    win_dx -= diff;
  }

  if(win_y < 0){
    int diff;
    diff = win_y*-1;
    if(diff>win_dy){
      diff = win_dy;
    }
    win_y   = 0;
    src_y  += diff;
    src_dy -= diff;
    win_dy -= diff;
  }

//  GF_BGL_BmpWinPrint( win, src, src_x, src_y, src_dx, src_dy, win_x, win_y, win_dx, win_dy );
/*
  GFL_BMP_Print( src, win, src_x, src_y, src_dx, src_dy, win_x, win_y, 0 );
*/
  {
    u8 *sign = (u8*)win;
    draw_pen( sign, src, win_x, win_y, OEKAKI_BOARD_W*8, OEKAKI_BOARD_H*8 );
  }
}
#endif


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
 * @retval  lines 
 */
//------------------------------------------------------------------
static int DrawPoint_to_Line( 
  GFL_BMPWIN *win, 
  const u8 *brush, 
  int px, int py, int *sx, int *sy, 
  int count, int flag, int scale_mode, int scale_side )
{
  int dx, dy, s, step;
  int x1 = *sx;
  int y1 = *sy;
  int x2 = px;
  int y2 = py;
  int lines = 0;

  // 初回は原点保存のみ
  if(count==0 && flag == 0){
    *sx = px;   *sy = py;
    return lines;
  }
  

  dx = MATH_IAbs(x2 - x1);  dy = MATH_IAbs(y2 - y1);
  if (dx > dy) {
    if (x1 > x2) {
      step = (y1 > y2) ? 1 : -1;
      s = x1;  x1 = x2;  x2 = s;  y1 = y2;
    } else step = (y1 < y2) ? 1: -1;

    draw_pen( (u8*)win, (void*)brush, x1, y1, OEKAKI_BOARD_W*8, OEKAKI_BOARD_H*8, scale_mode, scale_side );
    //_BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
    lines++;
    s = dx >> 1;
    while (++x1 <= x2) {
      if ((s -= dy) < 0) {
        s += dx;  y1 += step;
      };
      draw_pen( (u8*)win, (void*)brush, x1, y1, OEKAKI_BOARD_W*8, OEKAKI_BOARD_H*8, scale_mode, scale_side );
      //_BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
      lines++;
    }
  } else {
    if (y1 > y2) {
      step = (x1 > x2) ? 1 : -1;
      s = y1;  y1 = y2;  y2 = s;  x1 = x2;
    } else step = (x1 < x2) ? 1 : -1;
    draw_pen( (u8*)win, (void*)brush, x1, y1, OEKAKI_BOARD_W*8, OEKAKI_BOARD_H*8, scale_mode, scale_side );
    //_BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
    lines++;
    s = dy >> 1;
    while (++y1 <= y2) {
      if ((s -= dx) < 0) {
        s += dy;  x1 += step;
      }
      draw_pen( (u8*)win, (void*)brush, x1, y1, OEKAKI_BOARD_W*8, OEKAKI_BOARD_H*8, scale_mode, scale_side );
      //_BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
      lines++;
    }
  }
  
  
  *sx = px;     *sy = py;

  return lines;
}

static void Stock_OldTouch( TOUCH_INFO *all, TOUCH_INFO *stock )
{
  *stock = *all;

/*
  int i;
  stock[i].size = all[i].size;
  if(all[i].size!=0){
    stock[i].x = all[i].x[all[i].size-1];
    stock[i].y = all[i].y[all[i].size-1];
  }
*/
}



//----------------------------------------------------------------------------------
/**
 * @brief 取得したしたタッチパネルの結果データを下に描画する
 *
 * @param   win   書き込むBMPWIN
 * @param   all   タッチ頂点格納配列
 * @param   old   1sync前のタッチ頂点格納配列
 * @param   draw    メモリ上で行ったCGX変更を転送するか？(0:しない  1:する）
 * @param   SignData    メモリ上のサインデータ
 * @param   sign_mode   拡大モードかどうか（0:通常  1:拡大モード）
 * @param   scale_side  拡大モード時の左右モード（0:左　1:右）
 *
 * @retval  line        ドットをうった回数
 */
//----------------------------------------------------------------------------------
static  int DrawBrushLine( GFL_BMPWIN *win, TOUCH_INFO *all, TOUCH_INFO *old, int draw, u8 *SignData, u8 sign_mode, u8 scale_side )
{
  int px,py,i,r,flag=0, sx, sy, centerling;
  int line = 0;

  //OS_Printf("id0=%d,id1=%d,id2=%d,id3=%d,id4=%d\n",all[0].size,all[1].size,all[2].size,all[3].size,all[4].size);
  if(sign_mode==0){
    centerling = 4;
  }else{
    centerling = 2;
  }

  if(all->on!=0){
    if(old->on){
      sx = old->x-OEKAKI_BOARD_POSX*8-centerling;
      sy = old->y-OEKAKI_BOARD_POSY*8-2;
    }
    px = all->x - OEKAKI_BOARD_POSX*8-centerling;
    py = all->y - OEKAKI_BOARD_POSY*8-2;
      
//    OS_Printf("sx=%d, sy=%d, px=%d, py=%d\n", sx,sy,px,py);

    // BG1面用BMP（お絵かき画像）ウインドウ確保
    line = DrawPoint_to_Line(win, sign_brush[sign_mode][all->brush], px, py, &sx, &sy, 0, old->on, sign_mode, scale_side );
    
    flag = 1;
    
  }
  if(flag && draw){

    // サイングラフィックデータをVRAMに転送
    GFL_BG_LoadCharacter( TRC_BG_SIGN, SignData, SIGN_BYTE_SIZE, SIGN_CGX );
    
    //OS_Printf("write board %d times\n",debug_count++);
    
  }
  
  // 今回の最終座標のバックアップを取る   
    Stock_OldTouch(all, old);
//  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    all->on = 0;    // 一度描画したら座標情報は捨てる
//  }
  
  return line;
}

