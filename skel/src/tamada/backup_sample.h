
//=============================================================================================
//=============================================================================================
#pragma	once

#include "savedata_def.h"

extern GFL_SAVEDATA * BackupSample_Create(void);
extern void BackupSample_SetData(GFL_SAVEDATA * sv, const u16 * data);
extern void BackupSample_DumpData(GFL_SAVEDATA * sv);
extern BOOL BackupSample_Control(void);
