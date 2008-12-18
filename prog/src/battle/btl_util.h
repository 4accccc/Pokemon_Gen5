//=============================================================================================
/**
 * @file	btl_util.h
 * @brief	ポケモンWB バトルシステム 共有ツール群
 * @author	taya
 *
 * @date	2008.09.25	作成
 */
//=============================================================================================
#ifndef __BTL_UTIL_H__
#define __BTL_UTIL_H__


#include "poke_tool/poketype.h"

//===================================================================
// 単純なプロセス処理構造
//===================================================================

typedef BOOL (*BPFunc)(int*, void*);

typedef struct {
	BPFunc	initFunc;
	BPFunc	loopFunc;
	void*	work;
	int		seq;
}BTL_PROC;


static inline void BTL_UTIL_ClearProc( BTL_PROC* proc )
{
	proc->initFunc	= NULL;
	proc->loopFunc	= NULL;
	proc->work		= NULL;
	proc->seq		= 0;
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
// ポケモンタイプを合成して１変数として扱うための措置
//===================================================================
typedef u16 PokeTypePair;

static inline PokeTypePair PokeTypePair_Make( PokeType type1, PokeType type2 )
{
	return ( (((type1)&0xff)<<8) | (type2&0xff) );
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
	if( PokeTypePair_GetType1(pair) == type )
	{
		return TRUE;
	}
	if( PokeTypePair_GetType2(pair) == type )
	{
		return TRUE;
	}
	return FALSE;
}

//===================================================================
// バトル用Printf処理
//===================================================================
extern void BTL_UTIL_Printf( const char* filename, int line, const char* fmt, ... );
#define BTL_Printf( ... )	BTL_UTIL_Printf( __FILE__, __LINE__, __VA_ARGS__ )

extern void BTL_UTIL_DumpPrintf( const char* caption, const void* data, u32 size );
#define BTL_DUMP_Printf( cap, dat, siz )	BTL_UTIL_DumpPrintf( cap, dat, siz );

#endif
