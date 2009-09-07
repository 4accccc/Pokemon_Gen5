//============================================================================================
/**
 * @file	snd_viewer_mcs.c
 * @brief	サウンドステータスビューワー（ＭＣＳ：外部ＰＣコントロール版）
 * @author	
 * @date	
 */
//============================================================================================
#ifdef PM_DEBUG

#include "gflib.h"
#include "arc_def.h"

#include "sound/pm_sndsys.h"
#include "debug/gf_mcs.h"

#include "sound/snd_viewer_mcs.h"

#define DS_MCS_SRC
#include "sound/snd_viewer_mcs_comm.h"

static void MCS_Sound_Send(u32 comm, u32* param);
static void MCS_Sound_Recv(u32 comm, u32* param);

#define TRACK_NUM	(16)
static GFL_MCS_LINKIDX mcsLinkIdx = GFL_MCS_LINKIDX_INVALID; 
static SNDTrackInfo		trackInfo[TRACK_NUM];
static u16						trackActiveBit = 0;	
static u8							writeBuf[0x80];
static u8							readBuf[0x80];
static u32						soundNo = 0xffffffff;
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
void	GFL_MCS_SNDVIEWER_Main(void)
{
	// リンク判定
	if(mcsLinkIdx == GFL_MCS_LINKIDX_INVALID){
		// リンクされるまでここでHold
		GFL_MCS_Link(GFL_MCS_SNDVIEWER_ID, &mcsLinkIdx);
		return;
	}
	{
		NNSSndHandle*	sndHandle = PMSND_GetNowSndHandlePointer();
		u32 nowSoundNo = PMSND_GetBGMsoundNo();
		u16	nowTrackBit = 0;
		int i;

		// 現在のトラック状態を取得
		for( i=0; i<TRACK_NUM; i++ ){
			if(NNS_SndPlayerReadDriverTrackInfo(sndHandle, i, &trackInfo[i]) == TRUE){
				nowTrackBit |= (0x0001 << i);	// 有効/無効をBitFieldに変換
			}
		}
		// 送信判定
		if(nowSoundNo != soundNo){
			// 曲が変化した場合リセットコマンド転送
			u32 param[PNUM_COMM_PANEL_RESET] = {0};
			MCS_Sound_Send(COMM_PANEL_RESET, param);
			soundNo = nowSoundNo;
		}
		// 現在のトラック状態を取得
		if(nowTrackBit != trackActiveBit){
			// 差があった場合、現在のトラック状態を送信
			u32 param[PNUM_COMM_SET_TRACKST];
			param[0] = nowTrackBit;
			MCS_Sound_Send(COMM_SET_TRACKST, param);
			trackActiveBit = nowTrackBit;
		}
		// 受信判定
		if(GFL_MCS_Read(mcsLinkIdx, readBuf, 0x80) == TRUE){
			GFL_MCS_SNDVIEWER_HEADER* dataHeader = (GFL_MCS_SNDVIEWER_HEADER*)readBuf;
			OS_Printf("ID(%x), comm(%x), param(%x)\n",
									dataHeader->ID, dataHeader->command, dataHeader->param);
			//受信コマンド処理
			switch(dataHeader->ID){
			case GFL_MCS_SNDVIEWER_ID:
				MCS_Sound_Recv(dataHeader->command, &dataHeader->param);
				break;
			}
			GFL_STD_MemClear(readBuf, 0x80);
		}
	}
}

//============================================================================================
/**
 *
 * @brief	サウンド処理
 *
 */
//============================================================================================
static void MCS_Sound_Send(u32 comm, u32* param)
{
	GFL_MCS_SNDVIEWER_HEADER* dataHeader = (GFL_MCS_SNDVIEWER_HEADER*)writeBuf;
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

	dataHeader->ID = GFL_MCS_SNDVIEWER_ID;
	dataHeader->command = comm;
	for(i=0; i<paramNum; i++){ (&dataHeader->param)[i] = param[i]; }

	GFL_MCS_Write(mcsLinkIdx, writeBuf, size);
}

static void MCS_Sound_Recv(u32 comm, u32* param)
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
		break;

	case COMM_DISCONNECT:
		if(mcsLinkIdx != GFL_MCS_LINKIDX_INVALID){
			// 切断されたのでリンク情報リセット 
			GFL_MCS_Annul( mcsLinkIdx );
			mcsLinkIdx = GFL_MCS_LINKIDX_INVALID;
		}
		break;
	}
}

#endif
