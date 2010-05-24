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

#define ISSHU_MAX ( ZUKANNO_ISSHU_END+1 )     // イッシュ図鑑数（ビクティ分＋１）

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

  // ゼンコク図鑑完成に必要なポケモンの数
  ZUKANSAVE_ZENKOKU_COMP_NOPOKE_NUM = 15,  // 不必要なポケモン数
  ZUKANSAVE_ZENKOKU_COMP_NUM  = ZUKANSAVE_ZENKOKU_MONSMAX - ZUKANSAVE_ZENKOKU_COMP_NOPOKE_NUM,

  // イッシュ図鑑完成に必要なポケモンの数
  ZUKANSAVE_LOCAL_COMP_NOPOKE_NUM = 6,  // 不必要なポケモン数
  ZUKANSAVE_LOCAL_COMP_NUM  = ZUKANSAVE_LOCAL_MONSMAX - ZUKANSAVE_LOCAL_COMP_NOPOKE_NUM,
};

//----------------------------------------------------------
// 非公開型 構造体宣言
//----------------------------------------------------------
typedef struct ZUKAN_SAVEDATA ZUKAN_SAVEDATA;

//----------------------------------------------------------
/**
 * @brief		図鑑セーブデータ取得（セーブデータから）
 *
 * @param		sv      セーブデータ保持ワークへのポインタ
 *
 * @return  図鑑セーブデータ
 *
 * @li	通常は使用しないでください！
 */
//----------------------------------------------------------
extern ZUKAN_SAVEDATA * ZUKAN_SAVEDATA_GetZukanSave( SAVE_CONTROL_WORK * sv );

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

extern u16 ZUKANSAVE_GetLocalPokeGetCount( const ZUKAN_SAVEDATA * zs, HEAPID heapID );
extern u16 ZUKANSAVE_GetLocalPokeSeeCount( const ZUKAN_SAVEDATA * zs, HEAPID heapID );

// ポケモン見た登録・ポケモン捕まえた登録
extern void ZUKANSAVE_SetPokeSee(ZUKAN_SAVEDATA * zs, POKEMON_PARAM * pp);
extern void ZUKANSAVE_SetPokeGet(ZUKAN_SAVEDATA * zs, POKEMON_PARAM * pp);

// ゼンコク図鑑持っているときー＞全国用の各数字を返す
// イッシュ図鑑しかもっていないときー＞イッシュ図鑑用の各数字を返す
extern u16 ZUKANSAVE_GetZukanPokeGetCount( const ZUKAN_SAVEDATA * zs, HEAPID heapID );
extern u16 ZUKANSAVE_GetZukanPokeSeeCount( const ZUKAN_SAVEDATA * zs, HEAPID heapID );


// ゼンコク図鑑　イッシュ図鑑
// 完成に必要なポケモンだけでカウントした値を取得
extern u16 ZUKANSAVE_GetZenkokuGetCompCount(const ZUKAN_SAVEDATA * zs);
extern u16 ZUKANSAVE_GetLocalGetCompCount( const ZUKAN_SAVEDATA * zs, HEAPID heapID );
extern u16 ZUKANSAVE_GetLocalSeeCompCount( const ZUKAN_SAVEDATA * zs, HEAPID heapID );

extern BOOL ZUKANSAVE_CheckZenkokuComp(const ZUKAN_SAVEDATA * zs);
extern BOOL ZUKANSAVE_CheckLocalGetComp( const ZUKAN_SAVEDATA * zs, HEAPID heapID );
extern BOOL ZUKANSAVE_CheckLocalSeeComp( const ZUKAN_SAVEDATA * zs, HEAPID heapID );

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


extern BOOL ZUKANSAVE_GetZenkokuZukanFlag(const ZUKAN_SAVEDATA * zs);
extern void ZUKANSAVE_SetZenkokuZukanFlag(ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_GetGraphicVersionUpFlag(const ZUKAN_SAVEDATA * zs);
extern void ZUKANSAVE_SetGraphicVersionUpFlag(ZUKAN_SAVEDATA * zs);

extern BOOL ZUKANSAVE_GetTextVersionUpFlag(const ZUKAN_SAVEDATA * zs, u16 monsno, u32 country_code);

extern void ZUKANSAVE_SetTextVersionUpMasterFlag( ZUKAN_SAVEDATA * zs );
extern BOOL ZUKANSAVE_GetTextVersionUpMasterFlag(const ZUKAN_SAVEDATA * zs);

extern void ZUKANSAVE_Copy(const ZUKAN_SAVEDATA * from, ZUKAN_SAVEDATA * to);


extern void ZUKANSAVE_SetZukanMode( ZUKAN_SAVEDATA * zw, BOOL mode );
extern BOOL ZUKANSAVE_GetZukanMode( const ZUKAN_SAVEDATA * zw );
extern void ZUKANSAVE_SetDefaultMons( ZUKAN_SAVEDATA * zw, u16 mons );
extern u16 ZUKANSAVE_GetDefaultMons( const ZUKAN_SAVEDATA * zw );
//extern void ZUKANSAVE_SetShortcutMons( ZUKAN_SAVEDATA * zw, u16 mons );
//extern u16 ZUKANSAVE_GetShortcutMons( const ZUKAN_SAVEDATA * zw );
extern u32	ZUKANSAVE_GetFormMax( u16 mons );
extern BOOL ZUKANSAVE_CheckPokeSeeForm( const ZUKAN_SAVEDATA * zw, u16 monsno, int sex, int rare, int form );
extern void ZUKANSAVE_SetDrawData( ZUKAN_SAVEDATA * zw, u16 mons, u32 sex, BOOL rare, u32 form );
extern void ZUKANSAVE_GetDrawData( ZUKAN_SAVEDATA * zw, u16 mons, u32 * sex, BOOL * rare, u32 * form, HEAPID heapID );


//----------------------------------------------------------
//  デバッグ用
//----------------------------------------------------------
#ifdef	PM_DEBUG
extern void ZUKANSAVE_DebugDataClear( ZUKAN_SAVEDATA * zw, u16 start, u16 end );
extern void ZUKANSAVE_DebugDataSetSee( ZUKAN_SAVEDATA * zw, u16 start, u16 end, HEAPID heapID );
extern void ZUKANSAVE_DebugDataSetGet( ZUKAN_SAVEDATA * zw, u16 start, u16 end, HEAPID heapID );
#endif	// PM_DEBUG

#endif  // __ZUKAN_SAVEADATA_H__
