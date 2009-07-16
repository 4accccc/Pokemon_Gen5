//======================================================================
/**
 * @file	p_sta_skill.c
 * @brief	ポケモンステータス 技ページ
 * @author	ariizumi
 * @data	09/07/10
 *
 * モジュール名：PSTATUS_SKILL
 */
//======================================================================
#pragma once

#include "p_sta_local_def.h"

extern PSTATUS_SKILL_WORK* PSTATUS_SKILL_Init( PSTATUS_WORK *work );
extern void PSTATUS_SKILL_Term( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
extern void PSTATUS_SKILL_Main( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
extern void PSTATUS_SKILL_Draw( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );

//リソース初期化
void PSTATUS_SKILL_LoadResource( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork , ARCHANDLE *archandle );
void PSTATUS_SKILL_ReleaseResource( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );

//セル初期化
void PSTATUS_SKILL_InitCell( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
void PSTATUS_SKILL_TermCell( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );

//表示非表示切り替え
void PSTATUS_SKILL_DispPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
void PSTATUS_SKILL_ClearPage( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
void PSTATUS_SKILL_DispPage_Trans( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
void PSTATUS_SKILL_ClearPage_Trans( PSTATUS_WORK *work , PSTATUS_SKILL_WORK *skillWork );
