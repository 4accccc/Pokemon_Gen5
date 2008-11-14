//=============================================================================================
/**
 * @file	poke_personal.h
 * @brief	ポケモンパーソナルデータアクセス関数群
 * @author	taya
 *
 * @date	2008.11.06	作成
 */
//=============================================================================================
#ifndef __POKE_PERSONAL_H__
#define __POKE_PERSONAL_H__

//--------------------------------------------------------------
/**
 *	ポケモンパーソナルデータ構造体ハンドラ型の宣言
 */
//--------------------------------------------------------------
typedef struct pokemon_personal_data	POKEMON_PERSONAL_DATA;

//--------------------------------------------------------------
/**
 *	ポケモンパーソナルデータ取得のためのインデックス
 */
//--------------------------------------------------------------
typedef enum {

	POKE_PER_ID_basic_hp=0,			//基本ＨＰ
	POKE_PER_ID_basic_pow,			//基本攻撃力
	POKE_PER_ID_basic_def,			//基本防御力
	POKE_PER_ID_basic_agi,			//基本素早さ
	POKE_PER_ID_basic_spepow,		//基本特殊攻撃力
	POKE_PER_ID_basic_spedef,		//基本特殊防御力
	POKE_PER_ID_type1,				//属性１
	POKE_PER_ID_type2,				//属性２
	POKE_PER_ID_get_rate,			//捕獲率
	POKE_PER_ID_give_exp,			//贈与経験値
	POKE_PER_ID_pains_hp,			//贈与努力値ＨＰ
	POKE_PER_ID_pains_pow,			//贈与努力値攻撃力
	POKE_PER_ID_pains_def,			//贈与努力値防御力
	POKE_PER_ID_pains_agi,			//贈与努力値素早さ
	POKE_PER_ID_pains_spepow,		//贈与努力値特殊攻撃力
	POKE_PER_ID_pains_spedef,		//贈与努力値特殊防御力
	POKE_PER_ID_item1,				//アイテム１
	POKE_PER_ID_item2,				//アイテム２
	POKE_PER_ID_sex,					//性別ベクトル
	POKE_PER_ID_egg_birth,			//タマゴの孵化歩数
	POKE_PER_ID_friend,				//なつき度初期値
	POKE_PER_ID_grow,				//成長曲線識別
	POKE_PER_ID_egg_group1,			//こづくりグループ1
	POKE_PER_ID_egg_group2,			//こづくりグループ2
	POKE_PER_ID_speabi1,				//特殊能力１
	POKE_PER_ID_speabi2,				//特殊能力２
	POKE_PER_ID_escape,				//逃げる率
	POKE_PER_ID_color,				//色（図鑑で使用）
	POKE_PER_ID_reverse,				//反転フラグ
	POKE_PER_ID_machine1,			//技マシンフラグ１
	POKE_PER_ID_machine2,			//技マシンフラグ２
	POKE_PER_ID_machine3,			//技マシンフラグ３
	POKE_PER_ID_machine4				//技マシンフラグ４

}PokePersonalParamID;



//ポケモンパーソナル操作関数系
extern POKEMON_PERSONAL_DATA*  POKE_PERSONAL_OpenHandle( u16 mons_no, u16 form_no, HEAPID heapID );
extern void POKE_PERSONAL_CloseHandle( POKEMON_PERSONAL_DATA* handle );
extern u32 POKE_PERSONAL_GetParam( POKEMON_PERSONAL_DATA *ppd, PokePersonalParamID paramID );


extern	u8		PokePersonal_SexGet( POKEMON_PERSONAL_DATA* personalData, u16 monsno, u32 rnd );



#endif
