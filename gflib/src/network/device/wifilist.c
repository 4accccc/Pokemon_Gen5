//============================================================================================
/**
 * @file	wifilist.c
 * @brief	WIFIフレンドコード管理  > ともだち手帳のデータ管理
 * @author	mori san   >  k.ohno
 * @date	2006.03.27 くらい
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>
#include <dwc.h>
#include "gf_macro.h"
#include "gf_standard.h"
#include "assert.h"
#include "heapsys.h"
#include "strbuf.h"
#include "net.h"
#include "../net_def.h"

#include "wifilist.h"
#include "wifilist_local.h"
#include "dwc_rap.h"
#include "dwc_rapcommon.h"

//============================================================================================
//============================================================================================




//============================================================================================
//
//	セーブデータシステムが依存する関数
//
//============================================================================================
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
	list = GFL_HEAP_AllocMemory(heapID, sizeof(WIFI_LIST));
	
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
	MI_CpuCopy8(from, to, sizeof(WIFI_LIST));
}

//============================================================================================
//
//	WIFI_LIST操作のための関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief			Wifi用ともだちリスト・自分用データの初期化
 * @param			
 */
//----------------------------------------------------------
void WifiList_Init(WIFI_LIST * list)
{
	int i;
	
	MI_CpuClearFast(list, sizeof(WIFI_LIST));

    mydwc_createUserData(list);  //GameSpyログイン用仮userコードの作成(絶対必要）
}


//==============================================================================
/**
 * $brief   自分のWIFI接続署名情報のポインタを返す
 *
 * @param   list	WIFI_LIST構造体のポインタ
 *
 * @retval  u8*		
 */
//==============================================================================
DWCUserData* WifiList_GetMyUserInfo( WIFI_LIST* list )
{
	return &(list->my_dwcuser);
}

//==============================================================================
/**
 * $brief   ともだちリストから友達のWIFI用情報構造体のポインタを取得
 *
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 *
 * @retval  DWCFriendData	*		
 */
//==============================================================================
DWCFriendData	*WifiList_GetDwcDataPtr( WIFI_LIST* list, int no )
{
	GF_ASSERT( no < WIFILIST_FRIEND_MAX );

	return &(list->friend_dwc[no]);
}

//==============================================================================
/**
 * $brief   データが入っているかどうか
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  BOOL
 */
//==============================================================================
BOOL WifiList_IsFriendData( WIFI_LIST *list, int no )
{
	GF_ASSERT( no < WIFILIST_FRIEND_MAX );
    // フレンドコードが正しいかどうかで判断
    return DWC_IsValidFriendData(&list->friend_dwc[no]);
}

//==============================================================================
/**
 * $brief   入っている人数をかえす
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  BOOL
 */
//==============================================================================
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

//==============================================================================
/**
 * $brief   フレンドの名前を消す
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//==============================================================================
void WifiList_ResetData( WIFI_LIST *list, int no)
{
    int i;

    GF_ASSERT( no < WIFILIST_FRIEND_MAX);
    for(i = no; i < (WIFILIST_FRIEND_MAX-1); i++){
        MI_CpuCopy8(&list->friend_dwc[i+1], &list->friend_dwc[i], sizeof(DWCFriendData));
    }
    i = WIFILIST_FRIEND_MAX-1;
	MI_CpuClearFast(&list->friend_dwc[i],sizeof(DWCFriendData));
}


//==============================================================================
/**
 * $brief   フレンドの名前を移動する
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		    ここに移動
 * @param   moveNo		移動させるデータ
 * @retval  none
 */
//==============================================================================
static void WifiList_MoveData( WIFI_LIST *list, int no, int moveNo)
{
    int i;

//    MI_CpuCopy8(&list->friend[moveNo], &list->friend[no], sizeof(WIFI_FRIEND));
    MI_CpuCopy8(&list->friend_dwc[moveNo], &list->friend_dwc[no], sizeof(DWCFriendData));
//	MI_CpuClearFast(&list->friend[moveNo], sizeof(WIFI_FRIEND));
	MI_CpuClearFast(&list->friend_dwc[moveNo],sizeof(DWCFriendData));
//    list->friend[moveNo].name[0] = EOM_;
//    list->friend[moveNo].groupName[0] = EOM_;
//    list->friend[moveNo].sex = PM_NEUTRAL;
}

//==============================================================================
/**
 * $brief   フレンドをつめる
 * @param   list	WIFI_LIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//==============================================================================
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


//---------------------------------------------------------------------------
/**
 * @brief	WIFIリストをセーブデータから引き出す
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	WIFI_LIST	WIFIリスト
 */
//---------------------------------------------------------------------------
#if _SAVE_PROGRAM
WIFI_LIST* SaveData_GetWifiListData(SAVEDATA * sv)
{
	WIFI_LIST* pData;
	pData = SaveData_Get(sv, GMDATA_ID_WIFILIST);
	return pData;
}
#endif
