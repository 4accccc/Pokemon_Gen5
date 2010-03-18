//=============================================================================
/**
 *	@file		demo3d_cmd.h
 *	@brief  3Dデモコマンドローカルヘッダ
 *	@author Miyuki Iwasawa	
 *	@data		2010.03.17
 */
//=============================================================================

#pragma once

#include "demo3d_graphic.h"
#include "demo3d_mb.h" 

//データ
#include "demo3d_data.h"

#include "demo3d_engine.h"
#include "demo3d_cmdsys.h"
#include "demo3d_engine_local.h"

//--------------------------------------------------------------
///	コマンドワーク
//==============================================================
struct _DEMO3D_CMD_WORK {
  // [IN]
  HEAPID heapID;
  HEAPID tmpHeapID;
  DEMO3D_ID demo_id;
 
  // [PRIVATE]
  BOOL  is_cmd_end;
  int   pre_frame; ///< 1sync=1
  int   cmd_idx;

  //コマンド制御
  DEMO3D_ENGINE_WORK* core;

  GFL_TCBLSYS*  tcbsys;

  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  PRINT_STREAM* printStream;
  
  DEMO3D_MBL_WORK* mb;
};

extern void (*DATA_Demo3D_CmdTable[ DEMO3D_CMD_TYPE_MAX ])();

