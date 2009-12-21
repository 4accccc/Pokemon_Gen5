//=============================================================================================
/**
 * @file  btl_util.h
 * @brief ポケモンWB バトルシステム 共有ツール群
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================
#pragma once


#include "poke_tool/poketype.h"
#include "waza_tool/wazadata.h"
#include "btl_const.h"
#include "btl_common.h"

//===================================================================
// 単純なプロセス処理構造
//===================================================================

typedef BOOL (*BPFunc)(int*, void*);

typedef struct {
  BPFunc  initFunc;
  BPFunc  loopFunc;
  void* work;
  int   seq;
}BTL_PROC;


static inline void BTL_UTIL_ClearProc( BTL_PROC* proc )
{
  proc->initFunc  = NULL;
  proc->loopFunc  = NULL;
  proc->work    = NULL;
  proc->seq   = 0;
}

static inline BOOL BTL_UTIL_ProcIsCleared( const BTL_PROC* proc )
{
  return (proc->initFunc==NULL) && (proc->loopFunc==NULL);
}

static inline void BTL_UTIL_SetupProc( BTL_PROC* proc, void* work, BPFunc init, BPFunc loop )
{
  proc->initFunc = init;
  proc->loopFunc = loop;
  proc->work = work;
  proc->seq = 0;
}

static inline BOOL BTL_UTIL_CallProc( BTL_PROC* proc )
{
  if( proc->initFunc )
  {
    if( proc->initFunc( &(proc->seq), proc->work ) )
    {
      proc->initFunc = NULL;
      proc->seq = 0;
    }
    return FALSE;
  }
  else if( proc->loopFunc )
  {
    if( proc->loopFunc( &(proc->seq), proc->work ) )
    {
      proc->loopFunc = NULL;
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}

//===================================================================
// 状態異常等の継続パラメータ
//===================================================================

//--------------------------------------------------------------
/**
 *  状態異常継続パラメータ
 */
//--------------------------------------------------------------
typedef struct  {

  union {
    u32    raw;
    struct {
      u32  type : 4;
      u32  _0   : 28;
    };
    struct {
      u32  type_turn : 4;   ///< ターン数型
      u32  count     : 6;   ///< 有効ターン数
      u32  param     : 16;  ///< 汎用パラメータ16bit
      u32  flag      : 1;   ///< 汎用フラグ 1bit
      u32  _1        : 5;
    }turn;
    struct {
      u32  type_poke : 4;   ///< ポケ依存型
      u32  ID        : 6;   ///< 対象ポケID
      u32  param     : 16;  ///< 汎用パラメータ16bit
      u32  flag      : 1;   ///< 汎用フラグ 1bit
      u32  _2        : 5;
    }poke;
    struct {
      u32  type_perm : 4;   ///< 永続型
      u32  count_max : 6;   ///< ターンカウントアップ最大数
      u32  param     : 16;  ///< 汎用パラメータ16bit
      u32  flag      : 1;   ///< 汎用フラグ 1bit
      u32  _3        : 5;
    }permanent;
    struct {
      u32 type_poketurn : 4;  ///< ターン数・ポケ依存混合型
      u32 count         : 6;  ///< 有効ターン数
      u32 pokeID        : 6;  ///< 対象ポケID
      u32 param         : 16; ///< 汎用パラメータ16bit
      u32 flag          : 1;  ///< 汎用フラグ 1bit
    }poketurn;
  };

}BPP_SICK_CONT;


static inline BPP_SICK_CONT  BPP_SICKCONT_MakeNull( void )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_NONE;
  return cont;
}
static inline BOOL BPP_SICKCONT_IsNull( BPP_SICK_CONT cont )
{
  return cont.type == WAZASICK_CONT_NONE;
}
/*
 *  ターン数型 作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakeTurn( u8 turns )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_TURN;
  cont.turn.count = turns;
  cont.turn.param = 0;
  return cont;
}
/*
 *  ターン数型（パラメータ付き）作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakeTurnParam( u8 turns, u16 param )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_TURN;
  cont.turn.count = turns;
  cont.turn.param = param;
  return cont;
}
/*
 *  ポケ依存型作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePoke( u8 pokeID )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_POKE;
  cont.poke.ID = pokeID;
  return cont;
}
/*
 *  永続型作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanent( void )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  return cont;
}
/*
 *  永続型（ターン数カウントアップ）作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanentInc( u8 count_max )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  cont.permanent.count_max = count_max;
  return cont;
}
/*
 *  永続型（パラメータ付き）作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanentParam( u16 param )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  cont.permanent.param = param;
  return cont;
}
/*
 *  永続型（ターン数カウントアップ＆パラメータ付き）作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanentIncParam( u8 count_max, u16 param )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  cont.permanent.count_max = count_max;
  cont.permanent.param = param;
  return cont;
}
/*
 *  ターン数・ポケ依存混合型作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePokeTurn( u8 pokeID, u8 turns )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_POKETURN;
  cont.poketurn.count = turns;
  cont.poketurn.pokeID = pokeID;
  return cont;
}
/*
 *  ターン数・ポケ依存混合型（パラメータ付き）作成
 */
static inline BPP_SICK_CONT BPP_SICKCONT_MakePokeTurnParam( u8 pokeID, u8 turns, u16 param )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_POKETURN;
  cont.poketurn.count = turns;
  cont.poketurn.pokeID = pokeID;
  cont.poketurn.param = param;
  return cont;
}


static inline BPP_SICK_CONT BPP_SICKCONT_MakeMoudokuCont( void )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  cont.permanent.count_max = BTL_MOUDOKU_COUNT_MAX;
  return cont;
}

/*
 *  依存ポケIDを取得（ポケ依存でなければ BTL_POKEID_NULL）
 */
static inline u8 BPP_SICKCONT_GetPokeID( BPP_SICK_CONT cont )
{
  if( cont.type == WAZASICK_CONT_POKE ){
    return cont.poke.ID;
  }
  if( cont.type == WAZASICK_CONT_POKETURN ){
    return cont.poketurn.pokeID;
  }
  return BTL_POKEID_NULL;
}
/*
 *  有効ターン数を取得（ターン数制限がなければ 0）
 */
static inline u8 BPP_SICCONT_GetTurnMax( BPP_SICK_CONT cont )
{
  if( cont.type == WAZASICK_CONT_TURN ){
    return cont.turn.count;
  }
  if( cont.type == WAZASICK_CONT_POKETURN ){
    return cont.poketurn.count;
  }
  return 0;
}
/*
 *  汎用パラメータを設定
 */
static inline void BPP_SICKCONT_AddParam( BPP_SICK_CONT* cont, u16 param )
{
  if( cont->type == WAZASICK_CONT_PERMANENT ){
    cont->permanent.param = param;
    return;
  }
  else if( cont->type == WAZASICK_CONT_POKE ){
    cont->poke.param = param;
    return;
  }
  else if( cont->type == WAZASICK_CONT_POKETURN ){
    cont->poketurn.param = param;
    return;
  }
  else if( cont->type == WAZASICK_CONT_TURN ){
    cont->turn.param = param;
    return;
  }
}
/*
 *  設定した汎用パラメータを取得
 */
static inline u8 BPP_SICKCONT_GetParam( BPP_SICK_CONT cont )
{
  if( cont.type == WAZASICK_CONT_PERMANENT ){
    return cont.permanent.param;
  }
  if( cont.type == WAZASICK_CONT_POKE ){
    return cont.poke.param;
  }
  if( cont.type == WAZASICK_CONT_POKETURN ){
    return cont.poketurn.param;
  }
  if( cont.type == WAZASICK_CONT_TURN ){
    return cont.turn.param;
  }
  return 0;
}
/*
 *  汎用フラグセット
 */
static inline void BPP_SICKCONT_SetFlag( BPP_SICK_CONT* cont, BOOL flag )
{
  if( cont->type == WAZASICK_CONT_PERMANENT ){
    cont->permanent.flag = flag;
    return;
  }
  if( cont->type == WAZASICK_CONT_POKE ){
    cont->poke.flag = flag;
    return;
  }
  if( cont->type == WAZASICK_CONT_POKETURN ){
    cont->poketurn.flag = flag;
    return;
  }
  if( cont->type == WAZASICK_CONT_TURN ){
    cont->turn.flag = flag;
    return;
  }
}
/*
 *  汎用フラグ取得
 */
static inline BOOL BPP_SICKCONT_GetFlag( BPP_SICK_CONT cont )
{
  if( cont.type == WAZASICK_CONT_PERMANENT ){
    return cont.permanent.flag;
  }
  if( cont.type == WAZASICK_CONT_POKE ){
    return cont.poke.flag;
  }
  if( cont.type == WAZASICK_CONT_POKETURN ){
    return cont.poketurn.flag;
  }
  if( cont.type == WAZASICK_CONT_TURN ){
    return cont.turn.flag;
  }
  return 0;
}
/*
 *  継続ターン数増加
 */
static inline void BPP_SICKCONT_IncTurn( BPP_SICK_CONT* cont, u8 inc )
{
  if( cont->type == WAZASICK_CONT_TURN ){
    if( cont->turn.count < 8 ){
      cont->turn.count += inc;
    }
  }
  if( cont->type == WAZASICK_CONT_POKETURN ){
    if( cont->poketurn.count < 8 ){
      cont->poketurn.count += inc;
    }
  }
}
/*
 *  継続ターン数セット
 */
static inline void BPP_SICKCONT_SetTurn( BPP_SICK_CONT* cont, u8 turn )
{
  if( cont->type == WAZASICK_CONT_TURN ){
    if( cont->turn.count < 8 ){
      cont->turn.count = turn;
    }
  }
  if( cont->type == WAZASICK_CONT_POKETURN ){
    if( cont->poketurn.count < 8 ){
      cont->poketurn.count = turn;
    }
  }
}
//===================================================================
// ポケモンタイプを合成して１変数として扱うための措置
//===================================================================
typedef u16 PokeTypePair;

static inline PokeTypePair PokeTypePair_Make( PokeType type1, PokeType type2 )
{
  return ( (((type1)&0xff)<<8) | (type2&0xff) );
}
static inline PokeTypePair PokeTypePair_MakePure( PokeType type )
{
  return PokeTypePair_Make( type, type );
}

static inline PokeType PokeTypePair_GetType1( PokeTypePair pair )
{
  return (pair >> 8) & 0xff;
}

static inline PokeType PokeTypePair_GetType2( PokeTypePair pair )
{
  return pair & 0xff;
}

static inline void PokeTypePair_Split( PokeTypePair pair, PokeType* type1, PokeType* type2 )
{
  *type1 = PokeTypePair_GetType1( pair );
  *type2 = PokeTypePair_GetType2( pair );
}

static inline BOOL PokeTypePair_IsMatch( PokeTypePair pair, PokeType type )
{
  if( PokeTypePair_GetType1(pair) == type ){
    return TRUE;
  }
  if( PokeTypePair_GetType2(pair) == type ){
    return TRUE;
  }
  return FALSE;
}
static inline BOOL PokeTypePair_IsMatchEither( PokeTypePair pair1, PokeTypePair pair2 )
{
  if( PokeTypePair_GetType1(pair1) == PokeTypePair_GetType1(pair2) ){
    return TRUE;
  }
  if( PokeTypePair_GetType1(pair1) == PokeTypePair_GetType2(pair2) ){
    return TRUE;
  }
  if( PokeTypePair_GetType2(pair1) == PokeTypePair_GetType1(pair2) ){
    return TRUE;
  }
  if( PokeTypePair_GetType2(pair1) == PokeTypePair_GetType2(pair2) ){
    return TRUE;
  }
  return FALSE;
}
static inline BOOL PokeTypePair_IsPure( PokeTypePair pair )
{
  return PokeTypePair_GetType1(pair) == PokeTypePair_GetType2(pair);
}

//===================================================================
// バトル用Printf処理
//===================================================================

typedef enum {
  BTL_PRINTTYPE_UNKNOWN,
  BTL_PRINTTYPE_SERVER,
  BTL_PRINTTYPE_CLIENT,
  BTL_PRINTTYPE_STANDALONE,
}BtlPrintType;

//#if defined(DEBUG_ONLY_FOR_taya) | \
//    defined(DEBUG_ONLY_FOR_nishino) | defined(DEBUG_ONLY_FOR_morimoto)
#if defined(DEBUG_ONLY_FOR_taya)
#ifdef PM_DEBUG
//  #define BTL_PRINT_SYSTEM_ENABLE
#endif
#endif


#ifdef BTL_PRINT_SYSTEM_ENABLE

extern void BTL_UTIL_SetPrintType( BtlPrintType type );
extern void BTL_UTIL_Printf( const char* filename, int line, const char* fmt, ... );
extern void BTL_UTIL_DumpPrintf( const char* caption, const void* data, u32 size );

#define BTL_Printf( ... ) BTL_UTIL_Printf( __FILE__, __LINE__, __VA_ARGS__ )
#define BTL_PrintfEx( flg, ... )  if( flg ){ BTL_UTIL_Printf( __FILE__, __LINE__, __VA_ARGS__ ); }
#define BTL_PrintfSimple( ... ) OS_TPrintf( __VA_ARGS__ )
#define BTL_DUMP_Printf( cap, dat, siz )  BTL_UTIL_DumpPrintf( cap, dat, siz );

#else // #ifdef BTL_PRINT_SYSTEM_ENABLE

#define BTL_UTIL_SetPrintType(t)  /* */
#define BTL_Printf( ... )         /* */
#define BTL_PrintfEx( flg, ... )  /* */
#define BTL_PrintfSimple( ... )   /* */
#define BTL_DUMP_Printf( cap, dat, siz )  /* */

#endif  // #ifdef BTL_PRINT_SYSTEM_ENABLE




