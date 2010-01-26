//=============================================================================
/**
 * @file	  wm_icon.h
 * @brief	  通信アイコン
 * @author	Katsumi Ohno
 * @date    2005.07.08
 */
//=============================================================================

#pragma once

// 通信アイコンパレットデータが転送される位置
#define WM_ICON_PALETTE_NO	( 14 )

// 通信アイコンキャラデータを転送するVRAMの位置
// 通信アイコンキャラデータはﾒｲﾝOBJVRAMの一番最後に
// １６キャラ転送されます。

//OBJ16Kモードの時
#define WM_ICON_CHAR_OFFSET16  ( (512-16)*32 )	
//OBJ32Kモードの時
#define WM_ICON_CHAR_OFFSET32  ( (1024-16)*32 )	
//OBJ64Kモードの時
#define WM_ICON_CHAR_OFFSET64 ( (1024-16)*32 )	
//OBJ80Kモードの時
#define WM_ICON_CHAR_OFFSET80 ( (2560-16)*32 )	
//OBJ128Kモードの時
#define WM_ICON_CHAR_OFFSET128 ( (4096-16)*32 )	


//extern void GFL_NET_WirelessIconEasy(BOOL bWifi, HEAPID heapID);
extern void GFL_NET_WirelessIconEasyXY(int x, int y, BOOL bWifi, HEAPID heapID);
extern void GFL_NET_WirelessIconEasyEnd(void);
extern void GFL_NET_WirelessIconEasy_SetLevel(int level);
extern void GFL_NET_WirelessIconEasy_HoldLCD( BOOL bTop, HEAPID heapID );
extern void GFL_NET_WirelessIconEasy_DefaultLCD( void );
extern void GFL_NET_WirelessIconEasyFunc(void);
extern void GFL_NET_WirelessIconOBJWinON(void);
extern void GFL_NET_WirelessIconOBJWinOFF(void);

// 通信アイコンだすために転送するPAL_VRAMの位置・大きさ(14番パレット使用）
#define WM_ICON_PAL_POS		( 14 )
#define WM_ICON_PAL_OFFSET	( 16 * 2 * WM_ICON_PAL_POS )
#define WM_ICON_PAL_SIZE	( 16 * 2 )


