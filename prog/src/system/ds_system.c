//======================================================================
/**
 * @file	ds_system.c
 * @brief	DSの設定等の取得ラッパー
 * @author	ariizumi
 * @data	20/01/23
 *
 * モジュール名：DS_SYSTEM
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/ds_system.h"
#include "debug/debug_flg.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================

//--------------------------------------------------------------
//	@berif DSiで起動しているか？
//
//  @return BOOL TRUE DSi
//--------------------------------------------------------------
const BOOL DS_SYSTEM_IsRunOnTwl( void )
{
  return OS_IsRunOnTwl();
}


//--------------------------------------------------------------
//	@berif ペアレンタルコントロールで写真の送受信を拒否しているか？
//
//  @return BOOL TRUE 送受信拒否
//  @return BOOL FALSE 送受信可 or DS
//--------------------------------------------------------------
const BOOL DS_SYSTEM_IsRestrictPhotoExchange( void )
{
  if( DEBUG_FLG_GetFlg( DEBUG_FLG_RestricPhoto ) == TRUE )
  {
    return TRUE;
  }
  
#if (defined(SDK_TWL))
  if( DS_SYSTEM_IsRunOnTwl() )
  { 
    return OS_IsRestrictPhotoExchange();
  }
#endif
  return FALSE;
}

//--------------------------------------------------------------
//	@berif ペアレンタルコントロールでユーザー作成コンテンツの
//         送受信を拒否しているか？
//
//  @return BOOL TRUE 送受信拒否
//  @return BOOL FALSE 送受信可 or DS
//--------------------------------------------------------------
const BOOL DS_SYSTEM_IsRestrictUGC( void )
{
  if( DEBUG_FLG_GetFlg( DEBUG_FLG_RestricUGC ) == TRUE )
  {
    return TRUE;
  }

#if (defined(SDK_TWL))
  if( DS_SYSTEM_IsRunOnTwl() )
  { 
    return OS_IsRestrictUGC();
  }
#endif
  return FALSE;
}


//--------------------------------------------------------------
//	@berif DSの無線通信設定の取得
//
//  @return BOOL TRUE 通信許可 or DS
//  @return BOOL FALSE 通信不可
//--------------------------------------------------------------
const BOOL DS_SYSTEM_IsAvailableWireless( void )
{
  if( DEBUG_FLG_GetFlg( DEBUG_FLG_DisableWL ) == TRUE )
  {
    return FALSE;
  }

#if (defined(SDK_TWL))
  return OS_IsAvailableWireless();
#else
  return TRUE;
#endif
}

//--------------------------------------------------------------
//	@berif Wifiの使用規約同意チェック
//         ここでTRUEでもWifi接続時のチェック・バージョン更新などでつなげない時もある。
//
//  @return BOOL TRUE 同意済み
//  @return BOOL FALSE 同意してない
//--------------------------------------------------------------
const BOOL DS_SYSTEM_IsAgreeEULA( void )
{
  if( DEBUG_FLG_GetFlg( DEBUG_FLG_DisableEULA ) == TRUE )
  {
    return FALSE;
  }

#if (defined(SDK_TWL))
  if( DS_SYSTEM_IsRunOnTwl() == TRUE )
  {
    return OS_IsAgreeEULA();
  }
  else
  {
    //本来だとDSの場合FALSEを返すが、利便性のためTRUE
    return TRUE;
  }
#else
  return TRUE;
#endif
}

//--------------------------------------------------------------
//	@berif DSの誕生日設定の取得
//
//	@param  month 月格納先
//	@param  day   日格納先
//--------------------------------------------------------------
void DS_SYSTEM_GetBirthDay( u8* month, u8* day )
{
  OSOwnerInfo info;
  OS_GetOwnerInfo( &info );
  *month   = info.birthday.month;
  *day     = info.birthday.day;
}



