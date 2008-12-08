/*---------------------------------------------------------------------------*
  Project:  RevolutionSDK Extension
  File:	 demokpad.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: demokpad.c,v $
  Revision 1.11  2008/06/02 02:24:01  seiki_masashi
  KPADRead の仕様変更に対応

  Revision 1.10  2008/05/30 02:27:03  seiki_masashi
  SDK の巻き戻しに伴う巻き戻し

  Revision 1.8  2007/10/23 13:42:06  seiki_masashi
  added REXDEMOGetAnyKPadHold

  Revision 1.7  2007/06/16 06:31:07  hirose_kazuki
  warning への対処

  Revision 1.6  2007/06/16 06:17:36  hirose_kazuki
  GC Pad 関連の機能追加

  Revision 1.5  2006/09/05 10:56:12  yosizaki
  added REXDEMOGetAnyKPadTrigger.

  Revision 1.4  2006/08/29 07:19:20  adachi_hiroaki
  プレフィックス変更、そのほか整理

  Revision 1.3  2006/08/11 10:41:15  yasu
  KPADRead の返り値を無視する

  Revision 1.2  2006/08/10 12:09:47  adachi_hiroaki
  ヘッダファイルの位置を変更

  Revision 1.1  2006/08/09 13:28:49  yasu
  KPAD を使うためのライブラリを追加

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <revolution.h>
#include <revolution/kpad.h>
#include <revolution/mem.h>
#include "rexdemo/demokpad.h"

#define	 SAMPLE_PADHEAP_SIZE ( 128 * 1024 )

static MEMHeapHandle	samplePadHeap;
static void*			AllocFromPadHeap( u32 size );
static u8			   FreeToPadHeap( void* ptr );

static u16			  PadButtonPrev[PAD_MAX_CONTROLLERS];

KPADStatus  REXDEMOKPadStatus[WPAD_MAX_CONTROLLERS];
PADStatus   REXDEMOPadStatus[PAD_MAX_CONTROLLERS];


/*---------------------------------------------------------------------------*
	Name:		   REXDEMOKPadInit

	Description:	Initialize PAD library and exported status

	Arguments:	  None

	Returns:		None
 *---------------------------------------------------------------------------*/
void REXDEMOKPadInit(void)
{
	void* heapAddress;
	int   channel;

	/* Initialize heap for WPAD sampling */
	heapAddress = OSGetMEM2ArenaLo();
	OSSetMEM2ArenaLo( (void*)OSRoundUp32B( (u32)heapAddress + SAMPLE_PADHEAP_SIZE ) );
	samplePadHeap   = MEMCreateExpHeap( heapAddress, SAMPLE_PADHEAP_SIZE );
	
	if( samplePadHeap == NULL )
	{
		OSHalt( "Could not create heap.\n" );
	}
	WPADRegisterAllocator( AllocFromPadHeap, FreeToPadHeap );
	
	KPADInit();
	while( KPADRead( WPAD_CHAN0, &REXDEMOKPadStatus[0], 1 ) > 0 ) {}
	while( KPADRead( WPAD_CHAN1, &REXDEMOKPadStatus[1], 1 ) > 0 ) {}
	while( KPADRead( WPAD_CHAN2, &REXDEMOKPadStatus[2], 1 ) > 0 ) {}
	while( KPADRead( WPAD_CHAN3, &REXDEMOKPadStatus[3], 1 ) > 0 ) {}
	
	/* Initialize GameCube pads */
	(void)PADInit();
	for ( channel = 0 ; channel < PAD_MAX_CONTROLLERS ; ++channel )
	{
		PadButtonPrev[channel] = 0;
	}
}

/*---------------------------------------------------------------------------*
	Name:		   REXDEMOKPadRead

	Description:	Calls KPADRead().

	Arguments:	  None

	Returns:		None
 *---------------------------------------------------------------------------*/
void REXDEMOKPadRead( void )
{
	int   channel;
	
	(void)KPADRead( WPAD_CHAN0, &REXDEMOKPadStatus[0], 1 );
	(void)KPADRead( WPAD_CHAN1, &REXDEMOKPadStatus[1], 1 );
	(void)KPADRead( WPAD_CHAN2, &REXDEMOKPadStatus[2], 1 );
	(void)KPADRead( WPAD_CHAN3, &REXDEMOKPadStatus[3], 1 );
	
	for ( channel = 0 ; channel < PAD_MAX_CONTROLLERS ; ++channel )
	{
		PadButtonPrev[channel] = REXDEMOPadStatus[channel].button;
	}
	(void)PADRead( REXDEMOPadStatus );
	PADClamp( REXDEMOPadStatus );
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOGetAnyKPadTrigger
  Description : 全パッドチャンネルからのトリガ入力の論理和を取得。
  Arguments   : なし
  Returns	 : トリガ入力の論理和
 *---------------------------------------------------------------------------*/
u32
REXDEMOGetAnyKPadTrigger( void )
{
	u32	 retval = 0;
	int channel;
	for (channel = 0; channel < WPAD_MAX_CONTROLLERS; ++channel)
	{
		retval |= REXDEMOKPadStatus[channel].trig;
	}
	
	return retval;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOGetAnyMixedPadTrigger
  Description : 全パッドチャンネルからのトリガ入力の論理和を取得。
				(GCコントローラポートを含む)
  Arguments   : なし
  Returns	 : トリガ入力の論理和
 *---------------------------------------------------------------------------*/
u32
REXDEMOGetAnyMixedPadTrigger( void )
{
	u32	 retval = 0;
	int channel;
	for (channel = 0; channel < WPAD_MAX_CONTROLLERS; ++channel)
	{
		retval |= REXDEMOKPadStatus[channel].trig;
	}
	
	for (channel = 0; channel < PAD_MAX_CONTROLLERS; ++channel)
	{
		u16  padDown;
		padDown = PADButtonDown(PadButtonPrev[channel],
								REXDEMOPadStatus[channel].button);
		retval |= ((u32)padDown << 16);
	}
	
	return retval;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOGetAnyKPadHold
  Description : 全パッドチャンネルからの入力の論理和を取得。
  Arguments   : なし
  Returns	 : トリガ入力の論理和
 *---------------------------------------------------------------------------*/
u32
REXDEMOGetAnyKPadHold( void )
{
	u32	 retval = 0;
	int channel;
	for (channel = 0; channel < WPAD_MAX_CONTROLLERS; ++channel)
	{
		retval |= REXDEMOKPadStatus[channel].hold;
	}
	
	return retval;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOGetAnyMixedPadHold
  Description : 全パッドチャンネルからの入力の論理和を取得。
				(GCコントローラポートを含む)
  Arguments   : なし
  Returns	 : トリガ入力の論理和
 *---------------------------------------------------------------------------*/
u32
REXDEMOGetAnyMixedPadHold( void )
{
	u32	 retval = 0;
	int channel;
	for (channel = 0; channel < WPAD_MAX_CONTROLLERS; ++channel)
	{
		retval |= REXDEMOKPadStatus[channel].hold;
	}
	
	for (channel = 0; channel < PAD_MAX_CONTROLLERS; ++channel)
	{
		u16  padDown;
		padDown = REXDEMOPadStatus[channel].button;
		retval |= ((u32)padDown << 16);
	}
	
	return retval;
}

/*---------------------------------------------------------------------------*
  Name		: AllocFromPadHeap
  Description : WPAD ライブラリ用に動的にメモリを確保する。
  Arguments   : size	-   確保するメモリのサイズを指定。
  Returns	 : void*   -   確保したメモリの先頭アドレスを返す。
 *---------------------------------------------------------------------------*/
static void*
AllocFromPadHeap( u32 size )
{
	return MEMAllocFromExpHeap( samplePadHeap, size );
}

/*---------------------------------------------------------------------------*
  Name		: FreeToPadHeap
  Description : WPAD ライブラリ用に動的に確保したメモリを解放する。
  Arguments   : ptr	 -   解放するメモリの先頭アドレスを指定。
  Returns	 : u8	  -   解放に失敗した場合に 0 を返す。
 *---------------------------------------------------------------------------*/
static u8
FreeToPadHeap( void* ptr )
{
	if( ptr == NULL )
	{
		return 0;
	}
	MEMFreeToExpHeap( samplePadHeap, ptr );
	return 1;
}
