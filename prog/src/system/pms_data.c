//============================================================================================
/**
 * @file	pms_data.c
 * @bfief	簡易文章（簡易会話データと定型文を組み合わせた文章）
 * @author	taya
 * @date	05.02.03
 */
//============================================================================================
#include <gflib.h>

#include "print\wordset.h"
#include "system\pms_word.h"
#include "print\printsys.h"

#include "arc_def.h"
#include "message.naix"
#include "msg\msg_pmss_union.h"
#include "msg\msg_pmss_ready.h"
#include "msg\msg_pmss_won.h"
#include "msg\msg_pmss_lost.h"
#include "msg\msg_pms_word06.h"
#include "msg\msg_pms_word07.h"
#include "msg\msg_pms_word08.h"
#include "msg\msg_pms_word09.h"
#include "msg\msg_pms_word10.h"
#include "msg\msg_pms_word11.h"
#include "msg\msg_pms_word12.h"


#include "system\pms_data.h"


//--------------------------------------------------------------
/**
 *	定数
 */
//--------------------------------------------------------------
#define SENTENCE_ID_DEFAULT		(0)
#define SENTENCE_TYPE_CLEAR		(0xffff)

//TODO 適当
#define PMS_MESSAGE_BUF_SIZE (256)	//メッセージバッファの作成に使うサイズ

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
static const u16 SentenceDatID[] = {
	NARC_message_pmss_ready_dat,
	NARC_message_pmss_won_dat,
	NARC_message_pmss_lost_dat,
	NARC_message_pmss_mail_dat,
	NARC_message_pmss_union_dat,

};

//==============================================================
// Prototype
//==============================================================
static u32 get_include_word_max( u32 sentence_type, u32 sentence_id , const HEAPID heapID );




//------------------------------------------------------------------
/**
 * 空の文章としてクリアする
 *
 * @param   pms		
 *
 */
//------------------------------------------------------------------
void PMSDAT_Clear( PMS_DATA* pms )
{
	int i;

	pms->sentence_type = SENTENCE_TYPE_CLEAR;

	for(i=0; i<PMS_WORD_MAX; i++)
	{
		pms->word[i] = PMS_WORD_NULL;
	}
}


//------------------------------------------------------------------
/**
 * 内容のある文章として初期化する
 *
 * @param   pms		簡易文章構造体へのポインタ
 *
 */
//------------------------------------------------------------------
void PMSDAT_Init( PMS_DATA* pms, u32 sentence_type )
{
	int i;

	pms->sentence_type = sentence_type;
	pms->sentence_id = SENTENCE_ID_DEFAULT;

	for(i=0; i<PMS_WORD_MAX; i++)
	{
		pms->word[i] = PMS_WORD_NULL;
	}
}

//------------------------------------------------------------------
/**
 * ユニオンルームデフォルトあいさつとして初期化する
 *
 * @param   pms		簡易文章構造体へのポインタ
 *
 */
//------------------------------------------------------------------
void PMSDAT_SetupDefaultUnionMessage( PMS_DATA* pms )
{
	PMSDAT_Init( pms, PMS_TYPE_UNION );
	pms->sentence_id = pmss_union_06;
}


//------------------------------------------------------------------
/**
 * バトルタワーデフォルトあいさつとして初期化する
 *
 * @param   pms			
 * @param   msgType		
 *
 */
//------------------------------------------------------------------
/*
void PMSDAT_SetupDefaultBattleTowerMessage( PMS_DATA* pms, BTWR_PLAYER_MSG_ID msgType )
{
	static const struct {
		u8  sentenceType;
		u8  sentenceID;
		s16 word1_gmmID;
		u16 word1_wordID;
		s16 word2_gmmID;
		u16 word2_wordID;
	}sentenceDataTbl[] = {
		// BTWR_MSG_PLAYER_READY = 対戦前
		{
			PMS_TYPE_BATTLE_READY, pmss_ready_01,
			NARC_msg_pms_word06_dat, pms_word06_08,
			-1, 0,
		},
		// BTWR_MSG_PLAYER_WIN = 勝った時
		{
			PMS_TYPE_BATTLE_WON, pmss_won_01,
			NARC_msg_pms_word06_dat, pms_word06_34,
			-1, 0,
		},
		// BTWR_MSG_PLAYER_LOSE = 負けた時
		{
			PMS_TYPE_BATTLE_LOST, pmss_lost_01,
			NARC_msg_pms_word10_dat, pms_word10_11,
			-1, 0,
		},
		// BTWR_MSG_LEADER = リーダーになった時
		{
			PMS_TYPE_BATTLE_WON, pmss_won_05,
			NARC_msg_pms_word06_dat, pms_word06_02,
			-1, 0,
		}
	};

	GF_ASSERT(msgType<NELEMS(sentenceDataTbl));

	if( msgType < NELEMS(sentenceDataTbl) )
	{
		PMSDAT_Init( pms, sentenceDataTbl[msgType].sentenceType );
		pms->sentence_id = sentenceDataTbl[msgType].sentenceID;
		if( sentenceDataTbl[msgType].word1_gmmID != -1)
		{
			pms->word[0] = PMSW_GetWordNumberByGmmID(
				 sentenceDataTbl[msgType].word1_gmmID, sentenceDataTbl[msgType].word1_wordID );
		}
		if( sentenceDataTbl[msgType].word2_gmmID != -1)
		{
			pms->word[1] = PMSW_GetWordNumberByGmmID(
				 sentenceDataTbl[msgType].word2_gmmID, sentenceDataTbl[msgType].word2_wordID );
		}
	}
}
*/


//------------------------------------------------------------------
/**
 * 簡易文章データから、そのまま表示できる文字列を生成する
 *
 * @param   pms		
 * @param   heapID		
 *
 * @retval  STRBUF*		文字列
 */
//------------------------------------------------------------------
STRBUF* PMSDAT_ToString( const PMS_DATA* pms, u32 heapID )
{ 
  return PMSDAT_ToStringWithWordCount( pms, heapID, PMS_WORD_MAX );
}

//-----------------------------------------------------------------------------
/**
 * 簡易文章データから、そのまま表示できる文字列を生成する(タグ数指定版)
 *
 * @param   pms     文章型へのポインタ
 * @param   heapID    バッファ生成用ヒープID
 * @param   wordCount タグ数
 *
 * @retval  STRBUF*   生成された文字列を含むバッファ
 */
//-----------------------------------------------------------------------------
STRBUF* PMSDAT_ToStringWithWordCount( const PMS_DATA* pms, u32 heapID, u8 wordCount )
{
	STRBUF	*sentence,*baseStr;
	WORDSET	*wordset;
	GFL_MSGDATA*   msgman;
	int i;

	wordset = WORDSET_Create(heapID);

	for(i=0; i<wordCount; i++)
	{
		if( pms->word[i] != PMS_WORD_NULL )
		{
      // デコメ判定 add by genya hosaka
      if( (pms->word[i] >> PMS_WORD_DECO_BITSHIFT) & PMS_WORD_DECO_MASK )
      {
        PMS_DECO_ID deco_id;

        deco_id = pms->word[i] & PMS_WORD_NUM_MASK;

			  WORDSET_RegisterPMSDeco( wordset, i, deco_id );
      }
      else
      {
			  WORDSET_RegisterPMSWord( wordset, i, pms->word[i] );
      }
		}
		else
		{
      // 空白をあける
      // @@@
			WORDSET_RegisterPMSDeco( wordset, i, 1 );
			break;
		}
	}

	msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, SentenceDatID[pms->sentence_type], heapID );
	
	sentence = GFL_STR_CreateBuffer( PMS_MESSAGE_BUF_SIZE , heapID );
	baseStr = GFL_MSG_CreateString(msgman,pms->sentence_id);
	WORDSET_ExpandStr( wordset , sentence , baseStr );
	//sentence = MSGDAT_UTIL_AllocExpandString( wordset, msgman, pms->sentence_id, heapID );

  GFL_STR_DeleteBuffer(baseStr);
	GFL_MSG_Delete(msgman);
	WORDSET_Delete(wordset);

	return sentence;
}

//------------------------------------------------------------------
/**
 * 簡易会話文章データから、タグ展開前の文字列を取得
 * （おそらくこれを使うのは入力画面のみ）
 *
 * @param   pms		
 * @param   heapID		
 *
 * @retval  STRBUF*		
 */
//------------------------------------------------------------------
STRBUF*  PMSDAT_GetSourceString( const PMS_DATA* pms, u32 heapID )
{
	GFL_MSGDATA*   msgman;
	STRBUF*  baseStr;
	
	msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, SentenceDatID[pms->sentence_type], heapID );
	baseStr = GFL_MSG_CreateString(msgman,pms->sentence_id);
	GFL_MSG_Delete(msgman);

	return baseStr;
}

//------------------------------------------------------------------
/**
 * 空文章になっていないかチェック
 *
 * @param   pms		
 *
 * @retval  BOOL	
 */
//------------------------------------------------------------------
BOOL PMSDAT_IsEnabled( const PMS_DATA* pms )
{
	return (pms->sentence_type != SENTENCE_TYPE_CLEAR);
}

//------------------------------------------------------------------
/**
 * 文章として完成しているかチェック
 *
 * @param   pms		
 *
 * @retval  BOOL		
 */
//------------------------------------------------------------------
BOOL PMSDAT_IsComplete( const PMS_DATA* pms , const HEAPID heapID )
{
	u32 word_max, i;

	word_max = get_include_word_max( pms->sentence_type, pms->sentence_id , heapID);
	for(i=0; i<word_max; i++)
	{
		if( pms->word[i] == PMS_WORD_NULL )
		{
			return FALSE;
		}
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * 文章に含まれる単語数を返す
 *
 * @param   sentence_type		
 * @param   sentence_id		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 get_include_word_max( u32 sentence_type, u32 sentence_id , const HEAPID heapID )
{
	GFL_MSGDATA*   msgman;
	STRBUF  *buf;
	const STRCODE* cp;
	u32 cnt;

	GF_ASSERT( sentence_type < PMS_TYPE_MAX );
	GF_ASSERT( sentence_id < PMSDAT_GetSentenceIdMax( sentence_type ) );

	msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, SentenceDatID[sentence_type], heapID );
	buf = GFL_MSG_CreateString(msgman,sentence_id);
	GFL_MSG_Delete(msgman);
	
	cnt = PRINTSYS_GetTagCount( buf );
	/*
	cp = GFL_STR_GetStringCodePointer( buf );
	cnt = 0;
	while( *cp != EOM_ )
	{
		if( *cp == _CTRL_TAG )
		{
			if( STR_TOOL_GetTagType( cp ) )
			{
				cnt++;
			}
			cp = STR_TOOL_SkipTag( cp );
		}
		else
		{
			cp++;
		}
	}
	*/
	GFL_STR_DeleteBuffer(buf);
	return cnt;
}

//------------------------------------------------------------------
/**
 * @brief   単語ナンバーを返す
 *
 * @param   pms		
 * @param   pos		
 *
 * @retval  PMS_WORD		
 */
//------------------------------------------------------------------
PMS_WORD  PMSDAT_GetWordNumber( const PMS_DATA* pms, int pos )
{
	GF_ASSERT( pos < PMS_WORD_MAX );

  if( pms->word[ pos ] == PMS_WORD_NULL )
  {
	  return PMS_WORD_NULL;
  }

	return pms->word[ pos ] & PMS_WORD_NUM_MASK;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  単語ナンバーを生で返す
 *
 *	@param	const PMS_DATA* pms
 *	@param	pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PMS_WORD PMSDAT_GetWordNumberDirect( const PMS_DATA* pms, int pos )
{
	GF_ASSERT( pos < PMS_WORD_MAX );
	return pms->word[ pos ];
}

//-----------------------------------------------------------------------------
/**
 *	@brief  デコメ判定
 *
 *	@param	pms
 *	@param	pos 
 *
 *	@retval TRUE:デコメ FALSE:デコメでない（通常の単語）
 */
//-----------------------------------------------------------------------------
BOOL PMSDAT_IsDecoID( const PMS_DATA* pms, int pos )
{
  return PMSW_IsDeco( pms->word[pos] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  単語のデコメ判定
 *
 *	@param	const PMS_WORD* word 
 *
 *	@retval TRUE:デコメ
 */
//-----------------------------------------------------------------------------
BOOL PMSW_IsDeco( const PMS_WORD word )
{ 
  // NULL CHECK
  if( word == PMS_WORD_NULL )
  {
    return FALSE;
  }

  return ( word >> PMS_WORD_DECO_BITSHIFT) & PMS_WORD_DECO_MASK;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PMS_WORD から デコメIDを取得
 *
 *	@param	const PMS_WORD* word ワードID
 *
 *	@retval PMS_DECO_ID デコメID
 */
//-----------------------------------------------------------------------------
PMS_DECO_ID PMSW_GetDecoID( const PMS_WORD word )
{
  return ( word & PMS_WORD_NUM_MASK );
}

//------------------------------------------------------------------
/**
 * 設定されている文章タイプを取得
 *
 * @param   pms		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
u32  PMSDAT_GetSentenceType( const PMS_DATA* pms )
{
	return pms->sentence_type;
}
//------------------------------------------------------------------
/**
 * 設定されている文章IDを返す
 *
 * @param   pms		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
u32 PMSDAT_GetSentenceID( const PMS_DATA* pms )
{
	return pms->sentence_id;
}

//------------------------------------------------------------------
/**
 * 内容比較
 *
 * @param   pms1		文章型へのポインタ１
 * @param   pms2		文章型へのポインタ２
 *
 * @retval  BOOL		一致していればTRUE
 */
//------------------------------------------------------------------
BOOL PMSDAT_Compare( const PMS_DATA* pms1, const PMS_DATA* pms2 )
{
	int i;

	if(	(pms1->sentence_type != pms2->sentence_type)
	||	(pms1->sentence_id != pms2->sentence_id)
	){
		return FALSE;
	}

	for(i=0; i<PMS_WORD_MAX; i++)
	{
		if( pms1->word[i] != pms2->word[i] )
		{
			return FALSE;
		}
	}

	return TRUE;
}


//------------------------------------------------------------------
/**
 * 文章データのコピー
 *
 * @param   dst		コピー先ポインタ
 * @param   src		コピー元ポインタ
 */
//------------------------------------------------------------------
void PMSDAT_Copy( PMS_DATA* dst, const PMS_DATA* src )
{
	*dst = *src;
}

//==============================================================================================
//==============================================================================================
u32 PMSDAT_GetSentenceIdMax( u32 sentence_type )
{
	if( sentence_type < NELEMS(SentenceDatID) )
	{
    /// @TODO マジックナンバー
		// とりあえず現状は全タイプ20個になっているので...
		return 20;
	}
	return 0;
}

void PMSDAT_SetSentence( PMS_DATA* pms, u32 sentence_type, u32 sentence_id )
{
	GF_ASSERT( sentence_type < PMS_TYPE_MAX );

	pms->sentence_type = sentence_type;
	pms->sentence_id = sentence_id;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  単語ナンバーセット
 *
 *	@param	pms       文章型へのポインタ
 *	@param	pos       文章内にセットする位置
 *	@param	word      単語ナンバー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSDAT_SetWord( PMS_DATA* pms, u32 pos, PMS_WORD word )
{
	GF_ASSERT( pos < PMS_WORD_MAX );
	pms->word[pos] = word;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  デコメナンバーセット
 *
 *	@param	pms       文章型へのポインタ
 *	@param	pos       文章内にセットする位置
 *	@param	deco_id   デコメナンバー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSDAT_SetDeco( PMS_DATA* pms, u32 pos, PMS_DECO_ID deco_id )
{
	GF_ASSERT( pos < PMS_WORD_MAX );

  // デコメIDをセット
	pms->word[pos] = deco_id;
  // デコメフラグON
  pms->word[pos] += 1 << PMS_WORD_DECO_BITSHIFT;
}


//------------------------------------------------------------------
/**
 * 使わない単語領域はPMS_WORD_NULLでクリアしておく
 *
 * @param   pms		
 *
 */
//------------------------------------------------------------------
void PMSDAT_ClearUnnecessaryWord( PMS_DATA* pms , const HEAPID heapID )
{
	u32 max, i;

	max = get_include_word_max( pms->sentence_type, pms->sentence_id , heapID );
	for(i=max; i<PMS_WORD_MAX; i++)
	{
		pms->word[i] = PMS_WORD_NULL;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ内容が不正かをチェック
 *
 *	@param	const PMS_DATA *pms   文章型へのポインタ
 *	@param  HEAPID                テンポラリバッファ作成用ヒープID
 *
 *	@return TRUEならば正常  FALSEならば不正
 */
//-----------------------------------------------------------------------------
BOOL PMSDAT_IsValid( const PMS_DATA *pms, HEAPID heapID )
{ 
  if( pms->sentence_type < PMS_TYPE_MAX )
  { 
    if( pms->sentence_id < PMSDAT_GetSentenceIdMax( pms->sentence_type ) )
    { 
      int i;
      BOOL is_valid;
      u32 max;
      max = get_include_word_max( pms->sentence_type, pms->sentence_id, GFL_HEAP_LOWID( heapID ) );

      is_valid  = TRUE;
      for( i = 0; i < max; i++ )
      { 
        if( PMSW_IsDeco(pms->word[i]) )
        { 
          PMS_DECO_ID deco_id = PMSW_GetDecoID( pms->word[i]);
          //デコメならばデコメIDがあっているか
          if( deco_id >= PMS_DECOID_MAX )
          { 
            is_valid  = FALSE;
          }
        }
        else
        { 
          u32 wordID;
          u32 fileID;
          //単語ならば、単語ナンバーがあっているか
          if( !GetWordSorceID( pms->word[i], &fileID, &wordID ) )
          {
            is_valid  = FALSE;
          }
        }
      }
      return is_valid;
    }
  }

  return FALSE;
}


//==============================================================================================
//==============================================================================================


#ifdef PM_DEBUG
void PMSDAT_SetDebug( PMS_DATA* pms )
{
	// 入力画面が完成するまで、適当に実体を与えておく

	pms->sentence_type = PMS_TYPE_MAIL;
	pms->sentence_id = 5;
	pms->word[0] = 933;
	pms->word[1] = 1041;
}

void PMSDAT_SetDebugRandomDeco( PMS_DATA* pms, HEAPID heapID )
{ 
	// ランダムにデコメ簡易会話データをつっこむ
  int i;
  u32 max;
	
  pms->sentence_type = GFL_STD_MtRand(4);
	pms->sentence_id = GFL_STD_MtRand(5);

  max = get_include_word_max( pms->sentence_type, pms->sentence_id, heapID );

  HOSAKA_Printf("randsetdeco max=%d *** \n",max);

	pms->word[1] = PMS_WORD_NULL; // クリアしておく
  for( i=0; i<max; i++ )
  {
    PMSDAT_SetDeco( pms, i, GFL_STD_MtRand(10) + 1 );
  }
}

void PMSDAT_SetDebugRandom( PMS_DATA* pms )
{
	// ランダムに簡易会話データをつっこむ

	pms->sentence_type = GFL_STD_MtRand(4);
	pms->sentence_id = GFL_STD_MtRand(5);
	pms->word[0] = GFL_STD_MtRand(500);
	pms->word[1] = GFL_STD_MtRand(500);

}

#endif
