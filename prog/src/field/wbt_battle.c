//============================================================================================
/**
 * @file  wbt_battle.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.08.20
 *
 * wbt_system.cから戦闘呼び出しに関わる部分を切り離した。
 * WBT_GIMMICKのオーバーレイに配置されるため、会場内のみでしか呼べないので注意
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"

#include "system/main.h"

#include "wbt_system_local.h"
#include "wbt_tool.h"

//#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE

#include "savedata/bsubway_savedata.h"  //bsubway_battle_data.hの内部定義のため
#include "battle/bsubway_battle_data.h" //BSUBWAY_PARTNER_DATA
#include "fld_btl_inst_tool.h"
#include "message.naix"    //NARC_message_wbt_trname_dat
#include "tr_tool/ex_trainer_def.h" //TRID_OFFSET_WBT
#include "savedata/etc_save.h"    //ETC_SAVE_WORK

#include "savedata/save_tbl.h"
//#include "savedata/save_control.h"
//#include "savedata/wbt_ex_save.h"
#include "savedata/wbt_download.h"

#include "sound/swan_sound_data.sadl"

#include "system/vm.h"
#include "src/battle/tr_ai/tr_ai.h" //AI_THINK_BIT_～

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  //（一行１８文字＋改行コード）*２（海外は倍)*8(8行分）
  WBT_TRAINER_MSG_LEN = ((18+1)*2*8),
};

enum {
  ROM_POKE_MAX = 10,
};

//============================================================================================
//============================================================================================
#ifdef  PM_DEBUG
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void DEBUG_PrintString( const STRBUF * strbuf )
{
  enum { UNI_MAX_LEN = 64 };
  u16 unicode[UNI_MAX_LEN];
  int i;
  const STRCODE eom_code = GFL_STR_GetEOMCode();

  GFL_STR_GetStringCode( strbuf, (STRCODE*)unicode, UNI_MAX_LEN );
  for ( i = 0; i < UNI_MAX_LEN; i ++ )
  {
    if ( unicode[i] == eom_code )
    {
      unicode[i] = L'\0';
      break;
    }
  }
  unicode[UNI_MAX_LEN - 1] = L'\0';
  OS_Printf("%s", unicode );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void DEBUG_POKEPARAM_Dump( POKEMON_PARAM * pp, HEAPID tempHeapID )
{
  STRBUF * strbuf = GFL_STR_CreateBuffer( 32, tempHeapID );
  u32 value;
  int i;

  OS_Printf("POKEMON_PARAM:%08x\n", (u32)pp);
  value = PP_Get( pp, ID_PARA_monsno, NULL );
  OS_Printf("monsno:%03d ",value );
  value = PP_Get( pp, ID_PARA_form_no, NULL );
  OS_Printf("formno:%02d ", value );
  value = PP_Get( pp, ID_PARA_sex, NULL );
  OS_Printf("sex:%d ", value );
  value = PP_Get( pp, ID_PARA_seikaku, NULL );
  OS_Printf("seikaku:%02d ", value );
  value = PP_Get( pp, ID_PARA_speabino, NULL );
  OS_Printf("tokusei:%03d\n", value );

  value = PP_CheckRare( pp );
  OS_Printf("rare:%d ", value );
  value = PP_Get( pp, ID_PARA_item, NULL );
  OS_Printf("itemno:%3d ", value );
  value = PP_Get( pp, ID_PARA_get_ball, NULL );
  OS_Printf("ball_id:%2d ", value );
  value = PP_Get( pp, ID_PARA_level, NULL );
  OS_Printf("Lv:%3d ", value );
  value = PP_Get( pp, ID_PARA_friend, NULL );
  OS_Printf("friendly:%3d ", value );
  OS_PutString("\n");

  for ( i = 0; i < PTL_WAZA_MAX; i ++ )
  {
    value = PP_Get( pp, ID_PARA_waza1 + i, NULL );
    OS_Printf("WAZA%d:%3d ", i+1,value );
    value = PP_Get( pp, ID_PARA_pp1 + i, NULL );
    OS_Printf("%2d/", value );
    value = PP_Get( pp, ID_PARA_pp_max1 + i, NULL );
    OS_Printf("%2d ", value );
    value = PP_Get( pp, ID_PARA_pp_count1 + i, NULL );
    OS_Printf("(+ %d)\n", value );
  }

  //パワー乱数
  OS_Printf("POWER_RND(hp,pow,def,agi,speabi,spedef) = (");
  value = PP_Get( pp, ID_PARA_hp_rnd, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_pow_rnd, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_def_rnd, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_agi_rnd, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_spepow_rnd, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_spedef_rnd, NULL ); OS_Printf("%3d", value );
  OS_PutString(")\n");
  //努力値
  OS_Printf("EXP(hp,pow,def,agi,speabi,spedef) = (");
  value = PP_Get( pp, ID_PARA_hp_exp, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_pow_exp, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_def_exp, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_agi_exp, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_spepow_exp, NULL ); OS_Printf("%3d,", value );
  value = PP_Get( pp, ID_PARA_spedef_exp, NULL ); OS_Printf("%3d", value );
  OS_PutString(")\n");

  OS_Printf("hp:%3d ", value );
  value = PP_Get( pp, ID_PARA_hp, NULL );
  OS_Printf("pow:%3d ", value );
  value = PP_Get( pp, ID_PARA_pow, NULL );
  OS_Printf("def:%3d ", value );
  value = PP_Get( pp, ID_PARA_def, NULL );
  OS_Printf("agi:%3d ", value );
  value = PP_Get( pp, ID_PARA_agi, NULL );
  OS_Printf("spepow:%3d ", value );
  value = PP_Get( pp, ID_PARA_spepow, NULL );
  OS_Printf("spedef:%3d ", value );
  value = PP_Get( pp, ID_PARA_spedef, NULL );
  OS_PutString("\n");

  value = PP_Get( pp, ID_PARA_country_code, NULL );
  OS_Printf("country_code:%3d ", value );
  value = PP_Get( pp, ID_PARA_id_no, NULL );
  OS_Printf("ID:%08x ", value );
  value = PP_Get( pp, ID_PARA_personal_rnd, NULL );
  OS_Printf("personal_rnd:%08x ", value );
  value = PP_Get( pp, ID_PARA_power_rnd, NULL );
  OS_Printf("power_rnd:%08x\n", value );

  OS_Printf("\n");

  GFL_STR_DeleteBuffer( strbuf );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void DEBUG_POKEPARTY_dump( POKEPARTY * party, HEAPID tempHeapID )
{
  int i, max;
  max = PokeParty_GetPokeCount(party);
  OS_Printf("POKEPARTY(%d):%08x\n", max, (u32)party);
  for ( i = 0; i < max; i ++ )
  {
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
    DEBUG_POKEPARAM_Dump( pp, tempHeapID );
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void DEBUG_WBTDL_POKEMON_dump( const WBTDL_POKEMON * dl_poke )
{
  int i;
  OS_Printf("WBT DL POKEMON:\n");
  OS_Printf("nickname");
  for ( i = 0; i < NELEMS(dl_poke->nickname); i ++ )
  {
    OS_Printf("%04x ", dl_poke->nickname[i]);
  }
  OS_PutString("\n");
  OS_Printf("monsno:%03d ", dl_poke->monsno);
  OS_Printf("formno:%02d ", dl_poke->form_no);
  OS_Printf("sex:%d ", dl_poke->sex);
  OS_Printf("seikaku:%02d ", dl_poke->seikaku);
  OS_Printf("tokusei:%03d\n", dl_poke->tokusei );

  OS_Printf("rare:%d ", dl_poke->rare_flag );
  OS_Printf("itemno:%03d ", dl_poke->item_no );
  OS_Printf("ball_id:%2d ", dl_poke->ball_id );
  OS_Printf("Lv:%3d friendly:%3d\n", dl_poke->level, dl_poke->friend_rate );

  OS_Printf("WAZA1:%3d pp(%d)\n", dl_poke->waza_id[0], dl_poke->pp_count1 );
  OS_Printf("WAZA2:%3d pp(%d)\n", dl_poke->waza_id[1], dl_poke->pp_count2 );
  OS_Printf("WAZA3:%3d pp(%d)\n", dl_poke->waza_id[2], dl_poke->pp_count3 );
  OS_Printf("WAZA4:%3d pp(%d)\n", dl_poke->waza_id[3], dl_poke->pp_count4 );
  //パワー乱数
  OS_Printf("POWER_RND(hp,pow,def,agi,speabi,spedef) = (%3d,%3d,%3d,%3d,%3d,%3d)\n",
      dl_poke->hp_rnd, dl_poke->pow_rnd, dl_poke->def_rnd,
      dl_poke->agi_rnd, dl_poke->spepow_rnd, dl_poke->spedef_rnd);
  //努力値
  OS_Printf("EXP(hp,pow,def,agi,speabi,spedef) = (%3d,%3d,%3d,%3d,%3d,%3d)\n",
      dl_poke->hp_exp, dl_poke->pow_exp, dl_poke->def_exp,
      dl_poke->agi_exp, dl_poke->spepow_exp, dl_poke->spedef_exp);

  OS_Printf("\n");
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void DEBUG_WBTDL_CHARACTER_dump( const WBTDL_CHARACTER * dl_char )
{
  OS_Printf("WBT DL CHARACTER:\n");
  OS_Printf("trtype_id=%d\n", dl_char->trtype_id );
  OS_Printf("msg_before_id(%2d) msg_after_win_id(%2d) msg_after_lose_id(%2d)\n",
      dl_char->msg_before_id, dl_char->msg_after_win_id, dl_char->msg_after_lose_id );
  OS_Printf("msg_win_id(%2d) msg_lose_id(%2d) msg_announce_id(%2d)\n",
      dl_char->msg_win_id, dl_char->msg_lose_id, dl_char->msg_announce_id );
  OS_Printf("lang_id(%2d) ai_bit(%d)\n", dl_char->lang_id, dl_char->ai_bit );

}
#endif

//============================================================================================
//
//  ミックストーナメント用処理
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief ミックス処理
 */
//------------------------------------------------------------------
void WBTSYS_SetupMixParty( WBTSYS * wbtsys )
{
  POKEMON_PARAM * mine;
  POKEMON_PARAM * enemy;
  POKEMON_PARAM * copy_pp = GFL_HEAP_AllocMemoryLo( wbtsys->heapID, POKETOOL_GetWorkSize() );

  mine = PokeParty_GetMemberPointer( wbtsys->my_party, wbtsys->mix_pos[WBTMIX_TARGET_MINE1] );
  enemy = PokeParty_GetMemberPointer( wbtsys->npc_party, wbtsys->mix_pos[WBTMIX_TARGET_ENEMY1] );
  POKETOOL_CopyPPtoPP( mine, copy_pp );
  POKETOOL_CopyPPtoPP( enemy, mine );
  POKETOOL_CopyPPtoPP( copy_pp, enemy );
  if ( wbtsys->rule == WBTRULE_TRIPLE )
  {
    mine = PokeParty_GetMemberPointer( wbtsys->my_party, wbtsys->mix_pos[WBTMIX_TARGET_MINE2] );
    enemy = PokeParty_GetMemberPointer( wbtsys->npc_party, wbtsys->mix_pos[WBTMIX_TARGET_ENEMY2] );
    POKETOOL_CopyPPtoPP( mine, copy_pp );
    POKETOOL_CopyPPtoPP( enemy, mine );
    POKETOOL_CopyPPtoPP( copy_pp, enemy );
  }

  GFL_HEAP_FreeMemory( copy_pp );
}

//------------------------------------------------------------------
/**
 * @brief  ミックス用パラメータの初期化
 */
//------------------------------------------------------------------
void WBTSYS_MixInit( WBTSYS * wbtsys )
{
  PokeParty_Copy( wbtsys->sp_party, wbtsys->my_party );
  wbtsys->mix_pos[WBTMIX_TARGET_MINE1] = GFUser_GetPublicRand( WBTSYS_GetPartyMax(wbtsys) );
  wbtsys->mix_pos[WBTMIX_TARGET_MINE2] = 0xff;
  wbtsys->mix_pos[WBTMIX_TARGET_ENEMY1] = 0xff;
  wbtsys->mix_pos[WBTMIX_TARGET_ENEMY2] = 0xff;
}

//------------------------------------------------------------------
/**
 * @brief ミックスでの選択をセット
 * @param[in]   wbtsys
 * @param[in]   target
 * @param[in]   select_pos
 */
//------------------------------------------------------------------
void WBTSYS_MixSetSelect( WBTSYS * wbtsys, WBTMIX_TARGET target, u8 select_pos )
{
  //GF_ASSERT( select_pos < WBTSYS_GetPartyMax(wbtsys) );
  GF_ASSERT( target < WBTMIX_TARGET_NUM );
  wbtsys->mix_pos[target] = select_pos;
  TAMADA_Printf(" MIX SET(%d) = %d\n", target, select_pos );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_MixSetRandomSelect( WBTSYS * wbtsys, WBTMIX_TARGET target )
{
  u8 select_pos;
  u8 max = WBTSYS_GetPartyMax(wbtsys);
  switch ( target )
  {
  case WBTMIX_TARGET_MINE1:
  case WBTMIX_TARGET_ENEMY1:
    select_pos = GFUser_GetPublicRand( max );
    break;
  case WBTMIX_TARGET_MINE2:
    select_pos = GFUser_GetPublicRand( max - 1 );
    if ( select_pos >= wbtsys->mix_pos[WBTMIX_TARGET_MINE1] )
    {
      select_pos += 1;
    }
    break;
  case WBTMIX_TARGET_ENEMY2:
    select_pos = GFUser_GetPublicRand( max - 1 );
    if ( select_pos >= wbtsys->mix_pos[WBTMIX_TARGET_ENEMY1] )
    {
      select_pos += 1;
    }
    break;
  }
  WBTSYS_MixSetSelect( wbtsys, target, select_pos );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTSYS_MixGetSelected( const WBTSYS * wbtsys, WBTMIX_TARGET target )
{
  GF_ASSERT( target < WBTMIX_TARGET_NUM );
  return wbtsys->mix_pos[target];
}


//============================================================================================
//
//  ミックストーナメント用：手持ち表示ウィンドウ
//
//============================================================================================
#include "msg/script/msg_wbt_hall.h" //msg_wbt_menu_monsname
#include "scrcmd_work.h"
//--------------------------------------------------------------
//--------------------------------------------------------------

struct _WBT_MIXWIN{
  SCRCMD_WORK * scrcmd_work;
  WBTSYS * wbtsys;
  FLDMSGWIN * fldmsgwin;

  FLDMSGBG * msgbg;
  GFL_MSGDATA * msgdata;
  WORDSET * wordset;
  STRBUF * msgbuf;
  STRBUF * tmpbuf;
};

//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  WMW_FONTX = 12,
  WMW_FONTY = 12,

  WMW_PX = 1,
  WMW_PY = 7,
  WMW_LINE_HEIGHT = WMW_FONTY + 2,
  WMW_SX = 7,
  WMW_SY = 12,

  WMW_DOT_SX = WMW_FONTX * WMW_SX,
  WMW_DOT_SY = WMW_LINE_HEIGHT * WMW_SY,

  WMW_DOTPOSX = 0,
};
//--------------------------------------------------------------
//--------------------------------------------------------------
WBT_MIXWIN * WBT_MIXWIN_Create( void * wk, WBTSYS * wbtsys )
{
  WBT_MIXWIN * wmw;
  SCRCMD_WORK * work = wk;

  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );

  wmw = GFL_HEAP_AllocClearMemory( SCRCMD_WORK_GetHeapID(work), sizeof(WBT_MIXWIN) );
  wmw->scrcmd_work = work;
  wmw->wbtsys = wbtsys;

  wmw->msgbg = SCRCMD_WORK_GetFldMsgBG( work );
  wmw->msgdata = SCRCMD_WORK_GetMsgData( work );
  wmw->wordset = SCRIPT_GetWordSet( sc );
  wmw->msgbuf = SCRIPT_GetMsgBuffer( sc );
  wmw->tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
  return wmw;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void WBT_MIXWIN_Draw( WBT_MIXWIN * wmw )
{
  POKEPARTY * my_party = WBTSYS_GetMyParty( wmw->wbtsys );

  u16 dot_sy = WMW_LINE_HEIGHT * PokeParty_GetPokeCount( my_party );

  wmw->fldmsgwin = FLDMSGWIN_Add( wmw->msgbg, wmw->msgdata,
      WMW_PX, WMW_PY, MATH_ROUNDUP( WMW_DOT_SX, 8 ) / 8, MATH_ROUNDUP( dot_sy, 8 ) / 8 );

  {
    int max = WBTSYS_GetPartyMax( wmw->wbtsys );
    int i;
    u8 sel1, sel2;
    sel1 = WBTSYS_MixGetSelected( wmw->wbtsys, WBTMIX_TARGET_MINE1 );
    sel2 = WBTSYS_MixGetSelected( wmw->wbtsys, WBTMIX_TARGET_MINE2 );

    for ( i = 0; i < max; i ++ )
    {
      u16 msg_id = msg_wbt_menu_monsname;
      POKEMON_PARAM * pp = PokeParty_GetMemberPointer( my_party, i );
      if ( i == sel1 || i == sel2 )
      {
        msg_id = msg_wbt_menu_monsname_00;
      }
      GFL_MSG_GetString( wmw->msgdata, msg_id, wmw->tmpbuf );
      WORDSET_RegisterPokeMonsName( wmw->wordset, 0, pp );
      WORDSET_ExpandStr( wmw->wordset, wmw->msgbuf, wmw->tmpbuf );
      FLDMSGWIN_PrintStrBuf( wmw->fldmsgwin,
          WMW_DOTPOSX, WMW_LINE_HEIGHT * i, wmw->msgbuf );
    }
  }
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame( FLDMSGWIN_GetBmpWin( wmw->fldmsgwin ) ) );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void WBT_MIXWIN_Erase( WBT_MIXWIN * wmw )
{
  FLDMSGWIN_ClearWindow( wmw->fldmsgwin );
  FLDMSGWIN_Delete( wmw->fldmsgwin );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void WBT_MIXWIN_Delete( WBT_MIXWIN * wmw )
{
  GFL_HEAP_FreeMemory( wmw );
}


//============================================================================================
//
//
//  ダウンロード手持ちデータの設定処理
//
//
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkDLPokemon( const WBTDL_POKEMON * dl_poke )
{
  GF_ASSERT( dl_poke );
  if ( dl_poke->monsno == 0 || dl_poke->level == 0 ) return FALSE;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   DLデータからポケモンを生成
 * @param[out]  pp
 * @param[in]   dl_poke
 * @param[in]   tempHeapID
 */
//--------------------------------------------------------------
static void setDLPokemon( POKEMON_PARAM * pp, const WBTDL_POKEMON * dl_poke, HEAPID tempHeapID )
{
  int i;
  u64 personal_rnd;
  u32 trainer_id;
  u8 tokusei = dl_poke->tokusei;
  u8 tokusei_flag;
  u32 speabi1 = POKETOOL_GetPersonalParam( dl_poke->monsno, dl_poke->form_no, POKEPER_ID_speabi1 );
  u32 speabi2 = POKETOOL_GetPersonalParam( dl_poke->monsno, dl_poke->form_no, POKEPER_ID_speabi2 );
  u32 speabi3 = POKETOOL_GetPersonalParam( dl_poke->monsno, dl_poke->form_no, POKEPER_ID_speabi3 );

#ifdef PM_DEBUG
  DEBUG_WBTDL_POKEMON_dump( dl_poke );
#endif
  
  //特性割り出し
  if ( tokusei == speabi1 )
  {
    tokusei_flag = PTL_TOKUSEI_SPEC_1;
  }
  else if ( tokusei == speabi2 )
  {
    tokusei_flag = PTL_TOKUSEI_SPEC_2;
  }
  else if ( tokusei == speabi3 )
  {
    tokusei_flag = PTL_TOKUSEI_SPEC_1;
  }
  else
  {
    OS_TPrintf("[%d]は指定できない特性です\n", tokusei );
    //GF_ASSERT( 0 );
    tokusei_flag = PTL_TOKUSEI_SPEC_1;
    tokusei = speabi1;
  }

  PP_Clear( pp );

  trainer_id = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
  //個性乱数決定
  personal_rnd = POKETOOL_CalcPersonalRandSpec( trainer_id, dl_poke->monsno, dl_poke->form_no,
      dl_poke->sex, tokusei_flag, dl_poke->rare_flag );
  PP_SetupEx( pp, dl_poke->monsno, dl_poke->level, trainer_id, PTL_SETUP_ID_AUTO, personal_rnd );

  //form_no
  PP_ChangeFormNo( pp, dl_poke->form_no );

  if ( tokusei == speabi3 )
  {
    PP_SetTokusei3( pp, dl_poke->monsno, dl_poke->form_no );
  }
  //装備アイテム設定
  PP_Put( pp, ID_PARA_item, dl_poke->item_no );

  //技設定
  for ( i = 0; i < PTL_WAZA_MAX; i ++ )
  {
    u8 point;
    u8 buf8;
    PP_Put( pp, ID_PARA_waza1 + i, dl_poke->waza_id[i] );
    buf8 = (dl_poke->pp_count >> (i*2))&0x03;
    PP_Put( pp, ID_PARA_pp_count1 + i, buf8);
    point = PP_Get( pp, ID_PARA_pp_max1 + i, NULL );
    PP_Put( pp, ID_PARA_pp1 + i, point );
  }

  //パワー乱数設定
  PP_Put( pp, ID_PARA_hp_rnd, dl_poke->hp_rnd );
  PP_Put( pp, ID_PARA_pow_rnd, dl_poke->pow_rnd );
  PP_Put( pp, ID_PARA_def_rnd, dl_poke->def_rnd );
  PP_Put( pp, ID_PARA_agi_rnd, dl_poke->agi_rnd );
  PP_Put( pp, ID_PARA_spepow_rnd, dl_poke->spepow_rnd );
  PP_Put( pp, ID_PARA_spedef_rnd, dl_poke->spedef_rnd );

  //経験値設定
  PP_Put( pp, ID_PARA_hp_exp, dl_poke->hp_exp );
  PP_Put( pp, ID_PARA_pow_exp, dl_poke->pow_exp );
  PP_Put( pp, ID_PARA_def_exp, dl_poke->def_exp );
  PP_Put( pp, ID_PARA_agi_exp, dl_poke->agi_exp );
  PP_Put( pp, ID_PARA_spepow_exp, dl_poke->spepow_exp );
  PP_Put( pp, ID_PARA_spedef_exp, dl_poke->spedef_exp );

  //なつきど設定
  PP_Put( pp, ID_PARA_friend, dl_poke->friend_rate );

  //性格セット
  PP_SetSeikaku( pp, dl_poke->seikaku );

  { //ニックネームセット
    STRBUF * nickname;
    nickname = GFL_STR_CreateBuffer( MONS_NAME_SIZE + EOM_SIZE, tempHeapID );
    GFL_STR_SetStringCodeOrderLength( nickname, dl_poke->nickname, MONS_NAME_SIZE + EOM_SIZE );
    PP_Put( pp, ID_PARA_nickname, (u32)nickname );
    GFL_STR_DeleteBuffer( nickname );
  }
  //捕まえたボール
  PP_Put( pp, ID_PARA_get_ball, dl_poke->ball_id );

  PP_Renew( pp );
}

//--------------------------------------------------------------
/**
 * @brief DLデータから手持ちポケモンパーティを生成
 * @param[out]  party
 * @param[in]   wbtdl_data
 * @param[in]   dl_tr_id
 * @param[in]   num
 * @param[in]   tempHeapID
 * 
 * @todo  データ異常の場合のエラー処理
 */
//--------------------------------------------------------------
static void setDLPokeParty( POKEPARTY * party,
    const WBTDL_DATA * wbtdl_data, u32 dl_tr_id, u32 num, HEAPID tempHeapID )
{
  int i;
  POKEMON_PARAM * entry_pp;
  const WBTDL_POKEMON * dl_array[ TEMOTI_POKEMAX ];

  GFL_STD_MemClear32( dl_array, sizeof(dl_array) );
  {
    int total;
    total = 0;
    for ( i = 0; i < TEMOTI_POKEMAX; i ++ )
    {
      const WBTDL_POKEMON * dl_poke = WBTDL_DATA_GetPokemon( wbtdl_data, dl_tr_id, i );
      if ( checkDLPokemon( dl_poke ) )
      {
        dl_array[ total ] = dl_poke;
        total ++;
      }
    }
    GF_ASSERT( total >= num );

    //shuffle;
    i = total;
    while ( i )
    {
      int rand_pos = GFUser_GetPublicRand( i );
      i --;
      {
        const WBTDL_POKEMON * temp;
        temp = dl_array[ rand_pos ];
        dl_array[rand_pos] = dl_array[ i ];
        dl_array[i] = temp;
      }
    }
  }

  PokeParty_Init( party, TEMOTI_POKEMAX );

  entry_pp = GFL_HEAP_AllocMemoryLo( tempHeapID, POKETOOL_GetWorkSize() );

  for ( i = 0; i < num; i ++ )
  {
    const WBTDL_POKEMON * dl_poke = dl_array[ i ];
    //const WBTDL_POKEMON * dl_poke = WBTDL_DATA_GetPokemon( wbtdl_data, dl_tr_id, i );
    if ( checkDLPokemon( dl_poke ) )
    {
      setDLPokemon( entry_pp, dl_poke, tempHeapID );
      PokeParty_Add( party, entry_pp );
    }
    else
    {
      //万が一、データ異常の場合の救済処理：ロムから引っ張る？
    }
  }
  GFL_HEAP_FreeMemory( entry_pp );
}

//============================================================================================
//
//  ROM手持ちデータの設定処理
//
//============================================================================================
#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/work_define.h"  //WK_SYS_FIRST_POKETYPE
#include "../../../resource/fldmapdata/script/user_define.h"    //FIRST_POKETYPE_

//--------------------------------------------------------------
/**
 * @brief 最初の3体選択結果でトレーナーIDを振り分ける
 */
//--------------------------------------------------------------
static u32 getWBTRivalTrainerID( GAMESYS_WORK * gsys )
{
  EVENTWORK * evwork = GAMEDATA_GetEventWork( GAMESYSTEM_GetGameData( gsys ) );
  u16 * first_choice = EVENTWORK_GetEventWorkAdrs( evwork, WK_SYS_FIRST_POKETYPE );
  switch ( *first_choice )
  {
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case FIRST_POKETYPE_KUSA:
    return TRID_SWRIVAL_07;
  case FIRST_POKETYPE_HONOO:
    return TRID_SWRIVAL_08;
  case FIRST_POKETYPE_MIZU:
    return TRID_SWRIVAL_09;
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 対戦相手のROMトレーナーの手持ちポケモン選択処理
 * @param[out]  btpwd       選択したポケモンデータを受け取るワークへのポインタ
 * @param[in]   wbtcup      現在のトーナメント種別
 * @param[in]   wbt_tr      対戦相手トレーナー
 * @param[in]   poke_num    選択するポケモン数
 * @param[in]   tempHeapID  テンポラリに使用するヒープ指定ID
 */
//-----------------------------------------------------------------------------
static void makeRomTrainerData(
    BSUBWAY_POKEMON * btpwd,
    WBTCUP wbtcup, const WBTTRAINER * wbt_tr,
    u8 poke_num, HEAPID tempHeapID )
{
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  u8 pow_rnd      = WBTTRAINER_GetPowerRnd(wbt_tr);
  u8 tr_id        = WBTTRAINER_GetID(wbt_tr);
  u32 base_arc_id = WBTTOOL_GetRomArcID( wbtcup );
  GFL_STD_RandContext * rand_context = NULL;
  WBT_NGPMINFO * ng_info;

  ng_info = NGINFO_Create( tempHeapID, wbtcup );
  
  if ( WBTTRAINER_HasAce(wbt_tr) == TRUE )
  { //切り札ポケモンセット処理
    u16 index_no;
    //トレーナーROMデータ取得
    trd = FBI_TOOL_GetTrainerRomData( base_arc_id + WBTBTD_ARCOFS_TR_ACE, tr_id, tempHeapID  );
    //切り札ポケモンは１体だけなので０番決めうちで取得
    index_no = trd->use_poke_table[0];
    FBI_TOOL_MakePokemonParam( btpwd,
        base_arc_id + WBTBTD_ARCOFS_PM, index_no, tr_id, 0, pow_rnd, 0, FALSE, tempHeapID  );

    WBT_NGINFO_AddNGMons( ng_info, btpwd->mons_no );
    WBT_NGINFO_AddNGItem( ng_info, btpwd->item_no );
    //切り札ポケモンとして選ばれたポケモンは抽選されないようにする
    WBT_NGINFO_AddNGIndex( ng_info, index_no );
    btpwd ++;
    poke_num --;
    GFL_HEAP_FreeMemory( trd );
  }

  { //ポケモンセット処理
    int i;
    u16 index_array[ TEMOTI_POKEMAX ];

    GFL_STD_MemClear16(index_array, sizeof(index_array));

    //トレーナーROMデータ取得
    trd = FBI_TOOL_GetTrainerRomData( base_arc_id + WBTBTD_ARCOFS_TR_NORMAL, tr_id, tempHeapID  );

    TAMADA_Printf(" WBT TrData:arc_id=%d id=%d use_poke_cnt = %d\n", base_arc_id, tr_id, trd->use_poke_cnt );
    //ポケモンデータを選択
    GF_ASSERT( trd->use_poke_cnt < ROM_POKE_MAX );
    WBTSYS_ChooseBSubwayData( index_array, ng_info,
        tempHeapID , trd->use_poke_cnt, trd->use_poke_table,
        base_arc_id + WBTBTD_ARCOFS_PM, poke_num, NULL );

    for(i = 0; i < poke_num; i++)
    {
      u32 index_no = index_array[ i ];
      FBI_TOOL_MakePokemonParam( &(btpwd[i]),
        base_arc_id + WBTBTD_ARCOFS_PM, index_no, tr_id, 0, pow_rnd, i, FALSE, tempHeapID );
    }

    GFL_HEAP_FreeMemory( trd );
  }

  WBT_NGINFO_Delete( ng_info );
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static void setRomPokeParty( POKEPARTY * party,
    const WBTTRAINER * wbt_tr, WBTCUP wbtcup, u8 num, HEAPID tempHeapID )
{
  enum {
    DUMMY_LEVEL_VALUE = 50, //バトル直前であらためてレベル調整するので何でもよい
  };

  BSUBWAY_POKEMON * btpwd;

  btpwd = GFL_HEAP_AllocMemoryLo( tempHeapID, sizeof(BSUBWAY_POKEMON) * TEMOTI_POKEMAX );
  makeRomTrainerData( btpwd, wbtcup, wbt_tr, num, tempHeapID );
  FBI_TOOL_WBT_SetPokemon( party, btpwd, DUMMY_LEVEL_VALUE, num, tempHeapID );
 
  GFL_HEAP_FreeMemory( btpwd );
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 対戦相手の手持ちポケモン設定
 * @param wbtsys
 * @param gsys      GAMESYS_WORKへのポインタ（ライバル手持ち判定に使用）
 * @param wbt_tr    対戦相手のWBTTRAINER
 */
//------------------------------------------------------------------
void WBTSYS_SetupEnemyParty( WBTSYS * wbtsys, GAMESYS_WORK * gsys, const WBTTRAINER * wbt_tr )
{

  HEAPID tempHeapID = GFL_HEAP_LOWID( wbtsys->heapID );
  int num = WBTSYS_GetPartyMax( wbtsys );
#ifdef PM_DEBUG
  u32 tempHeapSize = GFL_HEAP_GetHeapFreeSize( tempHeapID );
#endif

  switch ( wbt_tr->wbt_tr_type )
  {
  case WBT_TRTYPE_ROM:
    if ( wbt_tr->btl_tr_type == TRTYPE_SWRIVAL )
    {//ライバルの場合、ポケモンデータを上書き
      u32 normal_tr_id = getWBTRivalTrainerID( gsys );
      TT_EncountTrainerPokeDataMake( normal_tr_id, wbtsys->npc_party, tempHeapID );
    }
    else
    {
      setRomPokeParty( wbtsys->npc_party, wbt_tr, WBTSYS_GetWBTCup(wbtsys), num, tempHeapID );
    }
    break;

  case WBT_TRTYPE_DL:
  case WBT_TRTYPE_DLHYBRID:
    setDLPokeParty( wbtsys->npc_party, &wbtsys->wbtdl_data, wbt_tr->dl_tr_id, num, tempHeapID );
    break;

  default:
  case WBT_TRTYPE_PLAYER:
    GF_ASSERT( 0 );
    break;
  }
#ifdef PM_DEBUG
  GF_ASSERT( tempHeapSize == GFL_HEAP_GetHeapFreeSize( tempHeapID ) );
#endif

}

//------------------------------------------------------------------
/**
 * @brief BATTLE_SETUP_PARAM用トレーナーデータの設定
 * @param[out]  tr_data
 * @param[in]   wbt_tr
 * @param[in]   aibit
 */
//------------------------------------------------------------------
static void setBSPTrainerData( BSP_TRAINER_DATA * tr_data, const WBTTRAINER * wbt_tr )
{
  u32 aibit = 0;

#if 1
  SDK_COMPILER_ASSERT( AI_THINK_BIT_BASIC == WBT_AI_BIT_BASIC );
  SDK_COMPILER_ASSERT( AI_THINK_BIT_STRONG == WBT_AI_BIT_STRONG );
  SDK_COMPILER_ASSERT( AI_THINK_BIT_EXPERT == WBT_AI_BIT_EXPERT );
  aibit |= ( wbt_tr->ai_bit & ( WBT_AI_BIT_BASIC|WBT_AI_BIT_STRONG|WBT_AI_BIT_EXPERT) );
#else
  if ( wbt_tr->ai_bit & WBT_AI_BIT_BASIC )
  {
    aibit |= AI_THINK_BIT_BASIC;
  }
  if ( wbt_tr->ai_bit & WBT_AI_BIT_STRONG )
  {
    aibit |= AI_THINK_BIT_STRONG;
  }
  if ( wbt_tr->ai_bit & WBT_AI_BIT_EXPERT )
  {
    aibit |= AI_THINK_BIT_EXPERT;
  }
#endif
  if ( wbt_tr->ai_bit & WBT_AI_BIT_DOUBLE )
  {
    aibit |= AI_THINK_BIT_DOUBLE;
  }

  tr_data->tr_id    = wbt_tr->tr_id + TRID_OFFSET_WBT;
  tr_data->tr_type  = wbt_tr->btl_tr_type;
  tr_data->ai_bit   = aibit;

  WBTTRAINER_CopyName( wbt_tr, tr_data->name );
    
  GFL_STD_MemClear16( tr_data->use_item, sizeof(u16)*4 );
  PMSDAT_Clear( &tr_data->win_word );
  PMSDAT_Clear( &tr_data->lose_word );

}

//------------------------------------------------------------------
/**
 * @brief WBT用BATTLE_SETUP_PARAM生成
 */
//------------------------------------------------------------------
BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys )
{
  BATTLE_SETUP_PARAM * bp;

  WBTRULE wbt_rule = WBTSYS_GetRule( wbtsys );
  BtlRule rule = WBTTOOL_WBTRuleToBtlRule( wbt_rule );
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );

  bp = FBI_TOOL_WBT_CreateBattleParam( gsys, wbtsys->my_party, rule );

  //自分手持ち：ホドモエイベント以外は、レギュレーションにあわせてレベル調整
  if ( WBTSYS_GetWBTCup(wbtsys) != WBTCUP_HODOMOE_EVENT )
  {
    PokeRegulation_ModifyLevelPokeParty( WBTSYS_GetRegulation(wbtsys), bp->party[BTL_CLIENT_PLAYER] );
  }

  //対戦トレーナーの情報をセットする
  setBSPTrainerData( bp->tr_data[BTL_CLIENT_ENEMY1], wbt_tr );
  PokeParty_Copy( wbtsys->npc_party, bp->party[BTL_CLIENT_ENEMY1] );

  //相手手持ち：レギュレーションにあわせてレベル調整
  PokeRegulation_ModifyLevelPokeParty( WBTSYS_GetRegulation(wbtsys), bp->party[BTL_CLIENT_ENEMY1] );

  //肩書STRBUFにポインタをつなぐ
  BATTLE_PARAM_SetTrainerTypeName( bp, BTL_CLIENT_ENEMY1, wbt_tr->title );

  //トレーナーがバトル内で話す台詞を事前生成、セットする
  wbtsys->win_msg   = GFL_STR_CreateBuffer( WBT_TRAINER_MSG_LEN, wbtsys->heapID );
  wbtsys->lose_msg  = GFL_STR_CreateBuffer( WBT_TRAINER_MSG_LEN, wbtsys->heapID );
  WBTSYS_GetTrainerMsg( wbtsys, WBT_TRMSG_FIGHT_LOSE, wbtsys->win_msg );
  WBTSYS_GetTrainerMsg( wbtsys, WBT_TRMSG_FIGHT_WIN, wbtsys->lose_msg );
  BATTLE_PARAM_SetWBTMsg( bp, wbtsys->win_msg, wbtsys->lose_msg );

  //対戦BGMのセット
  WBTSYS_GetOpponentBGM( wbtsys, &bp->musicDefault, &bp->musicWin );
  // 戦闘背景書き換え
  if( WBTSYS_GetWBTCup(wbtsys) == WBTCUP_DOWNLOAD
      &&  wbtsys->wbtdl_data.wbtdl_rule.bgm_flag == TRUE )
  {
    bp->fieldSituation.bgType = BATTLE_BG_TYPE_WCS;
    bp->fieldSituation.bgAttr = BATTLE_BG_ATTR_E_INDOOR;
  }
  else
  {
    //
    bp->fieldSituation.bgType = BATTLE_BG_TYPE_WBT;
    bp->fieldSituation.bgAttr = BATTLE_BG_ATTR_E_INDOOR;
    //WCS背景は対応していないため、通常トーナメントのみ決勝フラグを立てる
    if (WBTSYS_GetStage( wbtsys ) == WBTSTAGE_FINAL)
    { //WBT決勝かどうかを示すフラグ
      bp->WBTFinalFlag = TRUE;
    }
  }
#ifdef  PM_DEBUG
  OS_PutString("WBT:dump Enemy Party:\n");
  DEBUG_POKEPARTY_dump( bp->party[BTL_CLIENT_ENEMY1], wbtsys->heapID ); 
#endif
  return bp;
}

//------------------------------------------------------------------
/**
 * @brief WBT用BATTLE_SETUP_PARAMを削除
 */
//------------------------------------------------------------------
void WBTSYS_DeleteBattleSetupParam( WBTSYS * wbtsys, BATTLE_SETUP_PARAM * bsp )
{
  if (wbtsys->win_msg)
  {
    GFL_STR_DeleteBuffer( wbtsys->win_msg );
  }
  if (wbtsys->lose_msg)
  {
    GFL_STR_DeleteBuffer( wbtsys->lose_msg );
  }
  BATTLE_PARAM_Delete( bsp );
}

//------------------------------------------------------------------
/**
 * @brief 戦闘結果から生き残ったポケモンの数を取得する
 * @param[in] party
 * @param[in] party_state
 * @return  u32
 */
//------------------------------------------------------------------
static u32 countAliveStatus( const POKEPARTY * party, const BtlPokeStatCode * party_state )
{
  int i;
  u32 count = 0;
  for ( i = 0; i < TEMOTI_POKEMAX; i ++ )
  {
    //OS_Printf("%d:%d\n", i, party_state[i] );
    if ( party_state[i] == BTL_POKESTATE_DEAD )
    {
      count ++;
    }
  }
  return PokeParty_GetPokeCount(party) - count;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static u8 getBattleRecordIndex( WBTSTAGE stage )
{
  int pos = 0;
  switch (stage)
  {
  default:
    GF_ASSERT( 0 );
    break;
  case WBTSTAGE_1ST:    pos = 2; break;
  case WBTSTAGE_2ND:    pos = 1; break;
  case WBTSTAGE_FINAL:  pos = 0; break;
  }
  return  pos;
}

//------------------------------------------------------------------
/**
 * @brief
 * @param[in] wbtsys
 * @param[in] stage
 * @retval    u8  その対戦での手持ちの残りの数
 */
//------------------------------------------------------------------
u8 WBTSYS_GetSurvivalCount( const WBTSYS * wbtsys, WBTSTAGE stage )
{
  u8 pos = getBattleRecordIndex( stage );
  return wbtsys->survival_count[ pos ];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTSYS_SetSurvivalCount( WBTSYS * wbtsys, WBTSTAGE stage, u8 count )
{
  u8 pos = getBattleRecordIndex( stage );
  wbtsys->survival_count[ pos ] = count;
}

//------------------------------------------------------------------
/**
 * @brief WBT戦闘結果の反映
 * @param[in,out] wbtsys
 * @param[in,out] gamedata
 */
//------------------------------------------------------------------
void WBTSYS_ReflectBattleResult( WBTSYS * wbtsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp )
{
  u16 * ret_wk = WBTSYS_GetScriptAnswerWork( wbtsys );
  u32 result = GAMEDATA_GetLastBattleResult( gamedata ); 
  u32 my_count, enemy_count;

  enemy_count = countAliveStatus( bsp->party[BTL_CLIENT_ENEMY1], bsp->party_state[BTL_CLIENT_ENEMY1] );
  my_count = countAliveStatus( bsp->party[BTL_CLIENT_PLAYER], bsp->party_state[BTL_CLIENT_PLAYER] );

  TAMADA_Printf("my_count = %d, enemy_count = %d\n", my_count, enemy_count );

  //生き残った手持ちの数をセットする
  WBTSYS_SetSurvivalCount( wbtsys, wbtsys->stage, my_count );

  switch ( result )
  {
  default:
    GF_ASSERT_MSG( 0, "想定外のBtlResult(%d)\n", result );
  case BTL_RESULT_RUN:  //自分が逃げた
    result = BTL_RESULT_LOSE;
    break;
  case BTL_RESULT_LOSE:
  case BTL_RESULT_WIN:
  case BTL_RESULT_DRAW:
    break;
  }

  OS_Printf("WBT: is Final = %d\n", bsp->WBTFinalFlag );
  {
    const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
    WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );

    if ( wbtsys->wbtcup != WBTCUP_DOWNLOAD && wbt_tr->tr_id < WBTROM_TRID_MEM_MAX )
    {
      //対戦相手のトレーナーIDを保存する
      u8 pos = getBattleRecordIndex( wbtsys->stage );
      WBTSAVE_SetTrainerID( wbtsave, pos, wbt_tr->tr_id );

      //戦ったかどうかのフラグを反映
      WBTSAVE_SetFlag( wbtsave, wbt_tr->tr_id );
    }
    if ( result == BTL_RESULT_LOSE || wbtsys->isTutorial == TRUE )
    { //敗北した場合orチュートリアル中は無効なトレーナーIDをセットしておく
      u8 pos;
      for ( pos = 0; pos < WBT_BATTLE_NUM; pos ++ )
      {
        WBTSAVE_SetTrainerID( wbtsave, pos, WBTROM_TRID_INVALID );
      }
    }
  }
  *ret_wk = result;
}



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 対戦相手のトレーナーデータを取得
 */
//------------------------------------------------------------------
const WBTTRAINER * WBTSYS_GetOpponentTrainer( const WBTSYS * wbtsys )
{
  u32 pos = WBTSYS_GetOpponentPos( wbtsys );
  const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, pos );
  return wbt_tr;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の性別を取得する
 */
//------------------------------------------------------------------
u8 WBTSYS_GetOpponentSex( const WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->sex;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の動作モデル表示コードを取得する
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentMMdlObjCode( const WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->mmdl_id;
}
//------------------------------------------------------------------
/**
 * @brief 対戦相手の名前を取得する
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentName( const WBTSYS * wbtsys, STRBUF * strbuf )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  WBTTRAINER_CopyName( wbt_tr, strbuf );
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手のトレーナータイプを取得する
 * @param wbtsys
 * @return  u16 tr_type(include/tr_tool/trtype_def.h参照）
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentTrainerType( const WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->btl_tr_type;
}

//------------------------------------------------------------------
/**
 * @brief 戦闘BGMの取得
 * @param[in]   wbtsys
 * @param[out]  btl_bgm
 * @param[out]  win_bgm
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentBGM( const WBTSYS * wbtsys, u16 *btl_bgm, u16 *win_bgm )
{
  const WBTTRAINER * opponent = WBTSYS_GetOpponentTrainer( wbtsys );
  WBT_TRTYPE wbt_tr_type      = WBTTRAINER_GetWBTTrainerType( opponent );

  if ( wbtsys->wbtcup == WBTCUP_DOWNLOAD
      && wbtsys->wbtdl_data.wbtdl_rule.bgm_flag )
  {
    *btl_bgm = SEQ_BGM_VS_WCS;
    *win_bgm = SEQ_BGM_WIN5;
    return;
  }

  switch ( wbt_tr_type )
  {
  default:
    GF_ASSERT( 0 );

  case WBT_TRTYPE_DL:
    *btl_bgm = SEQ_BGM_VS_WBT;
    *win_bgm = SEQ_BGM_WIN_WBT;
    break;

  case WBT_TRTYPE_DLHYBRID:
  case WBT_TRTYPE_ROM:
    {
      WBT_TRDATA tr_rom_data;
      WBTTOOL_GetRomTrainerData( wbtsys->rom_trdata, opponent->tr_id, &tr_rom_data );
      *btl_bgm = tr_rom_data.btl_bgm_id;
      *win_bgm = tr_rom_data.win_bgm_id;
      //チャンピオン戦以外で決勝の場合はWBT専用曲にする
      if ( wbtsys->stage == WBTSTAGE_FINAL && wbtsys->wbtcup != WBTCUP_CHAMPION )
      {
        *btl_bgm = SEQ_BGM_VS_WBT;
        *win_bgm = SEQ_BGM_WIN_WBT;
      }
    }
    break;
  }
}

//============================================================================================
//============================================================================================
#include "msg/msg_wbt_trmsg.h"
#include "../../../resource/wbt/wbt_trainer/wbt_trmsg_id.cdat"

//------------------------------------------------------------------
/**
 * @brief ROMトレーナーに対応したメッセージを取得する
 *
 * @note
 * レンタルトーナメントの場合はメッセージを変える必要があるため
 */
//------------------------------------------------------------------
static void get_rom_msg( STRBUF * strbuf,
    HEAPID tempHeapID, WBTCUP wbtcup, const WBTTRAINER * opponent, WBT_TRMSG msg_type )
{
  WBTROM_TRID tr_id;
  GFL_MSGDATA * msgData;
  u16 msg_id;
  u8 idx = 0;

  if ( wbtcup == WBTCUP_RENTAL || wbtcup == WBTCUP_RENTALMASTER )
  { //レンタルのときはメッセージの種類を変えるトレーナーもいるため
    idx = 1;
  }
  tr_id = WBTTRAINER_GetID( opponent );
  msg_id = WBTTrainerMsgID[tr_id][idx] + msg_type;
  OS_Printf("tr_id = %2d msg_id = %3d\n", tr_id, msg_id );
  GF_ASSERT( tr_id < NELEMS(WBTTrainerMsgID) );
  msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_wbt_trmsg_dat, tempHeapID );
  GFL_MSG_GetString( msgData, msg_id, strbuf );
  GFL_MSG_Delete( msgData );
}

//------------------------------------------------------------------
/**
 * @brief DLトレーナーに対応したメッセージを取得する
 * @param[in]   wbtsys      WBTシステムへのポインタ
 * @param[in]   dl_tr_id    DLトレーナー指定ID
 * @param[in]   msg_type    取得したいメッセージの種別（WBT_TRMSG参照）
 * @param[out]  preset_id プリセットMSG指定
 * @return      STRCODE メッセージ文字列へのポインタ
 */
//------------------------------------------------------------------
static const STRCODE * get_dl_msg(
    const WBTSYS * wbtsys, u8 dl_tr_id, WBT_TRMSG msg_type, u16 * preset_id )
{
  const WBTDL_CHARACTER * dl_char = WBTDL_DATA_GetCharacter( &wbtsys->wbtdl_data, dl_tr_id );
  const STRCODE * freeword = NULL;
  *preset_id = 1;

#ifdef PM_DEBUG
  DEBUG_WBTDL_CHARACTER_dump( dl_char );
#endif
  switch ( msg_type )
  {
  case WBT_TRMSG_1ST_FIGHT_START:
  case WBT_TRMSG_2ND_FIGHT_START:
  case WBT_TRMSG_3RD_FIGHT_START:
    *preset_id = dl_char->msg_before_id;
    freeword  = dl_char->msg_before;
    break;
  case WBT_TRMSG_FIGHT_LOSE:
    *preset_id = dl_char->msg_win_id;
    freeword  = NULL;
    break;
  case WBT_TRMSG_FIGHT_WIN:
    *preset_id = dl_char->msg_lose_id;
    freeword  = NULL;
    break;
  case WBT_TRMSG_PLAYER_WIN:
    *preset_id = dl_char->msg_after_win_id;
    freeword  = dl_char->msg_after_win;
    break;
  case WBT_TRMSG_PLAYER_LOSE:
    *preset_id = dl_char->msg_after_lose_id;
    freeword  = dl_char->msg_after_lose;
    break;
  case WBT_TRMSG_ANNOUNCE:
    *preset_id = dl_char->msg_announce_id;
    freeword  = dl_char->msg_announce;
    break;
  }

  if ( *preset_id == 0 && freeword == NULL )
  {
    GF_ASSERT( 0 );
    *preset_id = 1;  //エラー対処
  }
  return freeword;
}

//------------------------------------------------------------------
/**
 * @brief       WBTトレーナー用メッセージの取得
 * @param[in]   wbtsys
 * @param[in]   msg_type
 * @param[out]  strbuf
 */
//------------------------------------------------------------------
void WBTSYS_GetTrainerMsg( const WBTSYS * wbtsys, WBT_TRMSG msg_type, STRBUF * strbuf )
{
  HEAPID tempHeapID = wbtsys->heapID;
  const WBTTRAINER * opponent = WBTSYS_GetOpponentTrainer( wbtsys );
  WBT_TRTYPE wbt_tr_type = WBTTRAINER_GetWBTTrainerType( opponent );

  if ( msg_type >= WBT_TRMSG_TYPE_NUM )
  {
    GF_ASSERT( 0 );
    return;
  }

  switch ( wbt_tr_type )
  {
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */

  case WBT_TRTYPE_ROM:
    get_rom_msg( strbuf, tempHeapID, wbtsys->wbtcup, opponent, msg_type );
    return;

  case WBT_TRTYPE_DLHYBRID:
    {
      u16 dl_tr_id = WBTTRAINER_GetDLID( opponent );
      u16 preset_id = 1;
      const STRCODE * freeword = NULL;
      freeword = get_dl_msg( wbtsys, dl_tr_id, msg_type, &preset_id );
      if ( preset_id == 0 )
      {
        GFL_STR_SetStringCodeOrderLength( strbuf, freeword, WBTDL_MSG_LEN );
      }
      else
      { //フリーワードでない場合はデフォルトのROMメッセージを使用する
        get_rom_msg( strbuf, tempHeapID, wbtsys->wbtcup, opponent, msg_type );
      }
    }
    break;

  case WBT_TRTYPE_DL:
    {
      u16 dl_tr_id = WBTTRAINER_GetDLID( opponent );
      u16 preset_id = 1;
      const STRCODE * freeword = NULL;
      freeword = get_dl_msg( wbtsys, dl_tr_id, msg_type, &preset_id );
      if ( preset_id == 0 )
      {
        GFL_STR_SetStringCodeOrderLength( strbuf, freeword, WBTDL_MSG_LEN );
      }
      else
      {
        u16 msg_id = ( preset_id - 1 ) * WBT_TRMSG_TYPE_NUM + msg_type;
        GFL_MSGDATA * msgData;
        msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
            NARC_message_wbt_trmsg_dl_dat, tempHeapID );
        GFL_MSG_GetString( msgData, msg_id, strbuf );
        GFL_MSG_Delete( msgData );
      }
    }
    break;
  }
}

