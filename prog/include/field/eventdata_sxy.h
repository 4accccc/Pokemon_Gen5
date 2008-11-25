//============================================================================================
/**
 * @file	event_check_data.h
 * @brief	イベント起動チェック用データ定義ヘッダ
 * @date	2005.10.14
 *
 * src/fielddata/eventdata/のイベントデータコンバータからも参照されている。
 * そのためこの中に記述できる内容にはかなり制限があることを注意すること。
 *
 * 2008.11.20	DPプロジェクトからコピー。なので上記注釈は今のところ無効
 */
//============================================================================================
#pragma once

#include "field/eventdata_system.h"
#include "field/location.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
///特殊接続指定用ID
//------------------------------------------------------------------
#define	ZONE_ID_SPECIAL		(0x0fff)
#define	SPECIAL_SPEXIT01	(0x0100)

enum {
	EXIT_TYPE_NONE = 0,
	EXIT_TYPE_UP,
	EXIT_TYPE_DOWN,
	EXIT_TYPE_LEFT,
	EXIT_TYPE_RIGHT,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _EVENTDATA_HEADER EVENTDATA_HEADER;

struct _EVENTDATA_HEADER {
	u32 bg_count;
	u32 obj_count;
	u32 connect_count;
	u32 pos_count;
};
//------------------------------------------------------------------
///ドアリンク用データ構造体
//------------------------------------------------------------------
typedef struct _CONNECT_DATA CONNECT_DATA;

struct _CONNECT_DATA{
	VecFx32 pos;
	u16	link_zone_id;
	u16	link_exit_id;
	s16	exit_type;
};

//------------------------------------------------------------------
///BG話かけデータ構造体
//------------------------------------------------------------------
typedef struct _BG_TALK_DATA BG_TALK_DATA;

struct _BG_TALK_DATA{
	u16	id;			// ID
	u16	type;		// データタイプ
	int	gx;			// X座標
	int	gz;			// Y座標
	int	height;		// 高さ
	u16	dir;		// 話しかけ方向タイプ
};

//------------------------------------------------------------------
///POS発動イベントデータ構造体
//------------------------------------------------------------------
typedef struct _POS_EVENT_DATA POS_EVENT_DATA;

struct _POS_EVENT_DATA{
	u16	id;			//ID
	u16	gx;			//x
	u16	gz;			//z
	u16	sx;			//sizeX
	u16	sz;			//sizeZ
	u16	height;		//height
	u16 param;
	u16 workID;
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern const CONNECT_DATA * EVENTDATA_SearchConnectByPos(const EVENTDATA_SYSTEM * evdata, const VecFx32 * pos);
extern const CONNECT_DATA * EVENTDATA_GetConnectByID(const EVENTDATA_SYSTEM * evdata, u16 exit_id);
extern void CONNECTDATA_SetLocation(const CONNECT_DATA * connect, LOCATION * loc);


