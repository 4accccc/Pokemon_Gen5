#include "common.h"
#include "itemdata.h"
#include "msgdata.h"


//============================================================================================
//	定数定義
//============================================================================================
#define	STR_BUFLEN		( 1024 )

#define	POCKET_MAX		( 5 )			// ポケット数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
extern void PrintErrorMsg( u16 item, u16 err );
extern void FileMake_Name( s8 * str, u16 num );
extern void FileMake_ItemSym( s8 * str, s8 * buf, u16 num, u32 hb );
extern void FileMake_WazaMachineSym( s8 * str, s8 * buf, u16 num );
extern void FileMake_Info( s8 * str, u16 num, u8 type );
extern u16 PM_StrGet( s8 * str );
extern void FileMake_ItemIndex( void * buf, u16 num, u8 type );
extern u8 PrgSymCheck( s8 * str, u8 flg );
extern void ItemTypeTableMake( s8 * str );

static u8 ErrorCheck( s8 * dat, u16 item, u16 err );
static s8 * DataGet( s8 * dat, s8 * buf );
static s8 * ItemTypeDataMake( s8 * dat, ITEMDATA * item );				// 分類
static s8 * ItemNameDataMake( s8 * dat, ITEMDATA * item, u16 num );
static s8 * MakeItemSym( s8 * dat, u16 num );
static s8 * WazaMachineDataMake( s8 * dat, ITEMDATA * item, u16 num );
static s8 * ItemInfoDataMake( s8 * dat, ITEMDATA * item );
static s8 * ItemIndexMake( s8 * dat, ITEMDATA * item, u16 num );
static s8 * PriceDataMake( s8 * dat, ITEMDATA * item );			// 買値	
static s8 * EquipDataMake( s8 * dat, ITEMDATA * item );			// 装備効果
static s8 * PowerDataMake( s8 * dat, ITEMDATA * item );			// 威力	
static s8 * ImportantDataMake( s8 * dat, ITEMDATA * item );		// 重要	
static s8 * ConvButtonDataMake( s8 * dat, ITEMDATA * item );	// 便利ボタン
static s8 * SortNumDataMake( s8 * dat, ITEMDATA * item );			// ソート番号
static s8 * PocketNumDataMake( s8 * dat, ITEMDATA * item );		// 保存先
static s8 * FieldFuncDataMake( s8 * dat, ITEMDATA * item );		// field機能
static s8 * BattleFuncDataMake( s8 * dat, ITEMDATA * item );	// battle機能
static s8 * SpendDataMake( s8 * dat, ITEMDATA * item );
static s8 * WorkTypeDataMake( s8 * dat, ITEMDATA * item );		// ワークタイプ
static s8 * BattlePocketDataMake( s8 * dat, ITEMDATA * item );
static s8 * NagePowerDataMake( s8 * dat, ITEMDATA * item );
static s8 * SizenPowerDataMake( s8 * dat, ITEMDATA * item );
static s8 * SizenTypeDataMake( s8 * dat, ITEMDATA * item );
static s8 * NageDataMake( s8 * dat, ITEMDATA * item );
static s8 * TuibamuDataMake( s8 * dat, ITEMDATA * item );

static s8 * WorkTypeNoneMake( s8 * dat, ITEMDATA * item, u16 num );
static s8 * WorkTypeRecoverMake( s8 * dat, ITEMDATA * item, u16 num );

static u8 EquipIndexMake( s8 * str );
static u8 NageIndexMake( s8 * str );
static u8 TuibamuIndexMake( s8 * str );

static void BattlePocketDataSet( ITEMDATA * item, u32 pos );


//============================================================================================
//	グローバル変数
//============================================================================================
extern u8 TypeNum;							// アイテム種類
extern u8	BattlePocket[1024];		// 戦闘保存ポケット


// 保存先参照データ（ポケット番号）
static const s8 * PocketName[] = {
	"NORMAL",		// 道具
	"DRUG",			// 薬
//	"BALL",			// ボール
	"WAZA",			// 技マシン
	"NUTS",			// 木の実
//	"MAIL",			// メール
//	"BATTLE",		// 戦闘用
	"EVENT",		// 大切な物
};

// ワークタイプ参照データ
static const s8 * WorkType[] = {
	"NONE",			// 通常
	"RECOVER",		// 回復
};

// タイプ参照データ
static const s8 * TypeName[] = {
	"ノーマル",
	"かくとう",
	"ひこう",
	"どく",
	"じめん",
	"いわ",
	"むし",
	"ゴースト",
	"はがね",
	"ほのお",
	"みず",
	"くさ",
	"でんき",
	"エスパー",
	"こおり",
	"ドラゴン",
	"あく",
};


s8	EquipIndex[256][128];		// 装備効果
s8	NageIndex[256][128];		// なげつける効果
s8	TuibamuIndex[256][128];		// ついばむ効果

s8	StrSymbol[512];				// ID文字列
s8	WazaSymbol[512];				// ID文字列

u16	ItemDataMax;
u16	WazaMachineDataMax;


//--------------------------------------------------------------------------------------------
/**
 * 半角文字チェック
 *
 * @param	str		文字列
 *
 * @retval	"正常 = TRUE"
 * @retval	"異常 = FALSE"
 */
//--------------------------------------------------------------------------------------------
static u8 CheckOneData( s8 * str )
{
	u16	i;

	for( i=0; i<STR_BUFLEN; i++ ){
		if( str[i] == '\0' ){ break; }
		if( _ismbblead( str[i] ) != 0 ) { return FALSE; }
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 全角文字チェック
 *
 * @param	str		文字列
 *
 * @retval	"正常 = TRUE"
 * @retval	"異常 = FALSE"
 */
//--------------------------------------------------------------------------------------------
static u8 CheckTwoData( s8 * str )
{
	u16	i;

	for( i=0; i<STR_BUFLEN; i++ ){
		if( str[i] == '\0' ){ break; }
		if( _ismbblead( str[i] )==0 && _ismbbtrail( str[i] )==0 ){
			return	FALSE;
		}
	}
	return TRUE;
}





u8 DataMake( s8 * dat, ITEMDATA * item, u16 num, u16 make )
{
	dat = DataGet( dat, NULL );								// Ｎｏ
	dat = ItemTypeDataMake( dat, item );			// 分類

	dat = ItemNameDataMake( dat, item, num );	// アイテム名
	if( dat == NULL ){ return 2; }
	if( dat == (s8 *)0xffffffff ){
		FileMake_ItemIndex( NULL, num, 0xfe );
		return 1;
	}
	dat = DataGet( dat, NULL );					// アイテム名(漢字)

	dat = ItemIndexMake( dat, item, num );
//	dat = DataGet( dat, NULL );					// アイコンキャラ(*.sch)
//	dat = DataGet( dat, NULL );					// アイコンパレット(*.scl)

	dat = PriceDataMake( dat, item );			// 買値
	if( ErrorCheck( dat, num, 0 ) == FALSE ){ return 2; }

	dat = EquipDataMake( dat, item );			// 装備効果
	if( ErrorCheck( dat, num, 1 ) == FALSE ){ return 2; }

	dat = PowerDataMake( dat, item );			// 威力
	if( ErrorCheck( dat, num, 2 ) == FALSE ){ return 2; }

	dat = DataGet( dat, NULL );					// 装備説明

	dat = NagePowerDataMake( dat, item );		// なげつける威力
	if( ErrorCheck( dat, num, 42 ) == FALSE ){ return 2; }

	dat = NageDataMake( dat, item );			// なげつける効果
	if( ErrorCheck( dat, num, 45 ) == FALSE ){ return 2; }

	dat = SizenPowerDataMake( dat, item );		// しぜんのめぐみ威力
	if( ErrorCheck( dat, num, 43 ) == FALSE ){ return 2; }

	dat = SizenTypeDataMake( dat, item );		// しぜんのめぐみタイプ
	if( ErrorCheck( dat, num, 44 ) == FALSE ){ return 2; }

	dat = TuibamuDataMake( dat, item );			// ついばむ効果
	if( ErrorCheck( dat, num, 46 ) == FALSE ){ return 2; }

	dat = ItemInfoDataMake( dat, item );		// 説明１～３
	dat = ItemInfoDataMake( dat, item );		// 戦闘説明１～３

	dat = MakeItemSym( dat, num );					// 複合
	if( ErrorCheck( dat, num, 51 ) == FALSE ){ return 2; }
	ItemTypeTableMake( StrSymbol );

	// 技マシン収録技
	dat = WazaMachineDataMake( dat, item, num );
	if( ErrorCheck( dat, num, 48 ) == FALSE ){ return 2; }

	dat = ImportantDataMake( dat, item );		// 重要	
	if( ErrorCheck( dat, num, 3 ) == FALSE ){ return 2; }

	dat = ConvButtonDataMake( dat, item );		// 便利ボタン
	if( ErrorCheck( dat, num, 4 ) == FALSE ){ return 2; }

	dat = SortNumDataMake( dat, item );		// ソート番号
	if( ErrorCheck( dat, num, 49 ) == FALSE ){ return 2; }

	dat = PocketNumDataMake( dat, item );		// 保存先
	if( ErrorCheck( dat, num, 5 ) == FALSE ){ return 2; }

	dat = BattlePocketDataMake( dat, item );	// 戦闘保存先（ボール、戦闘用、HP、状態異常、PP）
	if( ErrorCheck( dat, num, 41 ) == FALSE ){ return 2; }
	BattlePocketDataSet( item, make );

	dat = FieldFuncDataMake( dat, item );		// field機能
	if( ErrorCheck( dat, num, 6 ) == FALSE ){ return 2; }

	dat = BattleFuncDataMake( dat, item );		// battle機能
	if( ErrorCheck( dat, num, 7 ) == FALSE ){ return 2; }

	dat = SpendDataMake( dat, item );					// 消費するか（戦闘）
	if( ErrorCheck( dat, num, 47 ) == FALSE ){ return 2; }

	dat = WorkTypeDataMake( dat, item );		// ワークタイプ
	if( ErrorCheck( dat, num, 8 ) == FALSE ){ return 2; }

	// ワーク
	switch( item->work_type ){
	case 0:		// NONE
		dat = WorkTypeNoneMake( dat, item, num );
		if( dat == NULL ){ return 2; }
		break;
	case 1:		// RECOVER
		dat = WorkTypeNoneMake( dat, NULL, num );
		dat = WorkTypeRecoverMake( dat, item, num );
		if( dat == NULL ){ return 2; }
		break;
	}

	return 0;
}


static s8 * DataGet( s8 * dat, s8 * buf )
{
	u16	i, j;

	j = 0;
	for( i=0; i<STR_BUFLEN; i++ ){
		if( dat[i] == ',' || dat[i] == '\n' ){
			if( buf != NULL ){ buf[j] = '\0'; }
			break;
		}
		if( dat[i] != '\n' && dat[i] != '\t' ){
			if( buf != NULL ){
				buf[j] = dat[i];
				j++;
			}
		}
	}

	return &dat[i+1];
}


static u8 ErrorCheck( s8 * dat, u16 item, u16 err )
{
	if( dat == NULL ){
		PrintErrorMsg( item, err );
		return FALSE;
	}
	return TRUE;
}


//--------------------------------------------------------------------------------------------
// 分類チェック
//			カテゴリーの並びは「装備／ノーマル／戦闘／ボール／メール」の順となります。									
//--------------------------------------------------------------------------------------------
static s8 * ItemTypeDataMake( s8 * dat, ITEMDATA * item )
{
/*	D/P/PL/HG/SS
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	TypeNum = 0;
	if( CheckTwoData( str ) == FALSE ){ return NULL; }

	if( strcmp( str, "メール" ) == 0 ){
		TypeNum = 1;
	}else if( strcmp( str, "きのみ" ) == 0 ){
		TypeNum = 2;
	}

	return dat;
*/
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	TypeNum = 0;
	if( CheckTwoData( str ) == FALSE ){ return NULL; }

	if( strcmp( str, "装備" ) == 0 ){
		item->type = 1;
	}else if( strcmp( str, "ノーマル" ) == 0 ){
		item->type = 2;
	}else if( strcmp( str, "戦闘" ) == 0 ){
		item->type = 3;
	}else if( strcmp( str, "ボール" ) == 0 ){
		item->type = 4;
	}else if( strcmp( str, "メール" ) == 0 ){
		item->type = 5;
		TypeNum = 1;
	}else if( strcmp( str, "きのみ" ) == 0 ){
		item->type = 0;
		TypeNum = 2;
	}else{
		item->type = 0;
	}

	return dat;
}

//--------------------------------------------------------------------------------------------
// アイテム名
//--------------------------------------------------------------------------------------------
static s8 itemName[STR_BUFLEN];

static s8 * ItemNameDataMake( s8 * dat, ITEMDATA * item, u16 num )
{
//	s8	str[STR_BUFLEN];
//	u8	i;

	dat = DataGet( dat, itemName );		// アイテム名（ひらがな）

	if( CheckTwoData( itemName ) == FALSE ){ return NULL; }

	if( strcmp( itemName, "？？？" ) == 0 ){
		return (s8 *)0xffffffff;
	}

//	FileMake_Name( str, num );
/*
	for( i=0; i<ITEMCONV_NAME_SIZE; i++ ){
		if( str[i*2] == '\0' ){
			item->name[i] = EOM_;
			break;
		}
		item->name[i] = PM_StrGet( &str[i*2] );
	}
*/

//	FileMake_ItemSym( str, StrSymbol, num, atoi(hb) );
	ItemDataMax = num;

	return dat;
}

static s8 * MakeItemSym( s8 * dat, u16 num )
{
	s8	hb[STR_BUFLEN];

	dat = DataGet( dat, hb );			// 複合アイテム

	if( CheckOneData( hb ) == FALSE ){ return NULL; }

	FileMake_ItemSym( itemName, StrSymbol, num, atoi(hb) );

	return dat;
}


//--------------------------------------------------------------------------------------------
// 技マシン収録技
//--------------------------------------------------------------------------------------------
static s8 * WazaMachineDataMake( s8 * dat, ITEMDATA * item, u16 num )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckTwoData( str ) == FALSE ){ return NULL; }

	if( strcmp( str, "ー" ) != 0 ){
		FileMake_WazaMachineSym( str, WazaSymbol, WazaMachineDataMax );
		WazaMachineDataMax++;
	}

	return dat;
}

//--------------------------------------------------------------------------------------------
// 説明
//--------------------------------------------------------------------------------------------
static s8 * ItemInfoDataMake( s8 * dat, ITEMDATA * item )
{
	u8	i;

	for( i=0; i<3; i++ ){
		dat = DataGet( dat, NULL );
	}
	return dat;

/*
	s8	str[256];
	u8	i, j, p;

	p = 0;
	for( i=0; i<3; i++ ){
		dat = DataGet( dat, str );

		if( CheckTwoData( str ) == FALSE ){ return NULL; }

//		FileMake_Info( str, num, type );
		for( j=0; j<ITEMCONV_INFO_LINE_SIZE; j++ ){
			if( str[j*2] == '\0' ){
				if( i == 2 ){
					item->info[p+j] = EOM_;
				}else{
					item->info[p+j] = CR_;
				}
				break;
			}
			item->info[p+j] = PM_StrGet( &str[j*2] );
		}
		p += (j+1);
	}

	return dat;
*/
}

//--------------------------------------------------------------------------------------------
// データインデックス作成
//--------------------------------------------------------------------------------------------
static s8 * ItemIndexMake( s8 * dat, ITEMDATA * item, u16 num )
{
	s8	str[STR_BUFLEN];

	// データ
//	FileMake_ItemIndex( NULL, num, 0 );

	// アイコンキャラ
	dat = DataGet( dat, str );
	FileMake_ItemIndex( (void *)str, num, 1 );

	// アイコンパレット
	dat = DataGet( dat, str );
	FileMake_ItemIndex( (void *)str, num, 2 );

	// 旧シンボル
//	FileMake_ItemIndex( (void *)StrSymbol, num, 3 );

	return dat;
}

//--------------------------------------------------------------------------------------------
// 買値	
//--------------------------------------------------------------------------------------------
static s8 * PriceDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->price = (u16)( atoi(str) / 10 );	// 1/10にする 2010/05/24

	return dat;
}


//--------------------------------------------------------------------------------------------
// 装備効果
//--------------------------------------------------------------------------------------------
static s8 * EquipDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	if( strcmp( str, "0" ) == 0 ){
		item->eqp = 0;
	}else{
		item->eqp = EquipIndexMake( str );
	}

	return dat;
}

static u8 EquipIndexMake( s8 * str )
{
	u16	i;

	for( i=0; i<256; i++ ){
		if( EquipIndex[i][0] == '\0' ){
			strcpy( EquipIndex[i], str );
			break;
		}

		if( strcmp( str, EquipIndex[i] ) == 0 ){
			break;
		}
	}
	return (u8)i+1;
}

void EquipIndexStrInit(void)
{
	memset( EquipIndex, '\0', 256*128 );
	memset( NageIndex, '\0', 256*128 );
	memset( TuibamuIndex, '\0', 256*128 );
}


//--------------------------------------------------------------------------------------------
// なげつける効果
//--------------------------------------------------------------------------------------------
static s8 * NageDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	if( strcmp( str, "0" ) == 0 ){
		item->nage_eff = 0;
	}else{
		item->nage_eff = NageIndexMake( str );
	}

	return dat;
}

static u8 NageIndexMake( s8 * str )
{
	u16	i;

	for( i=0; i<256; i++ ){
		if( NageIndex[i][0] == '\0' ){
			strcpy( NageIndex[i], str );
			break;
		}

		if( strcmp( str, NageIndex[i] ) == 0 ){
			break;
		}
	}
	return (u8)i+1;
}

//--------------------------------------------------------------------------------------------
// ついばむ効果
//--------------------------------------------------------------------------------------------
static s8 * TuibamuDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	if( strcmp( str, "0" ) == 0 ){
		item->tuibamu_eff = 0;
	}else{
		item->tuibamu_eff = TuibamuIndexMake( str );
	}

	return dat;
}

static u8 TuibamuIndexMake( s8 * str )
{
	u16	i;

	for( i=0; i<256; i++ ){
		if( TuibamuIndex[i][0] == '\0' ){
			strcpy( TuibamuIndex[i], str );
			break;
		}

		if( strcmp( str, TuibamuIndex[i] ) == 0 ){
			break;
		}
	}
	return (u8)i+1;
}


//--------------------------------------------------------------------------------------------
// 威力	
//--------------------------------------------------------------------------------------------
static s8 * PowerDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->atc = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// なげつける威力
//--------------------------------------------------------------------------------------------
static s8 * NagePowerDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->nage_atc = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// しぜんのめぐみ威力
//--------------------------------------------------------------------------------------------
static s8 * SizenPowerDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->sizen_atc = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// しぜんのめぐみタイプ
//--------------------------------------------------------------------------------------------
static s8 * SizenTypeDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];
	u8	i;

	dat = DataGet( dat, str );

	if( CheckTwoData( str ) == FALSE ){ return NULL; }

	for( i=0; i<NELEMS(TypeName); i++ ){
		if( strcmp( str, TypeName[i] ) == 0 ){
			item->sizen_type = i;
			return dat;
		}
	}
	item->sizen_type = 0x1f;

	return dat;
}

//--------------------------------------------------------------------------------------------
// 重要	
//--------------------------------------------------------------------------------------------
static s8 * ImportantDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){
		return NULL;
	}

	item->imp = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// 便利ボタン
//--------------------------------------------------------------------------------------------
static s8 * ConvButtonDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->cnv_btn = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// ソート番号
//--------------------------------------------------------------------------------------------
static s8 * SortNumDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->sort = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// 保存先
//--------------------------------------------------------------------------------------------
static s8 * PocketNumDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];
	u8	i;

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	for( i=0; i<NELEMS(PocketName); i++ ){
		if( strcmp( str, PocketName[i] ) == 0 ){
			item->fld_pocket = i;
			break;
		}
	}

	return dat;
}

//--------------------------------------------------------------------------------------------
// 戦闘ポケット
//--------------------------------------------------------------------------------------------
static s8 * BattlePocketDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];
	u8	i;

	for( i=0; i<5; i++ ){
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){ return NULL; }
		item->btl_pocket |= ((u8)atoi(str)<<i);
	}
	return dat;
}

static void BattlePocketDataSet( ITEMDATA * item, u32 pos )
{
	BattlePocket[pos] = item->btl_pocket;
}


//--------------------------------------------------------------------------------------------
// field機能
//--------------------------------------------------------------------------------------------
static s8 * FieldFuncDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

//	item->field_func = (u8)atoi( str );
	item->field_func = PrgSymCheck( str, 0 );

	return dat;
}

//--------------------------------------------------------------------------------------------
// battle機能
//--------------------------------------------------------------------------------------------
static s8 * BattleFuncDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

//	item->battle_func = (u8)atoi( str );
	item->battle_func = PrgSymCheck( str, 1 );

	return dat;
}

//--------------------------------------------------------------------------------------------
// 消費するか
//--------------------------------------------------------------------------------------------
static s8 * SpendDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	item->spend = (u8)atoi( str );

	return dat;
}

//--------------------------------------------------------------------------------------------
// ワークタイプ
//--------------------------------------------------------------------------------------------
static s8 * WorkTypeDataMake( s8 * dat, ITEMDATA * item )
{
	s8	str[STR_BUFLEN];
	u8	i;

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){ return NULL; }

	for( i=0; i<NELEMS(WorkType); i++ ){
		if( strcmp( str, WorkType[i] ) == 0 ){
			item->work_type = i;
			break;
		}
	}

	return dat;
}


//--------------------------------------------------------------------------------------------
// ワーク：NONE
//--------------------------------------------------------------------------------------------
static s8 * WorkTypeNoneMake( s8 * dat, ITEMDATA * item, u16 num )
{
	s8	str[STR_BUFLEN];

	dat = DataGet( dat, str );

	if( CheckOneData( str ) == FALSE ){
		PrintErrorMsg( num, 9 );
		return NULL;
	}

	if( item != NULL ){
		item->work[0] = (u8)atoi( str );
	}

	return dat;
}

//--------------------------------------------------------------------------------------------
// ワーク：RECOVER
//--------------------------------------------------------------------------------------------
static s8 * WorkTypeRecoverMake( s8 * dat, ITEMDATA * item, u16 num )
{
	if( item != NULL ){
		ITEMPARAM_RCV * wk = (ITEMPARAM_RCV *)item->work;
		s8	str[STR_BUFLEN];

		// 能力ガード
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 10 );
			return NULL;
		}
		wk->ability_guard = (u8)atoi( str );

		// クリティカル
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 11 );
			return NULL;
		}
		wk->critical_up = (u8)atoi( str );

		// 攻撃力
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 12 );
			return NULL;
		}
		wk->atc_up = (u8)atoi( str );

		// 防御力	
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 13 );
			return NULL;
		}
		wk->def_up = (u8)atoi( str );

		// 素早さ
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 14 );
			return NULL;
		}
		wk->agi_up = (u8)atoi( str );

		// 命中率	
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 15 );
			return NULL;
		}
		wk->hit_up = (u8)atoi( str );

		// 特殊攻撃
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 16 );
			return NULL;
		}
		wk->spa_up = (u8)atoi( str );

		// 特殊防御
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 17 );
			return NULL;
		}
		wk->spd_up = (u8)atoi( str );

		// レベルup
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 18 );
			return NULL;
		}
		wk->lv_up = (u8)atoi( str );

		// pp回復flg
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 19 );
			return NULL;
		}
		if( atoi( str ) > 2 ){
			PrintErrorMsg( num, 19 );
			return NULL;
		}
		if( atoi( str ) != 0 ){
			if( atoi( str ) == 1 ){
				wk->pp_rcv = 1;
			}else if( atoi( str ) == 2 ){
				wk->allpp_rcv = 1;
			}
		}

		// pp回復値
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 20 );
			return NULL;
		}
		wk->prm_pp_rcv = (u8)atoi( str );

		// pp値up
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 21 );
			return NULL;
		}
		if( atoi( str ) > 2 ){
			PrintErrorMsg( num, 21 );
			return NULL;
		}
		if( atoi( str ) != 0 ){
			if( atoi( str ) == 1 ){
				wk->pp_up = 1;
			}else if( atoi( str ) == 2 ){
				wk->pp_3up = 1;
			}
		}

		// 努力値制御フラグ
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 50 );
			return NULL;
		}
		wk->exp_limit = (u8)atoi( str );
		if( wk->exp_limit != 0 ){ wk->exp_limit = 1; }
		
		// HP努力値
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 22 );
			return NULL;
		}
		wk->prm_hp_exp = (s8)atoi( str );
		if( wk->prm_hp_exp != 0 ){ wk->hp_exp = 1; }

		// 攻撃努力値
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 23 );
			return NULL;
		}
		wk->prm_pow_exp = (s8)atoi( str );
		if( wk->prm_pow_exp != 0 ){ wk->pow_exp = 1; }

		// 防御努力値
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 24 );
			return NULL;
		}
		wk->prm_def_exp = (s8)atoi( str );
		if( wk->prm_def_exp != 0 ){ wk->def_exp = 1; }

		// 素早努力値	
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 25 );
			return NULL;
		}
		wk->prm_agi_exp = (s8)atoi( str );
		if( wk->prm_agi_exp != 0 ){ wk->agi_exp = 1; }

		// 特攻努力値
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 26 );
			return NULL;
		}
		wk->prm_spa_exp = (s8)atoi( str );
		if( wk->prm_spa_exp != 0 ){ wk->spa_exp = 1; }

		// 特防努力値
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 27 );
			return NULL;
		}
		wk->prm_spd_exp = (s8)atoi( str );
		if( wk->prm_spd_exp != 0 ){ wk->spd_exp = 1; }

		// なつき度1
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 28 );
			return NULL;
		}
		wk->prm_friend1 = (s8)atoi( str );
		if( wk->prm_friend1 != 0 ){ wk->friend_exp1 = 1; }

		// なつき度2
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 29 );
			return NULL;
		}
		wk->prm_friend2 = (s8)atoi( str );
		if( wk->prm_friend2 != 0 ){ wk->friend_exp2 = 1; }

		// なつき度3
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 30 );
			return NULL;
		}
		wk->prm_friend3 = (s8)atoi( str );
		if( wk->prm_friend3 != 0 ){ wk->friend_exp3 = 1; }

		// 眠り回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 31 );
			return NULL;
		}
		wk->sleep_rcv = (s8)atoi( str );

		// 毒回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 32 );
			return NULL;
		}
		wk->poison_rcv = (s8)atoi( str );

		// やけど回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 33 );
			return NULL;
		}
		wk->burn_rcv = (s8)atoi( str );

		// こおり回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 34 );
			return NULL;
		}
		wk->ice_rcv = (s8)atoi( str );

		// まひ回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 35 );
			return NULL;
		}
		wk->paralyze_rcv = (s8)atoi( str );

		// 混乱回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 36 );
			return NULL;
		}
		wk->panic_rcv = (s8)atoi( str );

		// メロメロ回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 37 );
			return NULL;
		}
		wk->meromero_rcv = (s8)atoi( str );

		// ＨＰ回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 38 );
			return NULL;
		}
		wk->prm_hp_rcv = (u8)atoi( str );
		if( wk->prm_hp_rcv != 0 ){ wk->hp_rcv = 1; }

		// 瀕死回復
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 39 );
			return NULL;
		}
		if( atoi( str ) > 2 ){
			PrintErrorMsg( num, 39 );
			return NULL;
		}
		if( atoi( str ) != 0 ){
			wk->death_rcv = 1;
			if( atoi( str ) == 1 ){
				wk->hp_rcv = 0;					// 通常の瀕死回復はHP回復フラグを下げる
			}
			if( atoi( str ) == 2 ){
				wk->alldeath_rcv = 1;
			}
		}

		// 進化
		dat = DataGet( dat, str );
		if( CheckOneData( str ) == FALSE ){
			PrintErrorMsg( num, 40 );
			return NULL;
		}
		wk->evolution = (s8)atoi( str );

	}else{
		u8	i;

		for( i=0; i<31; i++ ){
			dat = DataGet( dat, NULL );
		}
	}

	return dat;
}

// ポケット名取得
const s8 * GetPocketNameStr( u32 num )
{
	if( num >= POCKET_MAX ){ return NULL; }

	return PocketName[num];
};
