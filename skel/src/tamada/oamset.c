//============================================================================================
/**
 * @file		oamset.c
 * @brief
 * @date		2006.11.30
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

//---- DMA 番号
#define DMA_NO   3


#include "gflib.h"
#include "procsys.h"

#include "data.h"

#include "tamada_internal.h"

static GXOamAttr oamBak[128];

void ObjSet(int objNo, int x, int y, int charNo, int paletteNo);
static void VBlankIntr(void * work);


static void OamPrintInit(void)
{
    //---- VRAM クリア
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    //---- OAMとパレットクリア
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);


    //---- バンクＡをＯＢＪへ
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);

    //---- グラフィックス表示モードにする
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

    //---- OBJの表示のみON
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);

    //---- 32KバイトのOBJで2Dマップモードで使用
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    //---- データロード
    MI_DmaCopy32(DMA_NO, sampleCharData, (void *)HW_OBJ_VRAM, sizeof(sampleCharData));
    MI_DmaCopy32(DMA_NO, samplePlttData, (void *)HW_OBJ_PLTT, sizeof(samplePlttData));

    //---- 非表示OBJは画面外へ移動
    MI_DmaFill32(DMA_NO, oamBak, 0xc0, sizeof(GXOamAttr) * 128);

    //---- 表示開始
    //OS_WaitVBlankIntr();
    GX_DispOn();
}

static GFL_PROC_RESULT OamKeyDemoProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mwk)
{
	DEBUG_TAMADA_CONTROL * ctrl = pwk;

	GFL_INTR_SetVblankFunc(VBlankIntr, NULL);

	OamPrintInit();

	return GFL_PROC_RES_FINISH;
}


static GFL_PROC_RESULT OamKeyDemoProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mwk)
{
	u16     keyData;

	//---- Vブランク割込終了待ち
	//OS_WaitVBlankIntr();

	//---- パッドデータ読み込み
	keyData = PAD_Read();

	//---- パッド情報を OBJ で表示
	ObjSet(0, 200, 90, 'A', (keyData & PAD_BUTTON_A) ? 1 : 2);
	ObjSet(1, 180, 95, 'B', (keyData & PAD_BUTTON_B) ? 1 : 2);

	ObjSet(2, 60, 50, 'L', (keyData & PAD_BUTTON_L) ? 1 : 2);
	ObjSet(3, 180, 50, 'R', (keyData & PAD_BUTTON_R) ? 1 : 2);

	ObjSet(4, 60, 80, 'U', (keyData & PAD_KEY_UP) ? 1 : 2);
	ObjSet(5, 60, 100, 'D', (keyData & PAD_KEY_DOWN) ? 1 : 2);
	ObjSet(6, 50, 90, 'L', (keyData & PAD_KEY_LEFT) ? 1 : 2);
	ObjSet(7, 70, 90, 'R', (keyData & PAD_KEY_RIGHT) ? 1 : 2);

	ObjSet(8, 130, 95, 'S', (keyData & PAD_BUTTON_START) ? 1 : 2);
	ObjSet(9, 110, 95, 'S', (keyData & PAD_BUTTON_SELECT) ? 1 : 2);

	ObjSet(10, 200, 75, 'X', (keyData & PAD_BUTTON_X) ? 1 : 2);
	ObjSet(11, 180, 80, 'Y', (keyData & PAD_BUTTON_Y) ? 1 : 2);

	//---- デバッグボタン情報を OBJ で表示
	ObjSet(12, 200, 120, 'D', (keyData & PAD_BUTTON_DEBUG) ? 1 : 2);
	//---- 折り畳み検知状態を OBJ で表示
	//ObjSet(13, 120, 30, 'F', (PAD_DetectFold())? 1 : 2);
    
	if (keyData & PAD_BUTTON_SELECT) {
		return GFL_PROC_RES_FINISH;
	} else {
		return GFL_PROC_RES_CONTINUE;
	}
}


static GFL_PROC_RESULT OamKeyDemoProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mwk)
{
	return GFL_PROC_RES_FINISH;
}


const GFL_PROC_DATA OamKeyDemoProcData = {
	OamKeyDemoProcInit,
	OamKeyDemoProcMain,
	OamKeyDemoProcEnd,
};

//--------------------------------------------------------------------------------
//  OBJ セット
//
void ObjSet(int objNo, int x, int y, int charNo, int paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&oamBak[objNo],
                  x,
                  y,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}


//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
static void VBlankIntr(void * work)
{
    //---- ＯＡＭの更新
    DC_FlushRange(oamBak, sizeof(GXOamAttr) * 128);
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(DMA_NO, oamBak, (void *)HW_OAM, sizeof(GXOamAttr) * 128);

    //---- 割り込みチェックフラグ
    //OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

