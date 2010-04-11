//============================================================================================
/**
 * @file	  mystery_data.c
 * @date	  2009.12.05
 * @author	k.ohno
 * @brief	  ふしぎ通信用セーブデータ関連
 */
//============================================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/mystery_data.h"
#include "mystery_data_local.h"

//============================================================================================
//============================================================================================


#define MYSTERY_DATA_NO_USED		0x00000000
#define MYSTERY_MENU_FLAG		(MYSTERY_DATA_MAX_EVENT - 1)


//=============================================================================
/**
 *  プロトタイプ
 */
//=============================================================================

//--------------------------------------------------------------
/**
 * @brief   暗号化
 * @param   rec   
 */
//--------------------------------------------------------------
static void _Coded(MYSTERY_DATA *pMysData)
{
  GF_ASSERT(pMysData->crc.coded_number==0);

  pMysData->crc.coded_number = OS_GetVBlankCount() | (OS_GetVBlankCount() << 8);
  if(pMysData->crc.coded_number==0){
    pMysData->crc.coded_number = 1;
  }
  pMysData->crc.crc16ccitt_hash = GFL_STD_CODED_CheckSum(pMysData, sizeof(MYSTERY_DATA) - sizeof(RECORD_CRC));
  GFL_STD_CODED_Coded(pMysData, sizeof(MYSTERY_DATA) - sizeof(RECORD_CRC), 
    pMysData->crc.crc16ccitt_hash + (pMysData->crc.coded_number << 16));
}

//--------------------------------------------------------------
/**
 * @brief   復号化
 * @param   rec   
 */
//--------------------------------------------------------------
static void _Decoded(MYSTERY_DATA *pMysData)
{
  GF_ASSERT(pMysData->crc.coded_number!=0);

  GFL_STD_CODED_Decoded(pMysData, sizeof(MYSTERY_DATA) - sizeof(RECORD_CRC), 
    pMysData->crc.crc16ccitt_hash + (pMysData->crc.coded_number << 16));

  pMysData->crc.coded_number = 0;
}


//============================================================================================
//
//		主にセーブシステムから呼ばれる関数
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	セーブデータサイズの取得
 * @return	int		ふしぎセーブデータのサイズ
 */
//------------------------------------------------------------------
int MYSTERYDATA_GetWorkSize(void)
{
  return sizeof(MYSTERY_DATA);
}
//------------------------------------------------------------------
/**
 * @brief	セーブデータ初期化
 * @param	fd		ふしぎセーブデータへのポインタ
 */
//------------------------------------------------------------------
void MYSTERYDATA_Init(MYSTERY_DATA * fd)
{
  GFL_STD_MemClear(fd, sizeof(MYSTERY_DATA));

  _Coded( fd );


}

//============================================================================================
//
//		利用するために呼ばれるアクセス関数
//
//============================================================================================


//------------------------------------------------------------------
/**
 * @brief	データが有効かどうか
 * @param	gift_type
 * @return	BOOL =TRUE 有効
 */
//------------------------------------------------------------------

static BOOL MYSTERYDATA_IsIn(u16 gift_type)
{
  if((gift_type > MYSTERYGIFT_TYPE_NONE) &&
     (gift_type < MYSTERYGIFT_TYPE_MAX)){
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータがセーブできるかチェック
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	size		データのサイズ
 * @return	TRUE: 空きがある : FALSE: 空きスロットが無い
 */
//------------------------------------------------------------------
static BOOL _CheckCardDataSpace(MYSTERY_DATA *fd)
{
  int i;
  for(i = 0; i < GIFT_DATA_MAX; i++){
    if(!MYSTERYDATA_IsIn(fd->card[i].gift_type)){
      return TRUE;
    }
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータが存在するか返す
 * @param	fd		ふしぎセーブデータへのポインタ
 * @return	BOOL	TRUEの時、存在する
 */
//------------------------------------------------------------------
static BOOL _IsExistsCard(const MYSTERY_DATA * fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);

  if(index < GIFT_DATA_MAX){
    if(MYSTERYDATA_IsIn(fd->card[index].gift_type)){
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カードデータの空きを詰める
 *
 *	@param	MYSTERY_DATA * fd ふしぎセーブデータへのポインタ
 *	@param  cardindex         空きカードのインデックス
 */
//-----------------------------------------------------------------------------
static void _FillSpaceCard( MYSTERY_DATA * fd, u32 cardindex )
{ 
  if( !_IsExistsCard( fd, cardindex ) )
  { 
    int i;
    for( i = cardindex; i < GIFT_DATA_MAX-1; i++ )
    { 
      fd->card[i] = fd->card[i+1];
    }
    GFL_STD_MemClear( &fd->card[i], sizeof(GIFT_PACK_DATA) );
  }
}



//------------------------------------------------------------------
/**
 * @brief	カードデータの取得 
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		贈り物データのインデックス
 * @param	GIFT_PACK_DATAのポインタ
 * @return	データを入れたらTRUE
 */
//------------------------------------------------------------------
static GIFT_PACK_DATA* _GetCardData(MYSTERY_DATA *fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);
  
  if(index < GIFT_DATA_MAX){
    if(MYSTERYDATA_IsIn(fd->card[index].gift_type)){
      return &fd->card[index];
    }
  }
  return NULL;
}



//------------------------------------------------------------------
/**
 * @brief	カードデータの取得 
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		贈り物データのインデックス
 * @param	GIFT_PACK_DATAのポインタ
 * @return	データを入れたらTRUE
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_GetCardData(MYSTERY_DATA *fd, u32 index,GIFT_PACK_DATA *pData)
{
  BOOL bRet=FALSE;
  GIFT_PACK_DATA* px;
  GF_ASSERT(index < GIFT_DATA_MAX);
  
  _Decoded(fd);
  px = _GetCardData(fd,index);
  if(px){
    GFL_STD_MemCopy(px, pData, sizeof(GIFT_PACK_DATA));
    bRet=TRUE;
  }
  _Coded(fd);
  return bRet;
}

GIFT_PACK_DATA *MYSTERYDATA_GetCardDataOld(MYSTERY_DATA *fd, u32 cardindex)
{
  return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータをセーブデータへ登録    暗号複合済み
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	p		データへのポインタ
 * @return	TRUE: セーブできた : FALSE: 空きスロットが無かった
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_SetCardData(MYSTERY_DATA *fd, const void *p)
{
  int i;
  GIFT_PACK_DATA *gc = (GIFT_PACK_DATA *)p;
  BOOL bRet = FALSE;

  _Decoded(fd);
  // セーブできる領域が無ければセーブ失敗
  if(_CheckCardDataSpace(fd) == TRUE)
  {
    // カードをセーブする
    for(i = 0; i < GIFT_DATA_MAX; i++){
      if(!MYSTERYDATA_IsIn(fd->card[i].gift_type)){
        GFL_STD_MemCopy(gc, &fd->card[i], sizeof(GIFT_PACK_DATA));
        bRet = TRUE;
        break;
      }
    }
  }
  _Coded(fd);
  return bRet;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータを抹消する  暗号複合済み
 * @param	fd		ふしぎセーブデータへのポインタ
 */
//------------------------------------------------------------------
void MYSTERYDATA_RemoveCardData(MYSTERY_DATA *fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);
  _Decoded(fd);
  if(index < GIFT_DATA_MAX){
    //消す
    fd->card[index].gift_type = MYSTERYGIFT_TYPE_NONE;
    //空いた分を詰める
    _FillSpaceCard( fd, index );
  }
  _Coded(fd);
}

//------------------------------------------------------------------
/**
 * @brief	カードデータがセーブできるかチェック  暗号複合済み
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	size		データのサイズ
 * @return	TRUE: 空きがある : FALSE: 空きスロットが無い
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_CheckCardDataSpace(MYSTERY_DATA *fd)
{
  BOOL bChk;

  _Decoded(fd);
  bChk = _CheckCardDataSpace(fd);
  _Coded(fd);

  return bChk;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータが存在するか返す    暗号複合済み
 * @param	fd		ふしぎセーブデータへのポインタ
 * @return	BOOL	TRUEの時、存在する
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsExistsCard(MYSTERY_DATA * fd, u32 index)
{
  BOOL bChk;
  _Decoded(fd);
  bChk = _IsExistsCard(fd,index);
  _Coded(fd);
  return bChk;
}

//------------------------------------------------------------------
/**
 * @brief	 指定のカードからデータを受け取り済みか返す 暗号複合済み
 * @param	 fd		ふしぎセーブデータへのポインタ
 * @param	 index		イベント番号
 * @return	BOOL	TRUEの時、受け取り済み
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsHavePresent(MYSTERY_DATA * fd, u32 index)
{
  BOOL bChk=FALSE;
  GF_ASSERT(index < GIFT_DATA_MAX);

  _Decoded(fd);
  if(index < GIFT_DATA_MAX){
    if(fd->card[index].have){
      bChk = TRUE;
    }
  }
  _Coded(fd);
  return bChk;
}

//------------------------------------------------------------------
/**
 * @brief	 指定のカードからデータを受け取ったことにする 暗号複合済み
 * @param	 fd		ふしぎセーブデータへのポインタ
 * @param	 index		イベント番号
 */
//------------------------------------------------------------------
void MYSTERYDATA_SetHavePresent(MYSTERY_DATA * fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);
  _Decoded(fd);

  if(index < GIFT_DATA_MAX){
    fd->card[index].have = TRUE;
  }
  _Coded(fd);
  
}
//------------------------------------------------------------------
/**
 * @brief	セーブデータ内にカードデータが存在するか返す 暗号複合済み
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsExistsCardAll(MYSTERY_DATA *fd)
{
  int i;
  BOOL bRet=FALSE;

  _Decoded(fd);
  for(i = 0; i < GIFT_DATA_MAX; i++){
    if(_IsExistsCard(fd, i) == TRUE){
      bRet = TRUE;
    }
  }
  _Coded(fd);
  
  return bRet;
}


//------------------------------------------------------------------
/**
 * @brief	指定のイベントはすでにもらったか返す 暗号複合済み
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	BOOL	TRUEの時、すでにもらっている
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsEventRecvFlag(MYSTERY_DATA * fd, u32 num)
{
  BOOL bRet=FALSE;
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);

  _Decoded(fd);
  if(num < MYSTERY_DATA_MAX_EVENT){
    if(fd->recv_flag[num / 8] & (1 << (num & 7))){
      bRet = TRUE;
    }
  }
  _Coded(fd);

  return bRet;
}


//------------------------------------------------------------------
/**
 * @brief	指定のイベントもらったよフラグを立てる 暗号複合済み
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_SetEventRecvFlag(MYSTERY_DATA * fd, u32 num)
{
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);

  _Decoded(fd);

  if(num < MYSTERY_DATA_MAX_EVENT){
    fd->recv_flag[num / 8] |= (1 << (num & 7));
  }

  _Coded(fd);

}

//----------------------------------------------------------------------------
/**
 *	@brief  カードデータを入れ替える 暗号複合済み
 *
 *	@param	MYSTERY_DATA * fd ふしぎセーブデータへのポインタ
 *	@param	cardindex1        入れ替えるカード１
 *	@param	cardindex2        入れ替えるカード２
 */
//-----------------------------------------------------------------------------
void MYSTERYDATA_SwapCard( MYSTERY_DATA * fd, u32 cardindex1, u32 cardindex2 )
{ 
  _Decoded(fd);

  if( _IsExistsCard( fd, cardindex1 )
    && _IsExistsCard( fd, cardindex2 ) )
  { 
    GIFT_PACK_DATA  temp;
    GIFT_PACK_DATA  *p_data1;
    GIFT_PACK_DATA  *p_data2;

    p_data1 = _GetCardData( fd, cardindex1 );
    p_data2 = _GetCardData( fd, cardindex2 );

    GFL_STD_MemCopy( p_data1, &temp, sizeof(GIFT_PACK_DATA) );
    GFL_STD_MemCopy( p_data2, p_data1, sizeof(GIFT_PACK_DATA) );
    GFL_STD_MemCopy( &temp, p_data2, sizeof(GIFT_PACK_DATA) );
  }

  _Coded(fd);

}

//------------------------------------------------------------------
/**
 * @brief	  セーブデータから不思議構造体ポインタを返す
 * @param	  sv		セーブコントロールワーク
 * @return	MYSTERY_DATAポインタ
 */
//------------------------------------------------------------------
MYSTERY_DATA * SaveData_GetMysteryData(SAVE_CONTROL_WORK * sv)
{
	return SaveControl_DataPtrGet(sv, GMDATA_ID_MYSTERYDATA);
}


