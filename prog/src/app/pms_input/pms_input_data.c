//============================================================================================
/**
	* @file	pms_input_data.c
	* @brief	簡易会話入力画面（カテゴリ・イニシャル等のデータ管理）
	* @author	taya
	* @date	06.01.28
	*/
//============================================================================================
#include <gflib.h>

#include "system\pms_word.h"
#include "system\pms_data.h"
//#include "savedata\zukanwork.h"
#include "message.naix"

#include "app\pms_input_data.h"
#include "pms_input_param.h"


#define __PMS_INPUT_RES__
#include "pms_input.res"

#define PMS_CATEGORY_PICTURE_MAX (PMS_DECOID_MAX - 1) ///< デコメ最大数


#define  WORD_ENABLE_FLAG_SIZE		((PMS_WORDNUM_MAX>>3)+1)	// 単語１つ =1bit換算で

#define  INITIAL_MAX	(NELEMS(PMS_InitialTable))

//--------------------------------------------------------------
/**
	*	データ管理オブジェクト実体
	*/
//--------------------------------------------------------------
struct _PMS_INPUT_DATA {
	const PMSI_PARAM*	input_param;
	PMSW_MAN*			word_man;

  // グループ
	u32  GroupEnableWordCnt[ CATEGORY_GROUP_MAX ];
	u32  GroupEnableWordPos[ CATEGORY_GROUP_MAX ];
	PMS_WORD	GroupEnableWord[PMS_WORDNUM_MAX];

  // イニシャル
	u32  InitialEnableWordCnt[ INITIAL_MAX ];
	u32  InitialEnableWordPos[ INITIAL_MAX ];
	PMS_WORD  InitialEnableWord[ PMS_WORDNUM_MAX ];

	u8  word_enable_flag[ WORD_ENABLE_FLAG_SIZE ];

};

//==============================================================
// Prototype
//==============================================================
static void SetupGroupEnableFlag( PMS_INPUT_DATA* pmsi );
static inline void SetWordEnableFlag( PMS_INPUT_DATA* data, u32 pos );
static inline BOOL GetWordEnableFlag( const PMS_INPUT_DATA* data, u32 pos );
static u32 CountupGruopPokemon( PMS_INPUT_DATA* pmsi, const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static u32 CountupGruopSkill( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static u32 CountupGroupNankai( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static u32 CountupGroupAisatsu( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static u32 CountupGroupPicture( PMS_INPUT_DATA* pmsi, const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static u32 CountupGruopDefault( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static u32 CountCategoryZero( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl );
static void SetupInitialEnableFlag( PMS_INPUT_DATA* pmsi );
static u32 CountupInitialWord( PMS_INPUT_DATA* pmsi, const PMS_WORD* src_tbl, PMS_WORD* dst_tbl, u32 dst_pos );


/*====================================================================================================*/
/*                                                                                                    */
/*  オブジェクトの構築・破棄                                                                          */
/*                                                                                                    */
/*====================================================================================================*/
//------------------------------------------------------------------
/**
	* データ管理オブジェクト構築
	*
	* @param   heapID		
	*
	* @retval  PMS_INPUT_DATA*		
	*/
//------------------------------------------------------------------
PMS_INPUT_DATA* PMSI_DATA_Create( u32 heapID, const PMSI_PARAM* input_param )
{
	PMS_INPUT_DATA*  pmsi;
	int i;

	pmsi = GFL_HEAP_AllocMemory( heapID, sizeof(PMS_INPUT_DATA) );

	pmsi->input_param = input_param;
	pmsi->word_man = PMSW_MAN_Create( heapID );

	for(i=0; i<WORD_ENABLE_FLAG_SIZE; i++)
	{
		pmsi->word_enable_flag[i] = 0;
	}

	SetupGroupEnableFlag( pmsi );
	SetupInitialEnableFlag( pmsi );

	return pmsi;
}
//------------------------------------------------------------------
/**
	* データ管理オブジェクト破棄
	*
	* @param   heapID		
	*
	* @retval  PMS_INPUT_DATA*		
	*/
//------------------------------------------------------------------
void PMSI_DATA_Delete( PMS_INPUT_DATA* pmsi )
{
	if( pmsi )
	{
		PMSW_MAN_Delete( pmsi->word_man );
		GFL_HEAP_FreeMemory( pmsi );
	}
}

/*====================================================================================================*/
/*                                                                                                    */
/*  カテゴリごとの有効単語カウント関数テーブル                                                        */
/*                                                                                                    */
/*====================================================================================================*/
typedef u32 (*CountupFunc)( PMS_INPUT_DATA*, const PMS_WORD*, u32, PMS_WORD* );

static const struct {
	CountupFunc      func;			///< 単語カウント関数
	const PMS_WORD*  src_tbl;		///< 単語テーブルアドレス
	u32              tbl_elems;		///< 単語テーブル要素数
}EnableWordCountupParam[CATEGORY_GROUP_MAX] = {
	{ CountupGruopPokemon,  PMS_CategoryTable_01,  PMS_Category_01_MaxNum },	// ポケモン
	{ CountupGruopDefault,  PMS_CategoryTable_03,  PMS_Category_03_MaxNum },	// ステータス
	{ CountupGruopSkill,    PMS_CategoryTable_02,  PMS_Category_02_MaxNum },	// わざ
	{ CountupGruopDefault,  PMS_CategoryTable_04,  PMS_Category_04_MaxNum },	// トレーナー
	{ CountupGruopDefault,  PMS_CategoryTable_10,  PMS_Category_10_MaxNum },	// ユニオン
	{ CountupGroupAisatsu,  PMS_CategoryTable_06,  PMS_Category_06_MaxNum },	// あいさつ
	{ CountupGruopDefault,  PMS_CategoryTable_12,  PMS_Category_12_MaxNum },	// こえ
	{ CountupGruopDefault,  PMS_CategoryTable_05,  PMS_Category_05_MaxNum },	// ひと
	{ CountupGruopDefault,  PMS_CategoryTable_07,  PMS_Category_07_MaxNum },	// せいかつ
	{ CountupGruopDefault,  PMS_CategoryTable_08,  PMS_Category_08_MaxNum },	// きもち
	{ CountupGruopDefault,  PMS_CategoryTable_09,  PMS_Category_09_MaxNum },	// ようご
	{ CountupGroupPicture,  PMS_CategoryTable_11,  PMS_Category_11_MaxNum },	// ピクチャ
};

/*====================================================================================================*/
/*                                                                                                    */
/*  カテゴリごとの有効単語数カウント＆単語IDテーブル化                                                */
/*                                                                                                    */
/*====================================================================================================*/
//------------------------------------------------------------------
/**
  *	@brief  グループ用のデータをセットアップ
	*
	* @param   pmsi		
	*
	*/
//------------------------------------------------------------------
static void SetupGroupEnableFlag( PMS_INPUT_DATA* pmsi )
{
	int i, cnt;

	for(i=0, cnt=0; i<CATEGORY_GROUP_MAX; i++)
	{
		pmsi->GroupEnableWordPos[i] = cnt;
		pmsi->GroupEnableWordCnt[i] = EnableWordCountupParam[i].func(
				pmsi, EnableWordCountupParam[i].src_tbl,
				EnableWordCountupParam[i].tbl_elems,
				&pmsi->GroupEnableWord[cnt] );
		cnt += pmsi->GroupEnableWordCnt[i];
	}

}

//-----------------------------------------------------------------------------
/**
 *	@brief  単語の有効フラグを立てる。単語一つ = 1bit換算
 *
 *	@param	PMS_INPUT_DATA* data
 *	@param	pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static inline void SetWordEnableFlag( PMS_INPUT_DATA* data, u32 pos )
{
	data->word_enable_flag[ (pos >> 3) ] |= (1 << (pos & 7));
#if 0
  HOSAKA_Printf( "SetWordEnableFlag (1<<(pos&7)=%d -> data->word_enable_flag[%d]=%d \n",
      (1<<(pos&7)), pos>>3, data->word_enable_flag[pos>>3] );
#endif
}

static inline BOOL GetWordEnableFlag( const PMS_INPUT_DATA* data, u32 pos )
{
	return (data->word_enable_flag[ (pos >> 3) ] & (1 << (pos & 7))) != 0;
}

static u32 CountupGruopPokemon( PMS_INPUT_DATA* pmsi, const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
  // @TODO ずかんチェック
/*
	const ZUKAN_WORK* zw;
	u32 i, cnt;

	cnt = 0;
	zw = PMSI_PARAM_GetZukanSaveData( pmsi->input_param );
	for(i=0; i<tbl_elems; i++)
	{
		if( ZukanWork_GetPokeSeeFlag( zw, src_tbl[i] ) )
		{
			SetWordEnableFlag( pmsi, src_tbl[i] );
			*dst_tbl++ = src_tbl[i];
			cnt++;
		}
	}

	return cnt;
*/

	u32 i, cnt;

	cnt = 0;
	for(i=0; i<tbl_elems; i++)
	{
		SetWordEnableFlag( pmsi, src_tbl[i] );
		*dst_tbl++ = src_tbl[i];
		cnt++;
	}

	return cnt;
}

static u32 CountupGruopSkill( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
	u32 i;

	// クリアしてたら全部OK
#if PMS_ALLOPEN
	if( TRUE )
#else
	if( PMSI_PARAM_GetGameClearFlag(pmsi->input_param) )
#endif
	{
		for(i=0; i<tbl_elems; i++)
		{
			SetWordEnableFlag( pmsi, *src_tbl );
			*dst_tbl++ = *src_tbl++;
		}
		return tbl_elems;
	}
	else
	{
		return 0;
	}
}

static u32 CountupGroupNankai( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
	const PMSW_SAVEDATA* sv = PMSI_PARAM_GetPMSW_SaveData(pmsi->input_param);
	u32 i, cnt;

	for(i=0, cnt=0; i<tbl_elems; i++)
	{
		if( PMSW_GetNankaiFlag(sv, i) )
		{
			SetWordEnableFlag( pmsi, src_tbl[i] );
			*dst_tbl++ = src_tbl[i];
			cnt++;
		}
	}
	return cnt;
}

static u32 CountupGroupAisatsu( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
	const PMSW_SAVEDATA* sv = PMSI_PARAM_GetPMSW_SaveData(pmsi->input_param);
	PMS_WORD  aisatsu_top, aisatsu_end;
	u32 i, cnt;

	aisatsu_top = PMSW_GetWordNumberByGmmID( NARC_message_pms_word08_dat, 0 );
	aisatsu_end = aisatsu_top + PMSW_AISATSU_HIDE_MAX - 1;

	cnt = 0;

	for(i=0; i<tbl_elems; i++)
	{
		if( (src_tbl[i] >= aisatsu_top) && (src_tbl[i] <= aisatsu_end) )
		{
			u32 id = src_tbl[i] - aisatsu_top;
			if( PMSW_GetAisatsuFlag(sv, id) == FALSE)
			{
				continue;
			}
		}
		SetWordEnableFlag( pmsi, src_tbl[i] );
		*dst_tbl++ = src_tbl[i];
		cnt++;
	}
	return cnt;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  グループ項目初期化 ピクチャ
 *
 *	@param	PMS_INPUT_DATA* pmsi  ワーク
 *	@param	PMS_WORD* src_tbl     テーブル
 *	@param	tbl_elems             テーブルの要素数
 *	@param	dst_tbl               [out] 出力テーブル
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 CountupGroupPicture( PMS_INPUT_DATA* pmsi, const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
	u32 i, cnt;

  // デコメ使用フラグが折れていた場合は項目を無効化
  if( PMSI_PARAM_GetPictureFlag( pmsi->input_param ) == FALSE )
  {
    return 0;
  }

  for(i=0, cnt=0; i<tbl_elems; i++)
  {
		SetWordEnableFlag( pmsi, src_tbl[i] );
    *dst_tbl++ = (PMS_DECOID_ORG + i) + (1<<PMS_WORD_DECO_BITSHIFT); ///< デコメフラグ + デコメID
//	*dst_tbl++ = src_tbl[i];
    cnt++;
  }
  
  return cnt;
}

static u32 CountupGruopDefault( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
	u32 i;

	for(i=0; i<tbl_elems; i++)
	{
		if( *src_tbl == PMS_WORDID_DUP )
		{
			u32 dup;

			src_tbl++;
			dup = *src_tbl++;
			*dst_tbl = *src_tbl;
			while( dup )
			{
				SetWordEnableFlag( pmsi, *src_tbl++ );
				dup--;
			}
		}
		else
		{
			SetWordEnableFlag( pmsi, *src_tbl );
			*dst_tbl++ = *src_tbl++;
		}
	}

	return tbl_elems;
}
#ifdef PM_DEBUG
static u32 CountCategoryZero( PMS_INPUT_DATA* pmsi,  const PMS_WORD* src_tbl, u32 tbl_elems, PMS_WORD* dst_tbl )
{
	return 0;
}
#endif

/*====================================================================================================*/
/*                                                                                                    */
/*  頭文字ごとの有効単語数カウント＆単語IDテーブル化                                                  */
/*  （カテゴリテーブル作成時に作った単語有効フラグを参照する）                                        */
/*                                                                                                    */
/*====================================================================================================*/
static void SetupInitialEnableFlag( PMS_INPUT_DATA* pmsi )
{
	int i, pos = 0;

	for(i=0; i<NELEMS(PMS_InitialTable); i++)
	{
		pmsi->InitialEnableWordPos[i] = pos;
		pmsi->InitialEnableWordCnt[i] = CountupInitialWord(pmsi, PMS_InitialTable[i], pmsi->InitialEnableWord, pos);
		pos += pmsi->InitialEnableWordCnt[i];
	}
}

static u32 CountupInitialWord( PMS_INPUT_DATA* pmsi, const PMS_WORD* src_tbl, PMS_WORD* dst_tbl, u32 dst_pos )
{
	u32 cnt = 0;

	while( *src_tbl != PMS_WORDID_END )
	{
		if( *src_tbl == PMS_WORDID_DUP )
		{
			u32  dup, i;

			src_tbl++;
			dup = *src_tbl++;
			for(i=0; i<dup; i++)
			{
				if( GetWordEnableFlag(pmsi, src_tbl[i]) )
				{
					dst_tbl[dst_pos++] = src_tbl[i];
					cnt++;
					break;
				}
			}
			src_tbl += dup;
		}
		else
		{
			if( GetWordEnableFlag(pmsi, *src_tbl) )
			{
				dst_tbl[dst_pos++] = *src_tbl;
				cnt++;
			}

			src_tbl++;
		}
	}

	return cnt;
}

/*====================================================================================================*/
/*                                                                                                    */
/*  アプリケーションへの情報提供                                                                      */
/*                                                                                                    */
/*====================================================================================================*/

//------------------------------------------------------------------
/**
	* カテゴリグループに含まれる有効単語数
	*
	* @param   pmsi		データ管理オブジェクト
	* @param   group		グループナンバー
	*
	* @retval  u32			有効単語数（０だったらそのカテゴリ自体が無効）
	*/
//------------------------------------------------------------------
u32 PMSI_DATA_GetGroupEnableWordCount( const PMS_INPUT_DATA* pmsi, u32 group )
{
  GF_ASSERT( group < CATEGORY_GROUP_MAX );

	return pmsi->GroupEnableWordCnt[group];
}
//------------------------------------------------------------------
/**
	* カテゴリグループの有効単語を文字列化してバッファにコピー
	*
	* @param   pmsi			データ管理オブジェクト
	* @param   group			グループナンバー
	* @param   word_idx		単語インデックス（0～）
	*
	*/
//------------------------------------------------------------------
void PMSI_DATA_GetGroupEnableWord( const PMS_INPUT_DATA* pmsi, u32 group, u32 word_idx, STRBUF* buf )
{
	u32 pos = pmsi->GroupEnableWordPos[group];
	PMSW_MAN_CopyStr( pmsi->word_man, pmsi->GroupEnableWord[pos+word_idx], buf );
}

//------------------------------------------------------------------
/**
	* カテゴリグループの有効単語の内、指定番の単語コードを返す
	*
	* @param   pmsi			データ管理オブジェクト
	* @param   group			グループナンバー
	* @param   word_idx		単語インデックス
	*
	* @retval  PMS_WORD		単語コード
	*/
//------------------------------------------------------------------
PMS_WORD  PMSI_DATA_GetGroupEnableWordCode( const PMS_INPUT_DATA* pmsi, u32 group, u32 word_idx )
{
	u32 pos = pmsi->GroupEnableWordPos[group];
	return pmsi->GroupEnableWord[pos+word_idx];
}

//------------------------------------------------------------------
/**
	* イニシャルグループに含まれる有効単語数
	*
	* @param   pmsi			データ管理オブジェクト
	* @param   initial			イニシャルナンバー
	*
	* @retval  u32			有効単語数（０だったらそのイニシャル自体が無効）
	*/
//------------------------------------------------------------------
u32 PMSI_DATA_GetInitialEnableWordCount( const PMS_INPUT_DATA* pmsi, u32 initial )
{
  GF_ASSERT(0); //未使用
	return pmsi->InitialEnableWordCnt[initial];
}

//------------------------------------------------------------------
/**
	* イニシャルグループの有効単語を文字列化してバッファにコピー
	*
	* @param   pmsi			データ管理オブジェクト
	* @param   initial			イニシャルナンバー
	* @param   word_idx		単語インデックス（0～）
	*
	*/
//------------------------------------------------------------------
void PMSI_DATA_GetInitialEnableWord( const PMS_INPUT_DATA* pmsi, u32 initial, u32 word_idx, STRBUF* buf )
{
	if( pmsi->InitialEnableWordCnt[initial] )
	{
		PMS_WORD  word_code = PMSI_DATA_GetInitialEnableWordCode(pmsi, initial, word_idx);
		PMSW_MAN_CopyStr( pmsi->word_man, word_code, buf );
	}
}

//------------------------------------------------------------------
/**
	* イニシャルグループの有効単語の内、指定番の単語コードを返す
	*
	* @param   pmsi			データ管理オブジェクト
	* @param   initial			イニシャルナンバー
	* @param   word_idx		単語インデックス（0～）
	*
	*/
//------------------------------------------------------------------
PMS_WORD PMSI_DATA_GetInitialEnableWordCode( const PMS_INPUT_DATA* pmsi, u32 initial, u32 word_idx )
{
	if( pmsi->InitialEnableWordCnt[initial] )
	{
		u32 pos = pmsi->InitialEnableWordPos[initial] + word_idx;
		return pmsi->InitialEnableWord[pos];
	}
	else
	{
		return PMS_WORD_NULL;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  イニシャルテーブルから無効データを除去
 *
 *	@param	const PMS_INPUT_DATA* data
 *	@param	initial
 *	@param	index 
 *
 *	@retval count
 */
//-----------------------------------------------------------------------------
u32 PMSI_DATA_GetInitialEnableWordTable( const PMS_INPUT_DATA* data, const PMS_WORD* src_tbl, PMS_WORD* dst_tbl )
{
  int dummy_pos = 0;
  // const版の関数を作るのも微妙なのでやむなくキャスト..
  return CountupInitialWord( (PMS_INPUT_DATA*)data, src_tbl, dst_tbl, dummy_pos );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  PMS_WORDID_DUP を末尾までスキップする
 *
 *	@param	const PMS_WORD* p_src_tbl 入力
 *
 *	@retval const PMS_WORD* 出力
 */
//-----------------------------------------------------------------------------
static const PMS_WORD* skip_dup( const PMS_WORD* p_src_tbl )
{
  u32 dup;

  p_src_tbl++;
  dup = *p_src_tbl++;
  p_src_tbl += (dup-1); // DUP行の末尾にあわせる

  return p_src_tbl;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ファイル座標からPMS_WORDを算出
 *
 *	@param	const PMS_INPUT_DATA* data 
 *
 *	@note   テーブル中の PMS_WORDID_DUP をスキップするのでテーブルに直接アクセスではいけない。
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PMS_WORD PMSI_DATA_GetWordToOriginalPos( const PMS_INPUT_DATA* data, u32 file_idx, u32 file_pos )
{
  int cnt = 0;
  const PMS_WORD* src_tbl;

  GF_ASSERT( file_idx < NELEMS(PMS_InitialTable) );
  
  src_tbl = &PMS_InitialTable[ file_idx ][0];

  while( *src_tbl != PMS_WORDID_END )
  {
    // 一致
    if( cnt == file_pos )
    {
      // 一致した行がDUPだったら
      if( *src_tbl == PMS_WORDID_DUP )
      {
        src_tbl = skip_dup( src_tbl );
      }

      return *src_tbl;
    }

    // 次の単語を参照
		if( *src_tbl == PMS_WORDID_DUP )
    {
      src_tbl = skip_dup( src_tbl );
    }
    else
    {
			src_tbl++;
    }
    
    cnt++;
  };

  GF_ASSERT(0);

  return 0;
}



//-----------------------------------------------------------------------------
/**
 *	@brief  PMS_WORDから文字列バッファを取得
 *
 *	@param	const PMS_INPUT_DATA* pmsi
 *	@param	word
 *	@param	dst_buf 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSI_DATA_GetWordString( const PMS_INPUT_DATA* pmsi, PMS_WORD word, STRBUF* dst_buf )
{
	PMSW_MAN_CopyStr( pmsi->word_man, word, dst_buf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  指定PMS_WORDの有効無効を取得
 *
 *	@param	const PMS_INPUT_DATA* data
 *	@param	word 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL PMSI_DATA_GetWordEnableFlag( const PMS_INPUT_DATA* data, PMS_WORD word )
{
  return GetWordEnableFlag( data, word );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  テーブル末尾につけるデータを取得
 *
 *	@param	const PMS_INPUT_DATA* data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 PMSI_DATA_GetWordTableEndData( const PMS_INPUT_DATA* data )
{
  return PMS_WORDID_END;
}

//======================================================================================
// ボックス壁紙パスワード用処理
//======================================================================================
static const u8 BoxPwdSrcGroup[] = {
	CATEGORY_GROUP_STATUS,
	CATEGORY_GROUP_TRAINER,
	CATEGORY_GROUP_PERSON,
	CATEGORY_GROUP_LIFE,
	CATEGORY_GROUP_MIND,
};



//-----------------------------------------------------------------------------
/**
 *	@brief  ボックスパスワードの最大値
 *
 *	@param	none
 *
 *	@retval 最大値
 */
//-----------------------------------------------------------------------------
int PMSI_DAT_GetBoxPwdMax(void)
{
	int i, max;
	for(i=0, max=0; i<NELEMS(BoxPwdSrcGroup); i++)
	{
		max += EnableWordCountupParam[ BoxPwdSrcGroup[i] ].tbl_elems;
	}
	return max;
}



//-----------------------------------------------------------------------------
/**
 *	@brief  PMS_WORDからボックスパスワードIDを計算
 *
 *	@param	PMS_WORD word 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
int PMSI_DAT_WordToBoxPwdID( PMS_WORD word )
{
	int i, w, tbl_max, pwd_id = 0;
	const PMS_WORD* tbl;

	for(i=0; i<NELEMS(BoxPwdSrcGroup); i++)
	{
		tbl = EnableWordCountupParam[ BoxPwdSrcGroup[i] ].src_tbl;
		tbl_max = EnableWordCountupParam[ BoxPwdSrcGroup[i] ].tbl_elems;

		for(w=0; w<tbl_max; w++)
		{
			if( tbl[w] == word )
			{
				return pwd_id;
			}
			pwd_id++;
		}
	}

	return -1;
}

