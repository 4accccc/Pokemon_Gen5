//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		dwc_tool.h
 *	@brief  WIFI関係の共通モジュールヘッダ
 *	@author	Toru=Nagihashi
 *	@date		2010.03.20
 *
 *  オーバーレイを無視するため、static inline定義です
 *
 *  現状のツールは以下です
 *  ・不正文字チェック    [DWC_TOOL_BADWORD]
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//	ライブラリ
#include <gflib.h>
#include <dwc.h>

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//
//    不正文字チェック  共通処理
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//=============================================================================
/**
 *					構造体
*/
//=============================================================================
#define DWC_TOOL_BADWORD_STRL_MAX  (10) //暫定で１０文字までです
//-------------------------------------
///	不正文字ワーク
//=====================================
typedef struct 
{
  STRCODE   badword_str[ DWC_TOOL_BADWORD_STRL_MAX ];
  u16       *p_badword_arry[1];
  char      badword_result[1];
  int       badword_num;
} DWC_TOOL_BADWORD_WORK;


//=============================================================================
/**
 *					関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  不正文字チェックをスタート
 *
 *	@param	DWC_TOOL_BADWORD_WORK *p_wk  ワーク
 *	@param	STRCODE *cp_str                 チェックするSTRCODE
 *	@param	str_len                         STRCODEの長さ
 */
//-----------------------------------------------------------------------------
static inline void DWC_TOOL_BADWORD_Start( DWC_TOOL_BADWORD_WORK *p_wk, const STRCODE *cp_str, u32 str_len )
{ 
  BOOL ret;
/*
  # 文字コードはUnicode（リトルエンディアンUTF16）を使用してください。
  それ以外の文字コードを使用している場合は、Unicodeに変換してください。
  # スクリーンネームにUnicode及びIPLフォントにない独自の文字を使用している場合は、スペースに置き換えてください。
  # 終端は"\0\0"（u16で0x0000）である必要があります。
  # 配列内の全ての文字列の合計が501文字まで（各文字列の終端を含む）にする必要があります。
  # 配列内の文字列にNULLを指定することはできません。 
  */
  { 
    int i;
    GF_ASSERT( str_len < DWC_TOOL_BADWORD_STRL_MAX );
    for( i = 0; i < str_len; i++ )
    { 
      if( GFL_STR_GetEOMCode() == cp_str[i] )
      {
        p_wk->badword_str[i]  = 0x0000;
      }
      else
      { 
        p_wk->badword_str[i]  = cp_str[i];
      }
    }
    p_wk->p_badword_arry[0] = p_wk->badword_str;
  }
  p_wk->badword_num = 0;


  ret = DWC_CheckProfanityExAsync( (const u16 **)p_wk->p_badword_arry,
                             1,
                             NULL,
                             0,
                             p_wk->badword_result,
                             &p_wk->badword_num,
                             DWC_PROF_REGION_ALL );
  GF_ASSERT( ret );
}
//----------------------------------------------------------------------------
/**
 *	@brief  不正文字チェックメイン
 *
 *	@param	DWC_TOOL_BADWORD_WORK *p_wk    ワーク
 *	@param	*p_is_bad_word                    TRUEならば不正文字  FALSEならば正常文字
 *
 *	@return TRUE処理終了  FALSE処理中
 */
//-----------------------------------------------------------------------------
static inline BOOL DWC_TOOL_BADWORD_Wait( DWC_TOOL_BADWORD_WORK *p_wk, BOOL *p_is_bad_word )
{ 
  BOOL ret;
  ret = DWC_CheckProfanityProcess() == DWC_PROF_STATE_SUCCESS;

  if( ret == TRUE )
  { 
    *p_is_bad_word  = p_wk->badword_num;
  }
  return ret;
}
