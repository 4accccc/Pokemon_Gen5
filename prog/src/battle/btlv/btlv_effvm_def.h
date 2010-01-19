//======================================================================
/**
 * @file	btlv_effvm_def.h
 * @brief	エフェクトシーケンス用のマクロ定義ファイル
 * @author	HisashiSogabe
 * @date	2009.02.12
 */
//======================================================================

//====================================================================================
/**
 *	定数定義
 */
//====================================================================================
#ifndef __BTLV_EFFVM_DEF_H_
#define __BTLV_EFFVM_DEF_H_

#define	BTLV_EFFVM_STACK_SIZE	( 16 )	///<使用するスタックのサイズ
#define	BTLV_EFFVM_REG_SIZE		( 8 )		///<使用するレジスタの数

//スクリプトアドレステーブル用定義
#define	BTLEFF_AA_SAME			( 0xffffffff )		//立ち位置AAと同じ
#define	BTLEFF_AA_REVERSE		( 0xfffffffe )		//立ち位置AAの反転
#define	BTLEFF_BB_SAME			( 0xfffffffd )		//立ち位置BBと同じ
#define	BTLEFF_BB_REVERSE		( 0xfffffffc )		//立ち位置BBの反転
#define	BTLEFF_A_SAME				( 0xfffffffb )		//立ち位置Aと同じ
#define	BTLEFF_A_REVERSE		( 0xfffffffa )		//立ち位置Aの反転
#define	BTLEFF_B_SAME				( 0xfffffff9 )		//立ち位置Bと同じ
#define	BTLEFF_B_REVERSE		( 0xfffffff8 )		//立ち位置Bの反転
#define	BTLEFF_C_SAME				( 0xfffffff7 )		//立ち位置Cと同じ
#define	BTLEFF_C_REVERSE		( 0xfffffff6 )		//立ち位置Cの反転
#define	BTLEFF_D_SAME				( 0xfffffff5 )		//立ち位置Dと同じ
#define	BTLEFF_D_REVERSE		( 0xfffffff4 )		//立ち位置Dの反転

//カメラ移動タイプ
#define	BTLEFF_CAMERA_MOVE_DIRECT										( 0 )		//ダイレクト
#define	BTLEFF_CAMERA_MOVE_INTERPOLATION						( 1 )		//追従
#define	BTLEFF_CAMERA_MOVE_INTERPOLATION_RELATIVITY	( 2 )		//追従（相対指定）

//カメラ移動先
#define	BTLEFF_CAMERA_POS_AA						( 0 )
#define	BTLEFF_CAMERA_POS_BB						( 1 )
#define	BTLEFF_CAMERA_POS_A							( 2 )
#define	BTLEFF_CAMERA_POS_B							( 3 )
#define	BTLEFF_CAMERA_POS_C							( 4 )
#define	BTLEFF_CAMERA_POS_D							( 5 )
#define	BTLEFF_CAMERA_POS_E							( 6 )
#define	BTLEFF_CAMERA_POS_F							( 7 )
#define	BTLEFF_CAMERA_POS_INIT					( 8 )
#define	BTLEFF_CAMERA_POS_ATTACK				( 9 )
#define	BTLEFF_CAMERA_POS_ATTACK_PAIR		( 10 )
#define	BTLEFF_CAMERA_POS_DEFENCE				( 11 )
#define	BTLEFF_CAMERA_POS_DEFENCE_PAIR	( 12 )

#define	BTLEFF_CAMERA_POS_NONE					( 0xffffffff )

//カメラゆれ
#define	BTLEFF_CAMERA_SHAKE_VERTICAL		( 0 )
#define	BTLEFF_CAMERA_SHAKE_HORIZON		  ( 1 )

//射影モード
#define	BTLEFF_CAMERA_PROJECTION_ORTHO					( 0 )
#define	BTLEFF_CAMERA_PROJECTION_PERSPECTIVE		( 1 )

//パーティクル再生
#define	BTLEFF_PARTICLE_PLAY_POS_AA				      ( 0 )
#define	BTLEFF_PARTICLE_PLAY_POS_BB				      ( 1 )
#define	BTLEFF_PARTICLE_PLAY_POS_A				      ( 2 )
#define	BTLEFF_PARTICLE_PLAY_POS_B				      ( 3 )
#define	BTLEFF_PARTICLE_PLAY_POS_C				      ( 4 )
#define	BTLEFF_PARTICLE_PLAY_POS_D				      ( 5 )
#define	BTLEFF_PARTICLE_PLAY_POS_E				      ( 6 )
#define	BTLEFF_PARTICLE_PLAY_POS_F				      ( 7 )
#define	BTLEFF_PARTICLE_PLAY_SIDE_NONE		      ( 8 )
#define	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	      ( 9 )
#define	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK_MINUS	( 10 )
#define	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	      ( 11 )
#define	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS	( 12 )

//エミッタ移動
#define	BTLEFF_EMITTER_MOVE_NONE							( 0 )
#define	BTLEFF_EMITTER_MOVE_STRAIGHT					( 1 )
#define	BTLEFF_EMITTER_MOVE_CURVE							( 2 )
#define	BTLEFF_EMITTER_MOVE_CURVE_HALF				( 3 )

//エミッタ円移動
#define BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_L   ( 0 )
#define BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_R   ( 1 )
#define BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_L  ( 2 )
#define BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_R  ( 3 )
#define BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_L   ( 4 )
#define BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_R   ( 5 )

//ポケモン操作
#define	BTLEFF_POKEMON_POS_AA							( 0 )
#define	BTLEFF_POKEMON_POS_BB							( 1 )
#define	BTLEFF_POKEMON_POS_A							( 2 )
#define	BTLEFF_POKEMON_POS_B							( 3 )
#define	BTLEFF_POKEMON_POS_C							( 4 )
#define	BTLEFF_POKEMON_POS_D							( 5 )
#define	BTLEFF_POKEMON_POS_E					    ( 6 )
#define	BTLEFF_POKEMON_POS_F							( 7 )
#define	BTLEFF_TRAINER_POS_AA					    ( 8 )
#define	BTLEFF_TRAINER_POS_BB					    ( 9 )
#define	BTLEFF_TRAINER_POS_A					    ( 10 )
#define	BTLEFF_TRAINER_POS_B					    ( 11 )
#define	BTLEFF_TRAINER_POS_C					    ( 12 )
#define	BTLEFF_TRAINER_POS_D					    ( 13 )
#define	BTLEFF_POKEMON_SIDE_ATTACK				( 14 )		//攻撃側
#define	BTLEFF_POKEMON_SIDE_ATTACK_PAIR		( 15 )		//攻撃側ペア
#define	BTLEFF_POKEMON_SIDE_DEFENCE				( 16 )		//防御側
#define	BTLEFF_POKEMON_SIDE_DEFENCE_PAIR	( 17 )		//防御側ペア

#define	BTLEFF_POKEMON_MOVE_DIRECT						( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_MOVE_INTERPOLATION			( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_MOVE_ROUNDTRIP					( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_MOVE_ROUNDTRIP_LONG		( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_POKEMON_MOVE_INIT		          ( BTLEFF_POKEMON_MOVE_ROUNDTRIP_LONG + 1 )
#define	BTLEFF_POKEMON_SCALE_DIRECT						( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_SCALE_INTERPOLATION		( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_SCALE_ROUNDTRIP				( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_SCALE_ROUNDTRIP_LONG		( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_POKEMON_ROTATE_DIRECT					( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_ROTATE_INTERPOLATION		( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_ROTATE_ROUNDTRIP				( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_ROTATE_ROUNDTRIP_LONG	( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_POKEMON_ALPHA_DIRECT					  ( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_ALPHA_INTERPOLATION		( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_ALPHA_ROUNDTRIP				( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_ALPHA_ROUNDTRIP_LONG	  ( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_MEPACHI_ON											( MCSS_MEPACHI_ON )
#define	BTLEFF_MEPACHI_OFF										( MCSS_MEPACHI_OFF )
#define	BTLEFF_MEPACHI_MABATAKI								( 2 )
#define	BTLEFF_ANM_STOP												( MCSS_ANM_STOP_ON )
#define	BTLEFF_ANM_START											( MCSS_ANM_STOP_OFF )

//ポケモン円移動
#define BTLEFF_AXIS_X_L                       ( 0 )     //Ｘ軸左
#define BTLEFF_AXIS_X_R                       ( 1 )     //Ｘ軸右
#define BTLEFF_AXIS_Y_L                       ( 2 )     //Ｙ軸左
#define BTLEFF_AXIS_Y_R                       ( 3 )     //Ｙ軸右
#define BTLEFF_AXIS_Z_L                       ( 4 )     //Ｚ軸左
#define BTLEFF_AXIS_Z_R                       ( 5 )     //Ｚ軸右

#define BTLEFF_SHIFT_H_P                      ( 0 )     //シフトＨ＋
#define BTLEFF_SHIFT_H_M                      ( 1 )     //シフトＨ－
#define BTLEFF_SHIFT_V_P                      ( 2 )     //シフトＶ＋
#define BTLEFF_SHIFT_V_M                      ( 3 )     //シフトＶ－

//トレーナー操作
#define	BTLEFF_TRAINER_MOVE_DIRECT						( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_TRAINER_MOVE_INTERPOLATION			( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_TRAINER_MOVE_ROUNDTRIP					( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_TRAINER_MOVE_ROUNDTRIP_LONG		( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )


//SE再生プレーヤー定義
#define BTLEFF_SEPLAY_SYSTEM                  ( 0 )
#define BTLEFF_SEPLAY_SE1                     ( 1 )
#define BTLEFF_SEPLAY_SE2                     ( 2 )
#define BTLEFF_SEPLAY_PSG                     ( 3 )
#define BTLEFF_SEPLAY_DEFAULT                 ( 4 )

//SEストッププレーヤー定義
#define BTLEFF_SESTOP_SYSTEM                  ( 0 )
#define BTLEFF_SESTOP_SE1                     ( 1 )
#define BTLEFF_SESTOP_SE2                     ( 2 )
#define BTLEFF_SESTOP_PSG                     ( 3 )
#define BTLEFF_SESTOP_ALL                     ( 4 )

//SEパン
#define BTLEFF_SEPAN_INTERPOLATION            ( 0 )
#define BTLEFF_SEPAN_ROUNDTRIP                ( 1 )

#define BTLEFF_SEPAN_ATTACK                   ( BTLEFF_POKEMON_SIDE_ATTACK )
#define BTLEFF_SEPAN_DEFENCE                  ( BTLEFF_POKEMON_SIDE_DEFENCE )

//SEエフェクト
#define BTLEFF_SEEFFECT_INTERPOLATION         ( 0 )
#define BTLEFF_SEEFFECT_ROUNDTRIP             ( 1 )

#define BTLEFF_SEEFFECT_PITCH                 ( 0 )
#define BTLEFF_SEEFFECT_VOLUME                ( 1 )
#define BTLEFF_SEEFFECT_PAN                   ( 2 )

//エフェクト終了待ち
#define	BTLEFF_EFFENDWAIT_ALL									( 0 )
#define	BTLEFF_EFFENDWAIT_CAMERA							( 1 )
#define	BTLEFF_EFFENDWAIT_PARTICLE						( 2 )
#define	BTLEFF_EFFENDWAIT_POKEMON							( 3 )
#define	BTLEFF_EFFENDWAIT_TRAINER							( 4 )
#define	BTLEFF_EFFENDWAIT_BG							    ( 5 )
#define	BTLEFF_EFFENDWAIT_PALFADE_STAGE		  	( 6 )
#define	BTLEFF_EFFENDWAIT_PALFADE_FIELD		  	( 7 )
#define	BTLEFF_EFFENDWAIT_PALFADE_3D			    ( 8 )
#define	BTLEFF_EFFENDWAIT_PALFADE_EFFECT	    ( 9 )
#define BTLEFF_EFFENDWAIT_SEALL               ( 10 )
#define BTLEFF_EFFENDWAIT_SE1                 ( 11 )
#define BTLEFF_EFFENDWAIT_SE2                 ( 12 )
#define BTLEFF_EFFENDWAIT_PSG                 ( 13 )
#define BTLEFF_EFFENDWAIT_SYSTEM              ( 14 )

//制御モード
#define	BTLEFF_CONTROL_MODE_CONTINUE					( 0 )
#define	BTLEFF_CONTROL_MODE_SUSPEND						( 1 )

//条件分岐
#define BTLEFF_COND_NO_EXIST                  ( 0 )
#define BTLEFF_COND_EXIST                     ( 1 )

#define BTLEFF_FX32_SHIFT											( 12 )

//BGパレットフェード対象定義
#define BTLEFF_PAL_FADE_STAGE                 ( 0 )
#define BTLEFF_PAL_FADE_FIELD                 ( 1 )
#define BTLEFF_PAL_FADE_3D                    ( 2 )
#define BTLEFF_PAL_FADE_EFFECT                ( 3 )
#define BTLEFF_PAL_FADE_ALL                   ( 4 )

//背景で使用されているパレットをビットで表現
#define BTLEFF_PAL_FADE_EFFECT_BIT            ( 0xff00 )

//BGプライオリティ設定
#define BTLEFF_BG_PRI_FRONT                   ( 0 )
#define BTLEFF_BG_PRI_BACK                    ( 1 )

//BGスクロール
#define	BTLEFF_BG_SCROLL_DIRECT						    ( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_BG_SCROLL_INTERPOLATION			  ( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_BG_SCROLL_ROUNDTRIP					  ( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_BG_SCROLL_ROUNDTRIP_LONG		    ( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_BG_SCROLL_EVERY		            ( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG + 1 )

//BGラスタースクロール
#define	BTLEFF_BG_RASTER_SCROLL_H						  ( 0 )
#define	BTLEFF_BG_RASTER_SCROLL_V						  ( 1 )

//BGα値
#define	BTLEFF_BG_ALPHA_DIRECT					      ( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_BG_ALPHA_INTERPOLATION		      ( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_BG_ALPHA_ROUNDTRIP				      ( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_BG_ALPHA_ROUNDTRIP_LONG	      ( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )

//BG表示/非表示
#define	BTLEFF_STAGE			( 0 )
#define	BTLEFF_FIELD			( 1 )
#define	BTLEFF_EFFECT			( 2 )

#define	BTLEFF_VANISH_OFF	( 0 )
#define	BTLEFF_VANISH_ON	( 1 )

//OBJ操作
#define	BTLEFF_OBJ_MOVE_DIRECT						    ( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_OBJ_MOVE_INTERPOLATION			    ( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_OBJ_MOVE_ROUNDTRIP					    ( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_OBJ_MOVE_ROUNDTRIP_LONG		    ( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )

#define	BTLEFF_OBJ_SCALE_DIRECT						    ( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_OBJ_SCALE_INTERPOLATION			    ( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_OBJ_SCALE_ROUNDTRIP					    ( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_OBJ_SCALE_ROUNDTRIP_LONG		    ( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )

//指定ワーク定義
#define BTLEFF_WORK_POS_AA_WEIGHT         ( 0 )   ///<POS_AAの体重
#define BTLEFF_WORK_POS_BB_WEIGHT         ( 1 )   ///<POS_BBの体重
#define BTLEFF_WORK_POS_A_WEIGHT          ( 2 )   ///<POS_Aの体重
#define BTLEFF_WORK_POS_B_WEIGHT          ( 3 )   ///<POS_Bの体重
#define BTLEFF_WORK_POS_C_WEIGHT          ( 4 )   ///<POS_Cの体重
#define BTLEFF_WORK_POS_D_WEIGHT          ( 5 )   ///<POS_Dの体重
#define BTLEFF_WORK_POS_E_WEIGHT          ( 6 )   ///<POS_Eの体重
#define BTLEFF_WORK_POS_F_WEIGHT          ( 7 )   ///<POS_Fの体重
#define BTLEFF_WORK_WAZA_RANGE            ( 8 )   ///<技の効果範囲
#define BTLEFF_WORK_TURN_COUNT            ( 9 )   ///< ターンによって異なるエフェクトを出す場合のターン指定。（ex.そらをとぶ）
#define BTLEFF_WORK_CONTINUE_COUNT        ( 10 )  ///< 連続して出すとエフェクトが異なる場合の連続カウンタ（ex. ころがる）
#define BTLEFF_WORK_YURE_CNT              ( 11 )  ///<ボールゆれるカウント
#define BTLEFF_WORK_GET_SUCCESS           ( 12 )  ///<捕獲成功かどうか
#define BTLEFF_WORK_ITEM_NO               ( 13 )  ///<ボールのアイテムナンバー
#define BTLEFF_WORK_SEQUENCE_WORK         ( 14 )  ///<汎用ワーク
#define BTLEFF_WORK_ATTACK_POKEMON_VANISH ( 15 )  ///<攻撃側のポケモンがきえているか

//条件式
#define BTLEFF_COND_EQUAL       ( 0 )   // ==
#define BTLEFF_COND_NOT_EQUAL   ( 1 )   // !=
#define BTLEFF_COND_MIMAN       ( 2 )   // <
#define BTLEFF_COND_KOERU       ( 3 )   // >
#define BTLEFF_COND_IKA         ( 4 )   // <=
#define BTLEFF_COND_IJOU        ( 5 )   // >=

//みがわり
#define BTLEFF_MIGAWARI_OFF     ( 0 )   // みがわりOFF
#define BTLEFF_MIGAWARI_ON      ( 1 )   // みがわりON

#endif //__BTLV_EFFVM_DEF_H_

//====================================================================================
/**
 *	VisualStudioで作成したエフェクトエディタツールでも使用されるファイルです 
 *	各命令マクロ定義のコメントからツールで表示されるコマンド名や引数の型などを判断します
 */
//====================================================================================

#if 0

ex)
 * #param_num	5
 * @param	type			カメラ移動タイプ
 * @param	move_pos	移動先
 * @param	frame			カメラタイプが追従のとき、何フレームで移動先に到達するかを指定
 * @param	wait			移動ウエイト
 * @param	brake			ブレーキをかけはじめるフレームを指定
 *
 * #param	COMBOBOX_TEXT	ダイレクト	追従
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_MOVE_DIRECT	BTLEFF_CAMERA_MOVE_INTERPOLATION
 * #param	COMBOBOX_TEXT	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D	初期位置	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_POS_AA	BTLEFF_CAMERA_POS_BB	BTLEFF_CAMERA_POS_A	BTLEFF_CAMERA_POS_B	BTLEFF_CAMERA_POS_C	BTLEFF_CAMERA_POS_D	BTLEFF_CAMERA_POS_INIT BTLEFF_CAMERA_POS_ATTACK BTLEFF_CAMERA_POS_DEFENCE
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT

#param_numを書いて、コマンドの引数の数を宣言をします(@paramの総数になります）
doxygen書式の@paramはそのままツール上のコメントとして表示されます
#paramを書くことで、ツールで読み込むパラメータであることの宣言をします
パラメータの型を記述します
・COMBOBOX_TEXT
　コンボボックス型にして、コンボボックスで表示するリストデータを列記します

・COMBOBOX_VALUE
　先に設定したコンボボックス型のインデックスに対応して実際のシーケンスソースに書き出されるラベル定義などを記述します

・VALUE_FX32
　FX32型の値（ツール上では、小数ありの数値入力を可能にします）
	型の宣言の後ろに、数値入力ダイアログ表示用のラベルを記述できます

・VALUE_INT
	INT型の値（ツール上では、小数なしの数値入力になります）
	型の宣言の後ろに、数値入力ダイアログ表示用のラベルを記述できます

・VALUE_VECFX32
	VecFx32型の値　型の宣言の後ろにタブ区切りで、数値入力ダイアログ表示用のラベルを記述できます
ex)
	VALUE_VECFX32	camPosX	camPosY	camPosZ
	
	こうすることによって、ツールのダイアログに、入力するための値の意味を表示させることができます
 
・VALUE_ANGLE
	角度入力用の値（0～360）
  コンバータで（0～0xffff）に変換します

・FILE_DIALOG
　ファイルダイアログを表示してファイル選択を促す　選択を絞る拡張子を指定できます

・FILE_DIALOG_COMBOBOX
　ファイルダイアログで選択したファイルをコンボボックスで表示します　選択を絞る拡張子を指定できます

・COMBOBOX_HEADER
　FILE_DIALOG_COMBOBOXで選択したファイルの拡張子.hファイルを読み込んでコンボボックスを表示します

#endif

//命令シンボル宣言
//COMMAND_START
#define	EC_CAMERA_MOVE							  ( 0 )
#define	EC_CAMERA_MOVE_COORDINATE		  ( 1 )
#define	EC_CAMERA_MOVE_ANGLE				  ( 2 )
#define	EC_CAMERA_SHAKE				        ( 3 )
#define	EC_CAMERA_PROJECTION				  ( 4 )
#define	EC_PARTICLE_LOAD						  ( 5 )
#define	EC_PARTICLE_PLAY						  ( 6 )
#define	EC_PARTICLE_PLAY_COORDINATE	  ( 7 )
#define	EC_PARTICLE_PLAY_ORTHO			  ( 8 )
#define	EC_PARTICLE_DELETE					  ( 9 )
#define	EC_EMITTER_MOVE							  ( 10 )
#define	EC_EMITTER_MOVE_COORDINATE	  ( 11 )
#define	EC_EMITTER_CIRCLE_MOVE        ( 12 )
#define	EC_EMITTER_CIRCLE_MOVE_ORTHO  ( 13 )
#define	EC_POKEMON_MOVE							  ( 14 )
#define	EC_POKEMON_CIRCLE_MOVE			  ( 15 )
#define	EC_POKEMON_SCALE						  ( 16 )
#define	EC_POKEMON_ROTATE						  ( 17 )
#define	EC_POKEMON_ALPHA						  ( 18 )
#define	EC_POKEMON_SET_MEPACHI_FLAG	  ( 19 )
#define	EC_POKEMON_SET_ANM_FLAG			  ( 20 )
#define	EC_POKEMON_PAL_FADE					  ( 21 )
#define	EC_POKEMON_VANISH					    ( 22 )
#define	EC_POKEMON_SHADOW_VANISH	    ( 23 )
#define	EC_POKEMON_DEL	              ( 24 )
#define	EC_TRAINER_SET							  ( 25 )
#define	EC_TRAINER_MOVE							  ( 26 )
#define	EC_TRAINER_ANIME_SET				  ( 27 )
#define	EC_TRAINER_DEL							  ( 28 )
#define	EC_BG_LOAD	   				        ( 29 )
#define	EC_BG_SCROLL		   				    ( 30 )
#define	EC_BG_RASTER_SCROLL		   	    ( 31 )
#define	EC_BG_PAL_ANM		   				    ( 32 )
#define	EC_BG_PRIORITY		   				  ( 33 )
#define	EC_BG_ALPHA		   				      ( 34 )
#define	EC_BG_PAL_FADE		   				  ( 35 )
#define	EC_BG_VANISH								  ( 36 )
#define	EC_OBJ_SET	   				        ( 37 )
#define	EC_OBJ_MOVE	   				        ( 38 )
#define	EC_OBJ_SCALE	   				      ( 39 )
#define	EC_OBJ_ANIME_SET	   				  ( 40 )
#define	EC_OBJ_DEL	   				        ( 41 )
#define	EC_SE_PLAY									  ( 42 )
#define	EC_SE_STOP									  ( 43 )
#define	EC_SE_PAN								      ( 44 )
#define	EC_SE_EFFECT								  ( 45 )
#define	EC_EFFECT_END_WAIT					  ( 46 )
#define	EC_WAIT											  ( 47 )
#define	EC_CONTROL_MODE							  ( 48 )
#define	EC_IF						              ( 49 )
#define	EC_MCSS_POS_CHECK						  ( 50 )
#define	EC_SET_WORK						        ( 51 )
#define	EC_MIGAWARI						        ( 52 )

//終了コマンドは必ず一番下になるようにする
#define	EC_SEQ_END									  ( 53 )

#ifndef __C_NO_DEF_

//勝手にパディングを入れられないようにする
	.option	alignment off

//命令マクロ定義
//======================================================================
/**
 * @brief		カメラ移動
 *
 * #param_num	5
 * @param	type			カメラ移動タイプ
 * @param	move_pos	移動先
 * @param	frame			カメラタイプが追従のとき、何フレームで移動先に到達するかを指定
 * @param	wait			移動ウエイト
 * @param	brake			ブレーキをかけはじめるフレームを指定
 *
 * #param	COMBOBOX_TEXT	追従	ダイレクト
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_MOVE_INTERPOLATION	BTLEFF_CAMERA_MOVE_DIRECT
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	初期位置	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_POS_ATTACK BTLEFF_CAMERA_POS_ATTACK_PAIR	BTLEFF_CAMERA_POS_DEFENCE BTLEFF_CAMERA_POS_DEFENCE_PAIR	BTLEFF_CAMERA_POS_INIT	BTLEFF_CAMERA_POS_AA	BTLEFF_CAMERA_POS_BB	BTLEFF_CAMERA_POS_A	BTLEFF_CAMERA_POS_B	BTLEFF_CAMERA_POS_C	BTLEFF_CAMERA_POS_D
 * #param	VALUE_INT	移動フレーム数
 * #param	VALUE_INT	移動ウエイト
 * #param	VALUE_INT	ブレーキ開始フレーム
 */
//======================================================================
	.macro	CAMERA_MOVE	type, move_pos, frame, wait, brake
	.short	EC_CAMERA_MOVE
	.long		\type
	.long		\move_pos
	.long		\frame
	.long		\wait
	.long		\brake
	.endm

//======================================================================
/**
 * @brief		カメラ移動（座標指定）
 *
 * #param_num	6
 * @param	type			カメラ移動タイプ
 * @param	move_pos	移動先位置
 * @param	move_tar	移動先ターゲット
 * @param	frame			カメラタイプが追従のとき、何フレームで移動先に到達するかを指定
 * @param	wait			移動ウエイト
 * @param	brake			ブレーキをかけはじめるフレームを指定
 *
 * #param	COMBOBOX_TEXT	追従	追従（相対指定）	ダイレクト
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_MOVE_INTERPOLATION	BTLEFF_CAMERA_MOVE_INTERPOLATION_RELATIVITY	BTLEFF_CAMERA_MOVE_DIRECT
 * #param	VALUE_VECFX32	CamPosX	CamPosY	CamPosZ
 * #param	VALUE_VECFX32	CamTarX	CamTarY	CamTarZ
 * #param	VALUE_INT	移動フレーム数
 * #param	VALUE_INT	移動ウエイト
 * #param	VALUE_INT	ブレーキ開始フレーム
 */
//======================================================================
	.macro	CAMERA_MOVE_COODINATE	type, pos_x, pos_y, pos_z, tar_x, tar_y, tar_z, frame, wait, brake
	.short	EC_CAMERA_MOVE_COORDINATE
	.long		\type
	.long		\pos_x
	.long		\pos_y
	.long		\pos_z
	.long		\tar_x
	.long		\tar_y
	.long		\tar_z
	.long		\frame
	.long		\wait
	.long		\brake
	.endm

//======================================================================
/**
 * @brief		カメラ移動（角度指定）
 *
 * #param_num	6
 * @param	type				カメラ移動タイプ
 * @param	angle_phi		移動先φ（追従のときは、現在値からの相対）
 * @param	angle_theta	移動先θ（追従のときは、現在値からの相対）
 * @param	frame				カメラタイプが追従のとき、何フレームで移動先に到達するかを指定
 * @param	wait				移動ウエイト
 * @param	brake				ブレーキをかけはじめるフレームを指定
 *
 * #param	COMBOBOX_TEXT	追従	ダイレクト
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_MOVE_INTERPOLATION	BTLEFF_CAMERA_MOVE_DIRECT
 * #param	VALUE_INT	移動先φ
 * #param	VALUE_INT	移動先θ
 * #param	VALUE_INT	移動フレーム数
 * #param	VALUE_INT	移動ウエイト
 * #param	VALUE_INT	ブレーキ開始フレーム
 */
//======================================================================
	.macro	CAMERA_MOVE_ANGLE	type, angle_phi, angle_theta, frame, wait, brake
	.short	EC_CAMERA_MOVE_ANGLE
	.long		\type
	.long		\angle_phi
	.long		\angle_theta
	.long		\frame
	.long		\wait
	.long		\brake
	.endm

//======================================================================
/**
 * @brief		カメラゆれ
 *
 * #param_num	6
 * @param	dir     ゆれ方向
 * @param	value   ゆれ幅
 * @param	offset	ゆれ幅オフセット
 * @param	frame   １ゆれにかかるフレーム
 * @param	wait    １フレーム毎のウエイト
 * @param	count   ゆれ回数
 *
 * #param	COMBOBOX_TEXT	縦ゆれ	横ゆれ
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_SHAKE_VERTICAL	BTLEFF_CAMERA_SHAKE_HORIZON
 * #param	VALUE_FX32	ゆれ幅
 * #param	VALUE_FX32	ゆれ幅オフセット（１ゆれごと）
 * #param	VALUE_INT	１ゆれにかかるフレーム
 * #param	VALUE_INT	１フレーム毎のウエイト
 * #param	VALUE_INT	ゆれ回数
 */
//======================================================================
	.macro	CAMERA_SHAKE	dir, value, offset, frame, wait, count
	.short	EC_CAMERA_SHAKE
	.long		\dir
	.long		\value
	.long		\offset
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief		射影モード
 *
 * #param_num	1
 * @param	type				射影タイプ
 *
 * #param	COMBOBOX_TEXT	正射影	透視射影
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_PROJECTION_ORTHO	BTLEFF_CAMERA_PROJECTION_PERSPECTIVE
 */
//======================================================================
	.macro	CAMERA_PROJECTION	type
	.short	EC_CAMERA_PROJECTION
	.long		\type
	.endm

//======================================================================
/**
 * @brief	パーティクルデータロード
 *
 * #param_num	1
 * @param	datID	アーカイブデータのdatID
 *
 * #param	FILE_DIALOG_WITH_ADD	.spa	パーティクル再生	パーティクル再生（座標指定）	エミッタ移動
 */
//======================================================================
	.macro	PARTICLE_LOAD	datID
	.short	EC_PARTICLE_LOAD
	.long		\datID
	.endm

//======================================================================
/**
 * @brief	パーティクル再生
 *
 * #param_num	6
 * @param	num				再生パーティクルナンバー
 * @param	index			spa内インデックスナンバー
 * @param	start_pos	パーティクル再生開始立ち位置
 * @param	dir_pos		パーティクル再生方向立ち位置
 * @param	ofs_y			パーティクル再生Y方向オフセット
 * @param	dir_angle	パーティクル再生方向Y角度
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param	COMBOBOX_TEXT	攻撃側	防御側	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	BTLEFF_PARTICLE_PLAY_POS_AA	BTLEFF_PARTICLE_PLAY_POS_BB	BTLEFF_PARTICLE_PLAY_POS_A  BTLEFF_PARTICLE_PLAY_POS_B	BTLEFF_PARTICLE_PLAY_POS_C  BTLEFF_PARTICLE_PLAY_POS_D  BTLEFF_PARTICLE_PLAY_POS_E  BTLEFF_PARTICLE_PLAY_POS_F
 * #param	COMBOBOX_TEXT	方向無し	攻撃側	防御側	攻撃側－  防御側－  POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_NONE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK_MINUS  BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS BTLEFF_PARTICLE_PLAY_POS_AA	BTLEFF_PARTICLE_PLAY_POS_BB BTLEFF_PARTICLE_PLAY_POS_A	BTLEFF_PARTICLE_PLAY_POS_B	BTLEFF_PARTICLE_PLAY_POS_C	BTLEFF_PARTICLE_PLAY_POS_D BTLEFF_PARTICLE_PLAY_POS_E BTLEFF_PARTICLE_PLAY_POS_F
 * #param	VALUE_FX32	パーティクル再生Y方向オフセット
 * #param	VALUE_FX32	パーティクル再生方向Y角度
 */
//======================================================================
	.macro	PARTICLE_PLAY	num, index, start_pos, dir_pos, ofs_y, dir_angle
	.short	EC_PARTICLE_PLAY
	.long		\num
	.long		\index
	.long		\start_pos
	.long		\dir_pos
	.long		\ofs_y
	.long		\dir_angle
	.endm

//======================================================================
/**
 * @brief	パーティクル再生（座標指定）
 *
 * #param_num	6
 * @param	num				再生パーティクルナンバー
 * @param	index			spa内インデックスナンバー
 * @param	start_pos	パーティクル再生開始位置
 * @param	dir_pos		パーティクル再生方向位置
 * @param	ofs_y			パーティクル再生Y方向オフセット
 * @param	dir_angle	パーティクル再生方向Y角度
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param	VALUE_VECFX32	POS_X	POS_Y	POS_Z
 * #param	VALUE_VECFX32	DIR_X	DIR_Y	DIR_Z
 * #param	VALUE_FX32	パーティクル再生Y方向オフセット
 * #param	VALUE_FX32	パーティクル再生方向Y角度
 */
//======================================================================
	.macro	PARTICLE_PLAY_COORDINATE	num, index, start_pos_x, start_pos_y, start_pos_z, dir_pos_x, dir_pos_y, dir_pos_z, ofs_y, dir_angle
	.short	EC_PARTICLE_PLAY_COORDINATE
	.long		\num
	.long		\index
	.long		\start_pos_x
	.long		\start_pos_y
	.long		\start_pos_z
	.long		\dir_pos_x
	.long		\dir_pos_y
	.long		\dir_pos_z
	.long		\ofs_y
	.long		\dir_angle
	.endm

//======================================================================
/**
 * @brief	パーティクル再生（正射影）
 *
 * #param_num	7
 * @param	num				再生パーティクルナンバー
 * @param	index			spa内インデックスナンバー
 * @param	start_pos	パーティクル再生開始立ち位置
 * @param	dir	      パーティクルの向き
 * @param	ofs			  パーティクル再生オフセット
 * @param	param			奥側で再生するときの補正値
 * @param	speed			奥側で再生するときの移動速度補正値
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param	COMBOBOX_TEXT	攻撃側	防御側	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	BTLEFF_PARTICLE_PLAY_POS_AA	BTLEFF_PARTICLE_PLAY_POS_BB	BTLEFF_PARTICLE_PLAY_POS_A  BTLEFF_PARTICLE_PLAY_POS_B	BTLEFF_PARTICLE_PLAY_POS_C  BTLEFF_PARTICLE_PLAY_POS_D  BTLEFF_PARTICLE_PLAY_POS_E  BTLEFF_PARTICLE_PLAY_POS_F
 * #param	COMBOBOX_TEXT	方向無し	攻撃側	防御側	攻撃側－  防御側－  POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_NONE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK_MINUS  BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS BTLEFF_PARTICLE_PLAY_POS_AA	BTLEFF_PARTICLE_PLAY_POS_BB BTLEFF_PARTICLE_PLAY_POS_A	BTLEFF_PARTICLE_PLAY_POS_B	BTLEFF_PARTICLE_PLAY_POS_C	BTLEFF_PARTICLE_PLAY_POS_D BTLEFF_PARTICLE_PLAY_POS_E BTLEFF_PARTICLE_PLAY_POS_F
 * #param	VALUE_VECFX32 ofs_x ofs_y ofs_z
 * #param	VALUE_VECFX32 エミッタ半径 エミッタ寿命 エミッタスケール
 * #param VALUE_INIT  2:1:2
 * #param	VALUE_FX32 エミッタ速度
 * #param VALUE_INIT  1
 */
//======================================================================
	.macro	PARTICLE_PLAY_ORTHO	num, index, start_pos, dir, ofs_x, ofs_y, ofs_z, radius, length, scale, speed
	.short	EC_PARTICLE_PLAY_ORTHO
	.long		\num
	.long		\index
	.long		\start_pos
	.long		\dir
	.long		\ofs_x
	.long		\ofs_y
	.long		\ofs_z
	.long		\radius
	.long		\length
	.long		\scale
	.long		\speed
	.endm

//======================================================================
/**
 * @brief	パーティクルデータ削除
 *
 * #param_num	1
 * @param	num			削除するパーティクルナンバー
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 */
//======================================================================
	.macro	PARTICLE_DELETE	num
	.short	EC_PARTICLE_DELETE
	.long		\num
	.endm

//======================================================================
/**
 * @brief	エミッタ移動
 *
 * #param_num	6
 * @param	num					再生パーティクルナンバー
 * @param	index				spa内インデックスナンバー
 * @param	move_type		移動タイプ（直線、放物線）
 * @param	start_pos		移動開始立ち位置
 * @param	end_pos			移動終了立ち位置
 * @param	move_param	立ち位置Y方向オフセット(ofs_y)	移動フレーム(move_frame)	放物線頂点（放物線時のみ）(top)
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param	COMBOBOX_TEXT	直線	放物線  放物線（半分）
 * #param	COMBOBOX_VALUE	BTLEFF_EMITTER_MOVE_STRAIGHT	BTLEFF_EMITTER_MOVE_CURVE BTLEFF_EMITTER_MOVE_CURVE_HALF
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE
 * #param	VALUE_VECFX32	立ち位置Y方向オフセット	移動フレーム	放物線頂点（放物線時のみ）
 */
//======================================================================
	.macro	EMITTER_MOVE	num, index, move_type, start_pos, end_pos, ofs_y, move_frame, top
	.short	EC_EMITTER_MOVE
	.long		\num
	.long		\index
	.long		\move_type
	.long		\start_pos
	.long		\end_pos
	.long		\ofs_y
	.long		\move_frame
	.long		\top
	.endm

//======================================================================
/**
 * @brief	エミッタ移動（座標指定）
 *
 * #param_num	6
 * @param	num					再生パーティクルナンバー
 * @param	index				spa内インデックスナンバー
 * @param	move_type		移動タイプ（直線、放物線）
 * @param	start_pos		移動開始座標
 * @param	end_pos			移動終了立ち位置
 * @param	move_param	立ち位置Y方向オフセット(ofs_y)	移動フレーム(move_frame)	放物線頂点（放物線時のみ）(top)
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param	COMBOBOX_TEXT	直線	放物線  放物線（半分）
 * #param	COMBOBOX_VALUE	BTLEFF_EMITTER_MOVE_STRAIGHT	BTLEFF_EMITTER_MOVE_CURVE BTLEFF_EMITTER_MOVE_CURVE_HALF
 * #param	VALUE_VECFX32	X座標	Y座標	Z座標
 * #param	COMBOBOX_TEXT	攻撃側	防御側 POS_AA POS_BB POS_A POS_B POS_C POS_D  POS_E POS_F
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	BTLEFF_PARTICLE_PLAY_POS_AA	BTLEFF_PARTICLE_PLAY_POS_BB	BTLEFF_PARTICLE_PLAY_POS_A	BTLEFF_PARTICLE_PLAY_POS_B	BTLEFF_PARTICLE_PLAY_POS_C	BTLEFF_PARTICLE_PLAY_POS_D BTLEFF_PARTICLE_PLAY_POS_E BTLEFF_PARTICLE_PLAY_POS_F
 * #param	VALUE_VECFX32	立ち位置Y方向オフセット	移動フレーム	放物線頂点（放物線時のみ）
 */
//======================================================================
	.macro	EMITTER_MOVE_COORDINATE	num, index, move_type, start_pos_x, start_pos_y, start_pos_z, end_pos, ofs_y, move_frame, top
	.short	EC_EMITTER_MOVE_COORDINATE
	.long		\num
	.long		\index
	.long		\move_type
	.long		\start_pos_x
	.long		\start_pos_y
	.long		\start_pos_z
	.long		\end_pos
	.long		\ofs_y
	.long		\move_frame
	.long		\top
	.endm

//======================================================================
/**
 * @brief	エミッタ円移動
 *
 * #param_num	6
 * @param	num					      再生パーティクルナンバー
 * @param	index				      spa内インデックスナンバー
 * @param center_pos        円運動の中心
 * @param param             横方向半径:縦方向半径:Y方向オフセット
 * @param	rotate_param			回転フレーム数（1回転何フレームでするか）:移動ウエイト:回転カウント
 * @param	rotate_after_wait 1回転したあとのウエイト（2回転以上で意味のある値です）
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param COMBOBOX_TEXT 攻撃側左  攻撃側右  防御側左  防御側右  画面中央左  画面中央右
 * #param COMBOBOX_VALUE  BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_L BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_R BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_L BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_R BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_L BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_R
 * #param VALUE_VECFX32  横方向半径 縦方向半径 Y方向オフセット
 * #param	VALUE_VECINT		回転フレーム数  移動ウエイト  回転カウント
 * #param	VALUE_INT 1回転したあとのウエイト（2回転以上で意味のある値です）
 */
//======================================================================
  .macro  EMITTER_CIRCLE_MOVE num, index, center_pos, radius_h, radius_v, offset_y, frame, wait, count, rotate_after_wait
  .short  EC_EMITTER_CIRCLE_MOVE
  .long   \num
  .long   \index
  .long   \center_pos
  .long   \radius_h
  .long   \radius_v
  .long   \offset_y
  .long   \frame
  .long   \wait
  .long   \count
  .long   \rotate_after_wait
  .endm

//======================================================================
/**
 * @brief	エミッタ円移動（正射影）
 *
 * #param_num	6
 * @param	num					      再生パーティクルナンバー
 * @param	index				      spa内インデックスナンバー
 * @param center_pos        円運動の中心
 * @param param             横方向半径:縦方向半径:Y方向オフセット
 * @param	rotate_param			回転フレーム数（1回転何フレームでするか）:移動ウエイト:回転カウント
 * @param	rotate_after_wait 1回転したあとのウエイト（2回転以上で意味のある値です）
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_HEADER
 * #param COMBOBOX_TEXT 攻撃側左  攻撃側右  防御側左  防御側右  画面中央左  画面中央右
 * #param COMBOBOX_VALUE  BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_L BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_R BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_L BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_R BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_L BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_R
 * #param VALUE_VECFX32  横方向半径 縦方向半径 Y方向オフセット
 * #param	VALUE_VECINT		回転フレーム数  移動ウエイト  回転カウント
 * #param	VALUE_INT 1回転したあとのウエイト（2回転以上で意味のある値です）
 */
//======================================================================
  .macro  EMITTER_CIRCLE_MOVE_ORTHO num, index, center_pos, radius_h, radius_v, offset_y, frame, wait, count, rotate_after_wait
  .short  EC_EMITTER_CIRCLE_MOVE_ORTHO
  .long   \num
  .long   \index
  .long   \center_pos
  .long   \radius_h
  .long   \radius_v
  .long   \offset_y
  .long   \frame
  .long   \wait
  .long   \count
  .long   \rotate_after_wait
  .endm

//======================================================================
/**
 * @brief	ポケモン移動
 *
 * #param_num	7
 * @param	pos					移動させるポケモンの立ち位置
 * @param	type				移動タイプ
 * @param	move_pos_x	移動先X座標
 * @param	move_pos_y	移動先Y座標
 * @param	frame				移動フレーム数（目的地まで何フレームで到達するか）
 * @param	wait				移動ウエイト
 * @param	count				往復カウント（移動タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A POS_B POS_C POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK  BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR  BTLEFF_POKEMON_POS_AA BTLEFF_POKEMON_POS_BB BTLEFF_POKEMON_POS_A  BTLEFF_POKEMON_POS_B  BTLEFF_POKEMON_POS_C  BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング  初期位置
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_MOVE_DIRECT	BTLEFF_POKEMON_MOVE_INTERPOLATION	BTLEFF_POKEMON_MOVE_ROUNDTRIP	BTLEFF_POKEMON_MOVE_ROUNDTRIP_LONG  BTLEFF_POKEMON_MOVE_INIT
 * #param	VALUE_FX32	移動先X座標
 * #param	VALUE_FX32	移動先Y座標
 * #param	VALUE_INT		移動フレーム数
 * #param	VALUE_INT		移動ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	POKEMON_MOVE	pos, type, move_pos_x, move_pos_y, frame, wait, count
	.short	EC_POKEMON_MOVE
	.long		\pos
	.long		\type
	.long		\move_pos_x
	.long		\move_pos_y
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	ポケモン円移動
 *
 * #param_num	7
 * @param	pos				      	円移動させるポケモンの立ち位置
 * @param	axis			      	回転軸
 * @param	shift			      	回転シフト
 * @param	radius_h	        横方向半径
 * @param	radius_v	        縦方向半径
 * @param	rotate_param			回転フレーム数（1回転何フレームでするか）:移動ウエイト:回転カウント
 * @param	rotate_after_wait 1回転したあとのウエイト（2回転以上で意味のある値です）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	Ｘ軸左  Ｘ軸右  Ｙ軸左  Ｙ軸右  Ｚ軸左  Ｚ軸右
 * #param	COMBOBOX_VALUE	BTLEFF_AXIS_X_L BTLEFF_AXIS_X_R BTLEFF_AXIS_Y_L BTLEFF_AXIS_Y_R BTLEFF_AXIS_Z_L BTLEFF_AXIS_Z_R
 * #param	COMBOBOX_TEXT シフトＨ＋  シフトＨ－  シフトＶ＋  シフトＶ－
 * #param	COMBOBOX_VALUE	BTLEFF_SHIFT_H_P BTLEFF_SHIFT_H_M BTLEFF_SHIFT_V_P BTLEFF_SHIFT_V_M
 * #param	VALUE_FX32	横方向半径
 * #param	VALUE_FX32	縦方向半径
 * #param	VALUE_VECFX32		回転フレーム数  移動ウエイト  回転カウント
 * #param	VALUE_INT 1回転したあとのウエイト（2回転以上で意味のある値です）
 */
//======================================================================
	.macro	POKEMON_CIRCLE_MOVE	pos, axis, shift, radius_h, radius_v, frame, rotate_wait, count, rotate_after_wait
	.short	EC_POKEMON_CIRCLE_MOVE
	.long		\pos
	.long		\axis
	.long		\shift
	.long		\radius_h
	.long		\radius_v
	.long		\frame
	.long		\rotate_wait
	.long		\count
	.long		\rotate_after_wait
	.endm

//======================================================================
/**
 * @brief	ポケモン拡縮
 *
 * #param_num	7
 * @param	pos			拡縮させるポケモンの立ち位置
 * @param	type		拡縮タイプ
 * @param	scale_x	X方向拡縮率
 * @param	scale_y	Y方向拡縮率
 * @param	frame		拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param	wait		拡縮ウエイト
 * @param	count		往復カウント（拡縮タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A POS_B POS_C POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK  BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR  BTLEFF_POKEMON_POS_AA BTLEFF_POKEMON_POS_BB BTLEFF_POKEMON_POS_A  BTLEFF_POKEMON_POS_B  BTLEFF_POKEMON_POS_C  BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SCALE_DIRECT	BTLEFF_POKEMON_SCALE_INTERPOLATION	BTLEFF_POKEMON_SCALE_ROUNDTRIP	BTLEFF_POKEMON_SCALE_ROUNDTRIP_LONG
 * #param	VALUE_FX32	X方向拡縮率
 * #param	VALUE_FX32	Y方向拡縮率
 * #param	VALUE_INT		拡縮フレーム数
 * #param	VALUE_INT		拡縮ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	POKEMON_SCALE	pos, type, scale_x, scale_y, frame, wait, count
	.short	EC_POKEMON_SCALE
	.long		\pos
	.long		\type
	.long		\scale_x
	.long		\scale_y
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	ポケモン回転
 *
 * #param_num	6
 * @param	pos			回転させるポケモンの立ち位置
 * @param	type		回転タイプ
 * @param	rotate	回転角度
 * @param	frame		回転フレーム数（設定した回転値まで何フレームで到達するか）
 * @param	wait		回転ウエイト
 * @param	count		往復カウント（回転タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_ROTATE_DIRECT	BTLEFF_POKEMON_ROTATE_INTERPOLATION	BTLEFF_POKEMON_ROTATE_ROUNDTRIP	BTLEFF_POKEMON_ROTATE_ROUNDTRIP_LONG
 * #param	VALUE_ANGLE	回転角度
 * #param	VALUE_INT		回転フレーム数（設定した回転値まで何フレームで到達するか）
 * #param	VALUE_INT		回転ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	POKEMON_ROTATE	pos, type, rotate, frame, wait, count
	.short	EC_POKEMON_ROTATE
	.long		\pos
	.long		\type
	.long		\rotate
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	ポケモンα値
 *
 * #param_num	6
 * @param	pos			α値セットするポケモンの立ち位置
 * @param	type		α値タイプ
 * @param	alpha	  α値
 * @param	frame		フレーム数（設定したα値まで何フレームで到達するか）
 * @param	wait		ウエイト
 * @param	count		往復カウント（α値タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A POS_B POS_C POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK  BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR  BTLEFF_POKEMON_POS_AA BTLEFF_POKEMON_POS_BB BTLEFF_POKEMON_POS_A  BTLEFF_POKEMON_POS_B  BTLEFF_POKEMON_POS_C  BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_ALPHA_DIRECT	BTLEFF_POKEMON_ALPHA_INTERPOLATION	BTLEFF_POKEMON_ALPHA_ROUNDTRIP	BTLEFF_POKEMON_ALPHA_ROUNDTRIP_LONG
 * #param	VALUE_INT	  α値
 * #param	VALUE_INT		フレーム数（設定したα値まで何フレームで到達するか）
 * #param	VALUE_INT		ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	POKEMON_ALPHA	pos, type, alpha, frame, wait, count
	.short	EC_POKEMON_ALPHA
	.long		\pos
	.long		\type
	.long		\alpha
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	ポケモンメパチ
 *
 * #param_num	4
 * @param	pos			メパチさせるポケモンの立ち位置
 * @param	type		メパチタイプ
 * @param	wait		まばたきウエイト（メパチタイプがまばたきのときだけ有効）
 * @param	count		まばたきカウント（メパチタイプがまばたきのときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	閉じる	開ける	まばたき
 * #param	COMBOBOX_VALUE	BTLEFF_MEPACHI_ON	BTLEFF_MEPACHI_OFF	BTLEFF_MEPACHI_MABATAKI
 * #param	VALUE_INT	まばたきウエイト（まばたき時有効）
 * #param	VALUE_INT	まばたきカウント（まばたき時有効）
 */
//======================================================================
	.macro	POKEMON_SET_MEPACHI_FLAG	pos, type, wait, count
	.short	EC_POKEMON_SET_MEPACHI_FLAG
	.long		\pos
	.long		\type
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	ポケモンアニメ
 *
 * #param_num	2
 * @param	pos			アニメ操作するポケモンの立ち位置
 * @param	flag		アニメフラグ
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR	BTLEFF_POKEMON_POS_AA	BTLEFF_POKEMON_POS_BB	BTLEFF_POKEMON_POS_A	BTLEFF_POKEMON_POS_B	BTLEFF_POKEMON_POS_C	BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A BTLEFF_TRAINER_POS_B BTLEFF_TRAINER_POS_C BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	アニメストップ	アニメスタート
 * #param	COMBOBOX_VALUE	BTLEFF_ANM_STOP	BTLEFF_ANM_START
 */
//======================================================================
	.macro	POKEMON_SET_ANM_FLAG	pos, flag
	.short	EC_POKEMON_SET_ANM_FLAG
	.long		\pos
	.long		\flag
	.endm

//======================================================================
/**
 * @brief	ポケモンパレットフェード
 *
 * #param_num	5
 * @param	pos				パレットフェードするポケモンの立ち位置
 * @param	start_evy	START_EVY値
 * @param	end_evy		END_EVY値
 * @param	wait			ウエイト
 * @param	rgb				開始or終了時の色
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR	BTLEFF_POKEMON_POS_AA	BTLEFF_POKEMON_POS_BB	BTLEFF_POKEMON_POS_A	BTLEFF_POKEMON_POS_B	BTLEFF_POKEMON_POS_C	BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A BTLEFF_TRAINER_POS_B BTLEFF_TRAINER_POS_C BTLEFF_TRAINER_POS_D
 * #param	VALUE_INT	START_EVY値
 * #param	VALUE_INT	END_EVY値
 * #param	VALUE_INT	ウエイト
 * #param	VALUE_VECFX32	R値(0-31)	G値(0-31)	B値(0-31)
 */
//======================================================================
	.macro	POKEMON_PAL_FADE	pos,	start_evy,	end_evy,	wait,	r, g, b
	.short	EC_POKEMON_PAL_FADE
	.long		\pos
	.long		\start_evy
	.long		\end_evy
	.long		\wait
	.long		( ( ( \b >> BTLEFF_FX32_SHIFT ) & 0x1f ) << 10 ) | ( ( ( \g >> BTLEFF_FX32_SHIFT ) & 0x1f ) << 5 ) | ( ( \r >> BTLEFF_FX32_SHIFT ) & 0x1f )
	.endm

//======================================================================
/**
 * @brief	ポケモンバニッシュ
 *
 * #param_num	2
 * @param	pos			バニッシュ操作するポケモンの立ち位置
 * @param	flag		バニッシュフラグ
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR	BTLEFF_POKEMON_POS_AA	BTLEFF_POKEMON_POS_BB	BTLEFF_POKEMON_POS_A	BTLEFF_POKEMON_POS_B	BTLEFF_POKEMON_POS_C	BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A BTLEFF_TRAINER_POS_B BTLEFF_TRAINER_POS_C BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	バニッシュオン	バニッシュオフ
 * #param	COMBOBOX_VALUE	BTLEFF_VANISH_ON	BTLEFF_VANISH_OFF
 */
//======================================================================
	.macro	POKEMON_VANISH	pos, flag
	.short	EC_POKEMON_VANISH
	.long		\pos
	.long		\flag
	.endm

//======================================================================
/**
 * @brief	ポケモン影バニッシュ
 *
 * #param_num	2
 * @param	pos			バニッシュ操作するポケモンの立ち位置
 * @param	flag		バニッシュフラグ
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア	防御側	防御側ペア	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR	BTLEFF_POKEMON_POS_AA	BTLEFF_POKEMON_POS_BB	BTLEFF_POKEMON_POS_A	BTLEFF_POKEMON_POS_B	BTLEFF_POKEMON_POS_C	BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A BTLEFF_TRAINER_POS_B BTLEFF_TRAINER_POS_C BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	バニッシュオン	バニッシュオフ
 * #param	COMBOBOX_VALUE	BTLEFF_VANISH_ON	BTLEFF_VANISH_OFF
 */
//======================================================================
	.macro	POKEMON_SHADOW_VANISH	pos, flag
	.short	EC_POKEMON_SHADOW_VANISH
	.long		\pos
	.long		\flag
	.endm

//======================================================================
/**
 * @brief	ポケモン削除
 *
 * #param_num	1
 * @param	index		削除するポケモンの立ち位置
 *
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK  BTLEFF_POKEMON_SIDE_DEFENCE
 */
//======================================================================
	.macro	POKEMON_DEL	pos
	.short	EC_POKEMON_DEL
	.long		\pos
	.endm


//======================================================================
/**
 * @brief	トレーナーセット
 *
 * #param_num	3
 * @param	trtype		セットするトレーナータイプ
 * @param	position	セットするトレーナーの立ち位置
 * @param	pos			  セットする座標(X:Y:Z)
 *
 * #param	VALUE_INT	トレーナータイプ
 * #param	COMBOBOX_TEXT POS_AA  POS_BB  POS_A POS_B POS_C POS_D
 * #param	COMBOBOX_VALUE  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	VALUE_VECFX32	X座標 Y座標 Z座標
 */
//======================================================================
	.macro	TRAINER_SET	index, position, pos_x, pos_y, pos_z
	.short	EC_TRAINER_SET
	.long		\index
	.long		\position
	.long		\pos_x
	.long		\pos_y
	.long		\pos_z
	.endm

//======================================================================
/**
 * @brief	トレーナー移動
 *
 * #param_num	7
 * @param	index				移動させるトレーナーの立ち位置
 * @param	type				移動タイプ
 * @param	move_pos_x	移動先X座標
 * @param	move_pos_y	移動先Y座標
 * @param	frame				移動フレーム数（目的地まで何フレームで到達するか）
 * @param	wait				移動ウエイト
 * @param	count				往復カウント（移動タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT POS_AA  POS_BB  POS_A POS_B POS_C POS_D
 * #param	COMBOBOX_VALUE  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_TRAINER_MOVE_DIRECT	BTLEFF_TRAINER_MOVE_INTERPOLATION	BTLEFF_TRAINER_MOVE_ROUNDTRIP	BTLEFF_TRAINER_MOVE_ROUNDTRIP_LONG
 * #param	VALUE_INT		移動先X座標
 * #param	VALUE_INT		移動先Y座標
 * #param	VALUE_INT		移動フレーム数
 * #param	VALUE_INT		移動ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	TRAINER_MOVE	pos, type, move_pos_x, move_pos_y, frame, wait, count
	.short	EC_TRAINER_MOVE
	.long		\pos
	.long		\type
	.long		\move_pos_x
	.long		\move_pos_y
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	トレーナーアニメセット
 *
 * #param_num	2
 * @param	pos			アニメセットするトレーナーの立ち位置
 * @param	anm_no	セットするアニメナンバー
 *
 * #param	COMBOBOX_TEXT POS_AA  POS_BB  POS_A POS_B POS_C POS_D
 * #param	COMBOBOX_VALUE  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	VALUE_INT		アニメナンバー
 */
//======================================================================
	.macro	TRAINER_ANIME_SET	pos, anm_no
	.short	EC_TRAINER_ANIME_SET
	.long		\pos
	.long		\anm_no
	.endm

//======================================================================
/**
 * @brief	トレーナー削除
 *
 * #param_num	1
 * @param	index		削除するトレーナーの立ち位置
 *
 * #param	COMBOBOX_TEXT POS_AA  POS_BB  POS_A POS_B POS_C POS_D
 * #param	COMBOBOX_VALUE  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 */
//======================================================================
	.macro	TRAINER_DEL	pos
	.short	EC_TRAINER_DEL
	.long		\pos
	.endm

//======================================================================
/**
 * @brief	BGのロード
 *
 * #param_num	1
 * @param	datID	アーカイブデータのdatID
 *
 * #param	FILE_DIALOG	.NSCR
 */
//======================================================================
	.macro	BG_LOAD	datID
	.short	EC_BG_LOAD
	.long		\datID
	.endm

//======================================================================
/**
 * @brief	BGのスクロール
 *
 * #param_num	6
 * @param	type				移動タイプ
 * @param	move_pos_x	移動先X座標
 * @param	move_pos_y	移動先Y座標
 * @param	frame				移動フレーム数（目的地まで何フレームで到達するか）
 * @param	wait				移動ウエイト
 * @param	count				往復カウント（移動タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	常に  ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_BG_SCROLL_EVERY  BTLEFF_BG_SCROLL_DIRECT	BTLEFF_BG_SCROLL_INTERPOLATION	BTLEFF_BG_SCROLL_ROUNDTRIP	BTLEFF_BG_SCROLL_ROUNDTRIP_LONG
 * #param	VALUE_INT		移動先X座標
 * #param	VALUE_INT		移動先Y座標
 * #param	VALUE_INT		移動フレーム数
 * #param	VALUE_INT		移動ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	BG_SCROLL	type, move_pos_x, move_pos_y, frame, wait, count
	.short	EC_BG_SCROLL
	.long   \type
  .long   \move_pos_x
  .long   \move_pos_y
  .long   \frame
  .long   \wait
  .long   \count
	.endm

//======================================================================
/**
 * @brief	BGのラスタースクロール
 *
 * #param_num	4
 * @param	type		ラスタータイプ
 * @param	radius  サインカーブの半径
 * @param	line	  サインカーブ1周を何ラインにするか
 * @param	wait		ウエイト
 *
 * #param	COMBOBOX_TEXT 横  縦
 * #param	COMBOBOX_VALUE  BTLEFF_BG_RASTER_SCROLL_H BTLEFF_BG_RASTER_SCROLL_V
 * #param	VALUE_INT サインカーブの半径
 * #param	VALUE_INT サインカーブ1周を何ラインにするか
 * #param	VALUE_INT ウエイト
 */
//======================================================================
  .macro  BG_RASTER_SCROLL  type, radius, line, wait
  .short  EC_BG_RASTER_SCROLL
  .long   \type
  .long   \radius
  .long   \line
  .long   \wait
  .endm

//======================================================================
/**
 * @brief	BGのパレットアニメ
 *
 * #param_num	5
 * @param	datID     アニメ用パレットのアーカイブデータID
 * @param	trans_pal 送り先パレット
 * @param	col_count 送るカラー数
 * @param	offset		送り元パレットの更新オフセット
 * @param	wait		  ウエイト
 *
 * #param	VALUE_INT アニメ用パレットのアーカイブデータID
 * #param	VALUE_INT 送り先パレット
 * #param	VALUE_INT 送るカラー数
 * #param	VALUE_INT 送り元パレットの更新オフセット
 * #param	VALUE_INT ウエイト
 */
//======================================================================
  .macro  BG_PAL_ANM  datID, trans_pal, col_count, offset, wait
  .short  EC_BG_PAL_ANM
  .long   \datID
  .long   \trans_pal
  .long   \col_count
  .long   \offset
  .long   \wait
  .endm

//======================================================================
/**
 * @brief	BGのプライオリティ
 *
 * #param_num	1
 * @param	pri	BGのプライオリティ
 *
 * #param	COMBOBOX_TEXT 前面  背面
 * #param	COMBOBOX_VALUE BTLEFF_BG_PRI_FRONT  BTLEFF_BG_PRI_BACK
 */
//======================================================================
  .macro  BG_PRIORITY pri
  .short  EC_BG_PRIORITY
  .long   \pri
  .endm

//======================================================================
/**
 * @brief	BGα値
 *
 * #param_num	6
 * @param	bg_num	α値セットするBGの種類
 * @param	type		α値タイプ
 * @param	alpha	  α値
 * @param	frame		フレーム数（設定したα値まで何フレームで到達するか）
 * @param	wait		ウエイト
 * @param	count		往復カウント（α値タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT お盆  フィールド  お盆＋フィールド  エフェクト面  すべて
 * #param	COMBOBOX_VALUE  BTLEFF_PAL_FADE_STAGE BTLEFF_PAL_FADE_FIELD BTLEFF_PAL_FADE_3D  BTLEFF_PAL_FADE_EFFECT BTLEFF_PAL_FADE_ALL
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_BG_ALPHA_DIRECT	BTLEFF_BG_ALPHA_INTERPOLATION	BTLEFF_BG_ALPHA_ROUNDTRIP	BTLEFF_BG_ALPHA_ROUNDTRIP_LONG
 * #param	VALUE_INT	  α値
 * #param	VALUE_INT		フレーム数（設定したα値まで何フレームで到達するか）
 * #param	VALUE_INT		ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
  .macro  BG_ALPHA  bg_num, type, alpha, frame, wait, count
  .short  EC_BG_ALPHA
  .long   \bg_num
  .long   \type
  .long   \alpha
  .long   \frame
  .long   \wait
  .long   \count
  .endm

//======================================================================
/**
 * @brief	BGのパレットフェード
 *
 * #param_num	5
 * @param	bg_num    操作する背景
 * @param	start_evy	START_EVY値
 * @param	end_evy		END_EVY値
 * @param	wait			ウエイト
 * @param	rgb				開始or終了時の色
 *
 * #param	COMBOBOX_TEXT お盆  フィールド  お盆＋フィールド  エフェクト面  すべて
 * #param	COMBOBOX_VALUE  BTLEFF_PAL_FADE_STAGE BTLEFF_PAL_FADE_FIELD BTLEFF_PAL_FADE_3D  BTLEFF_PAL_FADE_EFFECT BTLEFF_PAL_FADE_ALL
 * #param	VALUE_INT	START_EVY値
 * #param	VALUE_INT	END_EVY値
 * #param	VALUE_INT	ウエイト
 * #param	VALUE_VECFX32	R値(0-31)	G値(0-31)	B値(0-31)
 */
//======================================================================
	.macro	BG_PAL_FADE bg_num, start_evy, end_evy, wait, r, g, b
	.short	EC_BG_PAL_FADE
	.long		\bg_num
	.long		\start_evy
	.long		\end_evy
	.long		\wait
	.long		( ( ( \b >> BTLEFF_FX32_SHIFT ) & 0x1f ) << 10 ) | ( ( ( \g >> BTLEFF_FX32_SHIFT ) & 0x1f ) << 5 ) | ( ( \r >> BTLEFF_FX32_SHIFT ) & 0x1f )
	.endm

//======================================================================
/**
 * @brief	BGの表示/非表示
 *
 * #param_num	2
 * @param	bg_num	操作する背景
 * @param	sw			表示/非表示を指定
 *
 * #param	COMBOBOX_TEXT	お盆  フィールド  エフェクト
 * #param	COMBOBOX_VALUE  BTLEFF_STAGE  BTLEFF_FIELD  BTLEFF_EFFECT
 * #param	COMBOBOX_TEXT 非表示  表示
 * #param	COMBOBOX_VALUE  BTLEFF_VANISH_ON BTLEFF_VANISH_OFF
 */
//======================================================================
	.macro	BG_VISIBLE	bg_num, sw
	.short	EC_BG_VANISH
	.long		\bg_num
	.long		\sw
	.endm

//======================================================================
/**
 * @brief	OBJのセット
 *
 * #param_num	7
 * @param	index   管理用インデックス（0-3）
 * @param	datID   アーカイブデータのdatID
 * @param pos     表示する座標
 * @param ofs_x   オフセットX座標
 * @param ofs_y   オフセットY座標
 * @param scale_x オフセットX座標
 * @param scale_y オフセットY座標
 *
 * #param	VALUE_INT インデックス
 * #param	FILE_DIALOG .NCGR
 * #param	COMBOBOX_TEXT	攻撃側	防御側	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK  BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_POS_AA	BTLEFF_POKEMON_POS_BB	BTLEFF_POKEMON_POS_A	BTLEFF_POKEMON_POS_B	BTLEFF_POKEMON_POS_C	BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A BTLEFF_TRAINER_POS_B BTLEFF_TRAINER_POS_C BTLEFF_TRAINER_POS_D
 * #param VALUE_FX32  オフセットX座標
 * #param VALUE_FX32  オフセットY座標
 * #param VALUE_FX32  スケールX値
 * #param VALUE_FX32  スケールY値
 */
//======================================================================
  .macro  OBJ_SET index, datID, pos, ofs_x, ofs_y, scale_x, scale_y
  .short  EC_OBJ_SET
  .long   \index
  .long   \datID
  .long   \pos
  .long   \ofs_x
  .long   \ofs_y
  .long   \scale_x
  .long   \scale_y
  .endm

//======================================================================
/**
 * @brief	OBJ移動
 *
 * #param_num	7
 * @param	index				移動させるOBJの管理インデックス
 * @param	type				移動タイプ
 * @param	move_pos_x	移動先X座標
 * @param	move_pos_y	移動先Y座標
 * @param	frame				移動フレーム数（目的地まで何フレームで到達するか）
 * @param	wait				移動ウエイト
 * @param	count				往復カウント（移動タイプが往復のときだけ有効）
 *
 * #param	VALUE_INT		OBJの管理インデックス
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_OBJ_MOVE_DIRECT	BTLEFF_OBJ_MOVE_INTERPOLATION	BTLEFF_OBJ_MOVE_ROUNDTRIP	BTLEFF_OBJ_MOVE_ROUNDTRIP_LONG
 * #param	VALUE_INT   移動先X座標
 * #param	VALUE_INT   移動先Y座標
 * #param	VALUE_INT		移動フレーム数
 * #param	VALUE_INT		移動ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	OBJ_MOVE	index, type, move_pos_x, move_pos_y, frame, wait, count
	.short	EC_OBJ_MOVE
	.long		\index
	.long		\type
	.long		\move_pos_x
	.long		\move_pos_y
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	OBJ拡縮
 *
 * #param_num	7
 * @param	index		拡縮させるOBJの管理インデックス
 * @param	type		拡縮タイプ
 * @param	scale_x	X方向拡縮率
 * @param	scale_y	Y方向拡縮率
 * @param	frame		拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param	wait		拡縮ウエイト
 * @param	count		往復カウント（拡縮タイプが往復のときだけ有効）
 *
 * #param	VALUE_INT		OBJの管理インデックス
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_OBJ_SCALE_DIRECT	BTLEFF_OBJ_SCALE_INTERPOLATION	BTLEFF_OBJ_SCALE_ROUNDTRIP	BTLEFF_OBJ_SCALE_ROUNDTRIP_LONG
 * #param	VALUE_FX32	X方向拡縮率
 * #param	VALUE_FX32	Y方向拡縮率
 * #param	VALUE_INT		拡縮フレーム数
 * #param	VALUE_INT		拡縮ウエイト
 * #param	VALUE_INT		往復カウント（往復時有効）
 */
//======================================================================
	.macro	OBJ_SCALE	index, type, scale_x, scale_y, frame, wait, count
	.short	EC_OBJ_SCALE
	.long		\index
	.long		\type
	.long		\scale_x
	.long		\scale_y
	.long		\frame
	.long		\wait
	.long		\count
	.endm

//======================================================================
/**
 * @brief	OBJアニメセット
 *
 * #param_num	2
 * @param	index		アニメセットするOBJの管理インデックス
 * @param	anm_no	セットするアニメナンバー
 *
 * #param	VALUE_INT		OBJの管理インデックス
 * #param	VALUE_INT		アニメナンバー
 */
//======================================================================
	.macro	OBJ_ANIME_SET	pos, anm_no
	.short	EC_OBJ_ANIME_SET
	.long		\pos
	.long		\anm_no
	.endm

//======================================================================
/**
 * @brief	OBJ削除
 *
 * #param_num	1
 * @param	index		削除するOBJの管理インデックス
 *
 * #param	VALUE_INT		OBJの管理インデックス
 */
//======================================================================
	.macro	OBJ_DEL	pos
	.short	EC_OBJ_DEL
	.long		\pos
	.endm

//======================================================================
/**
 * @brief	SE再生
 *
 * #param_num	7
 * @param	se_no	      再生するSEナンバー
 * @param player      再生するPlayerNo
 * @param wait        再生までのウエイト
 * @param pitch       再生ピッチ
 * @param vol         再生ボリューム
 * @param mod_depth   再生モジュレーションデプス
 * @param mod_speed   再生モジュレーションスピード
 *
 * #param	VALUE_INT   再生するSEナンバー
 * #param COMBOBOX_TEXT デフォルト  SE1 SE2 PSG SYSTEM
 * #param COMBOBOX_VALUE BTLEFF_SEPLAY_DEFAULT  BTLEFF_SEPLAY_SE1 BTLEFF_SEPLAY_SE2 BTLEFF_SEPLAY_PSG BTLEFF_SEPLAY_SYSTEM
 * #param VALUE_INT   再生までのウエイト
 * #param VALUE_INT   再生ピッチ
 * #param VALUE_INT   再生ボリューム
 * #param VALUE_INIT  127
 * #param VALUE_INT   再生モジュレーションデプス
 * #param VALUE_INT   再生モジュレーションスピード
 */
//======================================================================
	.macro	SE_PLAY	se_no, player, wait, pitch, vol, mod_depth, mod_speed
	.short	EC_SE_PLAY
	.long		\se_no
	.long   \player
	.long   \wait
  .long   \pitch
  .long   \vol
  .long   \mod_depth
  .long   \mod_speed
	.endm

//======================================================================
/**
 * @brief	SEストップ
 *
 * #param_num	1
 * @param player  ストップするPlayerNo
 *
 * #param COMBOBOX_TEXT すべて  SE1 SE2 PSG SYSTEM
 * #param COMBOBOX_VALUE BTLEFF_SESTOP_ALL  BTLEFF_SESTOP_SE1 BTLEFF_SESTOP_SE2 BTLEFF_SESTOP_PSG BTLEFF_SESTOP_SYSTEM
 */
//======================================================================
	.macro	SE_STOP player
	.short	EC_SE_STOP
  .long   \player
	.endm

//======================================================================
/**
 * @brief	SEパン
 *
 * #param_num	6
 * @param player      セットするPlayerNo
 * @param type        変化タイプ
 * @param start       開始値
 * @param end         終了値
 * @param start_wait  開始ウエイト
 * @param para        startからendまでのフレーム数  フレーム間のウエイト  往復時の回数
 *
 * #param COMBOBOX_TEXT SE1 SE2 PSG SYSTEM
 * #param COMBOBOX_VALUE BTLEFF_SEPLAY_SE1 BTLEFF_SEPLAY_SE2 BTLEFF_SEPLAY_PSG  BTLEFF_SEPLAY_SYSTEM
 * #param COMBOBOX_TEXT 追従  往復
 * #param COMBOBOX_VALUE  BTLEFF_SEPAN_INTERPOLATION BTLEFF_SEPAN_ROUNDTRIP
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_SEPAN_ATTACK BTLEFF_SEPAN_DEFENCE
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_SEPAN_ATTACK BTLEFF_SEPAN_DEFENCE
 * #param VALUE_INT 開始ウエイト
 * #param VALUE_VECINT startからendまでのフレーム数  フレーム間のウエイト  往復回数
 * #param VALUE_INIT  256:0:0
 */
//======================================================================
	.macro	SE_PAN player, type, start, end, start_wait, frame, wait, count
	.short	EC_SE_PAN
  .long   \player
  .long   \type
  .long   \start
  .long   \end
  .long   \start_wait
  .long   \frame
  .long   \wait
  .long   \count
	.endm

//======================================================================
/**
 * @brief	SE動的変化
 *
 * #param_num	7
 * @param player      変更するPlayerNo
 * @param type        変化タイプ
 * @param param       変化させるパラメータ
 * @param start       開始値
 * @param end         終了値
 * @param start_wait  開始ウエイト
 * @param para        startからendまでのフレーム数  フレーム間のウエイト  往復時の回数
 *
 * #param COMBOBOX_TEXT SE1 SE2 PSG SYSTEM
 * #param COMBOBOX_VALUE BTLEFF_SEPLAY_SE1 BTLEFF_SEPLAY_SE2 BTLEFF_SEPLAY_PSG  BTLEFF_SEPLAY_SYSTEM
 * #param COMBOBOX_TEXT 追従  往復
 * #param COMBOBOX_VALUE BTLEFF_SEEFFECT_INTERPOLATION BTLEFF_SEEFFECT_ROUNDTRIP
 * #param COMBOBOX_TEXT ピッチ  ボリューム  パン
 * #param COMBOBOX_VALUE BTLEFF_SEEFFECT_PITCH  BTLEFF_SEEFFECT_VOLUME  BTLEFF_SEEFFECT_PAN
 * #param VALUE_INT 開始値
 * #param VALUE_INT 終了値
 * #param VALUE_INT 開始ウエイト
 * #param VALUE_VECINT startからendまでのフレーム数  フレーム間のウエイト  往復回数
 *
 */
//======================================================================
	.macro	SE_EFFECT player, type, param, start, end, start_wait, frame, wait, count
	.short	EC_SE_EFFECT
  .long   \player
  .long   \type
  .long   \param
  .long   \start
  .long   \end
  .long   \start_wait
  .long   \frame
  .long   \wait
  .long   \count
	.endm

//======================================================================
/**
 * @brief	エフェクト終了待ち
 *
 * #param_num	1
 * @param	kind	終了待ちする種類
 *
 * #param	COMBOBOX_TEXT	すべて	カメラ	パーティクル	ポケモン	トレーナー  BG  PALFADEお盆  PALFADEフィールド PALFADEお盆＋フィールド  PALFADEエフェクト SEPLAYすべて  SEPLAY1 SEPLAY2 SEPLAY_PSG SEPLAY_SYSTEM
 * #param COMBOBOX_VALUE	BTLEFF_EFFENDWAIT_ALL	BTLEFF_EFFENDWAIT_CAMERA	BTLEFF_EFFENDWAIT_PARTICLE	BTLEFF_EFFENDWAIT_POKEMON	BTLEFF_EFFENDWAIT_TRAINER BTLEFF_EFFENDWAIT_BG  BTLEFF_EFFENDWAIT_PALFADE_STAGE  BTLEFF_EFFENDWAIT_PALFADE_FIELD  BTLEFF_EFFENDWAIT_PALFADE_3D BTLEFF_EFFENDWAIT_PALFADE_EFFECT BTLEFF_EFFENDWAIT_SEALL  BTLEFF_EFFENDWAIT_SE1  BTLEFF_EFFENDWAIT_SE2 BTLEFF_EFFENDWAIT_PSG BTLEFF_EFFENDWAIT_SYSTEM
 */
//======================================================================
	.macro	EFFECT_END_WAIT	kind
	.short	EC_EFFECT_END_WAIT
	.long		\kind
	.endm

//======================================================================
/**
 * @brief	タイマーによるウエイト
 *
 * #param_num	1
 * @param		wait	ウエイト時間
 *
 * #param	VALUE_INT	ウエイト時間
 */
//======================================================================
	.macro	WAIT	wait
	.short	EC_WAIT
	.long		\wait
	.endm

//======================================================================
/**
 * @brief	制御モード変更
 *
 * #param_num	1
 * @param		mode	制御モード（継続：モードが切り替わるか、エフェクト終了まちまで処理を継続して実行　逐次：1行ごとに実行）
 *
 * #param	COMBOBOX_TEXT	継続	逐次
 * #param	COMBOBOX_VALUE	BTLEFF_CONTROL_MODE_CONTINUE	BTLEFF_CONTROL_MODE_SUSPEND
 */
//======================================================================
	.macro	CONTROL_MODE	mode
	.short	EC_CONTROL_MODE
	.long		\mode
	.endm

//======================================================================
/**
 * @brief	指定されたワークを見て条件分岐
 *
 * #param_num	4
 * @param		work  条件分岐に使うワーク
 * @param		cond  条件式
 * @param		value 比較する値
 * @param		adrs  飛び先
 *
 * #param COMBOBOX_TEXT WAZA_RANGE TURN_COUNT CONTINUE_COUNT YURE_CNT GET_SUCCESS ITEM_NO POS_AA_WEIGHT POS_BB_WEIGHT POS_A_WEIGHT POS_B_WEIGHT POS_C_WEIGHT POS_D_WEIGHT POS_E_WEIGHT POS_F_WEIGHT SEQUENCE_WORK ATTACK_POKEMON_VANISH
 * #param COMBOBOX_VALUE BTLEFF_WORK_WAZA_RANGE BTLEFF_WORK_TURN_COUNT BTLEFF_WORK_CONTINUE_COUNT BTLEFF_WORK_YURE_CNT BTLEFF_WORK_GET_SUCCESS BTLEFF_WORK_ITEM_NO BTLEFF_WORK_POS_AA_WEIGHT  BTLEFF_WORK_POS_BB_WEIGHT BTLEFF_WORK_POS_A_WEIGHT BTLEFF_WORK_POS_B_WEIGHT BTLEFF_WORK_POS_C_WEIGHT BTLEFF_WORK_POS_D_WEIGHT BTLEFF_WORK_POS_E_WEIGHT BTLEFF_WORK_POS_F_WEIGHT BTLEFF_WORK_SEQUENCE_WORK BTLEFF_WORK_ATTACK_POKEMON_VANISH
 * #param COMBOBOX_TEXT ==  !=  <  >  <=  >=
 * #param COMBOBOX_VALUE  BTLEFF_COND_EQUAL BTLEFF_COND_NOT_EQUAL BTLEFF_COND_MIMAN BTLEFF_COND_KOERU  BTLEFF_COND_IKA BTLEFF_COND_IJOU
 * #param VALUE_INT 比較する値
 * #param VALUE_INT 飛び先ラベル
 */
//======================================================================
	.macro	IF  work, cond, value, adrs
	.short  EC_IF
	.long   \work
	.long   \cond
	.long   \value
	.long		( \adrs - . ) - 4
	.endm

//======================================================================
/**
 * @brief	立ち位置チェック
 *
 * #param_num	3
 * @param		pos   立ち位置
 * @param	  cond  チェック条件
 * @param		adrs  チェック条件TRUE時の飛び先
 *
 * #param	COMBOBOX_TEXT	POS_AA	POS_BB	POS_A POS_B POS_C POS_D POS_E POS_F POS_TR_AA POS_TR_BB POS_TR_A  POS_TR_B  POS_TR_C  POS_TR_D 
 * #param	COMBOBOX_VALUE  BTLEFF_POKEMON_POS_AA BTLEFF_POKEMON_POS_BB BTLEFF_POKEMON_POS_A  BTLEFF_POKEMON_POS_B  BTLEFF_POKEMON_POS_C  BTLEFF_POKEMON_POS_D  BTLEFF_POKEMON_POS_E  BTLEFF_POKEMON_POS_F  BTLEFF_TRAINER_POS_AA BTLEFF_TRAINER_POS_BB BTLEFF_TRAINER_POS_A  BTLEFF_TRAINER_POS_B  BTLEFF_TRAINER_POS_C  BTLEFF_TRAINER_POS_D
 * #param	COMBOBOX_TEXT いない  いる
 * #param	COMBOBOX_VALUE BTLEFF_COND_NO_EXIST  BTLEFF_COND_EXIST
 * #param	VALUE_INT 飛び先ラベル
 */
//======================================================================
	.macro	MCSS_POS_CHECK	pos, cond, adrs
  .short  EC_MCSS_POS_CHECK
	.long		\pos
	.long		\cond
	.long		( \adrs - . ) - 4
	.endm

//======================================================================
/**
 * @brief	汎用ワークに値をセット
 *
 * #param_num	1
 * @param		value	セットする値
 *
 * #param	VALUE_INT セットする値
 */
//======================================================================
	.macro	SET_WORK	value
  .short  EC_SET_WORK
	.long		\value
	.endm

//======================================================================
/**
 * @brief	みがわり処理
 *
 * #param_num	3
 * @param sw	  ON/OFF
 * @param pos   処理するポジション
 * @param flag  内部で保持するフラグの立ち下げを制御
 *
 * #param COMBOBOX_TEXT ON  OFF
 * #param COMBOBOX_VALUE BTLEFF_MIGAWARI_ON BTLEFF_MIGAWARI_OFF
 * #param	COMBOBOX_TEXT	攻撃側  防御側
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK  BTLEFF_POKEMON_SIDE_DEFENCE
 * #param	VALUE_INT 0
 *
 */
//======================================================================
	.macro	MIGAWARI	sw, pos, flag
  .short  EC_MIGAWARI
	.long		\sw
	.long		\pos
	.long		\flag
	.endm

//======================================================================
/**
 * @brief	ラベル
 *
 * #param_num	1
 * @param		label	ラベル
 *
 * #param	VALUE_INT ラベル
 */
//======================================================================
	.macro	LABEL	label
\label:
	.endm

//======================================================================
/**
 * 	エフェクト終了はかならず最後になるようにする
 */
//======================================================================
//======================================================================
/**
 * @brief	エフェクト終了
 *
 * #param_num	0
 */
//======================================================================
	.macro	SEQ_END
	.short	EC_SEQ_END
	.endm

#endif
