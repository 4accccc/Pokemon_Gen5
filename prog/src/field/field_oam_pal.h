//==============================================================================
/**
 * @file    field_oam_pal.h
 * @brief   フィールドマップ：メイン画面のOAMパレット使用位置定義ファイル
 * @author  matsuda
 * @date    2009.12.18(金)
 */
//==============================================================================
#pragma once

//--------------------------------------------------------------
/**
 * フィールドマップ：メイン画面のOAMパレット番号
 */
//--------------------------------------------------------------
typedef enum{
//--------------------------------------------------------------
//  固定エリア
//--------------------------------------------------------------
  FLDOAM_PALNO_WEATHER,       ///<天気
  FLDOAM_PALNO_PLACENAME,     ///<地名表示  文字
  FLDOAM_PALNO_Y_MENU,        ///<Yボタンショートカットメニュー
  FLDOAM_PALNO_PLACENAME_PLACE_ICON,  //地名表示  場所アイコン
  FLDOAM_PALNO_PLACENAME_SEAZON_ICON, //地名表示  季節アイコン
  FLDOAM_PALNO_FMISSION_INFO, ///<フェスミッション情報表示
  
//--------------------------------------------------------------
//  フリーエリア
//--------------------------------------------------------------
  FLDOAM_PALNO_FREE_START,    ///<イベント毎に使用出来るフリーエリア開始位置
  
  //手持ち or バトルボックス選択リスト イベント中
  FLDOAM_PALNO_FREE_PSL_POKEICON = FLDOAM_PALNO_FREE_START,
  FLDOAM_PALNO_FREE_PSL_ITEM = FLDOAM_PALNO_FREE_PSL_POKEICON + 3,
  FLDOAM_PALNO_FREE_PSL_FONTOAM,

	//おしゃべり
	FLDOAM_PALNO_FREE_OSYABERI_POKE = FLDOAM_PALNO_FREE_START,		///<ポケモンＯＢＪ

}FLDOAM_PALNO;
