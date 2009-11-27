/*
 *  @file   event_debug_local.h
 *  @brief  フィールドデバッグメニュー　ローカル共有ヘッダ
 *  @author Miyuki Iwasawa
 *  @date   09.11.25
 */

#pragma once

//======================================================================
//  typedef struct
//======================================================================
//--------------------------------------------------------------
/// DEBUG_MENU_EVENT_WORK
//--------------------------------------------------------------
typedef struct _TAG_DEBUG_MENU_EVENT_WORK DEBUG_MENU_EVENT_WORK;

//--------------------------------------------------------------
/// メニュー呼び出し関数
/// BOOL TRUE=イベント継続 FALSE==デバッグメニューイベント終了
//--------------------------------------------------------------
typedef BOOL (* DEBUG_MENU_CALLPROC)(DEBUG_MENU_EVENT_WORK*);

//--------------------------------------------------------------
/// DEBUG_MENU_EVENT_WORK
//--------------------------------------------------------------
struct _TAG_DEBUG_MENU_EVENT_WORK
{
  HEAPID heapID;
  GMEVENT *gmEvent;
  GAMESYS_WORK *gmSys;
  GAMEDATA * gdata;
  FIELDMAP_WORK * fieldWork;
  
  DEBUG_MENU_CALLPROC call_proc;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
};

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef void MAKE_LIST_FUNC( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef u16 GET_MAX_FUNC( GAMESYS_WORK* gsys, void* c_work );

//--------------------------------------------------------------
/**
 * @brief   メニュー初期化構造体
 *
 * @note
 * 可変メニューの場合、makeListFunc/getMaxFuncも定義する必要がある。
 * 使用方法についてはどこでもジャンプなどを参照のこと。
 */
//--------------------------------------------------------------
typedef struct {
  u16 msg_arc_id;     ///<引用するメッセージアーカイブの指定
  u16 max;            ///<項目最大数
  const FLDMENUFUNC_LIST * menulist;  ///<参照するメニューデータ（生成する場合はNULL)
  const FLDMENUFUNC_HEADER * menuH;   ///<メニュー表示指定データ
  u8 px, py, sx, sy;
  MAKE_LIST_FUNC * makeListFunc;      ///<メニュー生成関数へのポインタ（固定メニューの場合はNULL)
  GET_MAX_FUNC * getMaxFunc;          ///<項目最大数取得関数へのポインタ（固定メニューの場合はNULL)
}DEBUG_MENU_INITIALIZER;

//======================================================================
// extern 
//======================================================================
/*
 *  @brief  デバッグメニュー初期化
 */
extern FLDMENUFUNC * DebugMenuInit(
    FIELDMAP_WORK * fieldmap, HEAPID heapID, const DEBUG_MENU_INITIALIZER * init );
FLDMENUFUNC * DebugMenuInitEx(
    FIELDMAP_WORK * fieldmap, HEAPID heapID, const DEBUG_MENU_INITIALIZER * init, void* cb_work );
