//============================================================================================
/**
 * @file field_goout_data.h
 * @brief field_goout_dataを使用するための定義情報です
 * @author Toru=Nagihashi
 * @note field_goout_effect_converter.rbから自動生成されています。
*/
//============================================================================================
#pragma once
//演出タイプ
#define GOOUT_EFF_DAYLIGHT_SPRING  (0)
#define GOOUT_EFF_DAYLIGHT_SUMMER  (1)
#define GOOUT_EFF_DAYLIGHT_AUTUMN  (2)
#define GOOUT_EFF_DAYLIGHT_WINTER  (3)
#define GOOUT_EFF_SUNSET_ALL  (4)
#define GOOUT_EFF_MOONLIGHT_ALL  (5)
#define GOOUT_EFF_MAX  (6)
#define GOOUT_EFF_NULL  (GOOUT_EFF_MAX)
//演出組み合わせ
#define GOOUT_COMBI_NONE  (0)
#define GOOUT_COMBI_DAYLIGHT_SPRING_ONLY  (1)
#define GOOUT_COMBI_DAYLIGHT_SUMMER_ONLY  (2)
#define GOOUT_COMBI_DAYLIGHT_AUTUMN_ONLY  (3)
#define GOOUT_COMBI_DAYLIGHT_WINTER_ONLY  (4)
#define GOOUT_COMBI_SUNSET_ALL_ONLY  (5)
#define GOOUT_COMBI_MOONLIGHT_ALL_ONLY  (6)
#define GOOUT_COMBI_TEST_CONBI  (7)
#define GOOUT_COMBI_MAX  (8)
#define GOOUT_COMBI_EFFECT_MAX  (4)
//定数
#define FIELD_GOOUT_ROOM_COUNT_MAX  (180)
