//======================================================================
/**
 * @file	local_tvt_local_def.h
 * @brief	非通信テレビトランシーバー 定義
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#pragma once

#include "app/local_tvt_sys.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define LTVT_FRAME_MESSAGE (GFL_BG_FRAME1_M)
#define LTVT_FRAME_CHARA   (GFL_BG_FRAME2_M)
#define LTVT_FRAME_BG      (GFL_BG_FRAME3_M)

#define LTVT_CHARA_SCREEN_WIDTH (16)
#define LTVT_CHARA_SCREEN_HEIGHT (24)

//拡張BGは面ごとに持つので両方0からでOK
#define LTVT_PLT_CHARA (0)
#define LTVT_PLT_BG (0)

#define LTVT_VRAM_ADR_CHARA (0x10000)
#define LTVT_VRAM_ADR_BG    (0x28000)
#define LTVT_VRAM_PAGE_SIZE (0x0C000)

#define LTVT_TRANS_COUNT_MAX (20)
#define LTVT_TRANS_SIZE (0x3000)    //1フレームに送る転送量
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

typedef enum
{
  LTTS_CHARA1,
  LTTS_CHARA2,
  LTTS_CHARA3,
  LTTS_CHARA4,
  LTTS_TRANS_WAIT,
  
  LTTS_MAX,
}LOCAL_TVT_TRANS_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct _LOCAL_TVT_CHARA LOCAL_TVT_CHARA;

typedef struct
{
  HEAPID heapId;
  
  u8 bufNo;
  u8 transCnt;
  
  LOCAL_TVT_TRANS_STATE transState;
  
  ARCHANDLE *archandle;
  LOCAL_TVT_CHARA *charaWork[LOCAL_TVT_MEMBER_MAX];
  LOCAL_TVT_CHARA *transChara;
  
  LOCAL_TVT_INIT_WORK *initWork;
}LOCAL_TVT_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

