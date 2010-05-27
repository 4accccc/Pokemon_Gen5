//======================================================================
/**
 * @file    debug_makepoke.h
 * @brief   デバッグ用ポケモンデータ生成ツール
 * @author  taya
 * @data    09/05/29
 */
//======================================================================
#pragma once

#include <procsys.h>
#include "poke_tool\poke_tool.h"
#include "savedata\mystatus.h"

typedef enum{
  DMP_RET_CANCEL,
  DMP_RET_REWRITE,
  DMP_RET_COPY,
}DEBUG_MAKEPOKE_RETCODE;

typedef enum{
  DMP_MODE_MAKE,
  DMP_MODE_D_FIGHT,
}DEBUG_MAKEPOKE_MODE;

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
typedef struct {

  POKEMON_PARAM*    dst;
  const MYSTATUS*   oyaStatus;
  u32               defaultMonsNo;
  DEBUG_MAKEPOKE_MODE     mode;
  DEBUG_MAKEPOKE_RETCODE  ret_code;
}PROCPARAM_DEBUG_MAKEPOKE;

//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
extern const GFL_PROC_DATA ProcData_DebugMakePoke;


