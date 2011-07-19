//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		medal_check.h
//   @brief   メダル達成チェック
//   @author	Masayuki Onoue
//   @date		2011.02.07
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void MEDAL_AchiveCheck( GAMEDATA* p_gamedata, SAVE_CONTROL_WORK *p_sv , HEAPID heapID );
extern void MEDAL_InitHintMedal( SAVE_CONTROL_WORK *p_sv , HEAPID heapID );
extern u32 MEDAL_GetMedalCategoryIndex( u8 graphic_type );

