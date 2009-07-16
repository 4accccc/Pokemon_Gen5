//======================================================================
/**
 * @file	p_sta_info.c
 * @brief	ポケモンステータス 情報ページ
 * @author	ariizumi
 * @data	09/07/06
 *
 * モジュール名：PSTATUS_INFO
 */
//======================================================================
#pragma once

#include "p_sta_local_def.h"

extern PSTATUS_INFO_WORK* PSTATUS_INFO_Init( PSTATUS_WORK *work );
extern void PSTATUS_INFO_Term( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );
extern void PSTATUS_INFO_Main( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );
extern void PSTATUS_INFO_Draw( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );

//リソース初期化
void PSTATUS_INFO_LoadResource( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , ARCHANDLE *archandle );
void PSTATUS_INFO_ReleaseResource( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );

//表示非表示切り替え
void PSTATUS_INFO_DispPage( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );
void PSTATUS_INFO_ClearPage( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );
void PSTATUS_INFO_DispPage_Trans( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );
void PSTATUS_INFO_ClearPage_Trans( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork );

