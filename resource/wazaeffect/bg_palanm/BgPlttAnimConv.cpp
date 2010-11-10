//=============================================================================
/**
 *
 *	@file		BgPlttAnimConv.cpp
 *	@brief		.nce から、パレットアニメーションのデータを取得する
 *	@author		hosaka genya
 *	@data		2008.05.19
 *
 */
//=============================================================================
#include <stdio.h>
#include <string>
#include <iostream>
#include <windows.h>

#include "bg_planm_data.h"

//#define DEBUG	///< デバッグ用処理 切り替えマクロ

using namespace std;

//--------------------------------------------------------------
// 定数
//==============================================================
enum
{
	CELL_IDX_END	= BG_PLANM_PLTT_DATA_END,
	CONV_DAT_MAX	= BG_PLANM_DAT_MAX,			///< 要素の最大数
};

//--------------------------------------------------------------
// グローバル変数
//==============================================================
int g_cell_pltt_no[ CONV_DAT_MAX ];	///< パレットデータ
int g_cell_idx[ CONV_DAT_MAX ];
int g_cell_wait[ CONV_DAT_MAX ];


//--------------------------------------------------------------
static void out_data_set( FILE* fp )
{
	BG_PLANM_CONV_DATA conv_dat;

	ZeroMemory( &conv_dat, sizeof(BG_PLANM_CONV_DATA) );

	// -- データ設定 --
	for( int i=0; i<BG_PLANM_DAT_MAX; i++ )
	{
		int pltt_no = ( g_cell_idx[i] == CELL_IDX_END ) ? CELL_IDX_END : g_cell_pltt_no[ g_cell_idx[i] ];
		conv_dat.pltt_no[i]	= pltt_no;
		conv_dat.wait[i]	= g_cell_wait[i];
		if( g_cell_idx[i] == CELL_IDX_END )
		{
			break;
		}
	}
	
	fwrite( &conv_dat, sizeof(BG_PLANM_CONV_DATA), 1, fp );
}

//--------------------------------------------------------------
// -- パレットデータ書き出し --
void write_data( string str )
{
	FILE*	fp;
	string	str_w;

	// 拡張子を変更
	str.replace( str.find_last_of(".") , 4, ".dat" );

	// ファイルオープン
	fp = fopen( str.c_str(), "wb" );

	if( fp == NULL )
	{
		cout << "file open error" << endl;
		exit( EXIT_FAILURE );
	}

	out_data_set( fp );

	fclose( fp );
}

// -- ブロック読み込みの成否チェック --
static void block_check_type( const char* block_type, const char* block_type_result )
{
	if (block_type[0] == block_type_result[0] &&
		block_type[1] == block_type_result[1] &&
		block_type[2] == block_type_result[2] &&
		block_type[3] == block_type_result[3] )
	{
		return;
	}
	else
	{
		cout << "-- block check error --" << endl;
		cout << "BlockType = " << block_type << endl;
		exit( EXIT_FAILURE );
	}
}

// -- ブロック読み飛ばし処理 --
void block_skip( FILE* fp, const char* block_type_result )
{
	char block_type[4];
	int size;

	// 識別符号を読み込む
	fread( block_type, 4, 1, fp );
	// 識別符号チェック
	block_check_type( block_type, block_type_result );
	// ブロックサイズを取得
	fread( &size, 4, 1, fp );
	// ブロックサイズ - 既に fread した値をスキップ
	fseek( fp, size-8, SEEK_CUR );
	
#ifdef DEBUG
	cout << block_type << endl;
	printf( "%d \n", size );
#endif
}

// -- セルブロックからパレット情報を取得 --
static void block_cell_read( FILE* fp )
{
	char block_type[4];
	int block_size;
	int cell_num;
	int obj_num;
	
	// 識別符号を読み込む
	fread( block_type, 4, 1, fp );
	// 識別符号チェック
	block_check_type( block_type, "CELL" );
	// ブロックサイズを取得
	fread( &block_size, 4, 1, fp );
	// セルの枚数を取得
	fread( &cell_num, 4, 1, fp );
	
	for( int i=0; i<cell_num; i++ )
	{
		//-------------------------------------------
		// 最初のOAMからパレット情報を取得
		//-------------------------------------------
		// OBJ数を取得
		fread( &obj_num, 4, 1, fp );
		//パレットデータまで進める
		fseek( fp, 15, SEEK_CUR );
		// パレットナンバーを取得
		fread( &g_cell_pltt_no[i], 1, 1, fp );
		// 次のOBJの頭までスキップ
		fseek( fp, 4, SEEK_CUR );
		// 次のCELLの頭までスキップ
		fseek( fp, ( 20 * (obj_num-1) ), SEEK_CUR );

#ifdef DEBUG
		cout << "i : " << i << ",\tobj_num : " << obj_num;
		cout << ",\tpos : " << ftell( fp ) << endl;
#endif
	}

	// 終了文字
	g_cell_pltt_no[ cell_num ] = BG_PLANM_PLTT_DATA_END;

#ifdef DEBUG
	cout << block_type << endl;
	cout << "size : " << block_size << endl;
	cout << "cell_num : " << cell_num << endl;
#endif

}

// -- アニメブロックからセルインデックス、ウェイトを取得 --
static void block_anim_read( FILE* fp )
{
	char block_type[4];
	int block_size;
	int anim_num;
	int cell_num;

	// 識別符号を読み込む
	fread( block_type, 4, 1, fp );
	// 識別符号チェック
	block_check_type( block_type, "ANIM" );
	// ブロックサイズを取得
	fread( &block_size, 4, 1, fp );
	// セルアニメの数を取得
	fread( &anim_num, 4, 1, fp );

	// @@@ 現状、1種類のアニメパターンしか読み込めない
	if( anim_num > 1 )
	{
		cout << "!!!!! anime data error !!!!!" << endl;
		cout << "現状、このコンバータでは複数のアニメパターンを処理できません。" << endl;
		cout << "１つのNCEファイルにつき、1種類のアニメを登録してください。" << endl;
		exit( EXIT_FAILURE );
	}

	fseek( fp, 8, SEEK_CUR );

	// セルアニメを構成するセルの数を取得
	fread( &cell_num, 4, 1, fp );
	for( int i=0; i<cell_num; i++ )
	{
		fread( &g_cell_idx[i], 2, 1, fp );
		fread( &g_cell_wait[i], 2, 1, fp );
		g_cell_wait[i] /= 2; // バトルが 30FPS なので2で割る
		fseek( fp, 20, SEEK_CUR );
	}

	// 終了文字
	g_cell_idx[ cell_num ] = CELL_IDX_END;
	g_cell_wait[ cell_num ] = BG_PLANM_CELL_WAIT_END;

#ifdef DEBUG
	cout << "cell_num : " << cell_num << endl;
#endif 

#ifdef DEBUG
	cout << "anim_num : " << anim_num << endl;
#endif 
}


// -- メイン処理 --
static void convert_data( string str )
{
	FILE *fp;

	// コンソールにファイル名を出力
	cout << "start convert " << str << endl;

	// ファイルオープン
	fp = fopen( str.c_str(), "rb" );
	if( fp == NULL )
	{
		cout << "file open error" << endl;
		exit( EXIT_FAILURE );
	}

	// ヘッダを飛ばす
	fseek( fp, 16, SEEK_SET );

	// セルデータからパレットデータを取得
	block_cell_read( fp );
	// キャラクタ番号データ ブロックをスキップ
	block_skip( fp, "CNUM" );
	// 1Dマッピング用キャラクタデータ ブロックをスキップ
	block_skip( fp, "CHAR" );
	// グループ番号データ ブロックをスキップ
	block_skip( fp, "GRP " );
	// アニメデータからセルインデックス、ウェイトフレームを取得
	block_anim_read( fp );

	// 閉じる
	fclose( fp );

	// 情報書き出し
	write_data( str );
}


//-----------------------------------------------------------------------------
/**
 *	@brief	メイン関数
 *
 *	@param	int argc	コマンドライン引数：個数
 *	@param	*argv[]		コマンドライン引数：内容
 *
 *	@return
 */
//-----------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
#ifdef DEBUG
	cout << "ファイル数=" << argc-1 << endl;
#endif

	for( int i=1; i<argc; i++ )
	{
		convert_data( argv[i] );
	}

	return 0;
}

