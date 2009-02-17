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
#define CC_POINT_DOT		80//(40)

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

///ゲームモード
enum{
	COMPATI_GAMEMODE_START,		///<ゲーム開始画面
	COMPATI_GAMEMODE_GAME,		///<ゲーム中
	COMPATI_GAMEMODE_RESULT,	///<結果発表画面
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
	u8 game_mode;						///<COMPATI_GAMEMODE_???
	u8 point;							///<自分の得点
	u8 partner_point;					///<相手の得点
	u8 padding[3];
	
	s32 irc_time_count_max;
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

///ゲーム中の赤外線接続管理ワーク
typedef struct{
	u8 seq;
	u8 shutdown_req;
	u8 connect;
	u8 padding;
	s16 wait;
	s16 timer;
}COMPATI_IRC_SYS;


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
	s32 irc_time_count_max;
}CCNET_RESULT_PARAM;

///通信管理ワーク
typedef struct{
	CCNET_FIRST_PARAM *send_first_param;		///<送信データへのポインタ
	CCNET_FIRST_PARAM *receive_first_param;		///<受信データ代入先
	CCNET_RESULT_PARAM *send_result_param;
	CCNET_RESULT_PARAM *receive_result_param;
	u8 seq;
	u8 receive_ok;
	u8 connect_ok;
	u8 connect_bit;		///<接続しているnetIDをビット管理
	u8 lib_finish;
	u8 padding[3];
}COMPATI_CONNECT_SYS;


#endif	//__COMPATI_TYPES_H__
