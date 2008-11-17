//=============================================================================================
/**
 * @file	personal.c
 * @brief	ポケモンWB  ポケモンパーソナルデータアクセッサ（DOS用仮作成）
 * @author	taya
 *
 * @date	2008.10.20	作成
 */
//=============================================================================================

#include "ds_types.h"

#include "poketype.h"
#include "personal.h"


typedef struct {
	u8		basic_hp;			//基本ＨＰ
	u8		basic_pow;			//基本攻撃力
	u8		basic_def;			//基本防御力
	u8		basic_agi;			//基本素早さ

	u8		basic_spepow;		//基本特殊攻撃力
	u8		basic_spedef;		//基本特殊防御力
	u8		type1;				//属性１
	u8		type2;				//属性２

	u8		get_rate;			//捕獲率
	u8		give_exp;			//贈与経験値

	u16		pains_hp	:2;		//贈与努力値ＨＰ
	u16		pains_pow	:2;		//贈与努力値攻撃力
	u16		pains_def	:2;		//贈与努力値防御力
	u16		pains_agi	:2;		//贈与努力値素早さ
	u16		pains_spepow:2;		//贈与努力値特殊攻撃力
	u16		pains_spedef:2;		//贈与努力値特殊防御力
	u16					:4;		//贈与努力値予備

	u16		item1;				//アイテム１
	u16		item2;				//アイテム２

	u8		sex;				//性別ベクトル
	u8		egg_birth;			//タマゴの孵化歩数
	u8		friend;				//なつき度初期値
	u8		grow;				//成長曲線識別

	u8		egg_group1;			//こづくりグループ1
	u8		egg_group2;			//こづくりグループ2
	u8		speabi1;			//特殊能力１
	u8		speabi2;			//特殊能力２

	u8		escape;				//逃げる率
	u8		color	:7;			//色（図鑑で使用）
	u8		reverse	:1;			//反転フラグ
	u32		machine1;			//技マシンフラグ１
	u32		machine2;			//技マシンフラグ２
	u32		machine3;			//技マシンフラグ２
	u32		machine4;			//技マシンフラグ２

}pokemon_personal_data;


u32 PERSONAL_GetValue( u16 monsno, PersonalValueID  pvid )
{
	static pokemon_personal_data  readBuf;

	char filename[ 64 ];
	FILE* fp;

	sprintf( filename, ".\\personal_bin\\per_%03d.bin", monsno );
	fp = fopen( filename, "rb" );

	if( fp )
	{
		fread( &readBuf, sizeof(readBuf), 1, fp );
		fclose( fp );

		switch( pvid ){
		case PERSONAL_VALUE_TYPE1:				return readBuf.type1;
		case PERSONAL_VALUE_TYPE2:				return readBuf.type2;
		case PERSONAL_VALUE_TOKUSEI1:			return readBuf.speabi1;
		case PERSONAL_VALUE_TOKUSEI2:			return readBuf.speabi2;
		case PERSONAL_VALUE_BASE_HP:			return readBuf.basic_hp;
		case PERSONAL_VALUE_BASE_ATTACK:		return readBuf.basic_pow;
		case PERSONAL_VALUE_BASE_DEFENCE:		return readBuf.basic_def;
		case PERSONAL_VALUE_BASE_AGILITY:		return readBuf.basic_agi;
		case PERSONAL_VALUE_BASE_SP_ATTACK:		return readBuf.basic_spepow;
		case PERSONAL_VALUE_BASE_SP_DEFENCE:	return readBuf.basic_spedef;
		default:
			GF_ASSERT(0);
			break;
		}
	}
	else
	{
		BTL_Printf("PERSONAL IREAGAL MONSNO : %d (%s)\n", monsno, filename);
		GF_ASSERT(0);
	}

	return 1;
}


PokeTypePair  PERSONAL_GetPokeTypePair( u16 monsno )
{
	PokeType  type1, type2;

	type1 = PERSONAL_GetValue( monsno, PERSONAL_VALUE_TYPE1 );
	type2 = PERSONAL_GetValue( monsno, PERSONAL_VALUE_TYPE2 );

	return PokeTypePair_Make( type1, type2 );

}



