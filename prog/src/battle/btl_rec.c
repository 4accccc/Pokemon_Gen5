//=============================================================================================
/**
 * @file  btl_rec.c
 * @brief ポケモンWB バトルシステム 録画データ生成
 * @author  taya
 *
 * @date  2009.12.06  作成
 */
//=============================================================================================

#include <gflib.h>
#include "savedata\battle_rec.h"

#include "btl_common.h"
#include "btl_util.h"
#include "btl_rec.h"




static inline u8 MakeRecFieldTag( BtlRecFieldType type, u8 numClient, u8 fChapter )
{
  GF_ASSERT(numClient < 16);
  GF_ASSERT(type < 8);
  GF_ASSERT(fChapter < 2);

  return ((fChapter&1) << 7) | ((type&0x07) << 4) | ((numClient)&0x0f);
}
static inline void ReadRecFieldTag( u8 tagCode, BtlRecFieldType* type, u8* numClient, u8* fChapter )
{
  *numClient = tagCode & 0x0f;
  *type = ((tagCode >> 4) & 0x07);
  *fChapter = ((tagCode>>7)&0x01);
}


static inline u8 MakeClientActionTag( u8 clientID, u8 numAction )
{
  GF_ASSERT(clientID < 8);
  GF_ASSERT(numAction < 32);

  return (clientID << 5) | (numAction);
}
static inline void ReadClientActionTag( u8 tagCode, u8* clientID, u8* numAction )
{
  *clientID  = ((tagCode>>5) & 0x07);
  *numAction = (tagCode & 0x01f);
}

static inline u8 MakeRotationTag( u8 clientID, BtlRotateDir dir )
{
  GF_ASSERT(clientID < 8);
  GF_ASSERT(dir < 32);

  return (clientID << 5) | (dir);
}
static inline void ReadRotationTag( u8 tagCode, u8* clientID, BtlRotateDir* dir )
{
  *clientID  = ((tagCode>>5) & 0x07);
  *dir = (tagCode & 0x01f);
}


/*===========================================================================================*/
/*                                                                                           */
/* クライアント用                                                                            */
/*                                                                                           */
/* 対戦時：録画データの蓄積／再生時：録画データの読み取り                                    */
/*                                                                                           */
/*===========================================================================================*/


enum {
  HEADER_TIMING_CODE = 0,
  HEADER_FIELD_TAG,
  HEADER_SIZE,
};

struct _BTL_REC {
//  u16  bufSize;
  u16  writePtr;
  u8   fSizeOver;
  u8   dmy;
  const void* readData;
  u8   buf[ BTLREC_OPERATION_BUFFER_SIZE ];
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
    const u8* pData = (const u8*)data;
    u32 endPtr;

    // HEADER_TIMING_CODE 部分は不要なのでスキップ
    ++pData;
    --size;

    endPtr = wk->writePtr + size;
    if( endPtr <= sizeof(wk->buf) )
    {
      GFL_STD_MemCopy( pData, &wk->buf[wk->writePtr], size );
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
 * タイミングコード取得
 *
 * @param   data
 *
 * @retval  BtlRecTiming
 */
//=============================================================================================
BtlRecTiming BTL_REC_GetTimingCode( const void* data )
{
  const u8* pByte = (const void*)data;
  return (*pByte);
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

//=============================================================================================
/**
 * 書き込みデータ取得
 *
 * @param   wk
 * @param   size
 *
 * @retval  const void*
 */
//=============================================================================================
const void* BTL_REC_GetDataPtr( const BTL_REC* wk, u32* size )
{
  *size = wk->writePtr;
  return wk->buf;
}

//=============================================================================================
/**
 * 読み込み開始
 *
 * @param   wk
 * @param   recordData
 * @param   dataSize
 */
//=============================================================================================
void BTL_RECREADER_Init( BTL_RECREADER* wk, const void* recordData, u32 dataSize )
{
  u32 i;

  wk->recordData = recordData;
  wk->dataSize = dataSize;
  wk->fError = FALSE;

  for(i=0; i<NELEMS(wk->readPtr); ++i){
    wk->readPtr[i] = 0;
  }
}
//=============================================================================================
/**
 * 読み込み位置をリセット
 *
 * @param   wk
 */
//=============================================================================================
void BTL_RECREADER_Reset( BTL_RECREADER* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->readPtr); ++i){
    wk->readPtr[i] = 0;
  }
}
//=============================================================================================
/**
 * バトル開始チャプターが打たれているかチェック（＆読み込みポインタをその分すすめる）
 *
 * @param   wk
 *
 * @retval  BOOL    打たれていたらTRUE
 */
//=============================================================================================
BOOL BTL_RECREADER_CheckBtlInCmd( BTL_RECREADER* wk, u8 clientID )
{
  BtlRecFieldType type;
  u8 numClient, fChapter;

  u32* rp;

  rp = &wk->readPtr[ clientID ];
  ReadRecFieldTag( wk->recordData[ (*rp) ], &type, &numClient, &fChapter );

  if( type == BTL_RECFIELD_BTLSTART )
  {
    (*rp)++;
    return fChapter;
  }
  return FALSE;
}
//=============================================================================================
/**
 * アクションデータ１件読み込み
 *
 * @param   wk
 * @param   clientID    対象クライアントID
 * @param   numAction   [out] アクションデータ数
 * @param   fChapter    [out] チャプター（区切り）コードが埋め込まれていたらTRUE
 *
 * @retval  const BTL_ACTION_PARAM*   読み込んだアクションデータ先頭
 */
//=============================================================================================
const BTL_ACTION_PARAM* BTL_RECREADER_ReadAction( BTL_RECREADER* wk, u8 clientID, u8 *numAction, u8* fChapter )
{
  BtlRecFieldType type;
  u8 numClient, readClientID, readNumAction;
  u32* rp;
  u32 i;

  if( wk->fError )
  {
    BTL_ACTION_PARAM* actionParam = (BTL_ACTION_PARAM*)(wk->readBuf[ clientID ]);
    BTL_ACTION_SetRecPlayError( actionParam );
    *numAction = 1;
    *fChapter = FALSE;
    return actionParam;
  }

  rp = &wk->readPtr[ clientID ];

  BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_ReadActStart, *rp );
  while( (*rp) < wk->dataSize )
  {
    ReadRecFieldTag( wk->recordData[ (*rp) ], &type, &numClient, fChapter );
    (*rp)++;

    if( (*rp) > wk->dataSize ){
      break;
    }
    if( type == BTL_RECFIELD_ACTION )
    {
      const BTL_ACTION_PARAM* returnPtr = NULL;
      BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_SeekClient, numClient);
      if( numClient==0 || (numClient>BTL_CLIENT_MAX) ){
        break;
      }
      for(i=0; i<numClient; ++i)
      {
        ReadClientActionTag( wk->recordData[ (*rp)++ ], &readClientID, &readNumAction );

        BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_CheckMatchClient, readClientID, clientID, readNumAction );

        if( ((*rp) >= wk->dataSize) ){ break; }
        if( readClientID != clientID )
        {
          (*rp) += (sizeof(BTL_ACTION_PARAM) * readNumAction);
        }
        else
        {
          returnPtr = (const BTL_ACTION_PARAM*)(&wk->recordData[(*rp)]);
          GFL_STD_MemCopy( returnPtr, wk->readBuf[clientID], readNumAction * sizeof(BTL_ACTION_PARAM) );
          returnPtr = (const BTL_ACTION_PARAM*)(wk->readBuf[ clientID ]);
          BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_ReadActParam, (*rp), returnPtr->gen.cmd, returnPtr->fight.waza);
          (*rp) += (sizeof(BTL_ACTION_PARAM) * readNumAction);
          *numAction = readNumAction;
        }
      }
      if( returnPtr ){
        return returnPtr;
      }
    }
    else if( type == BTL_RECFIELD_TIMEOVER )
    {
      BTL_ACTION_PARAM* actionParam = (BTL_ACTION_PARAM*)(wk->readBuf[ clientID ]);

      BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_ReadTimeOverCmd, (*rp) );
      BTL_ACTION_SetRecPlayOver( actionParam );
      *numAction = 1;
      *fChapter = FALSE;
      return actionParam;
    }
    else if( type == BTL_RECFIELD_BTLSTART )
    {
      BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_ReadActSkip, numClient, (*rp) );
    }
    else{
      break;
    }
    if( ((*rp) >= wk->dataSize) ){ break; }
  }


  GF_ASSERT_MSG(0, "不正なデータ読み取り clientID=%d, type=%d, readPtr=%d, datSize=%d",
            clientID, type, (*rp), wk->dataSize);

  wk->fError = TRUE;
  {
    BTL_ACTION_PARAM* actionParam = (BTL_ACTION_PARAM*)(wk->readBuf[ clientID ]);
    BTL_ACTION_SetRecPlayError( actionParam );
    *numAction = 1;
    *fChapter = FALSE;
    return actionParam;
  }
}

//=============================================================================================
/**
 * 記録されているチャプタ数を数える
 *
 * @param   wk
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_RECREADER_GetTurnCount( const BTL_RECREADER* wk )
{
  u32 p = 0;
  u32 turnCount = 0;

  BtlRecFieldType type;
  u8 fChapter, numClient, readClientID, readNumAction;

  while( p < wk->dataSize )
  {
    ReadRecFieldTag( wk->recordData[p++], &type, &numClient, &fChapter );
    if( fChapter ){
      ++turnCount;
    }
    if( (p >= wk->dataSize) ){ break; }
    if( type != BTL_RECFIELD_ACTION )
    {
      p += numClient;
    }
    else
    {
      u32 i;
      for(i=0; i<numClient; ++i)
      {
        ReadClientActionTag( wk->recordData[p++], &readClientID, &readNumAction );
        p += (sizeof(BTL_ACTION_PARAM) * readNumAction);
        if( (p >= wk->dataSize) ){ break; }
      }
    }
  }
  return turnCount;
}
//=============================================================================================
/**
 * 最後まで読み込まれたか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_RECREADER_IsReadComplete( const BTL_RECREADER* wk, u8 clientID )
{
  if( wk->fError == FALSE )
  {
    const u32* rp = &wk->readPtr[ clientID ];
    BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_CheckReadComp, clientID, (*rp), wk->dataSize );
    return ( (*rp) == wk->dataSize );
  }
  return FALSE;
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
 * @param   fChapter    チャプター（ターン先頭）フラグ
 */
//=============================================================================================
void BTL_RECTOOL_Init( BTL_RECTOOL* recTool, BOOL fChapter )
{
  GFL_STD_MemClear( recTool, sizeof(BTL_RECTOOL) );
  recTool->fChapter = fChapter;
}
//=============================================================================================
/**
 * RECTOOL バトル開始チャプタデータを生成
 *
 * @param   recTool
 * @param   dataSize    送信データサイズ
 *
 * @retval  void*   送信データポインタ
 */
//=============================================================================================
void* BTL_RECTOOL_PutBtlInTiming( BTL_RECTOOL* recTool, u32* dataSize, BOOL fChapter )
{
  recTool->buffer[ HEADER_TIMING_CODE ] = BTL_RECTIMING_BtlIn;
  recTool->buffer[ HEADER_FIELD_TAG ]   = MakeRecFieldTag( BTL_RECFIELD_BTLSTART, 0, fChapter );
  *dataSize = HEADER_SIZE;
  return recTool->buffer;
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
  // ヘッダ記述用に空けておく
  if( recTool->writePtr == 0 ){
    recTool->type = BTL_RECFIELD_ACTION;
    recTool->writePtr = HEADER_SIZE;
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
void* BTL_RECTOOL_FixSelActionData( BTL_RECTOOL* recTool, BtlRecTiming timingCode, u32* dataSize )
{
  GF_ASSERT(recTool->type == BTL_RECFIELD_ACTION);

  if( recTool->fError == 0 )
  {
    recTool->buffer[ HEADER_TIMING_CODE ] = timingCode;
    recTool->buffer[ HEADER_FIELD_TAG ]   = MakeRecFieldTag( recTool->type, recTool->numClients, recTool->fChapter );
    *dataSize = recTool->writePtr;
    return recTool->buffer;
  }
  return NULL;
}

//=============================================================================================
/**
 * RECTOOL タイムオーバー通知コードの書き込み処理
 *
 * @param   recTool
 * @param   dataSize
 *
 * @retval  void*
 */
//=============================================================================================
void* BTL_RECTOOL_PutTimeOverData( BTL_RECTOOL* recTool, u32* dataSize )
{
  recTool->buffer[ HEADER_TIMING_CODE ] = BTL_RECTIMING_None;
  recTool->buffer[ HEADER_FIELD_TAG ]   = MakeRecFieldTag( BTL_RECFIELD_TIMEOVER, 0, FALSE );
  *dataSize = HEADER_SIZE;

  BTL_N_PrintfEx( PRINT_CHANNEL_RECTOOL, DBGSTR_REC_TimeOverCmdWrite );
  return recTool->buffer;
}


//=============================================================================================
/**
 * [コマンド整合性チェック用] クライアントが受け取ったアクション選択データを RECTOOL に書き戻す
 *
 * @param   recTool
 * @param   data
 * @param   dataSize
 */
//=============================================================================================
void BTL_RECTOOL_RestoreStart( BTL_RECTOOL* recTool, const void* data, u32 dataSize )
{
  GF_ASSERT(dataSize<sizeof(recTool->buffer));

  GFL_STD_MemCopy( data, recTool->buffer, dataSize );
  recTool->writePtr = dataSize;
  recTool->clientBit = 0;
  recTool->numClients = 0;
  recTool->type = 0;
  recTool->fChapter = 0;
  recTool->fError = 0;
}

//=============================================================================================
/**
 * [コマンド整合性チェック用] クライアントが受け取ったアクション選択データ読み取り
 *
 * @param   recTool
 * @param   rp          [io]
 * @param   clientID    [out]
 * @param   posIdx      [out]
 * @param   action      [out]
 *
 * @retval  BOOL    これ以上読めなくなったらFALSE
 */
//=============================================================================================
BOOL BTL_RECTOOL_RestoreFwd( BTL_RECTOOL* recTool, u32* rp, u8* clientID, u8* numAction, BTL_ACTION_PARAM* action )
{
  if( (*rp) == 0 ){
    (*rp) = HEADER_SIZE;  // ヘッダ分をスキップ
  }

  if( (*rp) < recTool->writePtr )
  {
    u32 dataSize;

    ReadClientActionTag( recTool->buffer[ (*rp) ], clientID, numAction );

    GF_ASSERT((*clientID) < BTL_CLIENT_MAX);
    GF_ASSERT_MSG((*numAction) <= BTL_POSIDX_MAX, "numAction=%d", (*numAction));

    dataSize = sizeof(BTL_ACTION_PARAM) * (*numAction);
    ++(*rp);
    GFL_STD_MemCopy( &(recTool->buffer[(*rp)]), action, dataSize );
    (*rp) += dataSize;


    return TRUE;
  }

  return FALSE;
}
