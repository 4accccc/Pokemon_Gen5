#include "common.h"
#include "itemdata.h"
#include "msgdata.h"


extern u16 PM_StrGet( s8 * str );
extern const s8 * SYM_StrGet( s8 * str );
extern const s8 * PM_SymChange( u16 buf );
extern void MakeNewSymbol( s8 * str, s8 * buf );
extern void TextFileHeaderPrint( FILE * fp, s8 * str );
extern u8 AgbSymCheck( s8 * str );
extern const s8 * GetPocketNameStr( u32 num );

extern s8	EquipIndex[256][128];		// 装備効果
extern s8	NageIndex[256][128];		// なげつける効果
extern s8	TuibamuIndex[256][128];		// ついばむ効果
extern u16	ItemDataMax;
extern u16	WazaMachineDataMax;

extern s8	TypeIndex[1024][128];	// アイテム種類インデックス
extern u8	TypeMakeFlg[1024];		// アイテム種類作成フラグ
extern u8	TypeNum;				// アイテム種類フラグ


// 装備効果
void FileMake_EquipIndex(void)
{
	FILE * fp;
	u16	i;

	fp = fopen( "itemequip.h", "w" );
	if( fp == NULL ){ return; }

	TextFileHeaderPrint( fp, "itemequip.h" );
	fprintf( fp, "#ifndef ITEMEQUIP_H\n" );
	fprintf( fp, "#define ITEMEQUIP_H\n\n" );

	fprintf( fp, "// 装備効果\n" );
	for( i=0; i<256; i++ ){
		if( EquipIndex[i][0] == '\0' ){ break; }
		fprintf( fp, "#define\t%s\t\t( %d )\n", EquipIndex[i], i+1 );
	}
	fprintf( fp, "\n" );

	fprintf( fp, "// 技「なげつける」効果\n" );
	for( i=0; i<256; i++ ){
		if( NageIndex[i][0] == '\0' ){ break; }
		fprintf( fp, "#define\t%s\t\t( %d )\n", NageIndex[i], i+1 );
	}
	fprintf( fp, "\n" );

	fprintf( fp, "// 技「ついばむ」効果\n" );
	for( i=0; i<256; i++ ){
		if( TuibamuIndex[i][0] == '\0' ){ break; }
		fprintf( fp, "#define\t%s\t\t( %d )\n", TuibamuIndex[i], i+1 );
	}
	fprintf( fp, "\n" );

/*
	fprintf( fp, "// 装備効果\n" );
	fprintf( fp, "enum {\n" );
	for( i=0; i<256; i++ ){
		if( EquipIndex[i][0] == '\0' ){ break; }
		if( i == 0 ){
			fprintf( fp, "\t%s = 1,\n", EquipIndex[i] );
		}else{
			fprintf( fp, "\t%s,\n", EquipIndex[i] );
		}
	}
	fprintf( fp, "};\n\n" );

	fprintf( fp, "// 技「なげつける」効果\n" );
	fprintf( fp, "enum {\n" );
	for( i=0; i<256; i++ ){
		if( NageIndex[i][0] == '\0' ){ break; }
		if( i == 0 ){
			fprintf( fp, "\t%s = 1,\n", NageIndex[i] );
		}else{
			fprintf( fp, "\t%s,\n", NageIndex[i] );
		}
	}
	fprintf( fp, "};\n\n" );

	fprintf( fp, "// 技「ついばむ」効果\n" );
	fprintf( fp, "enum {\n" );
	for( i=0; i<256; i++ ){
		if( TuibamuIndex[i][0] == '\0' ){ break; }
		if( i == 0 ){
			fprintf( fp, "\t%s = 1,\n", TuibamuIndex[i] );
		}else{
			fprintf( fp, "\t%s,\n", TuibamuIndex[i] );
		}
	}
	fprintf( fp, "};\n\n" );
*/

	fprintf( fp, "#endif // ITEMEQUIP_H\n" );

	fclose( fp );
}

// アイテムデータ
void FileMake_ItemData( ITEMDATA * dat, u16 num )
{
	FILE * fp;
	s8	name[512];
	s8	dir[256];

	GetCurrentDirectory( 256, dir );

	if( num >= 100 ){
		sprintf( name, "%s\\itemdata\\item%d.dat", dir, num );
		fp = fopen( name, "wb" );
	}else if( num >= 10 ){
		sprintf( name, "%s\\itemdata\\item0%d.dat", dir, num );
		fp = fopen( name, "wb" );
	}else{
		sprintf( name, "%s\\itemdata\\item00%d.dat", dir, num );
		fp = fopen( name, "wb" );
	}

	if( fp == NULL ){ return; }

	fwrite( dat, sizeof(ITEMDATA), 1, fp );

	fclose( fp );
}

// アイテムID
void FileMake_ItemSym( s8 * str, s8 * buf, u16 num, u32 hb )
{
	FILE * fp;

	fp = fopen( "itemsym.h", "a" );
	if( fp == NULL ){ return; }

	if( num == 0xffff ){
		fprintf( fp, "\n#define ITEM_DATA_MAX\t\t( %d )\t\t// アイテム最大数\n", ItemDataMax );
		{
			u16	i;
			u16	max;

			max = 0;
			for( i=0; i<512; i++ ){
				if( TypeMakeFlg[i] == 0 ){ break; }
				if( TypeMakeFlg[i] != 1 ){ continue; }
				max++;
			}
			fprintf( fp, "\n#define ITEM_MAIL_MAX\t( %d )\t// メール数\n", max );

			max = 0;
			for( i=0; i<512; i++ ){
				if( TypeMakeFlg[i] == 0 ){ break; }
				if( TypeMakeFlg[i] != 2 ){ continue; }
				max++;
			}
			fprintf( fp, "#define ITEM_NUTS_MAX\t( %d )\t// きのみ数\n", max );
		}
		fprintf( fp, "\n#endif // ITEMSYM_H\n" );
	}else{
		memset( buf, '\0', 512 );
		MakeNewSymbol( str, buf );

		if( num == 1 ){
			TextFileHeaderPrint( fp, "itemsym.h" );
			fprintf( fp, "#ifndef ITEMSYM_H\n" );
			fprintf( fp, "#define ITEMSYM_H\n\n" );
			fprintf( fp, "#define ITEM_DUMMY_DATA\t\t( 0 )\t\t// ダミーデータ\n" );
		}
		// 複合チェック
		if( hb == 0 ){
			fprintf( fp, "#define ITEM_%s\t\t( %d )\t\t// %s\n", buf, num, str );
		}else{
			fprintf( fp, "#define ITEM_%s_%d\t\t( %d )\t\t// %s\n", buf, hb, num, str );
		}
	}

	fclose( fp );
}

// 
void FileMake_ItemIndex( void * buf, u16 num, u8 type )
{
	FILE * fp;

	fp = fopen( "itemindex.dat", "a" );
	if( fp == NULL ){ return; }

	if( num == 1 && type == 1 ){
		TextFileHeaderPrint( fp, "itemindex.dat" );
		fprintf( fp, "const ITEMDATA_INDEX ItemDataIndex[] =\n{\n" );
//		fprintf( fp, "\t{ 0xffff, 0xffff, 0xffff, 0xffff },\n" );
/*
		fprintf( fp, "\t{ " );
		fprintf( fp, "NARC_item_data_item000_dat," );
		fprintf( fp, " NARC_item_icon_item_dumy_NCGR," );
		fprintf( fp, " NARC_item_icon_item_dumy_NCLR," );
		fprintf( fp, " 0 },\n" );
*/
		fprintf( fp, "\t{ NARC_item_icon_item_dumy_NCGR," );
		fprintf( fp, " NARC_item_icon_item_dumy_NCLR },\n" );
	}

	switch( type ){
	case 0:		// data
/*
		if( num >= 100 ){
			fprintf( fp, "\t{ NARC_item_data_item%d_dat,", num );
		}else if( num >= 10 ){
			fprintf( fp, "\t{ NARC_item_data_item0%d_dat,", num );
		}else{
			fprintf( fp, "\t{ NARC_item_data_item00%d_dat,", num );
		}
*/
		break;
	case 1:		// cgx
//		fprintf( fp, " NARC_item_icon_%s_NCGR,", (s8 *)buf );
		fprintf( fp, "\t{ NARC_item_icon_%s_NCGR,", (s8 *)buf );
		break;
	case 2:		// pal
//		fprintf( fp, " NARC_item_icon_%s_NCLR,", (s8 *)buf );
		fprintf( fp, " NARC_item_icon_%s_NCLR },\n", (s8 *)buf );
		break;
	case 3:		// agb_sym
/*
		if( AgbSymCheck( (s8*)buf ) == TRUE ){
			fprintf( fp, " AGB_ITEM_%s },\n", (s8 *)buf );
		}else{
			fprintf( fp, " 0 },\n" );
		}
*/
/*
		if( num > AGB_ITEM_MAX ){
			fprintf( fp, " 0 },\n" );
		}else{
			fprintf( fp, " AGB_ITEM_%s },\n", (s8 *)buf );
		}
*/
		break;
	case 0xfe:
/*
		fprintf( fp, "\t{ " );
		fprintf( fp, "NARC_item_data_item000_dat," );
		fprintf( fp, " NARC_item_icon_item_dumy_NCGR," );
		fprintf( fp, " NARC_item_icon_item_dumy_NCLR," );
		fprintf( fp, " 0 },\n" );
*/
		fprintf( fp, "\t{ NARC_item_icon_item_dumy_NCGR," );
		fprintf( fp, " NARC_item_icon_item_dumy_NCLR },\n" );
		break;
	default:
		fprintf( fp, "};\n" );
	}

	fclose( fp );
}


// アイテム種類テーブル作成
void ItemTypeTableMake( s8 * str )
{
	u32	i;

	if( TypeNum == 0 ){ return; }

	for( i=0; i<512; i++ ){
		if( TypeMakeFlg[i] != 0 ){ continue; }
		TypeMakeFlg[i] = TypeNum;
		sprintf( &TypeIndex[i][0], "ITEM_%s", str );
		break;
	}
}

void ItemTypeTableSave(void)
{
	FILE * fp;
	u32	i;

	fp = fopen( "itemtype.dat", "w" );

	TextFileHeaderPrint( fp, "itemtype.dat" );

	fprintf( fp, "// メール\n" );
	fprintf( fp, "const u16 ItemMailTable[] =\n{\n" );
	for( i=0; i<1024; i++ ){
		if( TypeMakeFlg[i] == 0 ){ break; }
		if( TypeMakeFlg[i] != 1 ){ continue; }
		fprintf( fp, "\t%s,\n", &TypeIndex[i][0] );
	}
	fprintf( fp, "};\n\n" );

	fprintf( fp, "// きのみ\n" );
	fprintf( fp, "const u16 ItemNutsTable[] =\n{\n" );
	for( i=0; i<1024; i++ ){
		if( TypeMakeFlg[i] == 0 ){ break; }
		if( TypeMakeFlg[i] != 2 ){ continue; }
		fprintf( fp, "\t%s,\n", &TypeIndex[i][0] );
	}
	fprintf( fp, "};\n\n" );

	fclose( fp );
}

void PocketNameTableSave(void)
{
	FILE * fp;
	const s8 * name;
	u32	i;

	fp = fopen( "itempocket_def.h", "w" );

	TextFileHeaderPrint( fp, "itempocket_def.h" );
	fprintf( fp, "#pragma once\n\n" );

	i = 0;
	while(1){
		name = GetPocketNameStr( i );
		if( name == NULL ){
			break;
		}
		fprintf( fp, "#define ITEMPOCKET_%s\t\t( %d )\n", name, i );
		i++;
	}

	fclose( fp );
}


void FILEMAKE_ItemTypeDef(void)
{
//			カテゴリーの並びは「装備／ノーマル／戦闘／ボール／メール」の順となります。
	FILE * fp;

	fp = fopen( "itemtype_def.h", "w" );

	TextFileHeaderPrint( fp, "itemtype_def.h" );
	fprintf( fp, "#pragma once\n\n" );

	fprintf( fp, "#define ITEMTYPE_NONE\t\t( 0 )\t\t\t\t// 分類なし\n" );
	fprintf( fp, "#define ITEMTYPE_EQUIP\t\t( 1 )\t\t\t\t// 装備\n" );
	fprintf( fp, "#define ITEMTYPE_NORMAL\t\t( 2 )\t\t\t\t// ノーマル\n" );
	fprintf( fp, "#define ITEMTYPE_BATTLE\t\t( 3 )\t\t\t\t// 戦闘\n" );
	fprintf( fp, "#define ITEMTYPE_BALL\t\t( 4 )\t\t\t\t// ボール\n" );
	fprintf( fp, "#define ITEMTYPE_MAIL\t\t( 5 )\t\t\t\t// メール\n" );

	fclose( fp );
}


// 技マシン収録技テーブル
void FileMake_WazaMachineSym( s8 * str, s8 * buf, u16 num )
{
	FILE * fp;

	fp = fopen( "waza_mcn.dat", "a" );
	if( fp == NULL ){ return; }

	if( num == 0xffff ){
		fprintf( fp, "};\n" );
	}else{
		memset( buf, '\0', 512 );
		MakeNewSymbol( str, buf );

		if( num == 0 ){
			TextFileHeaderPrint( fp, "waza_mcn.dat" );
			fprintf( fp, "static const u16 MachineNo[] = {\n" );
		}

		fprintf( fp, "\tWAZANO_%s,\t\t// %03d: %s\n", buf, num+1, str );
	}

	fclose( fp );
}

// 戦闘ポケット
void FILEMAKE_BattlePocket( u8 * dat, u32 max )
{
	FILE * fp;

	fp = fopen( "battle_pocket.dat", "wb" );
	if( fp == NULL ){ return; }

	fwrite( dat, sizeof(u8), max, fp );

	fclose( fp );
}

