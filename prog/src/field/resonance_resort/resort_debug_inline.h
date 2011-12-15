//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_debug_inline.h
 *	@brief  event_debug_menu_resort.cとresort_debug_input.cをつなぐ処理
 *	@author	Toru=Nagihashi
 *	@date		2011.09.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG

#include <gflib.h>
#include "arc/script_message.naix"
#include "msg/script/msg_resort_scr.h"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "msg/debug/msg_debug_resort2.h"
#include "net_app/union/union_beacon_tool.h"
#include "field/resonance_resort_data.h"

//=============================================================================
/**
 *					デバッグ住人作成
*/
//=============================================================================

static inline void Debug_Resort_SetRecv( RESORT_RECV_DATA *p_wk, GAMEDATA *p_gamedata )
{
  u32 trainerID;
  u32 sex;
  u32 ver;

  //自分のデータをいれた後、トレーナーIDと名前を変える
  RESORT_RECV_DATA_SetMyData( p_wk, p_gamedata );

  {
    int i;
    for( i = 0; i < 8; i++ )
    {
      RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT2_RANK_00+i, GFL_STD_MtRand(9999999) );
    }
    RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXTEND_ADD, RESORT_EXTEND_TYPE_02 );
    RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXTEND_ADD, RESORT_EXTEND_TYPE_03 );
  }

  //トレーナーID
  RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_TRAINER_ID, GFL_STD_MtRand(GFL_STD_RAND_MAX) );
  trainerID = RESORT_RECV_DATA_GetData( p_wk, RESORT_DATA_PARAM_TRAINER_ID, NULL );

  //バージョン  一応B2orW2
  {
    ver = VERSION_B2;
    if( GFL_STD_MtRand(2) )
    {
      ver = VERSION_W2;
    }
    RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_VER, ver );
  }

  //性別
  RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_SEX, GFL_STD_MtRand(2) );
  sex = RESORT_RECV_DATA_GetData( p_wk, RESORT_DATA_PARAM_SEX, NULL );

  //見た目
  RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_OBJCODE, UnionView_GetObjCode(sex*8+GFL_STD_MtRand(8)) );

  
  //ビーコン
  RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_COMM_ID, RESORT_COMM_TYPE_BEACON );

  //乱数
  RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_RND_SEED, GFL_STD_MtRand(GFL_STD_RAND_MAX) );

  //なりたいお店
  {
    RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_WANT_JOB, RESORT_DATA_GetDefaultWantJob( 0, trainerID, ver ) );
  }

  //名前セット
  {
    GFL_MSGDATA* p_msg = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL,ARCID_DEBUG_MESSAGE,NARC_debug_message_debug_resort2_dat, GFL_HEAP_LOWID(HEAPID_PROC) );
    STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, RESORT_NAME_00+GFL_STD_MtRand(20) );
    RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_NAME, (u32)GFL_STR_GetStringCodePointer(p_strbuf) );
    GFL_STR_DeleteBuffer( p_strbuf );
    GFL_MSG_Delete( p_msg );
  }
  //自己紹介、感謝、くちぐせ
  {
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(HEAPID_PROC) );
    {
      STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, resort_introduction_def_00+trainerID%RESORT_INTRODUCTION_DEFAULT_NUM );
      RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_INTRODUCTION, (u32)GFL_STR_GetStringCodePointer(p_strbuf) );
      GFL_STR_DeleteBuffer( p_strbuf );
    }
    {
      STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, resort_kutiguse_def_00+trainerID%RESORT_PHRASE_DEFAULT_NUM );
      RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT6_PHRASE, (u32)GFL_STR_GetStringCodePointer(p_strbuf) );
      GFL_STR_DeleteBuffer( p_strbuf );
    }
    {
      STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, resort_thanks_def_00+trainerID%RESORT_THANKS_DEFAULT_NUM );
      RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT7_THANKS, (u32)GFL_STR_GetStringCodePointer(p_strbuf) );
      GFL_STR_DeleteBuffer( p_strbuf );
    }
    GFL_MSG_Delete( p_msg );
  }
}


#endif //PM_DEBUG
