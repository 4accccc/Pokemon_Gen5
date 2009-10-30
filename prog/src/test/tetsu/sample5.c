//============================================================================================
/**
 * @file	sample5.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

#include "debug_common.h"
#include "system/screentex.h"

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	
 *
 *
 *
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	HEAPID							heapID;
	DWS_SYS*						dws;

	int									seq;

	GFL_G3D_RES*				g3Dmdl;
	GFL_G3D_RES*				g3Dtex;
	GFL_G3D_RND*				g3Drnd;
	GFL_G3D_OBJ*				g3Dobj;

	GFL_G3D_RES*				g3DmdlBase;
	GFL_G3D_RES*				g3DicaBase;
	GFL_G3D_RND*				g3DrndBase;
	GFL_G3D_ANM*				g3DanmBase;
	GFL_G3D_OBJ*				g3DobjBase;

	int									timer;
	int									counter;
	int									length;
}Sample5_WORK;

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	呼び出し用PROC定義
 *
 *
 *
 *
 *
 */
//============================================================================================
static BOOL	sample5(Sample5_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT Sample5Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	Sample5_WORK* sw = NULL;

	sw = GFL_PROC_AllocWork(proc, sizeof(Sample5_WORK), GFL_HEAPID_APP);
  GFL_STD_MemClear(sw, sizeof(Sample5_WORK));
  sw->heapID = GFL_HEAPID_APP;
	sw->seq = 0;

	sw->dws = DWS_SYS_Setup(sw->heapID);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample5Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	Sample5_WORK*	sw;
	sw = mywk;

	DWS_SYS_Framework(sw->dws);

	sw->timer++;
	if(sample5(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample5Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	Sample5_WORK*	sw;
	sw = mywk;

	DWS_SYS_Delete(sw->dws);

	GFL_PROC_FreeWork(proc);

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeSample5ProcData = {
	Sample5Proc_Init,
	Sample5Proc_Main,
	Sample5Proc_End,
};





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	メイン
 *
 *
 *
 *
 *
 */
//============================================================================================
#include "arc/texViewTest.naix"

static const GFL_G3D_OBJSTATUS g3DobjStatus1 = {
	{ 0, 0, 0 },																				//座標
	{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
};

static const u16 backGroundColorTable[] = {
	GX_RGB(0x1f,0x00,0x00), GX_RGB(0x1f,0x00,0x1f), GX_RGB(0x1f,0x1f,0x00), GX_RGB(0x1f,0x1f,0x1f),
	GX_RGB(0x00,0x00,0x00), GX_RGB(0x00,0x00,0x1f), GX_RGB(0x00,0x1f,0x00), GX_RGB(0x00,0x1f,0x1f),
};

//============================================================================================
static BOOL	sample5(Sample5_WORK* sw)
{
	switch(sw->seq){

	case 0:
		// リソース作成
		sw->g3Dmdl = GFL_G3D_CreateResourceArc(ARCID_CAPTEX, NARC_texViewTest_texViewTest_nsbmd);
		sw->g3Dtex = GFL_SCRNTEX_CreateG3DresTex(sw->heapID, SCRNTEX_VRAM_B);
		// レンダー作成
		sw->g3Drnd = GFL_G3D_RENDER_Create(sw->g3Dmdl, 0, sw->g3Dtex); 
		// オブジェクト作成
		sw->g3Dobj = GFL_G3D_OBJECT_Create(sw->g3Drnd, NULL, 0); 
		{
			// 初期画像転送
			GXVRamTex vTex = GX_GetBankForTex(); 
			NNSG3dResFileHeader* header = GFL_ARC_LoadDataAlloc
						(ARCID_CAPTEX, NARC_texViewTest_texViewTest_nsbtx, GetHeapLowID(sw->heapID));
			NNSG3dResTex*	resTex = NNS_G3dGetTex(header);
			u32 dataAdrs = (u32)resTex + resTex->texInfo.ofsTex;

			GX_SetBankForLCDC(GX_VRAM_LCDC_B);
			MI_CpuCopy((void*)dataAdrs, (void*)HW_LCDC_VRAM_B, 256*192*2); 
			GFL_HEAP_FreeMemory(header);

			GX_DisableBankForLCDC();
			GX_SetBankForTex(vTex); 
		}
		sw->g3DmdlBase = GFL_G3D_CreateResourceArc(ARCID_CAPTEX, NARC_texViewTest_effect1_nsbmd);
		sw->g3DicaBase = GFL_G3D_CreateResourceArc(ARCID_CAPTEX, NARC_texViewTest_effect1_nsbca);
		sw->g3DrndBase = GFL_G3D_RENDER_Create(sw->g3DmdlBase, 0, sw->g3Dtex); 
		sw->g3DanmBase = GFL_G3D_ANIME_Create(sw->g3DrndBase, sw->g3DicaBase, 0); 
		sw->g3DobjBase = GFL_G3D_OBJECT_Create(sw->g3DrndBase, &sw->g3DanmBase, 1); 
		GFL_G3D_OBJECT_EnableAnime(sw->g3DobjBase, 0); 

		sw->counter = 0;
		sw->seq++;
		break;

	case 1:
		GFL_BG_SetBackGroundColor(GFL_BG_FRAME3_M, backGroundColorTable[sw->counter]);
		sw->counter++;
		sw->counter &= 7;

		sw->timer = 0;
		sw->length = 0x0098;
		DWS_SetCamStatus(sw->dws, 0, 0, sw->length * FX32_ONE);

		GFL_G3D_OBJECT_ResetAnimeFrame(sw->g3DobjBase, 0);

		sw->seq++;
		break;

	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){ sw->seq = 3; break; }
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){ sw->seq = 10; break; }

		if(sw->length < 0xf8){
			u16 angleH = DWS_GetCamAngleH(sw->dws);
			u16 angleV = DWS_GetCamAngleV(sw->dws);

			sw->length++;
			DWS_SetCamStatus(sw->dws, angleH, angleV, sw->length * FX32_ONE);
		} else {
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
				// キャプチャ
				GFL_SCRNTEX_Load(SCRNTEX_VRAM_D, SCRNTEX_VRAM_B);
				sw->seq = 1;
				break;
			}
		}

		//３Ｄ描画
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		
		{
			GFL_G3D_OBJSTATUS g3DobjStatus = g3DobjStatus1;
			MtxFx33	tmp;
			u16			rotate = sw->timer * 256;
      fx16    s = FX_SinIdx(rotate);
      fx16    c = FX_CosIdx(rotate);

			MTX_RotX33(&g3DobjStatus.rotate, s, c);
			MTX_RotY33(&tmp, s, c);
			MTX_Concat33(&g3DobjStatus.rotate, &tmp, &g3DobjStatus.rotate);
			MTX_RotZ33(&tmp, s, c);
			MTX_Concat33(&g3DobjStatus.rotate, &tmp, &g3DobjStatus.rotate);

			GFL_G3D_DRAW_DrawObject(sw->g3Dobj, &g3DobjStatus);
		}
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					

		sw->timer++;
		break;

	case 3:
		GFL_G3D_OBJECT_Delete(sw->g3DobjBase); 
		GFL_G3D_ANIME_Delete(sw->g3DanmBase); 
		GFL_G3D_RENDER_Delete(sw->g3DrndBase); 
		GFL_G3D_DeleteResource(sw->g3DicaBase);
		GFL_G3D_DeleteResource(sw->g3DmdlBase);

		// オブジェクト破棄
		GFL_G3D_OBJECT_Delete(sw->g3Dobj); 
		// レンダー破棄
		GFL_G3D_RENDER_Delete(sw->g3Drnd); 
		// リソース破棄
		GFL_G3D_DeleteResource(sw->g3Dtex);
		GFL_G3D_DeleteResource(sw->g3Dmdl);
		return FALSE;

	case 10:
#if 0
		DWS_SetCamStatus(sw->dws, 0, 0, 0x210 * FX32_ONE);

		//３Ｄ描画
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		
		{
			GFL_G3D_OBJSTATUS g3DobjStatus = g3DobjStatus1;
			MtxFx33	tmp;
			u16			rotate = 0x4000;
      fx16    s = FX_SinIdx(rotate);
      fx16    c = FX_CosIdx(rotate);

			MTX_RotX33(&g3DobjStatus.rotate, s, c);
			GFL_G3D_DRAW_DrawObject(sw->g3DobjBase, &g3DobjStatus);
		}
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
#else
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_SCRNTEX_DrawDefault(sw->g3DobjBase);
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
#endif
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A ){
			if(GFL_G3D_OBJECT_IncAnimeFrame(sw->g3DobjBase, 0, FX32_ONE) == FALSE){
				sw->seq = 1;
			}
		}
		break;
	}
	return TRUE;
}

