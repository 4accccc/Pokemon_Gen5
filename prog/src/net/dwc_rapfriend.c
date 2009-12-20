//=============================================================================
/**
 * @file	dwc_rapfriend.c
 * @bfief	DWCラッパー。常駐モジュールに置くもの
 * @author	kazuki yoshihara  -> k.ohno改造
 * @date	06/02/23
 */
//=============================================================================

#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"

#if GFL_NET_WIFI

#include "dwc_rapfriend.h"
#include "savedata/wifilist.h"




/**
 * @brief    すでに同じ人が登録されていないか。
 * @param[in]   data 検査するフレンドコード
 * @param[in]   myUserData 自分のDWCUserData
 * @param[in]   keyList    ともだちリストのフレンドコード
 * @retval  DWCFRIEND_INLIST …   すでに、同じデータがリスト上にある。この場合は何もする必要なし。
 * @retval  DWCFRIEND_OVERWRITE …すでに、同じデータがリスト上にあるが、上書きすることが望ましい場合。
 * @retval  DWCFRIEND_NODATA …   別のデータ
 */
static int _dwcfriend_CheckFriendByToken(DWCFriendData *data, DWCUserData *myUserData,DWCFriendData *matchFriend)
{
  // すでに同じ人が登録していないか検索

  if( DWC_IsEqualFriendData( data, matchFriend ) )
  {    // 全く同じデータが発見。
    return DWCFRIEND_INLIST;
  }
  else if( DWC_GetGsProfileId(myUserData,data) > 0 && DWC_GetGsProfileId(myUserData,data) == DWC_GetGsProfileId(myUserData,matchFriend) )
  {    // フレンドコードで登録されているデータがある。 この場合は、上書きしてもらうことを推奨。
    return DWCFRIEND_OVERWRITE;
  }
  return DWCFRIEND_NODATA;
}

//==============================================================================
/**
 * すでに同じ人が登録されていないか。
 * @param   index 同じデータが見つかったときの、その場所。
 * @param         見つからない場合は空いているところを返す
 * @param         どこも空いていない場合は、-1
 * @retval  DWCFRIEND_INLIST … すでに、同じデータがリスト上にある。この場合は何もする必要なし。
 * @retval  DWCFRIEND_OVERWRITE … すでに、同じデータがリスト上にあるが、上書きすることが望ましい場合。
 * @retval  DWCFRIEND_NODATA … 同じデータはリスト上にない。
 * @retval  DWCFRIEND_INVALID … 受け取ったDWCFriendDataが異常。
 */
//==============================================================================
int GFL_NET_DWC_CheckFriendByToken(DWCFriendData *data, int *index)
{
  // すでに同じ人が登録していないか検索
  int i;
  DWCUserData *myUserData = GFI_NET_GetMyDWCUserData();
  DWCFriendData *keyList  = GFI_NET_GetMyDWCFriendData();
  int numMax  = GFI_NET_GetFriendNumMax();
  *index = -1;

  GF_ASSERT(myUserData);

  if( !DWC_IsValidFriendData( data ) ) return DWCFRIEND_INVALID;

  for( i = 0; i < numMax; i++ )
  {

    if( DWC_IsEqualFriendData( data, keyList + i ) )
    {
      // 全く同じデータが発見。
      *index = i;
      return DWCFRIEND_INLIST;
    }
    else if( DWC_GetGsProfileId(myUserData,data) > 0 && DWC_GetGsProfileId(myUserData,data) == DWC_GetGsProfileId(myUserData,keyList + i) )
    {
      // フレンドコードで登録されているデータがある。
      // この場合は、上書きしてもらうことを推奨。
      *index = i;
      return DWCFRIEND_OVERWRITE;
    }
    else if( *index < 0 && !DWC_IsValidFriendData( keyList + i ) )
    {
      *index = i;
    }
  }

  // リストになし。
  return DWCFRIEND_NODATA;
}

//==============================================================================
/**
 * すでに同じ人が登録されていないか。
 * @param  WIFI_LISTのポインタ
 * @param   index 同じデータが見つかったときの、その場所。
 * @param         見つからない場合は空いているところを返す
 * @param         どこも空いていない場合は、-1
 * @retval  DWCFRIEND_INLIST … すでに、同じデータがリスト上にある。
 * @retval  DWCFRIEND_NODATA … 同じデータはリスト上にない。
 * @retval  DWCFRIEND_INVALID … 受け取ったfriend_keyaが正しくない。
 */
//==============================================================================
int GFL_NET_DWC_CheckFriendCodeByToken( WIFI_LIST* list,u64 friend_key,int *index)
{
  // すでに同じ人が登録していないか検索
  int i;
  DWCUserData *myUserData = WifiList_GetMyUserInfo(list);
  DWCFriendData *keyList  = WifiList_GetDwcDataPtr(list,0);
  int numMax  = WifiList_GetFriendDataNum(list);
  DWCFriendData token;

  // 友達登録鍵が正しいかどうか判定
  if( !DWC_CheckFriendKey( myUserData, friend_key ) ) return DWCFRIEND_INVALID;
  DWC_CreateFriendKeyToken( &token, friend_key );
  if( DWC_GetGsProfileId(myUserData, &token) <= 0 ) return DWCFRIEND_INVALID;

  *index = -1;
  for( i = 0; i < numMax; i++ )
  {
    if( DWC_GetGsProfileId(myUserData, &token) == DWC_GetGsProfileId(myUserData, keyList + i) )
    {
      // 登録されているデータがある。
      *index = i;
      return DWCFRIEND_INLIST;
    }
    else if( *index < 0 && !DWC_IsValidFriendData( keyList + i ) )
    {
      *index = i;
    }
  }

  // リストになし。
  return DWCFRIEND_NODATA;
}

//------------------------------------------------------------------
/**
 * $brief   接続している人のフレンドコード検査  登録する人が見つかったら登録
            WIFINOTE_MODE_AUTOINPUT_CHECK
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

int GFL_NET_DWC_FriendAutoInputCheck( DWCFriendData* pFriend )
{
  int i,hit = FALSE,pos,ret;
  DWCFriendData *keyList  = GFI_NET_GetMyDWCFriendData();

  GF_ASSERT(keyList);
  GF_ASSERT(pFriend);

  ret = GFL_NET_DWC_CheckFriendByToken(pFriend, &pos);

  NET_PRINT("%d check  \n",ret);
  GF_ASSERT(ret != DWCFRIEND_INVALID);  // 受け取りデータに異常がある

  if(ret == DWCFRIEND_INLIST){ //同一
  }
  else if(ret == DWCFRIEND_OVERWRITE){ //上書き
    MI_CpuCopy8(pFriend, &keyList[pos], sizeof(DWCFriendData));
  }
  else if(ret == DWCFRIEND_NODATA){  // 新規データ
    hit = TRUE;
  }
  return  hit;
}

//------------------------------------------------------------------
/**
 * $brief   無線用登録関数
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
void GFL_NET_DWC_FriendDataWrite(GAMEDATA* pGameData, MYSTATUS* pMyStatus,DWCFriendData* pFriend, int addListIndex, int heapID, int overWrite)
{
  WIFI_LIST* pList = GAMEDATA_GetWiFiList(pGameData);
  DWCFriendData *keyList  = WifiList_GetDwcDataPtr(pList, addListIndex);
  STRBUF* pBuf;

  if(overWrite != 2){
    //        pFriend = CommInfoGetDWCFriendCode(netID);
    MI_CpuCopy8(pFriend, keyList, sizeof(DWCFriendData));
  }
  if(overWrite == 0){  // 上書き時には名前を書き換えない
    pBuf = MyStatus_CreateNameString(pMyStatus, heapID);
    WifiList_SetFriendName(pList, addListIndex, pBuf);
    GFL_STR_DeleteBuffer(pBuf);
    WifiList_SetFriendInfo(pList, addListIndex, WIFILIST_FRIEND_SEX, MyStatus_GetMySex(pMyStatus));
    WifiList_SetFriendInfo(pList, addListIndex, WIFILIST_FRIEND_ID, MyStatus_GetID(pMyStatus));
  }
  else if(overWrite == 1){
    // 上書きだけど性別がまだ無い状態の場合書く
    if(WifiList_GetFriendInfo(pList,addListIndex,WIFILIST_FRIEND_SEX) == PM_NEUTRAL){
      WifiList_SetFriendInfo(pList, addListIndex, WIFILIST_FRIEND_SEX, MyStatus_GetMySex(pMyStatus));
      WifiList_SetFriendInfo(pList, addListIndex, WIFILIST_FRIEND_ID, MyStatus_GetID(pMyStatus));
    }
  }
  pBuf =  GFL_STR_CreateBuffer(120, heapID );
  GFL_STR_DeleteBuffer(pBuf);
  WifiList_SetFriendInfo(pList, addListIndex, WIFILIST_FRIEND_UNION_GRA, MyStatus_GetTrainerView(pMyStatus));

}

//------------------------------------------------------------------
/**
 * $brief   無線用ともだち追加関数
 * @param   SAVE_CONTROL_WORK
 * @param   MYSTATUS
 * @param   DWCFriendData
 * @param   heapID
 * @retval  int
 */
//------------------------------------------------------------------

void GFL_NET_DWC_FriendDataAdd(GAMEDATA* pGameData, MYSTATUS* pMyStatus,DWCFriendData* pFriend, int heapID)
{
  WIFI_LIST* pList = GAMEDATA_GetWiFiList(pGameData);
  int i,ret;

  if( !DWC_IsValidFriendData( pFriend ) ){
    return;
  }

  for(i = 0; i < WIFILIST_FRIEND_MAX;i++){
    DWCFriendData* pFL = WifiList_GetDwcDataPtr( pList, i );
    if( !WifiList_IsFriendData( pList, i ) ){
      GFL_NET_DWC_FriendDataWrite(pGameData, pMyStatus, pFriend, i, heapID, 0);
      break;
    }
    ret = _dwcfriend_CheckFriendByToken(pFriend,WifiList_GetMyUserInfo(pList),pFL);
    if(ret == DWCFRIEND_INLIST){ //同一
      break;
    }
    else if(ret == DWCFRIEND_OVERWRITE){ //上書き
      GFL_NET_DWC_FriendDataWrite(pGameData, pMyStatus, pFriend, i, heapID, 1);
      break;
    }
  }
}


#endif //GFL_NET_WIFI
