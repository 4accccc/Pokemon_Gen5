#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 * @file	dwc_rapcommon.h
 * @brief	Wi-Fiデバイスとのアクセス関数 汎用の処理
 * @author	GAME FREAK Inc.
 * @date    2007.2.20
 */
//=============================================================================

#pragma once

#if GFL_NET_WIFI
#include "gamesystem/game_comm.h"  //GAME_COMM_STATUS
#include <dwc.h>


//==============================================================================
/**
 * DWCライブラリ初期化
 * @param   heapID   ヒープID
 * @retval  DS本体に保存するユーザIDのチェック・作成結果。
 */
//==============================================================================
extern int mydwc_init(HEAPID heapID);

//==============================================================================
/**
 * @brief   DWC  UserDataを作る
 * @param   DWCUserData  ユーザーデータがない場合作成
 * @retval  none
 */
//==============================================================================
extern void mydwc_createUserData( DWCUserData *userdata );

//==============================================================================
/**
 * 自分のGSIDを取得する
 * @param   GFL_WIFI_FRIENDLIST  
 * @retval  ０と－１は失敗   正の値は成功
 */
//==============================================================================

//extern int mydwc_getMyGSID(void);

//==============================================================================
/**
 * 自分のGSIDが正しいのか検査する
 * @retval  TRUEが成功
 */
//==============================================================================
extern BOOL mydwc_checkMyGSID(void);

//==============================================================================
/**
 * @brief   WIFIで使うHEAPIDをセットする
 * @param   id     変更するHEAPID
 * @retval  none
 */
//==============================================================================
extern void DWC_RAPCOMMON_SetHeapID(HEAPID id,int size);
extern void DWC_RAPCOMMON_ResetHeapID(void);
extern void* DWC_RAPCOMMON_Alloc( DWCAllocType name, u32 size, int align );
extern void DWC_RAPCOMMON_Free( DWCAllocType name, void *ptr, u32 size );

extern void DWC_RAPCOMMON_SetSubHeapID( DWCAllocType SubAllocType, u32 size, HEAPID heapID );
extern void DWC_RAPCOMMON_ResetSubHeapID(void);

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンのタイプを得る
 * @param   ビーコン数
 * @param   ヒープID
 * @retval  GAME_COMM_STATUS
 */
//------------------------------------------------------------------------------
extern GAME_COMM_STATUS DWC_RAPCOMMON_GetBeaconType(int num, HEAPID heap);

#endif //

#ifdef __cplusplus
} /* extern "C" */
#endif
