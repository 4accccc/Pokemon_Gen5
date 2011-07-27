//============================================================================================
/**
 * @file    pokewood_save.c
 * @brief   ポケウッドセーブデータ
 * @author　Akito Mori
 * @date    11.07.26
 */
//============================================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/pokewood_savedata.h"

//SDK_COMPILER_ASSERT( FMISSION_ID_MAX <= FMISSION_STATUS_MAX );

// フラグ管理に必要な配列用の個数
#define POKEWOOD_ARRAY_NUM (( POKEWOOD_SENARIO_MAX/32 )+1)

#define POKEWOOD_RECSAVEDATA_MAX  ( 8 ) //正確にはpokewood_rec.hを参照

struct _POKEWOOD_SAVEDATA{

  u32 open_flag[POKEWOOD_ARRAY_NUM];          ///< ポケウッド解放フラグ      8
  u32 played_flag[POKEWOOD_ARRAY_NUM];        ///< ポケウッド撮影済みフラグ  16
  u8  showed_flag[POKEWOOD_RECSAVEDATA_MAX];  ///< 上映済みフラグ

  POKEWOOD_BOXOFFICE_RECORD boxoffice_record[POKEWOOD_BOXOFFICE_DATA_MAX];  // 興行成績ランキング 32*10+16

  u8 array[164];
};

//---------------------------------------------------------------------------
/**
 * @brief	ワークのサイズ取得
 * @return	int		サイズ
 */
//---------------------------------------------------------------------------
int POKEWOOD_SAVEDATA_GetWorkSize(void)
{
	return sizeof(POKEWOOD_SAVEDATA);
}

//---------------------------------------------------------------------------
/**
 * @brief	ワークの初期化
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void POKEWOOD_SAVEDATA_Init( POKEWOOD_SAVEDATA * sv )
{
	MI_CpuClear8( sv, sizeof(POKEWOOD_SAVEDATA));

  POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( sv, 0 ); /// ポケウッド初期状態
  POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( sv, 1 ); // ハチクマン
  POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( sv, 2 ); 

}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケウッドセーブデータ取得（GAMEDETAから）
 *
 * @param		gamedata		GAMEDATA
 *
 * @return  ポケウッドセーブデータ
 *
 */
//--------------------------------------------------------------------------------------------
POKEWOOD_SAVEDATA * GAMEDATA_GetPokewoodSavedata( GAMEDATA * gamedata )
{
	return SaveControl_DataPtrGet( GAMEDATA_GetSaveControlWork(gamedata), GMDATA_ID_POKEWOOD );
}

/// ポケウッドのフラグデータはu32に1bitずつ割り当てられる
#define POKEWOOD_SHIFT_MAX  ( 32 )
//----------------------------------------------------------------------------------
/**
 * @brief	noから配列の場所とシフト数を取得
 * @param   no		[in]入力値
 * @param   offset	[out]配列の添え字
 * @param   shift		[out]シフト数
 */
//----------------------------------------------------------------------------------
static void _get_shift_offset( int no, int *offset, int *shift )
{
  GF_ASSERT_MSG(no<POKEWOOD_SENARIO_MAX, "シナリオ%dは最大を超えた指定です\n", no);
  *offset = no / POKEWOOD_SHIFT_MAX;
  *shift  = no-*offset*POKEWOOD_SHIFT_MAX;
}

//=============================================================================================
/**
 * @brief	ポケウッドシナリオ解放状態を取得
 * @param   sv	POKEWOOD_SAVEDATA
 * @param   no		シナリオＮＯ
 * @retval  BOOL	TRUE:解放済み FALSEまだ
 */
//=============================================================================================
BOOL POKEWOOD_SAVEDATA_GetPokewoodOpenFlag( const POKEWOOD_SAVEDATA *sv, int no )
{
  int offset, shift;
  _get_shift_offset( no, &offset, &shift );

  return (sv->open_flag[offset]>>shift)&1;
}
//=============================================================================================
/**
 * @brief	ポケウッドシナリオ解放状態を設定
 * @param   sv	POKEWOOD_SAVEDATA
 * @param   no		シナリオＮＯ
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( POKEWOOD_SAVEDATA *sv, int no )
{
  int offset, shift;
  _get_shift_offset( no, &offset, &shift );

  sv->open_flag[offset] |= (1<<shift);
}
//=============================================================================================
/**
 * @brief	ポケウッドシナリオプレイ状態を取得
 * @param   sv	POKEWOOD_SAVEDATA
 * @param   no		シナリオＮＯ
 * @retval  BOOL	TRUE:未撮影 FALSE:撮影済み
 */
//=============================================================================================
BOOL POKEWOOD_SAVEDATA_GetPokewoodPlayedFlag( const POKEWOOD_SAVEDATA *sv, int no )
{
  int offset, shift;
  _get_shift_offset( no, &offset, &shift );

  return (sv->played_flag[offset]>>shift)&1;
  
}
//=============================================================================================
/**
 * @brief	ポケウッドシナリオプレイ状態を設定（撮影済みに書き換える）
 * @param   sv	POKEWOOD_SAVEDATA
 * @param   no		シナリオＮＯ
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetPokewoodPlayedFlag( POKEWOOD_SAVEDATA *sv, int no )
{
  int offset, shift;
  _get_shift_offset( no, &offset, &shift );

  sv->played_flag[offset] |= (1<<shift);
  
}

//=============================================================================================
/**
 * @brief	興行成績を登録（TOP10に入っていれば記録に残ります）
 *
 * @param   sv	  POKEWOOD_SAVEDATR
 * @param   data  POKEWOOD_BOXOFFICE_RECORD
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetBoxOfficeData( POKEWOOD_SAVEDATA *sv, POKEWOOD_BOXOFFICE_RECORD *data )
{
  return;
}

//=============================================================================================
/**
 * @brief	興行成績上位者のデータを取得する
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		no
 *
 * @retval  const POKEWOOD_BOXOFFICE_RECORD *		
 */
//=============================================================================================
const POKEWOOD_BOXOFFICE_RECORD *POKEWOOD_SAVEDATA_GetBoxOfficeData( POKEWOOD_SAVEDATA *sv, int no )
{
  GF_ASSERT_MSG( no >= POKEWOOD_BOXOFFICE_DATA_MAX, "ランキングの最大を超えています=%d\n",no);
  return &sv->boxoffice_record[no];
}

//=============================================================================================
/**
 * @brief	外部データの撮影データを上映したか？
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		撮影データスロット
 *
 * @retval  BOOL	TRUE:上映済み FALSE:上映してない
 */
//=============================================================================================
BOOL POKEWOOD_SAVEDATA_GetMovieShowed( POKEWOOD_SAVEDATA *sv, int no )
{
  GF_ASSERT_MSG( no >=POKEWOOD_RECSAVEDATA_MAX , "上映数の最大を超えています=%d\n",no);

  return sv->showed_flag[no];
}

//=============================================================================================
/**
 * @brief	撮影データを上映フラグを設定する
 *
 * @param   sv    POKEWOOD_SAVEDATA
 * @param   no    撮影データスロット
 * @param   value TRUE or FALSE
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetMovieShowd( POKEWOOD_SAVEDATA *sv, int no, BOOL value )
{
  GF_ASSERT_MSG( no >=POKEWOOD_RECSAVEDATA_MAX , "上映数の最大を超えています=%d\n",no);
  
  sv->showed_flag[no] = value;

}
