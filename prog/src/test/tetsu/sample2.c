//============================================================================================
/**
 * @file	sample2.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include <wchar.h>
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"
#include "font/font.naix"

#include "system/main.h"

#include "print/printsys.h"
#include "print/str_tool.h"

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
//------------------------------------------------------------------
/**
 * @brief	定数
 */
//------------------------------------------------------------------
#define STRBUF_SIZE		(64*2)

#define TEXT_FRAME		(GFL_BG_FRAME3_S)
#define TEXT_DISPSIDE	(PALTYPE_SUB_BG)
#define TEXT_PLTTID		(15)
#define COL_SIZ				(2)
#define PLTT_SIZ			(16*COL_SIZ)

//------------------------------------------------------------------
/**
 * @brief		ＢＧ描画データ
 */
//------------------------------------------------------------------
static const u16 plttData[PLTT_SIZ/2] = { 
	0x0000, 0x18C6, 0x2108, 0x021F, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

#define BCOL1 (1)
#define BCOL2 (2)
#define LCOL	(3)
#define COLDATA_LETTER1 (0x21f)
#define COLDATA_LETTER2 (0x31f)
//------------------------------------------------------------------
/**
 * @brief	タッチパネル判定テーブル
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
/**
 * @brief	初期値テーブル
 */
//------------------------------------------------------------------
static const u16 testMsg[] = { L"電光けいじばんを作ってみよう！！・・・・・・" };

//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	HEAPID				heapID;
	int						seq;

	PRINT_QUE*		printQue;
	GFL_FONT*			fontHandle;

	GFL_BMPWIN*		bmpwin;	

	STRBUF*				strBuf;
	GFL_BMP_DATA*	bmp;	

	int						timer;
}SAMPLE2_WORK;

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
static BOOL	sample2(SAMPLE2_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief	ワークの初期化＆破棄
 */
//------------------------------------------------------------------
static void	workInitialize(SAMPLE2_WORK* sw)
{
	sw->seq = 0;
	sw->timer = 0;

	sw->strBuf = GFL_STR_CreateBuffer(STRBUF_SIZE, sw->heapID);
}

static void	workFinalize(SAMPLE2_WORK* sw)
{
	GFL_STR_DeleteBuffer(sw->strBuf);
}

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT Sample2Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE2_WORK* sw = NULL;

	sw = GFL_PROC_AllocWork(proc, sizeof(SAMPLE2_WORK), GFL_HEAPID_APP);

    GFL_STD_MemClear(sw, sizeof(SAMPLE2_WORK));
    sw->heapID = GFL_HEAPID_APP;

	workInitialize(sw);

    return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample2Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE2_WORK*	sw;
	sw = mywk;

	sw->timer++;
	if(sample2(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample2Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE2_WORK*	sw;
	sw = mywk;

	workFinalize(sw);

	GFL_PROC_FreeWork(proc);

    return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeSample2ProcData = {
	Sample2Proc_Init,
	Sample2Proc_Main,
	Sample2Proc_End,
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
static void systemSetup(SAMPLE2_WORK* sw);
static void systemFramework(SAMPLE2_WORK* sw);
static void systemDelete(SAMPLE2_WORK* sw);

static void makeStr(const u16* str, STRBUF* strBuf);
static void printStr
			(const STRBUF* str, GFL_BMP_DATA* bmp, PRINT_QUE* printQue, GFL_FONT* fontHandle);
	
//------------------------------------------------------------------
/**
 *
 * @brief	フレームワーク
 *
 */
//------------------------------------------------------------------
static BOOL	sample2(SAMPLE2_WORK* sw)
{
	switch(sw->seq){
	case 0:
		systemSetup(sw);

		makeStr(testMsg, sw->strBuf);
		printStr(sw->strBuf, sw->bmp, sw->printQue, sw->fontHandle);

		sw->seq++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			sw->seq++;
		}
		{
			u16 col;
			if( sw->timer & 0x0004 ){ col = COLDATA_LETTER1; } else { col = COLDATA_LETTER2; }
			GFL_BG_LoadPalette(TEXT_FRAME, (void*)&col, COL_SIZ, TEXT_PLTTID*PLTT_SIZ + LCOL*COL_SIZ);
		}
		
		systemFramework(sw);
		break;

	case 2:
		systemDelete(sw);
		return FALSE;
	}
	return TRUE;
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	システムセットアップ
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_DISP_VRAM dispVram = {
	GX_VRAM_BG_128_A,				//メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_32_H,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_NONE,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_NONE,				//テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_NONE,			//テクスチャパレットスロットに割り当て
	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	

static const GFL_BG_BGCNT_HEADER Textcont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init(HEAPID heapID)
{
	//VRAM設定
	GFL_DISP_SetBank(&dispVram);

	//ＢＧシステム起動
	GFL_BG_Init(heapID);

	//ＢＧモード設定
	GFL_BG_SetBGMode(&bgsysHeader);

	//ＢＧコントロール設定
	GFL_BG_SetBGControl(TEXT_FRAME, &Textcont, GFL_BG_MODE_TEXT);
	GFL_BG_SetPriority(TEXT_FRAME, 0);
	GFL_BG_SetVisible(TEXT_FRAME, VISIBLE_ON);

	GFL_BG_FillCharacter(TEXT_FRAME, 0, 1, 0);	// 先頭にクリアキャラ配置
	GFL_BG_ClearScreen(TEXT_FRAME);

	//ビットマップウインドウ起動
	GFL_BMPWIN_Init(heapID);

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
	GFL_DISP_SetDispOn();
}

static void	bg_exit(void)
{
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl(TEXT_FRAME);
	GFL_BG_Exit();
}

//------------------------------------------------------------------
/**
 * @brief		セットアップ
 */
//------------------------------------------------------------------
static void systemSetup(SAMPLE2_WORK* sw)
{
	//基本セットアップ
	bg_init(sw->heapID);
	//フォント用パレット転送
	GFL_BG_LoadPalette( TEXT_FRAME, (void*)plttData, PLTT_SIZ, TEXT_PLTTID * PLTT_SIZ );
	//フォントハンドル作成
	sw->fontHandle = GFL_FONT_Create(	ARCID_FONT,
										NARC_font_large_nftr,
										GFL_FONT_LOADTYPE_FILE,
										FALSE,
										sw->heapID);
	//プリントキューハンドル作成
	sw->printQue = PRINTSYS_QUE_Create(sw->heapID);

	//描画用ビットマップ作成（画面全体）
	sw->bmpwin = GFL_BMPWIN_Create(	TEXT_FRAME,
									0, 0, 32, 24,
									TEXT_PLTTID, GFL_BG_CHRAREA_GET_F );
	GFL_BMPWIN_MakeScreen(sw->bmpwin);
	GFL_BG_LoadScreenReq(TEXT_FRAME);

	sw->bmp = GFL_BMPWIN_GetBmp(sw->bmpwin);
}

//------------------------------------------------------------------
/**
 * @brief		フレームワーク
 */
//------------------------------------------------------------------
static void systemFramework(SAMPLE2_WORK* sw)
{
	PRINTSYS_QUE_Main(sw->printQue);
	GFL_BMPWIN_TransVramCharacter(sw->bmpwin);
}

//------------------------------------------------------------------
/**
 * @brief		破棄
 */
//------------------------------------------------------------------
static void systemDelete(SAMPLE2_WORK* sw)
{
	PRINTSYS_QUE_Clear(sw->printQue);
	GFL_BMPWIN_Delete(sw->bmpwin);

	PRINTSYS_QUE_Delete(sw->printQue);
	GFL_FONT_Delete(sw->fontHandle);

	bg_exit();
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	コントローラ
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
//------------------------------------------------------------------
/**
 * @brief	ウインドウ背景クリア
 */
//------------------------------------------------------------------
static void clearBitmap(GFL_BMP_DATA* bmp)
{
	u8*	dataAdrs = GFL_BMP_GetCharacterAdrs(bmp);
	u32	writeSize = (GFL_BMP_GetBmpDataSize(bmp) / 2 );
	int	i;

	for( i= 0; i<writeSize; i++ ){ dataAdrs[i] = ((BCOL2 << 4) | BCOL1); }

}

//------------------------------------------------------------------
/**
 * @brief	名前描画
 */
//------------------------------------------------------------------
static void printStr
			(const STRBUF* str, GFL_BMP_DATA* bmp, PRINT_QUE* printQue, GFL_FONT* fontHandle)
{
	PRINTSYS_LSB	lsb = PRINTSYS_LSB_Make(LCOL,0,0);
	clearBitmap(bmp);

	PRINTSYS_PrintQueColor(printQue, bmp, 0, 0, str, fontHandle, lsb);
}

//------------------------------------------------------------------
/**
 * @brief	文字列作成
 */
//------------------------------------------------------------------
static void makeStr(const u16* str, STRBUF* strBuf)
{
	STRCODE strTmp[STRBUF_SIZE];
	u32 strLen;

	//文字列長さ取得
	const u16 checkLen = wcslen(str);
	if(checkLen >= STRBUF_SIZE){ strLen = STRBUF_SIZE - 1; }
	else { strLen = checkLen; }

	//終端コードを追加してからSTRBUFに変換
	GFL_STD_MemCopy(str, strTmp, strLen*2);
	strTmp[strLen] = GFL_STR_GetEOMCode();

	GFL_STR_SetStringCode(strBuf, strTmp);
}



