//============================================================================================
/**
 * @file	battle_box_save.c
 * @brief	バトルボックスセーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/09/25
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "savedata/battle_box_save.h"
#include "savedata/save_tbl.h"
#include "poke_tool/poke_tool_def.h"
#include "msg/msg_boxmenu.h"
#include "arc_def.h"
#include "message.naix"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

struct _BATTLE_BOX_SAVE
{
  POKEMON_PASO_PARAM ppp[BATTLE_BOX_PARTY_NUM][BATTLE_BOX_PARTY_MEMBER];
  STRCODE name[BATTLE_BOX_PARTY_NUM][BATTLE_BOX_NAME_BUFSIZE];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
int BATTLE_BOX_SAVE_GetWorkSize(void)
{
  return sizeof( BATTLE_BOX_SAVE );
}

void BATTLE_BOX_SAVE_InitWork(BATTLE_BOX_SAVE *btlBoxSave)
{
  u32 i,p;
	GFL_MSGDATA*  msgman;
  for(i = 0; i < BATTLE_BOX_PARTY_NUM; i++)
  {
    for(p = 0; p < BATTLE_BOX_PARTY_MEMBER; p++)
    {
      PPP_Clear( &(btlBoxSave->ppp[i][p]) );
    }
  }
  // デフォルトボックス名セット
  // ボックスと同じものをセット
  msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_boxmenu_dat, GFL_HEAPID_APP );
  if( msgman )
  {
    for(i = 0 ; i < BATTLE_BOX_PARTY_NUM; i++)
    {
      GFL_MSG_GetStringRaw( msgman, mes_boxmenu_02_02+i, btlBoxSave->name[i], BATTLE_BOX_NAME_BUFSIZE );
    }
    GFL_MSG_Delete( msgman );
  }
}

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
BATTLE_BOX_SAVE* BATTLE_BOX_SAVE_GetBattleBoxSave( SAVE_CONTROL_WORK *sv )
{
  return (BATTLE_BOX_SAVE*)SaveControl_DataPtrGet( sv , GMDATA_ID_BATTLE_BOX );
}


//======================================================================
//	取得系
//======================================================================

//----------------------------------------------------------
//  PPP取得
//----------------------------------------------------------
POKEMON_PASO_PARAM* BATTLE_BOX_SAVE_GetPPP( BATTLE_BOX_SAVE *btlBoxSave , const u32 boxIdx , const u32 idx )
{
  return &btlBoxSave->ppp[boxIdx][idx];
}

//----------------------------------------------------------
//  ボックス名取得
//----------------------------------------------------------
STRCODE* BATTLE_BOX_SAVE_GetBoxName( BATTLE_BOX_SAVE *btlBoxSave , const u32 boxIdx )
{
  return btlBoxSave->name[boxIdx];
}
