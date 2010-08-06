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


//コマンドがフリーに使えるBGフレーム定義
#define DEMO3D_CMD_FREE_BG_M1 (GFL_BG_FRAME1_M)

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
  BOOL  cmdsys_end_f; ///<コマンドタスクの自殺用
  int   pre_frame; ///< 1sync=1
  int   cmd_idx;

#ifdef BUGFIX_AF_MSGSPEED_DEMO3D_20100806
  /*
   *  MSGSPEED_GetWait()の戻り値はs8以上のsignedで受ける必要がある
   */
  s8    msg_spd;
#else
  u8    msg_spd;
#endif
  u8    bgm_push_f;
  u8    delay_f;

  //コマンド制御
  DEMO3D_ENGINE_WORK* core;

  GFL_TCBLSYS*  tcbsys;
  GFL_FONT*     fontHandle;
  ARCHANDLE*    h_2dgra; 
  DEMO3D_MBL_WORK* mb;
};

extern void (*DATA_Demo3D_CmdTable[ DEMO3D_CMD_TYPE_MAX ])();

