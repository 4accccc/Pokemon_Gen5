//---------------------------------------------------------------------------

//#include <vcl.h>
//#pragma hdrstop

//---------------------------------------------------------------------------

//#pragma argsused

/*
	itemsym.h		アイテムID
	itemequip.h		装備効果インデックス
	item000.dat		アイテムデータ


#define	ITEM_KIZUGUSURI		( 8 )		// ID

アーカイブID	( NARC_アーカイブファイル名_ファイル名 )
データ　：　NARC_DAT_ITEM_000_DATA
キャラ　：　NARC_CGX_ITEM_000_NCGR
パレット：　NARC_PAL_ITEM_000_NCLR




  { DATA_ARC_ID, CGX_ARC_ID, PAL_ARC_ID, AGB_NUM_ID },

*/


#include "common.h"
#include "itemdata.h"

#define	CNV_NAME	( "POKEMON W/B" )
#define	CNV_VER		( "ver.2.00" )
#define	XLS_VER		( "ver.0.00" )

#define	READ_DATA_SIZE	( 0x10000 )


extern u8 DataMake( s8 * dat, ITEMDATA * item, u16 num, u16 make );
extern void EquipIndexStrInit(void);
extern void FileMake_EquipIndex(void);
extern void FileMake_ItemData( ITEMDATA * dat, u16 num );
extern void FileMake_ItemIndex( void * buf, u16 num, u8 type );
extern void FileMake_ItemSym( s8 * str, s8 * buf, u16 num, u32 hb );
extern void FileMake_WazaMachineSym( s8 * str, s8 * buf, u16 num );
extern void ItemTypeTableSave(void);
extern void PocketNameTableSave(void);
extern void FILEMAKE_ItemTypeDef(void);
extern void FILEMAKE_BattlePocket( u8 * dat, u32 max );

static u8 DataRead( FILE * fp, s8 * dat );
static u8 AgbSymDataMake(void);
static u8 PrgSymDataMake(void);


static s8 ReadDataArea[ READ_DATA_SIZE ];
static ITEMDATA	ItemData;

s8	AgbSym[512][256];		// AGBのID
static u16 agbsymmax;		// AGBのID数

typedef struct {
	s8	str[256];
	u32	id;
}PRG_SYM;


PRG_SYM	PrgSym[1024];		// プログラム定義


s8	TypeIndex[1024][128];		// アイテム種類インデックス
u8	TypeMakeFlg[1024];			// アイテム種類作成フラグ
u8	TypeNum;					// アイテム種類フラグ

u8	BattlePocket[1024];			// 戦闘保存ポケット


//--------------------------------------------------------------------------------------------
/**
 * メイン処理
 *
 * @param	argc	
 * @param	argv
 *
 * @retrun	none
 */
//--------------------------------------------------------------------------------------------
void main( int argc, char * argv[] )
{
	FILE * fp;
	u16	num, make;

	printf( "%s ItemDataConv %s -item list %s-\n", CNV_NAME, CNV_VER, XLS_VER );

	// 引数チェック
	if( argc != 2 ){
		printf( "ERROR : param size\n\n" );
		return;
	}

	// AGB定義チェックデータ
	if( AgbSymDataMake() == FALSE ){
		return;
	}

	// プログラム定義データ
	if( PrgSymDataMake() == FALSE ){
		return;
	}

	EquipIndexStrInit();	// 装備効果クリア

	fp = fopen( argv[1], "r" );
	// ファイルオープンチェック
	if( fp == NULL ){
		printf( "ERROR : file open\n\n" );
		return;
	}

	memset( TypeIndex, '\0', 1024*128 );		// アイテム種類インデックス
	memset( TypeMakeFlg, 0, 1024 );					// アイテム種類作成フラグ
	memset( BattlePocket, 0, 1024 );				// 戦闘保存ポケット

	// 一行読み込み（いらない）
	DataRead( fp, ReadDataArea );

	num = 1;
	make = 1;
	while(1){
		u8	end;
		u8	ret;

		memset( ReadDataArea, 0, READ_DATA_SIZE );	// 読み込みエリアクリア
		memset( &ItemData, 0, sizeof(ITEMDATA) );	// 作成エリアクリア

		// 一行読み込み
		end = DataRead( fp, ReadDataArea );
		if( end == 2 ){
			printf( "complete : %d/%d data make\n\n", make-1, num-1 );
			break;
		}

		// データ作成
		ret = DataMake( ReadDataArea, &ItemData, num, make );
		if( ret == 0 ){
			FileMake_ItemData( &ItemData, num );	// ファイル作成
			make++;
		}else if( ret == 1 ){
			FileMake_ItemData( &ItemData, num );	// ファイル作成（ダミー）
			make++;
		}else{
			break;
		}


		if( end == 1 ){
			printf( "complete : %d/%d data make\n\n", make-1, num-1 );
			break;
		}
		if( feof( fp ) != 0 ){
			printf( "complete : %d/%d data make\n\n", make-1, num-1 );
			break;
		}

		num++;
	}

	fclose( fp );

	memset( &ItemData, 0, sizeof(ITEMDATA) );
	FileMake_ItemData( &ItemData, 0 );

	FileMake_ItemIndex( NULL, 0xffff, 0xff );
	FileMake_ItemSym( NULL, NULL, 0xffff, 0 );
	FileMake_WazaMachineSym( NULL, NULL, 0xffff );

	FileMake_EquipIndex();

	ItemTypeTableSave();

	PocketNameTableSave();

	FILEMAKE_ItemTypeDef();

	FILEMAKE_BattlePocket( BattlePocket, make );
}



//--------------------------------------------------------------------------------------------
/**
 * 一行読み込み
 *
 * @param	fp		ファイルポインタ
 * @param	dat		読み込み場所
 *
 * @retval	"0 = 通常"
 * @retval	"1 = 最後"
 * @retval	"2 = データなし"
 */
//--------------------------------------------------------------------------------------------
static u8 DataRead( FILE * fp, s8 * dat )
{
	u32	i;

	for( i=0; i<READ_DATA_SIZE; i++ ){
		fread( &dat[i], 1, 1, fp );
		if( dat[i] == '\n' ){
			break;
		}
		if( feof( fp ) != 0 ){
			if( dat[0] == '\0' || dat[0] == '\n' || dat[0] == '\t' ||
				dat[0] == ',' || dat[0] == ' ' ){

				return 2;
			}
			dat[i+1] = '\n';
			return 1;
		}
	}
	if( dat[0] == '\0' || dat[0] == '\n' || dat[0] == '\t' ||
		dat[0] == ',' || dat[0] == ' ' ){

		return 2;
	}

	return 0;
}




void TextFileHeaderPrint( FILE * fp, s8 * str )
{
	fprintf( fp, "//============================================================================================\n" );
	fprintf( fp, "/**\n" );
	fprintf( fp, " * %s\n", str );
	fprintf( fp, " * 2009 Game Freak Inc.\n" );
	fprintf( fp, " * item data cnv %s\n", CNV_VER );
	fprintf( fp, " */\n" );
	fprintf( fp, "//============================================================================================\n" );
}



static u8 AgbSymDataMake(void)
{
	FILE * fp;
	u16	i;
	s8	str[256];
	s8	flg;

	fp = fopen( "agb_itemsym.h", "r" );
	if( fp == NULL ){
		printf( "ERROR : File Open [agb_itemsym.h]\n" );
		return FALSE;
	}

	agbsymmax = 0;
	flg = 0;
	while(1){
		switch( DataRead( fp, str ) ){
		case 1:
			flg = 1;
		case 0:
			if( str[0] == '/' ){ break; }
			for( i=0; i<256; i++ ){
				AgbSym[agbsymmax][i] = str[17+i];
				if( AgbSym[agbsymmax][i] == '\t' || AgbSym[agbsymmax][i] == ' ' ){
					AgbSym[agbsymmax][i] = '\0';
					break;
				}
			}
			agbsymmax++;
			break;

		case 2:
			flg = 1;
			break;
		}
		if( flg == 1 ){ break; }
	}
	fclose( fp );

	return TRUE;
}

u8 AgbSymCheck( s8 * str )
{
	u16	i;

	for( i=0; i<agbsymmax; i++ ){
		if( strcmp( AgbSym[i], str ) == 0 ){
			return TRUE;
		}
	}
	return FALSE;
}





static u8 PrgSymDataMake(void)
{
	FILE * fp;
	u16	i, j, k;
	s8	str[256];
	s8	num[16];
//	s8	flg;

	fp = fopen( "itemuse_def.h", "r" );
	if( fp == NULL ){
		printf( "ERROR : File Open [itemuse_def.h]\n" );
		return FALSE;
	}

	memset( PrgSym, 0, sizeof(PRG_SYM) );

//	flg = 0;
	j = 0;
	while(1){
		memset( str, '\0', 256 );
		switch( DataRead( fp, str ) ){
		case 1:
//			flg = 1;
		case 0:
			if( str[0] == '/' ){ break; }
			for( i=0; i<256; i++ ){
				PrgSym[j].str[i] = str[16+i];
				if( PrgSym[j].str[i] == '\t' || PrgSym[j].str[i] == ' ' ){
					PrgSym[j].str[i] = '\0';
					break;
				}
			}
			for( i=0; i<256; i++ ){
				PrgSym[j].str[i] = str[16+i];
				if( PrgSym[j].str[i] == '\t' || PrgSym[j].str[i] == ' ' ){
					PrgSym[j].str[i] = '\0';
					break;
				}
			}
			k = 0;
			for( i=i; i<256; i++ ){
				if( str[16+i] == '0' || str[16+i] == '1' || str[16+i] == '2' ||
					str[16+i] == '3' || str[16+i] == '4' || str[16+i] == '5' ||
					str[16+i] == '6' || str[16+i] == '7' || str[16+i] == '8' || str[16+i] == '9' ){

					num[k] = str[16+i];
					k++;
				}else if( str[16+i] == ')' ){
					num[k] = '\0';
					PrgSym[j].id = atoi( num );
					break;
				}
			}
			printf( "%s - %d\n", PrgSym[j].str, PrgSym[j].id );
			j++;
			break;

		case 2:
//			flg = 1;
			break;
		}
		if( feof( fp ) != 0 ){ break; }

//		if( flg == 1 ){ break; }
	}
	fclose( fp );

	return TRUE;
}

u8 PrgSymCheck( s8 * str, u8 flg )
{
	s8	sym[256];
	u16	i;

	if( flg == 0 ){
		strcpy( sym, "FLD_" );
	}else{
		strcpy( sym, "BTL_" );
	}
	strcat( sym, str );

	for( i=0; i<1024; i++ ){
		if( strcmp( PrgSym[i].str, sym ) == 0 ){
			return (u8)PrgSym[i].id;
		}
	}
	return 0;
}
 