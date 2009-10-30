#pragma once

typedef enum {
	SCRNTEX_VRAM_A = 0,
	SCRNTEX_VRAM_B,
	SCRNTEX_VRAM_C,
	SCRNTEX_VRAM_D,
}SCRNTEX_VRAM;

GFL_G3D_RES*	GFL_SCRNTEX_CreateG3DresTex(
	HEAPID				heapID,			// リソース生成対象ヒープID 
	SCRNTEX_VRAM	texVram			// テクスチャとして使用する対象VRAM(要0x20000: 256*256*2)
);
void					GFL_SCRNTEX_Load(
	SCRNTEX_VRAM	capVram,		// キャプチャーで使用する対象VRAM(要0x20000: 実際は256*192*2)
	SCRNTEX_VRAM	texVram			// キャプチャー画像転送VRAM(転送不要の場合は capVramと同値にする)
);

