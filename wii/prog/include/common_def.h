#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMMON_DEF_H__
#define COMMON_DEF_H__
#include <revolution.h>
#include <revolution/mp.h>
#include <revolution/mpds.h>
#include <revolution/mem.h>
#include <revolution/enc.h>
#include "crc.h"

#define	 MY_NICKNAME		 L"Wii" // 子機の親機選択画面で表示するニックネーム
#define	 MY_NICKNAME_LENGTH  3	  // MY_NICKNAME の UTF16BE での文字数 (最大10文字)

#define	 MY_MAX_NODES	4   // 接続許可を行う子機台数の上限 (Wii の Sequential 通信では最大8台まで保証)
#define	 MY_DS_NODES	 4  // DataSharing を行う親機も含めたノード数 (NITRO-SDK の dataShare-Model デモに合わせている)
								// 通常のアプリケーションであれば (MY_MAX_NODES+1)

//#define	 MY_DS_LENGTH	12  // DataSharing で共有する1台あたりのデータのサイズ
#define	 MY_DS_LENGTH	0x30  // DataSharing で共有する1台あたりのデータのサイズ
#define	 MY_DS_PORT	  13  // DataSharing で使用するポート番号 (NITRO-SDK の dataShare-Model デモに合わせている)

// DataSharing を行う場合は、1台あたりの共有サイズ×共有台数＋4バイトの送信サイズが必要
#define	 MY_PARENT_MAX_SIZE ( (MY_DS_LENGTH * MY_DS_NODES) + MPDS_HEADER_SIZE )

#define	 STACK_SIZE	  8192 // スレッドのスタックサイズ; 割り込み処理で使用される分も考慮にいれる必要がある
#define	 THREAD_PRIORITY 14   // スレッドの優先度; メインスレッド (16) より高い優先度を指定する


// 共有データ構造体
// DS に合わせて Little Endian で処理する必要があることに注意
#pragma pack(push,1)
typedef struct ShareData
{
	u8	data[MY_DS_LENGTH];
/*
	u8 data:3;			   // グラフ用
	u8 level:2;			  // 電波受信強度
	u8 command:3;			// 指令（ゲーム状態の一斉切り換えなどに使用）
	u16 key;				 // キーデータ
	u32 count:24;			// フレーム数
*/
}
ShareData;
#pragma pack(pop)

extern void EnterCriticalSection( void );
extern void LeaveCriticalSection( void );

extern void* mpAlloc( u32 size );
extern void mpFree( void* ptr );
//DSから持ってきたファイルの互換を保つもの

extern MATHCRC16Table *crcTable;
typedef int HEAPID;
#define GFL_HEAPID_SYSTEM 0
#define MI_CpuCopy8(src,dest,size) NETMemCpy(dest,src,(u32)size)
#define MI_CpuFill8(src,dest,size) NETMemSet(src,dest,(u32)size)
#define GFL_STD_MemCopy(src,dest,size) NETMemCpy(dest,src,(u32)size)
#define GFL_STD_MemClear(src,size) NETMemSet(src,0,(u32)size)
#define OS_GetMacAddress NETGetWirelessMacAddress
//#define NET_PRINT(...) OSReport(__VA_ARGS__)
#define NET_PRINT(...) ((void)0)
#define OS_TPrintf OSReport 
#define GF_ASSERT ASSERT
#define OS_Panic OSHalt
#define GFL_HEAP_AllocMemory(work,size) mpAlloc((u32)size)
#define GFL_HEAP_FreeMemory mpFree

#define GFL_STD_CrcCalc(adr,len) MATH_CalcCRC16CCITT(crcTable,adr,len)

extern void* GFL_HEAP_AllocClearMemory(int work, int size);


#endif //COMMON_DEF_H__

#ifdef __cplusplus
} /* extern "C" */
#endif
