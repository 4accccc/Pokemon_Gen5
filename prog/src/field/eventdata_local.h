//============================================================================================
/**
 * @file	eventdata_local.h
 * @brief
 * @date	2008.12.05
 * @author	tamada GAMEFREAK inc.
 */
//============================================================================================

#pragma once

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _EVENTDATA_HEADER {
	u32 bg_count;
	u32 obj_count;
	u32 connect_count;
	u32 pos_count;
};


//------------------------------------------------------------------
///ドアリンク用データ構造体
//------------------------------------------------------------------
/// グリッドポジション
typedef struct {
  s16 x;              ///<
  s16 y;
  s16 z;
  u16 sizex;
  u16 sizez;
} CONNECT_DATA_GPOS;

/// レールポジション
typedef struct {
  u16 rail_index;       ///<レールインデックス
  u16 front_grid;       ///<前方レールグリッド座標  
  s16 side_grid;        ///<サイドレールグリッド座標
  u16 front_grid_size;  ///<前方範囲グリッドサイズ
  u16 side_grid_size;   ///<サイド範囲グリッドサイズ
} CONNECT_DATA_RPOS;

// ポジションバッファサイズ
#define CONNECT_DATA_POSBUF_SIZE  (12)

/// ドアリンクデータ構造体
struct _CONNECT_DATA{
	//VecFx32 pos;
	u16	link_zone_id;
	u16	link_exit_id;
	u8	exit_dir;
	u8	exit_type;

  u16 pos_type; // ポジションタイプ EVENTDATA_POSTYPE
  u8  pos_buf[CONNECT_DATA_POSBUF_SIZE];
};

//------------------------------------------------------------------
///BG話かけデータ構造体
//------------------------------------------------------------------
/// グリッドポジション
typedef struct {
	int	gx;			// X座標
	int	gz;			// Y座標
	int	height;		// 高さ
} BG_TALK_DATA_GPOS;

/// レールポジション
typedef struct {
  u16 rail_index;       ///<レールインデックス
  u16 front_grid;       ///<前方レールグリッド座標
  s16 side_grid;        ///<サイドレールグリッド座標
} BG_TALK_DATA_RPOS;

// ポジション構造体サイズ
#define BG_TALK_DATA_POSBUF_SIZE  (12)

/// BG話かけデータ構造体
struct _BG_TALK_DATA{
	u16	id;			// ID
	u16	type;		// データタイプ
	u16	dir;		// 話しかけ方向タイプ

  u16 pos_type; // ポジションタイプ EVENTDATA_POSTYPE
  u8  pos_buf[BG_TALK_DATA_POSBUF_SIZE];
};

//------------------------------------------------------------------
///POS発動イベントデータ構造体
//------------------------------------------------------------------
/// グリッドポジション
typedef struct {
	u16	gx;			//x
	u16	gz;			//z
	u16	sx;			//sizeX
	u16	sz;			//sizeZ
	s16	height;		//height
} POS_EVENT_DATA_GPOS;

/// レールポジション
typedef struct {
  u16 rail_index;       ///<レールインデックス
  u16 front_grid;       ///<前方レールグリッド座標
  s16 side_grid;        ///<サイドレールグリッド座標
  u16 front_grid_size;  ///<前方範囲グリッドサイズ
  u16 side_grid_size;   ///<サイド範囲グリッドサイズ
} POS_EVENT_DATA_RPOS;

// ポジションバッファサイズ
#define POS_EVENT_DATA_POSBUF_SIZE  (12)

/// POS発動イベントデータ構造体
struct _POS_EVENT_DATA{
	u16	id;			//ID
	u16 param;
	u16 workID;
  u16 check_type;

  u16 pos_type; // ポジションタイプ EVENTDATA_POSTYPE
  u8  pos_buf[POS_EVENT_DATA_POSBUF_SIZE];
};

