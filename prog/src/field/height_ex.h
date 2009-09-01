//============================================================================================
/**
 * @file	height_ex.h
 * @brief	ägí£çÇÇ≥ÉfÅ[É^
 * @author	Nozomu Saito
 * @date	2005.12.05
 */
//============================================================================================

#ifndef __HEIGHT_EX_H__
#define __HEIGHT_EX_H__

typedef struct EX_HEIGHT_LIST_tag * EHL_PTR;
typedef struct EX_HEIGHT_LIST_tag const * EHL_CONST_PTR;

extern EHL_PTR EXH_AllocHeightList(const u8 inNum, const u8 inHeapID);

extern void EXH_SetUpHeightData(	const u8 inIndex,
									const int inStX,
									const int inStZ,
									const int inSizX,
									const int inSizZ,
									const fx32 inHeight,
									EHL_PTR outExHeightList );

extern void EXH_FreeHeightList(EHL_PTR outExHeightList);

extern int EXH_GetDatNum(EHL_PTR inExHeightList);

extern BOOL EXH_HitCheckHeight(	const int inGridX, const int inGridZ,
									EHL_CONST_PTR inExHeightList,
									const u8 inIdx);

extern fx32 EXH_GetHeight(const u8 inIndex, EHL_CONST_PTR inExHeightList);

extern void EXH_SetHeight(	const u8 inIndex, const fx32 inHeight, EHL_PTR outExHeightList );

extern void EXH_SetHeightValid(	const u8 inIndex, const BOOL inValid, EHL_PTR outExHeightList );

#endif  //__HEIGHT_EX_H__

