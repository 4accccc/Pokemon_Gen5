//============================================================================================
/**
 * @file	sample3.c
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

#include "system/talkmsgwin.h"

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
#define WIN_PLTTID		(14)
#define TEXT_PLTTID		(15)
#define COL_SIZ				(2)
#define PLTT_SIZ			(16*COL_SIZ)

#define BACKGROUND_COLOR (0)
//------------------------------------------------------------------
/**
 * @brief		ＢＧ描画データ
 */
//------------------------------------------------------------------
#define BCOL1 (1)
#define BCOL2 (2)
#define LCOL	(3)
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
static const GFL_CAMADJUST_SETUP camAdjustData= {
	0,
	GFL_DISPUT_BGID_S0, GFL_DISPUT_PALID_15,
};

//------------------------------------------------------------------
/**
 * @brief	
 */
//------------------------------------------------------------------
static const u16 testMsg[] = { L"セリフを入れよう！！" };
static const u16 testMsg2[] = { L"連動" };

//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	const u16*		pCommand;
	u16						command;
	u16						commParam[8];
}TEST_MSG_COMM;

typedef struct {
	HEAPID							heapID;
	int									seq;

	GFL_FONT*						fontHandle;

	GFL_G3D_CAMERA*			g3Dcamera;	
	GFL_G3D_LIGHTSET*		g3Dlightset;	
	GFL_G3D_UTIL*				g3Dutil;
	u16									g3DutilUnitIdx;

	GFL_CAMADJUST*			gflCamAdjust;
	fx32								cameraLength;
	u16									cameraAngleV;
	u16									cameraAngleH;

	int									timer;

	TALKMSGWIN_SYS*			tmsgwinSys;
	int									tmsgwinIdx;
	VecFx32							tmsgTarget;
	BOOL								tmsgwinConnect;

	STRBUF*							strBuf[TALKMSGWIN_NUM];
	VecFx32							twinTarget[TALKMSGWIN_NUM];

	TEST_MSG_COMM				msgComm;
}SAMPLE3_WORK;

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
static BOOL	sample3(SAMPLE3_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT Sample3Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE3_WORK* sw = NULL;
	int i;

	sw = GFL_PROC_AllocWork(proc, sizeof(SAMPLE3_WORK), GFL_HEAPID_APP);

  GFL_STD_MemClear(sw, sizeof(SAMPLE3_WORK));
  sw->heapID = GFL_HEAPID_APP;
	sw->seq = 0;
	sw->timer = 0;
	sw->tmsgwinIdx = 0;
	sw->tmsgwinConnect = FALSE;

	for(i=0; i<TALKMSGWIN_NUM; i++){ sw->strBuf[i] = GFL_STR_CreateBuffer(STRBUF_SIZE, sw->heapID); }

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample3Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE3_WORK*	sw;
	sw = mywk;

	sw->timer++;
	if(sample3(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample3Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE3_WORK*	sw;
	int i;

	sw = mywk;

	for( i=0; i<TALKMSGWIN_NUM; i++ ){ GFL_STR_DeleteBuffer(sw->strBuf[i]); }

	GFL_PROC_FreeWork(proc);

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeSample3ProcData = {
	Sample3Proc_Init,
	Sample3Proc_Main,
	Sample3Proc_End,
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
static void systemSetup(SAMPLE3_WORK* sw);
static void systemFramework(SAMPLE3_WORK* sw);
static void systemDelete(SAMPLE3_WORK* sw);

static BOOL calcTarget(GFL_G3D_CAMERA* g3Dcamera, int x, int y, VecFx32* target);
static void makeStr(const u16* str, STRBUF* strBuf);
static void commFunc(SAMPLE3_WORK* sw);

static void setSampleMsg1(SAMPLE3_WORK* sw, int idx);
//------------------------------------------------------------------
/**
 *
 * @brief	フレームワーク
 *
 */
//------------------------------------------------------------------
static BOOL	sample3(SAMPLE3_WORK* sw)
{
	switch(sw->seq){
	case 0:
		systemSetup(sw);

		sw->gflCamAdjust = GFL_CAMADJUST_Create(&camAdjustData, sw->heapID);
		GFL_CAMADJUST_SetCameraParam
			(sw->gflCamAdjust, &sw->cameraAngleV, &sw->cameraAngleH, &sw->cameraLength); 

		sw->seq++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
			sw->seq = 3;
			break;
		}
		GFL_CAMADJUST_Main(sw->gflCamAdjust);

		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			setSampleMsg1(sw, 0);
			sw->seq++;
		} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
		} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
		} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		}
		systemFramework(sw);
		break;

	case 2:
		GFL_CAMADJUST_Main(sw->gflCamAdjust);

		commFunc(sw);
		systemFramework(sw);

		if(sw->msgComm.pCommand == NULL){ sw->seq = 1; }
		break;

	case 3:
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

#include "arc/elboard_test.naix"

enum {
	G3DRES_ELBOARD2_BMD = 0,
	G3DRES_ELBOARD2_BTX,
	G3DRES_ELBOARD2_BTA,
};

static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard2_test_nsbmd, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard2_test_nsbtx, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard2_test_nsbta, GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_ANM g3Dutil_anm2Tbl[] = {
	{ G3DRES_ELBOARD2_BTA, 0 },
};

enum {
	G3DOBJ_ELBOARD2 = 0,
};

static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_ELBOARD2_BMD, 0, G3DRES_ELBOARD2_BTX, g3Dutil_anm2Tbl, NELEMS(g3Dutil_anm2Tbl) },
};

static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};

static const GFL_G3D_OBJSTATUS g3DobjStatus1 = {
	{ 0, 0, 0 },																				//座標
	{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
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
	GFL_BG_SetBGControl(STATUS_FRAME, &Statuscont, GFL_BG_MODE_TEXT);
	GFL_BG_SetPriority(STATUS_FRAME, 0);
	GFL_BG_SetVisible(STATUS_FRAME, VISIBLE_ON);

//	GFL_BG_FillCharacter(TEXT_FRAME, 0, 1, 0);	// 先頭にクリアキャラ配置
//	GFL_BG_ClearScreen(TEXT_FRAME);

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
static void systemSetup(SAMPLE3_WORK* sw)
{
	//基本セットアップ
	bg_init(sw->heapID);
	//フォント用パレット転送
	GFL_BG_SetBackGroundColor( TEXT_FRAME, BACKGROUND_COLOR );
	//フォントハンドル作成
	sw->fontHandle = GFL_FONT_Create
		(	ARCID_FONT, NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, sw->heapID);

	//カメラ作成
	sw->g3Dcamera = GFL_G3D_CAMERA_CreateDefault(&cameraPos, &cameraTarget, sw->heapID);
	GFL_G3D_CAMERA_Switching(sw->g3Dcamera);

	//３Ｄオブジェクト作成
	{
		u16						objIdx, elboard1Idx;
		GFL_G3D_OBJ*	g3Dobj;
		GFL_G3D_RES*	g3Dtex;
		int i, anmCount;

		//リソース作成
		sw->g3Dutil = GFL_G3D_UTIL_Create(NELEMS(g3Dutil_resTbl), NELEMS(g3Dutil_objTbl), sw->heapID );
		sw->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( sw->g3Dutil, &g3Dutil_setup );

		//アニメーションを有効にする
		objIdx = GFL_G3D_UTIL_GetUnitObjIdx(sw->g3Dutil, sw->g3DutilUnitIdx);
		{
			elboard1Idx = objIdx + G3DOBJ_ELBOARD2;
			g3Dobj = GFL_G3D_UTIL_GetObjHandle(sw->g3Dutil, elboard1Idx);

			anmCount = GFL_G3D_OBJECT_GetAnimeCount(g3Dobj);
			for( i=0; i<anmCount; i++ ){ GFL_G3D_OBJECT_EnableAnime(g3Dobj, i); } 

			g3Dtex =	GFL_G3D_RENDER_GetG3DresTex(GFL_G3D_OBJECT_GetG3Drnd(g3Dobj));
		}
	}
	{
		TALKMSGWIN_SYS_INI ini = {TEXT_FRAME, WIN_PLTTID, TEXT_PLTTID};
		TALKMSGWIN_SYS_SETUP setup;		
		setup.heapID = sw->heapID;
		setup.g3Dcamera = sw->g3Dcamera;
		setup.fontHandle = sw->fontHandle;
		setup.ini = ini;

		sw->tmsgwinSys = TALKWINMSG_SystemCreate(&setup);
	}
}

//------------------------------------------------------------------
/**
 * @brief		フレームワーク
 */
//------------------------------------------------------------------
#define PITCH_LIMIT (0x200)
static void systemFramework(SAMPLE3_WORK* sw)
{
	//距離とアングルによるカメラ位置計算
	{
		VecFx32 target = {0,0,0};
		VecFx32 cameraPos;
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
	TALKMSGWIN_SystemMain(sw->tmsgwinSys);
	TALKMSGWIN_SystemDraw2D(sw->tmsgwinSys);

	//３Ｄ描画
	GFL_G3D_DRAW_Start();			//描画開始
	{
		GFL_G3D_CAMERA_Switching(sw->g3Dcamera);
		{
			u16						objIdx, elboard1Idx;
			GFL_G3D_OBJ*	g3Dobj;
			int i, anmCount;

			objIdx = GFL_G3D_UTIL_GetUnitObjIdx(sw->g3Dutil, sw->g3DutilUnitIdx );
			elboard1Idx = objIdx + G3DOBJ_ELBOARD2;
			g3Dobj = GFL_G3D_UTIL_GetObjHandle(sw->g3Dutil, elboard1Idx);

			GFL_G3D_DRAW_DrawObject(g3Dobj, &g3DobjStatus1);

			anmCount = GFL_G3D_OBJECT_GetAnimeCount(g3Dobj);
			for( i=0; i<anmCount; i++ ){ GFL_G3D_OBJECT_LoopAnimeFrame(g3Dobj, i, FX32_ONE ); } 
		}
		TALKMSGWIN_SystemDraw3D(sw->tmsgwinSys);

	}
	GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
}

//------------------------------------------------------------------
/**
 * @brief		破棄
 */
//------------------------------------------------------------------
static void systemDelete(SAMPLE3_WORK* sw)
{
	TALKMSGWIN_SystemDelete(sw->tmsgwinSys);

	GFL_G3D_UTIL_DelUnit(sw->g3Dutil, sw->g3DutilUnitIdx);
	GFL_G3D_UTIL_Delete(sw->g3Dutil);

	GFL_G3D_CAMERA_Delete(sw->g3Dcamera);

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
static BOOL calcTarget(GFL_G3D_CAMERA* g3Dcamera, int x, int y, VecFx32* target)
{
	BOOL result;
	fx32 near_backup;

	//変換精度を上げるためnearの距離をとる
	GFL_G3D_CAMERA_GetNear(g3Dcamera, &near_backup);
	{
		fx32 near = 64* FX32_ONE;

		GFL_G3D_CAMERA_SetNear(g3Dcamera, &near);
		GFL_G3D_CAMERA_Switching(g3Dcamera);
	}

	if( NNS_G3dScrPosToWorldLine(x, y, target, NULL) == -1 ){
		result = FALSE;
	} else {
		result = TRUE;
	}
	//near復帰
	GFL_G3D_CAMERA_SetNear(g3Dcamera, &near_backup);
	GFL_G3D_CAMERA_Switching(g3Dcamera);

	return result;
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
 * @brief	テストコマンドテーブル
 */
//------------------------------------------------------------------
typedef BOOL (COMM_FUNC)(SAMPLE3_WORK* sw);
typedef struct {
	COMM_FUNC*	commFunc;
	int					commParamNum;
}COMM_TBL;

enum {
	COMM_DMY = 0,
	COMMEND,
	COMMWAIT,
	COMMSETMSG,
	COMMCONNECTMSG,
	COMMDELMSG,

	COMM_MAX,
};
static const COMM_TBL commTbl[COMM_MAX];

enum {
	MSG_TEST1 = 0,
	MSG_TEST2,
	MSG_TEST3,
	MSG_TEST4,
	MSG_TEST5,
	MSG_TEST6,
	MSG_TEST7,
	MSG_TEST8,
	MSG_TEST9,

	MSG_TEST_MAX,
};
static const u16* testMsgTbl[MSG_TEST_MAX];
//------------------------------------------------------------------
/**
 * @brief	テストコマンド
 */
//------------------------------------------------------------------
static void commFunc(SAMPLE3_WORK* sw)
{
	int i;

	if(sw->msgComm.pCommand == NULL){ return; }

	if(sw->msgComm.command != 0){
		if( commTbl[sw->msgComm.command].commFunc(sw) == TRUE ){ return; }
	}

	sw->msgComm.command = *(sw->msgComm.pCommand);
	(sw->msgComm.pCommand)++;

	for( i=0; i<commTbl[sw->msgComm.command].commParamNum; i++ ){
		sw->msgComm.commParam[i] = *(sw->msgComm.pCommand);
		(sw->msgComm.pCommand)++;
	}
}

//------------------------------------------------------------------
static BOOL commEnd(SAMPLE3_WORK* sw)
{
	sw->msgComm.pCommand = NULL;
	sw->msgComm.command = 0;
	return TRUE;
}

//------------------------------------------------------------------
static BOOL commWait(SAMPLE3_WORK* sw)
{
	if(sw->msgComm.commParam[0] == 0){ return FALSE; }
	sw->msgComm.commParam[0]--;
	return TRUE;
}

//------------------------------------------------------------------
static BOOL commSetMsg(SAMPLE3_WORK* sw)
{
	TALKMSGWIN_SETUP tmsgwinSetup;
	u16	winIdx = sw->msgComm.commParam[0];

	tmsgwinSetup.color = GX_RGB(31,31,31);
	tmsgwinSetup.winpx = sw->msgComm.commParam[1];
	tmsgwinSetup.winpy = sw->msgComm.commParam[2];
	tmsgwinSetup.winsx = sw->msgComm.commParam[3];
	tmsgwinSetup.winsy = sw->msgComm.commParam[4];
	makeStr(testMsgTbl[sw->msgComm.commParam[5]], sw->strBuf[winIdx]);
	
	calcTarget(	sw->g3Dcamera, 
							sw->msgComm.commParam[6], 
							sw->msgComm.commParam[7], 
							&sw->twinTarget[winIdx]);

	TALKMSGWIN_CreateWindowIdx(	sw->tmsgwinSys,
															winIdx,
															&sw->twinTarget[winIdx],
															sw->strBuf[winIdx],
															&tmsgwinSetup);

	TALKMSGWIN_OpenWindow(sw->tmsgwinSys, winIdx);

	return FALSE;
}

//------------------------------------------------------------------
static BOOL commConnectMsg(SAMPLE3_WORK* sw)
{
	TALKMSGWIN_SETUP tmsgwinSetup;
	u16	winIdx = sw->msgComm.commParam[0];

	tmsgwinSetup.color = GX_RGB(31,31,31);
	tmsgwinSetup.winpx = sw->msgComm.commParam[1];
	tmsgwinSetup.winpy = sw->msgComm.commParam[2];
	tmsgwinSetup.winsx = sw->msgComm.commParam[3];
	tmsgwinSetup.winsy = sw->msgComm.commParam[4];
	makeStr(testMsgTbl[sw->msgComm.commParam[5]], sw->strBuf[winIdx]);
	
	TALKMSGWIN_CreateWindowIdxConnect(sw->tmsgwinSys,
																		winIdx,
																		sw->msgComm.commParam[6],
																		sw->strBuf[winIdx],
																		&tmsgwinSetup);

	TALKMSGWIN_OpenWindow(sw->tmsgwinSys, winIdx);

	return FALSE;
}

//------------------------------------------------------------------
static BOOL commDelMsg(SAMPLE3_WORK* sw)
{
	TALKMSGWIN_DeleteWindow(sw->tmsgwinSys, sw->msgComm.commParam[0]);

	return FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const COMM_TBL commTbl[COMM_MAX] = {
	{ NULL,							0 },
	{ commEnd,					0 },
	{ commWait,					1 },
	{ commSetMsg,				8 },
	{ commConnectMsg,		7 },
	{ commDelMsg,				1 },
};


//------------------------------------------------------------------
//------------------------------------------------------------------
static const u16* testMsgTbl[MSG_TEST_MAX] = {
	{ L"これでこの１巡だけでなく・・・" },
	{ L"４巡・・・！" },
	{ L"つまりラスト・・・" },
	{ L"１７巡までセーフ・・・！" },
	{ L"逃げ切り・・・！" },
	{ L"神が選びしは・・・" },
	{ L"やはり・・・" },
	{ L"わし・・・！" },
	{ L"ロン" },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const u16 sampleMsg1[] = {
	COMMSETMSG, 0, 10, 1, 20, 2, MSG_TEST1, 248, 184,
	COMMWAIT, 120,
	COMMCONNECTMSG, 1, 20, 6, 6, 2, MSG_TEST2, 0,
	COMMWAIT, 120,
	COMMCONNECTMSG, 2, 3, 6, 12, 2, MSG_TEST3, 1,
	COMMWAIT, 120,
	COMMCONNECTMSG, 3, 1, 11, 14, 2, MSG_TEST4, 2,
	COMMWAIT, 120,
	COMMCONNECTMSG, 4, 20, 11, 10, 2, MSG_TEST5, 3,
	COMMWAIT, 120,
	COMMCONNECTMSG, 5, 18, 16, 12, 2, MSG_TEST6, 4,
	COMMWAIT, 120,
	COMMCONNECTMSG, 6, 6, 16, 8, 2, MSG_TEST7, 5,
	COMMWAIT, 120,
	COMMCONNECTMSG, 7, 16, 20, 6, 2, MSG_TEST8, 6,
	COMMWAIT, 60,

	COMMSETMSG, 8, 6, 20, 6, 2, MSG_TEST9, 8, 184,
	COMMWAIT, 120,

	COMMDELMSG, 0,
	COMMDELMSG, 1,
	COMMDELMSG, 2,
	COMMDELMSG, 3,
	COMMDELMSG, 4,
	COMMDELMSG, 5,
	COMMDELMSG, 6,
	COMMDELMSG, 7,
	COMMDELMSG, 8,
	COMMEND,
};

static const u16* sampleMsgTbl[] = {
	sampleMsg1, sampleMsg1, sampleMsg1, sampleMsg1,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static void setSampleMsg1(SAMPLE3_WORK* sw, int idx)
{
	sw->msgComm.pCommand = sampleMsgTbl[idx];
}

