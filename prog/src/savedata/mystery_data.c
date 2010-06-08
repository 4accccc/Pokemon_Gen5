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

#include "savedata/save_outside.h"

FS_EXTERN_OVERLAY(outside_save);

//============================================================================================
//============================================================================================
#ifdef PM_DEBUG
#define DEBUG_MYSTERY_DATA_PRINT_ON //担当者のみのプリントON
#endif //PM_DEBUG


//担当者のみのプリントON
#ifdef DEBUG_MYSTERY_DATA_PRINT_ON
#if defined(DEBUG_ONLY_FOR_toru_nagihashi)
//#define MYSTERY_DATA_Printf(...)  OS_TFPrintf(1,__VA_ARGS__)
#endif  //def
#endif //DEBUG_MYSTERY_DATA_PRINT_ON

#ifndef MYSTERY_DATA_Printf
#define MYSTERY_DATA_Printf(...)  /*    */
#endif

#define MYSTERY_DATA_NO_USED		0x00000000
#define MYSTERY_MENU_FLAG		(MYSTERY_DATA_MAX_EVENT - 1)

//-------------------------------------
/// MYSTERY_ORIGINAL_DATAとOUTSIDE_MYSTERYを共通で使うためのアクセサ
//=====================================
struct _MYSTERY_DATA
{
  void *p_data_adrs;
  MYSTERY_DATA_TYPE type;
  OUTSIDE_SAVE_CONTROL *p_out_sv;
};

//=============================================================================
/**
 *  MYSTERY_DATAのアクセス関数
 *    MYSTERY_DATAはMYSTERY_ORIGINAL_DATAとOUTSIDE_MYSTERYを共通に扱えるようにするため
 *    直接MYSTERY_ORIGINAL_DATAやOUTSIDE_MYSTERYの変数を使わず、
 *    全て関数で取得してから使うようにしてください
 */
//=============================================================================
static void MysteryData_Init( MYSTERY_DATA *p_wk, SAVE_CONTROL_WORK *p_sv, MYSTERYDATA_LOADTYPE type, HEAPID heapID );
static void MysteryData_Exit( MYSTERY_DATA *p_wk );
static MYSTERY_DATA_TYPE MysteryData_GetDataType( const MYSTERY_DATA *cp_wk );
static u32 MysteryData_GetWorkSize( const MYSTERY_DATA *cp_wk );
static u32 MysteryData_GetGiftMax( const MYSTERY_DATA *cp_wk );
static void* MysteryData_GetDataPtr( const MYSTERY_DATA *cp_wk );
static u8* MysteryData_GetRecvFlag( MYSTERY_DATA *p_wk );
static GIFT_PACK_DATA* MysteryData_GetGiftPackData( const MYSTERY_DATA *cp_wk, u32 index );
static RECORD_CRC* MysteryData_GetCrc( MYSTERY_DATA *p_wk );
static void MysteryData_Coded( MYSTERY_DATA *p_wk );
static void MysteryData_Decoded( MYSTERY_DATA *p_wk );

//============================================================================================
//
//		主にセーブシステムから呼ばれる関数
//		ここは管理内セーブで仕様する箇所なので、抽象にしない
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
  return sizeof(MYSTERY_ORIGINAL_DATA);
}
//------------------------------------------------------------------
/**
 * @brief	セーブデータ初期化
 * @param	fd		ふしぎセーブデータへのポインタ
 */
//------------------------------------------------------------------
void MYSTERYDATA_Init(void * fd)
{
  //セーブデータからのみアクセスされるので
  //内部セーブ用に管理データを自前で作成
  MYSTERY_DATA  data;
  GFL_STD_MemClear( &data, sizeof(MYSTERY_DATA) );
  data.p_data_adrs  = fd;
  data.type         = MYSTERY_DATA_TYPE_ORIGINAL;

  GFL_STD_MemClear(fd, sizeof(MYSTERY_ORIGINAL_DATA));

  MysteryData_Coded( &data );
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
  GIFT_PACK_DATA* data;
  
  u32 max = MysteryData_GetGiftMax( fd );

  for(i = 0; i < max; i++){
    data  = MysteryData_GetGiftPackData( fd, i );
    if(!MYSTERYDATA_IsIn(data->gift_type)){
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
  GIFT_PACK_DATA* data  = MysteryData_GetGiftPackData( fd, index );
  u32 max = MysteryData_GetGiftMax( fd );

  GF_ASSERT(index < GIFT_DATA_MAX);

  if(index < GIFT_DATA_MAX){
    if(MYSTERYDATA_IsIn(data->gift_type)){
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
  GIFT_PACK_DATA* src;
  GIFT_PACK_DATA* dst;
  u32 max = MysteryData_GetGiftMax( fd );

  if( !_IsExistsCard( fd, cardindex ) )
  { 
    int i;
    for( i = cardindex; i < max-1; i++ )
    { 
      src = MysteryData_GetGiftPackData( fd, i );
      dst = MysteryData_GetGiftPackData( fd, i+1 );
      *src  = *dst;
    }
    src = MysteryData_GetGiftPackData( fd, i );
    GFL_STD_MemClear( src, sizeof(GIFT_PACK_DATA) );
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
  GIFT_PACK_DATA* data  = MysteryData_GetGiftPackData( fd, index );
  u32 max = MysteryData_GetGiftMax( fd );

  GF_ASSERT(index < max);
  
  if(index < max){
    if(MYSTERYDATA_IsIn(data->gift_type)){
      return data;
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

  u32 max = MysteryData_GetGiftMax( fd );

  GF_ASSERT(index < max);
  
  MysteryData_Decoded(fd);
  px = _GetCardData(fd,index);
  if(px){
    GFL_STD_MemCopy(px, pData, sizeof(GIFT_PACK_DATA));
    bRet=TRUE;
  }
  MysteryData_Coded(fd);
  return bRet;
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
  u32 max = MysteryData_GetGiftMax( fd );

  MysteryData_Decoded(fd);
  // セーブできる領域が無ければセーブ失敗
  if(_CheckCardDataSpace(fd) == TRUE)
  {
    GIFT_PACK_DATA* src;

    // カードをセーブする
    for(i = 0; i < max; i++){
      src = MysteryData_GetGiftPackData( fd, i );

      if(!MYSTERYDATA_IsIn(src->gift_type)){
        GFL_STD_MemCopy(gc, src, sizeof(GIFT_PACK_DATA));
        bRet = TRUE;
        break;
      }
    }
  }
  MysteryData_Coded(fd);
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
  u32 max = MysteryData_GetGiftMax( fd );

  GF_ASSERT(index < max);
  MysteryData_Decoded(fd);
  if(index < max){
    //消す
    GIFT_PACK_DATA* data = MysteryData_GetGiftPackData( fd, index );
    data->gift_type = MYSTERYGIFT_TYPE_NONE;
    //空いた分を詰める
    _FillSpaceCard( fd, index );
  }
  MysteryData_Coded(fd);
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

  MysteryData_Decoded(fd);
  bChk = _CheckCardDataSpace(fd);
  MysteryData_Coded(fd);

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
  MysteryData_Decoded(fd);
  bChk = _IsExistsCard(fd,index);
  MysteryData_Coded(fd);
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
  u32 max = MysteryData_GetGiftMax( fd );

  GF_ASSERT(index < max);

  MysteryData_Decoded(fd);
  if(index < max){
    GIFT_PACK_DATA* data  = MysteryData_GetGiftPackData( fd, index );
    if(data->have){
      bChk = TRUE;
    }
  }
  MysteryData_Coded(fd);
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
  u32 max = MysteryData_GetGiftMax( fd );
  GF_ASSERT(index < max);
  MysteryData_Decoded(fd);

  if(index < max){
    GIFT_PACK_DATA* data  = MysteryData_GetGiftPackData( fd, index );
    data->have = TRUE;
  }
  MysteryData_Coded(fd);
  
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
  u32 max = MysteryData_GetGiftMax( fd );

  MysteryData_Decoded(fd);
  for(i = 0; i < max; i++){
    if(_IsExistsCard(fd, i) == TRUE){
      bRet = TRUE;
    }
  }
  MysteryData_Coded(fd);
  
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
  u8* recv_flag = MysteryData_GetRecvFlag(fd);

  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);

  MysteryData_Decoded(fd);
  if(num < MYSTERY_DATA_MAX_EVENT){
    if(recv_flag[num / 8] & (1 << (num & 7))){
      bRet = TRUE;
    }
  }
  MysteryData_Coded(fd);

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
  u8* recv_flag = MysteryData_GetRecvFlag(fd);
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);

  MysteryData_Decoded(fd);

  if(num < MYSTERY_DATA_MAX_EVENT){
    recv_flag[num / 8] |= (1 << (num & 7));
  }

  MysteryData_Coded(fd);
}

//----------------------------------------------------------------------------
/**
 *	@brief  CRCが正しいかチェックする(複合化してあること)
 *
 *	@param	const DOWNLOAD_GIFT_DATA * fd ふしぎセーブへのポインタ
 *
 *	@return TRUEならば正しい  FALSEならば間違っている
 */
//-----------------------------------------------------------------------------
BOOL MYSTERYDATA_CheckCrc( const DOWNLOAD_GIFT_DATA * fd )
{ 
  MYSTERY_DATA_Printf( "CRCチェック buffer[0x%x]== calc[0x%x]\n", fd->crc, GFL_STD_CrcCalc( fd, sizeof(DOWNLOAD_GIFT_DATA) - 2 ));
  {
    int k,j;
    u8  *p_temp = (u8*)fd;
    for(j=0;j<sizeof(DOWNLOAD_GIFT_DATA);){
      for(k=0;k<16;k++){
          MYSTERY_DATA_Printf("%x ", p_temp[j]);
          j++;
      }
      MYSTERY_DATA_Printf("\n");
    }
  }
  return fd->crc == GFL_STD_CrcCalc( fd, sizeof(DOWNLOAD_GIFT_DATA) - 2 );
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
  MysteryData_Decoded(fd);

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

  MysteryData_Coded(fd);
}
//----------------------------------------------------------------------------
/**
 *	@brief  セーブ開始
 *
 *	@param	MYSTERY_DATA * fd   不思議セーブデータ
 *	@param	*p_gamedata         ゲームデータ
 */
//-----------------------------------------------------------------------------
void MYSTERYDATA_SaveAsyncStart( MYSTERY_DATA * fd, GAMEDATA *p_gamedata )
{ 
  if( MYSTERY_DATA_TYPE_OUTSIDE == MYSTERY_DATA_GetDataType( fd ) )
  { 
    OutsideSave_SaveAsyncInit( fd->p_out_sv );
  }
  else
  {
    GAMEDATA_SaveAsyncStart(p_gamedata);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  セーブメイン処理
 *
 *	@param	MYSTERY_DATA * fd   不思議セーブデータ
 *	@param	*p_gamedata         ゲームデータ
 *
 *	@return セーブ結果
 */
//-----------------------------------------------------------------------------
SAVE_RESULT MYSTERYDATA_SaveAsyncMain( MYSTERY_DATA * fd, GAMEDATA *p_gamedata )
{ 

  if( MYSTERY_DATA_TYPE_OUTSIDE == MYSTERY_DATA_GetDataType( fd ) )
  { 
    BOOL is_ret;
    is_ret  = OutsideSave_SaveAsyncMain( fd->p_out_sv );

    if( is_ret )
    { 
      return SAVE_RESULT_OK;
    }
    else
    { 
      return SAVE_RESULT_CONTINUE;
    }
  }
  else
  {
    return GAMEDATA_SaveAsyncMain(p_gamedata);
  }
}

//------------------------------------------------------------------
/**
 * @brief	  セーブデータから不思議構造体ポインタを返す
 * @param	  sv		セーブコントロールワーク
 * @return	MYSTERY_DATAポインタ
 */
//------------------------------------------------------------------
#if 0
MYSTERY_DATA * SaveData_GetMysteryData(SAVE_CONTROL_WORK * sv)
{
	return SaveControl_DataPtrGet(sv, GMDATA_ID_MYSTERYDATA);
}
#endif


//----------------------------------------------------------------------------
/**
 *	@brief  不思議な贈り物セーブデータを作成
 *
 *	@param	SAVE_CONTROL_WORK *p_sv   セーブコントロールワーク
 *	@param  type                      読み込みタイプ
 *	@param	heapID                    ヒープID
 *
 *	@return MYSTERY_DATA
 */
//-----------------------------------------------------------------------------
MYSTERY_DATA * MYSTERY_DATA_Load( SAVE_CONTROL_WORK *p_sv, MYSTERYDATA_LOADTYPE type, HEAPID heapID )
{ 
  MYSTERY_DATA *p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(MYSTERY_DATA) );
  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_DATA) );

  MysteryData_Init( p_wk, p_sv, type, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  不思議な贈り物セーブデータを破棄
 *
 *	@param	MYSTERY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_DATA_UnLoad( MYSTERY_DATA *p_wk )
{ 
  MysteryData_Exit( p_wk );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  最大保存件数を取得
 *
 *	@param	const MYSTERY_DATA *fd  ワーク
 *
 *	@return 最大保存件数
 */
//-----------------------------------------------------------------------------
u32 MYSTERY_DATA_GetCardMax( const MYSTERY_DATA *fd )
{ 
  return MysteryData_GetGiftMax( fd );
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブデータタイプを取得
 *
 *	@param	const MYSTERY_DATA *fd ワーク
 *
 *	@return MYSTERY_DATA_TYPE列挙を参照
 */
//-----------------------------------------------------------------------------
MYSTERY_DATA_TYPE MYSTERY_DATA_GetDataType( const MYSTERY_DATA *fd )
{ 
  return MysteryData_GetDataType( fd );
}
//=============================================================================
/**
 *  MYSTERY_DATAのデータ取得
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  MYSTERY_DATA初期化
 *
 *	@param	MYSTERY_DATA *p_wk  ワーク
 *	@param	*p_sv               セーブデータ
 *	@param  type                読み込みタイプ
 *	@param  HEAPID              ヒープID
 */
//-----------------------------------------------------------------------------
static void MysteryData_Init( MYSTERY_DATA *p_wk, SAVE_CONTROL_WORK *p_sv, MYSTERYDATA_LOADTYPE type, HEAPID heapID )
{ 
  BOOL is_normal_sv_use;

  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_DATA) );

  //ロードタイプ
  switch( type )
  { 
  case MYSTERYDATA_LOADTYPE_NORMAL:  //通常セーブ
    is_normal_sv_use  = TRUE;
    break;
  case MYSTERYDATA_LOADTYPE_AUTO:    //通常セーブと管理外セーブの自動判別モード
    is_normal_sv_use  = SaveData_GetExistFlag( p_sv);
    break;
  default:
    is_normal_sv_use  = TRUE;
    GF_ASSERT(0)
  }

  //通常セーブデータがあるかどうか
  if( is_normal_sv_use )
  {
    MYSTERY_DATA_Printf( "!!! 不思議な贈り物通常セーブデータで起動 !!!\n" );

    //通常セーブ
    p_wk->p_data_adrs = SaveControl_DataPtrGet(p_sv, GMDATA_ID_MYSTERYDATA);
    p_wk->type        = MYSTERY_DATA_TYPE_ORIGINAL;
  }
  else
  {
    MYSTERY_DATA_Printf( "!!! 不思議な贈り物管理外セーブデータで起動 !!!\n" );

    //管理外セーブ
    GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );

    p_wk->p_out_sv    = OutsideSave_SystemLoad( heapID );
    p_wk->p_data_adrs = OutsideSave_GetMysterPtr( p_wk->p_out_sv );
    p_wk->type        = MYSTERY_DATA_TYPE_OUTSIDE;

    //もしデータがなかったら自前で初期化する
    //(MYSTERYDATA_Initと同じような初期化を管理外セーブにたいして行う)
    if( !OutsideSave_GetExistFlag(p_wk->p_out_sv) )
    { 
      GFL_STD_MemClear( p_wk->p_data_adrs, sizeof(OUTSIDE_MYSTERY) );
      MysteryData_Coded( p_wk );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	MYSTERY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MysteryData_Exit( MYSTERY_DATA *p_wk )
{ 
  switch( p_wk->type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    OutsideSave_SystemUnload(p_wk->p_out_sv);
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );
    break;
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    break;
  }

  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  データの種類を取得
 *
 *	@param	const MYSTERY_DATA *cp_wk   ワーク
 *
 *	@return データの種類
 */
//-----------------------------------------------------------------------------
static MYSTERY_DATA_TYPE MysteryData_GetDataType( const MYSTERY_DATA *cp_wk )
{ 
  return cp_wk->type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークサイズを取得
 *
 *	@param	const MYSTERY_DATA *cp_wk MYSTERY_DATA
 *
 *	@return ワークサイズ
 */
//-----------------------------------------------------------------------------
static u32 MysteryData_GetWorkSize( const MYSTERY_DATA *cp_wk )
{ 
  switch( cp_wk->type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    return sizeof( OUTSIDE_MYSTERY );
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    return sizeof( MYSTERY_ORIGINAL_DATA );
  default:
    GF_ASSERT(0);
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  データの最大値取得
 *
 *	@param	const MYSTERY_DATA *cp_wk   ワーク
 *
 *	@return データの最大値
 */
//-----------------------------------------------------------------------------
static u32 MysteryData_GetGiftMax( const MYSTERY_DATA *cp_wk )
{ 
  switch( cp_wk->type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    return OUTSIDE_MYSTERY_MAX;
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    return GIFT_DATA_MAX;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  実態を取得
 *
 *	@param	const MYSTERY_DATA *cp_wk MYSTERY_DATA
 *
 *	@return 実態
 */
//-----------------------------------------------------------------------------
static void* MysteryData_GetDataPtr( const MYSTERY_DATA *cp_wk )
{
  return cp_wk->p_data_adrs;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レシーブフラグアドレス取得
 *
 *	@param	const MYSTERY_DATA *cp_wk ワーク
 *
 *	@return ワークレシーブフラグ
 */
//-----------------------------------------------------------------------------
static u8* MysteryData_GetRecvFlag( MYSTERY_DATA *p_wk )
{ 
  switch( p_wk->type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    { 
      OUTSIDE_MYSTERY * p_data  = MysteryData_GetDataPtr( p_wk );
      return p_data->recv_flag;
    }
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    { 
      MYSTERY_ORIGINAL_DATA * p_data  = MysteryData_GetDataPtr( p_wk );
      return p_data->recv_flag;
    }
  default:
    GF_ASSERT(0);
    return NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  不思議なおくりものデータを取得
 *
 *	@param	const MYSTERY_DATA *cp_wk ワーク
 *	@param	index インデックス
 *
 *	@return 不思議なおくりものデータ
 */
//-----------------------------------------------------------------------------
static GIFT_PACK_DATA* MysteryData_GetGiftPackData( const MYSTERY_DATA *cp_wk, u32 index )
{ 
  u32 max = MysteryData_GetGiftMax( cp_wk );

  GF_ASSERT( index < max );

  switch( cp_wk->type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    { 
      OUTSIDE_MYSTERY * p_data  = MysteryData_GetDataPtr( cp_wk );
      return &p_data->card[ index ];
    }
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    { 
      MYSTERY_ORIGINAL_DATA * p_data  = MysteryData_GetDataPtr( cp_wk );
      return &p_data->card[ index ];
    }
  default:
    GF_ASSERT(0);
    return NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  CRCの取得
 *
 *	@param	const MYSTERY_DATA *cp_wk   ワーク
 *
 *	@return CRC
 */
//-----------------------------------------------------------------------------
static RECORD_CRC* MysteryData_GetCrc( MYSTERY_DATA *p_wk )
{ 
  switch( p_wk->type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    { 
      OUTSIDE_MYSTERY * p_data  = MysteryData_GetDataPtr( p_wk );
      return &p_data->crc;
    }
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    { 
      MYSTERY_ORIGINAL_DATA * p_data  = MysteryData_GetDataPtr( p_wk );
      return &p_data->crc;
    }
  default:
    GF_ASSERT(0);
    return NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  暗号化
 *
 *	@param	MYSTERY_DATA *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MysteryData_Coded( MYSTERY_DATA *p_wk )
{ 
  void  *p_data = MysteryData_GetDataPtr( p_wk );
  MYSTERY_DATA_TYPE type  = MysteryData_GetDataType( p_wk );

  MYSTERYDATA_Coded( p_data, type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  復号化
 *
 *	@param	MYSTERY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MysteryData_Decoded( MYSTERY_DATA *p_wk )
{ 
  void  *p_data = MysteryData_GetDataPtr( p_wk );
  MYSTERY_DATA_TYPE type  = MysteryData_GetDataType( p_wk );

  MYSTERYDATA_Decoded( p_data, type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  暗号化
 *
 *	@param	void *p_data_adrs データのアドレス
 *	@param	type              データタイプ
 */
//-----------------------------------------------------------------------------
void MYSTERYDATA_Coded( void *p_data_adrs, MYSTERY_DATA_TYPE type )
{ 
  u32   size;
  RECORD_CRC* p_crc;

  switch( type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    size  = sizeof( OUTSIDE_MYSTERY );
    p_crc = &(((OUTSIDE_MYSTERY*)p_data_adrs)->crc);
    break;
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    size  = sizeof( MYSTERY_ORIGINAL_DATA );
    p_crc = &(((MYSTERY_ORIGINAL_DATA*)p_data_adrs)->crc);
    break;
  }

  GF_ASSERT( p_crc->coded_number == 0 );

  p_crc->coded_number = OS_GetVBlankCount() | (OS_GetVBlankCount() << 8);
  if(p_crc->coded_number==0){
    p_crc->coded_number = 1;
  }
  p_crc->crc16ccitt_hash = GFL_STD_CODED_CheckSum( p_data_adrs, size - sizeof(RECORD_CRC));
  GFL_STD_CODED_Coded( p_data_adrs, size - sizeof(RECORD_CRC), 
      p_crc->crc16ccitt_hash + (p_crc->coded_number << 16));
}

//----------------------------------------------------------------------------
/**
 *	@brief  復号化
 *
 *	@param	void *p_data_adrs データのアドレス
 *	@param	type              データタイプ
 */
//-----------------------------------------------------------------------------
void MYSTERYDATA_Decoded( void *p_data_adrs, MYSTERY_DATA_TYPE type )
{
  u32   size;
  RECORD_CRC* p_crc;

  switch( type )
  { 
  case MYSTERY_DATA_TYPE_OUTSIDE:  //管理外セーブ
    size  = sizeof( OUTSIDE_MYSTERY );
    p_crc = &(((OUTSIDE_MYSTERY*)p_data_adrs)->crc);
    break;
  case MYSTERY_DATA_TYPE_ORIGINAL: //通常のセーブ
    size  = sizeof( MYSTERY_ORIGINAL_DATA );
    p_crc = &(((MYSTERY_ORIGINAL_DATA*)p_data_adrs)->crc);
    break;
  }

  GF_ASSERT(p_crc->coded_number!=0);

  GFL_STD_CODED_Decoded( p_data_adrs, size - sizeof(RECORD_CRC), 
    p_crc->crc16ccitt_hash + (p_crc->coded_number << 16));

  p_crc->coded_number = 0;

}
