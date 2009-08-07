//============================================================================================
/**
 * @file	snd_viewer_mcs.c
 * @brief	サウンドステータスビューワー（ＭＣＳ：外部ＰＣコントロール版）
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "arc_def.h"

#include "sound/pm_sndsys.h"
#include "debug/gf_mcs.h"

#include "sound/snd_viewer_mcs.h"

#define DS_MCS_SRC
#include "sound/snd_viewer_mcs_comm.h"

FS_EXTERN_OVERLAY(mcs_lib);

static BOOL mcsExist = FALSE;
static HEAPID heapID;

static u32 MCS_Sound(u32 comm, u32* param);
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
void	GFL_MCS_SNDVIEWER_Init(HEAPID heapID)
{
	if(mcsExist == FALSE){
		GFL_OVERLAY_Load(FS_OVERLAY_ID(mcs_lib));
		mcsExist = TRUE;
	}
	if(MCS_CheckEnable() == FALSE){
		MCS_Init(heapID);
	}
}

void	GFL_MCS_SNDVIEWER_Exit(void)
{
	if(mcsExist == TRUE){
		if(MCS_CheckEnable() == TRUE){
			MCS_Exit();
		}
		GFL_OVERLAY_Unload(FS_OVERLAY_ID(mcs_lib));
		mcsExist = FALSE;
	}
}

u32	GFL_MCS_SNDVIEWER_Main(HEAPID heapID)
{
	u32 dataSize = 0;
	u32 result = 0;

	if(mcsExist == FALSE){ return 0; }
	if(MCS_CheckEnable() == FALSE){ return 0; }

	MCS_Main();
	// データ受信チェック
	dataSize = MCS_CheckRead();

	if(dataSize != 0){
		GFL_MCS_SNDVIEWER_HEADER* dataHeader;

		//受信処理
		dataHeader = GFL_HEAP_AllocMemory(GetHeapLowID(heapID), dataSize);
		MCS_Read(dataHeader, dataSize);

		//受信コマンド処理
		switch(dataHeader->ID){
		case GFL_MCS_SNDVIEWER_ID:
			result = MCS_Sound(dataHeader->command, &dataHeader->param);
			break;
		}
		GFL_HEAP_FreeMemory(dataHeader);
	}
	return result;
}

//============================================================================================
/**
 *
 * @brief	サウンド処理
 *
 */
//============================================================================================
void MCS_Sound_Send(u32 comm, u32* param, HEAPID heapID)
{
	if(mcsExist == FALSE){ return; }
	if(MCS_CheckEnable() == FALSE){ return; }
	{
		GFL_MCS_SNDVIEWER_HEADER* dataHeader;
		u32 paramNum;
		u32	size;
		int i;

		switch(comm){
		default:
			return;

		case COMM_PANEL_RESET:
			paramNum = PNUM_COMM_PANEL_RESET;
			break;
		case COMM_SET_TRACKST:
			paramNum = PNUM_COMM_SET_TRACKST;
			break;
		}

		size = sizeof(GFL_MCS_SNDVIEWER_HEADER) + sizeof(DWORD)*(paramNum-1);
		dataHeader = GFL_HEAP_AllocMemory(GetHeapLowID(heapID), size);

		dataHeader->ID = GFL_MCS_SNDVIEWER_ID;
		dataHeader->command = comm;
		for(i=0; i<paramNum; i++){ (&dataHeader->param)[i] = param[i]; }

		MCS_Write(0, dataHeader, size);

		GFL_HEAP_FreeMemory(dataHeader);
	}
}

static u32 MCS_Sound(u32 comm, u32* param)
{
	int val;
	u16 trackBit;

	switch(comm){
	default:
		break;
	case COMM_PLAYERVOL_CHANGE:
		val = param[0] & 0x7f;
		NNS_SndPlayerSetVolume(PMSND_GetBGMhandlePointer(), val);
		break;

	case COMM_PLAYERPITCH_CHANGE:
		val = param[0] & 0xffff;
		NNS_SndPlayerSetTrackPitch(PMSND_GetBGMhandlePointer(), 0xffff, val);
		break;

	case COMM_PLAYERTEMPO_CHANGE:
		val = param[0] & 0xffff;
		NNS_SndPlayerSetTempoRatio(PMSND_GetBGMhandlePointer(), val);
		break;

	case COMM_PLAYERPAN_CHANGE:
		val = param[0] & 0xff;
		NNS_SndPlayerSetTrackPan(PMSND_GetBGMhandlePointer(), 0xffff, val);
		break;

	case COMM_PLAYERREVERB_CHANGE:
		val = param[0] & 0x3f;
		NNS_SndCaptureSetReverbVolume(val, 0);
		break;

	case COMM_TRACKVOL_CHANGE:
		val = param[0] & 0x7f;
		trackBit = 0x0001 << param[1];
		NNS_SndPlayerSetTrackVolume(PMSND_GetBGMhandlePointer(), trackBit, val);
		break;

	case COMM_TRACKPITCH_CHANGE:
		val = param[0] & 0xffff;
		trackBit = 0x0001 << param[1];
		NNS_SndPlayerSetTrackPitch(PMSND_GetBGMhandlePointer(), trackBit, val);
		break;

	case COMM_TRACKMODD_CHANGE:
		val = param[0] & 0xff;
		trackBit = 0x0001 << param[1];
		NNS_SndPlayerSetTrackModDepth(PMSND_GetBGMhandlePointer(), trackBit, val);
		break;

	case COMM_TRACKMODS_CHANGE:
		val = param[0] & 0xff;
		trackBit = 0x0001 << param[1];
		NNS_SndPlayerSetTrackModSpeed(PMSND_GetBGMhandlePointer(), trackBit, val);
		break;

	case COMM_TRACKPAN_CHANGE:
		val = param[0] & 0xff;
		trackBit = 0x0001 << param[1];
		NNS_SndPlayerSetTrackPan(PMSND_GetBGMhandlePointer(), trackBit, val);
		break;

	case COMM_CONNECT:
		return 1;	// 特殊外部処理
	}
	return 0;
}

