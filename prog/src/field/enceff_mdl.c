//======================================================================
/*
 * @file	enceff1.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include <gflib.h>

#include "enceff.h"

#include "fieldmap.h"
#include "system/screentex.h"
#include "arc/texViewTest.naix"

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
}ENCEFF_MDL_WORK;

//======================================================================
//  proto
//======================================================================
static GMEVENT_RESULT ev_encEffectFunc( GMEVENT *event, int *seq, void *wk );

static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
                                  const int inMdl, const int inAnm  );


//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_MDL_Create1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork,
                           NARC_texViewTest_effect1_nsbmd,
                           NARC_texViewTest_effect1_nsbca  );
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
                                  const int inMdl, const int inAnm  )
{
  GMEVENT *event;
  ENCEFF_MDL_WORK *work;

  event = GMEVENT_Create( gsys, NULL, ev_encEffectFunc, sizeof(ENCEFF_MDL_WORK) );
  work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(ENCEFF_MDL_WORK) );
  work->fieldWork = fieldWork;
  work->MdlArcIdx = inMdl;
  work->AnmArcIdx = inAnm;

  return( event );

}

//--------------------------------------------------------------
/**
 * 描画関数
 * @param 
 * @retval none
 */
//--------------------------------------------------------------
void ENCEFF_MDL_Draw(ENCEFF_CNT_PTR ptr)
{
  void *wk = ENCEFF_GetWork(ptr);

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
  ENCEFF_MDL_WORK *work = wk;
  
  switch( (*seq) )
  {
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
      work->count++;
      
      if( work->count < 3 ){
        GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, -1 );
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

    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, 3 );
    (*seq)++;
    break;
	case 4:
		if(GFL_G3D_OBJECT_IncAnimeFrame(work->g3DobjEff, 0, FX32_ONE*2) == FALSE){
			(*seq)++;
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

