//=============================================================================================
/**
 * @file  btl_rec.c
 * @brief ポケモンWB バトルシステム 録画データ生成
 * @author  taya
 *
 * @date  2009.12.06  作成
 */
//=============================================================================================


#include "btl_common.h"
#include "btl_rec.h"



typedef enum {
  BTL_RECFIELD_NULL = 0,
  BTL_RECFIELD_ACTION,        ///< 通常行動選択
  BTL_RECFIELD_POKE_CHANGE,   ///< ターン途中のポケモン入れ替え
  BTL_RECFIELD_ROTATE,        ///< ローテーション
}BtlRecFieldType;


static inline u8 MakeRecFieldTag( BtlRecFieldType type, u8 numClient )
{
  GF_ASSERT(numClient < 16);
  GF_ASSERT(type < 16);

  return (type << 4) | (numClient);
}

static inline u8 MakeClientActionTag( u8 clientID, u8 numAction )
{
  GF_ASSERT(clientID < 8);
  GF_ASSERT(numAction < 32);

  return (clientID << 5) | (numAction);
}



enum {
  BTL_REC_SIZE = 4096,
};


struct _BTL_REC {
//  u16  bufSize;
  u16  writePtr;
  u8   fSizeOver;
  u8   dmy;
  u8   buf[ BTL_REC_SIZE ];
};


//=============================================================================================
/**
 * インスタンス生成
 *
 * @param   heapID
 *
 * @retval  BTL_REC*
 */
//=============================================================================================
BTL_REC* BTL_REC_Create( HEAPID heapID )
{
  BTL_REC* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_REC) );
  return wk;
}
//=============================================================================================
/**
 * インスタンス破棄
 *
 * @param   wk
 */
//=============================================================================================
void BTL_REC_Delete( BTL_REC* wk )
{
  GFL_HEAP_FreeMemory( wk );
}

//=============================================================================================
/**
 * データ書き込み
 *
 * @param   wk
 * @param   data
 * @param   size
 */
//=============================================================================================
void BTL_REC_Write( BTL_REC* wk, const void* data, u32 size )
{
  if( wk->fSizeOver == FALSE )
  {
    u32 endPtr = wk->writePtr + size;
    if( endPtr <= sizeof(wk->buf) )
    {
      GFL_STD_MemCopy( data, &wk->buf[wk->writePtr], size );
      wk->writePtr = endPtr;
    }
    else
    {
      wk->fSizeOver = TRUE;
    }
  }
}

//=============================================================================================
/**
 * 最後まで正常に書き込まれているかチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_REC_IsCorrect( const BTL_REC* wk )
{
  return !(wk->fSizeOver);
}









/*===========================================================================================*/
/*                                                                                           */
/* サーバー用                                                                                */
/*                                                                                           */
/* クライアントに送信するデータを生成するためのツール類                                      */
/*                                                                                           */
/*===========================================================================================*/

//=============================================================================================
/**
 * RECTOOL 初期化
 *
 * @param   recTool
 */
//=============================================================================================
void BTL_RECTOOL_Init( BTL_RECTOOL* recTool )
{
  GFL_STD_MemClear( recTool, sizeof(BTL_RECTOOL) );
}

//=============================================================================================
/**
 * RECTOOL アクション選択データを１クライアント分、追加
 *
 * @param   recTool
 * @param   clientID
 * @param   action
 * @param   numAction
 */
//=============================================================================================
void BTL_RECTOOL_PutSelActionData( BTL_RECTOOL* recTool, u8 clientID, const BTL_ACTION_PARAM* action, u8 numAction )
{
  // ヘッダ記述用に 1byte 空けておく
  if( recTool->writePtr == 0 ){
    recTool->type = BTL_RECFIELD_ACTION;
    recTool->writePtr++;
  }

  GF_ASSERT(recTool->type == BTL_RECFIELD_ACTION);

  if( (recTool->clientBit & (1 << clientID)) == 0 )
  {
    u32 endPtr = recTool->writePtr + ((sizeof(BTL_ACTION_PARAM) * numAction) + 1);
    if( endPtr <= sizeof(recTool->buffer) )
    {
      recTool->clientBit |= (1 << clientID);
      recTool->numClients++;

      recTool->buffer[ recTool->writePtr ] = MakeClientActionTag( clientID, numAction );
      GFL_STD_MemCopy( action, &recTool->buffer[recTool->writePtr+1], sizeof(BTL_ACTION_PARAM)*numAction );
      recTool->writePtr = endPtr;
    }
    else
    {
      GF_ASSERT(0);   // データ容量が多すぎる
      recTool->fError = 1;
    }
  }
  else
  {
    GF_ASSERT_MSG(0, "client_%d のデータ書き込みが２度発生している", clientID);
  }
}
//=============================================================================================
/**
 * RECTOOL アクション選択データの書き込み処理を終了
 *
 * @param   recTool
 * @param   dataSize  [out] クライアントに送信するデータサイズが入る
 *
 * @retval  void*   正常終了ならクライアントに送信するデータポインタ／エラー時はNULL
 */
//=============================================================================================
void* BTL_RECTOOL_FixSelActionData( BTL_RECTOOL* recTool, u32* dataSize )
{
  GF_ASSERT(recTool->type == BTL_RECFIELD_ACTION);

  if( recTool->fError == 0 )
  {
    recTool->buffer[0] = MakeRecFieldTag( recTool->type, recTool->numClients );
    *dataSize = recTool->writePtr;
    return recTool->buffer;
  }
  return NULL;
}
