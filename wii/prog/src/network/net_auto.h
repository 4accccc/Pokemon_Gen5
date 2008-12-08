//=============================================================================
/**
 * @file	net_auto.h
 * @brief	
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================

#ifndef __NET_AUTO_H__
#define __NET_AUTO_H__


typedef void (*PSyncroCallback)(void* pWork);



//==============================================================================
/**
 * @brief   自動でサーバを立ち上げる 認証等はすべてお任せ
 * @param   num     接続同期に必要な人数
 * @param   num     人数分の同期が取れたとき呼ばれるコールバック
 * @retval  none
 */
//==============================================================================

extern void GFL_NET_AutoInitServer(int num,PSyncroCallback pSyncroCallback);



#endif //__NET_AUTO_H__

