//============================================================================================
/**
 * @file	snd_status.c
 * @brief	サウンドステータスビューワー
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "arc_def.h"

#include "sound/snd_viewer.h"
//============================================================================================
/**
 *
 *
 *
 * @brief	サウンドステータスビューワー
 *
 *
 *
 */
//============================================================================================
// グラフィックアーカイブ内容ＩＤ定義
enum {
	NARC_soundstatus_soundstatus_NCGR = 0,
	NARC_soundstatus_soundstatus_NCLR = 1,
	NARC_soundstatus_soundstatus_NSCR = 2,
	NARC_soundstatus_soundstatus_vsw_NSCR = 3,
};

static const char arc_path[] = {"soundstatus.narc"};
//============================================================================================
// コントロール定義
enum {
	EVENT_CHECK_MODE = 0,
	SWITCH_SLIDE_MODE,
};

typedef enum {
	SWITCH_TR1 = 0,
	SWITCH_TR2,
	SWITCH_TR3,
	SWITCH_TR4,
	SWITCH_TR5,
	SWITCH_TR6,
	SWITCH_TR7,
	SWITCH_TR8,
	SWITCH_TR9,
	SWITCH_TR10,
	SWITCH_TR11,
	SWITCH_TR12,
	SWITCH_TR13,
	SWITCH_TR14,
	SWITCH_TR15,
	SWITCH_TR16,

	SWITCH_BGM_VOL,
	SWITCH_BGM_TEMPO,
	SWITCH_BGM_PITCH,
	SWITCH_BGM_REVERB,

	SWITCH_TREF_MODD,
	SWITCH_TREF_MODS,
	SWITCH_TREF_DUMMY,

	SWITCH_MAX,
}SWITCH_ID;

enum {
	TRACK_STBTN_NO_VOL = 0,
	TRACK_STBTN_VOL_ON,
	TRACK_STBTN_EFFECT_ON,

	TRACK_STBTN_NUM,
};

//============================================================================================
#define CHANNEL_NUM (16)
#define TRACK_NUM	(16)
#define SWITCH_VAL_MAX	(32)
#define SWITCH_VAL_ZERO	(16)
#define SWITCH_VAL_MIN	(0)

typedef struct {
	s8		valOffs;
}SWITCH_STATUS;

typedef struct {
	SWITCH_ID	volume;
	SWITCH_ID	tempo;
	SWITCH_ID	pitch;
	SWITCH_ID	reverb;
}MASTERTRACK_STATUS;

typedef struct {
	SWITCH_ID	mod_d;
	SWITCH_ID	mod_s;
	SWITCH_ID	dummy;
}TRACKEFFECT_STATUS;

typedef struct {
	SWITCH_ID	volume;
	BOOL		active;
	int			stBtn;
}TRACK_STATUS;

typedef struct {
	u8			seq;
	SWITCH_ID	swID;
	u8			tpy;
	u8			rectL;
}SWITCH_CONTROL;

struct _GFL_SNDVIEWER {
	HEAPID					heapID;
	int						seq;
	GFL_SNDVIEWER_SETUP		setup;
	GFL_DISPUT_VRAMSV*		vramSv;

	NNSSndHandle*			sndHandle;
	u32						playerNoIdx;
	BOOL					reverb;

	SNDChannelInfo			channelInfo[CHANNEL_NUM];
	SNDTrackInfo			bgmTrackInfo[TRACK_NUM];
	TRACK_STATUS			bgmTrackStatus[TRACK_NUM];
	MASTERTRACK_STATUS		bgmMasterTrackStatus;
	TRACKEFFECT_STATUS		bgmTrackEffectStatus;
	BOOL					bgmTrackEffectSw;
	u8						trackNumStack[8];	// プレーヤー階層分（現状は5）必要

	SWITCH_CONTROL			swControl;
	SWITCH_STATUS			switchStatus[SWITCH_MAX];
	GFL_UI_TP_HITTBL		eventSwitchTable[SWITCH_MAX+1];

	void*					volumeSwScrnPattern;
	u16						scrnBuf[32*32];
};

#define PLT_SIZ			(0x20)
#define PUSH_CGX_SIZ	(0x2000)
#define PUSH_SCR_SIZ	(0x800)

static void initStatus( GFL_SNDVIEWER* gflSndViewer );
static void initSwitchControl( GFL_SNDVIEWER* gflSndViewer );

static void loadGraphicData( GFL_SNDVIEWER* gflSndViewer );
static void makeEventSwitchTable( GFL_SNDVIEWER* gflSndViewer );

static BOOL SNDVIEWER_SetInfo( GFL_SNDVIEWER* gflSndViewer );
static BOOL SNDVIEWER_Control( GFL_SNDVIEWER* gflSndViewer );

static void SNDVIEWER_SetMod( GFL_SNDVIEWER* gflSndViewer, BOOL enable );
static void SNDVIEWER_SetTrackStatus( GFL_SNDVIEWER* gflSndViewer, int trackNo );
static void SNDVIEWER_SetSwitchParam( GFL_SNDVIEWER* gflSndViewer, SWITCH_ID swID );
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
extern GFL_SNDVIEWER*	GFL_SNDVIEWER_Create( const GFL_SNDVIEWER_SETUP* setup, HEAPID heapID )
{
	GFL_SNDVIEWER* gflSndViewer;
	int i;

	gflSndViewer = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), sizeof(GFL_SNDVIEWER) );

	gflSndViewer->heapID = heapID;
	gflSndViewer->setup = *setup;
	gflSndViewer->reverb = FALSE;

	gflSndViewer->seq = 0;
	{
		u16	bgPalMask = 0x0001 << gflSndViewer->setup.bgPalID;
		gflSndViewer->vramSv =  GFL_DISPUT_CreateVramSv(gflSndViewer->setup.bgID, 
														PUSH_CGX_SIZ, PUSH_SCR_SIZ, 
														bgPalMask, heapID );
	}
	gflSndViewer->volumeSwScrnPattern = GFL_ARC_LoadDataFilePathAlloc
					( arc_path, NARC_soundstatus_soundstatus_vsw_NSCR, heapID );

	initStatus( gflSndViewer );
	initSwitchControl( gflSndViewer );

	return gflSndViewer;
}

void	GFL_SNDVIEWER_Delete( GFL_SNDVIEWER* gflSndViewer )
{
	if( gflSndViewer == NULL ) return;

	GFL_HEAP_FreeMemory( gflSndViewer->volumeSwScrnPattern );

	GFL_DISPUT_DeleteVramSv( gflSndViewer->vramSv );
	GFL_HEAP_FreeMemory( gflSndViewer );
}

void	GFL_SNDVIEWER_InitControl( GFL_SNDVIEWER* gflSndViewer )
{
	initStatus( gflSndViewer );
	initSwitchControl( gflSndViewer );
}

void	GFL_SNDVIEWER_InitReverbControl( GFL_SNDVIEWER* gflSndViewer )
{
	gflSndViewer->switchStatus[SWITCH_BGM_REVERB].valOffs = SWITCH_VAL_MAX;
}

u16		GFL_SNDVIEWER_GetControl( GFL_SNDVIEWER* gflSndViewer )
{
	if( gflSndViewer == NULL ) return 0;

	return gflSndViewer->setup.controlFlag;
}

void	GFL_SNDVIEWER_SetControl( GFL_SNDVIEWER* gflSndViewer, u16 flag )
{
	if( gflSndViewer == NULL ) return;

	gflSndViewer->setup.controlFlag = flag;
	makeEventSwitchTable( gflSndViewer );
}

void	GFL_SNDVIEWER_ChangeSndHandle( GFL_SNDVIEWER* gflSndViewer, NNSSndHandle* pBgmHandle )
{
	//if( gflSndViewer == NULL ) return;

	//gflSndViewer->setup.pBgmHandle = pBgmHandle;
}

//============================================================================================
static void initStatus( GFL_SNDVIEWER* gflSndViewer )
{
	SWITCH_ID swID;
	int i;

	for( i=0; i<TRACK_NUM; i++ ){
		swID = SWITCH_TR1 + i;
		gflSndViewer->bgmTrackStatus[i].volume = swID;
		gflSndViewer->bgmTrackStatus[i].active = FALSE;
		gflSndViewer->bgmTrackStatus[i].stBtn = TRACK_STBTN_VOL_ON;
		gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_MAX;
	}
	swID = SWITCH_BGM_VOL;
	gflSndViewer->bgmMasterTrackStatus.volume = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_MAX;
	
	swID = SWITCH_BGM_TEMPO;
	gflSndViewer->bgmMasterTrackStatus.tempo = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_ZERO;

	swID = SWITCH_BGM_PITCH;
	gflSndViewer->bgmMasterTrackStatus.pitch = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_ZERO;

	swID = SWITCH_BGM_REVERB;
	gflSndViewer->bgmMasterTrackStatus.reverb = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_MAX;

	swID = SWITCH_TREF_MODD;
	gflSndViewer->bgmTrackEffectStatus.mod_d = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_MIN;
	
	swID = SWITCH_TREF_MODS;
	gflSndViewer->bgmTrackEffectStatus.mod_s = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_ZERO;

	swID = SWITCH_TREF_DUMMY;
	gflSndViewer->bgmTrackEffectStatus.dummy = swID;
	gflSndViewer->switchStatus[swID].valOffs = SWITCH_VAL_ZERO;

	gflSndViewer->bgmTrackEffectSw = FALSE;
}
	
static void initSwitchControl( GFL_SNDVIEWER* gflSndViewer )
{
	gflSndViewer->swControl.seq = EVENT_CHECK_MODE;
	gflSndViewer->swControl.swID = SWITCH_MAX;
	gflSndViewer->swControl.tpy = 0;
	gflSndViewer->swControl.rectL = 0;

	makeEventSwitchTable( gflSndViewer );
}

static void updateSndSystemInfo( GFL_SNDVIEWER* gflSndViewer )
{
	if( gflSndViewer->setup.getSndHandleFunc != NULL ){
		gflSndViewer->sndHandle = gflSndViewer->setup.getSndHandleFunc();
	} else {
		gflSndViewer->sndHandle = NULL;
	}
	if( gflSndViewer->setup.getPlayerNoFunc != NULL ){
		gflSndViewer->playerNoIdx = gflSndViewer->setup.getPlayerNoFunc();
	} else {
		gflSndViewer->playerNoIdx = 0;
	}
	if( gflSndViewer->setup.getReverbFlagFunc != NULL ){
		gflSndViewer->reverb = gflSndViewer->setup.getReverbFlagFunc();
	} else {
		gflSndViewer->reverb = FALSE;
	}
}

//============================================================================================
/**
 *
 * @brief	システムメイン
 *
 */
//============================================================================================
enum {
	SEQ_PUSHVRAM = 0,
	SEQ_MAKEDISP,
	SEQ_DISPON,
	SEQ_MAIN,
	SEQ_POPVRAM,
	SEQ_END,
};

BOOL	GFL_SNDVIEWER_Main( GFL_SNDVIEWER* gflSndViewer )
{
	if( gflSndViewer == NULL ){ return FALSE; }

	switch( gflSndViewer->seq ){

	case SEQ_PUSHVRAM:
		GFL_DISPUT_VisibleOff( gflSndViewer->setup.bgID );

		GFL_DISPUT_PushVram( gflSndViewer->vramSv );

		gflSndViewer->seq = SEQ_MAKEDISP;
		break;

	case SEQ_MAKEDISP:
		loadGraphicData( gflSndViewer );
		GFL_DISPUT_SetAlpha( gflSndViewer->setup.bgID, 31, 6 );

		gflSndViewer->seq = SEQ_DISPON;
		break;

	case SEQ_DISPON:
		GFL_DISPUT_VisibleOn( gflSndViewer->setup.bgID );
		gflSndViewer->seq = SEQ_MAIN;
		break;

	case SEQ_MAIN:
		if( gflSndViewer->setup.cancelKey ){
			if( GFL_UI_KEY_GetTrg() == gflSndViewer->setup.cancelKey ){
				gflSndViewer->seq = SEQ_POPVRAM;
				break;
			}
		}
		updateSndSystemInfo( gflSndViewer );

		if( gflSndViewer->setup.controlFlag != GFL_SNDVIEWER_CONTROL_NONE ){
			if( SNDVIEWER_Control( gflSndViewer ) == FALSE ){
				gflSndViewer->seq = SEQ_POPVRAM;
			}
		}
		SNDVIEWER_SetInfo( gflSndViewer );
		GFL_DISPUT_LoadScr(gflSndViewer->setup.bgID, gflSndViewer->scrnBuf, 0, PUSH_SCR_SIZ);
		break;

	case SEQ_POPVRAM:
		GFL_DISPUT_VisibleOff( gflSndViewer->setup.bgID );

		GFL_DISPUT_PopVram( gflSndViewer->vramSv );

		gflSndViewer->seq = SEQ_END;
		break;

	case SEQ_END:
		GFL_DISPUT_VisibleOn( gflSndViewer->setup.bgID );
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	描画関数
 *
 */
//============================================================================================
static void resetScrAttr( u16* scr, GFL_DISPUT_PALID palID )
{
	int i;
	u16 palMask = palID <<12;

	for( i=0; i<32*32; i++ ){
		scr[i] &= 0x0fff;
		scr[i] |= palMask;
	}
}

//============================================================================================
static void loadGraphicData( GFL_SNDVIEWER* gflSndViewer )
{
	void* binCgx;
	void* binScr;
	void* binPlt;
	void* cgx;
	void* scr;
	void* plt;

	binCgx = GFL_ARC_LoadDataFilePathAlloc
					( arc_path, NARC_soundstatus_soundstatus_NCGR, gflSndViewer->heapID );
	binScr = GFL_ARC_LoadDataFilePathAlloc
					( arc_path, NARC_soundstatus_soundstatus_NSCR, gflSndViewer->heapID );
	binPlt = GFL_ARC_LoadDataFilePathAlloc
					( arc_path, NARC_soundstatus_soundstatus_NCLR, gflSndViewer->heapID );

	cgx = GFL_DISPUT_GetCgxDataNNSbin( binCgx );
	scr = GFL_DISPUT_GetScrDataNNSbin( binScr ); 
	plt = GFL_DISPUT_GetPltDataNNSbin( binPlt );

	GFL_STD_MemCopy32( scr, (void*)gflSndViewer->scrnBuf, PUSH_SCR_SIZ );
	resetScrAttr( gflSndViewer->scrnBuf, gflSndViewer->setup.bgPalID );

	GFL_DISPUT_LoadCgx( gflSndViewer->setup.bgID, cgx, 0, PUSH_CGX_SIZ );
	GFL_DISPUT_LoadScr( gflSndViewer->setup.bgID, gflSndViewer->scrnBuf, 0, PUSH_SCR_SIZ );
	GFL_DISPUT_LoadPlt( gflSndViewer->setup.bgID, plt, PLT_SIZ * gflSndViewer->setup.bgPalID );

	GFL_HEAP_FreeMemory( binPlt );
	GFL_HEAP_FreeMemory( binScr );
	GFL_HEAP_FreeMemory( binCgx );
}

//============================================================================================
static void writeScrnSwitch( GFL_SNDVIEWER* gflSndViewer, u16 param, u16 x, u16 y )
{
	u16*	scrnBuf = gflSndViewer->scrnBuf;
	u16		palMask = gflSndViewer->setup.bgPalID << 12;
	u16*	swScrPattern = (u16*)GFL_DISPUT_GetScrDataNNSbin( gflSndViewer->volumeSwScrnPattern );
	int		offsx, offsy;
	int		i;

	offsy = (param/16)*(32*6);
	offsx = (param%16)*2;

	for( i=0; i<6; i++ ){
		scrnBuf[ (y+i)*32 + x + 0 ] = swScrPattern[offsy + i*32 + offsx + 0] | palMask;
		scrnBuf[ (y+i)*32 + x + 1 ] = swScrPattern[offsy + i*32 + offsx + 1] | palMask;
	}
}

//--------------------------------------------------------------------------------------------
static void writeTrackStatus( GFL_SNDVIEWER* gflSndViewer, TRACK_STATUS* status, u16 x, u16 y )
{
	u16		chrNo;
	u16*	scrnBuf = gflSndViewer->scrnBuf;
	u16		palMask = gflSndViewer->setup.bgPalID << 12;

	if( status->active == TRUE ){
		if( status->stBtn == TRACK_STBTN_VOL_ON ){
			chrNo = 0x42;
		} else if( status->stBtn == TRACK_STBTN_EFFECT_ON ){
			chrNo = 0x46;
		} else {
			chrNo = 0x44;
		}
	} else {
		chrNo = 0x40;
	}
	scrnBuf[ (y+7)*32 + x + 0 ] = (chrNo + 0x00) | palMask;
	scrnBuf[ (y+7)*32 + x + 1 ] = (chrNo + 0x01) | palMask;
	scrnBuf[ (y+8)*32 + x + 0 ] = (chrNo + 0x10) | palMask;
	scrnBuf[ (y+8)*32 + x + 1 ] = (chrNo + 0x11) | palMask;
}

//--------------------------------------------------------------------------------------------
static void writeReverbInfo( GFL_SNDVIEWER* gflSndViewer )
{
	u16		chrNo;
	u16*	scrnBuf = gflSndViewer->scrnBuf;
	u16		palMask = gflSndViewer->setup.bgPalID << 12;
	int		x = 12;
	int		y = 22;
	int		i;

	if( gflSndViewer->reverb == TRUE ){ chrNo = 0x1c; }
	else { chrNo = 0xa8; }

	for( i=0; i<4; i++ ){ scrnBuf[ y*32 + x + i ] = (chrNo + i) | palMask; }
}

//============================================================================================
static void SetScrnChannelStatus( GFL_SNDVIEWER* gflSndViewer )
{
	int		i, j;
	int		x = 0;
	int		y = 1;
	u16		volume;
	u16		gauge;
	u16*	scrnBuf = gflSndViewer->scrnBuf;
	u16		palMask = gflSndViewer->setup.bgPalID << 12;
	u16		volumeMeterPattern[4] = {0,0,0,0};

	for( i=0; i<CHANNEL_NUM; i++ ){
		volume = gflSndViewer->channelInfo[i].volume;
		gauge = volume * 32 / 2032;
		if( gauge > 32 ){ gauge = 32; }

		{
			int d = gauge / 8;
			int f = gauge % 8;
			for( j=3; j>-1; j-- ){
				if(d){
					volumeMeterPattern[j] = 8;
					d--;
				} else {
					if(f){
						volumeMeterPattern[j] = f;
						f = 0;
					} else {
						volumeMeterPattern[j] = 0;
					}
				}
			}
		}

		for( j=0; j<4; j++ ){
			scrnBuf[ (y+j)*32 + (x+i)*2 + 0 ] = (volumeMeterPattern[j] + 0x60) | palMask;
			scrnBuf[ (y+j)*32 + (x+i)*2 + 1 ] = (volumeMeterPattern[j] + 0x70) | palMask;
		}
	}
}

//--------------------------------------------------------------------------------------------
static void SetScrnTrackStatus( GFL_SNDVIEWER* gflSndViewer )
{
	TRACK_STATUS* status;
	int	x;
	int	y = 7;
	int i;

	for( i=0; i<TRACK_NUM; i++ ){
		status = &gflSndViewer->bgmTrackStatus[i];
		x = i*2;
		writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[status->volume].valOffs, x, y);
		writeTrackStatus( gflSndViewer, status, x, y );
	}
}

//--------------------------------------------------------------------------------------------
static void SetScrnMasterTrackStatus( GFL_SNDVIEWER* gflSndViewer )
{
	MASTERTRACK_STATUS* mst = &gflSndViewer->bgmMasterTrackStatus;
	TRACKEFFECT_STATUS* tst = &gflSndViewer->bgmTrackEffectStatus;
	u16*	scrnBuf = gflSndViewer->scrnBuf;

	writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[mst->volume].valOffs, 1, 16);
	writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[mst->tempo].valOffs, 5, 16);
	writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[mst->pitch].valOffs, 9, 16);
	writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[mst->reverb].valOffs, 13, 16);

	writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[tst->mod_d].valOffs, 21, 16);
	writeScrnSwitch( gflSndViewer, gflSndViewer->switchStatus[tst->mod_s].valOffs, 25, 16);

	writeReverbInfo( gflSndViewer );
}

//--------------------------------------------------------------------------------------------
static void SetScrnPlayerStatus( GFL_SNDVIEWER* gflSndViewer )
{
	u16*	scrnBuf = gflSndViewer->scrnBuf;
	u16		palMask = gflSndViewer->setup.bgPalID << 12;
	u16		chrNo, chrNo2, trackNum;
	int		i, x, y;

	x = 17;
	y = 21;

	for( i=0; i<5; i++ ){
		if( i == gflSndViewer->playerNoIdx ){ chrNo = 0x79; }
		else if( i < gflSndViewer->playerNoIdx ){ chrNo = 0x7a; }
		else { chrNo = 0x7b; }

		trackNum = gflSndViewer->trackNumStack[i];
		if(trackNum){ chrNo2 = trackNum -1 + 0xd0; } 
		else { chrNo2 = 0x4f; }

		scrnBuf[ (y-i)*32 + x + 0 ] = chrNo | palMask;
		scrnBuf[ (y-i)*32 + x + 1 ] = chrNo | palMask | 0x0400;	// 反転
		scrnBuf[ (y-i)*32 + x + 2 ] = chrNo2 | palMask;
	}
}

//============================================================================================
/**
 *
 * @brief	情報の取得と表示
 *
 */
//============================================================================================
static BOOL SNDVIEWER_SetInfo( GFL_SNDVIEWER* gflSndViewer )
{
	BOOL f;
	int i;

	// サウンドドライバ情報更新
	NNS_SndUpdateDriverInfo();	//pm_sndsys内に常駐された場合は削除すること（1_call/1_frame）

	// 更新されたチャンネル情報取得
	for( i=0; i<CHANNEL_NUM; i++ ){
		NNS_SndReadDriverChannelInfo( i, &gflSndViewer->channelInfo[i]); 
	}
	// 現在の階層以上のトラック数情報リセット
	for( i=0; i<8; i++ ){ 
		if(i >= gflSndViewer->playerNoIdx){
			gflSndViewer->trackNumStack[i] = 0;
		}
	}
	// 更新されたトラック情報取得
	if( gflSndViewer->sndHandle != NULL ){
		for( i=0; i<TRACK_NUM; i++ ){
			f = NNS_SndPlayerReadDriverTrackInfo
				(gflSndViewer->sndHandle, i, &gflSndViewer->bgmTrackInfo[i]);
			gflSndViewer->bgmTrackStatus[i].active = f;
			if(f == TRUE){ 
				gflSndViewer->trackNumStack[gflSndViewer->playerNoIdx]++; 
			}
		}
	} else {
		for( i=0; i<TRACK_NUM; i++ ){
			gflSndViewer->bgmTrackStatus[i].active = FALSE;
		}
	}
	SetScrnChannelStatus( gflSndViewer );
	SetScrnTrackStatus( gflSndViewer );
	SetScrnMasterTrackStatus( gflSndViewer );
	SetScrnPlayerStatus( gflSndViewer );

	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	タッチパネル判定テーブル
 *
 */
//============================================================================================
#define SWITCH_SX	(16-1)
#define BUTTON_SX	(16-1)
#define BUTTON_SY	(12)

#define TRACK_BUTTON_PY	(112)
#define TRACK_SWITCH_PY	(56)
#define TRACK1_PX		(0)
#define TRACK2_PX		(16)
#define TRACK3_PX		(32)
#define TRACK4_PX		(48)
#define TRACK5_PX		(64)
#define TRACK6_PX		(80)
#define TRACK7_PX		(96)
#define TRACK8_PX		(112)
#define TRACK9_PX		(128)
#define TRACK10_PX		(144)
#define TRACK11_PX		(160)
#define TRACK12_PX		(176)
#define TRACK13_PX		(192)
#define TRACK14_PX		(208)
#define TRACK15_PX		(224)
#define TRACK16_PX		(240)

#define MTRACK_SWITCH_PY	(128)
#define MTRACK_VOLUME_PX	(8)
#define MTRACK_TEMPO_PX		(40)
#define MTRACK_PITCH_PX		(72)
#define MTRACK_REVERB_PX	(104)

#define TRACKEF_MODD_PX		(168)
#define TRACKEF_MODS_PX		(200)

#define EXIT_PX			(232)
#define EXIT_PY			(0)
//------------------------------------------------------------------
enum {
	TOUCH_BUTTON_TR1 = 0,
	TOUCH_BUTTON_TR2,
	TOUCH_BUTTON_TR3,
	TOUCH_BUTTON_TR4,
	TOUCH_BUTTON_TR5,
	TOUCH_BUTTON_TR6,
	TOUCH_BUTTON_TR7,
	TOUCH_BUTTON_TR8,
	TOUCH_BUTTON_TR9,
	TOUCH_BUTTON_TR10,
	TOUCH_BUTTON_TR11,
	TOUCH_BUTTON_TR12,
	TOUCH_BUTTON_TR13,
	TOUCH_BUTTON_TR14,
	TOUCH_BUTTON_TR15,
	TOUCH_BUTTON_TR16,

	TOUCH_EXIT,
};

static const GFL_UI_TP_HITTBL eventTouchPanelTable[] = {
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK1_PX, TRACK1_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK2_PX, TRACK2_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK3_PX, TRACK3_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK4_PX, TRACK4_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK5_PX, TRACK5_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK6_PX, TRACK6_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK7_PX, TRACK7_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK8_PX, TRACK8_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK9_PX, TRACK9_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK10_PX, TRACK10_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK11_PX, TRACK11_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK12_PX, TRACK12_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK13_PX, TRACK13_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK14_PX, TRACK14_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK15_PX, TRACK15_PX + BUTTON_SX },
{ TRACK_BUTTON_PY, TRACK_BUTTON_PY + BUTTON_SY, TRACK16_PX, TRACK16_PX + BUTTON_SX },

{ EXIT_PY, EXIT_PY+7, EXIT_PX, EXIT_PX+23 },

{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

//------------------------------------------------------------------
static void setEventSwitchTable( GFL_SNDVIEWER* gflSndViewer, SWITCH_ID swID, u8 x, u8 y )
{
	GFL_UI_TP_HITTBL* tbl = &gflSndViewer->eventSwitchTable[swID];
	u8	cy;

	cy = (y + 24) - (gflSndViewer->switchStatus[swID].valOffs - SWITCH_VAL_ZERO);// 中心算出;

	// switch y判定設定(幅8)
	tbl->rect.top = cy - 4;
	tbl->rect.bottom = cy + 3;
	// switch x判定設定
	tbl->rect.left = x;
	tbl->rect.right = x + SWITCH_SX;
}

//------------------------------------------------------------------
static void makeEventSwitchTable( GFL_SNDVIEWER* gflSndViewer )
{
	u8	offs, cy;
	int i;

	for( i=0; i<SWITCH_MAX+1; i++ ){
		gflSndViewer->eventSwitchTable[i].rect.top = 0;
		gflSndViewer->eventSwitchTable[i].rect.bottom = 0;
		gflSndViewer->eventSwitchTable[i].rect.left = 0;
		gflSndViewer->eventSwitchTable[i].rect.right = 0;
	}
	for( i=0; i<SWITCH_BGM_VOL; i++ ){
		setEventSwitchTable(gflSndViewer, SWITCH_TR1 + i, TRACK1_PX + 16*i, TRACK_SWITCH_PY);
	}
	setEventSwitchTable(gflSndViewer, SWITCH_BGM_VOL, MTRACK_VOLUME_PX, MTRACK_SWITCH_PY);
	setEventSwitchTable(gflSndViewer, SWITCH_BGM_TEMPO, MTRACK_TEMPO_PX, MTRACK_SWITCH_PY);
	setEventSwitchTable(gflSndViewer, SWITCH_BGM_PITCH, MTRACK_PITCH_PX, MTRACK_SWITCH_PY);
	setEventSwitchTable(gflSndViewer, SWITCH_BGM_REVERB, MTRACK_REVERB_PX, MTRACK_SWITCH_PY);

	setEventSwitchTable(gflSndViewer, SWITCH_TREF_MODD, TRACKEF_MODD_PX, MTRACK_SWITCH_PY);
	setEventSwitchTable(gflSndViewer, SWITCH_TREF_MODS, TRACKEF_MODS_PX, MTRACK_SWITCH_PY);

	gflSndViewer->eventSwitchTable[SWITCH_MAX].rect.top = GFL_UI_TP_HIT_END;//終了データ埋め込み
	gflSndViewer->eventSwitchTable[SWITCH_MAX].rect.bottom = 0;
	gflSndViewer->eventSwitchTable[SWITCH_MAX].rect.left = 0;
	gflSndViewer->eventSwitchTable[SWITCH_MAX].rect.right = 0;
}

//============================================================================================
/**
 *
 * @brief	タッチパネルイベント判定
 *
 */
//============================================================================================
static BOOL checkTouchPanelEvent( GFL_SNDVIEWER* gflSndViewer )
{
	int tblPos;
	u32 tpx, tpy;
	BOOL modFlag = FALSE;

	if( GFL_UI_TP_GetPointTrg( &tpx, &tpy ) == FALSE ){
		return TRUE;
	} 
	tblPos = GFL_UI_TP_HitTrg(eventTouchPanelTable);
	if( tblPos != GFL_UI_TP_HIT_NONE ){
		if( tblPos == TOUCH_EXIT ){
			if( gflSndViewer->setup.controlFlag & GFL_SNDVIEWER_CONTROL_EXIT ){
				return FALSE;	// 終了
			}
		}
		if((tblPos >= TOUCH_BUTTON_TR1)&&(tblPos <= TOUCH_BUTTON_TR16)){
			SNDVIEWER_SetTrackStatus( gflSndViewer, tblPos - TOUCH_BUTTON_TR1 );
		}
		return TRUE;
	}
	if( gflSndViewer->sndHandle == NULL ){ return TRUE; }

	tblPos = GFL_UI_TP_HitTrg(gflSndViewer->eventSwitchTable);
	if( tblPos == SWITCH_BGM_REVERB ){
		//reverbスイッチはreverb有効になっていなければ無視
		if( gflSndViewer->reverb == FALSE ){ return TRUE; }
	}
	if( tblPos != GFL_UI_TP_HIT_NONE ){
		gflSndViewer->swControl.seq = SWITCH_SLIDE_MODE;
		gflSndViewer->swControl.swID = tblPos;
		gflSndViewer->swControl.tpy = tpy;
		gflSndViewer->swControl.rectL = gflSndViewer->eventSwitchTable[tblPos].rect.left;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	タッチパネルスライド判定
 *
 */
//============================================================================================
static BOOL checkTouchPanelSlide( GFL_SNDVIEWER* gflSndViewer )
{
	u32 tpx, tpy;
	u8	wxL, wxR;
	s8	valOffs, diff;

	if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){ return FALSE; } 

	wxL = gflSndViewer->swControl.rectL;
	wxR = wxL + SWITCH_SX;
	if( (tpy < wxL)&&(tpy > wxR) ){ return FALSE; }	// x座標範囲外

	diff = -(tpy - gflSndViewer->swControl.tpy);
	if( diff != 0 ){
		gflSndViewer->swControl.tpy = tpy;

		valOffs = gflSndViewer->switchStatus[gflSndViewer->swControl.swID].valOffs;
		valOffs += diff;
		if(valOffs < -SWITCH_VAL_MIN){
			valOffs = -SWITCH_VAL_MIN;
		} else if( valOffs > SWITCH_VAL_MAX ){
			valOffs = SWITCH_VAL_MAX;
		}
		gflSndViewer->switchStatus[gflSndViewer->swControl.swID].valOffs = valOffs;

		SNDVIEWER_SetSwitchParam( gflSndViewer, gflSndViewer->swControl.swID );
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	メインコントロール	
 *
 */
//============================================================================================
static BOOL SNDVIEWER_Control( GFL_SNDVIEWER* gflSndViewer )
{
	switch( gflSndViewer->swControl.seq ){
	
		case EVENT_CHECK_MODE:
			return checkTouchPanelEvent( gflSndViewer );

		case SWITCH_SLIDE_MODE:
			if( checkTouchPanelSlide( gflSndViewer ) == FALSE ){
				initSwitchControl( gflSndViewer );
			}
			break;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	ドライバ設定
 *
 */
//============================================================================================
static const int tempoRatioTable[32+1] = {
	64, 72, 80, 88, 96, 104, 112, 120, 128, 144, 160, 176, 192, 208, 224, 240,
	256,
	288, 320, 352, 384, 416, 448, 480, 512, 576, 640, 704, 768, 832, 896, 960, 1024,
};
static const int modSpeedTable[32+1] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16,
	16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,
};

//------------------------------------------------------------------
static void SNDVIEWER_SetMod( GFL_SNDVIEWER* gflSndViewer, BOOL enable )
{
	TRACK_STATUS* status;
	u16 bitMask = 0;
	int	depth, speed;
	int i;

	if( enable == TRUE ){
		for( i=0; i<TRACK_NUM; i++ ){
			TRACK_STATUS* status = &gflSndViewer->bgmTrackStatus[i];
			if((status->active == TRUE)&&(status->stBtn == TRACK_STBTN_EFFECT_ON)){
				bitMask |= (0x0001 << i);
			}
		}
		depth = gflSndViewer->switchStatus[SWITCH_TREF_MODD].valOffs * 8;
		if(depth > 255){ depth = 255; }
		speed = modSpeedTable[gflSndViewer->switchStatus[SWITCH_TREF_MODS].valOffs];
	} else {
		bitMask = 0xffff;
		depth = 0;
		speed = 16;
	}
	NNS_SndPlayerSetTrackModDepth( gflSndViewer->sndHandle, bitMask, depth );
	NNS_SndPlayerSetTrackModSpeed( gflSndViewer->sndHandle, bitMask, speed );
}

//------------------------------------------------------------------
static void SNDVIEWER_SetTrackStatus( GFL_SNDVIEWER* gflSndViewer, int trackNo )
{
	u16	bitMask = 0x0001 << trackNo;
	TRACK_STATUS* status = &gflSndViewer->bgmTrackStatus[trackNo];
	BOOL modFlag = FALSE;

	if( gflSndViewer->sndHandle == NULL ){ return; }

	if( status->active == TRUE ){
		status->stBtn++;
		if(status->stBtn >= TRACK_STBTN_NUM){ status->stBtn = TRACK_STBTN_NO_VOL; }

		if( status->stBtn == TRACK_STBTN_VOL_ON ){
			NNS_SndPlayerSetTrackMute(gflSndViewer->sndHandle, bitMask, FALSE);
		} else if( status->stBtn == TRACK_STBTN_EFFECT_ON ){
			modFlag = TRUE;
			NNS_SndPlayerSetTrackMute(gflSndViewer->sndHandle, bitMask, FALSE);
		} else {
			NNS_SndPlayerSetTrackMute(gflSndViewer->sndHandle, bitMask, TRUE);
		}
		if( modFlag == TRUE ){
			SNDVIEWER_SetMod( gflSndViewer, TRUE );
			gflSndViewer->bgmTrackEffectSw = TRUE;
		} else {
			if( gflSndViewer->bgmTrackEffectSw == TRUE ){
				SNDVIEWER_SetMod( gflSndViewer, FALSE );
				gflSndViewer->bgmTrackEffectSw = FALSE;
			}
		}
	}
}

//------------------------------------------------------------------
static void SNDVIEWER_SetSwitchParam( GFL_SNDVIEWER* gflSndViewer, SWITCH_ID swID )
{
	int	value;
	if( gflSndViewer->sndHandle == NULL ){ return; }

	switch( swID ){

	case SWITCH_TR1:
	case SWITCH_TR2:
	case SWITCH_TR3:
	case SWITCH_TR4:
	case SWITCH_TR5:
	case SWITCH_TR6:
	case SWITCH_TR7:
	case SWITCH_TR8:
	case SWITCH_TR9:
	case SWITCH_TR10:
	case SWITCH_TR11:
	case SWITCH_TR12:
	case SWITCH_TR13:
	case SWITCH_TR14:
	case SWITCH_TR15:
	case SWITCH_TR16:
		{
			// volume幅 0～127
			u32	bitMask;
			value = gflSndViewer->switchStatus[gflSndViewer->swControl.swID].valOffs * 4;
			if(value > 127){ value = 127; }

			bitMask = 0x00000001 << swID;
			NNS_SndPlayerSetTrackVolume( gflSndViewer->sndHandle, bitMask, value );
		}
		break;

	case SWITCH_BGM_VOL:
		// volume幅 0～127
		value = gflSndViewer->switchStatus[SWITCH_BGM_VOL].valOffs * 4;
		if(value > 127){ value = 127; }
		NNS_SndPlayerSetVolume( gflSndViewer->sndHandle, value );
		break;
	case SWITCH_BGM_TEMPO:
		value = tempoRatioTable[gflSndViewer->switchStatus[SWITCH_BGM_TEMPO].valOffs];
		NNS_SndPlayerSetTempoRatio( gflSndViewer->sndHandle, value );
		break;
	case SWITCH_BGM_PITCH:
		// pitch幅 -32768～32767
		value = (gflSndViewer->switchStatus[SWITCH_BGM_PITCH].valOffs - SWITCH_VAL_ZERO) * 64;
		NNS_SndPlayerSetTrackPitch( gflSndViewer->sndHandle, 0xffff, value );
		break;
	case SWITCH_BGM_REVERB:
		// reverb幅 0～0x2000
		if( gflSndViewer->reverb == TRUE ){
			// ＲＥＶＥＲＢ操作あり
			value = gflSndViewer->switchStatus[SWITCH_BGM_REVERB].valOffs * 2;
			if(value > 63){ value = 63; }
			NNS_SndCaptureSetReverbVolume(value, 0);
			//PMSND_ChangeCaptureReverb(value, PMSND_NOEFFECT, PMSND_NOEFFECT, PMSND_NOEFFECT);
		}
		break;

	case SWITCH_TREF_MODD:
	case SWITCH_TREF_MODS:
		SNDVIEWER_SetMod( gflSndViewer, TRUE );
		break;
	}
}







