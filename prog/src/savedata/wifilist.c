//============================================================================================
/**
 * @file	wifilist.c
 * @brief	WIFIフレンドコード管理  > ともだち手帳のデータ管理
 * @author	mori san   >  k.ohno
 * @date	2006.03.27 くらい
 */
//============================================================================================

//#include "savedata/savedata_def.h"	//SAVE_CONTROL_WORK参照のため

#include "gflib.h"
#include <dwc.h>

#include "system/rtc_tool.h"
#include "savedata/save_tbl.h"
#include "savedata/wifilist.h"
#include "wifilist_local.h"

FS_EXTERN_OVERLAY(dev_wifi);
FS_EXTERN_OVERLAY(dev_wifilib);

//-----------------------------------------------------------------==============
//-----------------------------------------------------------------==============
///前方宣言
WIFI_LIST* SaveData_GetWifiListData(SAVE_CONTROL_WORK* pSave);

//-----------------------------------------------------------------==============
//
//	セーブデータシステムが依存する関数
//
//-----------------------------------------------------------------==============
//----------------------------------------------------------
/**
 * @brief	自分状態保持ワークのサイズ取得
 * @return	int		サイズ（バイト単位）
 */
//----------------------------------------------------------
int WifiList_GetWorkSize(void)
{
  return sizeof(WIFI_LIST);
}

//----------------------------------------------------------
/**
 * @brief	自分状態保持ワークの確保
 * @param	heapID		メモリ確保をおこなうヒープ指定
 * @return	WIFI_LIST	取得したワークへのポインタ
 */
//----------------------------------------------------------
WIFI_LIST * WifiList_AllocWork(u32 heapID)
{
  int i;
  WIFI_LIST * list;
  list = GFL_HEAP_AllocClearMemory(heapID, sizeof(WIFI_LIST));

  WifiList_Init( list );

  return list;
}

//----------------------------------------------------------
/**
 * @brief	WIFI_LISTのコピー
 * @param	from	コピー元WIFI_LISTへのポインタ
 * @param	to		コピー先WIFI_LISTへのポインタ
 */
//----------------------------------------------------------
void WifiList_Copy(const WIFI_LIST * from, WIFI_LIST * to)
{
  GFL_STD_MemCopy(from, to, sizeof(WIFI_LIST));
}

//-----------------------------------------------------------------==============
//
//	WIFI_LIST操作のための関数
//
//-----------------------------------------------------------------==============
//----------------------------------------------------------
/**
 * @brief			Wifi用ともだちリスト・自分用データの初期化
 * @param
 */
//----------------------------------------------------------
void WifiList_Init(WIFI_LIST * list)
{
  int i;

  GFL_STD_MemClear(list, sizeof(WIFI_LIST));

  GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifi ) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifilib ) );
  GFL_NET_WIFI_InitUserData(&list->my_dwcuser);  //GameSpyログイン用仮userコードの作成(絶対必要）
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifilib ) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifi ) );

}


//-----------------------------------------------------------------
/**
 * $brief   自分のWIFI接続署名情報のポインタを返す
 *
 * @param   list	WIFI_LIST構造体のポインタ
 *
 * @retval  u8*
 */
//-----------------------------------------------------------------
DWCUserData* WifiList_GetMyUserInfo( WIFI_LIST* list )
{
  return &(list->my_dwcuser);
}



//-----------------------------------------------------------------
/**
 * $brief   WIFI友達情報をタイプ毎に返す
 *
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何番目の友達か
 * @param   type	設定する情報のタイプ（wifilist.hにenum宣言あり)
 *
 * @retval  u32
 */
//-----------------------------------------------------------------
u32 WifiList_GetFriendInfo( WIFI_LIST* list, int no, int type )
{
  u32 result=0;

  GF_ASSERT( no < WIFILIST_FRIEND_MAX );

  switch(type){
  case WIFILIST_FRIEND_ID:
    result = list->friendData[no].id;
    break;
  case WIFILIST_FRIEND_BATTLE_WIN:
    result = list->friendData[no].battle_win;
    break;
  case WIFILIST_FRIEND_BATTLE_LOSE:
    result = list->friendData[no].battle_lose;
    break;
  case WIFILIST_FRIEND_TRADE_NUM:
    result = list->friendData[no].trade_num;
    break;
  case WIFILIST_FRIEND_LASTBT_YEAR:
    result = list->friendData[no].year;
    break;
  case WIFILIST_FRIEND_LASTBT_MONTH:
    result = list->friendData[no].month;
    break;
  case WIFILIST_FRIEND_LASTBT_DAY:
    result = list->friendData[no].day;
    break;
  case WIFILIST_FRIEND_SEX:
    result = list->friendData[no].sex;
    break;
  case WIFILIST_FRIEND_UNION_GRA:
    result = list->friendData[no].unionGra;
    break;
  case WIFILIST_FRIEND_POFIN_NUM:  //todoなくなります
//    result = list->friendData[no].pofin_num;
    break;
  case WIFILIST_FRIEND_BALLSLOW_NUM:
//    result = list->friendData[no].minigame_ballslow;
    break;
  case WIFILIST_FRIEND_BALANCEBALL_NUM:
//    result = list->friendData[no].minigame_balanceball;
    break;
  case WIFILIST_FRIEND_BALLOON_NUM:
//    result = list->friendData[no].minigame_balloon;
    break;
  }
  return result;
}


//-----------------------------------------------------------------
/**
 * $brief
 *
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何番目の友達か
 * @param   type	設定する情報のタイプ（wifilist.hにenum宣言あり)
 * @param   value	設定する値
 *
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_SetFriendInfo( WIFI_LIST* list, int no, int type, u32 value )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );

  switch(type){
  case WIFILIST_FRIEND_ID:
    list->friendData[no].id = value;
    break;
  case WIFILIST_FRIEND_BATTLE_WIN:
    GF_ASSERT(0);
    break;
  case WIFILIST_FRIEND_BATTLE_LOSE:
    GF_ASSERT(0);
    break;
  case WIFILIST_FRIEND_TRADE_NUM:
    GF_ASSERT(0);
    break;
  case WIFILIST_FRIEND_LASTBT_YEAR:
    list->friendData[no].year = value;
    break;
  case WIFILIST_FRIEND_LASTBT_MONTH:
    list->friendData[no].month = value;
    break;
  case WIFILIST_FRIEND_LASTBT_DAY:
    list->friendData[no].day = value;
    break;
  case WIFILIST_FRIEND_SEX:
    list->friendData[no].sex = value;
    break;
  case WIFILIST_FRIEND_UNION_GRA:
    list->friendData[no].unionGra = value;
    break;
  case WIFILIST_FRIEND_POFIN_NUM:
    GF_ASSERT(0);
    break;
  }
}


//-----------------------------------------------------------------
/**
 * $brief   ともだちリストから友達のWIFI用情報構造体のポインタを取得
 *
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 *
 * @retval  DWCFriendData	*
 */
//-----------------------------------------------------------------
DWCFriendData	*WifiList_GetDwcDataPtr( WIFI_LIST* list, int no )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );

  return &(list->friend_dwc[no]);
}


//-----------------------------------------------------------------
/**
 * $brief
 *
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 *
 * @retval  u8 *
 */
//-----------------------------------------------------------------
STRCODE *WifiList_GetFriendNamePtr( WIFI_LIST *list, int no )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );

  return list->friendData[no].name;
}

//-----------------------------------------------------------------
/**
 * $brief   セーブエリアの名前をSTRBUFにいれる
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_GetFriendName( WIFI_LIST *list, int no, STRBUF* pBuf )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );
  GFL_STR_SetStringCodeOrderLength(pBuf, list->friendData[no].name, sizeof(list->friendData[no].name));
}

//-----------------------------------------------------------------
/**
 * $brief   STRBUFの中にある名前を、セーブエリアに格納する
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_SetFriendName( WIFI_LIST *list, int no, STRBUF* pBuf )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );
  GFL_STR_GetStringCode(pBuf, list->friendData[no].name, sizeof(list->friendData[no].name));
}

//-----------------------------------------------------------------
/**
 * $brief
 *
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 *
 * @retval  u8 *
 */
//-----------------------------------------------------------------
  //@todo なくなります
STRCODE *WifiList_GetFriendGroupNamePtr( WIFI_LIST *list, int no )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );
//  return list->friendData[no].groupName;
  //@todo なくなります
  return list->friendData[no].name;
}

//-----------------------------------------------------------------
/**
 * $brief   セーブエリアの名前をSTRBUFにいれる
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  u8 *
 */
//-----------------------------------------------------------------
  //@todo なくなります
void WifiList_GetFriendGroupName( WIFI_LIST *list, int no, STRBUF* pBuf )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );
  GFL_STR_SetStringCodeOrderLength(pBuf, list->friendData[no].name, sizeof(list->friendData[no].name));
}

//-----------------------------------------------------------------
/**
 * $brief   STRBUFの中にある名前を、セーブエリアに格納する
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  u8 *
 */
//-----------------------------------------------------------------
  //@todo なくなります
void WifiList_SetFriendGroupName( WIFI_LIST *list, int no, STRBUF* pBuf )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );

  GFL_STR_GetStringCode(pBuf, list->friendData[no].name, sizeof(list->friendData[no].name));
}

//-----------------------------------------------------------------
/**
 * $brief   データが入っているかどうか
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  BOOL
 */
//-----------------------------------------------------------------
BOOL WifiList_IsFriendData( WIFI_LIST *list, int no )
{
  GF_ASSERT( no < WIFILIST_FRIEND_MAX );
  if(no >= WIFILIST_FRIEND_MAX){
    return FALSE;
  }
  // フレンドコードが正しいかどうかで判断
  return DWC_IsValidFriendData(&list->friend_dwc[no]);
}

//-----------------------------------------------------------------
/**
 * $brief   入っている人数をかえす
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  BOOL
 */
//-----------------------------------------------------------------
int WifiList_GetFriendDataNum( WIFI_LIST *list )
{
  int i,num = 0;

  for(i = 0; i < WIFILIST_FRIEND_MAX;i++){
    if(WifiList_IsFriendData(list,i)){
      num++;
    }
  }
  return num;
}

//----------------------------------------------------------------------------
/**
 *	@brief	入っている友達の最後のインデックスを取得する
 *
 *	@param	list	構造体
 *
 *	@retval	最後のインデックス
 */
//-----------------------------------------------------------------------------
int WifiList_GetFriendDataLastIdx( WIFI_LIST *list )
{
  int i,last = 0;

  for(i = 0; i < WIFILIST_FRIEND_MAX;i++){
    if(WifiList_IsFriendData(list,i)){
      last = i+1;
    }
  }
  return last;
}

//-----------------------------------------------------------------
/**
 * $brief   フレンドの名前を消す
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_ResetData( WIFI_LIST *list, int no)
{
  int i;

  GF_ASSERT( no < WIFILIST_FRIEND_MAX);
  if(no < WIFILIST_FRIEND_MAX){
    for(i = no; i < (WIFILIST_FRIEND_MAX-1); i++){
      GFL_STD_MemCopy(&list->friendData[i+1], &list->friendData[i], sizeof(WIFI_FRIEND));
      GFL_STD_MemCopy(&list->friend_dwc[i+1], &list->friend_dwc[i], sizeof(DWCFriendData));
    }
    i = WIFILIST_FRIEND_MAX-1;
    GFL_STD_MemClear(&list->friendData[i], sizeof(WIFI_FRIEND));
    GFL_STD_MemClear(&list->friend_dwc[i],sizeof(DWCFriendData));
  }
}


//-----------------------------------------------------------------
/**
 * $brief   フレンドの名前を移動する
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		    ここに移動
 * @param   moveNo		移動させるデータ
 * @retval  none
 */
//-----------------------------------------------------------------
static void WifiList_MoveData( WIFI_LIST *list, int no, int moveNo)
{
  int i;

  GFL_STD_MemCopy(&list->friendData[moveNo], &list->friendData[no], sizeof(WIFI_FRIEND));
  GFL_STD_MemCopy(&list->friend_dwc[moveNo], &list->friend_dwc[no], sizeof(DWCFriendData));
  GFL_STD_MemClear(&list->friendData[moveNo], sizeof(WIFI_FRIEND));
  GFL_STD_MemClear(&list->friend_dwc[moveNo],sizeof(DWCFriendData));
}

//-----------------------------------------------------------------
/**
 * $brief   フレンドのデータをコピーする
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		    ここに移動
 * @param   moveNo		移動させるデータ
 * @retval  none
 */
//-----------------------------------------------------------------
#ifdef PM_DEBUG
void WifiList_CopyData( WIFI_LIST *list, int no, int copyNo)
{
  GFL_STD_MemCopy(&list->friendData[copyNo], &list->friendData[no], sizeof(WIFI_FRIEND));
  GFL_STD_MemCopy(&list->friend_dwc[copyNo], &list->friend_dwc[no], sizeof(DWCFriendData));
}

void WifiList_SetCountStopNum( WIFI_LIST *list, int no )
{
  list->friendData[no].battle_win = WIFILIST_COUNT_RANGE_MAX;
  list->friendData[no].battle_lose = WIFILIST_COUNT_RANGE_MAX;
  list->friendData[no].trade_num = WIFILIST_COUNT_RANGE_MAX;

  WifiList_SetLastPlayDate(list, no);

}

#endif

//-----------------------------------------------------------------
/**
 * $brief   フレンドをつめる
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_FormUpData( WIFI_LIST *list)
{
  int i,blank = -1;

  for(i = 0; i < WIFILIST_FRIEND_MAX; i++){
    if(WifiList_IsFriendData(list, i)){
      if(blank != -1){
        WifiList_MoveData(list, blank, i);  // 空きに移動
        OS_TPrintf("friend move %d < %d\n",blank,i);
        i = -1;  // もう一回検査
        blank = -1;
      }
    }
    else{
      if(blank == -1){
        blank = i;  // 空きを記憶
      }
    }
  }
}

//-----------------------------------------------------------------
/**
 * $brief   日時の記録をつける
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_SetLastPlayDate( WIFI_LIST *list, int no)
{
  RTCDate	rtc;
  GFL_RTC_GetDate(&rtc);
  list->friendData[no].year = rtc.year+2000;
  list->friendData[no].month = rtc.month;
  list->friendData[no].day = rtc.day;
}

//-----------------------------------------------------------------
/**
 * $brief   勝ち負けの記録をつける
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @param   winNum  勝ち数
 * @param   loseNum 負け数
 * @param   trade   交換数
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_SetResult( WIFI_LIST *list, int no, int winNum, int loseNum,int trade)
{
  list->friendData[no].battle_win += winNum;
  if(list->friendData[no].battle_win > WIFILIST_COUNT_RANGE_MAX){
    list->friendData[no].battle_win = WIFILIST_COUNT_RANGE_MAX;
  }
  list->friendData[no].battle_lose += loseNum;
  if(list->friendData[no].battle_lose > WIFILIST_COUNT_RANGE_MAX){
    list->friendData[no].battle_lose = WIFILIST_COUNT_RANGE_MAX;
  }
  list->friendData[no].trade_num += trade;
  if(list->friendData[no].trade_num > WIFILIST_COUNT_RANGE_MAX){
    list->friendData[no].trade_num = WIFILIST_COUNT_RANGE_MAX;
  }
  OS_TPrintf("list->friendData[  %d ].trade_num  %d\n",no,list->friendData[no].trade_num);
  WifiList_SetLastPlayDate(list, no);
}

//-----------------------------------------------------------------
/**
 * @brief   Aの人のデータをBの人にマージする
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @param   winNum  勝ち数
 * @param   loseNum 負け数
 * @param   trade   交換数
 * @retval  none
 */
//-----------------------------------------------------------------
void WifiList_DataMarge( WIFI_LIST *list, int delNo, int no)
{
  list->friendData[no].battle_win += list->friendData[delNo].battle_win;
  if(list->friendData[no].battle_win > WIFILIST_COUNT_RANGE_MAX){
    list->friendData[no].battle_win = WIFILIST_COUNT_RANGE_MAX;
  }
  list->friendData[no].battle_lose += list->friendData[delNo].battle_lose;
  if(list->friendData[no].battle_lose > WIFILIST_COUNT_RANGE_MAX){
    list->friendData[no].battle_lose = WIFILIST_COUNT_RANGE_MAX;
  }
  list->friendData[no].trade_num += list->friendData[delNo].trade_num;
  if(list->friendData[no].trade_num > WIFILIST_COUNT_RANGE_MAX){
    list->friendData[no].trade_num = WIFILIST_COUNT_RANGE_MAX;
  }

  GFL_STD_MemClear(&list->friendData[delNo], sizeof(WIFI_FRIEND));
}

//---------------------------------------------------------------------------
/**
 * @brief	相手に送信するフレンドコードを得る
 * @param	pList  WIFIリストポインタ
 * @param	pFriend  フレンドデータ
 * @return	WIFI_LIST	WIFIリスト
 */
//---------------------------------------------------------------------------

void GFL_NET_DWC_GetMySendedFriendCode( const WIFI_LIST* pList, DWCFriendData* pFriend )
{
  DWC_CreateExchangeToken(&pList->my_dwcuser, pFriend);
}


//---------------------------------------------------------------------------
/**
 * @brief   自分のGSIDを取得する
 * @param   WIFI_LIST構造体
 * @retval  正の整数  	GSプロファイルID
            0 	友達登録鍵が不正のため、取得できない
            -1 	ログインIDが格納されているため、取得できない
 */
//---------------------------------------------------------------------------

int WifiList_GetMyGSID( WIFI_LIST* pWifiList )
{
  DWCUserData *userdata = WifiList_GetMyUserInfo(pWifiList);
  DWCFriendData friendData;

  GF_ASSERT(userdata);
  DWC_CreateExchangeToken(userdata,&friendData);
  return DWC_GetGsProfileId( userdata,&friendData);
}

//==============================================================================
/**
 * 自分のGSIDが正しいのか検査する
 * @param   GFL_WIFI_FRIENDLIST
 * @retval  TRUEが成功
 */
//==============================================================================
BOOL WifiList_CheckMyGSID( WIFI_LIST* pWifiList )
{
  DWCUserData *userdata = WifiList_GetMyUserInfo(pWifiList);

  GF_ASSERT(userdata);
  if( DWC_CheckHasProfile( userdata )
      && DWC_CheckValidConsole( userdata ) ){
    return TRUE;
  }
  return FALSE;
}



//---------------------------------------------------------------------------
/**
 * @brief	WIFIリストをセーブデータから引き出す
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	WIFI_LIST	WIFIリスト
 */
//---------------------------------------------------------------------------
WIFI_LIST* SaveData_GetWifiListData(SAVE_CONTROL_WORK * sv)
{
  WIFI_LIST* pData;
  STRCODE* pCode;

  pData = SaveControl_DataPtrGet(sv, GMDATA_ID_WIFILIST);

  pCode = WifiList_GetFriendNamePtr(pData,0);

  OS_TPrintf("id = %x / %d %d %d\n",WifiList_GetFriendInfo(pData,0,WIFILIST_FRIEND_ID),
             pCode[0],
             pCode[1],
             pCode[2]
             );

  return pData;
}

