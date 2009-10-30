//============================================================================================
/**
 * @file	screenTex.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

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
GFL_G3D_RES* GFL_SCRNTEX_CreateG3DresTex(HEAPID heapID, SCRNTEX_VRAM texVram)
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
		case SCRNTEX_VRAM_D:
			if(vTex |= GX_VRAM_C){ VRAMoffs += 0x20000; }
		case SCRNTEX_VRAM_C:
			if(vTex |= GX_VRAM_B){ VRAMoffs += 0x20000; }
		case SCRNTEX_VRAM_B:
			if(vTex |= GX_VRAM_A){ VRAMoffs += 0x20000; }
		case SCRNTEX_VRAM_A:
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
void GFL_SCRNTEX_Load(SCRNTEX_VRAM capVram, SCRNTEX_VRAM texVram)
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

//--------------------------------------------------------------------------------------------
//
//
// 画面描画(カメラターゲット = 原点)
//
//
//--------------------------------------------------------------------------------------------
void GFL_SCRNTEX_DrawDefault(GFL_G3D_OBJ* g3Dobj)
{
	VecFx32 camP = { 0, 0, 0x210 * FX32_ONE };
	VecFx32 camT = { 0, 0, 0 };
	VecFx32 camU = { 0, FX32_ONE, 0 };
	GFL_G3D_OBJSTATUS status = {
		{ 0, 0, 0 },																				//座標
		{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
	};
  fx16    s = FX_SinIdx(0x4000);
  fx16    c = FX_CosIdx(0x4000);

	NNS_G3dGlbLookAt(&camP, &camU, &camT);

	MTX_RotX33(&status.rotate, s, c);
	GFL_G3D_DRAW_DrawObject(g3Dobj, &status);
}



