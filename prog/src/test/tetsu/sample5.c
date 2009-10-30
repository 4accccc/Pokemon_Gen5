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

// gfl_captex.h
//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef enum {
	CAPTEX_VRAM_A = 0,
	CAPTEX_VRAM_B,
	CAPTEX_VRAM_C,
	CAPTEX_VRAM_D,
}CAPTEX_VRAM;

GFL_G3D_RES*	GFL_CAPTEX_CreateG3DresTex(HEAPID heapID, CAPTEX_VRAM texVram);
void					GFL_CAPTEX_Load(GFL_G3D_RES* g3DresTex, CAPTEX_VRAM capVram, CAPTEX_VRAM texVram);
// gfl_captex.h

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

//	GFL_G3D_UTIL*				g3Dutil;
//	u16									g3DutilUnitIdx;

	GFL_G3D_RES*				g3Dmdl;
	GFL_G3D_RES*				g3Dtex;
	GFL_G3D_RND*				g3Drnd;
	GFL_G3D_OBJ*				g3Dobj;

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
		sw->g3Dtex = GFL_CAPTEX_CreateG3DresTex(sw->heapID, CAPTEX_VRAM_B);
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
		sw->seq++;
		break;

	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){ sw->seq = 3; }

		if(sw->length < 0xf8){
			u16 angleH = DWS_GetCamAngleH(sw->dws);
			u16 angleV = DWS_GetCamAngleV(sw->dws);

			sw->length++;
			DWS_SetCamStatus(sw->dws, angleH, angleV, sw->length * FX32_ONE);
		} else {
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
				// キャプチャ
				GFL_CAPTEX_Load(sw->g3Dtex, CAPTEX_VRAM_D, CAPTEX_VRAM_B);
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
		// オブジェクト破棄
		GFL_G3D_OBJECT_Delete(sw->g3Dobj); 
		// レンダー破棄
		GFL_G3D_RENDER_Delete(sw->g3Drnd); 
		// リソース破棄
		GFL_G3D_DeleteResource(sw->g3Dtex);
		GFL_G3D_DeleteResource(sw->g3Dmdl);
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
//
//
// 画面キャプチャ→テクスチャ変換関数
//
//
//============================================================================================
//--------------------------------------------------------------------------------------------
//
//
// 画面キャプチャ画像格納テクスチャリソース作成
//
//
//--------------------------------------------------------------------------------------------
static const u8 directTexFileHeader[] = {
0x42, 0x54, 0x58, 0x30, 0xff, 0xfe, 0x01, 0x00, 0x8c, 0x00, 0x02, 0x00, 0x10, 0x00, 0x01, 0x00,
0x14, 0x00, 0x00, 0x00, 0x54, 0x45, 0x58, 0x30, 0x78, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x02, 0x00, 0x78, 0x00, 0x02, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x78, 0x00, 0x02, 0x00,
0x00, 0x01, 0x2c, 0x00, 0x08, 0x00, 0x10, 0x00, 0x7f, 0x01, 0x00, 0x00, 0x2e, 0x00, 0x01, 0x00,
0x08, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xd0, 0x1e, 0x00, 0x01, 0x08, 0x80, 0x64, 0x69, 0x72, 0x65,
0x63, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
0x08, 0x00, 0x0c, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00,
};
GFL_G3D_RES* GFL_CAPTEX_CreateG3DresTex(HEAPID heapID, CAPTEX_VRAM texVram)
{
	// VRAM_Dをイメージ対象とした256x256ダイレクトビットマップテクスチャ作成
	// nsbtxファイルヘッダを擬似作成した後TexKeyを指定VRAMに設定する
	GFL_G3D_RES* g3DresTex = GFL_HEAP_AllocMemory(heapID, GFL_G3D_GetResourceHeaderSize());
	void* file = GFL_HEAP_AllocMemory(heapID, sizeof(directTexFileHeader));
	{
		// ヘッダ内容のコピー
		GFL_STD_MemCopy(directTexFileHeader, file, sizeof(directTexFileHeader));
		// テクスチャリソース作成
		GFL_G3D_CreateResource(g3DresTex, GFL_G3D_RES_CHKTYPE_TEX, file);
	}
	{
		NNSG3dResTex*	NNSresTex = NNS_G3dGetTex(file);
		NNSGfdTexKey	texKey, tex4x4Key;
		u32						texSize	= NNS_G3dTexGetRequiredSize(NNSresTex);	// 256*256*2
		GXVRamTex			vTex = GX_GetBankForTex(); 
		u32						VRAMoffs = 0x00000;
	
		switch(texVram){
		case CAPTEX_VRAM_D:
			if(vTex |= GX_VRAM_C){ VRAMoffs += 0x20000; }
		case CAPTEX_VRAM_C:
			if(vTex |= GX_VRAM_B){ VRAMoffs += 0x20000; }
		case CAPTEX_VRAM_B:
			if(vTex |= GX_VRAM_A){ VRAMoffs += 0x20000; }
		case CAPTEX_VRAM_A:
			break;
		}
		texKey = NNS_GfdMakeTexKey(VRAMoffs, texSize, FALSE);
		tex4x4Key = 0;

		//リソースへのＶＲＡＭキーの設定
		NNS_G3dTexSetTexKey(NNSresTex, texKey, tex4x4Key);
	}
	return g3DresTex;
}

//--------------------------------------------------------------------------------------------
//
//
// 画面キャプチャを行いテクスチャに変換する
//
//
//--------------------------------------------------------------------------------------------
static const struct {
	u32     bankDef;
	u32     capDef;
	u32     vramDef;
} vramDefTbl[4] = {
	{ GX_VRAM_LCDC_A, GX_CAPTURE_DEST_VRAM_A_0x00000, HW_LCDC_VRAM_A },
	{ GX_VRAM_LCDC_B, GX_CAPTURE_DEST_VRAM_B_0x00000, HW_LCDC_VRAM_B },
	{ GX_VRAM_LCDC_C, GX_CAPTURE_DEST_VRAM_C_0x00000, HW_LCDC_VRAM_C },
	{ GX_VRAM_LCDC_D, GX_CAPTURE_DEST_VRAM_D_0x00000, HW_LCDC_VRAM_D },
};
void GFL_CAPTEX_Load(GFL_G3D_RES* g3DresTex, CAPTEX_VRAM capVram, CAPTEX_VRAM texVram)
{
	GXVRamTex vTex = GX_GetBankForTex(); 

	GX_SetBankForLCDC(vramDefTbl[capVram].bankDef);

	// キャプチャー設定（次の0ライン目からキャプチャー）
	GX_SetCapture(GX_CAPTURE_SIZE_256x192,  // Capture size
                GX_CAPTURE_MODE_A,				// Capture mode
                GX_CAPTURE_SRCA_2D3D,     // Blend src A
                GX_CAPTURE_SRCB_VRAM_0x00000,				// Blend src B
								vramDefTbl[capVram].capDef,
                16, 0);             // Blend parameter for src A, B

	OS_WaitVBlankIntr();	// 0ライン待ちウエイト
	OS_WaitVBlankIntr();	// キャプチャー待ちウエイト

	GX_DisableBankForLCDC();
	GX_SetBankForTex(vTex); 

	if(capVram == texVram){ return; }	// コピーの必要なし

	vTex = GX_ResetBankForTex(); 

	MI_CpuCopyFast
		((void*)vramDefTbl[capVram].vramDef, (void*)vramDefTbl[texVram].vramDef, 256*192*2); 

	GX_DisableBankForLCDC();
	GX_SetBankForTex(vTex); 
}


