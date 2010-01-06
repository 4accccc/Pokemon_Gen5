//======================================================================
/*
 * @file	enceff2.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include <gflib.h>

#include "enceff.h"

#include "fieldmap.h"

#include "system/main.h"


#define MUL_VAL (16)
#define Z_MUL_VAL (64)
#define POLY_W_NUM  (32)
#define POLY_H_NUM  (24)

#define VTX_W_NUM (POLY_W_NUM+1)
#define VTX_H_NUM (POLY_H_NUM+1)

#define WAVE_WAIT (8)
#define WAVE_COUNT (7)

#define WAVE_AFTER_WAIT (6)

#define WAVE_HEIGHT (FX16_ONE/4)

//--------------------------------------------------------------
/// ENCEFF2_WORK
//--------------------------------------------------------------
typedef struct
{
  int Count;
  GXVRamTex						vTex;
	BOOL								vTexRecover;
	DRAW3DMODE					dMode;
  BOOL CapEndFlg;

  VecFx16 Vtx[VTX_W_NUM*VTX_H_NUM];
  fx16 TmpZ[VTX_W_NUM*VTX_H_NUM];

  GFL_G3D_CAMERA *g3Dcamera;
  NNSGfdTexKey	texKey;
  int WaveWait;
  int WaveCount;
}ENCEFF2_WORK;

static void DrawMesh(ENCEFF_CNT_PTR ptr);
static GMEVENT_RESULT EffEvt( GMEVENT* event, int* seq, void* work );

static void ReqCapture(ENCEFF2_WORK *evt_work);
static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work );
static void SetVramKey(ENCEFF2_WORK *evt_work);
static BOOL WaveMain(ENCEFF2_WORK *evt_work);

//--------------------------------------------------------------------------------------------
/**
 * イベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   fieldWork   フィールドマップポインタ
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *ENCEFF_CreateEff2(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT * event;
  ENCEFF2_WORK *work;
  int size;
  int i;
  size = sizeof(ENCEFF2_WORK);
  //イベント作成
  {
    event = GMEVENT_Create( gsys, NULL, EffEvt, size );

    work = GMEVENT_GetEventWork(event);
    MI_CpuClear8( work, size );
    {
      VecFx32 pos = {0, 0, FX32_ONE*(256)};
      VecFx32 target = {0,0,0};
      //カメラ作成
      work->g3Dcamera = GFL_G3D_CAMERA_CreateDefault(
          &pos, &target, HEAPID_FLD3DCUTIN );   //カットインのヒープを使用
    }

    //波の回数セット
    work->WaveCount = WAVE_COUNT;
    work->WaveWait = 0;


/**
    //パネル初期化
    for (i=0;i<POLY_W_NUM*POLY_H_NUM;i++)
    {
      int x,y;
      PANEL_WK* panel = &work->Panel[i];
      x = i%POLY_W_NUM;
      y = i/POLY_W_NUM;
      panel->Pos.x = (x * 8 * FX32_ONE)-DISP_OFSET_W;
      panel->Pos.y = -(y * 8 * FX32_ONE)+DISP_OFSET_H;
      panel->Pos.z = 0;
      panel->MoveOnFlg = FALSE;
      panel->MoveEndFlg = FALSE;
      panel->MoveFunc = _MoveFunc; //@todo
    }
*/
    //頂点初期化
    for (i=0;i<VTX_H_NUM*VTX_W_NUM;i++)
    {
      VecFx16 *vec;
      vec = &work->Vtx[i];
      vec->x = ((i%VTX_W_NUM)*FX16_ONE) /  MUL_VAL;
      vec->y = ((i/VTX_W_NUM)*FX16_ONE) /  MUL_VAL;
      vec->z = 0;
    }
  }
  return event;
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
static GMEVENT_RESULT EffEvt( GMEVENT* event, int* seq, void* work )
{
  ENCEFF2_WORK *evt_work = work;
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
      evt_work->Count++;
      
      if( evt_work->Count < 3 ){
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
      SetVramKey(evt_work);
      
      (*seq)++;
    }
    break;
  case 5:
    //セットアップ終了したので3Ｄ面もオン
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
    
    //フィールド表示モード切替
    FIELDMAP_SetDraw3DMode(fieldmap, DRAW3DMODE_ENCEFF);

    (*seq)++;
    break;
  case 6:
    {
      BOOL rc;
      rc = WaveMain(evt_work);
      if (rc)
      {
        //ホワイトアウト開始
        GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, 0 );
        evt_work->WaveWait = WAVE_AFTER_WAIT;
        (*seq)++;
      }
    }
    break;
  case 7:
    //ウェイト
    if ( evt_work->WaveWait != 0)
    {
      evt_work->WaveWait--;
      break;
    }

    //ホワイトアウト待ち
    if ( GFL_FADE_CheckFade() != FALSE ) break;

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
static void ReqCapture(ENCEFF2_WORK *evt_work)
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
static void SetVramKey(ENCEFF2_WORK *evt_work)
{
  GXVRamTex			vTex = GX_GetBankForTex();
  u32 VRAMoffs = 0x00000;
  u32						texSize	= 256*256*2;

  if(vTex |= GX_VRAM_C){ VRAMoffs += 0x20000; }
  if(vTex |= GX_VRAM_B){ VRAMoffs += 0x20000; }
  if(vTex |= GX_VRAM_A){ VRAMoffs += 0x20000; }

  evt_work->texKey = NNS_GfdMakeTexKey(VRAMoffs, texSize, FALSE);
}

//----------------------------------------------------------------------------
/**
 *	@brief	伝播処理
 *
 *	@param	GFL_TCB *p_tcb
 *	@param	*p_work 
 *
 *	@return BOOL TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL WaveMain(ENCEFF2_WORK *evt_work)
{
  int i,max;
  fx16 vtx[VTX_W_NUM*VTX_H_NUM];

  max = VTX_W_NUM*VTX_H_NUM;

  //現在の頂点情報を保存
  for(i=0;i<max;i++)
  {
/**    
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y)
    {
      evt_work->Vtx[i].z = 0;
      evt_work->TmpZ[i] = evt_work->Vtx[i].z;
    }
*/    
    vtx[i] = evt_work->Vtx[i].z;
  }

  if (evt_work->WaveWait == 0)
  {
    //波をたてる
    evt_work->Vtx[12*33+16].z = WAVE_HEIGHT;
    //ウェイトセット
    evt_work->WaveWait = WAVE_WAIT;
    evt_work->WaveCount--;
  }else evt_work->WaveWait--;
  
  //全ての頂点に計算をかける
  for(i=0;i<max;i++)
  {
    fx16 v1,v2,v3,v4;
    int idx;
    int base_w,base_h;
    int w,h;
		//2次座標に変換
		base_w = i%VTX_W_NUM;
		base_h = i/VTX_W_NUM;
    if (i==411){
      OS_Printf("411\n");
    }

    //周囲4つのデータを取得

    //左
    if (base_w-1>=0)
    {
      w = base_w-1;
      h = base_h;
      idx = h*VTX_W_NUM+w;
      v1 = vtx[idx];
    }
    else v1 = 0;
    //右
		if (base_w+1<VTX_W_NUM)
    {
      w = base_w+1;
      h = base_h;
      idx = h*VTX_W_NUM+w;
      v2 = vtx[idx];
    }
    else v2 = 0;
		//上
		if (base_h-1>=0)
    {
      h = base_h-1;
      w = base_w;
      idx = h*VTX_W_NUM+w;
      v3 = vtx[idx];
    }
    else v3 = 0;
		//下
		if(base_h+1<VTX_H_NUM)
    {
      h = base_h+1;
      w = base_w;
      idx = h*VTX_W_NUM+w;
      v4 = vtx[idx];
    }
    else v4 = 0;

    evt_work->TmpZ[i] += ( (v1+v2+v3+v4)/4 - evt_work->Vtx[i].z );

    evt_work->Vtx[i].z += evt_work->TmpZ[i];
    if ( evt_work->Vtx[i].z > FX16_ONE ) evt_work->Vtx[i].z = FX16_ONE;
    if ( evt_work->Vtx[i].z < -FX16_ONE ) evt_work->Vtx[i].z = -FX16_ONE;

    //縁は伝播対象からはずす処理（画面外を見えなくするため）
    {
      int base_w,base_h;
      //2次座標に変換
      base_w = i%VTX_W_NUM;
      base_h = i/VTX_W_NUM;

      if ( (base_w == 0) || (base_w == VTX_W_NUM-1) ||
          (base_h == 0) || (base_h == VTX_H_NUM-1))
      {
        evt_work->Vtx[i].z = 0;
      }
    }
  }

  if (evt_work->WaveCount<=0) return TRUE;

  return FALSE;

}


//--------------------------------------------------------------------------------------------
/**
 * 描画
 *
 * @param   ptr     管理ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ENCEFF_DrawEff2( ENCEFF_CNT_PTR ptr )
{
  DrawMesh(ptr);
}

//--------------------------------------------------------------------------------------------
/**
 * 描画
 *
 * @param   ptr     管理ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DrawMesh(ENCEFF_CNT_PTR ptr)
{
  int i,j;
  void *wk = ENCEFF_GetWork(ptr);

  ENCEFF2_WORK *work = (ENCEFF2_WORK*)wk;

  {
    VecFx32 camPos;
    GFL_G3D_CAMERA_GetPos( work->g3Dcamera, &camPos );
    camPos.z = 33*FX32_ONE;
    GFL_G3D_CAMERA_SetPos( work->g3Dcamera, &camPos );
  }

  G3X_Reset();
  {
    VecFx32		camPos, camUp, target, vecNtmp;
		MtxFx43		mtxCamera, mtxCameraInv;

		GFL_G3D_CAMERA_GetPos( work->g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( work->g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( work->g3Dcamera, &target );

		G3_LookAt( &camPos, &camUp, &target, &mtxCamera );	//mtxCameraには行列計算結果が返る
  }

  G3_PushMtx();

  G3_TexImageParam( GX_TEXFMT_DIRECT, GX_TEXGEN_TEXCOORD,
                    GX_TEXSIZE_S256, GX_TEXSIZE_T256,
                    GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
                    GX_TEXPLTTCOLOR0_USE, NNS_GfdGetTexKeyAddr(work->texKey) );

  G3_MaterialColorDiffAmb(
							GX_RGB(31, 31, 31),
							GX_RGB(31, 31, 31),
              FALSE
        );

  G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE);

  //ライトカラー
//  G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));
  // ポリゴンアトリビュート設定
	G3_PolygonAttr(
				   GX_LIGHTMASK_0123,			  // ライトを反映
				   GX_POLYGONMODE_MODULATE,	  // モジュレーションポリゴンモード
				   //GX_CULL_BACK,             // カリング
				   GX_CULL_NONE,             // カリング
				   0,                         // ポリゴンＩＤ ０
				   31,					  // アルファ値
				   GX_POLYGON_ATTR_MISC_NONE );


  {
    VecFx32 pos = {-FX32_ONE*16,FX32_ONE*12,0};
    VecFx32 scale = {FX32_ONE*MUL_VAL, FX32_ONE*MUL_VAL, FX32_ONE*Z_MUL_VAL};
    fx32  s0, t0, t1;

    // 位置設定
		G3_Translate(pos.x, pos.y, pos.z);
    // スケール設定
		G3_Scale(scale.x, scale.y, scale.z);

    for (i=0;i<POLY_H_NUM;i++)
    {
      G3_Begin( GX_BEGIN_QUAD_STRIP );
      for (j=0;j<VTX_W_NUM;j++)
      {
        fx16 x1,x2,y1,y2,z1,z2;
        

        s0 = j * 8 * FX32_ONE;
        t0 = i * 8 * FX32_ONE;
        t1 = t0 + (8 * FX32_ONE);
        
        x1 = work->Vtx[i*VTX_W_NUM+j].x;
        x2 = work->Vtx[(i+1)*VTX_W_NUM+j].x;
        y1 = work->Vtx[i*VTX_W_NUM+j].y;
        y2 = work->Vtx[(i+1)*VTX_W_NUM+j].y;
        z1 = work->Vtx[i*VTX_W_NUM+j].z;
        z2 = work->Vtx[(i+1)*VTX_W_NUM+j].z;

        G3_Normal( 0, 0, FX16_ONE );

        G3_TexCoord(s0, t0);
	      G3_Vtx( x1, -y1, z1 );
        G3_TexCoord(s0, t1);
	      G3_Vtx( x2, -y2, z2 );
      }
      G3_End();
    }
  }

  G3_PopMtx(1);
}


