#ifndef __IRC_INTERNAL_H__
#define __IRC_INTERNAL_H__

#define USE_DEBUGPRINT	(0)

#if USE_DEBUGPRINT==1
#define PRINTF printf
#else
#define PRINTF TRUE ? (void)0 : printf
#endif
/******************************************************************************
******************************************************************************/
typedef signed   char	s8;
typedef unsigned char	u8;
typedef signed   short	s16;
typedef unsigned short	u16;
typedef signed   int	s32;
typedef unsigned int	u32;
/******************************************************************************
******************************************************************************/
typedef struct{
  u8	command;	// コマンド
  u8	value;		// ユーザフリー
  u8	sumL;		// チェックサム上位
  u8	sumH;		// チェックサム下位
}Header;
/***********************************************************************
NitroSDK 内で定義してある関数のextern 宣言
***********************************************************************/
extern u16 _calcsum(void *input, u32 length);
extern void dhc_init(void); // NitroSDK 内に実装
extern u8 dhc_read(u8 *buf); // ユーザは呼ばなくてよいはず
extern void dhci_write(u8 *buf, u8 size, u8 command, u8 value); // ユーザは呼ばなくてよいはず
extern void dhc_write(u8 *buf, u8 size, u8 command, u8 value);
extern void dhc_savedata(void); // ユーザは呼ばなくてよいはず
extern u8* dhc_getretrydata(void); // ユーザは呼ばなくてよいはず
extern void dhc_retry(void); // ユーザは呼ばなくてよいはず
extern void dhc_setreadwait(u8 time); // ユーザは呼ばなくてよいはず
extern void dhc_setwritewait(u8 time); // ユーザは呼ばなくてよいはず
extern void dhci_command(u8 command); // 1バイト送信関数

#endif
