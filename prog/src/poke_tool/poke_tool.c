//============================================================================================
/**
 * @file  poke_tool.c
 * @bfief ポケモンパラメータツール群（WB改定版）
 * @author  HisashiSogabe
 * @date  08.11.12
 */
//============================================================================================
#include    <gflib.h>

#include    "waza_tool/wazano_def.h"
#include    "poke_tool/poke_personal.h"
#include    "poke_tool/poke_tool.h"
#include    "poke_tool/monsno_def.h"
#include    "poke_tool/tokusyu_def.h"
#include    "waza_tool/wazadata.h"
#include    "print/global_msg.h"
#include    "print/str_tool.h"
#include    "system/gfl_use.h"

#include    "poke_tool_def.h"
#include    "poke_personal_local.h"
#include    "arc_def.h"
#include    "savedata/mail.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  // 親名，ポケモン名などの処理で使い回す文字列バッファサイズ（海外版も考慮して長めに）
  STRBUFFER_LEN = 32,

  // 技覚えテーブルに関する定義
  LEVELUPWAZA_OBOE_MAX = 22,  ///<（技20個＋終端コード:1+4バイト境界補正:1）u16の配列でスタティックに確保
//  LEVELUPWAZA_OBOE_END = 0xffff,
//  LEVELUPWAZA_LEVEL_MASK = 0xfe00,
//  LEVELUPWAZA_WAZA_MASK = 0x01ff,
//  LEVELUPWAZA_LEVEL_SHIFT = 9,

  PRAND_TOKUSEI_SHIFT = 16,   ///< 個性乱数：とくせい種類を決定するBitのIndex
  PRAND_TOKUSEI_MASK = (1 << PRAND_TOKUSEI_SHIFT ),
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void  calc_renew_core( POKEMON_PARAM *pp );

static  BOOL  pp_decordAct( POKEMON_PARAM *pp );
static  BOOL  ppp_decordAct( POKEMON_PASO_PARAM *ppp );
static  void  pp_encode_act( POKEMON_PARAM *pp );
static  void  ppp_encode_act( POKEMON_PASO_PARAM *ppp );

static  u32   pp_getAct( POKEMON_PARAM *pp, int id, void *buf);
static  u32   ppp_getAct( POKEMON_PASO_PARAM *ppp, int id, void *buf);
static  void  pp_putAct( POKEMON_PARAM *pp, int paramID, u32 arg );
static  void  ppp_putAct( POKEMON_PASO_PARAM *ppp, int paramID, u32 arg );
static  void  pp_addAct( POKEMON_PARAM *pp, int id, int value);
static  void  ppp_addAct( POKEMON_PASO_PARAM *ppp, int id, int value);

static  void  round_calc( u8 *data, int value, int max );
static  u16   rand_next( u32 *code );
static  u16   calc_abi_seikaku( u8 chr, u16 para, u8 cond );
static  void  load_grow_table( int para, u32 *GrowTable );
static  u32   get_growtbl_param( int para, int level );
static  BOOL  pppAct_check_nickname( POKEMON_PASO_PARAM* ppp );
static POKEMON_PERSONAL_DATA* Personal_Load( u16 monsno, u16 formno );
static u8 Personal_GetTokuseiCount( POKEMON_PERSONAL_DATA* ppd );
static void change_monsno_sub_tokusei( POKEMON_PASO_PARAM* ppp, u16 new_monsno, u16 old_monsno );
static void change_monsno_sub_sex( POKEMON_PASO_PARAM* ppp, u16 new_monsno, u16 old_monsno );
static  PokeType  get_type_from_item( u16 item );
static void	PokeParaWazaDelPos(POKEMON_PARAM *pp,u32 pos);

//============================================================================================
/**
 *  スタティック変数
 */
//============================================================================================
static  u32 GrowTable[ GROW_TBL_SIZE ];         //成長曲線テーブル
static  STRCODE StrBuffer[STRBUFFER_LEN];       //文字列バッファ
static  STRCODE StrBufferSub[STRBUFFER_LEN];    //文字列バッファ
static  POKEMON_PERSONAL_DATA PersonalDataWork; //ポケモン１体分のパーソナルデータ構造体
static  u16  PersonalLatestMonsNo;              //直前に読み込んだパーソナルデータ引数（モンスターナンバー）
static  u16  PersonalLatestFormNo;              //直前に読み込んだパーソナルデータ引数（フォルムナンバー）


//=============================================================================================
/**
 * システム初期化（プログラム起動後に１度だけ呼び出す）
 *
 * @param   heapID    システム初期化用ヒープID
 *
 */
//=============================================================================================
void POKETOOL_InitSystem( HEAPID heapID )
{
  POKE_PERSONAL_InitSystem( heapID );
  PersonalLatestMonsNo = MONSNO_MAX;
}


//============================================================================================
/**
 * @brief POKEMON_PASO_PARAM構造体のサイズを返す
 *
 * @retval  POKEMON_PASO_PARAM構造体のサイズ
 */
//============================================================================================
u32   POKETOOL_GetPPPWorkSize( void )
{
  return sizeof( POKEMON_PASO_PARAM );
}

//============================================================================================
/**
 *  POKEMON_PARAM構造体のサイズを返す
 *
 * @retval  POKEMON_PARAM構造体のサイズ
 */
//============================================================================================
u32   POKETOOL_GetWorkSize( void )
{
  return sizeof( POKEMON_PARAM );
}


//============================================================================================
/**
 * ポケモンパラメータ操作関数系
 */
//============================================================================================


//=============================================================================================
/**
 * ポケモンパラメータ実体を生成
 *
 * @param   mons_no       ポケモンナンバー
 * @param   level         レベル
 * @param   id            ID
 * @param   heapID        実体確保用のヒープID
 *
 * @retval  POKEMON_PARAM*
 */
//=============================================================================================
POKEMON_PARAM* PP_Create( u16 mons_no, u16 level, u64 id, HEAPID heapID )
{
  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );

  if( pp )
  {
    PP_Setup( pp, mons_no, level, id );
  }

  return pp;
}
//=============================================================================================
/**
 * ポケモンパソコンパラメータから、ポケモンパラメータを構築する
 *
 * @param   ppp
 * @param   heapID
 *
 * @retval  POKEMON_PARAM*
 */
//=============================================================================================
POKEMON_PARAM* PP_CreateByPPP( const POKEMON_PASO_PARAM* ppp, HEAPID heapID )
{
  POKEMON_PARAM* pp;
  BOOL fast_flag;
  u32 i=0;

  fast_flag = PPP_FastModeOn( (POKEMON_PASO_PARAM*)ppp );
  {
    u16 mons_no = PPP_Get( ppp, ID_PARA_monsno, NULL );
    u16 level = PPP_Get( ppp, ID_PARA_level, NULL );
    u32 ID = PPP_Get( ppp, ID_PARA_id_no, NULL );

    pp = PP_Create( mons_no, level, ID, heapID );
  }
  PPP_FastModeOff( (POKEMON_PASO_PARAM*)ppp, fast_flag );

  pp->ppp=*ppp;

//コンディションセット
  PP_Put(pp,ID_PARA_condition,i);
//HP初期化
  PP_Put(pp,ID_PARA_hp,i);
  PP_Put(pp,ID_PARA_hpmax,i);
  //メールデータ
#if 0
  {
    MAIL_DATA* mail_data = MailData_CreateWork(heapID);
    PP_Put(pp,ID_PARA_mail_data, mail_data);
    sys_FreeMemoryEz(mail_data);
  }
#endif
  PP_Renew(pp);

  return pp;
}


//============================================================================================
/**
 *  ポケモンパラメータ構造体領域を初期化する
 *
 * @param[in] pp  初期化するポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void  PP_Clear( POKEMON_PARAM *pp )
{
  MI_CpuClearFast( pp, sizeof( POKEMON_PARAM ) );

// パラメータを暗号化
  POKETOOL_encode_data( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, pp->ppp.checksum );
  POKETOOL_encode_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
}
//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体領域を初期化する
 *
 * @param[in] ppp 初期化するボックスポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void  PPP_Clear( POKEMON_PASO_PARAM *ppp )
{
  MI_CpuClearFast( ppp, sizeof( POKEMON_PASO_PARAM ) );

// 個性乱数セットしたところで暗号化
  POKETOOL_encode_data( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
}

//=============================================================================================
/**
 *  ポケモンパラメータ構造体の内容を構築
 *
 * @param[out] pp       構築先構造体アドレス
 * @param[in]  mons_no  ポケモンナンバー
 * @param[in]  level    レベル
 * @param[in]  id       ID
 */
//=============================================================================================
void PP_Setup( POKEMON_PARAM *pp, u16 mons_no, u16 level, u64 ID )
{
  PP_SetupEx( pp, mons_no, level, ID, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_AUTO );
}

//============================================================================================
/**
 *  ポケモンパラメータ構造体にポケモンパラメータを構築【拡張版】
 *
 * @param[out]  pp       パラメータを生成するポケモンパラメータ構造体のポインタ
 * @param[in]   mons_no  ポケモンナンバー
 * @param[in]   level    レベル
 * @param[in]   ID       ID指定（PTL_SETUP_ID_AUTO でランダム）
 * @param[in]   pow      個体値指定（PTL_SETUP_POW_AUTO でランダム／詳しくはpoke_tool.h 参照）
 * @param[in]   rnd      個性乱数指定（PTL_SETUP_RND_AUTOでランダム／詳しくはpoke_tool.h 参照）
 */
//============================================================================================
void  PP_SetupEx( POKEMON_PARAM *pp, u16 mons_no, u16 level, u64 ID, PtlSetupPow pow, u64 rnd )
{
  u32     i;
//メールとカスタムボール対応がまだです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MAIL_DATA and CB_CORE nothing
#endif
//  MAIL_DATA *mail_data;
//  CB_CORE   cb_core;

  PP_Clear( pp );

  PPP_SetupEx( ( POKEMON_PASO_PARAM * )&pp->ppp, mons_no, level, ID, pow, rnd );

//個性乱数がきまったら、PCPを再暗号化
  POKETOOL_encode_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), 0 );
  POKETOOL_encode_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );

//レベルセット
  PP_Put( pp, ID_PARA_level, level );

#if 0
//メールデータ
  mail_data = MailData_CreateWork( HEAPID_BASE_SYSTEM );
  PP_Put( pp, ID_PARA_mail_data, mail_data );
  GFL_HEAP_FreeMemory( mail_data );

//カスタムボールID
  i = 0;
  PP_Put( pp, ID_PARA_cb_id, (u8 *)&i );

//カスタムボール
  MI_CpuClearFast( &cb_core, sizeof( CB_CORE ) );
  PP_Put( pp, ID_PARA_cb_core, ( CB_CORE * )&cb_core );
#endif

  PP_Renew( pp );
}

//=============================================================================================
/**
 *  ボックスポケモンパラメータ構造体の内容を構築
 *
 * @param[out]  pp      構築先構造体アドレス
 * @param[in]   mons_no ポケモンナンバー
 * @param[in]   level   レベル
 * @param[in]   id      ID
 *
 */
//=============================================================================================
void  PPP_Setup( POKEMON_PASO_PARAM *ppp, u16 mons_no, u16 level, u64 id )
{
  PPP_SetupEx( ppp, mons_no, level, id, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_AUTO );
}

//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体にポケモンパラメータを構築【拡張版】
 *
 * @param[out]  ppp     パラメータを生成するボックスポケモンパラメータ構造体のポインタ
 * @param[in]   mons_no ポケモンナンバー
 * @param[in]   level   レベル
 * @param[in]   id      ID指定（PTL_SETUP_ID_AUTO でランダム）
 * @param[in]   pow     個体値指定（PTL_SETUP_POW_AUTO でランダム／詳しくはpoke_tool.h 参照）
 * @param[in]   rnd     個性乱数指定（PTL_SETUP_RND_AUTOでランダム／詳しくはpoke_tool.h 参照）
 */
//============================================================================================
void  PPP_SetupEx( POKEMON_PASO_PARAM *ppp, u16 mons_no, u16 level, u64 id, PtlSetupPow pow, u64 rnd )
{
  POKEMON_PERSONAL_DATA* ppd;
  u32   val;
  BOOL  flag;
  u16   sum;

  PPP_Clear( ppp );

  flag = PPP_FastModeOn( ppp );

// パーソナルデータをロードしておく
  ppd = Personal_Load( mons_no, PTL_FORM_NONE );

//IDナンバーセット
  if( id == PTL_SETUP_ID_AUTO )
  {
    id = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
  }
  //トレーナーの持ちポケモンは個性乱数を固定にしてレアは出さないようにするので、IDで調整をする
  else if( id == PTL_SETUP_ID_NOT_RARE )
  {
    id = (u32)(rnd+1);
    while( POKETOOL_CheckRare( id, rnd ) ){ id = (id+1)&0xffffffff; }
  }
  PPP_Put( ppp, ID_PARA_id_no, (int)id );

//個性乱数セット
  if( rnd == PTL_SETUP_RND_AUTO )
  {
    rnd = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
      __GFL_STD_MtRand();
  }
  else if( rnd == PTL_SETUP_RND_RARE )
  {
    rnd = id;
  }
  PPP_Put( ppp, ID_PARA_personal_rnd, (u32)rnd );


#ifdef DEBUG_ONLY_FOR_sogabe
#warning CasetteLanguage Nothing
#endif

//国コード
//  PPP_Put( ppp, ID_PARA_country_code, ( u8 * )&CasetteLanguage );

//ポケモンナンバーセット
  PPP_Put( ppp, ID_PARA_monsno, mons_no );

//ニックネームセット
  GFL_MSG_GetStringRaw( GlobalMsg_PokeName, mons_no, StrBuffer, NELEMS(StrBuffer) );
  PPP_Put( ppp, ID_PARA_nickname_raw, (u32)StrBuffer );

//経験値セット
  val = POKETOOL_GetMinExp( mons_no, PTL_FORM_NONE, level );
  PPP_Put( ppp, ID_PARA_exp, val );

//友好値セット
  val = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_friend );
  PPP_Put( ppp, ID_PARA_friend, val );

//捕獲データセット
  PPP_Put( ppp, ID_PARA_get_level, level );

#ifdef DEBUG_ONLY_FOR_sogabe
#warning CasetteVersion Nothing
#endif
//  PPP_Put( ppp, ID_PARA_get_cassette, ( u8 * )&CasetteVersion );
  //デフォルトはモンスターボールにしておく
  PPP_Put( ppp, ID_PARA_get_ball, ITEM_MONSUTAABOORU );

//パワー乱数セット
  if( pow == PTL_SETUP_POW_AUTO )
  {
    PPP_Put( ppp, ID_PARA_hp_rnd,     GFL_STD_MtRand(32) );
    PPP_Put( ppp, ID_PARA_pow_rnd,    GFL_STD_MtRand(32) );
    PPP_Put( ppp, ID_PARA_def_rnd,    GFL_STD_MtRand(32) );
    PPP_Put( ppp, ID_PARA_spepow_rnd, GFL_STD_MtRand(32) );
    PPP_Put( ppp, ID_PARA_spedef_rnd, GFL_STD_MtRand(32) );
    PPP_Put( ppp, ID_PARA_agi_rnd,    GFL_STD_MtRand(32) );
  }
  else
  {
    PPP_Put( ppp, ID_PARA_hp_rnd,     PTL_SETUP_POW_UNPACK(pow, PTL_ABILITY_HP) );
    PPP_Put( ppp, ID_PARA_pow_rnd,    PTL_SETUP_POW_UNPACK(pow, PTL_ABILITY_ATK) );
    PPP_Put( ppp, ID_PARA_def_rnd,    PTL_SETUP_POW_UNPACK(pow, PTL_ABILITY_DEF) );
    PPP_Put( ppp, ID_PARA_spepow_rnd, PTL_SETUP_POW_UNPACK(pow, PTL_ABILITY_SPATK) );
    PPP_Put( ppp, ID_PARA_spedef_rnd, PTL_SETUP_POW_UNPACK(pow, PTL_ABILITY_SPDEF) );
    PPP_Put( ppp, ID_PARA_agi_rnd,    PTL_SETUP_POW_UNPACK(pow, PTL_ABILITY_AGI) );
  }

// とくせいセット
  {
    u16 param = POKEPER_ID_speabi1;
    if( Personal_GetTokuseiCount(ppd) == 2 ){
      if( rnd & PRAND_TOKUSEI_MASK ){
        param = POKEPER_ID_speabi2;
      }
    }
    val = POKE_PERSONAL_GetParam( ppd, param );
    PPP_Put( ppp, ID_PARA_speabino, val );
  }

//性別セット
  val = PPP_GetSex( ppp );
  PPP_Put( ppp, ID_PARA_sex, val );

//性格セット
  val = GFUser_GetPublicRand( PTL_SEIKAKU_MAX );
  PPP_Put( ppp, ID_PARA_seikaku, val );

//技セット
  PPP_SetWazaDefault( ppp );

  PPP_FastModeOff( ppp, flag );
}

//=============================================================================================
/**
 * モンスターナンバーを書き換える。
 * 必要に応じて計算し直す必要のあるパラメータも合わせて変更する（進化時などの使用を想定）
 *
 * @param   pp
 * @param   monsno
 */
//=============================================================================================
void PP_ChangeMonsNo( POKEMON_PARAM* pp, u16 monsno )
{
  u8 fast_flag = PP_FastModeOn( pp );
  {
    PPP_ChangeMonsNo( &pp->ppp, monsno );
    PP_Renew( pp );
  }
  PP_FastModeOff( pp, fast_flag );
}
//=============================================================================================
/**
 * モンスターナンバーを書き換える。
 * 必要に応じて計算し直す必要のあるパラメータも合わせて変更する（進化時などの使用を想定）
 *
 * @param   ppp
 * @param   mons_no   書き換え後のモンスターナンバー
 */
//=============================================================================================
void PPP_ChangeMonsNo( POKEMON_PASO_PARAM* ppp, u16 next_monsno )
{
  u8 fast_flag = PPP_FastModeOn( ppp );
  {
    u16 old_monsno = PPP_Get( ppp, ID_PARA_monsno, NULL );
    if( old_monsno != next_monsno )
    {
      u16 form_no = PPP_Get( ppp, ID_PARA_form_no, NULL );
      BOOL chg_nickname = pppAct_check_nickname( ppp );

      PPP_Put( ppp, ID_PARA_monsno, next_monsno );
      change_monsno_sub_tokusei( ppp, next_monsno, old_monsno );
      change_monsno_sub_sex( ppp, next_monsno, old_monsno );
      //デフォルト名のままなら、ニックネームも書き換える
      if( chg_nickname == FALSE )
      {
        GFL_MSG_GetStringRaw( GlobalMsg_PokeName, next_monsno, StrBuffer, NELEMS(StrBuffer) );
        PPP_Put( ppp, ID_PARA_nickname_raw, (u32)StrBuffer );
      }
    }
  }
  PPP_FastModeOff( ppp, fast_flag );
}

//=============================================================================================
/**
 * フォルムナンバーを書き換える。
 * 必要に応じて計算し直す必要のあるパラメータも合わせて変更する
 *
 * @param   pp
 * @param   formno
 */
//=============================================================================================
BOOL PP_ChangeFormNo( POKEMON_PARAM* pp, u16 formno )
{
  BOOL  ret;
  u8 fast_flag = PP_FastModeOn( pp );
  {
    ret = PPP_ChangeFormNo( &pp->ppp, formno );
    if( ret == TRUE )
    {
      PP_Renew( pp );
    }
  }
  PP_FastModeOff( pp, fast_flag );

  return ret;
}
//=============================================================================================
/**
 * フォルムナンバーを書き換える。
 * 必要に応じて計算し直す必要のあるパラメータも合わせて変更する
 *
 * @param   ppp
 * @param   next_formno   書き換え後のフォルムナンバー
 */
//=============================================================================================
BOOL PPP_ChangeFormNo( POKEMON_PASO_PARAM* ppp, u16 next_formno )
{
  BOOL  ret = FALSE;
  u8 fast_flag = PPP_FastModeOn( ppp );
  {
    u16 old_formno = PPP_Get( ppp, ID_PARA_form_no, NULL );
    if( old_formno != next_formno )
    {
      u16 form_no = PPP_Get( ppp, ID_PARA_form_no, NULL );
      u16 mons_no = PPP_Get( ppp, ID_PARA_monsno, NULL );

      //ポケモン毎固有のチェックを後ほど

      PPP_Put( ppp, ID_PARA_form_no, next_formno );
      change_monsno_sub_tokusei( ppp, mons_no, mons_no );
      change_monsno_sub_sex( ppp, mons_no, mons_no );

      ret = TRUE;
    }
  }
  PPP_FastModeOff( ppp, fast_flag );

  return ret;
}

//--------------------------------------------------------------
/**
 * @brief   ロトムをフォルムチェンジする
 *
 * @param   pp					セットするPOKEMON_PARAM構造体へのポインタ
 * @param   new_form_no			新しいフォルム番号
 * @param   del_waza_pos		対応技を覚えさせる為、技を削除する位置
 * 								(対応技の削除が行われた場合、そこに新しい対応技を差し込む)
 *
 * @retval  TRUE:フォルムチェンジ成功
 * @retval	FALSE:ロトムではない
 */
//--------------------------------------------------------------
BOOL PP_ChangeRotomFormNo(POKEMON_PARAM *pp, int new_form_no, int del_waza_pos)
{
	int monsno;
	
	monsno = PP_Get(pp, ID_PARA_monsno, NULL);
	if(monsno != MONSNO_ROTOMU){
		return FALSE;
	}
	
	{
		int i, s, new_wazano, wazano;
		const u16 rotom_waza_tbl[] = {
			0,
			WAZANO_OOBAAHIITO,
			WAZANO_HAIDOROPONPU,
			WAZANO_HUBUKI,
			WAZANO_EASURASSYU,
			WAZANO_RIIHUSUTOOMU,
		};

		new_wazano = rotom_waza_tbl[new_form_no];
		
		//対応技を全て忘れさせる
		for(i = 0; i < 4; i++)
    {
			wazano = PP_Get(pp, ID_PARA_waza1 + i, NULL);
			for(s = 1; s < NELEMS(rotom_waza_tbl); s++)
      {
				if(wazano != 0 && wazano == rotom_waza_tbl[s])
        {
					if(new_wazano != 0)
          {
						//新しいフォルムの技と入れ替える
            PP_SetWazaPos( pp, new_wazano, i );
						new_wazano = 0;
						break;
					}
					else
          {	//技忘れ
						PokeParaWazaDelPos(pp, i);
						i--;
						break;
					}
				}
			}
		}
		
		//対応技を覚えていなかった場合はここで技入れ替え
		if(new_wazano != 0)
    {
			for(i = 0; i < 4; i++)
      {
				if(PP_Get(pp, ID_PARA_waza1 + i, NULL) == 0)
        {
          PP_SetWazaPos( pp, new_wazano, i );
					break;
				}
			}
			if(i == 4)
      {
        PP_SetWazaPos( pp, new_wazano, del_waza_pos );
			}
		}
		
		//対応技を忘れさせた結果手持ちの技が0になっていたら電機ショックを覚えさせる
		if(PP_Get(pp, ID_PARA_waza1, NULL) == 0)
    {
      PP_SetWazaPos( pp, WAZANO_DENKISYOKKU, 0 );
		}
	}

  PP_ChangeFormNo( pp, new_form_no );
	
	return TRUE;
}

//============================================================================================
/**
 * 指定位置の技をクリアして、シフトする
 *
 * @param[in]	pp		クリアする構造体のポインタ
 * @param[in]	pos		クリア位置
 */
//============================================================================================
static void	PokeParaWazaDelPos(POKEMON_PARAM *pp,u32 pos)
{
	u32	i;
	u16	waza_no;
	u8	pp_now;
	u8	pp_count;

	for(i=pos;i<3;i++){
		waza_no  = PP_Get(pp,ID_PARA_waza1+i+1,NULL);
		pp_now   = PP_Get(pp,ID_PARA_pp1+i+1,NULL);
		pp_count = PP_Get(pp,ID_PARA_pp_count1+i+1,NULL);
		
		PP_Put(pp,ID_PARA_waza1+i,waza_no);
		PP_Put(pp,ID_PARA_pp1+i,pp_now);
		PP_Put(pp,ID_PARA_pp_count1+i,pp_count);
	}
	waza_no  = 0;
	pp_now   = 0;
	pp_count = 0;
	PP_Put(pp,ID_PARA_waza4,waza_no);
	PP_Put(pp,ID_PARA_pp4,pp_now);
	PP_Put(pp,ID_PARA_pp_count4,pp_count);
}

//----------------------------------------------------------------------------------
/**
 * モンスターナンバー書き換え後の とくせい 変化処理
 * （PPP_ChangeMonsNoの下請け）
 *
 * @param   ppp
 * @param   next_monsno
 * @param   old_monsno
 */
//----------------------------------------------------------------------------------
static void change_monsno_sub_tokusei( POKEMON_PASO_PARAM* ppp, u16 next_monsno, u16 old_monsno )
{
  u32 form_no = PPP_Get( ppp, ID_PARA_form_no, NULL );
  POKEMON_PERSONAL_DATA* ppd = Personal_Load( next_monsno, form_no );
  u32 rnd = PPP_Get( ppp, ID_PARA_personal_rnd, NULL );
  u16 param = POKEPER_ID_speabi1;
  if( Personal_GetTokuseiCount(ppd) == 2 ){
    if( rnd & 0x200 ){
      param = POKEPER_ID_speabi2;
    }
  }

  {
    u32 val = POKE_PERSONAL_GetParam( ppd, param );
    PPP_Put( ppp, ID_PARA_speabino, val );
  }
}
//----------------------------------------------------------------------------------
/**
 * モンスターナンバー書き換え後の 性別 変化処理
 * （PPP_ChangeMonsNoの下請け）
 *
 * @param   ppp
 * @param   new_monsno
 * @param   old_monsno
 */
//----------------------------------------------------------------------------------
static void change_monsno_sub_sex( POKEMON_PASO_PARAM* ppp, u16 next_monsno, u16 old_monsno )
{
  u32 form_no = PPP_Get( ppp, ID_PARA_form_no, NULL );
  POKEMON_PERSONAL_DATA* ppd = Personal_Load( next_monsno, form_no );
  u32 next_param, old_param;

  next_param = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
  ppd = Personal_Load( old_monsno, form_no );
  old_param = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );

  if( PokePersonal_SexVecTypeGet( next_param ) != POKEPER_SEXTYPE_FIX
  ||  (old_param == POKEPER_SEX_UNKNOWN)
  ){
    u32 rnd = PPP_Get( ppp, ID_PARA_personal_rnd, NULL );
    u8 sex = POKETOOL_GetSex( next_monsno, form_no, rnd );
    PPP_Put( ppp, ID_PARA_sex, sex );
  }
}



//============================================================================================
/**
 *  ポケモンパラメータ構造体へのアクセスを高速化モードへ
 *
 * @param[in] pp  ポケモンパラメータ構造体へのポインタ
 *
 * @retval  FALSE:すでに高速化モードになっている　TRUE:高速化モードへ移行
 */
//============================================================================================
BOOL  PP_FastModeOn( POKEMON_PARAM *pp )
{
  BOOL  ret;

  ret = FALSE;

  if( pp->ppp.pp_fast_mode == 0 ){
    ret = TRUE;
    GF_ASSERT_MSG( pp->ppp.ppp_fast_mode == 0, "pppが高速化モードになっています\n" );
    pp->ppp.pp_fast_mode = 1;
    pp->ppp.ppp_fast_mode = 1;
    POKETOOL_decord_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
    POKETOOL_decord_data( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, pp->ppp.checksum );
  }

  return ret;
}
//============================================================================================
/**
 *  ポケモンパラメータ構造体へのアクセスを通常モードへ
 *
 * @param[in] pp  ポケモンパラメータ構造体へのポインタ
 * @param[in] flag  モード移行可能か判断するためのフラグ（FALSE:移行不可、TRUE:移行する）
 *
 * @retval  FALSE:すでに通常モードになっている　TRUE:通常モードへ移行
 */
//============================================================================================
BOOL  PP_FastModeOff( POKEMON_PARAM *pp, BOOL flag )
{
  BOOL  ret;

  ret = FALSE;

  if( ( pp->ppp.pp_fast_mode == 1 ) && ( flag == TRUE ) ){
    ret = TRUE;
    pp->ppp.pp_fast_mode = 0;
    pp->ppp.ppp_fast_mode = 0;

    POKETOOL_encode_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
    pp->ppp.checksum = POKETOOL_make_checksum( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
    POKETOOL_encode_data( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, pp->ppp.checksum );
  }

  return ret;
}


//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体へのアクセスを高速化モードへ
 *
 * @param[in] ppp ボックスポケモンパラメータ構造体へのポインタ
 *
 * @retval  FALSE:すでに高速化モードになっている　TRUE:高速化モードへ移行
 */
//============================================================================================
BOOL  PPP_FastModeOn( POKEMON_PASO_PARAM *ppp )
{
  BOOL  ret;

  ret = FALSE;

  if( ppp->ppp_fast_mode == 0 ){
    ret = TRUE;
    ppp->ppp_fast_mode = 1;
    POKETOOL_decord_data( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
  }

  return ret;
}

//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体へのアクセスを通常モードへ
 *
 * @param[in] ppp   ボックスポケモンパラメータ構造体へのポインタ
 * @param[in] flag  モード移行可能か判断するためのフラグ（FALSE:移行不可、TRUE:移行する）
 *
 * @retval  FALSE:すでに通常モードになっている　TRUE:通常モードへ移行
 */
//============================================================================================
BOOL  PPP_FastModeOff( POKEMON_PASO_PARAM *ppp, BOOL flag )
{
  BOOL  ret;

  ret = FALSE;

  if( ( ppp->ppp_fast_mode == 1 ) && ( flag == TRUE ) ){
    ret = TRUE;
    ppp->ppp_fast_mode = 0;


    ppp->checksum = POKETOOL_make_checksum( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
    POKETOOL_encode_data( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
  }

  return ret;
}
//============================================================================================
/**
 *  ポケモンパラメータ構造体から任意で値を取得（暗号／復号／チェックサムチェックあり）
 *
 * @param[in] pp  取得するポケモンパラメータ構造体のポインタ
 * @param[in] id  取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]  buf 取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return    取得したデータ
 */
//============================================================================================
u32 PP_Get( const POKEMON_PARAM *pp, int id, void *buf )
{
  u32 ret;

  pp_decordAct( (POKEMON_PARAM*)pp );

  ret = pp_getAct( (POKEMON_PARAM*)pp, id, buf );

  pp_encode_act( (POKEMON_PARAM*)pp );

  return ret;
}

//============================================================================================
/**
 *  ポケモンパラメータ構造体に任意で値を格納（暗号／復号／チェックサムチェックあり）
 *
 * @param[out]  ppp     格納したいボックスポケモンパラメータ構造体のポインタ
 * @param[in] paramID   格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in] arg     格納したいデータ値／配列等のアドレスとしても利用可（paramIDの指定による）
 */
//============================================================================================
void  PP_Put( POKEMON_PARAM *pp, int paramID, u32 arg )
{
  if( pp_decordAct( pp ) == TRUE )
  {
    pp_putAct( pp, paramID, arg );
  }

  pp_encode_act( pp );
}
//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体に任意で値を格納（暗号／復号／チェックサムチェックあり）
 *
 * @param[out]  ppp     格納したいボックスポケモンパラメータ構造体のポインタ
 * @param[in] paramID   格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in] arg     格納したいデータ値／配列等のアドレスとしても利用可（paramIDの指定による）
 */
//============================================================================================
void  PPP_Put( POKEMON_PASO_PARAM *ppp, int paramID, u32 arg )
{
  if( ppp_decordAct( ppp ) == TRUE )
  {
    ppp_putAct( ppp, paramID, arg );
  }
  ppp_encode_act( ppp );
}

//============================================================================================
/**
 *  ポケモンパラメータ構造体に任意で値を加算（暗号／復号／チェックサムチェックあり）
 *
 * @param[in] pp    加算したいポケモンパラメータ構造体のポインタ
 * @param[in] id    加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in] value 加算したいデータのポインタ
 */
//============================================================================================
void  PP_Add( POKEMON_PARAM *pp, int id, int value )
{
  if( pp_decordAct( pp ) == TRUE ){
    pp_addAct( pp, id, value );
  }

  pp_encode_act( pp );
}

//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体から任意で値を取得（暗号／復号／チェックサムチェックあり）
 *
 * @param[in] pp  取得するボックスポケモンパラメータ構造体のポインタ
 * @param[in] id  取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]  buf 取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return    取得したデータ
 */
//============================================================================================
u32 PPP_Get( const POKEMON_PASO_PARAM *ppp, int id, void *buf )
{
  u32 ret;

  ppp_decordAct( (POKEMON_PASO_PARAM*)ppp );

  ret = ppp_getAct( (POKEMON_PASO_PARAM*)ppp, id, buf );

  ppp_encode_act( (POKEMON_PASO_PARAM*)ppp );

  return ret;
}

//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体に任意で値を加算（暗号／復号／チェックサムチェックあり）
 *
 * @param[in] ppp   加算したいボックスポケモンパラメータ構造体のポインタ
 * @param[in] id    加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in] value 加算したいデータのポインタ
 */
//============================================================================================
void  PPP_Add( POKEMON_PASO_PARAM *ppp, int id, int value )
{
  if( ppp_decordAct( ppp ) == TRUE ){
    ppp_addAct( ppp, id, value );
  }

  ppp_encode_act( ppp );
}


//==============================================================================
/**
 * @brief   POKEMON_PARAMからレアかどうかを判定する
 *
 * @param[in] pp    ポケモンパラメータ構造体のポインタ
 *
 * @retval  FALSE:レアじゃない  TRUE:レア
 */
//==============================================================================
BOOL  PP_CheckRare( const POKEMON_PARAM *pp )
{
  return PPP_CheckRare( &pp->ppp );
}

//==============================================================================
/**
 * @brief   POKEMON_PASO_PARAMからレアかどうかを判定する
 *
 * @param[in] ppp   ボックスポケモンパラメータ構造体のポインタ
 *
 * @retval  FALSE:レアじゃない  TRUE:レア
 */
//==============================================================================
BOOL  PPP_CheckRare( const POKEMON_PASO_PARAM *ppp )
{
  u32 id  = PPP_Get( ppp, ID_PARA_id_no,      NULL );
  u32 rnd = PPP_Get( ppp, ID_PARA_personal_rnd, NULL );

  return POKETOOL_CheckRare( id, rnd );
}
//==============================================================================
/**
 * @brief   ＩＤと個性乱数からレアかどうかを判定する
 *
 * @param   id    ID
 * @param   rnd   個性乱数
 *
 * @retval  FALSE:レアじゃない  TRUE:レア
 */
//==============================================================================
BOOL  POKETOOL_CheckRare( u32 id, u32 rnd )
{
  return ( ( ( ( id & 0xffff0000 ) >> 16 ) ^ ( id & 0xffff ) ^ ( ( rnd & 0xffff0000 ) >> 16 ) ^ ( rnd & 0xffff ) ) < 8 );
}


//============================================================================================
/**
 *  ポケモンパラメータからポケモンの性別を取得
 *
 * @param[in] pp    ポケモンパラメータ構造体
 *
 * @retval  PTL_SEX_MALE:♂　PTL_SEX_FEMALE:♀　PTL_SEX_UNK:性別不明
 */
//============================================================================================
u8  PP_GetSex( const POKEMON_PARAM *pp )
{
  return ( PPP_GetSex( ( POKEMON_PASO_PARAM * )&pp->ppp ) );
}
//============================================================================================
/**
 *  ボックスポケモンパラメータからポケモンの性別を取得
 *
 * @param[in] pp    ポケモンパラメータ構造体
 *
 * @retval  PTL_SEX_MALE:♂　PTL_SEX_FEMALE:♀　PTL_SEX_UNK:性別不明
 */
//============================================================================================
u8  PPP_GetSex( const POKEMON_PASO_PARAM *ppp )
{
  u16 mons_no;
  int form_no;
  u32 rnd;
  int flag;

  flag = PPP_FastModeOn( (POKEMON_PASO_PARAM*)ppp );
  mons_no = PPP_Get( ppp, ID_PARA_monsno, 0 );
  form_no = PPP_Get( ppp, ID_PARA_form_no, 0 );
  rnd = PPP_Get( ppp, ID_PARA_personal_rnd, 0 );
  PPP_FastModeOff( (POKEMON_PASO_PARAM*)ppp, flag );

  return  POKETOOL_GetSex( mons_no, form_no, rnd );
}
//============================================================================================
/**
 *  ポケモンの性別を取得（内部でパーソナルデータをロードする）
 *
 * @param[in] mons_no 性別を取得するポケモンナンバー
 * @param[in] form_no 性別を取得するポケモンのフォルムナンバー
 * @param[in] rnd   性別を取得するポケモンの個性乱数
 *
 * @retval  PTL_SEX_MALE:♂　PTL_SEX_FEMALE:♀　PTL_SEX_UNK:性別不明
 */
//============================================================================================
u8 POKETOOL_GetSex( u16 mons_no, u16 form_no, u32 personal_rnd )
{
  u8 sex_param;

  POKEMON_PERSONAL_DATA* ppd = Personal_Load( mons_no, form_no );
  sex_param = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );

// 性別固定のケース
  switch( sex_param ){
  case POKEPER_SEX_MALE:
    return PTL_SEX_MALE;
  case POKEPER_SEX_FEMALE:
    return PTL_SEX_FEMALE;
  case POKEPER_SEX_UNKNOWN:
    return PTL_SEX_UNKNOWN;
  }

// 個性乱数により性別が決まるケース
  {
    u8 rnd_sex_bit = ( personal_rnd & 0xff );

    if( rnd_sex_bit >= sex_param )
    {
      return PTL_SEX_MALE;
    }
    else
    {
      return PTL_SEX_FEMALE;
    }
  }
}

//=============================================================================================
/**
 * ポケモンの状態異常を取得
 *
 * @param[in]   pp ポケモンパラメータ構造体
 *
 * @retval  PokeSick    状態異常ID（異常が無い場合 POKESICK_NULL）
 */
//=============================================================================================
PokeSick PP_GetSick( const POKEMON_PARAM* pp )
{
  return PP_Get( pp, ID_PARA_condition, NULL );
}
//=============================================================================================
/**
 * ポケモンの状態異常を設定
 *
 * @param   pp      ポケモンパラメータ構造体
 * @param   sick    状態異常ID（異常を無くす場合 POKESICK_NULL）
 *
 */
//=============================================================================================
void PP_SetSick( POKEMON_PARAM* pp, PokeSick sick )
{
  PP_Put( pp, ID_PARA_condition, sick );
}



//============================================================================================
/**
 *  モンスターナンバーとレベルからデフォルト技をセットする。
 *
 * @param[in] pp  セットするポケモンデータ構造体のポインタ
 */
//============================================================================================
void  PP_SetWazaDefault( POKEMON_PARAM *pp )
{
  PPP_SetWazaDefault( &pp->ppp );
}

//============================================================================================
/**
 *  モンスターナンバーとレベルからデフォルト技をセットする。
 *
 * @param[in] ppp セットするポケモンデータ構造体のポインタ
 */
//============================================================================================
void  PPP_SetWazaDefault( POKEMON_PASO_PARAM *ppp )
{
  static  POKEPER_WAZAOBOE_CODE wot[ POKEPER_WAZAOBOE_TABLE_ELEMS ];  //技覚えテーブル

  BOOL  flag;
  int i;
  u16 mons_no;
  u16 waza_no;
  int form_no;
  u16 ret;
  u8  level;

  flag = PPP_FastModeOn( ppp );

  mons_no = PPP_Get( ppp, ID_PARA_monsno, 0 );
  form_no = PPP_Get( ppp, ID_PARA_form_no, 0 );
  level = PPP_CalcLevel( ppp );
  POKE_PERSONAL_LoadWazaOboeTable( mons_no, form_no, wot );

// 最初に全クリアしておく
  for(i=0; i<PTL_WAZA_MAX; i++)
  {
    // @@@ ワザナンバー無効値=0 という前提の記述。いずれシンボル化する。
    PPP_Put( ppp, ID_PARA_waza1 + i,    0 );
    PPP_Put( ppp, ID_PARA_pp1 + i,      0 );
    PPP_Put( ppp, ID_PARA_pp_count1 + i,  0 );
  }

  i = 0;
  while( !POKEPER_WAZAOBOE_IsEndCode(wot[i]) )
  {
    if( POKEPER_WAZAOBOE_GetLevel(wot[i]) <= level )
    {
      waza_no = POKEPER_WAZAOBOE_GetWazaID( wot[i] );
      ret = PPP_SetWaza( ppp, waza_no );
      if( ret == PTL_WAZASET_FAIL ){
        PPP_SetWazaPush( ppp, waza_no );
      }
    }
    else{
      break;
    }
    i++;
  }

  PPP_FastModeOff( ppp, flag );
}

//============================================================================================
/**
 *  あいている場所に技を追加する。あいていない場合はその旨を返す。
 *
 *  @param[in]  pp    セットするポケモンパラメータ構造体のポインタ
 *  @param[in]  wazano  セットする技ナンバー
 *
 *  @retvl  wazano        正常終了
 *      PTL_WAZASET_SAME  すでに覚えている技と同じ技を覚えようとした
 *      PTL_WAZASET_FAIL  場所が空いていない
 */
//============================================================================================
u16 PP_SetWaza( POKEMON_PARAM *pp, u16 wazano )
{
  return PPP_SetWaza( &pp->ppp, wazano );
}

//============================================================================================
/**
 *  あいている場所に技を追加する。あいていない場合はその旨を返す。
 *
 *  @param[in]  ppp   セットするポケモンパラメータ構造体のポインタ
 *  @param[in]  wazano  セットする技ナンバー
 *
 *  @retvl  wazano      正常終了
 *      POKETOOL_WAZASET_SAME すでに覚えている技と同じ技を覚えようとした
 *      POKETOOL_WAZASET_FAIL   場所が空いていない
 */
//============================================================================================
u16 PPP_SetWaza( POKEMON_PASO_PARAM *ppp, u16 wazano )
{
  int i;
  u8  pp;
  u16 waza;
  u16 ret;
  BOOL  flag;

  ret = PTL_WAZASET_FAIL;

  flag = PPP_FastModeOn( ppp );

  for( i = 0 ; i < PTL_WAZA_MAX ; i++ ){
    if( ( waza = PPP_Get( ppp, ID_PARA_waza1 + i, NULL ) ) == 0 ){
      PPP_SetWazaPos( ppp, wazano, i );
      ret = wazano;
      break;
    }
    else{
      //同じ技を覚えちゃだめ
      if( waza == wazano ){
        ret = PTL_WAZASET_SAME;
        break;
      }
    }
  }

  PPP_FastModeOff( ppp, flag );

  return  ret;
}

//============================================================================================
/**
 *  押し出し式に技を覚える。
 *
 * @param[in] pp    セットするポケモンパラメータ構造体のポインタ
 * @param[in] wazano  覚える技
 */
//============================================================================================
void  PP_SetWazaPush( POKEMON_PARAM *pp, u16 wazano )
{
  PPP_SetWazaPush( &pp->ppp, wazano );
}

//============================================================================================
/**
 *  押し出し式に技を覚える。
 *
 * @param[in] ppp   セットするポケモンパラメータ構造体のポインタ
 * @param[in] wazano  覚える技
 */
//============================================================================================
void  PPP_SetWazaPush( POKEMON_PASO_PARAM *ppp, u16 wazano )
{
  u16 waza[PTL_WAZA_MAX];
  u8  pp[PTL_WAZA_MAX];
  u8  ppcnt[PTL_WAZA_MAX];
  BOOL  flag;
  int i;

  u16 LAST_IDX = PTL_WAZA_MAX - 1;

  flag = PPP_FastModeOn( ppp );

  for( i = 0 ; i < LAST_IDX ; i++ ){
    waza[i] = PPP_Get( ppp, ID_PARA_waza2 + i,   NULL );
    pp[i] =   PPP_Get( ppp, ID_PARA_pp2 + i,     NULL );
    ppcnt[i] =  PPP_Get( ppp, ID_PARA_pp_count2 + i, NULL );
  }

  waza[LAST_IDX] = wazano;
  pp[LAST_IDX] = WAZADATA_GetParam( wazano, WAZAPARAM_BASE_PP );
  ppcnt[LAST_IDX] = 0;

  for( i=0 ; i < PTL_WAZA_MAX ; i++ ){
    PPP_Put( ppp, ID_PARA_waza1 + i,   waza[i] );
    PPP_Put( ppp, ID_PARA_pp1 + i,     pp[i] );
    PPP_Put( ppp, ID_PARA_pp_count1 + i, ppcnt[i] );
  }

  PPP_FastModeOff( ppp, flag );
}

//=============================================================================================
/**
 * 覚えている全てのワザのPPを最大値まで回復させる
 *
 * @param   pp
 */
//=============================================================================================
void PP_RecoverWazaPPAll( POKEMON_PARAM* pp )
{
  BOOL flag = PP_FastModeOn( pp );
  {
    u32 waza, i;
    for(i=0; i<PTL_WAZA_MAX; ++i)
    {
      waza = PP_Get( pp, ID_PARA_waza1+i, NULL );
      if( waza != WAZANO_NULL )
      {
        u32 upsPP = PP_Get( pp, ID_PARA_pp_count1+i, NULL );
        u32 maxPP = WAZADATA_GetMaxPP( waza, upsPP );
        PP_Put( pp, ID_PARA_pp1+i, maxPP );
      }
    }
  }
  PP_FastModeOff( pp, flag );
}

//============================================================================================
/**
 *  場所を指定して技をセットする。
 *
 * @param[in] pp    セットするポケモンパラメータ構造体のポインタ
 * @param[in] wazano  セットする技ナンバー
 * @param[in] pos   技をセットする場所
 */
//============================================================================================
void  PP_SetWazaPos( POKEMON_PARAM *pp, u16 wazano, u8 pos )
{
  PPP_SetWazaPos( &pp->ppp, wazano, pos );
}

//============================================================================================
/**
 *  場所を指定して技をセットする。
 *
 * @param[in] ppp   セットするポケモンパラメータ構造体のポインタ
 * @param[in] wazano  セットする技ナンバー
 * @param[in] pos   技をセットする場所
 */
//============================================================================================
void  PPP_SetWazaPos( POKEMON_PASO_PARAM *ppp, u16 wazano, u8 pos )
{
  GF_ASSERT(pos<PTL_WAZA_MAX);

  {
    u8 maxPP = 0;

    PPP_Put( ppp, ID_PARA_waza1 + pos, wazano );
    PPP_Put( ppp, ID_PARA_pp_count1 + pos, 0 );

    if( wazano != WAZANO_NULL ){
      maxPP = WAZADATA_GetMaxPP( wazano, 0 );
    }

    PPP_Put( ppp, ID_PARA_pp1 + pos, maxPP );
  }
}

//============================================================================================
/**
 *  今のレベルで覚える技をセットする（レベルアップ時に呼ばれること想定）
 *
 * @param[in]     pp      セットするポケモンパラメータ構造体のポインタ
 * @param[in/out] index   技覚えテーブル参照インデックス
 * @param[in]     heapID  技覚えテーブル確保用ヒープのヒープID
 *
 * @retval  セットした技ナンバー（ PTL_WAZAOBOE_NONE:覚えなかった PTL_WAZAOBOE_FULLビットが立っている:手持ち技がいっぱい）
 */
//============================================================================================
WazaID  PP_CheckWazaOboe( POKEMON_PARAM *pp, int* index, HEAPID heapID )
{
  POKEPER_WAZAOBOE_CODE*  wot = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( heapID ),
                                                      POKEPER_WAZAOBOE_TABLE_ELEMS * sizeof( POKEPER_WAZAOBOE_CODE ) );
  WazaID  ret = PTL_WAZAOBOE_NONE;
  u16     mons_no;
  int     form_no;
  u8      level;

  mons_no = PP_Get( pp, ID_PARA_monsno, NULL );
  form_no = PP_Get( pp, ID_PARA_form_no, NULL );
  level = PP_Get( pp, ID_PARA_level, NULL );
  POKE_PERSONAL_LoadWazaOboeTable( mons_no, form_no, wot );

  while( !POKEPER_WAZAOBOE_IsEndCode( wot[ index[ 0 ] ] ) ){
    if( POKEPER_WAZAOBOE_GetLevel( wot[ index[ 0 ] ] ) == level )
    {
      ret = PP_SetWaza( pp, POKEPER_WAZAOBOE_GetWazaID( wot[ index[ 0 ] ] ) );
      if( ret == PTL_WAZASET_FAIL )
      {
        ret = POKEPER_WAZAOBOE_GetWazaID( wot[ index[ 0 ] ] ) | PTL_WAZAOBOE_FULL;
        index[ 0 ]++;
        break;
      }
      else if( ret != PTL_WAZASET_SAME )
      {
        index[ 0 ]++;
        break;
      }
    }
    index[ 0 ]++;
  }

  GFL_HEAP_FreeMemory( wot );

  return  ret;
}

//============================================================================================
/**
 *  ポケモンデータからポケモンのレベルを取得
 *
 * @param[in] pp  取得したいポケモンパラメータ構造体のポインタ
 *
 * @return  取得したレベル
 */
//============================================================================================
u32 PP_CalcLevel( const POKEMON_PARAM *pp )
{
  return PPP_CalcLevel( &pp->ppp );
}
//============================================================================================
/**
 *  ボックスポケモンデータからポケモンのレベルを取得
 *
 * @param[in] ppp 取得したいボックスポケモンパラメータ構造体のポインタ
 *
 * @return  取得したレベル
 */
//============================================================================================
u32 PPP_CalcLevel( const POKEMON_PASO_PARAM *ppp )
{
  int mons_no;
  int form_no;
  u32 exp;
  BOOL  flag;

  flag = PPP_FastModeOn( (POKEMON_PASO_PARAM*)ppp );

  mons_no = PPP_Get( ppp, ID_PARA_monsno, 0 );
  form_no = PPP_Get( ppp, ID_PARA_form_no, 0 );
  exp = PPP_Get( ppp, ID_PARA_exp, 0 );

  PPP_FastModeOff( (POKEMON_PASO_PARAM*)ppp, flag );

  return POKETOOL_CalcLevel( mons_no, form_no, exp );
}

//=============================================================================================
/**
 * ポケモンデータとプレイヤーデータの親一致チェック
 *
 * @param   pp        ポケモンデータ
 * @param   player    プレイヤーデータ
 *
 * @retval  BOOL      プレイヤーデータがポケモンの親ならTRUE
 */
//=============================================================================================
BOOL PP_IsMatchOya( const POKEMON_PARAM* pp, const MYSTATUS* player )
{
  return PPP_IsMatchOya( &pp->ppp, player );
}
//=============================================================================================
/**
 * ボックスポケモンデータとプレイヤーデータの親一致チェック
 *
 * @param   ppp       ボックスポケモンデータ
 * @param   player    プレイヤーデータ
 *
 * @retval  BOOL      プレイヤーデータがポケモンの親ならTRUE
 */
//=============================================================================================
BOOL PPP_IsMatchOya( const POKEMON_PASO_PARAM* ppp, const MYSTATUS* player )
{
  u8 fastFlag = PPP_FastModeOn( (POKEMON_PASO_PARAM*)ppp );
  BOOL result = FALSE;

  {
    u32 playerID = MyStatus_GetID( player );
    u32 oyaID = PPP_Get( ppp, ID_PARA_id_no, NULL );

    u32 playerSex = MyStatus_GetMySex( player );
    u32 oyaSex = PPP_Get( ppp, ID_PARA_oyasex, NULL );

    MyStatus_CopyNameStrCode( player, StrBuffer, NELEMS(StrBuffer) );
    PPP_Get( ppp, ID_PARA_oyaname_raw, StrBufferSub );

    if( (playerID == oyaID)
    &&  (playerSex == oyaSex)
    &&  (STRTOOL_Comp(StrBuffer, StrBufferSub) == 0)
    ){
      result = TRUE;
    }
  }

  PPP_FastModeOff( (POKEMON_PASO_PARAM*)ppp, fastFlag );

  return result;
}


//============================================================================================
/**
 *  ポケモンナンバー、経験値からポケモンのレベルを計算
 *  （内部でパーソナルデータをロードする）
 *
 * @param[in] mons_no 取得したいポケモンナンバー
 * @param[in] form_no 取得したいポケモンのフォルムナンバー
 * @param[in] exp   取得したいポケモンの経験値
 *
 * @return  取得したレベル
 */
//============================================================================================
u32 POKETOOL_CalcLevel( u16 mons_no, u16 form_no, u32 exp )
{
  u16 growType, level;

  POKEMON_PERSONAL_DATA* ppd = Personal_Load( mons_no, form_no );
  growType = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_grow );
  load_grow_table( growType, &GrowTable[0] );

  for( level = 1 ; level < 101 ; level++ ){
    if( GrowTable[ level ] > exp ) break;
  }

  return level - 1;
}

//============================================================================================
/**
 *  ポケモンパラメータ構造体から経験値データを取得
 *
 * @param[in] pp  ポケモンパラメータ構造体のポインタ
 */
//============================================================================================
u32 PP_GetMinExp( const POKEMON_PARAM *pp )
{
  return PPP_GetMinExp( &pp->ppp );
}

//============================================================================================
/**
 *  ポケモンパラメータ構造体から経験値データを取得
 *
 * @param[in] pp  ポケモンパラメータ構造体のポインタ
 */
//============================================================================================
u32 PPP_GetMinExp( const POKEMON_PASO_PARAM *ppp )
{
  return POKETOOL_GetMinExp(
              PPP_Get( ppp, ID_PARA_monsno, NULL ),
              PPP_Get( ppp, ID_PARA_form_no, NULL ),
              PPP_Get( ppp, ID_PARA_level, NULL )
  );
}

//============================================================================================
/**
 *  ポケモンナンバーとレベルから、そのレベルになるための最小経験値を取得
 *
 * @param[in] mons_no   取得するモンスターナンバー
 * @param[in] form_no   取得するモンスターのフォルムナンバー
 * @param[in] level   レベル指定
 *
 * @retval  u32   指定レベルに達するための最小経験値
 */
//============================================================================================
u32 POKETOOL_GetMinExp( u16 mons_no, u16 form_no, u16 level )
{
  return  get_growtbl_param( POKETOOL_GetPersonalParam(mons_no, form_no, POKEPER_ID_grow), level );
}



//============================================================================================
/**
 *  ポケモンの性格を取得（引数がPOKEMON_PARAM)
 *
 * @param[in] pp  取得したいポケモンパラメータ構造体のポインタ
 *
 * @return  取得した性格
 */
//============================================================================================
u8  PP_GetSeikaku( const POKEMON_PARAM *pp )
{
  return  PPP_GetSeikaku( &pp->ppp );
}
//============================================================================================
/**
 *  ポケモンの性格を取得（引数がPOKEMON_PASO_PARAM)
 *
 * @param[in] ppp 取得したいポケモンパラメータ構造体のポインタ
 *
 * @return  取得した性格
 */
//============================================================================================
u8  PPP_GetSeikaku( const POKEMON_PASO_PARAM *ppp )
{
  return PPP_Get( ppp, ID_PARA_seikaku, 0 );
}

//=============================================================================================
/**
 * 内包するPPPデータを基に、PPデータ部を更新する
 *
 * @param   pp    更新するPPデータポインタ
 */
//=============================================================================================
void  PP_Renew( POKEMON_PARAM *pp )
{
  int   level;
  BOOL  flag;

  flag = PP_FastModeOn( pp );

//レベルセット
  level = PP_CalcLevel( pp );
  PP_Put( pp, ID_PARA_level, level );

  calc_renew_core( pp );

  PP_FastModeOff( pp, flag );
}

//=============================================================================================
/**
 * POKEMON_PASO_PARAMのポインタを取得
 *
 * @param[in] pp  ポケモンパラメータ構造体のポインタ
 */
//=============================================================================================
POKEMON_PASO_PARAM  *PP_GetPPPPointer( POKEMON_PARAM *pp )
{
  return &pp->ppp;
}

//=============================================================================================
/**
 * POKEMON_PASO_PARAMのポインタを取得
 *
 * @param[in] pp  ポケモンパラメータ構造体のポインタ
 */
//=============================================================================================
const POKEMON_PASO_PARAM  *PP_GetPPPPointerConst( const POKEMON_PARAM *pp )
{
  return &pp->ppp;
}

//============================================================================================
/**
 *  指定されたパラメータになるように個性乱数を計算する
 *
 * @param[in] mons_no     モンスターナンバー
 * @param[in] form_no     フォルムーナンバー（不要なら PTR_FORM_NONE）
 * @param[in] sex         性別( PTL_SEX_MALE or PTL_SEX_FEMALE or PTL_SEX_UNKNOWN )
 *
 * @retval  計算した個性乱数
 */
//============================================================================================
u32  POKETOOL_CalcPersonalRand( u16 mons_no, u16 form_no, u8 sex )
{
  u32 rnd = 8;  // ID == 0 の時にレアにならないよう初期値を8にしておく

  // 性別 : 下位 1byte を調整
  {
    POKEMON_PERSONAL_DATA* ppd = Personal_Load( mons_no, form_no );
    u8 sex_param = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
    if( PokePersonal_SexVecTypeGet( sex_param ) != POKEPER_SEXTYPE_FIX)
    {
      if( sex == PTL_SEX_MALE ){
        rnd = sex_param;
      }else{
        if( sex_param ){
          rnd = sex_param - 1;
        }else{
          rnd = 0;  // ffffffffになってしまわないよう念のため
        }
      }
    }
  }

  return rnd;
}

//============================================================================================
/**
 *	指定されたパラメータになるように個性乱数を計算する（特性、レア指定可能版）
 *
 * @param[in] id					トレーナーID
 * @param[in] mons_no			個性乱数を計算するモンスターナンバー
 * @param[in] form_no     フォルムーナンバー（不要なら PTR_FORM_NONE）
 * @param[in] sex					性別
 * @param[in] tokusei			特性（ 0 or 1 で指定 ）進化後に特性１or特性２になるかを指定するためのものです
 * @param[in] rare_flag		レアにするかどうか( FALSE:レアではない　TRUE:レアにする ）
 *
 * @retval	計算した個性乱数
 */
//============================================================================================
u32		POKETOOL_CalcPersonalRandEx( u32 id, u16 mons_no, u16 form_no, u8 sex, u8 tokusei, BOOL rare_flag )
{	
	u32	rnd;

	//特性ナンバーではないので、2以上はアサートにする
	GF_ASSERT( tokusei < 2 );
	//性別にPTL_SEX_UNKNOWNは指定できないのでアサート
	GF_ASSERT( sex != PTL_SEX_UNKNOWN );

	if( rare_flag )
	{	
		u32	mask = ( ( ( id & 0xffff0000 ) >> 16 ) ^ ( id & 0x0000ffff ) );

		rnd = POKETOOL_CalcPersonalRand( mons_no, form_no, sex );
    if( ( rnd & 0x00000001 ) != tokusei )
    { 
			if( sex == PTL_SEX_MALE )
      { 
        rnd++;
      }
      else
      { 
        //乱数が０のときは、思ったような値にならないがあきらめてもらうしかない
        if( rnd )
        { 
          rnd--;
        }
      }
    }
		rnd |= ( mask ^ ( rnd & 0x0000ffff ) ) << 16;
	}
	else
	{	
    POKEMON_PERSONAL_DATA* ppd = Personal_Load( mons_no, form_no );
    u8 per_sex = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );

		rnd = ( ( ( id & 0xffff0000 ) >> 16 ) ^ ( id & 0x0000ffff ) ) & 0xff00;
		rnd = ( rnd ^ 0xff00 ) << 16;

    if( PokePersonal_SexVecTypeGet( per_sex ) != POKEPER_SEXTYPE_FIX )
		{	
			rnd |= per_sex;
			if( sex == PTL_SEX_MALE )
			{	
				if( ( rnd & 1 ) != tokusei )
				{	
          rnd++;
				}
			}
			else
			{	
        rnd--;
				if( ( rnd & 1 ) != tokusei )
				{	
					rnd--;
				}
			}
		}
	}
	return rnd;
}

//=============================================================================================
/**
 * ワザ「めざめるパワー」の実行時タイプ取得
 *
 * @param   pp
 *
 * @retval  PokeType
 */
//=============================================================================================
PokeType POKETOOL_GetMezaPa_Type( const POKEMON_PARAM* pp )
{
  static const u8 typeTbl[] = {
    POKETYPE_KAKUTOU, POKETYPE_HIKOU, POKETYPE_DOKU,   POKETYPE_JIMEN,
    POKETYPE_IWA,     POKETYPE_MUSHI, POKETYPE_GHOST,  POKETYPE_HAGANE,
    POKETYPE_HONOO,   POKETYPE_MIZU,  POKETYPE_KUSA,   POKETYPE_DENKI,
    POKETYPE_ESPER,   POKETYPE_KOORI, POKETYPE_DRAGON, POKETYPE_AKU,
  };

  u32 val=0;
  if( PP_Get(pp, ID_PARA_hp_rnd, NULL) & 1){ val += 1; }
  if( PP_Get(pp, ID_PARA_pow_rnd, NULL) & 1){ val += 2; }
  if( PP_Get(pp, ID_PARA_def_rnd, NULL) & 1){ val += 4; }
  if( PP_Get(pp, ID_PARA_agi_rnd, NULL) & 1){ val += 8; }
  if( PP_Get(pp, ID_PARA_spepow_rnd, NULL) & 1){ val += 16; }
  if( PP_Get(pp, ID_PARA_spedef_rnd, NULL) & 1){ val += 32; }

  val = val * 15 / 63;
  while( val > NELEMS(typeTbl) ){ // テーブルサイズを越えることは有り得ないハズだが念のため
    val -= NELEMS(typeTbl);
  }

  return typeTbl[ val ];
}
//=============================================================================================
/**
 * ワザ「めざめるパワー」の実行時威力を取得
 *
 * @param   pp
 *
 * @retval  PokeType
 */
//=============================================================================================
u32 POKETOOL_GetMezaPa_Power( const POKEMON_PARAM* pp )
{
  u32 pow=0;
  if( (PP_Get(pp, ID_PARA_hp_rnd, NULL) % 4)    > 1){ pow += 1; }
  if( (PP_Get(pp, ID_PARA_pow_rnd, NULL) % 4)   > 1){ pow += 2; }
  if( (PP_Get(pp, ID_PARA_def_rnd, NULL) % 4)   > 1){ pow += 4; }
  if( (PP_Get(pp, ID_PARA_agi_rnd, NULL) % 4)   > 1){ pow += 8; }
  if( (PP_Get(pp, ID_PARA_spepow_rnd, NULL) % 4)> 1){ pow += 16; }
  if( (PP_Get(pp, ID_PARA_spedef_rnd, NULL) % 4)> 1){ pow += 32; }

  pow = 30 + (pow * 40 / 63);
  return pow;
}

//=============================================================================================
/**
 * 技マシンで技を覚えるかチェック
 *
 * @param[in] pp          ポケモンパラメータ構造体
 * @param[in] machine_no  チェックする技マシンナンバー
 *
 * @retval  FALSE:覚えない　TRUE:覚える
 */
//=============================================================================================
BOOL  PP_CheckWazaMachine( const POKEMON_PARAM *pp, int machine_no )
{
  return PPP_CheckWazaMachine( &pp->ppp, machine_no );
}

//=============================================================================================
/**
 * 技マシンで技を覚えるかチェック
 *
 * @param[in] ppp
 * @param[in] machine_no  チェックする技マシンナンバー
 *
 * @retval  FALSE:覚えない　TRUE:覚える
 */
//=============================================================================================
BOOL  PPP_CheckWazaMachine( const POKEMON_PASO_PARAM *ppp, int machine_no )
{
  u16 mons_no = PPP_Get( ppp, ID_PARA_monsno, NULL );
  u16 form_no = PPP_Get( ppp, ID_PARA_form_no, NULL );

  return POKETOOL_CheckWazaMachine( mons_no, form_no, machine_no );
}

//=============================================================================================
/**
 * 技マシンで技を覚えるかチェック
 *
 * @param[in] mons_no     チェックするポケモンナンバー
 * @param[in] form_no     チェックするポケモンのフォルムナンバー
 * @param[in] machine_no  チェックする技マシンナンバー
 *
 * @retval  FALSE:覚えない　TRUE:覚える
 */
//=============================================================================================
BOOL  POKETOOL_CheckWazaMachine( u16 mons_no, u16 form_no, int machine_no )
{
  PokePersonalParamID perID;
  u32 machine_bit;

  if( mons_no == MONSNO_TAMAGO )
  {
    return FALSE;
  }

  machine_bit = ( 1 << ( machine_no % 32 ) );
  perID = POKEPER_ID_machine1 + ( machine_no / 32 );

  return ( ( POKETOOL_GetPersonalParam( mons_no, form_no, perID ) & machine_bit ) != 0 );
}

//=============================================================================================
/**
 * ポケモンの好みの味かチェック
 *
 * @param[in] pp     ポケモンパラメータ構造体
 * @param[in] taste  チェックする味
 *
 * @retval  PTL_TASTE_LIKE:好き　PTL_TASTE_DISLIKE:嫌い　PTL_TASTE_NORMAL:普通
 */
//=============================================================================================
PtlTasteJudge PP_CheckDesiredTaste( const POKEMON_PARAM *pp, PtlTaste taste )
{
  return PPP_CheckDesiredTaste( &pp->ppp, taste );
}

//=============================================================================================
/**
 * ポケモンの好みの味かチェック
 *
 * @param[in] ppp    ポケモンパラメータ構造体
 * @param[in] taste  チェックする味
 *
 * @retval  PTL_TASTE_LIKE:好き　PTL_TASTE_DISLIKE:嫌い　PTL_TASTE_NORMAL:普通
 */
//=============================================================================================
PtlTasteJudge PPP_CheckDesiredTaste( const POKEMON_PASO_PARAM *ppp, PtlTaste taste )
{
  u8 seikaku = PPP_Get( ppp, ID_PARA_seikaku, NULL );

  return POKETOOL_CheckDesiredTaste( seikaku, taste );
}

//=============================================================================================
/**
 * ポケモンの好みの味かチェック
 *
 * @param[in] seikaku 性格
 * @param[in] taste   チェックする味
 *
 * @retval  PTL_TASTE_LIKE:好き　PTL_TASTE_DISLIKE:嫌い　PTL_TASTE_NORMAL:普通
 */
//=============================================================================================
#include "taste.cdat"
PtlTasteJudge POKETOOL_CheckDesiredTaste( u8 seikaku, PtlTaste taste )
{
  return desired_taste_tbl[ seikaku ][ taste ];
}

//--------------------------------------------------------------------------
/**
 * PPPデータ部からPP独自データ部を再計算
 *
 * @param[io]   pp
 *
 */
//--------------------------------------------------------------------------
static void  calc_renew_core( POKEMON_PARAM *pp )
{
  int oldhpmax, hp, hpmax;
  int pow, def, agi, spepow, spedef;
  int hp_rnd, pow_rnd, def_rnd, agi_rnd, spepow_rnd, spedef_rnd;
  int hp_exp, pow_exp, def_exp, agi_exp, spepow_exp, spedef_exp;
  int monsno;
  int level;
  int form_no;
  int speabi1,speabi2,rnd;
  POKEMON_PERSONAL_DATA* ppd;

  level =     PP_Get( pp, ID_PARA_level,      0);
  oldhpmax =    PP_Get( pp, ID_PARA_hpmax,      0);
  hp =      PP_Get( pp, ID_PARA_hp,     0);
  hp_rnd =    PP_Get( pp, ID_PARA_hp_rnd,   0);
  hp_exp =    PP_Get( pp, ID_PARA_hp_exp,   0);
  pow_rnd =   PP_Get( pp, ID_PARA_pow_rnd,    0);
  pow_exp =   PP_Get( pp, ID_PARA_pow_exp,    0);
  def_rnd =   PP_Get( pp, ID_PARA_def_rnd,    0);
  def_exp =   PP_Get( pp, ID_PARA_def_exp,    0);
  agi_rnd =   PP_Get( pp, ID_PARA_agi_rnd,    0);
  agi_exp =   PP_Get( pp, ID_PARA_agi_exp,    0);
  spepow_rnd =  PP_Get( pp, ID_PARA_spepow_rnd, 0);
  spepow_exp =  PP_Get( pp, ID_PARA_spepow_exp, 0);
  spedef_rnd =  PP_Get( pp, ID_PARA_spedef_rnd, 0);
  spedef_exp =  PP_Get( pp, ID_PARA_spedef_exp, 0);
  form_no =   PP_Get( pp, ID_PARA_form_no,    0);

  monsno = PP_Get( pp, ID_PARA_monsno, 0 );

  ppd = Personal_Load( monsno, form_no );

  if( monsno == MONSNO_NUKENIN ){
    hpmax = 1;
  }
  else{
    hpmax = ( ( 2 * ppd->basic_hp + hp_rnd + hp_exp / 4 ) * level / 100 + level + 10 );
  }

  PP_Put( pp, ID_PARA_hpmax, hpmax );

  pow = ( ( 2 * ppd->basic_pow + pow_rnd + pow_exp / 4 ) * level / 100 + 5 );
  pow = calc_abi_seikaku( PP_GetSeikaku( pp ), pow, PTL_ABILITY_ATK );
  PP_Put( pp, ID_PARA_pow, pow );

  def = ( ( 2 * ppd->basic_def + def_rnd + def_exp / 4 ) * level / 100 + 5 );
  def = calc_abi_seikaku( PP_GetSeikaku( pp ), def, PTL_ABILITY_DEF );
  PP_Put( pp, ID_PARA_def, def );

  agi = ( ( 2 * ppd->basic_agi + agi_rnd + agi_exp / 4 ) * level / 100 + 5 );
  agi = calc_abi_seikaku( PP_GetSeikaku( pp ), agi, PTL_ABILITY_AGI );
  PP_Put( pp, ID_PARA_agi, agi );

  spepow = ( ( 2 * ppd->basic_spepow + spepow_rnd + spepow_exp / 4 ) * level / 100 + 5 );
  spepow = calc_abi_seikaku( PP_GetSeikaku( pp ), spepow, PTL_ABILITY_SPATK );
  PP_Put( pp, ID_PARA_spepow, spepow );

  spedef = ( ( 2 * ppd->basic_spedef + spedef_rnd + spedef_exp / 4 ) * level / 100 + 5 );
  spedef = calc_abi_seikaku( PP_GetSeikaku( pp ), spedef, PTL_ABILITY_SPDEF );
  PP_Put( pp, ID_PARA_spedef, spedef );

  if( ( hp == 0 ) && ( oldhpmax != 0 ) ){
    ;
  }
  else{
    if( monsno == MONSNO_NUKENIN ){
      hp = 1;
    }
    else if( hp == 0 ){
      hp = hpmax;
    }
    else{
      if( ( hpmax - oldhpmax ) < 0 ){
        if( hp > hpmax ){
          hp = hpmax;
        }
      }
      else{
        hp += ( hpmax - oldhpmax );
      }
    }
  }

  if( hp ){
    PP_Put( pp, ID_PARA_hp, hp );
  }
}



//--------------------------------------------------------------------------
/**
 *  ポケモンパラメータ構造体を復号してチェックサムをチェックして不正ならダメタマゴにする
 *
 * @param[in] pp    復号するポケモンパラメータ構造体のポインタ
 *
 * @return    FALSE:不正データ　TRUE:正常データ
 */
//--------------------------------------------------------------------------
static  BOOL  pp_decordAct( POKEMON_PARAM *pp )
{
  BOOL  ret = TRUE;
  if( pp->ppp.pp_fast_mode == 0 ){
    POKETOOL_decord_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
    ret = ppp_decordAct( &pp->ppp );
  }
  return ret;
}
//--------------------------------------------------------------------------
/**
 *  ポケモンパラメータ構造体を復号してチェックサムをチェックして不正ならダメタマゴにする
 *
 * @param[in] ppp   復号するボックスポケモンパラメータ構造体のポインタ
 *
 * @return    FALSE:不正データ　TRUE:正常データ
 */
//--------------------------------------------------------------------------
static  BOOL  ppp_decordAct( POKEMON_PASO_PARAM *ppp )
{
  BOOL  ret = TRUE;
  u16   sum;

  if( ppp->ppp_fast_mode == 0 ){
    POKETOOL_decord_data( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
    sum = POKETOOL_make_checksum( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
    if( sum != ppp->checksum ){
      GF_ASSERT_MSG( ( sum == ppp->checksum ), "checksum Crash!\n" );
      ppp->fusei_tamago_flag = 1;
      ret = FALSE;
    }
  }
  return ret;
}
//--------------------------------------------------------------------------
/**
 *  ポケモンパラメータ構造体を暗号化する
 *
 * @param[io] ppp   復号するボックスポケモンパラメータ構造体のポインタ
 */
//--------------------------------------------------------------------------
static  void  pp_encode_act( POKEMON_PARAM *pp )
{
  if( pp->ppp.pp_fast_mode == 0 ){
    POKETOOL_encode_data( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
    ppp_encode_act( &pp->ppp );
  }
}

//--------------------------------------------------------------------------
/**
 *  ボックスポケモンパラメータ構造体を暗号化する
 *
 * @param[io] ppp   復号するボックスポケモンパラメータ構造体のポインタ
 */
//--------------------------------------------------------------------------
static  void  ppp_encode_act( POKEMON_PASO_PARAM *ppp )
{
  if( ppp->ppp_fast_mode == 0 ){
    ppp->checksum = POKETOOL_make_checksum( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
    POKETOOL_encode_data( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
  }
}

//--------------------------------------------------------------------------
/**
 *  ポケモンパラメータ構造体から任意で値を取得
 *
 * @param[in] pp  取得するポケモンパラメータ構造体のポインタ
 * @param[in] id  取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]  buf 取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return    取得したデータ
 */
//--------------------------------------------------------------------------
static  u32 pp_getAct( POKEMON_PARAM *pp, int id, void *buf )
{
  u32 ret = 0;

//メールとカスタムボール対応がまだです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning CB_CORE nothing
#endif
  switch( id ){
  case ID_PARA_condition:
    ret = pp->pcp.condition;
    break;
  case ID_PARA_level:
    ret = pp->pcp.level;
    break;
  case ID_PARA_cb_id:
    ret = pp->pcp.cb_id;
    break;
  case ID_PARA_hp:
    ret = pp->pcp.hp;
    break;
  case ID_PARA_hpmax:
    ret = pp->pcp.hpmax;
    break;
  case ID_PARA_pow:
    ret = pp->pcp.pow;
    break;
  case ID_PARA_def:
    ret = pp->pcp.def;
    break;
  case ID_PARA_agi:
    ret = pp->pcp.agi;
    break;
  case ID_PARA_spepow:
    ret = pp->pcp.spepow;
    break;
  case ID_PARA_spedef:
    ret = pp->pcp.spedef;
    break;
  case ID_PARA_mail_data:
    MailData_Copy( ( MAIL_DATA * )pp->pcp.mail_data, ( MAIL_DATA * )buf );
    ret = TRUE;
    break;
  case ID_PARA_cb_core:
//    CB_Tool_CoreData_Copy( &pp->pcp.cb_core, ( CB_CORE * )buf );
    ret = TRUE;
    break;
  default:
    ret = ppp_getAct( ( POKEMON_PASO_PARAM * )&pp->ppp, id, buf );
    break;
  }
  return  ret;
}

//--------------------------------------------------------------------------
/**
 *  ボックスポケモンパラメータ構造体から任意で値を取得
 *
 * @param[in] pp  取得するボックスポケモンパラメータ構造体のポインタ
 * @param[in] id  取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]  buf 取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return    取得したデータ
 */
//--------------------------------------------------------------------------
static  u32 ppp_getAct( POKEMON_PASO_PARAM *ppp, int id, void *buf )
{
  u32 ret = 0;

  POKEMON_PASO_PARAM1 *ppp1;
  POKEMON_PASO_PARAM2 *ppp2;
  POKEMON_PASO_PARAM3 *ppp3;
  POKEMON_PASO_PARAM4 *ppp4;

  ppp1 = ( POKEMON_PASO_PARAM1 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA1 );
  ppp2 = ( POKEMON_PASO_PARAM2 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA2 );
  ppp3 = ( POKEMON_PASO_PARAM3 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA3 );
  ppp4 = ( POKEMON_PASO_PARAM4 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA4 );

  switch( id ){
    default:
      ret = 0;
      break;
//PARAM
    case ID_PARA_personal_rnd:
      ret = ppp->personal_rnd;
      break;
    case ID_PARA_pp_fast_mode:
      ret = ppp->pp_fast_mode;
      break;
    case ID_PARA_ppp_fast_mode:
      ret = ppp->ppp_fast_mode;
      break;
    case ID_PARA_fusei_tamago_flag:
      ret = ppp->fusei_tamago_flag;
      break;
    case ID_PARA_checksum:
      ret = ppp->checksum;
      break;
    case ID_PARA_poke_exist:
      ret = ( ppp1->monsno != 0 );
      break;
    case ID_PARA_tamago_exist:
      if( ppp->fusei_tamago_flag ){
        ret = ppp->fusei_tamago_flag;
      }
      else{
        ret = ppp2->tamago_flag;
      }
      break;
    case ID_PARA_monsno_egg:
      ret = ppp1->monsno;
      if( ret == 0){
        break;
      }
      else if( ( ppp2->tamago_flag ) || ( ppp->fusei_tamago_flag ) ){
        ret = MONSNO_TAMAGO;
      }
      break;
    case ID_PARA_level:
      ret = POKETOOL_CalcLevel( ppp1->monsno, ppp2->form_no, ppp1->exp );
      break;
//PARAM1
    case ID_PARA_monsno:
      if( ppp->fusei_tamago_flag ){
        ret = MONSNO_TAMAGO;
      }
      else{
        ret = ppp1->monsno;
      }
      break;
    case ID_PARA_item:
      ret = ppp1->item;
      break;
    case ID_PARA_id_no:
      ret = ppp1->id_no;
      break;
    case ID_PARA_exp:
      ret = ppp1->exp;
      break;
    case ID_PARA_friend:
      ret = ppp1->friend;
      break;
    case ID_PARA_speabino:
      ret = ppp1->speabino;
      break;
    case ID_PARA_mark:
      ret = ppp1->mark;
      break;
    case ID_PARA_country_code:
      ret = ppp1->country_code;
      break;
    case ID_PARA_hp_exp:
      ret = ppp1->hp_exp;
      break;
    case ID_PARA_pow_exp:
      ret = ppp1->pow_exp;
      break;
    case ID_PARA_def_exp:
      ret = ppp1->def_exp;
      break;
    case ID_PARA_agi_exp:
      ret = ppp1->agi_exp;
      break;
    case ID_PARA_spepow_exp:
      ret = ppp1->spepow_exp;
      break;
    case ID_PARA_spedef_exp:
      ret = ppp1->spedef_exp;
      break;
    case ID_PARA_style:
      ret = ppp1->style;
      break;
    case ID_PARA_beautiful:
      ret = ppp1->beautiful;
      break;
    case ID_PARA_cute:
      ret = ppp1->cute;
      break;
    case ID_PARA_clever:
      ret = ppp1->clever;
      break;
    case ID_PARA_strong:
      ret = ppp1->strong;
      break;
    case ID_PARA_fur:
      ret = ppp1->fur;
      break;
    case ID_PARA_sinou_champ_ribbon:        //シンオウチャンプリボン
    case ID_PARA_sinou_battle_tower_ttwin_first:  //シンオウバトルタワータワータイクーン勝利1回目
    case ID_PARA_sinou_battle_tower_ttwin_second: //シンオウバトルタワータワータイクーン勝利2回目
    case ID_PARA_sinou_battle_tower_2vs2_win50:   //シンオウバトルタワータワーダブル50連勝
    case ID_PARA_sinou_battle_tower_aimulti_win50:  //シンオウバトルタワータワーAIマルチ50連勝
    case ID_PARA_sinou_battle_tower_siomulti_win50: //シンオウバトルタワータワー通信マルチ50連勝
    case ID_PARA_sinou_battle_tower_wifi_rank5:   //シンオウバトルタワーWifiランク５入り
    case ID_PARA_sinou_syakki_ribbon:       //シンオウしゃっきリボン
    case ID_PARA_sinou_dokki_ribbon:        //シンオウどっきリボン
    case ID_PARA_sinou_syonbo_ribbon:       //シンオウしょんぼリボン
    case ID_PARA_sinou_ukka_ribbon:         //シンオウうっかリボン
    case ID_PARA_sinou_sukki_ribbon:        //シンオウすっきリボン
    case ID_PARA_sinou_gussu_ribbon:        //シンオウぐっすリボン
    case ID_PARA_sinou_nikko_ribbon:        //シンオウにっこリボン
    case ID_PARA_sinou_gorgeous_ribbon:       //シンオウゴージャスリボン
    case ID_PARA_sinou_royal_ribbon:        //シンオウロイヤルリボン
    case ID_PARA_sinou_gorgeousroyal_ribbon:    //シンオウゴージャスロイヤルリボン
    case ID_PARA_sinou_ashiato_ribbon:        //シンオウあしあとリボン
    case ID_PARA_sinou_record_ribbon:       //シンオウレコードリボン
    case ID_PARA_sinou_history_ribbon:        //シンオウヒストリーリボン
    case ID_PARA_sinou_legend_ribbon:       //シンオウレジェンドリボン
    case ID_PARA_sinou_red_ribbon:          //シンオウレッドリボン
    case ID_PARA_sinou_green_ribbon:        //シンオウグリーンリボン
    case ID_PARA_sinou_blue_ribbon:         //シンオウブルーリボン
    case ID_PARA_sinou_festival_ribbon:       //シンオウフェスティバルリボン
    case ID_PARA_sinou_carnival_ribbon:       //シンオウカーニバルリボン
    case ID_PARA_sinou_classic_ribbon:        //シンオウクラシックリボン
    case ID_PARA_sinou_premiere_ribbon:       //シンオウプレミアリボン
    case ID_PARA_sinou_amari_ribbon:        //あまり
      {
        u64 bit = 1;
        ret = ( ( ppp1->sinou_ribbon & ( bit << id - ID_PARA_sinou_champ_ribbon ) ) != 0 );
      }
      break;
//PARAM2
    case ID_PARA_waza1:
    case ID_PARA_waza2:
    case ID_PARA_waza3:
    case ID_PARA_waza4:
      ret = ppp2->waza[ id-ID_PARA_waza1 ];
      break;
    case ID_PARA_pp1:
    case ID_PARA_pp2:
    case ID_PARA_pp3:
    case ID_PARA_pp4:
      ret = ppp2->pp[ id-ID_PARA_pp1 ];
      break;
    case ID_PARA_pp_count1:
    case ID_PARA_pp_count2:
    case ID_PARA_pp_count3:
    case ID_PARA_pp_count4:
      ret = ppp2->pp_count[ id - ID_PARA_pp_count1 ];
      break;
    case ID_PARA_pp_max1:
    case ID_PARA_pp_max2:
    case ID_PARA_pp_max3:
    case ID_PARA_pp_max4:
      {
        WazaID waza = ppp2->waza[ id-ID_PARA_pp_max1 ];
        if( waza != WAZANO_NULL ){
          u8 maxup_cnt = ppp2->pp_count[ id-ID_PARA_pp_max1 ];
          ret = WAZADATA_GetMaxPP( waza, maxup_cnt );
        }else{
          ret = 0;
        }
      }
      break;
    case ID_PARA_hp_rnd:
      ret = ppp2->hp_rnd;
      break;
    case ID_PARA_pow_rnd:
      ret = ppp2->pow_rnd;
      break;
    case ID_PARA_def_rnd:
      ret = ppp2->def_rnd;
      break;
    case ID_PARA_agi_rnd:
      ret = ppp2->agi_rnd;
      break;
    case ID_PARA_spepow_rnd:
      ret = ppp2->spepow_rnd;
      break;
    case ID_PARA_spedef_rnd:
      ret = ppp2->spedef_rnd;
      break;
    case ID_PARA_tamago_flag:
      if( ppp->fusei_tamago_flag ){
        ret = ppp->fusei_tamago_flag;
      }
      else{
        ret = ppp2->tamago_flag;
      }
      break;
    case ID_PARA_nickname_flag:
      ret = ppp2->nickname_flag;
      break;
    case ID_PARA_stylemedal_normal:
    case ID_PARA_stylemedal_super:
    case ID_PARA_stylemedal_hyper:
    case ID_PARA_stylemedal_master:
    case ID_PARA_beautifulmedal_normal:
    case ID_PARA_beautifulmedal_super:
    case ID_PARA_beautifulmedal_hyper:
    case ID_PARA_beautifulmedal_master:
    case ID_PARA_cutemedal_normal:
    case ID_PARA_cutemedal_super:
    case ID_PARA_cutemedal_hyper:
    case ID_PARA_cutemedal_master:
    case ID_PARA_clevermedal_normal:
    case ID_PARA_clevermedal_super:
    case ID_PARA_clevermedal_hyper:
    case ID_PARA_clevermedal_master:
    case ID_PARA_strongmedal_normal:
    case ID_PARA_strongmedal_super:
    case ID_PARA_strongmedal_hyper:
    case ID_PARA_strongmedal_master:
    case ID_PARA_champ_ribbon:
    case ID_PARA_winning_ribbon:
    case ID_PARA_victory_ribbon:
    case ID_PARA_bromide_ribbon:
    case ID_PARA_ganba_ribbon:
    case ID_PARA_marine_ribbon:
    case ID_PARA_land_ribbon:
    case ID_PARA_sky_ribbon:
    case ID_PARA_country_ribbon:
    case ID_PARA_national_ribbon:
    case ID_PARA_earth_ribbon:
    case ID_PARA_world_ribbon:
      {
        u64 bit = 1;
        ret = ( ( ppp2->old_ribbon & ( bit << id - ID_PARA_stylemedal_normal ) ) !=0 );
      }
      break;
    case ID_PARA_event_get_flag:
      ret = ppp2->event_get_flag;
      break;
    case ID_PARA_sex:
      //必ずパラメータから計算して返すようする
      ret = POKETOOL_GetSex( ppp1->monsno, ppp2->form_no, ppp->personal_rnd );
      //再計算したものを代入しておく
      ppp2->sex = ret;
      //チェックサムを再計算
      ppp->checksum = POKETOOL_make_checksum( &ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
      break;
    case ID_PARA_form_no:
      ret = ppp2->form_no;
      break;
    case ID_PARA_seikaku:
      ret = ppp2->seikaku;
      break;
    case ID_PARA_dummy_p2_2:
      ret = ppp2->dummy_p2_2;
      break;
//PARAM3
    case ID_PARA_nickname:
      if( ppp->fusei_tamago_flag ){
        GFL_MSG_GetString( GlobalMsg_PokeName, MONSNO_DAMETAMAGO, (STRBUF*)buf );
      }
      else{
        GFL_STR_SetStringCode( (STRBUF*)buf, ppp3->nickname );
      }
      break;

    case ID_PARA_nickname_raw:
      if( ppp->fusei_tamago_flag ){
        GFL_MSG_GetStringRaw( GlobalMsg_PokeName, MONSNO_DAMETAMAGO, (STRCODE*)buf, NELEMS(ppp3->nickname) );
      }
      else{
        STRTOOL_Copy( ppp3->nickname, (STRCODE*)buf, NELEMS(ppp3->nickname) );
      }
      break;

    case ID_PARA_pref_code:
      ret = ppp3->pref_code;
      break;
    case ID_PARA_get_cassette:
      ret = ppp3->get_cassette;
      break;
    case ID_PARA_trial_stylemedal_normal:
    case ID_PARA_trial_stylemedal_super:
    case ID_PARA_trial_stylemedal_hyper:
    case ID_PARA_trial_stylemedal_master:
    case ID_PARA_trial_beautifulmedal_normal:
    case ID_PARA_trial_beautifulmedal_super:
    case ID_PARA_trial_beautifulmedal_hyper:
    case ID_PARA_trial_beautifulmedal_master:
    case ID_PARA_trial_cutemedal_normal:
    case ID_PARA_trial_cutemedal_super:
    case ID_PARA_trial_cutemedal_hyper:
    case ID_PARA_trial_cutemedal_master:
    case ID_PARA_trial_clevermedal_normal:
    case ID_PARA_trial_clevermedal_super:
    case ID_PARA_trial_clevermedal_hyper:
    case ID_PARA_trial_clevermedal_master:
    case ID_PARA_trial_strongmedal_normal:
    case ID_PARA_trial_strongmedal_super:
    case ID_PARA_trial_strongmedal_hyper:
    case ID_PARA_trial_strongmedal_master:
    case ID_PARA_amari_ribbon:
      {
        u64 bit = 1;
        ret = ( ( ppp3->new_ribbon & ( bit << id - ID_PARA_trial_stylemedal_normal ) ) != 0 );
      }
      break;
//PARAM4
    case ID_PARA_oyaname:
      GFL_STR_SetStringCode( (STRBUF*)buf, ppp4->oyaname );
      break;

    case ID_PARA_oyaname_raw:
      STRTOOL_Copy( ppp4->oyaname, (STRCODE*)buf, NELEMS(ppp4->oyaname) );
      break;

    case ID_PARA_get_year:              //捕まえた年
      ret = ppp4->get_year;
      break;
    case ID_PARA_get_month:             //捕まえた月
      ret = ppp4->get_month;
      break;
    case ID_PARA_get_day:             //捕まえた日
      ret = ppp4->get_day;
      break;
    case ID_PARA_birth_year:            //生まれた年
      ret = ppp4->birth_year;
      break;
    case ID_PARA_birth_month:           //生まれた月
      ret = ppp4->birth_month;
      break;
    case ID_PARA_birth_day:             //生まれた日
      ret = ppp4->birth_day;
      break;
    case ID_PARA_get_place:             //捕まえた場所
    case ID_PARA_new_get_place:           //捕まえた場所
      //旧領域の値が「とおいばしょ」で、新領域の値が 0 以外なら、新領域の値を返す
      if( ( ppp4->get_place == ID_TOOIBASYO ) && ( ppp2->new_get_place ) ){
        ret = ppp2->new_get_place;
      }
      //前項以外の場合、旧領域の値を返す
      else{
        ret = ppp4->get_place;
      }
      break;
    case ID_PARA_birth_place:           //生まれた場所
    case ID_PARA_new_birth_place:         //生まれた場所
      //旧領域の値が「とおいばしょ」で、新領域の値が 0 以外なら、新領域の値を返す
      if( ( ppp4->birth_place == ID_TOOIBASYO ) && ( ppp2->new_birth_place ) ){
        ret = ppp2->new_birth_place;
      }
      //前項以外の場合、旧領域の値を返す
      else{
        ret = ppp4->birth_place;
      }
      break;
      break;
    case ID_PARA_pokerus:             //ポケルス
      ret = ppp4->pokerus;
      break;
    case ID_PARA_get_ball:              //捕まえたボール
      ret = ppp4->get_ball;
      break;
    case ID_PARA_get_level:             //捕まえたレベル
      ret = ppp4->get_level;
      break;
    case ID_PARA_oyasex:              //親の性別
      ret = ppp4->oyasex;
      break;
    case ID_PARA_get_ground_id:           //捕まえた場所の地形アトリビュート（ミノッチ用）
      ret = ppp4->get_ground_id;
      break;
    case ID_PARA_dummy_p4_1:            //あまり
      ret = ppp4->dummy_p4_1;
      break;

    case ID_PARA_power_rnd:
      ret = ( ppp2->hp_rnd    <<  0 )|
          ( ppp2->pow_rnd   <<  5 )|
          ( ppp2->def_rnd   << 10 )|
          ( ppp2->agi_rnd   << 15 )|
          ( ppp2->spepow_rnd  << 20 )|
          ( ppp2->spedef_rnd  << 25 );
      break;
    //モンスターナンバーがニドランの時にnickname_flagが立っているかチェック
    case ID_PARA_nidoran_nickname:
      if( ( ( ppp1->monsno == MONSNO_NIDORAN_F ) || ( ppp1->monsno == MONSNO_NIDORAN_M ) ) &&
          ( ppp2->nickname_flag == 0 ) ){
        ret = FALSE;
      }
      else{
        ret = TRUE;
      }
      break;
    case ID_PARA_type1:
    case ID_PARA_type2:
      if( ( ppp1->monsno == MONSNO_ARUSEUSU ) && ( ppp1->speabino == TOKUSYU_MARUTITAIPU ) )
      {
          ret = get_type_from_item( ppp1->item );
      }
      else
      {
        ret = POKETOOL_GetPersonalParam( ppp1->monsno, ppp2->form_no, POKEPER_ID_type1 + ( id - ID_PARA_type1 ) );
      }
      break;
  }
  return  ret;
}

//--------------------------------------------------------------------------
/**
 *  復号済みポケモンパラメータ構造体に任意で値を格納
 *
 * @param[out]  pp      格納したいポケモンパラメータ構造体のポインタ
 * @param[in] paramID   格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in] arg     格納したいデータ／配列アドレス等としても利用（paramIDによる）
 */
//--------------------------------------------------------------------------
static  void  pp_putAct( POKEMON_PARAM *pp, int paramID, u32 arg )
{
  switch( paramID ){
  case ID_PARA_condition:
    pp->pcp.condition = arg;
    break;
  case ID_PARA_level:
    pp->pcp.level = arg;
    break;
  case ID_PARA_cb_id:
    pp->pcp.cb_id = arg;
    break;
  case ID_PARA_hp:
    pp->pcp.hp = arg;
    break;
  case ID_PARA_hpmax:
    pp->pcp.hpmax = arg;
    break;
  case ID_PARA_pow:
    pp->pcp.pow = arg;
    break;
  case ID_PARA_def:
    pp->pcp.def = arg;
    break;
  case ID_PARA_agi:
    pp->pcp.agi = arg;
    break;
  case ID_PARA_spepow:
    pp->pcp.spepow = arg;
    break;
  case ID_PARA_spedef:
    pp->pcp.spedef = arg;
    break;
  case ID_PARA_mail_data:
    MailData_Copy( ( MAIL_DATA * )arg, ( MAIL_DATA * )pp->pcp.mail_data );
    break;
  case ID_PARA_cb_core:
//カスタムボール処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning CB_CORE Nothing
#endif
//    CB_Tool_CoreData_Copy( ( CB_CORE * )buf, &pp->pcp.cb_core );
    break;

  default:
    ppp_putAct( ( POKEMON_PASO_PARAM * )&pp->ppp, paramID, arg );
    break;
  }
}

//--------------------------------------------------------------------------
/**
 *  復号済みボックスポケモンパラメータ構造体に任意で値を格納
 *
 * @param[out]  pp      格納したいポケモンパラメータ構造体のポインタ
 * @param[in] paramID   格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in] arg     格納したいデータ／配列アドレス等としても利用（paramIDによる）
 */
//--------------------------------------------------------------------------
static  void  ppp_putAct( POKEMON_PASO_PARAM *ppp, int paramID, u32 arg )
{
  POKEMON_PASO_PARAM1 *ppp1;
  POKEMON_PASO_PARAM2 *ppp2;
  POKEMON_PASO_PARAM3 *ppp3;
  POKEMON_PASO_PARAM4 *ppp4;

  ppp1 = ( POKEMON_PASO_PARAM1 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA1 );
  ppp2 = ( POKEMON_PASO_PARAM2 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA2 );
  ppp3 = ( POKEMON_PASO_PARAM3 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA3 );
  ppp4 = ( POKEMON_PASO_PARAM4 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA4 );

  switch( paramID ){
//PARAM
    case ID_PARA_personal_rnd:
      ppp->personal_rnd = arg;
      break;
    case ID_PARA_pp_fast_mode:
      GF_ASSERT_MSG( ( 0 ), "fast_modeへ不正な書き込み\n" );
      ppp->pp_fast_mode = arg;
      break;
    case ID_PARA_ppp_fast_mode:
      GF_ASSERT_MSG( ( 0 ), "fast_modeへ不正な書き込み\n" );
      ppp->ppp_fast_mode = arg;
      break;
    case ID_PARA_fusei_tamago_flag:
      ppp->fusei_tamago_flag = arg;
      break;
    case ID_PARA_checksum:
      ppp->checksum = arg;
      break;
//PARAM1
    case ID_PARA_monsno:
      ppp1->monsno = arg;
      break;
    case ID_PARA_item:
      ppp1->item = arg;
      break;
    case ID_PARA_id_no:
      ppp1->id_no = arg;
      break;
    case ID_PARA_exp:
      ppp1->exp = arg;
      break;
    case ID_PARA_friend:
      ppp1->friend = arg;
      break;
    case ID_PARA_speabino:
      ppp1->speabino = arg;
      break;
    case ID_PARA_mark:
      ppp1->mark = arg;
      break;
    case ID_PARA_country_code:
      ppp1->country_code = arg;
      break;
    case ID_PARA_hp_exp:
      ppp1->hp_exp = arg;
      break;
    case ID_PARA_pow_exp:
      ppp1->pow_exp = arg;
      break;
    case ID_PARA_def_exp:
      ppp1->def_exp = arg;
      break;
    case ID_PARA_agi_exp:
      ppp1->agi_exp = arg;
      break;
    case ID_PARA_spepow_exp:
      ppp1->spepow_exp = arg;
      break;
    case ID_PARA_spedef_exp:
      ppp1->spedef_exp = arg;
      break;
    case ID_PARA_style:
      ppp1->style = arg;
      break;
    case ID_PARA_beautiful:
      ppp1->beautiful = arg;
      break;
    case ID_PARA_cute:
      ppp1->cute = arg;
      break;
    case ID_PARA_clever:
      ppp1->clever = arg;
      break;
    case ID_PARA_strong:
      ppp1->strong = arg;
      break;
    case ID_PARA_fur:
      ppp1->fur = arg;
      break;
    case ID_PARA_sinou_champ_ribbon:        //シンオウチャンプリボン
    case ID_PARA_sinou_battle_tower_ttwin_first:  //シンオウバトルタワータワータイクーン勝利1回目
    case ID_PARA_sinou_battle_tower_ttwin_second: //シンオウバトルタワータワータイクーン勝利2回目
    case ID_PARA_sinou_battle_tower_2vs2_win50:   //シンオウバトルタワータワーダブル50連勝
    case ID_PARA_sinou_battle_tower_aimulti_win50:  //シンオウバトルタワータワーAIマルチ50連勝
    case ID_PARA_sinou_battle_tower_siomulti_win50: //シンオウバトルタワータワー通信マルチ50連勝
    case ID_PARA_sinou_battle_tower_wifi_rank5:   //シンオウバトルタワーWifiランク５入り
    case ID_PARA_sinou_syakki_ribbon:       //シンオウしゃっきリボン
    case ID_PARA_sinou_dokki_ribbon:        //シンオウどっきリボン
    case ID_PARA_sinou_syonbo_ribbon:       //シンオウしょんぼリボン
    case ID_PARA_sinou_ukka_ribbon:         //シンオウうっかリボン
    case ID_PARA_sinou_sukki_ribbon:        //シンオウすっきリボン
    case ID_PARA_sinou_gussu_ribbon:        //シンオウぐっすリボン
    case ID_PARA_sinou_nikko_ribbon:        //シンオウにっこリボン
    case ID_PARA_sinou_gorgeous_ribbon:       //シンオウゴージャスリボン
    case ID_PARA_sinou_royal_ribbon:        //シンオウロイヤルリボン
    case ID_PARA_sinou_gorgeousroyal_ribbon:    //シンオウゴージャスロイヤルリボン
    case ID_PARA_sinou_ashiato_ribbon:        //シンオウあしあとリボン
    case ID_PARA_sinou_record_ribbon:       //シンオウレコードリボン
    case ID_PARA_sinou_history_ribbon:        //シンオウヒストリーリボン
    case ID_PARA_sinou_legend_ribbon:       //シンオウレジェンドリボン
    case ID_PARA_sinou_red_ribbon:          //シンオウレッドリボン
    case ID_PARA_sinou_green_ribbon:        //シンオウグリーンリボン
    case ID_PARA_sinou_blue_ribbon:         //シンオウブルーリボン
    case ID_PARA_sinou_festival_ribbon:       //シンオウフェスティバルリボン
    case ID_PARA_sinou_carnival_ribbon:       //シンオウカーニバルリボン
    case ID_PARA_sinou_classic_ribbon:        //シンオウクラシックリボン
    case ID_PARA_sinou_premiere_ribbon:       //シンオウプレミアリボン
    case ID_PARA_sinou_amari_ribbon:        //あまり
      {
        u64 bit = 1 << ( paramID - ID_PARA_sinou_champ_ribbon );
        if( arg ){
          ppp1->sinou_ribbon |= bit;
        }
        else{
          ppp1->sinou_ribbon &= ( bit ^ 0xffffffff );
        }
      }
      break;
//PARAM2
    case ID_PARA_waza1:
    case ID_PARA_waza2:
    case ID_PARA_waza3:
    case ID_PARA_waza4:
      ppp2->waza[ paramID - ID_PARA_waza1 ] = arg;
      break;
    case ID_PARA_pp1:
    case ID_PARA_pp2:
    case ID_PARA_pp3:
    case ID_PARA_pp4:
      ppp2->pp[ paramID - ID_PARA_pp1 ] = arg;
      break;
    case ID_PARA_pp_count1:
    case ID_PARA_pp_count2:
    case ID_PARA_pp_count3:
    case ID_PARA_pp_count4:
      ppp2->pp_count[ paramID - ID_PARA_pp_count1 ] = arg;
      break;
    case ID_PARA_pp_max1:
    case ID_PARA_pp_max2:
    case ID_PARA_pp_max3:
    case ID_PARA_pp_max4:
      OS_Printf("不正な書き込み\n");
      break;
    case ID_PARA_hp_rnd:
      ppp2->hp_rnd = arg;
      break;
    case ID_PARA_pow_rnd:
      ppp2->pow_rnd = arg;
      break;
    case ID_PARA_def_rnd:
      ppp2->def_rnd = arg;
      break;
    case ID_PARA_agi_rnd:
      ppp2->agi_rnd = arg;
      break;
    case ID_PARA_spepow_rnd:
      ppp2->spepow_rnd = arg;
      break;
    case ID_PARA_spedef_rnd:
      ppp2->spedef_rnd = arg;
      break;
    case ID_PARA_tamago_flag:
      ppp2->tamago_flag = arg;
      break;
    case ID_PARA_stylemedal_normal:
    case ID_PARA_stylemedal_super:
    case ID_PARA_stylemedal_hyper:
    case ID_PARA_stylemedal_master:
    case ID_PARA_beautifulmedal_normal:
    case ID_PARA_beautifulmedal_super:
    case ID_PARA_beautifulmedal_hyper:
    case ID_PARA_beautifulmedal_master:
    case ID_PARA_cutemedal_normal:
    case ID_PARA_cutemedal_super:
    case ID_PARA_cutemedal_hyper:
    case ID_PARA_cutemedal_master:
    case ID_PARA_clevermedal_normal:
    case ID_PARA_clevermedal_super:
    case ID_PARA_clevermedal_hyper:
    case ID_PARA_clevermedal_master:
    case ID_PARA_strongmedal_normal:
    case ID_PARA_strongmedal_super:
    case ID_PARA_strongmedal_hyper:
    case ID_PARA_strongmedal_master:
    case ID_PARA_champ_ribbon:
    case ID_PARA_winning_ribbon:
    case ID_PARA_victory_ribbon:
    case ID_PARA_bromide_ribbon:
    case ID_PARA_ganba_ribbon:
    case ID_PARA_marine_ribbon:
    case ID_PARA_land_ribbon:
    case ID_PARA_sky_ribbon:
    case ID_PARA_country_ribbon:
    case ID_PARA_national_ribbon:
    case ID_PARA_earth_ribbon:
    case ID_PARA_world_ribbon:
      {
        u64 bit = 1 << ( paramID - ID_PARA_stylemedal_normal );
        if( arg ){
          ppp2->old_ribbon |= bit;
        }
        else{
          ppp2->old_ribbon &= ( bit ^ 0xffffffff );
        }
      }
      break;
    case ID_PARA_event_get_flag:
      ppp2->event_get_flag = arg;
      break;
    case ID_PARA_sex:
      //ppp2->sex=buf8[0];
      //必ずパラメータから計算して代入する
      ppp2->sex = POKETOOL_GetSex( ppp1->monsno, ppp2->form_no, ppp->personal_rnd );
      GF_ASSERT_MSG( ( arg == ppp2->sex ), "monsno=%d, Disagreement personal_rnd(%d) <> ID_PARA_sex(%d)\n", ppp1->monsno, ppp2->sex, arg);
      break;
    case ID_PARA_form_no:
      ppp2->form_no = arg;
      break;
    case ID_PARA_seikaku:
      ppp2->seikaku = arg;
      break;
    case ID_PARA_dummy_p2_2:
      ppp2->dummy_p2_2 = arg;
      break;

    case ID_PARA_nickname_flag:
      GF_ASSERT_MSG(0, "ニックネームフラグを直接セットすることはできません\n");
      break;
//PARAM3
    case ID_PARA_nickname:
      GFL_STR_GetStringCode( (STRBUF*)arg, ppp3->nickname, NELEMS(ppp3->nickname) );
      ppp2->nickname_flag = pppAct_check_nickname( ppp );
      break;
    case ID_PARA_nickname_raw:
      STRTOOL_Copy( (const STRCODE*)arg, ppp3->nickname, NELEMS(ppp3->nickname) );
      ppp2->nickname_flag = pppAct_check_nickname( ppp );
      break;

    case ID_PARA_pref_code:
      ppp3->pref_code = arg;
      break;
    case ID_PARA_get_cassette:
      ppp3->get_cassette = arg;
      break;
    case ID_PARA_trial_stylemedal_normal:
    case ID_PARA_trial_stylemedal_super:
    case ID_PARA_trial_stylemedal_hyper:
    case ID_PARA_trial_stylemedal_master:
    case ID_PARA_trial_beautifulmedal_normal:
    case ID_PARA_trial_beautifulmedal_super:
    case ID_PARA_trial_beautifulmedal_hyper:
    case ID_PARA_trial_beautifulmedal_master:
    case ID_PARA_trial_cutemedal_normal:
    case ID_PARA_trial_cutemedal_super:
    case ID_PARA_trial_cutemedal_hyper:
    case ID_PARA_trial_cutemedal_master:
    case ID_PARA_trial_clevermedal_normal:
    case ID_PARA_trial_clevermedal_super:
    case ID_PARA_trial_clevermedal_hyper:
    case ID_PARA_trial_clevermedal_master:
    case ID_PARA_trial_strongmedal_normal:
    case ID_PARA_trial_strongmedal_super:
    case ID_PARA_trial_strongmedal_hyper:
    case ID_PARA_trial_strongmedal_master:
    case ID_PARA_amari_ribbon:
      {
        u64 bit = 1 << ( paramID - ID_PARA_trial_stylemedal_normal );
        if( arg ){
          ppp3->new_ribbon |= bit;
        }
        else{
          ppp3->new_ribbon &= ( bit ^ 0xffffffffffffffff );
        }
      }
      break;
//PARAM4
    case ID_PARA_oyaname:
      GFL_STR_GetStringCode( (STRBUF*)arg, ppp4->oyaname, NELEMS(ppp4->oyaname) );
      break;
    case ID_PARA_oyaname_raw:
      STRTOOL_Copy( (const STRCODE*)arg, ppp4->oyaname, NELEMS(ppp4->oyaname) );
      break;

    case ID_PARA_get_year:              //捕まえた年
      ppp4->get_year = arg;
      break;
    case ID_PARA_get_month:             //捕まえた月
      ppp4->get_month = arg;
      break;
    case ID_PARA_get_day:             //捕まえた日
      ppp4->get_day = arg;
      break;
    case ID_PARA_birth_year:            //生まれた年
      ppp4->birth_year = arg;
      break;
    case ID_PARA_birth_month:           //生まれた月
      ppp4->birth_month = arg;
      break;
    case ID_PARA_birth_day:             //生まれた日
      ppp4->birth_day = arg;
      break;
    case ID_PARA_get_place:             //捕まえた場所
    case ID_PARA_new_get_place:           //捕まえた場所
//処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning PlaceName_RangeCheckDP Nothing
#endif
#if 0
      //DPにもあった場所なら、新領域と旧領域にその地名IDを書き込む
      if( buf16[0] == 0 || PlaceName_RangeCheckDP( buf16[0] ) == TRUE ){
        //get_placeが0かどうかでタマゴで孵ったか、生体で取得したか判定しているので
        //get_placeの0セットはありにする
        ppp4->get_place = buf16[0];       //捕まえた場所
        ppp2->new_get_place = buf16[0];     //捕まえた場所
      }
      //DPになかった場所なら、新領域にその地名ID、旧領域に「とおいばしょ」を書き込む
      else{
        ppp4->get_place = ID_TOOIBASYO;     //捕まえた場所
        ppp2->new_get_place = buf16[0];     //捕まえた場所
      }
#endif
      break;
    case ID_PARA_birth_place:           //生まれた場所
    case ID_PARA_new_birth_place:         //生まれた場所
//処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning PlaceName_RangeCheckDP Nothing
#endif
#if 0
      //DPにもあった場所なら、新領域と旧領域にその地名IDを書き込む
      if( buf16[0] == 0 || PlaceName_RangeCheckDP( buf16[0] ) == TRUE ){
        //範囲外マップで捕まえたポケモンは「なぞのばしょ」と表示する為、
        //birth_placeの0セットもありにした。
        ppp4->birth_place = buf16[0];     //生まれた場所
        ppp2->new_birth_place = buf16[0];   //生まれた場所
      }
      //DPになかった場所なら、新領域にその地名ID、旧領域に「とおいばしょ」を書き込む
      else{
        ppp4->birth_place = ID_TOOIBASYO;   //生まれた場所
        ppp2->new_birth_place = buf16[0];   //生まれた場所
      }
#endif
      break;
    case ID_PARA_pokerus:             //ポケルス
      ppp4->pokerus = arg;
      break;
    case ID_PARA_get_ball:              //捕まえたボール
      ppp4->get_ball = arg;
      break;
    case ID_PARA_get_level:             //捕まえたレベル
      ppp4->get_level = arg;
      break;
    case ID_PARA_oyasex:              //親の性別
      ppp4->oyasex = arg;
      break;
    case ID_PARA_get_ground_id:           //捕まえた場所の地形アトリビュート（ミノッチ用）
      ppp4->get_ground_id = arg;
      break;
    case ID_PARA_dummy_p4_1:            //あまり
      ppp4->dummy_p4_1 = arg;
      break;

    case ID_PARA_power_rnd:
      ppp2->hp_rnd   = PTL_SETUP_POW_UNPACK( arg, PTL_ABILITY_HP );
      ppp2->pow_rnd  = PTL_SETUP_POW_UNPACK( arg, PTL_ABILITY_ATK );
      ppp2->def_rnd  = PTL_SETUP_POW_UNPACK( arg, PTL_ABILITY_DEF );
      ppp2->spepow_rnd = PTL_SETUP_POW_UNPACK( arg, PTL_ABILITY_SPATK );
      ppp2->spedef_rnd = PTL_SETUP_POW_UNPACK( arg, PTL_ABILITY_SPDEF );
      ppp2->agi_rnd  = PTL_SETUP_POW_UNPACK( arg, PTL_ABILITY_AGI );
      break;

    //モンスターナンバーがニドランの時にnickname_flagが立っているかチェック
    case ID_PARA_nidoran_nickname:
    case ID_PARA_type1:
    case ID_PARA_type2:
      OS_TPrintf("不正な書き込み\n");
      break;
  }
}



//============================================================================================
/**
 *  ポケモンパラメータ構造体に任意で値を加算
 *
 * @param[in] pp    加算したいポケモンパラメータ構造体のポインタ
 * @param[in] id    加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in] value 加算したいデータのポインタ
 */
//============================================================================================
static  void  pp_addAct( POKEMON_PARAM *pp, int id, int value )
{
  switch( id ){
    case ID_PARA_hp:
      if( ( pp->pcp.hp + value ) > pp->pcp.hpmax ){
        pp->pcp.hp = pp->pcp.hpmax;
      }
      else if( ( pp->pcp.hp + value ) < 0 ){
        pp->pcp.hp = 0;
      }
      else{
        pp->pcp.hp += value;
      }
      break;
    case ID_PARA_condition:
    case ID_PARA_level:
    case ID_PARA_cb_id:
    case ID_PARA_hpmax:
    case ID_PARA_pow:
    case ID_PARA_def:
    case ID_PARA_agi:
    case ID_PARA_spepow:
    case ID_PARA_spedef:
    case ID_PARA_mail_data:
      GF_ASSERT_MSG( (0), "Addできないパラメータです\n" );
      break;
    default:
      ppp_addAct( ( POKEMON_PASO_PARAM * )&pp->ppp, id, value );
      break;
  }
}

//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体に任意で値を加算
 *
 * @param[in] ppp   加算したいボックスポケモンパラメータ構造体のポインタ
 * @param[in] id    加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in] value 加算したいデータのポインタ
 */
//============================================================================================
static  void  ppp_addAct( POKEMON_PASO_PARAM *ppp, int id, int value )
{
  POKEMON_PASO_PARAM1 *ppp1;
  POKEMON_PASO_PARAM2 *ppp2;
  POKEMON_PASO_PARAM3 *ppp3;
  POKEMON_PASO_PARAM4 *ppp4;

  ppp1 = ( POKEMON_PASO_PARAM1 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA1 );
  ppp2 = ( POKEMON_PASO_PARAM2 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA2 );
  ppp3 = ( POKEMON_PASO_PARAM3 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA3 );
  ppp4 = ( POKEMON_PASO_PARAM4 * )POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA4 );

  switch( id ){
    case ID_PARA_exp:
      {
        u32 max_exp = POKETOOL_GetMinExp( ppp1->monsno, ppp2->form_no, PTL_LEVEL_MAX );
        if( ( ppp1->exp + value ) > max_exp )
        {
          ppp1->exp = max_exp;
        }
        else
        {
          ppp1->exp += value;
        }
      }
      break;
    case ID_PARA_friend:
      round_calc( &ppp1->friend, value, PTL_FRIEND_MAX );
      break;
    case ID_PARA_hp_exp:
    case ID_PARA_pow_exp:
    case ID_PARA_def_exp:
    case ID_PARA_agi_exp:
    case ID_PARA_spepow_exp:
    case ID_PARA_spedef_exp:
      GF_ASSERT_MSG( ( 0 ), "不正な加算：努力値は、合計値の上限が決まっているので、個別の加算はサポートしません\n");
      break;
    case ID_PARA_style:
      round_calc( &ppp1->style, value, PTL_STYLE_MAX );
      break;
    case ID_PARA_beautiful:
      round_calc( &ppp1->beautiful, value, PTL_BEAUTIFUL_MAX );
      break;
    case ID_PARA_cute:
      round_calc( &ppp1->cute, value, PTL_CUTE_MAX );
      break;
    case ID_PARA_clever:
      round_calc( &ppp1->clever, value, PTL_CLEVER_MAX );
      break;
    case ID_PARA_strong:
      round_calc( &ppp1->strong, value, PTL_STRONG_MAX );
      break;
    case ID_PARA_fur:
      round_calc( &ppp1->fur, value, PTL_FUR_MAX );
      break;
    case ID_PARA_pp1:
    case ID_PARA_pp2:
    case ID_PARA_pp3:
    case ID_PARA_pp4:
      round_calc( &ppp2->pp[ id - ID_PARA_pp1 ],
             value,
             WAZADATA_GetMaxPP( ppp2->waza[ id - ID_PARA_pp1 ], ppp2->pp_count[ id - ID_PARA_pp1 ] ) );
      break;
    case ID_PARA_pp_count1:
    case ID_PARA_pp_count2:
    case ID_PARA_pp_count3:
    case ID_PARA_pp_count4:
      round_calc( &ppp2->pp_count[ id - ID_PARA_pp_count1 ], value, PTL_WAZAPP_COUNT_MAX );
      break;
    default:
      GF_ASSERT_MSG( ( 0 ), "不正な加算\n" );
      break;
  }
}
//--------------------------------------------------------------------------
/**
 * 数値計算後、0以上～指定上限値以下になるように丸める
 *
 * @param[io]   data    計算前数値ワーク
 * @param[in]   value   加算（減算）したい数値
 * @param[in]   max     上限値
 */
//--------------------------------------------------------------------------
static  void  round_calc( u8 *data, int value, int max )
{
  int work=*data;

  if( ( work + value ) > max ){
    work = max;
  }
  if( ( work + value ) < 0 ){
    work = 0;
  }
  else{
    work += value;
  }

  *data = ( u8 )work;
}



//--------------------------------------------------------------------------
/**
 * 暗号処理
 *
 * @param[io] data  暗号化するデータのポインタ
 * @param[in] size  暗号化するデータのサイズ
 * @param[in] code  暗号化キーの初期値
 */
//--------------------------------------------------------------------------
void  POKETOOL_encode_data( void *data, u32 size, u32 code )
{
  int i;
  u16 *data_p = ( u16 * )data;

  //暗号は、乱数暗号キーでマスク
  for( i = 0 ; i < size / 2 ; i++ ){
    data_p[i] ^= rand_next( &code );
  }
}
//--------------------------------------------------------------------------
/**
 *  復号処理
 *
 * @param[io] data  復号するデータのポインタ
 * @param[in] size  復号するデータのサイズ
 * @param[in] code  暗号化キーの初期値
 */
//--------------------------------------------------------------------------
//static  void  POKETOOL_decord_data( void *data, u32 size, u32 code )
//{
//  POKETOOL_encode_data( data, size, code );
//}

//--------------------------------------------------------------------------
/**
 *  乱数暗号キー生成ルーチン
 *
 * @param[io] code  暗号キー格納ワークのポインタ
 *
 * @return  暗号キー格納ワークの上位2バイトを暗号キーとして返す
 */
//--------------------------------------------------------------------------
static  u16 rand_next( u32 *code )
{
    code[0] = code[0] *1103515245L + 24691;
    return ( u16 )( code[0] / 65536L ) ;
}

//--------------------------------------------------------------------------
/**
 *  ポケモンパラメータのチェックサムを生成
 *
 * @param[in] data  ポケモンパラメータ構造体のポインタ
 * @param[in] size  チェックサムを生成するデータのサイズ
 *
 * @return  生成したチェックサム
 */
//--------------------------------------------------------------------------
u16 POKETOOL_make_checksum( const void *data, u32 size )
{
  const u16 *data_p = ( u16 * )data;
  u16 sum = 0;
  int i;

  for( i = 0 ; i < size / 2 ; i++ ){
    sum += data_p[i];
  }

  return sum;
}

//--------------------------------------------------------------------------
/**
 * 復号済みPPPについて、ニックネームが付けられているか判定
 *
 * @param   ppp     復号済みPPP
 *
 * @retval  BOOL    nicknameフィールドにデフォルト名が入っていない場合はTRUE
 */
//--------------------------------------------------------------------------
static BOOL pppAct_check_nickname( POKEMON_PASO_PARAM* ppp )
{
  POKEMON_PASO_PARAM1 *ppp1;
  POKEMON_PASO_PARAM3 *ppp3;

  ppp1 = (POKEMON_PASO_PARAM1*)POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA1 );
  ppp3 = (POKEMON_PASO_PARAM3*)POKETOOL_ppp_get_param_block( ppp, ppp->personal_rnd, ID_POKEPARA3 );

  GFL_MSG_GetStringRaw( GlobalMsg_PokeName, ppp1->monsno, StrBuffer, NELEMS(StrBuffer) );
  return !STRTOOL_Comp( StrBuffer, ppp3->nickname );
}

//============================================================================================
/**
 *  性格によるパラメータ変化値テーブル
 */
//============================================================================================
static  const s8  SeikakuAbiTbl[][5]={
//    atk def sat sdf agi
  {  0,  0,  0,  0,  0, },  // がんばりや
  {  1, -1,  0,  0,  0, },  // さみしがり
  {  1,  0,  0,  0, -1, },  // ゆうかん
  {  1,  0, -1,  0,  0, },  // いじっぱり
  {  1,  0,  0, -1,  0, },  // やんちゃ
  { -1,  1,  0,  0,  0, },  // ずぶとい
  {  0,  0,  0,  0,  0, },  // すなお
  {  0,  1,  0,  0, -1, },  // のんき
  {  0,  1, -1,  0,  0, },  // わんぱく
  {  0,  1,  0, -1,  0, },  // のうてんき
  { -1,  0,  0,  0,  1, },  // おくびょう
  {  0, -1,  0,  0,  1, },  // せっかち
  {  0,  0,  0,  0,  0, },  // まじめ
  {  0,  0, -1,  0,  1, },  // ようき
  {  0,  0,  0, -1,  1, },  // むじゃき
  { -1,  0,  1,  0,  0, },  // ひかえめ
  {  0, -1,  1,  0,  0, },  // おっとり
  {  0,  0,  1,  0, -1, },  // れいせい
  {  0,  0,  0,  0,  0, },  // てれや
  {  0,  0,  1, -1,  0, },  // うっかりや
  { -1,  0,  0,  1,  0, },  // おだやか
  {  0, -1,  0,  1,  0, },  // おとなしい
  {  0,  0,  0,  1, -1, },  // なまいき
  {  0,  0, -1,  1,  0, },  // しんちょう
  {  0,  0,  0,  0,  0, },  // きまぐれ
};

//============================================================================================
/**
 *  性格によるパラメータ変化計算ルーチン
 *
 * @param[in] chr   計算する性格
 * @param[in] para  計算するパラメータ値
 * @param[in] cond  取り出すパラメータ変化値テーブルのインデックス（poke_tool.hに定義）
 *
 * @return  計算結果
 */
//============================================================================================
static  u16 calc_abi_seikaku( u8 chr, u16 para, u8 cond )
{
  u16 ret;

  if( cond >= PTL_ABILITY_MAX )
  {
    GF_ASSERT(0);
    return para;
  }

  switch( SeikakuAbiTbl[ chr ][ cond - 1 ]){
    case 1:
      ret = para * 110;
      ret /= 100;
      break;
    case -1:
      ret = para * 90;
      ret /= 100;
      break;
    default:
      ret = para;
      break;
  }
  return  ret;
}

//============================================================================================
/**
 *  ポケモン成長テーブルデータを取得
 *
 * @param[in] para    取得する成長テーブルのインデックス（0～7）
 * @param[out]  GrowTable 取得した成長テーブルの格納先
 */
//============================================================================================
static  void  load_grow_table( int para, u32 *GrowTable )
{
  GF_ASSERT_MSG( para < 8, "load_grow_table:TableIndexOver!\n" );
  GFL_ARC_LoadData( GrowTable, ARCID_GROW_TBL, para );
}

//============================================================================================
/**
 *  ポケモン成長テーブルデータから任意で経験値データを取得
 *
 * @param[in] para    取得する成長テーブルのインデックス（0～7）
 * @param[in] level   取得する成長テーブルのレベル（0～100）
 *
 * @return  取得した経験値データ
 */
//============================================================================================
static  u32 get_growtbl_param( int para, int level )
{
  GF_ASSERT_MSG( para < 8, "get_growtbl_param:TableIndexOver!\n" );
  GF_ASSERT_MSG( level <= 101, "get_growtbl_param:Level Over!\n" );

  load_grow_table( para, &GrowTable[0] );

  return GrowTable[ level ];
}

//=============================================================================================
/**
 *  装備アイテムからタイプを取得
 *
 * @param[in] item 装備アイテム
 *
 * @retval  PokeType
 */
//=============================================================================================
static  PokeType  get_type_from_item( u16 item )
{
  PokeType  type;

  switch( item ){
  case ITEM_HINOTAMAPUREETO:
    type = POKETYPE_HONOO;
    break;
  case ITEM_SIZUKUPUREETO:
    type = POKETYPE_MIZU;
    break;
  case ITEM_IKAZUTIPUREETO:
    type = POKETYPE_DENKI;
    break;
  case ITEM_MIDORINOPUREETO:
    type = POKETYPE_KUSA;
    break;
  case ITEM_TURARANOPUREETO:
    type = POKETYPE_KOORI;
    break;
  case ITEM_KOBUSINOPUREETO:
    type = POKETYPE_KAKUTOU;
    break;
  case ITEM_MOUDOKUPUREETO:
    type = POKETYPE_DOKU;
    break;
  case ITEM_DAITINOPUREETO:
    type = POKETYPE_JIMEN;
    break;
  case ITEM_AOZORAPUREETO:
    type = POKETYPE_HIKOU;
    break;
  case ITEM_HUSIGINOPUREETO:
    type = POKETYPE_ESPER;
    break;
  case ITEM_TAMAMUSIPUREETO:
    type = POKETYPE_MUSHI;
    break;
  case ITEM_GANSEKIPUREETO:
    type = POKETYPE_IWA;
    break;
  case ITEM_MONONOKEPUREETO:
    type = POKETYPE_GHOST;
    break;
  case ITEM_RYUUNOPUREETO:
    type = POKETYPE_DRAGON;
    break;
  case ITEM_KOWAMOTEPUREETO:
    type = POKETYPE_AKU;
    break;
  case ITEM_KOUTETUPUREETO:
    type = POKETYPE_HAGANE;
    break;
  default:
    type = POKETYPE_NORMAL;
    break;
  }
  return type;
}


//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体アドレス取得用テーブル値
 */
//============================================================================================

//============================================================================================
/**
 *  ボックスポケモンパラメータ構造体内のメンバのアドレスを取得
 *
 * @param[in] ppp 取得したいボックスポケモンパラメータ構造体のポインタ
 * @param[in] rnd 構造体入れ替えのキー
 * @param[in] id  取り出したいメンバのインデックス（poke_tool.hに定義）
 *
 * @return  取得したアドレス
 */
//============================================================================================
void  *POKETOOL_ppp_get_param_block( POKEMON_PASO_PARAM *ppp, u32 rnd, u8 id )
{
  enum {
    POS1 = ( sizeof( POKEMON_PASO_PARAM1 ) * 0 ),
    POS2 = ( sizeof( POKEMON_PASO_PARAM1 ) * 1 ),
    POS3 = ( sizeof( POKEMON_PASO_PARAM1 ) * 2 ),
    POS4 = ( sizeof( POKEMON_PASO_PARAM1 ) * 3 ),
  };

  static  const u8 PokeParaAdrsTbl[32][4] =
  {
    { POS1, POS2, POS3, POS4 },
    { POS1, POS2, POS4, POS3 },
    { POS1, POS3, POS2, POS4 },
    { POS1, POS4, POS2, POS3 },
    { POS1, POS3, POS4, POS2 },
    { POS1, POS4, POS3, POS2 },
    { POS2, POS1, POS3, POS4 },
    { POS2, POS1, POS4, POS3 },
    { POS3, POS1, POS2, POS4 },
    { POS4, POS1, POS2, POS3 },
    { POS3, POS1, POS4, POS2 },
    { POS4, POS1, POS3, POS2 },
    { POS2, POS3, POS1, POS4 },
    { POS2, POS4, POS1, POS3 },
    { POS3, POS2, POS1, POS4 },
    { POS4, POS2, POS1, POS3 },
    { POS3, POS4, POS1, POS2 },
    { POS4, POS3, POS1, POS2 },
    { POS2, POS3, POS4, POS1 },
    { POS2, POS4, POS3, POS1 },
    { POS3, POS2, POS4, POS1 },
    { POS4, POS2, POS3, POS1 },
    { POS3, POS4, POS2, POS1 },
    { POS4, POS3, POS2, POS1 },
    { POS1, POS2, POS3, POS4 },
    { POS1, POS2, POS4, POS3 },
    { POS1, POS3, POS2, POS4 },
    { POS1, POS4, POS2, POS3 },
    { POS1, POS3, POS4, POS2 },
    { POS1, POS4, POS3, POS2 },
    { POS2, POS1, POS3, POS4 },
    { POS2, POS1, POS4, POS3 },
  };

  void  *ret;

  rnd = ( rnd & 0x0003e000 ) >> 13;

  GF_ASSERT_MSG( rnd <= ID_POKEPARADATA62, "POKETOOL_ppp_get_param_block:RND Index Over!" );
  GF_ASSERT_MSG( id <= ID_POKEPARA4, "POKETOOL_ppp_get_param_block:ID Index Over!" );

  ret = &ppp->paradata[ PokeParaAdrsTbl[ rnd ][ id ] ];

  return  ret;
}

//============================================================================================
/**
 * ポケモンパーソナル操作関数系
 */
//============================================================================================

//----------------------------------------------------------------------------------
/**
 * ローカルワークにパーソナルデータ読み込み（直前と同じデータならそのまま何もしない）
 *
 * @param   monsno
 * @param   formno
 *
 * @retval  POKEMON_PERSONAL_DATA*    読み込み先ワークポインタ
 */
//----------------------------------------------------------------------------------
static POKEMON_PERSONAL_DATA* Personal_Load( u16 monsno, u16 formno )
{
  if( (monsno != PersonalLatestMonsNo) || (formno != PersonalLatestFormNo) ){
    PersonalLatestFormNo = formno;
    PersonalLatestMonsNo = monsno;
    POKE_PERSONAL_LoadData( monsno, formno, &PersonalDataWork );
  }
  return &PersonalDataWork;
}

//----------------------------------------------------------------------------------
/**
 * とくせい数をカウント
 *
 * @param   ppd
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 Personal_GetTokuseiCount( POKEMON_PERSONAL_DATA* ppd )
{
  if( POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi2 ) ){
    return 2;
  }
  return 1;
}

//============================================================================================
/**
 *  ポケモンパーソナル構造体データから任意でデータを取得
 *  （一発でデータを取り出したいときに使用してください、
 *    パーソナルデータをたくさん取り出したいときはOpen,Get,Closeした方が軽いです）
 *
 * @param[in] mons_no 取得したいポケモンナンバー
 * @param[in] form_no 取得したいポケモンのフォルムナンバー
 * @param[in] param 取得したデータのインデックス（poke_tool.hに定義）
 *
 * @return  取得したデータ
 */
//============================================================================================
u32 POKETOOL_GetPersonalParam( u16 mons_no, u16 form_no, PokePersonalParamID param )
{
  POKEMON_PERSONAL_DATA* ppd = Personal_Load( mons_no, form_no );
  return POKE_PERSONAL_GetParam( ppd, param );
}

//==================================================================
/**
 * 指定したレベルにポケモンパラメータを補正します
 *
 * @param   pp      対象のポケモンへのポインタ
 * @param   level   レベル
 */
//==================================================================
void POKETOOL_MakeLevelRevise(POKEMON_PARAM *pp, u32 level)
{
  u32 level_exp;
  u32 monsno, form_no;

  monsno = PP_Get(pp, ID_PARA_monsno, NULL);
  form_no = PP_Get(pp, ID_PARA_form_no, NULL);

  level_exp = POKETOOL_GetMinExp( monsno, form_no, level );
  PP_Put(pp, ID_PARA_exp, level_exp);
  PP_Renew( pp );
}

//==================================================================
/**
 * ポケモンパラメータ構造体をコピー
 *
 * @param   src コピー元
 * @param   dst コピー先
 */
//==================================================================
void POKETOOL_CopyPPtoPP( POKEMON_PARAM* pp_src, POKEMON_PARAM* pp_dst )
{
  GF_ASSERT( pp_src != NULL );
  GF_ASSERT( pp_dst != NULL );
  *pp_dst = *pp_src;
}

