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
///何ドットで1点とするか
#define CC_POINT_DOT		(40)

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

///円の掴み状況
enum{
	CIRCLE_TOUCH_NULL,		///<円を掴んでいない
	CIRCLE_TOUCH_HOLD,		///<円を掴んでいる
	CIRCLE_TOUCH_OUTSIDE,	///<掴んでいるが範囲外座標を指している
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

///円とタッチの当たり判定チェック用ワーク
typedef struct{
	s32 old_len_x;
	s32 old_len_y;
	s32 total_len;		///総距離

	u8 touch_hold;		///<円の掴み状況：CIRCLE_TOUCH_???
	u8 hold_circle_no;	///<掴んでいる円の番号
	u8 padding[2];
}CCT_TOUCH_SYS;


//--------------------------------------------------------------
//	通信で交換しあうデータ
//--------------------------------------------------------------
///初回接続時に交換しあうデータ
typedef struct{
	CC_CIRCLE_PACKAGE circle_package;	///<サークルパッケージ
	u8 macAddress[6];					///<通信相手のMacAddress
	u8 padding[2];
}CCNET_FIRST_PARAM;

///ゲーム終了後に交換しあうデータ
typedef struct{
	u8 point;							///<得点
	u8 dummy[3];
}CCNET_RESULT_PARAM;


#endif	//__COMPATI_TYPES_H__
