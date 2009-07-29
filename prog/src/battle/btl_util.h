//=============================================================================================
/**
 * @file  btl_util.h
 * @brief ポケモンWB バトルシステム 共有ツール群
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================
#ifndef __BTL_UTIL_H__
#define __BTL_UTIL_H__


#include "poke_tool/poketype.h"

#include "btl_const.h"

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
    u16    raw;
    struct {
      u16  type : 3;
      u16  _0   : 12;
    };
    struct {
      u16  type_turn : 3;
      u16  count     : 5;
      u16  param     : 8;
    }turn;
    struct {
      u16  type_poke : 3;
      u16  ID        : 6;
      u16  _2        : 6;
    }poke;
    struct {
      u16  type_perm : 3;
      u16  count_max : 5;
      u16  param     : 8;
    }permanent;
    struct {
       u16 type_poketurn : 3;
       u16 count         : 5;
       u16 pokeID        : 8;
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

static inline BPP_SICK_CONT BPP_SICKCONT_MakeTurn( u8 turns )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_TURN;
  cont.turn.count = turns;
  cont.turn.param = 0;
  return cont;
}
static inline BPP_SICK_CONT BPP_SICKCONT_MakeTurnParam( u8 turns, u8 param )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_TURN;
  cont.turn.count = turns;
  cont.turn.param = param;
  return cont;
}
static inline BPP_SICK_CONT BPP_SICKCONT_MakePokeTurn( u8 pokeID, u8 turns )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_POKETURN;
  cont.poketurn.count = turns;
  cont.poketurn.pokeID = pokeID;
  return cont;
}
static inline BPP_SICK_CONT BPP_SICKCONT_MakePoke( u8 pokeID )
{
  BPP_SICK_CONT  cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_POKE;
  cont.poke.ID = pokeID;
  return cont;
}
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanent( void )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  return cont;
}
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanentInc( u8 count_max )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  cont.permanent.count_max = count_max;
  return cont;
}
static inline BPP_SICK_CONT BPP_SICKCONT_MakePermanentParam( u8 param )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_PERMANENT;
  cont.permanent.param = param;
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

static inline BOOL BPP_SICKCONT_IsMoudokuCont( BPP_SICK_CONT cont )
{
  return ((cont.type == WAZASICK_CONT_PERMANENT) && (cont.permanent.count_max > 0));
}

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
static inline u8 BPP_SICKCONT_GetParam( BPP_SICK_CONT cont )
{
  if( cont.type == WAZASICK_CONT_PERMANENT ){
    return cont.permanent.param;
  }
  return 0;
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

#ifdef DEBUG_ONLY_FOR_taya
#ifdef PM_DEBUG
  #define BTL_PRINT_SYSTEM_ENABLE
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

#endif
