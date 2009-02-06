//======================================================================
/**
 * @file	btl_effect_def.h
 * @brief	エフェクトシーケンス用のマクロ定義ファイル
 * @author	HisashiSogabe
 * @date	2009.01.21
 */
//======================================================================

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
 * #param	COMBOBOX_TEXT	初期位置	ダイレクト	追従
 * #param	COMBOBOX_VALUE	CAMERA_MOVE_INIT	CAMERA_MOVE_DIRECT	CAMERA_MOVE_INTERPOLATION
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
　ファイルダイアログを表示してファイル選択を促す　選択を絞る拡張子指定できます

#endif


#define	__ASM_NO_DEF_

	.macro	INIT_CMD
def_cmd_count = 0
	.endm

	.macro	DEF_CMD	symname
\symname = def_cmd_count
def_cmd_count = ( def_cmd_count + 1 )
	.endm

//命令シンボル宣言
	INIT_CMD
	DEF_CMD	WS_CAMERA_MOVE
	DEF_CMD	WS_PARTICLE_LOAD
	DEF_CMD	WS_PARTICLE_PLAY
	DEF_CMD	WS_PARTICLE_PLAY_WITH_POS

//命令マクロ定義
//======================================================================
/**
 * @brief		カメラ移動
 *
 * #param_num	4
 * @param	type	カメラ移動タイプ
 * @param	pos_x	カメラ移動先X座標
 * @param	pos_y	カメラ移動先Y座標
 * @param	pos_z	カメラ移動先Z座標
 *
 * #param	COMBOBOX_TEXT	初期位置	ダイレクト	追従
 * #param	COMBOBOX_VALUE	CAMERA_MOVE_INIT	CAMERA_MOVE_DIRECT	CAMERA_MOVE_INTERPOLATION
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	CAMERA_MOVE	type, pos_x, pos_y, pos_z
	.long	WS_CAMERA_MOVE
	.long	\pos_x
	.long	\pos_y
	.long	\pos_z
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
	.long	WS_PARTICLE_LOAD
	.long	\datID
	.endm

//======================================================================
/**
 * @brief	パーティクル再生
 *
 * #param_num	3
 * @param	num			再生パーティクルナンバー
 * @param	start_pos	パーティクル再生開始立ち位置
 * @param	dir_pos		パーティクル再生方向立ち位置
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 * #param	COMBOBOX_TEXT	攻撃側	防御側
 */
//======================================================================
	.macro	PARTICLE_PLAY	pos_x, pos_y, pos_z, angle
	.long	WS_PARTICLE_PLAY
	.long	\num
	.long	\start_pos
	.long	\dir_pos
	.endm

//======================================================================
/**
 * @brief	パーティクル再生（座標指定あり）
 *
 * #param_num	5
 * @param	num		再生パーティクルナンバー
 * @param	pos_x	パーティクル再生位置X座標
 * @param	pos_y	パーティクル再生位置Y座標
 * @param	pos_z	パーティクル再生位置Z座標
 * @param	angle	パーティクル角度
 *
 * #param	FILE_DIALOG_COMBOBOX .spa
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 * #param	VALUE_FX32
 */
//======================================================================
	.macro	PARTICLE_PLAY_WITH_POS	num, pos_x, pos_y, pos_z, angle
	.long	WS_PARTICLE_PLAY_WITH_POS
	.long	\num
	.long	\pos_x
	.long	\pos_y
	.long	\pos_z
	.long	\angle
	.endm

