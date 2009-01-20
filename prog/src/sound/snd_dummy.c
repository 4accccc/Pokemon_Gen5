//=============================================================================
/**
 * @file	sound_dummy.c
 * @brief	サウンド用のメモリ確保を予約したファイル
            サウンドを作る際には削除しなければいけない
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	2009.01.20
 */
//=============================================================================

#include <gflib.h>

static u8 dummySoundData[0xB0000];
extern void SoundDummyInit(void);

void SoundDummyInit(void)
{
    GFL_STD_MemClear(dummySoundData,sizeof(dummySoundData));
}

