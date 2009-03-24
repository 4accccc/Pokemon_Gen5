//=============================================================================
/**
 * @file	dwc_rapfriend.c
 * @bfief	DWCラッパー。常駐モジュールに置くもの
 * @author	kazuki yoshihara  -> k.ohno改造
 * @date	06/02/23
 */
//=============================================================================

#ifndef __DWC_RAPFRIEND_H__
#define  __DWC_RAPFRIEND_H__

#include <dwc.h>
#include "savedata/save_control.h"
#include "savedata/mystatus.h"

//==============================================================================
/**
 * すでに同じ人が登録されていないか。
 * @param   pSaveData フレンド関係がはいっているセーブデータ
 * @param   index 同じデータが見つかったときの、その場所。
 * @param         見つからない場合は空いているところを返す
 * @param         どこも空いていない場合は、-1 
 * @retval  DWCFRIEND_INLIST … すでに、同じデータがリスト上にある。
 * @retval  DWCFRIEND_NODATA … 同じデータはリスト上にない。
 * @retval  DWCFRIEND_INVALID … 受け取ったfriend_keyaが正しくない。
 */
//==============================================================================
extern int dwc_checkFriendCodeByToken( u64 friend_key, int *index);
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
extern void GFL_NET_DWC_FriendDataWrite(SAVE_CONTROL_WORK* pSaveData, MYSTATUS* pMyStatus,DWCFriendData* pFriend, int addListIndex, int heapID, int overWrite);

extern void GFL_NET_DWC_FriendDataAdd(SAVE_CONTROL_WORK* pSaveData, MYSTATUS* pMyStatus,DWCFriendData* pFriend, int heapID);

#endif //__DWC_RAPFRIEND_H__
