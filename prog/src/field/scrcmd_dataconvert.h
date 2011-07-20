//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_dataconvert.h
 *	@brief  データコンバート用スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.07.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//include/multiboot/wb_save_convert.hのWB_SAVE_CONV_FLAGの順番と対応
#define DATACONV_FLAG_GAME_CLEAR        ( 0)
#define DATACONV_FLAG_FIRST_POKE_GET    ( 1)
#define DATACONV_FLAG_CHAMPION_WIN      ( 2)
#define DATACONV_FLAG_BSUBWAY_AWARD_01  ( 3)
#define DATACONV_FLAG_BSUBWAY_AWARD_02  ( 4)
#define DATACONV_FLAG_BSUBWAY_TROPHY_01 ( 5)
#define DATACONV_FLAG_BSUBWAY_TROPHY_02 ( 6)
#define DATACONV_FLAG_BSUBWAY_TROPHY_03 ( 7)
#define DATACONV_FLAG_BADGE_01          ( 8)
#define DATACONV_FLAG_BADGE_02          ( 9)
#define DATACONV_FLAG_BADGE_03          (10)
#define DATACONV_FLAG_BADGE_04          (11)
#define DATACONV_FLAG_BADGE_05          (12)
#define DATACONV_FLAG_BADGE_06          (13)
#define DATACONV_FLAG_BADGE_07          (14)
#define DATACONV_FLAG_BADGE_08          (15)

//include/multiboot/wb_save_convert.hのWB_SAVE_CONV_WORKの順番と対応
#define DATACONV_N_BATTLE_RAIMON        ( 0)

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
#ifndef	ASM_CPP
extern VMCMD_RESULT EvCmdDataConv_GetFlag( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdDataConv_GetWork( VMHANDLE *p_core, void *p_wk_adrs );
#endif //ASM_CPP
