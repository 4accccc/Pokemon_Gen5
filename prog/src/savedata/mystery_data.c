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

//============================================================================================
//============================================================================================


#define MYSTERY_DATA_NO_USED		0x00000000
#define MYSTERY_DATA_MAX_EVENT		2048
#define MYSTERY_MENU_FLAG		(MYSTERY_DATA_MAX_EVENT - 1)

//------------------------------------------------------------------
/**
 * @brief	ふしぎデータの定義
 */
//------------------------------------------------------------------
struct MYSTERY_DATA{
  u8 recv_flag[MYSTERY_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  GIFT_PACK_DATA card[GIFT_DATA_MAX];			// カード情報
};

//=============================================================================
/**
 *  プロトタイプ
 */
//=============================================================================
static void MYSTERYDATA_FillSpaceCard( MYSTERY_DATA * fd, u32 cardindex );

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
  GFL_STD_MemClear(fd, sizeof(MYSTERY_DATA));  //セーブデータ全消去隠しコマンドに必要
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
 * @brief	カードデータの取得
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		贈り物データのインデックス
 * @return	GIFT_CARD	カードデータへのポインタ
 */
//------------------------------------------------------------------
GIFT_PACK_DATA *MYSTERYDATA_GetCardData(MYSTERY_DATA *fd, u32 index)
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
 * @brief	カードデータをセーブデータへ登録
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

  // セーブできる領域が無ければセーブ失敗
  if(MYSTERYDATA_CheckCardDataSpace(fd) == FALSE)
  {
    return FALSE;
  }

  // カードをセーブする
  for(i = 0; i < GIFT_DATA_MAX; i++){
    if(!MYSTERYDATA_IsIn(fd->card[i].gift_type)){
      GFL_STD_MemCopy(gc, &fd->card[i], sizeof(GIFT_PACK_DATA));
      bRet = TRUE;
      break;
    }
  }
  return bRet;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータを抹消する
 * @param	fd		ふしぎセーブデータへのポインタ
 */
//------------------------------------------------------------------
void MYSTERYDATA_RemoveCardData(MYSTERY_DATA *fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);
  if(index < GIFT_DATA_MAX){
    //消す
    fd->card[index].gift_type = MYSTERYGIFT_TYPE_NONE;

    //空いた分を詰める
    MYSTERYDATA_FillSpaceCard( fd, index );
  }
}

//------------------------------------------------------------------
/**
 * @brief	カードデータがセーブできるかチェック
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	size		データのサイズ
 * @return	TRUE: 空きがある : FALSE: 空きスロットが無い
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_CheckCardDataSpace(MYSTERY_DATA *fd)
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
BOOL MYSTERYDATA_IsExistsCard(const MYSTERY_DATA * fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);

  if(index < GIFT_DATA_MAX){
    if(MYSTERYDATA_IsIn(fd->card[index].gift_type)){
      return TRUE;
    }
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	 指定のカードからデータを受け取り済みか返す
 * @param	 fd		ふしぎセーブデータへのポインタ
 * @param	 index		イベント番号
 * @return	BOOL	TRUEの時、受け取り済み
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsHavePresent(const MYSTERY_DATA * fd, u32 index)
{
  GF_ASSERT(index < GIFT_DATA_MAX);

  if(index < GIFT_DATA_MAX){
    if(fd->card[index].have){
      return TRUE;
    }
  }
  return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief	セーブデータ内にカードデータが存在するか返す
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsExistsCardAll(const MYSTERY_DATA *fd)
{
  int i;
  for(i = 0; i < GIFT_DATA_MAX; i++){
    if(MYSTERYDATA_IsExistsCard(fd, i) == TRUE){
      return TRUE;
    }
  }
  return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief	指定のイベントはすでにもらったか返す
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	BOOL	TRUEの時、すでにもらっている
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsEventRecvFlag(MYSTERY_DATA * fd, u32 num)
{
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);

  if(num < MYSTERY_DATA_MAX_EVENT){
    if(fd->recv_flag[num / 8] & (1 << (num & 7))){
      return TRUE;
    }
  }
  return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief	指定のイベントもらったよフラグを立てる
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_SetEventRecvFlag(MYSTERY_DATA * fd, u32 num)
{
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);
  if(num < MYSTERY_DATA_MAX_EVENT){
    fd->recv_flag[num / 8] |= (1 << (num & 7));
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カードデータを入れ替える
 *
 *	@param	MYSTERY_DATA * fd ふしぎセーブデータへのポインタ
 *	@param	cardindex1        入れ替えるカード１
 *	@param	cardindex2        入れ替えるカード２
 */
//-----------------------------------------------------------------------------
void MYSTERYDATA_SwapCard( MYSTERY_DATA * fd, u32 cardindex1, u32 cardindex2 )
{ 
  if( MYSTERYDATA_IsExistsCard( fd, cardindex1 )
    && MYSTERYDATA_IsExistsCard( fd, cardindex2 ) )
  { 
    GIFT_PACK_DATA  temp;
    GIFT_PACK_DATA  *p_data1;
    GIFT_PACK_DATA  *p_data2;

    p_data1 = MYSTERYDATA_GetCardData( fd, cardindex1 );
    p_data2 = MYSTERYDATA_GetCardData( fd, cardindex2 );

    GFL_STD_MemCopy( p_data1, &temp, sizeof(GIFT_PACK_DATA) );
    GFL_STD_MemCopy( p_data2, p_data1, sizeof(GIFT_PACK_DATA) );
    GFL_STD_MemCopy( &temp, p_data2, sizeof(GIFT_PACK_DATA) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カードデータの空きを詰める
 *
 *	@param	MYSTERY_DATA * fd ふしぎセーブデータへのポインタ
 *	@param  cardindex         空きカードのインデックス
 */
//-----------------------------------------------------------------------------
static void MYSTERYDATA_FillSpaceCard( MYSTERY_DATA * fd, u32 cardindex )
{ 
  if( !MYSTERYDATA_IsExistsCard( fd, cardindex ) )
  { 
    int i;
    for( i = cardindex; i < GIFT_DATA_MAX-1; i++ )
    { 
      fd->card[i] = fd->card[i+1];
    }
    GFL_STD_MemClear( &fd->card[i], sizeof(GIFT_PACK_DATA) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  不正を見つけたら修正する関数
 *
 *	@param	DOWNLOAD_GIFT_DATA *p_data  修正するデータ
 *
 *	@return 不正カウンタ（不正無しなら０  内部の不正を見つけるたびに＋１）
 */
//-----------------------------------------------------------------------------
u32 MYSTERYDATA_ModifyDownloadData( DOWNLOAD_GIFT_DATA *p_data )
{ 
  u32 dirty = 0;

  //説明テキストが不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    for( i = 0; i < GIFT_DATA_CARD_TEXT_MAX+EOM_SIZE; i++ )
    { 
      if( p_data->event_text[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
      }
    }

    if( is_darty )
    { 
      dirty++;
      p_data->event_text[0] = L'　';
      p_data->event_text[1] = GFL_STR_GetEOMCode();
      OS_TPrintf( "DOWNLOAD_GIFT_DATA:説明テキストが不正のため空文字にしました\n" );
    }
  }

  //付属データもチェック
  dirty += MYSTERYDATA_ModifyGiftData( &p_data->data );

  return dirty;
}
//----------------------------------------------------------------------------
/**
 *	@brief  不正を見つけたら修正する関数
 *
 *	@param	GIFT_PACK_DATA *p_data  修正するデータ
 *
 *	@return 不正カウンタ（不正無しなら０  内部の不正を見つけるたびに＋１）
 */
//-----------------------------------------------------------------------------
u32 MYSTERYDATA_ModifyGiftData( GIFT_PACK_DATA *p_data )
{ 
  u32 dirty = 0;

  //イベントタイトルが不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    for( i = 0; i < GIFT_DATA_CARD_TITLE_MAX+EOM_SIZE; i++ )
    { 
      if( p_data->event_name[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
      }
    }

    if( is_darty )
    { 
      dirty++;
      p_data->event_name[0] = L'　';
      p_data->event_name[1] = GFL_STR_GetEOMCode();
      OS_TPrintf( "GIFT_PACK_DATA:イベントタイトルが不正のため空文字にしました\n" );
    }
  }

  //イベントID
  { 
    if( !(0 <= p_data->event_id && p_data->event_id < MYSTERY_DATA_MAX_EVENT ) )
    { 
      p_data->event_id  = 0;
      dirty++;
      OS_TPrintf( "GIFT_PACK_DATA:イベントIDが不正のため０にしました\n" );
    }
  }

  return dirty;
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


