//======================================================================
/**
 * @file	p_sta_sub.c
 * @brief	ポケモンステータス 右側情報画面
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PSTATUS_SUB
 */
//======================================================================
#pragma once

#include "p_sta_local_def.h"

#define PSTATUS_SUB_SHADOW_SCALE_X (FX32_ONE)
#define PSTATUS_SUB_SHADOW_SCALE_Y (FX32_CONST(2.2f))
#define PSTATUS_SUB_SHADOW_SCALE_Z (FX32_ONE)
#define PSTATUS_SUB_SHADOW_OFFSET_X (FX32_CONST(0.3f))
#define PSTATUS_SUB_SHADOW_OFFSET_Y (0)
#define PSTATUS_SUB_SHADOW_OFFSET_Z (-FX32_ONE)
#define PSTATUS_SUB_SHADOW_ROTATE (0xD6C1)

extern PSTATUS_SUB_WORK* PSTATUS_SUB_Init( PSTATUS_WORK *work );
extern void PSTATUS_SUB_Term( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
extern void PSTATUS_SUB_Main( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
extern void PSTATUS_SUB_Draw( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );

//リソース初期化
void PSTATUS_SUB_LoadResource( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , ARCHANDLE *archandle );
void PSTATUS_SUB_ReleaseResource( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );

//セル初期化
void PSTATUS_SUB_InitCell( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
void PSTATUS_SUB_TermCell( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );

//表示非表示切り替え
void PSTATUS_SUB_DispPage( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
void PSTATUS_SUB_ClearPage( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
void PSTATUS_SUB_DispPage_Trans( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
void PSTATUS_SUB_ClearPage_Trans( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );


//デバグ用
void PSTATUS_SUB_SetShadowScale( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );
