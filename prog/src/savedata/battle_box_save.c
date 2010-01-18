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
  u16 lockFlg:1;  //バトルボックスがロックされているかどうか
  u16 dummy:15;
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
//	設定
//======================================================================

//----------------------------------------------------------
//  PPPセット
//----------------------------------------------------------
void BATTLE_BOX_SAVE_SetPPP( BATTLE_BOX_SAVE * btlBoxSave, const u32 boxIdx, const u32 idx, const POKEMON_PASO_PARAM * ppp )
{
	btlBoxSave->ppp[boxIdx][idx] = *ppp;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		バトルボックスのセーブデータからPOEPARTYを作成
 *
 * @param		sv				バトルボックスのセーブデータ
 * @param		heapID		ヒープＩＤ
 *
 * @return	作成したデータ
 */
//--------------------------------------------------------------------------------------------
POKEPARTY * BATTLE_BOX_SAVE_MakePokeParty( BATTLE_BOX_SAVE * sv, HEAPID heapID )
{
	POKEMON_PASO_PARAM * ppp;
	POKEMON_PARAM * pp;
	POKEPARTY * party;
	u32	i;

	party = PokeParty_AllocPartyWork( heapID );
	PokeParty_Init( party, TEMOTI_POKEMAX );

	for( i=0; i<TEMOTI_POKEMAX; i++ ){
		ppp = BATTLE_BOX_SAVE_GetPPP( sv, 0, i );
		if( PPP_Get( ppp, ID_PARA_poke_exist, NULL ) == 0 ){
			break;
		}
		pp = PP_CreateByPPP( ppp, GFL_HEAP_LOWID(heapID) );
		PokeParty_Add( party, pp );
		GFL_HEAP_FreeMemory( pp );
	}

	return party;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		POEPARTYをバトルボックスのセーブデータに設定
 *
 * @param		sv				バトルボックスのセーブデータ
 * @param		party			POKEPARTY
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BATTLE_BOX_SAVE_SetPokeParty( BATTLE_BOX_SAVE * sv, POKEPARTY * party )
{
	POKEMON_PARAM * pp;
	u32	i;

	// セーブデータ初期化
	for( i=0; i<BATTLE_BOX_PARTY_MEMBER; i++ ){
		PPP_Clear( &sv->ppp[0][i] );
	}

	for( i=0; i<PokeParty_GetPokeCount(party); i++ ){
		pp = PokeParty_GetMemberPointer( party, i );
		BATTLE_BOX_SAVE_SetPPP( sv, 0, i, PP_GetPPPPointerConst(pp) );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		バトルボックスにポケモンが入っているかどうか
 * @param		sv				バトルボックスのセーブデータ
 * @return	いたらTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL BATTLE_BOX_SAVE_IsIn( BATTLE_BOX_SAVE * sv )
{
	POKEMON_PASO_PARAM * ppp;
	POKEMON_PARAM * pp;

  ppp = BATTLE_BOX_SAVE_GetPPP( sv, 0, 0 );
  if( PPP_Get( ppp, ID_PARA_poke_exist, NULL ) == 0 ){
    return FALSE;
  }
	return TRUE;
}


//----------------------------------------------------------
//  バトルボックスをロックする
//----------------------------------------------------------
void BATTLE_BOX_SAVE_SetLockFlg( BATTLE_BOX_SAVE *btlBoxSave,int flg )
{
  btlBoxSave->lockFlg = flg;
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

//----------------------------------------------------------
//  ロックされているかどうか
//----------------------------------------------------------
BOOL BATTLE_BOX_SAVE_GetLockFlg( BATTLE_BOX_SAVE *btlBoxSave )
{
  return btlBoxSave->lockFlg;
}

