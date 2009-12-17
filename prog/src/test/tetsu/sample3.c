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
#include "msg/msg_d_tetsu.h"

#include "debug_common.h"
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
#define WIN_PLTTID		(14)
#define TEXT_PLTTID		(15)

#define BACKGROUND_COLOR (0)
#define BACKGROUND_COLOR2 (0x7c00)
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
	DWS_SYS*						dws;

	int									seq;

	int									timer;

	TALKMSGWIN_SYS*			tmsgwinSys;
	int									tmsgwinIdx;
	VecFx32							cameraTarget;
	BOOL								tmsgwinConnect;

	STRBUF*							strBuf[TALKMSGWIN_NUM];
	VecFx32							twinTarget[TALKMSGWIN_NUM];

	TEST_MSG_COMM				msgComm;

	int									mode;

	int									testPat;
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

	sw->tmsgwinSys = NULL;
	sw->timer = 0;
	sw->tmsgwinIdx = 0;
	sw->tmsgwinConnect = FALSE;
	sw->mode = 0;

	for(i=0; i<TALKMSGWIN_NUM; i++){ sw->strBuf[i] = GFL_STR_CreateBuffer(STRBUF_SIZE, sw->heapID); }

	sw->dws = DWS_SYS_Setup(sw->heapID);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample3Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE3_WORK*	sw;
	sw = mywk;

	DWS_SYS_Framework(sw->dws);

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

	DWS_SYS_Delete(sw->dws);

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
static void commFunc(SAMPLE3_WORK* sw);

static void setSampleMsg1(SAMPLE3_WORK* sw, int idx);
static BOOL calcTarget(GFL_G3D_CAMERA* g3Dcamera, int x, int y, VecFx32* target);
//------------------------------------------------------------------
/**
 *
 * @brief	フレームワーク
 *
 */
//------------------------------------------------------------------
#define FIX_WIN_IDX	(0)
static BOOL	sample3(SAMPLE3_WORK* sw)
{
	//モードセレクト
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		if( sw->mode == 0 ){
			DWS_CamAdjustOff(sw->dws);
			sw->mode = 1;
		} else {
			DWS_CamAdjustOn(sw->dws);
			sw->mode = 0;
		}
	} 
	//コントロール
	switch(sw->seq){
	case 0:
		{
			TALKMSGWIN_SYS_INI ini = {TEXT_FRAME, WIN_PLTTID, TEXT_PLTTID};
			TALKMSGWIN_SYS_SETUP setup;		
			setup.heapID = sw->heapID;
			setup.g3Dcamera = DWS_GetG3Dcamera(sw->dws);
			setup.fontHandle = DWS_GetFontHandle(sw->dws);
			setup.chrNumOffs = 0x10;
			setup.ini = ini;
	
			sw->tmsgwinSys = TALKMSGWIN_SystemCreate(&setup);
			TALKMSGWIN_SystemDebugOn(sw->tmsgwinSys);
		}
		sw->testPat = 0;
		sw->seq++;
		return TRUE;

	case 1:
		if( sw->testPat == 0 ){
			VEC_Set(&sw->cameraTarget,0,0,0);
			GFL_BG_SetBackGroundColor( TEXT_FRAME, BACKGROUND_COLOR );
		} else {
			VEC_Set(&sw->cameraTarget,0x10000*FX32_ONE,0,0*FX32_ONE);
			GFL_BG_SetBackGroundColor( TEXT_FRAME, BACKGROUND_COLOR2 );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
			sw->seq = 4;
			break;
		}
		if( sw->mode == 0 ){
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
				setSampleMsg1(sw, 0);
				sw->seq = 2;
			} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
			} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
			} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
			} else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
				if(sw->testPat)	{ sw->testPat = 0; }
				else						{ sw->testPat = 1; }
			}
		} else {
			u32 tpx, tpy;

			if( GFL_UI_TP_GetPointTrg( &tpx, &tpy ) == TRUE ){
				GFL_MSG_GetString(DWS_GetMsgManager(sw->dws), DEBUG_TETSU_STR0_4, sw->strBuf[FIX_WIN_IDX]);
				//calcTarget(	sw->g3Dcamera, tpx, tpy, &sw->twinTarget[FIX_WIN_IDX]);
				VEC_Set(&sw->twinTarget[FIX_WIN_IDX], 
								sw->cameraTarget.x + 16*FX32_ONE,
								sw->cameraTarget.y - 16*FX32_ONE,
								sw->cameraTarget.z + 16*FX32_ONE);

				TALKMSGWIN_CreateFixWindowAuto
					(sw->tmsgwinSys, FIX_WIN_IDX, &sw->twinTarget[FIX_WIN_IDX], sw->strBuf[FIX_WIN_IDX], 15, TALKMSGWIN_TYPE_NORMAL );
				TALKMSGWIN_OpenWindow(sw->tmsgwinSys, FIX_WIN_IDX);
				sw->seq = 3;
			}
		}
		break;

	case 2:
		commFunc(sw);

		if(sw->msgComm.pCommand == NULL){ sw->seq = 1; }
		break;

	case 3:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
			TALKMSGWIN_DeleteWindow(sw->tmsgwinSys, FIX_WIN_IDX);
			sw->seq = 1;
		}
		break;

	case 4:
		TALKMSGWIN_SystemDelete(sw->tmsgwinSys);
		return FALSE;
	}
	//メイン動作
	if( sw->tmsgwinSys ){
		TALKMSGWIN_SystemMain(sw->tmsgwinSys);
		TALKMSGWIN_SystemDraw2D(sw->tmsgwinSys);
	}
	//描画
	{
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();

		if( sw->tmsgwinSys ){ TALKMSGWIN_SystemDraw3D(sw->tmsgwinSys); }
		DWS_DrawLocalOriginPlane(sw->dws, 32*FX32_ONE, GX_RGB(31, 0, 0));

		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
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
	GFL_MSGDATA* msgManager = DWS_GetMsgManager(sw->dws);
	u16	winIdx = sw->msgComm.commParam[0];
	u16	msgID = sw->msgComm.commParam[3];
	u16	msgWidth;
	u16	msgHeight = 2;

	GFL_MSG_GetString(msgManager, msgID, sw->strBuf[winIdx]);
	msgWidth = GFL_MSG_GetDispAreaWidth(msgManager, msgID) / 8;

//	calcTarget(	sw->g3Dcamera, 
//							sw->msgComm.commParam[4], 
//							sw->msgComm.commParam[5], 
//							&sw->twinTarget[winIdx]);
	VEC_Set(&sw->twinTarget[winIdx], 
					sw->cameraTarget.x + 16*FX32_ONE,
					sw->cameraTarget.y - 16*FX32_ONE,
					sw->cameraTarget.z + 16*FX32_ONE);

	TALKMSGWIN_CreateFloatWindowIdx(	sw->tmsgwinSys,
																		winIdx,
																		&sw->twinTarget[winIdx],
																		sw->strBuf[winIdx],
																		sw->msgComm.commParam[1] - msgWidth/2,
																		sw->msgComm.commParam[2],
																		msgWidth, msgHeight,
																		15, TALKMSGWIN_TYPE_NORMAL );

	TALKMSGWIN_OpenWindow(sw->tmsgwinSys, winIdx);

	return FALSE;
}

//------------------------------------------------------------------
static BOOL commConnectMsg(SAMPLE3_WORK* sw)
{
	GFL_MSGDATA* msgManager = DWS_GetMsgManager(sw->dws);
	u16	winIdx = sw->msgComm.commParam[0];
	u16	msgID = sw->msgComm.commParam[3];
	u16	msgWidth;
	u16	msgHeight = 2;

	GFL_MSG_GetString(msgManager, msgID, sw->strBuf[winIdx]);
	msgWidth = GFL_MSG_GetDispAreaWidth(msgManager, msgID) / 8;
	
	TALKMSGWIN_CreateFloatWindowIdxConnect( sw->tmsgwinSys,
																					winIdx,
																					sw->msgComm.commParam[4],
																					sw->strBuf[winIdx],
																					sw->msgComm.commParam[1] - msgWidth/2,
																					sw->msgComm.commParam[2],
																					msgWidth, msgHeight,
																					15, TALKMSGWIN_TYPE_NORMAL );

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
	{ commSetMsg,				6 },
	{ commConnectMsg,		5 },
	{ commDelMsg,				1 },
};


//------------------------------------------------------------------
//------------------------------------------------------------------
static const u16 sampleMsg1[] = {
	COMMSETMSG,				0, 20, 2, DEBUG_TETSU_STR0_1, 128, 160,
	COMMWAIT,					120,
	COMMCONNECTMSG,		1, 19, 8, DEBUG_TETSU_STR0_2, 0,
	COMMWAIT,					120,
	COMMCONNECTMSG,		2, 18, 14, DEBUG_TETSU_STR0_3, 1,
	COMMWAIT,					120,

	COMMDELMSG,				0,
	COMMDELMSG,				1,
	COMMDELMSG,				2,
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

