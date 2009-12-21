//=============================================================================================
/**
 * @file  btl_string.h
 * @brief ポケモンWB バトル 文字列生成処理
 * @author  taya
 *
 * @date  2008.10.06  作成
 */
//=============================================================================================
#pragma once

#include "btl_main_def.h"
#include "btl_client.h"
#include "btl_pokeparam.h"

typedef enum {
  BTL_STRTYPE_NULL = 0,
  BTL_STRTYPE_STD,
  BTL_STRTYPE_SET,
  BTL_STRTYPE_UI,
  BTL_STRTYPE_WAZA,
  BTL_STRTYPE_WAZAOBOE,
  BTL_STRTYPE_YESNO,
}BtlStrType;

enum {
  BTL_STR_ARG_MAX = 8,
};


extern void BTL_STR_InitSystem( const BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, const BTL_POKE_CONTAINER* pokeCon, HEAPID heapID );
extern void BTL_STR_QuitSystem( void );

//---------------------------------------------------------------------------------------
/**
 * 標準メッセージの生成
 *
 * 標準メッセージ：対象者なし、あるいは対象が誰であっても一定のフォーマットで生成される文字列
 */
//---------------------------------------------------------------------------------------

#include "msg/msg_btl_std.h"

enum {
  BTL_STRID_STD_MAX = msg_btl_std_max,
};

typedef u16 BtlStrID_STD;

extern void BTL_STR_MakeStringStd( STRBUF* buf, BtlStrID_STD strID, u32 numArgs, ... );
extern void BTL_STR_MakeStringStdWithArgArray( STRBUF* buf, BtlStrID_STD strID, const int* array );


//---------------------------------------------------------------------------------------
/**
 * セットメッセージの生成
 *
 * セットメッセージ：自分側・敵側（やせいの）・敵側（あいての）が必ず３つセットになっている文字列
 * IDは、自分側のものを指定すれば良い。
 */
//---------------------------------------------------------------------------------------

#include "msg/msg_btl_set.h"

typedef u16 BtlStrID_SET;

extern void BTL_STR_MakeStringSet( STRBUF* buf, BtlStrID_SET strID, const int* args );


//=============================================================================================
/**
 * ワザメッセージの生成
 * ※ワザメッセージ：○○の××こうげき！とか。セットメッセージと同様、必ず３つセット。
 *
 * @param   buf
 * @param   pokeID
 * @param   waza
 *
 * @retval  extern void
 */
//=============================================================================================
extern void BTL_STR_MakeStringWaza( STRBUF* buf, u8 pokeID, u16 waza );





#include "msg/msg_btl_ui.h"

extern void BTL_STR_GetUIString( STRBUF* dst, u16 strID );
extern void BTL_STR_MakeWazaUIString( STRBUF* dst, u16 wazaID, u8 wazaPP, u8 wazaPPMax );


//---------------------------------------------------------------------------------------
/**
 * 技覚えメッセージの生成
 *
 * 標準メッセージ：対象者なし、あるいは対象が誰であっても一定のフォーマットで生成される文字列
 */
//---------------------------------------------------------------------------------------

#include "msg/msg_waza_oboe.h"

enum {
  BTL_STRID_WAZAOBOE_MAX = msg_waza_oboe_max,
};

typedef u16 BtlStrID_WAZAOBOE;

extern void BTL_STR_MakeStringWazaOboeWithArgArray( STRBUF* buf, BtlStrID_WAZAOBOE strID, const int* array );


//---------------------------------------------------------------------------------------
/**
 * はい／いいえメッセージの生成
 *
 * 標準メッセージ：対象者なし、あるいは対象が誰であっても一定のフォーマットで生成される文字列
 */
//---------------------------------------------------------------------------------------

#include "msg/msg_yesnomenu.h"

enum {
  BTL_STRID_YESNO_MAX = msg_yesnomenu_max,
  BTL_YESNO_MSG_LENGTH = 32,    //はい／いいえメッセージの長さ
};

typedef u16 BtlStrID_YESNO;

extern void BTL_STR_MakeStringYesNoWithArgArray( STRBUF* buf, BtlStrID_YESNO strID, const int* array );


