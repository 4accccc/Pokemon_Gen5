/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     base64codec.c

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
#include "poketool.h"
#include "dpw_assert.h"
#include "monsno.h"
#include "trans_pokecode.h"
#include "tokusyu_def.h"
#include "itemsym.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/

#define	NELEMS(array)	( sizeof(array) / sizeof(array[0]) )

#define	ARC_HEAD_SIZE_POS	(12)
#define	FAT_HEAD_SIZE		(12)
#define	IMG_HEAD_SIZE		(8)
#define	SIZE_OFFSET			(4)

/*======== ポケモンのソースからコピーしてきた、パラメータ取得のための型 ========*/

#define	OFS_NO_SET		(0)		// ArchiveDataLoadOfs,ArchiveDataLoadOfs用OFS値なしの定数
#define	SIZE_NO_SET		(0)		// ArchiveDataLoadOfs,ArchiveDataLoadOfs用SIZE値なしの定数

// ポケモンパーソナルデータの構造体
typedef struct pokemon_personal_data POKEMON_PERSONAL_DATA;

typedef struct pokemon_para_data11 POKEMON_PARA_DATA11;
typedef struct pokemon_para_data12 POKEMON_PARA_DATA12;
typedef struct pokemon_para_data13 POKEMON_PARA_DATA13;
typedef struct pokemon_para_data14 POKEMON_PARA_DATA14;
typedef struct pokemon_para_data15 POKEMON_PARA_DATA15;
typedef struct pokemon_para_data16 POKEMON_PARA_DATA16;
typedef struct pokemon_para_data21 POKEMON_PARA_DATA21;
typedef struct pokemon_para_data22 POKEMON_PARA_DATA22;
typedef struct pokemon_para_data23 POKEMON_PARA_DATA23;
typedef struct pokemon_para_data24 POKEMON_PARA_DATA24;
typedef struct pokemon_para_data25 POKEMON_PARA_DATA25;
typedef struct pokemon_para_data26 POKEMON_PARA_DATA26;
typedef struct pokemon_para_data31 POKEMON_PARA_DATA31;
typedef struct pokemon_para_data32 POKEMON_PARA_DATA32;
typedef struct pokemon_para_data33 POKEMON_PARA_DATA33;
typedef struct pokemon_para_data34 POKEMON_PARA_DATA34;
typedef struct pokemon_para_data35 POKEMON_PARA_DATA35;
typedef struct pokemon_para_data36 POKEMON_PARA_DATA36;
typedef struct pokemon_para_data41 POKEMON_PARA_DATA41;
typedef struct pokemon_para_data42 POKEMON_PARA_DATA42;
typedef struct pokemon_para_data43 POKEMON_PARA_DATA43;
typedef struct pokemon_para_data44 POKEMON_PARA_DATA44;
typedef struct pokemon_para_data45 POKEMON_PARA_DATA45;
typedef struct pokemon_para_data46 POKEMON_PARA_DATA46;

enum {
	ID_POKEPARA1 = 0,
	ID_POKEPARA2,
	ID_POKEPARA3,
	ID_POKEPARA4,
};

// ポケモンパーソナルデータの構造体宣言
struct	pokemon_personal_data
{
	u8		basic_hp;			//基本ＨＰ
	u8		basic_pow;			//基本攻撃力
	u8		basic_def;			//基本防御力
	u8		basic_agi;			//基本素早さ

	u8		basic_spepow;		//基本特殊攻撃力
	u8		basic_spedef;		//基本特殊防御力
	u8		type1;				//属性１
	u8		type2;				//属性２

	u8		get_rate;			//捕獲率
	u8		give_exp;			//贈与経験値

	u16		pains_hp	:2;		//贈与努力値ＨＰ
	u16		pains_pow	:2;		//贈与努力値攻撃力
	u16		pains_def	:2;		//贈与努力値防御力
	u16		pains_agi	:2;		//贈与努力値素早さ
	u16		pains_spepow:2;		//贈与努力値特殊攻撃力
	u16		pains_spedef:2;		//贈与努力値特殊防御力
	u16					:4;		//贈与努力値予備

	u16		item1;				//アイテム１
	u16		item2;				//アイテム２

	u8		sex;				//性別ベクトル
	u8		egg_birth;			//タマゴの孵化歩数
	u8		_friend;			//なつき度初期値
	u8		grow;				//成長曲線識別

	u8		egg_group1;			//こづくりグループ1
	u8		egg_group2;			//こづくりグループ2
	u8		speabi1;			//特殊能力１
	u8		speabi2;			//特殊能力２

	u8		escape;				//逃げる率
	u8		color	:7;			//色（図鑑で使用）
	u8		reverse	:1;			//反転フラグ
	u32		machine1;			//技マシンフラグ１
	u32		machine2;			//技マシンフラグ２
	u32		machine3;			//技マシンフラグ２
	u32		machine4;			//技マシンフラグ２
};

// ポケモンパラメータ取得系の定義
enum{
	ID_POKEPARADATA11=0,	//0
	ID_POKEPARADATA12,		//1
	ID_POKEPARADATA13,		//2
	ID_POKEPARADATA14,		//3
	ID_POKEPARADATA15,		//4
	ID_POKEPARADATA16,		//5
	ID_POKEPARADATA21,		//6
	ID_POKEPARADATA22,		//7
	ID_POKEPARADATA23,		//8
	ID_POKEPARADATA24,		//9
	ID_POKEPARADATA25,		//10
	ID_POKEPARADATA26,		//11
	ID_POKEPARADATA31,		//12
	ID_POKEPARADATA32,		//13
	ID_POKEPARADATA33,		//14
	ID_POKEPARADATA34,		//15
	ID_POKEPARADATA35,		//16
	ID_POKEPARADATA36,		//17
	ID_POKEPARADATA41,		//18
	ID_POKEPARADATA42,		//19
	ID_POKEPARADATA43,		//20
	ID_POKEPARADATA44,		//21
	ID_POKEPARADATA45,		//22
	ID_POKEPARADATA46,		//23

	ID_POKEPARADATA51,		//24
	ID_POKEPARADATA52,		//25
	ID_POKEPARADATA53,		//26
	ID_POKEPARADATA54,		//27
	ID_POKEPARADATA55,		//28
	ID_POKEPARADATA56,		//29

	ID_POKEPARADATA61,		//30
	ID_POKEPARADATA62,		//31
};

struct pokemon_para_data11
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data12
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data13
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data14
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data15
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data16
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data21
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data22
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data23
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data24
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data25
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data26
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data31
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data32
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data33
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data34
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data35
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data36
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data41
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data42
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data43
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data44
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data45
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data46
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
};

// ポケモンパーソナルデータ取得のためのインデックス
enum{
	ID_PER_basic_hp=0,			//基本ＨＰ
	ID_PER_basic_pow,			//基本攻撃力
	ID_PER_basic_def,			//基本防御力
	ID_PER_basic_agi,			//基本素早さ
	ID_PER_basic_spepow,		//基本特殊攻撃力
	ID_PER_basic_spedef,		//基本特殊防御力
	ID_PER_type1,				//属性１
	ID_PER_type2,				//属性２
	ID_PER_get_rate,			//捕獲率
	ID_PER_give_exp,			//贈与経験値
	ID_PER_pains_hp,			//贈与努力値ＨＰ
	ID_PER_pains_pow,			//贈与努力値攻撃力
	ID_PER_pains_def,			//贈与努力値防御力
	ID_PER_pains_agi,			//贈与努力値素早さ
	ID_PER_pains_spepow,		//贈与努力値特殊攻撃力
	ID_PER_pains_spedef,		//贈与努力値特殊防御力
	ID_PER_item1,				//アイテム１
	ID_PER_item2,				//アイテム２
	ID_PER_sex,					//性別ベクトル
	ID_PER_egg_birth,			//タマゴの孵化歩数
	ID_PER_friend,				//なつき度初期値
	ID_PER_grow,				//成長曲線識別
	ID_PER_egg_group1,			//こづくりグループ1
	ID_PER_egg_group2,			//こづくりグループ2
	ID_PER_speabi1,				//特殊能力１
	ID_PER_speabi2,				//特殊能力２
	ID_PER_escape,				//逃げる率
	ID_PER_color,				//色（図鑑で使用）
	ID_PER_reverse,				//反転フラグ
	ID_PER_machine1,			//技マシンフラグ１
	ID_PER_machine2,			//技マシンフラグ２
	ID_PER_machine3,			//技マシンフラグ３
	ID_PER_machine4				//技マシンフラグ４
};

/*-----------------------------------------------------------------------*
					関数プロトタイプ宣言
 *-----------------------------------------------------------------------*/

static u16 PokeParaCheckSum(void *data, u32 size);
static void PokeParaDecoded(void *data, u32 size, u32 code);
static void PokeParaCoded(void *data, u32 size, u32 code);
static u16 CodeRand(u32 *code);
static u32 PokeParaGetAct(POKEMON_PARAM *pp, int id, void *buf);
static u32 PokePasoParaGetAct(POKEMON_PASO_PARAM *ppp,int id, void *buf);
static void *PokeParaAdrsGet(POKEMON_PASO_PARAM *ppp, u32 rnd, u8 id);
static u32 PokeLevelCalc(u16 mons_no, u32 exp);
static POKEMON_PERSONAL_DATA* PokePersonalPara_Open(int mons_no);
static void PokePersonalDataGet(int mons_no, POKEMON_PERSONAL_DATA *ppd);
static void ArchiveDataLoad(void *data, const char* file, int datID);
static void ArchiveDataLoadIndex(void *data, const char *name, int index, int ofs, int ofs_size);
static u32 PokePersonal_LevelCalc(POKEMON_PERSONAL_DATA* personalData, u16 monsno, u32 exp);
static u32 PokePersonalPara_Get(POKEMON_PERSONAL_DATA *ppd, int para);
static void PokeGrowDataGet(int para, u32 *grow_tbl);
static void PokePersonalPara_Close(POKEMON_PERSONAL_DATA *ppd);
static void PokeParaPutAct(POKEMON_PARAM *pp, int id, const void *buf);
static void PokePasoParaPutAct(POKEMON_PASO_PARAM *ppp, int id, const void *buf);
static u8 PokeRareGetPara(u32 id,u32 rnd);

/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/

#ifdef WIN32

static const wchar_t* pokemon_default_name[MONSNO_END][COUNTRY_CODE_MAX] = {
	{ L"フシギダネ",	L"BULBASAUR",	L"BULBIZARRE",	L"BULBASAUR",	L"BISASAM",	L"",	L"BULBASAUR",	L"BULBASAUR" },
	{ L"フシギソウ",	L"IVYSAUR",	L"HERBIZARRE",	L"IVYSAUR",	L"BISAKNOSP",	L"",	L"IVYSAUR",	L"IVYSAUR" },
	{ L"フシギバナ",	L"VENUSAUR",	L"FLORIZARRE",	L"VENUSAUR",	L"BISAFLOR",	L"",	L"VENUSAUR",	L"VENUSAUR" },
	{ L"ヒトカゲ",	L"CHARMANDER",	L"SALAMECHE",	L"CHARMANDER",	L"GLUMANDA",	L"",	L"CHARMANDER",	L"CHARMANDER" },
	{ L"リザ－ド",	L"CHARMELEON",	L"REPTINCEL",	L"CHARMELEON",	L"GLUTEXO",	L"",	L"CHARMELEON",	L"CHARMELEON" },
	{ L"リザ－ドン",	L"CHARIZARD",	L"DRACAUFEU",	L"CHARIZARD",	L"GLURAK",	L"",	L"CHARIZARD",	L"CHARIZARD" },
	{ L"ゼニガメ",	L"SQUIRTLE",	L"CARAPUCE",	L"SQUIRTLE",	L"SCHIGGY",	L"",	L"SQUIRTLE",	L"SQUIRTLE" },
	{ L"カメ－ル",	L"WARTORTLE",	L"CARABAFFE",	L"WARTORTLE",	L"SCHILLOK",	L"",	L"WARTORTLE",	L"WARTORTLE" },
	{ L"カメックス",	L"BLASTOISE",	L"TORTANK",	L"BLASTOISE",	L"TURTOK",	L"",	L"BLASTOISE",	L"BLASTOISE" },
	{ L"キャタピ－",	L"CATERPIE",	L"CHENIPAN",	L"CATERPIE",	L"RAUPY",	L"",	L"CATERPIE",	L"CATERPIE" },
	{ L"トランセル",	L"METAPOD",	L"CHRYSACIER",	L"METAPOD",	L"SAFCON",	L"",	L"METAPOD",	L"METAPOD" },
	{ L"バタフリ－",	L"BUTTERFREE",	L"PAPILUSION",	L"BUTTERFREE",	L"SMETTBO",	L"",	L"BUTTERFREE",	L"BUTTERFREE" },
	{ L"ビ－ドル",	L"WEEDLE",	L"ASPICOT",	L"WEEDLE",	L"HORNLIU",	L"",	L"WEEDLE",	L"WEEDLE" },
	{ L"コク－ン",	L"KAKUNA",	L"COCONFORT",	L"KAKUNA",	L"KOKUNA",	L"",	L"KAKUNA",	L"KAKUNA" },
	{ L"スピア－",	L"BEEDRILL",	L"DARDARGNAN",	L"BEEDRILL",	L"BIBOR",	L"",	L"BEEDRILL",	L"BEEDRILL" },
	{ L"ポッポ",	L"PIDGEY",	L"ROUCOOL",	L"PIDGEY",	L"TAUBSI",	L"",	L"PIDGEY",	L"PIDGEY" },
	{ L"ピジョン",	L"PIDGEOTTO",	L"ROUCOUPS",	L"PIDGEOTTO",	L"TAUBOGA",	L"",	L"PIDGEOTTO",	L"PIDGEOTTO" },
	{ L"ピジョット",	L"PIDGEOT",	L"ROUCARNAGE",	L"PIDGEOT",	L"TAUBOSS",	L"",	L"PIDGEOT",	L"PIDGEOT" },
	{ L"コラッタ",	L"RATTATA",	L"RATTATA",	L"RATTATA",	L"RATTFRATZ",	L"",	L"RATTATA",	L"RATTATA" },
	{ L"ラッタ",	L"RATICATE",	L"RATTATAC",	L"RATICATE",	L"RATTIKARL",	L"",	L"RATICATE",	L"RATICATE" },
	{ L"オニスズメ",	L"SPEAROW",	L"PIAFABEC",	L"SPEAROW",	L"HABITAK",	L"",	L"SPEAROW",	L"SPEAROW" },
	{ L"オニドリル",	L"FEAROW",	L"RAPASDEPIC",	L"FEAROW",	L"IBITAK",	L"",	L"FEAROW",	L"FEAROW" },
	{ L"ア－ボ",	L"EKANS",	L"ABO",	L"EKANS",	L"RETTAN",	L"",	L"EKANS",	L"EKANS" },
	{ L"ア－ボック",	L"ARBOK",	L"ARBOK",	L"ARBOK",	L"ARBOK",	L"",	L"ARBOK",	L"ARBOK" },
	{ L"ピカチュウ",	L"PIKACHU",	L"PIKACHU",	L"PIKACHU",	L"PIKACHU",	L"",	L"PIKACHU",	L"PIKACHU" },
	{ L"ライチュウ",	L"RAICHU",	L"RAICHU",	L"RAICHU",	L"RAICHU",	L"",	L"RAICHU",	L"RAICHU" },
	{ L"サンド",	L"SANDSHREW",	L"SABELETTE",	L"SANDSHREW",	L"SANDAN",	L"",	L"SANDSHREW",	L"SANDSHREW" },
	{ L"サンドパン",	L"SANDSLASH",	L"SABLAIREAU",	L"SANDSLASH",	L"SANDAMER",	L"",	L"SANDSLASH",	L"SANDSLASH" },
	{ L"ニドラン♀",	L"NIDORAN>",	L"NIDORAN>",	L"NIDORAN>",	L"NIDORAN>",	L"",	L"NIDORAN>",	L"NIDORAN>" },
	{ L"ニドリ－ナ",	L"NIDORINA",	L"NIDORINA",	L"NIDORINA",	L"NIDORINA",	L"",	L"NIDORINA",	L"NIDORINA" },
	{ L"ニドクイン",	L"NIDOQUEEN",	L"NIDOQUEEN",	L"NIDOQUEEN",	L"NIDOQUEEN",	L"",	L"NIDOQUEEN",	L"NIDOQUEEN" },
	{ L"ニドラン♂",	L"NIDORAN<",	L"NIDORAN<",	L"NIDORAN<",	L"NIDORAN<",	L"",	L"NIDORAN<",	L"NIDORAN<" },
	{ L"ニドリ－ノ",	L"NIDORINO",	L"NIDORINO",	L"NIDORINO",	L"NIDORINO",	L"",	L"NIDORINO",	L"NIDORINO" },
	{ L"ニドキング",	L"NIDOKING",	L"NIDOKING",	L"NIDOKING",	L"NIDOKING",	L"",	L"NIDOKING",	L"NIDOKING" },
	{ L"ピッピ",	L"CLEFAIRY",	L"MELOFEE",	L"CLEFAIRY",	L"PIEPI",	L"",	L"CLEFAIRY",	L"CLEFAIRY" },
	{ L"ピクシ－",	L"CLEFABLE",	L"MELODELFE",	L"CLEFABLE",	L"PIXI",	L"",	L"CLEFABLE",	L"CLEFABLE" },
	{ L"ロコン",	L"VULPIX",	L"GOUPIX",	L"VULPIX",	L"VULPIX",	L"",	L"VULPIX",	L"VULPIX" },
	{ L"キュウコン",	L"NINETALES",	L"FEUNARD",	L"NINETALES",	L"VULNONA",	L"",	L"NINETALES",	L"NINETALES" },
	{ L"プリン",	L"JIGGLYPUFF",	L"RONDOUDOU",	L"JIGGLYPUFF",	L"PUMMELUFF",	L"",	L"JIGGLYPUFF",	L"JIGGLYPUFF" },
	{ L"プクリン",	L"WIGGLYTUFF",	L"GRODOUDOU",	L"WIGGLYTUFF",	L"KNUDDELUFF",	L"",	L"WIGGLYTUFF",	L"WIGGLYTUFF" },
	{ L"ズバット",	L"ZUBAT",	L"NOSFERAPTI",	L"ZUBAT",	L"ZUBAT",	L"",	L"ZUBAT",	L"ZUBAT" },
	{ L"ゴルバット",	L"GOLBAT",	L"NOSFERALTO",	L"GOLBAT",	L"GOLBAT",	L"",	L"GOLBAT",	L"GOLBAT" },
	{ L"ナゾノクサ",	L"ODDISH",	L"MYSTHERBE",	L"ODDISH",	L"MYRAPLA",	L"",	L"ODDISH",	L"ODDISH" },
	{ L"クサイハナ",	L"GLOOM",	L"ORTIDE",	L"GLOOM",	L"DUFLOR",	L"",	L"GLOOM",	L"GLOOM" },
	{ L"ラフレシア",	L"VILEPLUME",	L"RAFFLESIA",	L"VILEPLUME",	L"GIFLOR",	L"",	L"VILEPLUME",	L"VILEPLUME" },
	{ L"パラス",	L"PARAS",	L"PARAS",	L"PARAS",	L"PARAS",	L"",	L"PARAS",	L"PARAS" },
	{ L"パラセクト",	L"PARASECT",	L"PARASECT",	L"PARASECT",	L"PARASEK",	L"",	L"PARASECT",	L"PARASECT" },
	{ L"コンパン",	L"VENONAT",	L"MIMITOSS",	L"VENONAT",	L"BLUZUK",	L"",	L"VENONAT",	L"VENONAT" },
	{ L"モルフォン",	L"VENOMOTH",	L"AEROMITE",	L"VENOMOTH",	L"OMOT",	L"",	L"VENOMOTH",	L"VENOMOTH" },
	{ L"ディグダ",	L"DIGLETT",	L"TAUPIQUEUR",	L"DIGLETT",	L"DIGDA",	L"",	L"DIGLETT",	L"DIGLETT" },
	{ L"ダグトリオ",	L"DUGTRIO",	L"TRIOPIKEUR",	L"DUGTRIO",	L"DIGDRI",	L"",	L"DUGTRIO",	L"DUGTRIO" },
	{ L"ニャ－ス",	L"MEOWTH",	L"MIAOUSS",	L"MEOWTH",	L"MAUZI",	L"",	L"MEOWTH",	L"MEOWTH" },
	{ L"ペルシアン",	L"PERSIAN",	L"PERSIAN",	L"PERSIAN",	L"SNOBILIKAT",	L"",	L"PERSIAN",	L"PERSIAN" },
	{ L"コダック",	L"PSYDUCK",	L"PSYKOKWAK",	L"PSYDUCK",	L"ENTON",	L"",	L"PSYDUCK",	L"PSYDUCK" },
	{ L"ゴルダック",	L"GOLDUCK",	L"AKWAKWAK",	L"GOLDUCK",	L"ENTORON",	L"",	L"GOLDUCK",	L"GOLDUCK" },
	{ L"マンキ－",	L"MANKEY",	L"FEROSINGE",	L"MANKEY",	L"MENKI",	L"",	L"MANKEY",	L"MANKEY" },
	{ L"オコリザル",	L"PRIMEAPE",	L"COLOSSINGE",	L"PRIMEAPE",	L"RASAFF",	L"",	L"PRIMEAPE",	L"PRIMEAPE" },
	{ L"ガ－ディ",	L"GROWLITHE",	L"CANINOS",	L"GROWLITHE",	L"FUKANO",	L"",	L"GROWLITHE",	L"GROWLITHE" },
	{ L"ウインディ",	L"ARCANINE",	L"ARCANIN",	L"ARCANINE",	L"ARKANI",	L"",	L"ARCANINE",	L"ARCANINE" },
	{ L"ニョロモ",	L"POLIWAG",	L"PTITARD",	L"POLIWAG",	L"QUAPSEL",	L"",	L"POLIWAG",	L"POLIWAG" },
	{ L"ニョロゾ",	L"POLIWHIRL",	L"TETARTE",	L"POLIWHIRL",	L"QUAPUTZI",	L"",	L"POLIWHIRL",	L"POLIWHIRL" },
	{ L"ニョロボン",	L"POLIWRATH",	L"TARTARD",	L"POLIWRATH",	L"QUAPPO",	L"",	L"POLIWRATH",	L"POLIWRATH" },
	{ L"ケ－シィ",	L"ABRA",	L"ABRA",	L"ABRA",	L"ABRA",	L"",	L"ABRA",	L"ABRA" },
	{ L"ユンゲラ－",	L"KADABRA",	L"KADABRA",	L"KADABRA",	L"KADABRA",	L"",	L"KADABRA",	L"KADABRA" },
	{ L"フ－ディン",	L"ALAKAZAM",	L"ALAKAZAM",	L"ALAKAZAM",	L"SIMSALA",	L"",	L"ALAKAZAM",	L"ALAKAZAM" },
	{ L"ワンリキ－",	L"MACHOP",	L"MACHOC",	L"MACHOP",	L"MACHOLLO",	L"",	L"MACHOP",	L"MACHOP" },
	{ L"ゴ－リキ－",	L"MACHOKE",	L"MACHOPEUR",	L"MACHOKE",	L"MASCHOCK",	L"",	L"MACHOKE",	L"MACHOKE" },
	{ L"カイリキ－",	L"MACHAMP",	L"MACKOGNEUR",	L"MACHAMP",	L"MACHOMEI",	L"",	L"MACHAMP",	L"MACHAMP" },
	{ L"マダツボミ",	L"BELLSPROUT",	L"CHETIFLOR",	L"BELLSPROUT",	L"KNOFENSA",	L"",	L"BELLSPROUT",	L"BELLSPROUT" },
	{ L"ウツドン",	L"WEEPINBELL",	L"BOUSTIFLOR",	L"WEEPINBELL",	L"ULTRIGARIA",	L"",	L"WEEPINBELL",	L"WEEPINBELL" },
	{ L"ウツボット",	L"VICTREEBEL",	L"EMPIFLOR",	L"VICTREEBEL",	L"SARZENIA",	L"",	L"VICTREEBEL",	L"VICTREEBEL" },
	{ L"メノクラゲ",	L"TENTACOOL",	L"TENTACOOL",	L"TENTACOOL",	L"TENTACHA",	L"",	L"TENTACOOL",	L"TENTACOOL" },
	{ L"ドククラゲ",	L"TENTACRUEL",	L"TENTACRUEL",	L"TENTACRUEL",	L"TENTOXA",	L"",	L"TENTACRUEL",	L"TENTACRUEL" },
	{ L"イシツブテ",	L"GEODUDE",	L"RACAILLOU",	L"GEODUDE",	L"KLEINSTEIN",	L"",	L"GEODUDE",	L"GEODUDE" },
	{ L"ゴロ－ン",	L"GRAVELER",	L"GRAVALANCH",	L"GRAVELER",	L"GEOROK",	L"",	L"GRAVELER",	L"GRAVELER" },
	{ L"ゴロ－ニャ",	L"GOLEM",	L"GROLEM",	L"GOLEM",	L"GEOWAZ",	L"",	L"GOLEM",	L"GOLEM" },
	{ L"ポニ－タ",	L"PONYTA",	L"PONYTA",	L"PONYTA",	L"PONITA",	L"",	L"PONYTA",	L"PONYTA" },
	{ L"ギャロップ",	L"RAPIDASH",	L"GALOPA",	L"RAPIDASH",	L"GALLOPA",	L"",	L"RAPIDASH",	L"RAPIDASH" },
	{ L"ヤドン",	L"SLOWPOKE",	L"RAMOLOSS",	L"SLOWPOKE",	L"FLEGMON",	L"",	L"SLOWPOKE",	L"SLOWPOKE" },
	{ L"ヤドラン",	L"SLOWBRO",	L"FLAGADOSS",	L"SLOWBRO",	L"LAHMUS",	L"",	L"SLOWBRO",	L"SLOWBRO" },
	{ L"コイル",	L"MAGNEMITE",	L"MAGNETI",	L"MAGNEMITE",	L"MAGNETILO",	L"",	L"MAGNEMITE",	L"MAGNEMITE" },
	{ L"レアコイル",	L"MAGNETON",	L"MAGNETON",	L"MAGNETON",	L"MAGNETON",	L"",	L"MAGNETON",	L"MAGNETON" },
	{ L"カモネギ",	L"FARFETCH'D",	L"CANARTICHO",	L"FARFETCH'D",	L"PORENTA",	L"",	L"FARFETCH'D",	L"FARFETCH'D" },
	{ L"ド－ド－",	L"DODUO",	L"DODUO",	L"DODUO",	L"DODU",	L"",	L"DODUO",	L"DODUO" },
	{ L"ド－ドリオ",	L"DODRIO",	L"DODRIO",	L"DODRIO",	L"DODRI",	L"",	L"DODRIO",	L"DODRIO" },
	{ L"パウワウ",	L"SEEL",	L"OTARIA",	L"SEEL",	L"JUROB",	L"",	L"SEEL",	L"SEEL" },
	{ L"ジュゴン",	L"DEWGONG",	L"LAMANTINE",	L"DEWGONG",	L"JUGONG",	L"",	L"DEWGONG",	L"DEWGONG" },
	{ L"ベトベタ－",	L"GRIMER",	L"TADMORV",	L"GRIMER",	L"SLEIMA",	L"",	L"GRIMER",	L"GRIMER" },
	{ L"ベトベトン",	L"MUK",	L"GROTADMORV",	L"MUK",	L"SLEIMOK",	L"",	L"MUK",	L"MUK" },
	{ L"シェルダ－",	L"SHELLDER",	L"KOKIYAS",	L"SHELLDER",	L"MUSCHAS",	L"",	L"SHELLDER",	L"SHELLDER" },
	{ L"パルシェン",	L"CLOYSTER",	L"CRUSTABRI",	L"CLOYSTER",	L"AUSTOS",	L"",	L"CLOYSTER",	L"CLOYSTER" },
	{ L"ゴ－ス",	L"GASTLY",	L"FANTOMINUS",	L"GASTLY",	L"NEBULAK",	L"",	L"GASTLY",	L"GASTLY" },
	{ L"ゴ－スト",	L"HAUNTER",	L"SPECTRUM",	L"HAUNTER",	L"ALPOLLO",	L"",	L"HAUNTER",	L"HAUNTER" },
	{ L"ゲンガ－",	L"GENGAR",	L"ECTOPLASMA",	L"GENGAR",	L"GENGAR",	L"",	L"GENGAR",	L"GENGAR" },
	{ L"イワ－ク",	L"ONIX",	L"ONIX",	L"ONIX",	L"ONIX",	L"",	L"ONIX",	L"ONIX" },
	{ L"スリ－プ",	L"DROWZEE",	L"SOPORIFIK",	L"DROWZEE",	L"TRAUMATO",	L"",	L"DROWZEE",	L"DROWZEE" },
	{ L"スリ－パ－",	L"HYPNO",	L"HYPNOMADE",	L"HYPNO",	L"HYPNO",	L"",	L"HYPNO",	L"HYPNO" },
	{ L"クラブ",	L"KRABBY",	L"KRABBY",	L"KRABBY",	L"KRABBY",	L"",	L"KRABBY",	L"KRABBY" },
	{ L"キングラ－",	L"KINGLER",	L"KRABBOSS",	L"KINGLER",	L"KINGLER",	L"",	L"KINGLER",	L"KINGLER" },
	{ L"ビリリダマ",	L"VOLTORB",	L"VOLTORBE",	L"VOLTORB",	L"VOLTOBAL",	L"",	L"VOLTORB",	L"VOLTORB" },
	{ L"マルマイン",	L"ELECTRODE",	L"ELECTRODE",	L"ELECTRODE",	L"LEKTROBAL",	L"",	L"ELECTRODE",	L"ELECTRODE" },
	{ L"タマタマ",	L"EXEGGCUTE",	L"NOEUNOEUF",	L"EXEGGCUTE",	L"OWEI",	L"",	L"EXEGGCUTE",	L"EXEGGCUTE" },
	{ L"ナッシ－",	L"EXEGGUTOR",	L"NOADKOKO",	L"EXEGGUTOR",	L"KOKOWEI",	L"",	L"EXEGGUTOR",	L"EXEGGUTOR" },
	{ L"カラカラ",	L"CUBONE",	L"OSSELAIT",	L"CUBONE",	L"TRAGOSSO",	L"",	L"CUBONE",	L"CUBONE" },
	{ L"ガラガラ",	L"MAROWAK",	L"OSSATUEUR",	L"MAROWAK",	L"KNOGGA",	L"",	L"MAROWAK",	L"MAROWAK" },
	{ L"サワムラ－",	L"HITMONLEE",	L"KICKLEE",	L"HITMONLEE",	L"KICKLEE",	L"",	L"HITMONLEE",	L"HITMONLEE" },
	{ L"エビワラ－",	L"HITMONCHAN",	L"TYGNON",	L"HITMONCHAN",	L"NOCKCHAN",	L"",	L"HITMONCHAN",	L"HITMONCHAN" },
	{ L"ベロリンガ",	L"LICKITUNG",	L"EXCELANGUE",	L"LICKITUNG",	L"SCHLURP",	L"",	L"LICKITUNG",	L"LICKITUNG" },
	{ L"ドガ－ス",	L"KOFFING",	L"SMOGO",	L"KOFFING",	L"SMOGON",	L"",	L"KOFFING",	L"KOFFING" },
	{ L"マタドガス",	L"WEEZING",	L"SMOGOGO",	L"WEEZING",	L"SMOGMOG",	L"",	L"WEEZING",	L"WEEZING" },
	{ L"サイホ－ン",	L"RHYHORN",	L"RHINOCORNE",	L"RHYHORN",	L"RIHORN",	L"",	L"RHYHORN",	L"RHYHORN" },
	{ L"サイドン",	L"RHYDON",	L"RHINOFEROS",	L"RHYDON",	L"RIZEROS",	L"",	L"RHYDON",	L"RHYDON" },
	{ L"ラッキ－",	L"CHANSEY",	L"LEVEINARD",	L"CHANSEY",	L"CHANEIRA",	L"",	L"CHANSEY",	L"CHANSEY" },
	{ L"モンジャラ",	L"TANGELA",	L"SAQUEDENEU",	L"TANGELA",	L"TANGELA",	L"",	L"TANGELA",	L"TANGELA" },
	{ L"ガル－ラ",	L"KANGASKHAN",	L"KANGOUREX",	L"KANGASKHAN",	L"KANGAMA",	L"",	L"KANGASKHAN",	L"KANGASKHAN" },
	{ L"タッツ－",	L"HORSEA",	L"HYPOTREMPE",	L"HORSEA",	L"SEEPER",	L"",	L"HORSEA",	L"HORSEA" },
	{ L"シ－ドラ",	L"SEADRA",	L"HYPOCEAN",	L"SEADRA",	L"SEEMON",	L"",	L"SEADRA",	L"SEADRA" },
	{ L"トサキント",	L"GOLDEEN",	L"POISSIRENE",	L"GOLDEEN",	L"GOLDINI",	L"",	L"GOLDEEN",	L"GOLDEEN" },
	{ L"アズマオウ",	L"SEAKING",	L"POISSOROY",	L"SEAKING",	L"GOLKING",	L"",	L"SEAKING",	L"SEAKING" },
	{ L"ヒトデマン",	L"STARYU",	L"STARI",	L"STARYU",	L"STERNDU",	L"",	L"STARYU",	L"STARYU" },
	{ L"スタ－ミ－",	L"STARMIE",	L"STAROSS",	L"STARMIE",	L"STARMIE",	L"",	L"STARMIE",	L"STARMIE" },
	{ L"バリヤ－ド",	L"MR. MIME",	L"M. MIME",	L"MR. MIME",	L"PANTIMOS",	L"",	L"MR. MIME",	L"MR. MIME" },
	{ L"ストライク",	L"SCYTHER",	L"INSECATEUR",	L"SCYTHER",	L"SICHLOR",	L"",	L"SCYTHER",	L"SCYTHER" },
	{ L"ル－ジュラ",	L"JYNX",	L"LIPPOUTOU",	L"JYNX",	L"ROSSANA",	L"",	L"JYNX",	L"JYNX" },
	{ L"エレブ－",	L"ELECTABUZZ",	L"ELEKTEK",	L"ELECTABUZZ",	L"ELEKTEK",	L"",	L"ELECTABUZZ",	L"ELECTABUZZ" },
	{ L"ブ－バ－",	L"MAGMAR",	L"MAGMAR",	L"MAGMAR",	L"MAGMAR",	L"",	L"MAGMAR",	L"MAGMAR" },
	{ L"カイロス",	L"PINSIR",	L"SCARABRUTE",	L"PINSIR",	L"PINSIR",	L"",	L"PINSIR",	L"PINSIR" },
	{ L"ケンタロス",	L"TAUROS",	L"TAUROS",	L"TAUROS",	L"TAUROS",	L"",	L"TAUROS",	L"TAUROS" },
	{ L"コイキング",	L"MAGIKARP",	L"MAGICARPE",	L"MAGIKARP",	L"KARPADOR",	L"",	L"MAGIKARP",	L"MAGIKARP" },
	{ L"ギャラドス",	L"GYARADOS",	L"LEVIATOR",	L"GYARADOS",	L"GARADOS",	L"",	L"GYARADOS",	L"GYARADOS" },
	{ L"ラプラス",	L"LAPRAS",	L"LOKHLASS",	L"LAPRAS",	L"LAPRAS",	L"",	L"LAPRAS",	L"LAPRAS" },
	{ L"メタモン",	L"DITTO",	L"METAMORPH",	L"DITTO",	L"DITTO",	L"",	L"DITTO",	L"DITTO" },
	{ L"イ－ブイ",	L"EEVEE",	L"EVOLI",	L"EEVEE",	L"EVOLI",	L"",	L"EEVEE",	L"EEVEE" },
	{ L"シャワ－ズ",	L"VAPOREON",	L"AQUALI",	L"VAPOREON",	L"AQUANA",	L"",	L"VAPOREON",	L"VAPOREON" },
	{ L"サンダ－ス",	L"JOLTEON",	L"VOLTALI",	L"JOLTEON",	L"BLITZA",	L"",	L"JOLTEON",	L"JOLTEON" },
	{ L"ブ－スタ－",	L"FLAREON",	L"PYROLI",	L"FLAREON",	L"FLAMARA",	L"",	L"FLAREON",	L"FLAREON" },
	{ L"ポリゴン",	L"PORYGON",	L"PORYGON",	L"PORYGON",	L"PORYGON",	L"",	L"PORYGON",	L"PORYGON" },
	{ L"オムナイト",	L"OMANYTE",	L"AMONITA",	L"OMANYTE",	L"AMONITAS",	L"",	L"OMANYTE",	L"OMANYTE" },
	{ L"オムスタ－",	L"OMASTAR",	L"AMONISTAR",	L"OMASTAR",	L"AMOROSO",	L"",	L"OMASTAR",	L"OMASTAR" },
	{ L"カブト",	L"KABUTO",	L"KABUTO",	L"KABUTO",	L"KABUTO",	L"",	L"KABUTO",	L"KABUTO" },
	{ L"カブトプス",	L"KABUTOPS",	L"KABUTOPS",	L"KABUTOPS",	L"KABUTOPS",	L"",	L"KABUTOPS",	L"KABUTOPS" },
	{ L"プテラ",	L"AERODACTYL",	L"PTERA",	L"AERODACTYL",	L"AERODACTYL",	L"",	L"AERODACTYL",	L"AERODACTYL" },
	{ L"カビゴン",	L"SNORLAX",	L"RONFLEX",	L"SNORLAX",	L"RELAXO",	L"",	L"SNORLAX",	L"SNORLAX" },
	{ L"フリ－ザ－",	L"ARTICUNO",	L"ARTIKODIN",	L"ARTICUNO",	L"ARKTOS",	L"",	L"ARTICUNO",	L"ARTICUNO" },
	{ L"サンダ－",	L"ZAPDOS",	L"ELECTHOR",	L"ZAPDOS",	L"ZAPDOS",	L"",	L"ZAPDOS",	L"ZAPDOS" },
	{ L"ファイヤ－",	L"MOLTRES",	L"SULFURA",	L"MOLTRES",	L"LAVADOS",	L"",	L"MOLTRES",	L"MOLTRES" },
	{ L"ミニリュウ",	L"DRATINI",	L"MINIDRACO",	L"DRATINI",	L"DRATINI",	L"",	L"DRATINI",	L"DRATINI" },
	{ L"ハクリュ－",	L"DRAGONAIR",	L"DRACO",	L"DRAGONAIR",	L"DRAGONIR",	L"",	L"DRAGONAIR",	L"DRAGONAIR" },
	{ L"カイリュ－",	L"DRAGONITE",	L"DRACOLOSSE",	L"DRAGONITE",	L"DRAGORAN",	L"",	L"DRAGONITE",	L"DRAGONITE" },
	{ L"ミュウツ－",	L"MEWTWO",	L"MEWTWO",	L"MEWTWO",	L"MEWTU",	L"",	L"MEWTWO",	L"MEWTWO" },
	{ L"ミュウ",	L"MEW",	L"MEW",	L"MEW",	L"MEW",	L"",	L"MEW",	L"MEW" },
	{ L"チコリ－タ",	L"CHIKORITA",	L"GERMIGNON",	L"CHIKORITA",	L"ENDIVIE",	L"",	L"CHIKORITA",	L"CHIKORITA" },
	{ L"ベイリ－フ",	L"BAYLEEF",	L"MACRONIUM",	L"BAYLEEF",	L"LORBLATT",	L"",	L"BAYLEEF",	L"BAYLEEF" },
	{ L"メガニウム",	L"MEGANIUM",	L"MEGANIUM",	L"MEGANIUM",	L"MEGANIE",	L"",	L"MEGANIUM",	L"MEGANIUM" },
	{ L"ヒノアラシ",	L"CYNDAQUIL",	L"HERICENDRE",	L"CYNDAQUIL",	L"FEURIGEL",	L"",	L"CYNDAQUIL",	L"CYNDAQUIL" },
	{ L"マグマラシ",	L"QUILAVA",	L"FEURISSON",	L"QUILAVA",	L"IGELAVAR",	L"",	L"QUILAVA",	L"QUILAVA" },
	{ L"バクフ－ン",	L"TYPHLOSION",	L"TYPHLOSION",	L"TYPHLOSION",	L"TORNUPTO",	L"",	L"TYPHLOSION",	L"TYPHLOSION" },
	{ L"ワニノコ",	L"TOTODILE",	L"KAIMINUS",	L"TOTODILE",	L"KARNIMANI",	L"",	L"TOTODILE",	L"TOTODILE" },
	{ L"アリゲイツ",	L"CROCONAW",	L"CROCRODIL",	L"CROCONAW",	L"TYRACROC",	L"",	L"CROCONAW",	L"CROCONAW" },
	{ L"オ－ダイル",	L"FERALIGATR",	L"ALIGATUEUR",	L"FERALIGATR",	L"IMPERGATOR",	L"",	L"FERALIGATR",	L"FERALIGATR" },
	{ L"オタチ",	L"SENTRET",	L"FOUINETTE",	L"SENTRET",	L"WIESOR",	L"",	L"SENTRET",	L"SENTRET" },
	{ L"オオタチ",	L"FURRET",	L"FOUINAR",	L"FURRET",	L"WIESENIOR",	L"",	L"FURRET",	L"FURRET" },
	{ L"ホ－ホ－",	L"HOOTHOOT",	L"HOOTHOOT",	L"HOOTHOOT",	L"HOOTHOOT",	L"",	L"HOOTHOOT",	L"HOOTHOOT" },
	{ L"ヨルノズク",	L"NOCTOWL",	L"NOARFANG",	L"NOCTOWL",	L"NOCTUH",	L"",	L"NOCTOWL",	L"NOCTOWL" },
	{ L"レディバ",	L"LEDYBA",	L"COXY",	L"LEDYBA",	L"LEDYBA",	L"",	L"LEDYBA",	L"LEDYBA" },
	{ L"レディアン",	L"LEDIAN",	L"COXYCLAQUE",	L"LEDIAN",	L"LEDIAN",	L"",	L"LEDIAN",	L"LEDIAN" },
	{ L"イトマル",	L"SPINARAK",	L"MIMIGAL",	L"SPINARAK",	L"WEBARAK",	L"",	L"SPINARAK",	L"SPINARAK" },
	{ L"アリアドス",	L"ARIADOS",	L"MIGALOS",	L"ARIADOS",	L"ARIADOS",	L"",	L"ARIADOS",	L"ARIADOS" },
	{ L"クロバット",	L"CROBAT",	L"NOSTENFER",	L"CROBAT",	L"IKSBAT",	L"",	L"CROBAT",	L"CROBAT" },
	{ L"チョンチ－",	L"CHINCHOU",	L"LOUPIO",	L"CHINCHOU",	L"LAMPI",	L"",	L"CHINCHOU",	L"CHINCHOU" },
	{ L"ランタ－ン",	L"LANTURN",	L"LANTURN",	L"LANTURN",	L"LANTURN",	L"",	L"LANTURN",	L"LANTURN" },
	{ L"ピチュ－",	L"PICHU",	L"PICHU",	L"PICHU",	L"PICHU",	L"",	L"PICHU",	L"PICHU" },
	{ L"ピィ",	L"CLEFFA",	L"MELO",	L"CLEFFA",	L"PII",	L"",	L"CLEFFA",	L"CLEFFA" },
	{ L"ププリン",	L"IGGLYBUFF",	L"TOUDOUDOU",	L"IGGLYBUFF",	L"FLUFFELUFF",	L"",	L"IGGLYBUFF",	L"IGGLYBUFF" },
	{ L"トゲピ－",	L"TOGEPI",	L"TOGEPI",	L"TOGEPI",	L"TOGEPI",	L"",	L"TOGEPI",	L"TOGEPI" },
	{ L"トゲチック",	L"TOGETIC",	L"TOGETIC",	L"TOGETIC",	L"TOGETIC",	L"",	L"TOGETIC",	L"TOGETIC" },
	{ L"ネイティ",	L"NATU",	L"NATU",	L"NATU",	L"NATU",	L"",	L"NATU",	L"NATU" },
	{ L"ネイティオ",	L"XATU",	L"XATU",	L"XATU",	L"XATU",	L"",	L"XATU",	L"XATU" },
	{ L"メリ－プ",	L"MAREEP",	L"WATTOUAT",	L"MAREEP",	L"VOLTILAMM",	L"",	L"MAREEP",	L"MAREEP" },
	{ L"モココ",	L"FLAAFFY",	L"LAINERGIE",	L"FLAAFFY",	L"WAATY",	L"",	L"FLAAFFY",	L"FLAAFFY" },
	{ L"デンリュウ",	L"AMPHAROS",	L"PHARAMP",	L"AMPHAROS",	L"AMPHAROS",	L"",	L"AMPHAROS",	L"AMPHAROS" },
	{ L"キレイハナ",	L"BELLOSSOM",	L"JOLIFLOR",	L"BELLOSSOM",	L"BLUBELLA",	L"",	L"BELLOSSOM",	L"BELLOSSOM" },
	{ L"マリル",	L"MARILL",	L"MARILL",	L"MARILL",	L"MARILL",	L"",	L"MARILL",	L"MARILL" },
	{ L"マリルリ",	L"AZUMARILL",	L"AZUMARILL",	L"AZUMARILL",	L"AZUMARILL",	L"",	L"AZUMARILL",	L"AZUMARILL" },
	{ L"ウソッキ－",	L"SUDOWOODO",	L"SIMULARBRE",	L"SUDOWOODO",	L"MOGELBAUM",	L"",	L"SUDOWOODO",	L"SUDOWOODO" },
	{ L"ニョロトノ",	L"POLITOED",	L"TARPAUD",	L"POLITOED",	L"QUAXO",	L"",	L"POLITOED",	L"POLITOED" },
	{ L"ハネッコ",	L"HOPPIP",	L"GRANIVOL",	L"HOPPIP",	L"HOPPSPROSS",	L"",	L"HOPPIP",	L"HOPPIP" },
	{ L"ポポッコ",	L"SKIPLOOM",	L"FLORAVOL",	L"SKIPLOOM",	L"HUBELUPF",	L"",	L"SKIPLOOM",	L"SKIPLOOM" },
	{ L"ワタッコ",	L"JUMPLUFF",	L"COTOVOL",	L"JUMPLUFF",	L"PAPUNGHA",	L"",	L"JUMPLUFF",	L"JUMPLUFF" },
	{ L"エイパム",	L"AIPOM",	L"CAPUMAIN",	L"AIPOM",	L"GRIFFEL",	L"",	L"AIPOM",	L"AIPOM" },
	{ L"ヒマナッツ",	L"SUNKERN",	L"TOURNEGRIN",	L"SUNKERN",	L"SONNKERN",	L"",	L"SUNKERN",	L"SUNKERN" },
	{ L"キマワリ",	L"SUNFLORA",	L"HELIATRONC",	L"SUNFLORA",	L"SONNFLORA",	L"",	L"SUNFLORA",	L"SUNFLORA" },
	{ L"ヤンヤンマ",	L"YANMA",	L"YANMA",	L"YANMA",	L"YANMA",	L"",	L"YANMA",	L"YANMA" },
	{ L"ウパ－",	L"WOOPER",	L"AXOLOTO",	L"WOOPER",	L"FELINO",	L"",	L"WOOPER",	L"WOOPER" },
	{ L"ヌオ－",	L"QUAGSIRE",	L"MARAISTE",	L"QUAGSIRE",	L"MORLORD",	L"",	L"QUAGSIRE",	L"QUAGSIRE" },
	{ L"エ－フィ",	L"ESPEON",	L"MENTALI",	L"ESPEON",	L"PSIANA",	L"",	L"ESPEON",	L"ESPEON" },
	{ L"ブラッキ－",	L"UMBREON",	L"NOCTALI",	L"UMBREON",	L"NACHTARA",	L"",	L"UMBREON",	L"UMBREON" },
	{ L"ヤミカラス",	L"MURKROW",	L"CORNEBRE",	L"MURKROW",	L"KRAMURX",	L"",	L"MURKROW",	L"MURKROW" },
	{ L"ヤドキング",	L"SLOWKING",	L"ROIGADA",	L"SLOWKING",	L"LASCHOKING",	L"",	L"SLOWKING",	L"SLOWKING" },
	{ L"ムウマ",	L"MISDREAVUS",	L"FEUFOREVE",	L"MISDREAVUS",	L"TRAUNFUGIL",	L"",	L"MISDREAVUS",	L"MISDREAVUS" },
	{ L"アンノ－ン",	L"UNOWN",	L"ZARBI",	L"UNOWN",	L"ICOGNITO",	L"",	L"UNOWN",	L"UNOWN" },
	{ L"ソ－ナンス",	L"WOBBUFFET",	L"QULBUTOKE",	L"WOBBUFFET",	L"WOINGENAU",	L"",	L"WOBBUFFET",	L"WOBBUFFET" },
	{ L"キリンリキ",	L"GIRAFARIG",	L"GIRAFARIG",	L"GIRAFARIG",	L"GIRAFARIG",	L"",	L"GIRAFARIG",	L"GIRAFARIG" },
	{ L"クヌギダマ",	L"PINECO",	L"POMDEPIK",	L"PINECO",	L"TANNZA",	L"",	L"PINECO",	L"PINECO" },
	{ L"フォレトス",	L"FORRETRESS",	L"FORETRESS",	L"FORRETRESS",	L"FORSTELLKA",	L"",	L"FORRETRESS",	L"FORRETRESS" },
	{ L"ノコッチ",	L"DUNSPARCE",	L"INSOLOURDO",	L"DUNSPARCE",	L"DUMMISEL",	L"",	L"DUNSPARCE",	L"DUNSPARCE" },
	{ L"グライガ－",	L"GLIGAR",	L"SCORPLANE",	L"GLIGAR",	L"SKORGLA",	L"",	L"GLIGAR",	L"GLIGAR" },
	{ L"ハガネ－ル",	L"STEELIX",	L"STEELIX",	L"STEELIX",	L"STAHLOS",	L"",	L"STEELIX",	L"STEELIX" },
	{ L"ブル－",	L"SNUBBULL",	L"SNUBBULL",	L"SNUBBULL",	L"SNUBBULL",	L"",	L"SNUBBULL",	L"SNUBBULL" },
	{ L"グランブル",	L"GRANBULL",	L"GRANBULL",	L"GRANBULL",	L"GRANBULL",	L"",	L"GRANBULL",	L"GRANBULL" },
	{ L"ハリ－セン",	L"QWILFISH",	L"QWILFISH",	L"QWILFISH",	L"BALDORFISH",	L"",	L"QWILFISH",	L"QWILFISH" },
	{ L"ハッサム",	L"SCIZOR",	L"CIZAYOX",	L"SCIZOR",	L"SCHEROX",	L"",	L"SCIZOR",	L"SCIZOR" },
	{ L"ツボツボ",	L"SHUCKLE",	L"CARATROC",	L"SHUCKLE",	L"POTTROTT",	L"",	L"SHUCKLE",	L"SHUCKLE" },
	{ L"ヘラクロス",	L"HERACROSS",	L"SCARHINO",	L"HERACROSS",	L"SKARABORN",	L"",	L"HERACROSS",	L"HERACROSS" },
	{ L"ニュ－ラ",	L"SNEASEL",	L"FARFURET",	L"SNEASEL",	L"SNIEBEL",	L"",	L"SNEASEL",	L"SNEASEL" },
	{ L"ヒメグマ",	L"TEDDIURSA",	L"TEDDIURSA",	L"TEDDIURSA",	L"TEDDIURSA",	L"",	L"TEDDIURSA",	L"TEDDIURSA" },
	{ L"リングマ",	L"URSARING",	L"URSARING",	L"URSARING",	L"URSARING",	L"",	L"URSARING",	L"URSARING" },
	{ L"マグマッグ",	L"SLUGMA",	L"LIMAGMA",	L"SLUGMA",	L"SCHNECKMAG",	L"",	L"SLUGMA",	L"SLUGMA" },
	{ L"マグカルゴ",	L"MAGCARGO",	L"VOLCAROPOD",	L"MAGCARGO",	L"MAGCARGO",	L"",	L"MAGCARGO",	L"MAGCARGO" },
	{ L"ウリム－",	L"SWINUB",	L"MARCACRIN",	L"SWINUB",	L"QUIEKEL",	L"",	L"SWINUB",	L"SWINUB" },
	{ L"イノム－",	L"PILOSWINE",	L"COCHIGNON",	L"PILOSWINE",	L"KEIFEL",	L"",	L"PILOSWINE",	L"PILOSWINE" },
	{ L"サニ－ゴ",	L"CORSOLA",	L"CORAYON",	L"CORSOLA",	L"CORASONN",	L"",	L"CORSOLA",	L"CORSOLA" },
	{ L"テッポウオ",	L"REMORAID",	L"REMORAID",	L"REMORAID",	L"REMORAID",	L"",	L"REMORAID",	L"REMORAID" },
	{ L"オクタン",	L"OCTILLERY",	L"OCTILLERY",	L"OCTILLERY",	L"OCTILLERY",	L"",	L"OCTILLERY",	L"OCTILLERY" },
	{ L"デリバ－ド",	L"DELIBIRD",	L"CADOIZO",	L"DELIBIRD",	L"BOTOGEL",	L"",	L"DELIBIRD",	L"DELIBIRD" },
	{ L"マンタイン",	L"MANTINE",	L"DEMANTA",	L"MANTINE",	L"MANTAX",	L"",	L"MANTINE",	L"MANTINE" },
	{ L"エア－ムド",	L"SKARMORY",	L"AIRMURE",	L"SKARMORY",	L"PANZAERON",	L"",	L"SKARMORY",	L"SKARMORY" },
	{ L"デルビル",	L"HOUNDOUR",	L"MALOSSE",	L"HOUNDOUR",	L"HUNDUSTER",	L"",	L"HOUNDOUR",	L"HOUNDOUR" },
	{ L"ヘルガ－",	L"HOUNDOOM",	L"DEMOLOSSE",	L"HOUNDOOM",	L"HUNDEMON",	L"",	L"HOUNDOOM",	L"HOUNDOOM" },
	{ L"キングドラ",	L"KINGDRA",	L"HYPOROI",	L"KINGDRA",	L"SEEDRAKING",	L"",	L"KINGDRA",	L"KINGDRA" },
	{ L"ゴマゾウ",	L"PHANPY",	L"PHANPY",	L"PHANPY",	L"PHANPY",	L"",	L"PHANPY",	L"PHANPY" },
	{ L"ドンファン",	L"DONPHAN",	L"DONPHAN",	L"DONPHAN",	L"DONPHAN",	L"",	L"DONPHAN",	L"DONPHAN" },
	{ L"ポリゴン２",	L"PORYGON2",	L"PORYGON2",	L"PORYGON2",	L"PORYGON2",	L"",	L"PORYGON2",	L"PORYGON2" },
	{ L"オドシシ",	L"STANTLER",	L"CERFROUSSE",	L"STANTLER",	L"DAMHIRPLEX",	L"",	L"STANTLER",	L"STANTLER" },
	{ L"ド－ブル",	L"SMEARGLE",	L"QUEULORIOR",	L"SMEARGLE",	L"FARBEAGLE",	L"",	L"SMEARGLE",	L"SMEARGLE" },
	{ L"バルキ－",	L"TYROGUE",	L"DEBUGANT",	L"TYROGUE",	L"RABAUZ",	L"",	L"TYROGUE",	L"TYROGUE" },
	{ L"カポエラ－",	L"HITMONTOP",	L"KAPOERA",	L"HITMONTOP",	L"KAPOERA",	L"",	L"HITMONTOP",	L"HITMONTOP" },
	{ L"ムチュ－ル",	L"SMOOCHUM",	L"LIPPOUTI",	L"SMOOCHUM",	L"KUSSILLA",	L"",	L"SMOOCHUM",	L"SMOOCHUM" },
	{ L"エレキッド",	L"ELEKID",	L"ELEKID",	L"ELEKID",	L"ELEKID",	L"",	L"ELEKID",	L"ELEKID" },
	{ L"ブビィ",	L"MAGBY",	L"MAGBY",	L"MAGBY",	L"MAGBY",	L"",	L"MAGBY",	L"MAGBY" },
	{ L"ミルタンク",	L"MILTANK",	L"ECREMEUH",	L"MILTANK",	L"MILTANK",	L"",	L"MILTANK",	L"MILTANK" },
	{ L"ハピナス",	L"BLISSEY",	L"LEUPHORIE",	L"BLISSEY",	L"HEITEIRA",	L"",	L"BLISSEY",	L"BLISSEY" },
	{ L"ライコウ",	L"RAIKOU",	L"RAIKOU",	L"RAIKOU",	L"RAIKOU",	L"",	L"RAIKOU",	L"RAIKOU" },
	{ L"エンテイ",	L"ENTEI",	L"ENTEI",	L"ENTEI",	L"ENTEI",	L"",	L"ENTEI",	L"ENTEI" },
	{ L"スイクン",	L"SUICUNE",	L"SUICUNE",	L"SUICUNE",	L"SUICUNE",	L"",	L"SUICUNE",	L"SUICUNE" },
	{ L"ヨ－ギラス",	L"LARVITAR",	L"EMBRYLEX",	L"LARVITAR",	L"LARVITAR",	L"",	L"LARVITAR",	L"LARVITAR" },
	{ L"サナギラス",	L"PUPITAR",	L"YMPHECT",	L"PUPITAR",	L"PUPITAR",	L"",	L"PUPITAR",	L"PUPITAR" },
	{ L"バンギラス",	L"TYRANITAR",	L"TYRANOCIF",	L"TYRANITAR",	L"DESPOTAR",	L"",	L"TYRANITAR",	L"TYRANITAR" },
	{ L"ルギア",	L"LUGIA",	L"LUGIA",	L"LUGIA",	L"LUGIA",	L"",	L"LUGIA",	L"LUGIA" },
	{ L"ホウオウ",	L"HO-OH",	L"HO-OH",	L"HO-OH",	L"HO-OH",	L"",	L"HO-OH",	L"HO-OH" },
	{ L"セレビィ",	L"CELEBI",	L"CELEBI",	L"CELEBI",	L"CELEBI",	L"",	L"CELEBI",	L"CELEBI" },
	{ L"キモリ",	L"TREECKO",	L"ARCKO",	L"TREECKO",	L"GECKARBOR",	L"",	L"TREECKO",	L"TREECKO" },
	{ L"ジュプトル",	L"GROVYLE",	L"MASSKO",	L"GROVYLE",	L"REPTAIN",	L"",	L"GROVYLE",	L"GROVYLE" },
	{ L"ジュカイン",	L"SCEPTILE",	L"JUNGKO",	L"SCEPTILE",	L"GEWALDRO",	L"",	L"SCEPTILE",	L"SCEPTILE" },
	{ L"アチャモ",	L"TORCHIC",	L"POUSSIFEU",	L"TORCHIC",	L"FLEMMLI",	L"",	L"TORCHIC",	L"TORCHIC" },
	{ L"ワカシャモ",	L"COMBUSKEN",	L"GALIFEU",	L"COMBUSKEN",	L"JUNGGLUT",	L"",	L"COMBUSKEN",	L"COMBUSKEN" },
	{ L"バシャ－モ",	L"BLAZIKEN",	L"BRASEGALI",	L"BLAZIKEN",	L"LOHGOCK",	L"",	L"BLAZIKEN",	L"BLAZIKEN" },
	{ L"ミズゴロウ",	L"MUDKIP",	L"GOBOU",	L"MUDKIP",	L"HYDROPI",	L"",	L"MUDKIP",	L"MUDKIP" },
	{ L"ヌマクロ－",	L"MARSHTOMP",	L"FLOBIO",	L"MARSHTOMP",	L"MOORABBEL",	L"",	L"MARSHTOMP",	L"MARSHTOMP" },
	{ L"ラグラ－ジ",	L"SWAMPERT",	L"LAGGRON",	L"SWAMPERT",	L"SUMPEX",	L"",	L"SWAMPERT",	L"SWAMPERT" },
	{ L"ポチエナ",	L"POOCHYENA",	L"MEDHYENA",	L"POOCHYENA",	L"FIFFYEN",	L"",	L"POOCHYENA",	L"POOCHYENA" },
	{ L"グラエナ",	L"MIGHTYENA",	L"GRAHYENA",	L"MIGHTYENA",	L"MAGNAYEN",	L"",	L"MIGHTYENA",	L"MIGHTYENA" },
	{ L"ジグザグマ",	L"ZIGZAGOON",	L"ZIGZATON",	L"ZIGZAGOON",	L"ZIGZACHS",	L"",	L"ZIGZAGOON",	L"ZIGZAGOON" },
	{ L"マッスグマ",	L"LINOONE",	L"LINEON",	L"LINOONE",	L"GERADAKS",	L"",	L"LINOONE",	L"LINOONE" },
	{ L"ケムッソ",	L"WURMPLE",	L"CHENIPOTTE",	L"WURMPLE",	L"WAUMPEL",	L"",	L"WURMPLE",	L"WURMPLE" },
	{ L"カラサリス",	L"SILCOON",	L"ARMULYS",	L"SILCOON",	L"SCHALOKO",	L"",	L"SILCOON",	L"SILCOON" },
	{ L"アゲハント",	L"BEAUTIFLY",	L"CHARMILLON",	L"BEAUTIFLY",	L"PAPINELLA",	L"",	L"BEAUTIFLY",	L"BEAUTIFLY" },
	{ L"マユルド",	L"CASCOON",	L"BLINDALYS",	L"CASCOON",	L"PANEKON",	L"",	L"CASCOON",	L"CASCOON" },
	{ L"ドクケイル",	L"DUSTOX",	L"PAPINOX",	L"DUSTOX",	L"PUDOX",	L"",	L"DUSTOX",	L"DUSTOX" },
	{ L"ハスボ－",	L"LOTAD",	L"NENUPIOT",	L"LOTAD",	L"LOTURZEL",	L"",	L"LOTAD",	L"LOTAD" },
	{ L"ハスブレロ",	L"LOMBRE",	L"LOMBRE",	L"LOMBRE",	L"LOMBRERO",	L"",	L"LOMBRE",	L"LOMBRE" },
	{ L"ルンパッパ",	L"LUDICOLO",	L"LUDICOLO",	L"LUDICOLO",	L"KAPPALORES",	L"",	L"LUDICOLO",	L"LUDICOLO" },
	{ L"タネボ－",	L"SEEDOT",	L"GRAINIPIOT",	L"SEEDOT",	L"SAMURZEL",	L"",	L"SEEDOT",	L"SEEDOT" },
	{ L"コノハナ",	L"NUZLEAF",	L"PIFEUIL",	L"NUZLEAF",	L"BLANAS",	L"",	L"NUZLEAF",	L"NUZLEAF" },
	{ L"ダ－テング",	L"SHIFTRY",	L"TENGALICE",	L"SHIFTRY",	L"TENGULIST",	L"",	L"SHIFTRY",	L"SHIFTRY" },
	{ L"スバメ",	L"TAILLOW",	L"NIRONDELLE",	L"TAILLOW",	L"SCHWALBINI",	L"",	L"TAILLOW",	L"TAILLOW" },
	{ L"オオスバメ",	L"SWELLOW",	L"HELEDELLE",	L"SWELLOW",	L"SCHWALBOSS",	L"",	L"SWELLOW",	L"SWELLOW" },
	{ L"キャモメ",	L"WINGULL",	L"GOELISE",	L"WINGULL",	L"WINGULL",	L"",	L"WINGULL",	L"WINGULL" },
	{ L"ペリッパ－",	L"PELIPPER",	L"BEKIPAN",	L"PELIPPER",	L"PELIPPER",	L"",	L"PELIPPER",	L"PELIPPER" },
	{ L"ラルトス",	L"RALTS",	L"TARSAL",	L"RALTS",	L"TRASLA",	L"",	L"RALTS",	L"RALTS" },
	{ L"キルリア",	L"KIRLIA",	L"KIRLIA",	L"KIRLIA",	L"KIRLIA",	L"",	L"KIRLIA",	L"KIRLIA" },
	{ L"サ－ナイト",	L"GARDEVOIR",	L"GARDEVOIR",	L"GARDEVOIR",	L"GUARDEVOIR",	L"",	L"GARDEVOIR",	L"GARDEVOIR" },
	{ L"アメタマ",	L"SURSKIT",	L"ARAKDO",	L"SURSKIT",	L"GEHWEIHER",	L"",	L"SURSKIT",	L"SURSKIT" },
	{ L"アメモ－ス",	L"MASQUERAIN",	L"MASKADRA",	L"MASQUERAIN",	L"MASKEREGEN",	L"",	L"MASQUERAIN",	L"MASQUERAIN" },
	{ L"キノココ",	L"SHROOMISH",	L"BALIGNON",	L"SHROOMISH",	L"KNILZ",	L"",	L"SHROOMISH",	L"SHROOMISH" },
	{ L"キノガッサ",	L"BRELOOM",	L"CHAPIGNON",	L"BRELOOM",	L"KAPILZ",	L"",	L"BRELOOM",	L"BRELOOM" },
	{ L"ナマケロ",	L"SLAKOTH",	L"PARECOOL",	L"SLAKOTH",	L"BUMMELZ",	L"",	L"SLAKOTH",	L"SLAKOTH" },
	{ L"ヤルキモノ",	L"VIGOROTH",	L"VIGOROTH",	L"VIGOROTH",	L"MUNTIER",	L"",	L"VIGOROTH",	L"VIGOROTH" },
	{ L"ケッキング",	L"SLAKING",	L"MONAFLEMIT",	L"SLAKING",	L"LETARKING",	L"",	L"SLAKING",	L"SLAKING" },
	{ L"ツチニン",	L"NINCADA",	L"NINGALE",	L"NINCADA",	L"NINCADA",	L"",	L"NINCADA",	L"NINCADA" },
	{ L"テッカニン",	L"NINJASK",	L"NINJASK",	L"NINJASK",	L"NINJASK",	L"",	L"NINJASK",	L"NINJASK" },
	{ L"ヌケニン",	L"SHEDINJA",	L"MUNJA",	L"SHEDINJA",	L"NINJATOM",	L"",	L"SHEDINJA",	L"SHEDINJA" },
	{ L"ゴニョニョ",	L"WHISMUR",	L"CHUCHMUR",	L"WHISMUR",	L"FLURMEL",	L"",	L"WHISMUR",	L"WHISMUR" },
	{ L"ドゴ－ム",	L"LOUDRED",	L"RAMBOUM",	L"LOUDRED",	L"KRAKEELO",	L"",	L"LOUDRED",	L"LOUDRED" },
	{ L"バクオング",	L"EXPLOUD",	L"BROUHABAM",	L"EXPLOUD",	L"KRAWUMMS",	L"",	L"EXPLOUD",	L"EXPLOUD" },
	{ L"マクノシタ",	L"MAKUHITA",	L"MAKUHITA",	L"MAKUHITA",	L"MAKUHITA",	L"",	L"MAKUHITA",	L"MAKUHITA" },
	{ L"ハリテヤマ",	L"HARIYAMA",	L"HARIYAMA",	L"HARIYAMA",	L"HARIYAMA",	L"",	L"HARIYAMA",	L"HARIYAMA" },
	{ L"ルリリ",	L"AZURILL",	L"AZURILL",	L"AZURILL",	L"AZURILL",	L"",	L"AZURILL",	L"AZURILL" },
	{ L"ノズパス",	L"NOSEPASS",	L"TARINOR",	L"NOSEPASS",	L"NASGNET",	L"",	L"NOSEPASS",	L"NOSEPASS" },
	{ L"エネコ",	L"SKITTY",	L"SKITTY",	L"SKITTY",	L"ENECO",	L"",	L"SKITTY",	L"SKITTY" },
	{ L"エネコロロ",	L"DELCATTY",	L"DELCATTY",	L"DELCATTY",	L"ENEKORO",	L"",	L"DELCATTY",	L"DELCATTY" },
	{ L"ヤミラミ",	L"SABLEYE",	L"TENEFIX",	L"SABLEYE",	L"ZOBIRIS",	L"",	L"SABLEYE",	L"SABLEYE" },
	{ L"クチ－ト",	L"MAWILE",	L"MYSDIBULE",	L"MAWILE",	L"FLUNKIFER",	L"",	L"MAWILE",	L"MAWILE" },
	{ L"ココドラ",	L"ARON",	L"GALEKID",	L"ARON",	L"STOLLUNIOR",	L"",	L"ARON",	L"ARON" },
	{ L"コドラ",	L"LAIRON",	L"GALEGON",	L"LAIRON",	L"STOLLRAK",	L"",	L"LAIRON",	L"LAIRON" },
	{ L"ボスゴドラ",	L"AGGRON",	L"GALEKING",	L"AGGRON",	L"STOLLOSS",	L"",	L"AGGRON",	L"AGGRON" },
	{ L"アサナン",	L"MEDITITE",	L"MEDITIKKA",	L"MEDITITE",	L"MEDITIE",	L"",	L"MEDITITE",	L"MEDITITE" },
	{ L"チャ－レム",	L"MEDICHAM",	L"CHARMINA",	L"MEDICHAM",	L"MEDITALIS",	L"",	L"MEDICHAM",	L"MEDICHAM" },
	{ L"ラクライ",	L"ELECTRIKE",	L"DYNAVOLT",	L"ELECTRIKE",	L"FRIZELBLIZ",	L"",	L"ELECTRIKE",	L"ELECTRIKE" },
	{ L"ライボルト",	L"MANECTRIC",	L"ELECSPRINT",	L"MANECTRIC",	L"VOLTENSO",	L"",	L"MANECTRIC",	L"MANECTRIC" },
	{ L"プラスル",	L"PLUSLE",	L"POSIPI",	L"PLUSLE",	L"PLUSLE",	L"",	L"PLUSLE",	L"PLUSLE" },
	{ L"マイナン",	L"MINUN",	L"NEGAPI",	L"MINUN",	L"MINUN",	L"",	L"MINUN",	L"MINUN" },
	{ L"バルビ－ト",	L"VOLBEAT",	L"MUCIOLE",	L"VOLBEAT",	L"VOLBEAT",	L"",	L"VOLBEAT",	L"VOLBEAT" },
	{ L"イルミ－ゼ",	L"ILLUMISE",	L"LUMIVOLE",	L"ILLUMISE",	L"ILLUMISE",	L"",	L"ILLUMISE",	L"ILLUMISE" },
	{ L"ロゼリア",	L"ROSELIA",	L"ROSELIA",	L"ROSELIA",	L"ROSELIA",	L"",	L"ROSELIA",	L"ROSELIA" },
	{ L"ゴクリン",	L"GULPIN",	L"GLOUPTI",	L"GULPIN",	L"SCHLUPPUCK",	L"",	L"GULPIN",	L"GULPIN" },
	{ L"マルノ－ム",	L"SWALOT",	L"AVALTOUT",	L"SWALOT",	L"SCHLUKWECH",	L"",	L"SWALOT",	L"SWALOT" },
	{ L"キバニア",	L"CARVANHA",	L"CARVANHA",	L"CARVANHA",	L"KANIVANHA",	L"",	L"CARVANHA",	L"CARVANHA" },
	{ L"サメハダ－",	L"SHARPEDO",	L"SHARPEDO",	L"SHARPEDO",	L"TOHAIDO",	L"",	L"SHARPEDO",	L"SHARPEDO" },
	{ L"ホエルコ",	L"WAILMER",	L"WAILMER",	L"WAILMER",	L"WAILMER",	L"",	L"WAILMER",	L"WAILMER" },
	{ L"ホエルオ－",	L"WAILORD",	L"WAILORD",	L"WAILORD",	L"WAILORD",	L"",	L"WAILORD",	L"WAILORD" },
	{ L"ドンメル",	L"NUMEL",	L"CHAMALLOT",	L"NUMEL",	L"CAMAUB",	L"",	L"NUMEL",	L"NUMEL" },
	{ L"バク－ダ",	L"CAMERUPT",	L"CAMERUPT",	L"CAMERUPT",	L"CAMERUPT",	L"",	L"CAMERUPT",	L"CAMERUPT" },
	{ L"コ－タス",	L"TORKOAL",	L"CHARTOR",	L"TORKOAL",	L"QURTEL",	L"",	L"TORKOAL",	L"TORKOAL" },
	{ L"バネブ－",	L"SPOINK",	L"SPOINK",	L"SPOINK",	L"SPOINK",	L"",	L"SPOINK",	L"SPOINK" },
	{ L"ブ－ピッグ",	L"GRUMPIG",	L"GRORET",	L"GRUMPIG",	L"GROINK",	L"",	L"GRUMPIG",	L"GRUMPIG" },
	{ L"パッチ－ル",	L"SPINDA",	L"SPINDA",	L"SPINDA",	L"PANDIR",	L"",	L"SPINDA",	L"SPINDA" },
	{ L"ナックラ－",	L"TRAPINCH",	L"KRAKNOIX",	L"TRAPINCH",	L"KNACKLION",	L"",	L"TRAPINCH",	L"TRAPINCH" },
	{ L"ビブラ－バ",	L"VIBRAVA",	L"VIBRANINF",	L"VIBRAVA",	L"VIBRAVA",	L"",	L"VIBRAVA",	L"VIBRAVA" },
	{ L"フライゴン",	L"FLYGON",	L"LIBEGON",	L"FLYGON",	L"LIBELLDRA",	L"",	L"FLYGON",	L"FLYGON" },
	{ L"サボネア",	L"CACNEA",	L"CACNEA",	L"CACNEA",	L"TUSKA",	L"",	L"CACNEA",	L"CACNEA" },
	{ L"ノクタス",	L"CACTURNE",	L"CACTURNE",	L"CACTURNE",	L"NOKTUSKA",	L"",	L"CACTURNE",	L"CACTURNE" },
	{ L"チルット",	L"SWABLU",	L"TYLTON",	L"SWABLU",	L"WABLU",	L"",	L"SWABLU",	L"SWABLU" },
	{ L"チルタリス",	L"ALTARIA",	L"ALTARIA",	L"ALTARIA",	L"ALTARIA",	L"",	L"ALTARIA",	L"ALTARIA" },
	{ L"ザング－ス",	L"ZANGOOSE",	L"MANGRIFF",	L"ZANGOOSE",	L"SENGO",	L"",	L"ZANGOOSE",	L"ZANGOOSE" },
	{ L"ハブネ－ク",	L"SEVIPER",	L"SEVIPER",	L"SEVIPER",	L"VIPITIS",	L"",	L"SEVIPER",	L"SEVIPER" },
	{ L"ルナト－ン",	L"LUNATONE",	L"SELEROC",	L"LUNATONE",	L"LUNASTEIN",	L"",	L"LUNATONE",	L"LUNATONE" },
	{ L"ソルロック",	L"SOLROCK",	L"SOLAROC",	L"SOLROCK",	L"SONNFEL",	L"",	L"SOLROCK",	L"SOLROCK" },
	{ L"ドジョッチ",	L"BARBOACH",	L"BARLOCHE",	L"BARBOACH",	L"SCHMERBE",	L"",	L"BARBOACH",	L"BARBOACH" },
	{ L"ナマズン",	L"WHISCASH",	L"BARBICHA",	L"WHISCASH",	L"WELSAR",	L"",	L"WHISCASH",	L"WHISCASH" },
	{ L"ヘイガニ",	L"CORPHISH",	L"ECRAPINCE",	L"CORPHISH",	L"KREBSCORPS",	L"",	L"CORPHISH",	L"CORPHISH" },
	{ L"シザリガ－",	L"CRAWDAUNT",	L"COLHOMARD",	L"CRAWDAUNT",	L"KREBUTACK",	L"",	L"CRAWDAUNT",	L"CRAWDAUNT" },
	{ L"ヤジロン",	L"BALTOY",	L"BALBUTO",	L"BALTOY",	L"PUPPANCE",	L"",	L"BALTOY",	L"BALTOY" },
	{ L"ネンド－ル",	L"CLAYDOL",	L"KAORINE",	L"CLAYDOL",	L"LEPUMENTAS",	L"",	L"CLAYDOL",	L"CLAYDOL" },
	{ L"リリ－ラ",	L"LILEEP",	L"LILIA",	L"LILEEP",	L"LILIEP",	L"",	L"LILEEP",	L"LILEEP" },
	{ L"ユレイドル",	L"CRADILY",	L"VACILYS",	L"CRADILY",	L"WIELIE",	L"",	L"CRADILY",	L"CRADILY" },
	{ L"アノプス",	L"ANORITH",	L"ANORITH",	L"ANORITH",	L"ANORITH",	L"",	L"ANORITH",	L"ANORITH" },
	{ L"ア－マルド",	L"ARMALDO",	L"ARMALDO",	L"ARMALDO",	L"ARMALDO",	L"",	L"ARMALDO",	L"ARMALDO" },
	{ L"ヒンバス",	L"FEEBAS",	L"BARPAU",	L"FEEBAS",	L"BARSCHWA",	L"",	L"FEEBAS",	L"FEEBAS" },
	{ L"ミロカロス",	L"MILOTIC",	L"MILOBELLUS",	L"MILOTIC",	L"MILOTIC",	L"",	L"MILOTIC",	L"MILOTIC" },
	{ L"ポワルン",	L"CASTFORM",	L"MORPHEO",	L"CASTFORM",	L"FORMEO",	L"",	L"CASTFORM",	L"CASTFORM" },
	{ L"カクレオン",	L"KECLEON",	L"KECLEON",	L"KECLEON",	L"KECLEON",	L"",	L"KECLEON",	L"KECLEON" },
	{ L"カゲボウズ",	L"SHUPPET",	L"POLICHOMBR",	L"SHUPPET",	L"SHUPPET",	L"",	L"SHUPPET",	L"SHUPPET" },
	{ L"ジュペッタ",	L"BANETTE",	L"BRANETTE",	L"BANETTE",	L"BANETTE",	L"",	L"BANETTE",	L"BANETTE" },
	{ L"ヨマワル",	L"DUSKULL",	L"SKELENOX",	L"DUSKULL",	L"ZWIRRLICHT",	L"",	L"DUSKULL",	L"DUSKULL" },
	{ L"サマヨ－ル",	L"DUSCLOPS",	L"TERACLOPE",	L"DUSCLOPS",	L"ZWIRRKLOP",	L"",	L"DUSCLOPS",	L"DUSCLOPS" },
	{ L"トロピウス",	L"TROPIUS",	L"TROPIUS",	L"TROPIUS",	L"TROPIUS",	L"",	L"TROPIUS",	L"TROPIUS" },
	{ L"チリ－ン",	L"CHIMECHO",	L"EOKO",	L"CHIMECHO",	L"PALIMPALIM",	L"",	L"CHIMECHO",	L"CHIMECHO" },
	{ L"アブソル",	L"ABSOL",	L"ABSOL",	L"ABSOL",	L"ABSOL",	L"",	L"ABSOL",	L"ABSOL" },
	{ L"ソ－ナノ",	L"WYNAUT",	L"OKEOKE",	L"WYNAUT",	L"ISSO",	L"",	L"WYNAUT",	L"WYNAUT" },
	{ L"ユキワラシ",	L"SNORUNT",	L"STALGAMIN",	L"SNORUNT",	L"SCHNEPPKE",	L"",	L"SNORUNT",	L"SNORUNT" },
	{ L"オニゴ－リ",	L"GLALIE",	L"ONIGLALI",	L"GLALIE",	L"FIRNONTOR",	L"",	L"GLALIE",	L"GLALIE" },
	{ L"タマザラシ",	L"SPHEAL",	L"OBALIE",	L"SPHEAL",	L"SEEMOPS",	L"",	L"SPHEAL",	L"SPHEAL" },
	{ L"トドグラ－",	L"SEALEO",	L"PHOGLEUR",	L"SEALEO",	L"SEEJONG",	L"",	L"SEALEO",	L"SEALEO" },
	{ L"トドゼルガ",	L"WALREIN",	L"KAIMORSE",	L"WALREIN",	L"WALRAISA",	L"",	L"WALREIN",	L"WALREIN" },
	{ L"パ－ルル",	L"CLAMPERL",	L"COQUIPERL",	L"CLAMPERL",	L"PERLU",	L"",	L"CLAMPERL",	L"CLAMPERL" },
	{ L"ハンテ－ル",	L"HUNTAIL",	L"SERPANG",	L"HUNTAIL",	L"AALABYSS",	L"",	L"HUNTAIL",	L"HUNTAIL" },
	{ L"サクラビス",	L"GOREBYSS",	L"ROSABYSS",	L"GOREBYSS",	L"SAGANABYSS",	L"",	L"GOREBYSS",	L"GOREBYSS" },
	{ L"ジ－ランス",	L"RELICANTH",	L"RELICANTH",	L"RELICANTH",	L"RELICANTH",	L"",	L"RELICANTH",	L"RELICANTH" },
	{ L"ラブカス",	L"LUVDISC",	L"LOVDISC",	L"LUVDISC",	L"LIEBISKUS",	L"",	L"LUVDISC",	L"LUVDISC" },
	{ L"タツベイ",	L"BAGON",	L"DRABY",	L"BAGON",	L"KINDWURM",	L"",	L"BAGON",	L"BAGON" },
	{ L"コモル－",	L"SHELGON",	L"DRACKHAUS",	L"SHELGON",	L"DRASCHEL",	L"",	L"SHELGON",	L"SHELGON" },
	{ L"ボ－マンダ",	L"SALAMENCE",	L"DRATTAK",	L"SALAMENCE",	L"BRUTALANDA",	L"",	L"SALAMENCE",	L"SALAMENCE" },
	{ L"ダンバル",	L"BELDUM",	L"TERHAL",	L"BELDUM",	L"TANHEL",	L"",	L"BELDUM",	L"BELDUM" },
	{ L"メタング",	L"METANG",	L"METANG",	L"METANG",	L"METANG",	L"",	L"METANG",	L"METANG" },
	{ L"メタグロス",	L"METAGROSS",	L"METALOSSE",	L"METAGROSS",	L"METAGROSS",	L"",	L"METAGROSS",	L"METAGROSS" },
	{ L"レジロック",	L"REGIROCK",	L"REGIROCK",	L"REGIROCK",	L"REGIROCK",	L"",	L"REGIROCK",	L"REGIROCK" },
	{ L"レジアイス",	L"REGICE",	L"REGICE",	L"REGICE",	L"REGICE",	L"",	L"REGICE",	L"REGICE" },
	{ L"レジスチル",	L"REGISTEEL",	L"REGISTEEL",	L"REGISTEEL",	L"REGISTEEL",	L"",	L"REGISTEEL",	L"REGISTEEL" },
	{ L"ラティアス",	L"LATIAS",	L"LATIAS",	L"LATIAS",	L"LATIAS",	L"",	L"LATIAS",	L"LATIAS" },
	{ L"ラティオス",	L"LATIOS",	L"LATIOS",	L"LATIOS",	L"LATIOS",	L"",	L"LATIOS",	L"LATIOS" },
	{ L"カイオ－ガ",	L"KYOGRE",	L"KYOGRE",	L"KYOGRE",	L"KYOGRE",	L"",	L"KYOGRE",	L"KYOGRE" },
	{ L"グラ－ドン",	L"GROUDON",	L"GROUDON",	L"GROUDON",	L"GROUDON",	L"",	L"GROUDON",	L"GROUDON" },
	{ L"レックウザ",	L"RAYQUAZA",	L"RAYQUAZA",	L"RAYQUAZA",	L"RAYQUAZA",	L"",	L"RAYQUAZA",	L"RAYQUAZA" },
	{ L"ジラ－チ",	L"JIRACHI",	L"JIRACHI",	L"JIRACHI",	L"JIRACHI",	L"",	L"JIRACHI",	L"JIRACHI" },
	{ L"デオキシス",	L"DEOXYS",	L"DEOXYS",	L"DEOXYS",	L"DEOXYS",	L"",	L"DEOXYS",	L"DEOXYS" },
	{ L"ナエトル",	L"TURTWIG",	L"TORTIPOUSS",	L"TURTWIG",	L"CHELAST",	L"",	L"TURTWIG",	L"TURTWIG" },
	{ L"ハヤシガメ",	L"GROTLE",	L"BOSKARA",	L"GROTLE",	L"CHELCARAIN",	L"",	L"GROTLE",	L"GROTLE" },
	{ L"ドダイトス",	L"TORTERRA",	L"TORTERRA",	L"TORTERRA",	L"CHELTERRAR",	L"",	L"TORTERRA",	L"TORTERRA" },
	{ L"ヒコザル",	L"CHIMCHAR",	L"OUISTICRAM",	L"CHIMCHAR",	L"PANFLAM",	L"",	L"CHIMCHAR",	L"CHIMCHAR" },
	{ L"モウカザル",	L"MONFERNO",	L"CHIMPENFEU",	L"MONFERNO",	L"PANPYRO",	L"",	L"MONFERNO",	L"MONFERNO" },
	{ L"ゴウカザル",	L"INFERNAPE",	L"SIMIABRAZ",	L"INFERNAPE",	L"PANFERNO",	L"",	L"INFERNAPE",	L"INFERNAPE" },
	{ L"ポッチャマ",	L"PIPLUP",	L"TIPLOUF",	L"PIPLUP",	L"PLINFA",	L"",	L"PIPLUP",	L"PIPLUP" },
	{ L"ポッタイシ",	L"PRINPLUP",	L"PRINPLOUF",	L"PRINPLUP",	L"PLIPRIN",	L"",	L"PRINPLUP",	L"PRINPLUP" },
	{ L"エンペルト",	L"EMPOLEON",	L"PINGOLEON",	L"EMPOLEON",	L"IMPOLEON",	L"",	L"EMPOLEON",	L"EMPOLEON" },
	{ L"ムックル",	L"STARLY",	L"ETOURMI",	L"STARLY",	L"STARALILI",	L"",	L"STARLY",	L"STARLY" },
	{ L"ムクバ－ド",	L"STARAVIA",	L"ETOURVOL",	L"STARAVIA",	L"STARAVIA",	L"",	L"STARAVIA",	L"STARAVIA" },
	{ L"ムクホ－ク",	L"STARAPTOR",	L"ETOURAPTOR",	L"STARAPTOR",	L"STARAPTOR",	L"",	L"STARAPTOR",	L"STARAPTOR" },
	{ L"ビッパ",	L"BIDOOF",	L"KEUNOTOR",	L"BIDOOF",	L"BIDIZA",	L"",	L"BIDOOF",	L"BIDOOF" },
	{ L"ビ－ダル",	L"BIBAREL",	L"CASTORNO",	L"BIBAREL",	L"BIDIFAS",	L"",	L"BIBAREL",	L"BIBAREL" },
	{ L"コロボ－シ",	L"KRICKETOT",	L"CRIKZIK",	L"KRICKETOT",	L"ZIRPURZE",	L"",	L"KRICKETOT",	L"KRICKETOT" },
	{ L"コロトック",	L"KRICKETUNE",	L"MELOKRIK",	L"KRICKETUNE",	L"ZIRPEISE",	L"",	L"KRICKETUNE",	L"KRICKETUNE" },
	{ L"コリンク",	L"SHINX",	L"LIXY",	L"SHINX",	L"SHEINUX",	L"",	L"SHINX",	L"SHINX" },
	{ L"ルクシオ",	L"LUXIO",	L"LUXIO",	L"LUXIO",	L"LUXIO",	L"",	L"LUXIO",	L"LUXIO" },
	{ L"レントラ－",	L"LUXRAY",	L"LUXRAY",	L"LUXRAY",	L"LUXTRA",	L"",	L"LUXRAY",	L"LUXRAY" },
	{ L"スボミ－",	L"BUDEW",	L"ROZBOUTON",	L"BUDEW",	L"KNOSPI",	L"",	L"BUDEW",	L"BUDEW" },
	{ L"ロズレイド",	L"ROSERADE",	L"ROSERADE",	L"ROSERADE",	L"ROSERADE",	L"",	L"ROSERADE",	L"ROSERADE" },
	{ L"ズガイドス",	L"CRANIDOS",	L"KRANIDOS",	L"CRANIDOS",	L"KOKNODON",	L"",	L"CRANIDOS",	L"CRANIDOS" },
	{ L"ラムパルド",	L"RAMPARDOS",	L"CHARKOS",	L"RAMPARDOS",	L"RAMEIDON",	L"",	L"RAMPARDOS",	L"RAMPARDOS" },
	{ L"タテトプス",	L"SHIELDON",	L"DINOCLIER",	L"SHIELDON",	L"SCHILTERUS",	L"",	L"SHIELDON",	L"SHIELDON" },
	{ L"トリデプス",	L"BASTIODON",	L"BASTIODON",	L"BASTIODON",	L"BOLLTERUS",	L"",	L"BASTIODON",	L"BASTIODON" },
	{ L"ミノムッチ",	L"BURMY",	L"CHENITI",	L"BURMY",	L"BURMY",	L"",	L"BURMY",	L"BURMY" },
	{ L"ミノマダム",	L"WORMADAM",	L"CHENISELLE",	L"WORMADAM",	L"BURMADAME",	L"",	L"WORMADAM",	L"WORMADAM" },
	{ L"ガ－メイル",	L"MOTHIM",	L"PAPILORD",	L"MOTHIM",	L"MOTERPEL",	L"",	L"MOTHIM",	L"MOTHIM" },
	{ L"ミツハニ－",	L"COMBEE",	L"APITRINI",	L"COMBEE",	L"WADRIBIE",	L"",	L"COMBEE",	L"COMBEE" },
	{ L"ビ－クイン",	L"VESPIQUEN",	L"APIREINE",	L"VESPIQUEN",	L"HONWEISEL",	L"",	L"VESPIQUEN",	L"VESPIQUEN" },
	{ L"パチリス",	L"PACHIRISU",	L"PACHIRISU",	L"PACHIRISU",	L"PACHIRISU",	L"",	L"PACHIRISU",	L"PACHIRISU" },
	{ L"ブイゼル",	L"BUIZEL",	L"MUSTEBOUEE",	L"BUIZEL",	L"BAMELIN",	L"",	L"BUIZEL",	L"BUIZEL" },
	{ L"フロ－ゼル",	L"FLOATZEL",	L"MUSTEFLOTT",	L"FLOATZEL",	L"BOJELIN",	L"",	L"FLOATZEL",	L"FLOATZEL" },
	{ L"チェリンボ",	L"CHERUBI",	L"CERIBOU",	L"CHERUBI",	L"KIKUGI",	L"",	L"CHERUBI",	L"CHERUBI" },
	{ L"チェリム",	L"CHERRIM",	L"CERIFLOR",	L"CHERRIM",	L"KINOSO",	L"",	L"CHERRIM",	L"CHERRIM" },
	{ L"カラナクシ",	L"SHELLOS",	L"SANCOKI",	L"SHELLOS",	L"SCHALELLOS",	L"",	L"SHELLOS",	L"SHELLOS" },
	{ L"トリトドン",	L"GASTRODON",	L"TRITOSOR",	L"GASTRODON",	L"GASTRODON",	L"",	L"GASTRODON",	L"GASTRODON" },
	{ L"エテボ－ス",	L"AMBIPOM",	L"CAPIDEXTRE",	L"AMBIPOM",	L"AMBIDIFFEL",	L"",	L"AMBIPOM",	L"AMBIPOM" },
	{ L"フワンテ",	L"DRIFLOON",	L"BAUDRIVE",	L"DRIFLOON",	L"DRIFTLON",	L"",	L"DRIFLOON",	L"DRIFLOON" },
	{ L"フワライド",	L"DRIFBLIM",	L"GRODRIVE",	L"DRIFBLIM",	L"DRIFZEPELI",	L"",	L"DRIFBLIM",	L"DRIFBLIM" },
	{ L"ミミロル",	L"BUNEARY",	L"LAPOREILLE",	L"BUNEARY",	L"HASPIROR",	L"",	L"BUNEARY",	L"BUNEARY" },
	{ L"ミミロップ",	L"LOPUNNY",	L"LOCKPIN",	L"LOPUNNY",	L"SCHLAPOR",	L"",	L"LOPUNNY",	L"LOPUNNY" },
	{ L"ムウマ－ジ",	L"MISMAGIUS",	L"MAGIREVE",	L"MISMAGIUS",	L"TRAUNMAGIL",	L"",	L"MISMAGIUS",	L"MISMAGIUS" },
	{ L"ドンカラス",	L"HONCHKROW",	L"CORBOSS",	L"HONCHKROW",	L"KRAMSHEF",	L"",	L"HONCHKROW",	L"HONCHKROW" },
	{ L"ニャルマ－",	L"GLAMEOW",	L"CHAGLAM",	L"GLAMEOW",	L"CHARMIAN",	L"",	L"GLAMEOW",	L"GLAMEOW" },
	{ L"ブニャット",	L"PURUGLY",	L"CHAFFREUX",	L"PURUGLY",	L"SHNURGARST",	L"",	L"PURUGLY",	L"PURUGLY" },
	{ L"リ－シャン",	L"CHINGLING",	L"KORILLON",	L"CHINGLING",	L"KLINGPLIM",	L"",	L"CHINGLING",	L"CHINGLING" },
	{ L"スカンプ－",	L"STUNKY",	L"MOUFOUETTE",	L"STUNKY",	L"SKUNKAPUH",	L"",	L"STUNKY",	L"STUNKY" },
	{ L"スカタンク",	L"SKUNTANK",	L"MOUFFLAIR",	L"SKUNTANK",	L"SKUNTANK",	L"",	L"SKUNTANK",	L"SKUNTANK" },
	{ L"ド－ミラ－",	L"BRONZOR",	L"ARCHEOMIRE",	L"BRONZOR",	L"BRONZEL",	L"",	L"BRONZOR",	L"BRONZOR" },
	{ L"ド－タクン",	L"BRONZONG",	L"ARCHEODONG",	L"BRONZONG",	L"BRONZONG",	L"",	L"BRONZONG",	L"BRONZONG" },
	{ L"ウソハチ",	L"BONSLY",	L"MANZAI",	L"BONSLY",	L"MOBAI",	L"",	L"BONSLY",	L"BONSLY" },
	{ L"マネネ",	L"MIME JR.",	L"MIME JR.",	L"MIME JR.",	L"PANTIMIMI",	L"",	L"MIME JR.",	L"MIME JR." },
	{ L"ピンプク",	L"HAPPINY",	L"PTIRAVI",	L"HAPPINY",	L"WONNEIRA",	L"",	L"HAPPINY",	L"HAPPINY" },
	{ L"ペラップ",	L"CHATOT",	L"PIJAKO",	L"CHATOT",	L"PLAUDAGEI",	L"",	L"CHATOT",	L"CHATOT" },
	{ L"ミカルゲ",	L"SPIRITOMB",	L"SPIRITOMB",	L"SPIRITOMB",	L"KRYPPUK",	L"",	L"SPIRITOMB",	L"SPIRITOMB" },
	{ L"フカマル",	L"GIBLE",	L"GRIKNOT",	L"GIBLE",	L"KAUMALAT",	L"",	L"GIBLE",	L"GIBLE" },
	{ L"ガバイト",	L"GABITE",	L"CARMACHE",	L"GABITE",	L"KNARKSEL",	L"",	L"GABITE",	L"GABITE" },
	{ L"ガブリアス",	L"GARCHOMP",	L"CARCHACROK",	L"GARCHOMP",	L"KNAKRACK",	L"",	L"GARCHOMP",	L"GARCHOMP" },
	{ L"ゴンベ",	L"MUNCHLAX",	L"GOINFREX",	L"MUNCHLAX",	L"MAMPFAXO",	L"",	L"MUNCHLAX",	L"MUNCHLAX" },
	{ L"リオル",	L"RIOLU",	L"RIOLU",	L"RIOLU",	L"RIOLU",	L"",	L"RIOLU",	L"RIOLU" },
	{ L"ルカリオ",	L"LUCARIO",	L"LUCARIO",	L"LUCARIO",	L"LUCARIO",	L"",	L"LUCARIO",	L"LUCARIO" },
	{ L"ヒポポタス",	L"HIPPOPOTAS",	L"HIPPOPOTAS",	L"HIPPOPOTAS",	L"HIPPOPOTAS",	L"",	L"HIPPOPOTAS",	L"HIPPOPOTAS" },
	{ L"カバルドン",	L"HIPPOWDON",	L"HIPPODOCUS",	L"HIPPOWDON",	L"HIPPOTERUS",	L"",	L"HIPPOWDON",	L"HIPPOWDON" },
	{ L"スコルピ",	L"SKORUPI",	L"RAPION",	L"SKORUPI",	L"PIONSKORA",	L"",	L"SKORUPI",	L"SKORUPI" },
	{ L"ドラピオン",	L"DRAPION",	L"DRASCORE",	L"DRAPION",	L"PIONDRAGI",	L"",	L"DRAPION",	L"DRAPION" },
	{ L"グレッグル",	L"CROAGUNK",	L"CRADOPAUD",	L"CROAGUNK",	L"GLIBUNKEL",	L"",	L"CROAGUNK",	L"CROAGUNK" },
	{ L"ドクロッグ",	L"TOXICROAK",	L"COATOX",	L"TOXICROAK",	L"TOXIQUAK",	L"",	L"TOXICROAK",	L"TOXICROAK" },
	{ L"マスキッパ",	L"CARNIVINE",	L"VORTENTE",	L"CARNIVINE",	L"VENUFLIBIS",	L"",	L"CARNIVINE",	L"CARNIVINE" },
	{ L"ケイコウオ",	L"FINNEON",	L"ECAYON",	L"FINNEON",	L"FINNEON",	L"",	L"FINNEON",	L"FINNEON" },
	{ L"ネオラント",	L"LUMINEON",	L"LUMINEON",	L"LUMINEON",	L"LUMINEON",	L"",	L"LUMINEON",	L"LUMINEON" },
	{ L"タマンタ",	L"MANTYKE",	L"BABIMANTA",	L"MANTYKE",	L"MANTIRPS",	L"",	L"MANTYKE",	L"MANTYKE" },
	{ L"ユキカブリ",	L"SNOVER",	L"BLIZZI",	L"SNOVER",	L"SHNEBEDECK",	L"",	L"SNOVER",	L"SNOVER" },
	{ L"ユキノオ－",	L"ABOMASNOW",	L"BLIZZAROI",	L"ABOMASNOW",	L"REXBLISAR",	L"",	L"ABOMASNOW",	L"ABOMASNOW" },
	{ L"マニュ－ラ",	L"WEAVILE",	L"DIMORET",	L"WEAVILE",	L"SNIBUNNA",	L"",	L"WEAVILE",	L"WEAVILE" },
	{ L"ジバコイル",	L"MAGNEZONE",	L"MAGNEZONE",	L"MAGNEZONE",	L"MAGNEZONE",	L"",	L"MAGNEZONE",	L"MAGNEZONE" },
	{ L"ベロベルト",	L"LICKILICKY",	L"COUDLANGUE",	L"LICKILICKY",	L"SCHLURPLEK",	L"",	L"LICKILICKY",	L"LICKILICKY" },
	{ L"ドサイドン",	L"RHYPERIOR",	L"RHINASTOC",	L"RHYPERIOR",	L"RIHORNIOR",	L"",	L"RHYPERIOR",	L"RHYPERIOR" },
	{ L"モジャンボ",	L"TANGROWTH",	L"BOULDENEU",	L"TANGROWTH",	L"TANGOLOSS",	L"",	L"TANGROWTH",	L"TANGROWTH" },
	{ L"エレキブル",	L"ELECTIVIRE",	L"ELEKABLE",	L"ELECTIVIRE",	L"ELEVOLTEK",	L"",	L"ELECTIVIRE",	L"ELECTIVIRE" },
	{ L"ブ－バ－ン",	L"MAGMORTAR",	L"MAGANON",	L"MAGMORTAR",	L"MAGBRANT",	L"",	L"MAGMORTAR",	L"MAGMORTAR" },
	{ L"トゲキッス",	L"TOGEKISS",	L"TOGEKISS",	L"TOGEKISS",	L"TOGEKISS",	L"",	L"TOGEKISS",	L"TOGEKISS" },
	{ L"メガヤンマ",	L"YANMEGA",	L"YANMEGA",	L"YANMEGA",	L"YANMEGA",	L"",	L"YANMEGA",	L"YANMEGA" },
	{ L"リ－フィア",	L"LEAFEON",	L"PHYLLALI",	L"LEAFEON",	L"FOLIPURBA",	L"",	L"LEAFEON",	L"LEAFEON" },
	{ L"グレイシア",	L"GLACEON",	L"GIVRALI",	L"GLACEON",	L"GLAZIOLA",	L"",	L"GLACEON",	L"GLACEON" },
	{ L"グライオン",	L"GLISCOR",	L"SCORVOL",	L"GLISCOR",	L"SKORGRO",	L"",	L"GLISCOR",	L"GLISCOR" },
	{ L"マンム－",	L"MAMOSWINE",	L"MAMMOCHON",	L"MAMOSWINE",	L"MAMUTEL",	L"",	L"MAMOSWINE",	L"MAMOSWINE" },
	{ L"ポリゴンＺ",	L"PORYGON-Z",	L"PORYGON-Z",	L"PORYGON-Z",	L"PORYGON-Z",	L"",	L"PORYGON-Z",	L"PORYGON-Z" },
	{ L"エルレイド",	L"GALLADE",	L"GALLAME",	L"GALLADE",	L"GALAGLADI",	L"",	L"GALLADE",	L"GALLADE" },
	{ L"ダイノ－ズ",	L"PROBOPASS",	L"TARINORME",	L"PROBOPASS",	L"VOLUMINAS",	L"",	L"PROBOPASS",	L"PROBOPASS" },
	{ L"ヨノワ－ル",	L"DUSKNOIR",	L"NOCTUNOIR",	L"DUSKNOIR",	L"ZWIRRFINST",	L"",	L"DUSKNOIR",	L"DUSKNOIR" },
	{ L"ユキメノコ",	L"FROSLASS",	L"MOMARTIK",	L"FROSLASS",	L"FROSDEDJE",	L"",	L"FROSLASS",	L"FROSLASS" },
	{ L"ロトム",	L"ROTOM",	L"MOTISMA",	L"ROTOM",	L"ROTOM",	L"",	L"ROTOM",	L"ROTOM" },
	{ L"ユクシ－",	L"UXIE",	L"CREHELF",	L"UXIE",	L"SELFE",	L"",	L"UXIE",	L"UXIE" },
	{ L"エムリット",	L"MESPRIT",	L"CREFOLLET",	L"MESPRIT",	L"VESPRIT",	L"",	L"MESPRIT",	L"MESPRIT" },
	{ L"アグノム",	L"AZELF",	L"CREFADET",	L"AZELF",	L"TOBUTZ",	L"",	L"AZELF",	L"AZELF" },
	{ L"ディアルガ",	L"DIALGA",	L"DIALGA",	L"DIALGA",	L"DIALGA",	L"",	L"DIALGA",	L"DIALGA" },
	{ L"パルキア",	L"PALKIA",	L"PALKIA",	L"PALKIA",	L"PALKIA",	L"",	L"PALKIA",	L"PALKIA" },
	{ L"ヒ－ドラン",	L"HEATRAN",	L"HEATRAN",	L"HEATRAN",	L"HEATRAN",	L"",	L"HEATRAN",	L"HEATRAN" },
	{ L"レジギガス",	L"REGIGIGAS",	L"REGIGIGAS",	L"REGIGIGAS",	L"REGIGIGAS",	L"",	L"REGIGIGAS",	L"REGIGIGAS" },
	{ L"ギラティナ",	L"GIRATINA",	L"GIRATINA",	L"GIRATINA",	L"GIRATINA",	L"",	L"GIRATINA",	L"GIRATINA" },
	{ L"クレセリア",	L"CRESSELIA",	L"CRESSELIA",	L"CRESSELIA",	L"CRESSELIA",	L"",	L"CRESSELIA",	L"CRESSELIA" },
	{ L"フィオネ",	L"PHIONE",	L"PHIONE",	L"PHIONE",	L"PHIONE",	L"",	L"PHIONE",	L"PHIONE" },
	{ L"マナフィ",	L"MANAPHY",	L"MANAPHY",	L"MANAPHY",	L"MANAPHY",	L"",	L"MANAPHY",	L"MANAPHY" },
	{ L"ダ－クライ",	L"DARKRAI",	L"DARKRAI",	L"DARKRAI",	L"DARKRAI",	L"",	L"DARKRAI",	L"DARKRAI" },
	{ L"シェイミ",	L"SHAYMIN",	L"SHAYMIN",	L"SHAYMIN",	L"SHAYMIN",	L"",	L"SHAYMIN",	L"SHAYMIN" },
	{ L"アルセウス",	L"ARCEUS",	L"ARCEUS",	L"ARCEUS",	L"ARCEUS",	L"",	L"ARCEUS",	L"ARCEUS" }
};

#endif

/*-----------------------------------------------------------------------*
					グローバル関数定義
 *-----------------------------------------------------------------------*/

/*!
	ポケモンデータを暗号化します。

	@param src	暗号化を行うポケモンデータ
	@param dst	暗号化を行ったデータの出力先
*/
void PokeTool_EncodePokemonParam(const POKEMON_PARAM* src, POKEMON_PARAM* dst)
{
	*dst = *src;

	PokeParaCoded(&dst->pcp, sizeof(POKEMON_CALC_PARAM), dst->ppp.personal_rnd);
	dst->ppp.checksum=PokeParaCheckSum(&dst->ppp.paradata, sizeof(POKEMON_PASO_PARAM1)*4);
	PokeParaCoded(&dst->ppp.paradata, sizeof(POKEMON_PASO_PARAM1)*4, dst->ppp.checksum);
}

/*!
	ポケモンデータの暗号を解除します。

	@param src	暗号解除を行うポケモンデータ
	@param dst	暗号解除を行ったデータの出力先

	@retval TRUE	暗号解除に成功した
	@retval FALSE	暗号解除に失敗した。内部のチェックサムが合わなかった。
*/
BOOL PokeTool_DecodePokemonParam(const POKEMON_PARAM* src, POKEMON_PARAM* dst)
{
	u16	sum;
	*dst = *src;

	PokeParaDecoded(&dst->pcp, sizeof(POKEMON_CALC_PARAM), dst->ppp.personal_rnd);
	PokeParaDecoded(&dst->ppp.paradata, sizeof(POKEMON_PASO_PARAM1)*4, dst->ppp.checksum);
	sum = PokeParaCheckSum(&dst->ppp.paradata, sizeof(POKEMON_PASO_PARAM1)*4);
	if (sum != dst->ppp.checksum) {
		return FALSE;
	}

	return TRUE;
}

/*!
	ポケモンデータ内のパラメータを取得します。
	暗号を解除したデータに対してのみ使用できます。

	取得結果が返値になるか、bufに入るかはパラメータによって異なりますので、
	ソースを読んで判断してください。

	@param pp	パラメータを取得するポケモンデータ
	@param id	取得したいパラメータのID
	@param buf	取得したパラメータの出力先

	@return	取得したパラメータ、もしくは取得が成功したか失敗したか。
*/
u32 PokeTool_GetPokeParam(POKEMON_PARAM *pp, PokeToolParamID id, void *buf)
{
	return PokeParaGetAct(pp, id, buf);
}

/*!
	ポケモンデータ内のパラメータを設定します。
	暗号を解除したデータに対してのみ使用できます。

	データの与え方はパラメータによって異なりますので、ソースを読んで判断
	してください。

	@param pp	パラメータを設定するポケモンデータ
	@param id	設定したいパラメータのID
	@param buf	設定したいパラメータの入力元
*/
void PokeTool_SetPokeParam(POKEMON_PARAM *pp, PokeToolParamID id, void *buf)
{
	PokeParaPutAct(pp, id, buf);
}

/*!
	ポケモンデータ内のニックネームフラグを取得します。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	ニックネームフラグを取得するポケモンデータ

	@retval TRUE	ニックネームが設定されている
	@retval FALSE	ニックネームが設定されていない
*/
BOOL PokeTool_GetNickNameFlag(POKEMON_PARAM *pp)
{
	return PokeTool_GetPokeParam(pp, ID_PARA_nickname_flag, NULL);
}

/*!
	ポケモンデータ内のニックネームフラグを設定します。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	ニックネームフラグを設定するポケモンデータ
	@param flag	設定するフラグ。TRUEでニックネームが設定されている、FALSEでニックネームが設定
				されていないという意味になります。
*/
void PokeTool_SetNickNameFlag(POKEMON_PARAM *pp, BOOL flag)
{
	PokeTool_SetPokeParam(pp, ID_PARA_nickname_flag, &flag);
}

/*!
	ポケモンデータ内のニックネームを取得します。結果はUTF-16LEで出力されます。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	ニックネームを取得するポケモンデータ
	@param buf	取得したニックネームの保存先。(MONS_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE)以上の
				大きさとしてください。

	@retval TRUE	取得が成功した。
	@retval FALSE	取得が失敗した。ニックネームが正しいポケモンコードではなかった。
*/
BOOL PokeTool_GetNickName(POKEMON_PARAM *pp, u16* buf)
{
	u16 tempbuf[MONS_NAME_SIZE+EOM_SIZE];

	memset(tempbuf, 0, sizeof(tempbuf));
	PokeTool_GetPokeParam(pp, ID_PARA_nickname, tempbuf);
	return (pokecode_to_unicode(tempbuf, (wchar_t*)buf, sizeof(u16)*(MONS_NAME_SIZE+EOM_SIZE)) != -1);
}

/*!
	ポケモンデータ内のニックネームを設定します。名前はUTF-16LEで与えてください。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	ニックネームを設定するポケモンデータ
	@param buf	設定するニックネーム。MONS_NAME_SIZE以下の長さとし、必ず0で終端してください。

	@retval TRUE	設定が成功した。
	@retval FALSE	設定が失敗した。与えられた名前をポケモンコードに変換できなかった。
*/
BOOL PokeTool_SetNickName(POKEMON_PARAM *pp, const u16* buf)
{
	u16 tempbuf[MONS_NAME_SIZE+EOM_SIZE];

	memset(tempbuf, 0, sizeof(tempbuf));
	if (unicode_to_pokecode((const wchar_t*)buf, tempbuf, sizeof(tempbuf)) == -1) {
		return FALSE;
	}
	PokeTool_SetPokeParam(pp, ID_PARA_nickname, tempbuf);

	return TRUE;
}

/*!
	ポケモンデータ内の親の名前を取得します。結果はUTF-16LEで出力されます。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	親の名前を取得するポケモンデータ
	@param buf	取得した親の名前の保存先。(PERSON_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE)以上の
				大きさとしてください。

	@retval TRUE	取得が成功した。
	@retval FALSE	取得が失敗した。親の名前が正しいポケモンコードではなかった。
*/
BOOL PokeTool_GetParentName(POKEMON_PARAM *pp, u16* buf)
{
	u16 tempbuf[PERSON_NAME_SIZE+EOM_SIZE];

	memset(tempbuf, 0, sizeof(tempbuf));
	PokeTool_GetPokeParam(pp, ID_PARA_oyaname, tempbuf);
	return (pokecode_to_unicode(tempbuf, (wchar_t*)buf, sizeof(u16)*(PERSON_NAME_SIZE+EOM_SIZE)) != -1);
}

/*!
	ポケモンデータ内の親の名前を設定します。名前はUTF-16LEで与えてください。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	親の名前を設定するポケモンデータ
	@param buf	設定する親の名前。PERSON_NAME_SIZE以下の長さとし、必ず0で終端してください。

	@retval TRUE	設定が成功した。
	@retval FALSE	設定が失敗した。与えられた名前をポケモンコードに変換できなかった。
*/
BOOL PokeTool_SetParentName(POKEMON_PARAM *pp, const u16* buf)
{
	u16 tempbuf[PERSON_NAME_SIZE+EOM_SIZE];

	memset(tempbuf, 0, sizeof(tempbuf));
	if (unicode_to_pokecode((const wchar_t*)buf, tempbuf, sizeof(tempbuf)) == -1) {
		return FALSE;
	}
	PokeTool_SetPokeParam(pp, ID_PARA_oyaname, tempbuf);

	return TRUE;
}

/*!
	ポケモンデータ内のメールの名前を取得します。結果はUTF-16LEで出力されます。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	メールの名前を取得するポケモンデータ
	@param buf	取得したメールの名前の保存先。(PERSON_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE)以上の
				大きさとしてください。

	@retval TRUE	取得が成功した。
	@retval FALSE	取得が失敗した。メールの名前が正しいポケモンコードではなかった。
*/
BOOL PokeTool_GetMailName(POKEMON_PARAM *pp, u16* buf)
{
	MAIL_DATA mail_data;

	PokeTool_GetPokeParam(pp, ID_PARA_mail_data, &mail_data);
	return (pokecode_to_unicode(mail_data.name, (wchar_t*)buf, sizeof(u16)*(PERSON_NAME_SIZE+EOM_SIZE)) != -1);
}

/*!
	ポケモンデータ内のメールの名前を設定します。名前はUTF-16LEで与えてください。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	メールの名前を設定するポケモンデータ
	@param buf	設定するメールの名前。PERSON_NAME_SIZE以下の長さとし、必ず0で終端してください。

	@retval TRUE	設定が成功した。
	@retval FALSE	設定が失敗した。与えられた名前をポケモンコードに変換できなかった。
*/
BOOL PokeTool_SetMailName(POKEMON_PARAM *pp, const u16* buf)
{
	MAIL_DATA mail_data;
	u16 tempbuf[PERSON_NAME_SIZE+EOM_SIZE];

	memset(tempbuf, 0, sizeof(tempbuf));
	if (unicode_to_pokecode((const wchar_t*)buf, tempbuf, sizeof(tempbuf)) == -1) {
		return FALSE;
	}
	PokeTool_GetPokeParam(pp, ID_PARA_mail_data, &mail_data);
	memcpy(mail_data.name, tempbuf, sizeof(u16)*(PERSON_NAME_SIZE+EOM_SIZE));
	PokeTool_SetPokeParam(pp, ID_PARA_mail_data, &mail_data);

	return TRUE;
}

/*!
	バトルタワーのポケモンデータ内のニックネームを取得します。結果はUTF-16LEで出力されます。

	@param btp	ニックネームを取得するポケモンデータ
	@param buf	取得したニックネームの保存先。(MONS_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE)以上の
				大きさとしてください。

	@retval TRUE	取得に成功した。
	@retval FALSE	取得に失敗した。ニックネームが正しいポケモンコードではなかった。
*/
BOOL PokeTool_BtGetNickName(B_TOWER_POKEMON* btp, u16* buf) {

	return (pokecode_to_unicode(btp->nickname, (wchar_t*)buf, sizeof(u16)*(MONS_NAME_SIZE+EOM_SIZE)) != -1);
}

/*!
	バトルタワーのポケモンデータ内のニックネームを設定します。ニックネームはUTF-16LEで与えてください。

	@param btp	ニックネームを設定するポケモンデータ
	@param buf	設定するニックネーム。MONS_NAME_SIZE以下の長さとし、必ず0で終端してください。

	@retval TRUE	設定が成功した。
	@retval FALSE	設定が失敗した。与えられた名前をポケモンコードに変換できなかった。
*/
BOOL PokeTool_BtSetNickName(B_TOWER_POKEMON* btp, const u16* buf) {

	u16 tempbuf[MONS_NAME_SIZE+EOM_SIZE];

	if (unicode_to_pokecode((const wchar_t*)buf, tempbuf, sizeof(tempbuf)) == -1) {
		return FALSE;
	}
	memcpy(btp->nickname, tempbuf, sizeof(u16)*(MONS_NAME_SIZE+EOM_SIZE));

	return TRUE;
}

/*!
	指定したポケモンがレア（色違い）であるか否かを返します。
	暗号を解除したデータに対してのみ使用できます。

	@param pp レアかどうかを判定したいポケモンデータ

	@retval	TRUE	指定したポケモンはレア（色違い）である
	@retval	FALSE	指定したポケモンはレアでない
*/
BOOL PokeTool_IsRare(POKEMON_PARAM *pp) {

	return (PokeRareGetPara(PokeTool_GetPokeParam(pp, ID_PARA_id_no, NULL),
		PokeTool_GetPokeParam(pp, ID_PARA_personal_rnd, NULL)) != 0);
}

/*!
	指定したバトルタワーのポケモンがレア（色違い）であるか否かを返します。

	@param btp レアかどうかを判定したいバトルタワーのポケモンデータ

	@retval	TRUE	指定したポケモンはレア（色違い）である
	@retval	FALSE	指定したポケモンはレアでない
*/
BOOL PokeTool_BtIsRare(B_TOWER_POKEMON *btp) {

	return (PokeRareGetPara(btp->id_no, btp->personal_rnd) != 0);
}

/*!
	指定したポケモンがメールを持っているかを返します。
	暗号を解除したデータに対してのみ使用できます。

	@param pp メールを持っているかどうかを判定したいポケモンデータ

	@retval	TRUE	指定したポケモンはメールを持っている
	@retval	FALSE	指定したポケモンはメールを持っていない
*/
BOOL PokeTool_HasMail(POKEMON_PARAM *pp) {

	u16 item = (u16)PokeTool_GetPokeParam(pp, ID_PARA_item, NULL);

	if (ITEM_GURASUMEERU <= item && item <= ITEM_BURIKKUMEERU) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*!
	指定したポケモンのメールの言語コードを返します。
	暗号を解除したデータに対してのみ使用できます。

	この関数は、データ内の生の数値をそのまま返します。必ずPokeTool_HasMailで
	メールを保持しているかどうかを確認してから使用してください。

	@param pp メールの言語コードを取得したいポケモンデータ

	@return	メールの言語コード。正常なデータでは、COUNTRY_CODEで定義された値になります。
*/
u8 PokeTool_GetMailLang(POKEMON_PARAM *pp) {

	MAIL_DATA mail_data;

	PokeTool_GetPokeParam(pp, ID_PARA_mail_data, &mail_data);

	return mail_data.region;
}

/*!
	指定したモンスターナンバーのポケモンのデフォルト名を返します。

	@param monsno	デフォルト名を取得したいポケモンのモンスターナンバー。1以上MONSNO_END以下の値にしてください。
	@param countryCode	デフォルト名を取得したい国コード。1以上COUNTRY_CODE以下の値にしてください。

	@return ポケモンのデフォルト名。UTF-16LEで与えられます。

	@bug この関数は、UNIX環境では正常に動作しません。
*/
const wchar_t* PokeTool_GetDefaultName(u32 monsno, u8 countryCode) {

	const wchar_t* result;

	DPW_MINMAX_TASSERT(monsno, 1, MONSNO_END);
	DPW_MINMAX_TASSERT(countryCode, 1, COUNTRY_CODE_MAX);
    if(monsno < 1 || monsno > MONSNO_END)
    {
        return POKEMON_NONE_NAME;
    }
    if(countryCode < 1 || countryCode > COUNTRY_CODE_MAX)
    {
        return POKEMON_NONE_NAME;
    }

#ifdef WIN32
	result = pokemon_default_name[monsno - 1][countryCode - 1];
#else
	result = L"";
#endif

	return wcscmp(result, L"") != 0 ? result : POKEMON_NONE_NAME;
}

/*!
	ポケモンの名前をデフォルトに設定します。
	具体的には、ニックネームネームフラグをOFFにし、ニックネームをデフォルト名に変更します。
	暗号を解除したデータに対してのみ使用できます。

	@param pp	名前をデフォルトに設定するポケモンデータ

	@retval TRUE	設定に成功した。
	@retval FALSE	設定に失敗した。ポケモンデータ内のモンスターナンバーが不正な値だった。

	@bug この関数は、UNIX環境では正常に動作しません。
*/
BOOL PokeTool_SetNickNameDefault(POKEMON_PARAM *pp) {

	u32 monsno = PokeTool_GetPokeParam(pp, ID_PARA_monsno, NULL);
	u8 countryCode = (u8)PokeTool_GetPokeParam(pp, ID_PARA_country_code, NULL);

	if (monsno == 0 || monsno > MONSNO_END) {
		return FALSE;
	}

	if (countryCode == 0 || countryCode > COUNTRY_CODE_MAX || countryCode == COUNTRY_CODE_RESERVE) {
		return FALSE;
	}

	PokeTool_SetNickNameFlag(pp, FALSE);
	(void)PokeTool_SetNickName(pp, (const u16*)PokeTool_GetDefaultName(monsno, countryCode));

	return TRUE;
}


/*-----------------------------------------------------------------------*
					ローカル関数定義
 *-----------------------------------------------------------------------*/

/**
 *	ポケモンパラメータのチェックサムを生成
 *
 * @param[in]	data	ポケモンパラメータ構造体のポインタ
 * @param[in]	size	チェックサムを生成するデータのサイズ
 *
 * @return	生成したチェックサム
 */
static u16 PokeParaCheckSum(void *data, u32 size)
{
	u32	i;
	u16* data_p = (u16*)data;
	u16	sum = 0;

	for (i=0; i < size/2; i++) {
		sum += data_p[i];
	}

	return sum;
}

/**
 *	復号処理
 *
 * @param[in]	data	復号するデータのポインタ
 * @param[in]	size	復号するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
static void PokeParaDecoded(void *data, u32 size, u32 code)
{
	PokeParaCoded(data, size, code);
}

/**
 *	暗号処理
 *
 * @param[in]	data	暗号化するデータのポインタ
 * @param[in]	size	暗号化するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
static void PokeParaCoded(void *data, u32 size, u32 code)
{
	u32	i;
	u16* data_p = (u16*)data;

	//暗号は、乱数暗号キーでマスク
	for (i=0; i < size/2; i++) {
		data_p[i] ^= CodeRand(&code);
	}
}

/**
 *	乱数暗号キー生成ルーチン
 *
 * @param[in,out]	code	暗号キー格納ワークのポインタ
 *
 * @return	暗号キー格納ワークの上位2バイトを暗号キーとして返す
 */
static u16 CodeRand(u32 *code)
{
    code[0] = code[0]*1103515245L + 24691;
    return (u16)(code[0] / 65536L);
}

/**
 *	ポケモンパラメータ構造体から任意で値を取得
 *
 * @param[in]	pp	取得するポケモンパラメータ構造体のポインタ
 * @param[in]	id	取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]	buf	取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return		取得したデータ
 */
static u32 PokeParaGetAct(POKEMON_PARAM *pp, int id, void *buf)
{
	u32	ret=0;

	switch(id){
	case ID_PARA_condition:
		ret=pp->pcp.condition;
		break;
	case ID_PARA_level:
		ret=pp->pcp.level;
		break;
	case ID_PARA_cb_id:
		ret=pp->pcp.cb_id;
		break;
	case ID_PARA_hp:
		ret=pp->pcp.hp;
		break;
	case ID_PARA_hpmax:
		ret=pp->pcp.hpmax;
		break;
	case ID_PARA_pow:
		ret=pp->pcp.pow;
		break;
	case ID_PARA_def:
		ret=pp->pcp.def;
		break;
	case ID_PARA_agi:
		ret=pp->pcp.agi;
		break;
	case ID_PARA_spepow:
		ret=pp->pcp.spepow;
		break;
	case ID_PARA_spedef:
		ret=pp->pcp.spedef;
		break;
	case ID_PARA_mail_data:
		memcpy(buf, &pp->pcp.mail_data, sizeof(MAIL_DATA));
		ret = TRUE;
		break;
	case ID_PARA_cb_core:
		memcpy(buf, &pp->pcp.cb_core,  sizeof(CB_CORE));
		ret = TRUE;
		break;
	default:
		ret = PokePasoParaGetAct((POKEMON_PASO_PARAM *)&pp->ppp, id, buf);
		break;
	}

	return	ret;
}

/**
 *	ボックスポケモンパラメータ構造体から任意で値を取得
 *
 * @param[in]	pp	取得するボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id	取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]	buf	取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return		取得したデータ
 */
static u32 PokePasoParaGetAct(POKEMON_PASO_PARAM *ppp,int id, void *buf)
{
	u32	ret=0;
	u64	bit;
	POKEMON_PASO_PARAM1	*ppp1;
	POKEMON_PASO_PARAM2	*ppp2;
	POKEMON_PASO_PARAM3	*ppp3;
	POKEMON_PASO_PARAM4	*ppp4;

	ppp1=(POKEMON_PASO_PARAM1 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA1);
	ppp2=(POKEMON_PASO_PARAM2 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA2);
	ppp3=(POKEMON_PASO_PARAM3 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA3);
	ppp4=(POKEMON_PASO_PARAM4 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA4);

	switch(id){
		default:
			ret=0;
			break;
//PARAM
		case ID_PARA_personal_rnd:
			ret=ppp->personal_rnd;
			break;
		case ID_PARA_pp_fast_mode:
			ret=ppp->pp_fast_mode;
			break;
		case ID_PARA_ppp_fast_mode:
			ret=ppp->ppp_fast_mode;
			break;
		case ID_PARA_checksum:
			ret=ppp->checksum;
			break;
		case ID_PARA_poke_exist:
			ret=(ppp1->monsno!=0);
			break;
		case ID_PARA_tamago_exist:
			ret=ppp2->tamago_flag;
			break;
		case ID_PARA_monsno_egg:
			ret=ppp1->monsno;
			if(ret==0){
				break;
			}
			if(ppp2->tamago_flag){
				ret=MONSNO_TAMAGO;
			}
			break;
		case ID_PARA_level:
			DPW_TASSERTMSG(FALSE, "get ID_PARA_level is not implemented\n");
//			ret=PokeLevelCalc(ppp1->monsno,ppp1->exp);
			break;
//PARAM1
		case ID_PARA_monsno:
			ret=ppp1->monsno;
			break;
		case ID_PARA_item:
			ret=ppp1->item;
			break;
		case ID_PARA_id_no:
			ret=ppp1->id_no;
			break;
		case ID_PARA_exp:
			ret=ppp1->exp;
			break;
		case ID_PARA_friend:
			ret=ppp1->_friend;
			break;
		case ID_PARA_speabino:
			ret=ppp1->speabino;
			break;
		case ID_PARA_mark:
			ret=ppp1->mark;
			break;
		case ID_PARA_country_code:
			ret=ppp1->country_code;
			break;
		case ID_PARA_hp_exp:
			ret=ppp1->hp_exp;
			break;
		case ID_PARA_pow_exp:
			ret=ppp1->pow_exp;
			break;
		case ID_PARA_def_exp:
			ret=ppp1->def_exp;
			break;
		case ID_PARA_agi_exp:
			ret=ppp1->agi_exp;
			break;
		case ID_PARA_spepow_exp:
			ret=ppp1->spepow_exp;
			break;
		case ID_PARA_spedef_exp:
			ret=ppp1->spedef_exp;
			break;
		case ID_PARA_style:
			ret=ppp1->style;
			break;
		case ID_PARA_beautiful:
			ret=ppp1->beautiful;
			break;
		case ID_PARA_cute:
			ret=ppp1->cute;
			break;
		case ID_PARA_clever:
			ret=ppp1->clever;
			break;
		case ID_PARA_strong:
			ret=ppp1->strong;
			break;
		case ID_PARA_fur:
			ret=ppp1->fur;
			break;
		case ID_PARA_sinou_champ_ribbon:				//シンオウチャンプリボン
		case ID_PARA_sinou_battle_tower_ttwin_first:	//シンオウバトルタワータワータイクーン勝利1回目
		case ID_PARA_sinou_battle_tower_ttwin_second:	//シンオウバトルタワータワータイクーン勝利2回目
		case ID_PARA_sinou_battle_tower_2vs2_win50:		//シンオウバトルタワータワーダブル50連勝
		case ID_PARA_sinou_battle_tower_aimulti_win50:	//シンオウバトルタワータワーAIマルチ50連勝
		case ID_PARA_sinou_battle_tower_siomulti_win50:	//シンオウバトルタワータワー通信マルチ50連勝
		case ID_PARA_sinou_battle_tower_wifi_rank5:		//シンオウバトルタワーWifiランク５入り
		case ID_PARA_sinou_syakki_ribbon:				//シンオウしゃっきリボン
		case ID_PARA_sinou_dokki_ribbon:				//シンオウどっきリボン
		case ID_PARA_sinou_syonbo_ribbon:				//シンオウしょんぼリボン
		case ID_PARA_sinou_ukka_ribbon:					//シンオウうっかリボン
		case ID_PARA_sinou_sukki_ribbon:				//シンオウすっきリボン
		case ID_PARA_sinou_gussu_ribbon:				//シンオウぐっすリボン
		case ID_PARA_sinou_nikko_ribbon:				//シンオウにっこリボン
		case ID_PARA_sinou_gorgeous_ribbon:				//シンオウゴージャスリボン
		case ID_PARA_sinou_royal_ribbon:				//シンオウロイヤルリボン
		case ID_PARA_sinou_gorgeousroyal_ribbon:		//シンオウゴージャスロイヤルリボン
		case ID_PARA_sinou_ashiato_ribbon:				//シンオウあしあとリボン
		case ID_PARA_sinou_record_ribbon:				//シンオウレコードリボン
		case ID_PARA_sinou_history_ribbon:				//シンオウヒストリーリボン
		case ID_PARA_sinou_legend_ribbon:				//シンオウレジェンドリボン
		case ID_PARA_sinou_red_ribbon:					//シンオウレッドリボン
		case ID_PARA_sinou_green_ribbon:				//シンオウグリーンリボン
		case ID_PARA_sinou_blue_ribbon:					//シンオウブルーリボン
		case ID_PARA_sinou_festival_ribbon:				//シンオウフェスティバルリボン
		case ID_PARA_sinou_carnival_ribbon:				//シンオウカーニバルリボン
		case ID_PARA_sinou_classic_ribbon:				//シンオウクラシックリボン
		case ID_PARA_sinou_premiere_ribbon:				//シンオウプレミアリボン
		case ID_PARA_sinou_amari_ribbon:				//あまり
			bit=1;
			ret=((ppp1->sinou_ribbon&(bit<<(id-ID_PARA_sinou_champ_ribbon)))!=0);
			break;
//PARAM2
		case ID_PARA_waza1:
		case ID_PARA_waza2:
		case ID_PARA_waza3:
		case ID_PARA_waza4:
			ret=ppp2->waza[id-ID_PARA_waza1];
			break;
		case ID_PARA_pp1:
		case ID_PARA_pp2:
		case ID_PARA_pp3:
		case ID_PARA_pp4:
			ret=ppp2->pp[id-ID_PARA_pp1];
			break;
		case ID_PARA_pp_count1:
		case ID_PARA_pp_count2:
		case ID_PARA_pp_count3:
		case ID_PARA_pp_count4:
			ret=ppp2->pp_count[id-ID_PARA_pp_count1];
			break;
		case ID_PARA_pp_max1:
		case ID_PARA_pp_max2:
		case ID_PARA_pp_max3:
		case ID_PARA_pp_max4:
			DPW_TASSERTMSG(FALSE, "get ID_PARA_pp_max? is not implemented\n");
//			ret=WT_PPMaxGet(ppp2->waza[id-ID_PARA_pp_max1],ppp2->pp_count[id-ID_PARA_pp_max1]);
			break;
		case ID_PARA_hp_rnd:
			ret=ppp2->hp_rnd;
			break;
		case ID_PARA_pow_rnd:
			ret=ppp2->pow_rnd;
			break;
		case ID_PARA_def_rnd:
			ret=ppp2->def_rnd;
			break;
		case ID_PARA_agi_rnd:
			ret=ppp2->agi_rnd;
			break;
		case ID_PARA_spepow_rnd:
			ret=ppp2->spepow_rnd;
			break;
		case ID_PARA_spedef_rnd:
			ret=ppp2->spedef_rnd;
			break;
		case ID_PARA_tamago_flag:
			ret=ppp2->tamago_flag;
			break;
		case ID_PARA_nickname_flag:
			ret=ppp2->nickname_flag;
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
			bit=1;
			ret=((ppp2->old_ribbon&(bit<<(id-ID_PARA_stylemedal_normal)))!=0);
			break;
		case ID_PARA_event_get_flag:
			ret=ppp2->event_get_flag;
			break;
		case ID_PARA_sex:
			ret=ppp2->sex;
			break;
		case ID_PARA_form_no:
			if((ppp1->monsno==MONSNO_AUSU)&&(ppp1->speabino==TOKUSYU_MARUTITAIPU)){
				DPW_TASSERTMSG(FALSE, "get ID_PARA_form_no for MONSNO_AUSU is not implemented\n");
//				ret=AusuTypeGet(ItemParamGet(ppp1->item,ITEM_PRM_EQUIP,HEAPID_BASE_SYSTEM));
			}
			else{
				ret=ppp2->form_no;
			}
			break;
		case ID_PARA_dummy_p2_1:
			ret=ppp2->dummy_p2_1;
			break;
		case ID_PARA_dummy_p2_2:
			ret=ppp2->dummy_p2_2;
			break;
//PARAM3
		case ID_PARA_nickname:
		{
			u16	*buf16=(u16*)buf;
			for(ret=0;ret<MONS_NAME_SIZE;ret++){
				buf16[ret]=ppp3->nickname[ret];
			}
			buf16[ret]=EOM_;
		}
		break;
		case ID_PARA_nickname_buf_flag:
			ret=ppp2->nickname_flag;
		case ID_PARA_nickname_buf:
			DPW_TASSERTMSG(FALSE, "get ID_PARA_nickname_buf is not implemented\n");
//			STRBUF_SetStringCode((STRBUF*)buf, ppp3->nickname);
#if 0
			if( ppp2->nickname_flag )
			{
				STRBUF_SetStringCode((STRBUF*)buf, ppp3->nickname);
			}
			else
			{
				STRBUF*  default_monsname = MSGDAT_UTIL_GetMonsName(ppp1->monsno, HEAPID_BASE_SYSTEM);
				STRBUF_Copy((STRBUF*)buf, default_monsname);
				STRBUF_Delete(default_monsname);
			}
#endif
			break;

		case ID_PARA_pref_code:
			ret=ppp3->pref_code;
			break;
		case ID_PARA_get_cassette:
			ret=ppp3->get_cassette;
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
			bit=1;
			ret=((ppp3->new_ribbon&(bit<<(id-ID_PARA_trial_stylemedal_normal)))!=0);
			break;
//PARAM4
		case ID_PARA_oyaname:
		{
			u16 *buf16=(u16*)buf;
			for(ret=0;ret<PERSON_NAME_SIZE;ret++){
				buf16[ret]=ppp4->oyaname[ret];
			}
			buf16[ret]=EOM_;
			break;
		}

		case ID_PARA_oyaname_buf:
			DPW_TASSERTMSG(FALSE, "get ID_PARA_oyaname_buf is not implemented\n");
//			STRBUF_SetStringCode((STRBUF*)buf, ppp4->oyaname);
			break;

		case ID_PARA_get_year:							//捕まえた年
			ret=ppp4->get_year;							//捕まえた年
			break;
		case ID_PARA_get_month:							//捕まえた月
			ret=ppp4->get_month;						//捕まえた月
			break;
		case ID_PARA_get_day:							//捕まえた日
			ret=ppp4->get_day;							//捕まえた日
			break;
		case ID_PARA_birth_year:						//生まれた年
			ret=ppp4->birth_year;						//生まれた年
			break;
		case ID_PARA_birth_month:						//生まれた月
			ret=ppp4->birth_month;						//生まれた月
			break;
		case ID_PARA_birth_day:							//生まれた日
			ret=ppp4->birth_day;						//生まれた日
			break;
		case ID_PARA_get_place:							//捕まえた場所
			ret=ppp4->get_place;						//捕まえた場所
			break;
		case ID_PARA_birth_place:						//生まれた場所
			ret=ppp4->birth_place;						//生まれた場所
			break;
		case ID_PARA_pokerus:							//ポケルス
			ret=ppp4->pokerus;							//ポケルス
			break;
		case ID_PARA_get_ball:							//捕まえたボール
			ret=ppp4->get_ball;							//捕まえたボール
			break;
		case ID_PARA_get_level:							//捕まえたレベル
			ret=ppp4->get_level;						//捕まえたレベル
			break;
		case ID_PARA_oyasex:							//親の性別
			ret=ppp4->oyasex;							//親の性別
			break;
		case ID_PARA_get_ground_id:						//捕まえた場所の地形アトリビュート（ミノッチ用）
			ret=ppp4->get_ground_id;					//捕まえた場所の地形アトリビュート（ミノッチ用）
			break;
		case ID_PARA_dummy_p4_1:						//あまり
			ret=ppp4->dummy_p4_1;						//あまり
			break;

		case ID_PARA_power_rnd:
			ret=(ppp2->hp_rnd		<< 0)|
				(ppp2->pow_rnd		<< 5)|
				(ppp2->def_rnd		<<10)|
				(ppp2->agi_rnd		<<15)|
				(ppp2->spepow_rnd	<<20)|
				(ppp2->spedef_rnd	<<25);
			break;
		//モンスターナンバーがニドランの時にnickname_flagが立っているかチェック
		case ID_PARA_nidoran_nickname:
			if(((ppp1->monsno==MONSNO_NIDORAN_F)||(ppp1->monsno==MONSNO_NIDORAN_M))&&(ppp2->nickname_flag==0)){
				ret=FALSE;
			}
			else{
				ret=TRUE;
			}
			break;
		case ID_PARA_type1:
		case ID_PARA_type2:
			DPW_TASSERTMSG(FALSE, "get ID_PARA_type? is not implemented\n");
//			if((ppp1->monsno==MONSNO_AUSU)&&(ppp1->speabino==TOKUSYU_MARUTITAIPU)){
//				ret=AusuTypeGet(ItemParamGet(ppp1->item,ITEM_PRM_EQUIP,HEAPID_BASE_SYSTEM));
//			}
//			else{
//				ret=PokeFormNoPersonalParaGet(ppp1->monsno,ppp2->form_no,ID_PER_type1+(id-ID_PARA_type1));
//			}
			break;
		case ID_PARA_default_name:						//ポケモンのデフォルト名
			DPW_TASSERTMSG(FALSE, "get ID_PARA_default_name is not implemented\n");
//			MSGDAT_MonsNameGet(ppp1->monsno,HEAPID_BASE_SYSTEM,buf);
			break;
	}

	return	ret;
}

/**
 *	ボックスポケモンパラメータ構造体内のメンバのアドレスを取得
 *
 * @param[in]	ppp	取得したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	rnd	構造体入れ替えのキー
 * @param[in]	id	取り出したいメンバのインデックス（poke_tool.hに定義）
 *
 * @return	取得したアドレス
 */
static void *PokeParaAdrsGet(POKEMON_PASO_PARAM *ppp, u32 rnd, u8 id)
{
	void	*ret;
	rnd=(rnd&0x0003e000)>>13;

	DPW_TASSERTMSG(rnd<=ID_POKEPARADATA62,"PokeParaAdrsGet:Index Over!");

	switch(rnd){
	case ID_POKEPARADATA11:
	case ID_POKEPARADATA51:
		{
			POKEMON_PARA_DATA11 *PPD11=(POKEMON_PARA_DATA11 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD11->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD11->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD11->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD11->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA12:
	case ID_POKEPARADATA52:
		{
			POKEMON_PARA_DATA12 *PPD12=(POKEMON_PARA_DATA12 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD12->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD12->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD12->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD12->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA13:
	case ID_POKEPARADATA53:
		{
			POKEMON_PARA_DATA13 *PPD13=(POKEMON_PARA_DATA13 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD13->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD13->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD13->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD13->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA14:
	case ID_POKEPARADATA54:
		{
			POKEMON_PARA_DATA14 *PPD14=(POKEMON_PARA_DATA14 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD14->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD14->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD14->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD14->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA15:
	case ID_POKEPARADATA55:
		{
			POKEMON_PARA_DATA15 *PPD15=(POKEMON_PARA_DATA15 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD15->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD15->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD15->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD15->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA16:
	case ID_POKEPARADATA56:
		{
			POKEMON_PARA_DATA16 *PPD16=(POKEMON_PARA_DATA16 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD16->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD16->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD16->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD16->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA21:
	case ID_POKEPARADATA61:
		{
			POKEMON_PARA_DATA21 *PPD21=(POKEMON_PARA_DATA21 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD21->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD21->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD21->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD21->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA22:
	case ID_POKEPARADATA62:
		{
			POKEMON_PARA_DATA22 *PPD22=(POKEMON_PARA_DATA22 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD22->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD22->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD22->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD22->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA23:
		{
			POKEMON_PARA_DATA23 *PPD23=(POKEMON_PARA_DATA23 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD23->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD23->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD23->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD23->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA24:
		{
			POKEMON_PARA_DATA24 *PPD24=(POKEMON_PARA_DATA24 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD24->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD24->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD24->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD24->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA25:
		{
			POKEMON_PARA_DATA25 *PPD25=(POKEMON_PARA_DATA25 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD25->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD25->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD25->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD25->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA26:
		{
			POKEMON_PARA_DATA26 *PPD26=(POKEMON_PARA_DATA26 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD26->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD26->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD26->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD26->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA31:
		{
			POKEMON_PARA_DATA31 *PPD31=(POKEMON_PARA_DATA31 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD31->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD31->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD31->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD31->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA32:
		{
			POKEMON_PARA_DATA32 *PPD32=(POKEMON_PARA_DATA32 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD32->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD32->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD32->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD32->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA33:
		{
			POKEMON_PARA_DATA33 *PPD33=(POKEMON_PARA_DATA33 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD33->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD33->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD33->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD33->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA34:
		{
			POKEMON_PARA_DATA34 *PPD34=(POKEMON_PARA_DATA34 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD34->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD34->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD34->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD34->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA35:
		{
			POKEMON_PARA_DATA35 *PPD35=(POKEMON_PARA_DATA35 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD35->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD35->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD35->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD35->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA36:
		{
			POKEMON_PARA_DATA36 *PPD36=(POKEMON_PARA_DATA36 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD36->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD36->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD36->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD36->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA41:
		{
			POKEMON_PARA_DATA41 *PPD41=(POKEMON_PARA_DATA41 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD41->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD41->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD41->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD41->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA42:
		{
			POKEMON_PARA_DATA42 *PPD42=(POKEMON_PARA_DATA42 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD42->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD42->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD42->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD42->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA43:
		{
			POKEMON_PARA_DATA43 *PPD43=(POKEMON_PARA_DATA43 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD43->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD43->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD43->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD43->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA44:
		{
			POKEMON_PARA_DATA44 *PPD44=(POKEMON_PARA_DATA44 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD44->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD44->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD44->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD44->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA45:
		{
			POKEMON_PARA_DATA45 *PPD45=(POKEMON_PARA_DATA45 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD45->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD45->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD45->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD45->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA46:
		{
			POKEMON_PARA_DATA46 *PPD46=(POKEMON_PARA_DATA46 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD46->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD46->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD46->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD46->ppp4;
					break;
			}
			break;
		}
	}
	return	ret;
}

/**
 *	ポケモンナンバー、経験値からポケモンのレベルを計算
 *	（内部でパーソナルデータをロードする）
 *
 * @param[in]	mons_no	取得したいポケモンナンバー
 * @param[in]	exp		取得したいポケモンの経験値
 *
 * @return	取得したレベル
 */
static u32 PokeLevelCalc(u16 mons_no, u32 exp)
{
	u32	level;

	POKEMON_PERSONAL_DATA* personalData = PokePersonalPara_Open(mons_no);

	level = PokePersonal_LevelCalc( personalData, mons_no, exp );

	PokePersonalPara_Close( personalData );

	return level;
}

/**
 * パーソナルデータオープン
 *
 * この関数でオープンしたデータをPokePersonalPara_Getで取得します
 * PokePersonalPara_Closeで解放
 *
 * @param   mons_no		
 * @param   HeapID		
 *
 * @retval  POKEMON_PERSONAL_DATA *		
 */
static POKEMON_PERSONAL_DATA *PokePersonalPara_Open(int mons_no)
{
	POKEMON_PERSONAL_DATA *ppd;
	
	ppd = (POKEMON_PERSONAL_DATA*)malloc(sizeof(POKEMON_PERSONAL_DATA));
	PokePersonalDataGet(mons_no, ppd);
	
	return ppd;
}

/**
 * パーソナルデータ解放
 *
 * PokePersonalPara_Openで確保したメモリを解放します
 *
 * @param   ppd		
 *
 * @retval  none		
 */
static void PokePersonalPara_Close(POKEMON_PERSONAL_DATA *ppd)
{
	DPW_NULL_TASSERT( ppd );

	free(ppd);
}

/**
 *	ポケモンパーソナル構造体データを取得
 *
 * @param[in]	mons_no	取得したいポケモンナンバー
 * @param[out]	ppd		取得したパーソナルデータの格納先を指定
 */
static void PokePersonalDataGet(int mons_no, POKEMON_PERSONAL_DATA *ppd)
{
	ArchiveDataLoad(ppd, "personal.narc", mons_no);
}

/**
 * アーカイブファイルデータロード
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	file		読み込むアーカイブファイルのファイル名
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 */
static void ArchiveDataLoad(void *data, const char* file, int datID)
{
	ArchiveDataLoadIndex(data, file, datID, OFS_NO_SET, SIZE_NO_SET);
}

/**
 *
 *	nnsarcで作成したアーカイブファイルに対してIndexを指定して任意のデータを取り出す
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	index		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	ofs_size	読み込むデータサイズ
 *
 */
static void ArchiveDataLoadIndex(void *data, const char *name, int index, int ofs, int ofs_size)
{
	FILE*		p_file;
	u32			size=0;
	u32			fat_top=0;
	u32			fnt_top=0;
	u32			img_top=0;
	u32			top=0;
	u32			bottom=0;
	u16			file_cnt=0;

	p_file = fopen(name, "rb");
	DPW_NULL_TASSERT(p_file);

	fseek(p_file, ARC_HEAD_SIZE_POS, SEEK_SET);			// アーカイブヘッダのサイズ格納位置に移動
	fread(&size, 2, 1, p_file);							// アーカイブヘッダサイズをロード
	fat_top = size;

	fseek(p_file, fat_top + SIZE_OFFSET, SEEK_SET);		// FATのサイズ格納位置に移動
	fread(&size, 4, 1, p_file);							// FATサイズをロード
	fread(&file_cnt, 2, 1, p_file);							// FileCountをロード
	DPW_TASSERTMSG(file_cnt > index, "ServerArchiveDataLoadIndex file=%s, fileCnt=%d, index=%d", name, file_cnt, index);
	fnt_top = fat_top + size;

	fseek(p_file, fnt_top + SIZE_OFFSET, SEEK_SET);		// FNTのサイズ格納位置に移動
	fread(&size, 4, 1, p_file);							// FNTサイズをロード
	img_top = fnt_top + size;

	fseek(p_file, fat_top + FAT_HEAD_SIZE + index*8, SEEK_SET);	// 取り出したいFATテーブルに移動
	fread(&top, 4, 1, p_file);									// FATテーブルtopをロード
	fread(&bottom, 4, 1, p_file);								// FATテーブルbottomをロード

	fseek(p_file, img_top + IMG_HEAD_SIZE + top + ofs, SEEK_SET);	// 取り出したいIMGの先頭に移動
	
	if (ofs_size) {
		size = ofs_size;
	} else{
		size = bottom - top;
	}

	DPW_TASSERTMSG(size!=0,"ServerArchiveDataLoadIndex:ReadDataSize=0!");

	fread(&data, size, 1, p_file);								// FATテーブルbottomをロード

	fclose(p_file);
}

/**
 *	ポケモンナンバー、経験値からポケモンのレベルを計算
 *	（ロード済みパーソナルデータを利用する）
 *
 * @param[in]	personalData	ポケモンパーソナルデータ
 * @param[in]	mons_no			ポケモンナンバー
 * @param[in]	exp				経験値
 *
 * @return	取得したレベル
 */
static u32 PokePersonal_LevelCalc(POKEMON_PERSONAL_DATA* personalData, u16 monsno, u32 exp)
{
	static u32 grow_tbl[101];
	int grow, level;

	grow = PokePersonalPara_Get( personalData, ID_PER_grow );
	PokeGrowDataGet( grow, grow_tbl );

	for(level=1;level<101;level++){
		if(grow_tbl[level]>exp) break;
	}

	return level-1;
}

/**
 * パーソナルデータ取得
 *
 * PokePersonalPara_Openで関数で準備してからデータを取得します
 * PokePersonalPara_Closeで解放
 *
 * @param   ppd		
 * @param   para		
 *
 * @retval  u32		
 */
u32 PokePersonalPara_Get(POKEMON_PERSONAL_DATA *ppd, int para)
{
	u32 ret;

	DPW_NULL_TASSERT( ppd );
	
	switch(para){
	case ID_PER_basic_hp:		//基本ＨＰ
		ret=ppd->basic_hp;
		break;
	case ID_PER_basic_pow:		//基本攻撃力
		ret=ppd->basic_pow;
		break;
	case ID_PER_basic_def:		//基本防御力
		ret=ppd->basic_def;
		break;
	case ID_PER_basic_agi:		//基本素早さ
		ret=ppd->basic_agi;
		break;
	case ID_PER_basic_spepow:	//基本特殊攻撃力
		ret=ppd->basic_spepow;
		break;
	case ID_PER_basic_spedef:	//基本特殊防御力
		ret=ppd->basic_spedef;
		break;
	case ID_PER_type1:			//属性１
		ret=ppd->type1;
		break;
	case ID_PER_type2:			//属性２
		ret=ppd->type2;
		break;
	case ID_PER_get_rate:		//捕獲率
		ret=ppd->get_rate;
		break;
	case ID_PER_give_exp:		//贈与経験値
		ret=ppd->give_exp;
		break;
	case ID_PER_pains_hp:		//贈与努力値ＨＰ
		ret=ppd->pains_hp;
		break;
	case ID_PER_pains_pow:		//贈与努力値攻撃力
		ret=ppd->pains_pow;
		break;
	case ID_PER_pains_def:		//贈与努力値防御力
		ret=ppd->pains_def;
		break;
	case ID_PER_pains_agi:		//贈与努力値素早さ
		ret=ppd->pains_agi;
		break;
	case ID_PER_pains_spepow:	//贈与努力値特殊攻撃力
		ret=ppd->pains_spepow;
		break;
	case ID_PER_pains_spedef:	//贈与努力値特殊防御力
		ret=ppd->pains_spedef;
		break;
	case ID_PER_item1:			//アイテム１
		ret=ppd->item1;
		break;
	case ID_PER_item2:			//アイテム２
		ret=ppd->item2;
		break;
	case ID_PER_sex:			//性別ベクトル
		ret=ppd->sex;
		break;
	case ID_PER_egg_birth:		//タマゴの孵化歩数
		ret=ppd->egg_birth;
		break;
	case ID_PER_friend:			//なつき度初期値
		ret=ppd->_friend;
		break;
	case ID_PER_grow:			//成長曲線識別
		ret=ppd->grow;
		break;
	case ID_PER_egg_group1:		//こづくりグループ1
		ret=ppd->egg_group1;
		break;
	case ID_PER_egg_group2:		//こづくりグループ2
		ret=ppd->egg_group2;
		break;
	case ID_PER_speabi1:		//特殊能力１
		ret=ppd->speabi1;
		break;
	case ID_PER_speabi2:		//特殊能力２
		ret=ppd->speabi2;
		break;
	case ID_PER_escape:			//逃げる率
		ret=ppd->escape;
		break;
	case ID_PER_color:			//色（図鑑で使用）
		ret=ppd->color;
		break;
	case ID_PER_reverse:		//反転フラグ
		ret=ppd->reverse;
		break;
	case ID_PER_machine1:		//技マシンフラグ１
		ret=ppd->machine1;
		break;
	case ID_PER_machine2:		//技マシンフラグ２
		ret=ppd->machine2;
		break;
	case ID_PER_machine3:		//技マシンフラグ３
		ret=ppd->machine3;
		break;
	case ID_PER_machine4:		//技マシンフラグ４
		ret=ppd->machine4;
		break;
	}
	return ret;
}

/**
 *	ポケモン成長テーブルデータを取得
 *
 * @param[in]	para		取得する成長テーブルのインデックス（0～7）
 * @param[out]	grow_tbl	取得した成長テーブルの格納先
 */
static void PokeGrowDataGet(int para, u32 *grow_tbl)
{
	DPW_TASSERTMSG(para < 8, "PokeGrowDataGet:TableIndexOver!");
	ArchiveDataLoad(grow_tbl, "growtbl.narc", para);
}

/**
 *	ポケモンパラメータ構造体に任意で値を格納
 *
 * @param[in]	pp	格納したいポケモンパラメータ構造体のポインタ
 * @param[in]	id	格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	buf	格納したいデータのポインタ
 */
static void PokeParaPutAct(POKEMON_PARAM *pp, int id, const void *buf)
{
	u32	*buf32=(u32 *)buf;
	u16	*buf16=(u16 *)buf;
	u8	*buf8=(u8 *)buf;

	switch(id){
	case ID_PARA_condition:
		pp->pcp.condition=buf32[0];
		break;
	case ID_PARA_level:
		pp->pcp.level=buf8[0];
		break;
	case ID_PARA_cb_id:
		pp->pcp.cb_id=buf8[0];	
		break;
	case ID_PARA_hp:
		pp->pcp.hp=buf16[0];
		break;
	case ID_PARA_hpmax:
		pp->pcp.hpmax=buf16[0];
		break;
	case ID_PARA_pow:
		pp->pcp.pow=buf16[0];
		break;
	case ID_PARA_def:
		pp->pcp.def=buf16[0];
		break;
	case ID_PARA_agi:
		pp->pcp.agi=buf16[0];
		break;
	case ID_PARA_spepow:
		pp->pcp.spepow=buf16[0];
		break;
	case ID_PARA_spedef:
		pp->pcp.spedef=buf16[0];
		break;
	case ID_PARA_mail_data:
		memcpy(&pp->pcp.mail_data, buf, sizeof(MAIL_DATA));
		break;
	case ID_PARA_cb_core:
		memcpy(&pp->pcp.cb_core, buf, sizeof(CB_CORE));
		break;
	default:
		PokePasoParaPutAct((POKEMON_PASO_PARAM *)&pp->ppp,id,buf);
		break;
	}
}

/**
 *	ボックスポケモンパラメータ構造体に任意で値を格納
 *
 * @param[in]	ppp	格納したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id	格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	buf	格納したいデータのポインタ
 */
static void PokePasoParaPutAct(POKEMON_PASO_PARAM *ppp, int id, const void *buf)
{
	int	i;
	u64	bit;
	u32	*buf32=(u32 *)buf;
	u16	*buf16=(u16 *)buf;
	u8	*buf8=(u8 *)buf;
	POKEMON_PASO_PARAM1	*ppp1;
	POKEMON_PASO_PARAM2	*ppp2;
	POKEMON_PASO_PARAM3	*ppp3;
	POKEMON_PASO_PARAM4	*ppp4;

	ppp1=(POKEMON_PASO_PARAM1 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA1);
	ppp2=(POKEMON_PASO_PARAM2 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA2);
	ppp3=(POKEMON_PASO_PARAM3 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA3);
	ppp4=(POKEMON_PASO_PARAM4 *)PokeParaAdrsGet(ppp,ppp->personal_rnd,ID_POKEPARA4);

	switch(id){
//PARAM
		case ID_PARA_personal_rnd:
			ppp->personal_rnd=buf32[0];
			break;
		case ID_PARA_pp_fast_mode:
			DPW_TASSERTMSG(FALSE, "fast_modeへ不正な書き込み\n");
			ppp->pp_fast_mode=buf8[0];
			break;
		case ID_PARA_ppp_fast_mode:
			DPW_TASSERTMSG(FALSE, "fast_modeへ不正な書き込み\n");
			ppp->ppp_fast_mode=buf8[0];
			break;
		case ID_PARA_checksum:
			ppp->checksum=buf16[0];
			break;
//PARAM1
		case ID_PARA_monsno:
			ppp1->monsno=buf16[0];
			break;
		case ID_PARA_item:
			ppp1->item=buf16[0];
			break;
		case ID_PARA_id_no:
			ppp1->id_no=buf32[0];
			break;
		case ID_PARA_exp:
			ppp1->exp=buf32[0];
			break;
		case ID_PARA_friend:
			ppp1->_friend=buf8[0];
			break;
		case ID_PARA_speabino:
			ppp1->speabino=buf8[0];
			break;
		case ID_PARA_mark:
			ppp1->mark=buf8[0];		
			break;
		case ID_PARA_country_code:
			ppp1->country_code=buf8[0];		
			break;
		case ID_PARA_hp_exp:
			ppp1->hp_exp=buf8[0];
			break;
		case ID_PARA_pow_exp:
			ppp1->pow_exp=buf8[0];
			break;
		case ID_PARA_def_exp:
			ppp1->def_exp=buf8[0];
			break;
		case ID_PARA_agi_exp:
			ppp1->agi_exp=buf8[0];
			break;
		case ID_PARA_spepow_exp:
			ppp1->spepow_exp=buf8[0];
			break;
		case ID_PARA_spedef_exp:
			ppp1->spedef_exp=buf8[0];
			break;
		case ID_PARA_style:
			ppp1->style=buf8[0];
			break;
		case ID_PARA_beautiful:
			ppp1->beautiful=buf8[0];
			break;
		case ID_PARA_cute:
			ppp1->cute=buf8[0];
			break;
		case ID_PARA_clever:
			ppp1->clever=buf8[0];
			break;
		case ID_PARA_strong:
			ppp1->strong=buf8[0];	
			break;
		case ID_PARA_fur:
			ppp1->fur=buf8[0];		
			break;
		case ID_PARA_sinou_champ_ribbon:				//シンオウチャンプリボン
		case ID_PARA_sinou_battle_tower_ttwin_first:	//シンオウバトルタワータワータイクーン勝利1回目
		case ID_PARA_sinou_battle_tower_ttwin_second:	//シンオウバトルタワータワータイクーン勝利2回目
		case ID_PARA_sinou_battle_tower_2vs2_win50:		//シンオウバトルタワータワーダブル50連勝
		case ID_PARA_sinou_battle_tower_aimulti_win50:	//シンオウバトルタワータワーAIマルチ50連勝
		case ID_PARA_sinou_battle_tower_siomulti_win50:	//シンオウバトルタワータワー通信マルチ50連勝
		case ID_PARA_sinou_battle_tower_wifi_rank5:		//シンオウバトルタワーWifiランク５入り
		case ID_PARA_sinou_syakki_ribbon:				//シンオウしゃっきリボン
		case ID_PARA_sinou_dokki_ribbon:				//シンオウどっきリボン
		case ID_PARA_sinou_syonbo_ribbon:				//シンオウしょんぼリボン
		case ID_PARA_sinou_ukka_ribbon:					//シンオウうっかリボン
		case ID_PARA_sinou_sukki_ribbon:				//シンオウすっきリボン
		case ID_PARA_sinou_gussu_ribbon:				//シンオウぐっすリボン
		case ID_PARA_sinou_nikko_ribbon:				//シンオウにっこリボン
		case ID_PARA_sinou_gorgeous_ribbon:				//シンオウゴージャスリボン
		case ID_PARA_sinou_royal_ribbon:				//シンオウロイヤルリボン
		case ID_PARA_sinou_gorgeousroyal_ribbon:		//シンオウゴージャスロイヤルリボン
		case ID_PARA_sinou_ashiato_ribbon:				//シンオウあしあとリボン
		case ID_PARA_sinou_record_ribbon:				//シンオウレコードリボン
		case ID_PARA_sinou_history_ribbon:				//シンオウヒストリーリボン
		case ID_PARA_sinou_legend_ribbon:				//シンオウレジェンドリボン
		case ID_PARA_sinou_red_ribbon:					//シンオウレッドリボン
		case ID_PARA_sinou_green_ribbon:				//シンオウグリーンリボン
		case ID_PARA_sinou_blue_ribbon:					//シンオウブルーリボン
		case ID_PARA_sinou_festival_ribbon:				//シンオウフェスティバルリボン
		case ID_PARA_sinou_carnival_ribbon:				//シンオウカーニバルリボン
		case ID_PARA_sinou_classic_ribbon:				//シンオウクラシックリボン
		case ID_PARA_sinou_premiere_ribbon:				//シンオウプレミアリボン
		case ID_PARA_sinou_amari_ribbon:				//あまり
			bit=buf8[0];
			bit=bit<<(id-ID_PARA_sinou_champ_ribbon);
			if(buf8[0]){
				ppp1->sinou_ribbon|=bit;
			}
			else{
				ppp1->sinou_ribbon&=(bit^0xffffffff);
			}
			break;
//PARAM2
		case ID_PARA_waza1:
		case ID_PARA_waza2:
		case ID_PARA_waza3:
		case ID_PARA_waza4:
			ppp2->waza[id-ID_PARA_waza1]=buf16[0];
			break;
		case ID_PARA_pp1:
		case ID_PARA_pp2:
		case ID_PARA_pp3:
		case ID_PARA_pp4:
			ppp2->pp[id-ID_PARA_pp1]=buf8[0];
			break;
		case ID_PARA_pp_count1:
		case ID_PARA_pp_count2:
		case ID_PARA_pp_count3:
		case ID_PARA_pp_count4:
			ppp2->pp_count[id-ID_PARA_pp_count1]=buf8[0];
			break;
		case ID_PARA_pp_max1:
		case ID_PARA_pp_max2:
		case ID_PARA_pp_max3:
		case ID_PARA_pp_max4:
			DPW_TASSERTMSG(FALSE, "不正な書き込み\n");
			break;
		case ID_PARA_hp_rnd:
			ppp2->hp_rnd=buf8[0];
			break;
		case ID_PARA_pow_rnd:
			ppp2->pow_rnd=buf8[0];
			break;
		case ID_PARA_def_rnd:
			ppp2->def_rnd=buf8[0];
			break;
		case ID_PARA_agi_rnd:
			ppp2->agi_rnd=buf8[0];
			break;
		case ID_PARA_spepow_rnd:
			ppp2->spepow_rnd=buf8[0];
			break;
		case ID_PARA_spedef_rnd:
			ppp2->spedef_rnd=buf8[0];
			break;
		case ID_PARA_tamago_flag:
			ppp2->tamago_flag=buf8[0];
			break;
		case ID_PARA_nickname_flag:
			ppp2->nickname_flag=buf8[0];
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
			bit=buf8[0];
			bit=bit<<(id-ID_PARA_stylemedal_normal);
			if(buf8[0]){
				ppp2->old_ribbon|=bit;
			}
			else{
				ppp2->old_ribbon&=(bit^0xffffffff);
			}
			break;
		case ID_PARA_event_get_flag:
			ppp2->event_get_flag=buf8[0];
			break;
		case ID_PARA_sex:
			ppp2->sex=buf8[0];
			break;
		case ID_PARA_form_no:
			ppp2->form_no=buf8[0];
			break;
		case ID_PARA_dummy_p2_1:
			ppp2->dummy_p2_1=buf8[0];
			break;
		case ID_PARA_dummy_p2_2:
			ppp2->dummy_p2_2=buf16[0];
			break;
//PARAM3
		case ID_PARA_nickname:
			for(i=0;i<NELEMS(ppp3->nickname);i++){
				ppp3->nickname[i]=buf16[i];
			}
			break;
		case ID_PARA_nickname_buf_flag:
			ppp2->nickname_flag=1;
		case ID_PARA_nickname_buf:
			DPW_TASSERTMSG(FALSE, "put ID_PARA_nickname_buf is not implemented\n");
//			STRBUF_GetStringCode((STRBUF*)buf, ppp3->nickname, NELEMS(ppp3->nickname));
			break;
		case ID_PARA_pref_code:
			ppp3->pref_code=buf8[0];
			break;
		case ID_PARA_get_cassette:
			ppp3->get_cassette=buf8[0];
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
			bit=buf8[0];
			bit=bit<<(id-ID_PARA_trial_stylemedal_normal);
			if(buf8[0]){
				ppp3->new_ribbon|=bit;
			}
			else{
				ppp3->new_ribbon&=~bit;
			}
			break;
//PARAM4
		case ID_PARA_oyaname:
			for(i=0;i<NELEMS(ppp4->oyaname);i++)
				ppp4->oyaname[i]=buf16[i];
			break;
		case ID_PARA_oyaname_buf:
			DPW_TASSERTMSG(FALSE, "put ID_PARA_oyaname_buf is not implemented\n");
//			STRBUF_GetStringCode((STRBUF*)buf, ppp4->oyaname, NELEMS(ppp4->oyaname));
			break;
		case ID_PARA_get_year:							//捕まえた年
			ppp4->get_year=buf8[0];						//捕まえた年
			break;
		case ID_PARA_get_month:							//捕まえた月
			ppp4->get_month=buf8[0];					//捕まえた月
			break;
		case ID_PARA_get_day:							//捕まえた日
			ppp4->get_day=buf8[0];						//捕まえた日
			break;
		case ID_PARA_birth_year:						//生まれた年
			ppp4->birth_year=buf8[0];					//生まれた年
			break;
		case ID_PARA_birth_month:						//生まれた月
			ppp4->birth_month=buf8[0];					//生まれた月
			break;
		case ID_PARA_birth_day:							//生まれた日
			ppp4->birth_day=buf8[0];					//生まれた日
			break;
		case ID_PARA_get_place:							//捕まえた場所
			ppp4->get_place=buf16[0];					//捕まえた場所
			break;
		case ID_PARA_birth_place:						//生まれた場所
			ppp4->birth_place=buf16[0];					//生まれた場所
			break;
		case ID_PARA_pokerus:							//ポケルス
			ppp4->pokerus=buf8[0];						//ポケルス
			break;
		case ID_PARA_get_ball:							//捕まえたボール
			ppp4->get_ball=buf8[0];						//捕まえたボール
			break;
		case ID_PARA_get_level:							//捕まえたレベル
			ppp4->get_level=buf8[0];					//捕まえたレベル
			break;
		case ID_PARA_oyasex:							//親の性別
			ppp4->oyasex=buf8[0];						//親の性別
			break;
		case ID_PARA_get_ground_id:						//捕まえた場所の地形アトリビュート（ミノッチ用）
			ppp4->get_ground_id=buf8[0];				//捕まえた場所の地形アトリビュート（ミノッチ用）
			break;
		case ID_PARA_dummy_p4_1:						//あまり
			ppp4->dummy_p4_1=buf16[0];					//あまり
			break;

		case ID_PARA_power_rnd:
			ppp2->hp_rnd	=(buf32[0]>> 0)&0x0000001f;
			ppp2->pow_rnd	=(buf32[0]>> 5)&0x0000001f;
			ppp2->def_rnd	=(buf32[0]>>10)&0x0000001f;
			ppp2->agi_rnd	=(buf32[0]>>15)&0x0000001f;
			ppp2->spepow_rnd=(buf32[0]>>20)&0x0000001f;
			ppp2->spedef_rnd=(buf32[0]>>25)&0x0000001f;
			break;
		//モンスターナンバーがニドランの時にnickname_flagが立っているかチェック
		case ID_PARA_nidoran_nickname:
		case ID_PARA_type1:
		case ID_PARA_type2:
			DPW_TASSERTMSG(FALSE, "不正な書き込み\n");
			break;
		case ID_PARA_default_name:						//ポケモンのデフォルト名
			DPW_TASSERTMSG(FALSE, "put ID_PARA_default_name is not implemented\n");
//			{
//				STRBUF	*name_buf;
//
//				name_buf=MSGDAT_UTIL_GetMonsName(ppp1->monsno, HEAPID_BASE_SYSTEM);
//				STRBUF_GetStringCode(name_buf, ppp3->nickname, NELEMS(ppp3->nickname));
//				STRBUF_Delete(name_buf);
//			}
			break;
	}
}

//==============================================================================
/**
 * @brief   ＩＤと個性乱数からレアかどうかを判定する
 *
 * @param   id		ID
 * @param   rnd		個性乱数
 *
 * @retval  u8		0:レアじゃない	0以外:レア
 */
//==============================================================================
static u8 PokeRareGetPara(u32 id,u32 rnd)
{
	return((((id&0xffff0000)>>16)^(id&0xffff)^((rnd&0xffff0000)>>16)^(rnd&0xffff))<8);
}
