//=============================================================================
/**
 * @file	comm_mystery_state.h
 * @brief	通信状態を管理するサービス  通信の上位にある
 *          	スレッドのひとつとして働き、自分の通信状態や他の機器の
 *          	開始や終了を管理する
 *		※comm_field_state.cの真似っ子
 * @author	Satoshi Mitsuhara
 * @date    	2006.05.17
 */
//=============================================================================
#ifndef __COMM_MYSTERY_STATE_H__
#define __COMM_MYSTERY_STATE_H__

#include "comm_mystery_gift.h"
#include "net/network_define.h"


extern void CommMysteryStateEnterGiftParent(MYSTERYGIFT_WORK *pMSys, SAVE_CONTROL_WORK *sv, int serviceNo);
extern int CommMysteryGetCommChild(void);
extern void CommMysterySendGiftDataParent(const void *p, int size);


extern void CommMysteryStateEnterGiftChild(MYSTERYGIFT_WORK *pMSys, int serviceNo);
extern void CommMysteryStateConnectGiftChild(int connectIndex);
extern int CommMysteryCheckParentBeacon(MYSTERYGIFT_WORK *wk);
extern int CommMysteryCheckRecvData(void);
extern void CommMysteryResultRecvData(void);

extern int CommMysteryGetRecvPlaceSize(void);
extern u8* CommGetMysteryGiftRecvBuff( int netID, void* pWork, int size);
extern int CommMysteryGiftGetRecvCheck(void);
extern void CommMysteryExitGift(void);

//他の場所でライブラリだけ初期化する必要が出てきたので
extern void CommMysteryInitNetLib(void* pWork);

/// ふしぎなおくりもの専用通信コマンドの定義。
enum CommCommandField_e {
	CM_GIFT_DATA = GFL_NET_CMD_MYSTERY,	// プレゼントのデータを送る
	CM_RECV_RESULT,						// ちゃんと受け取りましたよ返答
	//------------------------------------------------ここまで
	CM_COMMAND_MAX   // 終端--------------これは移動させないでください
};

extern void CommCommandMysteryInitialize(void* pWork);

#endif	// __COMM_MYSTERY_STATE_H__
/*  */
