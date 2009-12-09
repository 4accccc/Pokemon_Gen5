//=============================================================================
/**
 * @file	dwc_rapfriend.c
 * @bfief	DWCラッパー。常駐モジュールに置くもの
 * @author	kazuki yoshihara  -> k.ohno改造
 * @date	06/02/23
 */
//=============================================================================

#pragma once

#include <dwc.h>
#include "savedata/save_control.h"
#include "savedata/mystatus.h"
#include "savedata/wifilist.h"
#include "gamesystem/game_data.h"

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
extern int GFL_NET_DWC_CheckFriendByToken(DWCFriendData *data, int *index);

//==============================================================================
/**
 * すでに同じ人が登録されていないか。
 * @param   index 同じデータが見つかったときの、その場所。
 * @param         見つからない場合は空いているところを返す
 * @param         どこも空いていない場合は、-1 
 * @retval  DWCFRIEND_INLIST … すでに、同じデータがリスト上にある。
 * @retval  DWCFRIEND_NODATA … 同じデータはリスト上にない。
 * @retval  DWCFRIEND_INVALID … 受け取ったfriend_keyaが正しくない。
 */
//==============================================================================
extern int GFL_NET_DWC_CheckFriendCodeByToken( WIFI_LIST* list,u64 friend_key,int *index);

//------------------------------------------------------------------
/**
 * $brief   接続している人のフレンドコード検査  登録する人が見つかったら登録
            WIFINOTE_MODE_AUTOINPUT_CHECK
 * @param   wk		
 * @param   seq		
 * @retval  int 	
 */
//------------------------------------------------------------------

extern int GFL_NET_DWC_FriendAutoInputCheck( DWCFriendData* pFriend );

//------------------------------------------------------------------
/**
 * $brief   無線用登録関数
 * @param   wk		
 * @param   seq		
 * @retval  int 	
 */
//------------------------------------------------------------------
extern void GFL_NET_DWC_FriendDataWrite(GAMEDATA* pSaveData, MYSTATUS* pMyStatus,DWCFriendData* pFriend, int addListIndex, int heapID, int overWrite);

extern void GFL_NET_DWC_FriendDataAdd(GAMEDATA* pSaveData, MYSTATUS* pMyStatus,DWCFriendData* pFriend, int heapID);

