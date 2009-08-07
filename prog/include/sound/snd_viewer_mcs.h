#pragma once

extern void	GFL_MCS_SNDVIEWER_Init(HEAPID heapID);
extern void	GFL_MCS_SNDVIEWER_Exit(void);
extern u32	GFL_MCS_SNDVIEWER_Main(HEAPID heapID);

extern void MCS_Sound_Send(u32 comm, u32* param, HEAPID heapID);
