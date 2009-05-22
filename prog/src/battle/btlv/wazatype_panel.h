//==============================================================================
/**
 * @file	wazatype_panel.h
 * @brief	技タイプ毎のパネルデータの取出しなど
 * @author	matsuda changed by soga
 * @date	2009.04.28(火)
 */
//==============================================================================

#pragma once

#include "system/palanm.h"

//==============================================================================
//	外部関数宣言
//==============================================================================
extern int WazaPanel_ArcIDGet(void);
extern u32 WazaPanel_CharIndexGet(int waza_type);
extern const u16 * WazaPanel_PalDataAdrsGet(int waza_type);
extern void WazaPanel_CharLoad( HEAPID heap_id, int frame_no, u32 trans_pos, int screen_type, const u8 *charcter );
extern void WazaPanel_EasyCharLoad( int waza_type, HEAPID heap_id, int frame_no, u32 trans_pos, int screen_type );
extern void WazaPanel_EasyPalLoad( PALETTE_FADE_PTR pfd, int waza_type, HEAPID heap_id, FADEREQ req, int palette_pos );
extern void WazaPanel_EasyCharLoad_ChainSet( int waza_type, HEAPID heap_id, 
	int frame_no, const u16 trans_pos[], int trans_num, int screen_type);


