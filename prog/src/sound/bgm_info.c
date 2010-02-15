#include <gflib.h>
#include "bgm_info.h"
#include "arc/arc_def.h"
#include "arc/bgm_info.naix"
#include "../../../resource/sound/bgm_info/iss_type.h"


//=============================================================================================
/**
 *
 * @brief 定数
 * 
 */
//=============================================================================================
//#define DEBUG_PRINT_ON
#define ARC_ID      (ARCID_BGM_INFO)				// BGM情報のアーカイブ・インデックス
#define ARC_DATA_ID (NARC_bgm_info_bgm_info_bin)	// アーカイブ内データ・インデックス


//=============================================================================================
/**
 *
 * @brief BGM情報構造体
 * 
 */
//=============================================================================================
struct _BGM_INFO_SYS_WORK
{
	// データ数
	u8 dataNum;		

	// 以下, 要素数dataNumの配列
	u16* seqIndex;		// シーケンス番号
	u8*  issType;		// ISSタイプ
};


//=============================================================================================
/**
 *
 * @brief 非公開関数のプロトタイプ宣言
 * 
 */
//=============================================================================================
// BGM情報を読み込む
void LoadInfo( BGM_INFO_SYS* p_sys, HEAPID heap_id );


//=============================================================================================
/**
 *
 * @brief システム作成・破棄関数
 *
 */
//=============================================================================================

//----------------------------------------------------------------------------
/**
 * @brief BGM情報管理システムを作成する
 *
 * @param heap_id ヒープID
 *
 * @return システムワークへのポインタ
 */
//----------------------------------------------------------------------------
BGM_INFO_SYS* BGM_INFO_CreateSystem( HEAPID heap_id )
{
	BGM_INFO_SYS* p_sys;

	// システムワークを確保
	p_sys = (BGM_INFO_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( BGM_INFO_SYS ) );

	// データ読み込み
	LoadInfo( p_sys, heap_id );

	// 作成したシステムワークを返す
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 * @brief BGM情報管理システムを破棄する
 *
 * @param p_sys 破棄するシステム
 */
//----------------------------------------------------------------------------
void BGM_INFO_DeleteSystem( BGM_INFO_SYS* p_sys )
{
	// システムが持つデータを破棄
	GFL_HEAP_FreeMemory( p_sys->seqIndex );
	GFL_HEAP_FreeMemory( p_sys->issType );
	
	// 本体を破棄
	GFL_HEAP_FreeMemory( p_sys );

	// DEBUG:
#ifdef DEBUG_PRINT_ON
	OBATA_Printf( "-----------------free size = %d\n", sizeof( u16 ) * p_sys->dataNum );
	OBATA_Printf( "-----------------free size = %d\n", sizeof( u8 ) * p_sys->dataNum );
	OBATA_Printf( "-----------------free size = %d\n", sizeof( BGM_INFO_SYS ) );
#endif
}


//====================================================================================
/**
 *
 * @brief データ取得関数
 *
 */
//====================================================================================

//----------------------------------------------------------------------------
/**
 * @brief 指定したBGMのISSタイプを取得する
 * 
 * @param p_sys   取得対象のシステム
 * @param seq_idx BGMのシーケンス・インデックス( SEQ_XXXX )
 *
 * @return 指定BGMのISSタイプ( 街ISS, 道路ISS など )
 */
//----------------------------------------------------------------------------
u8 BGM_INFO_GetIssType( BGM_INFO_SYS* p_sys, u16 seq_idx )
{
	int i;

	// シーケンス番号をキーに検索
	for( i=0; i<p_sys->dataNum; i++ )
	{
		// 指定されたシーケンス番号発見したら, そのISSタイプを返す
		if( p_sys->seqIndex[i] == seq_idx )
		{
			return p_sys->issType[i];
		}
	}

	// 指定シーケンス番号がテーブルに登録されていない場合, 通常BGMとする
	return ISS_TYPE_NORMAL;
}


//=============================================================================================
/**
 *
 * @brief 非公開関数の実装
 * 
 */
//=============================================================================================

//----------------------------------------------------------------------------
/**
 * @brief BGM情報を読み込む
 *
 * @param p_sys   読み込んだデータの格納先システム
 * @param heap_id 使用するヒープID
 *
 * [バイナリ・データのフォーマット]
 * [0] データ数
 * [1] シーケンス番号
 * [2]
 * [3] ISSタイプ
 * 以下、[1]-[3]がデータの数だけ存在
 */
//---------------------------------------------------------------------------- 
void LoadInfo( BGM_INFO_SYS* p_sys, HEAPID heap_id )
{
	int i;
	int pos = 0;

	// データ数を取得
	GFL_ARC_LoadDataOfs( &p_sys->dataNum, ARC_ID, ARC_DATA_ID, pos, sizeof( u8 ) );
	pos += 1;

	// 各種データの配列を確保
	p_sys->seqIndex = (u16*)GFL_HEAP_AllocMemory( heap_id, sizeof( u16 ) * p_sys->dataNum );
	p_sys->issType  = ( u8*)GFL_HEAP_AllocMemory( heap_id, sizeof(  u8 ) * p_sys->dataNum );

	// 各種データを取得
	for( i=0; i<p_sys->dataNum; i++ )
	{
		// シーケンス番号
		GFL_ARC_LoadDataOfs( &p_sys->seqIndex[i], ARC_ID, ARC_DATA_ID, pos, sizeof( u16 ) );
		pos += 2;

		// ISSタイプ
		GFL_ARC_LoadDataOfs( &p_sys->issType[i], ARC_ID, ARC_DATA_ID, pos, sizeof( u8 ) );
		pos += 1;
	}

	// DEBUG:
#ifdef DEBUG_PRINT_ON
	OBATA_Printf( "--------------------------------------\n" );
	OBATA_Printf( "dataNum = %d\n", p_sys->dataNum );
	for( i=0; i<p_sys->dataNum; i++ )
	{
		OBATA_Printf( "[%d]: seq=%d, iss=%d\n", i, p_sys->seqIndex[i], p_sys->issType[i] );
	}
#endif
}
