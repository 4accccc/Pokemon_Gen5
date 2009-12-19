//======================================================================
/**
 * @file	ctvt_mic.c
 * @brief	通信TVTシステム：マイク関係
 * @author	ariizumi
 * @data	09/12/19
 *
 * モジュール名：CTVT_MIC
 */
//======================================================================
#pragma once

#include "comm_tvt_local_def.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define
//送信データのサイズの型がu16だから要注意
#define CTVT_SEND_WAVE_SIZE (0xc000)
#define CTVT_SEND_WAVE_SIZE_REAL (CTVT_SEND_WAVE_SIZE/4)
#define CTVT_SEND_WAVE_SIZE_ONE  (0x2000)
#define CTVT_SEND_WAVE_SIZE_ONE_REAL  (CTVT_SEND_WAVE_SIZE_ONE/4)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern CTVT_MIC_WORK* CTVT_MIC_Init( const HEAPID heapId );
extern void CTVT_MIC_Term( CTVT_MIC_WORK *micWork );
extern void CTVT_MIC_Main( CTVT_MIC_WORK *micWork );
extern const BOOL CTVT_MIC_StartRecord( CTVT_MIC_WORK *micWork );
extern const BOOL CTVT_MIC_StopRecord( CTVT_MIC_WORK *micWork );
extern const BOOL CTVT_MIC_IsRecording( CTVT_MIC_WORK *micWork );
extern const u32 CTVT_MIC_GetRecSize( CTVT_MIC_WORK *micWork );
extern void* CTVT_MIC_GetRecBuffer( CTVT_MIC_WORK *micWork );
extern const u32 CTVT_MIC_EncodeData( CTVT_MIC_WORK *micWork , void* decData , void *encData , const u32 dataSize );
extern const u32 CTVT_MIC_DecodeData( CTVT_MIC_WORK *micWork , void* encData , void *decData , const u32 dataSize );
extern const BOOL CTVT_MIC_PlayWave( CTVT_MIC_WORK *micWork , void *buffer , u32 size , u8 volume , int speed );
extern void CTVT_MIC_StopWave( CTVT_MIC_WORK *micWork );
extern const BOOL CTVT_MIC_IsPlayWave( CTVT_MIC_WORK *micWork );
extern const BOOL CTVT_MIC_IsFinishWave( CTVT_MIC_WORK *micWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------