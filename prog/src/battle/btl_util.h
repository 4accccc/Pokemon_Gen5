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
      u32  type : 3;
      u32  _0   : 29;
    };
    struct {
      u32  type_turn : 3;   ///< ターン数型
      u32  count     : 6;   ///< 有効ターン数
      u32  param     : 16;  ///< 汎用パラメータ16bit
      u32  flag      : 1;   ///< 汎用フラグ 1bit
      u32  _1        : 6;
    }turn;
    struct {
      u32  type_poke : 3;   ///< ポケ依存型
      u32  ID        : 6;   ///< 対象ポケID
      u32  param     : 16;  ///< 汎用パラメータ16bit
      u32  flag      : 1;   ///< 汎用フラグ 1bit
      u32  _2        : 6;
    }poke;
    struct {
      u32  type_perm : 3;   ///< 永続型
      u32  count_max : 6;   ///< ターンカウントアップ最大数
      u32  param     : 16;  ///< 汎用パラメータ16bit
      u32  flag      : 1;   ///< 汎用フラグ 1bit
      u32  _3        : 6;
    }permanent;
    struct {
      u32 type_poketurn : 3;  ///< ターン数・ポケ依存混合型
      u32 count         : 6;  ///< 有効ターン数
      u32 pokeID        : 6;  ///< 対象ポケID
      u32 param         : 16; ///< 汎用パラメータ16bit
      u32 flag          : 1;  ///< 汎用フラグ 1bit
    }poketurn;
  };

}BPP_SICK_CONT;

/**
 *  NULL型 作成
 */
extern BPP_SICK_CONT  BPP_SICKCONT_MakeNull( void );

/**
 *  NULL型かどうか判別
 */
extern BOOL BPP_SICKCONT_IsNull( BPP_SICK_CONT cont );

/*
 *  ターン数型 作成
 */
extern BPP_SICK_CONT  BPP_SICKCONT_MakeTurn( u8 turns );

/*
 *  ターン数型（パラメータ付き）作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakeTurnParam( u8 turns, u16 param );

/*
 *  ポケ依存型作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePoke( u8 pokeID );

/*
 *  永続型作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePermanent( void );

/*
 *  永続型（ターン数カウントアップ）作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePermanentInc( u8 count_max );

/*
 *  永続型（パラメータ付き）作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePermanentParam( u16 param );

/*
 *  永続型（ターン数カウントアップ＆パラメータ付き）作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePermanentIncParam( u8 count_max, u16 param );

/*
 *  ターン数・ポケ依存混合型作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePokeTurn( u8 pokeID, u8 turns );

/*
 *  ターン数・ポケ依存混合型（パラメータ付き）作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakePokeTurnParam( u8 pokeID, u8 turns, u16 param );

/**
 * もうどく用型作成
 */
extern BPP_SICK_CONT BPP_SICKCONT_MakeMoudokuCont( void );

/**
 * もうどく用型かどうか判別
 */
extern BOOL BPP_SICKCONT_IsMoudokuCont( BPP_SICK_CONT cont );

/*
 *  依存ポケIDを取得（ポケ依存でなければ BTL_POKEID_NULL）
 */
extern u8 BPP_SICKCONT_GetPokeID( BPP_SICK_CONT cont );

/*
 *  依存ポケIDを書き換え（ポケ依存でなければ何もしないL）
 */
extern void BPP_SICKCONT_SetPokeID( BPP_SICK_CONT* cont, u8 pokeID );


/*
 *  有効ターン数を取得（ターン数制限がなければ 0）
 */
extern u8 BPP_SICCONT_GetTurnMax( BPP_SICK_CONT cont );

/*
 *  汎用パラメータを設定
 */
extern void BPP_SICKCONT_AddParam( BPP_SICK_CONT* cont, u16 param );

/*
 *  設定した汎用パラメータを取得
 */
extern u16 BPP_SICKCONT_GetParam( BPP_SICK_CONT cont );

/*
 *  汎用フラグセット
 */
extern void BPP_SICKCONT_SetFlag( BPP_SICK_CONT* cont, BOOL flag );

/*
 *  汎用フラグ取得
 */
extern BOOL BPP_SICKCONT_GetFlag( BPP_SICK_CONT cont );

/*
 *  継続ターン数増加
 */
extern void BPP_SICKCONT_IncTurn( BPP_SICK_CONT* cont, u8 inc );

/*
 *  継続ターン数セット
 */
extern void BPP_SICKCONT_SetTurn( BPP_SICK_CONT* cont, u8 turn );



//===================================================================
// ポケモンタイプを合成して１変数として扱うための措置
//===================================================================
typedef u16 PokeTypePair;

extern PokeTypePair PokeTypePair_Make( PokeType type1, PokeType type2 );
extern PokeTypePair PokeTypePair_MakePure( PokeType type );
extern PokeType PokeTypePair_GetType1( PokeTypePair pair );
extern PokeType PokeTypePair_GetType2( PokeTypePair pair );
extern void PokeTypePair_Split( PokeTypePair pair, PokeType* type1, PokeType* type2 );
extern BOOL PokeTypePair_IsMatch( PokeTypePair pair, PokeType type );
extern BOOL PokeTypePair_IsMatchEither( PokeTypePair pair1, PokeTypePair pair2 );
extern BOOL PokeTypePair_IsPure( PokeTypePair pair );


//===================================================================
// バトル用Printf処理
//===================================================================

#include "btl_debug_print.h"

extern void BTL_UTIL_PRINTSYS_Init( void );
extern void BTL_UTIL_PRINTSYS_Quit( void );

#if defined(DEBUG_ONLY_FOR_taya)
//    defined(DEBUG_ONLY_FOR_nishino) | defined(DEBUG_ONLY_FOR_morimoto)
#ifdef PM_DEBUG
  #define BTL_PRINT_SYSTEM_ENABLE
#endif
#endif


#ifdef BTL_PRINT_SYSTEM_ENABLE

enum {
  PRINT_CHANNEL_PRESSURE = 0,
  PRINT_CHANNEL_EVENTSYS = 4,
  PRINT_CHANNEL_HEMSYS = 3,
  PRINT_CHANNEL_RECTOOL = 4,
};



extern void BTL_UTIL_SetPrintType( BtlPrintType type );
extern void BTL_UTIL_Printf( const char* filename, int line, u32 channel, u32 strID, ... );
extern void BTL_UTIL_PrintfSimple( const char* filename, u32 channel, u32 strID, ... );
extern void BTL_UTIL_DumpPrintf( const char* caption, const void* data, u32 size );

#define BTL_N_Printf( ... ) BTL_UTIL_Printf( __FILE__, __LINE__, 1, __VA_ARGS__ )
#define BTL_N_PrintfEx( ch, ... )  if( ch!=0 ){ BTL_UTIL_Printf( __FILE__, __LINE__, ch, __VA_ARGS__ ); }
#define BTL_N_PrintfSimple( ... ) BTL_UTIL_PrintfSimple( __FILE__, 1, __VA_ARGS__ )
#define BTL_N_PrintfSimpleEx( ch, ... ) if( ch!=0 ){ BTL_UTIL_PrintfSimple( __FILE__, ch, __VA_ARGS__ ); }

#else // #ifdef BTL_PRINT_SYSTEM_ENABLE

#define BTL_UTIL_SetPrintType(t)         /* */
#define BTL_N_Printf( ... )              /* */
#define BTL_N_PrintfEx( flg, ... )       /* */
#define BTL_N_PrintfSimple( ... )        /* */
#define BTL_N_PrintfSimpleEx( flg, ... ) /* */

#endif  // #ifdef BTL_PRINT_SYSTEM_ENABLE




/* 旧Print処理 */
#define BTL_Printf( ... )         /* */
#define BTL_PrintfEx( flg, ... )  /* */
#define BTL_PrintfSimple( ... )   /* */

