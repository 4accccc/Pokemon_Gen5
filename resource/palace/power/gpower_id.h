//============================================================
//       power_conv.rb で出力されたファイルです
//============================================================

#pragma once

typedef enum{
	GPOWER_ID_SOUGUU_INC_A,	//0 : エンカウント率が1.5倍上昇する。
	GPOWER_ID_SOUGUU_INC_B,	//1 : エンカウント率が2倍上昇する。
	GPOWER_ID_SOUGUU_INC_C,	//2 : エンカウント率が3倍上昇する。
	GPOWER_ID_SOUGUU_DEC_A,	//3 : エンカウント率が0.66倍になる。
	GPOWER_ID_SOUGUU_DEC_B,	//4 : エンカウント率が0.5倍になる。
	GPOWER_ID_SOUGUU_DEC_C,	//5 : エンカウント率が0.33倍になる。
	GPOWER_ID_EGG_INC_A,	//6 : タマゴの孵化が1.25倍になる。（具体的な数値は言わない）
	GPOWER_ID_EGG_INC_B,	//7 : タマゴの孵化が1.5倍になる。（具体的な数値は言わない）
	GPOWER_ID_EGG_INC_C,	//8 : タマゴの孵化が2倍になる。（具体的な数値は言わない）
	GPOWER_ID_NATSUKI_A,	//9 : ポケモンのなつき増加が、常に＋１される。（具体的な数値は言わない）
	GPOWER_ID_NATSUKI_B,	//10 : ポケモンのなつき増加が、常に＋２される。（具体的な数値は言わない）
	GPOWER_ID_NATSUKI_C,	//11 : ポケモンのなつき増加が、常に＋３される。（具体的な数値は言わない）
	GPOWER_ID_SALE_A,	//12 : 買い物の値段が１０％オフになる
	GPOWER_ID_SALE_B,	//13 : 買い物の値段が２５％オフになる
	GPOWER_ID_SALE_C,	//14 : 買い物の値段が５０％オフになる
	GPOWER_ID_HP_RESTORE_A,	//15 : てもち先頭のポケモンのHPが少し回復する（きずぐすり）
	GPOWER_ID_HP_RESTORE_B,	//16 : てもち先頭のポケモンのHPが回復する（いいきずぐすり）
	GPOWER_ID_HP_RESTORE_C,	//17 : てもち先頭のポケモンのHPが全て回復する（すごいきずぐすり）
	GPOWER_ID_PP_RESTORE_A,	//18 : てもち先頭のポケモンの４つのPPが５回復する
	GPOWER_ID_PP_RESTORE_B,	//19 : てもち先頭のポケモンの４つのPPが１０回復する
	GPOWER_ID_PP_RESTORE_C,	//20 : てもち先頭のポケモンの４つのPPが全部回復する
	GPOWER_ID_EXP_INC_A,	//21 : バトル後の経験値が1.2倍になる。（具体的な数値は言わない）
	GPOWER_ID_EXP_INC_B,	//22 : バトル後の経験値が1.5倍になる。（具体的な数値は言わない）
	GPOWER_ID_EXP_INC_C,	//23 : バトル後の経験値が２倍になる。（具体的な数値は言わない）
	GPOWER_ID_EXP_DEC_A,	//24 : バトル後の経験値が0.8倍になる。（具体的な数値は言わない）
	GPOWER_ID_EXP_DEC_B,	//25 : バトル後の経験値が0.66倍になる。（具体的な数値は言わない）
	GPOWER_ID_EXP_DEC_C,	//26 : バトル後の経験値が0.5倍になる。（具体的な数値は言わない）
	GPOWER_ID_MONEY_A,	//27 : バトル後のおこづかいが1.5倍になる。（具体的な数値は言わない）
	GPOWER_ID_MONEY_B,	//28 : バトル後のおこづかいが２倍になる。（具体的な数値は言わない）
	GPOWER_ID_MONEY_C,	//29 : バトル後のおこづかいが３倍になる。（具体的な数値は言わない）
	GPOWER_ID_CAPTURE_A,	//30 : ポケモンの捕獲率が×１.１倍される。（具体的な数値は言わない）
	GPOWER_ID_CAPTURE_B,	//31 : ポケモンの捕獲率が×１.２倍される。（具体的な数値は言わない）
	GPOWER_ID_CAPTURE_C,	//32 : ポケモンの捕獲率が×１.３倍される。（具体的な数値は言わない）
	GPOWER_ID_EGG_INC_S,	//33 : タマゴの孵化が2倍になる。（配布：パレス用に効果時間１０倍）
	GPOWER_ID_SALE_S,	//34 : 買い物の値段が５０％オフになる（配布：パレス用に効果時間１０倍）
	GPOWER_ID_NATSUKI_S,	//35 : ポケモンのなつき増加が、常に＋３される。（配布：パレス用に効果時間１０倍）
	GPOWER_ID_EXP_INC_S,	//36 : バトル後の経験値が2倍になる。（具体的な数値は言わない）（配布：パレス用に効果時間１０倍））
	GPOWER_ID_MONEY_S,	//37 : バトル後のおこづかいが３倍になる。（具体的な数値は言わない）（配布：パレス用に効果時間１０倍）
	GPOWER_ID_CAPTURE_S,	//38 : ポケモンの捕獲率が×１．３倍される。（具体的な数値は言わない）（配布：パレス用に効果時間１０倍）
	GPOWER_ID_HP_RESTORE_MAX,	//39 : てもち全ポケモンのHPが完全回復する。ポケセンと同じ（パレス用）
	GPOWER_ID_EGG_INC_MAX,	//40 : タマゴの孵化が2倍になる。（パレス用に効果時間１時間）
	GPOWER_ID_SALE_MAX,	//41 : 買い物の値段が５０％オフになる（パレス用に効果時間１時間）
	GPOWER_ID_NATSUKI_MAX,	//42 : ポケモンのなつき増加が、常に＋３される。（パレス用に効果時間１時間）
	GPOWER_ID_EXP_INC_MAX,	//43 : バトル後の経験値が2倍になる。（具体的な数値は言わない）（パレス用に効果時間１時間）
	GPOWER_ID_MONEY_MAX,	//44 : バトル後のおこづかいが３倍になる。（具体的な数値は言わない）（パレス用に効果時間１時間）
	GPOWER_ID_CAPTURE_MAX,	//45 : ポケモンの捕獲率が×１．３倍される。（具体的な数値は言わない）（パレス用に効果時間１時間）
	GPOWER_ID_HAPPEN_MAX,	//46 : ダミーデータ
	GPOWER_ID_DISTRIBUTION_MAX,	//47 : ダミーデータ

	GPOWER_ID_DISTRIBUTION_START = 33,		//配布Gパワー開始ID(このIDを含む)
	GPOWER_ID_DISTRIBUTION_END = 38,		//配布Gパワー終了ID(このIDを含む)

	GPOWER_ID_PALACE_START = 39,		//パレスGパワー開始ID(このIDを含む)
	GPOWER_ID_PALACE_END = 47,		//パレスGパワー終了ID(このIDを含む)

	GPOWER_ID_MAX = 48,		//Gパワー最大数
	GPOWER_ID_NULL = GPOWER_ID_MAX,		//Gパワーが発動していない
}GPOWER_ID;



//効果系統　　!!出力元データによってenumの並びが変わる場合があります!!
typedef enum{
	GPOWER_TYPE_ENCOUNT_UP,		//0
	GPOWER_TYPE_ENCOUNT_DOWN,		//1
	GPOWER_TYPE_HATCH_UP,		//2
	GPOWER_TYPE_NATSUKI_UP,		//3
	GPOWER_TYPE_SALE,		//4
	GPOWER_TYPE_HP_RESTORE,		//5
	GPOWER_TYPE_PP_RESTORE,		//6
	GPOWER_TYPE_EXP_UP,		//7
	GPOWER_TYPE_EXP_DOWN,		//8
	GPOWER_TYPE_MONEY_UP,		//9
	GPOWER_TYPE_CAPTURE_UP,		//10
	GPOWER_TYPE_HAPPEN,		//11
	GPOWER_TYPE_DISTRIBUTION,		//12

	GPOWER_TYPE_MAX,
	GPOWER_TYPE_NULL = GPOWER_TYPE_MAX,
}GPOWER_TYPE;

