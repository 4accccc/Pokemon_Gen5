//============================================================================================
/**
 * @file  options_local.h
 * @brief ユーザーが設定する細かいセーブデータを集めるところ
 * @author  tamada
 * @date  2011.08.24
 */
//============================================================================================
#pragma once

#include "savedata/options_savedata.h"

#include "my_pms_data_local.h"

#include "c_gear_data_local.h"

#include "shortcut_local.h"

#include "boast_message_local.h"

//----------------------------------------------------------
/**
 * @brief
 */
//----------------------------------------------------------
struct _OPTIONS_SAVEDATA{
  MYPMS_DATA mypms_data;

  CGEAR_SAVEDATA cgear_savedata;

  SHORTCUT shortcut;

  BOAST_MSG_SAVEDATA boast_message;

};

