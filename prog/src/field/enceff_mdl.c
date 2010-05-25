//======================================================================
/*
 * @file	enceff_mdl.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include <gflib.h>

#include "enceff.h"

#include "fieldmap.h"
#include "system/screentex.h"
#include "arc/enceff_mdl.naix"

#include "system/main.h"

#define FADE_MARGINE  (20*FX32_ONE)

#define ENC_SE_NONE (0xffffffff)

//--------------------------------------------------------------
/// ENCEFF_MDL_WORK
//--------------------------------------------------------------
typedef struct
{
  int count;
  FIELDMAP_WORK *fieldWork;

	GXVRamTex						vTex;
	BOOL								vTexRecover;
	DRAW3DMODE					dMode;
	GFL_G3D_RES*				g3DmdlEff;
	GFL_G3D_RES*				g3DtexEff;
	GFL_G3D_RES*				g3DicaEff;
	GFL_G3D_RND*				g3DrndEff;
	GFL_G3D_ANM*				g3DanmEff;
	GFL_G3D_OBJ*				g3DobjEff;
  int MdlArcIdx;
  int AnmArcIdx;
  int Fade;
  BOOL FadeStart;
  fx32 FadeStartFrm;
  u32 SE;
}ENCEFF_MDL_WORK;

//======================================================================
//  proto
//======================================================================
static GMEVENT_RESULT ev_encEffectFunc( GMEVENT *event, int *seq, void *wk );

static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
                                  const int inMdl, const int inAnm, const BOOL inIsFadeWhite  );


//--------------------------------------------------------------
/**
 * イベント作成  通常野生
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_enceff_mdl_ee_yasei1_nsbmd,
                           NARC_enceff_mdl_ee_yasei1_nsbca,
                           inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成 野生　強
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create2(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_enceff_mdl_ee_yasei2_nsbmd,
                           NARC_enceff_mdl_ee_yasei2_nsbca,
                           inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成   トレーナー通常
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create3(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_enceff_mdl_ee_trnor_nsbmd,
                           NARC_enceff_mdl_ee_trnor_nsbca,
                           inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成   トレーナー水上
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create4(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_enceff_mdl_ee_trmizu_nsbmd,
                           NARC_enceff_mdl_ee_trmizu_nsbca,
                           inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成  エフェクト
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create5(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_enceff_mdl_ee_symbol_nsbmd,
                           NARC_enceff_mdl_ee_symbol_nsbca,
                           inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成   ゾロアーク
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create6(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_enceff_mdl_ee_zoroark_nsbmd,
                           NARC_enceff_mdl_ee_zoroark_nsbca,
                           inIsFadeWhite );
  //SEセット
  {
    ENCEFF_MDL_WORK *work;
    ENCEFF_CNT_PTR cnt_ptr;
    cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldWork);
    work = ENCEFF_GetUserWorkPtr(cnt_ptr);
    work->SE = SEQ_SE_ZORO_01;
  }
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成共通
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param inMdl   モデルアーカイブインデックス
 * @param inAnm   アニメアーカイブインデックス
 *
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
                                  const int inMdl, const int inAnm, const BOOL inIsFadeWhite  )
{
  ENCEFF_CNT_PTR cnt_ptr;
  GMEVENT *event;
  ENCEFF_MDL_WORK *work;
  int size;

  size = sizeof(ENCEFF_MDL_WORK);

  //ワークを確保
  {
    FIELDMAP_WORK * fieldmap;
    fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
    cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldmap);
  }
  work = ENCEFF_AllocUserWork(cnt_ptr, size, HEAPID_FLD3DCUTIN);

  event = GMEVENT_Create( gsys, NULL, ev_encEffectFunc, 0 );
  work->fieldWork = fieldWork;
  work->MdlArcIdx = inMdl;
  work->AnmArcIdx = inAnm;
  work->SE = ENC_SE_NONE;
  if (inIsFadeWhite) work->Fade = GFL_FADE_MASTER_BRIGHT_WHITEOUT;
  else work->Fade = GFL_FADE_MASTER_BRIGHT_BLACKOUT;

  return( event );

}

//--------------------------------------------------------------
/**
 * 描画関数
 * @param     ptr   エフェクトポインタ
 * @retval none
 */
//--------------------------------------------------------------
void ENCEFF_MDL_Draw(ENCEFF_CNT_PTR ptr)
{
  void *wk = ENCEFF_GetUserWorkPtr(ptr);

  ENCEFF_MDL_WORK *work = (ENCEFF_MDL_WORK*)wk;

	GFL_SCRNTEX_DrawDefault(work->g3DobjEff);
}

//--------------------------------------------------------------
/**
 * イベント　エンカウントエフェクト
 * @param event GMEVENT
 * @param seq イベントシーケンス
 * @param wk イベントワーク
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_encEffectFunc( GMEVENT *event, int *seq, void *wk )
{
  ENCEFF_MDL_WORK *work;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  ENCEFF_CNT_PTR cnt_ptr;

  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldmap);
  work = ENCEFF_GetUserWorkPtr(cnt_ptr);
  
  switch( (*seq) )
  {
  case 0:
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, FLASH_SPEED );
    (*seq)++;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE ){
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, FLASH_SPEED );
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_FADE_CheckFade() == FALSE ){
      work->count++;
      
      if( work->count < FLASH_COUNT ){
        GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, FLASH_SPEED );
        (*seq) = 1;
      }else{
				(*seq)++;
      }
    }
    break;
  case 3:
		// VRAMバンクセットアップ(VRAM_Dをテクスチャとして使用)
		work->vTex = GX_GetBankForTex(); 
		work->vTexRecover = FALSE;
		if(!(work->vTex & GX_VRAM_D)){
			work->vTexRecover = TRUE;
			work->vTex |= GX_VRAM_D;
			GX_SetBankForTex(work->vTex); 
		}

		// リソース作成
		work->g3DmdlEff = GFL_G3D_CreateResourceArc(ARCID_CAPTEX, work->MdlArcIdx);
		work->g3DicaEff = GFL_G3D_CreateResourceArc(ARCID_CAPTEX, work->AnmArcIdx);
		{
			HEAPID heapID = FIELDMAP_GetHeapID(work->fieldWork);
			work->g3DtexEff = GFL_SCRNTEX_CreateG3DresTex(heapID, SCRNTEX_VRAM_D);
		}
		// レンダー作成
		work->g3DrndEff = GFL_G3D_RENDER_Create(work->g3DmdlEff, 0, work->g3DtexEff); 
		// アニメ作成
		work->g3DanmEff = GFL_G3D_ANIME_Create(work->g3DrndEff, work->g3DicaEff, 0); 
		// オブジェクト作成
		work->g3DobjEff = GFL_G3D_OBJECT_Create(work->g3DrndEff, &work->g3DanmEff, 1); 
		GFL_G3D_OBJECT_EnableAnime(work->g3DobjEff, 0); 

		// スクリーンテクスチャ作成
		GFL_SCRNTEX_Load(SCRNTEX_VRAM_D, SCRNTEX_VRAM_D);	// 他VRAMバンクへの転送はなし

		work->dMode = FIELDMAP_GetDraw3DMode(work->fieldWork);
		FIELDMAP_SetDraw3DMode(work->fieldWork, DRAW3DMODE_ENCEFF);
    
    work->FadeStart = FALSE;

    {
      fx32 maxfrm;
      GFL_G3D_OBJECT_GetAnimeFrameMax( work->g3DobjEff, 0, (int*)&maxfrm );
      work->FadeStartFrm = maxfrm - FADE_MARGINE;
      NOZOMU_Printf("mdl enceff maxfrm = %x %d\n",maxfrm, maxfrm/FX32_ONE);
    }

    //SE再生
    if (work->SE != ENC_SE_NONE)
    {
      PMSND_PlaySE(work->SE);
    }
    (*seq)++;
    break;
	case 4:
    {
      fx32 anmFrm;
      int frm;
      GFL_G3D_OBJECT_GetAnimeFrame( work->g3DobjEff, 0, &frm );
      anmFrm = (fx32)frm;
      if ( (!work->FadeStart) && (anmFrm >= work->FadeStartFrm) )
      {
        GFL_FADE_SetMasterBrightReq(work->Fade, 0, 16, 3 );  //両画面フェードアウト
        work->FadeStart = TRUE;
      }

      if ( (GFL_G3D_OBJECT_IncAnimeFrame(work->g3DobjEff, 0, FX32_ONE) == FALSE) &&
           (GFL_FADE_CheckFade() == FALSE) )
      {
        (*seq)++;
      }
		}
    break;
	case 5:
		// VRAMバンク復帰
		if(work->vTexRecover == TRUE){
			work->vTex &= (GX_VRAM_D^0xffffffff);
			GX_SetBankForTex(work->vTex); 
		}
		// 使用リソース関連破棄
		GFL_G3D_OBJECT_Delete(work->g3DobjEff); 
		GFL_G3D_ANIME_Delete(work->g3DanmEff); 
		GFL_G3D_RENDER_Delete(work->g3DrndEff); 
		GFL_G3D_DeleteResource(work->g3DicaEff);
		GFL_G3D_DeleteResource(work->g3DtexEff);
		GFL_G3D_DeleteResource(work->g3DmdlEff);

		FIELDMAP_SetDraw3DMode(work->fieldWork, work->dMode);
    return( GMEVENT_RES_FINISH );

  }
  
  return( GMEVENT_RES_CONTINUE );
}

