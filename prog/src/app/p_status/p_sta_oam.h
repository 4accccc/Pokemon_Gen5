//==============================================================================
/**
 * @file  p_sta_oam.c
 * @brief BMPをOAMに描画(ポケモンステータス用カスタム
 * @author  ariizumi
 * @date  2009.07.08
 */
//==============================================================================
#pragma once

//==============================================================================
//	構造体定義
//==============================================================================
///BMPOAMシステムの不定形ポインタ
typedef struct _PSTA_OAM_SYS * PSTA_OAM_SYS_PTR;

///BMPOAMアクターの不定形ポインタ
typedef struct _PSTA_OAM_ACT * PSTA_OAM_ACT_PTR;

///BMPOAMアクターヘッダデータ構造体
typedef struct{
	GFL_BMP_DATA *bmp;	///<OAMとして表示させるBMPデータへのポインタ
	s16 x;				///<左上X座標
	s16 y;				///<左上Y座標
	u32 pltt_index;		///<適用するパレットのindex(GFL_CLGRP_PLTT_Registerの戻り値)
	u32 pal_offset;		///<pltt_indexのパレット内でのオフセット
	u8 soft_pri;		///<ソフトプライオリティ
	u8 bg_pri;			///<BGプライオリティ
	u16 setSerface;		///<CLSYS_DEFREND_MAIN,SUB or CLWK_SETSF_NONE
	CLSYS_DRAW_TYPE draw_type;
}PSTA_OAM_ACT_DATA;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern PSTA_OAM_SYS_PTR PSTA_OAM_Init(HEAPID heap_id, GFL_CLUNIT* p_unit);
extern void PSTA_OAM_Exit(PSTA_OAM_SYS_PTR bsp);
extern PSTA_OAM_ACT_PTR PSTA_OAM_ActorAdd(PSTA_OAM_SYS_PTR bsp, const PSTA_OAM_ACT_DATA *head);
extern void PSTA_OAM_ActorDel(PSTA_OAM_ACT_PTR bact);
extern void PSTA_OAM_ActorSetDrawEnable(PSTA_OAM_ACT_PTR bact, BOOL on_off);
extern BOOL PSTA_OAM_ActorGetDrawEnable(PSTA_OAM_ACT_PTR bact);
extern void PSTA_OAM_ActorBmpTrans(PSTA_OAM_ACT_PTR bact);
extern void PSTA_OAM_ActorGetPos(PSTA_OAM_ACT_PTR bact, s16 *x, s16 *y);
extern void PSTA_OAM_ActorSetPos(PSTA_OAM_ACT_PTR bact, s16 x, s16 y);

//bmp の入れ替え(同じサイズのbmp限定！！入れ替え後要Trans
extern void PSTA_OAM_SwapBmp(PSTA_OAM_ACT_PTR act1, PSTA_OAM_ACT_PTR act2);

