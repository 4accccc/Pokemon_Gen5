//======================================================================
/**
 * @file  p_sta_local_def.c
 * @brief ポケモンステータス ローカル定義
 * @author  ariizumi
 * @data  09/07/01
 *
 * モジュール名：POKE_STATUS
 */
//======================================================================
#pragma once

#include "app/p_status.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "sound/pm_sndsys.h"
#include "debug/debugwin_sys.h"

#include "p_status_gra.naix"
#include "msg/msg_pokestatus.h"

//BG面定義
#define PSTATUS_BG_MAIN_BG (GFL_BG_FRAME3_M)
#define PSTATUS_BG_PLATE   (GFL_BG_FRAME2_M)
#define PSTATUS_BG_PARAM   (GFL_BG_FRAME1_M)
#define PSTATUS_BG_3D      (GFL_BG_FRAME0_M)

#define PSTATUS_BG_SUB_BG  (GFL_BG_FRAME3_S)

//BGパレット
#define PSTATUS_BG_PLT_FONT (0xe)

//メインページの幅(キャラ単位)
#define PSTATUS_MAIN_PAGE_WIDTH (20)

typedef struct _PSTATUS_SUB_WORK PSTATUS_SUB_WORK;

typedef struct
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;
  
  PSTATUS_DATA *psData;
  
  u8  dataPos;  //何番目か？
  u8  scrollCnt;
  u32 befVCount;

  //3D
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS       *bbdSys;
  
  //MSG系
  GFL_MSGDATA *msgHandle;
  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;

  //サブワーク
  PSTATUS_SUB_WORK *subWork;

#if PM_DEBUG
  POKEMON_PARAM *debPp;
#endif

}PSTATUS_WORK;

