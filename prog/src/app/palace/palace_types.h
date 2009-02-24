//==============================================================================
/**
 * @file	palace_types.h
 * @brief	パレス：共通定義
 * @author	matsuda
 * @date	2009.02.20(金)
 */
//==============================================================================
#ifndef __PALACE_TYPES_H__
#define __PALACE_TYPES_H__


//==============================================================================
//	定数定義
//==============================================================================
///エディットサイズX
#define PALACE_EDIT_X		(16)
///エディットサイズY
#define PALACE_EDIT_Y		(16)
///エディットサイズZ
#define PALACE_EDIT_Z		(16)

///ブロックのポリゴン枚数
#define PALACE_POLYGON_MAX	((PALACE_EDIT_X + PALACE_EDIT_Y + PALACE_EDIT_Z) * 2)
///パレスのブロック最大数
#define PALACE_BLOCK_MAX	(PALACE_EDIT_X * PALACE_EDIT_Y * PALACE_EDIT_Z)
///1byteにいくつのブロックデータを混ぜるか
#define PALACE_ONE_BLOCK_NUM	(2)

#define ONE_GRID		(0x100)	//fx16
#define HALF_GRID		(ONE_GRID/2)	//fx16


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	u8 block[PALACE_BLOCK_MAX / PALACE_ONE_BLOCK_NUM];
}PALACE_SAVEDATA;

#endif	//__PALACE_TYPES_H__
