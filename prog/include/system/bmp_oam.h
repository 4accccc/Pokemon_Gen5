//==============================================================================
/**
 * @file	bmp_oam.h
 * @brief	BMPOAMのヘッダ
 * @author	matsuda
 * @date	2009.02.26(木)
 */
//==============================================================================
#ifndef __BMP_OAM_H__
#define __BMP_OAM_H__

//==============================================================================
//	構造体定義
//==============================================================================
///BMPOAMシステムの不定形ポインタ
typedef struct _BMPOAM_SYS * BMPOAM_SYS_PTR;

///BMPOAMアクターの不定形ポインタ
typedef struct _BMPOAM_ACT * BMPOAM_ACT_PTR;

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
}BMPOAM_ACT_DATA;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern BMPOAM_SYS_PTR BmpOam_Init(HEAPID heap_id, GFL_CLUNIT* p_unit);
extern void BmpOam_Exit(BMPOAM_SYS_PTR bsp);
extern BMPOAM_ACT_PTR BmpOam_ActorAdd(BMPOAM_SYS_PTR bsp, const BMPOAM_ACT_DATA *head);
extern void BmpOam_ActorDel(BMPOAM_ACT_PTR bact);
extern void BmpOam_ActorSetDrawEnable(BMPOAM_ACT_PTR bact, BOOL on_off);
extern BOOL BmpOam_ActorGetDrawEnable(BMPOAM_ACT_PTR bact);
extern void BmpOam_ActorBmpTrans(BMPOAM_ACT_PTR bact);
extern void BmpOam_ActorGetPos(BMPOAM_ACT_PTR bact, s16 *x, s16 *y);
extern void BmpOam_ActorSetPos(BMPOAM_ACT_PTR bact, s16 x, s16 y);
extern void BmpOam_ActorSetObjMode( BMPOAM_ACT_PTR bact, GXOamMode mode );
extern void BmpOam_ActorSetSoftPriprity( BMPOAM_ACT_PTR bact, u8 soft_pri );


#endif	//__BMP_OAM_H__
