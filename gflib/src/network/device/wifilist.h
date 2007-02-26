//============================================================================================
/**
 * @file	WifiList.h
 * @brief	ともだちてちょう用 
 * @author	mori GAME FREAK inc.  --> k.ohno
 * @date	2006.02.10
 */
//============================================================================================
#ifndef __WIFILIST_H__
#define __WIFILIST_H__

#include <dwc.h>

#if _SAVE_PROGRAM
#include "savedata/savedata_def.h"	//SAVEDATA参照のため
#endif

//#include "gflib/msg_print.h"		//STRCODE参照のため
#include "strbuf.h"			//STRBUF参照のため


#define WIFILIST_FRIEND_MAX	( 32 )	// WIFIリストが持つフレンドの数
#define WIFILIST_COUNT_RANGE_MAX  (9999)   // かちまけ等の回数上限

//============================================================================================
//============================================================================================


// WifiList_<Get*Set>FriendInfoで使うenum
enum{
	WIFILIST_FRIEND_ID = 0,
//	WIFILIST_FRIEND_BATTLE_NUM,
	WIFILIST_FRIEND_BATTLE_WIN,
	WIFILIST_FRIEND_BATTLE_LOSE,
	WIFILIST_FRIEND_TRADE_NUM,
	WIFILIST_FRIEND_LASTBT_YEAR,
	WIFILIST_FRIEND_LASTBT_MONTH,
	WIFILIST_FRIEND_LASTBT_DAY,
	WIFILIST_FRIEND_UNION_GRA,
	WIFILIST_FRIEND_SEX,
};

//============================================================================================
//============================================================================================
//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int WifiList_GetWorkSize(void);
extern GFL_WIFI_FRIENDLIST * WifiList_AllocWork(u32 heapID);
extern void WifiList_Copy(const GFL_WIFI_FRIENDLIST * from, GFL_WIFI_FRIENDLIST * to);
extern void WifiList_Init(GFL_WIFI_FRIENDLIST * list);
extern DWCUserData* WifiList_GetMyUserInfo( GFL_WIFI_FRIENDLIST* list );
extern u32 WifiList_GetFriendInfo( GFL_WIFI_FRIENDLIST* list, int no, int type );
extern void WifiList_SetFriendInfo( GFL_WIFI_FRIENDLIST* list, int no, int type, u32 value );
extern DWCFriendData* WifiList_GetDwcDataPtr( GFL_WIFI_FRIENDLIST* list, int no );
extern STRCODE *WifiList_GetFriendNamePtr( GFL_WIFI_FRIENDLIST *list, int no );
extern void WifiList_SetFriendName( GFL_WIFI_FRIENDLIST *list, int no, STRBUF* pBuf );
extern STRCODE *WifiList_GetFriendGroupNamePtr( GFL_WIFI_FRIENDLIST *list, int no );
extern void WifiList_SetFriendGroupName( GFL_WIFI_FRIENDLIST *list, int no, STRBUF* pBuf );
extern BOOL WifiList_IsFriendData( GFL_WIFI_FRIENDLIST *list, int no );
extern int WifiList_GetFriendDataNum( GFL_WIFI_FRIENDLIST *list );
extern void WifiList_SetLastPlayDate( GFL_WIFI_FRIENDLIST *list, int no);

extern void WifiList_ResetData( GFL_WIFI_FRIENDLIST *list, int no);
extern void WifiList_SetResult( GFL_WIFI_FRIENDLIST *list, int no, int winNum, int loseNum,int trade);
extern void WifiList_DataMarge( GFL_WIFI_FRIENDLIST *list, int delNo, int no);
extern void WifiList_FormUpData( GFL_WIFI_FRIENDLIST *list);

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------

//@@OO
//extern GFL_WIFI_FRIENDLIST* SaveData_GetWifiListData(SAVEDATA* pSave);

#endif