//============================================================================================
/**
 * @file  trainer_card.c
 * @bfief トレーナーカード
 * @author  Nozomu Saito
 * @date  05.11.15
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
#include "test/ariizumi/ari_debug.h"

// キーでカードがめくれないようにする
#define KEY_LR_OK


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
  SEQ_OUT,
  SEQ_REVERSE,
  SEQ_SIGN_CALL,
  SEQ_SIGN,
  SEQ_COVER,
  SEQ_SCALING,
};

typedef enum {
  TRC_KEY_REQ_NONE = 0,
  TRC_KEY_REQ_TP_BEAT,
  TRC_KEY_REQ_TP_BRUSH,
  TRC_KEY_REQ_REV_BUTTON,
  TRC_KEY_REQ_SIGN_CALL,
  TRC_KEY_REQ_END_BUTTON,
  TRC_KEY_REQ_TRAINER_TYPE,
  TRC_KEY_REQ_PERSONALITY,
  TRC_KEY_REQ_SCALE_BUTTON,
  TRC_KEY_REQ_PMS_CALL,
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

//============================================
// 結構広いタッチ範囲定義
//============================================

// スコアスクロール領域
#define TP_SCORE_SCROL_X  (   24 )
#define TP_SCORE_SCROL_Y  (   16 )
#define TP_SCORE_SCROL_W  (  208 )
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


//ユニオンルームトレーナー表示テーブル
static const int UniTrTable[UNION_TR_MAX] =
{
  //男
  NARC_trc_union_trdp_schoolb256_NCGR,
  NARC_trc_union_trdp_mushi256_NCGR,
  NARC_trc_union_trdp_elitem256_NCGR,
  NARC_trc_union_trdp_heads256_NCGR,
  NARC_trc_union_trdp_iseki256_NCGR,
  NARC_trc_union_trdp_karate256_NCGR,
  NARC_trc_union_trdp_prince256_NCGR,
  NARC_trc_union_trdp_espm256_NCGR,
  //女
  NARC_trc_union_trdp_mini256_NCGR,
  NARC_trc_union_trdp_battleg256_NCGR,
  NARC_trc_union_trdp_sister256_NCGR,
  NARC_trc_union_trdp_elitew256_NCGR,
  NARC_trc_union_trdp_idol256_NCGR,
  NARC_trc_union_trdp_madam256_NCGR,
  NARC_trc_union_trdp_cowgirl256_NCGR,
  NARC_trc_union_trdp_princess256_NCGR,
};

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
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
static void SetCardPalette(TR_CARD_WORK *wk ,u8 inCardRank, const u8 inPokeBookHold);
static void SetCasePalette(TR_CARD_WORK *wk ,const u8 inVersion);
static void SetUniTrainerPalette(TR_CARD_WORK *wk ,const u8 inTrainerNo);

static void VBlankFunc( GFL_TCB *tcb, void *work );
static void DispTrainer(TR_CARD_WORK *wk);
static void ClearTrainer(TR_CARD_WORK *wk);

static void ResetAffinePlane(void);
static const u8 GetBadgeLevel(const int inCount);

static void DecodeSignData(const u8 *inRawData, u8 *outData);
static void EncodeSignData( const u8 *inBmpData, u8 *outData );

static void TransSignData(const int inFrame, const u8 *inSignData, int flag);
static void UpdatePlayTime(TR_CARD_WORK *wk, const u8 inUpdateFlg);
static void DispTouchBarObj( TR_CARD_WORK *wk, int is_back );
static void UpdateSignAnime( TR_CARD_WORK *wk );
static void Trans_SignScreen( const int inFrame, const int flag );
static BOOL CardScaling( TR_CARD_WORK *wk );
static void Change_SignAnimeButton( TR_CARD_WORK *wk, int flag, int OnOff );

static int SignCall( TR_CARD_WORK *wk );

static void Stock_TouchPoint( TR_CARD_WORK *wk, int scale_mode );
static void _BmpWinPrint_Rap(     GFL_BMPWIN * win, void * src,
      int src_x, int src_y, int src_dx, int src_dy,
      int win_x, int win_y, int win_dx, int win_dy );
static void DrawPoint_to_Line(  GFL_BMPWIN *win, 
  const u8 *brush, 
  int px, int py, int *sx, int *sy, 
  int count, int flag );
static void Stock_OldTouch( TOUCH_INFO *all, TOUCH_INFO *stock );
static void DrawBrushLine( GFL_BMPWIN *win, TOUCH_INFO *all, TOUCH_INFO *old, int draw, u8 *SignData, u8 sign_mode );
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
  CONFIG* configSave; ///<コンフィグセーブデータ

  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
  WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  WIPE_ResetWndMask(WIPE_DISP_MAIN);
  WIPE_ResetWndMask(WIPE_DISP_SUB);

  GFL_UI_KEY_SetRepeatSpeed( 4, 8 );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TR_CARD, 0x50000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(TR_CARD_WORK), HEAPID_TR_CARD );
  GFL_STD_MemFill( wk, 0, sizeof(TR_CARD_WORK) );

  wk->heapId = HEAPID_TR_CARD;
  wk->ObjWork.heapId = HEAPID_TR_CARD;
  //引継ぎパラメータ取得
  wk->tcp = pwk;
  wk->TrCardData = wk->tcp->TrCardData;
  wk->key_mode = GFL_UI_CheckTouchOrKey();

  configSave = SaveData_GetConfig(SaveControl_GetPointer());
  wk->msg_spd = MSGSPEED_GetWait();
  wk->win_type = CONFIG_GetWindowType(configSave);

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
//  if( CommIsConnect(u16 netID) ){
  if(GFL_NET_GetConnectNum() > 0){
    wk->isComm = TRUE;
  }

  GFL_BG_Init( wk->heapId );

  AllocStrBuf(wk);

  GFL_DISP_SetBank( &vramBank );
  SetTrCardBg();
  SetTrCardBgGraphic( wk );

//  InitTPSystem();           // タッチパネルシステム初期化
//  InitTPNoBuff(4);

  //音関連初期化
//  Snd_BadgeWorkInit( &wk->SndBadgeWork );
//  PMSND_PlaySE( SND_TRCARD_CALL );    //呼び出し音

  InitTRCardCellActor( &wk->ObjWork , &vramBank );

  SetTrCardActor( &wk->ObjWork, wk->badge ,wk->isClear);
  SetTrCardActorSub( &wk->ObjWork);

  //Bmpウィンドウ初期化
  TRCBmp_AddTrCardBmp( wk );

  //コロン描く
  TRCBmp_WriteSec(wk,wk->win[TRC_BMPWIN_PLAY_TIME], TRUE, wk->SecBuf);

  if(wk->tcp->value){
    wk->is_back = FALSE;
  }else{
    wk->is_back = FALSE;    //表面からスタート
  }
  wk->tcp->value = FALSE;
  wk->IsOpen = COVER_CLOSE;     //ケースは閉じた状態からスタート

//  wk->touch = RECT_HIT_NONE;      //タッチパネルは押されていない

  wk->ButtonPushFlg = FALSE;      //ボタン押されてない
  wk->SignAnimeWait    = 0;
  if(wk->TrCardData->SignAnimeOn){  // サインアニメしているか？
    wk->SignAnimePat = SCREEN_SIGN_LEFT;    
  }else{
    wk->SignAnimePat = SCREEN_SIGN_ALL;    
  }
  wk->scrol_point   = 0;
  wk->pen = 0;
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
  // 受信強度アイコンを通信接続中なら表示するし、
  // ユニオンのように通信回路は動作しているが接続はしていない状態なら出さない
  //WirelessIconEasyUnion();

  //BGMをしぼる
  //Snd_BgmFadeOut( BGM_VOL_TR_CASE, BGM_FADE_TR_CASE_TIME );
  //Snd_PlayerSetPlayerVolume( PLAYER_FIELD, SND_PLAYER_VOL_TR_CASE );
  //PMSND_ChangeBGMVolume( ISS_GetNowBgmBit(), SND_PLAYER_VOL_TR_CASE );    // @@TODO ISSでBGMトラック毎に
                                                                            // ボリューム指定しているので、
                                                                            // 許可bitをISSに教えてもらう必要がある

  WIPE_SYS_Start( WIPE_PATTERN_FSAM, WIPE_TYPE_SHUTTERIN_DOWN,
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
      req = CheckInput(wk);

#if 0
      if (wk->TrCardData->BrushValid){
        ClearScruchSnd(&wk->ScruchSnd);
      }
#endif
      if (req == TRC_KEY_REQ_REV_BUTTON){
        //カードひっくり返す
        wk->sub_seq = 0;  //サブシーケンス初期化
        *seq = SEQ_REVERSE; //カード回転
      }else if(req == TRC_KEY_REQ_SCALE_BUTTON){
        *seq = SEQ_SCALING;
      }else if(req == TRC_KEY_REQ_SIGN_CALL){
        SetSActDrawSt(&wk->ObjWork,ACTS_BTN_TURN,ANMS_SIGN_ON,TRUE);
//        SetEffActDrawSt(&wk->ObjWork, ACTS_BTN_TURN ,TRUE);
        wk->sub_seq = 0;
        *seq = SEQ_SIGN_CALL;
      }else if(req == TRC_KEY_REQ_PMS_CALL){
        wk->sub_seq = 0;
        *seq = SEQ_SIGN;
      }else if (req == TRC_KEY_REQ_END_BUTTON){
        //終了
        SetSActDrawSt(&wk->ObjWork,ACTS_BTN_BACK,ANMS_BACK_ON,TRUE);
//        SetEffActDrawSt(&wk->ObjWork, ACTS_BTN_BACK ,TRUE);
        WIPE_SYS_Start( WIPE_PATTERN_FMAS, WIPE_TYPE_SHUTTEROUT_UP,
                WIPE_TYPE_SHUTTEROUT_UP, WIPE_FADE_BLACK,
                WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );
        *seq = SEQ_OUT;
      }else if(req==TRC_KEY_REQ_TRAINER_TYPE){
        if(++wk->trainer_type>7){
          wk->trainer_type = 0;
        }
        TRCBmp_PrintTrainerType( wk, wk->trainer_type, 1 );
        
      }else if(req==TRC_KEY_REQ_PERSONALITY){
        if(++wk->TrCardData->Personality>24){
          wk->TrCardData->Personality = 0;
        }
        TRCBmp_PrintPersonality( wk, wk->TrCardData->Personality, 1 );
      }
      
      UpdatePlayTime(wk, wk->TrCardData->TimeUpdate);
      UpdateSignAnime(wk);
      
      DrawBrushLine( (GFL_BMPWIN*)wk->TrSignData, &wk->AllTouchResult, 
                      &wk->OldTouch, 1, wk->TrSignData, wk->ScaleMode );
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
  case SEQ_SIGN_CALL: //サイン処理を呼び出すか？
    {
      int ret;
      ret = SignCall(wk);
      switch(ret){
      case 1:
        *seq = SEQ_MAIN;
        break;
      case 2:
        *seq = SEQ_SIGN;
        break;
      }
    }
    break;
  case SEQ_SIGN:  //サイン処理を呼ぶ
    WIPE_SYS_Start( WIPE_PATTERN_FMAS, WIPE_TYPE_SHUTTEROUT_UP,
            WIPE_TYPE_SHUTTEROUT_UP, WIPE_FADE_BLACK,
            WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapId );
    wk->tcp->value = TRUE;
    *seq = SEQ_OUT;
    break;
  }
  
  if(++wk->scrl_ct >= 128){
    wk->scrl_ct = 0;
  }
  GFL_CLACT_SYS_Main();
  if( wk->vblankTcblSys != NULL )
    GFL_TCBL_Main( wk->vblankTcblSys );

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

  //サイン圧縮
  EncodeSignData( wk->TrSignData, wk->TrCardData->SignRawData);

  {
    u8 *SignBuf;
    SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(param->gameData);
    // サイン書き出しバッファポインタ取得
    SignBuf = (u8*)TRCSave_GetSignDataPtr(TRCSave_GetSaveDataPtr(sv));
    MI_CpuCopyFast(  wk->TrCardData->SignRawData, SignBuf, SIGN_SIZE_X*SIGN_SIZE_Y*8);
  }

  //使用した拡縮面を元に戻す
  ResetAffinePlane();

  FreeStrBuf( wk );         //文字列解放

  RereaseCellObject(&wk->ObjWork);    //2Dオブジェクト関連領域解放

  GFL_HEAP_FreeMemory( wk->TrArcData ); //トレーナーキャラアーカイブデータ解放

  GFL_HEAP_FreeMemory( wk->pScrnBCase );  //バッジケースクリーン解放
  GFL_HEAP_FreeMemory( wk->TrScrnArcData );//トレーナースクリーン解放

  TRCBmp_ExitTrCardBmpWin( wk );      // BMPウィンドウ開放
  TrCardBgExit();     // BGL削除
//  StopTP();             // タッチパネル終了
  GFL_TCB_DeleteTask( wk->vblankFunc );   // VBlankセット
  GFL_TCBL_Exit( wk->vblankTcblSys );
  //動作モード書き戻し
  GFL_UI_SetTouchOrKey(wk->key_mode);

  GFL_PROC_FreeWork( proc );        // ワーク開放

  GFL_HEAP_DeleteHeap( HEAPID_TR_CARD );

  //BGMを元の音量に戻す
  //Snd_BgmFadeIn( BGM_VOL_MAX, BGM_FADE_TR_CASE_TIME, BGM_FADEIN_START_VOL_NOW );
  //Snd_PlayerSetPlayerVolume( PLAYER_FIELD, SND_PLAYER_DEFAULT_VOL );
  //PMSND_ChangeBGMVolume( ISS_GetNowBgmBit(), SND_PLAYER_DEFAULT_VOL );    // @@TODO ISSでBGMトラック毎に
                                                                            // ボリューム指定しているので、
                                                                            // 許可bitをISSに教えてもらう必要がある
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
 * @param   inPokeBookHold    
 */
//----------------------------------------------------------------------------------
static void SetCardPalette(TR_CARD_WORK *wk ,u8 inCardRank, const u8 inPokeBookHold)
{
  if (inPokeBookHold){
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
      GFL_ARC_UTIL_TransVramPalette( ARCID_TRAINERCARD, card_palette_table[inCardRank][Version],
        PALTYPE_SUB_BG , 0 , 2*16*16 ,wk->heapId );
  } else{
      GFL_ARC_UTIL_TransVramPalette( ARCID_TRAINERCARD, NARC_trainer_case_card_6_NCLR,
        PALTYPE_SUB_BG , 0 , 2*16*16 ,wk->heapId );
   
  }
}

//--------------------------------------------------------------------------------------------
/**
 * ケースパレットセット
 *
 * @param inVersion バージョン
 *
 * @return  なし
 */
//--------------------------------------------------------------------------------------------
static void SetCasePalette(TR_CARD_WORK *wk ,const u8 inVersion)
{
  void *buf;
  NNSG2dPaletteData *dat;
/*
  switch(inVersion){
  case VERSION_DIAMOND:   //ダイヤ
    buf = GFL_ARC_UTIL_LoadPalette(
        ARCID_TRAINERCARD, NARC_trainer_case_card_case_d_NCLR, &dat, wk->heapId );
    break;
  case VERSION_PEARL:   //パール
    buf = GFL_ARC_UTIL_LoadPalette(
        ARCID_TRAINERCARD, NARC_trainer_case_card_case_p_NCLR, &dat, wk->heapId );
    break;
  case VERSION_GOLD:
  case VERSION_SILVER:
    buf = GFL_ARC_UTIL_LoadPalette(
        ARCID_TRAINERCARD, NARC_trainer_case_card_case_g_NCLR, &dat, wk->heapId );
    break;
  case VERSION_PLATINUM:
  default:  //別バージョン
    buf = GFL_ARC_UTIL_LoadPalette(
        ARCID_TRAINERCARD, NARC_trainer_case_card_case_x_NCLR, &dat, wk->heapId );
    break;
  }
*/
  //とりあえずGOLDの動作に統一
#if 0
  buf = GFL_ARC_UTIL_LoadPalette(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_g_NCLR, &dat, wk->heapId );


  DC_FlushRange( dat->pRawData, 2*16 );
  GX_LoadBGPltt( dat->pRawData, CASE_BD_PAL*32, 2*16 );
  GXS_LoadBGPltt( dat->pRawData, CASE_BD_PAL*32, 2*16 );
  GFL_HEAP_FreeMemory(buf);
 #endif
 
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
  void *buf;
  u8 *addr;
  NNSG2dPaletteData *dat;
  buf = GFL_ARC_UTIL_LoadPalette(
      ARCID_TRC_UNION, NARC_trc_union_trdp_union_card_NCLR, &dat, wk->heapId );

  addr = (u8*)(dat->pRawData);

  DC_FlushRange(&addr[2*16*inTrainerNo] , 2*16 );
  GXS_LoadBGPltt( &addr[2*16*inTrainerNo], 2*16*UNI_TRAINER_PLTT_NO, 2*16 );
  GFL_HEAP_FreeMemory(buf);
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
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( TRC_BG_BADGE_BACK, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( TRC_BG_BADGE_BACK );
    GFL_BG_SetVisible( TRC_BG_BADGE_BACK, VISIBLE_OFF );
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
  SetCardPalette(wk,wk->TrCardData->CardRank, wk->TrCardData->PokeBookFlg);

  // UNSER_CASE_COVER PALETTE(UNDER_DISPLAY)
  {
    GFL_ARC_UTIL_TransVramPalette( ARCID_TRAINERCARD, NARC_trainer_case_card_case_g_NCLR,
          PALTYPE_MAIN_BG , 0 , 16*2*16 ,wk->heapId );
  }
  // CASE PALETTE
  SetCasePalette(wk,wk->TrCardData->Version);

  //TRAINER
  if (wk->TrCardData->UnionTrNo == UNION_TR_NONE){
    {
      wk->TrArcData = GFL_ARC_UTIL_LoadBGCharacter( ARCID_TRAINERCARD, NARC_trainer_case_card_trainer_NCGR,
                    FALSE, &wk->TrCharData, wk->heapId);
    }

    if (wk->TrCardData->TrSex == PM_MALE){
      //男
      wk->TrScrnArcData = GFL_ARC_UTIL_LoadScreen(ARCID_TRAINERCARD, NARC_trainer_case_card_trainer01_NSCR,
                          0, &wk->ScrnData, wk->heapId);
    }else{
      //女
      wk->TrScrnArcData = GFL_ARC_UTIL_LoadScreen(ARCID_TRAINERCARD, NARC_trainer_case_card_trainer02_NSCR,
                          0, &wk->ScrnData, wk->heapId);
    }
  }else{
    //ユニオンルームで他の人のデータを見る時
    {
      wk->TrArcData = GFL_ARC_UTIL_LoadBGCharacter( ARCID_TRC_UNION, UniTrTable[wk->TrCardData->UnionTrNo],
                    FALSE, &wk->TrCharData, wk->heapId);

      wk->TrScrnArcData = GFL_ARC_UTIL_LoadScreen(ARCID_TRC_UNION, NARC_trc_union_card_test256_NSCR,
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
  //CASE
  GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_NCGR, TRC_BG_BACK, 0, 0, 0, wk->heapId );
  GFL_ARC_UTIL_TransVramScreen(
      ARCID_TRAINERCARD, NARC_trainer_case_card_case_NSCR, TRC_BG_BACK, 0, 0, 0, wk->heapId );

  //BADGE_CASE & BADGE_BACK
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TRAINERCARD, NARC_trainer_case_card_case2_NCGR,
    TRC_BG_BADGE_CASE, 0, 0, 0, wk->heapId );

  //スクリーンデータ取得
  wk->pScrnBCase = GFL_ARC_UTIL_LoadScreen(ARCID_TRAINERCARD, NARC_trainer_case_card_case4_NSCR,
              0, &wk->pSBCase, wk->heapId);

  if(wk->isClear){
    int i;
    GFL_ARC_UTIL_TransVramScreen( ARCID_TRAINERCARD, NARC_trainer_case_card_case3_NSCR,
      TRC_BG_BADGE_CASE, 0, 0, 0, wk->heapId );

    for(i = 0;i < 8;i++){
      if(wk->badge[i+8] == 0){
        GFL_BG_WriteScreenExpand(TRC_BG_BADGE_CASE,
          LEADER_ICON_X+(LEADER_ICON_OX*(i%4)),LEADER_ICON_KY+(LEADER_ICON_OY*(i/4)),
          LEADER_ICON_SX,LEADER_ICON_SY,wk->pSBCase->rawData,0,LEADER_ICON_PY,
          wk->pSBCase->screenWidth/8,wk->pSBCase->screenHeight/8);
      }
    }
  }else{
    int i;
    GFL_ARC_UTIL_TransVramScreen( ARCID_TRAINERCARD, NARC_trainer_case_card_case2_NSCR,
      TRC_BG_BADGE_CASE, 0, 0, 0, wk->heapId );

    for(i = 0;i < 8;i++){
      if(wk->badge[i] == 0){
        GFL_BG_WriteScreenExpand(TRC_BG_BADGE_CASE,
          LEADER_ICON_X+(LEADER_ICON_OX*(i%4)),LEADER_ICON_JY+(LEADER_ICON_OY*(i/4)),
          LEADER_ICON_SX,LEADER_ICON_SY,wk->pSBCase->rawData,0,LEADER_ICON_PY,
          wk->pSBCase->screenWidth/8,wk->pSBCase->screenHeight/8);
      }
    }
  }
  GFL_ARC_UTIL_TransVramScreen( ARCID_TRAINERCARD, NARC_trainer_case_card_case2bg_NSCR,
    TRC_BG_BADGE_BACK, 0, 0, 0, wk->heapId );

  //殿堂入りマーク表示
  if(!wk->isClear && wk->TrCardData->BadgeFlag >= 0x00FF){
    GFL_BG_WriteScreenExpand(TRC_BG_BADGE_CASE,
      0,7,7,9,wk->pSBCase->rawData,0,0,
      wk->pSBCase->screenWidth/8,wk->pSBCase->screenHeight/8);
  }else if(wk->TrCardData->BadgeFlag == 0xFFFF){
    GFL_BG_WriteScreenExpand(TRC_BG_BADGE_CASE,
      0,14,7,9,wk->pSBCase->rawData,7,0,
      wk->pSBCase->screenWidth/8,wk->pSBCase->screenHeight/8);
  }
  GFL_BG_LoadScreenReq(TRC_BG_BADGE_CASE);
  //サイン展開
  DecodeSignData(wk->TrCardData->SignRawData, wk->TrSignData);

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

  if (wk->is_back == FALSE){  //表
    //トレーナー表示
    DispTrainer(wk);
  }else{  //裏面表示
    TransSignData(TRC_BG_SIGN, wk->TrSignData, wk->SignAnimePat);
  }
  
  // タッチバーOBJ周りの再描画
  DispTouchBarObj( wk, wk->is_back );

  TRCBmp_WriteTrWinInfo(wk, wk->win, wk->TrCardData );
  TRCBmp_WriteTrWinInfoRev(wk, wk->win, wk->TrCardData );
  TRCBmp_TransTrWinInfo(wk,wk->win);
}


//----------------------------------------------------------------------------------
/**
 * @brief 表裏の切り替わり時にタッチバーのOBJ表示状態を変更する
 *
 * @param   wk    
 * @param   is_back   
 */
//----------------------------------------------------------------------------------
static void DispTouchBarObj( TR_CARD_WORK *wk, int is_back )
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
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_LOUPE, ANMS_LOUPE_L,             TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN,  ANMS_TURN_L,              TRUE);

      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_BLACK_PEN_L+wk->pen*2, TRUE);

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
      SetSActDrawSt(&wk->ObjWork,  ACTS_BTN_PEN,    ANMS_BLACK_PEN_L+wk->pen*2,    TRUE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_TURN,   ANMS_TURN_L,                   FALSE);
      SetSActDrawSt( &wk->ObjWork, ACTS_BTN_CHANGE, ANMS_ANIME_L,                  FALSE);
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 * サインアプリを呼ぶか確認
 *
 * @param wk    画面のワーク
 *
 * @return  BOOL  TRUE:終了 FALSE：処理中
 */
//--------------------------------------------------------------------------------------------
static int SignCall( TR_CARD_WORK *wk )
{
  int ret = 0;

  switch(wk->sub_seq){
  case 0: //サインを書きますか？
    TRCBmp_SignDrawMsgPut(wk,0);
    wk->sub_seq++;
    break;
  case 1:
    //描画終了待ち
//    if( GF_MSG_PrintEndCheck( wk->msgIdx ))
    if( PRINTSYS_PrintStreamGetState( wk->printHandle ) != PRINTSTREAM_STATE_DONE )
    {
      return 0;
    }
    PRINTSYS_PrintStreamDelete( wk->printHandle );
    TRCBmp_SignDrawYesNoCall(wk,0);
    wk->sub_seq++;
    break;
  case 2: //選択待ち
    ret = TRCBmp_SignDrawYesNoWait(wk,0);
    if(ret < 0){
      break;
    }
    if(ret == 0){
      return 1;
    }
    if(wk->TrCardData->MySignValid){
      TRCBmp_SignDrawMsgPut(wk,1);
    }else{
      return 2; //サインを書く
    }
    wk->sub_seq++;
    break;
  case 3:
//    if( GF_MSG_PrintEndCheck( wk->msgIdx ))
    if( PRINTSYS_PrintStreamGetState( wk->printHandle ) != PRINTSTREAM_STATE_DONE )
    {
      return 0;
    }
    PRINTSYS_PrintStreamDelete( wk->printHandle );
    TRCBmp_SignDrawYesNoCall(wk,1);
    wk->sub_seq++;
    break;
  case 4: //元のデータが消えてもいいですか？
    ret = TRCBmp_SignDrawYesNoWait(wk,1);
    switch(ret){
    case 0:
      return 1;
    case 1:
      return 2;
    }
    break;
  }
  return 0;
}

#define ROTATE_SPEED (0x300)    //90度のカウントアップ
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
    }
    break;
  }
  //アフィン変換実行リクエスト
  wk->aff_req = TRUE;
  return rc;
}


#define SCALE_CENTERX   (

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
  case 0:
    if(wk->ScaleSide==0){
      wk->CardCenterX = TRCARD_LEFT_SCALE_CENTER_POSX;
      wk->CardCenterY = TRCARD_SCALE_CENTER_POSY;
    }else{
      wk->CardCenterX = TRCARD_RIGHT_SCALE_CENTER_POSX;
      wk->CardCenterY = TRCARD_SCALE_CENTER_POSY;
    }
    wk->sub_seq = 1;
    break;
  case 1:
    wk->CardScaleX+=0x100;
    wk->CardScaleY+=0x100;
    if(wk->CardScaleX >= 0x2000){
      wk->CardScaleX = 0x2000;
      wk->CardScaleY = 0x2000;
      wk->sub_seq = 2;
    }
    break;
  case 2:
    wk->ScaleMode = 1;
    DispTouchBarObj( wk, wk->is_back ); // タッチバーOBJ周りの再描画
    return TRUE;
    break;

  // 通常画面へ移行
  case 3:
    wk->sub_seq = 4;
    break;
  case 4:
    wk->CardScaleX-=0x100;
    wk->CardScaleY-=0x100;
    if(wk->CardScaleX <= 0x1000){
      wk->CardScaleX = 0x1000;
      wk->CardScaleY = 0x1000;
      wk->sub_seq = 5;
    }
    break;
  case 5:
    wk->CardCenterX = TRCARD_CENTER_POSX;
    wk->CardCenterY = TRCARD_CENTER_POSY;
    wk->ScaleMode = 0;
    DispTouchBarObj( wk, wk->is_back ); // タッチバーOBJ周りの再描画
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
    PMSND_PlaySE( SND_TRCARD_CANCEL );   //終了音
    return TRC_KEY_REQ_END_BUTTON;
  }

#ifdef KEY_LR_OK
  if(keyTrg & (PAD_KEY_LEFT|PAD_KEY_RIGHT))
  {
    return TRC_KEY_REQ_REV_BUTTON;
  }
#endif
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
  if(flag==0){
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_CHANGE, ANMS_ANIME_L, OnOff);
  }else{
    SetSActDrawSt(&wk->ObjWork,ACTS_BTN_CHANGE, ANMS_STOP_L,  OnOff);
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
  switch(hitNo){
  case 0:     // 戻る
    PMSND_PlaySE( SND_TRCARD_DECIDE );
    return TRC_KEY_REQ_END_BUTTON;
    break;
  case 1:     // 終了
    PMSND_PlaySE( SND_TRCARD_END );
    return TRC_KEY_REQ_END_BUTTON;
    break;
  case 2:     // カード裏返しボタン
    GFL_UI_TP_GetPointCont( &wk->tp_x , &wk->tp_y );
    return TRC_KEY_REQ_REV_BUTTON;
    break;
  case 3:     // バッジ画面ボタン・アニメON/OFFボタン
    if(wk->is_back){
      wk->TrCardData->SignAnimeOn ^=1;
      Trans_CardBackScreen(wk, wk->TrCardData->Version);
      Change_SignAnimeButton( wk, wk->TrCardData->SignAnimeOn, TRUE);
      Change_SignAnime( wk, wk->TrCardData->SignAnimeOn );
      OS_Printf("SignAnime = %d\n", wk->TrCardData->SignAnimeOn);
      PMSND_PlaySE( SND_TRCARD_ANIME );
    }
    break;
  case 4:     // 精密描画ボタン
    if(wk->is_back && (!wk->isComm) && wk->TrCardData->SignAnimeOn==0){
//      return TRC_KEY_REQ_SIGN_CALL;
        if(wk->ScaleMode==0){
          wk->sub_seq = 0;
        }else{
          wk->sub_seq = 3;
        }
        PMSND_PlaySE( SND_TRCARD_LOUPE );
        return TRC_KEY_REQ_SCALE_BUTTON;
    }
    break;
  case 5:     // ペン先ボタン
    if(wk->is_back){
      PMSND_PlaySE( SND_TRCARD_PEN );
      wk->pen ^=1;
      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_BLACK_PEN_L+wk->pen*2, TRUE);
      OS_Printf("pen touch\n");
    }
    break;
  case 6:     // ブックマークボタン
    PMSND_PlaySE( SND_TRCARD_BOOKMARK );
    SetBookMark( wk );
    break;
  case 7:     // トレーナータイプ
    if(wk->is_back==0){
      PMSND_PlaySE( SND_TRCARD_TRTYPE );
      return TRC_KEY_REQ_TRAINER_TYPE;
    }
    break;
  case 8:     // 性格
    if(wk->is_back==0){
      PMSND_PlaySE( SND_TRCARD_PERSONALITY );
      return TRC_KEY_REQ_PERSONALITY;
    }
    break;
  case 9:     // 簡易会話
    if(wk->is_back==0){
      PMSND_PlaySE( SND_TRCARD_PMS );
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
    if(wk->is_back && (!wk->isComm)){
      PMSND_PlaySE( SND_TRCARD_ANIME );
      wk->sub_seq = 3;
      return TRC_KEY_REQ_SCALE_BUTTON;
    }
    break;
  case 5:     // ペン先ボタン
    if(wk->is_back){
      PMSND_PlaySE( SND_TRCARD_PEN );
      wk->pen ^=1;
      SetSActDrawSt(&wk->ObjWork,ACTS_BTN_PEN, ANMS_BLACK_PEN_L+wk->pen*2, TRUE);
      OS_Printf("pen touch\n");
    }
    break;
  case 6:     // ブックマークボタン
    PMSND_PlaySE( SND_TRCARD_BOOKMARK );
    SetBookMark( wk );
    break;
  }
  return TRC_KEY_REQ_NONE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void normal_sign_func( TR_CARD_WORK *wk )
{
  // スクロールとサイン簡易版
  static const GFL_UI_TP_HITTBL Scrol_TpRect[] = {
    { TP_SCORE_SCROL_Y, TP_SCORE_SCROL_Y+TP_SCORE_SCROL_H, TP_SCORE_SCROL_X, TP_SCORE_SCROL_X+TP_SCORE_SCROL_W }, // スクロール
    { TP_PAINTS_Y, TP_PAINTS_Y+TP_PAINTS_H, TP_PAINTS_X, TP_PAINTS_X+TP_PAINTS_W }, // サイン面
    {GFL_UI_TP_HIT_END,0,0,0}
  };
    u32 x,y;
    if(GFL_UI_TP_HitCont(Scrol_TpRect)==0){
      if(GFL_UI_TP_GetTrg()){
        GFL_UI_TP_GetPointTrg( &x, &y );
        wk->touch_sy    = y;
        wk->scrol_start = wk->scrol_point;
      }
      if(GFL_UI_TP_GetCont()){
        GFL_UI_TP_GetPointCont( &x, &y );
        wk->scrol_point = wk->scrol_start - (wk->touch_sy-y);
//        OS_Printf("sy=%d,y=%d,start=%d,point=%d\n",wk->touch_sy,y,wk->scrol_start,wk->scrol_point);
      }
      // 範囲外チェック
      if(wk->scrol_point<-(SCORE_LINE_MAX-4)*16){
        wk->scrol_point = -(SCORE_LINE_MAX-4)*16;
      }else if(wk->scrol_point>0){
        wk->scrol_point = 0;
      }
      if(wk->old_scrol_point!=wk->scrol_point){
        TRCBmp_WriteScoreListWin( wk, wk->scrol_point, 1 );
      }
      wk->old_scrol_point=wk->scrol_point;
    }

}

//----------------------------------------------------------------------------------
/**
 * @brief 
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void large_sign_func( TR_CARD_WORK *wk )
{
  // サイン精密版
  static const GFL_UI_TP_HITTBL PaintL_TpRect[] = {
    { TP_PAINTL_Y, TP_PAINTL_Y+TP_PAINTL_H, TP_PAINTL_X, TP_PAINTL_X+TP_PAINTL_W }, // サイン面
    { GFL_UI_TP_HIT_END, 0, 0, 0 },
  };

    u32 x,y;
    if(GFL_UI_TP_HitCont(PaintL_TpRect)==0){
      if(GFL_UI_TP_GetCont()){
        GFL_UI_TP_GetPointCont( &x, &y );
      }
      // 範囲外チェック
      if(wk->scrol_point<-(SCORE_LINE_MAX-4)*16){
        wk->scrol_point = -(SCORE_LINE_MAX-4)*16;
      }else if(wk->scrol_point>0){
        wk->scrol_point = 0;
      }
      if(wk->old_scrol_point!=wk->scrol_point){
        TRCBmp_WriteScoreListWin( wk, wk->scrol_point, 1 );
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
  // タッチバー
  static const GFL_UI_TP_HITTBL Btn_TpRect[] = {
    { TP_RETURN_Y,   TP_RETURN_Y+TP_RETURN_H,    TP_RETURN_X,   TP_RETURN_X+TP_RETURN_W }, // 戻る
    { TP_END_Y,      TP_END_Y+TP_END_H,          TP_END_X,      TP_END_X+TP_END_W },       // 終了
    { TP_CARD_Y,     TP_CARD_Y+TP_CARD_H,        TP_CARD_X,     TP_CARD_X+TP_CARD_W },     // カード裏返しボタン
    { TP_BADGE_Y,    TP_BADGE_Y+TP_BADGE_H,      TP_BADGE_X,    TP_BADGE_X+TP_BADGE_W },   // バッジ画面ボタン
    { TP_LOUPE_Y,    TP_LOUPE_Y+TP_LOUPE_H,      TP_LOUPE_X,    TP_LOUPE_X+TP_LOUPE_W },   // 精密描画ボタン
    { TP_PEN_Y,      TP_PEN_Y+TP_PEN_H,          TP_PEN_X,      TP_PEN_X+TP_PEN_W },       // ペン先ボタン
    { TP_BOOKMARK_Y, TP_BOOKMARK_Y+TP_BOOKMARK_H,TP_BOOKMARK_X, TP_BOOKMARK_X+TP_BOOKMARK_W },   // ブックマークボタン
    { TP_TRTYPE_Y, TP_TRTYPE_Y+TP_TRTYPE_H,TP_TRTYPE_X, TP_TRTYPE_X+TP_TRTYPE_W },          // トレーナータイプ
    { TP_PERSONAL_Y, TP_PERSONAL_Y+TP_PERSONAL_H,TP_PERSONAL_X, TP_PERSONAL_X+TP_PERSONAL_W },   // 性格
    { TP_PMS_Y,      TP_PMS_Y+TP_PMS_H,          TP_PMS_X,      TP_PMS_X+TP_PMS_W },        // 簡易会話
    {GFL_UI_TP_HIT_END,0,0,0}
  };


  // ボタン処理
  const int hitNo = GFL_UI_TP_HitTrg( Btn_TpRect );
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
    if(wk->ScaleMode==0){
      // スクロール・サイン処理
      normal_sign_func(wk);
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

  if(wk->key_mode == GFL_APP_KTST_TOUCH){
    return CheckTouch(wk);
  }else{
    return CheckKey(wk);
  }
  return key_req;
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

  //背景スクロール
  scr = -(wk->scrl_ct);
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
#define TRAINER_SCREEN_PASTE_Y  (  4 )
#define TRAINER_SCREEN_PASTE_W  ( 10 )
#define TRAINER_SCREEN_PASTE_H  ( 11 )
#define TRAINER_SCREEN_SRC_X    ( 21 )
#define TRAINER_SCREEN_SRC_Y    (  7 )

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

#if 0
  if( GFL_BG_ScreenAdrsGet( TRC_BG_TRAINER ) != NULL ){
    GFL_BG_ScreenBufSet( TRC_BG_TRAINER, wk->ScrnData->rawData, transSize );
  }

  GFL_BG_LoadScreen( TRC_BG_TRAINER, wk->ScrnData->rawData, transSize, 0 );
#else
  GFL_BG_WriteScreenExpand(TRC_BG_TRAINER,
    TRAINER_SCREEN_PASTE_X,TRAINER_SCREEN_PASTE_Y,
    TRAINER_SCREEN_PASTE_W,TRAINER_SCREEN_PASTE_H,
    wk->ScrnData->rawData,
    TRAINER_SCREEN_SRC_X,
    TRAINER_SCREEN_SRC_Y,
    wk->ScrnData->screenWidth/8,wk->ScrnData->screenHeight/8);
  GFL_BG_LoadScreenReq( TRC_BG_TRAINER );
#endif

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
 * バッジレベル取得
 *
 * @param inCount     磨き回数
 *
 * @return  u8        レベル
 */
//--------------------------------------------------------------------------------------------
static const u8 GetBadgeLevel(const int inCount)
{
  u8 lv;
  if ((0<=inCount)&&(inCount<COUNT_LV0)){
    lv = 0;
  }else if(inCount<COUNT_LV1){
    lv =1;
  }else if (inCount<COUNT_LV2){
    lv = 2;
  }else if (inCount<COUNT_LV3){
    lv = 3;
  }else if (inCount<COUNT_LV4){
    lv = 4;
  }else{
    GF_ASSERT(0&&"BadgeLevelOver");
    lv = 0;
  }
  return lv;
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

//--------------------------------------------------------------------------------------------
/**
 * プレイ時間更新
 *
 * @param wk        画面のワーク
 * @param inUpdateFlg   時間更新フラグ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void UpdatePlayTime(TR_CARD_WORK *wk, const u8 inUpdateFlg)
{
  //更新フラグがたっているかをチェック
  if (!inUpdateFlg){
    return;
  }

  if (!wk->is_back){  //表面の場合のみ描画
    if (wk->SecCount == 0){
      TRCBmp_WritePlayTime(wk,wk->win, wk->TrCardData, wk->PlayTimeBuf);
      //コロン描く
      TRCBmp_WriteSec(wk,wk->win[TRC_BMPWIN_PLAY_TIME], TRUE, wk->SecBuf);
    }else if(wk->SecCount == TRC_FRAME_RATE/2){
      //コロン消す
      TRCBmp_WriteSec(wk,wk->win[TRC_BMPWIN_PLAY_TIME], FALSE, wk->SecBuf);
    }
  }
  //カウントアップ
  wk->SecCount++;   //  1/TRC_FRAME_RATEなので
  if( wk->SecCount >= TRC_FRAME_RATE )
    wk->SecCount -= TRC_FRAME_RATE;
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
  {OEKAKI_BOARD_POSY*8,(OEKAKI_BOARD_POSY+OEKAKI_BOARD_H)*8-1, OEKAKI_BOARD_POSX*8, (OEKAKI_BOARD_POSX+OEKAKI_BOARD_W)*8-1},
  {GFL_UI_TP_HIT_END,0,0,0},
};

static const GFL_UI_TP_HITTBL large_sign_tbl[]={
  {LARGE_BOARD_POSY*8,(LARGE_BOARD_POSY+LARGE_BOARD_H)*8-1, LARGE_BOARD_POSX*8, (LARGE_BOARD_POSX+LARGE_BOARD_W)*8-1},
  {GFL_UI_TP_HIT_END,0,0,0},
};

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
  if(GFL_UI_TP_GetPointCont( &x, &y )){

    // 通常
    if(scale_mode==0){
      if( GFL_UI_TP_HitSelf( sign_tbl, x, y )==0){
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
      if( GFL_UI_TP_HitSelf( large_sign_tbl, x, y )==0){
        wk->MyTouchResult.x = OEKAKI_BOARD_POSX*8+(x-LARGE_BOARD_POSX*8+wk->ScaleSide*24*8)/2;
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

static void draw_pen( u8 *sign, u8* brush, int x, int y, int ww, int hh )
{
  int bx,by,px,py,wx,wy;
  int i,offset,w,h,count=0;
  
  for(h=0;h<4;h++){
    for(w=0;w<4;w++){
      wx = x+w;
      wy = y+h;
      if(wx<0 || wx >=ww || wy <0 || wy >=hh){
        continue;
      }
      bx = wx / 8;
      by = wy / 8;
      px = wx % 8;
      py = wy % 8;
      offset = (by*OEKAKI_BOARD_W+bx)*64 + py*8+px;
      if(brush[count]==1){
        sign[offset] = 1;
      }else if(brush[count]==2){
        sign[offset] = 0;
      }
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

    _BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
    s = dx >> 1;
    while (++x1 <= x2) {
      if ((s -= dy) < 0) {
        s += dx;  y1 += step;
      };
    _BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
    }
  } else {
    if (y1 > y2) {
      step = (x1 > x2) ? 1 : -1;
      s = y1;  y1 = y2;  y2 = s;  x1 = x2;
    } else step = (x1 < x2) ? 1 : -1;
    _BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
    s = dy >> 1;
    while (++y1 <= y2) {
      if ((s -= dx) < 0) {
        s += dy;  x1 += step;
      }
      _BmpWinPrint_Rap( win, (void*)brush,  0, 0, 4, 4, x1, y1, 4, 4 );
    }
  }
  
  
  *sx = px;     *sy = py;

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
 * @param   win   
 * @param   all   
 * @param   old   
 * @param   draw    メモリ上で行ったCGX変更を転送するか？(0:しない  1:する）
 * @param   SignData    
 * @param   sign_mode   
 */
//----------------------------------------------------------------------------------
static void DrawBrushLine( GFL_BMPWIN *win, TOUCH_INFO *all, TOUCH_INFO *old, int draw, u8 *SignData, u8 sign_mode )
{
  int px,py,i,r,flag=0, sx, sy;

//  OS_Printf("id0=%d,id1=%d,id2=%d,id3=%d,id4=%d\n",all[0].size,all[1].size,all[2].size,all[3].size,all[4].size);

  if(all->on!=0){
    if(old->on){
      sx = old->x-OEKAKI_BOARD_POSX*8;
      sy = old->y-OEKAKI_BOARD_POSY*8;
    }
    px = all->x - OEKAKI_BOARD_POSX*8;
    py = all->y - OEKAKI_BOARD_POSY*8;
      
//    OS_Printf("sx=%d, sy=%d, px=%d, py=%d\n", sx,sy,px,py);

    // BG1面用BMP（お絵かき画像）ウインドウ確保
    DrawPoint_to_Line(win, sign_brush[sign_mode][all->brush], px, py, &sx, &sy, 0, old->on);
    flag = 1;
    
  }
  if(flag && draw){

    // サイングラフィックデータをVRAMに転送
    GFL_BG_LoadCharacter( TRC_BG_SIGN, SignData, SIGN_BYTE_SIZE, SIGN_CGX );
    
//    OS_Printf("write board %d times\n",debug_count++);
    //GF_BGL_BmpWinOn( win );
    
  }
  
  // 今回の最終座標のバックアップを取る   
    Stock_OldTouch(all, old);
//  for(i=0;i<OEKAKI_MEMBER_MAX;i++){
    all->on = 0;    // 一度描画したら座標情報は捨てる
//  }
  
}

