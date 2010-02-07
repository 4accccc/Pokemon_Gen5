//============================================================
//       power_conv.rb で出力されたファイルです
//============================================================

#pragma once

typedef enum{
	GPOWER_ID_MAX = 83,		//Gパワー最大数
	GPOWER_ID_NULL = GPOWER_ID_MAX,		//Gパワーが発動していない
}GPOWER_ID;



//効果系統　　!!出力元データによってenumの並びが変わる場合があります!!
typedef enum{
	GPOWER_TYPE_ENCOUNT_UP,		//0
	GPOWER_TYPE_ENCOUNT_DOWN,		//1
	GPOWER_TYPE_HATCH_UP,		//2
	GPOWER_TYPE_NATSUKI_UP,		//3
	GPOWER_TYPE_SALE,		//4
	GPOWER_TYPE_HAPPEN,		//5
	GPOWER_TYPE_HP_RESTORE,		//6
	GPOWER_TYPE_PP_RESTORE,		//7
	GPOWER_TYPE_EXP_UP,		//8
	GPOWER_TYPE_EXP_DOWN,		//9
	GPOWER_TYPE_MONEY_UP,		//10
	GPOWER_TYPE_CAPTURE_UP,		//11
	GPOWER_TYPE_DISTRIBUTION,		//12

	GPOWER_TYPE_MAX,
	GPOWER_TYPE_NULL = GPOWER_TYPE_MAX,
}GPOWER_TYPE;

