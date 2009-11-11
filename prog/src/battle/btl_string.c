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
  SETTYPE_WILD,     ///< 野生のポケ
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
static void register_TrainerType( WORDSET* wset, u8 bufIdx, u8 clientID );
static void register_TrainerName( WORDSET* wset, u8 bufIdx, u8 clientID );
static inline SetStrFormat get_strFormat( u8 pokeID );
static inline u16 get_setStrID( u8 pokeID, u16 defaultStrID );
static inline u16 get_setStrID_Poke2( u8 pokeID1, u8 pokeID2, u16 defaultStrID );
static inline u16 get_setPtnStrID( u8 pokeID, u16 originStrID, u8 ptnNum );
static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_encount( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_encount_double( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_encount_tr1( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_tr_item( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_single( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_double( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_put_single_npc( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_select_action_ready( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_out_member1( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_kodawari_lock( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_waza_lock( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_waza_only( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void ms_side_eff( STRBUF* dst, BtlStrID_STD strID, const int* args );
static void registerWords( const STRBUF* buf, const int* args, WORDSET* wset );
static void ms_set_std( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankup( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankdown( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rank_limit( STRBUF* dst, u16 strID, const int* args );
static void ms_set_useitem( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke2poke( STRBUF* dst, u16 strID, const int* args );
static void ms_set_trace( STRBUF* dst, u16 strID, const int* args );
static void ms_set_yotimu( STRBUF* dst, u16 strID, const int* args );
static void ms_set_omitoosi( STRBUF* dst, u16 strID, const int* args );
static void ms_set_change_poke_type( STRBUF* dst, u16 strID, const int* args );
static void ms_set_item_common( STRBUF* dst, u16 strID, const int* args );
static void ms_set_kinomi_rankup( STRBUF* dst, u16 strID, const int* args );
static void ms_set_waza_sp( STRBUF* dst, u16 strID, const int* args );
static void ms_set_waza_num( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke_num( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke_trainer( STRBUF* dst, u16 strID, const int* args );
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
static inline void register_PokeName( u8 pokeID, u8 bufID )
{
  const BTL_POKEPARAM* bpp;
  const POKEMON_PARAM* pp;

  bpp = BTL_POKECON_GetPokeParamConst( SysWork.pokeCon, pokeID );
  pp = BPP_GetSrcData( bpp );

  WORDSET_RegisterPokeMonsName( SysWork.wset, bufID, pp );
}

static void register_TrainerType( WORDSET* wset, u8 bufIdx, u8 clientID )
{
  u32 trainerID = BTL_MAIN_GetClientTrainerID( SysWork.mainModule, clientID );
  GF_ASSERT( trainerID != TRID_NULL );
  WORDSET_RegisterTrTypeName( wset, bufIdx, trainerID );
}
static void register_TrainerName( WORDSET* wset, u8 bufIdx, u8 clientID )
{
  u32 trainerID = BTL_MAIN_GetClientTrainerID( SysWork.mainModule, clientID );
  if( trainerID != TRID_NULL ){
    WORDSET_RegisterTrainerName( wset, bufIdx, trainerID );
  }else{
    const MYSTATUS* status = BTL_MAIN_GetClientPlayerData( SysWork.mainModule, clientID );
    BTL_Printf("Register Trainer Name...myst=%p\n", status);
    WORDSET_RegisterPlayerName( wset, bufIdx, status );
  }

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
  u8 targetClientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );

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
static inline u16 get_setStrID_Poke2( u8 pokeID1, u8 pokeID2, u16 defaultStrID )
{
  SetStrFormat  fmt = get_strFormat( pokeID1 );
  switch( fmt ){
  case SETTYPE_MINE:
    return defaultStrID + get_strFormat( pokeID2 );
  case SETTYPE_WILD:
    defaultStrID += SETTYPE_MAX;
    break;
  case SETTYPE_ENEMY:
    defaultStrID += (SETTYPE_MAX + 2);
    break;
  }
  if( get_strFormat(pokeID2) == fmt ){
    defaultStrID++;
  }
  return defaultStrID;
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
  BTL_Printf(" strID=%d\n", strID);
  ms_std_simple( buf, strID, args );
}

static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  registerWords( SysWork.tmpBuf, args, SysWork.wset );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
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
// 野生エンカウントダブル
static void ms_encount_tr1( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  WORDSET_RegisterTrTypeName( SysWork.wset, 0, args[0] );
  WORDSET_RegisterTrainerName( SysWork.wset, 1, args[0] );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
static void ms_put_tr_item( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  // @@@ 後々はNPC，通信対戦とも共通の構成で名前を引っ張れるようにする。
  // @@@ 現状はアイテム名だけを参照
  WORDSET_RegisterItemName( SysWork.wset, 1, args[1] );
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
// NPCトレーナーがくりだしたシングル  arg0:TrainerID,  arg1:TrainerID,  arg2:pokeID
static void ms_put_single_npc( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  WORDSET_RegisterTrTypeName( SysWork.wset, 0, args[0] );
  WORDSET_RegisterTrainerName( SysWork.wset, 1, args[1] );
  register_PokeNickname( args[2], 2 );
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
    { BTL_STRID_SET_Rankup_ATK,           ms_set_rankup           },
    { BTL_STRID_SET_Rankdown_ATK,         ms_set_rankdown         },
    { BTL_STRID_SET_RankupMax_ATK,        ms_set_rank_limit       },
    { BTL_STRID_SET_RankdownMin_ATK,      ms_set_rank_limit       },
    { BTL_STRID_STD_UseItem_Self,         ms_set_useitem          },
    #if 0
    { BTL_STRID_SET_Trace,                ms_set_trace            },
    { BTL_STRID_SET_YotimuExe,            ms_set_yotimu           },
    { BTL_STRID_SET_Omitoosi,             ms_set_omitoosi         },
    { BTL_STRID_SET_ChangePokeType,       ms_set_change_poke_type },
    { BTL_STRID_SET_UseItem_RecoverHP,    ms_set_item_common      },
    { BTL_STRID_SET_UseItem_CureDoku,     ms_set_item_common      },
    { BTL_STRID_SET_UseItem_CureMahi,     ms_set_item_common      },
    { BTL_STRID_SET_UseItem_CureNemuri,   ms_set_item_common      },
    { BTL_STRID_SET_UseItem_CureKoori,    ms_set_item_common      },
    { BTL_STRID_SET_UseItem_CureYakedo,   ms_set_item_common      },
    { BTL_STRID_SET_UseItem_CureKonran,   ms_set_item_common      },
    { BTL_STRID_SET_UseItem_RecoverLittle,ms_set_item_common      },
    { BTL_STRID_SET_RankRecoverItem,      ms_set_item_common      },
    { BTL_STRID_SET_KoraeItem,            ms_set_item_common      },
    { BTL_STRID_SET_Item_DamageShrink,    ms_set_item_common      },
    { BTL_STRID_SET_Recycle,              ms_set_item_common      },
    { BTL_STRID_SET_Tuibamu,              ms_set_item_common      },
    { BTL_STRID_SET_Nagetukeru,           ms_set_item_common      },
    { BTL_STRID_SET_UseItem_Rankup_ATK,   ms_set_kinomi_rankup    },
    { BTL_STRID_SET_UseItem_RecoverPP,    ms_set_item_recover_pp  },
    { BTL_STRID_SET_KaifukuFuji,          ms_set_waza_sp          },
    { BTL_STRID_SET_ChouhatuWarn,         ms_set_waza_sp          },
    { BTL_STRID_SET_FuuinWarn,            ms_set_waza_sp          },
    { BTL_STRID_SET_Kanasibari,           ms_set_waza_sp          },
    { BTL_STRID_SET_Monomane,             ms_set_waza_sp          },
    { BTL_STRID_SET_DelayAttack,          ms_set_waza_sp          },
    { BTL_STRID_SET_Bind,                 ms_set_waza_sp          },
    { BTL_STRID_SET_BindCure,             ms_set_waza_sp          },
    { BTL_STRID_SET_PP_Recover,           ms_set_waza_sp          },
    { BTL_STRID_SET_Urami,                ms_set_waza_num         },
    { BTL_STRID_SET_HorobiCountDown,      ms_set_poke_num         },
    { BTL_STRID_SET_Takuwaeru,            ms_set_poke_num         },
    { BTL_STRID_SET_LockOn,               ms_set_poke             },
    { BTL_STRID_SET_Tedasuke,             ms_set_poke             },
    { BTL_STRID_SET_YokodoriExe,          ms_set_poke2poke        },
    { BTL_STRID_SET_MagicCoatExe,         ms_set_waza_sp          },
    { BTL_STRID_SET_Tonbogaeri,           ms_set_poke_trainer     },
    #endif
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
static void registerWords( const STRBUF* buf, const int* args, WORDSET* wset )
{
  enum {
    TAGIDX_TRAINER_NAME = 0,
    TAGIDX_POKE_NAME = 1,
    TAGIDX_POKE_NICKNAME = 2,
    TAGIDX_POKE_TYPE = 3,
    TAGIDX_TOKUSEI_NAME = 6,
    TAGIDX_WAZA_NAME = 7,
    TAGIDX_ITEM_NAME = 9,
    TAGIDX_TRAINER_TYPE = 14,
  };
  const STRCODE* sp = GFL_STR_GetStringCodePointer( buf );
  STRCODE eomCode, tagCode;
  u8  argIdxDec = 0;
  u8  clientID = BTL_CLIENT_MAX;

  eomCode = GFL_STR_GetEOMCode();
  tagCode = PRINTSYS_GetTagStartCode();

  while( *sp != eomCode )
  {
    if( *sp == tagCode )
    {
      if( PRINTSYS_IsWordSetTagGroup(sp) )
      {
        u32 bufIdx = PRINTSYS_GetTagParam( sp, 0 );
        u32 argIdx = bufIdx;
        u8  tagGrp = PRINTSYS_GetTagGroup( sp );
        if( argIdx >= argIdxDec ){
          argIdx -= argIdxDec;
        }
        BTL_Printf("bufIdx=%d ....\n", bufIdx);
        if( tagGrp == PRINTSYS_TAGGROUP_NUM )
        {
          u8 keta = PRINTSYS_GetTagIndex( sp ) + 1;
          BTL_Printf("[TAG] SetNumber : keta=%d, value=%d\n", keta, args[argIdx] );
          WORDSET_RegisterNumber( wset, bufIdx, args[argIdx], keta, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        }
        else
        {
          u8 tagIdx = PRINTSYS_GetTagIndex( sp );
          switch( tagIdx ){
          case TAGIDX_TRAINER_TYPE:
            clientID = args[ argIdx ];
            BTL_Printf("[TAG] SetTrainerID ... clientID=%d\n", clientID);
            register_TrainerType( wset, bufIdx, clientID );
            ++argIdxDec;
            break;
          case TAGIDX_TRAINER_NAME:
            if( clientID == BTL_CLIENT_MAX ){
              clientID = args[argIdx];
              BTL_Printf("[TAG] TrainerName (clientID = NULL)\n");
            }
            BTL_Printf("[TAG] SetTrainerName ... clientID=%d\n", clientID);
            register_TrainerName( wset, bufIdx, clientID );
            clientID = BTL_CLIENT_MAX;
            break;
          case TAGIDX_POKE_NICKNAME:
            BTL_Printf("[TAG] Set PokeNickName ... pokeID=%d\n", args[argIdx]);
            register_PokeNickname( args[argIdx], bufIdx );
            break;
          case TAGIDX_POKE_NAME:
            BTL_Printf("[TAG] Set PokeName ... pokeIdx=%d\n", args[argIdx] );
            register_PokeName( args[argIdx], bufIdx );
            break;
          case TAGIDX_POKE_TYPE:
            WORDSET_RegisterPokeTypeName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_TOKUSEI_NAME:
            WORDSET_RegisterTokuseiName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_WAZA_NAME:
            WORDSET_RegisterWazaName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_ITEM_NAME:
            WORDSET_RegisterItemName( wset, bufIdx, args[argIdx] );
            break;
          default:
            GF_ASSERT_MSG(0, "unknown tagIdx = %d\n", tagIdx);
            break;
          }
        }
      }
      sp = PRINTSYS_SkipTag( sp );
    }
    else{
      ++sp;
    }
  }
}
//--------------------------------------------------------------
/**
 *  標準処理（args[0] にポケモンID）
 */
//--------------------------------------------------------------
static void ms_set_std( STRBUF* dst, u16 strID, const int* args )
{
  BTL_Printf("ms set std strID=%d, args[0]=%d, args[1]=%d\n", strID, args[0], args[1]);
  strID = get_setStrID( args[0], strID );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );

  registerWords( SysWork.tmpBuf, args, SysWork.wset );

//  register_PokeNickname( args[0], BUFIDX_POKE_1ST );


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
static void ms_set_useitem( STRBUF* dst, u16 strID, const int* args )
{
  u8 clientID = args[0];

  if( BTL_MAIN_GetPlayerClientID(SysWork.mainModule) != clientID )
  {
    if( BTL_MAIN_GetCompetitor(SysWork.mainModule) == BTL_COMPETITOR_COMM ){
      strID = BTL_STRID_STD_UseItem_Player;
    }else{
      strID = BTL_STRID_STD_UseItem_NPC;
    }
  }
  else{
    strID = BTL_STRID_STD_UseItem_Self;
  }

  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  registerWords( SysWork.tmpBuf, args, SysWork.wset );
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
 *  ○○は××にねらいをさだめた！　等（野生→野生など含む７パターン）
 *  args... [0]:pokeID,  [1]:targetPokeID
 */
//--------------------------------------------------------------
static void ms_set_poke2poke( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  register_PokeNickname( args[1], BUFIDX_POKE_2ND );

  strID = get_setStrID_Poke2( args[0], args[1], strID );
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
 *  ○○は××（トレーナー名）のもとにかえっていく！など
 *  args... [0]:pokeID  [1]:clientID
 */
//--------------------------------------------------------------
static void ms_set_poke_trainer( STRBUF* dst, u16 strID, const int* args )
{
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  // @@@ これではプレイヤー名がどちらにも出てしまう…
  WORDSET_RegisterPlayerName( SysWork.wset, 1, BTL_MAIN_GetPlayerStatus(SysWork.mainModule) );

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


