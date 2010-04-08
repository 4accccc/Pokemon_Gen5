////////////////////////////////////////////////////////////////////////////
/**
 * @file   place_name_letter.h
 * @brief  地名表示の文字オブジェクト
 * @author obata_toshihiro
 * @date   2010.03.28
 */
////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "print/gf_font.h"  // for GFL_FONT
#include "system/bmp_oam.h" // for BmpOam_xxxx


//=========================================================================
// ■不完全型
//=========================================================================
typedef struct _PN_LETTER PN_LETTER;


//=========================================================================
// ■セットアップ・パラメータ
//=========================================================================
typedef struct {

	float x;		// 座標
	float y;		// 
	float sx;		// 速度
	float sy;		// 
	float dx;		// 目標位置
	float dy;		// 
	float dsx;	// 目標位置での速度
	float dsy;	// 

  GFL_FONT* font; // フォント
  STRCODE   code; // 文字

} PN_LETTER_SETUP_PARAM;


//=========================================================================
// ■public functions
//=========================================================================
extern PN_LETTER* PN_LETTER_Create( HEAPID heapID, BMPOAM_SYS_PTR bmpOamSys, u32 plttRegIdx );
extern void PN_LETTER_Delete( PN_LETTER* letter );
extern void PN_LETTER_Setup( PN_LETTER* letter, const PN_LETTER_SETUP_PARAM* param );
extern void PN_LETTER_Main( PN_LETTER* letter );
extern void PN_LETTER_MoveStart( PN_LETTER* letter );
extern const GFL_BMP_DATA* PN_LETTER_GetBitmap( const PN_LETTER* letter );
extern void PN_LETTER_SetDrawEnable( PN_LETTER* letter, BOOL enable );
extern int PN_LETTER_GetWidth( const PN_LETTER* letter );
extern int PN_LETTER_GetHeight( const PN_LETTER* letter );
extern int PN_LETTER_GetLeft( const PN_LETTER* letter );
extern int PN_LETTER_GetTop( const PN_LETTER* letter );
extern BOOL PN_LETTER_IsMoving( const PN_LETTER* letter );
