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

#include "test/camera_adjust_view.h"

typedef struct _EL_SCOREBOARD	EL_SCOREBOARD;

typedef enum {
	ELB_TEXSIZ_16x16 = 0,
	ELB_TEXSIZ_32x16,
	ELB_TEXSIZ_64x16,
	ELB_TEXSIZ_128x16,
	ELB_TEXSIZ_256x16,
	ELB_TEXSIZ_512x16,
	ELB_TEXSIZ_1024x16,
}ELB_TEXSIZ;

EL_SCOREBOARD*	ELBOARD_Add( ELB_TEXSIZ sizMode, u8 strLen, HEAPID heapID );
void						ELBOARD_Delete( EL_SCOREBOARD* elb );
void						ELBOARD_Main( EL_SCOREBOARD* elb );
void						ELBOARD_Draw( EL_SCOREBOARD* elb, 
															VecFx32* trans, fx32 scale, fx16 sizX, fx16 sizY, 
															GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset );
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

#define TEXT_FRAME		(GFL_BG_FRAME3_M)
#define STATUS_FRAME	(GFL_BG_FRAME0_S)
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

static const u16 plttData2[PLTT_SIZ/2] = { 
	0x0000, 0x18C6, 0x2108, 0x031F, 0x0000, 0x0000, 0x0000, 0x0000,
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

static const GFL_CAMADJUST_SETUP camAdjustData= {
	0,
	GFL_DISPUT_BGID_S0, GFL_DISPUT_PALID_15,
};

//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	HEAPID							heapID;
	int									seq;

	PRINT_QUE*					printQue;
	GFL_FONT*						fontHandle;

	GFL_BMPWIN*					bmpwin;	

	STRBUF*							strBuf;
	GFL_BMP_DATA*				bmp;	

	GFL_G3D_CAMERA*			g3Dcamera;	
	GFL_G3D_LIGHTSET*		g3Dlightset;	

	EL_SCOREBOARD*			elb;

	GFL_CAMADJUST*			gflCamAdjust;
	fx32								cameraLength;
	u16									cameraAngleV;
	u16									cameraAngleH;

	int									timer;
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

		sw->gflCamAdjust = GFL_CAMADJUST_Create(&camAdjustData, sw->heapID);
		GFL_CAMADJUST_SetCameraParam
			(sw->gflCamAdjust, &sw->cameraAngleV, &sw->cameraAngleH, &sw->cameraLength); 

		sw->seq++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			sw->seq++;
		}
		GFL_CAMADJUST_Main(sw->gflCamAdjust);
		{
			u16 col;
			if( sw->timer & 0x0004 ){ col = COLDATA_LETTER1; } else { col = COLDATA_LETTER2; }
			GFL_BG_LoadPalette(TEXT_FRAME, (void*)&col, COL_SIZ, TEXT_PLTTID*PLTT_SIZ + LCOL*COL_SIZ);
		}
		systemFramework(sw);
		break;

	case 2:
		GFL_CAMADJUST_Delete(sw->gflCamAdjust);
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
	GX_VRAM_BG_128_C,				//メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_32_H,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_NONE,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_01_AB,				//テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G,			//テクスチャパレットスロットに割り当て
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
static const GFL_BG_BGCNT_HEADER Statuscont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_128x128,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

//カメラ初期設定データ
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 cameraPos	= { 0, (FX32_ONE * 32), (FX32_ONE * 64) };

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };

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
	GFL_BG_SetBGControl(STATUS_FRAME, &Statuscont, GFL_BG_MODE_TEXT);
	GFL_BG_SetPriority(STATUS_FRAME, 0);
	GFL_BG_SetVisible(STATUS_FRAME, VISIBLE_ON);

	GFL_BG_FillCharacter(TEXT_FRAME, 0, 1, 0);	// 先頭にクリアキャラ配置
	GFL_BG_ClearScreen(TEXT_FRAME);

	//３Ｄシステム起動
	GFL_G3D_Init
		(GFL_G3D_VMANLNK, GFL_G3D_TEX384K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K, 0, heapID, NULL );
	GFL_BG_SetBGControl3D(1);

	//ビットマップウインドウ起動
	GFL_BMPWIN_Init(heapID);

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
	GFL_DISP_SetDispOn();
}

static void	bg_exit(void)
{
	GFL_G3D_Exit();
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl(STATUS_FRAME);
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
	GFL_BG_SetBackGroundColor( TEXT_FRAME, 0x7fff );
	GFL_BG_LoadPalette( TEXT_FRAME, (void*)plttData, PLTT_SIZ, TEXT_PLTTID * PLTT_SIZ );
	//フォントハンドル作成
	sw->fontHandle = GFL_FONT_Create(	ARCID_FONT,
										NARC_font_large_nftr,
										GFL_FONT_LOADTYPE_FILE,
										FALSE,
										sw->heapID);
	//プリントキューハンドル作成
	sw->printQue = PRINTSYS_QUE_Create(sw->heapID);

	//描画用ビットマップ作成
	sw->bmpwin = GFL_BMPWIN_Create(	TEXT_FRAME, 4, 2, 24, 2, TEXT_PLTTID, GFL_BG_CHRAREA_GET_F );
	GFL_BMPWIN_MakeScreen(sw->bmpwin);
	GFL_BG_LoadScreenReq(TEXT_FRAME);

	//カメラ作成
	sw->g3Dcamera = GFL_G3D_CAMERA_CreateDefault(&cameraPos, &cameraTarget, sw->heapID);
	GFL_G3D_CAMERA_Switching(sw->g3Dcamera);
	//ライト作成
	sw->g3Dlightset = GFL_G3D_LIGHT_Create( &light0Setup, sw->heapID );
	GFL_G3D_LIGHT_Switching(sw->g3Dlightset);

	sw->bmp = GFL_BMPWIN_GetBmp(sw->bmpwin);

	sw->elb = ELBOARD_Add( ELB_TEXSIZ_256x16, 64, sw->heapID );
}

//------------------------------------------------------------------
/**
 * @brief		フレームワーク
 */
//------------------------------------------------------------------
#define PITCH_LIMIT (0x200)
static void systemFramework(SAMPLE2_WORK* sw)
{
	PRINTSYS_QUE_Main(sw->printQue);
	GFL_BMPWIN_TransVramCharacter(sw->bmpwin);

	ELBOARD_Main(sw->elb);

	//３Ｄ描画
	{
		VecFx32 target = {0,0,0};
		VecFx32 cameraPos;

		//距離とアングルによるカメラ位置計算
		{
			VecFx32 vecCamera;
			fx16 sinYaw = FX_SinIdx(sw->cameraAngleV);
			fx16 cosYaw = FX_CosIdx(sw->cameraAngleV);
			fx16 sinPitch = FX_SinIdx(sw->cameraAngleH);
			fx16 cosPitch = FX_CosIdx(sw->cameraAngleH);
		
			if(cosPitch < 0){ cosPitch = -cosPitch; }	// 裏周りしないようにマイナス値はプラス値に補正
			if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }	// 0値近辺は不正表示になるため補正

			// カメラの座標計算
			VEC_Set( &vecCamera, sinYaw * cosPitch, sinPitch * FX16_ONE, cosYaw * cosPitch);
			VEC_Normalize(&vecCamera, &vecCamera);
			VEC_MultAdd(sw->cameraLength, &vecCamera, &target, &cameraPos);

			GFL_G3D_CAMERA_SetPos(sw->g3Dcamera, &cameraPos);
		}

		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		
		{
			fx32		scale = FX32_ONE;
			fx16		sizX = 7 * FX16_ONE;
			fx16		sizY = 1 * FX16_ONE;

			ELBOARD_Draw(sw->elb, &target, scale, sizX, sizY, sw->g3Dcamera, sw->g3Dlightset);
		}
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
	}
}

//------------------------------------------------------------------
/**
 * @brief		破棄
 */
//------------------------------------------------------------------
static void systemDelete(SAMPLE2_WORK* sw)
{
	ELBOARD_Delete(sw->elb);

	GFL_G3D_LIGHT_Delete(sw->g3Dlightset);
	GFL_G3D_CAMERA_Delete(sw->g3Dcamera);

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
 * @file	ele_scoreboard.c
 * @brief	電光掲示板システム
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
 *
 * @brief	型宣言
 *
 */
//------------------------------------------------------------------
struct _EL_SCOREBOARD {
	HEAPID				heapID;

	GXTexSizeS		s;						//SDK用データサイズX定義
	GXTexSizeT		t;						//SDK用データサイズY定義
	u16						texSizX;			//テクスチャデータサイズX
	u16						texSizY;			//テクスチャデータサイズY
	NNSGfdTexKey	texVramKey;		//テクスチャＶＲＡＭキー
	NNSGfdPlttKey	plttVramKey;	//パレットＶＲＡＭキー

	PRINT_QUE*		printQue;
	GFL_FONT*			fontHandle;

	STRBUF*				strBuf1;
	STRBUF*				strBuf2;

	GFL_BMP_DATA*	bmp;	
	GFL_BMP_DATA*	bmpTmp;	

	int						timer;
};

//------------------------------------------------------------------
//SDK用データサイズ定義　変換テーブル（GFL_BBD_TEXSIZ に対応）
typedef struct {
	GXTexSizeS	s;
	GXTexSizeT	t;
	u16					sizX;
	u16					sizY;
}GX_SIZ_TBL;

static const GX_SIZ_TBL GX_sizTbl[] = {
	{ GX_TEXSIZE_S16,		GX_TEXSIZE_T16,		16,		16		}, 
	{ GX_TEXSIZE_S32,		GX_TEXSIZE_T16,		32,		16		}, 
	{ GX_TEXSIZE_S64,		GX_TEXSIZE_T16,		64,		16		}, 
	{ GX_TEXSIZE_S128,	GX_TEXSIZE_T16,		128,	16		}, 
	{ GX_TEXSIZE_S256,	GX_TEXSIZE_T16,		256,	16		}, 
	{ GX_TEXSIZE_S512,	GX_TEXSIZE_T16,		512,	16		}, 
	{ GX_TEXSIZE_S1024,	GX_TEXSIZE_T16,		1024,	16		}, 
};

//------------------------------------------------------------------
/**
 * @brief	ウインドウ背景クリア
 */
//------------------------------------------------------------------
static void clearBitmap(GFL_BMP_DATA* bmp)
{
	u8*	dataAdrs = GFL_BMP_GetCharacterAdrs(bmp);
	u32	writeSize = GFL_BMP_GetBmpDataSize(bmp);
	int	i;

	for( i= 0; i<writeSize; i++ ){ dataAdrs[i] = ((BCOL2 << 4) | BCOL1); }

}

//------------------------------------------------------------------
/**
 * @brief	文字列描画
 */
//------------------------------------------------------------------
static void printStr
			(const STRBUF* str, GFL_BMP_DATA* bmp, PRINT_QUE* printQue, GFL_FONT* fontHandle)
{
	PRINTSYS_LSB	lsb = PRINTSYS_LSB_Make(LCOL,0,0);
	clearBitmap(bmp);

	PRINTSYS_PrintQueColor(printQue, bmp, 0, 2, str, fontHandle, lsb);
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

//------------------------------------------------------------------
/**
 * @brief	ビットマップ変換
 */
//------------------------------------------------------------------
static void convBitmap(GFL_BMP_DATA* src, GFL_BMP_DATA* dst)
{
	u16	size_x = GFL_BMP_GetSizeX(src)/8;						//画像データのXdotサイズ
	u16	size_y = GFL_BMP_GetSizeY(src)/8;						//画像データのYdotサイズ
	u16	col = (u16)GFL_BMP_GetColorFormat(src);			//カラーモード＆データサイズ
	u16	size_cx = col/8;														//キャラクタデータXサイズ
	u8*	srcAdrs = GFL_BMP_GetCharacterAdrs(src);
	u8*	dstAdrs = GFL_BMP_GetCharacterAdrs(dst);
	int	i, cx, cy, x, y;

	for( i=0; i<size_x * size_y * col; i++ ){
		y = i/(size_x * size_cx);
		x = i%(size_x * size_cx);
		dstAdrs[i] = srcAdrs[ ((y/8)*size_x + (x/size_cx))*col + ((y%8)*size_cx + (x%size_cx)) ];
	}
}


//------------------------------------------------------------------
/**
 * @brief	オブジェクト追加
 */
//------------------------------------------------------------------
EL_SCOREBOARD* ELBOARD_Add( ELB_TEXSIZ sizMode, u8 strLen, HEAPID heapID )
{
	EL_SCOREBOARD* elb = GFL_HEAP_AllocClearMemory(heapID, sizeof(EL_SCOREBOARD));

	elb->heapID = heapID;

	//描画用設定
	if( sizMode >= NELEMS(GX_sizTbl) ){ sizMode = NELEMS(GX_sizTbl)-1; }
	elb->s = GX_sizTbl[sizMode].s;
	elb->t = GX_sizTbl[sizMode].t;
	elb->texSizX = GX_sizTbl[sizMode].sizX;
	elb->texSizY = GX_sizTbl[sizMode].sizY;

	//ＶＲＡＭ確保
	elb->texVramKey = NNS_GfdAllocTexVram(elb->texSizX * elb->texSizY, FALSE, 0);
	elb->plttVramKey = NNS_GfdAllocPlttVram(16, FALSE, 0);

	//フォントハンドル作成
	elb->fontHandle = GFL_FONT_Create
						(ARCID_FONT, NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID);
	//プリントキューハンドル作成
	elb->printQue = PRINTSYS_QUE_Create(heapID);

	//文字列バッファ作成
	elb->strBuf1 = GFL_STR_CreateBuffer(strLen, heapID);
	elb->strBuf2 = GFL_STR_CreateBuffer(strLen, heapID);

	//ビットマップ作成
	elb->bmp = GFL_BMP_Create(elb->texSizX/8, elb->texSizY/8, GFL_BMP_16_COLOR, heapID);
	elb->bmpTmp = GFL_BMP_Create(elb->texSizX/8, elb->texSizY/8, GFL_BMP_16_COLOR, heapID);

	elb->timer = 0;

	return elb;
}	

//------------------------------------------------------------------
/**
 * @brief	オブジェクト破棄
 */
//------------------------------------------------------------------
void	ELBOARD_Delete( EL_SCOREBOARD* elb )
{
	GFL_BMP_Delete(elb->bmpTmp);
	GFL_BMP_Delete(elb->bmp);

	GFL_STR_DeleteBuffer(elb->strBuf2);
	GFL_STR_DeleteBuffer(elb->strBuf1);

	PRINTSYS_QUE_Clear(elb->printQue);
	PRINTSYS_QUE_Delete(elb->printQue);
	GFL_FONT_Delete(elb->fontHandle);

	NNS_GfdFreePlttVram(elb->plttVramKey);
	NNS_GfdFreeTexVram(elb->texVramKey);
}	

//------------------------------------------------------------------
/**
 *
 * @brief	動作
 *
 */
//------------------------------------------------------------------
void	ELBOARD_Main( EL_SCOREBOARD* elb )
{
	elb->timer++;

	makeStr(testMsg, elb->strBuf1);
	printStr(elb->strBuf1, elb->bmp, elb->printQue, elb->fontHandle);
	convBitmap(elb->bmp, elb->bmpTmp);

	{
		void* src;
		u32 dst, siz;

		if(elb->timer & 0x0004){ src = (void*)plttData; } else { src = (void*)plttData2; }
		dst = NNS_GfdGetPlttKeyAddr(elb->plttVramKey);
		siz = PLTT_SIZ;
		NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, siz);

		src = GFL_BMP_GetCharacterAdrs(elb->bmpTmp);
		dst = NNS_GfdGetTexKeyAddr(elb->texVramKey);
		siz = NNS_GfdGetTexKeySize(elb->texVramKey);
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_VRAM, dst, src, siz);
	}
}

//------------------------------------------------------------------
/**
 *
 * @brief	描画
 *
 */
//------------------------------------------------------------------
#define ELB_DIF			(GX_RGB(31, 31, 31))
#define ELB_AMB			(GX_RGB(16, 16, 16))
#define ELB_SPE			(GX_RGB(16, 16, 16))
#define ELB_EMI			(GX_RGB(0, 0, 0))
#define ELB_POLID 	(63)

void	ELBOARD_Draw( EL_SCOREBOARD* elb, 
										VecFx32* trans, fx32 scale, fx16 sizX, fx16 sizY, 
										GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset )
{
	MtxFx33			mtx;
	VecFx16			vecN;
	VecFx32			transTmp;
	fx32				s0, t0, s1, t1;

	G3X_Reset();

	//カメラ設定取得
	{
		VecFx32		camPos, camUp, target, vecNtmp;
		MtxFx43		mtxCamera;

		GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
		GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

		G3_LookAt( &camPos, &camUp, &target, &mtxCamera );	//mtxCameraには行列計算結果が返る
		MTX_Copy43To33( &mtxCamera, &mtx );		//カメラ逆行列から回転行列を取り出す

		VEC_Subtract( &camPos, &target, &vecNtmp );
		VEC_Normalize( &vecNtmp, &vecNtmp );
		VEC_Fx16Set( &vecN, vecNtmp.x, vecNtmp.y, vecNtmp.z );
	}
	if( g3Dlightset ){ GFL_G3D_LIGHT_Switching( g3Dlightset ); }

	//グローバルスケール設定
#if 0
	G3_Scale( scale, scale, scale );
#else
	G3_Scale( scale * FX16_ONE, scale * FX16_ONE, scale * FX16_ONE );
#endif
	s0 = 0;
	s1 = s0 + elb->texSizX * FX32_ONE;
	t0 = 0;
	t1 = t0 + elb->texSizY * FX32_ONE;

	G3_PushMtx();

	VEC_Set(&transTmp, FX_Div(trans->x,scale), FX_Div(trans->y,scale), FX_Div(trans->z,scale));
	//回転、平行移動パラメータ設定
	G3_MultTransMtx33(&mtx, &transTmp);

	G3_TexImageParam(	GX_TEXFMT_PLTT16, GX_TEXGEN_TEXCOORD, elb->s, elb->t,
										GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
										GX_TEXPLTTCOLOR0_TRNS, NNS_GfdGetTexKeyAddr(elb->texVramKey) );
	G3_TexPlttBase( NNS_GfdGetPlttKeyAddr(elb->plttVramKey), GX_TEXFMT_PLTT16 );

	//マテリアル設定
	G3_MaterialColorDiffAmb( ELB_DIF, ELB_AMB, TRUE );
	G3_MaterialColorSpecEmi( ELB_SPE, ELB_EMI, FALSE );
	G3_PolygonAttr(	GFL_BBD_LIGHTMASK_0123, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 
									ELB_POLID, 31, GX_POLYGON_ATTR_MISC_FOG );
	
	G3_Begin( GX_BEGIN_QUADS );

	if( g3Dlightset )	{ G3_Normal(vecN.x, vecN.y, vecN.z); } 	//平面ポリゴンなので法線ベクトルは共用
	else							{ G3_Color(GX_RGB(31, 31, 31)); }
#if 0
	G3_TexCoord(s0, t0);
	G3_Vtx(-sizX, sizY, 0);

	G3_TexCoord(s0, t1);
	G3_Vtx(-sizX, -sizY, 0);

	G3_TexCoord(s1, t1);
	G3_Vtx(sizX, -sizY, 0);

	G3_TexCoord(s1, t0);
	G3_Vtx(sizX, sizY, 0);
#else
	{
		fx16	setSizX = elb->texSizX / 2;
		fx16	setSizY = elb->texSizY / 2;

		G3_TexCoord(s0, t0);
		G3_Vtx(-setSizX, setSizY, 0);

		G3_TexCoord(s0, t1);
		G3_Vtx(-setSizX, -setSizY, 0);

		G3_TexCoord(s1, t1);
		G3_Vtx(setSizX, -setSizY, 0);

		G3_TexCoord(s1, t0);
		G3_Vtx(setSizX, setSizY, 0);
	}
#endif
	G3_End();
	G3_PopMtx(1);

	//↓後にG3D_Systemで行うので、ここではやらない
	//G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}


