//============================================================================================
/**
 * @file	eventdata_type.h
 * @brief	イベント起動チェック用データ定義ヘッダ
 * @author  tamada GAMEFREAK inc.
 * @date	2005.10.14
 *
 * resource/fldmapdata/eventdata/のイベントデータコンバータからも参照されている。
 * そのためこの中に記述できる内容にはかなり制限があることを注意すること。
 *
 * 2008.11.20	DPプロジェクトからコピー。
 * 2009.07.25 eventdata_sxy.hから分離
 */
//============================================================================================
#pragma once

//#include "field/eventdata_system.h"
//#include "field/location.h"
//#include "field/fldmmdl.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
///  無効接続指定用ID：ゾーン指定
//------------------------------------------------------------------
#define ZONE_ID_NONE    (0xffff)

//------------------------------------------------------------------
/// 無効接続指定用ID：出入口指定
//------------------------------------------------------------------
#define EXIT_ID_NONE		(0xffff)

//------------------------------------------------------------------
/// 接続イベントの種類
//------------------------------------------------------------------
typedef enum {
	EXIT_TYPE_NONE = 0,
	EXIT_TYPE_MAT,
	EXIT_TYPE_STAIRS,
	EXIT_TYPE_DOOR,
	EXIT_TYPE_WALL,
  EXIT_TYPE_WARP,
  EXIT_TYPE_INTRUDE,
  EXIT_TYPE_SP1,
  EXIT_TYPE_SP2,
  EXIT_TYPE_SP3,
  EXIT_TYPE_SP4,
  EXIT_TYPE_SP5,
  EXIT_TYPE_SP6,
  EXIT_TYPE_SP7,
  EXIT_TYPE_SP8,
  EXIT_TYPE_SP9,
  EXIT_TYPE_SP10,
  EXIT_TYPE_SP11,
  EXIT_TYPE_SP12,
  EXIT_TYPE_SP13,
  EXIT_TYPE_SP14,
  EXIT_TYPE_SP15,
  EXIT_TYPE_SP16,
  EXIT_TYPE_SP17,
  EXIT_TYPE_SP18,
  EXIT_TYPE_SP19,
  EXIT_TYPE_SP20,
  EXIT_TYPE_SP21,
  EXIT_TYPE_SP22,
  EXIT_TYPE_SP23,
  EXIT_TYPE_SP24,
  EXIT_TYPE_SP25,
  EXIT_TYPE_SP26,
  EXIT_TYPE_SP27,
  EXIT_TYPE_SP28,
  EXIT_TYPE_SP29,
  EXIT_TYPE_SP30,

  EXIT_TYPE_MAX,
}EXIT_TYPE;

//------------------------------------------------------------------
/// 出入口方向指定
//------------------------------------------------------------------
typedef enum {
	EXIT_DIR_NON = 0,
	EXIT_DIR_UP,
	EXIT_DIR_DOWN,
	EXIT_DIR_LEFT,
	EXIT_DIR_RIGHT,

	EXIT_DIR_MAX
}EXIT_DIR;

//------------------------------------------------------------------
/// POSイベントの種類
//------------------------------------------------------------------
typedef enum {
  POS_CHECK_TYPE_NORMAL = 0,
  POS_CHECK_TYPE_UP,
  POS_CHECK_TYPE_DOWN,
  POS_CHECK_TYPE_LEFT,
  POS_CHECK_TYPE_RIGHT,

  POS_CHECK_TYPE_MAX,
}POS_CHECK_TYPE;


//------------------------------------------------------------------
/// BGイベントの種類
//------------------------------------------------------------------
typedef enum{
  BG_TALK_TYPE_NORMAL,  //話し掛けイベント
  BG_TALK_TYPE_BOARD, //看板イベント
  BG_TALK_TYPE_HIDE,  //隠しアイテム
}BG_TALK_TYPE;

//------------------------------------------------------------------
/// BGイベントの話しかけ種類指定
//------------------------------------------------------------------
typedef enum{
  BG_TALK_DIR_DOWN = 0,
  BG_TALK_DIR_LEFT,
  BG_TALK_DIR_RIGHT,
  BG_TALK_DIR_UP,
  BG_TALK_DIR_ALL,
  BG_TALK_DIR_SIDE,
  BG_TALK_DIR_UPDOWN,
}BG_TALK_DIR;

//------------------------------------------------------------------
// ポジションタイプ
//------------------------------------------------------------------
typedef enum
{
  EVENTDATA_POSTYPE_GRID = 0,   // グリッドポジション
  EVENTDATA_POSTYPE_RAIL,   // レールポジション

  EVENTDATA_POSTYPE_MAX,  // 最大数
} EVENTDATA_POSTYPE;


//------------------------------------------------------------------
///ドアリンク用データ構造体
//------------------------------------------------------------------
typedef struct _CONNECT_DATA CONNECT_DATA;

//------------------------------------------------------------------
///BG話かけデータ構造体
//------------------------------------------------------------------
typedef struct _BG_TALK_DATA BG_TALK_DATA;

//------------------------------------------------------------------
///POS発動イベントデータ構造体
//------------------------------------------------------------------
typedef struct _POS_EVENT_DATA POS_EVENT_DATA;

