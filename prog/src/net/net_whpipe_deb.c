//=============================================================================
/**
 * @file	net_whpipe_deb.c
 * @brief	通信システム  フィールド数値入力デバッグ機能オーバーレイ配置ソース
 * @author	GAME FREAK Inc.
 * @date    2010.05.21
 *
 * @note  通信Off時でも数値入力時デバッグ機能から呼び出せる関数を配置するために、
 *        net_whpipe.cから分離 by iwasawa
 */
//=============================================================================
#include "net/net_whpipe.h"

#ifdef PM_DEBUG

#include "net_alone_test.h"
extern u8 DebugAloneTest;

//-----------------------------------------------
/**
 *  @brief  接続相手限定コードのデバッグ数値入力　
 */
//-----------------------------------------------
u32 DEBUG_NET_WHPIPE_AloneTestCodeGet( void )
{
  if(DebugAloneTest == _DEBUG_ALONETEST_DEFAULT){
    return 0;
  }
  else if(DebugAloneTest == 0){
    return 255;
  }
  return DebugAloneTest;
}

///値を設定するための関数
void DEBUG_NET_WHPIPE_AloneTestCodeSet( u32 value )
{
  if( value == 0 ){
    value = _DEBUG_ALONETEST_DEFAULT;
  }
  else if( value == 255 ){  //製品版と繋がる
    value = 0;
  }
  DebugAloneTest = value;

  //通信が有効なら、設定値を即時反映
  if(GFL_NET_IsInit() == TRUE){
    DEBUG_GFL_NET_AloneTestCodeReflect();
  }
}

#endif  //PM_DEBUG


