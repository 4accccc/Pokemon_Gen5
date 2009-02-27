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
#define	BTLV_EFFVM_REG_SIZE		( 8 )	///<使用するレジスタの数

//スクリプトアドレステーブル用定義
#define	BTLEFF_AA_SAME			( 0xffffffff )		//立ち位置AAと同じ
#define	BTLEFF_AA_REVERSE		( 0xfffffffe )		//立ち位置AAの反転
#define	BTLEFF_BB_SAME			( 0xfffffffd )		//立ち位置BBと同じ
#define	BTLEFF_BB_REVERSE		( 0xfffffffc )		//立ち位置BBの反転
#define	BTLEFF_A_SAME			( 0xfffffffb )		//立ち位置Aと同じ
#define	BTLEFF_A_REVERSE		( 0xfffffffa )		//立ち位置Aの反転
#define	BTLEFF_B_SAME			( 0xfffffff9 )		//立ち位置Bと同じ
#define	BTLEFF_B_REVERSE		( 0xfffffff8 )		//立ち位置Bの反転
#define	BTLEFF_C_SAME			( 0xfffffff7 )		//立ち位置Cと同じ
#define	BTLEFF_C_REVERSE		( 0xfffffff6 )		//立ち位置Cの反転
#define	BTLEFF_D_SAME			( 0xfffffff5 )		//立ち位置Dと同じ
#define	BTLEFF_D_REVERSE		( 0xfffffff4 )		//立ち位置Dの反転

//カメラ移動タイプ
#define	BTLEFF_CAMERA_MOVE_DIRECT			( 0 )		//ダイレクト
#define	BTLEFF_CAMERA_MOVE_INTERPOLATION	( 1 )		//追従

//カメラ移動先
#define	BTLEFF_CAMERA_POS_AA		( 0 )
#define	BTLEFF_CAMERA_POS_BB		( 1 )
#define	BTLEFF_CAMERA_POS_A			( 2 )
#define	BTLEFF_CAMERA_POS_B			( 3 )
#define	BTLEFF_CAMERA_POS_C			( 4 )
#define	BTLEFF_CAMERA_POS_D			( 5 )
#define	BTLEFF_CAMERA_POS_INIT		( 6 )

//パーティクル再生
#define	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	( 0 )
#define	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE	( 1 )

//ポケモン操作
#define	BTLEFF_POKEMON_SIDE_ATTACK				( 0 )		//攻撃側
#define	BTLEFF_POKEMON_SIDE_ATTACK_PAIR			( 1 )		//攻撃側ペア
#define	BTLEFF_POKEMON_SIDE_DEFENCE				( 2 )		//防御側
#define	BTLEFF_POKEMON_SIDE_DEFENCE_PAIR		( 3 )		//防御側ペア

#define	BTLEFF_POKEMON_MOVE_DIRECT				( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_MOVE_INTERPOLATION		( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_MOVE_ROUNDTRIP			( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_MOVE_ROUNDTRIP_LONG		( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_POKEMON_SCALE_DIRECT				( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_SCALE_INTERPOLATION		( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_SCALE_ROUNDTRIP			( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_SCALE_ROUNDTRIP_LONG		( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_POKEMON_ROTATE_DIRECT			( EFFTOOL_CALCTYPE_DIRECT )
#define	BTLEFF_POKEMON_ROTATE_INTERPOLATION		( EFFTOOL_CALCTYPE_INTERPOLATION )
#define	BTLEFF_POKEMON_ROTATE_ROUNDTRIP			( EFFTOOL_CALCTYPE_ROUNDTRIP )
#define	BTLEFF_POKEMON_ROTATE_ROUNDTRIP_LONG	( EFFTOOL_CALCTYPE_ROUNDTRIP_LONG )
#define	BTLEFF_MEPACHI_ON						( POKE_MCSS_MEPACHI_ON )
#define	BTLEFF_MEPACHI_OFF						( POKE_MCSS_MEPACHI_OFF )
#define	BTLEFF_MEPACHI_MABATAKI					( 2 )
#define	BTLEFF_ANM_STOP							( POKE_MCSS_ANM_STOP_ON )
#define	BTLEFF_ANM_START						( POKE_MCSS_ANM_STOP_OFF )

#endif //__BTLV_EFFVM_DEF_H_

//====================================================================================
/**
 *	VisualStudioで作成したエフェクトエディタツールでも使用されるファイルです 
 *	各命令マクロ定義のコメントからツールで表示されるコマンド名や引数の型などを判断します
 */
//====================================================================================

#if 0

ex)
 * #param_num	4
 * @param	type	カメラ移動タイプ
 * @param	pos_x	カメラ移動先X座標
 * @param	pos_y	カメラ移動先Y座標
 * @param	pos_z	カメラ移動先Z座標
 *
 * #param	COMBOBOX_TEXT	ダイレクト	追従
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_MOVE_DIRECT	BTLEFF_CAMERA_MOVE_INTERPOLATION
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32

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
・FILE_DIALOG
　ファイルダイアログを表示してファイル選択を促す　選択を絞る拡張子を指定できます
・FILE_DIALOG_COMBOBOX
　ファイルダイアログで選択したファイルをコンボボックスで表示します　選択を絞る拡張子を指定できます

#endif

#ifndef __C_NO_DEF_

//勝手にパディングを入れられないようにする
	.option	alignment off

	.macro	INIT_CMD
def_cmd_count = 0
	.endm

	.macro	DEF_CMD	symname
\symname = def_cmd_count
def_cmd_count = ( def_cmd_count + 1 )
	.endm

//命令シンボル宣言
	INIT_CMD
	DEF_CMD	EC_CAMERA_MOVE
	DEF_CMD	EC_PARTICLE_LOAD
	DEF_CMD	EC_PARTICLE_PLAY
	DEF_CMD	EC_PARTICLE_PLAY_WITH_DIR
	DEF_CMD	EC_POKEMON_MOVE
	DEF_CMD	EC_POKEMON_SCALE
	DEF_CMD	EC_POKEMON_ROTATE
	DEF_CMD	EC_POKEMON_SET_MEPACHI_FLAG
	DEF_CMD	EC_POKEMON_SET_ANM_FLAG

//終了コマンドは必ず一番下になるようにする
	DEF_CMD	EC_SEQ_END

//命令マクロ定義
//======================================================================
/**
 * @brief		カメラ移動
 *
 * #param_num	4
 * @param	type		カメラ移動タイプ
 * @param	move_pos	移動先
 * @param	frame		カメラタイプが追従のとき、何フレームで移動先に到達するかを指定
 * @param	brake		ブレーキをかけはじめるフレームを指定
 *
 * #param	COMBOBOX_TEXT	ダイレクト	追従
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_MOVE_DIRECT	BTLEFF_CAMERA_MOVE_INTERPOLATION
 * #param	COMBOBOX_TEXT	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D	初期位置
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_POS_AA	BTLEFF_CAMERA_POS_BB	BTLEFF_CAMERA_POS_A	BTLEFF_CAMERA_POS_B	BTLEFF_CAMERA_POS_C	BTLEFF_CAMERA_POS_D	BTLEFF_CAMERA_POS_INIT
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	CAMERA_MOVE	type, move_pos, frame, brake
	.short	EC_CAMERA_MOVE
	.long	\type
	.long	\move_pos
	.long	\frame
	.long	\brake
	.endm

//======================================================================
/**
 * @brief	パーティクルデータロード
 *
 * #param_num	1
 * @param	datID	アーカイブデータのdatID
 *
 * #param	FILE_DIALOG	.spa
 */
//======================================================================
	.macro	PARTICLE_LOAD	datID
	.short	EC_PARTICLE_LOAD
	.long	\datID
	.endm

//======================================================================
/**
 * @brief	パーティクル再生
 *
 * #param_num	3
 * @param	num			再生パーティクルナンバー
 * @param	index		spa内インデックスナンバー
 * @param	start_pos	パーティクル再生開始立ち位置
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	VALUE_INT
 * #param	COMBOBOX_TEXT	POS_AA	POS_BB	POS_A	POS_B	POS_C	POS_D
 * #param	COMBOBOX_VALUE	BTLEFF_CAMERA_POS_AA	BTLEFF_CAMERA_POS_BB	BTLEFF_CAMERA_POS_A	BTLEFF_CAMERA_POS_B	BTLEFF_CAMERA_POS_C	BTLEFF_CAMERA_POS_D	BTLEFF_CAMERA_POS_INIT
 */
//======================================================================
	.macro	PARTICLE_PLAY	num, index, start_pos
	.short	EC_PARTICLE_PLAY
	.long	\num
	.long	\index
	.long	\start_pos
	.endm

//======================================================================
/**
 * @brief	パーティクル再生（方向指定あり）
 *
 * #param_num	5
 * @param	num			再生パーティクルナンバー
 * @param	index		spa内インデックスナンバー
 * @param	start_pos	パーティクル再生開始立ち位置
 * @param	dir_pos		パーティクル再生方向立ち位置
 * @param	dir_angle	パーティクル再生方向Y角度
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	VALUE_INT
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_VALUE	BTLEFF_PARTICLE_PLAY_SIDE_ATTACK	BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	PARTICLE_PLAY_WITH_DIR	num, index, start_pos, dir_pos, dir_angle
	.short	EC_PARTICLE_PLAY_WITH_DIR
	.long	\num
	.long	\index
	.long	\start_pos
	.long	\dir_pos
	.long	\dir_angle
	.endm

//======================================================================
/**
 * @brief	ポケモン移動
 *
 * #param_num	7
 * @param	pos			移動させるポケモンの立ち位置
 * @param	type		移動タイプ
 * @param	move_pos_x	移動先X座標
 * @param	move_pos_y	移動先Y座標
 * @param	frame		移動フレーム数（目的地まで何フレームで到達するか）
 * @param	wait		移動ウエイト
 * @param	count		往復カウント（移動タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア　防御側　防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_MOVE_DIRECT	BTLEFF_POKEMON_MOVE_INTERPOLATION	BTLEFF_POKEMON_MOVE_ROUNDTRIP	BTLEFF_POKEMON_MOVE_ROUNDTRIP_LONG
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	POKEMON_MOVE	pos, type, move_pos_x, move_pos_y, frame, wait, count
	.short	EC_POKEMON_MOVE
	.long	\pos
	.long	\type
	.long	\move_pos_x
	.long	\move_pos_y
	.long	\frame
	.long	\wait
	.long	\count
	.endm

//======================================================================
/**
 * @brief	ポケモン拡縮
 *
 * #param_num	7
 * @param	pos			拡縮させるポケモンの立ち位置
 * @param	type		拡縮タイプ
 * @param	scale_x		X方向拡縮率
 * @param	scale_y		Y方向拡縮率
 * @param	frame		拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param	wait		拡縮ウエイト
 * @param	count		往復カウント（拡縮タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア　防御側　防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SCALE_DIRECT	BTLEFF_POKEMON_SCALE_INTERPOLATION	BTLEFF_POKEMON_SCALE_ROUNDTRIP	BTLEFF_POKEMON_SCALE_ROUNDTRIP_LONG
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	POKEMON_SCALE	pos, type, scale_x, scale_y, frame, wait, count
	.short	EC_POKEMON_SCALE
	.long	\pos
	.long	\type
	.long	\scale_x
	.long	\scale_y
	.long	\frame
	.long	\wait
	.long	\count
	.endm

//======================================================================
/**
 * @brief	ポケモン回転
 *
 * #param_num	6
 * @param	pos			回転させるポケモンの立ち位置
 * @param	type		回転タイプ
 * @param	rotate		回転角度
 * @param	frame		回転フレーム数（設定した回転値まで何フレームで到達するか）
 * @param	wait		回転ウエイト
 * @param	count		往復カウント（回転タイプが往復のときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア　防御側　防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	ダイレクト	追従	往復	往復ロング
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_ROTATE_DIRECT	BTLEFF_POKEMON_ROTATE_INTERPOLATION	BTLEFF_POKEMON_ROTATE_ROUNDTRIP	BTLEFF_POKEMON_ROTATE_ROUNDTRIP_LONG
 * #param	VALUE_FX32
 * #param	VALUE_INT
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	POKEMON_ROTATE	pos, type, rotate, frame, wait, count
	.short	EC_POKEMON_ROTATE
	.long	\pos
	.long	\type
	.long	\rotate
	.long	\frame
	.long	\wait
	.long	\count
	.endm

//======================================================================
/**
 * @brief	ポケモンメパチ
 *
 * #param_num	4
 * @param	pos			メパチさせるポケモンの立ち位置
 * @param	type		メパチタイプ
 * @param	wait		メパチウエイト（メパチタイプがまばたきのときだけ有効）
 * @param	count		メパチカウント（メパチタイプがまばたきのときだけ有効）
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア　防御側　防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	閉じる	開ける	まばたき
 * #param	COMBOBOX_VALUE	BTLEFF_MEPACHI_ON	BTLEFF_MEPACHI_OFF	BTLEFF_MEPACHI_MABATAKI
 * #param	VALUE_INT
 * #param	VALUE_INT
 */
//======================================================================
	.macro	POKEMON_SET_MEPACHI_FLAG	pos, type, wait, count
	.short	EC_POKEMON_SET_MEPACHI_FLAG
	.long	\pos
	.long	\type
	.long	\wait
	.long	\count
	.endm

//======================================================================
/**
 * @brief	ポケモンアニメ
 *
 * #param_num	2
 * @param	pos			アニメ操作するポケモンの立ち位置
 * @param	flag		アニメフラグ
 *
 * #param	COMBOBOX_TEXT	攻撃側	攻撃側ペア　防御側　防御側ペア
 * #param	COMBOBOX_VALUE	BTLEFF_POKEMON_SIDE_ATTACK BTLEFF_POKEMON_SIDE_ATTACK_PAIR BTLEFF_POKEMON_SIDE_DEFENCE BTLEFF_POKEMON_SIDE_DEFENCE_PAIR
 * #param	COMBOBOX_TEXT	アニメストップ	アニメスタート
 * #param	COMBOBOX_VALUE	BTLEFF_ANM_STOP	BTLEFF_ANM_START
 */
//======================================================================
	.macro	POKEMON_SET_ANM_FLAG	pos, flag
	.short	EC_POKEMON_SET_ANM_FLAG
	.long	\pos
	.long	\flag
	.endm

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
