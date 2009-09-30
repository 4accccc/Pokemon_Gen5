//============================================================================================
/**
 * @file	b_plist_ui.h
 * @brief	êÌì¨ópÉ|ÉPÉÇÉìÉäÉXÉgâÊñ  ÉCÉìÉ^Å[ÉtÉFÅ[ÉXä÷òA
 * @author	Hiroyuki Nakamura
 * @date	09.09.29
 */
//============================================================================================
#pragma once


//============================================================================================
//============================================================================================

enum {
	BPLIST_UI_LIST_POKE1 = 0,
	BPLIST_UI_LIST_POKE2,
	BPLIST_UI_LIST_POKE3,
	BPLIST_UI_LIST_POKE4,
	BPLIST_UI_LIST_POKE5,
	BPLIST_UI_LIST_POKE6,
	BPLIST_UI_LIST_RETURN,

	BPLIST_UI_CHG_ENTER = 0,	// Ç¢ÇÍÇ©Ç¶ÇÈ
	BPLIST_UI_CHG_STATUS,			// Ç¬ÇÊÇ≥ÇÇ›ÇÈ
	BPLIST_UI_CHG_WAZA,				// ÇÌÇ¥ÇÇ›ÇÈ
	BPLIST_UI_CHG_RETURN,			// Ç‡Ç«ÇÈ

	BPLIST_UI_STATUS_UP = 0,	// ëOÇÃÉ|ÉPÉÇÉìÇ÷
	BPLIST_UI_STATUS_DOWN,		// éüÇÃÉ|ÉPÉÇÉìÇ÷
	BPLIST_UI_STATUS_WAZA,		// ÇÌÇ¥ÇÇ›ÇÈ
	BPLIST_UI_STATUS_RETURN,	// Ç‡Ç«ÇÈ

	BPLIST_UI_WAZA_SEL1 = 0,	// ãZÇP
	BPLIST_UI_WAZA_SEL2,			// ãZÇQ
	BPLIST_UI_WAZA_SEL3,			// ãZÇR
	BPLIST_UI_WAZA_SEL4,			// ãZÇS
	BPLIST_UI_WAZA_UP,				// ëOÇÃÉ|ÉPÉÇÉìÇ÷
	BPLIST_UI_WAZA_DOWN,			// éüÇÃÉ|ÉPÉÇÉìÇ÷
	BPLIST_UI_WAZA_STATUS,		// Ç¬ÇÊÇ≥ÇÇ›ÇÈ
	BPLIST_UI_WAZA_RETURN,		// Ç‡Ç«ÇÈ

	BPLIST_UI_WAZAINFO_SEL1 = 0,	// ãZÇP
	BPLIST_UI_WAZAINFO_SEL2,			// ãZÇQ
	BPLIST_UI_WAZAINFO_SEL3,			// ãZÇR
	BPLIST_UI_WAZAINFO_SEL4,			// ãZÇS
	BPLIST_UI_WAZAINFO_RETURN,		// Ç‡Ç«ÇÈ

	BPLIST_UI_PPRCV_WAZA1 = 0,	// ãZÇP
	BPLIST_UI_PPRCV_WAZA2,			// ãZÇQ
	BPLIST_UI_PPRCV_WAZA3,			// ãZÇR
	BPLIST_UI_PPRCV_WAZA4,			// ãZÇS
	BPLIST_UI_PPRCV_RETURN,			// Ç‡Ç«ÇÈ

	BPLIST_UI_WAZADEL_SEL1 = 0,	// ãZÇP
	BPLIST_UI_WAZADEL_SEL2,			// ãZÇQ
	BPLIST_UI_WAZADEL_SEL3,			// ãZÇR
	BPLIST_UI_WAZADEL_SEL4,			// ãZÇS
	BPLIST_UI_WAZADEL_SEL5,			// ãZÇT
	BPLIST_UI_WAZADEL_RETURN,		// Ç‡Ç«ÇÈ

	BPLIST_UI_DELINFO_ENTER = 0,	// ÇÌÇ∑ÇÍÇÈ
	BPLIST_UI_DELINFO_RETURN,			// Ç‡Ç«ÇÈ
};


//============================================================================================
//============================================================================================

extern void BPLISTUI_Init( BPLIST_WORK * wk, u32 page, u32 pos );
extern void BPLISTUI_Exit( BPLIST_WORK * wk );
extern void BPLISTUI_ChangePage( BPLIST_WORK * wk, u32 page, u32 pos );
