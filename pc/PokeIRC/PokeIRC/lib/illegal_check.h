/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     illegal_check.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*! @file
	@brief	ポケモンデータの不正チェックを行うライブラリ
	
	@author	
	
	@version 0.03 (2006/09/13)
		@li 新規追加しました。
*/

#ifndef ILLEGAL_CHECK_H_
#define ILLEGAL_CHECK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "poketool.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/

#define POKETOOL_WAZA_KIND_NONE			0x00
#define POKETOOL_WAZA_KIND_NORMAL		0x01
#define POKETOOL_WAZA_KIND_XD			0x02
#define POKETOOL_WAZA_KIND_HAIFU		0x04
#define POKETOOL_ITEMNO_RESERVED_START	112
#define POKETOOL_ITEMNO_RESERVED_END	134

#define POKETOOL_WAZA_MAX				467
#define POKETOOL_MONS_WAZADATA_MAX		500
#define POKETOOL_DORYOKU_COUNT_MAX		6
#define POKETOOL_KOTAI_COUNT_MAX		6
#define POKETOOL_SPEABI_COUNT_MAX		2
#define POKETOOL_MAX_ALLOWED_ITEMNO		419

typedef enum {
	POKETOOL_CHECKRESULT_OK,
	POKETOOL_CHECKRESULT_ILLEGAL_DATA,
	POKETOOL_CHECKRESULT_CHEAT_DATA,
	POKETOOL_CHECKRESULT_NO_WAZA_DATA
} PokeToolCheckResult;

typedef enum {
    POKETOOL_HANGEULCHECK_NONE           = 0x0,
    POKETOOL_HANGEULCHECK_POKEMON_NAME   = 0x1,
    POKETOOL_HANGEULCHECK_PARENT_NAME    = 0x2,
    POKETOOL_HANGEULCHECK_MAIL_NAME      = 0x8
} PokeToolHangeulCheck;

// COMインタフェース用構造体
typedef struct {
    u32 idNo;   // 捕まえたときのトレーナーID
    u32 sex;
	u16 waza[WAZA_TEMOTI_MAX];
	u32 monsno;
	u32 paraDoryoku[POKETOOL_DORYOKU_COUNT_MAX];
	u32 paraKotai[POKETOOL_KOTAI_COUNT_MAX];
	u32 paraItem;
	u32 paraSpeabino;
	u32 paraEventGet;
	u32 countryCode;
	u32 level;
    u32 place;
    u32 birthPlace;
	u32 form_no;
    u32 nicknameFlag;
    u32 oyasex;
}SimplePokemonData;

/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					関数外部宣言
 *-----------------------------------------------------------------------*/

PokeToolCheckResult PokeTool_GetSimplePokemonData(POKEMON_PARAM *pp, SimplePokemonData* spd);
PokeToolCheckResult PokeTool_CheckData(POKEMON_PARAM *pp, const char* absPath);
PokeToolCheckResult PokeTool_BtCheckData(B_TOWER_POKEMON* btp, const char* absPath);
BOOL PokeTool_PrintWazaData(const char* outFileName);
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
u32 PokeTool_CheckCapturePlace(const u32 monsno, u32 place, u32 birthPlace);
u32 PokeTool_GetCapturePlace(u32 monsno);
u32 PokeTool_CheckCaptureCassette(const u32 monsno, u32 cassette);
u32 PokeTool_GetCaptureCassette(u32 monsno);


#ifdef WIN32
void PokeTool_UnifyName(const u16* in, u16* out);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // ILLEGAL_CHECK_H_