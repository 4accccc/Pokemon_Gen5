//============================================================================================
/**
 * @file	fld_faceup.c
 * @brief	顔アップ
 */
//============================================================================================
#include "arc_def.h"
#include "gamesystem/game_event.h"
#include "system/main.h"    //for HEAPID_FLD3DCUTIN 
#include "field_bg_def.h"
#include "field_place_name.h" //for   FIELD_PLACE_NAME
#include "field_debug.h"      //for FIELD_DEBUG～
#include "fld_faceup_def.h"
#include "fld_faceup.h"
#include "fieldmap.h"

#include "arc/fieldmap/fld_faceup.naix"

#define BG_PLT_NO (0)
#define BG_PLT_NUM (10)
#define FACE_PLT_NO (0)
#define FACE_PLT_NUM  (2) 

#define SCR_SIZE_W  (32)
#define SCR_SIZE_H  (24)

#define ALPHA_MASK_FACE  ( GX_BLEND_PLANEMASK_BG3 )  // αブレンディング第一対象面(顔)
#define ALPHA_MASK_BG   ( GX_BLEND_PLANEMASK_BG2 )  // αブレンディング第二対象面(背景)
#define ALPHA_MAX          (16)                      // α最大値

#define ALPHA_COUNT_MAX (30)

#define SCR_BYTE_SIZE (2)

#define EYE_SRC_TRNAS_CHR_NO (16*22)
#define MOUTH_SRC_TRNAS_CHR_NO (16*22+12*3)
#define EYE_DST_TRNAS_SCR_POS (32*8+13)
#define MOUTH_DST_TRNAS_SCR_POS (32*11+14)
#define TRNAS_WIDTH_MAX (6)
#define EYE_TRNAS_WIDTH ( TRNAS_WIDTH_MAX )
#define MOUTH_TRNAS_WIDTH (4)
#define FACE_WIDTH (16)
#define TRANS_HEIGHT  (2)
#define SCR_WIDTH (32)

#define EYE_ANM_MAX (5)
#define MOUTH_ANM_MAX (4)

#define EYE_DEF_COUNT (80)
#define EYE_RND_COUNT (20)

#define EYE_WAIT_MAX    (2)
#define MOUTH_WAIT_MAX_SLOW    (3)
#define MOUTH_WAIT_MAX_NORMAL    (2)
#define MOUTH_WAIT_MAX_FAST    (1)

typedef struct ANM_PAT_tag
{
  u16 No;
  u16 Wait;
}ANM_PAT;

typedef struct ANM_CNT_tag
{
  BOOL Stop;
  BOOL TransReq;
  u8 Wait;
  u8  WaitMax;
  u16  PatIdx;
  u16 TransDat[TRANS_HEIGHT][TRNAS_WIDTH_MAX];
}ANM_CNT;

typedef struct FACEUP_WORK_tag
{
  //HEAPID HeapID;
  u8 BgPriority[4];
  GXPlaneMask Mask;
  GXBg23ControlText CntText;

  int BackNo;       //背景BG

  SCRCMD_WORK *ScrCmdWork;
  GFL_TCB* MainTcb;
  GFL_TCB* TransTcb;
  BOOL MsgEnd;
  BOOL EyeTransReq;

  u16 AlphaCount;
  u16 EyeAnmWaitCount;
  
  ANM_CNT EyeAnm;
  ANM_CNT MouthAnm;

  BOOL Last;
  BOOL AnmReq;
}FACEUP_WORK;


static GMEVENT_RESULT SetupEvt( GMEVENT* event, int* seq, void* work );
static void Setup(FACEUP_WK_PTR ptr, FIELDMAP_WORK *fieldmap);
static GMEVENT_RESULT ReleaseEvt( GMEVENT* event, int* seq, void* work );
static void Release(FIELDMAP_WORK * fieldmap, FACEUP_WK_PTR ptr);

static void PushPriority(FACEUP_WK_PTR ptr);
static void PushDisp(FACEUP_WK_PTR ptr);
static void PopPriority(FACEUP_WK_PTR ptr);
static void PopDisp(FACEUP_WK_PTR ptr);

//static void ChangeScreenPlt( void *rawData, u8 px, u8 py, u8 sx, u8 sy, u8 pal );
static BOOL AlphaFunc(FACEUP_WK_PTR ptr);

static void MainTcbFunc( GFL_TCB* tcb, void* work );
static void TransTcbFunc( GFL_TCB* tcb, void* work );

static void MoveAnm(  const ANM_PAT *inPatDat,
                      const u32 inAnmMax,
                      const u32 inTransChrNo,
                      const u32 inTansSize,
                      const u32 inTransWidth,
                      ANM_CNT *cnt );
static void InitAnmCnt(ANM_CNT *cnt);

//目アニメパターン
static const ANM_PAT EyeAnmPat[EYE_ANM_MAX] = {
  {0, 2},
  {1, 2},
  {2, 2},
  {1, 2},
  {0, 2},
};

//口アニメパターン
static const ANM_PAT MouthAnmPat[MOUTH_ANM_MAX] = {
  {1, 3},
  {2, 3},
  {1, 3},
  {0, 3},
};

//--------------------------------------------------------------
/**
 * セットアップイベント
 * @param     inBackNo	    背景ナンバー
 * @param     inCharNo      人物ナンバー    最終的にＮしかいなくなったので未使用
 * @param     inLast        最終使用フラグ TRUEならば、終了時にフェードイン処理をしない
 * @param     gsys          ゲームシステムポインタ
 * @param     scrCmdWork    スクリプトコマンドワーク
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
GMEVENT *FLD_FACEUP_Start(const int inBackNo, const int inCharNo, const BOOL inLast, GAMESYS_WORK *gsys, SCRCMD_WORK *scrCmdWork)
{
  GMEVENT * event;
  HEAPID heapID;
  FACEUP_WK_PTR ptr;

  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  heapID = HEAPID_FLD3DCUTIN;
  
  //ワークをアロケーションして、フェイスアップデータポインタにセット
  ptr = GFL_HEAP_AllocClearMemory(heapID, sizeof(FACEUP_WORK));   //カットインヒープからアロケート

  if ( *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) != NULL )
  {
    GF_ASSERT(0);
    return NULL;
  }

  *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) = ptr;
//  ptr->HeapID = heapID;
  ptr->BackNo = inBackNo;
  ptr->ScrCmdWork = scrCmdWork;
  ptr->Last = inLast;
  //アニメーション管理初期化
  {
    InitAnmCnt(&ptr->EyeAnm);
    InitAnmCnt(&ptr->MouthAnm);
  }

  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
    CONFIG*  cfg = SaveData_GetConfig( save );
    MSGSPEED speed = CONFIG_GetMsgSpeed( cfg );
    if ( speed == MSGSPEED_SLOW) ptr->MouthAnm.WaitMax = MOUTH_WAIT_MAX_SLOW;
    else if( speed == MSGSPEED_NORMAL ) ptr->MouthAnm.WaitMax = MOUTH_WAIT_MAX_NORMAL;
    else ptr->MouthAnm.WaitMax = MOUTH_WAIT_MAX_FAST;

    ptr->EyeAnm.WaitMax = EYE_WAIT_MAX;
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, SetupEvt, 0 );

  {
    GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );

    // TCBを追加
    ptr->MainTcb = GFL_TCB_AddTask( tcbsys, MainTcbFunc, ptr, 0 );
    ptr->TransTcb = GFUser_VIntr_CreateTCB(TransTcbFunc, ptr, 1 );

    ptr->MsgEnd = FALSE;
    ptr->AnmReq = TRUE;
  }
  return event;
}

//--------------------------------------------------------------
/**
 * セットアップイベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SetupEvt( GMEVENT* event, int* seq, void* work )
{
  FACEUP_WK_PTR ptr;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);

  switch(*seq){
  case 0:
    //ブラックアウト開始
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, 0 );
    (*seq)++;
    break;
  case 1:
    //ブラックアウト待ち
    if ( GFL_FADE_CheckFade() ) break;
    //セットアップ
    Setup(ptr, fieldmap);
    //ブラックイン開始
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 0 );
    (*seq)++;
    break;
  case 2:
    if ( GFL_FADE_CheckFade() == FALSE ){
      ptr->AlphaCount = 0;    //アルファカウント初期化
      (*seq)++;
    }
    break;
  case 3:
    //アルファ処理
    if ( AlphaFunc(ptr) )
    {
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * セットアップ
 * @param     ptr	            ワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
static void Setup(FACEUP_WK_PTR ptr, FIELDMAP_WORK *fieldmap)
{
  int ncgr;
  int nscr;
  int nclr;
  HEAPID heapID = HEAPID_FIELDMAP;

  //プライオリティ保存
  PushPriority(ptr);
  //表示状態の保存
  PushDisp(ptr);
  //BG設定保存
  ptr->CntText = G2_GetBG3ControlText();
  
  //ＢＧ3セットアップ（顔BG256色）もともとの設定は地名表示がしてくれている
  G2_SetBG3ControlText(
      GX_BG_SCRSIZE_TEXT_256x256,
      GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x1000,
      GX_BG_CHARBASE_0x04000);

  //アルファセット
  G2_SetBlendAlpha( ALPHA_MASK_FACE, ALPHA_MASK_BG, 0, ALPHA_MAX );

  { //BGリソースをクリア
    BOOL rc;
    FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( fieldmap );
    rc = FLDMSGBG_ReleaseBG2Resource( fmb );
    if (rc) FLDMSGBG_ReqResetBG2( fmb );
    else
    {
      //現在の状態を解放
      GFL_BG_FreeBGControl( FLDBG_MFRM_EFF1 );
    }
    //セットしなおし
    {
      //BG Frame
		  GFL_BG_BGCNT_HEADER bgcntText = {
		   0, 0, FLDBG_MFRM_EFF1_SCRSIZE, 0,
		   GFL_BG_SCRSIZ_256x256, /*FLDBG_MFRM_EFF1_COLORMODE*/GX_BG_COLORMODE_256,
		   FLDBG_MFRM_EFF1_SCRBASE, FLDBG_MFRM_EFF1_CHARBASE, FLDBG_MFRM_EFF1_CHARSIZE,
		   GX_BG_EXTPLTT_01, FLDBG_MFRM_EFF1_PRI, 0, 0, FALSE
		  };
      GFL_BG_SetBGControl( FLDBG_MFRM_EFF1, &bgcntText, GFL_BG_MODE_TEXT );
    }
	}

  if (ptr->BackNo == FLD_FACEUP_BG_WHEEL)
  {
    ncgr = NARC_fld_faceup_faceup_bg_wheel_ncgr;
    nscr = NARC_fld_faceup_faceup_bg_wheel_nscr;
    nclr = NARC_fld_faceup_faceup_bg_wheel_nclr;
  }
  else if (ptr->BackNo == FLD_FACEUP_BG_CATSLE)
  {
    ncgr = NARC_fld_faceup_faceup_bg_catsle_ncgr;
    nscr = NARC_fld_faceup_faceup_bg_catsle_nscr;
    nclr = NARC_fld_faceup_faceup_bg_catsle_nclr;
  }
  else
  {
    ncgr = NARC_fld_faceup_faceup_bg_catsle2_ncgr;
    nscr = NARC_fld_faceup_faceup_bg_catsle2_nscr;
    nclr = NARC_fld_faceup_faceup_bg_catsle2_nclr;
  }
  
  //タイプごとの背景、顔BGをロード
  {
    void *buf;    
    //背景
    //キャラ
    {
      NNSG2dCharacterData *chr;
		  buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, ncgr, GFL_HEAP_LOWID(heapID) );
		  GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadCharacter( FLDBG_MFRM_EFF1, chr->pRawData, chr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }    
    //スクリーン
    {
      NNSG2dScreenData *scr;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, nscr, GFL_HEAP_LOWID(heapID) );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedScreenData(buf,&scr) == FALSE ){
        GF_ASSERT( 0 );
      }
//      ChangeScreenPlt( scr->rawData, 0, 0, SCR_SIZE_W, SCR_SIZE_H, BG_PLT_NO );
      GFL_BG_LoadScreen( FLDBG_MFRM_EFF1, scr->rawData, scr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    //パレット
    {
      NNSG2dPaletteData *pal;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, nclr, GFL_HEAP_LOWID(heapID) );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedPaletteData(buf,&pal) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadPalette( FLDBG_MFRM_EFF1, pal->pRawData, BG_PLT_NUM*32, BG_PLT_NO*32 );
      GFL_HEAP_FreeMemory( buf );
    }
    //顔
    //キャラ
    {
      NNSG2dCharacterData *chr;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_faceup_ncgr, GFL_HEAP_LOWID(heapID) );
		  GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadCharacter( FLDBG_MFRM_EFF2, chr->pRawData, chr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    //スクリーン
    {
      NNSG2dScreenData *scr;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_faceup_nscr, GFL_HEAP_LOWID(heapID) );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedScreenData(buf,&scr) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadScreen( FLDBG_MFRM_EFF2, scr->rawData, scr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    
  }

  //プライオリティ
  GFL_BG_SetPriority( GFL_BG_FRAME0_M, 3 );
  GFL_BG_SetPriority( GFL_BG_FRAME1_M, 0 );
  GFL_BG_SetPriority( GFL_BG_FRAME2_M, 2 );
  GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );

  //表示
  GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * 終了関数
 * @param     gsys    ゲームシステムポインタ
 * @return    none
 */
//--------------------------------------------------------------
GMEVENT *FLD_FACEUP_End(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  if ( *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) == NULL )
  {
    GF_ASSERT(0);
    return NULL;
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ReleaseEvt, 0 );
  return event;
}

//--------------------------------------------------------------
/**
 * リリース
 * @param     fieldmap    フィールドマップポインタ
 * @return    none
 */
//--------------------------------------------------------------
void FLD_FACEUP_Release( FIELDMAP_WORK *fieldmap )
{
  FACEUP_WK_PTR ptr;
  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);
  Release(fieldmap, ptr);
}

//--------------------------------------------------------------
/**
 * アニメ開始
 * @param     fieldmap    フィールドマップポインタ
 * @return    none
 */
//--------------------------------------------------------------
void FLD_FACEUP_AnmStart( FIELDMAP_WORK *fieldmap )
{
  FACEUP_WK_PTR ptr;
  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);
  ptr->MsgEnd = FALSE;
  ptr->AnmReq = TRUE;
}

//--------------------------------------------------------------
/**
 * リリースイベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ReleaseEvt( GMEVENT* event, int* seq, void* work )
{
  FACEUP_WK_PTR ptr;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);

  switch(*seq){
  case 0:
    //ブラックアウト開始
    if (ptr->Last)
    {
      //両面フェード
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, -8 );
    }
    else
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, 0 );
    }
    (*seq)++;
    break;
  case 1:
    //ブラックアウト待ち
    if ( GFL_FADE_CheckFade() ) break;

    //メインＴＣＢ削除
    GFL_TCB_DeleteTask( ptr->MainTcb );
    (*seq)++;
    break;
  case 2:
    {
      BOOL last;
      //転送タスク終了待ち
      if (ptr->EyeAnm.TransReq || ptr->MouthAnm.TransReq)  break;

      //転送ＴＣＢ削除
      GFL_TCB_DeleteTask( ptr->TransTcb );
      //ラストフラグ保存
      last = ptr->Last;
      //リリース
      Release(fieldmap, ptr);

      if ( last )
      {
        return GMEVENT_RES_FINISH;
      }
      else
      {
        //ブラックイン開始
        GFL_FADE_SetMasterBrightReq(
              GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 0 );
        (*seq)++;
      }
    }
    break;
  case 3:
    if ( GFL_FADE_CheckFade() == FALSE ){
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * リリース
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static void Release(FIELDMAP_WORK * fieldmap, FACEUP_WK_PTR ptr)
{
  FIELD_PLACE_NAME *place_name_sys = FIELDMAP_GetPlaceNameSys( fieldmap );

  GFL_BG_ClearFrame( GFL_BG_FRAME2_M );

  //BG設定復帰
  G2_SetBG3ControlText(
      ptr->CntText.screenSize,
      ptr->CntText.colorMode,
      ptr->CntText.screenBase,
      ptr->CntText.charBase);
  //復帰
  FIELD_PLACE_NAME_RecoverBG( place_name_sys );
#ifndef  PM_DEBUG
  {
    FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( fieldmap );
    FLDMSGBG_ReqResetBG2( fmb );  //BG2復帰リクエスト（実際の復帰はFLDMSGBGのメインで実行）
  }
#else
  { //実行内容は↑FLDMSGBG_ReqResetBG2＋デバッグ描画OFF
    FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( fieldmap );
    FIELD_DEBUG_RecoverBgCont( debug );
  }
#endif

  G2_BlendNone();

  //プライオリティ復帰
  PopPriority(ptr);
  //表示状態復帰
  PopDisp(ptr);
  //ワーク解放
  GFL_HEAP_FreeMemory( ptr );
  *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プライオリティプッシュ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PushPriority(FACEUP_WK_PTR ptr)
{
  int i;
  for(i=0;i<4;i++){
    ptr->BgPriority[i] = GFL_BG_GetPriority( i );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示状態プッシュ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PushDisp(FACEUP_WK_PTR ptr)
{
  ptr->Mask = GFL_DISP_GetMainVisible();
}

//----------------------------------------------------------------------------
/**
 *	@brief	プライオリティポップ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PopPriority(FACEUP_WK_PTR ptr)
{
  int i;
  for(i=0;i<4;i++){
    GFL_BG_SetPriority( i, ptr->BgPriority[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示状態ポップ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PopDisp(FACEUP_WK_PTR ptr)
{
  GFL_DISP_GX_SetVisibleControlDirect(ptr->Mask);
}

//------------------------------------------------------------------------------------------
/**
 * @brief アルファコントロール実行関数
 * @param ptr   管理ポインタ
 */
//------------------------------------------------------------------------------------------
static BOOL AlphaFunc(FACEUP_WK_PTR ptr)
{
  int   alpha1;
  int   alpha2;
  ptr->AlphaCount++;
  alpha1 = (ALPHA_MAX * ptr->AlphaCount) / ALPHA_COUNT_MAX;
  alpha2 = ALPHA_MAX - alpha1;

  G2_SetBlendAlpha( ALPHA_MASK_FACE, ALPHA_MASK_BG, alpha1, alpha2 );

  if ( ptr->AlphaCount >=  ALPHA_COUNT_MAX ) return TRUE;
  else return FALSE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief TCB実行関数
 */
//------------------------------------------------------------------------------------------
static void MainTcbFunc( GFL_TCB* tcb, void* work )
{
  FACEUP_WK_PTR ptr = (FACEUP_WK_PTR)work;
  
  if (!ptr->MsgEnd)
  {
    //プレーンウィンドウ以外のときは処理しない
    if ( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) )
    {
      FLDPLAINMSGWIN *win = SCRCMD_WORK_GetMsgWinPtr( ptr->ScrCmdWork );
      if (win != NULL)
      {
        PRINTSTREAM_STATE state = FLDPLAINMSGWIN_GetStreamState( win );
        if ( state == PRINTSTREAM_STATE_RUNNING )
        {
          NOZOMU_Printf("口パクしてＯＫ\n");
          ptr->MouthAnm.Stop = FALSE;
          ptr->AnmReq = FALSE;    //リクエスト消化
        }
        else
        {
          //
          NOZOMU_Printf("口閉じる\n");
          if ( state == PRINTSTREAM_STATE_DONE )
          {
            if ( ptr->AnmReq == FALSE )   //リクエスト消化済みならメッセージを終了させる
            {
              NOZOMU_Printf("メッセージ終了\n");
              ptr->MsgEnd = TRUE;
            }
          }
        }
      }
      else{
        NOZOMU_Printf("NOTHING WIN\n");
      }
    }
  }

  //口パク
  {
    MoveAnm(  MouthAnmPat, MOUTH_ANM_MAX,
              MOUTH_SRC_TRNAS_CHR_NO,
              MOUTH_TRNAS_WIDTH*TRANS_HEIGHT,
              MOUTH_TRNAS_WIDTH,
              &ptr->MouthAnm );
  }

  //目パチ
  {
    if (ptr->EyeAnmWaitCount<=0){
      ptr->EyeAnm.Stop = FALSE;
      ptr->EyeAnmWaitCount = EYE_DEF_COUNT + GFUser_GetPublicRand( EYE_RND_COUNT );
    }
    else ptr->EyeAnmWaitCount--;
    
    //アニメ
    MoveAnm(  EyeAnmPat, EYE_ANM_MAX,
              EYE_SRC_TRNAS_CHR_NO,
              EYE_TRNAS_WIDTH*TRANS_HEIGHT,
              EYE_TRNAS_WIDTH,
              &ptr->EyeAnm );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief TCB実行関数
 */
//------------------------------------------------------------------------------------------
static void TransTcbFunc( GFL_TCB* tcb, void* work )
{
  ANM_CNT *cnt;
  FACEUP_WK_PTR ptr = (FACEUP_WK_PTR)work;

  //目
  cnt = &ptr->EyeAnm;
  if ( cnt->TransReq )
  {
    //転送
    int byte = EYE_TRNAS_WIDTH * SCR_BYTE_SIZE;
    int ofs;
    //転送
    ofs = EYE_DST_TRNAS_SCR_POS;
    GFL_BG_LoadScreen( FLDBG_MFRM_EFF2, cnt->TransDat[0], byte, ofs );
    ofs += SCR_WIDTH;
    GFL_BG_LoadScreen( FLDBG_MFRM_EFF2, cnt->TransDat[1], byte, ofs );
    cnt->TransReq = FALSE;
  }

  //口
  cnt = &ptr->MouthAnm;
  if ( cnt->TransReq )
  {
    int byte = MOUTH_TRNAS_WIDTH * SCR_BYTE_SIZE;
    int ofs;
    //転送
    ofs = MOUTH_DST_TRNAS_SCR_POS;
    GFL_BG_LoadScreen( FLDBG_MFRM_EFF2, cnt->TransDat[0], byte, ofs );
    ofs += SCR_WIDTH;
    GFL_BG_LoadScreen( FLDBG_MFRM_EFF2, cnt->TransDat[1], byte, ofs );
    cnt->TransReq = FALSE;
  }
}

//--------------------------------------------------------------
/**
 * アニメ
 * @param     inPatDat        アニメパターンデータポインタ
 * @param     inAnmMax        アニメ数         
 * @param     inTransChrNo    スクリーン転送位置
 * @param     inTransSize     転送サイズ
 * @param     inTransWidth    転送幅
 * @param     cnt             アニメコントローラポインタ
 * @return    none
 */
//--------------------------------------------------------------
static void MoveAnm( const ANM_PAT *inPatDat,
                     const u32 inAnmMax,
                     const u32 inTransChrNo,
                     const u32 inTansSize,
                     const u32 inTransWidth,
                     ANM_CNT *cnt )
{
  int i,j;
  int base;
  if ( (!cnt->TransReq) )
  {
    if (cnt->Wait <= 0 )
    {
      //ストップがかかっている場合は処理しない
      if ( (cnt->PatIdx == 0)&&cnt->Stop ) return;
      //patarn番目の転送リクエスト
      cnt->TransReq = TRUE;
      base = inTransChrNo + inPatDat[cnt->PatIdx].No * inTansSize;
      for (j=0;j<TRANS_HEIGHT;j++)
      {
        for(i=0;i<inTransWidth;i++)
        {
          cnt->TransDat[j][i] = (j*inTransWidth)+base+i;
        }
      }
      //転送後のウェイトセット
      cnt->Wait = cnt->WaitMax;
      //パターン更新
      cnt->PatIdx++;

      if ( cnt->PatIdx >= inAnmMax )
      {
        //開始フレームに戻す
        cnt->PatIdx = 0;
        //停止
        cnt->Stop = TRUE;
      }
    }
    else cnt->Wait--;
  }
}

//--------------------------------------------------------------
/**
 * アニメコントローラ初期化
 * @param     cnt             アニメコントローラポインタ
 * @return    none
 */
//--------------------------------------------------------------
static void InitAnmCnt(ANM_CNT *cnt)
{
  cnt->Stop = TRUE;
  cnt->TransReq = FALSE;
  cnt->Wait = 0;
  cnt->WaitMax = 0;
  cnt->PatIdx = 0;
}

