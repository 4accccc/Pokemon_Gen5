//============================================================================================
/**
 * @file	mystery_data.c
 * @date	2006.04.28
 * @author	tamada / mitsuhara
 * @brief	ふしぎ通信用セーブデータ関連
 */
//============================================================================================
#include <gflib.h>
#include "savedata/mystery_data.h"

//============================================================================================
//============================================================================================

extern MYSTERY_DATA * SaveData_GetMysteryData(SAVE_CONTROL_WORK * sv);

#define UNIQ_TYPE			0xEDB88320L
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
  GIFT_DELIVERY delivery[GIFT_DELIVERY_MAX];		// 配達員８つ
  GIFT_CARD card[GIFT_CARD_MAX];			// カード情報３つ
};


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
#ifdef DEBUG_ONLY_FOR_mituhara
  OS_TPrintf("ふしぎデータ初期化\n");
  // この処理はsaveload_system.cのSVDT_Initで行われているので何もしない
  MI_CpuClearFast(fd, sizeof(MYSTERY_DATA));
#endif


  OS_TPrintf("GIFT_DELIVERY %d\n",sizeof(GIFT_DELIVERY));
  OS_TPrintf("GIFT_CARD %d\n",sizeof(GIFT_CARD));

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
 * @brief	配達員データの取得
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		配達員データのインデックス（０オリジン）
 * @return	GIFT_DATA	配達員データへのポインタ
 */
//------------------------------------------------------------------
GIFT_DELIVERY * MYSTERYDATA_GetDeliData(MYSTERY_DATA * fd, int index)
{
    if((index >= 0) && (index < GIFT_DELIVERY_MAX)){
        if(MYSTERYDATA_IsIn(fd->delivery[index].gift_type)){
            // データが有効なのでポインタを返す
            return &fd->delivery[index];
        }
    }
    return (GIFT_DELIVERY *)NULL;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータの取得
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		贈り物データのインデックス（０オリジン）
 * @return	GIFT_CARD	カードデータへのポインタ
 */
//------------------------------------------------------------------
GIFT_CARD *MYSTERYDATA_GetCardData(MYSTERY_DATA *fd, int index)
{
    if((index >= 0) && (index < GIFT_CARD_MAX)){
        if(MYSTERYDATA_IsIn(fd->card[index].gift_type)){
            return &fd->card[index];
        }
    }
    return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	配達員データをセーブデータへ登録
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	p		データへのポインタ
 * @return	TRUE: セーブできた : FALSE: 空きスロットが無かった
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_SetDeliData(MYSTERY_DATA *fd, const void *p, int link)
{
  int i;
    BOOL bRet=FALSE;

  // セーブできる領域が無ければセーブ失敗
  if(MYSTERYDATA_CheckDeliDataSpace(fd) == FALSE)	return FALSE;

  // ↓これ以降は容量的にはセーブに成功するはず
  
  for(i = 0; i < GIFT_DELIVERY_MAX; i++){
    if(!MYSTERYDATA_IsIn(fd->delivery[i].gift_type)){
      MI_CpuCopy8(p, &fd->delivery[i], sizeof(GIFT_DELIVERY));
      fd->delivery[i].link = link;
      bRet = TRUE;
        break;
    }
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
  return bRet;
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
    GIFT_CARD *gc = (GIFT_CARD *)p;
    BOOL bRet = FALSE;

    // セーブできる領域が無ければセーブ失敗
    if(MYSTERYDATA_CheckCardDataSpace(fd) == FALSE)	return FALSE;
    // 配達員を含むデータの場合は配達員側もチェック
    if(gc->beacon.delivery_flag == TRUE &&
       MYSTERYDATA_CheckDeliDataSpace(fd) == FALSE)	return FALSE;

  // ↓これ以降は容量的にはセーブに成功するはず
  
  // カードをセーブする
    for(i = 0; i < GIFT_CARD_MAX; i++){
        if(!MYSTERYDATA_IsIn(fd->card[i].gift_type)){
#ifdef DEBUG_ONLY_FOR_mituhara
            OS_TPrintf("カードをセーブしました [%d]\n", i);
#endif
            MI_CpuCopy8(gc, &fd->card[i], sizeof(GIFT_CARD));

      // 配達員をセーブする
            if(gc->beacon.delivery_flag == TRUE){
                MYSTERYDATA_SetDeliData(fd, (const void *)&gc->gift_type, i);
            }
            bRet = TRUE;
            break;
        }
    }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
    return bRet;
}

//------------------------------------------------------------------
/**
 * @brief	配達員データを抹消する
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		配達員データのインデックス
 * @return	TRUE: セーブできた : FALSE: 空きスロットが無かった
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_RemoveDeliData(MYSTERY_DATA *fd, int index)
{
    GF_ASSERT(index < GIFT_DELIVERY_MAX);
    fd->delivery[index].gift_type = MYSTERYGIFT_TYPE_NONE;
    fd->delivery[index].link = 0;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
    return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータを抹消する 配達員も  BITもおとす
 * @param	fd		ふしぎセーブデータへのポインタ
 * @return	TRUE: セーブできた : FALSE: 空きスロットが無かった
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_RemoveCardDataPlusBit(MYSTERY_DATA *fd, int index)
{
    GF_ASSERT(index < GIFT_CARD_MAX);
    fd->card[index].gift_type = MYSTERYGIFT_TYPE_NONE;
    //BITもおとす
    MYSTERYDATA_ResetEventRecvFlag(fd, fd->card[index].beacon.event_id);
    // リンクされているカードも一緒に抹消
    MYSTERYDATA_RemoveCardLinkDeli(fd, index);

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
    return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	カードデータを抹消する
 * @param	fd		ふしぎセーブデータへのポインタ
 * @return	TRUE: セーブできた : FALSE: 空きスロットが無かった
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_RemoveCardData(MYSTERY_DATA *fd, int index)
{
    GF_ASSERT(index < GIFT_CARD_MAX);
    fd->card[index].gift_type = MYSTERYGIFT_TYPE_NONE;

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
    return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	配達員データがセーブできるかチェック
 * @param	fd		ふしぎセーブデータへのポインタ
 * @return	TRUE: 空きがある : FALSE: 空きスロットが無い
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_CheckDeliDataSpace(MYSTERY_DATA *fd)
{
    int i;
    for(i = 0; i < GIFT_DELIVERY_MAX; i++){
        if(!MYSTERYDATA_IsIn(fd->delivery[i].gift_type)){
            return TRUE;
        }
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
BOOL MYSTERYDATA_CheckCardDataSpace(MYSTERY_DATA *fd)
{
  int i;
  for(i = 0; i < GIFT_CARD_MAX; i++){
      if(!MYSTERYDATA_IsIn(fd->card[i].gift_type)){
          return TRUE;
      }
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	贈り物データの存在チェック
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	index		贈り物データのインデックス（０オリジン）
 * @return	BOOL	TRUEの時、存在する
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsExistsDelivery(const MYSTERY_DATA * fd, int index)
{
    GF_ASSERT(index < GIFT_DELIVERY_MAX);
  
    if(MYSTERYDATA_IsIn(fd->delivery[index].gift_type)){
        return TRUE;
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
BOOL MYSTERYDATA_IsExistsCard(const MYSTERY_DATA * fd, int index)
{
    GF_ASSERT(index < GIFT_CARD_MAX);

    if(MYSTERYDATA_IsIn(fd->card[index].gift_type)){
        return TRUE;
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
  for(i = 0; i < GIFT_CARD_MAX; i++)
    if(MYSTERYDATA_IsExistsCard(fd, i) == TRUE)
      return TRUE;
  return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief	指定のカードにリンクされている配達員が存在するか
 * @param	※indexは0～2が有効
 * @return	TRUE: 配達員は存在する FALSE: 存在しない
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_GetCardLinkDeli(const MYSTERY_DATA *fd, int index)
{
    int i;

    for(i = 0; i < GIFT_DELIVERY_MAX; i++){
        if(MYSTERYDATA_IsIn(fd->delivery[i].gift_type)){
            if(fd->delivery[i].link == index){
                return TRUE;
            }
        }
    }
    return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief	指定カードにリンクされている配達員を削除
 * @param	※indexは0～2が有効(無ければ何もしない)
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_RemoveCardLinkDeli(const MYSTERY_DATA *fd, int index)
{
    int i;

    for(i = 0; i < GIFT_DELIVERY_MAX; i++){
        if(MYSTERYDATA_IsIn(fd->delivery[i].gift_type)){
            if(fd->delivery[i].link == index){
#if (DEBUG_ONLY_FOR_mituhara | DEBUG_ONLY_FOR_ohno)
                OS_TPrintf("カードと一緒に %d 番のおくりものも消しました\n", i );
#endif
                MYSTERYDATA_RemoveDeliData((MYSTERY_DATA *)fd, i);
                return;
            }
        }
    }
}

//------------------------------------------------------------------
/**
 * @brief	指定のイベントはすでにもらったか返す
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	BOOL	TRUEの時、すでにもらっている
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsEventRecvFlag(MYSTERY_DATA * fd, int num)
{
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);
  return !!(fd->recv_flag[num / 8] & (1 << (num & 7)));
}


//------------------------------------------------------------------
/**
 * @brief	指定のイベントもらったよフラグを立てる
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_SetEventRecvFlag(MYSTERY_DATA * fd, int num)
{
  GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);
  fd->recv_flag[num / 8] |= (1 << (num & 7));
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
}

//------------------------------------------------------------------
/**
 * @brief	指定のイベントもらったよフラグを落す カードと配達員を一緒に消す時だけ使用する
 * @param	fd		ふしぎセーブデータへのポインタ
 * @param	num		イベント番号
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_ResetEventRecvFlag(MYSTERY_DATA * fd, int num)
{
    u8 notbit = (u8)~(1 << (num & 7));
    GF_ASSERT(num < MYSTERY_DATA_MAX_EVENT);
    
#if (DEBUG_ONLY_FOR_mituhara | DEBUG_ONLY_FOR_ohno)
    OS_TPrintf("MYSTERYDATA_ResetEventRecvFlag %x ",  fd->recv_flag[num / 8]);
#endif
  fd->recv_flag[num / 8] &= notbit;
#if (DEBUG_ONLY_FOR_mituhara | DEBUG_ONLY_FOR_ohno)
    OS_TPrintf(" %x \n",  fd->recv_flag[num / 8]);
#endif
    
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
}

//------------------------------------------------------------------
/**
 * @brief	ふしぎなおくりものを表示出来るか？
 * @param	NONE
 * @return	TRUE: 表示　FALSE: 表示しない
 */
//------------------------------------------------------------------
BOOL MYSTERYDATA_IsMysteryMenu(MYSTERY_DATA *fd)
{
  return MYSTERYDATA_IsEventRecvFlag(fd, MYSTERY_MENU_FLAG);
}


//------------------------------------------------------------------
/**
 * @brief	ふしぎなおくりものの表示フラグをONにする
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_SetMysteryMenu(MYSTERY_DATA *fd)
{
  MYSTERYDATA_SetEventRecvFlag(fd, MYSTERY_MENU_FLAG);
}


//============================================================================================
//
//		さらに上位の関数群　主にこちらを呼んでください
//
//============================================================================================

static MYSTERY_DATA *_mystery_ptr = NULL;

//------------------------------------------------------------------
/**
 * @brief	これ以下の関数を使うために必要な初期化
 * @param	sv		セーブデータ構造へのポインタ
 * @param	heap_id		ワークを取得するヒープのID
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_InitSlot(SAVE_CONTROL_WORK * sv, int heap_id)
{
  LOAD_RESULT result;

  if(_mystery_ptr == NULL){
    _mystery_ptr = SaveData_GetMysteryData(sv);
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_MYSTERYDATA)
	SVLD_SetCrc(GMDATA_ID_MYSTERYDATA);
#endif //CRC_LOADCHECK
}

//------------------------------------------------------------------
/**
 * @brief	これ以下の関数を使い終わった後の後始末
 * @param	sv		セーブデータ構造へのポインタ
 * @param	flag		TRUE: セーブする / FALSE: セーブしない
 * @return	NONE
 */
//------------------------------------------------------------------
void MYSTERYDATA_FinishSlot(SAVE_CONTROL_WORK * sv, int flag)
{
  SAVE_RESULT result;
  if(_mystery_ptr){
#if 0
    if(flag == TRUE)
      SaveData_Save(sv);
#endif
    _mystery_ptr = NULL;
  }
}
     
//------------------------------------------------------------------
/**
 * @brief	スロットにデータがあるか返す関数
 * @param	NONE
 * @return	-1..データなし: 0以上 データindex番号
*/
//------------------------------------------------------------------
int MYSTERYDATA_GetSlotData(void)
{
    int i;
    for(i = 0; i < GIFT_DELIVERY_MAX; i++){
        if(MYSTERYDATA_IsExistsDelivery(_mystery_ptr, i) == TRUE)
            return i;
    }
    return -1;
}

//------------------------------------------------------------------
/**
 * @brief	スロットにデータがあるか返す関数
 * @param	NONE
 * @return	FALSE..データなし: TRUE データあり
*/
//------------------------------------------------------------------
BOOL MYSTERYDATA_CheckSlotData(void)
{
    if(MYSTERYDATA_GetSlotData() == -1){
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	指定番号のスロットデータのタイプを返す
 * @param	index		スロットのインデックス番号
 * @return	int		MYSTERYGIFT_TYPE_xxxxx
 *
 * ※インデックス番号はMYSTERYDATA_GetSlotDataで返された値
*/
//------------------------------------------------------------------
int MYSTERYDATA_GetSlotType(int index)
{
    GIFT_DELIVERY *dv;

    dv = MYSTERYDATA_GetDeliData(_mystery_ptr, index);
    if(dv){
        return (int)dv->gift_type;
    }
    return MYSTERYGIFT_TYPE_NONE;
}


//------------------------------------------------------------------
/**
 * @brief	指定番号のスロット構造体へのポインタを返す
 * @param	index		スロットのインデックス番号
 * @return	GIFT_PRESENT	構造体へのポインタ
 *
 * ※インデックス番号はMYSTERYDATA_GetSlotDataで返された値
 */
//------------------------------------------------------------------
GIFT_PRESENT *MYSTERYDATA_GetSlotPtr(int index)
{
    GIFT_DELIVERY *dv;

    dv = MYSTERYDATA_GetDeliData(_mystery_ptr, index);
    if(dv){
        return &dv->data;
    }
    return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	指定のスロットを消去する
 * @param	index		スロットのインデックス番号
 * @return	NONE
 *
 * ※インデックス番号はMYSTERYDATA_GetSlotDataで返された値
 */
//------------------------------------------------------------------
void MYSTERYDATA_RemoveSlot(int index)
{
  MYSTERYDATA_RemoveDeliData(_mystery_ptr, index);
}

//------------------------------------------------------------------


