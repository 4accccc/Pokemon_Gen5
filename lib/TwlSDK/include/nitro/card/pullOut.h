/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     pullOut.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_PULLOUT_H_
#define NITRO_CARD_PULLOUT_H_


#include <nitro/types.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

//---- callback type for card pulled out
typedef BOOL (*CARDPulledOutCallback) (void);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_InitPulledOutCallback

  Description:  set system callback for being pulled out card

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CARD_InitPulledOutCallback(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_IsPulledOut

  Description:  return whether card is pulled out

  Arguments:    None

  Returns:      TRUE if detect pulled out
 *---------------------------------------------------------------------------*/
BOOL    CARD_IsPulledOut(void);

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         CARD_SetPulledOutCallback

  Description:  set user callback for being pulled out card

  Arguments:    callback : callback

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CARD_SetPulledOutCallback(CARDPulledOutCallback callback);

/*---------------------------------------------------------------------------*
  Name:         CARD_TerminateForPulledOut

  Description:  terminate for pulling out card
                send message to do termination to ARM7

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CARD_TerminateForPulledOut(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_PulledOutCallbackProc

  Description:  callback for card pulled out

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CARD_PulledOutCallbackProc(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckPulledOut

  Description:  get whether system has detected pulled out card
                by comparing IPL cardID with current cardID
                (notice that once card pulled out, IDs are different absolutely)

  Arguments:    None

  Returns:      TRUE if current cardID is equal to IPL cardID
 *---------------------------------------------------------------------------*/
void    CARD_CheckPulledOut(void);

#endif

#ifdef SDK_ARM7

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckPullOut_Polling

  Description:  polling that card is pulled out

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CARD_CheckPullOut_Polling(void);

#endif


/*---------------------------------------------------------------------------*
 * internal functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CARDi_ResetSlotStatus

  Description:  スロットが再挿入されたことによりカード抜け状態を初期化。
                この関数は本体開発用の内部関数であり、
                通常のアプリケーションが単にこれを呼び出しただけでは
                スロットのハードウェア状態は何も初期化されない点に注意。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CARDi_ResetSlotStatus(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetSlotResetCount

  Description:  スロットが再挿入された回数を取得。
                単にCARDi_ResetSlotStatus関数を呼ばれた回数を返す。

  Arguments:    None

  Returns:      スロットが再挿入された回数。初期値は0。
 *---------------------------------------------------------------------------*/
u32     CARDi_GetSlotResetCount(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsPulledOutEx

  Description:  スロットからカードが引き抜かれたか判定。

  Arguments:    count        前回確認したスロット再挿入回数。
                             CARDi_GetSlotResetCount関数で取得する。

  Returns:      指定されたスロット再挿入回数が現在も同じままで、
                かつ現在カード抜け状態でなければTRUE。
 *---------------------------------------------------------------------------*/
BOOL    CARDi_IsPulledOutEx(u32 count);




#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_PULLOUT_H_
