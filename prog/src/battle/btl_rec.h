//=============================================================================================
/**
 * @file  btl_rec.h
 * @brief ポケモンWB バトルシステム クライアント操作記録データ生成
 * @author  taya
 *
 * @date  2009.12.06  作成
 */
//=============================================================================================
#pragma once

#include "btl_action.h"

/*===========================================================================================*/
/*                                                                                           */
/* サーバー用                                                                                */
/*                                                                                           */
/* クライアントに送信するデータを生成するためのツール類                                      */
/*                                                                                           */
/*===========================================================================================*/

enum {
  BTL_RECTOOL_BUFSIZE = 60,
};

/**
 *  サーバが記録データを生成させるためのワーク領域
 */
typedef struct {

  u8  writePtr;         ///< 書き込みポインタ
  u8  clientBit;        ///< クライアント毎の書き込み完了ビット
  u8  numClients : 3;   ///< 書き込んだクライアント数
  u8  type       : 4;   ///< 書き込み中のデータタイプ
  u8  fError     : 1;   ///< エラー発生フラグ

  u8  buffer[ BTL_RECTOOL_BUFSIZE ];   ///< 書き込みバッファ

}BTL_RECTOOL;

extern void BTL_RECTOOL_Init( BTL_RECTOOL* recTool );
extern void BTL_RECTOOL_PutSelActionData( BTL_RECTOOL* recTool, u8 clientID, const BTL_ACTION_PARAM* action, u8 numAction );
extern void* BTL_RECTOOL_FixSelActionData( BTL_RECTOOL* recTool, u32* dataSize );



/*===========================================================================================*/
/*                                                                                           */
/* クライアント用                                                                            */
/*                                                                                           */
/* サーバーから送られてきたデータを格納＆読み取るための管理モジュール                        */
/*                                                                                           */
/*===========================================================================================*/

typedef struct _BTL_REC   BTL_REC;

extern BTL_REC* BTL_REC_Create( HEAPID heapID );
extern void BTL_REC_Delete( BTL_REC* wk );
extern void BTL_REC_Write( BTL_REC* wk, const void* data, u32 size );
extern BOOL BTL_REC_IsCorrect( const BTL_REC* wk );




