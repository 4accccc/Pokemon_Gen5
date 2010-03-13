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

typedef enum {
  BTL_RECFIELD_NULL = 0,
  BTL_RECFIELD_ACTION,        ///< 通常行動選択
  BTL_RECFIELD_ROTATION,      ///< ローテーション
//  BTL_RECFIELD_POKE_CHANGE,   ///< ターン途中のポケモン入れ替え
}BtlRecFieldType;

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
  u8  numClients;       ///< 書き込んだクライアント数
  u8  type       : 6;   ///< 書き込み中のデータタイプ
  u8  fChapter   : 1;   ///< チャプターフラグ
  u8  fError     : 1;   ///< エラー発生フラグ

  u8  buffer[ BTL_RECTOOL_BUFSIZE ];   ///< 書き込みバッファ

}BTL_RECTOOL;

extern void BTL_RECTOOL_Init( BTL_RECTOOL* recTool, BOOL fChapter );
extern void BTL_RECTOOL_PutSelActionData( BTL_RECTOOL* recTool, u8 clientID, const BTL_ACTION_PARAM* action, u8 numAction );
extern void* BTL_RECTOOL_FixSelActionData( BTL_RECTOOL* recTool, u32* dataSize );
extern void BTL_RECTOOL_PutRotationData( BTL_RECTOOL* recTool, u8 clientID, BtlRotateDir dir );
extern void* BTL_RECTOOL_FixRotationData( BTL_RECTOOL* recTool, u32* dataSize );



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
extern const void* BTL_REC_GetDataPtr( const BTL_REC* wk, u32* size );


typedef struct {

  const u8*   recordData;
  u32         dataSize;
//  u32         readPtr;
  u32         readPtr[ BTL_CLIENT_MAX ];
  u8          readBuf[ BTL_CLIENT_MAX ][64];

}BTL_RECREADER;


extern void BTL_RECREADER_Init( BTL_RECREADER* wk, const void* recordData, u32 dataSize );
extern void BTL_RECREADER_Reset( BTL_RECREADER* wk );
extern const BTL_ACTION_PARAM* BTL_RECREADER_ReadAction( BTL_RECREADER* wk, u8 clientID, u8 *numAction, u8* fChapter );
extern BtlRotateDir BTL_RECREADER_ReadRotation( BTL_RECREADER* wk, u8 clientID );
extern u32 BTL_RECREADER_GetTurnCount( const BTL_RECREADER* wk );

