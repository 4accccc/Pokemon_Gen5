//=============================================================================================
/**
 * @file  btl_string.c
 * @brief ポケモンWB バトル 文字列生成処理
 * @author  taya
 *
 * @date  2008.10.06  作成
 */
//=============================================================================================
#include <gflib.h>

#include "print\wordset.h"
#include "waza_tool\wazadata.h"

#include "btl_common.h"
#include "btl_string.h"

#include "arc_def.h"
#include "message.naix"
#include "msg\msg_btl_std.h"
#include "msg\msg_btl_set.h"
#include "msg\msg_btl_attack.h"
#include "msg\msg_btl_ui.h"
#include "msg\msg_wazaname.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/

enum {
  TMP_STRBUF_SIZE = 256,
  MSG_ARG_MAX = 8,
};

//------------------------------------------------------
/**
 *  WORDSETバッファ用途
 */
//------------------------------------------------------
typedef enum {

  BUFIDX_POKE_1ST = 0,
  BUFIDX_POKE_2ND,
  BUFIDX_TOKUSEI,

}WordBufID;

//------------------------------------------------------
/**
 *  同内容メッセージセットの分類
 */
//------------------------------------------------------
typedef enum {

  SETTYPE_MINE = 0, ///< 自分のポケ
  SETTYPE_WILD,   ///< 野生のポケ
  SETTYPE_ENEMY,    ///< 相手のポケ

  SETTYPE_MAX,

}SetStrFormat;

//------------------------------------------------------
/**
 *  文字列リソースID
 */
//------------------------------------------------------
typedef enum {
  MSGSRC_STD = 0,
  MSGSRC_SET,
  MSGSRC_ATK,
  MSGSRC_UI,
  MSGSRC_TOKUSEI,

  MSGDATA_MAX,
}MsgSrcID;


//------------------------------------------------------------------------------
/**
 *
 *  システムワーク
 *
 */
//------------------------------------------------------------------------------
static struct {

  const BTL_MAIN_MODULE*  mainModule;   ///< メインモジュール
  const BTL_CLIENT*       client;       ///< UIクライアント
  const BTL_POKE_CONTAINER* pokeCon;    ///< ポケモンコンテナ
  WORDSET*          wset;               ///< WORDSET
  STRBUF*           tmpBuf;             ///< 文字列一時展開用バッファ
  GFL_MSGDATA*      msg[ MSGDATA_MAX ]; ///< メッセージデータハンドル
  int               args[ BTL_STR_ARG_MAX ];  ///< 引数保存配列

  HEAPID          heapID;               ///< ヒープID
  u8              clientID;             ///< UIクライアントID

}SysWork;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline void register_PokeNickname( u8 pokeID, WordBufID bufID );
static inline SetStrFormat get_strFormat( u8 pokeID );
static inline u16 get_setStrID( u8 pokeID, u16 defaultStrID );
static inline u16 get_setPtnStrID( u8 pokeID, u16 originStrID, u8 ptnNum );
static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID );
static void ms_encount( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_encount_double( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_single( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_double( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_single_enemy( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_select_action_ready( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_out_member1( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_kodawari_lock( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_waza_lock( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_waza_only( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_side_eff( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_set_std( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankup( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankdown( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rank_limit( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke( STRBUF* dst, u16 strID, const int* args );
static void ms_set_trace( STRBUF* dst, u16 strID, const int* args );
static void ms_set_yotimu( STRBUF* dst, u16 strID, const int* args );
static void ms_set_omitoosi( STRBUF* dst, u16 strID, const int* args );
static void ms_set_change_poke_type( STRBUF* dst, u16 strID, const int* args );
static void ms_set_item_common( STRBUF* dst, u16 strID, const int* args );
static void ms_set_kinomi_rankup( STRBUF* dst, u16 strID, const int* args );
static void ms_set_waza_sp( STRBUF* dst, u16 strID, const int* args );
static void ms_set_waza_num( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke_num( STRBUF* dst, u16 strID, const int* args );
static void ms_set_item_recover_pp( STRBUF* dst, u16 strID, const int* args );




//=============================================================================================
/**
 * 文字列生成システム初期化（バトル起動直後に１回だけ呼ぶ）
 *
 * @param   mainModule
 * @param   client
 * @param   heapID
 *
 */
//=============================================================================================
void BTL_STR_InitSystem( const BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, const BTL_POKE_CONTAINER* pokeCon, HEAPID heapID )
{
  static const u16 msgDataID[] = {
    NARC_message_btl_std_dat,
    NARC_message_btl_set_dat,
    NARC_message_btl_attack_dat,
    NARC_message_btl_ui_dat,
    NARC_message_tokusei_dat,
  };

  int i;

  SysWork.mainModule = mainModule;
  SysWork.client = client;
  SysWork.pokeCon = pokeCon;
  SysWork.heapID = heapID;
  SysWork.clientID = BTL_CLIENT_GetClientID( client );

  SysWork.wset = WORDSET_Create( heapID );
  SysWork.tmpBuf = GFL_STR_CreateBuffer( TMP_STRBUF_SIZE, heapID );

  for(i=0; i<MSGDATA_MAX; i++)
  {
    SysWork.msg[i] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msgDataID[i], heapID );
  }
}

//=============================================================================================
/**
 * 文字列生成システム終了
 */
//=============================================================================================
void BTL_STR_QuitSystem( void )
{
  WORDSET_Delete( SysWork.wset );
  GFL_STR_DeleteBuffer( SysWork.tmpBuf );

  {
    int i;

    for(i=0; i<MSGDATA_MAX; i++)
    {
      GFL_MSG_Delete( SysWork.msg[i] );
    }
  }
}

//--------------------------------------------------------------------------------------

static inline void register_PokeNickname( u8 pokeID, WordBufID bufID )
{
  const BTL_POKEPARAM* bpp;
  const POKEMON_PARAM* pp;

  bpp = BTL_POKECON_GetPokeParamConst( SysWork.pokeCon, pokeID );
  pp = BPP_GetSrcData( bpp );

  WORDSET_RegisterPokeNickName( SysWork.wset, bufID, pp );
}

//--------------------------------------------------------------------------
/**
 * ポケモンIDから、「自分の・野生の・相手の」いずれのパターンか判定
 *
 * @param   pokeID
 *
 * @retval  SetStrFormat
 */
//--------------------------------------------------------------------------
static inline SetStrFormat get_strFormat( u8 pokeID )
{
  u8 targetClientID = BTL_MAIN_PokeIDtoClientID( SysWork.mainModule, pokeID );

  if( BTL_MAIN_IsOpponentClientID(SysWork.mainModule, SysWork.clientID, targetClientID) )
  {
    if( BTL_MAIN_GetCompetitor(SysWork.mainModule) == BTL_COMPETITOR_WILD )
    {
      return SETTYPE_WILD;
    }
    else
    {
      return SETTYPE_ENEMY;
    }
  }
  return SETTYPE_MINE;
}
static inline u16 get_setStrID( u8 pokeID, u16 defaultStrID )
{
  return defaultStrID + get_strFormat( pokeID );
}

static inline u16 get_setPtnStrID( u8 pokeID, u16 originStrID, u8 ptnNum )
{
  return originStrID + (ptnNum * SETTYPE_MAX) + get_strFormat( pokeID );
}

//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------

//=============================================================================================
/**
 * 標準メッセージの生成
 * ※標準メッセージ：対象者なし、あるいは対象が誰であっても一定のフォーマットで生成される文字列
 *
 * @param   buf       [out] 生成文字列格納バッファ
 * @param   strID     文字列ID
 * @param   numArgs   可変個引数の数
 * @param   ...       引数（可変）
 *
 */
//=============================================================================================
void BTL_STR_MakeStringStd( STRBUF* buf, BtlStrID_STD strID, u32 numArgs, ... )
{
  GF_ASSERT(numArgs<=MSG_ARG_MAX);
  {
    va_list   list;
    u32 i;

    BTL_Printf("STD STR ID=%d, argCnt=%d\n", strID, numArgs);

    va_start( list, numArgs );
    for(i=0; i<numArgs; ++i)
    {
      SysWork.args[i] = va_arg( list, int );
      BTL_Printf("  arg(%d)=%d\n", i, SysWork.args[i]);
    }
    va_end( list );

    // ゴミ除去ねんのため
    while( i < NELEMS(SysWork.args) ){
      SysWork.args[i++] = 0;
    }
    BTL_STR_MakeStringStdWithArgArray( buf, strID, SysWork.args );
  }
}
//=============================================================================================
/**
 * 標準メッセージの生成（引数配列渡し版）
 *
 * @param   buf
 * @param   strID
 * @param   args
 *
 */
//=============================================================================================
void BTL_STR_MakeStringStdWithArgArray( STRBUF* buf, BtlStrID_STD strID, const int* args )
{
  static const struct {
    BtlStrID_STD   strID;
    void  (* const func)( STRBUF* buf, BtlStrID_STD strID, const int* args );
  }funcTbl[] = {
    { BTL_STRID_STD_Encount,          ms_encount },
    { BTL_STRID_STD_Encount_Double,   ms_encount_double },
    { BTL_STRID_STD_PutSingle,        ms_put_single },
    { BTL_STRID_STD_PutDouble,        ms_put_double },
    { BTL_STRID_STD_PutSingle_Enemy,  ms_put_single_enemy },
    { BTL_STRID_STD_MemberOut1,       ms_out_member1 },
    { BTL_STRID_STD_SelectAction,     ms_select_action_ready },
    { BTL_STRID_STD_KodawariLock,     ms_kodawari_lock },
    { BTL_STRID_STD_WazaLock,         ms_waza_lock },
    { BTL_STRID_STD_YubiWoFuru,       ms_waza_only },
    { BTL_STRID_STD_SizenNoTikara,    ms_waza_only },
    { BTL_STRID_STD_Reflector,        ms_side_eff },
    { BTL_STRID_STD_ReflectorOff,     ms_side_eff },
    { BTL_STRID_STD_HikariNoKabe,     ms_side_eff },
    { BTL_STRID_STD_HikariNoKabeOff,  ms_side_eff },
    { BTL_STRID_STD_SinpiNoMamori,    ms_side_eff },
    { BTL_STRID_STD_SinpiNoMamoriOff, ms_side_eff },
    { BTL_STRID_STD_SiroiKiri,        ms_side_eff },
    { BTL_STRID_STD_SiroiKiriOff,     ms_side_eff },
    { BTL_STRID_STD_Oikaze,           ms_side_eff },
    { BTL_STRID_STD_OikazeOff,        ms_side_eff },
    { BTL_STRID_STD_Omajinai,         ms_side_eff },
    { BTL_STRID_STD_OmajinaiOff,      ms_side_eff },
    { BTL_STRID_STD_StealthRock,      ms_side_eff },
    { BTL_STRID_STD_Makibisi,         ms_side_eff },
    { BTL_STRID_STD_Dokubisi,         ms_side_eff },
  };
  u32 i;

  for(i=0; i<NELEMS(funcTbl); ++i)
  {
    if( funcTbl[i].strID == strID )
    {
      funcTbl[i].func( buf, strID, args );
      return;
    }
  }

  ms_std_simple( buf, strID );
}

static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID )
{
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, dst );
}

// 野生エンカウントシングル
static void ms_encount( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// 野生エンカウントダブル
static void ms_encount_double( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  register_PokeNickname( args[1], BUFIDX_POKE_2ND );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// ゆけっ！シングル
static void ms_put_single( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// ゆけっ！ダブル
static void ms_put_double( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  register_PokeNickname( args[1], BUFIDX_POKE_2ND );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// あいてがくりだしたシングル
static void ms_put_single_enemy( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// ○○はどうする？
static void ms_select_action_ready( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// ○○　もどれ！  args[0]:pokeID
static void ms_out_member1( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// [arg0]のこうかで [arg1]しかだせない！
// arg0: アイテムID, arg1:ワザID
static void ms_kodawari_lock( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  WORDSET_RegisterItemName( SysWork.wset, 0, args[0] );
  WORDSET_RegisterWazaName( SysWork.wset, 1, args[1] );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// [arg0]は [arg1]しかだせない！
// arg0: ポケID, arg1:ワザID
static void ms_waza_lock( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterWazaName( SysWork.wset, 1, args[1] );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// [arg0]が出た！など
// arg0: ワザID
static void ms_waza_only( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  WORDSET_RegisterWazaName( SysWork.wset, 0, args[0] );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
// arg[0]: SideID、味方側と相手側で文字列IDを変える
static void ms_side_eff( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  if( BTL_MAIN_GetClientSide(SysWork.mainModule, SysWork.clientID) != args[0] ){
    ++strID;
  }
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, dst );
}





//----------------------------------------------------------------------

//=============================================================================================
/**
 * セットメッセージの生成
 * ※セットメッセージ：自分側，敵側（やせいの），敵側（あいての）が必ず３つセットになった文字列
 *
 * @param   buf
 * @param   strID
 * @param   args
 *
 */
//=============================================================================================
void BTL_STR_MakeStringSet( STRBUF* buf, BtlStrID_SET strID, const int* args )
{
  static const struct {
    u16   strID;
    void  (* func)( STRBUF*, u16, const int* );
  }funcTbl[] = {
    { BTL_STRID_SET_Rankup_ATK,           ms_set_rankup     },
    { BTL_STRID_SET_Rankdown_ATK,         ms_set_rankdown   },
    { BTL_STRID_SET_RankupMax_ATK,        ms_set_rank_limit },
    { BTL_STRID_SET_RankdownMin_ATK,      ms_set_rank_limit },
    { BTL_STRID_SET_Trace,                ms_set_trace      },
    { BTL_STRID_SET_YotimuExe,            ms_set_yotimu     },
    { BTL_STRID_SET_Omitoosi,             ms_set_omitoosi   },
    { BTL_STRID_SET_ChangePokeType,       ms_set_change_poke_type },
    { BTL_STRID_SET_UseItem_RecoverHP,    ms_set_item_common },
    { BTL_STRID_SET_UseItem_CureDoku,     ms_set_item_common },
    { BTL_STRID_SET_UseItem_CureMahi,     ms_set_item_common },
    { BTL_STRID_SET_UseItem_CureNemuri,   ms_set_item_common },
    { BTL_STRID_SET_UseItem_CureKoori,    ms_set_item_common },
    { BTL_STRID_SET_UseItem_CureYakedo,   ms_set_item_common },
    { BTL_STRID_SET_UseItem_CureKonran,   ms_set_item_common },
    { BTL_STRID_SET_UseItem_RecoverLittle,ms_set_item_common },
    { BTL_STRID_SET_RankRecoverItem,      ms_set_item_common },
    { BTL_STRID_SET_KoraeItem,            ms_set_item_common },
    { BTL_STRID_SET_UseItem_Rankup_ATK,   ms_set_kinomi_rankup },
    { BTL_STRID_SET_UseItem_RecoverPP,    ms_set_item_recover_pp },
    { BTL_STRID_SET_KaifukuFuji,          ms_set_waza_sp },
    { BTL_STRID_SET_ChouhatuWarn,         ms_set_waza_sp },
    { BTL_STRID_SET_FuuinWarn,            ms_set_waza_sp },
    { BTL_STRID_SET_Kanasibari,           ms_set_waza_sp },
    { BTL_STRID_SET_Monomane,             ms_set_waza_sp },
    { BTL_STRID_SET_Urami,                ms_set_waza_num },
    { BTL_STRID_SET_HorobiCountDown,      ms_set_poke_num },
    { BTL_STRID_SET_Takuwaeru,            ms_set_poke_num },
    { BTL_STRID_SET_LockOn,               ms_set_poke },
    { BTL_STRID_SET_Tedasuke,             ms_set_poke },



  };

  int i;

  for(i=0; i<NELEMS(funcTbl); i++)
  {
    if( funcTbl[i].strID == strID )
    {
      funcTbl[i].func( buf, strID, args );
      return;
    }
  }

  ms_set_std( buf, strID, args );
}
//--------------------------------------------------------------
/**
 *  標準処理（args[0] にポケモンID）
 */
//--------------------------------------------------------------
static void ms_set_std( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  strID = get_setStrID( args[0], strID );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○の××が（ぐーんと）あがった！
 *  args... [0]:pokeID,  [1]:statusType, [2]:volume
 */
//--------------------------------------------------------------
static void ms_set_rankup( STRBUF* dst, u16 strID, const int* args )
{
  u8 statusType = args[1] - WAZA_RANKEFF_ORIGIN;
  if( args[2] > 1 )
  {
    strID += (SETTYPE_MAX * WAZA_RANKEFF_NUMS);
  }
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  strID = get_setPtnStrID( args[0], strID, statusType );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○の××が（がくっと）さがった！
 *  args... [0]:pokeID,  [1]:statusType, [2]:volume
 */
//--------------------------------------------------------------
static void ms_set_rankdown( STRBUF* dst, u16 strID, const int* args )
{
  u8 statusType = args[1] - WAZA_RANKEFF_ORIGIN;
  if( args[2] > 1 )
  {
    strID += (SETTYPE_MAX * WAZA_RANKEFF_NUMS);
  }
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  strID = get_setPtnStrID( args[0], strID, statusType );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○の××は　もうあがらない（さがらない）！
 *  args... [0]:pokeID,  [1]:statusType
 */
//--------------------------------------------------------------
static void ms_set_rank_limit( STRBUF* dst, u16 strID, const int* args )
{
  u8 statusType = args[1] - WAZA_RANKEFF_ORIGIN;

  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  strID = get_setPtnStrID( args[0], strID, statusType );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は××にねらいをさだめた！　等
 *  args... [0]:pokeID,  [1]:targetPokeID
 */
//--------------------------------------------------------------
static void ms_set_poke( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  register_PokeNickname( args[1], BUFIDX_POKE_2ND );

  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );

}
//--------------------------------------------------------------
/**
 *  ○○の××をトレースした！
 *  args... [0]:pokeID,  [1]:targetPokeID, [2]:tokusei
 */
//--------------------------------------------------------------
static void ms_set_trace( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[1], BUFIDX_POKE_1ST );
  WORDSET_RegisterTokuseiName( SysWork.wset, 1, args[2] );
  strID = get_setStrID( args[1], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は××をよみとった！
 *  args... [0]:pokeID,  [1]:wazaID
 */
//--------------------------------------------------------------
static void ms_set_yotimu( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterWazaName( SysWork.wset, 1, args[1] );
  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は××を　おみとおしだ！
 *  args... [0]:pokeID,  [1]:itemID
 */
//--------------------------------------------------------------
static void ms_set_omitoosi( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterItemName( SysWork.wset, 1, args[1] );
  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は××タイプにかわった！
 *  args... [0]:pokeID,  [1]:typeID
 */
//--------------------------------------------------------------
static void ms_set_change_poke_type( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterPokeTypeName( SysWork.wset, 1, args[1] );
  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は××で　たいりょくをかいふくした！  等々、アイテム効果。
 *  args... [0]:pokeID,  [1]:itemID
 */
//--------------------------------------------------------------
static void ms_set_item_common( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterItemName( SysWork.wset, 1, args[1] );
  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は××で△△が　（ぐーんと）あがった！ （木の実でランク効果）
 *  args... [0]:pokeID  [1]:statusType  [2]:volume  [3]:itemID
 */
//--------------------------------------------------------------
static void ms_set_kinomi_rankup( STRBUF* dst, u16 strID, const int* args )
{
  u8 statusType = args[1] - WAZA_RANKEFF_ORIGIN;
  if( args[2] > 1 )
  {
    strID += (SETTYPE_MAX * WAZA_RANKEFF_NUMS);
  }
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterItemName( SysWork.wset, 1, args[3] );

  strID = get_setPtnStrID( args[0], strID, statusType );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は　かいふくふうじで  ××が使えない！　など
 *  args... [0]:pokeID  [1]:wazaID
 */
//--------------------------------------------------------------
static void ms_set_waza_sp( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterWazaName( SysWork.wset, 1, args[1] );

  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○の　××××を　△けずった！　など
 *  args... [0]:pokeID  [1]:wazaID  [2]: number(1桁)
 */
//--------------------------------------------------------------
static void ms_set_waza_num( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterWazaName( SysWork.wset, 1, args[1] );
  WORDSET_RegisterNumber( SysWork.wset, 2, args[2], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○の　カウントが△になった！　など
 *  args... [0]:pokeID  [1]:number(1桁)
 */
//--------------------------------------------------------------
static void ms_set_poke_num( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterNumber( SysWork.wset, 1, args[1], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

//--------------------------------------------------------------
/**
 *  ○○は××で△△のPPが回復！など
 *  args... [0]:pokeID  [1]:itemID  [2]:wazaID
 */
//--------------------------------------------------------------
static void ms_set_item_recover_pp( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterItemName( SysWork.wset, 1, args[1] );
  WORDSET_RegisterWazaName( SysWork.wset, 2, args[2] );
  strID = get_setStrID( args[0], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

//=============================================================================================
/**
 * ワザメッセージの生成
 * ※ワザメッセージ：○○の××こうげき！とか。セットメッセージと同様、必ず３つセット。
 *
 * @param   buf
 * @param   strID
 * @param   args
 *
 */
//=============================================================================================
void BTL_STR_MakeStringWaza( STRBUF* dst, u8 pokeID, u16 waza )
{
  u16 strID = get_setStrID( pokeID, waza * SETTYPE_MAX );;

  register_PokeNickname( pokeID, BUFIDX_POKE_1ST );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_ATK], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}






//=============================================================================================
/**
 *
 *
 * @param   dst
 * @param   strID
 *
 */
//=============================================================================================
void BTL_STR_GetUIString( STRBUF* dst, u16 strID )
{
  GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], strID, dst );
}

//--------------------------------------------------------------------

void BTL_STR_MakeWazaUIString( STRBUF* dst, u16 wazaID, u8 wazaPP, u8 wazaPPMax )
{
  WORDSET_RegisterWazaName( SysWork.wset, 0, wazaID );
  WORDSET_RegisterNumber( SysWork.wset, 1, wazaPP, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( SysWork.wset, 2, wazaPPMax, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], BTLMSG_UI_SEL_WAZA, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}


//--------------------------------------------------------------------
// この下はちょう仮作成。いずれ消えますたぶん…
//--------------------------------------------------------------------


void BTL_STR_MakeStatusWinStr( STRBUF* dst, const BTL_POKEPARAM* bpp, u16 hp )
{
  u16 hpMax = BPP_GetValue( bpp, BPP_MAX_HP );

  WORDSET_RegisterPokeNickName( SysWork.wset, 0, BPP_GetSrcData(bpp) );
  WORDSET_RegisterNumber( SysWork.wset, 1, hp, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( SysWork.wset, 2, hpMax, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], BTLSTR_UI_StatWinForm, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

void BTL_STR_MakeWarnStr( STRBUF* dst, const BTL_POKEPARAM* bpp, u16 strID )
{
  WORDSET_RegisterPokeNickName( SysWork.wset, 0, BPP_GetSrcData(bpp) );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}


