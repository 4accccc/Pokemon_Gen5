//======================================================================
/*
 * @file	enceff_prg.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include "enceff_prg.h"

#include "fieldmap.h"

#include "system/main.h"

typedef struct ENCEFF_PRG_WORK_tag
{
  int BrightCount;
  GXVRamTex						vTex;
	BOOL								vTexRecover;
	DRAW3DMODE					dMode;
  BOOL CapEndFlg;
  GFL_G3D_CAMERA *g3Dcamera;
  NNSGfdTexKey	texKey;

  void *Work;

  CREATE_FUNC CreateFunc;
  DRAW_FUNC DrawFunc;
}ENCEFF_PRG_WORK;


static GMEVENT_RESULT MainEvt( GMEVENT* event, int* seq, void* work );
static void ReqCapture(ENCEFF_PRG_WORK *evt_work);
static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work );
static NNSGfdTexKey SetVramKey(void);

//--------------------------------------------------------------------------------------------
/**
 * イベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   inCamPos    デフォルトカメラ位置
 * @param   mainFunc    設定メインファンクション
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *ENCEFF_PRG_Create(
    GAMESYS_WORK *gsys,
    const VecFx32 *inCamPos,
    CREATE_FUNC createFunc,
    DRAW_FUNC drawFunc )
{
  GMEVENT * event;
  ENCEFF_PRG_WORK *work;
  int size;
  int i;
  size = sizeof(ENCEFF_PRG_WORK);
  //イベント作成
  {
    event = GMEVENT_Create( gsys, NULL, MainEvt, size );

    work = GMEVENT_GetEventWork(event);
    MI_CpuClear8( work, size );
    {
      VecFx32 target = {0,0,0};
      //カメラ作成
      work->g3Dcamera = GFL_G3D_CAMERA_CreateDefault(
          inCamPos, &target, HEAPID_FLD3DCUTIN );   //カットインのヒープを使用
    }
    work->CreateFunc = createFunc;
    work->DrawFunc = drawFunc;
  }
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画関数
 *
 *	@param	ptr     管理ポインタ
 *
 *	@return none
 */
//-----------------------------------------------------------------------------
void ENCEFF_PRG_Draw(ENCEFF_PRG_PTR ptr)
{
  G3X_Reset();
  {
    VecFx32		camPos, camUp, target, vecNtmp;
		MtxFx43		mtxCamera, mtxCameraInv;

		GFL_G3D_CAMERA_GetPos( ptr->g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( ptr->g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( ptr->g3Dcamera, &target );

		G3_LookAt( &camPos, &camUp, &target, &mtxCamera );	//mtxCameraには行列計算結果が返る
  }

  G3_TexImageParam( GX_TEXFMT_DIRECT, GX_TEXGEN_TEXCOORD,
                    GX_TEXSIZE_S256, GX_TEXSIZE_T256,
                    GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
                    GX_TEXPLTTCOLOR0_USE, NNS_GfdGetTexKeyAddr(ptr->texKey) );


  ptr->DrawFunc(ptr->Work);

}

//--------------------------------------------------------------------------------------------
/**
 * イベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT MainEvt( GMEVENT* event, int* seq, void* work )
{
  ENCEFF_PRG_WORK *evt_work = work;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  switch(*seq){
  case 0:
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, -1 );
    (*seq)++;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE ){
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, -1 );
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_FADE_CheckFade() == FALSE ){
      evt_work->BrightCount++;
      
      if( evt_work->BrightCount < 3 ){
        GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, -1 );
        (*seq) = 1;
      }else{
				(*seq)++;
      }
    }
    break;
  case 3:
    {
      //キャプチャリクエスト
      ReqCapture(evt_work);
      (*seq)++;
    }
    break;
  case 4:
    //キャプチャ終了待ち
    if (evt_work->CapEndFlg){
      //キャプチャ終わったら、プライオリティ変更と表示変更
      GFL_BG_SetPriority( 0, 0 );
      GFL_BG_SetPriority( 2, 3 );

      evt_work->dMode = FIELDMAP_GetDraw3DMode(fieldmap);

      // VRAMバンクセットアップ(VRAM_Dをテクスチャとして使用)
      evt_work->vTex = GX_GetBankForTex(); 
      evt_work->vTexRecover = FALSE;
      if(!(evt_work->vTex & GX_VRAM_D)){
        evt_work->vTex |= GX_VRAM_D;
        evt_work->vTexRecover = TRUE;
      }
      GX_DisableBankForLCDC();
      GX_SetBankForTex(evt_work->vTex);
      evt_work->texKey = SetVramKey();
      
      (*seq)++;
    }
    break;
  case 5:
    //セットアップ終了したので3Ｄ面もオン
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
    
    //フィールド表示モード切替
    FIELDMAP_SetDraw3DMode(fieldmap, DRAW3DMODE_ENCEFF);

    //生成関数コール
    {
      GMEVENT *call_event;
      ENCEFF_CNT_PTR cnt_ptr;
      call_event = evt_work->CreateFunc(gsys);
      //ワークセット
//      evt_work->Work = GMEVENT_GetEventWork(call_event);
      cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldmap);
      evt_work->Work = ENCEFF_GetUserWorkPtr(cnt_ptr);
      //イベントコール
      GMEVENT_CallEvent( event, call_event );
    }
    (*seq)++;
    break;
  case 6:
    //オーバーレイアンロード
    ;
    // VRAMバンク復帰
		if(evt_work->vTexRecover == TRUE){
			evt_work->vTex &= (GX_VRAM_D^0xffffffff);
			GX_SetBankForTex(evt_work->vTex); 
		}

		FIELDMAP_SetDraw3DMode(fieldmap, evt_work->dMode);
    //カメラ解放
    GFL_G3D_CAMERA_Delete(evt_work->g3Dcamera);
    return( GMEVENT_RES_FINISH );
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャプチャリクエスト
 *
 *	@param	evt_work  ワークポインタ
 *
 *	@return none
 */
//-----------------------------------------------------------------------------
static void ReqCapture(ENCEFF_PRG_WORK *evt_work)
{
  evt_work->CapEndFlg = FALSE;
  GX_SetBankForLCDC(GX_VRAM_LCDC_D);
  GFUser_VIntr_CreateTCB(Graphic_Tcb_Capture, &evt_work->CapEndFlg, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vブランクタスク
 *
 *	@param	GFL_TCB *p_tcb
 *	@param	*p_work 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work )
{
  BOOL *cap_end_flg = p_work;
  
	GX_SetCapture(GX_CAPTURE_SIZE_256x192,  // Capture size
                      GX_CAPTURE_MODE_A,			   // Capture mode
                      GX_CAPTURE_SRCA_3D,						 // Blend src A
                      GX_CAPTURE_SRCB_VRAM_0x00000,     // Blend src B
                      GX_CAPTURE_DEST_VRAM_D_0x00000,   // Output VRAM
                      16,             // Blend parameter for src A
                      0);            // Blend parameter for src B

  GFL_TCB_DeleteTask( p_tcb );
  if (cap_end_flg != NULL){
    *cap_end_flg = TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	VＲＡＭキーセット
 *
 *	@param	evt_work    イベントワークポインタ
 *
 *	@return none
 */
//-----------------------------------------------------------------------------
static NNSGfdTexKey SetVramKey(void)
{
  GXVRamTex			vTex = GX_GetBankForTex();
  u32 VRAMoffs = 0x00000;
  u32						texSize	= 256*256*2;

  if(vTex |= GX_VRAM_C){ VRAMoffs += 0x20000; }
  if(vTex |= GX_VRAM_B){ VRAMoffs += 0x20000; }
  if(vTex |= GX_VRAM_A){ VRAMoffs += 0x20000; }

  return NNS_GfdMakeTexKey(VRAMoffs, texSize, FALSE);
}

