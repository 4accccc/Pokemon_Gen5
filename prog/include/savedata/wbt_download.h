//============================================================================================
/**
 * @file  wbt_download.h
 * @brief
 * @author  tamada GAME FREAK inc.
 * @date  2011.02.15
 *
 * @todo  ダウンロードデータのサイズ、項目確認（肩書の長さなど）
 */
//============================================================================================
#pragma once

#include "field/wbt.h"

#include "battle/battle.h"
#include "poke_tool/poketype_def.h" //POKETYPE_MAX
#include "waza_tool/wazano_def.h" //WAZANO_MAX
#include "item/itemsym.h"   //ITEM_DATA_MAX


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBTDL_POKENUMBER_MAX = MATH_ROUNDUP( MONSNO_END, 8 ) / 8,
  //WBTDL_POKEITEM_MAX = MATH_ROUNDUP( ITEM_DATA_MAX, 8 ) / 8,
  WBTDL_POKEITEM_MAX = (608/8),
  WBTDL_POKEWAZA_MAX = MATH_ROUNDUP( WAZANO_MAX, 8 ) / 8,

  DUMMY_SHOOTER_ITEM_BIT_TBL_MAX = 7,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u8 num_lo;    ///<参加数下限(1-6)
  u8 num_hi;    ///<参加数上限(1-6)

  u8 level;         ///<参加レベル(1-100、0なら無制限）
  u8 level_range;   ///<REGULATION_LEVEL_RANGE_TYPE レベル範囲
  u16 level_total;  ///<レベル合計制限（0ならば制限無し）

  u8 both_poke;     ///<同じポケモンを許可するか
  u8 both_item;     ///<同じアイテムを許可するか

  ///ポケモン制限フラグ（ずかんIDと1対1)
  u8 veto_poke_bit[ WBTDL_POKENUMBER_MAX ];

  ///所持どうぐ制限（どうぐIDと1対1）
  u8 veto_item_bit[ WBTDL_POKEITEM_MAX ];

  u8 pad;
  u8 btl_rule;      ///<BtlRule

  u8 bgm_flag;      ///<O=通常BGM 1=WCS用BGM
  u8 need_poketype; ///<必須ポケモンタイプ
  u16 veto_waza_no;   ///<禁止わざ指定
}WBTDL_RULE;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 monsno;
  STRCODE nickname[MONS_NAME_SIZE+EOM_SIZE];
  u16 item_no;
  u8 rare_flag  :1;
  u8 pad1        :7;
  u8 tokusei;
	u8	hp_exp;								//HP努力値
	u8	pow_exp;							//攻撃力努力値
	u8	def_exp;							//防御力努力値
	u8	agi_exp;							//素早さ努力値
	u8	spepow_exp;							//特攻努力値
	u8	spedef_exp;							//特防努力値
  u16 waza_id[PTL_WAZA_MAX];
  union {
    struct {
    u8  pp_count1:2;
    u8  pp_count2:2;
    u8  pp_count3:2;
    u8  pp_count4:2;
    };
    u8 pp_count;
  };
  u8 seikaku;

  union {
    struct {
    u32	hp_rnd			:5;					//		HP乱数
    u32	pow_rnd			:5;					//		攻撃力乱数
    u32	def_rnd			:5;					//		防御力乱数
    u32	agi_rnd			:5;					//		素早さ乱数
    u32	spepow_rnd		:5;					//		特攻乱数
    u32	spedef_rnd		:5;					//		特防乱数
    u32 sex         :2;
    };
    u32 power_rnd;
  };

  u8 form_no;
  u8 ball_id;

  u8 level;

  u8  friend_rate;        ///<なつき度

}WBTDL_POKEMON;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBTDL_MATCH_TRAINER_RND = 0,    ///<ランダム（ROMデータから）
  WBTDL_MATCH_TRAINER_RND_DMY,    ///<ランダム（ROMデータから、戦えない）
  WBTDL_MATCH_TRAINER_DATA,       ///<DLデータ指定
  WBTDL_MATCH_TRAINER_ROM,        ///<ROMデータ指定
}WBTDL_MATCH_TRAINER;

typedef struct {
  u8 type;  ///<WBTDL_MATCH_TRAINER
  u8 pri;
  u16 id;
}WBTDL_MATCH;

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBTDL_MSG_LEN = (36+1)*2,					// (36*2+1) -> (36+1)*2

  WBTDL_TOURNAMENT_NAME_LEN = 36 + 1,		// (20+1) -> (36+1)

  WBTDL_TRAINER_TITLE_LEN = BUFLEN_TRAINER_TYPE_NAME,

  WBTDL_TOURNAMENT_REG_EXPLAIN_LEN = (36+1) * 3,
  WBTDL_TOURNAMENT_EXPLAIN_LEN = (36+1) * 2 + 1,	// (36+1)*3 -> (36+1)*2+1

  WBTDL_CHAR_MAX = 4,

  WBTDL_MATCH_MAX = WBT_TRAINER_NUM - 1,
};

//------------------------------------------------------------------
/**
 * @brief WBT DownLoad キャラクターデータ
 */
//------------------------------------------------------------------
typedef struct {
  WBTDL_POKEMON pokemon[ TEMOTI_POKEMAX ];

  STRCODE name[ BUFLEN_PERSON_NAME ];		///< 14 -> 14+2
  STRCODE title[ WBTDL_TRAINER_TITLE_LEN ];
  STRCODE msg_before[ WBTDL_MSG_LEN ];   ///<対戦前MSG
  STRCODE msg_after_win[ WBTDL_MSG_LEN ];      ///<相手の負けMSG
  STRCODE msg_after_lose[ WBTDL_MSG_LEN ];     ///<相手の勝ちMSG
  STRCODE msg_announce[ WBTDL_MSG_LEN ]; ///<決勝戦　前口上
  u8 trtype_id;            ///<トレーナータイプ指定ID（1～、「WBT配信データフォーマット案::トレーナタイプ指定」参照）
  u8  msg_before_id;      ///<対戦前MSGのID
  u8  msg_after_win_id;   ///<対戦後勝ちメッセージのID
  u8  msg_after_lose_id;  ///<対戦後負けメッセージのID
  u8  msg_win_id;         ///<相手の負けMSGのID
  u8  msg_lose_id;        ///<相手の勝ちMSGのID
  u8  msg_announce_id;    ///<決勝戦前口上のメッセージID
  u8 lang_id;             ///<国コード
  u16 ai_bit;             ///<AI指定
  u16 pad1;               ///<パディング
}WBTDL_CHARACTER;

//------------------------------------------------------------------
/**
 * @brief WBT DownLoad トーナメントデータ
 */
//------------------------------------------------------------------
typedef struct {
  ///受信可能なカセットバージョン
  u32 cassette_version;

  ///配信開始年月日
  u8 year, month, day;

  ///言語ＩＤ 日=1、英=2、仏=3、伊=4、独=5、西=7、韓=8
  u8 lang_id;

  ///トーナメントＩＤ
  u16 tournament_id;

  ///トーナメント名
  STRCODE tournament_name[ WBTDL_TOURNAMENT_NAME_LEN ];

  ///トーナメントの内容説明
  STRCODE contents_explain[ WBTDL_TOURNAMENT_EXPLAIN_LEN ];

  ///トーナメントにおいてのレギュレーション説明
  STRCODE regulation_explain[ WBTDL_TOURNAMENT_REG_EXPLAIN_LEN ];

  ///レギュレーション指定
  WBTDL_RULE wbtdl_rule;

  ///ダウンロードトレーナーデータ
  WBTDL_CHARACTER character[ WBTDL_CHAR_MAX ];

  ///組み合わせデータ
  WBTDL_MATCH match[ WBTDL_MATCH_MAX ];

  u32 crc;
}WBTDL_DATA;


//============================================================================================
//
//  WiFiダウンロードデータ定義
//
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
enum {
  WBTDL_TOTAL_NUM = 12,
};

//------------------------------------------------------------------
/**
 * @brief ダウンロードデータのかたまり
 */
//------------------------------------------------------------------
typedef struct {
  WBTDL_DATA data[ WBTDL_TOTAL_NUM ];
  u32 crc;
}WBTDL_TOTAL_DATA;


//============================================================================================
//
//  ダウンロードデータ用レギュレーションアクセス関数
//
//  内部仕様が変更になる可能性があるため、直接アクセスせずに
//  この関数を使用すること！
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 禁止ポケモン指定ビット列の取得
 */
//------------------------------------------------------------------
static inline const u8 * WBTDL_DATA_GetVetoPokemonBit( const WBTDL_DATA * wbtdl_data )
{
  return wbtdl_data->wbtdl_rule.veto_poke_bit;
}

//------------------------------------------------------------------
/**
 * @brief  禁止アイテム指定ビット列の取得
 */
//------------------------------------------------------------------
static inline const u8 * WBTDL_DATA_GetVetoItemBit( const WBTDL_DATA * wbtdl_data )
{
  return wbtdl_data->wbtdl_rule.veto_item_bit;
}

//------------------------------------------------------------------
/**
 * @brief 禁止わざかどうかの判定
 */
//------------------------------------------------------------------
static inline BOOL WBTDL_DATA_IsVetoWazaNo( const WBTDL_DATA * wbtdl_data, u16 waza_no )
{
  return ( waza_no != wbtdl_data->wbtdl_rule.veto_waza_no );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BOOL WBTDL_DATA_CheckVersion( const WBTDL_DATA * wbtdl_data )
{
#if 1  // 仮対応
  BOOL bflg=FALSE;
  //
  if((GET_VERSION() == VERSION_W2)||(GET_VERSION() == VERSION_B2)){
    bflg = TRUE;
  }
  return( bflg );
#else
  return wbtdl_data->cassette_version == PM_VERSION;
#endif
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BOOL WBTDL_DATA_CheckLang( const WBTDL_DATA * wbtdl_data )
{
  return wbtdl_data->lang_id == PM_LANG;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BOOL WBTDL_TOTAL_DATA_CheckCRC( const WBTDL_TOTAL_DATA *cp_data )
{
  return cp_data->crc == GFL_STD_CrcCalc( cp_data, sizeof(WBTDL_TOTAL_DATA)-sizeof(cp_data->crc) );
}
static inline BOOL WBTDL_DATA_CheckCRC( const WBTDL_DATA * cp_data )
{
  return cp_data->crc == GFL_STD_CrcCalc( cp_data, sizeof(WBTDL_DATA)-sizeof(cp_data->crc) );
}
static inline BOOL WBTDL_DATA_IsExist( const WBTDL_DATA * cp_data )
{
  return cp_data->tournament_id != 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BOOL WBTDL_TOTAL_DATA_CheckVersion( const WBTDL_TOTAL_DATA *cp_data )
{
  BOOL check  = TRUE;
  int i;
  for( i = 0; i < WBTDL_TOTAL_NUM; i++ )
  {
    const WBTDL_DATA *cp_wbtdl = &cp_data->data[i];
    // idが0の場合はデータなしと判定
    if(cp_wbtdl->tournament_id == 0){
      continue;
    }
    check &= WBTDL_DATA_CheckVersion( cp_wbtdl );
  }

  return check;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BOOL WBTDL_TOTAL_DATA_CheckLang( const WBTDL_TOTAL_DATA *cp_data )
{
  BOOL check  = TRUE;
  int i;
  for( i = 0; i < WBTDL_TOTAL_NUM; i++ )
  {
    const WBTDL_DATA *cp_wbtdl = &cp_data->data[i];
    // idが0の場合はデータなしと判定
    if(cp_wbtdl->tournament_id == 0){
      continue;
    }
    check &= WBTDL_DATA_CheckLang( cp_wbtdl );
  }

  return check;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WBTDL_DATAの正常チェック
 *
 *	@param	const WBTDL_DATA * cp_data データ
 *
 *	@return TRUE正常  FALSE中身がないか言語やバージョンが違うかCRCがあってない
 */
//-----------------------------------------------------------------------------
static inline BOOL WBTDL_DATA_CheckIllegal( const WBTDL_DATA * cp_data )
{
  if( WBTDL_DATA_IsExist( cp_data )
      && WBTDL_DATA_CheckVersion( cp_data )
      && WBTDL_DATA_CheckLang( cp_data )
      && WBTDL_DATA_CheckCRC( cp_data ) )
  {

    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WBTDL_TOTAL_DATAの中身で正常なものの数を返す
 *	        （０ならば正常なものがない、１以上は正常なものの数）
 *
 *	@param	const WBTDL_TOTAL_DATA *cp_data データ
 *
 *	@return ０正常なデータがなかった  １～１２正常なデータの件数
 */
//-----------------------------------------------------------------------------
static inline int WBTDL_TOTAL_DATA_CheckIllegal( const WBTDL_TOTAL_DATA *cp_data )
{
  //全体のCRCチェック
  if( WBTDL_TOTAL_DATA_CheckCRC( cp_data ) )
  {
    //１件以上落とせるデータがあるか
    int i;
    int cnt = 0;
    for( i = 0; i < WBTDL_TOTAL_NUM; i++ )
    {
      if( WBTDL_DATA_CheckIllegal( &cp_data->data[i] ) )
      {
        cnt++;
      }
    }
    return cnt;
  }

  return 0;
}

//------------------------------------------------------------------
// バトルルールの取得
//------------------------------------------------------------------
static inline BOOL WBTDL_DATA_GetBtlRule( const WBTDL_DATA * wbtdl_data )
{
  return wbtdl_data->wbtdl_rule.btl_rule;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static inline const WBTDL_MATCH * WBTDL_DATA_GetMatchData( const WBTDL_DATA * wbtdl_data )
{
  return wbtdl_data->match;
}

//------------------------------------------------------------------
// トーナメント名の取得
//------------------------------------------------------------------
static inline const STRCODE * WBTDL_DATA_GetTournamentName( const WBTDL_DATA * wbtdl_data )
{
  return wbtdl_data->tournament_name;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static inline const WBTDL_CHARACTER * WBTDL_DATA_GetCharacter(
    const WBTDL_DATA * wbtdl_data, u8 dl_tr_id )
{
  GF_ASSERT( dl_tr_id < WBTDL_CHAR_MAX );
  return &( wbtdl_data->character[ dl_tr_id ] );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline const WBTDL_POKEMON * WBTDL_DATA_GetPokemon(
    const WBTDL_DATA * wbtdl_data, u8 dl_tr_id, u8 mons_id )
{
  const WBTDL_CHARACTER * trainer = WBTDL_DATA_GetCharacter( wbtdl_data, dl_tr_id );
  GF_ASSERT( mons_id < TEMOTI_POKEMAX );
  return &( trainer->pokemon[ mons_id ] );
}

//------------------------------------------------------------------
// トーナメントの内容説明の取得
//------------------------------------------------------------------
static inline const STRCODE * WBTDL_DATA_GetContentsExplain( const WBTDL_DATA * wbtdl_data )
{
  return( &wbtdl_data->contents_explain[0] );
}

//------------------------------------------------------------------
// トーナメントのレギュレーション説明の取得
//------------------------------------------------------------------
static inline const STRCODE * WBTDL_DATA_GetRegulationExplain( const WBTDL_DATA * wbtdl_data )
{
  return( &wbtdl_data->regulation_explain[0] );
}

//------------------------------------------------------------------
// トーナメントIDの取得
//------------------------------------------------------------------
static inline const u16 WBTDL_DATA_GetTournamentId( const WBTDL_DATA * wbtdl_data )
{
  return( wbtdl_data->tournament_id );
}

