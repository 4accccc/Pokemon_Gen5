//============================================================================================
/**
 * @file	sample6.c
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
#include "system/poke2dgra.h"

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
static void* PMVEFF_MakeEffectImage(u8* image, u16 sizX, u16 sizY, HEAPID heapID, BOOL imgFree);
static void* PMVEFF_MakeEffectPalette(u16* pltt, HEAPID heapID, BOOL plttFree);

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
	int									timer;
	int									pokeNum;
	//GFL_BMPWIN*					bmpwin;

	NNSGfdTexKey				texVramKey;		//テクスチャＶＲＡＭキー
	NNSGfdPlttKey				plttVramKey;	//パレットＶＲＡＭキー
	NNSGfdTexKey				texVramKeyD;	//ダイレクトテクスチャＶＲＡＭキー
	NNSGfdPlttKey				plttVramKey256;	//パレットＶＲＡＭキー

	GFL_BMP_DATA*				pokeImgBitmap;
	GFL_BMP_DATA*				pokeImgBitmap2;
	u16									pokePltt[16];

//	u16*								pokeDirectBitmap;
//	u16*								pokeDirectBitmap2;
	u16*								effectBitmap;
	u16*								effectBitmap2;
	u16*								effectPalette;
	BOOL								viewMode;
	BOOL								pixelMode;
}sample6_WORK;

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
static BOOL	sample6(sample6_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT sample6Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	sample6_WORK* sw = NULL;

	sw = GFL_PROC_AllocWork(proc, sizeof(sample6_WORK), GFL_HEAPID_APP);
  GFL_STD_MemClear(sw, sizeof(sample6_WORK));
  sw->heapID = GFL_HEAPID_APP;
	sw->seq = 0;

	sw->dws = DWS_SYS_Setup(sw->heapID);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT sample6Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	sample6_WORK*	sw;
	sw = mywk;

	DWS_SYS_Framework(sw->dws);

	sw->timer++;
	if(sample6(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT sample6Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	sample6_WORK*	sw;
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
const GFL_PROC_DATA DebugWatanabeSample6ProcData = {
	sample6Proc_Init,
	sample6Proc_Main,
	sample6Proc_End,
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
#define TEXSIZ_S			(256)
#define TEXSIZ_T			(256)
#define TEXVRAMSIZ		(TEXSIZ_S/8 * TEXSIZ_T/8 * 0x20)	// chrNum_x * chrNum_y * chrSiz
#define TEXVRAMSIZD		(TEXSIZ_S * TEXSIZ_T * 2)					// dot_x * dot_y * colSiz
static void draw3Dview(sample6_WORK* sw);
static void getBitmapView(sample6_WORK* sw);
#if 0
static void makeEffectBitmap(sample6_WORK* sw, u16 bkColor);
static void makeEffectBitmap2(sample6_WORK* sw, u16 bkColor);
static void makeEffectPalette(sample6_WORK* sw);
#endif

static BOOL	sample6(sample6_WORK* sw)
{
	switch(sw->seq){

	case 0:
		sw->timer = 0;
		sw->pokeNum = 6;
#if 0
		//テクスチャVRAM確保
		sw->texVramKey = NNS_GfdAllocTexVram(TEXVRAMSIZ, FALSE, 0);
		sw->plttVramKey = NNS_GfdAllocPlttVram(16*2, FALSE, 0);
		sw->texVramKeyD = NNS_GfdAllocTexVram(TEXVRAMSIZD, FALSE, 0);
		sw->plttVramKey256 = NNS_GfdAllocPlttVram(256*2, FALSE, 0);

		//テクスチャ用格納bitmap作成 pmViewSize x = 12chr y = 12chr
		sw->pokeImgBitmap = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, sw->heapID);
		sw->pokeImgBitmap2 = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, sw->heapID);
		//ダイレクトテクスチャbitmap作成 pmViewSize x = 12chr y = 12chr
		sw->pokeDirectBitmap = GFL_HEAP_AllocClearMemory(sw->heapID, TEXVRAMSIZD);
		sw->pokeDirectBitmap2 = GFL_HEAP_AllocClearMemory(sw->heapID, TEXVRAMSIZD);
		//テクスチャ加工用bitmap作成 pmViewSize x = 12chr y = 12chr
		sw->effectBitmap = GFL_HEAP_AllocClearMemory(sw->heapID, TEXVRAMSIZD);
		sw->effectBitmap2 = GFL_HEAP_AllocClearMemory(sw->heapID, TEXVRAMSIZD);
		//256色加工用palette作成
		sw->effectPalette = NULL;
#else
		//テクスチャVRAM確保
		sw->texVramKey = NNS_GfdAllocTexVram(TEXVRAMSIZ, FALSE, 0);
		sw->plttVramKey = NNS_GfdAllocPlttVram(16*2, FALSE, 0);
		sw->texVramKeyD = NNS_GfdAllocTexVram(TEXVRAMSIZ*2, FALSE, 0);
		sw->plttVramKey256 = NNS_GfdAllocPlttVram(256*2, FALSE, 0);

		//テクスチャ用格納bitmap作成 pmViewSize x = 12chr y = 12chr
		sw->pokeImgBitmap = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, sw->heapID);

		sw->effectBitmap = NULL;
		sw->effectPalette = NULL;
#endif
		sw->viewMode = FALSE;
		sw->pixelMode = FALSE;
		sw->seq++;
		break;

	case 1:
#if 0
		if(sw->pixelMode == FALSE){
			//GFL_BG_SetBackGroundColor(GFL_BG_FRAME3_M, GX_RGB(0x1f, 0x00, 0x00));
		} else {
			//GFL_BG_SetBackGroundColor(GFL_BG_FRAME3_M, GX_RGB(0x00, 0x00, 0x1f));
		}
		//画像変換
		getBitmapView(sw);
		makeEffectBitmap(sw, *(u16*)HW_BG_PLTT);
		makeEffectBitmap2(sw, *(u16*)HW_BG_PLTT);
		makeEffectPalette(sw);
		if(sw->effectPalette){ GFL_HEAP_FreeMemory(sw->effectPalette); }
		sw->effectPalette = PMVEFF_MakeEffectPalette(sw->pokePltt, sw->heapID);
#else
		getBitmapView(sw);
		if(sw->effectBitmap)	{ GFL_HEAP_FreeMemory(sw->effectBitmap); }
		if(sw->effectPalette)	{ GFL_HEAP_FreeMemory(sw->effectPalette); }
		{
			u8* image = GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap);
			sw->effectBitmap = PMVEFF_MakeEffectImage(image, 256, 256, sw->heapID, FALSE);
			sw->effectPalette = PMVEFF_MakeEffectPalette(sw->pokePltt, sw->heapID, FALSE);
		}
#endif
		DWS_SetCamStatus(sw->dws, 0, 0, 32 * FX32_ONE);
		sw->seq++;

	case 2:
#if 0
		//テクスチャVRAM転送
		{
			void*	src;
			u32		dst;

			if(sw->pixelMode == FALSE){
				src = (void*)sw->effectBitmap;
				dst = NNS_GfdGetTexKeyAddr(sw->texVramKeyD);
				NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZD);

				src = (void*)GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap);
				dst = NNS_GfdGetTexKeyAddr(sw->texVramKey);
				NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZ);

			} else {
				src = (void*)sw->effectBitmap2;
				dst = NNS_GfdGetTexKeyAddr(sw->texVramKeyD);
				NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZD);

				src = (void*)GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap2);
				dst = NNS_GfdGetTexKeyAddr(sw->texVramKey);
				NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZ);
			}
		}
		//パレットVRAM転送
		{
			void*	src;
			u32		dst;

			src = (void*)sw->pokePltt;
			dst = NNS_GfdGetPlttKeyAddr(sw->plttVramKey);
			NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, PLTT_SIZ);

			src = (void*)sw->effectPalette;
			dst = NNS_GfdGetPlttKeyAddr(sw->plttVramKey256);
			NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, PLTT_SIZ*16);
		}
#else
		{
			void*	src;
			u32		dst;

			src = (void*)GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap);
			dst = NNS_GfdGetTexKeyAddr(sw->texVramKey);
			NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZ);

			src = (void*)sw->effectBitmap;
			dst = NNS_GfdGetTexKeyAddr(sw->texVramKeyD);
			NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZ*2);

			src = (void*)sw->pokePltt;
			dst = NNS_GfdGetPlttKeyAddr(sw->plttVramKey);
			NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, PLTT_SIZ);

			src = (void*)sw->effectPalette;
			dst = NNS_GfdGetPlttKeyAddr(sw->plttVramKey256);
			NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, PLTT_SIZ*16);
		}
#endif
		sw->seq++;
		break;

	case 3:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){ sw->seq++; break; }
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
			if(sw->pokeNum < 9){ sw->pokeNum++; sw->seq=1; break; }
		}
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
			if(sw->pokeNum > 1){ sw->pokeNum--; sw->seq=1; break; }
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			sw->pixelMode = (sw->pixelMode == TRUE)? FALSE : TRUE;
			sw->seq = 2;
			break;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
			sw->viewMode = (sw->viewMode == TRUE)? FALSE : TRUE;
		}

		//３Ｄ描画
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		
		draw3Dview(sw);
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）	
		break;

	case 4:
#if 0
		GFL_HEAP_FreeMemory(sw->effectPalette);

		GFL_HEAP_FreeMemory(sw->effectBitmap2);
		GFL_HEAP_FreeMemory(sw->effectBitmap);
		GFL_HEAP_FreeMemory(sw->pokeDirectBitmap2);
		GFL_HEAP_FreeMemory(sw->pokeDirectBitmap);
		GFL_BMP_Delete(sw->pokeImgBitmap2);
		GFL_BMP_Delete(sw->pokeImgBitmap);

		NNS_GfdFreePlttVram(sw->plttVramKey256);
		NNS_GfdFreeTexVram(sw->texVramKeyD);
		NNS_GfdFreePlttVram(sw->plttVramKey);
		NNS_GfdFreeTexVram(sw->texVramKey);
#else
		if(sw->effectBitmap)	{ GFL_HEAP_FreeMemory(sw->effectBitmap); }
		if(sw->effectPalette)	{ GFL_HEAP_FreeMemory(sw->effectPalette); }

		GFL_BMP_Delete(sw->pokeImgBitmap);

		NNS_GfdFreePlttVram(sw->plttVramKey256);
		NNS_GfdFreeTexVram(sw->texVramKeyD);
		NNS_GfdFreePlttVram(sw->plttVramKey);
		NNS_GfdFreeTexVram(sw->texVramKey);
#endif
		return FALSE;
	}
	return TRUE;
}


//============================================================================================
static void draw3Dview(sample6_WORK* sw)
{
	fx32 s0, s1, t0, t1;

	G3_PushMtx();
	//平行移動パラメータ設定
	G3_Translate(0, 0, 0);
#if 0
	//グローバルスケール設定
	{
		fx32 scale;
		if(sw->pixelMode == FALSE){
			scale = FX32_ONE*32;
			s0 = 0;
			s1 = TEXSIZ_S*FX32_ONE;
			t0 = 0;
			t1 = TEXSIZ_T*FX32_ONE;
		} else {
			scale = FX32_ONE*16;
			s0 = TEXSIZ_S*FX32_ONE/2 - TEXSIZ_S*FX32_ONE/4;
			s1 = TEXSIZ_S*FX32_ONE/2 + TEXSIZ_S*FX32_ONE/4;
			t0 = 0;
			t1 = TEXSIZ_T*FX32_ONE;
		}
		G3_Scale(scale, scale, scale);
	}
	{
		G3_MtxMode(GX_MTXMODE_TEXTURE);
		G3_Identity();
		G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
	}

	if(sw->viewMode == TRUE){
		G3_TexImageParam(	GX_TEXFMT_DIRECT, GX_TEXGEN_TEXCOORD,
											GX_TEXSIZE_S256, GX_TEXSIZE_T256, 
											GX_TEXREPEAT_ST, GX_TEXFLIP_NONE,
											GX_TEXPLTTCOLOR0_TRNS, NNS_GfdGetTexKeyAddr(sw->texVramKeyD) );
	} else {
		G3_TexImageParam(	GX_TEXFMT_PLTT16, GX_TEXGEN_TEXCOORD,
											GX_TEXSIZE_S256, GX_TEXSIZE_T256, 
											GX_TEXREPEAT_ST, GX_TEXFLIP_NONE,
											GX_TEXPLTTCOLOR0_TRNS, NNS_GfdGetTexKeyAddr(sw->texVramKey) );
		G3_TexPlttBase( NNS_GfdGetPlttKeyAddr(sw->plttVramKey), GX_TEXFMT_PLTT16 );
	}
#else
	//グローバルスケール設定
	s0 = 0;
	s1 = TEXSIZ_S*FX32_ONE;
	t0 = 0;
	t1 = TEXSIZ_T*FX32_ONE;
	{
		fx32 scale = FX32_ONE*32;
		G3_Scale(scale, scale, scale);
	}
	{
		G3_MtxMode(GX_MTXMODE_TEXTURE);
		G3_Identity();
		G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
	}

	if(sw->viewMode == TRUE){
		G3_TexImageParam(	GX_TEXFMT_PLTT256, GX_TEXGEN_TEXCOORD,
											GX_TEXSIZE_S256, GX_TEXSIZE_T256, 
											GX_TEXREPEAT_ST, GX_TEXFLIP_NONE,
											GX_TEXPLTTCOLOR0_TRNS, NNS_GfdGetTexKeyAddr(sw->texVramKeyD) );
		G3_TexPlttBase( NNS_GfdGetPlttKeyAddr(sw->plttVramKey256), GX_TEXFMT_PLTT256 );
	} else {
		G3_TexImageParam(	GX_TEXFMT_PLTT16, GX_TEXGEN_TEXCOORD,
											GX_TEXSIZE_S256, GX_TEXSIZE_T256, 
											GX_TEXREPEAT_ST, GX_TEXFLIP_NONE,
											GX_TEXPLTTCOLOR0_TRNS, NNS_GfdGetTexKeyAddr(sw->texVramKey) );
		G3_TexPlttBase( NNS_GfdGetPlttKeyAddr(sw->plttVramKey), GX_TEXFMT_PLTT16 );
	}
#endif
	//マテリアル設定
	G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE);
	G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE);
	G3_PolygonAttr(	GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);
		
	G3_Begin( GX_BEGIN_QUADS );
	
	G3_TexCoord(s0, t0);
	G3_Vtx(-(FX16_ONE-1), (FX16_ONE-1), 0);

	G3_TexCoord(s0, t1);
	G3_Vtx(-(FX16_ONE-1), -(FX16_ONE-1), 0);

	G3_TexCoord(s1, t1);
	G3_Vtx((FX16_ONE-1), -(FX16_ONE-1), 0);

	G3_TexCoord(s1, t0);
	G3_Vtx((FX16_ONE-1), (FX16_ONE-1), 0);
	
	G3_End();
	G3_PopMtx(1);
}

//============================================================================================
// ポケモングラフィックデータをbitmapに変換取得
static void getBitmapView(sample6_WORK* sw)
{
	u32 chrOffs = 1;
	// 既存関数を利用してBGキャラクターデータとしてBGVRAMに展開
	POKE2DGRA_BG_TransResource(	sw->pokeNum,				// int mons_no
															0,									// int form_no
															0,									// int sex
															0,									// int rare
															POKEGRA_DIR_BACK,		// int dir
                              0,
                              0,
															TEXT_FRAME_M,				// u32 frm
															chrOffs,						// u32 chr_offs
															TEXT_PLTTID,				// u32 plt_offs
															sw->heapID );
	// VRAMからBGbitmapデータとして変換拡大取得
	{
		GFL_BMP_DATA* bmpTmp = GFL_BMP_Create(12, 12, GFL_BMP_16_COLOR, sw->heapID);
		GFL_BMP_DATA* bmpTmp2 = GFL_BMP_Create(24, 24, GFL_BMP_16_COLOR, sw->heapID);
		{
			// VRAM → tmp
			// BGbank = BCVRAM_C
			GXVRamBG	vBG = GX_ResetBankForBG();
			u8*				src = (u8*)(HW_LCDC_VRAM_C + chrOffs*0x20);
			u8*				dst = GFL_BMP_GetCharacterAdrs(bmpTmp);
			int				chrNum = 0;
			int				x, y;

			//8x8,4x8,8x4,4x4の順に並んでいるので並び替え
			for(y=0; y<8; y++){
				for(x=0; x<8; x++){
					GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), &dst[(y*12+x)*0x20], 0x20);
					chrNum++;
				}
			}
			for(y=0; y<8; y++){
				for(x=8; x<12; x++){
					GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), &dst[(y*12+x)*0x20], 0x20);
					chrNum++;
				}
			}
			for(y=8; y<12; y++){
				for(x=0; x<8; x++){
					GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), &dst[(y*12+x)*0x20], 0x20);
					chrNum++;
				}
			}
			for(y=8; y<12; y++){
				for(x=8; x<12; x++){
					GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), &dst[(y*12+x)*0x20], 0x20);
					chrNum++;
				}
			}
			GX_DisableBankForLCDC();
			GX_SetBankForBG(vBG);
		}
		{
			int x, y;
			for(y=0; y<12*8; y++){
				for(x=0; x<12*8; x++){
					u8 pixel = GFL_BMP_GetPixel(bmpTmp, x, y);
					//GFL_BMP_Fill(bmpTmp2, x*2, y*2, 2, 2, pixel);
					GFL_BMP_Fill(bmpTmp2, x, y*2, 1, 2, pixel);
				}
			}
		}
		// テクスチャ作成
		{
			//u8*	src = GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp(sw->bmpwin));
			u8	*src, *dst; 
			int x, y;

			src = GFL_BMP_GetCharacterAdrs(bmpTmp);
			dst = GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap);
			GFL_STD_MemClear32((void*)dst, 32*32*0x20);
			dst += (10*32+10) * 0x20;
			for(y=0; y<12; y++){
				for(x=0; x<12; x++){
					GFL_STD_MemCopy32((void*)(&src[(y*12+x)*0x20]), (void*)(&dst[(y*32+x)*0x20]), 0x20);
				}
			}
			GFL_BMP_DataConv_to_BMPformat(sw->pokeImgBitmap, FALSE, sw->heapID);
#if 0
			src = GFL_BMP_GetCharacterAdrs(bmpTmp2);
			dst = GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap2);
			GFL_STD_MemClear32((void*)dst, 32*32*0x20);
#if 0
			dst += (4*32+4) * 0x20;
			for(y=0; y<24; y++){
				for(x=0; x<24; x++){
					GFL_STD_MemCopy32((void*)(&src[(y*24+x)*0x20]), (void*)(&dst[(y*32+x)*0x20]), 0x20);
				}
			}
#else
			dst += (4*32+10) * 0x20;
			for(y=0; y<24; y++){
				for(x=0; x<12; x++){
					GFL_STD_MemCopy32((void*)(&src[(y*24+x)*0x20]), (void*)(&dst[(y*32+x)*0x20]), 0x20);
				}
			}
#endif
			GFL_BMP_DataConv_to_BMPformat(sw->pokeImgBitmap2, FALSE, sw->heapID);
#endif
		}
		GFL_BMP_Delete(bmpTmp2);
		GFL_BMP_Delete(bmpTmp);
	}
	{
		// パレット取得
		GFL_STD_MemCopy32((void*)(HW_BG_PLTT + TEXT_PLTTID*PLTT_SIZ), sw->pokePltt, PLTT_SIZ);
	}
#if 0
	// ダイレクトカラーテクスチャ作成
	{
		u8*	src;
		int i, d, f, colID;
		u16 color;

		src = GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap);
		for(i=0; i<256*256; i++){
			d = i/2;
			f = i%2;
			if(f){ colID = (src[d]&0xf0)>>4; }
			else { colID = src[d]&0x0f; }

			if(colID)	{ color = 0x8000 | (sw->pokePltt[colID]&0x7fff); }
			else			{ color = 0x0000; }	// 透明

			sw->pokeDirectBitmap[i] = color;
		}

		src = GFL_BMP_GetCharacterAdrs(sw->pokeImgBitmap2);
		for(i=0; i<256*256; i++){
			d = i/2;
			f = i%2;
			if(f){ colID = (src[d]&0xf0)>>4; }
			else { colID = src[d]&0x0f; }

			if(colID)	{ color = 0x8000 | (sw->pokePltt[colID]&0x7fff); }
			else			{ color = 0x0000; }	// 透明

			sw->pokeDirectBitmap2[i] = color;
		}
	}
#endif
}

//============================================================================================
#if 0
static u16 calcColLerp(u16 col, u16 colRef)
{
	u8 r1 = col & 0x001f;
	u8 r2 = colRef & 0x001f;
	u8 g1 = (col & 0x03e0)>>5;
	u8 g2 = (colRef & 0x03e0)>>5;
	u8 b1 = (col & 0x7c00)>>10;
	u8 b2 = (colRef & 0x7c00)>>10;

	return ((r1+r2)/2) | (((g1+g2)/2)<<5) | (((b1+b2)/2)<<10);
}

static void col_UDLR_ref(u16* src, u16* dst, int x, int y, u16 bkColor)
{
	u16		col, colRef;
	u16		alpha;
	BOOL	f;

	col = src[y*256+x] & 0x7fff;
	alpha = src[y*256+x] & 0x8000;

	if(col)	{ f = TRUE; }
	else		{ f = FALSE; }

	if(y != 0){
		colRef = src[(y-1)*256+x];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if(y != 255){
		colRef = src[(y+1)*256+x];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if(x != 0){
		colRef = src[y*256+(x-1)];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if(x != 255){
		colRef = src[y*256+(x+1)];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if(f == FALSE)	{ col = 0x0000; } 
	else						{ col |= 0x8000; } 

	dst[y*256+x] = col;
}

static void col_DRdr_ref(u16* src, u16* dst, int x, int y, u16 bkColor)
{
	u16		col, colRef;
	u16		alpha;
	BOOL	f;

	col = src[y*256+x] & 0x7fff;
	alpha = src[y*256+x] & 0x8000;

	if(col)	{ f = TRUE; }
	else		{ f = FALSE; }

	if(y != 255){
		colRef = src[(y+1)*256+x];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if(x != 255){
		colRef = src[y*256+(x+1)];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if((x != 255)&&(y != 255)){
		colRef = src[(y+1)*256+(x+1)];
		if(colRef == 0x0000){ colRef = bkColor; }
		else								{ f |= TRUE; }
		col = calcColLerp(col, colRef);
	}
	if(f == FALSE)	{ col = 0x0000; } 
	else						{ col |= 0x8000; } 

	dst[y*256+x] = col;
}

static void col_UD_ref(u16* src, u16* dst, int x, int y, u16 bkColor)
{
	u16		col, colRef;
	u16		alpha;
	BOOL	f;

	col = src[y*256+x] & 0x7fff;
	alpha = src[y*256+x] & 0x8000;

	if(alpha){
		if(y != 0){
			colRef = src[(y-1)*256+x];
			if(colRef&0x8000)	{ col = calcColLerp(col, colRef); }
			else							{ col = calcColLerp(col, bkColor); }
		}
		if(y != 255){
			colRef = src[(y+1)*256+x];
			if(colRef&0x8000)	{ col = calcColLerp(col, colRef); }
			else							{ col = calcColLerp(col, bkColor); }
		}
		col |= 0x8000;
	}
	dst[y*256+x] = col;
}

static void col_D_ref(u16* src, u16* dst, int x, int y, u16 bkColor)
{
	u16		col, colRef;
	u16		alpha;
	BOOL	f;

	col = src[y*256+x] & 0x7fff;
	alpha = src[y*256+x] & 0x8000;

	if(alpha){
		if(y != 255){
			colRef = src[(y+1)*256+x];
			if(colRef&0x8000)	{ col = calcColLerp(col, colRef); }
			else							{ col = calcColLerp(col, bkColor); }
		}
		col |= 0x8000;
	}
	dst[y*256+x] = col;
}

static void col_U_ref(u16* src, u16* dst, int x, int y, u16 bkColor)
{
	u16		col, colRef;
	u16		alpha;
	BOOL	f;

	col = src[y*256+x] & 0x7fff;
	alpha = src[y*256+x] & 0x8000;

	if(alpha){
		if(y != 0){
			colRef = src[(y-1)*256+x];
			if(colRef&0x8000)	{ col = calcColLerp(col, colRef); }
			else							{ col = calcColLerp(col, bkColor); }
		}
		col |= 0x8000;
	}
	dst[y*256+x] = col;
}

static void col_R_ref(u16* src, u16* dst, int x, int y, u16 bkColor)
{
	u16		col, colRef;
	u16		alpha;
	BOOL	f;

	col = src[y*256+x] & 0x7fff;
	alpha = src[y*256+x] & 0x8000;

	if(alpha){
		if(x != 255){
			colRef = src[y*256+(x+1)];
			if(colRef&0x8000)	{ col = calcColLerp(col, colRef); }
			else							{ col = calcColLerp(col, bkColor); }
		}
		col |= 0x8000;
	}
	dst[y*256+x] = col;
}

//--------------------------------------------------------------------------------------------
static void makeEffectBitmap(sample6_WORK* sw, u16 bkColor)
{
	u16*	src = sw->pokeDirectBitmap;
	u16*	dst = sw->effectBitmap;
	int x, y;
	//bkColor = 0;

	for(y=0; y<256; y++){
		for(x=0; x<256; x++){
			col_U_ref(src, dst, x, y, bkColor);
		}
	}
}


static void makeEffectBitmap2(sample6_WORK* sw, u16 bkColor)
{
	u16*	src = sw->pokeDirectBitmap2;
	u16*	dst = sw->effectBitmap2;
	int x, y;

	bkColor = GX_RGB(20, 20, 20);
	for(y=0; y<256; y++){
		for(x=0; x<256; x++){
			col_UD_ref(src, dst, x, y, bkColor);
		}
	}
}

//============================================================================================
static void makeEffectPalette(sample6_WORK* sw)
{
	u16 col, colR, colG, colB;
	int i;

	for(i=0; i<16; i++){
		col = sw->pokePltt[i];
		colR = col & 0x001f;
		colG = (col & 0x03e0)>>5;
		colB = (col & 0x7c00)>>10;
	}
	//sw->effectPalette;
}
#endif

//============================================================================================
// 上pixelを参照して中間色カラーIDに変換し256色イメージ作成
static void* PMVEFF_MakeEffectImage(u8* image, u16 sizX, u16 sizY, HEAPID heapID, BOOL imgFree)
{
	u8*	retImg = GFL_HEAP_AllocMemory(GFL_HEAP_LOWID(heapID), sizX*sizY);
	u8*	tmpImg = GFL_HEAP_AllocMemory(GFL_HEAP_LOWID(heapID), sizX*sizY);
	u8	col, colRef;
	int	i, x, y;

	// 256色モードに変換
	for(i=0; i<sizX*sizY; i++){
		tmpImg[i] = (i%2)? (image[i/2]&0xf0)>>4 : image[i/2]&0x0f;
	}
	// ライン0はそのまま
	GFL_STD_MemCopy32((void*)tmpImg, (void*)retImg, 256);

	for(y=1; y<sizY; y++){
		for(x=0; x<sizX; x++){
			col = tmpImg[y*sizX+x];
			if(col){
				colRef = tmpImg[(y-1)*sizX+x];
				col = (col<<4) | colRef;
			}
			retImg[y*sizX+x] = col;
		}
	}
	if(imgFree == TRUE){ GFL_HEAP_FreeMemory(image); }
	GFL_HEAP_FreeMemory(tmpImg);

	return (void*)retImg;
}

//--------------------------------------------------------------------------------------------
#define GETCOL_R(col)	(col & 0x001f)
#define GETCOL_G(col)	((col & 0x03e0)>>5)
#define GETCOL_B(col)	((col & 0x7c00)>>10)

// 各色の中間色256パレットを作成
static void* PMVEFF_MakeEffectPalette(u16* pltt, HEAPID heapID, BOOL plttFree)
{
	u16*	retPltt = GFL_HEAP_AllocMemory(GFL_HEAP_LOWID(heapID), 16*16*2);
	u16		r, g, b;
	int		i, j;

	// パレット0～15はそのまま
	GFL_STD_MemCopy32((void*)pltt, (void*)retPltt, 0x20);
	retPltt[0] = GX_RGB(16, 16, 16);	// 透明色

	for(i=1; i<16; i++){
		for(j=0; j<16; j++){
			// calc lerp
			r = (GETCOL_R(retPltt[i]) + GETCOL_R(retPltt[j])) / 2;
			g = ((GETCOL_G(retPltt[i]) + GETCOL_G(retPltt[j])) / 2)<<5;
			b = ((GETCOL_B(retPltt[i]) + GETCOL_B(retPltt[j])) / 2)<<10;
			retPltt[i*16+j] = r | g | b;
		}
	}
	if(plttFree == TRUE){ GFL_HEAP_FreeMemory(pltt); }
	return (void*)retPltt;
}






