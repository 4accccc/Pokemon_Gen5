//============================================================================
/**
 *
 *	@file		snd_mic.h
 *	@brief  マイク関連モジュール
 *	@author	hosaka genya
 *	@data		2009.09.10
 *
 */
//============================================================================
#pragma once

#include "savedata/perapvoice.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *								プロトタイプ宣言
 */
//=============================================================================
extern void SND_MIC_Init( HEAPID heap_id );
extern void SND_MIC_Exit( void );
extern void SND_MIC_Main( void );
extern void SND_MIC_ClearAmpOnWait( void );
extern BOOL SND_MIC_IsAmpOnWaitFlag( void );
extern MICResult SND_MIC_StartAutoSampling( MICAutoParam* p );
extern MICResult SND_MIC_StopAutoSampling(void);
extern void SND_MIC_StopSleep(void);
extern void SND_MIC_ReStartSleep(void);
extern MICResult SND_MIC_ManualSampling(MICSamplingType type ,void* heap,MICCallback callback,void* arg);
extern MICResult SND_PERAP_VoiceRecStart( void * cbFunc, void * cbWork );
extern MICResult SND_PERAP_VoiceRecStop( void );
extern void SND_PERAP_VoiceDataSave( PERAPVOICE* perap );


