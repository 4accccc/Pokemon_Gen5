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
//ライブラリ
#include <gflib.h>
#include <dwc.h>

//システム
#include "system/main.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_namein.h"

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

  STRBUF    *p_src_str;
  HEAPID    heapID;
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
 *	@param	DWC_TOOL_BADWORD_WORK *p_wk   ワーク
 *	@param	STRCODE *p_str               チェックするSTRBUF(不正文字時はこの中を書き換えます)
 *	@param	heapID                        テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static inline void DWC_TOOL_BADWORD_Start( DWC_TOOL_BADWORD_WORK *p_wk, STRBUF *p_str, HEAPID heapID )
{ 
  BOOL ret;

  GFL_STD_MemClear( p_wk, sizeof(DWC_TOOL_BADWORD_WORK));
  p_wk->p_src_str = p_str;
  p_wk->heapID    = heapID;

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
    const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer( p_wk->p_src_str );
    GF_ASSERT( GFL_STR_GetBufferLength( p_wk->p_src_str ) < DWC_TOOL_BADWORD_STRL_MAX );
    for( i = 0; i < GFL_STR_GetBufferLength( p_wk->p_src_str ); i++ )
    { 
      if( GFL_STR_GetEOMCode() == cp_strcode[i] )
      {
        p_wk->badword_str[i]  = 0x0000;
      }
      else
      { 
        p_wk->badword_str[i]  = cp_strcode[i];
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

    if( *p_is_bad_word )
    { 
      //不正時は名前を入れ替える
      GFL_MSGDATA *p_msg  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_namein_dat, p_wk->heapID );

      if ( GET_VERSION() == VERSION_BLACK )
      {
        //ブラックバージョンの処理
        GFL_MSG_GetString( p_msg, NAMEIN_DEF_NAME_005, p_wk->p_src_str );
      }
      else
      {
        //ホワイトバージョンの処理
        GFL_MSG_GetString( p_msg, NAMEIN_DEF_NAME_004, p_wk->p_src_str );
      }

      GFL_MSG_Delete( p_msg );
    }
  }
  return ret;
}
