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
 *  ・言語別の時間取得    [DWC_TOOL_GetLocalDateTime]
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
#define DWC_TOOL_BADWORD_STR_MAX  (10+EOM_SIZE) //プレイヤー名の不正文字チェックを想定しているので１０文字+EOMまでとしています
//-------------------------------------
///	不正文字ワーク
//=====================================
typedef struct 
{
  STRCODE   badword_str[ DWC_TOOL_BADWORD_STR_MAX ];
  u16       *p_badword_arry[1];
  char      badword_result[1];
  int       badword_num;

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
 *	@param	STRCODE *cp_str                チェックするSTRBUF(中でコピーするのですぐ解放して構いません)
 *	@param	heapID                        テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static inline void DWC_TOOL_BADWORD_Start( DWC_TOOL_BADWORD_WORK *p_wk, const STRBUF *cp_str, HEAPID heapID )
{ 
  BOOL ret;

  GFL_STD_MemClear( p_wk, sizeof(DWC_TOOL_BADWORD_WORK));
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
    const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer( cp_str );
    GF_ASSERT( GFL_STR_GetBufferLength( cp_str )+EOM_SIZE < DWC_TOOL_BADWORD_STR_MAX );
    for( i = 0; i < GFL_STR_GetBufferLength( cp_str )+EOM_SIZE; i++ )
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

  if( ret )
  {
    *p_is_bad_word  = (p_wk->badword_num > 0);
    
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ニックネームの不正名を取得
 *
 *	@param	heapID              STRBUF作成用ヒープID
 *	@retval STRBUF              strbuf
 */
//-----------------------------------------------------------------------------
static inline STRBUF * DWC_TOOL_CreateBadNickName( HEAPID heapID )
{ 
  GFL_MSGDATA *p_msg  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_namein_dat, heapID );
  STRBUF      *p_strbuf;

  if ( GET_VERSION() == VERSION_BLACK )
  {
    //ブラックバージョンの処理
    p_strbuf  = GFL_MSG_CreateString( p_msg, NAMEIN_DEF_NAME_005 );
  }
  else
  {
    //ホワイトバージョンの処理
    p_strbuf  = GFL_MSG_CreateString( p_msg, NAMEIN_DEF_NAME_004 );
  }

  GFL_MSG_Delete( p_msg );

  return p_strbuf;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ニックネームの不正名をSTRCODEに設定
 *
 *	@param	STRCODE *p_strcode  STRCODE
 *	@param	len                 EOMを含む長さ
 *	@param	heapID              テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static inline void DWC_TOOL_SetBadNickName( STRCODE *p_strcode, u16 len, HEAPID heapID )
{
  GF_ASSERT( p_strcode );
  GF_ASSERT( len );
  { 
    STRBUF * p_src_buff = DWC_TOOL_CreateBadNickName( heapID );
    const STRCODE *p_src_code = GFL_STR_GetStringCodePointer( p_src_buff );

    int i;
    for( i = 0; i < len-1 && i < GFL_STR_GetBufferLength(p_src_buff); i++ ) //-1はEOM分を考慮して
    { 
      p_strcode[i]  = p_src_code[i];
    }
    p_strcode[i] = GFL_STR_GetEOMCode();

    GFL_STR_DeleteBuffer( p_src_buff );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  言語別時間を取得
 *
 *	@param	RTCDate *p_date 言語別日付を格納する領域
 *	@param	RTCTime *p_time 言語別時刻を格納する領域
 *
 *	@return TRUE取得に成功  FALSE取得に失敗
 */
//-----------------------------------------------------------------------------
static inline BOOL DWC_TOOL_GetLocalDateTime( RTCDate *date, RTCTime *time )
{
#if  ( PM_LANG == LANG_JAPAN )
  enum
  {
    LOCAL_TIME = 9*60*60, //+0900(JST)
  };
#else
#error
#endif //

  BOOL ret = DWC_GetDateTime( date, time );
  if( ret )
  {
    s64 sec = RTC_ConvertDateTimeToSecond( date, time );
    sec += LOCAL_TIME;
    RTC_ConvertSecondToDateTime( date, time, sec );
  }

  return ret;
}
