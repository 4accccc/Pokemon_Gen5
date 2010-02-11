//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_no.h
 *	@brief		天気ナンバー
 *	@author		tomoya takahashi
 *	@data		2009.03.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef _cplusplus
extern "C"{
#endif

#ifndef __WEATHER_NO_H__
#define __WEATHER_NO_H__


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

#ifndef __ASM_NO_DEF_
//-------------------------------------
///	天気ナンバー型指定(battle.h BtlWeather型との差別化のため)
//=====================================
typedef u16 WEATHER_NO;
#endif  //__ASM_NO_DEF_

//-------------------------------------
///	天気ナンバー無効
//=====================================
#define   WEATHER_NO_NONE   (0xffff)

//-------------------------------------
///	天気ナンバー
//=====================================
#define		WEATHER_NO_SUNNY			(0)				// 晴れ
#define		WEATHER_NO_SNOW				(1)				// 雪		
#define		WEATHER_NO_RAIN				(2)				// 雨		
#define		WEATHER_NO_STORM			(3)				// 砂嵐		
#define		WEATHER_NO_SPARK			(4)				// 雷雨
#define		WEATHER_NO_SNOWSTORM		(5)				// 吹雪
#define		WEATHER_NO_ARARE			(6)				// 吹雪
#define		WEATHER_NO_RAIKAMI			(7)				// ライカミ
#define		WEATHER_NO_KAZAKAMI			(8)				// カザカミ
#define		WEATHER_NO_DIAMONDDUST	(9)				// ダイヤモンドダスト
#define		WEATHER_NO_MIST	        (10)		// 霧

#define		WEATHER_NO_MIRAGE			(11)				// 蜃気楼

#define		WEATHER_NO_NUM				(12)			// 天気数

	// 戦闘フィールド状態していよう
	// バトルファクトリーのルーレット施設で
	// 実際のフィールドおきる天気以外の状態を
	// バトルに渡さなくてはいけなくなったため作成。
	//
	// PLとGSの通信でも使用される定数なので、
	// 値は一定にしてください。
#define		WEATHER_NO_BTONLYSTART (1000)	// バトルオンリーの状態開始定数
#define		WEATHER_NO_HIGHSUNNY	(1001)	// 日本晴れ
#define		WEATHER_NO_TRICKROOM	(1002)	// トリックルーム



#if 0		// プラチナの天気一覧
#define		WEATHER_NO_SUNNY			(0)				// 晴れ
#define		WEATHER_NO_CLOUDINESS		(1)				// 曇り		BG	
#define		WEATHER_NO_RAIN				(2)				// 雨		OAM	FOG
#define		WEATHER_NO_STRAIN			(3)				// 大雨		OAM FOG 
#define		WEATHER_NO_SPARK			(4)				// 大雨		OAM FOG BG
#define		WEATHER_NO_SNOW				(5)				// 雪		OAM FOG
#define		WEATHER_NO_SNOWSTORM		(6)				// 吹雪		OAM FOG
#define		WEATHER_NO_SNOWSTORM_H		(7)				// 猛吹雪	OAM FOG BG
#define		WEATHER_NO_FOG				(8)				// 霧大		FOG	
#define		WEATHER_NO_VOLCANO			(9)				// 火山灰	OAM FOG BG
#define		WEATHER_NO_SANDSTORM		(10)			// BG使用砂嵐	OAM FOG BG
#define		WEATHER_NO_DIAMONDDUST		(11)			// スノーダスト	OAM FOG
#define		WEATHER_NO_SPIRIT			(12)			// 気合		OAM
#define		WEATHER_NO_MYSTIC			(13)			// 神秘		OAM FOG
#define		WEATHER_NO_MIST1			(14)			// 霧払いの霧	FOG	BG
#define		WEATHER_NO_MIST2			(15)			// 霧払いの霧	FOG	BG
#define		WEATHER_NO_FLASH			(16)			// フラッシュ		BG

#define		WEATHER_NO_SPARK_EFF		(17)			// 雷		BG
#define		WEATHER_NO_FOGS				(18)			// 霧		FOG
#define		WEATHER_NO_FOGM				(19)			// 霧中		FOG

#define		WEATHER_NO_RAINBOW			(20)			// 虹		BG
#define		WEATHER_NO_SNOW_STORM_BG	(21)			// BG使用吹雪	OAM FOG BG
#define		WEATHER_NO_STORM			(22)			// 砂嵐		OAM FOG

#define		WEATHER_NO_KOGOREBI			(23)			// 凝れ美	BG
#define		WEATHER_NO_DOUKUTU00		(24)			// 洞窟	FOG
#define		WEATHER_NO_DOUKUTU01		(25)			// 洞窟	FOG
#define		WEATHER_NO_GINGA00			(26)			// 銀河団がいて薄暗い
#define		WEATHER_NO_LOSTTOWER00		(27)			// LostTower薄暗い
#define		WEATHER_NO_HARDMOUNTAIN		(28)			// ハードマウンテン
#define		WEATHER_NO_DOUKUTU02		(29)			// 洞窟　FOG　青
#define		WEATHER_NO_USUGURAI			(30)			// 薄暗い　FOG

#define		WEATHER_NO_NUM				(31)			// 天気数
#endif





#endif		// __WEATHER_NO_H__

#ifdef _cplusplus
}
#endif

