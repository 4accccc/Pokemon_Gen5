//==============================================================================
/**
 * @file  guru2.h
 * @brief ÇÆÇÈÇÆÇÈåä∑
 * @author  AkitoMori(à⁄êAÅj/ kagaya(çÏê¨Åj
 * @data  09.01.15
 */
//==============================================================================
#ifndef GURU2_H_FILE
#define GURU2_H_FILE

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "net_app/union_app.h"

//#include "savedata/savedata_def.h"
#include "savedata/record.h"

//==============================================================================
//  define
//==============================================================================

//==============================================================================
//  typedef
//==============================================================================
typedef struct GURU2PROC_WORK GURU2PROC_WORK;

//--------------------------------------------------------------
/// GURU2_PARAM
//--------------------------------------------------------------
typedef struct
{
  int max;
  
  GAMEDATA              *gamedata;
  SAVE_CONTROL_WORK     *sv;
  RECORD                *record;
  GURUGURU_PARENT_WORK  *guru2union;
//  FIELDSYS_WORK *fsys;
}GURU2_PARAM;

//==============================================================================
//  extern
//==============================================================================
extern GURU2PROC_WORK * Guru2_ProcInit( GURU2_PARAM *param, u32 heap_id );
extern void Guru2_ProcDelete( GURU2PROC_WORK *g2p );
extern BOOL Guru2_ReceiptRetCheck( GURU2PROC_WORK *g2p );

extern void * EventCmd_Guru2ProcWorkAlloc( GAMEDATA *gamedata );
extern BOOL EventCmd_Guru2Proc( void *wk );

extern GFL_PROC_RESULT Guru2Receipt_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT Guru2Receipt_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT Guru2Receipt_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk );

extern GFL_PROC_RESULT Guru2Main_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT Guru2Main_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT Guru2Main_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk );

#endif //GURU2_H_FILE
