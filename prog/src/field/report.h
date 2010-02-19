
#pragma	once

#include "gamesystem/gamesystem.h"

// オーバーレイ
FS_EXTERN_OVERLAY(report);


typedef struct _REPORT_WORK	REPORT_WORK;

extern REPORT_WORK * REPORT_Init( GAMESYS_WORK * gs, HEAPID heapID );

extern void REPORT_Exit( REPORT_WORK * wk );

extern void REPORT_Update( REPORT_WORK * wk );
