//============================================================================
/**
 *@file		tr_ai_weather.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
// 	最初のターンに天気技をする
//========================================================

WeatherAISeq:
	IF_MIKATA_ATTACK	WeatherAISeq_End					// 対象が味方なら終了2006.6.20

	CHECK_TURN
	IFN_EQUAL	0,WeatherAISeq_End						//最初のターン
	
	IF_WAZA_SEQNO_JUMP	137,WeatherAI_Nihonbare		// にほんばれ
	IF_WAZA_SEQNO_JUMP	136,WeatherAI_Amagoi		// あまごい
	IF_WAZA_SEQNO_JUMP	115,WeatherAI_Sunaarashi	// すなあらし
	IF_WAZA_SEQNO_JUMP	164,WeatherAI_Arare			// あられ

WeatherAI_Nihonbare:// にほんばれ	
	CHECK_WEATHER
	IF_EQUAL	WEATHER_HARE,WeatherAISeq_End
	JUMP	WeatherAI_Check_Ok

WeatherAI_Amagoi:// あまごい	
	CHECK_WEATHER
	IF_EQUAL	WEATHER_AME,WeatherAISeq_End
	JUMP	WeatherAI_Check_Ok

WeatherAI_Sunaarashi:// すなあらし	
	CHECK_WEATHER
	IF_EQUAL	WEATHER_SUNAARASHI,WeatherAISeq_End
	JUMP	WeatherAI_Check_Ok

WeatherAI_Arare:// あられ	
	CHECK_WEATHER
	IF_EQUAL	WEATHER_ARARE,WeatherAISeq_End
	JUMP	WeatherAI_Check_Ok


WeatherAI_Check_Ok:	
	CHECK_NEKODAMASI	CHECK_ATTACK
	IF_EQUAL	0,WeatherAISeq_End
	INCDEC		5

WeatherAISeq_End:	
	AIEND

