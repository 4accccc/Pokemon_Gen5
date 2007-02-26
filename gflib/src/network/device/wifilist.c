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
	return sizeof(GFL_WIFI_FRIENDLIST);
}

//----------------------------------------------------------
/**
 * @brief	自分状態保持ワークの確保
 * @param	heapID		メモリ確保をおこなうヒープ指定
 * @return	GFL_WIFI_FRIENDLIST	取得したワークへのポインタ
 */
//----------------------------------------------------------
GFL_WIFI_FRIENDLIST * WifiList_AllocWork(u32 heapID)
{
	int i;
	GFL_WIFI_FRIENDLIST * list;
	list = GFL_HEAP_AllocMemory(heapID, sizeof(GFL_WIFI_FRIENDLIST));
	
	WifiList_Init( list );

	return list;
}

//----------------------------------------------------------
/**
 * @brief	GFL_WIFI_FRIENDLISTのコピー
 * @param	from	コピー元GFL_WIFI_FRIENDLISTへのポインタ
 * @param	to		コピー先GFL_WIFI_FRIENDLISTへのポインタ
 */
//----------------------------------------------------------
void WifiList_Copy(const GFL_WIFI_FRIENDLIST * from, GFL_WIFI_FRIENDLIST * to)
{
	MI_CpuCopy8(from, to, sizeof(GFL_WIFI_FRIENDLIST));
}

//============================================================================================
//
//	GFL_WIFI_FRIENDLIST操作のための関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief			Wifi用ともだちリスト・自分用データの初期化
 * @param			
 */
//----------------------------------------------------------
void WifiList_Init(GFL_WIFI_FRIENDLIST * list)
{
	int i;
	
	MI_CpuClearFast(list, sizeof(GFL_WIFI_FRIENDLIST));

    mydwc_createUserData(list);  //GameSpyログイン用仮userコードの作成(絶対必要）
}


//==============================================================================
/**
 * $brief   自分のWIFI接続署名情報のポインタを返す
 *
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 *
 * @retval  u8*		
 */
//==============================================================================
DWCUserData* WifiList_GetMyUserInfo( GFL_WIFI_FRIENDLIST* list )
{
	return &(list->my_dwcuser);
}

//==============================================================================
/**
 * $brief   ともだちリストから友達のWIFI用情報構造体のポインタを取得
 *
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 * @param   no		何人目の友達か
 *
 * @retval  DWCFriendData	*		
 */
//==============================================================================
DWCFriendData	*WifiList_GetDwcDataPtr( GFL_WIFI_FRIENDLIST* list, int no )
{
	GF_ASSERT( no < WIFILIST_FRIEND_MAX );

	return &(list->friend_dwc[no]);
}

//==============================================================================
/**
 * $brief   データが入っているかどうか
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  BOOL
 */
//==============================================================================
BOOL WifiList_IsFriendData( GFL_WIFI_FRIENDLIST *list, int no )
{
	GF_ASSERT( no < WIFILIST_FRIEND_MAX );
    // フレンドコードが正しいかどうかで判断
    return DWC_IsValidFriendData(&list->friend_dwc[no]);
}

//==============================================================================
/**
 * $brief   入っている人数をかえす
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  BOOL
 */
//==============================================================================
int WifiList_GetFriendDataNum( GFL_WIFI_FRIENDLIST *list )
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
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//==============================================================================
void WifiList_ResetData( GFL_WIFI_FRIENDLIST *list, int no)
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
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 * @param   no		    ここに移動
 * @param   moveNo		移動させるデータ
 * @retval  none
 */
//==============================================================================
static void WifiList_MoveData( GFL_WIFI_FRIENDLIST *list, int no, int moveNo)
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
 * @param   list	GFL_WIFI_FRIENDLIST構造体のポインタ
 * @param   no		何人目の友達か
 * @retval  none
 */
//==============================================================================
void WifiList_FormUpData( GFL_WIFI_FRIENDLIST *list)
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
 * @return	GFL_WIFI_FRIENDLIST	WIFIリスト
 */
//---------------------------------------------------------------------------
#if _SAVE_PROGRAM
GFL_WIFI_FRIENDLIST* SaveData_GetWifiListData(SAVEDATA * sv)
{
	GFL_WIFI_FRIENDLIST* pData;
	pData = SaveData_Get(sv, GMDATA_ID_WIFILIST);
	return pData;
}
#endif
