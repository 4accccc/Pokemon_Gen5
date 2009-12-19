//======================================================================
/**
 * @file	ctvt_talk.c
 * @brief	通信TVTシステム：会話時下画面
 * @author	ariizumi
 * @data	09/12/18
 *
 * モジュール名：CTVT_TALK
 */

//======================================================================
#pragma once

#include "comm_tvt_local_def.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define

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
extern CTVT_TALK_WORK* CTVT_TALK_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId );
extern void CTVT_TALK_TermSystem( COMM_TVT_WORK *work , CTVT_TALK_WORK *commWork );
extern const COMM_TVT_MODE CTVT_TALK_Main( COMM_TVT_WORK *work , CTVT_TALK_WORK *commWork );
extern void CTVT_TALK_InitMode( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );
extern void CTVT_TALK_TermMode( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------