//==============================================================================
/**
 * @file	compati_types.h
 * @brief	相性チェックの共通ヘッダ
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#ifndef __COMPATI_TYPES_H__
#define __COMPATI_TYPES_H__

#include <gflib.h>


//==============================================================================
//	定数定義
//==============================================================================
///円アクターを同時に出せる数
#define CC_CIRCLE_MAX	(8)

///円のタイプ	※アニメのシーケンス番号と並びを同じにしておくと！
enum{
	CC_CIRCLE_TYPE_XL,
	CC_CIRCLE_TYPE_L,
	CC_CIRCLE_TYPE_M,
	CC_CIRCLE_TYPE_S,
	
	CC_CIRCLE_TYPE_NULL,	//データ終端コードとして使用
};

//==============================================================================
//	型定義
//==============================================================================
///円のパラメータ
typedef struct{
	u8 type;			//円の種類(CC_CIRCLE_TYPE_NULLの場合はデータ終端)
	u8 x;
	u8 y;
	u8 point_limit;		//点数の上限
}CC_CIRCLE_PARAM;

///判定で使用する円パラメータの塊
typedef struct{
	CC_CIRCLE_PARAM data[CC_CIRCLE_MAX];
}CC_CIRCLE_PACKAGE;

///コントロールPROCから子PROCへ渡すParentWork
typedef struct{
	CC_CIRCLE_PACKAGE circle_package;	///<サークルパッケージ
	
	u8 partner_macAddress[6];			///<通信相手のMacAddress
	u8 padding[2];
}COMPATI_PARENTWORK;


#endif	//__COMPATI_TYPES_H__
