//=============================================================================
/**
 * @file	union_beacon_tool.H
 * @brief	ユニオンルーム見た目設定処理
 * @author	Akito Mori
 * @date    	2006.03.16
 */
//=============================================================================
#pragma once

#include "print/wordset.h"


enum UNION_VIEW_INFO {
  UNIONVIEW_OBJCODE,  // フィールドOBJコード
  UNIONVIEW_TRTYPE,		// トレーナーグラフィックのＩＤ（バトルやカード時のグラフィック）
  UNIONVIEW_MSGTYPE,	// トレーナータイプのメッセージＮＯ(msg_trtype_name_dat)

  UNIONVIEW_ICONINDEX,	// ２Ｄ画像用(人物ＯＢＪ）のテーブルの場所（0-18)
};

// トレーナーVIEWTYPEと性別から情報を引き出す
extern int  UnionView_GetTrainerInfo( int view_type, int sex, int info );

extern int UnionView_GetTrainerTypeIndex( u32 id, int sex, u32 select );
extern int  UnionView_GetTrainerType( u32 id, int sex, u32 select );
extern void UnionView_SetUpTrainerTypeSelect( u32 id, int sex, WORDSET *wordset );
extern int UnionView_GetCharaNo( int sex, int view_type );
extern int UnionView_GetObjCode(int view_index);
extern int UnionView_GetTrType(int view_index);
extern u16 *UnionView_GetPalNo( u16 * table, int sex, int view_type );
extern u16 *UnionView_PalleteTableAlloc( int heapID );

//GSのpoke_toolから移植

#define	TR_PARA_BACK		(0)		///<背面
#define	TR_PARA_FRONT		(2)		///<正面

//TrCLACTGraDataGetで使用される構造体
typedef struct{
	int	arcID;		//トレーナーグラフィックデータのArcID
	int	ncgrID;		//トレーナーグラフィックデータのキャラID
	int	nclrID;		//トレーナーグラフィックデータのパレットID
	int	ncerID;		//トレーナーグラフィックデータのセルID
	int	nanrID;		//トレーナーグラフィックデータのセルアニメID
	int	ncbrID;		//トレーナーグラフィックデータのキャラID（ソフトスプライト用）
}TR_CLACT_GRA;

extern void	TrCLACTGraDataGet(int trtype,int dir,TR_CLACT_GRA *tcg);
extern void	TrCLACTGraDataGetEx(int trtype,int dir,BOOL non_throw,TR_CLACT_GRA *tcg);

