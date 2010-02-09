//======================================================================
/**
 * @file	scrcmd_pokemon_fld.c
 * @brief	スクリプトコマンド：ポケモン関連：フィールドでしか使用しないもの
 * @date  2010.01.23
 * @author	tamada GAMEFREAK inc.
 *
 * サブイベントで限定使用されるものなど、常時呼び出さなくても大丈夫なものは
 * scrcmd_pokemon.cでなくこちらに配置する
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_pokemon.h"
#include "scrcmd_pokemon_fld.h"

#include "savedata/misc.h"
#include "pm_define.h"  //TEMOTI_POKEMAX

#include "waza_tool/wazano_def.h" // for WAZANO_xxxx
#include "event_poke_status.h" // for EvCmdPartyPokeSelect
//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief お気に入りポケモンのセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetFavoritePoke( VMHANDLE * core, void *wk )
{

  u16 pos = SCRCMD_GetVMWorkValue( core, wk );  //ポケモンの位置
  POKEMON_PARAM * pp;

  if ( SCRCMD_GetTemotiPP( wk, pos, &pp ) == TRUE )
  { //エラー対処
    GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
    MISC * misc = GAMEDATA_GetMiscWork( gamedata );
    u16 monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    u16 form_no = PP_Get( pp, ID_PARA_form_no, NULL );
    u16 egg_flag = PP_Get( pp, ID_PARA_tamago_flag, NULL );

    MISC_SetFavoriteMonsno( misc, monsno, form_no, egg_flag );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief ジムリーダー戦勝利情報のセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetGymVictoryInfo( VMHANDLE * core, void *wk )
{
  int i;
  u16 monsnos[TEMOTI_POKEMAX];
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );

  u16 badge_id = SCRCMD_GetVMWorkValue( core, wk );
  for ( i = 0 ; i < TEMOTI_POKEMAX; i++ )
  {
    monsnos[i] = 0;
  }
  for (i = 0; i < PokeParty_GetPokeCount( party ); i++)
  {
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
    if ( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == FALSE )
    {
      monsnos[i] = PP_Get( pp, ID_PARA_monsno, NULL);
    }
  }
  MISC_SetGymVictoryInfo( GAMEDATA_GetMiscWork(gamedata), badge_id, monsnos );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief ジムリーダー戦勝利情報の取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetGymVictoryInfo( VMHANDLE * core, void *wk )
{
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  SCRCMD_WORK*       work = wk;
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*        wordset = SCRIPT_GetWordSet( sc );

  int i, count;
  u16 badge_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );  //結果格納ワーク
  u16 monsnos[TEMOTI_POKEMAX];

  MISC_GetGymVictoryInfo( GAMEDATA_GetMiscWork(gamedata), badge_id, monsnos );
  for ( i = 0, count = 0; i < TEMOTI_POKEMAX; i++ )
  {
    if (monsnos[i] != 0)
    {
      WORDSET_RegisterPokeMonsNameNo( wordset, i, monsnos[i] );
      count ++;
    }
  }
  *ret_wk = count;
  return VMCMD_RESULT_CONTINUE;
}



//--------------------------------------------------------------
/**
 * @brief   ジャッジイベントの判定データ取得コマンド
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetBreederJudgeResult( VMHANDLE * core, void *wk )
{
  u16 max_val;  //0～31
  u16 total;
  u16 max_bits;
  u32 i;
  u16 temp[6];
  POKEMON_PARAM* pp;
  u16 pos         = SCRCMD_GetVMWorkValue( core, wk );    //手持ちの何番目かを代入しておく
  u16 mode        = SCRCMD_GetVMWorkValue( core, wk );    //取得したい値の指定
  u16* ret_wk    = SCRCMD_GetVMWork( core, wk );   //パワー乱数の合計

  //ポケモンへのポインタ取得
  //
  if (SCRCMD_GetTemotiPP( wk, pos, &pp ) == FALSE )
  { //エラー対処
    *ret_wk = 0;
    return VMCMD_RESULT_CONTINUE;
  }

  //パワー乱数取得
  temp[0] = PP_Get( pp, ID_PARA_hp_rnd, NULL );
  temp[1] = PP_Get( pp, ID_PARA_pow_rnd, NULL );
  temp[2] = PP_Get( pp, ID_PARA_def_rnd, NULL );
  temp[3] = PP_Get( pp, ID_PARA_agi_rnd, NULL );
  temp[4] = PP_Get( pp, ID_PARA_spepow_rnd, NULL );
  temp[5] = PP_Get( pp, ID_PARA_spedef_rnd, NULL );

  //パワー乱数の合計ならびに、最高値を調べる
  total = 0;
  max_val = 0;
  for( i=0; i < 6 ;i++ )
  {
    total += temp[i];
    if (max_val < temp[i]){
      max_val = temp[i];
    }
  }
  switch ( mode )
  {
  case SCR_JUDGE_MODE_TOTAL:
    *ret_wk = total;
    break;
  case SCR_JUDGE_MODE_HIGH_VALUE:
    *ret_wk = max_val;
    break;
  case SCR_JUDGE_MODE_HP:
    *ret_wk = temp[0];
    break;
  case SCR_JUDGE_MODE_POW:
    *ret_wk = temp[1];
    break;
  case SCR_JUDGE_MODE_DEF:
    *ret_wk = temp[2];
    break;
  case SCR_JUDGE_MODE_AGI:
    *ret_wk = temp[3];
    break;
  case SCR_JUDGE_MODE_SPPOW:
    *ret_wk = temp[4];
    break;
  case SCR_JUDGE_MODE_SPDEF:
    *ret_wk = temp[5];
    break;
  default:
    GF_ASSERT_MSG( 0, "未対応のSCR_JUDGE_MODE(%d)です\n", mode );
  }

  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//
//
//
//======================================================================
#include "msg/script/msg_hyouka_scr.h"

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct{
  u16 range;
  u16 msg_id;
} ZUKAN_HYOUKA_TABLE;

static const ZUKAN_HYOUKA_TABLE LocalSeeTable[] = {
  {  14, msg_hyouka_i00 },
  {  24, msg_hyouka_i01 },
  {  39, msg_hyouka_i02 },
  {  59, msg_hyouka_i03 },
  {  89, msg_hyouka_i04 },
  { 114, msg_hyouka_i05 },
  { 134, msg_hyouka_i06 },
  { 144, msg_hyouka_i07 },
  { 149, msg_hyouka_i08 },
  { 150, msg_hyouka_i09 },
  {   0, msg_hyouka_i00 },  //Sentinel
};

static const ZUKAN_HYOUKA_TABLE LocalGetTable[] = {
  {  29, msg_hyouka_i10 },
  {  49, msg_hyouka_i11 },
  {  69, msg_hyouka_i12 },
  {  89, msg_hyouka_i13 },
  { 104, msg_hyouka_i14 },
  { 119, msg_hyouka_i15 },
  { 134, msg_hyouka_i16 },
  { 144, msg_hyouka_i17 },
  { 149, msg_hyouka_i18 },
  { 150, msg_hyouka_i19 },
  {   0, msg_hyouka_i10 },  //Sentinel
};

static const ZUKAN_HYOUKA_TABLE GlobalGetTable[] = {
  {  39, msg_hyouka_z01 },
  {  79, msg_hyouka_z02 },
  { 119, msg_hyouka_z03 },
  { 159, msg_hyouka_z04 },
  { 199, msg_hyouka_z05 },
  { 249, msg_hyouka_z06 },
  { 299, msg_hyouka_z07 },
  { 349, msg_hyouka_z08 },
  { 399, msg_hyouka_z09 },
  { 449, msg_hyouka_z10 },
  { 499, msg_hyouka_z11 },
  { 549, msg_hyouka_z12 },
  { 599, msg_hyouka_z13 },
  { 614, msg_hyouka_z14 },
  { 629, msg_hyouka_z15 },
  { 633, msg_hyouka_z16 },
  { 634, msg_hyouka_z17 },
  {   0, msg_hyouka_z01 },  //Sentinel
};

//--------------------------------------------------------------
/**
 * @brief ずかん評価
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetZukanHyouka( VMHANDLE * core, void *wk )
{
  u16     mode = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_msgid = SCRCMD_GetVMWork( core, wk );
  u16 * ret_count = SCRCMD_GetVMWork( core, wk );

  GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
  ZUKAN_SAVEDATA * zw = GAMEDATA_GetZukanSave( gamedata );
  BOOL zenkoku_flag = ZUKANSAVE_GetZenkokuZukanFlag( zw );
  u32 count = 0;
  int i;
  const ZUKAN_HYOUKA_TABLE * tbl;

  *ret_msgid = msg_hyouka_i01; //念のため
  *ret_count = 0;              //念のため
  switch ( mode )
  {
  case SCR_ZUKAN_HYOUKA_MODE_AUTO:  //パソコン
    if ( zenkoku_flag == FALSE )
    {
      //ちほうずかん（全国モード前）は「見た数」
      count = ZUKANSAVE_GetLocalPokeSeeCount( zw );
      tbl = LocalSeeTable;
    }
    else
    {
      //ぜんこくずかんは「捕まえた数」
      count = ZUKANSAVE_GetZukanPokeGetCount( zw );
      tbl = GlobalGetTable;
    }
    break;

  case SCR_ZUKAN_HYOUKA_MODE_LOCAL: //博士（娘）
    if ( zenkoku_flag == TRUE )
    {
      //ちほうずかん（全国モード後）は「捕まえた数」
      count = ZUKANSAVE_GetLocalPokeGetCount( zw );
      tbl = LocalGetTable;
    }
    else
    {
      //ちほうずかん（全国モード前）は「見た数」
      count = ZUKANSAVE_GetLocalPokeSeeCount( zw );
      tbl = LocalSeeTable;
    }
    break;

  case SCR_ZUKAN_HYOUKA_MODE_GLOBAL:  //博士（父）
    if ( zenkoku_flag == TRUE )
    {
      //ぜんこくずかんは「捕まえた数」
      count = ZUKANSAVE_GetZukanPokeGetCount( zw );
      tbl = GlobalGetTable;
    }
    else
    {
      GF_ASSERT( 0 );
      return VMCMD_RESULT_CONTINUE;
    }
    break;

  default:
    GF_ASSERT( 0 );
    return VMCMD_RESULT_CONTINUE;
  }

  *ret_count = count;

  for ( i = 0; ; i ++ )
  {
    if ( tbl[i].range == 0 )
    { //番兵：万が一テーブルにない値が来た場合
      *ret_msgid = tbl[i].msg_id;
      break;
    }
    if ( count <= tbl[i].range )
    { //範囲以下の場合、その対応メッセージIDを返す
      *ret_msgid = tbl[i].msg_id;
      break;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//
//    技覚え関連
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  SKILLTEACH_ID_COALESCENCE_KUSA = 0,
  SKILLTEACH_ID_COALESCENCE_HONOO,
  SKILLTEACH_ID_COALESCENCE_MIZU,

  SKILLTEACH_ID_STRONGEST_KUSA,
  SKILLTEACH_ID_STRONGEST_HONOO,
  SKILLTEACH_ID_STRONGEST_MIZU,

  SKILLTEACH_ID_DRAGON,

  SKILLTEACH_ID_MAX,
};

//--------------------------------------------------------------
/**
 * @brief 教える技ナンバーを取得する
 * @param pp    対象となるポケモン
 * @param mode  技覚えモード指定
 * @return  u16 技ナンバー
 */
//--------------------------------------------------------------
static u16 get_teach_waza( const POKEMON_PARAM * pp, u16 mode )
{
  int i;
  u16 monsno;

  switch ( mode )
  {
  case SCR_SKILLTEACH_MODE_STRONGEST:
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_STRONGEST_KUSA ) )
    {
      return WAZANO_HAADOPURANTO;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_STRONGEST_HONOO ) )
    {
      return WAZANO_BURASUTOBAAN;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_STRONGEST_MIZU ) )
    {
      return WAZANO_HAIDOROKANON;
    }
    GF_ASSERT(0);
    break;

  case SCR_SKILLTEACH_MODE_DRAGON:
    return WAZANO_RYUUSEIGUN;

  case SCR_SKILLTEACH_MODE_COALESCENCE:
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_COALESCENCE_KUSA ) )
    {
      return WAZANO_KUSANOTIKAI;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_COALESCENCE_HONOO ) )
    {
      return WAZANO_HONOONOTIKAI;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_COALESCENCE_MIZU ) )
    {
      return WAZANO_MIZUNOTIKAI;
    }
    GF_ASSERT(0);
    break;

  default:
    GF_ASSERT(0);
  }
  return WAZANO_BURASUTOBAAN;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 get_teach_limit_friendly( u16 mode )
{
  /* とりあえず、現状は最大値必要としている */
  return PTL_FRIEND_MAX;
}

//--------------------------------------------------------------
/**
 * @brief 技を教えられるかどうかのチェック
 * @param pp    対象となるポケモン
 * @param mode  技覚えモード指定
 * @return  BOOL  TRUEの時＝教えられる、FALSEの時＝教えられない
 */
//--------------------------------------------------------------
static BOOL check_teach_mons( const POKEMON_PARAM * pp, u16 mode )
{
  int i;

  switch ( mode )
  {
  case SCR_SKILLTEACH_MODE_STRONGEST:
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_STRONGEST_KUSA ) )
    {
      return TRUE;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_STRONGEST_HONOO ) )
    {
      return TRUE;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_STRONGEST_MIZU ) )
    {
      return TRUE;
    }
    return FALSE;

  case SCR_SKILLTEACH_MODE_DRAGON:
#if 1
    {
      u16 type1 = PP_Get( pp, ID_PARA_type1, NULL );
      u16 type2 = PP_Get( pp, ID_PARA_type2, NULL );
      if ( type1 == POKETYPE_DRAGON && type2 == POKETYPE_DRAGON )
      {
        return TRUE;
      }
    }
#else
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_DRAGON ) )
    {
      return TRUE;
    }
#endif
    return FALSE;

  case SCR_SKILLTEACH_MODE_COALESCENCE:
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_COALESCENCE_KUSA ) )
    {
      return TRUE;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_COALESCENCE_HONOO ) )
    {
      return TRUE;
    }
    if ( PP_CheckWazaOshie( pp, SKILLTEACH_ID_COALESCENCE_MIZU ) )
    {
      return TRUE;
    }
    return FALSE;
  }
  GF_ASSERT(0);
  return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちに技教えが可能かどうかの判定
 * @param core
 * @param wk
 * @param mode    技教えモード指定
 * @param oboeBit ビット単位でどのポケモンに教えられるか？を返すためのワーク
 *
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_OK  教えられる
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_POKEMON_NG  対象ポケモンがいない
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_NATSUKI_NG  なつきが足りない
 */
//--------------------------------------------------------------
static u32 checkSkillTeach( VMHANDLE * core, void * wk, u16 mode, u8 * oboeBit )
{
  GAMEDATA*           gdata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY*          party = GAMEDATA_GetMyPokemon( gdata );
  int                   max = PokeParty_GetPokeCount( party );
  BOOL     has_pokemon_flag = FALSE;
  BOOL  enough_natsuki_flag = FALSE;

  int pos;

  * oboeBit = 0;
  for ( pos = 0; pos < max; pos++ )
  {
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, pos );
    if ( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == TRUE)
    {
      continue;
    }
    if ( check_teach_mons( pp, mode ) == FALSE )
    {
      continue;
    }
    has_pokemon_flag = TRUE;
    if ( PP_Get( pp, ID_PARA_friend, NULL ) >= get_teach_limit_friendly( mode ) )
    {
      enough_natsuki_flag = TRUE;
      *oboeBit |= ( 1 << pos );
    }
  }

  if ( has_pokemon_flag == FALSE )
  {
    return SCR_SKILLTEACH_CHECK_RESULT_POKEMON_NG;
  }
  if ( enough_natsuki_flag == FALSE )
  {
    return SCR_SKILLTEACH_CHECK_RESULT_NATSUKI_NG;
  }
  return SCR_SKILLTEACH_CHECK_RESULT_OK;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちに技を教える対象がいるか？のチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSkillTeachCheckParty( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16 mode = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  u8 oboeBit;

  *ret_wk = checkSkillTeach( core, wk, mode, &oboeBit );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 教える技IDを取得する
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSkillTeachGetWazaID( VMHANDLE * core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16 mode = SCRCMD_GetVMWorkValue( core, work );
  u16 pos = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  POKEMON_PARAM * pp;

  if ( SCRCMD_GetTemotiPP( work, pos, &pp ) == TRUE )
  {
    *ret_wk = get_teach_waza( pp, mode );
  }
  else
  {
    *ret_wk = 0;
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSkillTeachCheckPokemon( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16 mode = SCRCMD_GetVMWorkValue( core, work );
  u16 pos = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  POKEMON_PARAM * pp;
  
  *ret_wk = SCR_SKILLTEACH_CHECK_RESULT_POKEMON_NG; //エラー対処用

  if ( SCRCMD_GetTemotiPP( work, pos, &pp ) == FALSE )
  { //エラー対処
    return VMCMD_RESULT_CONTINUE;
  }

  if ( check_teach_mons( pp, mode ) == FALSE )
  {
    //対象となるポケモンでない
    *ret_wk = SCR_SKILLTEACH_CHECK_RESULT_POKEMON_NG;
  }
  else if ( SCRCMD_CheckTemotiWaza( pp, get_teach_waza( pp, mode ) ) == TRUE )
  {
    //すでに覚えている
    *ret_wk = SCR_SKILLTEACH_CHECK_RESULT_ALREADY_NG;
  }
  else if ( PP_Get( pp, ID_PARA_friend, NULL ) < get_teach_limit_friendly( mode ) )
  {
    //なつきが足りない
    *ret_wk = SCR_SKILLTEACH_CHECK_RESULT_NATSUKI_NG;
  }
  else
  {
    //教えることができる！
    *ret_wk = SCR_SKILLTEACH_CHECK_RESULT_OK;
  }

  return  VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   技教え対象のポケモンを選択する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSkillTeachSelectPokemon( VMHANDLE * core, void * wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16                mode = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  u16*         ret_decide = SCRCMD_GetVMWork( core, work );       // コマンド第2引数
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第3引数
  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );

  u8 learnBit;
  if ( checkSkillTeach( core, wk, mode, &learnBit ) != SCR_SKILLTEACH_CHECK_RESULT_OK )
  {
    *ret_decide = FALSE;
  }
  else
  {
    GMEVENT *event = EVENT_CreatePokeSelect( gsys , ret_decide , ret_wk );
    //GMEVENT *event = EVENT_CreatePokeSelectWazaOboe( gsys , ret_decide , ret_wk, learnBit );
    SCRIPT_CallEvent( scw, event );
  }
  
  return VMCMD_RESULT_SUSPEND;
}














