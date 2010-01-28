//============================================================================================
/**
 * @file  zukan_savedata.h
 * @brief 図鑑セーブデータアクセス処理群ヘッダ
 * @author  mori / tamada GAME FREAK inc.
 * @date  2009.10.26
 *
 * ジョウト→イッシュ→？とその度に名前を書き換えるのもなんなので、
 * これからは全国は「ZENKOKU」、地方図鑑の事は「LOCAL」と記述する事にします。
 *
 */
//============================================================================================
#ifndef __ZUKAN_SAVEDATA_H__
#define __ZUKAN_SAVEDATA_H__

#include "gamesystem/gamedata_def.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#define ISSHU_MAX ( 156 )     // @TODO ISSHU_MAX定義は monsno_def.hで宣言されたら消す

//============================================================================================
//============================================================================================
enum{
  ZUKANSAVE_RANDOM_PACHI = 0,   // パッチール個性乱数

  // ポケモン性別取得用
  ZUKANSAVE_SEX_FIRST = 0,
  ZUKANSAVE_SEX_SECOND,

  // ZUKANSAVE_GetPokeSexFlag系関数のエラーコード
  ZUKANSAVE_GET_SEX_ERR = 0xffffffff,


  // 全国図鑑最大
  ZUKANSAVE_ZENKOKU_MONSMAX   = MONSNO_END,

  // イッシュ図鑑最大数
  ZUKANSAVE_LOCAL_MONSMAX = ISSHU_MAX,

  // 全国図鑑は捕まえた条件なのでマナフィーのチェックは必要になる(08.06.20)
  // ゼンコク図鑑完成に必要なポケモンの数
  ZUKANSAVE_ZENKOKU_COMP_NOPOKE_NUM = 9,  // 不必要なポケモン数
  ZUKANSAVE_ZENKOKU_COMP_NUM  = ZUKANSAVE_ZENKOKU_MONSMAX - ZUKANSAVE_ZENKOKU_COMP_NOPOKE_NUM,

  // イッシュ図鑑完成に必要なポケモンの数
  ZUKANSAVE_LOCAL_COMP_NOPOKE_NUM = 2,  // 不必要なポケモン数
  ZUKANSAVE_LOCAL_COMP_NUM  = ZUKANSAVE_LOCAL_MONSMAX - ZUKANSAVE_LOCAL_COMP_NOPOKE_NUM,



  // シーウシ　シードルゴ シェイミ　ギラティナの形状数
  ZUKANSAVE_UMIUSHI_FORM_NUM = 2, //(ここを変えただけではセーブ領域の方式やワークサイズは変わらないので、形状数が変わったときは内部も変更すること)

  // ミノムッチ　ミノメス　ピチュー　形状数
  ZUKANSAVE_MINOMUSHI_FORM_NUM = 3, //(ここを変えただけではセーブ領域の方式やワークサイズは変わらないので、形状数が変わったときは内部も変更すること)

  // デオキシスの形状数
  ZUKANSAVE_DEOKISISU_FORM_NUM = 4,
  ZUKANSAVE_DEOKISISU_FORM_SAVEDATA_CUTNUM = 2, // デオキシスセーブ領域は、急遽　捕獲見たフラグのけつ1byteずづに割り振っています。各1byteにセーブできる形状の数です
  POKEZUKAN_DEOKISISU_INIT = 0xf, // 初期化時格納されている値

  // ロトムの形状数
  ZUKANSAVE_ROTOM_FORM_NUM = 6, // (ここを変えただけではセーブ領域の方式やワークサイズは変わらないので、形状数が変わったときは内部も変更すること)

  // ポワルンの形状数（セーブデータには関係ありません）
  ZUKANSAVE_POWARUN_FORM_NUM = 4,

  // チェリムの形状数（セーブデータには関係ありません）
  ZUKANSAVE_CHERIMU_FORM_NUM = 2,
};

//----------------------------------------------------------
// 非公開型 構造体宣言
//----------------------------------------------------------
typedef struct ZUKAN_SAVEDATA ZUKAN_SAVEDATA;

//----------------------------------------------------------
//  セーブデータ取得のための関数（GAMEDATA経由）
//----------------------------------------------------------
extern ZUKAN_SAVEDATA * GAMEDATA_GetZukanSave( GAMEDATA * gamedata );


//----------------------------------------------------------
//  セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int ZUKANSAVE_GetWorkSize(void);
extern ZUKAN_SAVEDATA * ZUKANSAVE_AllocWork( HEAPID heapID );


//----------------------------------------------------------
//----------------------------------------------------------
extern void ZUKANSAVE_Init(ZUKAN_SAVEDATA * zs);

extern u16 ZUKANSAVE_GetPokeGetCount(const ZUKAN_SAVEDATA * zs);
extern u16 ZUKANSAVE_GetPokeSeeCount(const ZUKAN_SAVEDATA * zs);

extern u16 ZUKANSAVE_GetLocalPokeGetCount(const ZUKAN_SAVEDATA * zs);
extern u16 ZUKANSAVE_GetLocalPokeSeeCount(const ZUKAN_SAVEDATA * zs);

// ポケモン見た登録・ポケモン捕まえた登録
extern void ZUKANSAVE_SetPokeSee(ZUKAN_SAVEDATA * zs, const POKEMON_PARAM * pp);
extern void ZUKANSAVE_SetPokeGet(ZUKAN_SAVEDATA * zs, const POKEMON_PARAM * pp);

// ゼンコク図鑑持っているときー＞全国用の各数字を返す
// イッシュ図鑑しかもっていないときー＞イッシュ図鑑用の各数字を返す
extern u16 ZUKANSAVE_GetZukanPokeGetCount(const ZUKAN_SAVEDATA * zs);
extern u16 ZUKANSAVE_GetZukanPokeSeeCount(const ZUKAN_SAVEDATA * zs);


// ゼンコク図鑑　イッシュ図鑑
// 完成に必要なポケモンだけでカウントした値を取得
extern u16 ZUKANSAVE_GetZenkokuGetCompCount(const ZUKAN_SAVEDATA * zs);
extern u16 ZUKANSAVE_GetLocalGetCompCount(const ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_CheckZenkokuComp(const ZUKAN_SAVEDATA * zs);
extern BOOL ZUKANSAVE_CheckLocalComp(const ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_GetPokeGetFlag(const ZUKAN_SAVEDATA * zs, u16 monsno);
extern BOOL ZUKANSAVE_GetPokeSeeFlag(const ZUKAN_SAVEDATA * zs, u16 monsno);
extern u32 ZUKANSAVE_GetPokeSexFlag(const ZUKAN_SAVEDATA * zw, u16 monsno, int first_second, HEAPID heapId );
extern u32 ZUKANSAVE_GetPokeAnnoonForm(const ZUKAN_SAVEDATA * zs, int count, BOOL get_f );
extern u32 ZUKANSAVE_GetPokeAnnoonNum(const ZUKAN_SAVEDATA * zs, BOOL get_f);
extern u32 ZUKANSAVE_GetPokeSiiusiForm(const ZUKAN_SAVEDATA * zs, int count);
extern u32 ZUKANSAVE_GetPokeSiiusiSeeNum(const ZUKAN_SAVEDATA * zs);
extern u32 ZUKANSAVE_GetPokeSiidorugoForm(const ZUKAN_SAVEDATA * zs, int count);
extern u32 ZUKANSAVE_GetPokeSiidorugoSeeNum(const ZUKAN_SAVEDATA * zs);
extern u32 ZUKANSAVE_GetPokeMinomuttiForm(const ZUKAN_SAVEDATA * zs, int count);
extern u32 ZUKANSAVE_GetPokeMinomuttiSeeNum(const ZUKAN_SAVEDATA * zs);
extern u32 ZUKANSAVE_GetPokeMinomesuForm(const ZUKAN_SAVEDATA * zs, int count);
extern u32 ZUKANSAVE_GetPokeMinomesuSeeNum(const ZUKAN_SAVEDATA * zs);
extern u32 ZUKANSAVE_GetPokePityuuForm(const ZUKAN_SAVEDATA * zs, int count);
extern u32 ZUKANSAVE_GetPokePityuuSeeNum(const ZUKAN_SAVEDATA * zs);
extern u32 ZUKANSAVE_GetPokeRandomFlag(const ZUKAN_SAVEDATA * zs, u8 random_poke);
extern u32 ZUKANSAVE_GetPokeDeokisisuForm(const ZUKAN_SAVEDATA * zs, int count);
extern u32 ZUKANSAVE_GetPokeDeokisisuFormSeeNum(const ZUKAN_SAVEDATA * zs);
extern u32 ZUKANSAVE_GetPokeForm( const ZUKAN_SAVEDATA* zs, int monsno, int count );
extern u32 ZUKANSAVE_GetPokeFormNum( const ZUKAN_SAVEDATA* zs, int monsno );

extern BOOL ZUKANSAVE_GetZukanGetFlag(const ZUKAN_SAVEDATA * zs);
extern void ZUKANSAVE_SetZukanGetFlag(ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_GetZenkokuZukanFlag(const ZUKAN_SAVEDATA * zs);
extern void ZUKANSAVE_SetZenkokuZukanFlag(ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_GetGraphicVersionUpFlag(const ZUKAN_SAVEDATA * zs);
extern void ZUKANSAVE_SetGraphicVersionUpFlag(ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_GetTextVersionUpFlag(const ZUKAN_SAVEDATA * zs, u16 monsno, u32 country_code);

extern void ZUKANSAVE_SetTextVersionUpMasterFlag( ZUKAN_SAVEDATA * zs );
extern BOOL ZUKANSAVE_GetTextVersionUpMasterFlag(const ZUKAN_SAVEDATA * zs);

extern void ZUKANSAVE_Copy(const ZUKAN_SAVEDATA * from, ZUKAN_SAVEDATA * to);


//----------------------------------------------------------
//  デバッグ用
//----------------------------------------------------------
extern void Debug_ZUKANSAVE_Make(ZUKAN_SAVEDATA * zs, int start, int end, BOOL see_flg, HEAPID heapId);
extern void Debug_ZUKANSAVE_Make_PM_LANG(ZUKAN_SAVEDATA * zs, int start, int end, BOOL see_flg, HEAPID heapId);
extern void Debug_ZUKANSAVE_LangSetRand( ZUKAN_SAVEDATA * zs, int start, int end );
extern void Debug_ZUKANSAVE_LangSetAll( ZUKAN_SAVEDATA * zs, int start, int end );
extern void Debug_ZUKANSAVE_LangSet( ZUKAN_SAVEDATA * zs, int start, int end, u8 lang );
extern void Debug_ZUKANSAVE_AnnoonGetSet( ZUKAN_SAVEDATA * zs, int start, int end, HEAPID heapId );
extern void Debug_ZUKANSAVE_DeokisisuBuckUp( ZUKAN_SAVEDATA * zs );

#ifdef CREATE_INDEX
extern void *Index_Get_Zukan_Offset(ZUKAN_SAVEDATA *zs);
extern void *Index_Get_Zenkoku_Zukan_Offset(ZUKAN_SAVEDATA *zs);
extern void *Index_Get_Get_Flag_Offset(ZUKAN_SAVEDATA *zs);
extern void *Index_Get_See_Flag_Offset(ZUKAN_SAVEDATA *zs);
extern void *Index_Get_Sex_Flag_Offset(ZUKAN_SAVEDATA *zs);
#endif

#endif  // __ZUKAN_SAVEADATA_H__
