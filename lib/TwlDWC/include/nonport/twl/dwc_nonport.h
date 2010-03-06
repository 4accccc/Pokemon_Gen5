/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/os.h>
#include <nitro/types.h>

#include <nonport/twl/dwci_np_types.h>

#define DWC_ASSERT SDK_ASSERT
#define DWC_ASSERTMSG SDK_ASSERTMSG

#ifdef SDK_DEBUG
#define RVLDWC_DEBUG
#endif
#ifdef SDK_RELEASE
#define RVLDWC_RELEASE
#endif
#ifdef SDK_FINALROM
#define RVLDWC_FINAL
#endif

typedef OSTick  DWCTick;

#define DWCi_Np_TicksToMilliSeconds OS_TicksToMilliSeconds
#define DWCi_Np_MilliSecondsToTicks OS_MilliSecondsToTicks

#define DWC_INLINE inline

// ミューテックス
#define DWCMutex OSMutex
#define DWC_InitMutex OS_InitMutex
#define DWC_LockMutex OS_LockMutex
#define DWC_UnlockMutex OS_UnlockMutex

u64 DWCi_Np_GetConsoleId( void );
const char* DWCi_Np_GetConsoleName( void );
const char* DWCi_Np_GetAppGameName( void );
BOOL    DWCi_Np_SetRootCAList( const char *url );
BOOL    DWCi_Np_CleanupRootCAList( const char *url );

static inline DWCTick DWCi_Np_GetTick(void)
{
    return OS_GetTick();
}

static inline u64 DWCi_Np_GetTimeInMilliseconds( void )
{
    return (u64)OS_TicksToMilliSeconds(OS_GetTick());
}

static inline void DWCi_Np_CpuCopy8( register const void *srcp, register void *dstp, register u32 size )
{
    MI_CpuCopy8( srcp, dstp, size );
}

static inline void DWCi_Np_CpuCopy16( register const void *srcp, register void *dstp, register u32 size )
{
    MI_CpuCopy16( srcp, dstp, size );
}

static inline void DWCi_Np_CpuCopy32( register const void *srcp, register void *dstp, register u32 size )
{
    MI_CpuCopy32( srcp, dstp, size );
}



static inline void DWCi_Np_CpuClear8(void *dest, u32 size)
{
    MI_CpuClear8( dest, size );
}

static inline void DWCi_Np_CpuClear16(void *dest, u32 size)
{
    MI_CpuClear16( dest, size );
}

static inline void DWCi_Np_CpuClear32(void *dest, u32 size)
{
    MI_CpuClear32( dest, size );
}


static inline void DWCi_Np_GetMacAddress(u8 *macAddress)
{
    OS_GetMacAddress( macAddress );
}


// リトルエンディアンの場合なにもしない

static inline u16 DWCi_HtoLEs( u16 data )
{
    return data;
}
static inline u32 DWCi_HtoLEl( u32 data )
{
    return data;
}
static inline u16 DWCi_LEtoHs( u16 data )
{
    return data;
}
static inline u32 DWCi_LEtoHl( u32 data )
{
    return data;
}
