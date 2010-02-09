//=============================================================================================
/**
 * @file  poke_personal.h
 * @brief ポケモンパーソナルデータアクセス関数群
 * @author  taya
 *
 * @date  2008.11.06  作成
 */
//=============================================================================================
#ifndef __POKE_PERSONAL_H__
#define __POKE_PERSONAL_H__

//--------------------------------------------------------------
/**
 *  ポケモンパーソナルデータ構造体ハンドラ型の宣言
 */
//--------------------------------------------------------------
typedef struct pokemon_personal_data  POKEMON_PERSONAL_DATA;

//--------------------------------------------------------------
/**
 *  ポケモンパーソナルデータ取得のためのインデックス
 */
//--------------------------------------------------------------
typedef enum {

  POKEPER_ID_basic_hp=0,      //基本ＨＰ
  POKEPER_ID_basic_pow,       //基本攻撃力
  POKEPER_ID_basic_def,       //基本防御力
  POKEPER_ID_basic_agi,       //基本素早さ
  POKEPER_ID_basic_spepow,    //基本特殊攻撃力
  POKEPER_ID_basic_spedef,    //基本特殊防御力
  POKEPER_ID_type1,           //属性１
  POKEPER_ID_type2,           //属性２
  POKEPER_ID_get_rate,        //捕獲率
  POKEPER_ID_give_exp,        //贈与経験値
  POKEPER_ID_pains_hp,        //贈与努力値ＨＰ
  POKEPER_ID_pains_pow,       //贈与努力値攻撃力
  POKEPER_ID_pains_def,       //贈与努力値防御力
  POKEPER_ID_pains_agi,       //贈与努力値素早さ
  POKEPER_ID_pains_spepow,    //贈与努力値特殊攻撃力
  POKEPER_ID_pains_spedef,    //贈与努力値特殊防御力
  POKEPER_ID_item1,           //アイテム１
  POKEPER_ID_item2,           //アイテム２
  POKEPER_ID_item3,           //アイテム３
  POKEPER_ID_sex,             //性別ベクトル
  POKEPER_ID_egg_birth,       //タマゴの孵化歩数
  POKEPER_ID_friend,          //なつき度初期値
  POKEPER_ID_grow,            //成長曲線識別
  POKEPER_ID_egg_group1,      //こづくりグループ1
  POKEPER_ID_egg_group2,      //こづくりグループ2
  POKEPER_ID_speabi1,         //特殊能力１
  POKEPER_ID_speabi2,         //特殊能力２
  POKEPER_ID_speabi3,         //特殊能力３
  POKEPER_ID_escape,          //逃げる率
  POKEPER_ID_form_index,      //別フォルムパーソナルデータ開始位置
  POKEPER_ID_form_gra_index,  //別フォルムグラフィックデータ開始位置
  POKEPER_ID_form_max,        //別フォルムMAX
  POKEPER_ID_color,           //色（図鑑で使用）
  POKEPER_ID_reverse,         //反転フラグ
  POKEPER_ID_pltt_only,       //別フォルム時パレットのみ変化
  POKEPER_ID_rank,            //ポケモンランク
  POKEPER_ID_height,          //高さ
  POKEPER_ID_weight,          //重さ

  POKEPER_ID_machine1,        //技マシンフラグ１
  POKEPER_ID_machine2,        //技マシンフラグ２
  POKEPER_ID_machine3,        //技マシンフラグ３
  POKEPER_ID_machine4,        //技マシンフラグ４

  POKEPER_ID_waza_oshie1,     //技教え１

}PokePersonalParamID;


//パーソナルデータの色定義
typedef enum {
  POKEPER_COLOR_RED = 0,    //赤
  POKEPER_COLOR_BLUE,     //青
  POKEPER_COLOR_YELLOW,   //黄
  POKEPER_COLOR_GREEN,    //緑
  POKEPER_COLOR_BLACK,    //黒
  POKEPER_COLOR_BROWN,    //茶
  POKEPER_COLOR_PERPLE,   //紫
  POKEPER_COLOR_GRAY,     //灰
  POKEPER_COLOR_WHITE,    //白
  POKEPER_COLOR_PINK,     //桃
}PokePersonal_Color;


//--------------------------------------------------------------
/**
 * ポケモンパーソナルデータの関連定数
 */
//--------------------------------------------------------------
typedef enum {
  //  性別ベクトル（パーソナル上のオスメス比率）(1～253でオスメスの比率が変わる）
  POKEPER_SEX_MALE = 0,     ///<オスのみ
  POKEPER_SEX_FEMALE = 254,   ///<メスのみ
  POKEPER_SEX_UNKNOWN = 255,    ///<性別なし
}POKEPER_SEX;

typedef enum{
  //性別ベクトルのタイプ(オスメスランダムor固定)
  POKEPER_SEXTYPE_RND,
  POKEPER_SEXTYPE_FIX,
}POKEPER_SEX_TYPE;

enum{
  // こづくりグループ
POKEPER_EGG_GROUP_KAIJUU,     //１：怪獣
POKEPER_EGG_GROUP_SUISEI,     //２：水棲
POKEPER_EGG_GROUP_MUSI,       //３：虫
POKEPER_EGG_GROUP_TORI,       // ４：鳥
POKEPER_EGG_GROUP_DOUBUTU,    //５：動物
POKEPER_EGG_GROUP_YOUSEI,     // ６：妖精
POKEPER_EGG_GROUP_SYOKUBUTU,  // ７：植物
POKEPER_EGG_GROUP_HITOGATA,   // ８：人型
POKEPER_EGG_GROUP_KOUKAKU,    // ９：甲殻
POKEPER_EGG_GROUP_MUKIBUTU,   // 10：無機物
POKEPER_EGG_GROUP_FUTEIKEI,   // 11：不定形
POKEPER_EGG_GROUP_SAKANA,     // 12：魚
POKEPER_EGG_GROUP_METAMON,    // 13：メタモン
POKEPER_EGG_GROUP_DORAGON,    // 14：ドラゴン
POKEPER_EGG_GROUP_MUSEISYOKU, //15：無生殖
};
//--------------------------------------------------------------
/**
 * ポケモンワザ覚えデータ関連
 */
//--------------------------------------------------------------

///< ワザ覚えデータコード
typedef struct
{ 
  u16 waza_no;    //覚える技ナンバー
  u16 level;      //覚えるレベル
}POKEPER_WAZAOBOE_CODE;

enum {
  POKEPER_WAZAOBOE_TABLE_ELEMS = 26,  ///< ワザおぼえテーブルのロードに必要な要素数
  POKEPER_WAZAOBOE_ENDCODE = 0xffff,
};

// ワザ覚えデータコード：終端チェック
static inline BOOL POKEPER_WAZAOBOE_IsEndCode( POKEPER_WAZAOBOE_CODE code )
{
  return ( ( code.waza_no == POKEPER_WAZAOBOE_ENDCODE ) && ( code.level == POKEPER_WAZAOBOE_ENDCODE ) );
}
// ワザ覚えデータコード：レベル取得
static inline u16 POKEPER_WAZAOBOE_GetLevel( POKEPER_WAZAOBOE_CODE code )
{
  return code.level;
}
// ワザ覚えデータコード：ワザID取得
static inline u16 POKEPER_WAZAOBOE_GetWazaID( POKEPER_WAZAOBOE_CODE code )
{
  return code.waza_no;
}




//ポケモンパーソナル操作関数系
extern POKEMON_PERSONAL_DATA*  POKE_PERSONAL_OpenHandle( u16 mons_no, u16 form_no, HEAPID heapID );
extern void POKE_PERSONAL_CloseHandle( POKEMON_PERSONAL_DATA* handle );
extern u32 POKE_PERSONAL_GetParam( POKEMON_PERSONAL_DATA *ppd, PokePersonalParamID paramID );

// ワザおぼえテーブルロード（dst には POKEPER_WAZAOBOE_CODE * POKEPER_WAZAOBOETABLE_ELEMS の配列を渡すこと）
extern void POKE_PERSONAL_LoadWazaOboeTable( u16 mons_no, u16 form_no, POKEPER_WAZAOBOE_CODE* dst );

// 性別取得
extern  u8    PokePersonal_SexGet( POKEMON_PERSONAL_DATA* personalData, u16 monsno, u32 rnd );
extern POKEPER_SEX_TYPE PokePersonal_SexVecTypeGet( POKEPER_SEX sex_vec );

#endif
