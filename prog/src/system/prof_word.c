//======================================================================
/**
 * @file	prof_word.c
 * @brief	不正文字列検査
 * @author	ariizumi
 * @data	100428
 *
 * モジュール名：PROF_WORD
 */
//======================================================================
#include <gflib.h>

#include "print/str_tool.h"
#include "system/prof_word.h"
#include "arc_def.h"
#include "prof_word.naix"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PROF_WORD_LEN (32)
#define PROF_WORD_RAND_KEY (0x72012891)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static const BOOL PROF_WORD_CheckFile( STRCODE *code , u32 datId , const HEAPID heapId );
static const BOOL PROF_WORD_CheckWord( STRCODE *code , STRCODE *word );

//--------------------------------------------------------------
//  不正文字列チェック
//  TRUE 不正 FALSE 有効
//--------------------------------------------------------------
const BOOL PROF_WORD_CheckProfanityWord( const STRBUF *strBuf , const HEAPID heapId )
{
  return PROF_WORD_CheckProfanityWordCode( GFL_STR_GetStringCodePointer(strBuf) , heapId );
}

const BOOL PROF_WORD_CheckProfanityWordCode( const STRCODE *strCode , const HEAPID heapId )
{
  int i;
  BOOL ret = FALSE;
  STRCODE code[PROF_WORD_LEN];
  
  GFL_STD_MemCopy( strCode , code , PROF_WORD_LEN*2 );

  //チェックする文字列の変換
  for( i=0;i<PROF_WORD_LEN;i++ )
  {
    if( code[i] >= L'ァ' && code[i] <= L'ヴ' )
    {
      code[i] = code[i] - L'ァ' + L'ぁ';
    }
    else
    if( code[i] >= L'a' && code[i] <= L'z' )
    {
      code[i] = code[i] - L'a' + L'A';
    }
    else
    if( code[i] >= L'ａ' && code[i] <= L'z' )
    {
      code[i] = code[i] - L'ａ' + L'A';
    }
    else
    if( code[i] >= L'Ａ' && code[i] <= L'z' )
    {
      code[i] = code[i] - L'Ａ' + L'A';
    }
    else  //ヨーロッパ系の特殊文字
    if( code[i] >= 0xe0 && code[i] <= 0xfe )
    {
      code[i] = code[i] - 0xe0 + 0xc0;
    }
    
    //特殊な文字(アルファベット26文字以外)は大文字変換してません！
  }
  
  for( i=0;i<GFL_ARC_GetDataFileCnt(ARCID_PROF_WORD);i++ )
  {
    ret = PROF_WORD_CheckFile( code , i , heapId );
    if( ret == TRUE )
    {
      break;
    }
  }
  return ret;
}

static const BOOL PROF_WORD_CheckFile( STRCODE *code , u32 datId , const HEAPID heapId )
{
  //ファイル単位でチェック
  BOOL ret;
  int i;
  u16 wordNum;
  u32 fileSize;
  STRCODE *buf = (STRCODE*)GFL_ARC_UTIL_LoadEx( ARCID_PROF_WORD , datId , FALSE , heapId , &fileSize );
  
  GFL_STD_CODED_Decoded( buf , fileSize , PROF_WORD_RAND_KEY );
  wordNum = fileSize/(PROF_WORD_LEN*2);
  
  for( i=0;i<wordNum;i++ )
  {
    ret = PROF_WORD_CheckWord( code , &buf[i*PROF_WORD_LEN] );
    if( ret == TRUE )
    {
      break;
    }
  }
  
  GFL_HEAP_FreeMemory( buf );
  
  return ret;
}

static const BOOL PROF_WORD_CheckWord( STRCODE *code , STRCODE *word )
{
  int i;
  //完全一致チェックのため独自ルーチン
  for( i=0;i<PROF_WORD_LEN;i++ )
  {
    if( code[i] != word[i] )
    {
      return FALSE;
    }
    else
    if( code[i] == GFL_STR_GetEOMCode() &&
        word[i] == GFL_STR_GetEOMCode() )
    {
      return TRUE;
    }
  }
  return FALSE;
}
