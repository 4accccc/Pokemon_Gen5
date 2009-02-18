//=============================================================================================
/**
 * @file	disputil.h
 * @brief	ディスプレイユーティリティー
 * @author	
 * @date	
 */
//=============================================================================================
#ifndef _DISPUT_H_
#define _DISPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GFL_DISPUT_BGID_M0 = 0,
	GFL_DISPUT_BGID_M1,
	GFL_DISPUT_BGID_M2,
	GFL_DISPUT_BGID_M3,
	GFL_DISPUT_BGID_S0,
	GFL_DISPUT_BGID_S1,
	GFL_DISPUT_BGID_S2,
	GFL_DISPUT_BGID_S3,
}GFL_DISPUT_BGID;

typedef enum {
	GFL_DISPUT_PALID_0	= 0,
	GFL_DISPUT_PALID_1,
	GFL_DISPUT_PALID_2,
	GFL_DISPUT_PALID_3,
	GFL_DISPUT_PALID_4,
	GFL_DISPUT_PALID_5,
	GFL_DISPUT_PALID_6,
	GFL_DISPUT_PALID_7,
	GFL_DISPUT_PALID_8,
	GFL_DISPUT_PALID_9,
	GFL_DISPUT_PALID_10,
	GFL_DISPUT_PALID_11,
	GFL_DISPUT_PALID_12,
	GFL_DISPUT_PALID_13,
	GFL_DISPUT_PALID_14,
	GFL_DISPUT_PALID_15,
}GFL_DISPUT_PALID;

typedef enum {
	GFL_DISPUT_PALMASK_0	= (0x0001 << GFL_DISPUT_PALID_0),
	GFL_DISPUT_PALMASK_1	= (0x0001 << GFL_DISPUT_PALID_1),
	GFL_DISPUT_PALMASK_2	= (0x0001 << GFL_DISPUT_PALID_2),
	GFL_DISPUT_PALMASK_3	= (0x0001 << GFL_DISPUT_PALID_3),
	GFL_DISPUT_PALMASK_4	= (0x0001 << GFL_DISPUT_PALID_4),
	GFL_DISPUT_PALMASK_5	= (0x0001 << GFL_DISPUT_PALID_5),
	GFL_DISPUT_PALMASK_6	= (0x0001 << GFL_DISPUT_PALID_6),
	GFL_DISPUT_PALMASK_7	= (0x0001 << GFL_DISPUT_PALID_7),
	GFL_DISPUT_PALMASK_8	= (0x0001 << GFL_DISPUT_PALID_8),
	GFL_DISPUT_PALMASK_9	= (0x0001 << GFL_DISPUT_PALID_9),
	GFL_DISPUT_PALMASK_10	= (0x0001 << GFL_DISPUT_PALID_10),
	GFL_DISPUT_PALMASK_11	= (0x0001 << GFL_DISPUT_PALID_11),
	GFL_DISPUT_PALMASK_12	= (0x0001 << GFL_DISPUT_PALID_12),
	GFL_DISPUT_PALMASK_13	= (0x0001 << GFL_DISPUT_PALID_13),
	GFL_DISPUT_PALMASK_14	= (0x0001 << GFL_DISPUT_PALID_14),
	GFL_DISPUT_PALMASK_15	= (0x0001 << GFL_DISPUT_PALID_15),
}GFL_DISPUT_PALMASK;

//============================================================================================
/**
 *
 * @brief	ＶＲＡＭ保存関数
 *
 */
//============================================================================================
typedef struct _GFL_DISPUT_VRAMSV GFL_DISPUT_VRAMSV;

//============================================================================================
/**
 *
 * @brief	ＶＲＡＭ保存関数
 *
 */
//============================================================================================
extern GFL_DISPUT_VRAMSV* GFL_DISPUT_CreateVramSv
		( GFL_DISPUT_BGID bgID, u32 cgxSize, u32 scrSize, u16 pltMask, HEAPID heapID );
extern void GFL_DISPUT_DeleteVramSv( GFL_DISPUT_VRAMSV* vramSv );

//============================================================================================
extern void GFL_DISPUT_PushVram( GFL_DISPUT_VRAMSV* vramSv );
extern void GFL_DISPUT_PopVram( GFL_DISPUT_VRAMSV* vramSv );

//============================================================================================
/**
 *
 * @brief	ハードアクセス関数など
 *
 */
//============================================================================================
extern void GFL_DISPUT_VisibleOn( GFL_DISPUT_BGID bgID );
extern void GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID bgID );
//============================================================================================
extern void* GFL_DISPUT_GetCgxPtr( GFL_DISPUT_BGID bgID );
extern void GFL_DISPUT_LoadCgx( GFL_DISPUT_BGID bgID, void* src, u32 offs, u32 siz );
extern void* GFL_DISPUT_GetCgxDataNNSbin( void* NNSbin );
//============================================================================================
extern void* GFL_DISPUT_GetScrPtr( GFL_DISPUT_BGID bgID );
extern void GFL_DISPUT_LoadScr( GFL_DISPUT_BGID bgID, void* src, u32 offs, u32 siz );
extern void* GFL_DISPUT_GetScrDataNNSbin( void* NNSbin );
//============================================================================================
extern void* GFL_DISPUT_GetPltPtr( GFL_DISPUT_BGID bgID );
extern void GFL_DISPUT_LoadPlt( GFL_DISPUT_BGID bgID, void* src, u32 offs );
extern void* GFL_DISPUT_GetPltDataNNSbin( void* NNSbin );
//============================================================================================
extern void GFL_DISPUT_GetControl( GFL_DISPUT_BGID bgID, u16* control );
extern void GFL_DISPUT_SetControl( GFL_DISPUT_BGID bgID, u16* control );
extern void GFL_DISPUT_SetScroll( GFL_DISPUT_BGID bgID, int* x, int* y );
extern void GFL_DISPUT_SetAlpha( GFL_DISPUT_BGID bgID, int e1, int e2 );

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
