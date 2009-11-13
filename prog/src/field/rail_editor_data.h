//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		rail_editor_data.h
 *	@brief	レールエディター送受信情報
 *	@author	tomoya takahashi
 *	@date		2009.07.13
 *
 *	モジュール名：
 *
 *	*レールエディターPC側でも使用しています。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	送受信データタイプ
//=====================================
typedef enum {
	RE_MCS_DATA_NONE,				// ダミー
	RE_MCS_DATA_MCSUSE,			// ダミー
	RE_MCS_DATA_RAIL,				// レール情報			DS<->PC
	RE_MCS_DATA_AREA,				// エリア情報			DS<->PC
	RE_MCS_DATA_ATTR,				// アトリビュート情報	DS<->PC
	RE_MCS_DATA_PLAYERDATA,	// プレイヤー情報	DS->PC
	RE_MCS_DATA_CAMERADATA,	// カメラ情報			DS->PC
	RE_MCS_DATA_SELECTDATA,	// 選択情報				DS<-PC
	RE_MCS_DATA_RAIL_REQ,		// レール情報吸出しリクエスト	DS<-PC
	RE_MCS_DATA_AREA_REQ,		// エリア情報吸出しリクエスト	DS<-PC
	RE_MCS_DATA_ATTR_REQ,		// アトリビュート情報吸出しリクエスト	DS<-PC
	RE_MCS_DATA_PLAYER_REQ,	// 主人公情報吸出しリクエスト	DS<-PC
	RE_MCS_DATA_CAMERA_REQ,	// カメラ情報吸出しリクエスト	DS<-PC
	RE_MCS_DATA_RESET_REQ,	// 画面リセットリクエスト	DS<-PC
	RE_MCS_DATA_END_REQ,		// 終了クエスト	DS<-PC
	RE_MCS_DATA_RAILLOCATIONDATA,		// レールロケーション情報				DS->PC
	RE_MCS_DATA_RAILLOCATION_REQ,	// レールロケーションリクエスト	DS<-PC

	RE_MCS_DATA_TYPE_MAX,
} RE_MCS_DATA_TYPE;


//-------------------------------------
///	選択情報
//=====================================
typedef enum {
	RE_SELECT_DATA_NONE,
	RE_SELECT_DATA_POINT,
	RE_SELECT_DATA_LINE,
	RE_SELECT_DATA_AREA,

	RE_SELECT_DATA_MAX,
} RE_SELECT_DATA_TYPE;

//-------------------------------------
///	選択情報ごとの、管理シーケンス
//=====================================
typedef enum {
	RE_SELECT_DATA_SEQ_NONE,
	RE_SELECT_DATA_SEQ_FREE_NORMAL,
	RE_SELECT_DATA_SEQ_FREE_CIRCLE,
	RE_SELECT_DATA_SEQ_FREE_GRID,
	RE_SELECT_DATA_SEQ_RAIL,
	RE_SELECT_DATA_SEQ_CAMERA_POS,
	RE_SELECT_DATA_SEQ_CAMERA_TARGET,
	RE_SELECT_DATA_SEQ_LINEPOS_CENTER_POS,

	RE_SELECT_DATA_SEQ_TYPE_MAX,
} RE_SELECT_DATA_SEQ_TYPE;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	送受信情報header
//=====================================
typedef struct {
	u32 data_type;		// RE_MCS_DATA_TYPE
} RE_MCS_HEADER;

//-------------------------------------
///	レール情報構造体
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	u32				reset;	//DS側受信用　画面をいったんリセットします。
	u32				rail[];
} RE_MCS_RAIL_DATA;

//-------------------------------------
///	エリア情報構造体
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	u32				reset;	//DS側受信用　画面をいったんリセットします。
	u32				area[];
} RE_MCS_AREA_DATA;

//-------------------------------------
///	エリア情報構造体
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	u32				attr[];
} RE_MCS_ATTR_DATA;

//-------------------------------------
///	プレイヤー情報構造体
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	fx32					pos_x;
	fx32					pos_y;
	fx32					pos_z;
	fx32					target_length;
} RE_MCS_PLAYER_DATA;
//-------------------------------------
///	カメラ情報構造体
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	fx32					pos_x;
	fx32					pos_y;
	fx32					pos_z;
	fx32					target_x;
	fx32					target_y;
	fx32					target_z;
	u16						pitch;
	u16						yaw;
	fx32					len;
} RE_MCS_CAMERA_DATA;

//-------------------------------------
///	選択情報構造体
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	u32						select_data;			// 選択データタイプ	RE_SELECT_DATA_TYPE
	u32						select_index;			// 選択データインデックス
	u32						select_seq;				// 選択データ管理シーケンス	RE_SELECT_DATA_SEQ_TYPE
} RE_MCS_SELECT_DATA;

//-------------------------------------
///	レールロケーション
//=====================================
typedef struct {
	RE_MCS_HEADER header;
	u32						rail_type;			// レールタイプ	FIELD_RAIL_TYPE
	u32						index;					// データインデックス
} RE_MCS_RAILLOCATION_DATA;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

#ifdef _cplusplus
}	// extern "C"{
#endif



