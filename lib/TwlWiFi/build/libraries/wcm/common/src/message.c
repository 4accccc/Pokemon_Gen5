/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     message.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-10#$
  $Rev: 28 $
  $Author: $
 *---------------------------------------------------------------------------*/

#include "wcm_private.h"

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/

#if WCM_DEBUG

// 警告文用テキスト雛形
const char   WCMi_WText_AlreadyInit[] = { "WCM library is already initialized.\n" };
const char   WCMi_WText_IllegalParam[] = { "Illegal parameter ( %s )\n" };
const char   WCMi_WText_NotInit[] = { "WCM library is not initialized yet.\n" };
const char   WCMi_WText_IllegalPhase[] = { "Could not accept request now. ( PHASE: %d )\n" };
const char   WCMi_WText_InvalidWmState[] = { "Invalid state of WM library. Don't use WM or WVR library while using WCM library.\n" };
const char   WCMi_WText_UnsuitableArm7[] = { "Unsuitable ARM7 component. Could not drive wireless module.\n" };
const char   WCMi_WText_WirelessForbidden[] = { "Forbidden to use wireless module." };
const char   WCMi_WText_InIrqMode[] = { "Processor mode is IRQ mode now.\n" };
const char   WCMi_WText_NoIndexData[] = { "Could not found AP information that have index %d." };

// 報告文用テキスト雛形
const char   WCMi_RText_WmSyncError[] = { "%s failed syncronously. ( ERRCODE: 0x%02x )\n" };
const char   WCMi_RText_WmAsyncError[] = { "%s failed asyncronously. ( ERRCODE: 0x%02x - 0x%04x - 0x%04x )\n" };
const char   WCMi_RText_WmDisconnected[] = { "%s succeeded asyncronously , but already disconnected from AP.\n" };
const char   WCMi_RText_InvalidAid[] = { "%s succeeded asyncronously , but given Auth-ID is invalid.\n" };
const char   WCMi_RText_SupportRateset[] = { "Failed association to \"%s\" because of illegal support rate set.\n  So retry with camouflage 5.5M and 11M automatically.\n" };

#endif

