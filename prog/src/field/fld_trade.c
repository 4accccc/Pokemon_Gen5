//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		  fld_trade.c
 *	@brief		ゲーム内交換
 *	@author		tomoya takahashi (obata)
 *	@data		  2006.05.15 (2009.12.09 HGSSより移植)
 *
 *	このソースは field_tradeオーバーレイ領域に属しています
 *	このソースに定義された全関数は field_tradeオーバーレイ領域を明示的にロードしてからでないと
 *	利用できないので注意してください！
 *
 *	オーバーレイロード＆アンロード構文
 *
 *	#include "system/pm_overlay.h"
 *	
 *	FS_EXTERN_OVERLAY(field_trade);
 *	Overlay_Load( FS_OVERLAY_ID( field_trade ), OVERLAY_LOAD_NOT_SYNCHRONIZE );
 *	Overlay_UnloadID( FS_OVERLAY_ID( field_trade ) );
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#if 0
#include "common.h"
#include "system/arc_util.h"
#include "include/msgdata/msg.naix"
#include "include/gflib/strbuf_family.h"
#include "fieldsys.h"
#include "field_event.h"
#include "ev_mapchange.h"
#include "ev_time.h"
#include "savedata/zukanwork.h"
#include "savedata/get_poke.h"
#include "savedata/mail.h"
#include "savedata/mail_util.h"
#include "poketool/poke_memo.h"
#include "itemtool/itemsym.h"
#include "itemtool/item.h"
#include "zonedata.h"
#include "fielddata/maptable/zone_id.h"
#include "fld_trade.h"
#include "fld_trade_local.h"
#include "onlyone_poke.h"
#endif

#include <gflib.h>
#include "fld_trade_local.h"
#include "fld_trade.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "savedata/mystatus.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "arc/arc_def.h"  // for ARCID_MESSAGE, ARCID_FLD_TRADE_POKE
#include "arc/message.naix"  // for NARC_message_fld_trade_dat
#include "net_app/pokemontrade.h"  // for PokemonTradeDemoProcData
#include "event_fieldmap_control.h"  // for EVENT_FieldSubProc
#include "system/main.h"  // for HEAPID_PROC
#include "savedata/zukan_savedata.h"  // for ZUKANSAVE_xxxx
#include "../../../resource/fld_trade/fld_trade_list.h"  // for FLD_TRADE_POKE_xxxx
#include "poke_tool/shinka_check.h"
#include "demo/shinka_demo.h"

FS_EXTERN_OVERLAY(pokemon_trade);


//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
struct _FLD_TRADE_WORK 
{
	FLD_TRADE_POKEDATA* p_pokedata;		// 交換ポケモンデータ
	POKEMON_PARAM*		        p_pp;		// 交換ポケモン
	MYSTATUS*			         p_myste;		// 交換相手パラメータ
	u32                   trade_no;	  // 交換ナンバー
	u32                     heapID;
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static STRBUF* GetTradeMsgData( u32 heapID, u32 idx );

static void SetPokemonParam( POKEMON_PARAM* pp, FLD_TRADE_POKEDATA* data, 
	                           u32 trade_no, u32 zone_id, int heapID );

static void PP_Dump( const POKEMON_PARAM* pp );
static void FTP_Dump( const FLD_TRADE_POKEDATA* ftp );


//----------------------------------------------------------------------------
/**
 *	@brief	ワーク確保
 *
 *	@param	heap_id		ヒープID
 *	@param	trade_no	交換番号
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
FLD_TRADE_WORK* FLD_TRADE_WORK_Create( u32 heap_id, u32 trade_no )
{
	FLD_TRADE_WORK* work;
	STRBUF* strbuf;
	STRCODE str_arry[ 128 ];

	GF_ASSERT( trade_no < FLD_TRADE_POKE_NUM );
	
	work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(FLD_TRADE_WORK) );
	work->heapID   = heap_id;
	work->trade_no = trade_no;

	// 交換データ読み込み
	//work->p_pokedata = ArcUtil_Load( ARC_FIELD_TRADE_POKE, trade_no, FALSE, heap_id, ALLOC_TOP );
  work->p_pokedata = GFL_ARC_LoadDataAllocOfs( ARCID_FLD_TRADE_POKE, trade_no, 
                                               heap_id, 0, sizeof(FLD_TRADE_POKEDATA) );
  // 交換ポケモンデータ作成
	work->p_pp = GFL_HEAP_AllocMemory( heap_id, sizeof(POKEMON_PARAM) );

	// 親データ作成
	work->p_myste = MyStatus_AllocWork( heap_id );
	MyStatus_Init( work->p_myste );
	strbuf = GetTradeMsgData( heap_id, FLD_TRADE_GET_OYA_GMM( trade_no ) );
	GFL_STR_GetStringCode( strbuf, str_arry, 128 );
	GFL_STR_DeleteBuffer( strbuf );
	MyStatus_SetMyName( work->p_myste, str_arry );
	MyStatus_SetMySex( work->p_myste, work->p_pokedata->oya_sex );
	
	return work;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ワーク破棄
 *
 *	@param	work		
 */
//-----------------------------------------------------------------------------
void FLD_TRADE_WORK_Delete( FLD_TRADE_WORK* work )
{
	GFL_HEAP_FreeMemory( work->p_pokedata );
	GFL_HEAP_FreeMemory( work->p_pp );
	GFL_HEAP_FreeMemory( work->p_myste );
	GFL_HEAP_FreeMemory( work );
}

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	フィールド　ポケモン預かり(他人名義のポケモンを貰う)
 *
 *	@param	heapID		ヒープID
 *	@param	trade_no	交換番号
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
void FLD_KeepPokemonAdd( FIELDSYS_WORK* fsys, u8 trade_no ,u8 level,u16 zoneID)
{
	POKEPARTY* party;
	POKEMON_PARAM* pp;
	FLD_TRADE_POKEDATA* p_data;
	
	pp = PokemonParam_AllocWork( HEAPID_WORLD );
	
	// 交換データ読み込み
	p_data = ArcUtil_Load( ARC_FIELD_TRADE_POKE, trade_no, FALSE, HEAPID_WORLD, ALLOC_BOTTOM );

	//パラメータ生成
	SetPokemonParam( pp, p_data, level , trade_no, zoneID ,TRMEMO_FLDKEEP_PLACESET,HEAPID_WORLD);

	// 図鑑に登録
	SaveData_GetPokeRegister( fsys->savedata, pp );

	//手持ちの一番最後に追加
	party = SaveData_GetTemotiPokemon( fsys->savedata );
	PokeParty_Add(party, pp);

	//メールポケモンだった場合、メールデータをセット
	if(trade_no == FLD_TRADE_ONISUZUME){
		MAIL_DATA* dat;
		STRBUF* str;
		POKEMON_PARAM* tp = PokeParty_GetMemberPointer( party, PokeParty_GetPokeCount(party)-1);
		
		str = GetTradeMsgData( HEAPID_WORLD, FLD_TRADE_GET_OYA_GMM(trade_no) );

		dat = MailData_MailEventDataMake(pp,
				ItemMailDesignGet( p_data->item ),p_data->oya_sex,str,p_data->mons_id);
		PP_Put(tp,ID_PARA_mail_data,dat);

		GFL_STR_DeleteBuffer(str);
		sys_FreeMemoryEz(dat);
	}
	
	sys_FreeMemoryEz( p_data );
	sys_FreeMemoryEz( pp );
}
#endif


#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	フィールド　メールポケモンイベント専用メールデータを生成(オニスズメのメール専用)
 *
 *	@return	ワーク
 *
 *	@com	メモリをAllocして返すので、呼び出し側が解放すること！
 */
//-----------------------------------------------------------------------------
MAIL_DATA* FLD_MakeKeepPokemonEventMail( void )
{
	POKEMON_PARAM* pp;
	FLD_TRADE_POKEDATA* p_data;
	STRBUF* str;
	MAIL_DATA* dat;
	
	pp = PokemonParam_AllocWork( HEAPID_WORLD );
	
	// 交換データ読み込み
	p_data = ArcUtil_Load( ARC_FIELD_TRADE_POKE, FLD_TRADE_ONISUZUME, FALSE, HEAPID_WORLD, ALLOC_BOTTOM );

	//パラメータ生成
	SetPokemonParam( pp, p_data, 20 ,FLD_TRADE_ONISUZUME ,
		ZONE_ID_R35R0101 ,TRMEMO_FLDKEEP_PLACESET,HEAPID_WORLD);

	str = GetTradeMsgData( HEAPID_WORLD, FLD_TRADE_GET_OYA_GMM(FLD_TRADE_ONISUZUME) );

	dat = MailData_MailEventDataMake(pp,
			ItemMailDesignGet( p_data->item ),p_data->oya_sex,str,p_data->mons_id);
	
	GFL_STR_DeleteBuffer(str);
	
	sys_FreeMemoryEz( p_data );
	sys_FreeMemoryEz( pp );

	return dat;
}
#endif


#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	フィールド　預かりポケモン返却可不可チェック
 *
 *	手持ちのポケモンが預かりポケモンと一致するかや、アイテムの有無をチェック
 *
 *	@param	trade_no	交換番号
 *	@param	pos			チェックする手持ちのポケモン位置
 *
 *	@retval	KEEP_POKE_RERR_NONE		返せる
 *	@retval	KEEP_POKE_RERR_NGPOKE	違うポケモンだから返せない
 *	@retval	KEEP_POKE_RERR_ITEM		アイテムを持ってるから返せない
 *	@retval	KEEP_POKE_RERR_CBALL	カスタムボールがセットされてるから返せない
 */
//-----------------------------------------------------------------------------
u16 FLD_KeepPokemonReturnCheck( FIELDSYS_WORK* fsys, u8 trade_no ,u8 pos)
{
	POKEPARTY* party;
	POKEMON_PARAM* pp;
	
	party = SaveData_GetTemotiPokemon( fsys->savedata );
	pp = PokeParty_GetMemberPointer(party,pos);

	//預かったポケモンかどうかチェック
	if(OnlyonePoke_KeepPokeCheck(pp,trade_no) == FALSE){
		return KEEP_POKE_RERR_NGPOKE;
	}

	//ホントに返していいかどうかチェック
	
	//ボールカプセルがセットされているかどうか
	{
		u8 cb_id;

		cb_id = PokeParaGet( pp, ID_PARA_cb_id, NULL);
		if(cb_id != 0){
			return KEEP_POKE_RERR_CBALL;
		}
	}
	//闘える手持ちが２匹以上いるかどうか？
	{
		int i,max,num = 0;
		POKEMON_PARAM* pp;
		max = PokeParty_GetPokeCount(party);
		for(i = 0;i < max;i++){
			pp = PokeParty_GetMemberPointer(party,i);
			if(	PokeParaGet(pp,ID_PARA_fusei_tamago_flag,NULL) == TRUE || 
				PokeParaGet(pp,ID_PARA_hp,NULL) == 0 ||
				PokeParaGet(pp,ID_PARA_tamago_flag,NULL)){
				continue;
			}
			num++;
		}
		if(num < 2){
			return KEEP_POKE_RERR_ONLYONE;
		}
	}
	
	//アイテムを持っているかどうか？
	{
		u16 itemno;
		
		itemno = PokeParaGet( pp, ID_PARA_item, NULL );
		if(itemno != ITEM_DUMMY_DATA){
			return KEEP_POKE_RERR_ITEM;
		}
	}
	return 0;
}
#endif



//----------------------------------------------------------------------------
/**
 *	@brief	交換するポケモンナンバー取得
 *
 *	@param	cwork		ワーク
 *
 *	@return	ポケモンナンバー
 */
//-----------------------------------------------------------------------------
u32 FLD_TRADE_WORK_GetTradeMonsno( const FLD_TRADE_WORK* cwork )
{
	return cwork->p_pokedata->monsno;
}

//----------------------------------------------------------------------------
/**
 *	@brief	交換に必要なポケモンナンバー取得
 *
 *	@param	cwork		ワーク
 *
 *	@return	ポケモンナンバー
 */
//-----------------------------------------------------------------------------
u32 FLD_TRADE_WORK_GetChangeMonsno( const FLD_TRADE_WORK* cwork )
{
	return cwork->p_pokedata->change_monsno;
}

//----------------------------------------------------------------------------
/**
 *	@brief	交換に必要なポケモンの性別を取得
 *
 *	@param	cwork		ワーク
 *
 *	@return	必要なポケモンの性別（0:オス　1:メス　2:問わず（もしくは性別なし）
 *          MONS_MALE、MONS_FEMALE、MONS_UNKに準拠してます。
 */
//-----------------------------------------------------------------------------
u32 FLD_TRADE_WORK_GetChangeMonsSex( const FLD_TRADE_WORK* cwork )
{
	return cwork->p_pokedata->change_monssex;
}

//----------------------------------------------------------------------------
/**
 *	@brief	親性別　取得
 *
 *	@param	cwork		ワーク
 *
 *	@return	性別
 */
//-----------------------------------------------------------------------------
u32 FLD_TRADE_WORK_GetTradeMonsOyaSex( const FLD_TRADE_WORK* cwork )
{
	return cwork->p_pokedata->oya_sex;
}


#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	交換
 *
 *	@param	p_fsys		フィールドシステム
 *	@param	work		交換ワーク
 *	@param	party_pos	パーティーポジション
 */
//-----------------------------------------------------------------------------
void FLD_Trade( FIELDSYS_WORK * p_fsys, FLD_TRADE_WORK* work, int party_pos )
{
	POKEPARTY * party = SaveData_GetTemotiPokemon( p_fsys->savedata );

	// 交換実行
	PokeParty_SetMemberData( party, party_pos, work->p_pp );

	// 図鑑に登録
	SaveData_GetPokeRegister( p_fsys->savedata, work->p_pp );
}
#endif


#if 0
void FLD_TradeDemoDataMake( FIELDSYS_WORK * p_fsys, FLD_TRADE_WORK* work, int party_pos, DEMO_TRADE_PARAM* p_demo, POKEMON_PARAM* sendPoke, POKEMON_PARAM* recvPoke )
{
	POKEPARTY * party = SaveData_GetTemotiPokemon( p_fsys->savedata );
	POKEMON_PARAM* p_chg_poke;
	STRBUF* str;
	u32 lev;		// レベル
	int time_zone;

	// 交換するポケモン
	p_chg_poke = PokeParty_GetMemberPointer( party, party_pos );
	lev = PokeParaGet( p_chg_poke, ID_PARA_level, NULL );
	
	// 交換ポケモンの作成
	SetPokemonParam( work->p_pp, work->p_pokedata, lev, work->trade_no,
		p_fsys->location->zone_id ,TRMEMO_OFFTRADE_PLACESET,work->heapID);

	// デモ用のポケモンパラメータに格納
	PokeCopyPPtoPP( p_chg_poke, sendPoke );
	PokeCopyPPtoPP( work->p_pp, recvPoke );

	// デモデータ作成
	p_demo->sendPoke	= PPPPointerGet( sendPoke );
	p_demo->recvPoke	= PPPPointerGet( recvPoke );
	p_demo->partner	= work->p_myste;
	p_demo->seqFlag	= DEMO_TRADE_SEQ_FULL;
	p_demo->config		= SaveData_GetConfig( p_fsys->savedata );
	time_zone = EVTIME_GetTimeZone( p_fsys );
	if( (time_zone == TIMEZONE_MORNING) || 
		(time_zone == TIMEZONE_NOON) ){
		p_demo->bgType		= DEMO_TRADE_BGTYPE_MORNING;
	}else if( time_zone == TIMEZONE_EVENING ){
		p_demo->bgType		= DEMO_TRADE_BGTYPE_EVENING;
	}else{
		p_demo->bgType		= DEMO_TRADE_BGTYPE_NIGHT;
	}
}
#endif




//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ取得
 *	破棄が必要
 */
//-----------------------------------------------------------------------------
static STRBUF* GetTradeMsgData( u32 heapID, u32 str_id )
{
	STRBUF* strbuf;
	
	GFL_MSGDATA* msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
                                     ARCID_MESSAGE, NARC_message_fld_trade_dat, 
                                     heapID );
	strbuf = GFL_MSG_CreateString( msg, str_id );
  GFL_MSG_Delete( msg );
	return strbuf;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンパラムにデータの設定
 *
 *	@param	pp		    ポケモンパラム
 *	@param	data		  データ
 *	@param	trade_no	tradeナンバー
 *	@param	zoneID		トレーナーメモに記載するゾーンID
 *	@param	heapID		heapID
 */
//-----------------------------------------------------------------------------
static void SetPokemonParam( POKEMON_PARAM* pp, FLD_TRADE_POKEDATA* data, 
	                           u32 trade_no, u32 zone_id, int heapID )
{
	STRBUF* strbuf;
	u32 placeid; 
  u32 personal_rnd;

  // 指定の性別になるような個性乱数を生成
  personal_rnd = POKETOOL_CalcPersonalRandEx( data->mons_id, data->monsno, data->formno, data->sex, 0, FALSE );
	
	// モンスターナンバー　レベル　固体乱数　ID設定
	PP_SetupEx( pp, 
              data->monsno, 
              data->level, 
              data->mons_id, 
              PTL_SETUP_POW_AUTO, 
              personal_rnd );
  // フォーム
  PP_Put( pp, ID_PARA_form_no, data->formno );

  PP_Dump( pp );

	// ニックネーム
	strbuf = GetTradeMsgData( heapID, FLD_TRADE_GET_POKE_GMM(trade_no) );
	PP_Put( pp, ID_PARA_nickname, (u32)strbuf );
	GFL_STR_DeleteBuffer( strbuf );

	// 各種乱数設定
	PP_Put( pp, ID_PARA_hp_rnd,     data->hp_rnd );
	PP_Put( pp, ID_PARA_pow_rnd,    data->at_rnd );
	PP_Put( pp, ID_PARA_def_rnd,    data->df_rnd );
	PP_Put( pp, ID_PARA_agi_rnd,    data->sp_rnd );
	PP_Put( pp, ID_PARA_spepow_rnd, data->sa_rnd );
	PP_Put( pp, ID_PARA_spedef_rnd, data->sd_rnd );

	// 特性
  PP_Put( pp, ID_PARA_speabino, data->speabino );

  // 性格
  PP_Put( pp, ID_PARA_seikaku, data->seikaku );

	// かっこよさなど
	PP_Put( pp, ID_PARA_style,     data->style );
	PP_Put( pp, ID_PARA_beautiful, data->beautiful );
	PP_Put( pp, ID_PARA_cute,      data->cute );
	PP_Put( pp, ID_PARA_clever,    data->clever );
	PP_Put( pp, ID_PARA_strong,    data->strong );
	
	// アイテム
	PP_Put( pp, ID_PARA_item, data->item );

	// 親の名前
	strbuf = GetTradeMsgData( heapID, FLD_TRADE_GET_OYA_GMM(trade_no) );
	PP_Put( pp, ID_PARA_oyaname, (u32)strbuf );
	GFL_STR_DeleteBuffer( strbuf );

	// 親の性別	
	PP_Put( pp, ID_PARA_oyasex, data->oya_sex );

	// 国コード
	PP_Put( pp, ID_PARA_country_code, data->world_code );

#if 0
	// トレーナーメモ設定
	placeid = ZoneData_GetPlaceNameID( zone_id );
	TrainerMemoSetPP( pp, NULL, memo, placeid, heapID );
#endif
  // 捕まえた場所
	PP_Put( pp, ID_PARA_get_place, zone_id );
	PP_Put( pp, ID_PARA_new_get_place, zone_id );

  PP_Dump( pp );

	// 計算しなおし
	PP_Renew( pp );

	// レアにならないようにデータ設定されていなかった場合NG
  GF_ASSERT( PP_CheckRare( pp ) == FALSE && "レアポケです。");

  // 特性が種の持ち得ないものだったらNG
  {
    POKEMON_PERSONAL_DATA* ppd;
    u32 monsno, formno;
    u32 speabi;
    u32 sp1, sp2, sp3;
    monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    formno = PP_Get( pp, ID_PARA_form_no, NULL );
    speabi = PP_Get( pp, ID_PARA_speabino, NULL );
    ppd = POKE_PERSONAL_OpenHandle( monsno, formno, heapID );
    sp1 = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi1 );
    sp2 = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi2 );
    sp3 = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi3 );
    POKE_PERSONAL_CloseHandle( ppd );
    // 不正な特性を検出
    if( (speabi!=sp1)||(speabi!=sp2)||(speabi!=sp3) )
    {
      OBATA_Printf( "======================================\n" );
      OBATA_Printf( "不正な特性を検出しました。\n" );
      OBATA_Printf( "正しい特性は %d or %d or %d です。\n", sp1, sp2, sp3 );
      OBATA_Printf( "交換ポケモンの特性を %d に設定します。\n", sp1 );
      OBATA_Printf( "======================================\n" );
      PP_Put( pp, ID_PARA_speabino, sp1 );
    }
  }
} 


static void PP_Dump( const POKEMON_PARAM* pp )
{
  u32 val; 
  OBATA_Printf( "---------------------------------------------- PP\n" );
  val = PP_Get( pp, ID_PARA_personal_rnd, NULL ); OBATA_Printf( "personal_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_monsno, NULL ); OBATA_Printf( "monsno = %d\n", val );
  val = PP_Get( pp, ID_PARA_form_no, NULL ); OBATA_Printf( "form_no = %d\n", val );
  val = PP_Get( pp, ID_PARA_item, NULL ); OBATA_Printf( "item = %d\n", val );
  val = PP_Get( pp, ID_PARA_id_no, NULL ); OBATA_Printf( "id_no = %d\n", val );
  val = PP_Get( pp, ID_PARA_exp, NULL ); OBATA_Printf( "exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_friend, NULL ); OBATA_Printf( "friend = %d\n", val );
  val = PP_Get( pp, ID_PARA_speabino, NULL ); OBATA_Printf( "speabino = %d\n", val );
  val = PP_Get( pp, ID_PARA_sex, NULL ); OBATA_Printf( "sex = %d\n", val );
  val = PP_Get( pp, ID_PARA_seikaku, NULL ); OBATA_Printf( "seikaku = %d\n", val );
  val = PP_Get( pp, ID_PARA_mark, NULL ); OBATA_Printf( "mark = %d\n", val );
  val = PP_Get( pp, ID_PARA_country_code, NULL ); OBATA_Printf( "country_code = %d\n", val );
  val = PP_Get( pp, ID_PARA_hp_exp, NULL ); OBATA_Printf( "hp_exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_pow_exp, NULL ); OBATA_Printf( "pow_exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_def_exp, NULL ); OBATA_Printf( "def_exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_agi_exp, NULL ); OBATA_Printf( "agi_exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_spepow_exp, NULL ); OBATA_Printf( "spepow_exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_spedef_exp, NULL ); OBATA_Printf( "spedef_exp = %d\n", val );
  val = PP_Get( pp, ID_PARA_style, NULL ); OBATA_Printf( "style = %d\n", val );
  val = PP_Get( pp, ID_PARA_beautiful, NULL ); OBATA_Printf( "beautiful = %d\n", val );
  val = PP_Get( pp, ID_PARA_cute, NULL ); OBATA_Printf( "cute = %d\n", val );
  val = PP_Get( pp, ID_PARA_clever, NULL ); OBATA_Printf( "clever = %d\n", val );
  val = PP_Get( pp, ID_PARA_strong, NULL ); OBATA_Printf( "strong = %d\n", val );
  val = PP_Get( pp, ID_PARA_fur, NULL ); OBATA_Printf( "fur = %d\n", val );
  val = PP_Get( pp, ID_PARA_hp_rnd, NULL ); OBATA_Printf( "hp_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_pow_rnd, NULL ); OBATA_Printf( "pow_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_def_rnd, NULL ); OBATA_Printf( "def_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_agi_rnd, NULL ); OBATA_Printf( "agi_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_spepow_rnd, NULL ); OBATA_Printf( "spepow_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_spedef_rnd, NULL ); OBATA_Printf( "spedef_rnd = %d\n", val );
  val = PP_Get( pp, ID_PARA_oyasex, NULL ); OBATA_Printf( "oyasex = %d\n", val );
  val = PP_Get( pp, ID_PARA_level, NULL ); OBATA_Printf( "level = %d\n", val );
  val = PP_Get( pp, ID_PARA_get_place, NULL ); OBATA_Printf( "get_place = %d\n", val );
  val = PP_Get( pp, ID_PARA_new_get_place, NULL ); OBATA_Printf( "new_get_place = %d\n", val );
  OBATA_Printf( "-------------------------------------------------\n" );
}

static void FTP_Dump( const FLD_TRADE_POKEDATA* ftp )
{
  OBATA_Printf( "--------------------------------------------- FTP\n" );
  OBATA_Printf( "monsno = %d\n", ftp->monsno );
  OBATA_Printf( "formno = %d\n", ftp->formno );
  OBATA_Printf( "level = %d\n", ftp->level );
  OBATA_Printf( "hp_rnd = %d\n", ftp->hp_rnd );
  OBATA_Printf( "at_rnd = %d\n", ftp->at_rnd );
  OBATA_Printf( "df_rnd = %d\n", ftp->df_rnd );
  OBATA_Printf( "sp_rnd = %d\n", ftp->sp_rnd );
  OBATA_Printf( "sa_rnd = %d\n", ftp->sa_rnd );
  OBATA_Printf( "sd_rnd = %d\n", ftp->sd_rnd );
  OBATA_Printf( "speabino = %d\n", ftp->speabino );
  OBATA_Printf( "seikaku = %d\n", ftp->seikaku );
  OBATA_Printf( "sex = %d\n", ftp->sex );
  OBATA_Printf( "mons_id = %d\n", ftp->mons_id );
  OBATA_Printf( "style = %d\n", ftp->style );
  OBATA_Printf( "beautiful = %d\n", ftp->beautiful );
  OBATA_Printf( "cute = %d\n", ftp->cute );
  OBATA_Printf( "clever = %d\n", ftp->clever );
  OBATA_Printf( "strong = %d\n", ftp->strong );
  OBATA_Printf( "item = %d\n", ftp->item );
  OBATA_Printf( "oya_sex = %d\n", ftp->oya_sex );
  OBATA_Printf( "fur = %d\n", ftp->fur );
  OBATA_Printf( "world_code = %d\n", ftp->world_code );
  OBATA_Printf( "change_monsno = %d\n", ftp->change_monsno );
  OBATA_Printf( "change_monssex = %d\n", ftp->change_monssex );
  OBATA_Printf( "-------------------------------------------------\n" );
}


//========================================================================================
// ■イベント
//========================================================================================
typedef struct 
{
  GAMESYS_WORK*          gameSystem;
  GAMEDATA*                gameData;
  POKEPARTY*              pokeParty;  // 手持ちポケパーティ
  u8                        tradeNo;  // 交換データNo.
  u8                       partyPos;  // 交換に出すポケモンの手持ちインデックス
  FLD_TRADE_WORK*         tradeWork;  // 交換ワーク
  POKEMONTRADE_DEMO_PARAM tradeDemoParam;  // 交換デモ パラメータ
  SHINKA_DEMO_PARAM*      shinkaDemoParam;  // 進化デモ パラメータ

} FLD_TRADE_EVWORK;

enum{
  SEQ_INIT,         // イベント初期化
  SEQ_TRADE,        // 交換デモ
  SEQ_DATA_UPDATE,  // データ更新
  SEQ_EVOLUTION,    // 進化デモ
  SEQ_EXIT,         // イベント終了
};

//----------------------------------------------------------------------------------------
/**
 * @brief ゲーム内交換イベント処理関数
 */
//----------------------------------------------------------------------------------------
static GMEVENT_RESULT FieldPokeTradeEvent( GMEVENT* event, int* seq, void* wk )
{
  FLD_TRADE_EVWORK* work = (FLD_TRADE_EVWORK*)wk;
  GAMESYS_WORK* gameSystem = work->gameSystem;
  GAMEDATA* gameData = work->gameData;
  POKEPARTY* pokeParty = work->pokeParty;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gameSystem );

  switch( *seq )
  {
  // イベント初期化
  case SEQ_INIT:
    // 交換ワーク生成
    {
      u16 zoneID = FIELDMAP_GetZoneID( fieldmap );
       
      work->tradeWork = FLD_TRADE_WORK_Create( HEAPID_PROC, work->tradeNo );
      SetPokemonParam( work->tradeWork->p_pp, work->tradeWork->p_pokedata, 
                       work->tradeNo, zoneID, HEAPID_PROC );
    }
    // DEBUG:
    PP_Dump( work->tradeWork->p_pp );
    FTP_Dump( work->tradeWork->p_pokedata );
    *seq = SEQ_TRADE;
    break;
  // 交換デモ呼び出し
  case SEQ_TRADE:
    {
      GMEVENT* demo;
      FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gameSystem );
      work->tradeDemoParam.gamedata = gameData; 
      work->tradeDemoParam.pMy      = GAMEDATA_GetMyStatus( gameData );
      work->tradeDemoParam.pMyPoke  = PokeParty_GetMemberPointer( pokeParty, work->partyPos );
      work->tradeDemoParam.pNPC     = work->tradeWork->p_myste;
      work->tradeDemoParam.pNPCPoke = work->tradeWork->p_pp;
      demo = EVENT_FieldSubProc( gameSystem, fieldmap, 
                                  FS_OVERLAY_ID(pokemon_trade), 
                                  &PokemonTradeDemoProcData, &work->tradeDemoParam );
      GMEVENT_CallEvent( event, demo );
    }
    *seq = SEQ_EVOLUTION;
    break;
  // データ更新
  case SEQ_DATA_UPDATE:
    // 手持ちポケ上書き
    PokeParty_SetMemberData( pokeParty, work->partyPos, work->tradeWork->p_pp );
    // 図鑑登録
    {
      ZUKAN_SAVEDATA* zukan = GAMEDATA_GetZukanSave( gameData );
      ZUKANSAVE_SetPokeSee( zukan, work->tradeWork->p_pp );  // 見た
      ZUKANSAVE_SetPokeGet( zukan, work->tradeWork->p_pp );  // 捕まえた
    }
    *seq = SEQ_EXIT;
    break;
  // 進化デモ呼び出し
  case SEQ_EVOLUTION:
    {
      SHINKA_COND cond;
      HEAPID heapID = FIELDMAP_GetHeapID( fieldmap );
      POKEMON_PARAM* pokeParam = PokeParty_GetMemberPointer( pokeParty, work->partyPos );
      u16 afterMonsNo = SHINKA_Check( pokeParty, pokeParam, SHINKA_TYPE_TUUSHIN, 0, &cond, heapID );

      if( afterMonsNo )
      {
        GFL_OVERLAY_Load( FS_OVERLAY_ID(shinka_demo) );
        work->shinkaDemoParam = SHINKADEMO_AllocParam( 
            heapID, gameData, pokeParty, afterMonsNo, work->partyPos, cond, TRUE );
        GFL_PROC_SysCallProc( NO_OVERLAY_ID, &ShinkaDemoProcData, work->shinkaDemoParam );
      }
    }
    *seq = SEQ_EXIT;
    break;
  // イベント終了処理
  case SEQ_EXIT:
    // 進化デモの後始末
    if( work->shinkaDemoParam )
    {
      SHINKADEMO_FreeParam( work->shinkaDemoParam );
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(shinka_demo) );
    }
    // 交換ワーク破棄
    FLD_TRADE_WORK_Delete( work->tradeWork );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------------------
/**
 * @brief ゲーム内交換イベントを生成する
 *
 * @param gsys ゲームシステム
 * @param trade_no  交換データNo.
 * @param party_pos 交換に出すポケモンの手持ちインデックス
 *
 * @return 生成したイベント
 */
//----------------------------------------------------------------------------------------
GMEVENT* EVENT_FieldPokeTrade( GAMESYS_WORK* gsys, u8 trade_no, u8 party_pos )
{
  GMEVENT* event;
  FLD_TRADE_EVWORK* work;
  GAMEDATA* gameData;
  POKEPARTY* pokeParty;

  gameData = GAMESYSTEM_GetGameData( gsys );
  pokeParty = GAMEDATA_GetMyPokemon( gameData );

  // 生成
  event = GMEVENT_Create( gsys, NULL, FieldPokeTradeEvent, sizeof(FLD_TRADE_EVWORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gsys;
  work->gameData = gameData;
  work->pokeParty = pokeParty;
  work->tradeNo = trade_no;
  work->partyPos = party_pos; 
  work->shinkaDemoParam = NULL;
  return event;
}
