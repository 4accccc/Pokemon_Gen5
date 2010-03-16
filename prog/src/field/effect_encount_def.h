/*
 *  @file   effect_encount_def.h
 *  @brief  エフェクトエンカウント　リテラル定義
 *  @author Miyuki Iwasawa
 *  @date   10.03.04
 */

#pragma once

#define EFFENC_NORMAL_INTERVAL (50)  //エフェクト抽選デフォルトインターバル
#define EFFENC_NORMAL_PROB (20)  //エフェクト抽選確率デフォルト
#define EFFENC_CAVE_INTERVAL (50)  //エフェクト抽選洞窟インターバル
#define EFFENC_CAVE_PROB (20)  //エフェクト抽選確率洞窟
#define EFFENC_BRIDGE_INTERVAL (50)  //エフェクト抽選橋インターバル
#define EFFENC_BRIDGE_PROB (40)  //エフェクト抽選確率橋

//======================================================================
//  define
//======================================================================
typedef enum{
 EFFENC_TYPE_SGRASS_NORMAL,  ///<短い草(ノーマル)
 EFFENC_TYPE_SGRASS_SNOW,  ///<短い草(豪雪地帯)
 EFFENC_TYPE_SGRASS_WARM,  ///<短い草(常春)
 EFFENC_TYPE_LGRASS,  ///<長い草
 EFFENC_TYPE_CAVE,    ///<洞窟
 EFFENC_TYPE_WATER,   ///<淡水
 EFFENC_TYPE_SEA,     ///<海
 EFFENC_TYPE_BRIDGE,  ///<橋
 EFFENC_TYPE_MAX,
}EFFENC_TYPE_ID;

