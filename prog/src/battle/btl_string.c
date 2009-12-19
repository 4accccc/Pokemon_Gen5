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
#include "btl_main.h"

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
  MSGSRC_WAZAOBOE,
  MSGSRC_YESNO,

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
static inline void register_PokeNicknameTruth( u8 pokeID, WordBufID bufID );
static inline void register_PokeName( u8 pokeID, u8 bufID );
static void register_TrainerType( WORDSET* wset, u8 bufIdx, u8 clientID );
static void register_TrainerName( WORDSET* wset, u8 bufIdx, u8 clientID );
static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID, const int* args );
static inline SetStrFormat get_strFormat( u8 pokeID );
static inline u16 get_setStrID( u8 pokeID, u16 defaultStrID );
static inline u16 get_setStrID_Poke2( u8 pokeID1, u8 pokeID2, u16 defaultStrID );
static inline u16 get_setPtnStrID( u8 pokeID, u16 originStrID, u8 ptnNum );
static void registerWords( const STRBUF* buf, const int* args, WORDSET* wset );
static void ms_set_default( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankup_item( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankup( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankdown( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rank_limit( STRBUF* dst, u16 strID, const int* args );
static void ms_set_useitem( STRBUF* dst, u16 strID, const int* args );
static void ms_set_poke2poke( STRBUF* dst, u16 strID, const int* args );
static void ms_wazaoboe_simple( STRBUF* dst, BtlStrID_WAZAOBOE strID, const int* args );
static void ms_yesno_simple( STRBUF* dst, BtlStrID_WAZAOBOE strID, const int* args );




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
    NARC_message_waza_oboe_dat,
    NARC_message_yesnomenu_dat,
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
  pp = BPP_GetViewSrcData( bpp );

  WORDSET_RegisterPokeNickName( SysWork.wset, bufID, pp );
}

static inline void register_PokeNicknameTruth( u8 pokeID, WordBufID bufID )
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
  pp = BPP_GetViewSrcData( bpp );

  WORDSET_RegisterPokeMonsName( SysWork.wset, bufID, pp );
}

static void register_TrainerType( WORDSET* wset, u8 bufIdx, u8 clientID )
{
  u32 trainerID = BTL_MAIN_GetClientTrainerID( SysWork.mainModule, clientID );
  WORDSET_RegisterTrTypeName( wset, bufIdx, trainerID );
}
static void register_TrainerName( WORDSET* wset, u8 bufIdx, u8 clientID )
{
  if( clientID != BTL_CLIENTID_COMM_SUPPORT )
  {
    u32 trainerID = BTL_MAIN_GetClientTrainerID( SysWork.mainModule, clientID );
    if( trainerID != TRID_NULL ){
      WORDSET_RegisterTrainerName( wset, bufIdx, trainerID );
    }else{
      const MYSTATUS* status = BTL_MAIN_GetClientPlayerData( SysWork.mainModule, clientID );
      WORDSET_RegisterPlayerName( wset, bufIdx, status );
    }
  }
  else
  {
    const MYSTATUS* status = BTL_MAIN_GetClientPlayerData( SysWork.mainModule, clientID );
    WORDSET_RegisterPlayerName( wset, bufIdx, status );
  }
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
  /**
   *  対象陣営に応じてIDを補正する文字列群
   */
  static const u16 sideConvStrID[] = {
    BTL_STRID_STD_Makibisi,
    BTL_STRID_STD_Dokubisi,
    BTL_STRID_STD_StealthRock,
    BTL_STRID_STD_WideGuard,
    BTL_STRID_STD_FastGuard,
    BTL_STRID_STD_Kinchoukan,
  };
  int i;

  BTL_Printf(" STD:strID=%d\n", strID);

  // 対象陣営による補正：対象陣営ID（BtlSide）は args[0] に入れておくこと
  for(i=0; i<NELEMS(sideConvStrID); ++i)
  {
    if( strID == sideConvStrID[i] )
    {
      if( !BTL_MAIN_IsPlayerSide(SysWork.mainModule, args[0]) ){
        ++strID;
        BTL_Printf("  ->=%d\n", strID);
      }
      break;
    }
  }


  ms_std_simple( buf, strID, args );
}

static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID, const int* args )
{
  GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
  registerWords( SysWork.tmpBuf, args, SysWork.wset );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}



//----------------------------------------------------------------------
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
/**
 *  セット文字列ID変換（自分の・野生の・相手の３パターン）
 */
static inline u16 get_setStrID( u8 pokeID, u16 defaultStrID )
{
  return defaultStrID + get_strFormat( pokeID );
}
/**
 *  セット文字列ID変換（自分→自分、自分→野生、野生→自分など、ポケ名２つを含む７パターン）
 */
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
/**
 *  セット文字列ID変換（パターン数指定）
 */
static inline u16 get_setPtnStrID( u8 pokeID, u16 originStrID, u8 ptnNum )
{
  return originStrID + (ptnNum * SETTYPE_MAX) + get_strFormat( pokeID );
}

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
    { BTL_STRID_SET_Rankdown_ATK,         ms_set_rankup           },
    { BTL_STRID_SET_RankupMax_ATK,        ms_set_rank_limit       },
    { BTL_STRID_SET_RankdownMin_ATK,      ms_set_rank_limit       },
    { BTL_STRID_SET_UseItem_Rankup_ATK,   ms_set_rankup_item      },
    { BTL_STRID_STD_UseItem_Self,         ms_set_useitem          },
    { BTL_STRID_SET_YokodoriExe,          ms_set_poke2poke        },
    { BTL_STRID_SET_Dorobou,              ms_set_poke2poke        },
    { BTL_STRID_SET_MirrorType,           ms_set_poke2poke        },
    { BTL_STRID_SET_GiftPass,             ms_set_poke2poke        },
    { BTL_STRID_SET_FreeFall,             ms_set_poke2poke        },
    { BTL_STRID_SET_RedCard,              ms_set_poke2poke        },
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

  ms_set_default( buf, strID, args );
}

//--------------------------------------------------------------
/**
 *  タグ種類（gmm）
 */
//--------------------------------------------------------------
typedef enum {
  TAGIDX_TRAINER_NAME  = 0,
  TAGIDX_POKE_NAME     = 1,
  TAGIDX_POKE_NICKNAME = 2,
  TAGIDX_POKE_TYPE     = 3,
  TAGIDX_TOKUSEI_NAME  = 6,
  TAGIDX_WAZA_NAME     = 7,
  TAGIDX_ITEM_NAME     = 9,
  TAGIDX_POKE_NICKNAME_TRUTH = 12,
  TAGIDX_TRAINER_TYPE  = 14,
  TAGIDX_POKETYPE_NAME  = 15,
}TagIndexType;

static const STRCODE* seekNextTag( const STRCODE* sp, PrintSysTagGroup* tagGroup, u16* tagIndex, u8* argIndex )
{
  if( sp != NULL )
  {
    STRCODE EOMCode, TAGCode;

    EOMCode = GFL_STR_GetEOMCode();
    TAGCode = PRINTSYS_GetTagStartCode();
    while( (*sp) != EOMCode )
    {
        if( *sp == TAGCode )
        {
          if( PRINTSYS_IsWordSetTagGroup(sp) )
          {
            *argIndex = PRINTSYS_GetTagParam( sp, 0 );
            *tagGroup = PRINTSYS_GetTagGroup( sp );
            *tagIndex = PRINTSYS_GetTagIndex( sp );
            return PRINTSYS_SkipTag( sp );
          }
          else{
            sp = PRINTSYS_SkipTag( sp );
          }
        }
        else{
          ++sp;
        }
    }
    return NULL;
  }
  return NULL;
}

//----------------------------------------------------------------------------------
/**
 * 文字列中に含まれるポケモン名タグの数を返す
 *
 * @param   buf
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 searchPokeTagCount( const STRBUF* buf )
{
  const STRCODE* sp = GFL_STR_GetStringCodePointer( buf );
  PrintSysTagGroup tagGroup;
  u16 tagIdx;
  u8  argIdx;
  u8 count = 0;

  while( sp )
  {
    sp = seekNextTag( sp, &tagGroup, &tagIdx, &argIdx );
    if( sp )
    {
      if( tagGroup == PRINTSYS_TAGGROUP_WORD )
      {
        if( (tagIdx == TAGIDX_POKE_NAME)
        ||  (tagIdx == TAGIDX_POKE_NICKNAME)
        ||  (tagIdx == TAGIDX_POKE_NICKNAME_TRUTH)
        ){
          ++count;
        }
      }
    }
  }
  return count;
}

//----------------------------------------------------------------------------------
/**
 * 文字列中のタブIDから必要な情報を判別してWORDSETに登録する
 *
 * @param   buf
 * @param   args
 * @param   wset
 */
//----------------------------------------------------------------------------------
static void registerWords( const STRBUF* buf, const int* args, WORDSET* wset )
{
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
            register_TrainerType( wset, bufIdx, clientID );
            ++argIdxDec;
            break;
          case TAGIDX_TRAINER_NAME:
            if( clientID == BTL_CLIENT_MAX ){
              clientID = args[argIdx];
            }
            register_TrainerName( wset, bufIdx, clientID );
            clientID = BTL_CLIENT_MAX;
            break;
          case TAGIDX_POKE_NICKNAME:
            register_PokeNickname( args[argIdx], bufIdx );
            break;
          case TAGIDX_POKE_NICKNAME_TRUTH:
            register_PokeNicknameTruth( args[argIdx], bufIdx );
            break;
          case TAGIDX_POKE_NAME:
            register_PokeName( args[argIdx], bufIdx );
            break;
          case TAGIDX_POKE_TYPE:
            WORDSET_RegisterPokeTypeName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_TOKUSEI_NAME:
            WORDSET_RegisterTokuseiName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_WAZA_NAME:
            BTL_Printf("[TAG] Set WazaName ... waza=%d\n", args[argIdx] );
            WORDSET_RegisterWazaName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_ITEM_NAME:
            WORDSET_RegisterItemName( wset, bufIdx, args[argIdx] );
            break;
          case TAGIDX_POKETYPE_NAME:
            WORDSET_RegisterPokeTypeName( wset, bufIdx, args[argIdx] );
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
static void ms_set_default( STRBUF* dst, u16 strID, const int* args )
{
  BTL_Printf("ms set_default strID=%d, args[0]=%d, args[1]=%d\n", strID, args[0], args[1]);
  strID = get_setStrID( args[0], strID );

  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );

  registerWords( SysWork.tmpBuf, args, SysWork.wset );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *  ○○は△△で××が（ぐーんと）あがった！
 *  args... [0]:pokeID,  [1]:itemID,  [2]:statusType,  [3]:volume
 */
//--------------------------------------------------------------
static void ms_set_rankup_item( STRBUF* dst, u16 strID, const int* args )
{
  u8 statusType = args[2] - WAZA_RANKEFF_ORIGIN;
  if( args[3] > 1 )
  {
    strID += (SETTYPE_MAX * WAZA_RANKEFF_NUMS);
  }
  strID = get_setPtnStrID( args[0], strID, statusType );
  register_PokeNickname( args[0], BUFIDX_POKE_1ST );
  WORDSET_RegisterItemName( SysWork.wset, 1, args[1] );

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
 *  ○○は××にねらいをさだめた！　等（野生→野生など含む７パターン）
 *  args... [0]:pokeID,  [1]:targetPokeID
 */
//--------------------------------------------------------------
static void ms_set_poke2poke( STRBUF* dst, u16 strID, const int* args )
{
  strID = get_setStrID_Poke2( args[0], args[1], strID );
  GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
  registerWords( SysWork.tmpBuf, args, SysWork.wset );
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

//=============================================================================================
/**
 * 技覚えメッセージの生成（引数配列渡し版）
 *
 * @param   buf
 * @param   strID
 * @param   args
 *
 */
//=============================================================================================
void BTL_STR_MakeStringWazaOboeWithArgArray( STRBUF* buf, BtlStrID_WAZAOBOE strID, const int* args )
{
  BTL_Printf(" strID=%d\n", strID);
  ms_wazaoboe_simple( buf, strID, args );
}

static void ms_wazaoboe_simple( STRBUF* dst, BtlStrID_WAZAOBOE strID, const int* args )
{
  GFL_MSG_GetString( SysWork.msg[MSGSRC_WAZAOBOE], strID, SysWork.tmpBuf );
  registerWords( SysWork.tmpBuf, args, SysWork.wset );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

//=============================================================================================
/**
 * 技覚えメッセージの生成（引数配列渡し版）
 *
 * @param   buf
 * @param   strID
 * @param   args
 *
 */
//=============================================================================================
void BTL_STR_MakeStringYesNoWithArgArray( STRBUF* buf, BtlStrID_WAZAOBOE strID, const int* args )
{
  BTL_Printf(" strID=%d\n", strID);
  ms_yesno_simple( buf, strID, args );
}

static void ms_yesno_simple( STRBUF* dst, BtlStrID_WAZAOBOE strID, const int* args )
{
  GFL_MSG_GetString( SysWork.msg[MSGSRC_YESNO], strID, SysWork.tmpBuf );
  registerWords( SysWork.tmpBuf, args, SysWork.wset );
  WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

