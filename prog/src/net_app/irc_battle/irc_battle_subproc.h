//======================================================================
/**
 * @file	irc_battle_subproc.h
 * @brief	赤外線バトル リスト・ステータス呼び出し
 * @author	ariizumi
 * @data	09/01/19
 *
 * モジュール名：IRC_BATTLE_SUBPROC
 */
//======================================================================
#pragma once

#include "irc_battle_local_def.h"

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

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern IRC_BATTLE_SUBPROC_WORK* IRC_BATTLE_SUBPROC_InitSubProc( IRC_BATTLE_WORK *work );
extern void IRC_BATTLE_SUBPROC_TermSubProc( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork );
extern const BOOL IRC_BATTLE_SUBPROC_UpdateSubProc( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork );

extern void IRC_BATTLE_SUBPROC_PostFinishPokelist( IRC_BATTLE_SUBPROC_WORK *procWork );
