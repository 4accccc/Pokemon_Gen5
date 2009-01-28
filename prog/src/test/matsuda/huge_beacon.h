//==============================================================================
/**
 * @file	huge_beacon.h
 * @brief	ビーコンを使用して巨大データを送受信のヘッダ
 * @author	matsuda
 * @date	2009.01.23(金)
 */
//==============================================================================
#ifndef __HUGE_BEACON_H__
#define __HUGE_BEACON_H__

//==============================================================================
//	定数定義
//==============================================================================
#define GGID_HUGE_DATA		(0x589)	//GGID：ビーコン巨大データ送受信テスト用


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void HUGEBEACON_SystemCreate(int heap_id, u32 send_data_size, const void *send_data, void *receive_buf);
extern void HUGEBEACON_SystemExit(void);
extern void HUGEBEACON_Start(void);
extern void HUGEBEACON_Main(void);

#endif	//__HUGE_BEACON_H__
