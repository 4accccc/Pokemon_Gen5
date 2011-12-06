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
#define POKEWOOD_ARRAY_NUM  (( POKEWOOD_SENARIO_MAX/32 )+1)
#define POKEWOOD_RECSAVEDATA_MAX  ( 8 ) // 正確にはpokewood_rec.hを参照
#define POKEWOOD_KANBAN_MAX       ( 4 ) // ポケウッド劇場の外の看板の最大数

// 初期化時の興行収入値
#define POKEWOOD_BOXOFFICE_FIRST_PROFITS ( -9999 )

//----------------------------------------------------
// ポケウッドセーブデータ実体
//----------------------------------------------------
struct _POKEWOOD_SAVEDATA{

  u32 open_flag[POKEWOOD_ARRAY_NUM];          ///< ポケウッド解放フラグ      8
  u32 played_flag[POKEWOOD_ARRAY_NUM];        ///< ポケウッド撮影済みフラグ  8
  u32 success_flag[POKEWOOD_ARRAY_NUM];       ///< 成功フラグ取得済みフラグ  8
  u32 announce_flag[POKEWOOD_ARRAY_NUM];      ///< ポケウッドシナリオ解放告知済フラグ 8
  u8  showed_flag[POKEWOOD_RECSAVEDATA_MAX];  ///< 上映済みフラグ            8
                                                                      //     =40
  POKEWOOD_BOXOFFICE_RECORD boxoffice_record[POKEWOOD_BOXOFFICE_DATA_MAX];  // 興行成績ランキング 16*40
  s16 star_point;                             ///< プレイヤーのスターポイント 2
  s16 star_rank_announce;                     ///< スターポイントが上がった事を告知したか
  u8  title_no[POKEWOOD_KANBAN_MAX];         ///< 上映中の映画の看板ナンバー
  u32 strange_flag[POKEWOOD_ARRAY_NUM];       ///< ストレンジエンド取得済みフラグ
  u32 senario_theatergo_flag[POKEWOOD_ARRAY_NUM];///< 指定のシナリオを一度でも上映して観た事がある
  u32 bad_flag[POKEWOOD_ARRAY_NUM];           ///< バッドエンド取得済みフラグ

  u8 array[124];                              ///< 余り
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
  int i;
	MI_CpuClear8( sv, sizeof(POKEWOOD_SAVEDATA));

  // 成績モニターデータ初期化
  for(i=0;i<POKEWOOD_BOXOFFICE_DATA_MAX;i++){
    sv->boxoffice_record[i].profits = POKEWOOD_BOXOFFICE_FIRST_PROFITS;
  }

  // 初期シナリオオープン状態を作成
  POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( sv, 0 ); /// ポケウッド初期状態/ハチクマン/チュートリアル
  POKEWOOD_SAVEDATA_SetPokewoodFlag( sv, PWSV_TYPE_SENARIO_ANNOUNCED, 0 );

  // スターポイント告知済みフラグ設定
  POKEWOOD_SAVEDATA_SetStarRankAnnounced( sv, 0, 1 );
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

//----------------------------------------------------------------------------------
/**
 * @brief	各々のフラグに関してテーブルポインタを返す
 */
//----------------------------------------------------------------------------------
static u32* _get_flagtable_pointer( POKEWOOD_SAVEDATA *sv, PokewoodFlagType type )
{
  switch(type){
  case PWSV_TYPE_SENARIO_OPEN:
    return sv->open_flag;
  case PWSV_TYPE_SENARIO_PLAYED:
    return sv->played_flag;
  case PWSV_TYPE_SENARIO_SUCCESS:
    return sv->success_flag;
  case PWSV_TYPE_SENARIO_ANNOUNCED:
    return sv->announce_flag;
  case PWSV_TYPE_SENARIO_STRANGE:
    return sv->strange_flag;
  case PWSV_TYPE_SENARIO_THEATERGO:
    return sv->senario_theatergo_flag;
  case PWSV_TYPE_SENARIO_BAD:
    return sv->bad_flag;
  }
  GF_ASSERT(0);
  return NULL;
}

//=============================================================================================
/**
 * @brief	指定タイプのフラグを返す
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   type	PWSV_TYPE_～(pokewood_savedata.hに記述）
 * @param   no		シナリオNO
 *
 * @retval  BOOL	TRUE or FALSE
 */
//=============================================================================================
BOOL POKEWOOD_SAVEDATA_GetPokewoodFlag( const POKEWOOD_SAVEDATA *sv, PokewoodFlagType type, int no )
{
  int offset, shift;
  u32 *tbl=_get_flagtable_pointer( (POKEWOOD_SAVEDATA *)sv, type );

  _get_shift_offset( no, &offset, &shift );
  return (tbl[offset]>>shift)&1;
}

//=============================================================================================
/**
 * @brief	指定タイプのフラグをセット
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   type	PWSV_TYPE_～(pokewood_savedata.hに記述）
 * @param   no		シナリオNO
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetPokewoodFlag( POKEWOOD_SAVEDATA *sv, PokewoodFlagType type, int no )
{
  int offset, shift;
  u32 *tbl=_get_flagtable_pointer( sv, type );

  _get_shift_offset( no, &offset, &shift );
  tbl[offset] |= (1<<shift);
  
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
  return POKEWOOD_SAVEDATA_GetPokewoodFlag( sv, PWSV_TYPE_SENARIO_OPEN, no );
}
void POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( POKEWOOD_SAVEDATA *sv, int no )
{
  POKEWOOD_SAVEDATA_SetPokewoodFlag( sv, PWSV_TYPE_SENARIO_OPEN, no );
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
  return POKEWOOD_SAVEDATA_GetPokewoodFlag( sv, PWSV_TYPE_SENARIO_PLAYED, no );
}
void POKEWOOD_SAVEDATA_SetPokewoodPlayedFlag( POKEWOOD_SAVEDATA *sv, int no )
{
  POKEWOOD_SAVEDATA_SetPokewoodFlag( sv, PWSV_TYPE_SENARIO_PLAYED, no );
}

//=============================================================================================
/**
 * @brief	このシナリオは成功フラグを立てているか？
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		シナリオNO
 *
 * @retval  BOOL	TRUE:上映済み FALSE:上映してない
 */
//=============================================================================================
BOOL POKEWOOD_SAVEDATA_GetSuccessFlag( POKEWOOD_SAVEDATA *sv, int no )
{
  GF_ASSERT_MSG( no<POKEWOOD_RECSAVEDATA_MAX , "上映数の最大を超えています=%d\n",no);
  return POKEWOOD_SAVEDATA_GetPokewoodFlag( sv, PWSV_TYPE_SENARIO_SUCCESS, no );
}
void POKEWOOD_SAVEDATA_SetSuccessFlag( POKEWOOD_SAVEDATA *sv, int no, BOOL value )
{
  GF_ASSERT_MSG( no<POKEWOOD_RECSAVEDATA_MAX , "上映数の最大を超えています=%d\n",no);
  POKEWOOD_SAVEDATA_SetPokewoodFlag( sv, PWSV_TYPE_SENARIO_SUCCESS, no );
}


//=============================================================================================
/**
 * @brief	興行成績を登録（TOP10に入っていれば記録に残ります）
 *
 * @param   sv	  POKEWOOD_SAVEDATR
 * @param   data  POKEWOOD_BOXOFFICE_RECORD
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetBoxOfficeData( POKEWOOD_SAVEDATA *sv, int no, POKEWOOD_BOXOFFICE_RECORD *data )
{
  // 興行収入が上回っている場合はランキングに残す
  if(data->profits >= sv->boxoffice_record[no].profits){
    // 初回登録
    if(sv->boxoffice_record[no].new_flag==0){
      sv->boxoffice_record[no]             = *data;
    // 興行収入が上回った時の登録
    }else{
      sv->boxoffice_record[no]             = *data;
      sv->boxoffice_record[no].update_flag = 1;
      sv->boxoffice_record[no].new_flag    = 1;
    }
  }
}

//=============================================================================================
/**
 * @brief	興行成績データの更新状態をOFFにする
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		シナリオNO
 */
//=============================================================================================
void  POKEWOOD_SAVEDATA_ClearUpdate( POKEWOOD_SAVEDATA *sv, int no )
{
  GF_ASSERT_MSG( no < POKEWOOD_BOXOFFICE_DATA_MAX, "ランキングの最大を超えています=%d\n",no);
  sv->boxoffice_record[no].new_flag    = 1;
  sv->boxoffice_record[no].update_flag = 0;
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
  GF_ASSERT_MSG( no < POKEWOOD_BOXOFFICE_DATA_MAX, "ランキングの最大を超えています=%d\n",no);
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
  GF_ASSERT_MSG( no <POKEWOOD_RECSAVEDATA_MAX , "上映数の最大を超えています=%d\n",no);

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
void POKEWOOD_SAVEDATA_SetMovieShowed( POKEWOOD_SAVEDATA *sv, int no, BOOL value )
{
  GF_ASSERT_MSG( no<POKEWOOD_RECSAVEDATA_MAX , "上映数の最大を超えています=%d\n",no);
  
  sv->showed_flag[no] = value;

}


enum{
  POKEWOOD_STAR_POINT_MIN =  0,
  POKEWOOD_STAR_POINT_MAX = 99,
};


//=============================================================================================
/**
 * @brief	プレイヤーのスターポイントを返す
 *
 * @param   sv		
 *
 * @retval  u8		0-99
 */
//=============================================================================================
int POKEWOOD_SAVEDATA_GetStarPoint( POKEWOOD_SAVEDATA *sv )
{
  GF_ASSERT(sv);
  
  // 範囲外は0に修正
  if(sv->star_point>POKEWOOD_STAR_POINT_MAX || sv->star_point<POKEWOOD_STAR_POINT_MIN){
    sv->star_point = 0;
  }
  return sv->star_point;
}

//=============================================================================================
/**
 * @brief	プレイヤーのスターポイントを足す
 *
 * @param   sv		
 * @param   value		
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_AddStarPoint( POKEWOOD_SAVEDATA *sv, int value )
{
  GF_ASSERT(sv);
  sv->star_point += value;
  if(sv->star_point>POKEWOOD_STAR_POINT_MAX){
    sv->star_point = POKEWOOD_STAR_POINT_MAX;
  }

}

//=============================================================================================
/**
 * @brief	プレイヤーのスターポイントを引く
 *
 * @param   sv		
 * @param   value		
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_DecStarPoint( POKEWOOD_SAVEDATA *sv, int value )
{
  GF_ASSERT(sv);
  sv->star_point -= value;
  if(sv->star_point<POKEWOOD_STAR_POINT_MIN){
    sv->star_point = POKEWOOD_STAR_POINT_MIN;
  }
}

//=============================================================================================
/**
 * @brief	プレイヤーのスターポイントを直接設定する
 *
 * @param   sv		  
 * @param   value		設定したい値
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetStarPoint( POKEWOOD_SAVEDATA *sv, int value )
{
  POKEWOOD_SAVEDATA_DecStarPoint( sv, POKEWOOD_STAR_POINT_MAX );
  POKEWOOD_SAVEDATA_AddStarPoint( sv, value );
}


#define POKEWOOD_STARRANK0_MIN    ( 0  )
#define POKEWOOD_STARRANK0_MAX    ( 20 )
#define POKEWOOD_STARRANK1_MAX    ( 40 )
#define POKEWOOD_STARRANK2_MAX    ( 70 )
#define POKEWOOD_STARRANK3_MAX    ( 95 )
#define POKEWOOD_STARRANK4_MAX    ( 99 )
//=============================================================================================
/**
 * @brief	プレイヤースターポイントをスターランクに変換する
 *
 * @param   point	スターポイント
 *
 * @retval  int		スターランク( 0-4 )
 */
//=============================================================================================
int POKEWOOD_SAVEDATA_CalcStarPoint2StarRank( int point )
{
  if(point>=POKEWOOD_STARRANK0_MIN && point<POKEWOOD_STARRANK0_MAX){
    return 0;
  }else if(point>=POKEWOOD_STARRANK0_MAX && point<POKEWOOD_STARRANK1_MAX){
    return 1;
  }else if(point>=POKEWOOD_STARRANK1_MAX && point<POKEWOOD_STARRANK2_MAX){
    return 2;
  }else if(point>=POKEWOOD_STARRANK2_MAX && point<POKEWOOD_STARRANK3_MAX){
    return 3;
  }else if(point>=POKEWOOD_STARRANK3_MAX && point<=POKEWOOD_STARRANK4_MAX){
    return 4;
  }
  
  GF_ASSERT_MSG(0,"スターランク確認：ここに来たらおかしい\n");
  return 0;
}

//=============================================================================================
/**
 * @brief	指定のスターランクを告知済みか？
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   rank	スターランク（スターポインｔをPOKEWOOD_SAVEDATA_CalcStarPoint2StarRankで変換してください）
 *
 * @retval  BOOL	告知済みであればTRUE.まだであればFALSE
 */
//=============================================================================================
BOOL POKEWOOD_SAVEDATA_GetStarRankAnnounced( POKEWOOD_SAVEDATA *sv, int rank )
{
  return (sv->star_rank_announce>>rank)&1;
}

void POKEWOOD_SAVEDATA_SetStarRankAnnounced( POKEWOOD_SAVEDATA *sv, int rank, BOOL value )
{
  sv->star_rank_announce &= (0xffff^(1<<rank));
  sv->star_rank_announce |= (value<<rank);
}


//=============================================================================================
/**
 * @brief	上映中の看板ワークを設定
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   index	上映映画INDEX(0-4)
 *
 * @retval  int		看板NO
 */
//=============================================================================================
int POKEWOOD_SAVEDATA_GetKanbanNo( POKEWOOD_SAVEDATA *sv, u32 index )
{
  GF_ASSERT_MSG( index<POKEWOOD_KANBAN_MAX, "上映映画のindexが4を超えている\n" );
  return sv->title_no[index];
}

//=============================================================================================
/**
 * @brief	上映中の看板ワークを設定
 *
 * @param   sv        POKEWOOD_SAVEDATA
 * @param   index	    上映映画INDEX(0-4)
 * @param   title_no	設定する看板NO
 */
//=============================================================================================
void POKEWOOD_SAVEDATA_SetKanbanNo( POKEWOOD_SAVEDATA *sv, u32 index, int title_no )
{
  // セーブデータが４つ以上になったので、押し出し式に変更
//  if(index<POKEWOOD_KANBAN_MAX){
//    sv->title_no[index] = title_no;
//  }

  // 最新の看板は常に０として扱い、登録されるたびに押し出していく３を超えたら廃棄
  sv->title_no[3] = sv->title_no[2];
  sv->title_no[2] = sv->title_no[1];
  sv->title_no[1] = sv->title_no[0];
  sv->title_no[0] = title_no;
}

//=============================================================================================
/**
 * @brief	各種ポケウッド進行フラグの総数を返す
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   type	PokewoodFlagType
 *
 * @retval  int		合計数
 */
//=============================================================================================
int POKEWOOD_SAVEDATA_GetFlagTotal( POKEWOOD_SAVEDATA *sv, PokewoodFlagType type )
{
  
  int i,count=0;
  for(i=0;i<POKEWOOD_BOXOFFICE_DATA_MAX;i++){
    if(POKEWOOD_SAVEDATA_GetPokewoodFlag( sv, type, i )){
      count++;
    }
  }
  return count;

}

//=============================================================================================
/**
 * @brief	ポケウッドシナリオオープンの総数を返す
 *
 * @param   sv		POKEWOOD_SAVEDATA
 *
 * @retval  int		ポケウッドシナリオオープンの総数
 */
//=============================================================================================
int POKEWOOD_SAVEDATA_GetSenarioOpenNum( POKEWOOD_SAVEDATA *sv )
{
  return POKEWOOD_SAVEDATA_GetFlagTotal( sv, PWSV_TYPE_SENARIO_OPEN );
}

#ifdef PM_DEBUG
static u8 debugPokewoodBattleFlag = 0;   // ポケウッドの戦闘デバッグフラグ反映に使用

BOOL POKEWOOD_SAVEDATA_GetBattleDebugFlag( int index )
{
  return (debugPokewoodBattleFlag>>index) & 1;
}
void POKEWOOD_SAVEDATA_SetBattleDebugFlag( int index, int value )
{
  debugPokewoodBattleFlag &= 0xff ^ (1<<index);
  debugPokewoodBattleFlag |= (value<<index);
}
#endif