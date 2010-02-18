//=============================================================================
/**
 * @file	  nhttp_rap_evilcheck.h
 * @brief	  nhttpで不正チェックするときのレスポンスコードなど
 * @author	Toru=Nagihashi
 * @date    2010.02.18
 *
 */
//=============================================================================

#pragma once

//=============================================================================
/**
 *    定数
 */
//=============================================================================
//-------------------------------------
///	レスポンスコード
//    下記以外の値だとサーバエラー
//=====================================
#define NHTTP_RAP_EVILCHECK_RESPONSE_CODE_SUCCES  200         //不正チェック成功
#define NHTTP_RAP_EVILCHECK_RESPONSE_CODE_DIRTY_GAMEMODE  400 //ゲームモードが不正
#define NHTTP_RAP_EVILCHECK_RESPONSE_CODE_DIRTY_TOKEN  401    //認証トークンが不正  

//-------------------------------------
///	署名の長さ
//=====================================
#define NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN 128

//-------------------------------------
///	不正チェックの結果
//=====================================
typedef enum
{ 
	NHTTP_RAP_EVILCHECK_RESULT_OK	= 0,	//OK	バトルビデオの場合は、タマゴフラグがオンの場合は無条件で、OK
	NHTTP_RAP_EVILCHECK_RESULT_ON_EVIL_FLAG	= 10,	//不正フラグオン	
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_MONSNO	= 20,	//monsnoが範囲外	
	NHTTP_RAP_EVILCHECK_RESULT_LOCK_POKE	= 21,	//未解禁ポケモン	
	NHTTP_RAP_EVILCHECK_RESULT_RARE	= 22,	//色違いポケモンチェック	色違いが居るはずがないのに、色違い
	NHTTP_RAP_EVILCHECK_RESULT_UNAPPER_SUBWAY	= 23,	//サブウェイ出場禁止ポケモン	サブウェイ限定
	NHTTP_RAP_EVILCHECK_RESULT_RIBBON	= 24,	//外部リボン限定ポケモン	GTS限定。外部リボンのポケモンしか存在しないのに、GTSへアップされている
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_LV	= 30,	//レベル範囲外	1～100の範囲を超えている
	NHTTP_RAP_EVILCHECK_RESULT_LOW_LV	= 31,	//最低レベルチェック	
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_FORMNO	= 32,	//ポケモンのフォルム番号が範囲を超えている	
	NHTTP_RAP_EVILCHECK_RESULT_NOT_PRESENT_FLAG	= 33,	//配布限定ポケモンなのに、配布フラグがない	
	NHTTP_RAP_EVILCHECK_RESULT_EMPTY_NICKNAME	= 40,	//ニックネームが空欄	
	NHTTP_RAP_EVILCHECK_RESULT_UNDEF_NICKNAME	= 41,	//ニックネームに未定義の文字が使われている	
	NHTTP_RAP_EVILCHECK_RESULT_NOEOM_NICKNAME	= 42,	//ニックネームにEOMがない	
	NHTTP_RAP_EVILCHECK_RESULT_EMPTY_OYANAME	= 45,	//親名が空欄	
	NHTTP_RAP_EVILCHECK_RESULT_UNDEF_OYANAME	= 46,	//親名に未定義の文字が使われている	
	NHTTP_RAP_EVILCHECK_RESULT_NOEOM_OYANAME	= 47,	//親名にEOMがない	
	NHTTP_RAP_EVILCHECK_RESULT_ERR_CATCH_BALL	= 50,	//捕まえたボールエラー	こちらは、後から使えるボールが変更になる可能性があるか？
	NHTTP_RAP_EVILCHECK_RESULT_ERR_INIT_BALL	= 51,	//初期ポケモンのモンスターボールチェック	初期３体（チコ・ヒノ・ワニを除く）のボールがモンスターボール以外ならNG
	NHTTP_RAP_EVILCHECK_RESULT_ERR_EGG_BALL	= 52,	//タマゴポケのモンスターボールチェック	タマゴから孵ったポケモンが「モンスターボール」以外のボールに入っている
	NHTTP_RAP_EVILCHECK_RESULT_ERR_TOKUSEI	= 60,	//特性が不正	
	NHTTP_RAP_EVILCHECK_RESULT_ERR_ITEM	= 61,	//所持アイテムが不正	
	NHTTP_RAP_EVILCHECK_RESULT_ZERO_COUNTRY	= 70,	//ポケモンの国コードとが0が指定ならNG	
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_DORYOKUTI	= 80,	//努力値がパラメータ外	+n:0→HP　1→こうげき 2→ぼうぎょ　3→すばやさ　4→とくこう　5→とくぼう
	NHTTP_RAP_EVILCHECK_RESULT_OVER_DORYOKUTI	= 90,	//努力値の合計が510オーバー	
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_POWRND_S	= 100,	//パワー乱数が範囲外	+n:0→HP　1→こうげき 2→ぼうぎょ　3→すばやさ　4→とくこう　5→とくぼう
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_POWRND_E	= 105,    //↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_ERR_CALC_POWRND_S	= 110,	//能力値の計算値はあわない	+n:0→HP　1→こうげき 2→ぼうぎょ　3→すばやさ　4→とくこう　5→とくぼう
	NHTTP_RAP_EVILCHECK_RESULT_ERR_CALC_POWRND_E	= 115,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_WAZA_S	= 120,	//技の番号が範囲を超えている	+nは何個目の技がひっかかったか
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_WAZA_E	= 123,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_OVERLAP_WAZA_S	= 130,	//同じ技を二つ持っている	      +nは何個目の技がひっかかったか
	NHTTP_RAP_EVILCHECK_RESULT_OVERLAP_WAZA_E	= 133,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_EMPTY_SLOT1_WAZA	= 140,	//１スロット目に技がセットされていない	
	NHTTP_RAP_EVILCHECK_RESULT_BETWEEN_WAZA = 141,	//覚えていないわざが途中にある	
	NHTTP_RAP_EVILCHECK_RESULT_HAVE_CANT_WAZA_S	= 150,	//覚えられない技を覚えている	                +nは何個目の技がひっかかったか
	NHTTP_RAP_EVILCHECK_RESULT_HAVE_CANT_WAZA_E	= 153,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_NOT_PALPOKE_HAVE_AGB_WAZA_S	= 160,	//AGB技をパルパーク以外が覚えている	          +nは何個目の技がひっかかったか
	NHTTP_RAP_EVILCHECK_RESULT_NOT_PALPOKE_HAVE_AGB_WAZA_E	= 163,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_NOT_PALPOKE_HAVE_XD_WAZA_S	= 170,	//XD技をパルパーク以外が覚えている	          +nは何個目の技がひっかかったか
	NHTTP_RAP_EVILCHECK_RESULT_NOT_PALPOKE_HAVE_XD_WAZA_E	= 173,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_NOT_EGGPOKE_HAVE_EGG_WAZA_S	= 180,	//タマゴ技をタマゴから生まれたポケモン以外が  +nは何個目の技がひっかかったか
	NHTTP_RAP_EVILCHECK_RESULT_NOT_EGGPOKE_HAVE_EGG_WAZA_E	= 183,	//↑～この値までがエラー値
	NHTTP_RAP_EVILCHECK_RESULT_OVERLAP_AGB_XD_WAZA	= 190,	//XD技とタマゴ技の重複	
	NHTTP_RAP_EVILCHECK_RESULT_OVERLAP_PRESENT_EGG_WAZA	= 191,	//配布技とタマゴ技の重複	
	NHTTP_RAP_EVILCHECK_RESULT_OUTRANGE_SEIKAKU	= 200,	//「せいかく」が範囲外	
	NHTTP_RAP_EVILCHECK_RESULT_ERR_SEX	= 201,	//性別が、ありえない値	
	NHTTP_RAP_EVILCHECK_RESULT_ERR_PLACE	= 210,	//捕まえた場所／生まれた場所が不正	
	NHTTP_RAP_EVILCHECK_RESULT_ERR_PALPOKE	= 211,	//パルパーク限定ポケモンの捕まえた場所がパルパークでない	
	NHTTP_RAP_EVILCHECK_RESULT_ERR_NICKNAME	= 250,	//ニックネームがNG	エラーにするかは、モードによる
	NHTTP_RAP_EVILCHECK_RESULT_ERR_OYANAME	= 251,	//親名がNG	エラーにするかは、モードによる
	NHTTP_RAP_EVILCHECK_RESULT_ERR_NICK_OYA_NAME	= 252,	//ニックネーム・親名共にNG	エラーにするかは、モードによる
} NHTTP_RAP_EVILCHECK_RESULT;


//=============================================================================
/**
 *  外部公開関数
 */
//=============================================================================
//=============================================================================
/**
 *  不正検査で返ってきたデータの中身を取得する
 */
//=============================================================================
//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査 ステータスコードを取得
 * @param   cp_data  NHTTP_RAP_GetRecvBufferの中身
 *
 * @retval  ステータスコード  ０ならば成功  １ならば失敗
 */
//------------------------------------------------------------------------------
extern u8 NHTTP_RAP_EVILCHECK_GetStatusCode( const void *cp_data );

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査 ポケモン不正検査の結果
 * @param   cp_data     NHTTP_RAP_GetRecvBufferの中身
 * @param   poke_index  不正チェックのときに送ったポケモンの位置
 *
 * @retval  不正検査の結果  （NHTTP_RAP_EVILCHECK_RESULTの項目参照）
 */
//------------------------------------------------------------------------------
extern NHTTP_RAP_EVILCHECK_RESULT NHTTP_RAP_EVILCHECK_GetPokeResult( const void *cp_data, int poke_index );

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン不正検査  署名を取得
 *
 *  @param    cp_data     NHTTP_RAP_GetRecvBufferの中身
 *  @param    poke_max    不正チェックのときに送ったポケモンの最大数
 *
 *	@return 署名へのアドレス  コピーする場合、ここからNHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN分、memcpyしてください
 */
//-----------------------------------------------------------------------------
extern const s8 * NHTTP_RAP_EVILCHECK_GetSign( const void *cp_data, int poke_max );

//=============================================================================
/**
 *  署名とポケモンの証明をする関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン不正検査  署名とポケモンが一致していることを証明する
 *
 *	@param	const void *p_poke_buff ポケモンデータが格納されているバッファ  (以下の３つの関数を使ってください)
 *	@param	poke_size               不正チェックの時に送った１体あたりのサイズ
 *	@param	poke_max                不正チェックの時に送ったポケモンの最大数
 *	@param	s8 *cp_sign             不正チェックで受け取ったサイン
 *
 *	@return TRUE一致している  FALSE一致していない
 */
//-----------------------------------------------------------------------------
extern BOOL NHTTP_RAP_EVILCHECK_VerifySign( const void *p_poke_buff, int poke_size, int poke_max, const s8 *cp_sign );

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明のためのバッファを作成する
 *	        NHTTP_RAP_PokemonEvilCheckCreateは送信バッファの作成ですが、
 *	        こちらは、証明のためにPOKEPARTYからNHTTP_RAP_EVILCHECK_VerifySignで使用するバッファを
 *	        作成するために使います
 *
 *	@param	poke_size               不正チェックの時に送った１体あたりのサイズ
 *	@param	poke_max                不正チェックの時に送ったポケモンの最大数
 *	@param	HEAPID heapID バッファ作成のためのヒープID
 *
 *	@return バッファ
 */
//-----------------------------------------------------------------------------
extern void * NHTTP_RAP_EVILCHECK_CreateVerifyPokeBuffer( int poke_size, int poke_max, HEAPID heapID );

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明のためのバッファを破棄する
 *
 *	@param	void *p_buff バッファ
 */
//-----------------------------------------------------------------------------
extern void NHTTP_RAP_EVILCHECK_DeleteVerifyPokeBuffer( void *p_buff );

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明バッファへポケモンデータを追加する
 *
 *	@param	void *p_buff            バッファ
 *	@param	void *cp_data           ポケモンデータ
 *	@param	poke_size               不正チェックの時に送った１体あたりのサイズ
 *	@param	poke_index              格納する順番インデックス
 */
//-----------------------------------------------------------------------------
extern void NHTTP_RAP_EVILCHECK_AddPokeVerifyPokeBuffer( void *p_buff, const void *cp_data, int poke_size, int poke_index );
