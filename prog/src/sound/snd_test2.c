//============================================================================================
/**
 * @file	soundTest.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "textprint.h"
#include "arc_def.h"
#include "sound/wb_sound_data.sadl"		//サウンドラベルファイル

#include "message.naix"
#include "font/font.naix"

#include "msg/msg_snd_test_str.h"
#include "msg/msg_snd_test_name.h"
#include "msg/msg_monsname.h"

#include "system/main.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "msg/msg_debugname.h"

#include "poke_tool/monsno_def.h"
#include "print/str_tool.h"

#include "sound/snd_status.h"
#include "sound/pm_sndsys.h"
#include "sound/pm_voice.h"

#include "arc/soundtest.naix"
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
#include "snd_test2.dat"
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	HEAPID				heapID;
	int					seq;
	int					timer;

	GFL_SNDSTATUS*		gflSndStatus;
	GFL_SKB*			gflSkb;
	BOOL				gflSkbSw;

	void*				skbStrBuf;

	GFL_MSGDATA* msgman;			//メッセージマネージャー
	GFL_MSGDATA* monsmsgman;		//メッセージマネージャー

	BMP_MENULIST_DATA	*menuList;
	BMPMENULIST_WORK	*menuWork;
	PRINT_UTIL			printUtil;
	PRINT_QUE			*printQue;
	GFL_FONT 			*fontHandle;

	GFL_BMPWIN*			bmpwin[IDX_MAX];	

	GFL_TCB*			g2dVintr;		//2Dシステム用vIntrTaskハンドル

	int					bgmNum;
	int					seNum;
	int					voiceNum;

	int					mode;

	BOOL				soundTestFlag;
}SOUNDTEST_WORK;

enum {
	MODE_SOUND_SELECT = 0,
	MODE_SOUND_CONTROL,
};
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
static BOOL	SoundTest(SOUNDTEST_WORK* sw);
//------------------------------------------------------------------
/**
 * @brief	ワークの初期化＆破棄
 */
//------------------------------------------------------------------
static void	SoundWorkInitialize(SOUNDTEST_WORK* sw)
{
	sw->skbStrBuf = GFL_SKB_CreateSjisCodeBuffer(sw->heapID);
	sw->seq = 0;

	sw->bgmNum		= PMSND_BGM_START;
	sw->seNum		= PMSND_SE_START;
	sw->voiceNum	= PMVOICE_START;

	sw->mode = MODE_SOUND_SELECT;
}

static void	SoundWorkFinalize(SOUNDTEST_WORK* sw)
{
	GFL_SKB_DeleteSjisCodeBuffer(sw->skbStrBuf);
}

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT SoundTest2Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SOUNDTEST_WORK* sw = NULL;

	sw = GFL_PROC_AllocWork(proc, sizeof(SOUNDTEST_WORK), GFL_HEAPID_APP);

    GFL_STD_MemClear(sw, sizeof(SOUNDTEST_WORK));
    sw->heapID = GFL_HEAPID_APP;

	SoundWorkInitialize(sw);

	{
		GFL_SNDSTATUS_SETUP sndStatusSetup;

		sndStatusSetup = sndStatusData;
		sndStatusSetup.pBgmHandle = PMSND_GetBGMhandlePointer();

		sw->gflSndStatus = GFL_SNDSTATUS_Create( &sndStatusSetup, sw->heapID );
	}

    return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTest2Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SOUNDTEST_WORK*	sw;
	sw = mywk;

	GFL_SNDSTATUS_Main( sw->gflSndStatus );

	if(SoundTest(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTest2Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SOUNDTEST_WORK*	sw;
	sw = mywk;

	GFL_SNDSTATUS_Delete( sw->gflSndStatus );

	SoundWorkFinalize(sw);

	GFL_PROC_FreeWork(proc);

    return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA SoundTest2ProcData = {
	SoundTest2Proc_Init,
	SoundTest2Proc_Main,
	SoundTest2Proc_End,
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
static void	SetupSoundTestSys(SOUNDTEST_WORK* sw);
static void	RemoveSoundTestSys(SOUNDTEST_WORK* sw);
static void	MainSoundTestSys(SOUNDTEST_WORK* sw);

static BOOL checkTouchPanelEvent(SOUNDTEST_WORK* sw);
//------------------------------------------------------------------
/**
 *
 * @brief	コントロール切り替え
 *
 */
//------------------------------------------------------------------
static void	checkControlChange(SOUNDTEST_WORK* sw)
{
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		if( sw->mode == MODE_SOUND_SELECT ){
			GFL_SNDSTATUS_SetControlEnable
				( sw->gflSndStatus, GFL_SNDSTATUS_CONTOROL_BGM | GFL_SNDSTATUS_CONTOROL_SE );
			GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
			sw->mode = MODE_SOUND_CONTROL;
		} else {
			GFL_SNDSTATUS_SetControlEnable
				( sw->gflSndStatus, GFL_SNDSTATUS_CONTOROL_NONE );
			GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
			sw->mode = MODE_SOUND_SELECT;
		}
	} 
}

//------------------------------------------------------------------
/**
 *
 * @brief	フレームワーク
 *
 */
//------------------------------------------------------------------
static BOOL	SoundTest(SOUNDTEST_WORK* sw)
{
	switch(sw->seq){
	case 0:
		SetupSoundTestSys(sw);
		sw->soundTestFlag = FALSE;
		sw->seq++;
		break;

	case 1:
		MainSoundTestSys(sw);
		checkControlChange(sw);

		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
			if( sw->soundTestFlag == FALSE ){
				PMSND_EnableCaptureReverb( 16000, 63, 0 );
				sw->soundTestFlag = TRUE;
			} else {
				PMSND_DisableCaptureReverb();
				sw->soundTestFlag = FALSE;
			}
			break;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			//sw->seq = 100;
			break;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
			PMSND_PlaySystemSE(sw->seNum);
			break;
		}

		if( sw->mode == MODE_SOUND_SELECT ){
			checkTouchPanelEvent(sw);
		}
		break;

	case 2:
		RemoveSoundTestSys(sw);
		return FALSE;
//---------------------
	case 100:
		//PMSND_PlayVoiceChorus( sw->voiceNum, 16, 110 );
		sw->seq++;
		break;
	case 101:
		//if( PMSND_CheckPlayVoiceChorus() == FALSE ){
		//	sw->seq = 1;
		//}
		break;
//---------------------
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
 * @brief	セットアップ
 *
 *
 *
 *
 *
 */
//============================================================================================
static void	bg_init(SOUNDTEST_WORK* sw);
static void	bg_exit(SOUNDTEST_WORK* sw);

static void	g2d_load(SOUNDTEST_WORK* sw);
static void g2d_draw(SOUNDTEST_WORK* sw);
static void	g2d_unload(SOUNDTEST_WORK* sw);
static void	g2d_vblank( GFL_TCB* tcb, void* work );

static void printNum(GFL_BMPWIN* bmpwin, int num, u8 y_lineoffs);
//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static void	SetupSoundTestSys(SOUNDTEST_WORK* sw)
{
	//VRAM設定
	GFL_DISP_SetBank(&dispVram);
	//BG初期化
	bg_init(sw);
	//２Ｄデータのロード
	g2d_load(sw);
}

//------------------------------------------------------------------
/**
 * @brief	開放関数
 */
//------------------------------------------------------------------
static void	RemoveSoundTestSys(SOUNDTEST_WORK* sw)
{
	g2d_unload(sw);
	bg_exit(sw);
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数
 */
//------------------------------------------------------------------
static void	MainSoundTestSys(SOUNDTEST_WORK* sw)
{
	g2d_draw(sw);
}

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init(SOUNDTEST_WORK* sw)
{
	//ＢＧシステム起動
	GFL_BG_Init(sw->heapID);

	//ＢＧモード設定
	GFL_BG_SetBGMode(&bgsysHeader);

	//ＢＧコントロール設定
	GFL_BG_SetBGControl(BG_FRAME, &BGcont, GFL_BG_MODE_TEXT);
	GFL_BG_SetPriority(BG_FRAME, 1);
	GFL_BG_SetVisible(BG_FRAME, VISIBLE_ON);
	GFL_BG_SetBGControl(TEXT_FRAME, &Textcont, GFL_BG_MODE_TEXT);
	GFL_BG_SetPriority(TEXT_FRAME, 0);
	GFL_BG_SetVisible(TEXT_FRAME, VISIBLE_ON);
	GFL_BG_SetBGControl(STATUS_FRAME, &Statuscont, GFL_BG_MODE_TEXT);
	GFL_BG_SetPriority(STATUS_FRAME, 0);
	GFL_BG_SetVisible(STATUS_FRAME, VISIBLE_ON);

	//ビットマップウインドウ起動
	GFL_BMPWIN_Init(sw->heapID);

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
	GFL_DISP_SetDispOn();
}

static void	bg_exit(SOUNDTEST_WORK* sw)
{
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl(STATUS_FRAME);
	GFL_BG_FreeBGControl(TEXT_FRAME);
	GFL_BG_FreeBGControl(BG_FRAME);
	GFL_BG_Exit();
}

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load(SOUNDTEST_WORK* sw)
{
	int i;

	//背景読み込み
	void* bg_pltt = GFL_ARC_LoadDataAlloc
						(ARCID_SOUNDTEST, NARC_soundtest_soundtest_NCLR, sw->heapID);
	void* bg_char = GFL_ARC_LoadDataAlloc
						(ARCID_SOUNDTEST, NARC_soundtest_soundtest_NCGR, sw->heapID);
	void* bg_scrn = GFL_ARC_LoadDataAlloc
						(ARCID_SOUNDTEST, NARC_soundtest_soundtest_NSCR, sw->heapID);
	
	GFL_BG_LoadPalette
		(BG_FRAME, GFL_DISPUT_GetPltDataNNSbin(bg_pltt), PLTT_1*BG_PLTTNUM, PLTT_1*BG_PLTTID);
	GFL_BG_LoadCharacter
		(BG_FRAME, GFL_DISPUT_GetCgxDataNNSbin(bg_char), CHAR_1*BG_CHARNUM, CHAR_1*BG_CHARID);
	GFL_BG_LoadScreenBuffer
		(BG_FRAME, GFL_DISPUT_GetScrDataNNSbin(bg_scrn), SCRN_1*BG_SCRNNUM);

	GFL_HEAP_FreeMemory(bg_scrn);
	GFL_HEAP_FreeMemory(bg_char);
	GFL_HEAP_FreeMemory(bg_pltt);

	GFL_BG_LoadScreenReq(BG_FRAME);

	//文字表示ビットマップの作成
	GFL_BG_FillCharacter(TEXT_FRAME, 0, 1, 0);	// 先頭にクリアキャラ配置
	GFL_BG_ClearScreen(TEXT_FRAME);

	for(i=0; i<IDX_MAX;	i++){
		sw->bmpwin[i] = GFL_BMPWIN_Create(	TEXT_FRAME,
											bmpWinSt[i][0], bmpWinSt[i][1],
											bmpWinSt[i][2], bmpWinSt[i][3],
											TEXT_PLTTID, GFL_BG_CHRAREA_GET_F );
		GFL_BMPWIN_MakeScreen(sw->bmpwin[i]);
	}
	GFL_BG_LoadScreenReq(TEXT_FRAME);

	sw->g2dVintr = GFUser_VIntr_CreateTCB(g2d_vblank, (void*)sw, 0);
}

static void g2d_draw(SOUNDTEST_WORK* sw)
{
	int i;

	printNum(sw->bmpwin[IDX_BGM], sw->bgmNum, SNUM_PRINT_OFFS);
	printNum(sw->bmpwin[IDX_SE], sw->seNum, SNUM_PRINT_OFFS);
	printNum(sw->bmpwin[IDX_VOICE], sw->voiceNum, SNUM_PRINT_OFFS);

	//文字更新
	for(i=0; i<IDX_MAX; i++){ GFL_BMPWIN_TransVramCharacter(sw->bmpwin[i]); }
}

static void	g2d_unload(SOUNDTEST_WORK* sw)
{
	int i;

	GFL_TCB_DeleteTask(sw->g2dVintr);

	for(i=0; i<IDX_MAX; i++){ GFL_BMPWIN_Delete(sw->bmpwin[i]); }
}

static void	g2d_vblank(GFL_TCB* tcb, void* work)
{
	SOUNDTEST_WORK* sw =  (SOUNDTEST_WORK*)work;
	GFL_CLACT_SYS_VBlankFunc();
	//GFL_BG_VBlankFunc();
}

//------------------------------------------------------------------
/**
 * @brief		数字表示
 */
//------------------------------------------------------------------
static void printNum(GFL_BMPWIN* bmpwin, int num, u8 y_lineoffs)
{
	GFL_TEXT_PRINTPARAM	printparam;
	int numOffs[4];		// 4桁分
	int i;
	BOOL f;

	GFL_BMP_Clear(GFL_BMPWIN_GetBmp(bmpwin), 0);

	printparam.bmp = GFL_BMPWIN_GetBmp(bmpwin);
	printparam.writex = 0;
	printparam.writey = y_lineoffs;
	printparam.spacex = 1;
	printparam.spacey = 0;
	printparam.colorF = 1;	// 黒
	printparam.colorB = 0;	// 透明
	printparam.mode = GFL_TEXT_WRITE_16;

	numOffs[0] = num /1000;
	numOffs[1] = (num % 1000) /100;
	numOffs[2] = ((num % 1000) %100) /10;
	numOffs[3] = ((num % 1000) %100) %10;

	f = FALSE;

	//テキストをビットマップに表示
	for(i=0; i<4; i++){
		char numStr[] = {"0"};
		char spcStr[] = {" "};

		if((numOffs[i] == 0)&&(f == FALSE)){
			GFL_TEXT_PrintSjisCode(spcStr, &printparam);
		} else {
			numStr[0] += numOffs[i];
			GFL_TEXT_PrintSjisCode(numStr, &printparam);
			f = TRUE;
		}
	}
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
#define SEQ_VARIABLE_NUM (16)
typedef struct {
	s16		val;
	BOOL	enable;
}SEQ_BACKUP_STATUS;

typedef struct {
	SEQ_BACKUP_STATUS	localVariable[SEQ_VARIABLE_NUM];
	SEQ_BACKUP_STATUS	globalVariable[SEQ_VARIABLE_NUM];
}SEQ_BACKUP_PARAM;

static SEQ_BACKUP_PARAM seqBackupParam;

static void SeqBackup( NNSSndHandle* handle )
{
	int		i;
	s16		val;
	BOOL	enable;

	for( i=0; i<SEQ_VARIABLE_NUM; i++ ){
		enable = NNS_SndPlayerReadVariable( handle, i, &val ); 
		seqBackupParam.localVariable[i].enable = enable;
		seqBackupParam.localVariable[i].val = val;
		if( enable == TRUE ){
			OS_Printf("read success varNum = %d\n", i);
		} else {
			OS_Printf("read failed varNum = %d\n", i);
		}
	}
	for( i=0; i<SEQ_VARIABLE_NUM; i++ ){
		enable = NNS_SndPlayerReadGlobalVariable( i, &val ); 
		seqBackupParam.globalVariable[i].enable = enable;
		seqBackupParam.globalVariable[i].val = val;
	}
}

static void SeqRecover( NNSSndHandle* handle )
{
	int		i;
	s16		val;
	BOOL	result;

	for( i=0; i<SEQ_VARIABLE_NUM; i++ ){
		if( seqBackupParam.localVariable[i].enable == TRUE ){
			val = seqBackupParam.localVariable[i].val;
			result = NNS_SndPlayerWriteVariable( handle, i, val ); 
			if( result == TRUE ){
				OS_Printf("write success varNum = %d\n", i);
			} else {
				OS_Printf("write failed varNum = %d\n", i);
			}
		}
	}
	for( i=0; i<SEQ_VARIABLE_NUM; i++ ){
		if( seqBackupParam.globalVariable[i].enable == TRUE ){
			val = seqBackupParam.globalVariable[i].val;
			NNS_SndPlayerWriteGlobalVariable( i, val ); 
		}
	}
}

static BOOL pauseSw = FALSE;
//------------------------------------------------------------------
/**
 * @brief	タッチパネルイベント判定
 */
//------------------------------------------------------------------
static BOOL checkTouchPanelEvent(SOUNDTEST_WORK* sw)
{
	int tblPos = GFL_UI_TP_HitTrg(eventTouchPanelTable);

	if(tblPos == GFL_UI_TP_HIT_NONE){
		return FALSE;
	} 
	switch(tblPos){
		case SOUNDTEST_TPEV_BGM_NUMUP:
			if(sw->bgmNum < PMSND_BGM_END){ sw->bgmNum++; }
			break;
		case SOUNDTEST_TPEV_BGM_NUMDOWN:
			if(sw->bgmNum > PMSND_BGM_START){ sw->bgmNum--; }
			break;
		case SOUNDTEST_TPEV_BGM_NUMSET:
			OS_Printf("pressed bgm num_set\n");
			break;
		case SOUNDTEST_TPEV_BGM_PLAY:
			PMSND_PlayNextBGM(sw->bgmNum);
			GFL_SNDSTATUS_ChangeSndHandle(sw->gflSndStatus, PMSND_GetBGMhandlePointer());
			GFL_SNDSTATUS_InitControl(sw->gflSndStatus);
			pauseSw = FALSE;
			break;
		case SOUNDTEST_TPEV_BGM_STOP:
			PMSND_StopBGM();
			break;
		case SOUNDTEST_TPEV_BGM_PAUSE:
			if( pauseSw == FALSE ){
				PMSND_PauseBGM(TRUE);
				//PMSND_PushBGM();
				pauseSw = TRUE;
			} else {
				//PMSND_PopBGM();
				PMSND_PauseBGM(FALSE);
				pauseSw = FALSE;
			}
			break;
		case SOUNDTEST_TPEV_BGM_WINDOW:
			break;

		case SOUNDTEST_TPEV_SE_NUMUP:
			if(sw->seNum < PMSND_SE_END){ sw->seNum++; }
			break;
		case SOUNDTEST_TPEV_SE_NUMDOWN:
			if(sw->seNum > PMSND_SE_START){ sw->seNum--; }
			break;
		case SOUNDTEST_TPEV_SE_NUMSET:
			OS_Printf("pressed se num_se\n");
			break;
		case SOUNDTEST_TPEV_SE_PLAY:
			PMSND_PlaySE(sw->seNum);
			break;
		case SOUNDTEST_TPEV_SE_STOP:
			//NNS_SndPlayerStopSeq(&sw->seHandle, 0);
			PMSND_PauseBGM(TRUE);
			PMSND_PushBGM();
			break;
		case SOUNDTEST_TPEV_SE_PAUSE:
			PMSND_PopBGM();
			GFL_SNDSTATUS_ChangeSndHandle(sw->gflSndStatus, PMSND_GetBGMhandlePointer());
			PMSND_PauseBGM(FALSE);
			break;
		case SOUNDTEST_TPEV_SE_WINDOW:
			break;

		case SOUNDTEST_TPEV_VOICE_NUMUP:
			if(sw->voiceNum < PMVOICE_END){ sw->voiceNum++; }
			break;
		case SOUNDTEST_TPEV_VOICE_NUMDOWN:
			if(sw->voiceNum > PMVOICE_START){ sw->voiceNum--; }
			break;
		case SOUNDTEST_TPEV_VOICE_NUMSET:
			OS_Printf("pressed voice num_set\n");
			break;
		case SOUNDTEST_TPEV_VOICE_PLAY:
			PMVOICE_Play(sw->voiceNum, PMVOICE_MODE_NORMAL | PMVOICE_MODE_CHORUS);
			break;
		case SOUNDTEST_TPEV_VOICE_STOP:
			PMVOICE_Play(sw->voiceNum, PMVOICE_MODE_REVERSE | PMVOICE_MODE_CHORUS);
			break;
		case SOUNDTEST_TPEV_VOICE_PAUSE:
			//OS_Printf("pressed voice pause\n");
			break;
		case SOUNDTEST_TPEV_VOICE_WINDOW:
			break;
		case SOUNDTEST_TPEV_EXIT:
			sw->seq++;
			break;
	}
	return TRUE;
}

