/*---------------------------------------------------------------------------*
  Project:  RevolutionSDK Extension
  File:	 demokpad.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: demokpad.h,v $
  Revision 1.5  2007/10/23 13:41:53  seiki_masashi
  added REXDEMOGetAnyKPadHold

  Revision 1.4  2007/06/16 06:17:10  hirose_kazuki
  GC Pad 関連の機能追加

  Revision 1.3  2006/09/05 10:55:40  yosizaki
  added REXDEMOGetAnyKPadTrigger.

  Revision 1.2  2006/08/29 07:19:20  adachi_hiroaki
  プレフィックス変更、そのほか整理

  Revision 1.1  2006/08/10 12:09:06  adachi_hiroaki
  ヘッダファイルの位置を変更

  Revision 1.1  2006/08/09 13:28:49  yasu
  KPAD を使うためのライブラリを追加

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef DEMOS_SHARE_DEMOKPAD_H_
#define DEMOS_SHARE_DEMOKPAD_H_

#include <revolution/types.h>
#include <revolution/kpad.h>
#include <revolution/pad.h>

#ifdef __cplusplus
extern "C" {
#endif

void REXDEMOKPadInit(void);
void REXDEMOKPadRead(void);

extern KPADStatus   REXDEMOKPadStatus[WPAD_MAX_CONTROLLERS];
extern PADStatus	REXDEMOPadStatus[PAD_MAX_CONTROLLERS];

// inline functions for getting each component
static inline u32   REXDEMOKPadGetButton(u32 i)	{ return REXDEMOKPadStatus[i].hold;	 }
static inline u32   REXDEMOKPadGetButtonUp(u32 i)  { return REXDEMOKPadStatus[i].release;  }
static inline u32   REXDEMOKPadGetButtonDown(u32 i){ return REXDEMOKPadStatus[i].trig;	 }
static inline Vec2  REXDEMOKPadGetPos(u32 i)	   { return REXDEMOKPadStatus[i].pos;	  }
static inline s8	REXDEMOKPadGetErr(u32 i)	   { return REXDEMOKPadStatus[i].wpad_err; }

static inline u16   REXDEMOPadGetButton(u32 i)	 { return REXDEMOPadStatus[i].button;	}
static inline s16   REXDEMOPadGetStickX(u32 i)	 { return REXDEMOPadStatus[i].stickX;	}
static inline s16   REXDEMOPadGetStickY(u32 i)	 { return REXDEMOPadStatus[i].stickY;	}
static inline s16   REXDEMOPadGetSubStickX(u32 i)  { return REXDEMOPadStatus[i].substickX; }
static inline s16   REXDEMOPadGetSubStickY(u32 i)  { return REXDEMOPadStatus[i].substickY; }

u32 REXDEMOGetAnyKPadTrigger( void );
u32 REXDEMOGetAnyMixedPadTrigger( void );
u32 REXDEMOGetAnyKPadHold( void );
u32 REXDEMOGetAnyMixedPadHold( void );

#ifdef __cplusplus
}
#endif
#endif  /* DEMOS_SHARE_DEMOKPAD_H_ */
