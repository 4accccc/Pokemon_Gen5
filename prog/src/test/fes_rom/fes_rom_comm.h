//==============================================================================
/**
 * @file    fes_rom_comm.h
 * @brief   ä»íPÇ»ê‡ñæÇèëÇ≠
 * @author  iwasawa 
 * @date    2012.04.18(êÖ)
 */
//==============================================================================
#pragma once

extern void HaisinBeacon_Init( FEST_MISSION* fmwk, int beacon_space_time );
extern BOOL HaisinBeacon_InitWait(void);
extern void HaisinBeacon_Exit(void);
extern BOOL HaisinBeacon_ExitWait(void);
extern void HaisinBeacon_Update( FEST_MISSION* fmwk );
