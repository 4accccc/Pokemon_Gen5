//======================================================================
/**
 * @file	musical_script_command.h.h
 * @brief	ミュージカルスクリプト用のマクロ定義ファイル
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//======================================================================

#ifndef MUSICAL_SCRIPT_COMMAND_H__
#define MUSICAL_SCRIPT_COMMAND_H__


#endif MUSICAL_SCRIPT_COMMAND_H__

//命令シンボル宣言
//COMMAND_START
#define SCRIPT_ENUM_ScriptFinish 	 (0)
#define SCRIPT_ENUM_FrameWait 	 (1)
#define SCRIPT_ENUM_FrameWaitTime 	 (2)
#define SCRIPT_ENUM_CurtainUp 	 (3)
#define SCRIPT_ENUM_CurtainDown 	 (4)
#define SCRIPT_ENUM_CurtainMove 	 (5)
#define SCRIPT_ENUM_StageMove 	 (6)
#define SCRIPT_ENUM_StageChangeBg 	 (7)
#define SCRIPT_ENUM_PokeShow 	 (8)
#define SCRIPT_ENUM_PokeDir 	 (9)
#define SCRIPT_ENUM_PokeMove 	 (10)
#define SCRIPT_ENUM_PokeStopAnime 	 (11)
#define SCRIPT_ENUM_PokeStartAnime 	 (12)
#define SCRIPT_ENUM_PokeChangeAnime 	 (13)
#define SCRIPT_ENUM_PokeActionJump 	 (14)
#define SCRIPT_ENUM_ObjectCreate 	 (15)
#define SCRIPT_ENUM_ObjectDelete 	 (16)
#define SCRIPT_ENUM_ObjectShow 	 (17)
#define SCRIPT_ENUM_ObjectHide 	 (18)
#define SCRIPT_ENUM_ObjectMove 	 (19)
#define SCRIPT_ENUM_EffectCreate 	 (20)
#define SCRIPT_ENUM_EffectDelete 	 (21)
#define SCRIPT_ENUM_EffectStart 	 (22)
#define SCRIPT_ENUM_EffectStop 	 (23)
#define SCRIPT_ENUM_EffectRepeatStart 	 (24)
#define SCRIPT_ENUM_LightShowCircle 	 (25)
#define SCRIPT_ENUM_LightHide 	 (26)
#define SCRIPT_ENUM_LightMove 	 (27)
#define SCRIPT_ENUM_LightColor 	 (28)
#define SCRIPT_ENUM_MessageShow 	 (29)
#define SCRIPT_ENUM_MessageHide 	 (30)
#define SCRIPT_ENUM_MessageColor 	 (31)
#define SEQ_END 	 (32)

#ifndef __C_NO_DEF_

//勝手にパディングを入れられないようにする
	.option	alignment off
	
#pragma mark [>System Command
//命令マクロ定義
//======================================================================
/**
 * @brief	システム：ミュージカル終了
 *
 * #param_num	0
 */
//======================================================================
	.macro	ComScriptFinish
	.short	SCRIPT_ENUM_ScriptFinish)
	.endm

//======================================================================
/**
 * @brief	システム：指定フレームウェイト
 *
 * #param_num	1
 * @param	frame	フレーム数
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComFrameWait	frame
	.short	SCRIPT_ENUM_FrameWait
	.long	\frame
	.endm

//======================================================================
/**
 * @brief	システム：指定までフレームウェイト
 *
 * #param_num	1
 * @param	frame	フレーム数
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComFrameWaitTime	frame
	.short	SCRIPT_ENUM_FrameWaitTime
	.long	\frame
	.endm

#pragma mark [>Curtain Command
//======================================================================
/**
 * @brief	カーテン：幕を上げる(固定速度
 *
 * #param_num	0
 */
//======================================================================
	.macro	ComCurtainUp
	.short	SCRIPT_ENUM_CurtainUp
	.endm

//======================================================================
/**
 * @brief	カーテン：幕を下げる(固定速度
 *
 * #param_num	0
 */
//======================================================================
	.macro	ComCurtainDown
	.short	SCRIPT_ENUM_CurtainDown
	.endm

//======================================================================
/**
 * @brief	カーテン：幕を動かす(速度設定可
 *
 * #param_num	2
 * @param	frame	フレーム数
 * @param	heaight	Y座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComCurtainMove	frame	height
	.short	SCRIPT_ENUM_CurtainMove
	.long	\frame
	.long	\height
	.endm

#pragma mark [>Stage Command
//======================================================================
/**
 * @brief	ステージ：舞台のスクロール
 *
 * #param_num	2
 * @param	frame	フレーム数
 * @param	pos		X座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComStageMove	frame	pos
	.short	SCRIPT_ENUM_StageMove
	.long	\frame
	.long	\pos
	.endm

//======================================================================
/**
 * @brief	ステージ：背景の読み替え
 *
 * #param_num	1
 * @param	bgNo	背景番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComStageChangeBg	bgNo
	.short	SCRIPT_ENUM_StageChangeBg
	.long	\bgNo
	.endm

#pragma mark [>Pokemon Command
//======================================================================
/**
 * @brief	ポケモン：表示切替
 *
 * #param_num	2
 * @param	pokeNo	ポケモン番号
 * @param	flg		ON/OFF
 *
 * #param	VALUE_INT
 * #param	COMBOBOX_TEXT	ON	OFF
 * #param	COMBOBOX_VALUE	1	0
 */
//======================================================================
	.macro	ComPokeShow	pokeNo	flg
	.short	SCRIPT_ENUM_PokeShow
	.long	\pokeNo
	.long	\flg
	.endm

//======================================================================
/**
 * @brief	ポケモン：方向設定
 *
 * #param_num	2
 * @param	pokeNo	ポケモン番号
 * @param	dir		向き(左：右)
 *
 * #param	VALUE_INT
 * #param	COMBOBOX_TEXT	左	右
 * #param	COMBOBOX_VALUE	0	1
 */
//======================================================================
	.macro	ComPokeDir	pokeNo	dir
	.short	SCRIPT_ENUM_PokeDir
	.long	\pokeNo
	.long	\dir
	.endm

//======================================================================
/**
 * @brief	ポケモン：移動
 *
 * #param_num	5
 * @param	pokeNo	ポケモン番号
 * @param	frame	フレーム数
 * @param	posX	Ｘ座標
 * @param	posY	Ｙ座標
 * @param	posZ	Ｚ座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	ComPokeMove	pokeNo	frame	posX	posY	posZ
	.short	SCRIPT_ENUM_PokeMove
	.long	\pokeNo
	.long	\frame
	.long	\posX
	.long	\posY
	.long	\posZ
	.endm

//======================================================================
/**
 * @brief	ポケモン：アニメ停止
 *
 * #param_num	1
 * @param	pokeNo	ポケモン番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComPokeStopAnime	pokeNo
	.short	SCRIPT_ENUM_PokeStopAnime
	.long	\pokeNo
	.endm

//======================================================================
/**
 * @brief	ポケモン：アニメ開始
 *
 * #param_num	1
 * @param	pokeNo	ポケモン番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComPokeStartAnime	pokeNo
	.short	SCRIPT_ENUM_PokeStartAnime
	.long	\pokeNo
	.endm

//======================================================================
/**
 * @brief	ポケモン：アニメ変更
 *
 * #param_num	2
 * @param	pokeNo	ポケモン番号
 * @param	animeNo	アニメ番号
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComPokeChangeAnime	pokeNo	animeNo
	.short	SCRIPT_ENUM_PokeChangeAnime
	.long	\pokeNo
	.long	\animeNo
	.endm

#pragma mark [>Pokemon Action Command
//======================================================================
/**
 * @brief	ポケモンアクション：跳ねる
 *
 * #param_num	4
 * @param	pokeNo	ポケモン番号
 * @param	interval	間隔
 * @param	repeat	回数
 * @param	height	高さ
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	ComPokeActionJump	pokeNo	interval	repeat	height
	.short	SCRIPT_ENUM_PokeActionJump
	.long	\pokeNo
	.long	\interval
	.long	\repeat
	.long	\height
	.endm

#pragma mark [>Object Command
//======================================================================
/**
 * @brief	オブジェクト：作成
 *
 * #param_num	2
 * @param	objNo	オブジェクト管理番号
 * @param	type	オブジェクト番号
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComObjectCreate	objNo	type
	.short	SCRIPT_ENUM_ObjectCreate
	.long	\objNo
	.long	\type
	.endm

//======================================================================
/**
 * @brief	オブジェクト：削除
 *
 * #param_num	1
 * @param	objNo	オブジェクト管理番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComObjectDelete	objNo
	.short	SCRIPT_ENUM_ObjectDelete
	.long	\objNo
	.endm

//======================================================================
/**
 * @brief	オブジェクト：表示
 *
 * #param_num	1
 * @param	objNo	オブジェクト管理番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComObjectShow	objNo
	.short	SCRIPT_ENUM_ObjectShow
	.long	\objNo
	.endm

//======================================================================
/**
 * @brief	オブジェクト：非表示
 *
 * #param_num	1
 * @param	objNo	オブジェクト管理番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComObjectHide	objNo
	.short	SCRIPT_ENUM_ObjectHide
	.long	\objNo
	.endm

//======================================================================
/**
 * @brief	オブジェクト：移動
 *
 * #param_num	5
 * @param	objNo	オブジェクト管理番号
 * @param	frame	フレーム数
 * @param	posX	Ｘ座標
 * @param	posY	Ｙ座標
 * @param	posZ	Ｚ座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	ComObjectMove	objNo	frame	posX	posY	posZ
	.short	SCRIPT_ENUM_ObjectMove
	.long	\objNo
	.long	\frame
	.long	\posX
	.long	\posY
	.long	\posZ
	.endm


#pragma mark [>Effect Command
//======================================================================
/**
 * @brief	エフェクト：作成
 *
 * #param_num	2
 * @param	effNo	エフェクト管理番号
 * @param	type	エフェクト番号
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComEffectCreate	effNo	type
	.short	SCRIPT_ENUM_EffectCreate
	.long	\effNo
	.long	\type
	.endm

//======================================================================
/**
 * @brief	エフェクト：破棄
 *
 * #param_num	1
 * @param	effNo	エフェクト管理番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComEffectDelete	effNo
	.short	SCRIPT_ENUM_EffectDelete
	.long	\effNo
	.endm

//======================================================================
/**
 * @brief	エフェクト：再生
 *
 * #param_num	5
 * @param	effNo	エフェクト管理番号
 * @param	emitNo	エミッタ番号
 * @param	posX	Ｘ座標
 * @param	posY	Ｙ座標
 * @param	posZ	Ｚ座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	ComEffectStart	effNo	emitNo	posX	posY	posZ
	.short	SCRIPT_ENUM_EffectStart
	.long	\effNo
	.long	\emitNo
	.long	\posX
	.long	\posY
	.long	\posZ
	.endm

//======================================================================
/**
 * @brief	エフェクト：停止
 *
 * #param_num	2
 * @param	effNo	エフェクト管理番号
 * @param	emitNo	エミッタ番号
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComEffectStop	effNo	emitNo
	.short	SCRIPT_ENUM_EffectStop
	.long	\effNo
	.long	\emitNo
	.endm

//======================================================================
/**
 * @brief	エフェクト：連続再生
 *
 * #param_num	7
 * @param	effNo	エフェクト管理番号
 * @param	emitNo	エミッタ番号
 * @param	interval	間隔
 * @param	repeat	回数
 * @param	posX1	Ｘ座標
 * @param	posY1	Ｙ座標
 * @param	posZ1	Ｚ座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	ComEffectRepeatStart	effNo	emitNo	interval	repeat	posX1	posY1	posZ1
	.short	SCRIPT_ENUM_EffectRepeatStart
	.long	\effNo
	.long	\emitNo
	.long	\interval
	.long	\repeat
	.long	\posX1
	.long	\posX1
	.long	\posY1
	.long	\posY1
	.long	\posZ1
	.long	\posZ1
	.endm

#pragma mark [>Light Command
//======================================================================
/**
 * @brief	スポットライト：作成
 *
 * #param_num	2
 * @param	lightNo	スポットライト管理番号
 * @param	radian	半径(未動作)
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComLightShowCircle	lightNo	radian
	.short	SCRIPT_ENUM_LightShowCircle
	.long	\lightNo
	.long	\radian
	.endm

//======================================================================
/**
 * @brief	スポットライト：破棄
 *
 * #param_num	1
 * @param	lightNo	スポットライト管理番号
 *
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComLightHide	lightNo
	.short	SCRIPT_ENUM_LightHide
	.long	\lightNo
	.endm

//======================================================================
/**
 * @brief	スポットライト：移動
 *
 * #param_num	5
 * @param	lightNo	スポットライト管理番号
 * @param	frame	フレーム数
 * @param	posX	Ｘ座標
 * @param	posY	Ｙ座標
 * @param	posZ	Ｚ座標
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	ComLightMove	lightNo	frame	posX	posY	posZ
	.short	SCRIPT_ENUM_LightMove
	.long	\lightNo
	.long	\frame
	.long	\posX
	.long	\posY
	.long	\posZ
	.endm

//======================================================================
/**
 * @brief	スポットライト：色設定
 *
 * #param_num	5
 * @param	lightNo	スポットライト管理番号
 * @param	colR	色：Ｒ
 * @param	colG	色：Ｇ
 * @param	colB	色：Ｂ
 * @param	alpha	透明度
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComLightColor	lightNo	colR	colG	colB	alpha
	.short	SCRIPT_ENUM_LightColor
	.long	\lightNo
	.long	\colR
	.long	\colG
	.long	\colB
	.long	\alpha
	.endm

#pragma mark [>Message Command
//======================================================================
/**
 * @brief	メッセージ：表示
 *
 * #param_num	2
 * @param	msgNo	メッセージ番号
 * @param	speed	メッセージ速度(1文字辺りのフレーム数)
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComMessageShow	msgNo	speed
	.short	SCRIPT_ENUM_MessageShow
	.long	\msgNo
	.long	\speed
	.endm

//======================================================================
/**
 * @brief	メッセージ：非表示
 *
 * #param_num	0
 *
 */
//======================================================================
	.macro	ComMessageHide	
	.short	SCRIPT_ENUM_MessageHide
	.endm

//======================================================================
/**
 * @brief	メッセージ：色変更
 *
 * #param_num	3
 * @param	colLetter	文字色
 * @param	colShadow	影色
 * @param	colBack	背景色
 *
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	ComMessageColor	colLetter	colShadow	colBack
	.short	SCRIPT_ENUM_MessageColor
	.long	\colLetter
	.long	\colShadow
	.long	\colBack
	.endm

#pragma mark [>Other Command
//======================================================================
/**
 * @brief	ミュージカル終了(ダミー
 *
 * #param_num	0
 */
//======================================================================
	.macro	ComSEQ_END
	.short	SCRIPT_ENUM_ScriptFinish
	.endm

#endif //__C_NO_DEF_

