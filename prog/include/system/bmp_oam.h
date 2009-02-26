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
	GFL_BMP_DATA *bmp;
	s16 x;
	s16 y;
	u32 pltt_index;
	u32 pal_offset;
	u8 soft_pri;
	u8 bg_pri;
	u16 setSerface;
	CLSYS_DRAW_TYPE draw_type;
}BMPOAM_ACT_DATA;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern BMPOAM_SYS_PTR BmpOam_Init(HEAPID heap_id, GFL_CLUNIT* p_unit);
extern void BmpOam_Exit(BMPOAM_SYS_PTR bsp);
extern BMPOAM_ACT_PTR BmpOam_ActorAdd(BMPOAM_SYS_PTR bsp, const BMPOAM_ACT_DATA *head);
extern void BmpOam_ActorDel(BMPOAM_ACT_PTR bact);
extern void BmpOam_ActorBmpTrans(BMPOAM_ACT_PTR bact);


#endif	//__BMP_OAM_H__
