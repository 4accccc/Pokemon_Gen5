/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PM
  File:     pm.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-04-21#$
  $Rev: 10443 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro/spi/ARM9/pm.h>
#include <nitro/pxi.h>
#include <nitro/gx.h>
#include <nitro/spi/common/config.h>
#include <nitro/ctrdg.h>
#include <nitro/mb.h>
#ifdef SDK_TWL
#include "../../os/common/include/application_jump_private.h"
#endif

//---------------- for IS-TWL-DEBUGGER
#ifdef          SDK_LINK_ISTD
# pragma warn_extracomma off
# include       <istdbglib.h>          // has extracomma in enum
# pragma warn_extracomma reset
#endif

//---- PM work area
typedef struct
{
	//---- lock flag
    BOOL    lock;

	//---- callback
    PMCallback callback;
    void   *callbackArg;

	//---- free work area
    void   *work;
}
PMiWork;

#define PMi_LCD_WAIT_SYS_CYCLES  0x360000
#define PMi_PXI_WAIT_TICK        10

//---- compare method
#define PMi_COMPARE_GT           0    // compare by '>'
#define PMi_COMPARE_GE           1    // compare by '>='

//----------------
inline u32 PMi_MakeData1(u32 bit, u32 seq, u32 command, u32 data)
{
    return (bit) | ((seq) << SPI_PXI_INDEX_SHIFT) | ((command) << 8) | ((data) & 0xff);
}

//----------------
inline u32 PMi_MakeData2(u32 bit, u32 seq, u32 data)
{
    return (bit) | ((seq) << SPI_PXI_INDEX_SHIFT) | ((data) & 0xffff);
}

static u32 PMi_TryToSendPxiData(u32* sendData, int num, u16* retValue, PMCallback callback, void* arg);
static void PMi_TryToSendPxiDataTillSuccess(u32* sendData, int num);

static u32 PMi_ForceToPowerOff(void);
static void PMi_CallPostExitCallbackAndReset(BOOL isExit);

//---------------- wait unlock
static void PMi_WaitBusy(void);

//---------------- dummy callback
void    PMi_DummyCallback(u32 result, void *arg);

//---------------- callback list operation
static void PMi_InsertList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info, int priority, int method);
static void PMi_DeleteList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info);
static void PMi_ClearList(PMGenCallbackInfo **listp);
static void PMi_ExecuteList(PMGenCallbackInfo *listp);

#ifdef SDK_TWL
static void PMi_FinalizeDebugger(void);
#include <twl/ltdmain_begin.h>
static void PMi_ProceedToExit(PMExitFactor factor);
static void PMi_ClearPreExitCallback(void);
static void PMi_ClearPostExitCallback(void);
#include <twl/ltdmain_end.h>
#endif

static void PMi_LCDOnAvoidReset(void);

static void PMi_WaitVBlank(void);

static PMiWork PMi_Work;

static volatile BOOL PMi_SleepEndFlag;

//---- VBlank count LCD turned off
static u32 PMi_LCDCount;
static u32 PMi_DispOffCount;

//---- callback list
static PMSleepCallbackInfo        *PMi_PreSleepCallbackList = NULL;
static PMSleepCallbackInfo        *PMi_PostSleepCallbackList = NULL;
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static PMExitCallbackInfo         *PMi_PreExitCallbackList = NULL;
static PMExitCallbackInfo         *PMi_PostExitCallbackList = NULL;
static PMBatteryLowCallbackInfo    PMi_BatteryLowCallbackInfo = {NULL,NULL,NULL};
#include <twl/ltdmain_end.h>
#endif

//---- mic power config
static u32 PMi_SetAmp(PMAmpSwitch status);
static PMAmpSwitch sAmpSwitch = PM_AMP_OFF;

//---- auto reset/shutdown flag
#ifdef SDK_TWL
static BOOL PMi_AutoExitFlag = TRUE;
#ifndef SDK_FINALROM
static BOOL PMi_ExitSequenceFlag = FALSE;
#endif
static PMExitFactor PMi_ExitFactor = PM_EXIT_FACTOR_NONE;
#endif
static u32 PMi_PreDmaCnt[4];

#define PMi_WAITBUSY_METHOD_CPUMODE   (1<<1)
#define PMi_WAITBUSY_METHOD_CPSR      (1<<2)
#define PMi_WAITBUSY_METHOD_IME       (1<<3)
static BOOL PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE;

//================================================================
//      INTERNAL FUNCTIONS
//================================================================

extern void PXIi_HandlerRecvFifoNotEmpty(void);
//----------------------------------------------------------------
//  PMi_WaitBusy
//
//       wait while locked
//
static void PMi_WaitBusy(void)
{
    volatile BOOL *p = &PMi_Work.lock;

    while (*p)
    {
        if ( (    PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_CPUMODE && OS_GetProcMode() == OS_PROCMODE_IRQ )
             || ( PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_CPSR && OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE )
             || ( PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_IME && OS_GetIrq() == OS_IME_DISABLE ) )
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}

//----------------------------------------------------------------
//  PMi_DummyCallback
//
static void PMi_DummyCallback(u32 result, void *arg)
{
	if ( arg )
	{
		*(u32 *)arg = result;
	}
}

//----------------------------------------------------------------
//   PMi_CallCallbackAndUnlock
//
static void PMi_CallCallbackAndUnlock(u32 result)
{
    PMCallback callback = PMi_Work.callback;
    void       *arg     = PMi_Work.callbackArg;

	PMi_Work.lock = FALSE;

	//---- call callback
    if (callback)
    {
        PMi_Work.callback = NULL;
        (callback) (result, arg);
    }
}

//----------------------------------------------------------------
//  PMi_WaitVBlankCount
//
static void PMi_WaitVBlank(void)
{
    vu32    vcount = OS_GetVBlankCount();
    while (vcount == OS_GetVBlankCount())
    {
    }
}

//================================================================
//      INIT
//================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_Init

  Description:  initialize PM

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_Init(void)
{
	static u16 PMi_IsInit = FALSE;

    if (PMi_IsInit)
    {
        return;
    }
    PMi_IsInit = TRUE;

    //---- initialize work area
    PMi_Work.lock = FALSE;
    PMi_Work.callback = NULL;

#ifdef SDK_TWL
    *(u32*)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_NONE;
#endif

    //---- wait for till ARM7 PXI library start
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_PM, PXI_PROC_ARM7))
    {
		SVC_WaitByLoop(100);
    }

    //---- set receive callback
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_PM, PMi_CommonCallback);

    //---- init LCD count
    PMi_LCDCount = PMi_DispOffCount = OS_GetVBlankCount();
}

//================================================================
//      PXI CALLBACK
//================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_CommonCallback

  Description:  callback to receive data from PXI

  Arguments:    tag  : tag from PXI (unused)
                data : data from PXI
                err  : error bit

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    u16     command;
    u16     pxiResult;
	BOOL    callCallback = TRUE;

    command = (u16)((data & SPI_PXI_RESULT_COMMAND_MASK) >> SPI_PXI_RESULT_COMMAND_SHIFT);
    pxiResult = (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);

    //---- check PXI communication error
    if (err)
    {
        switch ( command )
        {
        case SPI_PXI_COMMAND_PM_SLEEP_START:
        case SPI_PXI_COMMAND_PM_UTILITY:
            pxiResult = PM_RESULT_BUSY;
            break;

        default:
            pxiResult = PM_RESULT_ERROR;
        }

        PMi_CallCallbackAndUnlock(pxiResult);
        return;
    }

	switch( command )
	{
	    case SPI_PXI_COMMAND_PM_SLEEP_START:
            // do nothing
            break;

            //---- result for utility
		case SPI_PXI_COMMAND_PM_UTILITY:
            if (PMi_Work.work)
            {
                *(u16*)PMi_Work.work = (u16)pxiResult;
            }
            pxiResult = (u16)PM_RESULT_SUCCESS;
			break;

			//---- sync with arm7
		case SPI_PXI_COMMAND_PM_SYNC:
			pxiResult = (u16)PM_RESULT_SUCCESS;
            break;

			//---- end of sleep
		case SPI_PXI_COMMAND_PM_SLEEP_END:
			PMi_SleepEndFlag = TRUE;
			break;

#ifdef SDK_TWL
			// notify power-down/reset from ARM7.
		case SPI_PXI_COMMAND_PM_NOTIFY:
			switch( pxiResult )
			{
				case PM_NOTIFY_POWER_SWITCH:
					OS_TPrintf("[ARM9] Pushed power button.\n" );
					PMi_ProceedToExit(PM_EXIT_FACTOR_PWSW);
                    *(u32*)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_FORCED;
					break;

				case PM_NOTIFY_SHUTDOWN:
					OS_TPrintf("[ARM9] Shutdown\n" );
					// do nothing
					break;

				case PM_NOTIFY_RESET_HARDWARE:
					OS_TPrintf("[ARM9] Reset Hardware\n" );
					// do nothing
					break;
				case PM_NOTIFY_BATTERY_LOW:
					OS_TPrintf("[ARM9] Battery low\n" );
					if ( PMi_BatteryLowCallbackInfo.callback )
					{
						(PMi_BatteryLowCallbackInfo.callback)(PMi_BatteryLowCallbackInfo.arg);
					}
					break;
				case PM_NOTIFY_BATTERY_EMPTY:
					OS_TPrintf("[ARM9] Battery empty\n" );
					PMi_ProceedToExit(PM_EXIT_FACTOR_BATTERY);
                    *(u32*)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_FORCED;
					break;
				default:
					OS_TPrintf("[ARM9] unknown %x\n", pxiResult );
					break;
			}

			callCallback = FALSE;
			break;

#endif	/* SDK_TWL */
	}

	if ( callCallback )
	{
		PMi_CallCallbackAndUnlock(pxiResult);
	}
}

//================================================================================
//           SEND COMMAND TO ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_TryToSendPxiData

  Description:  check lock and send command data

  Arguments:    sendData  : command data array to send
                num       : number of data in array
                retValue  : return value (when success command)
                callback  : callback called in finishing proc
                arg       : argument of callback

  Returns:      PM_BUSY    ... busy (locked)
                PM_SUCCESS ... sent command data
 *---------------------------------------------------------------------------*/
static u32 PMi_TryToSendPxiData(u32* sendData, int num, u16* retValue, PMCallback callback, void* arg)
{
    int n;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check lock
    if (PMi_Work.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return PM_BUSY;
    }
    PMi_Work.lock = TRUE;

    //---- set callback
    PMi_Work.work = (void*)retValue;
    PMi_Work.callback = callback;
    PMi_Work.callbackArg = arg;

    //---- send command to ARM7
    for( n=0; n<num; n++ )
    {
        PMi_SendPxiData( sendData[n] );
    }

    (void)OS_RestoreInterrupts(enabled);
    return PM_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_TryToSendPxiDataTillSuccess

  Description:  send data, till success

  Arguments:    sendData : command data array to send
                num      : number of data in array

  Returns:      None
 *---------------------------------------------------------------------------*/
#define PMi_UNUSED_RESULT 0xffff0000  //リターン値としてありえない値
void PMi_TryToSendPxiDataTillSuccess(u32* sendData, int num)
{
    volatile u32 result;
    while(1)
    {
        result = PMi_UNUSED_RESULT;
        while( PMi_TryToSendPxiData( sendData, num, NULL, PMi_DummyCallback, (void*)&result) != PM_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }

        //---- wait for finishing command
        while(result == PMi_UNUSED_RESULT)
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
        if ( result == SPI_PXI_RESULT_SUCCESS )
        {
            break;
        }

        //---- wait
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SendSleepStart

  Description:  send SLEEP START command to ARM7

  Arguments:    trigger     : factors to wake up | backlight status to recover
                keyIntrData : key pattern and logic in waking up by key interrupt

  Returns:      result of issueing command
                always return PM_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
u32 PMi_SendSleepStart(u16 trigger, u16 keyIntrData)
{
    u32 sendData[2];

    //---- send SYNC
    sendData[0] =  PMi_MakeData1(SPI_PXI_START_BIT | SPI_PXI_END_BIT, 0, SPI_PXI_COMMAND_PM_SYNC, 0);
    PMi_TryToSendPxiDataTillSuccess( sendData, 1 );

    //---- turn LCD off
    while( PMi_SetLCDPower(PM_LCD_POWER_OFF, PM_LED_BLINK_LOW, FALSE, TRUE) != TRUE )
    {
        // no need to insert spinwait (because wait in PMi_SetLCDPower)
    }

    //---- send SLEEP_START
    sendData[0] = PMi_MakeData1(SPI_PXI_START_BIT, 0, SPI_PXI_COMMAND_PM_SLEEP_START, trigger);
    sendData[1] = PMi_MakeData2(SPI_PXI_END_BIT,   1, keyIntrData);
    PMi_TryToSendPxiDataTillSuccess( sendData, 2 );

    return PM_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SendUtilityCommandAsync / PM_SendUtilityCommand

  Description:  send utility command to ARM7

  Arguments:    number    : action number ( PM_UTIL_xxx in spi/common/pm_common.h )
                parameter : parameter for utility command
                retValue  : return value (when success command)
                callback  : callback called in finishing proc
                arg       : argument of callback

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy because other PM function uses SPI
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
u32 PM_SendUtilityCommandAsync(u32 number, u16 parameter, u16* retValue, PMCallback callback, void *arg)
{
    u32 sendData[2];

    sendData[0] = PMi_MakeData1(SPI_PXI_START_BIT, 0, SPI_PXI_COMMAND_PM_UTILITY, number);
    sendData[1] = PMi_MakeData2(SPI_PXI_END_BIT, 1, parameter);

    return PMi_TryToSendPxiData( sendData, 2, retValue, callback, arg );
}

//---------------- sync version
u32 PM_SendUtilityCommand(u32 number, u16 parameter, u16* retValue)
{
    u32     commandResult;
    u32     sendResult = PM_SendUtilityCommandAsync(number, parameter, retValue, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

//================================================================================
//           ACTION ABOUT PMIC
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SetLEDAsync / PMi_SetLED

  Description:  change LED status

  Arguments:    status : PM_LED_ON         : on
                         PM_LED_BLINK_HIGH : blinking in high speed
                         PM_LED_BLINK_LOW  : blinking in low speed
                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PMi_SetLEDAsync(PMLEDStatus status, PMCallback callback, void *arg)
{
    u32     command;

    switch (status)
    {
    case PM_LED_ON:
        command = PM_UTIL_LED_ON;
        break;
    case PM_LED_BLINK_HIGH:
        command = PM_UTIL_LED_BLINK_HIGH_SPEED;
        break;
    case PM_LED_BLINK_LOW:
        command = PM_UTIL_LED_BLINK_LOW_SPEED;
        break;
    default:
        command = 0;
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- sync version
u32 PMi_SetLED(PMLEDStatus status)
{
    u32     commandResult;
    u32     sendResult = PMi_SetLEDAsync(status, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetBackLightAsync / PM_SetBackLight

  Description:  change backlight switch

  Arguments:    targer   : target LCD.
                           PM_LCD_TOP    : top LCD
                           PM_LCD_BOTTOM : bottom LCD
                           PM_LCD_ALL    : top and bottom LCD
                sw       : switch of top LCD.
                           PM_BACKLIGHT_OFF : off
                           PM_BACKLIGHT_ON  : on
                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_INVALID_COMMAND : bad status given
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PM_SetBackLightAsync(PMLCDTarget target, PMBackLightSwitch sw, PMCallback callback, void *arg)
{
    u32     command = 0;

    if (target == PM_LCD_TOP)
    {
        if (sw == PM_BACKLIGHT_ON)
        {
            command = PM_UTIL_LCD2_BACKLIGHT_ON;
        }
        if (sw == PM_BACKLIGHT_OFF)
        {
            command = PM_UTIL_LCD2_BACKLIGHT_OFF;
        }
    }
    else if (target == PM_LCD_BOTTOM)
    {
        if (sw == PM_BACKLIGHT_ON)
        {
            command = PM_UTIL_LCD1_BACKLIGHT_ON;
        }
        if (sw == PM_BACKLIGHT_OFF)
        {
            command = PM_UTIL_LCD1_BACKLIGHT_OFF;
        }
    }
    else if (target == PM_LCD_ALL)
    {
        if (sw == PM_BACKLIGHT_ON)
        {
            command = PM_UTIL_LCD12_BACKLIGHT_ON;
        }
        if (sw == PM_BACKLIGHT_OFF)
        {
            command = PM_UTIL_LCD12_BACKLIGHT_OFF;
        }
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- sync version
u32 PM_SetBackLight(PMLCDTarget target, PMBackLightSwitch sw)
{
    u32     commandResult;
    u32     sendResult = PM_SetBackLightAsync(target, sw, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SetSoundPowerAsync / PMi_SetSoundPower

  Description:  change sound power switch

  Arguments:    sw       : switch of sound power
                          PM_SOUND_POWER_OFF : off
                          PM_SOUND_POWER_ON  : on
                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_INVALID_COMMAND : bad status given
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PMi_SetSoundPowerAsync(PMSoundPowerSwitch sw, PMCallback callback, void *arg)
{
    u32     command;

    switch (sw)
    {
    case PM_SOUND_POWER_ON:
        command = PM_UTIL_SOUND_POWER_ON;
        break;
    case PM_SOUND_POWER_OFF:
        command = PM_UTIL_SOUND_POWER_OFF;
        break;
    default:
        command = 0;
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- sync version
u32 PMi_SetSoundPower(PMSoundPowerSwitch sw)
{
    u32     commandResult;
    u32     sendResult = PMi_SetSoundPowerAsync(sw, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SetSoundVolumeAsync / PMi_SetSoundVolume

  Description:  change sound volume control switch

  Arguments:    sw       : switch of sound volume control
                          PM_SOUND_VOLUME_ON  : on
                          PM_SOUND_VOLUME_OFF : off
                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_INVALID_COMMAND : bad status given
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PMi_SetSoundVolumeAsync(PMSoundVolumeSwitch sw, PMCallback callback, void *arg)
{
    u32     command;

    switch (sw)
    {
    case PM_SOUND_VOLUME_ON:
        command = PM_UTIL_SOUND_VOL_CTRL_ON;
        break;
    case PM_SOUND_VOLUME_OFF:
        command = PM_UTIL_SOUND_VOL_CTRL_OFF;
        break;
    default:
        command = 0;
    }

    return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg) : PM_INVALID_COMMAND;
}

//---------------- sync version
u32 PMi_SetSoundVolume(PMSoundVolumeSwitch sw)
{
    u32     commandResult;
    u32     sendResult = PMi_SetSoundVolumeAsync(sw, PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusy();
        return commandResult;
    }
    return sendResult;
}

/*---------------------------------------------------------------------------*
  Name:         PM_ForceToPowerOffAsync / PMi_ForceToPowerOff / PM_ForceToPowerOff

  Description:  force to turn off main power

  Arguments:    callback : callback function
                arg      : callback argument

  Returns:      (PM_ForceToPowerOffAsync / PMi_ForceToPowerOff)
                result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI

                (PM_ForceToPowerOff)
                PM_RESULT_SUCCESS  : success to exec command

                If success, maybe you cannot do anything because power if off.
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PM_ForceToPowerOffAsync(PMCallback callback, void *arg)
{
#ifdef SDK_TWL
	//---- set exit factor
	PMi_ExitFactor = PM_EXIT_FACTOR_USER;
#endif

	//---- force to set LCD on avoid PMIC bug
	PMi_LCDOnAvoidReset();

#ifdef SDK_TWL
	if ( OS_IsRunOnTwl() )
	{
		//---- call exit callbacks
		PMi_ExecuteList(PMi_PostExitCallbackList);
	}
#endif

    return PM_SendUtilityCommandAsync(PM_UTIL_FORCE_POWER_OFF, 0, NULL, callback, arg);
}

//---------------- sync version
static u32 PMi_ForceToPowerOff(void)
{
    u32     commandResult;
    u32     sendResult = PM_ForceToPowerOffAsync(PMi_DummyCallback, &commandResult);
    if (sendResult == PM_SUCCESS)
    {
        PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
        PMi_WaitBusy();
        PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE;
        return commandResult;
    }
    return sendResult;
}
//---------------- wait version
u32 PM_ForceToPowerOff(void)
{
    while( PMi_ForceToPowerOff() != PM_RESULT_SUCCESS )
    {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    (void)OS_DisableInterrupts();

    //---- stop all dma
    MI_StopAllDma();
#ifdef SDK_TWL
	if ( OS_IsRunOnTwl() )
	{
		MI_StopAllNDma();
	}
#endif

    //---- Loop intentionally
    while (1)
    {
        OS_Halt();
    }

    //---- just avoid warning
    return PM_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpAsync / PM_SetAmp

  Description:  switch amp

  Arguments:    sw       : switch of programable gain amp
                          PM_AMP_ON   : on
                          PM_AMP_OFF  : off
                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PM_SetAmpAsync(PMAmpSwitch status, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMP, (u16)status, NULL, callback, arg );
}

//---------------- sync version
u32 PM_SetAmp(PMAmpSwitch status)
{
    //---- remember mic power flag
    sAmpSwitch = status;
    return PMi_SetAmp(status);
}

//---------------- sync version
static u32 PMi_SetAmp(PMAmpSwitch status)
{
    if (PM_GetLCDPower())
    {
		//---- change amp status in case of LCD-ON only
		return PM_SendUtilityCommand(PM_UTIL_SET_AMP, (u16)status, NULL );
	}
	else
	{
		return PM_RESULT_SUCCESS;
	}
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpGainAsync / PM_SetAmpGain

  Description:  change amp gain

  Arguments:    gain     : gain
                          PM_AMPGAIN_20   : gain=20
                          PM_AMPGAIN_40   : gain=40
                          PM_AMPGAIN_80   : gain=80
                          PM_AMPGAIN_160  : gain=160
                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PM_SetAmpGainAsync(PMAmpGain gain, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMPGAIN, (u16)gain, NULL, callback, arg );
}

//---------------- sync version
u32 PM_SetAmpGain(PMAmpGain gain)
{
	return PM_SendUtilityCommand(PM_UTIL_SET_AMPGAIN, (u16)gain, NULL );
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PM_SetAmpGainLevelAsync / PM_SetAmpGainLevel

  Description:  change amp gain

  Arguments:    level    : gain. 0 - 119 (0dB - 59.5dB)
                           (119 is defined as PM_AMPGAIN_LEVEL_MAX)

                callback : callback function
                arg      : callback argument

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PM_SetAmpGainLevelAsync(u8 level, PMCallback callback, void *arg)
{
	SDK_ASSERT( level <= PM_AMPGAIN_LEVEL_MAX );
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMPGAIN_LEVEL, (u16)level, NULL, callback, arg );
}

//---------------- sync version
u32 PM_SetAmpGainLevel(u8 level)
{
	SDK_ASSERT( level <= PM_AMPGAIN_LEVEL_MAX );
	return PM_SendUtilityCommand(PM_UTIL_SET_AMPGAIN_LEVEL, (u16)level, NULL );
}
#endif

//================================================================================
//           GET STATUS FROM ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_GetBattery

  Description:  get battery status

  Arguments:    batteryBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_GetBattery(PMBattery *batteryBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BATTERY, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (batteryBuf)
		{
			*batteryBuf = status? PM_BATTERY_LOW: PM_BATTERY_HIGH;
		}
	}
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetBatteryLevel

  Description:  get battery level

  Arguments:    levelBuf  : buffer to store result
                     the value range is from PM_BATTERY_LEVEL_MIN to PM_BATTERY_LEVEL_MAX.

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
                PM_RESULT_ERROR    : cannot use this function (running NITRO mode)
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static u32 PMi_GetBatteryLevelCore(PMBatteryLevel *levelBuf)
{
    u16 status;
    u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BATTERY_LEVEL, &status);

    if ( result == PM_RESULT_SUCCESS )
    {
        if (levelBuf)
        {
            *levelBuf = (PMBatteryLevel)status;
        }
    }
    return result;
}
#include <twl/ltdmain_end.h>

u32 PM_GetBatteryLevel(PMBatteryLevel *levelBuf)
{
	if ( OS_IsRunOnTwl() )
	{
		return PMi_GetBatteryLevelCore(levelBuf);
	}
	else
	{
		return PM_RESULT_ERROR;
	}
}

#endif

/*---------------------------------------------------------------------------*
  Name:         PM_GetACAdapter

  Description:  check if AC power adaptor is connected

  Arguments:    isConnectedBuf  : buffer to store result
                       TRUE  ... connected
                       FALSE ... not connected

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
                PM_RESULT_ERROR    : cannot use this function (running NITRO mode)
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL

#include <twl/ltdmain_begin.h>
static u32 PMi_GetACAdapterCore(BOOL *isConnectedBuf)
{
    u16 status;
    u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AC_ADAPTER, &status);

    if ( result == PM_RESULT_SUCCESS )
    {
        if (isConnectedBuf)
        {
            *isConnectedBuf = status? TRUE: FALSE;
        }
    }
    return result;
}
#include <twl/ltdmain_end.h>

u32 PM_GetACAdapter(BOOL *isConnectedBuf)
{
	if ( OS_IsRunOnTwl() )
	{
		return PMi_GetACAdapterCore(isConnectedBuf);
	}
	else
	{
		return PM_RESULT_ERROR;
	}
}
#endif

/*---------------------------------------------------------------------------*
  Name:         PM_GetBackLight

  Description:  get backlight status

  Arguments:    top    : buffer to set result of top LCD
                bottom : buffer to set result of bottom LCD

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_GetBackLight(PMBackLightSwitch *top, PMBackLightSwitch *bottom)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BACKLIGHT, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (top)
		{
            *top = (status & PMIC_CTL_BKLT2) ? PM_BACKLIGHT_ON : PM_BACKLIGHT_OFF;
		}
		if (bottom)
		{
            *bottom = (status & PMIC_CTL_BKLT1) ? PM_BACKLIGHT_ON : PM_BACKLIGHT_OFF;		
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetSoundPower

  Description:  get status of sound power switch

  Arguments:    swBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_ERROR    : some error occurred in ARM7
 *---------------------------------------------------------------------------*/
u32 PMi_GetSoundPower(PMSoundPowerSwitch *swBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_SOUND_POWER, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (swBuf)
		{
            *swBuf = status? PM_SOUND_POWER_ON : PM_SOUND_POWER_OFF;
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetSoundVolume

  Description:  get status of sound volume control switch

  Arguments:    swBuf : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_ERROR    : some error occurred in ARM7
 *---------------------------------------------------------------------------*/
u32 PMi_GetSoundVolume(PMSoundVolumeSwitch *swBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_SOUND_VOLUME, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (swBuf)
		{
			*swBuf = status? PM_SOUND_VOLUME_ON : PM_SOUND_VOLUME_OFF;
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetAmp

  Description:  get status of amp switch

  Arguments:    swBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_GetAmp(PMAmpSwitch *swBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMP, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (swBuf)
		{
			*swBuf = status? PM_AMP_ON: PM_AMP_OFF;
		}
	}
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetAmpGain

  Description:  get status of amp gain

  Arguments:    gainBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_GetAmpGain(PMAmpGain *gainBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMPGAIN, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (gainBuf)
		{
			*gainBuf = (PMAmpGain)status;
		}
	}
    return result;
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PM_GetAmpGainLevel

  Description:  get level of amp gain

  Arguments:    levelBuf  : buffer to store result

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_BUSY     : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
u32 PM_GetAmpGainLevel(u8 *levelBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMPGAIN_LEVEL, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (levelBuf)
		{
			*levelBuf = (u8)status;
		}
	}
	return result;
}
#endif

//================================================================================
//           WIRELESS LED
//================================================================================
#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         PMi_SetWirelessLED

  Description:  get status of amp gain

  Arguments:    sw : PM_WIRELESS_LED_ON  ... ON
                     PM_WIRELESS_LED_OFF ... OFF

  Returns:      result.
                PM_RESULT_SUCCESS  : success to exec command
                PM_RESULT_ERROR    : some error occurred in ARM7
 *---------------------------------------------------------------------------*/
u32 PMi_SetWirelessLED( PMWirelessLEDStatus sw )
{
	return PM_SendUtilityCommand( PM_UTIL_WIRELESS_LED, (u16)sw, NULL );
}
#endif  //ifdef SDK_TWL

//================================================================================
//           SEND DATA TO ARM7
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SendPxiData

  Description:  send data via PXI

  Arguments:    data : data to send

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_SendPxiData(u32 data)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_PM, data, FALSE) != PXI_FIFO_SUCCESS)
    {
        // do nothing
    }
}

//================================================================================
//           SLEEP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_PreSleepForDma

  Description:  get ready DMA to sleep mode

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static void PMi_PreSleepForNdma(u32 i)
{
    vu32 *ndmaCntp = (vu32*)MI_NDMA_REGADDR(i, MI_NDMA_REG_CNT_WOFFSET);
    if ( *ndmaCntp & MI_NDMA_IMM_MODE_ON )
    {
        // 新DMAの通常起動時はコンティニュアスモード設定は無効
        MI_WaitNDma(i);
        return;
    }
    switch ( *ndmaCntp & MI_NDMA_TIMING_MASK )
    {
        case MI_NDMA_TIMING_CARD:
            // カードDMAは通常コンティニュアスモード
            // メモリコントローラの停止を待てばDMAは再起動しない
            while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK)
            {
            }
            break;
        case MI_NDMA_TIMING_DISP_MMEM:
        case MI_NDMA_TIMING_DISP:
            // メインメモリ表示DMAと表示同期DMAはコンティニュアスモードでも
            // 1画面分のDMA完了後に停止する
            MI_WaitNDma(i);
        case MI_NDMA_TIMING_CAMERA:
            // カメラDMAはカメラがスタンバイモードに入ると
            // 待っても完了しなくなるため強制停止
            //   CAMERA_DmaPipeInfinityはコンティニュアスモード
            //   CAMERA_DmaRecv*は通常モード
            MI_StopNDma(i);
            break;
        default:
            // コンティニュアスモードでなければDMA完了待ち
            if ( ! (*ndmaCntp & MI_NDMA_CONTINUOUS_ON) )
            {
                MI_WaitNDma(i);
            }
            // コンティニュアスモードならDMA強制停止
            // MI_NDMA_TIMING_TIMER0-3（勝手に再開しても最初からになるためユーザの意図と異なってしまうはず）
            // MI_NDMA_TIMING_V_BLANK（公開関数ではMI_DMA_CONTINUOUS_ONは未使用）
            // MI_NDMA_TIMING_H_BLANK（公開関数でMI_DMA_CONTINUOUS_ONを使用しているが放置するとバッファオーバーラン）
            // MI_NDMA_TIMING_GXFIFO（公開関数ではMI_DMA_CONTINUOUS_ONは未使用）
            else
            {
                MI_StopNDma(i);
                SDK_WARNING( FALSE, "[ARM9] Force to stop NDMA%d before sleep.", i );
            }
            break;
    }
}
#include <twl/ltdmain_end.h>
#endif
static void PMi_PreSleepForDma(void)
{
    u32 i;

    for (i=0; i<=MI_DMA_MAX_NUM; i++)
    {
        // 旧DMA
        {
            vu32 *dmaCntp = (vu32*)MI_DMA_REGADDR(i, MI_DMA_REG_CNT_WOFFSET);

            PMi_PreDmaCnt[i] = *dmaCntp;

            switch ( *dmaCntp & MI_DMA_TIMING_MASK )
            {
                case MI_DMA_TIMING_CARD:
                    // カードDMAは通常コンティニュアスモード
                    // メモリコントローラの停止を待てばDMAは再起動しない
                    while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK)
                    {
                    }
                    break;
                case MI_DMA_TIMING_DISP_MMEM:
                case MI_DMA_TIMING_DISP:
                    // メインメモリ表示DMAと表示同期DMAはコンティニュアスモードでも
                    // 1画面分のDMA完了後に停止する
                    MI_WaitDma(i);
                    break;
                default:
                    // コンティニュアスモードでなければDMA完了待ち
                    if ( ! (*dmaCntp & MI_DMA_CONTINUOUS_ON) )
                    {
                        MI_WaitDma(i);
                    }
                    // コンティニュアスモードならDMA強制停止
                    // MI_DMA_TIMING_IMM（MI_DMA_CONTINUOUS_ONと併用するとCPUが動作できずに暴走）
                    // MI_DMA_TIMING_V_BLANK（公開関数ではMI_DMA_CONTINUOUS_ONは未使用）
                    // MI_DMA_TIMING_H_BLANK（公開関数でMI_DMA_CONTINUOUS_ONを使用しているが放置するとバッファオーバーラン）
                    // MI_DMA_TIMING_CARTRIDGE（MI_DMA_CONTINUOUS_ONを使用してもスリープ中のDMA要求がキャンセルされるため再設定が必要）
                    // MI_DMA_TIMING_GXFIFO（公開関数ではMI_DMA_CONTINUOUS_ONは未使用）
                    else
                    {
                        MI_StopDma(i);
                        SDK_WARNING( FALSE, "[ARM9] Force to stop DMA%d before sleep.", i );
                    }
                    break;
            }
        }
#ifdef SDK_TWL
        // 新DMA
        if (OS_IsRunOnTwl())
        {
            PMi_PreSleepForNdma(i);
        }
#endif // SDK_TWL
    }
}
/*---------------------------------------------------------------------------*
  Name:         PMi_PostSleepForDma

  Description:  resume DMA from sleep mode

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static void PMi_PostSleepForNdma(u32 i)
{
    vu32 *ndmaCntp = (vu32*)MI_NDMA_REGADDR(i, MI_NDMA_REG_CNT_WOFFSET);
    if ( *ndmaCntp & MI_NDMA_IMM_MODE_ON )
    {
        // 新DMAの通常起動時はコンティニュアスモード設定は無効
        return;
    }
    switch ( *ndmaCntp & MI_NDMA_TIMING_MASK )
    {
        case MI_NDMA_TIMING_V_BLANK:
        case MI_NDMA_TIMING_H_BLANK:
            // コンティニュアスモードならDMA再起動（テスト用）
            if ( *ndmaCntp & MI_NDMA_CONTINUOUS_ON )
            {
                *ndmaCntp |= MI_NDMA_ENABLE;
            }
            break;
    }
}
#include <twl/ltdmain_end.h>
#endif
static void PMi_PostSleepForDma(void)
{
    u32 i;

    for (i=0; i<=MI_DMA_MAX_NUM; i++)
    {
        // 旧DMA
        {
            vu32 *dmaCntp = (vu32*)MI_DMA_REGADDR(i, MI_DMA_REG_CNT_WOFFSET);
            u32 preCnt = PMi_PreDmaCnt[i];

            switch ( preCnt & MI_DMA_TIMING_MASK )
            {
                case MI_DMA_TIMING_V_BLANK:
                case MI_DMA_TIMING_H_BLANK:
                case MI_DMA_TIMING_CARTRIDGE:
                    // コンティニュアスモードならDMA再起動（テスト用）
                    if ( preCnt & MI_DMA_CONTINUOUS_ON )
                    {
                        *dmaCntp = preCnt;
                    }
                    break;
            }
        }
#ifdef SDK_TWL
        // 新DMA
        if (OS_IsRunOnTwl())
        {
            PMi_PostSleepForNdma(i);
        }
#endif // SDK_TWL
    }
}

/*---------------------------------------------------------------------------*
  Name:         PM_GoSleepMode

  Description:  go to be in sleep mode

  Arguments:    trigger    : factors to return from being on sleep
                logic      : key logic to key interrupt
                             PM_PAD_LOGIC_AND : occur interrupt at all specified button pushed
                             PM_PAD_LOGIC_OR  : occur interrupt at one of specified buttons pushed
                keyPattern : keys to wakeup

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_GoSleepMode(PMWakeUpTrigger trigger, PMLogic logic, u16 keyPattern)
{
    BOOL    prepIrq;                   // IME
    OSIntrMode prepIntrMode;           // CPSR-IRQ
    OSIrqMask prepIntrMask;            // IE
    BOOL    powerOffFlag = FALSE;

    PMBackLightSwitch preTop;
    PMBackLightSwitch preBottom;

    u32     preGX;
    u32     preGXS;
    PMLCDPower preLCDPower;

    //---- call pre-callbacks
    PMi_ExecuteList(PMi_PreSleepCallbackList);

    //---- disable all interrupt
    prepIrq = OS_DisableIrq();
    prepIntrMode = OS_DisableInterrupts();
    prepIntrMask = OS_DisableIrqMask( OS_IE_MASK_ALL );

    //---- interrupt setting
	{
		// enable PXI from ARM7 and TIMER0(if needed)
		OSIntrMode intr = OS_IE_FIFO_RECV | (OS_IsTickAvailable()? OS_IE_TIMER0: 0 );
		(void)OS_SetIrqMask( intr );
	}
    (void)OS_RestoreInterrupts(prepIntrMode);
    (void)OS_EnableIrq();

    //---- check card trigger
    if (trigger & PM_TRIGGER_CARD)
    {
		OSBootType type = OS_GetBootType();
		//---- if multiboot child or nand application, ignore card check flag
		if ( type == OS_BOOTTYPE_DOWNLOAD_MB || type == OS_BOOTTYPE_NAND )
		{
            trigger &= ~PM_TRIGGER_CARD;
		}
    }

    //---- check cartridge trigger
    if (trigger & PM_TRIGGER_CARTRIDGE)
    {
        //---- if running on TWL or cartridge not exist, ignore cartridge check flag
        if ( OS_IsRunOnTwl() || !CTRDG_IsExisting() )
        {
            trigger &= ~PM_TRIGGER_CARTRIDGE;
        }
    }

    //---- remember gx state
    preGX = reg_GX_DISPCNT;
    preGXS = reg_GXS_DB_DISPCNT;
    preLCDPower = PM_GetLCDPower();

    //---- set backlight off
    while( PM_GetBackLight(&preTop, &preBottom) != PM_RESULT_SUCCESS )
    {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }
    while( PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF) != PM_RESULT_SUCCESS )
    {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    //---- wait a few frames after backlight off for avoiding appearance of afterimage
    PMi_WaitVBlank();

    reg_GX_DISPCNT = reg_GX_DISPCNT & ~REG_GX_DISPCNT_MODE_MASK;    // main screen off
    GXS_DispOff();

    PMi_WaitVBlank();
    PMi_WaitVBlank(); // twice.

    PMi_PreSleepForDma();

    //---- send SLEEP_START command to ARM7
	PMi_SleepEndFlag = FALSE;
    {
        u16     param = (u16)(trigger
							  | (preTop? PM_BACKLIGHT_RECOVER_TOP_ON: PM_BACKLIGHT_RECOVER_TOP_OFF)
							  | (preBottom? PM_BACKLIGHT_RECOVER_BOTTOM_ON: PM_BACKLIGHT_RECOVER_BOTTOM_OFF));

        (void)PMi_SendSleepStart(param, (u16)(logic | keyPattern));
    }

    //==== Halt ================
	while( ! PMi_SleepEndFlag )
	{
		OS_Halt();
	}
    //==========================

    //---- check card remove
    if ((trigger & PM_TRIGGER_CARD) && (OS_GetRequestIrqMask() & OS_IE_CARD_IREQ))
    {
        powerOffFlag = TRUE;
    }

    //---- turn LCD on, and restore gx state
    if (!powerOffFlag)
    {
        if (preLCDPower == PM_LCD_POWER_ON)
        {
            while( PMi_SetLCDPower(PM_LCD_POWER_ON, PM_LED_ON, TRUE, TRUE) != TRUE )
            {
                // no need to insert spinwait (because wait in PMi_SetLCDPower)
            }
        }
        else
        {
            while( PMi_SetLED(PM_LED_ON) != PM_RESULT_SUCCESS )
            {
                OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
            }
        }

        reg_GX_DISPCNT = preGX;
        reg_GXS_DB_DISPCNT = preGXS;
    }
    
	//---- wait while specified period for LCD (0x360000sysClock == about 106ms)
    OS_SpinWaitSysCycles(PMi_LCD_WAIT_SYS_CYCLES);

    //---- restore all interrupt
    (void)OS_DisableInterrupts();
    (void)OS_SetIrqMask(prepIntrMask);
    (void)OS_RestoreInterrupts(prepIntrMode);
    (void)OS_RestoreIrq(prepIrq);

    //---- power off if need
    if (powerOffFlag)
    {
        (void)PM_ForceToPowerOff();
    }

    //---- call post-callbacks
    PMi_ExecuteList(PMi_PostSleepCallbackList);
}

//================================================================================
//             LCD
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SetLCDPower

  Description:  turn on/off LCD power

  Arguments:    sw : switch
                   PM_LCD_POWER_ON : on
                   PM_LCD_POWER_ON : off
                led : LED status
                   PM_LED_NONE : not touch
                   PM_LED_ON
                   PM_LED_BLINK_HIGH
                   PM_LED_BLINK_LOW
                skip : whether skip wait
                   TRUE  : skip
                   FALSE : wait
                isSync: whether this function is sync version
                   TRUE  : sync
                   FALSE : async

  Returns:      TRUE   if success
                FALSE  if fail
 *---------------------------------------------------------------------------*/
#define PMi_WAIT_FRAME_AFTER_LCDOFF  7
#define PMi_WAIT_FRAME_AFTER_GXDISP  2

BOOL PMi_SetLCDPower(PMLCDPower sw, PMLEDStatus led, BOOL skip, BOOL isSync)
{
    switch (sw)
    {
    case PM_LCD_POWER_ON:
        // compare with counter in which LCD power was turned off.
        //     the reason of waiting 100ms: The interval switching LCD power
        //     from OFF to ON must be more than 100ms. if short than that,
        //     PMIC will be shutdowned and never recover oneself.
        if (!skip && OS_GetVBlankCount() - PMi_LCDCount <= PMi_WAIT_FRAME_AFTER_LCDOFF)
        {
            return FALSE;
        }

        //---- LED
        if (led != PM_LED_NONE)
        {
            if (isSync)
            {
                while( PMi_SetLED(led) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
            else
            {
                while( PMi_SetLEDAsync(led, NULL, NULL) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
        }

        (void)GXi_PowerLCD(TRUE);

		//---- recover mic power flag
		while( PMi_SetAmp(sAmpSwitch) != PM_RESULT_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
        break;

    case PM_LCD_POWER_OFF:
		//---- force to turn mic power off
        while( PMi_SetAmp(PM_AMP_OFF) != PM_RESULT_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }

        //---- need 2 VBlank after GX_DispOff()
        if ( OS_GetVBlankCount() - PMi_DispOffCount <= PMi_WAIT_FRAME_AFTER_GXDISP )
        {
            PMi_WaitVBlank();
            PMi_WaitVBlank(); // twice.
        }

        //---- LCD power off
        (void)GXi_PowerLCD(FALSE);

        //---- remember LCD off timing
        PMi_LCDCount = OS_GetVBlankCount();

        //---- LED
        if (led != PM_LED_NONE)
        {
            if (isSync)
            {
                while( PMi_SetLED(led) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
            else
            {
                while( PMi_SetLEDAsync(led, NULL, NULL) != PM_RESULT_SUCCESS )
                {
                    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
                }
            }
        }
        break;

    default:
        // do nothing
        break;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetLCDPower

  Description:  turn on/off LCD power.
                when undefined SDK_NO_THREAD (= thread is valid),
                Tick system and Alarm system are needed.

                this function is sync version.

  Arguments:    sw : switch
                   PM_LCD_POWER_ON  : on
                   PM_LCD_POWER_OFF : off

  Returns:      TRUE   if success
                FALSE  if fail. maybe interval of LCD off->on is too short.
 *---------------------------------------------------------------------------*/
BOOL PM_SetLCDPower(PMLCDPower sw)
{
    if (sw != PM_LCD_POWER_ON)
    {
        sw = PM_LCD_POWER_OFF;
        if (GX_IsDispOn()) // LCD OFF する場合は確実にGX_DispOffの状態にしておく
        {
            GX_DispOff();
        }
    }
    return PMi_SetLCDPower(sw, PM_LED_NONE /*not touch */ , FALSE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetLCDPower

  Description:  get status of LCD power

  Arguments:    None

  Returns:      status.
                PM_LCD_POWER_ON  : on
                PM_LCD_POWER_OFF : off
 *---------------------------------------------------------------------------*/
PMLCDPower PM_GetLCDPower(void)
{
    return (reg_GX_POWCNT & REG_GX_POWCNT_LCD_MASK) ? PM_LCD_POWER_ON : PM_LCD_POWER_OFF;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetLCDOffCount

  Description:  get counter value LCD turned off

  Arguments:    None

  Returns:      counter value
 *---------------------------------------------------------------------------*/
u32 PMi_GetLCDOffCount(void)
{
    return PMi_LCDCount;
}


//================================================================================
//             LED
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SendLEDPatternCommandAsync

  Description:  set up LED blink pattern

  Arguments:    pattern  : LED blink pattern
                callback : callback function
                arg      : callback argument

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PMi_SendLEDPatternCommandAsync(PMLEDPattern pattern, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_SET_BLINK, pattern, NULL, callback, arg);	
}

//---------------- sync version
u32 PMi_SendLEDPatternCommand(PMLEDPattern pattern)
{
	return PM_SendUtilityCommand(PM_UTIL_SET_BLINK, pattern, NULL);	
}

/*---------------------------------------------------------------------------*
  Name:         PMi_GetLEDPattern

  Description:  get current LED blink pattern

  Arguments:    pattern  : LED blink pattern
                callback : callback function
                arg      : callback argument

  Returns:      result of issueing command
                PM_RESULT_SUCCESS : success
                PM_RESULT_BUSY    : busy because other device or other PM function uses SPI
 *---------------------------------------------------------------------------*/
//---------------- async version
u32 PM_GetLEDPatternAsync(PMLEDPattern *patternBuf, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BLINK, (u16*)&patternBuf, callback, arg);
}

//---------------- sync version
u32 PM_GetLEDPattern(PMLEDPattern *patternBuf)
{
	u16 status;
	u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BLINK, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (patternBuf)
		{
			*patternBuf = (PMLEDPattern)status;
		}
	}
    return result;
}

//================================================================================
//             CALLBACK LIST OPERATION (general)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_InsertList

  Description:  insert callback to callback info list as specified priority.
                subroutine of PM_Add*CallbackInfo().

  Arguments:    listp    : callback info list pointer
                info     : callback info to delete
                priority : priority
                method   : comparison operator

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_InsertList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info, int priority, int method)
{
	OSIntrMode intr;
	PMGenCallbackInfo *p;
	PMGenCallbackInfo *pre;

	if (!listp)
	{
		return;
	}

	info->priority = priority;

	intr = OS_DisableInterrupts();
	p = *listp;
	pre = NULL;

	while (p)
	{
		//---- found the position to insert
		if (method == PMi_COMPARE_GT && p->priority > priority )
		{
			break;
		}
		if (method == PMi_COMPARE_GE && p->priority >= priority )
		{
			break;
		}

		pre = p;
		p = p->next;
	}

	if ( p )
	{
		info->next = p;
	}
	else
	{
		info->next = NULL;
	}


	if ( pre )
	{
		pre->next = info;
	}
	else
	{
		//---- add to top
		*listp = info;
	}

    (void)OS_RestoreInterrupts(intr);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_DeleteList

  Description:  delete callback info from callback info list.
                subroutine of PM_Delete*CallbackInfo().

  Arguments:    listp : callback info list pointer
                info  : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_DeleteList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info)
{
    OSIntrMode  intr;
    PMGenCallbackInfo *p = *listp;
    PMGenCallbackInfo *pre;

    if (!listp)
    {
        return;
    }

    intr = OS_DisableInterrupts();
    pre = p = *listp;
    while (p)
    {
        //---- one to delete?
        if (p == info)
        {
            if (p == pre)
            {
                *listp = p->next;
            }
            else
            {
                pre->next = p->next;
            }
            break;
        }

        pre = p;
        p = p->next;
    }
    (void)OS_RestoreInterrupts(intr);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ClearList

  Description:  clear callback list

  Arguments:    listp : callback info list pointer

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_ClearList(PMGenCallbackInfo **listp)
{
	listp = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecuteList

  Description:  execute each callback registerd to info list

  Arguments:    listp : callback info list pointer

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_ExecuteList(PMGenCallbackInfo *listp)
{
    while (listp)
    {
        (listp->callback) (listp->arg);

        listp = listp->next;
    }
}

//================================================================================
//             SLEEP CALLBACK
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_AppendPreSleepCallback

  Description:  append callback info to pre-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPreSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PreSleepCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPreSleepCallback

  Description:  prepend callback info to pre-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPreSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PreSleepCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPostSleepCallback

  Description:  append callback info to post-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPostSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PostSleepCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPostSleepCallback

  Description:  prepend callback info to post-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPostSleepCallback(PMSleepCallbackInfo *info)
{
	PMi_InsertList(&PMi_PostSleepCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPreSleepCallback

  Description:  insert callback info to post-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_InsertPreSleepCallbackEx(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PreSleepCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPreSleepCallback(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPreSleepCallbackEx(info, priority);
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPostSleepCallback

  Description:  insert callback info to post-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_InsertPostSleepCallbackEx(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PostSleepCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPostSleepCallback(PMSleepCallbackInfo *info, int priority)
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPostSleepCallbackEx(info, priority);
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePreSleepCallback

  Description:  delete callback info from pre-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePreSleepCallback(PMSleepCallbackInfo *info)
{
    PMi_DeleteList(&PMi_PreSleepCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePostSleepCallback

  Description:  delete callback info from post-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePostSleepCallback(PMSleepCallbackInfo *info)
{
    PMi_DeleteList(&PMi_PostSleepCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPreSleepCallback

  Description:  clear pre-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ClearPreSleepCallback(void)
{
    PMi_ClearList(&PMi_PreSleepCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PM_ClearPostSleepCallback

  Description:  clear post-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ClearPostSleepCallback(void)
{
    PMi_ClearList(&PMi_PostSleepCallbackList);
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
//================================================================================
//             SHUTDOWN / RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_ProceedToExit

  Description:  run before shutdown/reset hardware

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_ProceedToExit(PMExitFactor factor)
{
    //---- check if reset already
    if ( ! PMi_TryLockForReset() )
    {
        return;
    }

	//---- set exit factor
	PMi_ExitFactor = factor;

#ifndef SDK_FINALROM
	//---- declare to enter exit sequence
	PMi_ExitSequenceFlag = TRUE;
#endif

	//---- call exit callbacks
    PMi_ExecuteList(PMi_PreExitCallbackList);

	if ( PMi_AutoExitFlag )
	{
		//---- tell ARM7 that ARM9 is ready to shutdown
		PM_ReadyToExit();
		// not reached here
	}
}

/*---------------------------------------------------------------------------*
  Name:         PM_ReadyToExit

  Description:  tell ARM7 that ARM9 is ready to exit

  Arguments:    None

  Returns:      None
                (This function will never return.)
 *---------------------------------------------------------------------------*/
void PM_ReadyToExit(void)
{
#ifndef SDK_FINALROM
	SDK_ASSERT( PMi_ExitSequenceFlag == TRUE );
#endif

    PMi_CallPostExitCallbackAndReset(TRUE);
    //never reached hear
}

/*---------------------------------------------------------------------------*
  Name:         PMi_FinalizeDebugger

  Description:  finalize debugger

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_FinalizeDebugger(void)
{
    //---- sync with ARM7
    OSi_SetSyncValue( OSi_SYNCVAL_NOT_READY );
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_SENDER, (void*)HW_INIT_LOCK_BUF );
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_RECVER, (void*)HW_INIT_LOCK_BUF );

    (void)OS_DisableInterrupts();

#ifndef SDK_FINALROM
    //---- finalize debugger for TS board
    if ( OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER )
    {
        _ISTDbgLib_OnBeforeResetHard();
    }
#endif

	//---- sync with ARM7
	OSi_SetSyncValue( OSi_SYNCVAL_READY );
}

/*---------------------------------------------------------------------------*
  Name:         PM_GetExitFactor

  Description:  get a factor which caused to exit

  Arguments:    None

  Returns:      factor :  PM_EXIT_FACTOR_PWSW    ... pushed power switch
                          PM_EXIT_FACTOR_BATTERY ... battery low
                          PM_EXIT_FACTOR_USER    ... called PM_ForceToResetHardware
                          PM_EXIT_FACTOR_NONE    ... not set yet
 *---------------------------------------------------------------------------*/
PMExitFactor PM_GetExitFactor(void)
{
	return PMi_ExitFactor;
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPreExitCallback

  Description:  append exit callback info to pre-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPreExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PreExitCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_AppendPostExitCallback

  Description:  append exit callback info to post-callback info list

  Arguments:    info : callback info to append

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_AppendPostExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PostExitCallbackList, info, PM_CALLBACK_PRIORITY_MAX, PMi_COMPARE_GT);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPreExitCallback

  Description:  prepend exit callback info to pre-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPreExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PreExitCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_PrependPostExitCallback

  Description:  prepend exit callback info to post-callback info list

  Arguments:    info : callback info to prepend

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_PrependPostExitCallback( PMExitCallbackInfo *info )
{
	PMi_InsertList(&PMi_PostExitCallbackList, info, PM_CALLBACK_PRIORITY_MIN, PMi_COMPARE_GE);
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPreExitCallback

  Description:  insert an exit callback info to pre-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_InsertPreExitCallbackEx( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PreExitCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPreExitCallback( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPreExitCallbackEx( info, priority );
}

/*---------------------------------------------------------------------------*
  Name:         PM_InsertPostExitCallback

  Description:  insert an exit callback info to post-callback info list

  Arguments:    info     : callback info to add
                priority : pritory

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_InsertPostExitCallbackEx( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority && priority <= PM_CALLBACK_PRIORITY_SYSMAX );
	PMi_InsertList(&PMi_PostExitCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPostExitCallback( PMExitCallbackInfo *info, int priority )
{
	SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority && priority <= PM_CALLBACK_PRIORITY_MAX );
	PMi_InsertPostExitCallbackEx( info, priority );
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePreExitCallback

  Description:  delete exit callback info from pre-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePreExitCallback( PMExitCallbackInfo *info )
{
    PMi_DeleteList(&PMi_PreExitCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PM_DeletePostExitCallback

  Description:  delete exit callback info from post-callback info list

  Arguments:    info : callback info to delete

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_DeletePostExitCallback( PMExitCallbackInfo *info )
{
    PMi_DeleteList(&PMi_PostExitCallbackList, info);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ClearPreExitCallback

  Description:  clear exit pre-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_ClearPreExitCallback(void)
{
    PMi_ClearList(&PMi_PreExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ClearPostExitCallback

  Description:  clear exit post-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_ClearPostExitCallback(void)
{
    PMi_ClearList(&PMi_PostExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecutePreExitCallbackList

  Description:  execute exit pre-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_ExecutePreExitCallbackList(void)
{
    PMi_ExecuteList(PMi_PreExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecutePostExitCallbackList

  Description:  execute exit post-callback info list

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_ExecutePostExitCallbackList(void)
{
    PMi_ExecuteList(PMi_PostExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ExecuteAllListsOfExitCallback

  Description:  execute all callbacks registerd to info lists

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_ExecuteAllListsOfExitCallback(void)
{
    PMi_ExecuteList(PMi_PreExitCallbackList);
    PMi_ExecuteList(PMi_PostExitCallbackList);
}

/*---------------------------------------------------------------------------*
  Name:         PM_SetAutoExit

  Description:  set flag to shutdown/reset (not) automatically after callback

  Arguments:    sw : TRUE  ... shutdown/reset after callback automatically
                     FALSE ... not shutdown/reset after callback

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_SetAutoExit( BOOL sw )
{
	PMi_AutoExitFlag = sw;
}
/*---------------------------------------------------------------------------*
  Name:         PM_GetAutoExit

  Description:  get the current setting of AutoExit

  Arguments:    None

  Returns:      TRUE  ... shutdown/reset after callback automatically
                FALSE ... not shutdown/reset after callback
 *---------------------------------------------------------------------------*/
BOOL PM_GetAutoExit(void)
{
	return PMi_AutoExitFlag;
}

//================================================================================
//             BATTERY CALLBACK
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_SetBatteryLowCallback

  Description:  set battery low callback

  Arguments:    callback : callback called in detecting battery low
                arg      : callback argument 

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_SetBatteryLowCallback( PMBatteryLowCallback callback, void* arg )
{
	PMi_BatteryLowCallbackInfo.callback = callback;
	PMi_BatteryLowCallbackInfo.arg = arg;
}

//================================================================================
//             RESET HARDWARE (TWL)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PM_ForceToResetHardware

  Description:  tell ARM7 to reset hardware

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_ForceToResetHardware(void)
{
    //---- set exit factor
    PMi_ExitFactor = PM_EXIT_FACTOR_USER;

    PMi_CallPostExitCallbackAndReset(FALSE);
    //never reached hear
}

/*---------------------------------------------------------------------------*
  Name:         PMi_CallPostExitCallbackAndReset

  Description:  call post exit callbacks and reset hardware

  Arguments:    isExit : TRUE  ... EXIT (HARDWARERESET or SHUTDOWN)
                         FALSE ... HARDWARERESET

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_CallPostExitCallbackAndReset(BOOL isExit)
{
    //---- call exit callbacks
    PMi_ExecuteList(PMi_PostExitCallbackList);

    //---- display off
    GX_DispOff();
    GXS_DispOff();

    //---- wait 2 vblank at least
    MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
    {
        int n;
        for( n=0; n<3; n++ )
        {
            u32 count = OS_GetVBlankCount();
            while( count == OS_GetVBlankCount() )
            {
                OS_SpinWait(100);
            }
        }
    }

    //---- send EXIT or RESET command
    while(1)
    {
        u16 result;
        u32 command = isExit? PM_UTIL_FORCE_EXIT: PM_UTIL_FORCE_RESET_HARDWARE;

        //---- exit if specified 'forced' .
        if ( *(u32*)HW_RESET_LOCK_FLAG_BUF == PM_RESET_FLAG_FORCED )
        {
            command = PM_UTIL_FORCE_EXIT;

            //---- clear launcher param
            ((LauncherParam*)HW_PARAM_LAUNCH_PARAM)->header.magicCode = 0;
        }

        while( PM_SendUtilityCommand(command, 0, &result) != PM_SUCCESS )
        {
            //---- wait
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
        if ( result == SPI_PXI_RESULT_SUCCESS )
        {
            break;
        }

        //---- wait
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    (void)OS_DisableInterrupts();

    //---- stop all dma
    MI_StopAllDma();
    MI_StopAllNDma();

    //---- finalize debugger
    PMi_FinalizeDebugger();

    OSi_TerminateCore();
    //never reached hear
}
#include <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         PMi_LCDOnAvoidReset

  Description:  Force to power on LCD to avoid the following:
                On USG, the operation of power off with LCD-ON may cause reset hardware not power off
                because of PMIC of the specified maker.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void PMi_LCDOnAvoidReset(void)
{
    BOOL preMethod;

	//---- wait while specified period for LCD (0x360000sysClock == about 106ms)
	OS_SpinWaitSysCycles(PMi_LCD_WAIT_SYS_CYCLES);

	//---- force to set LCD power on
    preMethod = PMi_WaitBusyMethod;
    PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE | PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
	if (PM_GetLCDPower() != PM_LCD_POWER_ON)
	{
		//---- set backlight off (for avoiding appearance of afterimage)
        while( PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF) != PM_RESULT_SUCCESS )
        {
            OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }

		while (!PM_SetLCDPower(PM_LCD_POWER_ON))
		{
			OS_SpinWait(PMi_PXI_WAIT_TICK);
		}
	}
    PMi_WaitBusyMethod = preMethod;
}

#ifndef SDK_FINALROM
//================================================================================
//     DIRECT REGISTER OPERATION (for DEBUG)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_ReadRegisterAsync

  Description:  send read register command  to ARM7

  Arguments:    registerAddr : PMIC register number (0-3)
                buffer       : buffer to store register value
                callback     : callback function
                arg          : callback argument

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
u32 PMi_ReadRegisterAsync(u16 registerAddr, u16 *buffer, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PMi_UTIL_READREG, registerAddr, buffer, callback, arg);
}
u32 PMi_ReadRegister(u16 registerAddr, u16 *buffer)
{
	return PM_SendUtilityCommand(PMi_UTIL_READREG, registerAddr, buffer);
}
/*---------------------------------------------------------------------------*
  Name:         PMi_WriteRegisterAsync

  Description:  send write register command to ARM7

  Arguments:    registerAddr : PMIC register number (0-3)
                data         : data written to PMIC register
                callback     : callback function
                arg          : callback argument

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PMi_UTIL_WRITEREG, (u16)((registerAddr<<8) | (data&0xff)), NULL, callback, arg);
}

u32 PMi_WriteRegister(u16 registerAddr, u16 data)
{
	return PM_SendUtilityCommand(PMi_UTIL_WRITEREG, (u16)((registerAddr<<8) | (data&0xff)), NULL);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         PMi_SetDispOffCount

  Description:  record disp off counter for internal use

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PMi_SetDispOffCount(void)
{
	PMi_DispOffCount = OS_GetVBlankCount();
}

//================================================================================
//   DUMP (for DEBUG)
//================================================================================
#ifndef SDK_FINALROM
/*---------------------------------------------------------------------------*
  Name:         PM_DumpSleepCallback

  Description:  dump sleep callbacks (for debug)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DumpSleepCallback(void)
{
	PMGenCallbackInfo *p;

	p = PMi_PreSleepCallbackList;
	OS_TPrintf("----PreSleep Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}

	p = PMi_PostSleepCallbackList;
	OS_TPrintf("----PostSleep Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
  Name:         PM_DumpExitCallback

  Description:  dump exit callbacks (for debug)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PM_DumpExitCallback(void)
{
	PMGenCallbackInfo *p;

	p = PMi_PreExitCallbackList;
	OS_TPrintf("----PreExit Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}

	p = PMi_PostExitCallbackList;
	OS_TPrintf("----PostExit Callback\n");
	while(p)
	{
		OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
		p = p->next;
	}
}
#include <twl/ltdmain_end.h>
#endif //ifdef SDK_TWL
#endif //ifndef SDK_FINALROM

//================================================================================
//   LOCK for RESET/EXIT
//================================================================================
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
  Name:         PMi_TryLockForReset

  Description:  Try to lock. This is for the forced reset and pushing battery button.

  Arguments:    None

  Returns:      TRUE  ... success to lock
                FALSE ... failed to lock
 *---------------------------------------------------------------------------*/
static volatile BOOL isLockedReset = FALSE;
BOOL PMi_TryLockForReset(void)
{
    OSIntrMode e = OS_DisableInterrupts();

    //---- try lock
    if ( isLockedReset )
    {
        (void)OS_RestoreInterrupts(e);
        return FALSE;
    }
    isLockedReset = TRUE;

    (void)OS_RestoreInterrupts(e);
    return TRUE;
}
#include <twl/ltdmain_end.h>
#endif // ifdef SDK_TWL
