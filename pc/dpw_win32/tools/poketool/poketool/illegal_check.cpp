/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     illegal_check.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "monsno.h"
#include "illegal_check.h"
#include "dpw_assert.h"
#include "trans_pokecode.h"

#ifdef WIN32
#include <windows.h>
#include <winnls.h>
#endif

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					関数プロトタイプ宣言
 *-----------------------------------------------------------------------*/
/*
BOOL PokeTool_LoadWazaData(const char* wazaFileName, const char* wazaLvFileName);
PokeToolCheckResult PokeTool_CheckWaza(const u32 monsno, const u16* waza, const char* absWazaFilePath, const u32 level, const u32 form_no);
PokeToolCheckResult PokeTool_CheckDoryoku(const u32 paraDoryoku[POKETOOL_DORYOKU_COUNT_MAX]);
PokeToolCheckResult PokeTool_CheckKotai(const u32 paraKotai[POKETOOL_KOTAI_COUNT_MAX]);
PokeToolCheckResult PokeTool_CheckSpeabi(const u32 monsno, const u32 paraSpeabino);
PokeToolCheckResult PokeTool_CheckItem(const u32 item);
PokeToolCheckResult PokeTool_CheckFlag(const u32 monsno, const BOOL paraEventGet, const BOOL rare);
PokeToolCheckResult PokeTool_CheckCountryCode(const u32 countryCode);
PokeToolCheckResult PokeTool_CheckPokemonLv(const u32 monsno, const u32 level);
u32 PokeTool_GetPokemonMinimumLv(const u32 monsno);
*/
/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/

static BOOL initialized = FALSE;
static u8 pokemon_waza_data[POKETOOL_MONS_WAZADATA_MAX][POKETOOL_WAZA_MAX];
//static u8 pokemon_waza_lv_data[MONSNO_END][POKETOOL_WAZA_MAX];
static u8 pokemon_speabi_data[POKETOOL_MONS_WAZADATA_MAX][POKETOOL_SPEABI_COUNT_MAX];

// 配布技を二つ以上覚えているポケモン番号をここに記述する
// 2004クリスマス配布やダークライ、ヒトカゲ、リザード、リザードンなどが含まれる
static const u32 twoHaifuMonsno[] = { 4, 5, 6, 83, 96, 97, 102, 103, 108, 113, 115, 242, 491 };

// バトルタワーでのみ禁止するポケモン。bt_pokemon_ng_list.csvより。
static const u32 bt_notAllowMonsno[] = { 150, 151, 250, 249, 251, 382, 383, 384, 385, 386, 483, 484, 487, 489, 490, 491, 492, 493 };



/*-----------------------------------------------------------------------*
					グローバル関数定義
 *-----------------------------------------------------------------------*/
/*!
	基本的なポケモンデータを読み取ります。
    COMインタフェース用に作成しました。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	不正チェックするポケモンデータ。IDを与えて中身を参照しないといけない。
    @param spd  [out] 取得したポケモンデータ。

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
*/
PokeToolCheckResult PokeTool_GetSimplePokemonData(POKEMON_PARAM *pp, SimplePokemonData* spd)
{
	u32 i;

	// ID読み出し
	spd->idNo = PokeTool_GetPokeParam(pp, ID_PARA_id_no, NULL);

    // 性別
	spd->sex = PokeTool_GetPokeParam(pp, ID_PARA_sex, NULL);

	// ポケモンNo読み出し
	spd->monsno = PokeTool_GetPokeParam(pp, ID_PARA_monsno, NULL);

	// 技データ読み出し
	for(i = 0; i<WAZA_TEMOTI_MAX; i++){
		spd->waza[i] = (u16)PokeTool_GetPokeParam(pp, (PokeToolParamID)(ID_PARA_waza1+i), NULL);
	}

	// 努力値読み出し
	spd->paraDoryoku[0] = PokeTool_GetPokeParam(pp, ID_PARA_hp_exp, NULL);
	spd->paraDoryoku[1] = PokeTool_GetPokeParam(pp, ID_PARA_pow_exp, NULL);
	spd->paraDoryoku[2] = PokeTool_GetPokeParam(pp, ID_PARA_def_exp, NULL);
	spd->paraDoryoku[3] = PokeTool_GetPokeParam(pp, ID_PARA_agi_exp, NULL);
	spd->paraDoryoku[4] = PokeTool_GetPokeParam(pp, ID_PARA_spepow_exp, NULL);
	spd->paraDoryoku[5] = PokeTool_GetPokeParam(pp, ID_PARA_spedef_exp, NULL);

	// 個体値読み出し
	spd->paraKotai[0] = PokeTool_GetPokeParam(pp, ID_PARA_hp_rnd, NULL);
	spd->paraKotai[1] = PokeTool_GetPokeParam(pp, ID_PARA_pow_rnd, NULL);
	spd->paraKotai[2] = PokeTool_GetPokeParam(pp, ID_PARA_def_rnd, NULL);
	spd->paraKotai[3] = PokeTool_GetPokeParam(pp, ID_PARA_agi_rnd, NULL);
	spd->paraKotai[4] = PokeTool_GetPokeParam(pp, ID_PARA_spepow_rnd, NULL);
	spd->paraKotai[5] = PokeTool_GetPokeParam(pp, ID_PARA_spedef_rnd, NULL);
	
	// 道具No読み出し
	spd->paraItem = PokeTool_GetPokeParam(pp, ID_PARA_item, NULL);

	// 特殊能力の読み出し
	spd->paraSpeabino = PokeTool_GetPokeParam(pp, ID_PARA_speabino, NULL);

	// 配布フラグの読み出し
	spd->paraEventGet = PokeTool_GetPokeParam(pp, ID_PARA_event_get_flag, NULL);

	// 国コード読み出し
	spd->countryCode = PokeTool_GetPokeParam(pp, ID_PARA_country_code, NULL);

	// レベル読み出し
	spd->level = PokeTool_GetPokeParam(pp, ID_PARA_level, NULL);

    // 捕獲場所読み出し
    spd->place = PokeTool_GetPokeParam(pp, ID_PARA_get_place, NULL);

    spd->birthPlace = PokeTool_GetPokeParam(pp, ID_PARA_birth_place, NULL);

    // 捕獲カセット読み出し
    //cassette = PokeTool_GetPokeParam(pp, ID_PARA_get_cassette, NULL);

	// フォームNo読み出し
	if (spd->monsno == MONSNO_AUSU) {
		spd->form_no = 0;
	} else {
		spd->form_no = PokeTool_GetPokeParam(pp, ID_PARA_form_no, NULL);
	}

    // ニックネームフラグ読み出し
    spd->nicknameFlag = PokeTool_GetPokeParam(pp, ID_PARA_nickname_flag, NULL);

    // おやの性別
    spd->oyasex = PokeTool_GetPokeParam(pp, ID_PARA_oyasex, NULL);
    return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンデータが不正なものか否かを返します。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	不正チェックするポケモンデータ。IDを与えて中身を参照しないといけない。
	@param absWazaFilePath 技データ定義ファイルへのパス

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
	@retval POKETOOL_CHECKRESULT_NO_WAZA_DATA	技データ定義ファイルが見つからない
*/
PokeToolCheckResult PokeTool_CheckData(POKEMON_PARAM *pp, const char* absWazaFilePath)
{
	u16 waza[WAZA_TEMOTI_MAX];
	u32 monsno;
	//u32 level;
	u32 i;
	u32 paraDoryoku[POKETOOL_DORYOKU_COUNT_MAX];
	u32 paraKotai[POKETOOL_KOTAI_COUNT_MAX];
	u32 paraItem;
	u32 paraSpeabino;
	u32 paraEventGet;
	u32 countryCode;
	u32 level;
    u32 place;
    u32 birthPlace;
    //u32 cassette;
	u32 form_no;
    u32 classic_ribbon;
	PokeToolCheckResult result;

	// ポケモンNo読み出し
	monsno = PokeTool_GetPokeParam(pp, ID_PARA_monsno, NULL);

	// 技データ読み出し
	for(i = 0; i<WAZA_TEMOTI_MAX; i++){
		waza[i] = (u16)PokeTool_GetPokeParam(pp, (PokeToolParamID)(ID_PARA_waza1+i), NULL);
	}

	// 努力値読み出し
	paraDoryoku[0] = PokeTool_GetPokeParam(pp, ID_PARA_hp_exp, NULL);
	paraDoryoku[1] = PokeTool_GetPokeParam(pp, ID_PARA_pow_exp, NULL);
	paraDoryoku[2] = PokeTool_GetPokeParam(pp, ID_PARA_def_exp, NULL);
	paraDoryoku[3] = PokeTool_GetPokeParam(pp, ID_PARA_agi_exp, NULL);
	paraDoryoku[4] = PokeTool_GetPokeParam(pp, ID_PARA_spepow_exp, NULL);
	paraDoryoku[5] = PokeTool_GetPokeParam(pp, ID_PARA_spedef_exp, NULL);

	// 個体値読み出し
	paraKotai[0] = PokeTool_GetPokeParam(pp, ID_PARA_hp_rnd, NULL);
	paraKotai[1] = PokeTool_GetPokeParam(pp, ID_PARA_pow_rnd, NULL);
	paraKotai[2] = PokeTool_GetPokeParam(pp, ID_PARA_def_rnd, NULL);
	paraKotai[3] = PokeTool_GetPokeParam(pp, ID_PARA_agi_rnd, NULL);
	paraKotai[4] = PokeTool_GetPokeParam(pp, ID_PARA_spepow_rnd, NULL);
	paraKotai[5] = PokeTool_GetPokeParam(pp, ID_PARA_spedef_rnd, NULL);
	
	// 道具No読み出し
	paraItem = PokeTool_GetPokeParam(pp, ID_PARA_item, NULL);

	// 特殊能力の読み出し
	paraSpeabino = PokeTool_GetPokeParam(pp, ID_PARA_speabino, NULL);

	// 配布フラグの読み出し
	paraEventGet = PokeTool_GetPokeParam(pp, ID_PARA_event_get_flag, NULL);

	// 国コード読み出し
	countryCode = PokeTool_GetPokeParam(pp, ID_PARA_country_code, NULL);

	// レベル読み出し
	level = PokeTool_GetPokeParam(pp, ID_PARA_level, NULL);

    // 捕獲場所読み出し
    place = PokeTool_GetPokeParam(pp, ID_PARA_get_place, NULL);

    birthPlace = PokeTool_GetPokeParam(pp, ID_PARA_birth_place, NULL);

    // 捕獲カセット読み出し
    //cassette = PokeTool_GetPokeParam(pp, ID_PARA_get_cassette, NULL);

	// フォームNo読み出し
	if (monsno == MONSNO_AUSU) {
		form_no = 0;
	} else {
		form_no = PokeTool_GetPokeParam(pp, ID_PARA_form_no, NULL);
	}

	// 技のチェック
	result = PokeTool_CheckWaza(monsno, waza, absWazaFilePath, 0 /*level*/, form_no);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 努力値のチェック
	result = PokeTool_CheckDoryoku(paraDoryoku);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 個体値のチェック
	result = PokeTool_CheckKotai(paraKotai);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 道具Noのチェック
    result = PokeTool_CheckItem(paraItem);
    if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 特殊能力のチェック
	result = PokeTool_CheckSpeabi(monsno, paraSpeabino);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// フラグのチェック
	result = PokeTool_CheckFlag(monsno, paraEventGet, PokeTool_IsRare(pp));
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 国コードのチェック
	result = PokeTool_CheckCountryCode(countryCode);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// メール内の国コードのチェック
	if(PokeTool_HasMail(pp)){
		result = PokeTool_CheckCountryCode(PokeTool_GetMailLang(pp));
		if(result != POKETOOL_CHECKRESULT_OK){
			return result;
		}
	}

	// 通常のレベルチェック
	if(level == 0 || level >100){
		return POKETOOL_CHECKRESULT_CHEAT_DATA;
	}

	// ポケモンの最低レベルチェック
	result = PokeTool_CheckPokemonLv(monsno, level);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// ポケモンの捕獲場所チェック
    result = (PokeToolCheckResult)PokeTool_CheckCapturePlace(monsno, place, birthPlace);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}
    
/*
	// ポケモンの捕獲カセットチェック
    result = PokeTool_CheckCaptureCassette(monsno, cassette);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}
*/    
    // ダークライのときクラシックリボンがついていることを確認する
    if(monsno == 491){
        // クラシックリボンの読み出し
        classic_ribbon = PokeTool_GetPokeParam(pp, ID_PARA_sinou_classic_ribbon, NULL);
        if(classic_ribbon == 0){
            return POKETOOL_CHECKRESULT_CHEAT_DATA;
        }
    }

	return POKETOOL_CHECKRESULT_OK;
}

/*!
	バトルタワーのポケモンデータが不正なものか否かを返します。

	@param btp	不正チェックするバトルタワーのポケモンデータ
	@param absWazaFilePath 技データ定義ファイルへのパス

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
	@retval POKETOOL_CHECKRESULT_NO_WAZA_DATA	技データ定義ファイルが見つからない
*/
PokeToolCheckResult PokeTool_BtCheckData(B_TOWER_POKEMON* btp, const char* absWazaFilePath)
{
	u32 paraDoryoku[POKETOOL_DORYOKU_COUNT_MAX];
	u32 paraKotai[POKETOOL_KOTAI_COUNT_MAX];
	PokeToolCheckResult result;
    int i;

	// 努力値読み出し
	paraDoryoku[0] = btp->hp_exp;
	paraDoryoku[1] = btp->pow_exp;
	paraDoryoku[2] = btp->def_exp;
	paraDoryoku[3] = btp->agi_exp;
	paraDoryoku[4] = btp->spepow_exp;
	paraDoryoku[5] = btp->spedef_exp;

	// 個体値読み出し
	paraKotai[0] = btp->hp_rnd;
	paraKotai[1] = btp->pow_rnd;
	paraKotai[2] = btp->def_rnd;
	paraKotai[3] = btp->agi_rnd;
	paraKotai[4] = btp->spepow_rnd;
	paraKotai[5] = btp->spedef_rnd;

	// 技、ポケモンNoのチェック
	result = PokeTool_CheckWaza(btp->mons_no, btp->waza, absWazaFilePath, 0/* レベルなし */, btp->form_no);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 努力値のチェック
	result = PokeTool_CheckDoryoku(paraDoryoku);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 個体値のチェック
	result = PokeTool_CheckKotai(paraKotai);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 道具Noのチェック
    result = PokeTool_CheckItem(btp->item_no);
    if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// 特殊能力(特性)のチェック
	result = PokeTool_CheckSpeabi(btp->mons_no, btp->tokusei);
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// フラグのチェック
	result = PokeTool_CheckFlag(btp->mons_no, TRUE/* フラグなし */, PokeTool_BtIsRare(btp));
	if(result != POKETOOL_CHECKRESULT_OK){
		return result;
	}

	// BTのみ弾くポケモンをチェック
	for(i=0; i<sizeof(bt_notAllowMonsno)/sizeof(bt_notAllowMonsno[0]); i++){
		if(btp->mons_no == bt_notAllowMonsno[i]){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}
	return POKETOOL_CHECKRESULT_OK;
}

#ifdef WIN32

#define MATH_MIN(a,b) (((a) <= (b)) ? (a) : (b))

/*!
	ポケモンで使用される名前を統制します。
	全角文字は半角に、大文字は小文字に、カタカナはひらがなに変換されます。
	出力バッファは、MONS_NAME_SIZE+EOM_SIZEのものを与えてください。

	@param in	入力する名前です。MONS_NAME_SIZE以下の長さで、NULL終端されたものにしてください。
	@param out	返還後の出力先です。
*/
void PokeTool_UnifyName(const u16* in, u16* out)
{
	u16 temp_str[MONS_NAME_SIZE+EOM_SIZE];

	LCMapStringW(MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), SORT_DEFAULT),
		LCMAP_HALFWIDTH | LCMAP_HIRAGANA,
		(LPCWSTR)in, -1, (LPWSTR)temp_str, (MONS_NAME_SIZE+EOM_SIZE)*sizeof(u16));

	LCMapStringW(MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), SORT_DEFAULT),
		LCMAP_LOWERCASE,
		(LPCWSTR)temp_str, -1, (LPWSTR)out, (MONS_NAME_SIZE+EOM_SIZE)*sizeof(u16));
}

#endif

/*-----------------------------------------------------------------------*
					ローカル関数定義
 *-----------------------------------------------------------------------*/

/*!
	ポケモンの技データを読み込みます。

	@param wazaFileName		ポケモン技フラグデータバイナリファイル名
	@param wazaLvFileName	ポケモン技レベルデータバイナリファイル名

	@retval TRUE				正常に読み込み完了
	@retval FALSE				読み込み失敗
*/
BOOL PokeTool_LoadWazaData(const char* wazaFilePath, const char* wazaLvFilePath)
{
	FILE *fp;
	u8 c;
	u32 i;

	// 技フラグデータの読み込み
	fp = fopen(wazaFilePath, "rb");
	if(!fp){
		printf("cannot open file: %s\n", wazaFilePath);
		return FALSE;
	}

	for(i = 0; i<POKETOOL_MONS_WAZADATA_MAX; i++){
		if((fread(pokemon_speabi_data[i], sizeof(u8), POKETOOL_SPEABI_COUNT_MAX, fp )) != POKETOOL_SPEABI_COUNT_MAX	// ポケモンごとの特性を読み込み
		|| (fread(pokemon_waza_data[i], sizeof(u8), POKETOOL_WAZA_MAX, fp )) != POKETOOL_WAZA_MAX					// ポケモンごとの技データ読み込み
		|| (fread(&c, sizeof(u8), 1, fp )) != 1																		// 区切り文字を読む
		|| c != 0xFF){																								// 区切り文字のチェック
			fclose(fp);
			printf("invalid file format: %s\n", wazaFilePath);
			return FALSE;
		}
	}
	fclose(fp);
/*
	// 技レベルデータの読み込み
	fp = fopen(wazaLvFileName, "rb");
	if(!fp){
		printf("cannot open file: %s\n", wazaLvFilePath);
		return FALSE;
	}

	for(i = 0; i<MONSNO_END; i++){
		if((fread(pokemon_waza_lv_data[i], sizeof(u8), POKETOOL_WAZA_MAX, fp )) != POKETOOL_WAZA_MAX				// ポケモンごとの技Lvデータ読み込み
		|| (fread(&c, sizeof(u8), 1, fp )) != 1																		// 区切り文字を読む
		|| c != 0xFF){																								// 区切り文字のチェック
			fclose(fp);
			printf("invalid file format: %s\n", wazaLvFileName);
			return FALSE;
		}
	}

	fclose(fp);
*/
	return TRUE;
}

/*!
	読み込んだポケモンの技データをテキストで出力します。(デバッグ用)

	@param outFileName	出力ファイル名

	@retval TRUE				正常に出力完了
	@retval FALSE				出力失敗
*/
BOOL PokeTool_PrintWazaData(const char* outFileName)
{
	FILE *fp;
	u32 i, j;

	if(!initialized){
		printf("PokeTool_PrintWazaData: ERROR - not initialized.\n");
	}

	fp = fopen(outFileName, "w");
	if(!fp){
		printf("cannot open file: %s\n", outFileName);
		return FALSE;
	}

	for(i = 0; i<POKETOOL_MONS_WAZADATA_MAX; i++){
		for(j = 0; j<POKETOOL_WAZA_MAX; j++){
			fprintf(fp, "%u", pokemon_waza_data[i][j]);
		}
		fputs("\n", fp);
	}

	fclose(fp);
	return TRUE;
}

/*!
	ポケモンの技データが不正なものか否かを返します。

	@param monsno	不正チェックするポケモンNo
	@param waza		不正チェックするポケモンの技データ
	@param absWazaFilePath	不正チェックするポケモンの技データファイルパス
	@param level	不正チェックするポケモンのレベル
	@param form_no	不正チェックするポケモンのフォームNo

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
	@retval POKETOOL_CHECKRESULT_NO_WAZA_DATA	技データが読み込めなかった
*/
PokeToolCheckResult PokeTool_CheckWaza(const u32 monsno, const u16* waza, const char* absWazaFilePath, const u32 level, const u32 form_no)
{
	int i, j;
	u8 wazaResult[WAZA_TEMOTI_MAX];
	int wazaNoneCount = 0;
	BOOL bXmas2004Haifu = FALSE;
	u32 table_no;

	// 技データの読み込み
	if(!initialized){
		if(PokeTool_LoadWazaData(absWazaFilePath, NULL /* WAZA_LV_FILE_NAME */)){
			initialized = TRUE;
		}else{
			return POKETOOL_CHECKRESULT_NO_WAZA_DATA;
		}
	}

	// ポケモンNoのチェック
	if (monsno == 0 || monsno > MONSNO_END) {
		return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
	}

	// 特定のポケモンの場合は、参照する表のNoを変更する
	if (monsno == MONSNO_DEOKISISU) {
		switch (form_no) {
		case 0:
			table_no = monsno;
			break;
		case 1:
			table_no = 496;
			break;
		case 2:
			table_no = 497;
			break;
		case 3:
			table_no = 498;
			break;
		default:
			return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
		}
	} else if (monsno == MONSNO_MINOMESU) {
		switch (form_no) {
		case 0:
			table_no = monsno;
			break;
		case 1:
			table_no = 499;
			break;
		case 2:
			table_no = 500;
			break;
		default:
			return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
		}
	} else {
		table_no = monsno;
	}

	// 技Noのチェック
	for(i = 0; i<WAZA_TEMOTI_MAX; i++){
		if(!(waza[i] <= POKETOOL_WAZA_MAX)){
			return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
		}
	}

	// 一つも技を持っていない場合をチェック
	for(i = 0; i<WAZA_TEMOTI_MAX; i++){
		if(waza[i] == 0){
			wazaNoneCount++;
		}
	}
	if(wazaNoneCount == WAZA_TEMOTI_MAX){
		return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
	}

	// 同じ技を持っていないかチェック
	for(i = 0; i<WAZA_TEMOTI_MAX-1; i++){
		for(j = i+1; j<WAZA_TEMOTI_MAX; j++){
			if(waza[i] == waza[j]){
				if(waza[i] != 0){
					return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
				}
			}
		}
	}

	// 技0の後に技が入っていないかチェック
	for(i = WAZA_TEMOTI_MAX-1; i>=0; i--){
		if(waza[i] != 0){
			break;
		}
	}
	for(i = i-1; i>=0; i--){
		if(waza[i] == 0){
			return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
		}
	}

	// 持っている技データを参照する
	for(i=0; i<WAZA_TEMOTI_MAX; i++){
		if(waza[i] == 0){
			// 技を持っていない場合は無視するため普通の技を持っていることにする
			wazaResult[i] = POKETOOL_WAZA_KIND_NORMAL;
		}else{
			wazaResult[i] = pokemon_waza_data[table_no-1][waza[i]-1];
		}
	}

	// ドーブル → 235 だけ別扱い。「わるあがき→165」「おしゃべり→448」「シードフレア→465」「さばきのつぶて→449」以外の全ての技を覚える
	if(table_no == 235){
		// 覚えられる技かのチェック
		for(i=0; i<WAZA_TEMOTI_MAX; i++){
			if(waza[i] == 165 || waza[i] == 448 || waza[i] == 465 || waza[i] == 449){
				return POKETOOL_CHECKRESULT_CHEAT_DATA;
			}
		}
		// ドーブルはここでチェック終了
		return POKETOOL_CHECKRESULT_OK;
	}

	// 覚えられる技かのチェック
	for(i=0; i<WAZA_TEMOTI_MAX; i++){
		if(!wazaResult[i]){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}

	// 2004年クリスマス配布のみ別扱い
	// 配布技を二つ持っている場合もOK
	for(i=0; i<sizeof(twoHaifuMonsno)/sizeof(twoHaifuMonsno[0]); i++){
		if(table_no == twoHaifuMonsno[i]){
			bXmas2004Haifu = TRUE;
			break;
		}
	}

	{
		int haifuWazaCount = 0;
		int xdWazaCount = 0;
		for(i=0; i<WAZA_TEMOTI_MAX; i++){
			if(!(wazaResult[i] & POKETOOL_WAZA_KIND_NORMAL)){
				if(wazaResult[i] & POKETOOL_WAZA_KIND_HAIFU){
					haifuWazaCount++;
				}
				if(wazaResult[i] & POKETOOL_WAZA_KIND_XD){
					xdWazaCount++;
				}
			}
		}
		if(bXmas2004Haifu){
			if(haifuWazaCount > 2 || (haifuWazaCount > 0 && xdWazaCount > 0)){
				return POKETOOL_CHECKRESULT_CHEAT_DATA;
			}
		}else{
			if(haifuWazaCount > 1 || (haifuWazaCount > 0 && xdWazaCount > 0)){
				return POKETOOL_CHECKRESULT_CHEAT_DATA;
			}
		}
	}
/*
	// 技レベルチェック
	for(i=0; i<WAZA_TEMOTI_MAX; i++){
		if(wazaResult[i] & POKETOOL_WAZA_KIND_NORMAL){
			continue;
		}
		if(level < pokemon_waza_lv_data[table_no-1][waza[i]-1]){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}
*/
	return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンの努力値が不正なものか否かを返します。
	6つの合計で510、一つの能力値値は0～255

	@param paraDoryoku		不正チェックするポケモンの努力値

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckDoryoku(const u32 paraDoryoku[POKETOOL_DORYOKU_COUNT_MAX])
{
	int i;
	u32 sumDoryoku = 0;
	for(i = 0; i<POKETOOL_DORYOKU_COUNT_MAX; i++){
		sumDoryoku += paraDoryoku[i];
		if(paraDoryoku[i] > 255){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}

	if(sumDoryoku > 510){
		return POKETOOL_CHECKRESULT_CHEAT_DATA;
	}

	return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンの個体値が不正なものか否かを返します。
	一つの能力値は0～0x1f

	@param paraKotai		不正チェックするポケモンの個体値

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckKotai(const u32 paraKotai[POKETOOL_KOTAI_COUNT_MAX])
{
	int i;
	for(i = 0; i<POKETOOL_KOTAI_COUNT_MAX; i++){
		if(paraKotai[i] > 0x1f){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}

	return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンの特性が不正なものか否かを返します。

	@param monsno	不正チェックするポケモンNo
	@param paraSpeabino		不正チェックするポケモンの特性No

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckSpeabi(const u32 monsno, const u32 paraSpeabino)
{
	int i;

	// 特性を持っていない場合をチェック
	if (paraSpeabino == 0) {
		return POKETOOL_CHECKRESULT_ILLEGAL_DATA;
	}

	// 不正な特性でないかチェック
	for(i = 0; i<POKETOOL_SPEABI_COUNT_MAX ; i++){
		if(pokemon_speabi_data[monsno-1][i] == paraSpeabino){
			return POKETOOL_CHECKRESULT_OK;
		}
	}

	return POKETOOL_CHECKRESULT_CHEAT_DATA;
}

/*!
	アイテムが不正なものか否かを返します。

	@param item

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckItem(const u32 item)
{
	// 道具Noのチェック
	if(item > POKETOOL_MAX_ALLOWED_ITEMNO
		|| (item >= POKETOOL_ITEMNO_RESERVED_START && item <= POKETOOL_ITEMNO_RESERVED_END)
        || item == 16   // プレシャスボール
        || item == 209  // ミクルのみ
        || item == 210  // イバンのみ
        || item == 211  // ジャボのみ
        || item == 212) // レンプのみ
	{
		return POKETOOL_CHECKRESULT_CHEAT_DATA;
	}

	return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンに不正なフラグが立っているか否かを返します。

	@param monsno		不正チェックするポケモンNo
	@param paraEventGet	不正チェックするポケモンの配布フラグ
	@param rare			不正チェックするポケモンのレアフラグ

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckFlag(const u32 monsno, const BOOL paraEventGet, const BOOL rare)
{
	// セレビィ → 251
	// ジラーチ → 385
	// マナフィ → 490
	// レアフラグチェック
	if(monsno == 251 || monsno == 385 || monsno == 490){
		if(rare){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}

	// ミュウ → 151
	// デオキシス → 386
	// マナフィ → 490
	// 配布フラグチェック
	if(monsno == 151 || monsno == 386 || monsno == 490){
		if(!paraEventGet){
			return POKETOOL_CHECKRESULT_CHEAT_DATA;
		}
	}

	return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンの国コードをチェックします。

	@param countryCode		不正チェックするポケモンの国コード

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckCountryCode(const u32 countryCode)
{
	if(countryCode == 0 || countryCode == 6 || countryCode >= 9){
		return POKETOOL_CHECKRESULT_CHEAT_DATA;
	}

	return POKETOOL_CHECKRESULT_OK;
}

/*!
	伝説系ポケモンの最低レベルをチェックします。

	@param monsno		不正チェックするポケモンNo
	@param level		不正チェックするポケモンのレベル

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_ILLEGAL_DATA	与えられたポケモンデータが壊れている（文字のエンコードができなかったときなど）
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
PokeToolCheckResult PokeTool_CheckPokemonLv(const u32 monsno, const u32 level)
{
	if(level < PokeTool_GetPokemonMinimumLv(monsno)){
		return POKETOOL_CHECKRESULT_CHEAT_DATA;
	}
	return POKETOOL_CHECKRESULT_OK;
}

/*!
	ポケモンの最低レベルを取得します。

	@param monsno		不正チェックするポケモンNo

	@retval 指定されたポケモンNoの最低レベル。伝説系以外のNoが与えられた場合は0。
*/
u32 PokeTool_GetPokemonMinimumLv(const u32 monsno)
{
	switch(monsno){
		case 1:
  			return 1;
  		case 2:
  			return 16;
  		case 3:
  			return 32;
  		case 4:
  			return 1;
  		case 5:
  			return 16;
  		case 6:
  			return 36;
  		case 7:
  			return 1;
  		case 8:
  			return 16;
  		case 9:
  			return 36;
  		case 25:
  			return 2;
  		case 26:
  			return 2;
  		case 65:
  			return 16;
  		case 68:
  			return 28;
  		case 94:
  			return 16;
  		case 130:
  			return 5;
  		case 139:
  			return 40;
  		case 141:
  			return 40;
  		case 144:
  			return 50;
  		case 145:
  			return 50;
  		case 146:
  			return 50;
  		case 149:
  			return 55;
  		case 150:
  			return 70;
  		case 151:
  			return 10;
  		case 152:
  			return 1;
  		case 153:
  			return 16;
  		case 154:
  			return 32;
  		case 155:
  			return 1;
  		case 156:
  			return 14;
  		case 157:
  			return 36;
  		case 158:
  			return 1;
  		case 159:
  			return 18;
  		case 160:
  			return 30;
  		case 217:
  			return 30;
  		case 230:
  			return 25;
  		case 243:
  			return 40;
  		case 244:
  			return 40;
  		case 245:
  			return 40;
  		case 248:
  			return 55;
  		case 249:
  			return 50;
  		case 250:
  			return 70;
  		case 251:
  			return 10;
  		case 252:
  			return 1;
  		case 253:
  			return 16;
  		case 254:
  			return 36;
  		case 255:
  			return 1;
  		case 256:
  			return 16;
  		case 257:
  			return 36;
  		case 258:
  			return 1;
  		case 259:
  			return 16;
  		case 260:
  			return 36;
  		case 272:
  			return 14;
  		case 275:
  			return 14;
  		case 282:
  			return 30;
  		case 286:
  			return 23;
  		case 287:
  			return 1;
  		case 288:
  			return 18;
  		case 289:
  			return 36;
  		case 290:
  			return 1;
  		case 291:
  			return 20;
  		case 292:
  			return 20;
  		case 323:
  			return 33;
  		case 330:
  			return 45;
  		case 346:
  			return 40;
  		case 348:
  			return 40;
  		case 354:
  			return 27;
  		case 356:
  			return 37;
  		case 362:
  			return 42;
  		case 365:
  			return 44;
  		case 373:
  			return 50;
  		case 376:
  			return 45;
  		case 377:
  			return 40;
  		case 378:
  			return 40;
  		case 379:
  			return 40;
  		case 380:
  			return 40;
  		case 381:
  			return 40;
  		case 382:
  			return 45;
  		case 383:
  			return 45;
  		case 384:
  			return 70;
  		case 385:
  			return 5;
  		case 386:
  			return 30;
  		case 387:
  			return 1;
  		case 388:
  			return 18;
  		case 389:
  			return 32;
  		case 390:
  			return 1;
  		case 391:
  			return 14;
  		case 392:
  			return 36;
  		case 393:
  			return 1;
  		case 394:
  			return 16;
  		case 395:
  			return 36;
  		case 398:
  			return 34;
  		case 405:
  			return 30;
  		case 409:
  			return 30;
  		case 411:
  			return 30;
  		case 437:
  			return 33;
  		case 445:
  			return 48;
  		case 450:
  			return 34;
  		case 452:
  			return 22;
  		case 460:
  			return 40;
  		case 462:
  			return 27;
  		case 464:
  			return 42;
  		case 466:
  			return 10;
  		case 467:
  			return 10;
  		case 473:
  			return 34;
  		case 475:
  			return 6;
  		case 477:
  			return 37;
  		case 479:
  			return 1;
  		case 480:
  			return 50;
  		case 481:
  			return 50;
  		case 482:
  			return 50;
  		case 483:
  			return 47;
  		case 484:
  			return 47;
  		case 485:
  			return 70;
  		case 486:
  			return 70;
  		case 487:
  			return 70;
  		case 488:
  			return 50;
  		case 489:
  			return 1;
  		case 490:
  			return 1;
  		case 491:
  			return 50;
  		case 492:
  			return 30;
  		case 493:
  			return 80;
  		default:
  			return 0;
  	}
}

/*!
	ポケモンの捕獲場所をチェックします。

    なぜか変数名が逆で、DPのデバッグモードもバグっている。
    デバッグモードで配布したポケモンがすでにあるのでplaceとbirthPlaceのいずれかが捕まえた場所ならよいこととする

	@param monsno		不正チェックするポケモンNo
    @param place        捕獲場所(実際は生まれた場所が入る)
    @param birthPlace   生まれた場所(実際は捕獲場所が入る)

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
u32 PokeTool_CheckCapturePlace(const u32 monsno, u32 place, u32 birthPlace)
{
    u32 defaultPlace = PokeTool_GetCapturePlace(monsno);
    if(defaultPlace == 0 || defaultPlace == place || defaultPlace == birthPlace){
        return POKETOOL_CHECKRESULT_OK;
    }else{
        return POKETOOL_CHECKRESULT_CHEAT_DATA;
    }
}

/*!
	ポケモンの適切な捕獲場所を取得します。

	@param monsno		取得するポケモンNo

	@retval 指定されたポケモンNoの適切な捕獲場所。
	@retval 0 捕獲場所の判定をしない。
*/
u32 PokeTool_GetCapturePlace(u32 monsno)
{
	switch(monsno){
		case 144:
  			return 55;
  		case 145:
  			return 55;
  		case 146:
  			return 55;
  		case 150:
  			return 55;
  		case 243:
  			return 55;
  		case 244:
  			return 55;
  		case 245:
  			return 55;
  		case 249:
  			return 55;
  		case 250:
  			return 55;
  		case 251:
  			return 55;
  		case 377:
  			return 55;
  		case 378:
  			return 55;
  		case 379:
  			return 55;
  		case 380:
  			return 55;
  		case 381:
  			return 55;
  		case 382:
  			return 55;
  		case 383:
  			return 55;
  		case 384:
  			return 55;
  		case 480:
  			return 89;
  		case 482:
  			return 88;
  		case 483:
  			return 51;
  		case 484:
  			return 51;
  		case 485:
  			return 84;
  		case 486:
  			return 64;
  		case 487:
  			return 62;
  		case 491:
  			return 3005;
  		default:
  			return 0;
  	}
}

/*!
	ポケモンの捕獲カセットをチェックします。

	@param monsno		不正チェックするポケモンNo

	@retval POKETOOL_CHECKRESULT_OK				与えられたポケモンデータは正常
	@retval POKETOOL_CHECKRESULT_CHEAT_DATA		与えられたポケモンデータは不正
*/
u32 PokeTool_CheckCaptureCassette(const u32 monsno, u32 cassette)
{
    u32 defaultCassette = PokeTool_GetCaptureCassette(monsno);
    if(defaultCassette & cassette){
        return POKETOOL_CHECKRESULT_OK;
    }else{
        return POKETOOL_CHECKRESULT_CHEAT_DATA;
    }
}

/*!
	ポケモンの適切な捕獲カセットを取得します。

	@param monsno		取得するポケモンNo

	@retval 指定されたポケモンNoの適切な捕獲カセットのビットフラグ。
    @retval 0xffffffff 任意の場所でOK。
*/
u32 PokeTool_GetCaptureCassette(u32 monsno)
{
	switch(monsno){
		case 144:
  			return 16415;
  		case 145:
  			return 16415;
  		case 146:
  			return 16415;
  		case 150:
  			return 24;
  		case 151:
  			return 31;
  		case 243:
  			return 16415;
  		case 244:
  			return 16415;
  		case 245:
  			return 16415;
  		case 249:
  			return 16412;
  		case 250:
  			return 16412;
  		case 251:
  			return 16415;
  		case 377:
  			return 7;
  		case 378:
  			return 7;
  		case 379:
  			return 7;
  		case 380:
  			return 31;
  		case 381:
  			return 31;
  		case 382:
  			return 7;
  		case 383:
  			return 7;
  		case 384:
  			return 7;
  		case 385:
  			return 31;
  		case 386:
  			return 31;
  		default:
  			return 0xffffffff;
  	}
}