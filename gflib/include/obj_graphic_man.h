//==============================================================================
/**
 *
 *@file		obj_graphic.h
 *@brief	OBJ用グラフィックデータ転送、管理システム
 *@author	taya
 *@data		2006.11.28
 *
 */
//==============================================================================
#ifndef __OBJ_GRAPHIC_H__
#define __OBJ_GRAPHIC_H__

//#include "vram.h"

#include "clact.h"

//==============================================================
//	初期化用構造体
//==============================================================
typedef struct {
	u16  CGR_RegisterMax;			///< 登録できるキャラデータ数
	u16  PLTT_RegisterMax;			///< 登録できるパレットデータ数
	u16  CELL_RegisterMax;			///< 登録できるセルアニメパターン数
	u16  MULTICELL_RegisterMax;		///< 登録できるマルチセルアニメパターン数（※現状未対応）
}GFL_OBJGRP_INIT_PARAM;


//==============================================================
//	アクター追加用構造体
//==============================================================
typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
}GFL_OBJGRP_CLWKDAT;

//==============================================================
//	定数
//==============================================================
enum {
	GFL_OBJGRP_REGISTER_FAILED = 0xffffffff,
};

//==============================================================
//	
//==============================================================
typedef enum {
	GFL_VRAM_2D_MAIN = 1,
	GFL_VRAM_2D_SUB = 2,
	GFL_VRAM_2D_BOTH = GFL_VRAM_2D_MAIN | GFL_VRAM_2D_SUB,
}GFL_VRAM_TYPE;




extern void GFL_OBJGRP_sysInit( void );
extern void GFL_OBJGRP_sysExit( void );

extern void GFL_OBJGRP_sysStart(	u16 heapID, const GFL_BG_DISPVRAM* vramBank,
									const GFL_OBJGRP_INIT_PARAM* initParam );
extern void GFL_OBJGRP_sysEnd( void );

extern u32 GFL_OBJGRP_RegisterCGR( void* dataPtr, GFL_VRAM_TYPE targetVram );
extern u32 GFL_OBJGRP_RegisterCGR_VramTransfer( void* dataPtr, GFL_VRAM_TYPE targetVram, 
												u32 cellIndex );
extern void GFL_OBJGRP_ReleaseCGR( u32 index );

extern u32 GFL_OBJGRP_RegisterCellAnim( void* cellDataPtr, void* animDataPtr );
extern BOOL GFL_OBJGRP_CellBankHasVramTransferData( u32 index );
extern void GFL_OBJGRP_ReleaseCellAnim( u32 index );

extern u32 GFL_OBJGRP_RegisterPltt( void* plttData, GFL_VRAM_TYPE vramType, u32 byteOffs );
extern void GFL_OBJGRP_ReleasePltt( u32 index );

#if 0
extern CLACT_WORK_PTR GFL_OBJGRP_CreateClAct(	CLACT_SET_PTR actset, u32 cgrIndex, u32 plttIndex, 
												u32 cellAnimIndex, const GFL_OBJGRP_CLWKDAT* param,
												u8 drawArea, u16 heapID );
#endif

#endif /* #ifndef __OBJMAN_H__ */
