//======================================================================
/**
 * @file  scr_seq_def.h
 * @bfief  スクリプトコマンドのマクロ定義ファイル
 * @author  GAME FREAK inc.
 *
 */
//======================================================================

//スクリプトで使用するヘッダーファイル
  .include  "usescript.h"

//データを自然な境界にアラインするかの設定
  .option alignment off

//スクリプトコマンドテーブル用マクロの定義
  .include "scrcmd_table/def_table_macro.h"
//スクリプト定義テーブル
  .include "../../../prog/src/field/scrcmd_table.cdat"

//======================================================================
// イベントデータ関連
//======================================================================
//--------------------------------------------------------------
/**
 *  @def  _EVENT_DATA
 *  @brief  イベントデータ　テーブル宣言
 *  @param adrs 宣言するデータラベルアドレス
 */
//--------------------------------------------------------------
  .macro  _EVENT_DATA  adrs
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  @def  _EVENT_DATA_END
 *  @brief  イベントデータ　テーブル宣言終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _EVENT_DATA_END
  .short  EV_DATA_END_CODE
  .endm

//--------------------------------------------------------------
/**
 *  @def  _SP_EVENT_DATA_END
 *  @brief  特殊スクリプト終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _SP_EVENT_DATA_END
  .short   SP_SCRID_NONE
  .endm


//=============================================================================
/**
 *	特殊スクリプト関連
 */
//=============================================================================

//--------------------------------------------------------------
/**
 * @brief 特殊スクリプト登録：シーン発動イベントの登録
 * @param adrs  シーン発動イベントテーブルのアドレス
 *
 * @note
 * シーンスクリプト（フィールドでワークの値が指定値の場合に強制発動するスクリプト）を
 * 登録する
 */
//--------------------------------------------------------------
	.macro	_SCENE_CHANGE_LABEL	adrs
	.short	SP_SCRID_SCENE_CHANGE
	.long	((\adrs-.)-4)
	.endm

//--------------------------------------------------------------
/**
 * @brief シーン発動イベントの要素記述
 * @param wk      参照するワーク指定ID
 * @param num     上記ワークの発動条件とする値
 * @param scr_id  スクリプトを指定するID
 *
	起動条件記述
 */
//--------------------------------------------------------------
	.macro	_SCENE_CHANGE_DATA		wk,num,scr_id
	.short	\wk
	.short	\num
	.short	\scr_id
	.endm

//--------------------------------------------------------------
/**
 * @brief シーン発動イベントの要素終端
 *
 * _SCENE_CHANGE_LABELで指定したシーン発動イベントテーブルの最後には
 * 必ず_SCENE_CHANGE_ENDを記述する必要がある
 */
//--------------------------------------------------------------
	.macro	_SCENE_CHANGE_END
	.short	0
	.endm

//--------------------------------------------------------------
/**
 *	@brief  特殊スクリプト登録：ゾーン切り替え処理
 *	@param  scr_id  登録するスクリプトID
 *
 *	@note
 *	ゾーン切り替えのタイミングで呼ばれます。
 *	歩いてマップをまたいだタイミング、
 *	出入口・そらを飛ぶなどフィールドマップがないタイミング
 *	の両方で呼ばれます。
 *	後者の際は、使用できるコマンドの制限に気をつけること！
 */
//--------------------------------------------------------------
	.macro	_ZONE_CHANGE_LABEL	scr_id
	.short	SP_SCRID_ZONE_CHANGE
	.short	\scr_id
	.short	0		//4byteに合わせるためにダミー
	.endm

//--------------------------------------------------------------
/**
 *	@brief  特殊スクリプト登録：マップ初回初期化処理
 *	@param  scr_id  登録するスクリプトID
 *	@note
 *	マップができて、ＯＢＪが配置されたあと呼ばれる。
 *	そのマップに入ったときに呼ばれます。
 */
//--------------------------------------------------------------
	.macro	_FIELD_INIT_LABEL	scr_id
	.short	SP_SCRID_FIELD_INIT
	.short	\scr_id
	.short	0		//4byteに合わせるためにダミー
	.endm

//--------------------------------------------------------------
/**
 *	@brief  特殊スクリプト登録：マップ復帰初期化処理
 *	@param  scr_id  登録するスクリプトID
 *	@note
 *	マップができて、ＯＢＪが配置されたあと呼ばれる。
 *	マップ復帰処理（メニューやバトルからの復帰）のたびによばれます。
 */
//--------------------------------------------------------------
	.macro	_FIELD_RECOVER_LABEL	scr_id
	.short	SP_SCRID_FIELD_RECOVER
	.short	\scr_id
	.short	0		//4byteに合わせるためにダミー
	.endm


//======================================================================
//
//
//  イベント開始・終了
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 通常イベント開始宣言
 */
//--------------------------------------------------------------
  .macro  EVENT_START label
\label:
  .short  EV_SEQ_COMMON_PROC_FIELD_EVENT_START
  .endm

//--------------------------------------------------------------
/**
 * 通常イベント終了宣言
 */
//--------------------------------------------------------------
  .macro  EVENT_END
  .short  EV_SEQ_CHECK_SCR_END
  .short  EV_SEQ_COMMON_PROC_FIELD_EVENT_END
  .short  EV_SEQ_END
  .endm

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
  .macro  INIT_EVENT_START  label
\label:
  .endm

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
  .macro  INIT_EVENT_END
  .short  EV_SEQ_END
  .endm


//======================================================================
//  基本コマンド
//======================================================================
//--------------------------------------------------------------
/**
 *  @def  _NOP
 *  @brief  何もしない
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _NOP
  .short  EV_SEQ_NOP
  .endm

//--------------------------------------------------------------
/**
 *  @def  _DUMMY
 *  @brief  ダミー
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _DUMMY
  .short  EV_SEQ_DUMMY
  .endm

//--------------------------------------------------------------
/**
 *  @def  _END
 *  @brief  スクリプトの終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _END
  .short  EV_SEQ_END
  .endm

//--------------------------------------------------------------
/**
 *  @def  _TIME_WAIT
 *  @brief  ウェイト
 *  @param time 待ちフレーム数
 */
//--------------------------------------------------------------
#define _TIME_WAIT( time ) _ASM_TIME_WAIT time

  .macro  _ASM_TIME_WAIT time, ret_wk
  .short  EV_SEQ_TIME_WAIT
  .short  \time
  .endm


//--------------------------------------------------------------
/**
 *  _CALL コール
 *  @param adrs 呼び出すラベルアドレス
 */
//--------------------------------------------------------------
  .macro  _CALL adrs
  .short  EV_SEQ_CALL
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _RET コールを呼び出したアドレスに戻る
 */
//--------------------------------------------------------------
  .macro  _RET
  .short  EV_SEQ_RET
  .endm

//--------------------------------------------------------------
/**
 * @def _DEBUG_PRINT_WK
 * @brief ワークの値を出力する
 * @param work 値を出力するワーク
 */
//--------------------------------------------------------------
#define _DEBUG_PRINT_WK( work ) \
    _ASM_DEBUG_PRINT_WK  work

  .macro  _ASM_DEBUG_PRINT_WK work
  .short  EV_SEQ_DEBUG_PRINT_WK
  .short  \work
  .endm

//======================================================================
//    判定演算用命令
//  ※スクリプトコンパイラが使用しています。
//  　スクリプト担当者は使用しないで下さい。
//======================================================================
//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
  .macro  _CMP_STACK  cond
  .short  EV_SEQ_CMP_STACK
  .short   \cond
  .endm
//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _PUSH_WORK  wk
  .short  EV_SEQ_PUSH_WORK
  .short  \wk
  .endm

//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _PUSH_VALUE  value
  .short  EV_SEQ_PUSH_VALUE
  .short  \value
  .endm
//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _PUSH_FLAG  flag
  .short  EV_SEQ_PUSH_FLAG
  .short  \flag
  .endm
//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _POP_WORK  wk
  .short  EV_SEQ_POP_WORK
  .short  \wk
  .endm

//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _POP
  .short  EV_SEQ_POP
  .endm


//======================================================================
//  データロード・ストア関連
//======================================================================
//--------------------------------------------------------------
/**
 * _LD_REG_VAL 仮想マシンの汎用レジスタに1byteの値を格納
 * @param r 値を格納するレジスタ
 * @param val 格納する値
 */
//--------------------------------------------------------------
  .macro  _LD_REG_VAL  r,val
  .short  EV_SEQ_LD_REG_VAL
  .byte  \r
  .byte  \val
  .endm

//--------------------------------------------------------------
/**
 * _LD_REG_WDATA 仮想マシンの汎用レジスタに4byteの値を格納
 * @param r 値を格納するレジスタ
 * @param val 格納する値
 */
//--------------------------------------------------------------
  .macro  _LD_REG_WDATA  r,wdata
  .short  EV_SEQ_LD_REG_WDATA
  .byte  \r
  .long  \wdata
  .endm

//--------------------------------------------------------------
/**
 * _LD_REG_ADR 仮想マシンの汎用レジスタにアドレスを格納
 * @param reg アドレスを格納するレジスタ
 * @@aram ADDRESS 格納するアドレス
 */
//--------------------------------------------------------------
  .macro  _LD_REG_ADR  reg,ADDRESS
  .short  EV_SEQ_LD_REG_ADR
  .byte  \reg
  .long  \ADDRESS
  .endm

//--------------------------------------------------------------
/**
 * _LD_ADR_VAL アドレスの中身に値を代入
 * @param adrs 値を代入するアドレス
 * @param val 代入する値
 */
//--------------------------------------------------------------
  .macro  _LD_ADR_VAL  adrs,val
  .short  EV_SEQ_LD_ADR_VAL
  .long  \adrs
  .byte  \val
  .endm

//--------------------------------------------------------------
/**
 * _LD_ADR_REG アドレスの中身に仮想マシンの汎用レジスタの値を代入
 * @param adrs 値を代入するアドレス
 * @param val 代入する値が格納されたレジスタ
 */
//--------------------------------------------------------------
  .macro  _LD_ADR_REG  adrs,reg
  .short  EV_SEQ_LD_ADR_REG
  .long  \adrs
  .byte  \reg
  .endm

//--------------------------------------------------------------
/**
 * _LD_REG_REG 仮想マシンの汎用レジスタの値を汎用レジスタにコピー
 * @param r1 コピー先レジスタ
 * @param r2 コピー元レジスタ
 */
//--------------------------------------------------------------
  .macro  _LD_REG_REG  r1,r2
  .short  EV_SEQ_LD_REG_REG
  .byte  \r1,\r2
  .endm

//--------------------------------------------------------------
/**
 * _LD_ADR_ADR アドレスの中身にアドレスの中身を代入
 * @param r1 代入先アドレス
 * @param r2 代入元アドレス
 */
//--------------------------------------------------------------
  .macro  _LD_ADR_ADR  adr1,adr2
  .short  EV_SEQ_LD_ADR_ADR
  .long  \adr1
  .long  \adr2
  .endm

//======================================================================
//  比較命令
//======================================================================
//--------------------------------------------------------------
/**
 *  _CP_REG_REG 仮想マシンの汎用レジスタを比較
 *  @param r1 比較するレジスタ
 *  @param r2 r1と比較するレジスタ
 */
//--------------------------------------------------------------
  .macro  _CP_REG_REG  r1,r2
  .short  EV_SEQ_CP_REG_REG
  .byte  \r1,\r2
  .endm

//--------------------------------------------------------------
/**
 *  _CP_REG_VAL 仮想マシンの汎用レジスタと値を比較
 *  @param r1 比較するレジスタ
 *  @param val 比較する値
 */
//--------------------------------------------------------------
  .macro  _CP_REG_VAL  r1,val
  .short  EV_SEQ_CP_REG_VAL
  .byte  \r1,\val
  .endm

//--------------------------------------------------------------
/**
 *  _CP_REG_ADR 仮想マシンの汎用レジスタとアドレスの中身を比較
 *  @param r1 比較するレジスタ
 *  @param val 比較する値
 */
//--------------------------------------------------------------
  .macro  _CP_REG_ADR  r1,adrs
  .short  EV_SEQ_CP_REG_ADR
  .byte  \r1
  .long  \adrs
  .endm

//--------------------------------------------------------------
/**
 *  _CP_ADR_REG アドレスの中身と仮想マシンの汎用レジスタを比較
 *  @param adrs 比較するアドレス
 *  @param r1 比較するレジスタ
 */
//--------------------------------------------------------------
  .macro  _CP_ADR_REG  adrs,r1
  .short  EV_SEQ_CP_ADR_REG
  .long  \adrs
  .byte  \r1
  .endm

//--------------------------------------------------------------
/**
 *  _CP_ADR_VAL アドレスの中身と値を比較
 *  @param adrs 比較するアドレス
 *  @param val 比較する値
 */
//--------------------------------------------------------------
  .macro  _CP_ADR_VAL  adrs,val
  .short  EV_SEQ_CP_ADR_VAL
  .long  \adrs
  .byte  \val
  .endm

//--------------------------------------------------------------
/**
 *  _CP_ADR_ADR アドレスの中身とアドレスの中身を比較
 *  @param adr1 比較するアドレスその1
 *  @param adr2 比較するアドレスその2
 */
//--------------------------------------------------------------
  .macro  _CP_ADR_ADR  adr1,adr2
  .short  EV_SEQ_CP_ADR_ADR
  .long  \adr1
  .long  \adr2
  .endm

//--------------------------------------------------------------
/**
 *  _CMPVAL ワークと値の比較
 *  @param wk 比較するワーク
 *  @param val 比較する値
 */
//--------------------------------------------------------------
  .macro  _CMPVAL  wk,val
  .short  EV_SEQ_CP_WK_VAL
  .short  \wk
  .short  \val
  .endm

//--------------------------------------------------------------
/**
 *  _CMPWK ワークとワークの比較
 *  @param 比較するワークその1
 *  @param 比較するワークその2
 */
//--------------------------------------------------------------
  .macro  _CMPWK  wk1,wk2
  .short  EV_SEQ_CP_WK_WK
  .short  \wk1
  .short  \wk2
  .endm


//======================================================================
//  仮想マシン関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _VM_ADD 仮想マシン追加(切り替えはせず、並列で動作します！)
 *  @param id 動作させる仮想マシンID
 */
//--------------------------------------------------------------
  .macro  _VM_ADD id
  .short  EV_SEQ_VM_ADD
  .short  \id
  .endm

//--------------------------------------------------------------
/**
 * _CHG_COMMON_SCR ローカルスクリプトをウェイト状態にして、
 * 共通スクリプトを動作させます
 * @param id 動作させるスクリプトID
 */
//--------------------------------------------------------------
  .macro  _CHG_COMMON_SCR id
  .short  EV_SEQ_CHG_COMMON_SCR
  .short  \id
  .endm

//--------------------------------------------------------------
/**
 * @def _GLOBAL_CALL
 * @brief ファイルをまたいだスクリプトの呼び出し（_CHG_COMMON_SCRの別名）
 * @param id 動作させるスクリプトID
 */
//--------------------------------------------------------------
  .macro  _GLOBAL_CALL  id
  .short  EV_SEQ_CHG_COMMON_SCR
  .short  \id
  .endm

//--------------------------------------------------------------
/**
 *  _CHG_LOCAL_SCR 共通スクリプトを終了して、ローカルスクリプトを再開させます
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _CHG_LOCAL_SCR
  .short  EV_SEQ_CHG_LOCAL_SCR
  .endm

//--------------------------------------------------------------
/**
 * @def _RESERVE_SCRIPT
 * @brief スクリプトの予約
 * @param scr_id  スクリプト指定ID
 *
 * シーンイベント発動タイミングで起動するスクリプトを予約登録する
 */
//--------------------------------------------------------------
#define _RESERVE_SCRIPT( scr_id ) \
    _ASM_RESERVE_SCRIPT scr_id

  .macro  _ASM_RESERVE_SCRIPT scr_id
  .short  EV_SEQ_RESERVE_SCRIPT
  .short  \scr_id
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_CONTINUE_RECOVER
 * @brief コンティニュー時かどうかの判定
 * @param ret_wk    TRUEのとき、コンティニュータイミング
 *
 * FIELD_RECOVER_LABEL/FIELD_INIT_LABELからのスクリプト以外では常にFALSEになる
 */
//--------------------------------------------------------------
#define _CHECK_CONTINUE_RECOVER( ret_wk ) \
    _ASM_CHECK_CONTINUE_RECOVER ret_wk

    .macro  _ASM_CHECK_CONTINUE_RECOVER ret_wk
    .short  EV_SEQ_CHECK_CONTINUE_RECOVER
    .short  \ret_wk
    .endm

//======================================================================
//  分岐命令
//======================================================================
//--------------------------------------------------------------
/**
 *  _JUMP ジャンプ
 *  @param adrs 飛び先のアドレス
 */
//--------------------------------------------------------------
  .macro  _JUMP adrs
  .short  EV_SEQ_JUMP
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _IF_JUMP IFジャンプ
 *  @param cond 比較コマンド LT,EQ,GT...
 *  @param adrs condと一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_JUMP cond,adrs
  .short  EV_SEQ_IF_JUMP
  .byte  \cond
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _IF_CALL IFコール
 *  @param cond 比較コマンド LT,EQ,GT...
 *  @param adrs condと一致していた際に呼び出すアドレス
 */
//--------------------------------------------------------------
  .macro  _IF_CALL cond,adrs
  .short  EV_SEQ_IF_CALL
  .byte  \cond
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _IFVAL_JUMP 値を比較、条件どおりの場合分岐(ジャンプ)
 *  @param wk 比較するワーク
 *  @param cond 比較コマンド LT,RQ,GT...
 *  @param val wkと比較する値
 *  @param adrs condと一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IFVAL_JUMP wk,cond,val,adrs
  _CMPVAL  \wk,\val
  .short  EV_SEQ_IF_JUMP
  .byte  \cond
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _IFVAL_CALL 値を比較、条件どおりの場合分岐(コール)
 *  @param wk 比較するワーク
 *  @param cond 比較コマンド LT,RQ,GT...
 *  @param val wkと比較する値
 *  @param adrs condと一致していた際に呼び出すアドレス
 */
//--------------------------------------------------------------
  .macro  _IFVAL_CALL wk,cond,val,adrs
  _CMPVAL  \wk,\val
  .short  EV_SEQ_IF_CALL
  .byte  \cond
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _IFWK_JUMP ワークを比較、条件どおりの場合分岐(ジャンプ)
 *  @param wk1 比較するワーク1
 *  @param cond 比較コマンド LT,RQ,GT...
 *  @param wk2 と比較するワーク2
 *  @param adrs condと一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IFWK_JUMP wk1,cond,wk2,adrs
  _CMPWK  \wk1,\wk2
  .short  EV_SEQ_IF_JUMP
  .byte  \cond
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _IFWK_CALL ワークを比較、条件どおりの場合分岐(コール)
 *  @param wk1 比較するワーク1
 *  @param cond 比較コマンド LT,RQ,GT...
 *  @param wk2 と比較するワーク2
 *  @param adrs condと一致していた際に呼び出すアドレス
 */
//--------------------------------------------------------------
  .macro  _IFWK_CALL wk1,cond,wk2,adrs
  _CMPWK  \wk1,\wk2
  .short  EV_SEQ_IF_CALL
  .byte  \cond
  .long  ((\adrs-.)-4)
  .endm


//======================================================================
//  イベントフラグ関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _FLAG_SET フラグのセット
 *  @param num セットするフラグナンバー
 */
//--------------------------------------------------------------
#define   _FLAG_SET( num ) _ASM_FLAG_SET num

  .macro  _ASM_FLAG_SET num
  .short  EV_SEQ_FLAG_SET
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _FLAG_RESET フラグのリセット
 *  @param リセットするフラグナンバー
 */
//--------------------------------------------------------------
#define _FLAG_RESET( num ) _ASM_FLAG_RESET num

  .macro  _ASM_FLAG_RESET num
  .short  EV_SEQ_FLAG_RESET
  .short  \num
  .endm


//--------------------------------------------------------------
/**
 *  _FLAG_CHECK フラグチェック
 *  @param チェックするフラグナンバー
 *  @param 結果を格納するワーク
 */
//--------------------------------------------------------------
#define _FLAG_CHECK( num, wk ) _ASM_FLAG_CHECK num,wk
  
  .macro  _ASM_FLAG_CHECK num,wk
  .short  EV_SEQ_FLAG_CHECK
  .short  \num
  .short  \wk
  .endm

//======================================================================
//  ワーク操作関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _ADD_WK ワークに値を足す
 *  @param 値を足すワーク
 *  @param num 足す値
 */
//--------------------------------------------------------------
  .macro  _ASM_ADD_WK wk,num
  .short  EV_SEQ_ADD_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _SUB_WK ワークから値を引く
 *  @param 値を引くワーク
 *  @param num 引く値
 */
//--------------------------------------------------------------
  .macro  _ASM_SUB_WK wk,num
  .short  EV_SEQ_SUB_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _ASM_MUL_WK ワークに値をかける
 *  @param 操作対象のワーク
 *  @param num かける値
 */
//--------------------------------------------------------------
  .macro  _ASM_MUL_WK wk,num
  .short  EV_SEQ_MUL_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _ASM_DIV_WK ワークを値で割る
 *  @param 操作対象のワーク
 *  @param num 割る値
 */
//--------------------------------------------------------------
  .macro  _ASM_DIV_WK wk,num
  .short  EV_SEQ_DIV_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _ASM_MOD_WK ワークの剰余をとる
 *  @param 操作対象のワーク
 *  @param num 割る値
 */
//--------------------------------------------------------------
  .macro  _ASM_MOD_WK wk,num
  .short  EV_SEQ_MOD_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 * _ASM_AND_WK ワークの論理和をとる
 * @param wk  操作対象のワーク
 * @param num   ANDをとる値
 */
//--------------------------------------------------------------
  .macro  _ASM_AND_WK  wk, num
  .short  EV_SEQ_AND_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 * _ASM_OR_WK ワークの論理和をとる
 * @param wk  操作対象のワーク
 * @param num   orをとる値
 */
//--------------------------------------------------------------
  .macro  _ASM_OR_WK  wk, num
  .short  EV_SEQ_OR_WK
  .short  \wk
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _LDVAL ワークに値を代入
 *  @param wk 値を格納するワーク
 *  @param val 格納する値
 */
//--------------------------------------------------------------
#define _LDVAL( wk, val ) _ASM_LDVAL wk,val

  .macro  _ASM_LDVAL  wk,val
  .short  EV_SEQ_LD_WK_VAL
  .short  \wk
  .short  \val
  .endm

//--------------------------------------------------------------
/**
 *  _LDWK ワークにワークの値を代入
 *  @param wk1 代入先ワーク
 *  @param wk2 代入する値が格納されたワーク
 */
//--------------------------------------------------------------
#define _LDWK( wk1, wk2 ) _ASM_LDWK wk1,wk2

  .macro  _ASM_LDWK  wk1,wk2
  .short  EV_SEQ_LD_WK_WK
  .short  \wk1
  .short  \wk2
  .endm

//--------------------------------------------------------------
/**
 *  _LDWKVAL ワークに値かワークの値を代入
 *  @param wk1 代入先ワーク
 *  @param wk2 代入する値もしくはワーク
 */
//--------------------------------------------------------------
  .macro  _ASM_LDWKVAL  wk1,wk2
  .short  EV_SEQ_LD_WK_WKVAL
  .short  \wk1
  .short  \wk2
  .endm

//======================================================================
//  キー入力関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _AB_KEYWAIT キー待ち
 *  @param none
 */
//--------------------------------------------------------------
#define _AB_KEYWAIT() _ASM_AB_KEYWAIT

  .macro  _ASM_AB_KEYWAIT
  .short  EV_SEQ_ABKEYWAIT
  .endm


//--------------------------------------------------------------
/**
 *  _LAST_KEYWAIT メッセージ最後のキー待ち
 *
 *  ※ABキーだけでなく、十字キーでも抜けることができる
 */
//--------------------------------------------------------------
#define _LAST_KEYWAIT() _ASM_LAST_KEYWAIT

  .macro  _ASM_LAST_KEYWAIT
  .short  EV_SEQ_LASTKEYWAIT
  .endm

//--------------------------------------------------------------
/**
 *  _AB_KEYWAIT_MARK 直前に出しているメッセージウィンドウへ
 *  キー待ち記号（▼）を表示させてキー待ち
 *  @param none
 *  @note 注意！待ち記号表示は原則GMM側でつけます。ME再生後、サブウィン表示後のみ使用します。
 *  詳細はスクリプト関連ルールを参照してください。
 */
//--------------------------------------------------------------
#define _AB_KEYWAIT_MARK() _ASM_AB_KEYWAIT_MARK

  .macro  _ASM_AB_KEYWAIT_MARK
  .short  EV_SEQ_ABKEYWAIT_MARK
  .endm

//======================================================================
//  window　自動メッセージ送り＋メッセージスピード一定
//======================================================================
//--------------------------------------------------------------
/**
 *  _MSGWIN_SET_AUTOPRINT 自動メッセージ送り＋メッセージスピード一定　設定
 *
 *  @param  flag      TRUE:設定ON   FALSE:設定OFF
 */
//--------------------------------------------------------------
#define _MSGWIN_SET_AUTOPRINT( flag ) _ASM_MSGWIN_SET_AUTOPRINT flag

  .macro  _ASM_MSGWIN_SET_AUTOPRINT flag
  .short  EV_SEQ_MSGWIN_SET_AUTOPRINT
  .short  \flag
  .endm

//======================================================================
//  フィールド　システムウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * @def _SYSWIN_MSG
 * @brief システムウィンドウでのメッセージ表示
 * @param msg_id  表示するメッセージID
 * @param pos     ウィンドウ表示位置の指定（POS_UP, POS_DOWN, POS_DEFAULT）
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG( msg_id, pos )  _ASM_SYSWIN_MSG msg_id, pos

  .macro  _ASM_SYSWIN_MSG msg_id,up_down
  .short  EV_SEQ_SYSWIN_MSG
  .short  \msg_id
  .short  \up_down
  .endm

//--------------------------------------------------------------
/**
 *  @def  _SYSWIN_MSG_ALLPUT
 *  @brief  展開メッセージを一括表示
 *  @param  msg_id  表示するメッセージid
 *  @param  pos     ウィンドウ表示位置の指定（POS_UP, POS_DOWN, POS_DEFAULT）
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG_ALLPUT( msg_id, pos ) \
    _ASM_SYSWIN_MSG_ALLPUT msg_id, pos

//--------------------------------------------------------------
/**
 *  _SYSWIN_MSG_ALLPUT_DOWN 展開メッセージを一括表示(1byte) ウィンドウ下
 *  @param  msg_id  表示するメッセージid
 *
 *  _SYSWIN_MSG_ALLPUTに統合し、_SYSWIN_MSG_ALLPUT_DOWNは廃止予定です。
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG_ALLPUT_DOWN( msg_id ) \
    _ASM_SYSWIN_MSG_ALLPUT msg_id, POS_DOWN
  
  .macro  _ASM_SYSWIN_MSG_ALLPUT msg_id, up_down
  .short  EV_SEQ_SYSWIN_MSG_ALLPUT
  .short  \msg_id
  .short  \up_down
  .endm

//--------------------------------------------------------------
/**
 *  @def    _SYSWIN_CLOSE
 *  @brief  システムウィンドウ閉じる    （タイムアイコンも同時に閉じます）
 *  @param none
 */
//--------------------------------------------------------------
#define _SYSWIN_CLOSE() _ASM_SYSWIN_CLOSE

  .macro  _ASM_SYSWIN_CLOSE
  .short  EV_SEQ_SYSWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 *  @def    _SYSWIN_TIMEICON
 *  @brief  システムウィンドウタイムアイコンを表示
 *  @param flag 
            SCR_SYSWIN_TIMEICON_ON  (0)   表示
            SCR_SYSWIN_TIMEICON_OFF (1)   非表示
 */
//--------------------------------------------------------------
#define _SYSWIN_TIMEICON( flag ) _ASM_SYSWIN_TIMEICON flag

  .macro  _ASM_SYSWIN_TIMEICON flag
  .short  EV_SEQ_SYSWIN_TIMEICON
  .short  \flag
  .endm

//======================================================================
//  フィールド　吹き出しウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  @def  _BALLOONWIN_OBJMSG
 *  @brief  吹き出しウィンドウ描画
 *  @param msg_id 表示するメッセージID
 *  @param obj_id 吹き出しを出す対象OBJ ID
 */
//--------------------------------------------------------------
#define _BALLOONWIN_OBJMSG( msg_id , obj_id ) \
    _ASM_BALLOONWIN_OBJMSG msg_id, obj_id

  .macro _ASM_BALLOONWIN_OBJMSG msg_id, obj_id
  .short  EV_SEQ_BALLOONWIN_OBJMSG
  .short 0x0400
  .short \msg_id
  .short \obj_id
  .short POS_DEFAULT
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 *  @def  _BALLOONWIN_TALKOBJ_MSG
 *  @brief  吹き出しウィンドウ描画　話し掛けOBJ専用
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _BALLOONWIN_TALKOBJ_MSG( msg_id ) \
    _ASM_BALLOONWIN_TALKOBJ_MSG msg_id
  
  .macro _ASM_BALLOONWIN_TALKOBJ_MSG msg_id
  .short  EV_SEQ_BALLOONWIN_TALKOBJ_MSG
  .short 0x0400
  .short \msg_id
  .short POS_DEFAULT
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 * @def _BALLOONWIN_TALKOBJ_MSG_ARC
 * @brief 吹き出しウィンドウ描画 話しかけOBJ専用、メッセージアーカイブ指定付き
 *  @param  arc_id  メッセージアーカイブ指定ID
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _BALLOONWIN_TALKOBJ_MSG_ARC( arc_id, msg_id ) \
    _ASM_BALLOONWIN_TALKOBJ_MSG_ARC arc_id, msg_id

  .macro  _ASM_BALLOONWIN_TALKOBJ_MSG_ARC arc_id, msg_id
  .short  EV_SEQ_BALLOONWIN_TALKOBJ_MSG
  .short \arc_id
  .short \msg_id
  .short POS_DEFAULT
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 *  @def  _BALLOONWIN_OBJMSG_POS
 *  @brief  吹き出しウィンドウ描画　位置指定あり
 *  @param msg_id 表示するメッセージID
 *  @param obj_id 吹き出しを出す対象OBJ ID
 *  @param pos 吹き出しウィンドウ位置 usescript.hのPOS_定義を参照
 */
//--------------------------------------------------------------
#define _BALLOONWIN_OBJMSG_POS( msg_id , obj_id, pos ) \
    _ASM_BALLOONWIN_OBJMSG_POS msg_id, obj_id, pos
  
  .macro _ASM_BALLOONWIN_OBJMSG_POS msg_id, obj_id, pos
  .short  EV_SEQ_BALLOONWIN_OBJMSG
  .short 0x0400
  .short \msg_id
  .short \obj_id
  .short \pos
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 *  @def  _BALLOONWIN_OBJMSG_MF
 *  @brief  吹き出しウィンドウ描画　プレイヤー性別メッセージ指定
 *  @param msg_id_m 表示するメッセージID　男性
 *  @param msg_id_f 表示するメッセージID　女性
 *  @param obj_id 吹き出しを出す対象OBJ ID
 *  @param pos 吹き出しウィンドウ位置 usescript.hのPOS_定義を参照
 */
//--------------------------------------------------------------
#define _BALLOONWIN_OBJMSG_MF( msg_id_m, msg_id_f, obj_id, pos ) \
    _ASM_BALLOONWIN_OBJMSG_MF msg_id_m, msg_id_f, obj_id, pos

  .macro _ASM_BALLOONWIN_OBJMSG_MF msg_id_m, msg_id_f, obj_id, pos
  .short  EV_SEQ_BALLOONWIN_OBJMSG_MF
  .short 0x0400
  .short \msg_id_m
  .short \msg_id_f
  .short \obj_id
  .short \pos
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 *  @def  _BALLOONWIN_OBJMSG_WB
 *  @brief  吹き出しウィンドウ描画　バージョン別
 *  @param msg_id_w 表示するメッセージID、ホワイト版
 *  @param msg_id_b 表示するメッセージID、ブラック版
 *  @param obj_id 吹き出しを出す対象OBJ ID
 *  @param pos 吹き出しウィンドウ位置 usescript.hのPOS_定義を参照
 */
//--------------------------------------------------------------
#define _BALLOONWIN_OBJMSG_WB( msg_id_w, msg_id_b, obj_id, pos ) \
    _ASM_BALLOONWIN_OBJMSG_WB msg_id_w, msg_id_b, obj_id, pos

  .macro _ASM_BALLOONWIN_OBJMSG_WB msg_id_w, msg_id_b, obj_id, pos
  .short  EV_SEQ_BALLOONWIN_OBJMSG_WB
  .short 0x0400
  .short \msg_id_w
  .short \msg_id_b
  .short \obj_id
  .short \pos
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 *  @def  _GIZAWIN_OBJMSG_POS
 *  @brief  吹き出しウィンドウ描画　ギザギザ　位置指定あり
 *  @param msg_id 表示するメッセージID
 *  @param obj_id 吹き出しを出す対象OBJ ID
 *  @param pos 吹き出しウィンドウ位置 usescript.hのPOS_定義を参照
 */
//--------------------------------------------------------------
#define _GIZAWIN_OBJMSG_POS( msg_id , obj_id, pos ) \
    _ASM_GIZAWIN_OBJMSG_POS msg_id, obj_id, pos
  
  .macro _ASM_GIZAWIN_OBJMSG_POS msg_id, obj_id, pos
  .short  EV_SEQ_BALLOONWIN_OBJMSG
  .short 0x0400
  .short \msg_id
  .short \obj_id
  .short \pos
  .short BALLOONWIN_TYPE_GIZA
  .endm

//--------------------------------------------------------------
/**
 *  _BALLOONWIN_CLOSE 吹き出しウィンドウ閉じる
 *  @param none
 */
//--------------------------------------------------------------
#define _BALLOONWIN_CLOSE() _ASM_BALLOONWIN_CLOSE

  .macro  _ASM_BALLOONWIN_CLOSE
  .short  EV_SEQ_BALLOONWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 *  _GIZAWIN_CLOSE 吹き出しウィンドウ閉じる
 *  @param none
 */
//--------------------------------------------------------------
#define _GIZAWIN_CLOSE() _ASM_BALLOONWIN_CLOSE

//======================================================================
//  所持金ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  @def _GOLD_WIN_OPEN
 *  @brief 所持金ウィンドウを表示する　右詰表示
 *  @param x 表示x位置(キャラ単位)
 *  @param y 表示x位置(キャラ単位)
 *  @param none
 */
//--------------------------------------------------------------
#define _GOLD_WIN_OPEN( x, y ) \
    _ASM_GOLD_WIN_OPEN x, y

  .macro _ASM_GOLD_WIN_OPEN x, y
  .short EV_SEQ_GOLD_WIN_OPEN
  .short \x
  .short \y
  .endm

//--------------------------------------------------------------
/**
 *  @def _GOLD_WIN_CLOSE
 *  @brief 所持金ウィンドウを消去する
 *  @param none
 */
//--------------------------------------------------------------
#define _GOLD_WIN_CLOSE() \
    _ASM_GOLD_WIN_CLOSE

  .macro _ASM_GOLD_WIN_CLOSE
  .short EV_SEQ_GOLD_WIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 *  @def _GOLD_WIN_UPDATE
 *  @brief 所持金ウィンドウを更新する
 *  @param none
 */
//--------------------------------------------------------------
#define _GOLD_WIN_UPDATE() \
    _ASM_GOLD_WIN_UPDATE

  .macro _ASM_GOLD_WIN_UPDATE
  .short EV_SEQ_GOLD_WIN_UPDATE
  .endm

//======================================================================
//  メッセージウィンドウ共通
//======================================================================
//--------------------------------------------------------------
/**
 *  @def _MSGWIN_CLOSE
 *  @brief 種類を問わず表示しているメッセージウィンドウを閉じる。
 *  @param none
 *  @note 該当ウィンドウは、
 *  システムウィンドウ、吹き出しウィンドウ、プレーンウィンドウ、BGウィンドウ
 */
//--------------------------------------------------------------
#define _MSGWIN_CLOSE() _ASM_MSGWIN_CLOSE

  .macro _ASM_MSGWIN_CLOSE
  .short EV_SEQ_MSGWIN_CLOSE
  .endm

//======================================================================
//  プレーンウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  @def _PLAINWIN_MSG_UP
 *  @brief プレーンウィンドウを上に表示
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _PLAINWIN_MSG_UP( msg_id ) _ASM_PLAINWIN_MSG msg_id, POS_UP

//--------------------------------------------------------------
/**
 *  @def _PLAINWIN_MSG_DOWN
 *  @brief プレーンウィンドウを下に表示
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _PLAINWIN_MSG_DOWN( msg_id ) _ASM_PLAINWIN_MSG msg_id, POS_DOWN
  
  .macro _ASM_PLAINWIN_MSG msg_id, up_down
  .short EV_SEQ_PLAINWIN_MSG
  .short \msg_id
  .byte \up_down
  .endm

//--------------------------------------------------------------
/**
 *  @def _PLAINWIN_CLOSE
 *  @brief プレーンウィンドウを閉じる
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _PLAINWIN_CLOSE() _ASM_PLAINWIN_CLOSE
  
  .macro _ASM_PLAINWIN_CLOSE
  .short EV_SEQ_PLAINWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 *  @def _GIZAPLAINWIN_MSG_POS
 *  @brief ギザプレーンウィンドウを表示
 *  @param msg_id 表示するメッセージID
 *  @param pos POS_UP,POS_DOWN
 */
//--------------------------------------------------------------
#define _GIZAPLAINWIN_MSG_POS( msg_id, pos ) _ASM_GIZAPLAINWIN_MSG_POS msg_id, pos
  
  .macro _ASM_GIZAPLAINWIN_MSG_POS msg_id, pos
  .short EV_SEQ_PLAINWIN_GIZA_MSG
  .short \msg_id
  .byte \pos
  .endm

//--------------------------------------------------------------
/**
 *  @def _GIZAPLAINWIN_CLOSE
 *  @brief ギザプレーンウィンドウを閉じる
 *  @param none
 */
//--------------------------------------------------------------
#define _GIZAPLAINWIN_CLOSE() _ASM_PLAINWIN_CLOSE

//======================================================================
//  サブウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  @def _SUBWIN_MSG
 *  @brief サブウィンドウを表示
 *  @param msg_id 表示するメッセージID
 *  @param pos_x 表示するX座標（キャラ単位
 *  @param pos_y 表示するY座標（キャラ単位
 *  @param win_id 識別の為に登録する任意のID
 */
//--------------------------------------------------------------
#define _SUBWIN_MSG( msg_id, pos_x, pos_y, win_id ) \
    _ASM_SUBWIN_MSG msg_id, pos_x, pos_y, win_id
  
  .macro _ASM_SUBWIN_MSG  msg_id, pos_x, pos_y, win_id
  .short EV_SEQ_SUBWIN_MSG
  .short \msg_id
  .short \pos_x
  .short \pos_y
  .short \win_id
  .endm

//--------------------------------------------------------------
/**
 *  @def _SUBWIN_CLOSE
 *  @brief サブウィンドウを閉じる
 *  @param win_id _SUBWIN_MSG()で指定したwin_id
 */
//--------------------------------------------------------------
#define _SUBWIN_CLOSE( win_id ) _ASM_SUBWIN_CLOSE win_id
  
  .macro _ASM_SUBWIN_CLOSE win_id
  .short EV_SEQ_SUBWIN_CLOSE
  .short \win_id
  .endm

//======================================================================
//  BGウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  @def _BGWIN_MSG
 *  @brief BGウィンドウを表示する
 *  @param msg_id 表示するメッセージID
 *  @param bg_type 表示するタイプ TYPE_INFO,TYPE_TOWN,TYPE_POST,TYPE_ROAD
 */
//--------------------------------------------------------------
#define _BGWIN_MSG( msg_id, bg_type ) _ASM_BGWIN_MSG msg_id, bg_type
  
  .macro _ASM_BGWIN_MSG msg_id, bg_type
  .short EV_SEQ_BGWIN_MSG
  .short \msg_id
  .short \bg_type
  .endm

//--------------------------------------------------------------
/**
 *  @def _BGWIN_SEATEMPLE_MSG
 *  @brief 海底神殿　暗号　BGウィンドウを表示する
 *  @param msg_id 表示するメッセージID
 *  @param bg_type 表示するタイプ TYPE_INFO,TYPE_TOWN,TYPE_POST,TYPE_ROAD
 */
//--------------------------------------------------------------
#define _BGWIN_SEATEMPLE_MSG( msg_id, bg_type ) _ASM_BGWIN_SEATEMPLE_MSG msg_id, bg_type
  
  .macro _ASM_BGWIN_SEATEMPLE_MSG msg_id, bg_type
  .short EV_SEQ_BGWIN_SEATEMPLE_MSG
  .short \msg_id
  .short \bg_type
  .endm

//--------------------------------------------------------------
/**
 *  @def _BGWIN_CLOSE
 *  @brief BGウィンドウを閉じる
 *  @param none 
 */
//--------------------------------------------------------------
#define _BGWIN_CLOSE()  _ASM_BGWIN_CLOSE
  
  .macro _ASM_BGWIN_CLOSE
  .short EV_SEQ_BGWIN_CLOSE
  .endm

//======================================================================
//  特殊ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  @def _SPWIN_MSG
 *  @brief 特殊ウィンドウを表示する
 *  @param msg_id 表示するメッセージID
 *  @param x ウィンドウ左上X座標 キャラ単位
 *  @param y ウィンドウ左上Y座標 キャラ単位
 *  @param type 表示するタイプ SPWIN_LETTER SPWIN_BOOK等
 */
//--------------------------------------------------------------
#define _SPWIN_MSG( msg_id, x, y, type ) _ASM_SPWIN_MSG msg_id, x, y, type
  
  .macro _ASM_SPWIN_MSG msg_id, x, y, type
  .short EV_SEQ_SPWIN_MSG
  .short \msg_id
  .byte \x
  .byte \y
  .short \type
  .endm

//--------------------------------------------------------------
/**
 *  @def _SPWIN_CLOSE
 *  @brief 特殊ウィンドウを閉じる
 *  @param none 
 */
//--------------------------------------------------------------
#define _SPWIN_CLOSE()  _ASM_SPWIN_CLOSE
  
  .macro _ASM_SPWIN_CLOSE
  .short EV_SEQ_SPWIN_CLOSE
  .endm


//======================================================================
//  動作モデル
//======================================================================
//--------------------------------------------------------------
/**
 *  _ANIME_LABEL アニメーションデータラベル
 *  @param label データラベル名
 */
//--------------------------------------------------------------
  .macro  _ANIME_LABEL label
  .align  4
  \label:
  .endm

//--------------------------------------------------------------
/**
 *  _ANIME_DATA アニメーションデータ(fldmmdl_code.h参照)
 *  @param code アニメーションコード
 *  @param num code実行回数
 */
//--------------------------------------------------------------
  .macro  _ANIME_DATA  code,num
  .short  \code
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _OBJ_ANIME フィールドOBJ アニメーションコマンド
 *  @param obj_id アニメーションを行うOBJ ID
 *  @param adrs アニメーションコマンドデータラベル
 *
 *  @note 注意！ 動作停止をセットしたい時は、
 *  _OBJ_PAUSE_ALLをもう一度呼ばないといけない！
 */
//--------------------------------------------------------------
#define _OBJ_ANIME( obj_id, adrs ) _ASM_OBJ_ANIME obj_id, adrs

  .macro  _ASM_OBJ_ANIME obj_id, adrs
  .short  EV_SEQ_OBJ_ANIME
  .short  \obj_id 
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _OBJ_ANIME_WAIT アニメーションコマンド終了待ち
 *  @param none
 */
//--------------------------------------------------------------
#define _OBJ_ANIME_WAIT() _ASM_OBJ_ANIME_WAIT

  .macro  _ASM_OBJ_ANIME_WAIT
  .short  EV_SEQ_OBJ_ANIME_WAIT
  .endm
  
//--------------------------------------------------------------
/**
 *  _MOVE_CODE_GET 動作コード取得(fldmmdl_code.h)
 *  @param ret_wk 動作コードを格納するワーク
 *  @param obj_id 動作コードを取得するOBJ ID
 */
//--------------------------------------------------------------
#define  _MOVE_CODE_GET(  ret_wk,obj_id ) _ASM_MOVE_CODE_GET ret_wk, obj_id

  .macro  _ASM_MOVE_CODE_GET  ret_wk, obj_id
  .short  EV_SEQ_MOVE_CODE_GET 
  .short  \ret_wk
  .short  \obj_id
  .endm
  
//--------------------------------------------------------------
/**
 *  _MOVE_CODE_CHANGE 動作コードを変更(fldmmdl_code.h)
 *  @param obj_id 動作コードを変更するOBJ ID
 *  @param code 変更する動作コード
 */
//--------------------------------------------------------------
#define  _MOVE_CODE_CHANGE( obj_id, code ) _ASM_MOVE_CODE_CHANGE obj_id, code

  .macro  _ASM_MOVE_CODE_CHANGE obj_id, code
  .short  EV_SEQ_MOVE_CODE_CHANGE
  .short  \obj_id
  .short  \code
  .endm

//--------------------------------------------------------------
/**
 * @def _OBJ_POS_GET
 * @brief OBJの位置取得
 * @param obj_id 取得するOBJ ID
 * @param x X座標格納先
 * @param z Z座標格納先
 */
//--------------------------------------------------------------
#define _OBJ_POS_GET( obj_id,x,z ) _ASM_OBJ_POS_GET obj_id, x, z

  .macro  _ASM_OBJ_POS_GET  obj_id, x, z
  .short  EV_SEQ_OBJ_POS_GET
  .short  \obj_id 
  .short  \x
  .short  \z
  .endm

//--------------------------------------------------------------
/**
 * @def _PLAYER_POS_GET
 * @brief 主人公の位置取得
 * @param x X座標格納先
 * @param z Z座標格納先
 */
//--------------------------------------------------------------
#define _PLAYER_POS_GET( x, z ) _ASM_PLAYER_POS_GET x, z

  .macro  _ASM_PLAYER_POS_GET x, z
  .short  EV_SEQ_PLAYER_POS_GET
  .short  \x
  .short  \z
  .endm

//--------------------------------------------------------------
/**
 * @def _PLAYER_DIR_GET
 * @brief プレイヤーの方向取得
 * @param ret_wk    方向を受け取るワーク
 *
 * @note  方向についてはprog/include/field/field_dir.hを参照
 * @todo  グリッドマップのみでレールマップのことは考慮されていない
 */
//--------------------------------------------------------------
#define _PLAYER_DIR_GET( ret_wk ) \
    _ASM_PLAYER_DIR_GET ret_wk

    .macro  _ASM_PLAYER_DIR_GET ret_wk
    .short  EV_SEQ_PLAYER_DIR_GET
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_OBJID
 * @brief 指定した座標にあるオブジェIDを取得する
 * @param ret_id    オブジェIDを受け取るワーク
 * @param ret_valid オブジェが存在するかどうかを受け取るワーク
 * @param gx        x座標[grid]
 * @param gy        y座標[grid]
 * @param gz        z座標[grid]
 * @return オブジェが存在する場合,   ret_valid = TRUE, ret_id = オブジェID
 *         オブジェが存在しない場合, ret_valid = FALSE
 */
//--------------------------------------------------------------
#define _GET_OBJID( ret_id, ret_valid, gx, gy, gz ) \
    _ASM_GET_OBJID ret_id, ret_valid, gx, gy, gz

    .macro  _ASM_GET_OBJID ret_id, ret_valid, gx, gy, gz 
    .short  EV_SEQ_GET_OBJID
    .short  \ret_id
    .short  \ret_valid
    .short  \gx
    .short  \gy
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_FRONT_OBJID
 * @brief プレイヤーの前方1グリッドにあるオブジェIDを取得する
 * @param ret_id    オブジェIDを受け取るワーク
 * @param ret_valid オブジェが存在するかどうかを受け取るワーク
 * @return オブジェが存在する場合,   ret_valid = TRUE, ret_id = オブジェID
 *         オブジェが存在しない場合, ret_valid = FALSE
 */
//--------------------------------------------------------------
#define _GET_FRONT_OBJID( ret_id, ret_valid ) \
    _ASM_GET_FRONT_OBJID ret_id, ret_valid

    .macro  _ASM_GET_FRONT_OBJID ret_id, ret_valid
    .short  EV_SEQ_GET_FRONT_OBJID
    .short  \ret_id
    .short  \ret_valid
    .endm

//--------------------------------------------------------------
/**
 * @def _OBJ_GET_PARAM
 * @brief 動作モデルのパラメータを取得する
 * @param obj_id  対象のOBJID
 * @param prm_no  取得するパラメータの指定（script_def.hのSCR_OBJPARAM?を参照）
 * @param ret_wk  結果を受け取るためのワーク
 */
//--------------------------------------------------------------
#define _OBJ_GET_PARAM( obj_id, prm_no, ret_wk )  \
      _ASM_OBJ_GET_PARAM obj_id, prm_no, ret_wk
      .macro  _ASM_OBJ_GET_PARAM obj_id, prm_no, ret_wk
      .short  EV_SEQ_OBJ_PARAM_GET
      .short  \obj_id
      .short  \prm_no
      .short  \ret_wk
      .endm

//--------------------------------------------------------------
/**
 * _GET_OBJ_EV_FLAG 指定のOBJに指定されているイベントフラグを取得
 * @param id OBJ ID
 * @param ret_id フラグ格納先
 */
//--------------------------------------------------------------
#define _GET_OBJ_EV_FLAG( id, ret_id ) \
  _ASM_GET_OBJ_EV_FLAG id, ret_id

  .macro _ASM_GET_OBJ_EV_FLAG id, ret_id
  .short EV_SEQ_OBJ_EV_FLAG_GET   
  .short \id
  .short \ret_id
  .endm

//--------------------------------------------------------------
/**
 * _OBJ_ADD_EX OBJを追加。
 * @param x 表示するグリッドX座標
 * @param z 表示するグリッドZ座標
 * @param dir 方向 DIR_UP等
 * @param id 識別用OBJ ID
 * @param code 表示コードBOY1等
 * @param move 動作コードMV_RND等
 * @note 識別用IDは MMDL_ID_EVENTOBJ_00 を使います。原則、このコマンドで追加するOBJは同時に１人まで
 * @note 処理が実行されてから画面に表示されるまでに１フレーム以上かかる可能性があります。暗転中に使う際は、暗転復帰してから表示されていないか注意。
 */
//--------------------------------------------------------------
#define _OBJ_ADD_EX( x,z,dir,id,code,move ) _ASM_OBJ_ADD_EX x, z, dir, id, code, move

.macro _ASM_OBJ_ADD_EX x, z, dir, id, code, move
  .short EV_SEQ_OBJ_ADD
  .short \x
  .short \z
  .short \dir
  .short \id
  .short \code
  .short \move
.endm

//--------------------------------------------------------------
/**
 * _OBJ_ADD ゾーン内で配置されているOBJ一覧内、指定のIDを持つOBJを追加
 * @param id 配置OBJ内で追加するOBJ ID
 * @note 注意！追加したOBJの動作停止処理が含まれています。フィールド初期化スクリプト内で追加する場合はポーズ解除を呼んでください。
 * @note 処理が実行されてから画面に表示されるまでに１フレーム以上かかる可能性があります。暗転中に使う際は、暗転復帰してから表示されていないか注意。
 */
//--------------------------------------------------------------
#define _OBJ_ADD( id ) \
  _ASM_OBJ_ADD id

  .macro _ASM_OBJ_ADD id
  .short EV_SEQ_OBJ_ADD_EV
  .short \id
  .endm

//--------------------------------------------------------------
/**
 * _OBJ_DEL 指定のOBJ_IDを持ったOBJを削除。
 * @param id 配置OBJ内で追加するOBJ ID
 * @attention OBJを削除するのみのコマンド。OBJが持つフラグ等の操作は無し。
 */
//--------------------------------------------------------------
#define _OBJ_DEL( id )  \
    _ASM_OBJ_DEL id

  .macro _ASM_OBJ_DEL id
  .short EV_SEQ_OBJ_DEL
  .short \id
  .endm

//--------------------------------------------------------------
/**
 * _OBJ_POS_CHANGE 指定のOBJ_IDを持ったOBJの座標位置を変更。
 * @param id 座標変更するOBJ ID
 * @param gx グリッドX座標
 * @param gy グリッドY座標
 * @param gz グリッドZ座標
 * @param dir 方向 DIR_UP等
 */
//--------------------------------------------------------------
#define _OBJ_POS_CHANGE( id, gx, gy, gz, dir )  \
    _ASM_OBJ_POS_CHANGE id, gx, gy, gz, dir

	.macro	_ASM_OBJ_POS_CHANGE	id,gx,gy,gz,dir
	.short	EV_SEQ_OBJ_POS_CHANGE
	.short	\id
	.short	\gx
	.short	\gy
	.short	\gz
	.short	\dir
	.endm

//--------------------------------------------------------------
/**
 * @def _PLAYER_REQUEST
 * @brief 自機にリクエストを出す
 * @param reqbit FIELD_PLAYER_REQBIT_NORMAL等 C:\home\pokemon_wb\prog\src\field\field_player_code.h
 */
//--------------------------------------------------------------
#define _PLAYER_REQUEST( reqbit ) _ASM_PLAYER_REQUEST reqbit

  .macro  _ASM_PLAYER_REQUEST reqbit
  .short  EV_SEQ_PLAYER_REQUEST
  .short  \reqbit
  .endm

//--------------------------------------------------------------
/**
 * @def _PLAYER_UPDOWN_EFFECT
 * @brief 自機の高さ変化演出
 * @param type  　動かす種類0の時原点から、1の時逆算した位置から
 * @param frame   動かす時間
 * @param length  移動距離（3D座標単位）
 * @param way     移動方向 0=↓  1=↑
 *
 * @note
 * 自機の内部情報を操作して戻す処理がないため使用箇所は要注意！
 */
//--------------------------------------------------------------
#define _PLAYER_UPDOWN_EFFECT( type, frame, length, way ) \
    _ASM_PLAYER_UPDOWN_EFFECT type, frame, length, way

  .macro  _ASM_PLAYER_UPDOWN_EFFECT type, frame, length, way
  .short  EV_SEQ_PLAYER_UPDOWN_EFFECT
  .short  \type
  .short  \frame
  .short  \length
  .short  \way
  .endm

//--------------------------------------------------------------
/**
 * @def _PLAYER_UPDOWN_EFFECT_POS
 * @brief 自機の高さ変化
 * @param type  　動かす種類0の時原点から、1の時逆算した位置から
 * @param frame   動かす時間
 * @param length  移動距離（3D座標単位）
 * @param way     移動方向 0=↓  1=↑
 *
 * @note
 * 自機の内部情報を操作して戻す処理がないため使用箇所は要注意！
 */
//--------------------------------------------------------------
#define _PLAYER_UPDOWN_POS( type, frame, length, way ) \
    _ASM_PLAYER_UPDOWN_POS type, frame, length, way

  .macro  _ASM_PLAYER_UPDOWN_POS type, frame, length, way
  .short  EV_SEQ_PLAYER_UPDOWN_POS
  .short  \type
  .short  \frame
  .short  \length
  .short  \way
  .endm

//--------------------------------------------------------------
/**
 * @def _POS_PLAYER_TURN
 * @brief 自機の高さ変化演出
 * @param type  　動かす種類
 * @param frame   動かす長さ
 * @param length
 */
//--------------------------------------------------------------
#define _POS_PLAYER_TURN() _ASM_POS_PLAYER_TURN

  .macro  _ASM_POS_PLAYER_TURN
  .short  EV_SEQ_POS_PLAYER_TURN
  .endm

//--------------------------------------------------------------
/**
 * @def _PLAYER_FORM_GET
 * @brief 自機の動作形態を取得
 * @param ret_wk 形態格納先
 * SCR_PLAYER_FORM_NORMAL　二足
 * SCR_PLAYER_FORM_CYCLE　自転車
 * SCR_PLAYER_FORM_SWIM　波乗り
 */
//--------------------------------------------------------------
#define _PLAYER_FORM_GET( ret_wk ) _ASM_PLAYER_FORM_GET ret_wk

  .macro _ASM_PLAYER_FORM_GET ret_wk
  .short EV_SEQ_GET_PLAYER_FORM
  .short \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @brief   自機のスクリプト用レールロケーション取得　！レールマップ専用！
 * @param index   戻り値１　インデックス
 * @param front   戻り値２  フロントインデックス
 * @param side    戻り値３  サイドインデックス
 */
//--------------------------------------------------------------
#define _GET_PLAYER_RAIL_LOC( index, front, side ) \
  _ASM_GET_PLAYER_RAIL_LOC index, front, side

  .macro  _ASM_GET_PLAYER_RAIL_LOC index, front, side
  .short  EV_SEQ_GET_PLAYER_RAIL_LOC
  .short  \index
  .short  \front
  .short  \side
  .endm

//--------------------------------------------------------------
/**
 * @brief   動作モデルのスクリプト用レールロケーション取得　！レールマップ専用！
 * @param id 座標変更するOBJ ID
 * @param index   戻り値１　インデックス
 * @param front   戻り値２  フロントインデックス
 * @param side    戻り値３  サイドインデックス
 */
//--------------------------------------------------------------
#define _GET_OBJ_RAIL_LOC( id, index, front, side ) \
  _ASM_GET_OBJ_RAIL_LOC id, index, front, side

  .macro  _ASM_GET_OBJ_RAIL_LOC id, index, front, side
  .short  EV_SEQ_GET_OBJ_RAIL_LOC
  .short  \id
  .short  \index
  .short  \front
  .short  \side
  .endm

//--------------------------------------------------------------
/**
 * @brief 動作モデルのワープによる消去
 * @param obj_id 適用するOBJID
 */
//--------------------------------------------------------------
#define _OBJ_WARP_OUT( obj_id ) \
    _ASM_OBJ_WARP_OUT obj_id

  .macro  _ASM_OBJ_WARP_OUT obj_id
  .short  EV_SEQ_OBJ_WARP_OUT
  .short  \obj_id
  .endm

//--------------------------------------------------------------
/**
 * @brief 動作モデルの落下による登場
 * @param obj_id 適用するOBJID
 * @param grid_x 着地地点の x 座標 ( グリッド単位 )
 * @param grid_z 着地地点の z 座標 ( グリッド単位 )
 * @param dir    動作モデルの向き ( DIR_xxxx )
 */
//--------------------------------------------------------------
#define _OBJ_FALL_IN( obj_id, grid_x, grid_z, dir ) \
    _ASM_OBJ_FALL_IN obj_id, grid_x, grid_z, dir

  .macro  _ASM_OBJ_FALL_IN obj_id, grid_x, grid_z, dir
  .short  EV_SEQ_OBJ_FALL_IN
  .short  \obj_id
  .short  \grid_x
  .short  \grid_z
  .short  \dir
  .endm

//--------------------------------------------------------------
/**
 * @brief 動作モデルのワープによる消去
 * @param obj_id 適用するOBJID
 */
//--------------------------------------------------------------
#define _OBJ_SET_NOT_DEL_ZONECHG( obj_id ) \
    _ASM_OBJ_SET_NOT_DEL_ZONECHG obj_id
  
  .macro _ASM_OBJ_SET_NOT_DEL_ZONECHG obj_id
  .short EV_SEQ_OBJ_SET_NOT_DEL_ZONECHG
  .short \obj_id
  .endm
  
//--------------------------------------------------------------
/**
 * @brief 自機の表示コードが自機専用のものかどうか
 * @param ret_wk チェック結果 TRUE=自機専用 FALSE=自機以外のコード
 */
//--------------------------------------------------------------
#define _CHECK_PLAYER_ILLEGAL_OBJCODE( ret_wk ) \
    _ASM_CHECK_PLAYER_ILLEGAL_OBJCODE ret_wk
  
  .macro _ASM_CHECK_PLAYER_ILLEGAL_OBJCODE ret_wk
  .short EV_SEQ_CHECK_PLAYER_ILLEGAL_OBJCODE
  .short \ret_wk
  .endm

//======================================================================
//  動作モデル　イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _TURN_HERO_SITE 話しかけたOBJを自機方向へ振り向き
 *  @param none
 */
//--------------------------------------------------------------
#define _TURN_HERO_SITE() _ASM_TURN_HERO_SITE

.macro  _ASM_TURN_HERO_SITE
.short  EV_SEQ_OBJ_TURN
.endm

//======================================================================
//  はい、いいえ　処理
//======================================================================
//--------------------------------------------------------------
/**
 *  _YES_NO_WIN はい、いいえ選択
 *  @param ret_wk はい、いいえ選択結果格納先
*  @retval SCR_YES (0)のとき、「はい」を選択
*  @retval SCR_NO (1)のとき、「いいえ」またはキャンセル
 */
//--------------------------------------------------------------
#define _YES_NO_WIN( ret_wk ) _ASM_YES_NO_WIN ret_wk

  .macro  _ASM_YES_NO_WIN ret_wk
  .short  EV_SEQ_YES_NO_WIN
  .short  \ret_wk
  .endm

//======================================================================
//  WORDSET関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _PLAYER_NAME 主人公名を指定タグにセット
 *  @param idx セットするタグナンバー
 */
//--------------------------------------------------------------
#define _PLAYER_NAME( idx ) _ASM_PLAYER_NAME idx

  .macro  _ASM_PLAYER_NAME idx
  .short  EV_SEQ_PLAYER_NAME
  .byte   \idx
  .endm

//--------------------------------------------------------------
/**
 *  _ITEM_NAME アイテム名を指定タグにセット
 *  @param idx セットするタグナンバー
 *  @param itemno 名前を取得するアイテムナンバー
 */
//--------------------------------------------------------------
#define _ITEM_NAME( idx, itemno ) _ASM_ITEM_NAME idx, itemno
  
  .macro _ASM_ITEM_NAME idx, itemno
  .short EV_SEQ_ITEM_NAME
  .byte \idx
  .short \itemno
  .endm

//--------------------------------------------------------------
/**
 *	技マシンの技名を指定タグにセット
 *  @param idx セットするタグナンバー
 *  @param itemno 名前を取得するアイテムナンバー
 */
//--------------------------------------------------------------
#define _ITEM_WAZA_NAME( idx, itemno ) _ASM_ITEM_WAZA_NAME idx, itemno

	.macro	_ASM_ITEM_WAZA_NAME	idx,itemno
	.short	EV_SEQ_ITEM_WAZA_NAME
	.byte	\idx
	.short	\itemno
	.endm

//--------------------------------------------------------------
/**
 *	技名を指定タグにセット
 *  @param idx セットするタグナンバー
 *  @param wazano 名前を取得する技番号
 */
//--------------------------------------------------------------
#define _WAZA_NAME( idx, waza ) _ASM_WAZA_NAME idx,waza

	.macro	_ASM_WAZA_NAME	idx,waza
	.short	EV_SEQ_WAZA_NAME
	.byte	\idx
	.short	\waza
	.endm

//--------------------------------------------------------------
/**
 * @def _POKETYPE_NAME
 * @brief ポケモン・わざのタイプ名を指定タグにセット
 *  @param idx セットするタグナンバー
 *  @param poketype 名前を取得するタイプ指定ID
 *  @note
 *  引数poketypeに渡す定義名は C:\home\pokemon_wb\prog\include\poke_tool\poketype_def.h参照
 */
//--------------------------------------------------------------
#define _POKETYPE_NAME( idx, poketype ) \
    _ASM_POKETYPE_NAME idx, poketype

    .macro  _ASM_POKETYPE_NAME idx, poketype
    .short  EV_SEQ_POKETYPE_NAME
    .byte   \idx
    .short  \poketype
    .endm

//--------------------------------------------------------------
/**
 * @def _MONS_NAME
 * @brief ずかんナンバーからポケモン名を指定タグにセット
 *  @param idx セットするタグナンバー
 *  @param monsno 名前を取得するポケモンのずかんナンバー
 */
//--------------------------------------------------------------
#define _MONS_NAME( idx, monsno ) \
    _ASM_MONS_NAME idx, monsno

    .macro  _ASM_MONS_NAME idx, monsno
    .short  EV_SEQ_MONS_NAME
    .byte   \idx
    .short  \monsno
    .endm

//--------------------------------------------------------------
/**
 * @def _PLACE_NAME
 * @brief 地名を指名タグにセット
 * @param idx     セットするタグナンバー
 * @param zone_id セットする地名のゾーンID
 */
//--------------------------------------------------------------
#define _PLACE_NAME( idx, zone_id ) \
    _ASM_PLACE_NAME idx, zone_id

    .macro  _ASM_PLACE_NAME idx, zone_id
    .short  EV_SEQ_PLACE_NAME
    .byte   \idx
    .short  \zone_id
    .endm

//--------------------------------------------------------------
/**
 *  ポケット名を指定タグにセット
 * @param idx セットするタグナンバー
 * @param pocket_id   バッグのポケット名を指すID
 */
//--------------------------------------------------------------
#define _POCKET_NAME( idx, pocket_id )  _ASM_POCKET_NAME idx, pocket_id

  .macro  _ASM_POCKET_NAME idx, pocket_id
  .short  EV_SEQ_POCKET_NAME
  .byte   \idx
  .short  \pocket_id
  .endm

//--------------------------------------------------------------
/**
 *  手持ちポケモンの種族名を指定タグにセット
 * @param idx セットするタグナンバー
 * @param pos セットするポケモンを指定
 */
//--------------------------------------------------------------
#define _TEMOTI_MONS_NAME( idx, pos ) \
    _ASM_TEMOTI_MONS_NAME idx, pos

  .macro _ASM_TEMOTI_MONS_NAME idx, pos
  .short EV_SEQ_TEMOTI_MONS_NAME
  .byte  \idx
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 *  手持ちポケモンのニックネームを指定タグにセット
 * @param idx セットするタグナンバー
 * @param pos セットするポケモンを指定
 */
//--------------------------------------------------------------
#define _TEMOTI_NICK_NAME( idx, pos ) \
    _ASM_TEMOTI_NICK_NAME idx, pos

  .macro _ASM_TEMOTI_NICK_NAME idx, pos
  .short EV_SEQ_TEMOTI_NICK_NAME
  .byte  \idx
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 *  育て屋のポケモンの種族名を指定タグにセット
 * @param idx セットするタグナンバー
 * @param pos セットするポケモンを指定
 */
//--------------------------------------------------------------
#define _SODATEYA_MONS_NAME( idx, pos ) \
    _ASM_SODATEYA_MONS_NAME idx, pos

  .macro _ASM_SODATEYA_MONS_NAME idx, pos
  .short EV_SEQ_SODATEYA_MONS_NAME
  .byte  \idx
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 *  育て屋のポケモンのニックネームを指定タグにセット
 * @param idx セットするタグナンバー
 * @param pos セットするポケモンを指定
 */
//--------------------------------------------------------------
#define _SODATEYA_NICK_NAME( idx, pos ) \
    _ASM_SODATEYA_NICK_NAME idx, pos

  .macro _ASM_SODATEYA_NICK_NAME idx, pos
  .short EV_SEQ_SODATEYA_NICK_NAME
  .byte  \idx
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 数値を指定タグにセット
 * @param  idx セットするタグナンバー
 * @param  num セットする数値
 * @param keta 桁数
 */
//--------------------------------------------------------------
#define _NUMBER( idx, num, keta ) \
    _ASM_NUMBER idx, num, keta

  .macro _ASM_NUMBER idx, num, keta
  .short EV_SEQ_NUMBER
  .byte  \idx
  .short \num
  .short \keta
  .endm

//--------------------------------------------------------------
/**
 *  ミュージカル関係のワードを指定タグにセット(下のラッパーを使ってください。
 * @param type セットする種類(0:演目名 1:グッズ名
 * @param idx セットするタグナンバー
 * @param val セットする種類番号(演目名:無効 グッズ名:グッズ番号
 */
//--------------------------------------------------------------
#define _MUSICAL_WORD( type, idx, val ) \
    _ASM_MUSICAL_WORD type, idx, val

  .macro _ASM_MUSICAL_WORD type, idx, val
  .short EV_SEQ_MUSICAL_WORD
  .byte  \type
  .byte  \idx
  .short \val
  .endm

//--------------------------------------------------------------
/**
 * ミュージカル ROMに入っている演目名をワードセットにセット
 * @param idx セットするタグナンバー
 */
//--------------------------------------------------------------
#define _MUSICAL_WORD_PROGRAM_TITLE( idx ) \
    _ASM_MUSICAL_WORD_PROGRAM_TITLE idx

  .macro _ASM_MUSICAL_WORD_PROGRAM_TITLE idx
  .short EV_SEQ_MUSICAL_WORD
  .byte  0
  .byte  \idx
  .short 0
  .endm

//--------------------------------------------------------------
/**
 * ミュージカル グッズ名をワードセットにセット
 * @param idx セットするタグナンバー
 * @param no  グッズ番号
 */
//--------------------------------------------------------------
#define _MUSICAL_WORD_GOODS_NAME( idx, no ) \
    _ASM_MUSICAL_WORD_GOODS_NAME idx, no

  .macro _ASM_MUSICAL_WORD_GOODS_NAME idx, no
  .short EV_SEQ_MUSICAL_WORD
  .byte  1
  .byte  \idx
  .short \no
  .endm

//--------------------------------------------------------------
/**
 * 簡易会話単語をバッファにセット
 * @param idx セットするタグナンバー
 * @param word  簡易会話ワード
 */
//--------------------------------------------------------------
#define _EASY_TALK_WORD_NAME( idx, word ) \
    _ASM_EASY_TALK_WORD_NAME idx, word

  .macro _ASM_EASY_TALK_WORD_NAME idx, word
  .short EV_SEQ_EASY_TALK_WORD_NAME
  .byte \idx
  .short \word
  .endm

//--------------------------------------------------------------
/**
 * 国名をバッファにセット
 * @param idx セットするタグナンバー
 * @param code  国コード
 */
//--------------------------------------------------------------
#define _COUNTRY_NAME( idx, code ) \
    _ASM_COUNTRY_NAME idx, code

  .macro _ASM_COUNTRY_NAME idx, code
  .short EV_SEQ_COUNTRY_NAME
  .byte \idx
  .short \code
  .endm

//--------------------------------------------------------------
/**
 * 趣味名をバッファにセット
 * @param idx     セットするタグナンバー
 * @param hobbyID 趣味ID ( HOBBY_ID_xxxx )
 *  
 * ◆趣味ID は resource/research_radar/data/hobby_id.h で定義
 */
//--------------------------------------------------------------
#define _HOBBY_NAME( idx, hobbyID ) \
    _ASM_HOBBY_NAME idx, hobbyID

  .macro _ASM_HOBBY_NAME idx, hobbyID
  .short EV_SEQ_HOBBY_NAME
  .byte  \idx
  .short \hobbyID
  .endm
  
//--------------------------------------------------------------
/**
 *  GPower名を指定タグにセット
 * @param idx セットするタグナンバー
 * @param gpower_id   GPOWER_ID
 */
//--------------------------------------------------------------
#define _GPOWER_NAME( idx, gpower_id )  _ASM_GPOWER_NAME idx, gpower_id

  .macro  _ASM_GPOWER_NAME idx, gpower_id
  .short  EV_SEQ_GPOWER_NAME
  .byte   \idx
  .short  \gpower_id
  .endm

//--------------------------------------------------------------
/**
 *  トレーナー種別を指定タグにセット
 * @param idx   セットするタグナンバー
 * @param tr_id セットするトレーナー種別
 */
//--------------------------------------------------------------
#define _TRAINER_TYPE_NAME( idx, tr_id ) \
    _ASM_TRAINER_TYPE_NAME idx, tr_id

  .macro  _ASM_TRAINER_TYPE_NAME idx, tr_id
  .short  EV_SEQ_TRAINER_TYPE_NAME
  .byte   \idx
  .short  \tr_id
  .endm
  
//--------------------------------------------------------------
/**
 * 回答をバッファにセット
 * @param idx      セットするタグナンバー
 * @param answerID 回答ID ( ANSWER_ID_xxxx )
 *  
 * ◆趣味ID は resource/research_radar/data/answer_id.h で定義
 */
//--------------------------------------------------------------
#define _ANSWER_NAME( idx, answerID ) \
    _ASM_ANSWER_NAME idx, answerID

  .macro _ASM_ANSWER_NAME idx, answerID
  .short EV_SEQ_ANSWER_NAME
  .byte  \idx
  .short \answerID
  .endm

//--------------------------------------------------------------
/**
 * 通算『すれ違い回数』をバッファにセット
 * @param idx セットするタグナンバー
 */
//--------------------------------------------------------------
#define _SURETIGAI_COUNT( idx ) \
    _ASM_SURETIGAI_COUNT idx

  .macro _ASM_SURETIGAI_COUNT idx
  .short EV_SEQ_SURETIGAI_COUNT
  .byte  \idx
  .endm

//--------------------------------------------------------------
/**
 * 通算『すれ違いでお礼を受けた回数』をバッファにセット
 * @param idx セットするタグナンバー
 */
//--------------------------------------------------------------
#define _SURETIGAI_THANKS_RECEIVE_COUNT( idx ) \
    _ASM_SURETIGAI_THANKS_RECEIVE_COUNT idx

  .macro _ASM_SURETIGAI_THANKS_RECEIVE_COUNT idx
  .short EV_SEQ_SURETIGAI_THANKS_RECEIVE_COUNT
  .byte  \idx
  .endm

//======================================================================
//  視線トレーナー関連
//======================================================================
//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_MOVE_SET
 *  @brief  視線：トレーナー移動呼び出し
 *  @param pos 移動させるトレーナー データ番号0,1
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_MOVE_SET( pos ) \
    _ASM_EYE_TRAINER_MOVE_SET pos

  .macro  _ASM_EYE_TRAINER_MOVE_SET  pos
  .short  EV_SEQ_EYE_TRAINER_MOVE_SET
  .short  \pos
  .endm
 
//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_MOVE
 *  @brief  視線：トレーナー移動
 *  @param none
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_MOVE()  \
    _ASM_EYE_TRAINER_MOVE

  .macro  _ASM_EYE_TRAINER_MOVE
  .short  EV_SEQ_EYE_TRAINER_MOVE
  .endm

//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_MOVE_TYPE_GET
 *  @brief  視線：トレーナー動作タイプ取得
 *  @param ret_wk トレーナータイプ格納先 SCR_EYE_TR_TYPE_SINGLE等が格納される
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_MOVE_TYPE_GET( ret_wk ) \
    _ASM_EYE_TRAINER_MOVE_TYPE_GET ret_wk

  .macro  _ASM_EYE_TRAINER_MOVE_TYPE_GET ret_wk
  .short  EV_SEQ_EYE_TRAINER_MOVE_TYPE_GET
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_ID_GET
 *  @brief  視線：トレーナーID取得
 *  @param  pos     位置指定（SCR_EYE_TR_0 / SCR_EYE_TR_1 )
 *  @param  ret_wk
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_ID_GET( pos, ret_wk ) \
  _ASM_EYE_TRAINER_ID_GET  pos,ret_wk
  .macro  _ASM_EYE_TRAINER_ID_GET  pos,ret_wk
  .short  EV_SEQ_EYE_TRAINER_ID_GET
  .short  \pos
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_OBJ_ID_GET
 *  @brief  視線：トレーナーOBJID取得
 *  @param  pos     位置指定（SCR_EYE_TR_0 / SCR_EYE_TR_1 )
 *  @param  ret_wk
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_OBJ_ID_GET( pos, ret_wk ) \
  _ASM_EYE_TRAINER_OBJ_ID_GET  pos,ret_wk
  .macro  _ASM_EYE_TRAINER_OBJ_ID_GET  pos,ret_wk
  .short  EV_SEQ_EYE_TRAINER_OBJ_ID_GET
  .short  \pos
  .short  \ret_wk
  .endm

//======================================================================
//  トレーナーバトル関連
//======================================================================
//--------------------------------------------------------------
/**
 *  @def  _TRAINER_ID_GET
 *  @brief  トレーナーID取得
 *  @param wk ID格納先ワーク
 */
//--------------------------------------------------------------
#define _TRAINER_ID_GET( wk ) \
  _ASM_TRAINER_ID_GET wk

  .macro  _ASM_TRAINER_ID_GET  wk
  .short  EV_SEQ_TRAINER_ID_GET
  .short  \wk
  .endm

//--------------------------------------------------------------
/**
 *  @def  _TRAINER_BTL_SET
 *  @brief  トレーナー戦闘呼び出し
 *  @param tr_id_0 トレーナーID0
 *  @param tr_id_1 トレーナーID1
 */
//--------------------------------------------------------------
#define _TRAINER_BTL_SET( tr_id0, tr_id1 ) \
    _ASM_TRAINER_BTL_SET tr_id0, tr_id1
  
  .macro  _ASM_TRAINER_BTL_SET  tr_id_0,tr_id_1
  .short  EV_SEQ_TRAINER_BTL_SET
  .short  \tr_id_0
  .short  \tr_id_1
  .short  SCR_BATTLE_MODE_NONE
  .endm

//--------------------------------------------------------------
/**
 *  @brief  トレーナー戦闘呼び出し（特殊指定あり）
 *  @param tr_id_0 トレーナーID0
 *  @param tr_id_1 トレーナーID1
 *
 *  @note flagsにSCR_BATTLE_MODE_NOLOSEを指定した場合、戦闘後は必ず「勝利」で戻ってきます。
 *      　この場合も戦闘後に_TRAINER_WIN()は必要になります。
 */
//--------------------------------------------------------------
#define _TRAINER_BTL_SP_SET( tr_id0, tr_id1, flags ) \
    _ASM_TRAINER_BTL_SP_SET tr_id0, tr_id1, flags
  
  .macro  _ASM_TRAINER_BTL_SP_SET  tr_id_0,tr_id_1 flags
  .short  EV_SEQ_TRAINER_BTL_SET
  .short  \tr_id_0
  .short  \tr_id_1
  .short  \flags
  .endm

//--------------------------------------------------------------
/**
 *  @def  _TRAINER_MULTI_BTL_SET
 *  @brief  トレーナー戦闘（マルチバトル）呼び出し
 *  @param partner_id パートナーのトレーナーID
 *  @param tr_id_0 対戦相手(左)のトレーナーID
 *  @param tr_id_1 対戦相手(右)のトレーナーID
 */
//--------------------------------------------------------------
#define _TRAINER_MULTI_BTL_SET( partner_id, tr_id_0, tr_id_1 ) \
  _ASM_TRAINER_MULTI_BTL_SET  partner_id, tr_id_0, tr_id_1

  .macro  _ASM_TRAINER_MULTI_BTL_SET  partner_id, tr_id_0, tr_id_1
  .short  EV_SEQ_TRAINER_MULTI_BTL_SET
  .short  \partner_id
  .short  \tr_id_0
  .short  \tr_id_1
  .short  SCR_BATTLE_MODE_NONE
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_MSG_SET トレーナー会話呼び出し
 *  @param tr_id メッセージ表示用トレーナーID
 *  @param kind_id 会話種類
 *  @param  obj_id  ふきだし位置とするOBJID
 */
//--------------------------------------------------------------
#define _TRAINER_MSG_SET( tr_id,kind_id,obj_id ) \
  _ASM_TRAINER_MSG_SET  tr_id,kind_id,obj_id

  .macro  _ASM_TRAINER_MSG_SET  tr_id,kind_id,obj_id
  .short  EV_SEQ_TRAINER_MSG_SET
  .short  \tr_id
  .short  \kind_id
  .short  \obj_id
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_TALK_TYPE_GET トレーナー会話の種類取得
 *  @param wk1 会話種類格納先ワーク 戦闘前
 *  @param wk2 会話種類格納先ワーク 戦闘後
 *  @param wk2 会話種類格納先ワーク ポケモン１体
 */
//--------------------------------------------------------------
#define _TRAINER_TALK_TYPE_GET(  wk1,wk2,wk3 ) \
  _ASM_TRAINER_TALK_TYPE_GET  wk1,wk2,wk3

  .macro  _ASM_TRAINER_TALK_TYPE_GET  wk1,wk2,wk3
  .short  EV_SEQ_TRAINER_TALK_TYPE_GET
  .short  \wk1
  .short  \wk2
  .short  \wk3
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_BTL_RULE_GET トレーナータイプ取得
 *  @param  tr_id   トレーナーID
 *  @param  ret_wk  戦闘ルール格納先ワーク
 *  @return SCR_TR_BTL_RULE_～(script_def.hで定義）
 */
//--------------------------------------------------------------
#define _TRAINER_BTL_RULE_GET( tr_id, ret_wk ) \
  _ASM_TRAINER_BTL_RULE_GET  tr_id, ret_wk

  .macro  _ASM_TRAINER_BTL_RULE_GET  tr_id, ret_wk
  .short  EV_SEQ_TRAINER_BTL_RULE_GET
  .short  \tr_id
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_BGM_SET トレーナーBGMセット
 *  @param tr_id トレーナーID
 */
//--------------------------------------------------------------
#define _TRAINER_BGM_SET(  tr_id ) \
  _ASM_TRAINER_BGM_SET  tr_id

  .macro  _ASM_TRAINER_BGM_SET  tr_id
  .short  EV_SEQ_TRAINER_BGM_SET
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 * @def _TRAINER_WIN
 * @brief トレーナー戦勝利
 *
 * @note
 * 実際にはフェード処理しか行っていませんが、
 * 今後の拡張を考えてトレーナー戦処理に入れておいて下さい。
 */
//--------------------------------------------------------------
#define _TRAINER_WIN() _ASM_TRAINER_WIN_CALL

  .macro  _ASM_TRAINER_WIN_CALL
    _ASM_TRAINER_WIN
  .endm

  .macro  _ASM_TRAINER_WIN
  .short  EV_SEQ_TRAINER_WIN
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_LOSE トレーナー敗北
 *  @param none
 *
 *  このコマンドを呼び出すとスクリプトが強制終了し、敗北後の
 *  復帰イベント（自宅＆ポケセン）に遷移します。
 */
//--------------------------------------------------------------
#define _TRAINER_LOSE() \
  _ASM_TRAINER_LOSE

  .macro  _ASM_TRAINER_LOSE
  .short  EV_SEQ_TRAINER_LOSE
  .endm

//--------------------------------------------------------------
/**
 *  @def  _TRAINER_LOSE_CHECK
 *  @brief  トレーナー敗北チェック
 *  @param ret_wk 結果格納先
 *  
 *  @retval SCR_BATTLE_WIN  勝利
 *  @retval SCR_BATTLE_LOSE 敗北
 */
//--------------------------------------------------------------
#define _TRAINER_LOSE_CHECK( ret_wk ) \
    _ASM_TRAINER_LOSE_CHECK ret_wk

  .macro  _ASM_TRAINER_LOSE_CHECK ret_wk
  .short  EV_SEQ_TRAINER_LOSE_CHECK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _TRAINER_SPTYPE_GET
 * @brief トレーナー特殊タイプの取得
 * @param tr_id   トレーナーID
 * @param ret_wk    タイプを受け取るワーク
 * @return  SCR_TR_SPTYPE_～  （script_def.hで定義）
 *
 * @li  SCR_TR_SPTYPE_NONE      通常トレーナー
 * @li  SCR_TR_SPTYPE_RECOVER   回復トレーナー
 * @li  SCR_TR_SPTYPE_ITEM      アイテムトレーナー
 */
//--------------------------------------------------------------
#define _TRAINER_SPTYPE_GET( tr_id, ret_wk ) \
    _ASM_TRAINER_SPTYPE_GET tr_id, ret_wk
    .macro  _ASM_TRAINER_SPTYPE_GET tr_id, ret_wk
    .short  EV_SEQ_TRAINER_SPTYPE_GET
    .short  \tr_id
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _TRAINER_ITEM_GET
 * @brief アイテムトレーナーのアイテムの種類を取得
 * @param tr_id   トレーナーID
 * @param ret_wk  アイテムナンバーを受け取るワーク
 * @return  アイテムナンバー
 */
//--------------------------------------------------------------
#define _TRAINER_ITEM_GET( tr_id, ret_wk ) \
    _ASM_TRAINER_ITEM_GET tr_id, ret_wk
    .macro  _ASM_TRAINER_ITEM_GET tr_id, ret_wk
    .short  EV_SEQ_TRAINER_ITEM_GET
    .short  \tr_id
    .short  \ret_wk
    .endm


//--------------------------------------------------------------
/**
 * @def _TRADE_AFTER_TRAINER_BTL_SET
 * @brief 交換後　ポケモン　再戦　バトル開始
 * @param tr_id       トレーナーID
 * @param trade_type  交換　ポケモン　タイプ
 *
#define SCR_TRPOKE_AFTER_SAVE_C02 (0)    // シッポウ
#define SCR_TRPOKE_AFTER_SAVE_C05 (1)    // ホドモエ
 */
//--------------------------------------------------------------
#define _TRADE_AFTER_TRAINER_BTL_SET( tr_id, trade_type ) \
    _ASM_TRADE_AFTER_TRAINER_BTL_SET tr_id, trade_type
    .macro  _ASM_TRADE_AFTER_TRAINER_BTL_SET tr_id, trade_type
    .short  EV_SEQ_TRADE_AFTER_TRAINER_BTL_SET
    .short  \tr_id
    .short  0
    .short  SCR_BATTLE_MODE_NONE
    .short  \trade_type
    .endm

//======================================================================
//
//    野生ポケモン戦関連
//
//======================================================================
//--------------------------------------------------------------
/**
 * _WILD_BTL_SET  野生戦呼び出し(通常)
 * @param mons_no エンカウントさせたいモンスターNo 
 * @param mons_lv エンカウントさせたいモンスターレベル
 *
 *  WILD_BTL_SET_XXX系はエンカウントメッセージが異なります
 * 「野生の○○が飛び出してきた」
 */
//--------------------------------------------------------------
#define _WILD_BTL_SET( mons_no, mons_lv ) \
    _ASM_WILD_BTL_SET mons_no, mons_lv, SCR_WILD_BTL_FLAG_NONE 

//--------------------------------------------------------------
/**
 * _WILD_BTL_SET_TALK  野生戦呼び出し(話しかけエンカウント系)
 * @param mons_no エンカウントさせたいモンスターNo 
 * @param mons_lv エンカウントさせたいモンスターレベル
 *
 *  WILD_BTL_SET_XXX系はエンカウントメッセージが異なります
 *  話しかけによるエンカウント「野生の○○が現れた」 
 */
//--------------------------------------------------------------
#define _WILD_BTL_SET_TALK( mons_no, mons_lv ) \
    _ASM_WILD_BTL_SET mons_no, mons_lv, SCR_WILD_BTL_FLAG_WILD_TALK

//--------------------------------------------------------------
/**
 * _WILD_BTL_SET_LEGEND  野生戦呼び出し(伝説系)
 * @param mons_no エンカウントさせたいモンスターNo 
 * @param mons_lv エンカウントさせたいモンスターレベル
 *
 *  WILD_BTL_SET_XXX系はエンカウントメッセージが異なります
 *  伝説系「○○が現れた」("野生の…"がないバージョンです) 
 */
//--------------------------------------------------------------
#define _WILD_BTL_SET_LEGEND( mons_no, mons_lv ) \
    _ASM_WILD_BTL_SET mons_no, mons_lv, SCR_WILD_BTL_FLAG_LEGEND

//--------------------------------------------------------------
/**
 * _WILD_BTL_SET_EX  野生戦呼び出し(フラグを自前で全部セットするバージョン)
 * @param mons_no エンカウントさせたいモンスターNo 
 * @param mons_lv エンカウントさせたいモンスターレベル
 * @param flags   SCR_WILD_BTL_FLAG_NONE他 script_def.hに定義
 */
//--------------------------------------------------------------
#define _WILD_BTL_SET_EX( mons_no, mons_lv, flags ) \
    _ASM_WILD_BTL_SET mons_no, mons_lv, flags

  .macro  _ASM_WILD_BTL_SET mons_no, mons_lv, flags
  .short  EV_SEQ_WILD_BTL_SET
  .short  \mons_no
  .short  \mons_lv
  .short  \flags
  .endm

//--------------------------------------------------------------
/**
 * _CAPTURE_DEMO_SET  捕獲デモセット
 */
//--------------------------------------------------------------
#define _CAPTURE_DEMO_SET( ) \
    _ASM_CAPTURE_DEMO_SET 

  .macro  _ASM_CAPTURE_DEMO_SET
  .short  EV_SEQ_CAPTURE_DEMO_SET
  .endm

//--------------------------------------------------------------
/**
 * @def _WILD_WIN
 * @brief 野生戦勝利処理
 */
//--------------------------------------------------------------
#define _WILD_WIN() _ASM_WILD_WIN_CALL

  .macro  _ASM_WILD_WIN_CALL
    _ASM_WILD_WIN
  .endm

  .macro  _ASM_WILD_WIN
  .short  EV_SEQ_WILD_WIN
  .endm

//--------------------------------------------------------------
/**
 * _WILD_LOSE  野生戦敗北処理
 * @param none
 *
 *  このコマンドを呼び出すとスクリプトが強制終了し、敗北後の
 *  復帰イベント（自宅＆ポケセン）に遷移します。
 */
//--------------------------------------------------------------
#define _WILD_LOSE() \
  _ASM_WILD_LOSE

  .macro  _ASM_WILD_LOSE
  .short  EV_SEQ_WILD_LOSE
  .endm

//--------------------------------------------------------------
/**
 *  @def  _WILD_LOSE_CHECK
 *  @brief  野生ポケモン戦での敗北チェック
 *  @param ret_wk 結果格納先
 *  
 *  @retval SCR_BATTLE_WIN  勝利
 *  @retval SCR_BATTLE_LOSE 敗北
 */
//--------------------------------------------------------------
#define  _WILD_LOSE_CHECK(  ret_wk ) \
  _ASM_WILD_LOSE_CHECK  ret_wk

  .macro  _ASM_WILD_LOSE_CHECK  ret_wk
  .short  EV_SEQ_WILD_LOSE_CHECK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  @def  _WILD_RETRY_CHECK
 *  @brief  野生ポケモン戦での再戦コードチェック
 *  @param ret_wk 結果格納先
 *  
 *  @retval SCR_WILD_BTL_RET_CAPTURE  捕まえた
 *  @retval SCR_WILD_BTL_RET_ESCAPE   逃げた
 *  @retval SCR_WILD_BTL_RET_WIN      倒した
 *  
 *  @note このコマンドは使用場所をリストアップしています。使う場合は斉藤マに使用場所を教えてください。
 */
//--------------------------------------------------------------
#define  _WILD_RETRY_CHECK(  ret_wk ) \
  _ASM_WILD_RETRY_CHECK  ret_wk

  .macro  _ASM_WILD_RETRY_CHECK  ret_wk
  .short  EV_SEQ_WILD_BTL_RETRY_CHECK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  @def  _GET_FISHING_ENCOUNT_MONSNO
 *  @brief  釣りエンカウトするポケモンの中からランダムで1種を選んでモンスターNoを返す
 *  @param ret_wk 結果格納先
 *  
 *  @retval モンスターNo
 */
//--------------------------------------------------------------
#define _GET_FISHING_ENCOUNT_MONSNO( ret_wk ) \
    _ASM_GET_FISHING_ENCOUNT_MONSNO ret_wk

  .macro  _ASM_GET_FISHING_ENCOUNT_MONSNO ret_wk
  .short  EV_SEQ_GET_FISHING_ENCOUNT_MONSNO
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  @def _LOOK_ELBOARD
 *  @brief 電光掲示板を見る
 *  @param frame カメラ移動に要するフレーム数
 */
//--------------------------------------------------------------
#define _LOOK_ELBOARD( frame ) \
    _ASM_LOOK_ELBOARD frame

  .macro  _ASM_LOOK_ELBOARD frame
  .short  EV_SEQ_LOOK_ELBOARD
  .short \frame
  .endm



//======================================================================
//  トレーナーフラグ
//======================================================================
//--------------------------------------------------------------
/**
 *  _TRAINER_FLAG_SET トレーナーフラグのセット
 *  @param tr_id セットするトレーナーID
 */
//--------------------------------------------------------------
#define _TRAINER_FLAG_SET( tr_id ) \
    _ASM_TRAINER_FLAG_SET tr_id

  .macro  _ASM_TRAINER_FLAG_SET tr_id
  .short  EV_SEQ_TRAINER_FLAG_SET
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_FLAG_RESET トレーナーフラグのリセット
 *  @param tr_id リセットするトレーナーID
 */
//--------------------------------------------------------------
#define _TRAINER_FLAG_RESET( tr_id ) \
    _ASM_TRAINER_FLAG_RESET tr_id

  .macro  _ASM_TRAINER_FLAG_RESET tr_id
  .short  EV_SEQ_TRAINER_FLAG_RESET
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 *  @def  _TRAINER_FLAG_CHECK
 *  @brief  トレーナーフラグチェック
 *  @param  tr_id   チェックするトレーナーID
 *  @param  ret_wk  値を受け取るワーク
 */
//--------------------------------------------------------------
#define _TRAINER_FLAG_CHECK( tr_id, ret_wk )  \
    _ASM_TRAINER_FLAG_CHECK tr_id, ret_wk

  .macro  _ASM_TRAINER_FLAG_CHECK tr_id, ret_wk
  .short  EV_SEQ_TRAINER_FLAG_CHECK
  .short  \tr_id
  .short  \ret_wk
  .endm



//--------------------------------------------------------------
/**
 *  @def  _OBJ_RAIL_SLIPDOWN
 *  @brief  動作もでるをレール上からすべりおろす
 *  @param  objid   動かす動作オブジェ
 */
//--------------------------------------------------------------
#define _OBJ_RAIL_SLIPDOWN( objid )  \
    _ASM_OBJ_RAIL_SLIPDOWN objid

  .macro  _ASM_OBJ_RAIL_SLIPDOWN objid
  .short  EV_SEQ_OBJ_RAIL_SLIPDOWN
  .short  \objid
  .endm

//--------------------------------------------------------------
/**
 *  @def  _OBJ_HIGH_JUMP
 *  @brief  動作モデル　高いところから飛び降りる。 ネジ山演出で使用
 *  @param  objid   動かす動作オブジェ
 *  @param  gx      着地ｘグリッド座標
 *  @param  gy      着地ｙグリッド座標
 *  @param  gz      着地ｚグリッド座標
 */
//--------------------------------------------------------------
#define _OBJ_HIGH_JUMP( objid, gx, gy, gz )  \
    _ASM_OBJ_HIGH_JUMP objid, gx, gy, gz

  .macro  _ASM_OBJ_HIGH_JUMP objid, gx, gy, gz
  .short  EV_SEQ_OBJ_HIGH_JUMP
  .short  \objid
  .short  \gx
  .short  \gy
  .short  \gz
  .endm


//--------------------------------------------------------------
/**
 *  @def  _PLAYER_RAIL_LOC_SET
 *  @brief  自機にレールロケーションを設定
 * @param index   インデックス
 * @param front   フロントインデックス
 * @param side    サイドインデックス
 */
//--------------------------------------------------------------
#define _PLAYER_RAIL_LOC_SET( index, front, side )  \
    _ASM_PLAYER_RAIL_LOC_SET index, front, side

  .macro  _ASM_PLAYER_RAIL_LOC_SET index, front, side
  .short  EV_SEQ_PLAYER_RAIL_LOC_SET
  .short  \index
  .short  \front
  .short  \side
  .endm


//--------------------------------------------------------------
/**
 *  @def  _SEQ_OBJ_RAIL_LOC_SET
 *  @brief  動作モデルにレールロケーションを設定
 * @param objid   動かす動作モデル
 * @param index   インデックス
 * @param front   フロントインデックス
 * @param side    サイドインデックス
 */
//--------------------------------------------------------------
#define _OBJ_RAIL_LOC_SET( objid, index, front, side )  \
    _ASM_OBJ_RAIL_LOC_SET objid, index, front, side

  .macro  _ASM_OBJ_RAIL_LOC_SET objid, index, front, side
  .short  EV_SEQ_OBJ_RAIL_LOC_SET
  .short  \objid
  .short  \index
  .short  \front
  .short  \side
  .endm


//======================================================================
//  サウンド BGM
//======================================================================
//--------------------------------------------------------------
/**
  * _BGM_PLAY BGMを鳴らす
  * @param no BGM Number
  */
//--------------------------------------------------------------
#define _BGM_PLAY(no) _ASM_BGM_PLAY no

   .macro  _ASM_BGM_PLAY no
  .short  EV_SEQ_BGM_PLAY
  .short  \no
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_PLAY_CHECK ＢＧＭなっているかチェック
 *  @param  music   BGMナンバー
 *  @param  ret     TRUEなっている
 */
//--------------------------------------------------------------
#define _BGM_PLAY_CHECK(music, ret) _ASM_BGM_PLAY_CHECK music, ret

  .macro  _ASM_BGM_PLAY_CHECK music, ret
  .short  EV_SEQ_BGM_PLAY_CHECK
  .short music
  .short ret
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_NOW_MAP_RECOVER 現在のマップのBGMを鳴らす
 *  @param none
 */
//--------------------------------------------------------------
#define _BGM_NOW_MAP_RECOVER()  _ASM_BGM_NOW_MAP_RECOVER

  .macro  _ASM_BGM_NOW_MAP_RECOVER
  .short  EV_SEQ_BGM_NOW_MAP_RECOVER
  .endm

//--------------------------------------------------------------
/**
  * _START_TEMP_EVENT_BGM 一時的なイベントBGMを再生する
  * @param no BGM Number
  *
  * フィールドBGMを退避し、イベントBGMを再生します。
  * この状態でバトルから復帰すると、フィールドBGMが再生されます。
  *
  * フィールドBGM ⇒ イベントBGM ⇒ バトル ⇒ フィールドBGM
  * の流れが必要な場面でのみ使用してください。
 */
//--------------------------------------------------------------
#define _START_TEMP_EVENT_BGM( no ) \
        _ASM_START_TEMP_EVENT_BGM no

  .macro  _ASM_START_TEMP_EVENT_BGM no
  .short  EV_SEQ_START_TEMP_EVENT_BGM
  .short  \no
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_WAIT 
 */
//--------------------------------------------------------------
#define _BGM_WAIT() _ASM_BGM_WAIT

  .macro  _ASM_BGM_WAIT
  .short  EV_SEQ_BGM_WAIT
  .endm

//--------------------------------------------------------------
/**
 *  @def  _ASM_BGM_PUSH
 *  @brief  現在のBGMをフェードしてPopする
 *  @param  fadeout_frame 現在なっている曲をフェードアウトするフレーム数 
 *
 *  @note   必ず_BGM_POPと対で使用すること！
 *  @note   単独での呼び出し禁止！ マクロで包んで使用すること
 *          使用箇所を限定したいので、利用は玉田さんの許可制
 */
//--------------------------------------------------------------
//#define _BGM_PUSH( fadeout_frame ) _ASM_BGM_PUSH fadeout_frame
  .macro  _ASM_BGM_PUSH fadeout_frame
  .short  EV_SEQ_BGM_PUSH
  .short  \fadeout_frame
  .endm

//--------------------------------------------------------------
/**
 *  @def  _ASM_BGM_POP
 *  @brief  現在のBGMをフェードして、保存されたBGMをPopしフェードインしながら戻す
 *  @param  fadeout_frame 現在なっている曲をフェードアウトするフレーム数 
 *
 *  @note   必ず_BGM_PUSHと対で使用すること！
 *  @note   単独での呼び出し禁止！ マクロで包んで使用すること
 *          使用箇所を限定したいので、利用は玉田さんの許可制
 *
 *  @note   鳴っているBGMがない時は、フェードアウトは無視されます 
 */
//--------------------------------------------------------------
//#define _BGM_POP( fadeout_frame, fadein_frame ) _ASM_BGM_POP fadeout_frame, fadein_frame
  .macro  _ASM_BGM_POP  fadeout_frame, fadein_frame
  .short  EV_SEQ_BGM_POP
  .short  \fadeout_frame
  .short  \fadein_frame
  .endm

//--------------------------------------------------------------
/**
 * @def _START_EVENT_BGM
 * @brief スクリプト簡易マクロ：イベントBGM開始
 * @param bgmno
 *
 * @note
 * 内部的には _BGM_PLAY()を呼んでいる
 */
//--------------------------------------------------------------
#define _START_EVENT_BGM( bgmno ) \
    _ASM_START_EVENT_BGM bgmno

  .macro _ASM_START_EVENT_BGM bgmno
//  _ASM_BGM_STOP
  _ASM_BGM_PLAY \bgmno
  .endm

//--------------------------------------------------------------
/**
  * @def _START_EVENT_BGM_EX 
	* @brief BGMを鳴らす ( FOフレーム数指定 ver. )
  * @param no    BGM Number
	* @param frame フェードアウトフレーム数
  */
//--------------------------------------------------------------
#define _START_EVENT_BGM_EX( no, frame ) \
		_ASM_START_EVENT_BGM_EX no, frame

  .macro  _ASM_START_EVENT_BGM_EX no, frame
  .short  EV_SEQ_START_EVENT_BGM_EX
  .short  \no
  .short  \frame
  .endm

//--------------------------------------------------------------
/**
  * @def _START_EVENT_BGM_SILENT 
	* @brief 無音曲を再生する
	* @param frame フェードアウトフレーム数
  */
//--------------------------------------------------------------
#define _START_EVENT_BGM_SILENT( frame ) \
		_ASM_START_EVENT_BGM_SILENT frame

  .macro  _ASM_START_EVENT_BGM_SILENT frame
  .short  EV_SEQ_START_EVENT_BGM_SILENT
  .short  \frame
  .endm

//--------------------------------------------------------------
/**
 * @def _END_EVENT_BGM
 * @brief スクリプト簡易マクロ：イベントBGM終了
 *
 * @note
 * 内部的には、_BGM_NOW_MAP_RECOVER()を呼んでいる
 */
//--------------------------------------------------------------
#define _END_EVENT_BGM() \
    _ASM_END_EVENT_BGM

  .macro  _ASM_END_EVENT_BGM
  //_ASM_BGM_FADEOUT  0, 30
  //_ASM_BGM_STOP
  _ASM_BGM_NOW_MAP_RECOVER
  .endm

//--------------------------------------------------------------
/**
 *  @def _END_EVENT_BGM_EX 
 *  @brief 現在のマップのBGMを鳴らす ( FOフレーム数指定 ver. )
 *  @param frame フェードアウトフレーム数
 */
//--------------------------------------------------------------
#define _END_EVENT_BGM_EX( frame ) \
		_ASM_END_EVENT_BGM_EX frame

  .macro  _ASM_END_EVENT_BGM_EX frame
  .short  EV_SEQ_END_EVENT_BGM_EX
  .short  \frame
  .endm

//--------------------------------------------------------------
/**
 *  @def _BGM_VOLUME_DOWN 
 *  @brief BGM の音量を下げる
 *  @param volume 音量 ( MIN 0～127 MAX )
 *  @param frame  フェードアウトフレーム数
 *
 *  ※ボリュームを下げたら, 
 *    必ず _BGM_VOLUME_RECOVER() で元に戻してください。
 */
//--------------------------------------------------------------
#define _BGM_VOLUME_DOWN( volume, frame ) \
		_ASM_BGM_VOLUME_DOWN volume, frame

  .macro  _ASM_BGM_VOLUME_DOWN volume, frame
  .short  EV_SEQ_BGM_VOLUME_DOWN
  .short  \volume
  .short  \frame
  .endm

//--------------------------------------------------------------
/**
 *  @def _BGM_VOLUME_RECOVER 
 *  @brief BGM の音量を下げる
 *  @param frame フェードインフレーム数
 */
//--------------------------------------------------------------
#define _BGM_VOLUME_RECOVER( frame ) \
		_ASM_BGM_VOLUME_RECOVER frame

  .macro  _ASM_BGM_VOLUME_RECOVER frame
  .short  EV_SEQ_BGM_VOLUME_RECOVER
  .short  \frame
  .endm


//======================================================================
//  サウンド ISS
//======================================================================

//--------------------------------------------------------------
/**
 * @brief スイッチISS　指定スイッチを ON にする
 * @def _ISS_SWITCH_ON 
 * @param idx ON にするスイッチ番号(0～8)
 */
//--------------------------------------------------------------
#define _ISS_SWITCH_ON( idx ) \
    _ASM_ISS_SWITCH_ON idx

  .macro  _ASM_ISS_SWITCH_ON idx
  .short  EV_SEQ_ISS_SWITCH_ON
  .short  \idx
  .endm

//--------------------------------------------------------------
/**
 * @brief スイッチISS　指定スイッチを OFF にする
 * @def _ISS_SWITCH_OFF
 * @param idx OFF にするスイッチ番号(0～8)
 */
//--------------------------------------------------------------
#define _ISS_SWITCH_OFF( idx ) \
    _ASM_ISS_SWITCH_OFF idx

  .macro  _ASM_ISS_SWITCH_OFF idx
  .short  EV_SEQ_ISS_SWITCH_OFF
  .short  \idx
  .endm

//--------------------------------------------------------------
/**
 * @brief スイッチISS　指定スイッチの ON/OFF を調べる
 * @def _ISS_SWITCH_CHECK
 * @param ret_wk 判定結果の格納先
 * @param idx    OFF にするスイッチ番号(0～8)
 * @return 指定スイッチが ON なら TRUE, そうでなければ FALSE
 */
//--------------------------------------------------------------
#define _ISS_SWITCH_CHECK( ret_wk, idx ) \
    _ASM_ISS_SWITCH_CHECK ret_wk, idx

  .macro  _ASM_ISS_SWITCH_CHECK ret_wk, idx
  .short  EV_SEQ_ISS_SWITCH_CHECK
  .short  \ret_wk
  .short  \idx
  .endm


//======================================================================
//  サウンド EnvSE 環境音SE
//======================================================================
//--------------------------------------------------------------
/**
 *  _CLEAR_ENVSE_BGMPLAY_PAUSE    BGMチェンジでの環境SEポーズ状態をCLEAR
 */
//-------------------------------------------------------------
#define   _CLEAR_ENVSE_EVENT_BGMPLAY_PAUSE()  \
    _ASM_CLEAR_ENVSE_EVENT_BGMPLAY_PAUSE

.macro  _ASM_CLEAR_ENVSE_EVENT_BGMPLAY_PAUSE
.short  EV_SEQ_CLEAR_ENVSE_BGMPLAY_PAUSE
.endm



//======================================================================
//  サウンド SE
//======================================================================
//--------------------------------------------------------------
/**
 *  _SE_PLAY SEを鳴らす
 *  @param no 再生するSEナンバー
 */
//-------------------------------------------------------------
#define   _SE_PLAY( no )  \
    _ASM_SE_PLAY no

.macro  _ASM_SE_PLAY  no
.short  EV_SEQ_SE_PLAY
.short  \no
.endm

//--------------------------------------------------------------
/**
 *  _SE_STOP SEを止める
 *  @param none
 */
//--------------------------------------------------------------
#define   _SE_STOP() \
  .short  EV_SEQ_SE_STOP

.macro  _ASM_SE_STOP
.short  EV_SEQ_SE_STOP
.endm
//--------------------------------------------------------------
/**
 *  _SE_WAIT SE終了待ち
 *  @param none
 */
//--------------------------------------------------------------
#define   _SE_WAIT()  _ASM_SE_WAIT

.macro  _ASM_SE_WAIT
.short  EV_SEQ_SE_WAIT
.endm

//======================================================================
//  サウンド ME
//======================================================================
//--------------------------------------------------------------
/**
 *  _ME_PLAY MEを鳴らす
 *  @param no 再生するBGMナンバー
 */
//--------------------------------------------------------------
#define _ME_PLAY( no ) _ASM_ME_PLAY no

.macro  _ASM_ME_PLAY  no
  .short  EV_SEQ_ME_PLAY
  .short  \no
.endm

//--------------------------------------------------------------
/**
 *  _ME_WAIT ME終了待ち
 *  @param none
 */
//--------------------------------------------------------------
#define _ME_WAIT() _ASM_ME_WAIT

.macro  _ASM_ME_WAIT
  .short  EV_SEQ_ME_WAIT
.endm

//======================================================================
//  サウンド 鳴き声
//======================================================================
//--------------------------------------------------------------
/**
 * ポケモン鳴き声再生
 *
 * @param monsno  ポケモンナンバー
 * @param formno  フォルムナンバー（特にない場合、０を指定）
 */
//--------------------------------------------------------------
#define _VOICE_PLAY( monsno, formno ) \
    _ASM_VOICE_PLAY monsno, formno

  .macro  _ASM_VOICE_PLAY monsno, formno
  .short  EV_SEQ_VOICE_PLAY
  .short  \monsno
  .short  \formno
  .endm

//--------------------------------------------------------------
/**
 * ポケモン鳴き声再生 ( ペラップ録音データ適用 ver. )
 *
 * @param monsno ポケモンナンバー
 * @param formno フォルムナンバー（特にない場合、０を指定）
 *
 * ※ペラップを指定した場合, 
 *    録音データが存在するなら, そのデータを再生する.
 *    録音データが存在しなければ, デフォルトの鳴き声を再生する.
 */
//--------------------------------------------------------------
#define _VOICE_PLAY_FOR_MINE( monsno, formno ) \
    _ASM_VOICE_PLAY_FOR_MINE monsno, formno

  .macro  _ASM_VOICE_PLAY_FOR_MINE monsno, formno
  .short  EV_SEQ_VOICE_PLAY_FOR_MINE
  .short  \monsno
  .short  \formno
  .endm

//--------------------------------------------------------------
/**
 * ポケモン鳴き声再生
 */
//--------------------------------------------------------------
#define _VOICE_WAIT() \
    _ASM_VOICE_WAIT

  .macro  _ASM_VOICE_WAIT
  .short  EV_SEQ_VOICE_WAIT
  .endm

//======================================================================
//  メニュー
//======================================================================
//--------------------------------------------------------------
/**
 *  _BMPMENU_INIT BMPメニュー　初期化
 *  @param x メニュー左上X座標 キャラ単位
 *  @param y メニュー左上Y座標 キャラ単位
 *  @param cursor カーソル初期位置
 *  @param cancel Bボタンキャンセル有効フラグ 1=有効 0=無効
 *  @param ret_wk 結果格納先ワークID
 */
//--------------------------------------------------------------
#define _BMPMENU_INIT( x,y,cursor,cancel,ret_wk ) \
    _ASM_BMPMENU_INIT x,y,cursor,cancel,ret_wk

   .macro  _ASM_BMPMENU_INIT x,y,cursor,cancel,ret_wk
  .short  EV_SEQ_BMPMENU_INIT
  .byte  \x
  .byte  \y
  .short \cursor
  .byte  \cancel
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  _BMPMENU_INIT_EX BMPメニュー　初期化(読み込んでいるgmmファイルを使用する)
 *  @param x メニュー左上X座標 キャラ単位
 *  @param y メニュー左上Y座標 キャラ単位
 *  @param cursor カーソル初期位置
 *  @param cancel Bボタンキャンセル有効フラグ 1=有効 0=無効
 *  @param ret_wk 結果格納先ワークID
 *  @note 座標指定は定義名を使います。MENU_X1,MENU_Y1 詳細はuser_define.hを参照。
 */
//--------------------------------------------------------------
#define _BMPMENU_INIT_EX( x,y,cursor,cancel,ret_wk ) \
   _ASM_BMPMENU_INIT_EX x,y,cursor,cancel,ret_wk

   .macro  _ASM_BMPMENU_INIT_EX x,y,cursor,cancel,ret_wk
  .short  EV_SEQ_BMPMENU_INIT_EX
  .byte  \x
  .byte  \y
  .short \cursor
  .byte  \cancel
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  _BMPMENU_INIT_EX BMPメニュー　初期化(右詰、読み込んでいるgmmファイルを使用する)
 *  @param x メニュー左上X座標 キャラ単位
 *  @param y メニュー左上Y座標 キャラ単位
 *  @param cursor カーソル初期位置
 *  @param cancel Bボタンキャンセル有効フラグ 1=有効 0=無効
 *  @param ret_wk 結果格納先ワークID
 *  @note 座標指定は定義名を使います。MENU_X1,MENU_Y1 詳細はuser_define.hを参照。
 */
//--------------------------------------------------------------
#define _BMPMENU_INIT_RIGHT( x,y,cursor,cancel,ret_wk ) \
   _ASM_BMPMENU_INIT_RIGHT x,y,cursor,cancel,ret_wk

   .macro  _ASM_BMPMENU_INIT_RIGHT x,y,cursor,cancel,ret_wk
  .short  EV_SEQ_BMPMENU_INIT_EX_RIGHT
  .byte  \x
  .byte  \y
  .short \cursor
  .byte  \cancel
  .short  \ret_wk
  .endm


//--------------------------------------------------------------
/**
 *  _BMPMENU_MAKE_LIST BMPメニュー　リスト作成
 *  @param msg_id メッセージID
 *  @param param メッセージが選択された際に結果として返す値
 */
//--------------------------------------------------------------
#define _BMPMENU_MAKE_LIST( msg_id, param ) \
    _ASM_BMPMENU_MAKE_LIST msg_id, SCR_BMPMENU_EXMSG_NULL, param

//--------------------------------------------------------------
/**
 *  _BMPMENU_MAKE_LIST BMPメニュー　リスト作成拡張版
 *  @param msg_id メッセージID
 *  @param ex_msg_id  各項目の説明メッセージID
 *  @param param メッセージが選択された際に結果として返す値
 *
 *  ex_msgを各項目選択時にウィンドウに描画します
 *  09.12.08現在 SYSWIN限定
 */
//--------------------------------------------------------------
#define _BMPMENU_MAKE_LIST_EX( msg_id, ex_msg_id, param ) \
    _ASM_BMPMENU_MAKE_LIST msg_id, ex_msg_id, param

  .macro  _ASM_BMPMENU_MAKE_LIST msg_id,ex_msg_id,param
  .short  EV_SEQ_BMPMENU_MAKE_LIST
  .short  \msg_id
  .short  \ex_msg_id 
  .short  \param
  .endm

//--------------------------------------------------------------
/**
 *  _BMPMENU_START BMPメニュー　開始
 *  @param none
 */
//--------------------------------------------------------------
#define _BMPMENU_START() _ASM_BMPMENU_START

  .macro  _ASM_BMPMENU_START
  .short  EV_SEQ_BMPMENU_START
  .endm

//--------------------------------------------------------------
/**
 *  _BMPMENU_START BMPメニュー　開始(xボタンによる中断あり)
 *  @param none
 */
//--------------------------------------------------------------
#define _BMPMENU_START_BREAKABLE() _ASM_BMPMENU_START_BREAKABLE

  .macro  _ASM_BMPMENU_START_BREAKABLE
  .short  EV_SEQ_BMPMENU_START_BREAKABLE
  .endm

//======================================================================
//  画面フェード
//======================================================================
//--------------------------------------------------------------
//  フェードモード(fade.h)
//--------------------------------------------------------------
#define DISP_FADE_BLACKOUT_MAIN (0x01) ///<メイン画面フェード ブラックアウト
#define DISP_FADE_BLACKOUT_SUB  (0x02) ///<サブ画面フェード ブラックアウト
#define DISP_FADE_BLACKOUT (0x03) ///<両画面フェード ブラックアウト
#define DISP_FADE_WHITEOUT_MAIN (0x04) ///<メイン画面フェード ホワイトアウト
#define DISP_FADE_WHITEOUT_SUB  (0x08) ///<サブ画面フェード ホワイトアウト
#define DISP_FADE_WHITEOUT (0x0c) ///<両画面フェード ホワイトアウト

//--------------------------------------------------------------
/**
 *  _DISP_FADE_START 画面フェードスタート
 *  @param mode 画面フェードモード 
 *  @param start_evy 初期輝度(0=元の色～16=指定輝度）
 *  @param end_evy 終了輝度(0=元の色～16=指定輝度)
 *  @param speed フェードスピード 0～
 */
//--------------------------------------------------------------
#define _DISP_FADE_START( mode,start_evy,end_evy,speed ) _ASM_DISP_FADE_START mode,start_evy,end_evy,speed

  .macro  _ASM_DISP_FADE_START mode,start_evy,end_evy,speed
  .short EV_SEQ_DISP_FADE_START
  .short \mode
  .short \start_evy
  .short \end_evy
  .short \speed
  .endm


//--------------------------------------------------------------
/**
 *  _DISP_FADE_CHECK 画面フェード終了チェック
 *  @param none
 */
//--------------------------------------------------------------
#define _DISP_FADE_END_CHECK()  _ASM_DISP_FADE_END_CHECK

  .macro  _ASM_DISP_FADE_END_CHECK
  .short EV_SEQ_DISP_FADE_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @def  _WHITE_OUT
 *  @brief  ホワイトアウト
 *  @param none
 *  @note _DISP_FADE_END_CHECK()で終了待ちをしてください
 */
//--------------------------------------------------------------
#define _WHITE_OUT() \
    _DISP_FADE_START(DISP_FADE_WHITEOUT,0,16,DISP_FADE_SPEED)

//--------------------------------------------------------------
/**
 *  @def  _WHITE_IN
 *  @brief  ホワイトイン
 *  @param none
 */
//--------------------------------------------------------------
#define _WHITE_IN() \
    _DISP_FADE_START(DISP_FADE_WHITEOUT,16,0,DISP_FADE_SPEED)

//--------------------------------------------------------------
/**
 *  @def  _BLACK_OUT
 *  @brief  ブラックアウト
 *  @param  none
 *  @note _DISP_FADE_END_CHECK()で終了待ちをしてください
 */
//--------------------------------------------------------------
#define _BLACK_OUT() \
    _DISP_FADE_START(DISP_FADE_BLACKOUT,0,16,DISP_FADE_SPEED)
  
//--------------------------------------------------------------
/**
 *  @def  _BLACK_IN
 *  @brief  ブラックイン
 *  @param  none
 */
//--------------------------------------------------------------
#define _BLACK_IN() \
    _DISP_FADE_START(DISP_FADE_BLACKOUT,16,0,DISP_FADE_SPEED)


//--------------------------------------------------------------
/**
 *  @def  _WHITE_OUT_SLOW
 *  @brief  遅いホワイトアウト
 *  @param none
 *  @note _DISP_FADE_END_CHECK()で終了待ちをしてください
 */
//--------------------------------------------------------------
#define _WHITE_OUT_SLOW() \
    _DISP_FADE_START(DISP_FADE_WHITEOUT,0,16,DISP_SLOW_FADE_SPEED)

//--------------------------------------------------------------
/**
 *  @def  _WHITE_IN_SLOW
 *  @brief  遅いホワイトイン
 *  @param none
 */
//--------------------------------------------------------------
#define _WHITE_IN_SLOW() \
    _DISP_FADE_START(DISP_FADE_WHITEOUT,16,0,DISP_SLOW_FADE_SPEED)

//--------------------------------------------------------------
/**
 *  @def  _BLACK_OUT_SLOW
 *  @brief  遅いブラックアウト
 *  @param  none
 *  @note _DISP_FADE_END_CHECK()で終了待ちをしてください
 */
//--------------------------------------------------------------
#define _BLACK_OUT_SLOW() \
    _DISP_FADE_START(DISP_FADE_BLACKOUT,0,16,DISP_SLOW_FADE_SPEED)
  
//--------------------------------------------------------------
/**
 *  @def  _BLACK_IN_SLOW
 *  @brief  遅いブラックイン
 *  @param  none
 */
//--------------------------------------------------------------
#define _BLACK_IN_SLOW() \
    _DISP_FADE_START(DISP_FADE_BLACKOUT,16,0,DISP_SLOW_FADE_SPEED)
  
//======================================================================
//  アイテム関連
//======================================================================
//--------------------------------------------------------------
/**
 *	アイテムを加える
 *	@param item_no 加えるアイテムナンバー
 *	@param num 加える個数
 *	@param ret_wk 結果代入先 TRUE=成功 FALSE=失敗
 *	@note このコマンドはアイテムを追加するだけの処理です。
 *	スクリプト班の人は入手メッセージ等がパッケージされた _ITEM_EVENT_KEYWAIT を使ってください。
 */
//--------------------------------------------------------------
#define _ADD_ITEM( item_no, num, ret_wk )  _ASM_ADD_ITEM item_no,num,ret_wk

	.macro	_ASM_ADD_ITEM	item_no,num,ret_wk
	.short	EV_SEQ_ADD_ITEM
	.short	\item_no
	.short	\num
	.short	\ret_wk
	.endm

//--------------------------------------------------------------
/**
 *	アイテムを減らす
 *	@param item_no 減らすアイテムナンバー
 *	@param num 減らす個数
 *	@param ret_wk 結果代入先 TRUE=成功 FALSE=失敗
 */
//--------------------------------------------------------------
#define _SUB_ITEM( item_no,num,ret_wk ) _ASM_SUB_ITEM item_no,num,ret_wk

	.macro	_ASM_SUB_ITEM item_no,num,ret_wk
	.short	EV_SEQ_SUB_ITEM
	.short	\item_no
	.short	\num
	.short	\ret_wk
	.endm

//--------------------------------------------------------------
/**
 *	アイテムを加えられるかチェック
 *	@param item_no 加えるアイテムナンバー
 *	@param num 加える個数
 *	@param ret_wk 結果代入先 TRUE=可能 FALSE=不可能
 */
//--------------------------------------------------------------
#define _ADD_ITEM_CHK( item_no, num, ret_wk ) _ASM_ADD_ITEM_CHK item_no,num,ret_wk

	.macro	_ASM_ADD_ITEM_CHK	item_no,num,ret_wk
	.short	EV_SEQ_ADD_ITEM_CHK
	.short	\item_no
	.short	\num
	.short	\ret_wk
	.endm

//--------------------------------------------------------------
/**
 *	バッグのアイテムチェック
 *	@param item_no チェックするアイテムナンバー
 *	@param num チェックする個数
 *	@param ret_wk 結果代入先 TRUE=存在 FALSE=存在しない
 */
//--------------------------------------------------------------
#define _ITEM_CHK( item_no, num, ret_wk ) _ASM_ITEM_CHK item_no,num,ret_wk

	.macro	_ASM_ITEM_CHK		item_no,num,ret_wk
	.short	EV_SEQ_ITEM_CHK
	.short	\item_no
	.short	\num
	.short	\ret_wk
	.endm

//--------------------------------------------------------------
/**
 *	バッグのアイテム個数取得
 *	@param item_no 取得するアイテムナンバー
 *	@param ret_wk 結果代入先
 */
//--------------------------------------------------------------
#define _GET_ITEM_NUM( item_no, ret_wk ) _ASM_GET_ITEM_NUM		item_no,ret_wk

	.macro	_ASM_GET_ITEM_NUM		item_no,ret_wk
	.short	EV_SEQ_GET_ITEM_NUM
	.short	\item_no
	.short	\ret_wk
	.endm

//--------------------------------------------------------------
/**
 *	技マシンのアイテムナンバーかチェック
 *	@param item_no チェックするアイテムナンバー
 *	@param ret_wk 結果代入先
 *	@retval TRUE=技マシンである
 *	@retval FALSE=技マシンではない
 */
//--------------------------------------------------------------
#define _WAZA_ITEM_CHK( item_no,ret_wk ) _ASM_WAZA_ITEM_CHK item_no,ret_wk

	.macro	_ASM_WAZA_ITEM_CHK		item_no,ret_wk
	.short	EV_SEQ_WAZA_ITEM_CHK
	.short	\item_no
	.short	\ret_wk
	.endm

//--------------------------------------------------------------
/**
 * @def _GET_POCKET_ID
 * @brief ポケットIDの取得する
 * @param item_no   アイテムナンバー
 * @param ret_wk    結果を受け取るワーク
 *
 * item_noで指定したアイテムに対応したバッグのポケットIDを取得する
 */
//--------------------------------------------------------------
#define _GET_POCKET_ID( item_no, ret_wk )   \
    _ASM_GET_POCKET_ID item_no, ret_wk


    .macro  _ASM_GET_POCKET_ID item_no, ret_wk
    .short  EV_SEQ_GET_POCKET_ID
    .short  \item_no
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ITEM_TYPE
 * @brief 指定アイテムの種類を取得する
 * @param item_no   アイテムナンバー
 * @param ret_wk    結果を受け取るワーク  itemtype_def.h参照
 *
 * item_noで指定したアイテムに対応したアイテムタイプを取得する
 */
//--------------------------------------------------------------
#define _GET_ITEM_TYPE( item_no, ret_wk )   \
    _ASM_GET_ITEM_TYPE item_no, ret_wk


    .macro  _ASM_GET_ITEM_TYPE item_no, ret_wk
    .short  EV_SEQ_GET_ITEM_TYPE
    .short  \item_no
    .short  \ret_wk
    .endm
    
//--------------------------------------------------------------
/**
 * @def _GET_EFFECT_ENCOUNT_ITEM
 * @brief エフェクトエンカウントアイテムイベント　アイテムNoを取得
 * @param ret_wk    結果を受け取るワーク
 */
//--------------------------------------------------------------
#define _GET_EFFECT_ENCOUNT_ITEM( ret_wk )   \
    _ASM_GET_EFFECT_ENCOUNT_ITEM ret_wk


    .macro  _ASM_GET_EFFECT_ENCOUNT_ITEM ret_wk
    .short  EV_SEQ_GET_EFFECT_ENCOUNT_ITEM
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * フィールドのアイテムイベント呼び出し
 * @param no   アイテムナンバー
 * @param num 加える値
 */
//--------------------------------------------------------------
.macro _FLD_ITEM_EVENT no,num
  _ASM_LDVAL SCWK_REG0,\no
  _ASM_LDVAL SCWK_REG1,\num
  _CALL ev_fld_item2
.endm

//======================================================================
//  会話イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _TALK_START_SE_PLAY
 *  会話イベント開始時のSE再生
 *  @param none
 */
//--------------------------------------------------------------
#define _TALK_START_SE_PLAY() _ASM_TALK_START_SE_PLAY

  .macro  _ASM_TALK_START_SE_PLAY
  _ASM_SE_PLAY( SEQ_SE_MESSAGE )
  .endm

//--------------------------------------------------------------
/**
 *  _TALK_OBJ_START
 *  @brief  OBJに対して話し掛け開始(主人公に対して振り向き有り)
 *
 * @note
 *  内部的には_TALK_START_SE_PLAY()と_TURN_HERO_SITE()を呼び出すのと同機能。
 *  今後の会話関連の機能拡張に備えて、これを呼び出すようにしておく。
 */
//--------------------------------------------------------------
#define _TALK_OBJ_START() _ASM_TALK_OBJ_START

.macro  _ASM_TALK_OBJ_START
    _ASM_TALK_START_SE_PLAY
    _ASM_TURN_HERO_SITE
.endm

//--------------------------------------------------------------
/**
 * @def _TALK_START_TURN_NOT
 * @brief  OBJに対して話し掛け開始(主人公に対して振り向きなし）
 *
 * @note
 *  内部的には_TALK_START_SE_PLAY()を呼び出すのと同機能。
 *  今後の会話関連の機能拡張に備えて、これを呼び出すようにしておく。
 */
//--------------------------------------------------------------
#define _TALK_START_TURN_NOT() _TALK_START_SE_PLAY()


//======================================================================
//  ミュージカル関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _MUSICAL_CALL ミュージカル：ミュージカル呼び出し
 *  @param pos 選択位置
 *  @param mode モード (0:非通信 1:通信 2:ミュージカルショット
 */
//--------------------------------------------------------------
#define _MUSICAL_CALL( pos, mode ) \
    _ASM_MUSICAL_CALL pos, mode

  .macro  _ASM_MUSICAL_CALL pos, mode
  .short EV_SEQ_MUSICAL_CALL
  .short \pos
  .byte \mode
  .endm

//--------------------------------------------------------------
/**
 *  _MUSICAL_CALL ミュージカル：ミュージカルショット呼び出し
 */
//--------------------------------------------------------------
#define _MUSICAL_CALL_MUSICAL_SHOT() \
    _ASM_MUSICAL_CALL_MUSICAL_SHOT

  .macro  _ASM_MUSICAL_CALL_MUSICAL_SHOT
  .short EV_SEQ_MUSICAL_CALL
  .short 0
  .byte 2
  .endm

//--------------------------------------------------------------
/**
 *  _MUSICAL_FITTING_CALL ミュージカル：試着室呼び出し
 *  @param pos 選択位置
 */
//--------------------------------------------------------------
#define _MUSICAL_FITTING_CALL( pos ) \
    _ASM_MUSICAL_FITTING_CALL pos

  .macro  _ASM_MUSICAL_FITTING_CALL pos
  .short EV_SEQ_MUSICAL_FITTING_CALL
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE ミュージカル：ミュージカル数値取得(汎用
 *      基本的に下にあるラッパーを呼んでください
 *  @param type 種類(参加回数・トップ回数・最終評価点・最終コンディション
 *  @param val  取得用番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE( type, val, retVal ) \
    _ASM_GET_MUSICAL_VALUE type, val, retVal

  .macro  _ASM_GET_MUSICAL_VALUE type, val, retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte \type
  .short \val
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_JOIN_NUM ミュージカル：参加回数取得
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_JOIN_NUM( retVal ) \
    _ASM_GET_MUSICAL_VALUE_JOIN_NUM retVal

  .macro  _ASM_GET_MUSICAL_VALUE_JOIN_NUM retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 0
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_TOP_NUM ミュージカル：トップ回数
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_TOP_NUM( retVal ) \
    _ASM_GET_MUSICAL_VALUE_TOP_NUM retVal

  .macro  _ASM_GET_MUSICAL_VALUE_TOP_NUM retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 1
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_LAST_POINT ミュージカル：最終評価点
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_LAST_POINT( retVal ) \
    _ASM_GET_MUSICAL_VALUE_LAST_POINT retVal

  .macro  _ASM_GET_MUSICAL_VALUE_LAST_POINT retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 2
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_LAST_CONDITION ミュージカル：最終コンディション
 *                                      最高が複数の場合はMUSICAL_CONDITION_NONEを返します
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_LAST_CONDITION( retVal ) \
    _ASM_GET_MUSICAL_VALUE_LAST_CONDITION retVal

  .macro  _ASM_GET_MUSICAL_VALUE_LAST_CONDITION retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 3
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_SELECT_PROGRAM ミュージカル：選択演目番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_SELECT_PROGRAM( retVal ) \
    _ASM_GET_MUSICAL_VALUE_SELECT_PROGRAM retVal

  .macro  _ASM_GET_MUSICAL_VALUE_SELECT_PROGRAM retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 4
  .short 0
  .short \retVal
  .endm
  
//--------------------------------------------------------------
/**
 *  _SET_MUSICAL_VALUE_SELECT_PROGRAM ミュージカル：選択演目番号設定
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _SET_MUSICAL_VALUE_SELECT_PROGRAM( setVal ) \
    _ASM_SET_MUSICAL_VALUE_SELECT_PROGRAM setVal

  .macro  _ASM_SET_MUSICAL_VALUE_SELECT_PROGRAM setVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 5
  .short \setVal
  .short 0
  .endm
  
//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_IS_EQUIP_ANY ミュージカル：装備グッズがあるかチェック
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_IS_EQUIP_ANY( retVal ) \
    _ASM_GET_MUSICAL_VALUE_IS_EQUIP_ANY retVal

  .macro  _ASM_GET_MUSICAL_VALUE_IS_EQUIP_ANY retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 6
  .short 0
  .short \retVal
  .endm

  
//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_IS_EQUIP_ANY ミュージカル：ミュージカルショットが有効？
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_IS_ENABLE_SHOT( retVal ) \
    _ASM_GET_MUSICAL_VALUE_IS_ENABLE_SHOT retVal

  .macro  _ASM_GET_MUSICAL_VALUE_IS_ENABLE_SHOT retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 7
  .short 0
  .short \retVal
  .endm


//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_LAST_CONDITION_MIN ミュージカル：最終コンディション(低い
 *                  最低が複数の場合、定義順の後ろから優先的に返します。
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_LAST_CONDITION_MIN( retVal ) \
    _ASM_GET_MUSICAL_VALUE_LAST_CONDITION_MIN retVal

  .macro  _ASM_GET_MUSICAL_VALUE_LAST_CONDITION_MIN retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 8
  .short 0
  .short \retVal
  .endm


//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_LAST_CONDITION_MAX ミュージカル：最終コンディション(高い
 *                  最高が複数の場合、定義順のに優先的に返します。
 *                  すべて同じ点だった場合、MUSICAL_CONDITION_BALANCE
 *                  何も装備していなかった場合、MUSICAL_CONDITION_NONE
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_LAST_CONDITION_MAX( retVal ) \
    _ASM_GET_MUSICAL_VALUE_LAST_CONDITION_MAX retVal

  .macro  _ASM_GET_MUSICAL_VALUE_LAST_CONDITION_MAX retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 9
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_LAST_CONDITION_2ND ミュージカル：最終コンディション(２位
 *                  ２位が複数の場合、MUSICAL_CONDITION_NONE
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_LAST_CONDITION_2ND( retVal ) \
    _ASM_GET_MUSICAL_VALUE_LAST_CONDITION_2ND retVal

  .macro  _ASM_GET_MUSICAL_VALUE_LAST_CONDITION_2ND retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 10
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_COUNT_MUSICAL_POKE ミュージカル：参加可能なポケモンの数
 *  @param ret_val  戻り値  (０なら全員参加できない！
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_COUNT_MUSICAL_POKE( retVal ) \
    _ASM_GET_MUSICAL_VALUE_COUNT_MUSICAL_POKE retVal

  .macro  _ASM_GET_MUSICAL_VALUE_COUNT_MUSICAL_POKE retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 11
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_CHECK_MUSICAL_POKE ミュージカル：参加ポケモンが参加可能か？
 *  @param ret_val  戻り値  (０できない！:１できる！
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_CHECK_MUSICAL_POKE( pos, retVal ) \
    _ASM_GET_MUSICAL_VALUE_CHECK_MUSICAL_POKE pos, retVal

  .macro  _ASM_GET_MUSICAL_VALUE_CHECK_MUSICAL_POKE pos, retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 12
  .short \pos
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE_IS_EQUIP_ANY_DRESSUP ミュージカル：何か装備しているか？(試着室有効
 *  @param ret_val  戻り値  (０ない:１ある
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_VALUE_IS_EQUIP_ANY_DRESSUP( retVal ) \
    _ASM_GET_MUSICAL_VALUE_IS_EQUIP_ANY_DRESSUP retVal

  .macro  _ASM_GET_MUSICAL_VALUE_IS_EQUIP_ANY_DRESSUP retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte 13
  .short 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_FAN_VALUE ミュージカル：ミュージカル数値取得(ファン用
 *      基本的に下にあるラッパーを呼んでください
 *  @param pos  立ち位置(0～4
 *  @param type 種類(見た目・応援Msg・プレゼントMsg・プレゼント種類・プレゼント番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_FAN_VALUE( pos, type, retVal ) \
    _ASM_GET_MUSICAL_FAN_VALUE pos, type, retVal

  .macro  _ASM_GET_MUSICAL_FAN_VALUE pos, type, retVal
  .short EV_SEQ_GET_MUSICAL_FAN_VALUE
  .short \pos
  .byte \type
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_FAN_VALUE_TYPE ミュージカル：ファン見た目
 *  @param pos  立ち位置(0～4
 *  @param ret_val  戻り値(0で居ない・それ以外はOBJID
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_FAN_VALUE_TYPE( pos, retVal ) \
    _ASM_GET_MUSICAL_FAN_VALUE_TYPE pos, retVal

  .macro  _ASM_GET_MUSICAL_FAN_VALUE_TYPE pos, retVal
  .short EV_SEQ_GET_MUSICAL_FAN_VALUE
  .short \pos
  .byte 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_FAN_VALUE_CHEER_MSG ミュージカル：ファン応援メッセージ
 *  @param pos  立ち位置(0～4
 *  @param ret_val  戻り値(メッセージID
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_FAN_VALUE_CHEER_MSG( pos, retVal ) \
    _ASM_GET_MUSICAL_FAN_VALUE_CHEER_MSG pos, retVal

  .macro  _ASM_GET_MUSICAL_FAN_VALUE_CHEER_MSG pos, retVal
  .short EV_SEQ_GET_MUSICAL_FAN_VALUE
  .short \pos
  .byte 1
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_FAN_VALUE_GIFT_MSG ミュージカル：ファン贈り物メッセージ
 *  @param pos  立ち位置(0～4
 *  @param ret_val  戻り値(メッセージID
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_FAN_VALUE_GIFT_MSG( pos, retVal ) \
    _ASM_GET_MUSICAL_FAN_VALUE_GIFT_MSG pos, retVal

  .macro  _ASM_GET_MUSICAL_FAN_VALUE_GIFT_MSG pos, retVal
  .short EV_SEQ_GET_MUSICAL_FAN_VALUE
  .short \pos
  .byte 2
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_FAN_VALUE_GIFT_TYPE ミュージカル：ファン贈り物種類
 *  @param pos  立ち位置(0～4
 *  @param ret_val  戻り値(0無し・1グッズ・2通常アイテム
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_FAN_VALUE_GIFT_TYPE( pos, retVal ) \
    _ASM_GET_MUSICAL_FAN_VALUE_GIFT_TYPE pos, retVal

  .macro  _ASM_GET_MUSICAL_FAN_VALUE_GIFT_TYPE pos, retVal
  .short EV_SEQ_GET_MUSICAL_FAN_VALUE
  .short \pos
  .byte 3
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_FAN_VALUE_GIFT_NUMBER ミュージカル：ファン贈り物番号
 *  @param pos  立ち位置(0～4
 *  @param ret_val  戻り値(グッズ(アイテム)番号
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_FAN_VALUE_GIFT_NUMBER( pos, retVal ) \
    _ASM_GET_MUSICAL_FAN_VALUE_GIFT_NUMBER pos, retVal

  .macro  _ASM_GET_MUSICAL_FAN_VALUE_GIFT_NUMBER pos, retVal
  .short EV_SEQ_GET_MUSICAL_FAN_VALUE
  .short \pos
  .byte 4
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_WAITROOM_VALUE ミュージカル：ミュージカル数値取得(控え室用
 *  @param type 種類(非公開
 *  @param val  取得用番号 (非公開
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_MUSICAL_WAITROOM_VALUE( type, val, retVal ) \
    _ASM_GET_MUSICAL_WAITROOM_VALUE type, val, retVal

  .macro  _ASM_GET_MUSICAL_WAITROOM_VALUE type, val, retVal
  .short EV_SEQ_GET_MUSICAL_VALUE
  .byte \type
  .short \val
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _ADD_MUSICAL_GOODS ミュージカル：グッズ追加
 *  @param no グッズ番号
 */
//--------------------------------------------------------------
#define _ADD_MUSICAL_GOODS( no ) \
    _ASM_ADD_MUSICAL_GOODS no

  .macro  _ASM_ADD_MUSICAL_GOODS no
  .short EV_SEQ_ADD_MUSICAL_GOODS
  .short \no
  .endm

//--------------------------------------------------------------
/**
 *  _RESET_MUSICAL_FAN_GIFT_FLG ミュージカル：ファン贈り物フラグ落とす
 *  @param pos  立ち位置(0～4
 */
//--------------------------------------------------------------
#define _RESET_MUSICAL_FAN_GIFT_FLG( pos ) \
    _ASM_RESET_MUSICAL_FAN_GIFT_FLG pos

  .macro  _ASM_RESET_MUSICAL_FAN_GIFT_FLG pos, retVal
  .short EV_SEQ_RESET_MUSICAL_FAN_GIFT_FLG
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 *  _SELECT_MUSCAL_POKE ミュージカル：ミュージカル参加ポケ選択
 *  @param decide  戻り値(０キャンセル：１決定
 *  @param pos     戻り値(参加番号
 */
//--------------------------------------------------------------
#define _SELECT_MUSCAL_POKE( decide , pos ) \
    _ASM_SELECT_MUSCAL_POKE  decide , pos

  .macro  _ASM_SELECT_MUSCAL_POKE decide, pos
  .short EV_SEQ_SELECT_MUSICAL_POKE
  .short \decide
  .short \pos
  .endm


//--------------------------------------------------------------
/**
 *  _MUSICAL_TOOLS ミュージカル：汎用ツール(通信起動とか
 *  @param val1  取得用番号
 *  @param val2  取得用番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _MUSICAL_TOOLS( type, val1, val2, ret_val ) \
    _ASM_MUSICAL_TOOLS  type, val1, val2, ret_val 

  .macro  _ASM_MUSICAL_TOOLS  type, val1, val2, ret_val 
  .short EV_SEQ_MUSICAL_TOOLS
  .short \type
  .short \val1
  .short \val2
  .short \ret_val
  .endm


//--------------------------------------------------------------
/**
 *  @brief  ミュージカルドレスアップ専用マップフェードアウト呼び出し
 *
 *  @note ドレスアップ専用のフェード処理です。
 *        必ず対で使ってください。ブラックアウト・ホワイトインです
 */
//--------------------------------------------------------------
#define _DEMO_MUSICAL_DRESSUP_FADEOUT_CALL() _ASM_DEMO_FADEOUT_BLACK_WITH_BGM_CALL
#define _DEMO_MUSICAL_DRESSUP_FADEIN_CALL() _ASM_DEMO_FADEIN_WHITE_WITH_BGM_CALL


//======================================================================
//  レポート関連
//======================================================================
//--------------------------------------------------------------
/**
 * @def _REPORT_CALL
 * @brief   セーブ機能呼び出し
 * @param ret_wk    結果を受け取るワーク
 *
 * 共通スクリプトのレポート機能呼び出しから利用されている。
 * 通常のスクリプトから直接呼び出すことはない。
 */
//--------------------------------------------------------------
#define _REPORT_CALL(ret_wk)  \
    _ASM_REPORT_CALL  ret_wk

  .macro  _ASM_REPORT_CALL  ret_wk
  .short  EV_SEQ_REPORT_CALL
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _REPORT_CHECK
 * @brief セーブ状況の確認
 * @param ret_exist  [out]セーブデータが存在するかどうか
 * @param ret_new    [out]現在のプレイデータが新規データかどうか
 * @param ret_volume [out]セーブするデータ量
 */
//--------------------------------------------------------------
#define _REPORT_CHECK( ret_exist, ret_new, ret_volume )  \
    _ASM_REPORT_CHECK  ret_exist, ret_new, ret_volume 

  .macro  _ASM_REPORT_CHECK ret_exist, ret_new, ret_volume 
  .short  EV_SEQ_REPORT_CHECK
  .short  \ret_exist
  .short  \ret_new
  .short  \ret_volume 
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_FIELD_COMM_NO
 * @brief フィールドの通信状態を返す
 *
 * コモン処理 _FIELD_COMM_EXIT_EVENT_CALL の中で使います
 * 原則として単体での呼び出しはせず、コモン処理を呼んでください
 */
//--------------------------------------------------------------
#define _GET_FIELD_COMM_NO( ret_work ) \
    _ASM_GET_FIELD_COMM_NO  ret_work

  .macro  _ASM_GET_FIELD_COMM_NO  ret_work
  .short  EV_SEQ_GET_FIELD_COMM_NO
  .short  \ret_work
  .endm

//--------------------------------------------------------------
/**
 * @def _FIELD_COMM_EXIT
 * @brief フィールドマップの通信を終了する
 *
 * コモン処理 _FIELD_COMM_EXIT_EVENT_CALL を作製してあります。
 * 原則として単体での呼び出しはせず、コモン処理を呼んでください
 */
//--------------------------------------------------------------
#define _FIELD_COMM_EXIT( ret_work ) \
    _ASM_FIELD_COMM_EXIT  ret_work

  .macro  _ASM_FIELD_COMM_EXIT  ret_work
  .short  EV_SEQ_FIELD_COMM_EXIT
  .short  \ret_work
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_AVAILABLE_WIRELESS
 * @brief 通信可能な状態かどうかの取得
 * @param ret_work  TRUEの時、通信可能
 *
 * DSi本体設定での通信ON/OFF状態を取得する。DSの場合は常にTRUEとなる
 */
//--------------------------------------------------------------
#define _GET_AVAILABLE_WIRELESS( ret_work ) \
    _ASM_GET_AVAILABLE_WIRELESS ret_work

  .macro  _ASM_GET_AVAILABLE_WIRELESS ret_work
  .short  EV_SEQ_GET_AVAILABLE_WIRELESS
  .short  \ret_work
  .endm

//--------------------------------------------------------------
/**
 * @def _REBOOT_BEACON_SEARCH
 * @brief   フィールドのビーコンサーチを再開する
 *
 * 一部の通信受付終了処理でのみ使用する専用コマンドです。
 * コンフィグが「常時接続ON」でないと機能しません。
 * 通信を停止している状態でなければ機能しません。
 */
//--------------------------------------------------------------
#define _REBOOT_BEACON_SEARCH() \
    _ASM_REBOOT_BEACON_SEARCH

    .macro  _ASM_REBOOT_BEACON_SEARCH
    .short  EV_SEQ_REBOOT_BEACON_SEARCH
    .endm

//--------------------------------------------------------------
/**
 * @def _DISABLE_NETWORK_IN_IMPORTANT_EVENT
 * @brief 重要イベント中の通信遮断処理
 */
//--------------------------------------------------------------
#define _DISABLE_NETWORK_IN_IMPORTANT_EVENT() \
    _ASM_DISABLE_NETWORK_IN_IMPORTANT_EVENT

    .macro  _ASM_DISABLE_NETWORK_IN_IMPORTANT_EVENT
    .short  EV_SEQ_DISABLE_FIELD_COMM
    .endm


//--------------------------------------------------------------
/**
 * @def _ENABLE_NETWORK_IN_IMPORTANT_EVENT
 * @brief 重要イベント中の通信遮断からの復帰処理
 */
//--------------------------------------------------------------
#define _ENABLE_NETWORK_IN_IMPORTANT_EVENT()  \
    _ASM_ENABLE_NETWORK_IN_IMPORTANT_EVENT

    .macro  _ASM_ENABLE_NETWORK_IN_IMPORTANT_EVENT
    .short  EV_SEQ_ENABLE_FIELD_COMM
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_CGEAR_STATUS
 * @brief Cギアの起動状態を取得
 * @param ret_wk  TRUEのときON,FALSEのときOFF
 *
 * CギアのON/OFF状態(==常時ビーコン通信のON/OFF）を取得する
 */
//--------------------------------------------------------------
#define _GET_CGEAR_STATUS( ret_wk ) \
    _ASM_GET_CGEAR_STATUS ret_wk
    .macro  _ASM_GET_CGEAR_STATUS ret_wk
    .short  EV_SEQ_GET_CGEAR_STATUS
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_CGEAR_OFF
 * @brief CギアをOFFにする
 *
 * CギアをOFF(==常時ビーコン通信をOFF)にする。このコマンドを呼んでも
 * すぐに下画面に反映するわけではないことに留意すること。
 */
//--------------------------------------------------------------
#define _SET_CGEAR_OFF()  \
    _ASM_SET_CGEAR_OFF
    .macro  _ASM_SET_CGEAR_OFF
    .short  EV_SEQ_SET_CGEAR_OFF
    .endm

//======================================================================
//  プログラム管理データの取得・セット
//======================================================================
//--------------------------------------------------------------
/**
 * @def _GET_ROM_VERSION
 * @brief ロムバージョンの取得
 * @retval  VERSION_WHITE ホワイトバージョン
 * @retval  VERSION_BLACK ブラックバージョン
 */
//--------------------------------------------------------------
#define _GET_ROM_VERSION( ret_wk )  \
    _ASM_GET_ROM_VERSION ret_wk

    .macro _ASM_GET_ROM_VERSION ret_wk
    .short  EV_SEQ_GET_ROM_VERSION
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 *  @def  _CHG_LANGID
 *  @brief  言語ID切り替え ひらがな<->漢字
 *  @param none
 */
//--------------------------------------------------------------
#define   _CHG_LANGID() _ASM_CHG_LANGID

  .macro  _ASM_CHG_LANGID
  .short  EV_SEQ_CHG_LANGID
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_LANG_ID
 * @brief 言語モードIDの取得
 * @param ret_wk
 * @retval  0   ひらがなモード
 * @retval  1   漢字モード
 */
//--------------------------------------------------------------
#define _GET_LANG_ID(ret_wk) \
    _ASM_GET_LANG_ID ret_wk

  .macro  _ASM_GET_LANG_ID
  .short  EV_SEQ_GET_LANG_ID
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  @def  _GET_RND
 *  @brief  ランダム数取得
 *  @param ret_wk 乱数格納先
 *  @param num 乱数上限値
 *
 *  @return 乱数（値の範囲は０～ num-1 までの間）
 */
//--------------------------------------------------------------
#define _GET_RND( ret_wk, num ) _ASM_GET_RND ret_wk, num

  .macro  _ASM_GET_RND ret_wk, num
  .short  EV_SEQ_GET_RND
  .short  \ret_wk
  .short  \num
  .endm

//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _ASM_GET_NOW_MSG_ARCID  work
  .short  EV_SEQ_GET_NOW_MSG_ARCID
  .short  \work
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TIMEZONE
 * @brief 時間帯の取得
 * @param ret_wk    チェック結果を受け取るワーク
 *
 * 戻り値はprog/include/system/timezone.hを参照
 *
 * 時間帯と対応する時間は季節により変わりますので注意が必要です。
 */
//--------------------------------------------------------------
#define _GET_TIMEZONE( ret_wk ) \
    _ASM_GET_TIMEZONE ret_wk

  .macro  _ASM_GET_TIMEZONE ret_wk
  .short  EV_SEQ_GET_TIMEZONE
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TRAINER_CARD_RANK
 * @brief トレーナーカードランクの取得
 * @param ret_wk    結果を受け取るワーク
 */
//--------------------------------------------------------------
#define _GET_TRAINER_CARD_RANK( ret_wk ) \
    _ASM_GET_TRAINER_CARD_RANK ret_wk

  .macro  _ASM_GET_TRAINER_CARD_RANK ret_wk
  .short  EV_SEQ_GET_TRAINER_CARD_RANK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_WEEK
 * @brief 曜日の取得
 * @param ret_wk    結果を受け取るワーク
 *
 * 戻り値はprog\src\field\script_def.hを参照
 */
//--------------------------------------------------------------
#define _GET_WEEK( ret_wk ) \
    _ASM_GET_WEEK ret_wk

  .macro  _ASM_GET_WEEK ret_wk
  .short  EV_SEQ_GET_WEEK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_DATE
 * @brief 日付の取得
 * @param ret_month 月を受け取るワーク
 * @param ret_day   日を受け取るワーク
 */
//--------------------------------------------------------------
#define _GET_DATE( ret_month, ret_day ) \
    _ASM_GET_DATE ret_month, ret_day

  .macro  _ASM_GET_DATE ret_month, ret_day
  .short  EV_SEQ_GET_DATE
  .short  \ret_month
  .short  \ret_day
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TIME
 * @brief 現在時間の取得
 * @param ret_hour    時を受け取るワーク
 * @param ret_minute  分を受け取るワーク
 */
//--------------------------------------------------------------
#define _GET_TIME( ret_hour, ret_minute ) \
    _ASM_GET_TIME ret_hour, ret_minute

  .macro  _ASM_GET_TIME ret_hour, ret_minute
  .short  EV_SEQ_GET_TIME
  .short  \ret_hour
  .short  \ret_minute
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_SEASON_ID
 * @brief 季節の取得
 * @param ret_wk    結果を受けるワーク
 * @return  季節指定ID（prog/include/gamesystem/pm_season.h参照）
 */
//--------------------------------------------------------------
#define _GET_SEASON_ID( ret_wk )  \
    _ASM_GET_SEASON_ID ret_wk

    .macro _ASM_GET_SEASON_ID ret_wk
    .short  EV_SEQ_GET_SEASON_ID
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ZONE_ID
 * @brief 自機のいるゾーンIDを取得
 * @param ret_wk    結果を受けるワーク
 * @return  ゾーンID
 */
//--------------------------------------------------------------
#define _GET_ZONE_ID( ret_wk )  \
    _ASM_GET_ZONE_ID ret_wk

    .macro _ASM_GET_ZONE_ID ret_wk
    .short  EV_SEQ_GET_ZONE_ID
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_BIRTHDAY
 * @brief 誕生日の取得
 * @param ret_month 月を受け取るワーク
 * @param ret_day   日を受け取るワーク
 *
 * DSに登録した誕生日を取得する
 */
//--------------------------------------------------------------
#define _GET_BIRTHDAY( ret_month, ret_day ) \
    _ASM_GET_BIRTHDAY ret_month, ret_day

    .macro _ASM_GET_BIRTHDAY ret_month, ret_day
    .short  EV_SEQ_GET_BIRTHDAY
    .short  \ret_month
    .short  \ret_day
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_MY_SEX
 * @brief 自分の性別の取得
 * @param ret_wk    結果を受けるワーク
 * @retval  PM_MALE     男性
 * @retval  PM_FEMALE   女性
 */
//--------------------------------------------------------------
#define _GET_MY_SEX( ret_wk ) \
    _ASM_GET_MY_SEX ret_wk

    .macro _ASM_GET_MY_SEX ret_wk
    .short  EV_SEQ_GET_MY_SEX
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ZUKAN_COUNT
 * @brief ずかんフラグのカウント
 * @param zukan_count_type  ZUKANCTRL_MODE_SEE / ZUKANCTRL_MODE_GET
 * @param ret_wk    結果を受けるワーク
 */
//--------------------------------------------------------------
#define _GET_ZUKAN_COUNT( zukan_count_type, ret_wk ) \
    _ASM_GET_ZUKAN_COUNT zukan_count_type, ret_wk

    .macro _ASM_GET_ZUKAN_COUNT zukan_count_type, ret_wk
    .short  EV_SEQ_GET_ZUKAN_COUNT
    .short  \zukan_count_type
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_ZUKAN_FLAG
 * @brief ずかんフラグをセットする
 * @param zukan_count_type  ZUKANCTRL_MODE_SEE / ZUKANCTRL_MODE_GET
 * @param monsno    セット対象ポケモンのナンバー
 */
//--------------------------------------------------------------
#define _SET_ZUKAN_FLAG( zukan_count_type, monsno ) \
    _ASM_SET_ZUKAN_FLAG zukan_count_type, monsno

    .macro _ASM_SET_ZUKAN_FLAG zukan_count_type, monsno
    .short  EV_SEQ_SET_ZUKAN_FLAG
    .short  \zukan_count_type
    .short  \monsno
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ZUKAN_FLAG
 * @brief ずかんフラグを取得する
 * @param zukan_count_type  ZUKANCTRL_MODE_SEE / ZUKANCTRL_MODE_GET
 * @param monsno    取得対象ポケモンのナンバー
 */
//--------------------------------------------------------------
#define _GET_ZUKAN_FLAG( zukan_count_type, monsno, ret_wk ) \
    _ASM_GET_ZUKAN_FLAG zukan_count_type, monsno, ret_wk

    .macro _ASM_GET_ZUKAN_FLAG zukan_count_type, monsno, ret_wk
    .short  EV_SEQ_GET_ZUKAN_FLAG
    .short  \zukan_count_type
    .short  \monsno
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ZENKOKU_ZUKAN_FLAG
 * @brief 全国図鑑モードの確認
 * @param ret_wk
 * @retval  TRUEの時、全国図鑑モード
 * @retval  FALSEの時、地方図鑑モード
 */
//--------------------------------------------------------------
#define _GET_ZENKOKU_ZUKAN_FLAG( ret_wk ) \
    _ASM_GET_ZENKOKU_ZUKAN_FLAG ret_wk

    .macro  _ASM_GET_ZENKOKU_ZUKAN_FLAG ret_wk
    .short  EV_SEQ_GET_ZUKAN_ZENKOKU_FLAG
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_ZENKOKU_ZUKAN_FLAG
 * @brief 全国図鑑モードのセット
 */
//--------------------------------------------------------------
#define _SET_ZENKOKU_ZUKAN_FLAG() \
    _ASM_SET_ZENKOKU_ZUKAN_FLAG

    .macro  _ASM_SET_ZENKOKU_ZUKAN_FLAG ret_wk
    .short  EV_SEQ_SET_ZUKAN_ZENKOKU_FLAG
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_ZUKAN_GRAPHIC_FLAG
 * @brief ずかんバージョンアップ：フォルム詳細画面追加
 */
//--------------------------------------------------------------
#define _SET_ZUKAN_GRAPHIC_FLAG() \
    _ASM_SET_ZUKAN_GRAPHIC_FLAG

    .macro  _ASM_SET_ZUKAN_GRAPHIC_FLAG ret_wk
    .short  EV_SEQ_SET_ZUKAN_GRAPHIC_FLAG
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_BADGE_FLAG
 * @brief バッジフラグの取得
 * @param ret_wk    結果を受けるワーク
 * @param badge_id  取得するバッジのID
 */
//--------------------------------------------------------------
#define _GET_BADGE_FLAG( ret_wk, badge_id ) \
    _ASM_GET_BADGE_FLAG ret_wk, badge_id

    .macro  _ASM_GET_BADGE_FLAG ret_wk, badge_id
    .short  EV_SEQ_GET_BADGE_FLAG
    .short  \ret_wk
    .short  \badge_id
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_BADGE_FLAG
 * @brief バッジフラグのセット
 * @param badge_id  セットするバッジのID
 */
//--------------------------------------------------------------
#define _SET_BADGE_FLAG( badge_id ) \
    _ASM_SET_BADGE_FLAG badge_id

    .macro  _ASM_SET_BADGE_FLAG badge_id
    .short  EV_SEQ_SET_BADGE_FLAG
    .short  \badge_id
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_BADGE_COUNT
 * @brief バッジ数の取得
 * @param ret_wk    結果を受けるワーク
 */
//--------------------------------------------------------------
#define _GET_BADGE_COUNT( ret_wk ) \
    _ASM_GET_BADGE_COUNT ret_wk

    .macro  _ASM_GET_BADGE_COUNT ret_wk
    .short  EV_SEQ_GET_BADGE_COUNT
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_STARTMENU_FLAG
 * @brief   タイトルからのスタートメニュー項目をオープンする
 * @param type    メニュー項目指定（script_def.hのSCR_STARTMENU_FLAG_～を参照）
 */
//--------------------------------------------------------------
#define _SET_STARTMENU_FLAG( type ) \
    _ASM_SET_STARTMENU_FLAG type
    .macro  _ASM_SET_STARTMENU_FLAG type
    .short  EV_SEQ_SET_START_MENU_FLAG
    .short  \type
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_SEATEMPLE_STEP_WALK
 * @brief   海底遺跡　歩行数の取得
 * @param ret_wk  歩数格納先
 */
//--------------------------------------------------------------
#define _GET_SEATEMPLE_STEP_WALK( ret_wk ) \
    _ASM_GET_SEATEMPLE_STEP_WALK ret_wk
    .macro  _ASM_GET_SEATEMPLE_STEP_WALK ret_wk
    .short  EV_SEQ_GET_SEATEMPLE_STEP_WALK
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_STARTMENU_FLAG
 * @brief   タイトルからのスタートメニュー項目の状態取得
 * @param type    メニュー項目指定（script_def.hのSCR_STARTMENU_FLAG_～を参照）
 * @param ret_wk  戻り値（TRUE/FALSEを返す）
 */
//--------------------------------------------------------------
#define _GET_STARTMENU_FLAG( type, ret_wk ) \
    _ASM_GET_STARTMENU_FLAG type, ret_wk
    .macro  _ASM_GET_STARTMENU_FLAG type, ret_wk
    .short  EV_SEQ_GET_START_MENU_FLAG
    .short  \type
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _CHK_STARTMENU_VIEW_FLAG
 * @brief   タイトルからのスタートメニュー項目の状態取得ＮＥＷがきえているか？
 * @param type    メニュー項目指定（script_def.hのSCR_STARTMENU_FLAG_～を参照）
 * @param ret_wk  戻り値（TRUE/FALSEを返す）TRUEで表示済み
 */
//--------------------------------------------------------------
#define _CHK_STARTMENU_VIEW_FLAG( type, ret_wk ) \
    _ASM_CHK_STARTMENU_VIEW_FLAG type, ret_wk
    .macro  _ASM_CHK_STARTMENU_VIEW_FLAG type, ret_wk
    .short  EV_SEQ_CHK_START_MENU_VIEW_FLAG
    .short  \type
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _CHK_DENDOU_DATA
 * @brief「殿堂入り」データが存在するかどうかをチェックする
 * @param ret_wk  戻り値（TRUE/FALSEを返す）TRUEでデータが存在
 */
//--------------------------------------------------------------
#define _CHK_DENDOU_DATA( ret_wk ) \
    _ASM_CHK_DENDOU_DATA ret_wk
    .macro  _ASM_CHK_DENDOU_DATA ret_wk
    .short  EV_SEQ_CHK_DENDOU_DATA
    .short  \ret_wk
    .endm
    

//--------------------------------------------------------------
/**
 * @brief 全滅時戻り先の設定
 * @param warp_id ワープ飛び先指定
 *
 * 全滅時、テレポート使用時の戻り先指定をおこなう。
 * 指定可能な定義についてはresource/fldmapdata/warpdata/warpdata.hを参照。
 */
//--------------------------------------------------------------
#define _SET_WARP_ID( warp_id )  \
    _ASM_SET_WARP_ID warp_id

    .macro  _ASM_SET_WARP_ID warp_id
    .short  EV_SEQ_SET_WARP_ID
    .short  \warp_id
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_SP_LOCATION_HERE
 * @brief 特殊接続先の設定：現在位置
 *
 * @note
 * グリッドマップにしか対応していない
 */
//--------------------------------------------------------------
#define _SET_SP_LOCATION_HERE() \
      _ASM_SET_SP_LOCATION_HERE

    .macro _ASM_SET_SP_LOCATION_HERE
    .short  EV_SEQ_SET_SP_LOCATION_HERE
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_SP_LOCATION_DIRECT
 * @brief 特殊接続先の設定：直接指定
 * @param zone_id マップID指定
 * @param dir 方向指定
 * @param gx  X位置指定（グリッド単位）
 * @param gy  Y位置指定（グリッド単位）
 * @param gz  Z位置指定（グリッド単位）
 *
 * @note
 * グリッドマップにしか対応していない
 */
//--------------------------------------------------------------
#define _SET_SP_LOCATION_DIRECT( zone_id, dir, gx, gy, gz ) \
      _ASM_SET_SP_LOCATION_DIRECT zone_id, dir, gx, gy, gz 

    .macro _ASM_SET_SP_LOCATION_DIRECT zone_id, dir, gx, gy, gz 
    .short  EV_SEQ_SET_SP_LOCATION_DIRECT
    .short  \zone_id
    .short  \dir
    .short  \gx, \gy, \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _CHANGE_MAPREPLACE_FLAG
 * @brief マップ置き換えイベントのフラグ操作
 * @param id            マップ置き換えの指定ID（prog/src/field/map_replace.h参照）
 * @param on_off        置き換えのON/OFF指定
 * @param direct_flag   即時反映フラグ（TRUE/FALSE)
 *
 * @note
 * マップ置き換えシステムが参照するイベントフラグの操作を行うスクリプトコマンド。
 * 即時反映フラグをTRUEに指定しても、フィールドから抜けて場面転換を行わない限りは反映しないので注意。
 */
//--------------------------------------------------------------
#define _CHANGE_MAPREPLACE_FLAG( id, on_off, direct_flag ) \
    _ASM_CHANGE_MAPREPLACE_FLAG id, on_off, direct_flag

    .macro  _ASM_CHANGE_MAPREPLACE_FLAG id, on_off, direct_flag
    .short  EV_SEQ_CHANGE_MAP_REPLACE_FLAG
    .short  \id
    .short  \on_off
    .short  \direct_flag
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_MAPREPLACE_FLAG
 * @brief マップ置き換えイベントのフラグ取得
 * @param id            マップ置き換えの指定ID（prog/src/field/map_replace.h参照）
 * @param ret_wk        取得するためのワーク
 * @return  BOOL        TRUEのとき、置き換えリクエストあり
 *
 * @note
 * _CHANGE_MAPREPLACE_FLAG
 * マップ置き換えシステムが参照するイベントフラグの取得をおこなう。
 */
//--------------------------------------------------------------
#define _GET_MAPREPLACE_FLAG( id, ret_wk ) \
    _ASM_GET_MAPREPLACE_FLAG id, ret_wk

    .macro  _ASM_GET_MAPREPLACE_FLAG id, ret_wk
    .short  EV_SEQ_GET_MAP_REPLACE_FLAG
    .short  \id
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_WIRELESS_SAVEMODE
 * @brief ワイヤレスセーブモードの取得
 */
//--------------------------------------------------------------
#define _GET_WIRELESS_SAVEMODE( ret_wk ) \
      _ASM_GET_WIRELESS_SAVEMODE ret_wk

    .macro  _ASM_GET_WIRELESS_SAVEMODE ret_wk
    .short  EV_SEQ_GET_WIRELESS_SAVEMODE
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_SHOES
 * @brief シューズの取得
 */
//--------------------------------------------------------------
#define _GET_SHOES() _ASM_GET_SHOES

    .macro  _ASM_GET_SHOES
    .short  EV_SEQ_GET_SHOES
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_BOX_WALLPAPER
 * @brief ボックスの追加壁紙取得
 * @param wallpaper_flag  script_def.hのSCR_BOX_EX_WALLPAPER?を使用
 */
//--------------------------------------------------------------
#define _GET_BOX_WALLPAPER( flag )  \
    _ASM_GET_BOX_WALLPAPER flag

    .macro  _ASM_GET_BOX_WALLPAPER flag
    .short  EV_SEQ_GET_BOX_WALLPAPER
    .short  \flag
    .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_PLAYER_VIEW_CHANGE
 * @brief プレイヤーのトレーナータイプが変更されているかどうかをチェックする.
 *        変更されていない場合, 
 *        → 変更済みに設定し, 
 *        → デフォルトで設定されたトレーナータイプを取得する.
 * @param ret_wk_chng トレーナータイプ変更状態の格納先
 * @param ret_wk_type プレイヤーのトレイナータイプの格納先
 *
 * ※ret_wk_chng
 * →TRUE  トレーナータイプ変更済み
 * →FALSE トレーナータイプ変更してない
 */
//--------------------------------------------------------------
#define _CHECK_PLAYER_VIEW_CHANGE( ret_wk_chng, ret_wk_type )  \
    _ASM_CHECK_PLAYER_VIEW_CHANGE ret_wk_chng, ret_wk_type

    .macro  _ASM_CHECK_PLAYER_VIEW_CHANGE ret_wk_chng, ret_wk_type
    .short  EV_SEQ_CHECK_PLAYER_VIEW_CHANGE
    .short  \ret_wk_chng
    .short  \ret_wk_type
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_GYM_VICTORY_INFO
 * @brief ジムリーダー戦勝利時の情報セット
 * @param gym_id  BADGE_ID_??を指定してください
 */
//--------------------------------------------------------------
#define _SET_GYM_VICTORY_INFO( gym_id ) \
      _ASM_SET_GYM_VICTORY_INFO gym_id

    .macro  _ASM_SET_GYM_VICTORY_INFO gym_id
    .short  EV_SEQ_SET_GYM_VICTORY_INFO
    .short  \gym_id
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_GYM_VICTORY_INFO
 * @brief ジムリーダー戦勝利時の手持ちポケモン種族名をセットする
 * @param gym_id  BADGE_ID_??を指定してください
 * @param ret_wk  ジムリーダー戦勝利時の手持ちポケモン数
 */
//--------------------------------------------------------------
#define _GET_GYM_VICTORY_INFO( gym_id, ret_wk ) \
    _ASM_GET_GYM_VICTORY_INFO gym_id, ret_wk

    .macro _ASM_GET_GYM_VICTORY_INFO gym_id, ret_wk
    .short  EV_SEQ_GET_GYM_VICTORY_INFO
    .short  \gym_id
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def   _SET_FAVORITE_POKEMON
 * @brief 好きなポケモンの指定（バトルレコーダー用）
 * @param pos   指定するポケモンの位置
 */
//--------------------------------------------------------------
#define _SET_FAVORITE_POKEMON( pos ) \
    _ASM_SET_FAVORITE_POKEMON pos
    .macro  _ASM_SET_FAVORITE_POKEMON pos
    .short  EV_SEQ_SET_FAVORITE_POKEMON
    .short  \pos
    .endm

//--------------------------------------------------------------
/**
 * @def	_GET_BREEDER_JUDGE_RESULT
 * @brief ポケモン個体値の評価パラメータ取得
 * @param pos   指定するポケモンの位置
 * @param	mode	(script_def.hのSCR_JUDGE_MODE_～を参照）
 * @param	ret_wk		取得した値
 */
//--------------------------------------------------------------
#define	_GET_BREEDER_JUDGE_RESULT( pos, mode, ret_wk ) \
		_ASM_GET_BREEDER_JUDGE_RESULT pos, mode, ret_wk

		.macro	_ASM_GET_BREEDER_JUDGE_RESULT pos, mode, ret_wk
		.short	EV_SEQ_GET_BREEDER_JUDGE_RESULT
		.short	\pos
		.short	\mode
		.short	\ret_wk
		.endm

//--------------------------------------------------------------
/**
 * @def _WAZAMACHINE_LOT
 * @brief 保持わざマシンから抽選する
 * @param	ret_wk		抽選された技ナンバー（０の時、対象なし）
 *
 * わざマシンナンバーでなく、技ナンバーを返す
 */
//--------------------------------------------------------------
#define _WAZAMACHINE_LOT( ret_wk ) \
    _ASM_WAZAMACHINE_LOT ret_wk

    .macro  _ASM_WAZAMACHINE_LOT ret_wk
    .short  EV_SEQ_WAZAMACHINE_LOT
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _TRADEPOKE_AFTER_SAVE
 * @brief 交換後ポケモン　対戦用のデータ　として保存
 * @param	party_pos   保存するポケモンの位置
 * @param	trade_type  保存する交換タイプ
 *
#define SCR_TRPOKE_AFTER_SAVE_C02 (0)    // シッポウ
#define SCR_TRPOKE_AFTER_SAVE_C05 (1)    // ホドモエ
 */
//--------------------------------------------------------------
#define _TRADEPOKE_AFTER_SAVE( party_pos, trade_type ) \
    _ASM_TRADEPOKE_AFTER_SAVE party_pos, trade_type

    .macro  _ASM_TRADEPOKE_AFTER_SAVE party_pos, trade_type
    .short  EV_SEQ_TRADEPOKE_AFTER_SAVE
    .short  \party_pos
    .short  \trade_type
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_WORD_TRADEAFTER_POKENAME
 * @brief 交換後ポケモン　ニックネームをワードセットに設定
 * @param	trade_type  保存する交換タイプ
 * @param	word_idx    GMMのワードバッファインデックス
 *
#define SCR_TRPOKE_AFTER_SAVE_C02 (0)    // シッポウ
#define SCR_TRPOKE_AFTER_SAVE_C05 (1)    // ホドモエ
 */
//--------------------------------------------------------------
#define _SET_WORD_TRADEAFTER_POKENAME( trade_type, word_idx ) \
    _ASM_SET_WORD_TRADEAFTER_POKENAME trade_type, word_idx

    .macro  _ASM_SET_WORD_TRADEAFTER_POKENAME trade_type, word_idx
    .short  EV_SEQ_SET_WORD_TRADEAFTER_POKENAME
    .short  \trade_type
    .short  \word_idx
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ZUKAN_HYOUKA
 * @brief ずかん評価用メッセージID取得
 * @param mode      ずかん評価モード（script_def.hのSCR_ZUKAN_HYOUKA_MODE_～を参照）
 * @param	ret_msgid		取得したMSGID
 * @param ret_count   ポケモン数
 */
//--------------------------------------------------------------
#define _GET_ZUKAN_HYOUKA( mode, ret_msgid, ret_count ) \
    _ASM_GET_ZUKAN_HYOUKA mode, ret_msgid, ret_count

    .macro  _ASM_GET_ZUKAN_HYOUKA mode, ret_msgid, ret_count
    .short  EV_SEQ_GET_ZUKAN_HYOUKA
    .short  \mode
    .short  \ret_msgid
    .short  \ret_count
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_ZUKAN_COMPLETE
 * @brief ずかん完成チェック
 * @param ret_wk    TRUEのとき、完成
 * @param mode      ずかん評価モード（script_def.hのSCR_ZUKAN_HYOUKA_MODE_～を参照）
 *
 * @li  SCR_ZUKAN_HYOUKA_MODE_GLOBAL    全国図鑑でのチェック
 * @li  SCR_ZUKAN_HYOUKA_MODE_LOCAL_GET ローカル図鑑でのチェック
 * @li  その他のSCR_ZUKAN_HYOUKA_MODE_～には対応していない
 */
//--------------------------------------------------------------
#define _GET_ZUKAN_COMPLETE( ret_wk, mode ) \
    _ASM_GET_ZUKAN_COMPLETE ret_wk, mode

    .macro  _ASM_GET_ZUKAN_COMPLETE ret_wk, mode
    .short  EV_SEQ_GET_ZUKAN_COMPLETE
    .short  \ret_wk
    .short  \mode
    .endm

//======================================================================
//
//    
//
//======================================================================
//--------------------------------------------------------------
/**
 * @def _CGEAR_ON_DEMO
 * @brief Cギア起動デモ
 *
 * @param power_on  TRUE:電源ON状態　FALSE:電源OFF状態
 */
//--------------------------------------------------------------
#define _CGEAR_ON_DEMO( power_on )  \
    _ASM_CGEAR_ON_DEMO power_on

    .macro  _ASM_CGEAR_ON_DEMO power_on
    .short  EV_SEQ_CALL_CGEAR_GET_DEMO
    .short  \power_on
    .endm

//--------------------------------------------------------------
/**
 * @def _PUT_CGEAR_COMMENT
 * @brief Cギア　警告comment表示
 *
 * @param on_off    TRUE:ON  FALSE:OFF
 */
//--------------------------------------------------------------
#define _PUT_CGEAR_COMMENT( on_off )  \
    _ASM_PUT_CGEAR_COMMENT on_off

    .macro  _ASM_PUT_CGEAR_COMMENT on_off
    .short  EV_SEQ_PUT_CGEAR_COMMENT
    .short  \on_off
    .endm


//--------------------------------------------------------------
/**
 * @def _GOTO_GAMECLEAR_DEMO
 * @brief ゲームクリアデモへの移行
 * @param mode  SCR_GAMECLEAR_MODE_（script_def.h）参照
 *
 * @li  SCR_GAMECLEAR_MODE_FIRST  初回ゲームクリア
 * @li  SCR_GAMECLEAR_MODE_DENDOU ２回目以降、殿堂入り
 */
//--------------------------------------------------------------
#define _GOTO_GAMECLEAR_DEMO( mode ) \
    _ASM_GOTO_GAMECLEAR_DEMO mode

    .macro  _ASM_GOTO_GAMECLEAR_DEMO mode
    .short  EV_SEQ_GOTO_GAMECLEAR
    .short  \mode
    .endm


//--------------------------------------------------------------
/**
 * @def _CHNAGE_SUBSCREEN
 * @brief サブスクリーンを変更
 * @param mode  サブスクリーンのモード
 * mode 
 *  SCR_EV_SUBSCREEN_MODE_SAVE    セーブ画面にする
 */
//--------------------------------------------------------------
#define _CHNAGE_SUBSCREEN( mode ) \
    _ASM_CHNAGE_SUBSCREEN mode

    .macro  _ASM_CHNAGE_SUBSCREEN mode
    .short  EV_SEQ_CHANGE_SUBSCREEN
    .short  \mode
    .endm

//--------------------------------------------------------------
/**
 * @def _BACK_SUBSCREEN_NORMAL
 * @brief サブスクリーンを通常状態に戻す
 */
//--------------------------------------------------------------
#define _BACK_SUBSCREEN_NORMAL() \
    _ASM_BACK_SUBSCREEN_NORMAL

    .macro  _ASM_BACK_SUBSCREEN_NORMAL
    .short  EV_SEQ_BACK_SUBSCREEN_NORMAL
    .endm




//======================================================================
//
//
//    ポケモン関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 手持ちポケモン回復
 */
//--------------------------------------------------------------
#define _TEMOTI_POKEMON_KAIFUKU() \
    _ASM_TEMOTI_POKEMON_KAIFUKU

  .macro  _ASM_TEMOTI_POKEMON_KAIFUKU
  .short  EV_SEQ_POKEMON_RECOVER
  .endm

//--------------------------------------------------------------
/**
 * @def _PARTY_POKE_NAME_INPUT
 * @brief 手持ちポケモンの名前入力画面を呼び出す
 * @param ret_wk 入力結果を受け取るワーク
 * @param index  名前を設定する手持ちポケモンのインデックスを指定
 * @param fade_flag 画面フェードを自動でおこなうかどうか
 * @retval  TRUEのとき、入力した名前を確定した場合
 * @retval  FALSEのとき、入力をキャンセルした場合
 *
 * @li  引数fade_flag=TRUEのとき、呼び出し前後のフェードは自動で行われる
 * @li  引数fade_flag=FALSEのとき、スクリプトでフェード処理を記述すること
 */
//--------------------------------------------------------------
#define _PARTY_POKE_NAME_INPUT( ret_wk, index, fade_flag ) \
    _ASM_PARTY_POKE_NAME_INPUT ret_wk, index, fade_flag

  .macro  _ASM_PARTY_POKE_NAME_INPUT ret_wk, index, fade_flag
  .short  EV_SEQ_PARTY_POKE_NAME_INPUT
  .short  \ret_wk
  .short  \index
  .short  \fade_flag
  .endm

//--------------------------------------------------------------
/**
 * @def _PARTY_POKE_EGG_BIRTH
 * @brief 手持ちのタマゴを孵化させる
 * @param ret_wk 生まれたポケモンの手持ちインデックスを受け取るワーク
 */
//--------------------------------------------------------------
#define _PARTY_POKE_EGG_BIRTH( ret_wk ) \
    _ASM_PARTY_POKE_EGG_BIRTH ret_wk

  .macro  _ASM_PARTY_POKE_EGG_BIRTH ret_wk
  .short  EV_SEQ_PARTY_POKE_EGG_BIRTH
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_TEMOTI_POKERUS
 * @brief 手持ちにポケルスがいるかどうかのチェック
 * @param ret_wk    チェック結果を受け取るワーク
 * @retval  TRUEのとき、ポケルス感染したポケモンが手持ちにいる
 * @retval  FALSEのとき、感染したポケモンはいない
 */
//--------------------------------------------------------------
#define _CHECK_TEMOTI_POKERUS( ret_wk ) \
    _ASM_CHECK_TEMOTI_POKERUS ret_wk

  .macro  _ASM_CHECK_TEMOTI_POKERUS ret_wk
  .short  EV_SEQ_CHECK_TEMOTI_POKERUS
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_TEMOTI_HP
 * @brief 手持ちポケモンのHPチェック
 * @param ret_wk  チェック結果を受け取るワーク
 * @param pos     チェックするポケモンの位置（０～５）
 * @retval  TRUE    チェックしたポケモンのHPが満タン
 * @retval  FALSE   チェックしたポケモンのHPは減っている
 */
//--------------------------------------------------------------
#define _CHECK_TEMOTI_HP( ret_wk, pos ) \
    _ASM_CHECK_TEMOTI_HP ret_wk, pos


  .macro  _ASM_CHECK_TEMOTI_HP ret_wk, pos
  .short  EV_SEQ_CHECK_TEMOTI_HP
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_TEMOTI_EGG
 * @brief 手持ちポケモンのEGGチェック
 * @param ret_wk  チェック結果を受け取るワーク
 * @param pos     チェックするポケモンの位置（０～５）
 * @retval  TRUE    チェックしたポケモンがタマゴの場合
 * @retval  FALSE   チェックしたポケモンがタマゴでない場合
 */
//--------------------------------------------------------------
#define _CHECK_TEMOTI_EGG( ret_wk, pos ) \
    _ASM_CHECK_TEMOTI_EGG ret_wk, pos


  .macro  _ASM_CHECK_TEMOTI_EGG ret_wk, pos
  .short  EV_SEQ_CHECK_TEMOTI_EGG
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_PARTY_POKE_COUNT
 * @brief 手持ちポケモンの数を取得
 * @param ret_wk チェック結果を受け取るワーク
 	!!注意!!
 	返り値は「１」から始まります。手持ちのポジション指定は「0」始まりなので合わせて使う場合は注意。
 * @param count_mode 数え方指定(POKECOUNT_MODE_XXXX)
 		手持ちポケモン数カウントモード
 		POKECOUNT_MODE_TOTAL         (0)  // 手持ちの数
 		POKECOUNT_MODE_NOT_EGG       (1)  // タマゴを除く手持ちの数
 		POKECOUNT_MODE_BATTLE_ENABLE (2)  // 戦える(タマゴと瀕死を除いた)ポケモン数
 		POKECOUNT_MODE_ONLY_EGG      (3)  // タマゴの数(駄目タマゴを除く) 
 		POKECOUNT_MODE_ONLY_DAME_EGG (4)  // 駄目タマゴの数
    POKECOUNT_MODE_EMPTY         (5)  // 空いている数
 	* @retval u16 手持ちポケモンの数
 */
//--------------------------------------------------------------
#define _GET_PARTY_POKE_COUNT( ret_wk, count_mode ) \
    _ASM_GET_PARTY_POKE_COUNT ret_wk, count_mode

  .macro _ASM_GET_PARTY_POKE_COUNT ret_wk, count_mode
  .short EV_SEQ_GET_PARTY_POKE_COUNT
  .short \ret_wk
  .short \count_mode
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_PARTY_POKE_COUNT_BY_MONSNO
 * @brief 指定モンスターナンバーのポケが手持ちに何匹いるかを返す(タマゴは除外)
 * @param monsno  対象モンスターナンバー
 * @param ret_wk  結果を受け取るワーク
 * @retval  0～6
 */
//--------------------------------------------------------------  
#define _GET_PARTY_POKE_COUNT_BY_MONSNO(monsno, ret_wk) \
    _ASM_GET_PARTY_POKE_COUNT_BY_MONSNO monsno, ret_wk

  .macro  _ASM_GET_PARTY_POKE_COUNT_BY_MONSNO monsno, ret_wk
  .short  EV_SEQ_GET_PARTY_POKE_COUNT_BY_MONSNO
  .short  \monsno
  .short  \ret_wk
  .endm  

//--------------------------------------------------------------
/**
 * @def _GET_PARTY_FRONT_POKEMON
 * @brief 手持ちの先頭ポケモンがいる位置を取得
 * @param ret_wk チェック結果を受け取るワーク（0～5の数値で返します）
 * @param count_mode 数え方指定(POKECOUNT_MODE_XXXX)
 		POKECOUNT_MODE_NOT_EGG       (1)  // タマゴを除く
 		POKECOUNT_MODE_BATTLE_ENABLE (2)  // 戦える(タマゴと瀕死を除いた)ポケモン
 	* @retval u16 位置
  *
  * _GET_PARTY_POKE_COUNTと同じ引数をとるが、使えないものもあるので注意！
 */
//--------------------------------------------------------------
#define _GET_PARTY_FRONT_POKEMON( ret_wk, count_mode ) \
    _ASM_GET_PARTY_FRONT_POKEMON ret_wk, count_mode

    .macro  _ASM_GET_PARTY_FRONT_POKEMON ret_wk, count_mode
    .short  EV_SEQ_GET_PARTY_FRONT_POKEMON
    .short  \ret_wk
    .short  \count_mode
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_PARTY_POKE_PARAM
 * @brief 手持ちポケモンのパラメータ取得
 * @param ret_wk      結果を受け取るワーク
 * @param para_id     取得したいパラメータ指定ID（script_def.hのSCR_POKEPARA_～）
 * @param pos         モンスターナンバーを調べる手持ちポケモン番号
 *
 * @note
 * SCR_POKEPARA_～のうち、ROもしくはRWと書かれているものを指定することができます。
 */
//--------------------------------------------------------------
#define _GET_PARTY_POKE_PARAM( ret_wk, pos, para_id ) \
    _ASM_GET_PARTY_POKE_PARAM ret_wk, pos, para_id

    .macro  _ASM_GET_PARTY_POKE_PARAM ret_wk, pos, para_id
    .short  EV_SEQ_GET_PARTY_POKE_PARAMETER
    .short  \ret_wk
    .short  \pos
    .short  \para_id
    .endm

//--------------------------------------------------------------
/**
 * @def _SET_PARTY_POKE_PARAM
 * @brief 手持ちポケモンのパラメータセット
 * @param para_id     取得したいパラメータ指定ID（script_def.hのSCR_POKEPARA_～）
 * @param pos         モンスターナンバーを調べる手持ちポケモン番号
 * @param value       セットする値
 *
 * @note
 * SCR_POKEPARA_～のうち、WOもしくはRWと書かれているものを指定することができます。
 */
//--------------------------------------------------------------
#define _SET_PARTY_POKE_PARAM( pos, para_id, value ) \
    _ASM_SET_PARTY_POKE_PARAM pos, para_id, value

    .macro  _ASM_SET_PARTY_POKE_PARAM pos, para_id, value
    .short  EV_SEQ_SET_PARTY_POKE_PARAMETER
    .short  \pos
    .short  \para_id
    .short  \value
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_PARTY_POKE_MONSNO
 * @brief 指定した手持ちポケモンのモンスターナンバーを取得する
 * @param ret_wk 結果を受け取るワーク
 * @param pos    モンスターナンバーを調べる手持ちポケモン番号
 * @retval u16   指定ポケモンのモンスターナンバー
 */
//--------------------------------------------------------------
#define _GET_PARTY_POKE_MONSNO( ret_wk, pos ) \
    _ASM_GET_PARTY_POKE_MONSNO ret_wk, pos

  .macro _ASM_GET_PARTY_POKE_MONSNO ret_wk, pos
  .short EV_SEQ_GET_PARTY_POKE_MONSNO
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_PARTY_POKE_FORMNO
 * @brief 指定した手持ちポケモンの形状ナンバーを取得する
 * @param ret_wk 結果を受け取るワーク
 * @param pos    形状ナンバーを調べる手持ちポケモン番号
 * @retval u16   指定ポケモンの形状ナンバー
 */
//--------------------------------------------------------------
#define _GET_PARTY_POKE_FORMNO( ret_wk, pos ) \
    _ASM_GET_PARTY_POKE_FORMNO ret_wk, pos

  .macro _ASM_GET_PARTY_POKE_FORMNO ret_wk, pos
  .short EV_SEQ_GET_PARTY_POKE_FORMNO
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _ADD_POKEMON_TO_PARTY
 * @brief ポケモンを手持ちに追加する
 * @param ret_wk 結果を受け取るワーク
 * @param monsno  モンスターナンバー
 * @param formno  フォームナンバー
 * @param level   レベル
 * @retval BOOL 追加できたらTRUE
 */
//--------------------------------------------------------------
#define _ADD_POKEMON_TO_PARTY( ret_wk, monsno, formno, level ) \
    _ASM_ADD_POKEMON_TO_PARTY ret_wk, monsno, formno, level

  .macro _ASM_ADD_POKEMON_TO_PARTY ret_wk, monsno, formno, level
  .short EV_SEQ_ADD_POKEMON_TO_PARTY
  .short \ret_wk
  .short \monsno
  .short \formno
  .short \level
  .endm

//--------------------------------------------------------------
/**
 * @def _ADD_POKEMON_TO_PARTY_EX
 * @brief ポケモンを手持ちに追加する  詳細設定型
 * @param ret_wk 結果を受け取るワーク
 * @param monsno  モンスターナンバー
 * @param formno  フォームナンバー
 * @param level   レベル
 * @param tokusei 特性指定    SCR_TOKUSEI_SPEC～を指定  script_def.h参照
 * @param sex     性別指定    SCR_SEX_SPEC～を指定  script_def.h参照
 * @param rare    レア指定    SCR_RARE_SPEC～を指定  script_def.h参照
 * @param itemno  所持アイテム
 * @param ball    捕獲ボールアイテムナンバー
 * @retval BOOL 追加できたらTRUE
 */
//--------------------------------------------------------------
#define _ADD_POKEMON_TO_PARTY_EX( ret_wk, monsno, formno, level, tokusei, sex, rare, itemno, ball ) \
    _ASM_ADD_POKEMON_TO_PARTY_EX ret_wk, monsno, formno, level, tokusei, sex, rare, itemno, ball

  .macro _ASM_ADD_POKEMON_TO_PARTY_EX ret_wk, monsno, formno, level, tokusei, sex, rare, itemno, ball
  .short EV_SEQ_ADD_POKEMON_TO_PARTY_EX
  .short \ret_wk
  .short \monsno
  .short \formno
  .short \level
  .short \tokusei
  .short \sex
  .short \rare
  .short \itemno
  .short \ball
  .endm
  

//--------------------------------------------------------------
/**
 * @def _ADD_POKEMON_TO_BOX
 * @brief ポケモンをボックスに追加する
 * @param ret_wk 結果を受け取るワーク
 * @param monsno  モンスターナンバー
 * @param formno  フォームナンバー
 * @param level   レベル
 * @retval BOOL 追加できたらTRUE
 */
//--------------------------------------------------------------
#define _ADD_POKEMON_TO_BOX( ret_wk, monsno, formno, level ) \
    _ASM_ADD_POKEMON_TO_BOX ret_wk, monsno, formno, level

  .macro _ASM_ADD_POKEMON_TO_BOX ret_wk, monsno, formno, level
  .short EV_SEQ_ADD_POKEMON_TO_BOX
  .short \ret_wk
  .short \monsno
  .short \formno
  .short \level
  .endm

//--------------------------------------------------------------
/**
 * @def _ADD_POKEMON_TO_BOX_EX
 * @brief ポケモンをボックスに追加する　詳細設定型
 * @param ret_wk 結果を受け取るワーク
 * @param monsno  モンスターナンバー
 * @param formno  フォームナンバー
 * @param level   レベル
 * @param tokusei 特性指定    SCR_TOKUSEI_SPEC～を指定  script_def.h参照
 * @param sex     性別指定    SCR_SEX_SPEC～を指定  script_def.h参照
 * @param rare    レア指定    SCR_RARE_SPEC～を指定  script_def.h参照
 * @param itemno  所持アイテム
 * @param ball    捕獲ボールアイテムナンバー
 * @retval BOOL 追加できたらTRUE
 */
//--------------------------------------------------------------
#define _ADD_POKEMON_TO_BOX_EX( ret_wk, monsno, formno, level, tokusei, sex, rare, itemno, ball ) \
    _ASM_ADD_POKEMON_TO_BOX_EX ret_wk, monsno, formno, level, tokusei, sex, rare, itemno, ball

  .macro _ASM_ADD_POKEMON_TO_BOX_EX ret_wk, monsno, formno, level, tokusei, sex, rare, itemno, ball
  .short EV_SEQ_ADD_POKEMON_TO_BOX_EX
  .short \ret_wk
  .short \monsno
  .short \formno
  .short \level
  .short \tokusei
  .short \sex
  .short \rare
  .short \itemno
  .short \ball
  .endm

//--------------------------------------------------------------
/**
 * @def _ADD_TAMAGO_TO_PARTY
 * @brief タマゴを手持ちに追加する 18番道路でもらうタマゴ専用コマンド
 * @param ret_wk 結果を受け取るワーク
 * @param monsno  モンスターナンバー
 * @param formno  フォームナンバー
 * @retval BOOL 追加できたらTRUE
 */
//--------------------------------------------------------------
#define _ADD_TAMAGO_TO_PARTY( ret_wk, monsno, formno ) \
    _ASM_ADD_TAMAGO_TO_PARTY ret_wk, monsno, formno

  .macro _ASM_ADD_TAMAGO_TO_PARTY ret_wk, monsno, formno, tokusei, level, itemno, ball
  .short EV_SEQ_ADD_TAMAGO_TO_PARTY
  .short \ret_wk
  .short \monsno
  .short \formno
  .endm
  
//--------------------------------------------------------------
/**
 * @def _CHECK_POKE_WAZA
 * @brief 指定された技を覚えているか調べる
 * @param ret_wk チェック結果を格納するワーク 0=覚えていない 1=覚えている
 * @param wazano チェックする技ナンバー
 * @param tno 調べる手持ちポケモンの一番号
 */
//--------------------------------------------------------------
#define _CHECK_POKE_WAZA( ret_wk, wazano, tno ) \
    _ASM_CHECK_POKE_WAZA ret_wk, wazano, tno

  .macro _ASM_CHECK_POKE_WAZA ret_wk, wazano, tno
  .short EV_SEQ_CHK_POKE_WAZA
  .short \ret_wk
  .short \wazano
  .short \tno
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_POKE_WAZA_GROUP 
 * @brief 指定された技を覚えているか調べる（手持ち全体）
 * @param ret_wk チェック結果を格納するワーク 技を覚えている手持ち位置。 6=覚えていない
 * @param wazano チェックする技ナンバー
 */
//--------------------------------------------------------------
#define _CHECK_POKE_WAZA_GROUP( ret_wk, wazano ) \
    _ASM_CHECK_POKE_WAZA_GROUP ret_wk, wazano

  .macro _ASM_CHECK_POKE_WAZA_GROUP ret_wk, wazano
  .short EV_SEQ_CHK_POKE_WAZA_GROUP
  .short \ret_wk
  .short \wazano
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TEMOTI_NATSUKI
 * @brief 手持ちポケモンのなつき度チェック
 * @param ret_wk  チェック結果を受け取るワーク
 * @param pos     チェックするポケモンの位置（０～５）
 * @return        チェックしたポケモンのなつき度
 */
//--------------------------------------------------------------
#define _GET_TEMOTI_NATSUKI( ret_wk, pos ) \
    _ASM_CHECK_GET_TEMOTI_NATSUKI ret_wk, pos

  .macro  _ASM_CHECK_GET_TEMOTI_NATSUKI ret_wk, pos
  .short  EV_SEQ_GET_TEMOTI_NATSUKI
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _SET_TEMOTI_NATSUKI
 * @brief 手持ちポケモンのなつき度をセット
 * @param pos     なつき度を操作するポケモンの位置（０～５）
 * @param value   設定したいなつき度
 * @param mode    コマンドの呼び出しモード(0:セット,1:加算,2:減算)
 */
//--------------------------------------------------------------
#define _SET_TEMOTI_NATSUKI( pos, value ) \
    _ASM_CHECK_SET_TEMOTI_NATSUKI pos, value

  .macro  _ASM_CHECK_SET_TEMOTI_NATSUKI pos,value
  .short  EV_SEQ_SET_TEMOTI_NATSUKI
  .short  \pos
  .short  \value
  .short   0
  .endm

//--------------------------------------------------------------
/**
 * @def _ADD_TEMOTI_NATSUKI
 * @brief 手持ちポケモンのなつき度を加算
 * @param pos     なつき度を操作するポケモンの位置（０～５）
 * @param value   加算したい値
 * @param mode    コマンドの呼び出しモード(0:セット,1:加算,2:減算)
 */
//--------------------------------------------------------------
#define _ADD_TEMOTI_NATSUKI( pos, value ) \
    _ASM_CHECK_ADD_TEMOTI_NATSUKI pos, value

  .macro  _ASM_CHECK_ADD_TEMOTI_NATSUKI pos,value
  .short  EV_SEQ_SET_TEMOTI_NATSUKI
  .short  \pos
  .short  \value
  .short  1
  .endm

//--------------------------------------------------------------
/**
 * @def _SUB_TEMOTI_NATSUKI
 * @brief 手持ちポケモンのなつき度を減算
 * @param pos     なつき度を操作するポケモンの位置（０～５）
 * @param value   減算したい値
 * @param mode    コマンドの呼び出しモード(0:セット,1:加算,2:減算)
 */
//--------------------------------------------------------------
#define _SUB_TEMOTI_NATSUKI( pos, value ) \
    _ASM_CHECK_SUB_TEMOTI_NATSUKI pos, value

  .macro  _ASM_CHECK_SUB_TEMOTI_NATSUKI pos,value
  .short  EV_SEQ_SET_TEMOTI_NATSUKI
  .short  \pos
  .short  \value
  .short  2
  .endm

//--------------------------------------------------------------
/**
 * @def _SELECT_TEMOTI_POKE
 * @brief 汎用手持ちポケモン選択
 * @param decide 選択結果の格納先 (TRUE：FALSE 選択したか)
 * @param ret_wk 選択結果の格納先 (０～５ 選択位置)
 * @param value  予備ワーク (０を入れておいてください
 * @retval u16   選択結果
 */
//--------------------------------------------------------------
#define _SELECT_TEMOTI_POKE( decide , ret_wk , value ) \
    _ASM_SELECT_TEMOTI_POKE decide, ret_wk, value

  .macro  _ASM_SELECT_TEMOTI_POKE decide, ret_wk, value
  .short  EV_SEQ_PARTY_POKE_SELECT
  .short  \decide
  .short  \ret_wk
  .short  \value
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TEMOTI_WAZA_NUM
 * @brief 手持ちポケモンの技個数取得
 * @param ret_wk 選択結果の格納先 
 * @param pos    対象の位置
 * @retval u16   選択結果
 */
//--------------------------------------------------------------
#define _GET_TEMOTI_WAZA_NUM( ret_wk, pos ) \
    _ASM_GET_TEMOTI_WAZA_NUM ret_wk, pos

  .macro  _ASM_GET_TEMOTI_WAZA_NUM ret_wk, pos
  .short  EV_SEQ_GET_TEMOTI_WAZA_NUM
  .short  \ret_wk
  .short  \pos
  .endm


//--------------------------------------------------------------
/**
 * @def _SELECT_WAZA_TEMOTI_POKE
 * @brief 汎用手持ちポケモンの技選択
 * @param decide 選択結果の格納先 (TRUE：FALSE 選択したか)
 * @param ret_wk 選択結果の格納先 (０～３ 選択位置)
 * @param poke_pos ポケモンの位置
 * @param value  予備ワーク (０を入れておいてください
 * @retval u16   選択結果
 */
//--------------------------------------------------------------
#define _SELECT_WAZA_TEMOTI_POKE( decide , ret_wk , poke_pos , value ) \
    _ASM_SELECT_WAZA_TEMOTI_POKE decide, ret_wk, poke_pos, value

  .macro  _ASM_SELECT_WAZA_TEMOTI_POKE decide, ret_wk, poke_pos, value
  .short  EV_SEQ_PARTY_POKE_WAZA_SELECT
  .short  \decide
  .short  \ret_wk
  .short  \poke_pos
  .short  \value
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TEMOTI_WAZA_ID
 * @brief 汎用手持ちポケモンの技番号取得
 * @param ret_wk 選択結果の格納先 (技ID)
 * @param poke_pos ポケモンの位置
 * @param waza_pos 技の位置
 * @retval u16   選択結果
 */
//--------------------------------------------------------------
#define _GET_TEMOTI_WAZA_ID( ret_wk , poke_pos , waza_pos ) \
    _ASM_GET_TEMOTI_WAZA_ID ret_wk, poke_pos, waza_pos

  .macro  _ASM_GET_TEMOTI_WAZA_ID ret_wk, poke_pos, waza_pos
  .short  EV_SEQ_GET_TEMOTI_WAZA_ID
  .short  \ret_wk
  .short  \poke_pos
  .short  \waza_pos
  .endm

//--------------------------------------------------------------
/**
 * @def _SET_TEMOTI_WAZA
 * @brief 手持ちポケモンの技セット(追加・置き換え・忘れ対応
 * @param poke_pos ポケモンの位置
 * @param waza_pos 技の位置(5で技を押し出し式で追加
 * @param waza_id  技の番号(0で技を忘れる
 * @retval u16   選択結果
 */
//--------------------------------------------------------------
#define _SET_TEMOTI_WAZA( poke_pos , waza_pos , waza_id ) \
    _ASM_SET_TEMOTI_WAZA poke_pos, waza_pos, waza_id 

  .macro  _ASM_SET_TEMOTI_WAZA poke_pos, waza_pos, waza_id 
  .short  EV_SEQ_SET_TEMOTI_WAZA
  .short  \poke_pos
  .short  \waza_pos
  .short  \waza_id
  .endm


//--------------------------------------------------------------
/**
 * @def _SKILLTEACH_CHECK_PARTY
 * @brief わざおぼえ：適用チェック
 * @param mode    技覚えモード（script_def.hのSCR_SKILLTEACH_MODE参照）
 * @param ret_wk  script_def.hのSCR_SKILLTEACH_CHECK_RESULT_を参照
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_OK          対象となるポケモンがいる
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_POKEMON_NG  対象となるポケモンがいない
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_NATSUKI_NG  対象となるポケモンはいるがなつきが足りない
 *
 * 最強技（あるいはドラゴン技）を覚えることができる手持ちかどうかを
 * チェックする
 */
//--------------------------------------------------------------
#define _SKILLTEACH_CHECK_PARTY( mode, ret_wk ) \
    _ASM_SKILLTEACH_CHECK_PARTY mode, ret_wk

    .macro  _ASM_SKILLTEACH_CHECK_PARTY mode, ret_wk
    .short  EV_SEQ_SKILLTEACH_CHECK_PARTY
    .short  \mode
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SKILLTEACH_CHECK_POKEMON
 * @brief わざおぼえ：適用ポケモンのチェック
 * @param mode    技覚えモード（script_def.hのSCR_SKILLTEACH_MODE参照）
 * @param pos     選択したポケモンの位置
 * @param ret_wk  script_def.hのSCR_SKILLTEACH_CHECK_RESULT_を参照
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_OK          覚えさせられる
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_POKEMON_NG  覚えないポケモンを選んだ
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_NATSUKI_NG  なつきが足りないポケモンを選んだ
 * @retval  SCR_SKILLTEACH_CHECK_RESULT_ALREADY_NG  すでに持っているポケモンを選んだ
 *
 * 最強技（あるいはドラゴン技）を覚えることができる手持ちかどうかを
 * チェックする
 */
//--------------------------------------------------------------
#define _SKILLTEACH_CHECK_POKEMON( mode, pos, ret_wk ) \
    _ASM_SKILLTEACH_CHECK_POKEMON mode, pos, ret_wk

    .macro  _ASM_SKILLTEACH_CHECK_POKEMON mode, pos, ret_wk
    .short  EV_SEQ_SKILLTEACH_CHECK_POKEMON
    .short  \mode
    .short  \pos
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SKILLTEACH_SELECT_POKEMON
 * @brief ポケモン選択：わざおぼえ用
 * @param mode    技覚えモード（script_def.hのSCR_SKILLTEACH_MODE参照）
 * @param ret_decide  選択したかどうかの戻り値（TRUE/FALSE）
 * @param ret_wk    選択したポケモンの位置
 */
//--------------------------------------------------------------
#define _SKILLTEACH_SELECT_POKEMON( mode, ret_decide, ret_wk ) \
    _ASM_SKILLTEACH_SELECT_POKEMON mode, ret_decide, ret_wk

    .macro  _ASM_SKILLTEACH_SELECT_POKEMON mode, ret_decide, ret_wk
    .short  EV_SEQ_SKILLTEACH_SELECT_POKEMON
    .short  \mode
    .short  \ret_decide
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SKILLTEACH_GET_WAZAID
 * @brief
 * @param mode    技覚えモード（script_def.hのSCR_SKILLTEACH_MODE参照）
 * @param sel_pos 選択したポケモンの位置 
 * @param ret_wk  対象となる技のID
 */
//--------------------------------------------------------------
#define _SKILLTEACH_GET_WAZAID( mode, sel_pos, ret_wk ) \
    _ASM_SKILLTEACH_GET_WAZAID mode, sel_pos, ret_wk

    .macro  _ASM_SKILLTEACH_GET_WAZAID mode, sel_pos, ret_wk
    .short  EV_SEQ_SKILLTEACH_GET_WAZAID
    .short  \mode
    .short  \sel_pos
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_MEZAMERU_POWER_TYPE
 * @brief 「めざめるパワー」のタイプを取得する
 * @param ret_wk    チェック結果を受け取るワーク
 * @param pos     チェックするポケモンの位置（０～５）
 * @retval  PokeType  ポケモンのタイプ指定（prog/include/poke_tool/poketype_def.h参照）
 */
//--------------------------------------------------------------
#define _GET_MEZAMERU_POWER_TYPE( ret_wk, pos ) \
    _ASM_GET_MEZAMERU_POWER_TYPE ret_wk, pos

  .macro  _ASM_GET_MEZAMERU_POWER_TYPE ret_wk, pos
  .short  EV_SEQ_GET_MEZAMERU_POWER_TYPE
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def   _GET_TEMOTI_POKE_TYPE
 * @brief 手持ちポケモンのタイプを取得する
 * @param ret_wk1 チェック結果を受け取るワークその１
 * @param ret_wk2 チェック結果を受け取るワークその２
 * @param pos     チェックするポケモンの位置（０～５）
 * @retval  PokeType  ポケモンのタイプ指定（prog/include/poke_tool/poketype_def.h参照）
 */
//--------------------------------------------------------------
#define _GET_TEMOTI_POKE_TYPE( ret_wk1, ret_wk2, pos ) \
    _ASM_GET_TEMOTI_POKE_TYPE ret_wk1, ret_wk2, pos

    .macro  _ASM_GET_TEMOTI_POKE_TYPE ret_wk1, ret_wk2, pos
    .short  EV_SEQ_GET_TEMOTI_POKE_TYPE
    .short  \ret_wk1
    .short  \ret_wk2
    .short  \pos
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_TEMOTI_PARAM_EXP
 * @brief 手持ちポケモンの努力値合計を受け取る
 * @param ret_wk    チェック結果を受け取るワーク
 * @param pos     チェックするポケモンの位置（０～５）
 */
//--------------------------------------------------------------
#define _GET_TEMOTI_PARAM_EXP( ret_wk , pos ) \
    _ASM_GET_TEMOTI_PARAM_EXP ret_wk , pos

  .macro  _ASM_GET_TEMOTI_PARAM_EXP ret_wk , pos
  .short  EV_SEQ_GET_PARAM_EXP
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_POKE_OWNER
 * @brief 手持ちポケモンの親が自分かをチェック
 * @param ret_wk  チェック結果を受け取るワーク
 * @param pos     チェックするポケモンの位置（０～５）
 * @retval  TRUE    チェックしたポケモンの親は自分
 * @retval  FALSE   チェックしたポケモンの親は他人
 */
//--------------------------------------------------------------
#define _CHECK_POKE_OWNER( ret_wk, pos ) \
    _ASM_CHECK_POKE_OWNER ret_wk, pos

  .macro  _ASM_CHECK_POKE_OWNER ret_wk, pos
  .short  EV_SEQ_CHECK_POKE_OWNER
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _CHANGE_FORM_NO
 * @brief 手持ちポケモンのフォルムチェンジ
 * @param pos     チェックするポケモンの位置（０～５）
 * @param formno    フォルムナンバー
 */
//--------------------------------------------------------------
#define _CHANGE_FORM_NO( pos, formno ) _ASM_CHANGE_FORM_NO pos, formno

  .macro  _ASM_CHANGE_FORM_NO pos, formno
  .short  EV_SEQ_CHG_FORM_NO
  .short  \pos
  .short  \formno
  .endm
  

//--------------------------------------------------------------
/**
 * @def _CHANGE_ROTOM_FORM_NO
 * @brief 手持ちポケモンのフォルムチェンジ
 * @param poke_pos     チェックするポケモンの位置（０～５）
 * @param waza_pos      技の位置（0～3）
 * @param formno    フォルムナンバー
 */
//--------------------------------------------------------------
#define _CHANGE_ROTOM_FORM_NO( poke_pos, waza_pos, formno ) _ASM_CHANGE_ROTOM_FORM_NO poke_pos, waza_pos, formno

  .macro  _ASM_CHANGE_ROTOM_FORM_NO poke_pos, waza_pos, formno
  .short  EV_SEQ_CHG_ROTOM_FORM_NO
  .short  \poke_pos
  .short  \waza_pos
  .short  \formno
  .endm
  
//--------------------------------------------------------------
/**
 * @def _GET_PARTY_POS
 * @brief 指定モンスターナンバーのポケが手持ちのどこにいるかを調べる
 * @param monsno  探すモンスターナンバー
 * @param ret_wk  TRUEなら発見 FALSEなら未発見
 * @param pos     発見したポケモンの位置（０～５）
 */
//--------------------------------------------------------------
#define _GET_PARTY_POS( monsno, ret_wk, pos ) _ASM_GET_PARTY_POS monsno, ret_wk, pos

  .macro  _ASM_GET_PARTY_POS monsno, ret_wk, pos
  .short  EV_SEQ_GET_PARTY_POS_BY_MONSNO
  .short  \monsno
  .short  \ret_wk
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_TEMOTI_GETPLACE
 * @brief 指定手持ちポケモンの捕獲場所IDをチェック
 *
 * @param ret_wk      TRUE  チェックタイプと一致　FALSE　不一致
 * @param poke_pos    手持ちポジション
 * @param check_type  捕獲場所チェックタイプ　POKE_GET_PLACE_CHECK_～
 *        prog/src/field/script_def.h
 */
//--------------------------------------------------------------
#define _CHECK_TEMOTI_GETPLACE( ret_wk, poke_pos, check_type ) _ASM_CHECK_TEMOTI_GETPLACE ret_wk, poke_pos, check_type
  

  .macro  _ASM_CHECK_TEMOTI_GETPLACE ret_wk, poke_pos, check_type
  .short  EV_SEQ_CHECK_TEMOTI_GETPLACE
  .short  \ret_wk
  .short  \poke_pos
  .short  \check_type
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_TEMOTI_GETDATE
 * @brief 指定手持ちポケモンの捕獲日付を取得
 *
 * @param ret_year   捕獲年の格納先
 * @param ret_month  捕獲月の格納先
 * @param ret_day    捕獲日の格納先
 * @param poke_pos  手持ちポジション
 */
//--------------------------------------------------------------
#define _GET_TEMOTI_GETDATE( ret_year, ret_month, ret_day, poke_pos ) _ASM_GET_TEMOTI_GETDATE ret_year, ret_month, ret_day, poke_pos

  .macro  _ASM_GET_TEMOTI_GETDATE ret_year, ret_month, ret_day, poke_pos
  .short  EV_SEQ_GET_TEMOTI_GETDATE
  .short  \ret_year
  .short  \ret_month
  .short  \ret_day
  .short  \poke_pos
  .endm

//======================================================================
// お金
//======================================================================
//--------------------------------------------------------------
/**
 * @def _ADD_GOLD
 * @brief お金を増やす
 * @param value 増やす金額(円)
 */
//--------------------------------------------------------------
#define _ADD_GOLD( value ) \
    _ASM_ADD_GOLD value

  .macro _ASM_ADD_GOLD value
  .short EV_SEQ_ADD_GOLD
  .short \value
  .endm

//--------------------------------------------------------------
/**
 * @def _SUBTRACT_GOLD
 * @brief お金を減らす
 * @param value 減らす金額(円)
 */
//--------------------------------------------------------------
#define _SUBTRACT_GOLD( value ) \
    _ASM_SUBTRACT_GOLD value

  .macro _ASM_SUBTRACT_GOLD value
  .short EV_SEQ_SUBTRACT_GOLD
  .short \value
  .endm

//--------------------------------------------------------------
/**
 * @def _CHECK_GOLD
 * @brief 指定金額を持っているかどうかチェックする
 * @param ret_wk 判定結果を受け取るワーク
 * @param value  判定する金額(円)
 * @retval TRUE  指定金額を持っている場合
 * @retval FALSE 指定金額を持っていない場合
 */
//--------------------------------------------------------------
#define _CHECK_GOLD( ret_wk, value ) \
    _ASM_CHECK_GOLD ret_wk, value

  .macro _ASM_CHECK_GOLD ret_wk, value
  .short EV_SEQ_CHECK_GOLD
  .short \ret_wk
  .short \value
  .endm


//======================================================================
// ボックス関連
//======================================================================

//--------------------------------------------------------------
/**
 * @def _GET_BOX_POKE_COUNT
 * @brief ボックス内のポケモン数を取得する
 * @param ret_wk ポケモン数を受け取るワーク
 * @param mode カウントモード( POKECOUNT_MODE_XXXX )
 */
//--------------------------------------------------------------
#define _GET_BOX_POKE_COUNT( ret_wk, mode ) \
    _ASM_GET_BOX_POKE_COUNT ret_wk, mode

  .macro _ASM_GET_BOX_POKE_COUNT ret_wk, mode
  .short EV_SEQ_GET_BOX_POKE_COUNT
  .short \ret_wk
  .short \mode
  .endm
    
  
//======================================================================
// マップ遷移関連
//======================================================================
//--------------------------------------------------------------
/**
 * マップ遷移コマンド (BGM変更なし)
 * @param zone_id   移動先ゾーンID指定
 * @param gx        移動先Ｘ位置指定（グリッド単位）
 * @param gz        移動先Ｚ位置指定（グリッド単位）
 * @param dir       移動後の向き
 */
//--------------------------------------------------------------
#define _MAP_CHANGE_BGM_KEEP( zone_id, gx, gz, dir )  \
    _ASM_MAP_CHANGE_BGM_KEEP zone_id, gx, gz, dir

  .macro  _ASM_MAP_CHANGE_BGM_KEEP zone_id, gx, gz, dir
  .short  EV_SEQ_MAP_CHANGE_BGM_KEEP
  .short  \zone_id
  .short  \gx
  .short  \gz
  .short  \dir
  .endm

//--------------------------------------------------------------
/**
 * マップ遷移コマンド（流砂）
 * @param zone_id   移動先ゾーンID指定
 * @param gx        移動先Ｘ位置指定（グリッド単位）
 * @param gz        移動先Ｙ位置指定（グリッド単位）
 */
//--------------------------------------------------------------
#define _MAP_CHANGE_SAND_STREAM( zone_id, gx, gz )  \
    _ASM_MAP_CHANGE_SAND_STREAM zone_id, gx, gz

  .macro  _ASM_MAP_CHANGE_SAND_STREAM zone_id, gx, gz
  .short  EV_SEQ_MAP_CHANGE_SAND_STREAM
  .short  \zone_id
  .short  \gx
  .short  \gz
  .endm

//--------------------------------------------------------------
/**
 * マップ遷移コマンド
 * @param zone_id   移動先ゾーンID指定
 * @param gx        移動先Ｘ位置指定（グリッド単位）
 * @param gz        移動先Ｚ位置指定（グリッド単位）
 * @param dir       移動後の向き
 */
//--------------------------------------------------------------
#define _MAP_CHANGE( zone_id, gx, gz, dir )  \
    _ASM_MAP_CHANGE zone_id, gx, gz, dir

  .macro  _ASM_MAP_CHANGE zone_id, gx, gz, dir
  .short  EV_SEQ_MAP_CHANGE
  .short  \zone_id
  .short  \gx
  .short  \gz
  .short  \dir
  .endm

//--------------------------------------------------------------
/**
 * マップ遷移コマンド ( ワープ )
 * @param zone_id   移動先ゾーンID指定
 * @param gx        移動先Ｘ位置指定（グリッド単位）
 * @param gz        移動先Ｚ位置指定（グリッド単位）
 * @param dir       移動後の向き
 */
//--------------------------------------------------------------
#define _MAP_CHANGE_WARP( zone_id, gx, gz, dir )  \
    _ASM_MAP_CHANGE_WARP zone_id, gx, gz, dir

  .macro  _ASM_MAP_CHANGE_WARP zone_id, gx, gz, dir
  .short  EV_SEQ_MAP_CHANGE_WARP
  .short  \zone_id
  .short  \gx
  .short  \gz
  .short  \dir
  .endm

//--------------------------------------------------------------
/**
 * レールマップへ遷移するコマンド
 * @param zone_id   移動先ゾーンID指定
 * @param rail_idx  移動先レールインデックス
 * @param front_idx 移動先フiロントインデックス
 * @param side_idx  移動先サイドインデックス
 * @param dir       移動後の向き
 */
//--------------------------------------------------------------
#define _RAIL_MAP_CHANGE( zone_id, rail_idx, front_idx, side_idx, dir )  \
    _ASM_RAIL_MAP_CHANGE zone_id, rail_idx, front_idx, side_idx, dir

  .macro  _ASM_RAIL_MAP_CHANGE zone_id, rail_idx, front_idx, side_idx, dir
  .short  EV_SEQ_RAIL_MAP_CHANGE
  .short  \zone_id
  .short  \rail_idx
  .short  \front_idx
  .short  \side_idx
  .short  \dir
  .endm

//--------------------------------------------------------------
/**
 * レールマップへ遷移するコマンド ノーフェード版
 * @param zone_id   移動先ゾーンID指定
 * @param rail_idx  移動先レールインデックス
 * @param front_idx 移動先フiロントインデックス
 * @param side_idx  移動先サイドインデックス
 * @param dir       移動後の向き
 */
//--------------------------------------------------------------
#define _RAIL_MAP_CHANGE_NO_FADE( zone_id, rail_idx, front_idx, side_idx, dir )  \
    _ASM_RAIL_MAP_CHANGE_NO_FADE zone_id, rail_idx, front_idx, side_idx, dir

  .macro  _ASM_RAIL_MAP_CHANGE_NO_FADE zone_id, rail_idx, front_idx, side_idx, dir
  .short  EV_SEQ_RAIL_MAP_CHANGE_NO_FADE
  .short  \zone_id
  .short  \rail_idx
  .short  \front_idx
  .short  \side_idx
  .short  \dir
  .endm
  
  
//--------------------------------------------------------------
/**
 * マップ遷移コマンド(ユニオンルーム)
 */
//--------------------------------------------------------------
#define _MAP_CHANGE_TO_UNION() \
    _ASM_MAP_CHANGE_TO_UNION 

  .macro  _ASM_MAP_CHANGE_TO_UNION 
  .short  EV_SEQ_MAP_CHANGE_TO_UNION
  .endm

//--------------------------------------------------------------
/**
 * フェードなしマップ遷移コマンド
 * @param zone_id   移動先ゾーンID指定
 * @param gx        移動先Ｘ位置指定（グリッド単位）
 * @param gy        移動先Ｙ位置指定（グリッド単位）
 * @param gz        移動先Ｚ位置指定（グリッド単位）
 * @param dir       移動後の向き
 */
//--------------------------------------------------------------
#define _MAP_CHANGE_NO_FADE( zone_id, gx, gy, gz, dir )  \
    _ASM_MAP_CHANGE_NO_FADE zone_id, gx, gy, gz, dir

  .macro  _ASM_MAP_CHANGE_NO_FADE zone_id, gx, gy, gz, dir
  .short  EV_SEQ_MAP_CHANGE_NO_FADE
  .short  \zone_id
  .short  \gx
  .short  \gy
  .short  \gz
  .short  \dir
  .endm  

//--------------------------------------------------------------
/**
 * @def _PLACE_NAME_DISPLAY
 * @brief 地名を表示する
 */
//--------------------------------------------------------------
#define _PLACE_NAME_DISPLAY() \
    _ASM_PLACE_NAME_DISPLAY

  .macro  _ASM_PLACE_NAME_DISPLAY
  .short  EV_SEQ_PLACE_NAME_DISPLAY
  .endm  

//======================================================================
//
//
//  特殊状況ギミック関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * ジムコマンド　電気ジム初期化
 */
//--------------------------------------------------------------
#define _GYM_ELEC_INIT( evt1, evt2 ) \
    _ASM_GYM_ELEC_INIT evt1, evt2

  .macro  _ASM_GYM_ELEC_INIT evt1, evt2
  .short  EV_SEQ_GYM_ELEC_INIT
  .short  \evt1
  .short  \evt2
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　電気ジムスイッチ
 * @param sw_idx   スイッチインデックス0～3
 */
//--------------------------------------------------------------
#define _GYM_ELEC_PUSH_SW( sw_idx ) \
    _ASM_GYM_ELEC_PUSH_SW sw_idx 

  .macro  _ASM_GYM_ELEC_PUSH_SW sw_idx
  .short  EV_SEQ_GYM_ELEC_PUSH_SW
  .short  \sw_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　電気ジムカプセルトレーナー戦勝利後処理
 * @param cap_idx   一人目に勝利のとき：1　二人目に勝利のとき：2
 */
//--------------------------------------------------------------
#define _GYM_ELEC_SET_TR_ENC_FLG( cap_idx ) \
    _ASM_GYM_ELEC_SET_TR_ENC_FLG cap_idx 

  .macro  _ASM_GYM_ELEC_SET_TR_ENC_FLG cap_idx
  .short  EV_SEQ_GYM_ELEC_SET_TR_ENC_FLG
  .short  \cap_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　電気ジム環境ＳＥ初期化（セーブ復帰用）
 */
//--------------------------------------------------------------
#define _GYM_ELEC_INIT_SE() \
    _ASM_GYM_ELEC_INIT_SE

  .macro  _ASM_GYM_ELEC_INIT_SE
  .short  EV_SEQ_GYM_ELEC_INIT_SE
  .endm
 
//--------------------------------------------------------------
/**
 * ジムコマンド　ノーマルジム初期化
 * @param opened    ギミック発動後か？　TRUE：発動後
 */
//--------------------------------------------------------------
#define _GYM_NORMAL_INIT( opened) \
    _ASM_GYM_NORMAL_INIT opened

  .macro  _ASM_GYM_NORMAL_INIT opened
  .short  EV_SEQ_GYM_NORMAL_INIT
  .short  \opened
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ノーマルジムロック解除
 */
//--------------------------------------------------------------
#define _GYM_NORMAL_UNROCK() \
    _ASM_GYM_NORMAL_UNROCK 

  .macro  _ASM_GYM_NORMAL_UNROCK
  .short  EV_SEQ_GYM_NORMAL_UNROCK
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ノーマルジムロック解除チェック
 */
//--------------------------------------------------------------
#define _GYM_NORMAL_CHK_UNROCK(ret_wk) \
    _ASM_GYM_NORMAL_CHK_UNROCK ret_wk

  .macro  _ASM_GYM_NORMAL_CHK_UNROCK ret_wk
  .short  EV_SEQ_GYM_NORMAL_CHK_UNROCK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ノーマルジム本棚移動
 * @param wall_idx   本棚インデックス　現在は0固定
 */
//--------------------------------------------------------------
#define _GYM_NORMAL_MOVE_WALL(wall_idx) \
    _ASM_GYM_NORMAL_MOVE_WALL wall_idx

  .macro  _ASM_GYM_NORMAL_MOVE_WALL wall_idx
  .short  EV_SEQ_GYM_NORMAL_MOVE_WALL
  .short  \wall_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　アンチジム初期化
 * @param curtain   SCR_GYM01_CURTAIN_～　script_def.h参照
 */
//--------------------------------------------------------------
#define _GYM_ANTI_INIT(curtain) \
    _ASM_GYM_ANTI_INIT curtain

  .macro  _ASM_GYM_ANTI_INIT curtain
  .short  EV_SEQ_GYM_ANTI_INIT
  .short  \curtain
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　アンチジムスイッチオン
 * @param sw_idx   スイッチインデックス0～8
 * @note  0,1,2： それぞれ火水草の当たりボタン、3,6:火のハズレボタン　4,7:水のハズレボタン 5,8:草のハズレボタン
 */
//--------------------------------------------------------------
#define _GYM_ANTI_SW_ON(sw_idx) \
    _ASM_GYM_ANTI_SW_ON sw_idx

  .macro  _ASM_GYM_ANTI_SW_ON sw_idx
  .short  EV_SEQ_GYM_ANTI_SW_ON
  .short  \sw_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　アンチジムスイッチオフ
 * @param sw_idx   スイッチインデックス0～8
 * @note  0,1,2： それぞれ火水草の当たりボタン、3,6:火のハズレボタン　4,7:水のハズレボタン 5,8:草のハズレボタン
 */
//--------------------------------------------------------------
#define _GYM_ANTI_SW_OFF(sw_idx) \
    _ASM_GYM_ANTI_SW_OFF sw_idx

  .macro  _ASM_GYM_ANTI_SW_OFF sw_idx
  .short  EV_SEQ_GYM_ANTI_SW_OFF
  .short  \sw_idx
  .endm
  
//--------------------------------------------------------------
/**
 * ジムコマンド　アンチジムスイッチオン
 * @param door_idx   ドアインデックス0～2
 */
//--------------------------------------------------------------
#define _GYM_ANTI_OPEN_DOOR(door_idx) \
    _ASM_GYM_ANTI_OPEN_DOOR door_idx

  .macro  _ASM_GYM_ANTI_OPEN_DOOR door_idx
  .short  EV_SEQ_GYM_ANTI_OPEN_DOOR
  .short  \door_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　虫ジム初期化
 */
//--------------------------------------------------------------
#define _GYM_INSECT_INIT(sw0, sw1 ,sw2, sw3, sw4, sw5, sw6, sw7) \
    _ASM_GYM_INSECT_INIT sw0, sw1 ,sw2, sw3, sw4, sw5, sw6, sw7

  .macro  _ASM_GYM_INSECT_INIT sw0, sw1 ,sw2, sw3, sw4, sw5, sw6, sw7
  .short  EV_SEQ_GYM_INSECT_INIT
  .short  \sw0
  .short  \sw1
  .short  \sw2
  .short  \sw3
  .short  \sw4
  .short  \sw5
  .short  \sw6
  .short  \sw7
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　虫ジムスイッチオン
 * @param sw_idx   スイッチインデックス0～7
 */
//--------------------------------------------------------------
#define _GYM_INSECT_SW_ON(sw_idx) \
    _ASM_GYM_INSECT_SW_ON sw_idx

  .macro  _ASM_GYM_INSECT_SW_ON sw_idx
  .short  EV_SEQ_GYM_INSECT_SW_ON
  .short  \sw_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　虫ジムポール解除
 * @param pl_idx   ポールインデックス0～9
 */
//--------------------------------------------------------------
#define _GYM_INSECT_PL_ON(pl_idx) \
    _ASM_GYM_INSECT_PL_ON pl_idx

  .macro  _ASM_GYM_INSECT_PL_ON pl_idx
  .short  EV_SEQ_GYM_INSECT_PL_ON
  .short  \pl_idx
  .endm
  
//--------------------------------------------------------------
/**
 * ジムコマンド　虫ジムトレーナートラップ
 * @param trp_evt_idx   インデックス0～1
 */
//--------------------------------------------------------------
#define _GYM_INSECT_TR_TRAP_ON(trp_evt_idx) \
    _ASM_GYM_INSECT_TR_TRAP_ON trp_evt_idx

  .macro  _ASM_GYM_INSECT_TR_TRAP_ON trp_evt_idx
  .short  EV_SEQ_GYM_INSECT_TR_TRAP_ON
  .short  \trp_evt_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　地面ジムリフト移動
 * @param lift_idx   インデックス0～5
 */
//--------------------------------------------------------------
#define _GYM_GROUND_MV_LIFT(lift_idx) \
    _ASM_GYM_GROUND_MV_LIFT lift_idx

  .macro  _ASM_GYM_GROUND_MV_LIFT lift_idx
  .short  EV_SEQ_GYM_GROUND_MV_LIFT
  .short  \lift_idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　地面ジム 入り口位置にリフトと自機をセット
 */
//--------------------------------------------------------------
#define _GYM_GROUND_SET_ENTER_POS() \
    _ASM_GYM_GROUND_SET_ENTER_POS

  .macro  _ASM_GYM_GROUND_SET_ENTER_POS
  .short  EV_SEQ_GYM_GROUND_SET_ENTER_POS
  .endm  

//--------------------------------------------------------------
/**
 * ジムコマンド　地面ジム出口リフト移動
 * @param exit   ジムから出るとき1　ジムに入るとき0
 */
//--------------------------------------------------------------
#define _GYM_GROUND_EXIT_LIFT(exit) \
    _ASM_GYM_GROUND_EXIT_LIFT exit

  .macro  _ASM_GYM_GROUND_EXIT_LIFT exit
  .short  EV_SEQ_GYM_GROUND_EXIT_LIFT
  .short  \exit
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　地面ジムエントランス 入り口位置にリフトと自機をセット
 */
//--------------------------------------------------------------
#define _GYM_GROUND_ENT_SET_ENTER_POS() \
    _ASM_GYM_GROUND_ENT_SET_ENTER_POS

  .macro  _ASM_GYM_GROUND_ENT_SET_ENTER_POS
  .short  EV_SEQ_GYM_GROUND_ENT_SET_ENTER_POS
  .endm  
  
//--------------------------------------------------------------
/**
 * ジムコマンド　地面ジムエントランス出口リフト移動
 * @param exit   ジムに行くとき1　ジムから戻るとき0
 */
//--------------------------------------------------------------
#define _GYM_GROUND_ENT_EXIT_LIFT(exit) \
    _ASM_GYM_GROUND_ENT_EXIT_LIFT exit

  .macro  _ASM_GYM_GROUND_ENT_EXIT_LIFT exit
  .short  EV_SEQ_GYM_GROUND_ENT_EXIT_LIFT
  .short  \exit
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　氷ジムスイッチアニメ
 * @param idx   ギミックインデックス0～2
 */
//--------------------------------------------------------------
#define _GYM_ICE_SW_ANM(idx) _ASM_GYM_ICE_SW_ANM idx

  .macro  _ASM_GYM_ICE_SW_ANM idx
  .short  EV_SEQ_GYM_ICE_SW_ANM
  .short  \idx
  .endm
  
//--------------------------------------------------------------
/**
 * ジムコマンド　氷ジム壁アニメ
 * @param idx   ギミックインデックス0～2
 */
//--------------------------------------------------------------
#define _GYM_ICE_WALL_ANM(idx) _ASM_GYM_ICE_WALL_ANM idx

  .macro  _ASM_GYM_ICE_WALL_ANM idx
  .short  EV_SEQ_GYM_ICE_WALL_ANM
  .short  \idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　氷ジムレールにチェンジ
 * @param idx   ギミックインデックス0～2
 */
//--------------------------------------------------------------
#define _GYM_ICE_CHG_RAIL(idx) _ASM_GYM_ICE_CHG_RAIL idx

  .macro  _ASM_GYM_ICE_CHG_RAIL idx
  .short  EV_SEQ_GYM_ICE_CHG_RAIL
  .short  \idx
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ドラゴンジム初期化
 */
//--------------------------------------------------------------
#define _GYM_DRAGON_INIT() \
    _ASM_GYM_DRAGON_INIT

  .macro  _ASM_GYM_DRAGON_INIT
  .short  EV_SEQ_GYM_DRAGON_INIT
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ドラゴンジムギミック発動
 */
//--------------------------------------------------------------
#define _GYM_DRAGON_CALL_GMK(head, arm) _ASM_GYM_DRAGON_CALL_GMK head, arm

  .macro  _ASM_GYM_DRAGON_CALL_GMK head, arm
  .short  EV_SEQ_GYM_DRAGON_CALL_GMK
  .short  \head
  .short  \arm
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ドラゴンジム自機飛び降り
 */
//--------------------------------------------------------------
#define _GYM_DRAGON_JUMP_DOWN(dir) _ASM_GYM_DRAGON_JUMP_DOWN dir

  .macro  _ASM_GYM_DRAGON_JUMP_DOWN dir
  .short  EV_SEQ_GYM_DRAGON_JUMP_DOWN
  .short  \dir
  .endm

//--------------------------------------------------------------
/**
 * ジムコマンド　ドラゴンジム腕移動
 */
//--------------------------------------------------------------
#define _GYM_DRAGON_MOVE_ARM(head, arm) _ASM_GYM_DRAGON_MOVE_ARM head, arm

  .macro  _ASM_GYM_DRAGON_MOVE_ARM head, arm
  .short  EV_SEQ_GYM_DRAGON_MOVE_ARM
  .short  \head
  .short  \arm
  .endm  
  
//--------------------------------------------------------------
/**
 * ジムコマンド　ドラゴンジム首移動
 */
//--------------------------------------------------------------
#define _GYM_DRAGON_MOVE_HEAD(head) _ASM_GYM_DRAGON_MOVE_HEAD head

  .macro  _ASM_GYM_DRAGON_MOVE_HEAD head
  .short  EV_SEQ_GYM_DRAGON_MOVE_HEAD
  .short  \head
  .endm

//======================================================================
//
//  モデルアニメ関連
//
//======================================================================
//--------------------------------------------------------------
/**
 * @def _DENDOU_ANIME
 * @brief 殿堂入りアニメ
 */
//--------------------------------------------------------------
#define _DENDOU_ANIME()  \
    _ASM_DENDOU_ANIME

  .macro  _ASM_DENDOU_ANIME
  .short  EV_SEQ_DENDOU_ANIME
  .endm

//--------------------------------------------------------------
/**
 * @def _POKECEN_RECOVER_ANIME
 * @brief ポケセン回復アニメ
 * @param poke_count    手持ちのポケモンの数
 */
//--------------------------------------------------------------
#define _POKECEN_RECOVER_ANIME( poke_count )  \
  _ASM_POKECEN_RECOVER_ANIME poke_count

  .macro  _ASM_POKECEN_RECOVER_ANIME poke_count
  .short  EV_SEQ_POKECEN_RECOVER_ANIME
  .short  \poke_count
  .endm

//--------------------------------------------------------------
/**
 * @def _POKECEN_PC_ON
 * @brief ポケセンPC起動アニメ
 */
//--------------------------------------------------------------
#define _POKECEN_PC_ON()  \
  _ASM_POKECEN_PC_ON

  .macro  _ASM_POKECEN_PC_ON
  .short  EV_SEQ_POKECEN_PC_ON
  .endm

//--------------------------------------------------------------
/**
 * @def _POKECEN_PC_RUN
 * @brief ポケセンPC起動中アニメ
 */
//--------------------------------------------------------------
#define _POKECEN_PC_RUN()  \
  _ASM_POKECEN_PC_RUN

  .macro  _ASM_POKECEN_PC_RUN
  .short  EV_SEQ_POKECEN_PC_RUN
  .endm

//--------------------------------------------------------------
/**
 * @def _POKECEN_PC_OFF
 * @brief ポケセンPC停止アニメ
 *
 * @param off_mode PC停止方法 ( SCR_PASOKON_OFF_xxxx )
 */
//--------------------------------------------------------------
#define _POKECEN_PC_OFF( off_mode )  \
  _ASM_POKECEN_PC_OFF off_mode

  .macro  _ASM_POKECEN_PC_OFF off_mode
  .short  EV_SEQ_POKECEN_PC_OFF
  .short  \off_mode
  .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_ANIME_CREATE
 * @brief 配置モデルアニメコントローラーの生成
 * @param ret_wk  anm_idをうけとるためのワーク
 * @param bm_id   配置モデルの種類指定ID（script_def.hを参照）
 * @param gx      アニメ生成する配置モデルのX位置（グリッド単位）
 * @param gz      アニメ生成する配置モデルのZ位置（グリッド単位）
 *
 * ret_wkで受け取ったアニメ指定IDを使って配置モデルアニメ各コマンド
 * （_BMODEL_ANIME_DELETE/_BMODEL_ANIME_SET/_BMODEL_ANIME_WAIT）を
 * 呼び出す。
 */
//--------------------------------------------------------------
#define _BMODEL_ANIME_CREATE( ret_wk, bm_id, gx, gz ) \
    _ASM_BMODEL_ANIME_CREATE ret_wk, bm_id, gx, gz

    .macro _ASM_BMODEL_ANIME_CREATE ret_wk, bm_id, gx, gz
    .short  EV_SEQ_BMODEL_ANIME_CREATE
    .short  \ret_wk
    .short  \bm_id
    .short  \gx
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_ANIME_DELETE
 * @brief 配置モデルアニメコントローラーの削除
 * @param anm_id  配置モデルを指定するためのID
 */
//--------------------------------------------------------------
#define _BMODEL_ANIME_DELETE( anm_id ) \
     _ASM_BMODEL_ANIME_DELETE anm_id

     .macro _ASM_BMODEL_ANIME_DELETE anm_id
     .short EV_SEQ_BMODEL_ANIME_DELETE
     .short \anm_id
     .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_ANIME_SET
 * @brief 配置モデルアニメのセット
 * @param anm_id    配置モデルを指定するためのID
 * @param anm_type  アニメーションの種類指定
 */
//--------------------------------------------------------------
#define _BMODEL_ANIME_SET( anm_id, anm_type ) \
    _ASM_BMODEL_ANIME_SET anm_id, anm_type

    .macro _ASM_BMODEL_ANIME_SET anm_id, anm_type
    .short  EV_SEQ_BMODEL_ANIME_SET
    .short  \anm_id
    .short  \anm_type
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_ANIME_LOOP_SET
 * @brief 配置モデルループアニメのセット
 * @param anm_id    配置モデルを指定するためのID
 * @param anm_type  アニメーションの種類指定
 */
//--------------------------------------------------------------
#define _BMODEL_ANIME_LOOP_SET( anm_id, anm_type ) \
    _ASM_BMODEL_ANIME_LOOP_SET anm_id, anm_type

    .macro _ASM_BMODEL_ANIME_LOOP_SET anm_id, anm_type
    .short  EV_SEQ_BMODEL_ANIME_LOOP_SET
    .short  \anm_id
    .short  \anm_type
    .endm    

//--------------------------------------------------------------
/**
 * @def _BMODEL_ANIME_STOP
 * @brief 配置モデルアニメの停止
 * @param anm_id  配置モデルを指定するためのID
 */
//--------------------------------------------------------------
#define _BMODEL_ANIME_STOP( anm_id ) \
    _ASM_BMODEL_ANIME_STOP anm_id

    .macro _ASM_BMODEL_ANIME_STOP anm_id
    .short  EV_SEQ_BMODEL_ANIME_STOP
    .short  \anm_id
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_ANIME_WAIT
 * @brief 配置モデルアニメの終了待ち
 * @param anm_id  配置モデルを指定するためのID
 */
//--------------------------------------------------------------
#define _BMODEL_ANIME_WAIT( anm_id ) \
    _ASM_BMODEL_ANIME_WAIT anm_id

    .macro _ASM_BMODEL_ANIME_WAIT anm_id
    .short  EV_SEQ_BMODEL_ANIME_WAIT
    .short  \anm_id
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_DIRECT_ANIME_SET_FINISHED
 * @brief 配置モデルアニメ状態を終了状態で固定する
 * @param bm_id   配置モデルの種類指定ID（script_def.hを参照）
 * @param gx      アニメ生成する配置モデルのX位置（グリッド単位）
 * @param gz      アニメ生成する配置モデルのZ位置（グリッド単位）
 *
 * @note
 * ・グローバルなアニメ状態を操作するので、マップ中の同じ種類のすべての配置モデルに適用される
 * ・状態は保持されないため、マップ再構成時に毎回呼び出す必要がある
 *
 * バッジチェックゲートを開けっ放しにするためのコマンドで、他では使い道がないかも。
 */
//--------------------------------------------------------------
#define _BMODEL_DIRECT_ANIME_SET_FINISHED( bm_id, gx, gz )  \
    _ASM_BMODEL_DIRECT_ANIME_SET_FINISHED bm_id, gx, gz

    .macro  _ASM_BMODEL_DIRECT_ANIME_SET_FINISHED bm_id, gx, gz
    .short  EV_SEQ_BMODEL_DIRECT_ANIME_SET_FINISHED
    .short  \bm_id
    .short  \gx
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_DIRECT_ANIME_SET_LOOP
 * @brief 配置モデルアニメ状態をループ状態で固定する
 * @param bm_id   配置モデルの種類指定ID（script_def.hを参照）
 * @param gx      アニメ生成する配置モデルのX位置（グリッド単位）
 * @param gz      アニメ生成する配置モデルのZ位置（グリッド単位）
 *
 * @note
 * ・グローバルなアニメ状態を操作するので、マップ中の同じ種類のすべての配置モデルに適用される
 * ・状態は保持されないため、マップ再構成時に毎回呼び出す必要がある
 *
 */
//--------------------------------------------------------------
#define _BMODEL_DIRECT_ANIME_SET_LOOP( bm_id, gx, gz )  \
    _ASM_BMODEL_DIRECT_ANIME_SET_LOOP bm_id, gx, gz

    .macro  _ASM_BMODEL_DIRECT_ANIME_SET_LOOP bm_id, gx, gz
    .short  EV_SEQ_BMODEL_DIRECT_ANIME_SET_LOOP
    .short  \bm_id
    .short  \gx
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_CHANGE_VIEW_FLAG
 * @brief 配置モデルの表示ON/OFF
 * @param bm_id   配置モデルの種類指定ID（script_def.hを参照）
 * @param gx      アニメ生成する配置モデルのX位置（グリッド単位）
 * @param gz      アニメ生成する配置モデルのZ位置（グリッド単位）
 * @param flag    ON/OFF指定
 *
 */
//--------------------------------------------------------------
#define _BMODEL_CHANGE_VIEW_FLAG( bm_id, gx, gz, flag ) \
    _ASM_BMODEL_CHANGE_VIEW_FLAG bm_id, gx, gz, flag

    .macro  _ASM_BMODEL_CHANGE_VIEW_FLAG bm_id, gx, gz, flag
    .short  EV_SEQ_BMODEL_DIRECT_CHANGE_VIEW_FLAG
    .short  \bm_id
    .short  \gx
    .short  \gz
    .short  \flag
    .endm

//--------------------------------------------------------------
/**
 * @def _BMODEL_DIRECT_CHANGE_MODEL_ID
 * @brief 配置モデルのモデルIDを変更
 * @param search_id   配置モデルの種類指定ID（script_def.hを参照）
 * @param gx          アニメ生成する配置モデルのX位置（グリッド単位）
 * @param gz          アニメ生成する配置モデルのZ位置（グリッド単位）
 * @param model_id    モデル指定ID
 *
 * モデル指定IDを直接スクリプトから記述する予定はない。
 * 現状は_GET_TRAIN_MODEL_ID()でのみ使用予定
 */
//--------------------------------------------------------------
#define _BMODEL_DIRECT_CHANGE_MODEL_ID( search_id, gx, gz, model_id ) \
    _ANM_BMODEL_DIRECT_CHANGE_MODEL_ID search_id, gx, gz, model_id

    .macro  _ANM_BMODEL_DIRECT_CHANGE_MODEL_ID search_id, gx, gz, model_id
    .short  EV_SEQ_BMODEL_DIRECT_CHANGE_MODEL_ID
    .short  \search_id
    .short  \gx
    .short  \gz
    .short  \model_id
    .endm

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @def _GET_TRAIN_TOWN_COND
 * @brief トレインタウンの状態を抽選する
 * @param ret_wk  戻り値を受け取るためのワーク
 * @retval  00  電車なし  日付の下一桁が0
 * @retval  01	シングルトレイン	日付の下一桁が1
 * @retval  02	スーパーシングルトレイン	日付の下一桁が5
 * @retval  03	ダブルトレイン	日付の下一桁が2
 * @retval  04	スーパーダブルトレイン	日付の下一桁が4
 * @retval  05	マルチトレイン	日付の下一桁が3
 * @retval  06	スーパーマルチトレイン	日付の下一桁が6
 * @retval  07	Wi-Fiトレイン	日付の下一桁が9
 * @retval  08	トレインタウン	日付の下一桁が7
 * @retval  09	古いレアトレイン　	日付の下一桁が0
 * @retval  10	新しいレアトレイン	特別な日
 * 
 */
//--------------------------------------------------------------
#define _GET_TRAIN_TOWN_COND( ret_wk )  \
    _ASM_GET_TRAIN_TOWN_COND ret_wk
    .macro  _ASM_GET_TRAIN_TOWN_COND ret_wk
    .short  EV_SEQ_GET_TRAIN_TOWN_COND
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_TRAIN_MODEL_ID
 * @brief   トレインタウンの電車モデルIDを取得
 * @param index   電車の種類指定（０～９）
 * @param ret_wk  電車のモデルIDを受け取るワーク
 *
 * トレインタウンでのみ使用するコマンド。
 */
//--------------------------------------------------------------
#define _GET_TRAIN_MODEL_ID( index, ret_wk ) \
    _ASM_GET_TRAIN_MODEL_ID index, ret_wk

    .macro  _ASM_GET_TRAIN_MODEL_ID index, ret_wk
    .short  EV_SEQ_GET_TRAIN_MODEL_ID
    .short  \index
    .short  \ret_wk
    .endm

//======================================================================
//
//
// 育て屋関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 育て屋のポケモンにタマゴが生まれたかどうかのチェック
 * 
 * @param ret_wk 結果の格納先
 * @return       タマゴがある場合 TRUE
 */
//--------------------------------------------------------------
#define _CHECK_SODATEYA_HAVE_EGG( ret_wk ) \
    _ASM_CHECK_SODATEYA_HAVE_EGG ret_wk

  .macro _ASM_CHECK_SODATEYA_HAVE_EGG ret_wk
  .short EV_SEQ_CHECK_SODATEYA_HAVE_EGG
  .short \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 育て屋のタマゴを受け取る
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_EGG() \
    _ASM_GET_SODATEYA_EGG

  .macro _ASM_GET_SODATEYA_EGG
  .short EV_SEQ_GET_SODATEYA_EGG
  .endm

//--------------------------------------------------------------
/**
 * 育て屋のタマゴを削除する
 */
//--------------------------------------------------------------
#define _DELETE_SODATEYA_EGG() \
    _ASM_DELETE_SODATEYA_EGG

  .macro _ASM_DELETE_SODATEYA_EGG
  .short EV_SEQ_DELETE_SODATEYA_EGG
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンの数を取得する
 *
 * @param ret_wk 結果の格納先
 * @return       育て屋に預けているポケモンの数
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKEMON_NUM( ret_wk ) \
    _ASM_GET_SODATEYA_POKEMON_NUM ret_wk

  .macro _ASM_GET_SODATEYA_POKEMON_NUM ret_wk
  .short EV_SEQ_GET_SODATEYA_POKEMON_NUM
  .short \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けている2体のポケモンの相性をチェックする
 *
 * @param ret_wk 結果の格納先
 * @return 
 */
//--------------------------------------------------------------
#define _SODATEYA_LOVE_CHECK( ret_wk ) \
    _ASM_SODATEYA_LOVE_CHECK ret_wk

  .macro _ASM_SODATEYA_LOVE_CHECK ret_wk
  .short EV_SEQ_SODATEYA_LOVE_CHECK
  .short \ret_wk
  .endm
//--------------------------------------------------------------
/**
 * ポケモンを育て屋に預ける
 *
 * @param pos 預けるポケモンの手持ち番号
 */
//--------------------------------------------------------------
#define _POKE_TO_SODATEYA( pos ) \
    _ASM_POKE_TO_SODATEYA pos

  .macro _ASM_POKE_TO_SODATEYA pos
  .short EV_SEQ_POKE_TO_SODATEYA
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * ポケモンを育て屋から受け取る
 *
 * @param pos 受け取るポケモンの育て屋内番号(0 or 1)
 */
//--------------------------------------------------------------
#define _POKE_FROM_SODATEYA( pos ) \
    _ASM_POKE_FROM_SODATEYA pos

  .macro _ASM_POKE_FROM_SODATEYA pos
  .short EV_SEQ_POKE_FROM_SODATEYA
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンのモンスターナンバーを取得する
 *
 * @param ret_wk 取得したモンスターナンバーの格納先
 * @param pos    取得対象ポケモンの育て屋内インデックス
 * @retval u16   指定ポケモンのモンスターナンバー
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKE_MONSNO( ret_wk, pos ) \
    _ASM_GET_SODATEYA_POKE_MONSNO ret_wk, pos

  .macro _ASM_GET_SODATEYA_POKE_MONSNO ret_wk, pos
  .short EV_SEQ_GET_SODATEYA_POKE_MONSNO
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンの形状ナンバーを取得する
 *
 * @param ret_wk 取得した形状ナンバーの格納先
 * @param pos    取得対象ポケモンの育て屋内インデックス
 * @retval u16   指定ポケモンの形状ナンバー
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKE_FORMNO( ret_wk, pos ) \
    _ASM_GET_SODATEYA_POKE_FORMNO ret_wk, pos

  .macro _ASM_GET_SODATEYA_POKE_FORMNO ret_wk, pos
  .short EV_SEQ_GET_SODATEYA_POKE_FORMNO
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンの現在のレベルを取得する
 *
 * @param ret_wk 取得したレベルの格納先
 * @param pos    取得対象ポケモンの育て屋内インデックス
 * @retval u16   指定ポケモンのレベル
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKE_LV( ret_wk, pos ) \
    _ASM_GET_SODATEYA_POKE_LV ret_wk, pos

  .macro _ASM_GET_SODATEYA_POKE_LV ret_wk, pos
  .short EV_SEQ_GET_SODATEYA_POKE_LV
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンについて, 成長したレベルを取得する
 *
 * @param ret_wk 取得した値の格納先
 * @param pos    取得対象ポケモンの育て屋内インデックス
 * @retval u16   指定ポケモンが成長したレベル
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKE_GROWUP_LV( ret_wk, pos ) \
    _ASM_GET_SODATEYA_POKE_GROWUP_LV ret_wk, pos

  .macro _ASM_GET_SODATEYA_POKE_GROWUP_LV ret_wk, pos
  .short EV_SEQ_GET_SODATEYA_POKE_GROWUP_LV
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンについて, 引き取り料金を取得する
 *
 * @param ret_wk 取得した値の格納先
 * @param pos    取得対象ポケモンの育て屋内インデックス
 * @retval u16   指定ポケモンの引き取り料金
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKE_CHARGE( ret_wk, pos ) \
    _ASM_GET_SODATEYA_POKE_CHARGE ret_wk, pos

  .macro _ASM_GET_SODATEYA_POKE_CHARGE ret_wk, pos
  .short EV_SEQ_GET_SODATEYA_POKE_CHARGE
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * 手持ちポケモン選択画面を呼び出す
 *
 * @param ret_wk 選択結果の格納先
 * @retval u16   選択結果
 */
//--------------------------------------------------------------
#define _SODATEYA_SELECT_POKEMON( ret_wk ) \
    _ASM_SODATEYA_SELECT_POKEMON ret_wk

  .macro _ASM_SODATEYA_SELECT_POKEMON ret_wk
  .short EV_SEQ_SODATEYA_SELECT_POKEMON
  .short \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 育て屋に預けているポケモンについて, 性別を取得する
 *
 * @param ret_wk 取得した値の格納先
 * @param pos    取得対象ポケモンの育て屋内インデックス
 * @retval u16   指定ポケモンの性別
 *               オス = 0
 *               メス = 1
 *               性別なし = 2
 */
//--------------------------------------------------------------
#define _GET_SODATEYA_POKE_SEX( ret_wk, pos ) \
    _ASM_GET_SODATEYA_POKE_SEX ret_wk, pos

  .macro _ASM_GET_SODATEYA_POKE_SEX ret_wk, pos
  .short EV_SEQ_GET_SODATEYA_POKE_SEX
  .short \ret_wk
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * @def _SODATEYA_CHECK
 * @brief 育て屋特殊チェック
 * 
 * @param ret_wk   結果の格納先
 * @param check_id チェックID ( SCR_SODATEYA_CHECK_ID_xxxx )
 * @param poke_pos 預けポケモンのインデックス
 *
 * @return チェックの結果 ( SCR_SODATEYA_CHECK_RESULT_xxxx )
 */
//--------------------------------------------------------------
#define _SODATEYA_CHECK( ret_wk, check_id, poke_pos ) \
    _ASM_SODATEYA_CHECK ret_wk, check_id, poke_pos

  .macro _ASM_SODATEYA_CHECK ret_wk, check_id, poke_pos
  .short EV_SEQ_SODATEYA_CHECK
  .short \ret_wk
  .short \check_id
  .short \poke_pos
  .endm

//======================================================================
//  サブプロセスコール関連
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドプロセスを生成する
 */
//--------------------------------------------------------------
#define _FIELD_OPEN() _ASM_FIELD_OPEN

  .macro _ASM_FIELD_OPEN
  .short EV_SEQ_FIELD_OPEN
  .endm

//--------------------------------------------------------------
/**
 * フィールドプロセスを終了する
 */
//--------------------------------------------------------------
#define _FIELD_CLOSE() _ASM_FIELD_CLOSE

  .macro _ASM_FIELD_CLOSE
  .short EV_SEQ_FIELD_CLOSE
  .endm

//--------------------------------------------------------------
/**
 * サブプロセスワークを解放する
 *
 * ＊使用は玉田さんの許可制です。
 * 　プロセスコールは原則として、呼び出し～戻り値取得までをOneコマンドで実現する設計にすること！
 */
//--------------------------------------------------------------
#define _FREE_SUBPROC_WORK() _ASM_SUBPROC_WORK

  .macro _ASM_SUBPROC_WORK
  .short EV_SEQ_FREE_SUB_PROC_WORK
  .endm

//--------------------------------------------------------------
/**
 * @brief 殿堂入り画面を呼び出す
 *
 * @param type   呼び出しタイプ ( DENDOU_PC_xxxx )
 * @param ret_wk 終了モードを受け取るワーク
 *
 * @return 0 通常終了
 *         1 ×ボタン終了
 */
//--------------------------------------------------------------
#define _CALL_DENDOU_PC_PROC( type, ret_wk ) \
    _ASM_CALL_DENDOU_PC_PROC type, ret_wk

  .macro  _ASM_CALL_DENDOU_PC_PROC type, ret_wk
  .short  EV_SEQ_CALL_DENDOU_PC_PROC
  .short  \type
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @brief ゲーム内マニュアル画面を呼び出す
 *
 * @param ret_wk 終了モードを受け取るワーク
 *
 * @return 0 通常終了
 *         1 ×ボタン終了
 */
//--------------------------------------------------------------
#define _CALL_GAME_MANUAL( ret_wk ) \
    _ASM_CALL_GAME_MANUAL ret_wk

  .macro  _ASM_CALL_GAME_MANUAL ret_wk
  .short  EV_SEQ_CALL_GAME_MANUAL
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  バッグプロセスを呼び出す
 *  @param  mode  呼び出しモード script_def.h SCR_BAGMODE_SELL, SCR_BAG_MODE_FIELD SCR_BAG_MODE_SELECT他
 *  @param  ret_mode  バッグの終了モード　script_def.h SCR_PROC_RETMODE_EXIT 他
 *  @param  ret_item  選択アイテムNo(0なら選択アイテム無し)
 *
 *  サブプロセスワークを解放しないので、使用後に _FREE_SUBPROC_WORK()を呼び出すこと
 */
//--------------------------------------------------------------
#define _CALL_BAG_PROC( mode, ret_mode, ret_item ) \
    _ASM_CALL_BAG_PROC mode, ret_mode, ret_item

  .macro  _ASM_CALL_BAG_PROC mode, ret_mode, ret_item
  .short  EV_SEQ_CALL_BAG_PROC
  .short  \mode
  .short  \ret_mode
  .short  \ret_item
  .endm

//--------------------------------------------------------------
/**
 *  @brief ボックスプロセスを呼び出す
 *  @param ret_end_mode ボックス終了モードを受け取るワーク
 *  @param mode         ボックス呼び出しモード( BOX_MODE_xxxx )
 *
 *  ※呼び出しモード prog/include/app/box_mode.h
 */
//--------------------------------------------------------------
#define _CALL_BOX_PROC( ret_end_mode, mode ) \
    _ASM_CALL_BOX_PROC ret_end_mode, mode

  .macro  _ASM_CALL_BOX_PROC ret_end_mode, mode
  .short  EV_SEQ_CALL_BOX_PROC
  .short  \ret_end_mode
  .short  \mode
  .endm

//--------------------------------------------------------------
/**
 *  @brief メールボックスプロセスを呼び出す
 */
//--------------------------------------------------------------
#define _CALL_MAILBOX_PROC( ret_end_mode ) \
    _ASM_CALL_MAILBOX_PROC ret_end_mode 

  .macro  _ASM_CALL_MAILBOX_PROC ret_end_mode
  .short  EV_SEQ_CALL_MAILBOX_PROC
  .short  \ret_end_mode
  .endm

//--------------------------------------------------------------
/**
 * @def _CALL_GEONET_PROC
 * @brief   ジオネット地球儀の呼び出し
 *
 * ジオネットの地球儀画面を呼び出します
 */
//--------------------------------------------------------------
#define _CALL_GEONET_PROC() _ASM_CALL_GEONET_PROC

  .macro  _ASM_CALL_GEONET_PROC
  .short  EV_SEQ_CALL_GEONET_PROC
  .endm

//--------------------------------------------------------------
/**
 * @def _CALL_ZUKAN_AWARD
 * @brief   図鑑完成表彰状＆路線図表示アプリ呼び出し
 *
 * 図鑑呼び出しID
 *  地方図鑑 SCR_ZUKAN_AWARD_CHIHOU
 *  全国図鑑 SCR_ZUKAN_AWARD_ZENKOKU
 */
//--------------------------------------------------------------
//図鑑
#define _CALL_ZUKAN_AWARD( demo_id, pattern ) \
    _ASM_ZUKAN_AWARD_PROC_CALL demo_id, pattern

//路線図
#define _CALL_SUBWAY_ROUTE_MAP() \
    _ASM_ZUKAN_AWARD_PROC_CALL SCR_ZUKAN_AWARD_SUBWAY_ROUTE_MAP, 0 

  .macro  _ASM_CALL_ZUKAN_AWARD demo_id, pattern
  .short  EV_SEQ_CALL_ZUKAN_AWARD
  .short  \demo_id
  .short  \pattern
  .endm

//--------------------------------------------------------------
/**
 * @def _CALL_FIRST_POKE_SELECT
 * @brief 最初の３体選択
 * @param ret_wk
 */
//--------------------------------------------------------------
#define _CALL_FIRST_POKE_SELECT( ret_wk ) \
    _ASM_FIRST_POKE_SELECT_CALL ret_wk

  .macro  _ASM_CALL_FIRST_POKE_SELECT ret_wk
  .short  EV_SEQ_CALL_FIRST_POKE_SELECT
  .short  \ret_wk
  .endm

//======================================================================
//  ショップ関連
//======================================================================
//--------------------------------------------------------------
/**
 * @def _SHOP_CALL
 * @brief 簡易イベントコマンド：ショップイベント呼び出し
 * @param shop_id
 */
//--------------------------------------------------------------
#define _SHOP_CALL( ) _ASM_SHOP_CALL  SCR_SHOPID_NULL,SHOP_GREETING_NORMAL 

//--------------------------------------------------------------
/**
 * @def _SHOP_CALL_GREETING_LESS
 * @brief 簡易イベントコマンド：ショップイベント呼び出し
 * @param shop_id
 * _SHOP_CALLとの違いは話しかけ時の「いらっしゃいませ」の挨拶無し
 * ※固定ショップ、BPショップなど各ショップに挨拶無しバージョンが存在します。
 */
//--------------------------------------------------------------
#define _SHOP_CALL_GREETING_LESS( ) _ASM_SHOP_CALL  SCR_SHOPID_NULL,SHOP_GREETING_LESS

//--------------------------------------------------------------
/**
 * @def _FIX_SHOP_CALL
 * @brief 簡易イベントコマンド：固定ショップイベント呼び出し
 * @param shop_id
 */
//--------------------------------------------------------------
#define _FIX_SHOP_CALL( shop_id ) _ASM_SHOP_CALL shop_id,SHOP_GREETING_NORMAL
#define _FIX_SHOP_CALL_GREETING_LESS( shop_id ) _ASM_SHOP_CALL shop_id,SHOP_GREETING_LESS
#define _FIX_SHOP_CALL_GREETING_WAZA( shop_id ) _ASM_SHOP_CALL shop_id,SHOP_GREETING_WAZA

//--------------------------------------------------------------
/**
 * @def _BP_ITEM_SHOP_CALL
 * @brief 簡易イベントコマンド：BPショップイベント呼び出し
 */
//--------------------------------------------------------------
#define _BP_ITEM_SHOP_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BP_ITEM,SHOP_GREETING_NORMAL
#define _BP_ITEM_SHOP_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BP_ITEM,SHOP_GREETING_LESS

//--------------------------------------------------------------
/**
 * @def _BP_WAZA_SHOP_CALL
 * @brief 簡易イベントコマンド：BPワザショップイベント呼び出し
 */
//--------------------------------------------------------------
#define _BP_WAZA_SHOP_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BP_WAZA,SHOP_GREETING_WAZA
#define _BP_WAZA_SHOP_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BP_WAZA,SHOP_GREETING_LESS

//--------------------------------------------------------------
/**
 * @def _BLACK_CITY_SHOP_CALL
 * @brief 簡易イベントコマンド：ブラックシティショップイベント呼び出し
 */
//--------------------------------------------------------------
#define _BLACK_CITY_SHOP0_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY0,SHOP_GREETING_NORMAL
#define _BLACK_CITY_SHOP0_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY0,SHOP_GREETING_LESS
#define _BLACK_CITY_SHOP1_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY1,SHOP_GREETING_NORMAL
#define _BLACK_CITY_SHOP1_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY1,SHOP_GREETING_LESS
#define _BLACK_CITY_SHOP2_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY2,SHOP_GREETING_NORMAL
#define _BLACK_CITY_SHOP2_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY2,SHOP_GREETING_LESS
#define _BLACK_CITY_SHOP3_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY3,SHOP_GREETING_NORMAL
#define _BLACK_CITY_SHOP3_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY3,SHOP_GREETING_LESS
#define _BLACK_CITY_SHOP4_CALL( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY4,SHOP_GREETING_NORMAL
#define _BLACK_CITY_SHOP4_CALL_GREETING_LESS( ) _ASM_SHOP_CALL SCR_SHOPID_BLACK_CITY4,SHOP_GREETING_LESS

//--------------------------------------------------------------
/**
 * @def _CALL_SHOP_PROC_BUY
 * @brief ショップイベント呼び出し
 * @param shop_id SCR_SHOPID_NULL を指定すると変動ショップ、それ以外なら固定ショップ
 *
 * shop_scr.ev専用　ゾーンスクリプトから直接呼び出さないこと！
 */
//--------------------------------------------------------------
#define _CALL_SHOP_PROC_BUY( shop_id, ret_mode )  _ASM_CALL_SHOP_PROC_BUY shop_id,ret_mode

  .macro  _ASM_CALL_SHOP_PROC_BUY shop_id, ret_mode
  .short  EV_SEQ_CALL_SHOP_PROC_BUY
  .short  \shop_id
  .short  \ret_mode
  .endm

//======================================================================
// WiFiクラブ関連
//======================================================================
//--------------------------------------------------------------
/**
 * @brief    GSIDが正しいかをチェックする
 */
//--------------------------------------------------------------
#define _WIFI_CHECK_MY_GSID( ret_work ) _ASM_WIFI_CHECK_MY_GSID ret_work

  .macro  _ASM_WIFI_CHECK_MY_GSID ret_work
  .short  EV_SEQ_WIFI_CHECK_MY_GSID
  .short  \ret_work
  .endm

//--------------------------------------------------------------------
/**
 * @brief   友だち手帳に登録されている人数を返す 
 */
//--------------------------------------------------------------------
#define _WIFI_GET_FRIEND_NUM( ret_work ) _ASM_WIFI_GET_FRIEND_NUM ret_work

  .macro  _ASM_WIFI_GET_FRIEND_NUM ret_work
  .short  EV_SEQ_WIFI_GET_FRIEND_NUM
  .short  \ret_work
  .endm

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fiクラブイベントを呼び出す
 */
//--------------------------------------------------------------------
#define _WIFI_CLUB_EVENT_CALL( ) _ASM_WIFI_CLUB_EVENT_CALL

  .macro  _ASM_WIFI_CLUB_EVENT_CALL
  .short  EV_SEQ_WIFI_CLUB_EVENT_CALL
  .endm

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fi GTSイベントを呼び出す
 */
//--------------------------------------------------------------------
#define _WIFI_GTS_EVENT_CALL( ) _ASM_WIFI_GTS_EVENT_CALL

  .macro  _ASM_WIFI_GTS_EVENT_CALL
  .short  EV_SEQ_WIFI_GTS_EVENT_CALL
  .endm

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fi GTSネゴシエーションイベントを呼び出す
 */
//--------------------------------------------------------------------
#define _WIFI_GTS_NEGO_EVENT_CALL( ) _ASM_WIFI_GTS_NEGO_EVENT_CALL

  .macro  _ASM_WIFI_GTS_NEGO_EVENT_CALL
  .short  EV_SEQ_WIFI_GTS_NEGO_EVENT_CALL
  .endm


//--------------------------------------------------------------------
/**
 * @brief   Wi-Fi GeoNetイベントを呼び出す
 */
//--------------------------------------------------------------------
#define _WIFI_GEO_NET_CALL( ) _ASM_WIFI_GEO_NET_CALL

  .macro  _ASM_WIFI_GEO_NET_CALL
  .short  EV_SEQ_WIFI_GEO_NET_CALL
  .endm

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fi ランダムマッチイベントを呼び出す
 */
//--------------------------------------------------------------------
#define _WIFI_RANDOM_MATCH_EVENT_CALL( regulation, party ) \
  _ASM_WIFI_RANDOM_MATCH_EVENT_CALL regulation, party

  .macro  _ASM_WIFI_RANDOM_MATCH_EVENT_CALL regulation, party
  .short  EV_SEQ_WIFI_RANDOM_MATCH_EVENT_CALL
  .short  \regulation
  .short  \party
  .endm

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fi バトルレコーダーイベントを呼び出す
 */
//--------------------------------------------------------------------
#define _WIFI_BATTLE_RECORDER_EVENT_CALL( mode ) _ASM_WIFI_BATTLE_RECORDER_EVENT_CALL mode

  .macro  _ASM_WIFI_BATTLE_RECORDER_EVENT_CALL mode
  .short  EV_SEQ_WIFI_BATTLE_RECORDER_EVENT_CALL
  .short  \mode
  .endm

//--------------------------------------------------------------
/**
 * @def _BTL_UTIL_PARTY_SELECT_CALL
 * @brief パーティ選択ウィンドウ呼び出し
 *
 * @param regulation  poke_tool/regulation_def.hに定義されたコード
 * @param ret_wk  どのパーティを選択したかを取得 btl_util_scr_local.h
 *                SCR_BTL_PARTY_SELECT_TEMOTI  手持ちを選択
 *                SCR_BTL_PARTY_SELECT_BTLBOX  バトルボックスを選択
 *                SCR_BTL_PARTY_SELECT_NG      どちらも適合しない
 *                SCR_BTL_PARTY_SELECT_CANCEL  キャンセル
 */
//--------------------------------------------------------------
#define _BTL_UTIL_PARTY_SELECT_CALL( regulation, ret_wk )  \
    _ASM_BTL_UTIL_PARTY_SELECT_CALL  regulation, ret_wk

  .macro  _ASM_BTL_UTIL_PARTY_SELECT_CALL regulation, ret_wk 
  .short  EV_SEQ_BTL_UTIL_PARTY_SELECT
  .short  \regulation
  .short  \ret_wk
  .endm

//======================================================================
//  秘伝技関連
//======================================================================
//--------------------------------------------------------------
/**
 * なみのり開始
 * @param none
 */
//--------------------------------------------------------------
#define _NAMINORI() _ASM_NAMINORI
  
  .macro _ASM_NAMINORI
  .short EV_SEQ_NAMINORI
  .endm

//--------------------------------------------------------------
/**
 * 滝登り開始
 * @param no 技を使う手持ちポケモン位置番号
 */
//--------------------------------------------------------------
#define _TAKINOBORI( no ) _ASM_TAKINOBORI no
  
  .macro _ASM_TAKINOBORI no
  .short EV_SEQ_TAKINOBORI
  .short \no
  .endm

//--------------------------------------------------------------
/**
 * @def _IAIGIRI_EFFECT
 * @brief 居合い切りエフェクト表示
 */
//--------------------------------------------------------------
#define _IAIGIRI_EFFECT() \
    _ASM_IAIGIRI_EFFECT
  
  .macro _ASM_IAIGIRI_EFFECT
  .short EV_SEQ_IAIGIRI_EFFECT
  .endm

//--------------------------------------------------------------
/**
 * @brief フィールド技：ダイビング
 * フィールド技スクリプトと海底遺跡でのみ使用しています。
 * 他の箇所では使用しないでください。
 *
 *  param 上昇と下降  SCR_EV_DIVING_MAPCHANGE_DOWN　
 *                    SCR_EV_DIVING_MAPCHANGE_UP 
 *                    SCR_EV_DIVING_MAPCHANGE_KURUKURU_UP
 */
//--------------------------------------------------------------
#define _DIVING( up_down ) _ASM_DIVING up_down
  .macro  _ASM_DIVING up_down
  .short  EV_SEQ_DIVING
  .short  \up_down
  .endm

//======================================================================
//ビーコンセットリクエスト系
//======================================================================
//--------------------------------------------------------------
/*
 *  ビーコンセットリクエスト共有マクロ
 */
//--------------------------------------------------------------
  .macro _ASM_BEACON_SET_REQ beacon_id, value
  .short EV_SEQ_BEACON_SET_REQUEST
  .short \beacon_id
  .short \value 
  .endm

/**
 * @def _BEACON_SET_REQ_ITEM_GET
 * @brief アイテム拾ったビーコンセット
 * @param itemno  取得したアイテムのナンバー
 */
#define _BEACON_SET_REQ_ITEM_GET( itemno ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_ITEM_GET, itemno

/**
 * @def _BEACON_SET_REQ_FSKILL_USE
 * @brief フィールド技使用ビーコンセット
 * @param wazano  使用した技No
 */
#define _BEACON_SET_REQ_FSKILL_USE( wazano ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_FSKILL_USE, wazano

/**
 * @def _BEACON_SET_REQ_UNION_COUNTER
 * @brief ユニオンルームカウンター処理開始ビーコンセット
 */
#define _BEACON_SET_REQ_UNION_COUNTER( ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_UNION_COUNTER, 0

/**
 * @def _BEACON_SET_REQ_TRIAL_HOUSE_START
 * @brief トライアルハウス挑戦開始ビーコンセット
 */
#define _BEACON_SET_REQ_TRIAL_HOUSE_START( ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_TRIAL_HOUSE_START, 0

/**
 * @def _BEACON_SET_REQ_TRIAL_HOUSE_RANK
 * @brief トライアルハウスランクビーコンセット
 */
#define _BEACON_SET_REQ_TRIAL_HOUSE_RANK( rank ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_TRIAL_HOUSE_RANK, rank

/**
 * @def _BEACON_SET_REQ_POKE_SHIFTER_START
 * @brief ポケシフター挑戦開始ビーコンセット
 */
#define _BEACON_SET_REQ_POKE_SHIFTER_START( ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_POKE_SHIFTER_START, 0

/**
 * @def _BEACON_SET_REQ_SUBWAY_START
 * @brief バトルサブウェイ挑戦開始ビーコンセット
 */
#define _BEACON_SET_REQ_SUBWAY_START( ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_SUBWAY_START, 0

/**
 * @def _BEACON_SET_REQ_SUBWAY_WIN
 * @brief バトルサブウェイラウンド数ビーコンセット
 */
#define _BEACON_SET_REQ_SUBWAY_WIN( round_num ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_SUBWAY_WIN, round_num 

/**
 * @def _BEACON_SET_REQ_SUBWAY_TROPHY_GET
 * @brief バトルサブウェイトロフィー取得ビーコンセット
 */
#define _BEACON_SET_REQ_SUBWAY_TROPHY_GET( ) \
    _ASM_BEACON_SET_REQ SCR_BEACON_SET_REQ_SUBWAY_TROPHY_GET, 0

//--------------------------------------------------------------
/**
 * @def _CHECK_GPOWER_FINISH
 * @brief Gパワーフィニッシュチェック
 */
//--------------------------------------------------------------
#define _CHECK_GPOWER_FINISH( ret_gpower, ret_next ) \
    _ASM_CHECK_GPOWER_FINISH ret_gpower, ret_next
  
  .macro _ASM_CHECK_GPOWER_FINISH ret_gpower, ret_next 
  .short EV_SEQ_CHECK_GPOWER_FINISH
  .short \ret_gpower
  .short \ret_next
  .endm

//--------------------------------------------------------------
/**
 * @def _GPOWER_USE_EFFECT
 * @brief 居合い切りエフェクト表示
 */
//--------------------------------------------------------------
#define _GPOWER_USE_EFFECT() \
    _ASM_GPOWER_USE_EFFECT
  
  .macro _ASM_GPOWER_USE_EFFECT
  .short EV_SEQ_GPOWER_USE_EFFECT
  .endm

//-----------------------------------------------------------------------------
// ビッグスタジアム リトルコート
//-----------------------------------------------------------------------------
//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_START
 * @brief 再戦トレーナー情報を確保
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_START() \
    _ASM_REBATTLE_TRAINER_START
  
  .macro _ASM_REBATTLE_TRAINER_START
  .short EV_SEQ_REBATTLE_TRAINER_START
  .endm

//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_END
 * @brief 再戦トレーナー情報を破棄
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_END() \
    _ASM_REBATTLE_TRAINER_END
  
  .macro _ASM_REBATTLE_TRAINER_END
  .short EV_SEQ_REBATTLE_TRAINER_END
  .endm

//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_SETUP_RND_TR
 * @brief 再戦トレーナー　ランダムトレーナー　設定
 *
 * @param objid0         設定するOBJ識別ID  
 * @param objid1         設定するOBJ識別ID  
 * @param objid2         設定するOBJ識別ID  
 * @param index0         設定するNPCインデックス
 * @param index1         設定するNPCインデックス  
 * @param index2         設定するNPCインデックス  
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_SETUP_RND_TR( objid0, objid1, objid2, index0, index1, index2 ) \
    _ASM_REBATTLE_TRAINER_SETUP_RND_TR objid0, objid1, objid2, index0, index1, index2
  
  .macro _ASM_REBATTLE_TRAINER_SETUP_RND_TR objid0, objid1, objid2, index0, index1, index2
  .short EV_SEQ_REBATTLE_TRAINER_SETUP_RND_TR
  .short \objid0
  .short \objid1
  .short \objid2
  .short \index0
  .short \index1
  .short \index2
  .endm

//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_SETUP_TRID
 * @brief 再戦トレーナー　固定トレーナー　設定
 *
 * @param rebattle_id   再戦トレーナー種別  (prog/src/field/rebattle_trainer.h)
 * @param objid         設定するOBJ識別ID  
 * @param level         段階(1～4)
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_SETUP_TRID( rebattle_id, objid, level ) \
    _ASM_REBATTLE_TRAINER_SETUP_TRID rebattle_id, objid, level
  
  .macro _ASM_REBATTLE_TRAINER_SETUP_TRID rebattle_id, objid, level
  .short EV_SEQ_REBATTLE_TRAINER_SETUP_TRID
  .short \rebattle_id
  .short \objid
  .short \level
  .endm

//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_SETUP_2VS2_TRID
 * @brief 再戦トレーナー　固定トレーナー　設定
 *
 * @param rebattle_id   再戦トレーナー種別  (prog/src/field/rebattle_trainer.h)
 * @param objid0        設定するOBJ識別ID  
 * @param objid1        設定するOBJ識別ID  
 * @param level         段階(1～4)
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_SETUP_2VS2_TRID( rebattle_id, objid0, objid1, level ) \
    _ASM_REBATTLE_TRAINER_SETUP_2VS2_TRID rebattle_id, objid0, objid1, level
  
  .macro _ASM_REBATTLE_TRAINER_SETUP_2VS2_TRID rebattle_id, objid0, objid1, level
  .short EV_SEQ_REBATTLE_TRAINER_SETUP_2VS2_TRID
  .short \rebattle_id
  .short \objid0
  .short \objid1
  .short \level
  .endm


//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_CLEAR_TR_EVENT_FLAG
 * @brief 再戦トレーナー　戦闘イベントフラグ　リセット
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_CLEAR_TR_EVENT_FLAG() \
    _ASM_REBATTLE_TRAINER_CLEAR_TR_EVENT_FLAG
  
  .macro _ASM_REBATTLE_TRAINER_CLEAR_TR_EVENT_FLAG
  .short EV_SEQ_REBATTLE_TRAINER_CLEAR_TR_EVENT_FLAG
  .endm


//======================================================================
//
//
//  マップフェード関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * マップフェード季節イン（コンティニュー専用）
 * @note
 * 特殊なコンティニュー処理を行うためのスクリプトコマンド。
 * 通常は使用禁止です。
 * 現在時刻の季節を表示してからフェードインします。
 */
//--------------------------------------------------------------
#define _MAP_FADE_SEASON_IN_FORCE() _ASM_MAP_FADE_SEASON_IN_FORCE

  .macro  _ASM_MAP_FADE_SEASON_IN_FORCE
  .short  EV_SEQ_MAP_FADE_SEASON_IN_FORCE
  .endm

//--------------------------------------------------------------
/**
 * マップフェードブラックイン（コンティニュー専用）
 * @note
 * 特殊なコンティニュー処理を行うためのスクリプトコマンド。
 * 通常は使用禁止です。
 */
//--------------------------------------------------------------
#define _MAP_FADE_BLACK_IN_FORCE() _ASM_MAP_FADE_BLACK_IN_FORCE

  .macro  _ASM_MAP_FADE_BLACK_IN_FORCE
  .short  EV_SEQ_MAP_FADE_BLACK_IN_FORCE
  .endm

//--------------------------------------------------------------
/**
 * マップフェードブラックイン
 * @note 暗転後にアプリ起動などで、マップが破壊される場合はこちらのフェードを使います。
 	* _BLACK_IN()は画面を暗くするだけの時にしか使えません。
 */
//--------------------------------------------------------------
#define _MAP_FADE_BLACK_IN() _ASM_MAP_FADE_BLACK_IN

  .macro  _ASM_MAP_FADE_BLACK_IN
  .short  EV_SEQ_MAP_FADE_BLACK_IN
  .endm

//--------------------------------------------------------------
/**
 * マップフェードブラックアウト
 * @note 暗転後にアプリ起動などで、マップが破壊される場合はこちらのフェードを使います。
 * _BLACK_OUT()は画面を暗くするだけの時にしか使えません。
 */
//--------------------------------------------------------------
#define _MAP_FADE_BLACK_OUT() _ASM_MAP_FADE_BLACK_OUT

  .macro  _ASM_MAP_FADE_BLACK_OUT
  .short  EV_SEQ_MAP_FADE_BLACK_OUT
  .endm

//--------------------------------------------------------------
/**
 * マップフェードホワイトイン
 */
//--------------------------------------------------------------
#define _MAP_FADE_WHITE_IN() _ASM_MAP_FADE_WHITE_IN

  .macro  _ASM_MAP_FADE_WHITE_IN
  .short  EV_SEQ_MAP_FADE_WHITE_IN
  .endm

//--------------------------------------------------------------
/**
 * マップフェードホワイトアウト
 */
//--------------------------------------------------------------
#define _MAP_FADE_WHITE_OUT() _ASM_MAP_FADE_WHITE_OUT

  .macro  _ASM_MAP_FADE_WHITE_OUT
  .short  EV_SEQ_MAP_FADE_WHITE_OUT
  .endm

//--------------------------------------------------------------
/**
 * マップフェードブラックイン　遅い版
 * @note 暗転後にアプリ起動などで、マップが破壊される場合はこちらのフェードを使います。
 * _BLACK_IN()は画面を暗くするだけの時にしか使えません。
 */
//--------------------------------------------------------------
#define _MAP_FADE_BLACK_IN_SLOW() _ASM_MAP_FADE_BLACK_IN_SLOW

  .macro  _ASM_MAP_FADE_BLACK_IN_SLOW
  .short  EV_SEQ_MAP_FADE_BLACK_IN_SLOW
  .endm

//--------------------------------------------------------------
/**
 * マップフェードブラックアウト　遅い版
 * @note 暗転後にアプリ起動などで、マップが破壊される場合はこちらのフェードを使います。
 * _BLACK_OUT()は画面を暗くするだけの時にしか使えません。
 */
//--------------------------------------------------------------
#define _MAP_FADE_BLACK_OUT_SLOW() _ASM_MAP_FADE_BLACK_OUT_SLOW

  .macro  _ASM_MAP_FADE_BLACK_OUT_SLOW
  .short  EV_SEQ_MAP_FADE_BLACK_OUT_SLOW
  .endm

//--------------------------------------------------------------
/**
 * マップフェードホワイトイン　遅い版
 */
//--------------------------------------------------------------
#define _MAP_FADE_WHITE_IN_SLOW() _ASM_MAP_FADE_WHITE_IN_SLOW

  .macro  _ASM_MAP_FADE_WHITE_IN_SLOW
  .short  EV_SEQ_MAP_FADE_WHITE_IN_SLOW
  .endm

//--------------------------------------------------------------
/**
 * マップフェードホワイトアウト　遅い版
 */
//--------------------------------------------------------------
#define _MAP_FADE_WHITE_OUT_SLOW() _ASM_MAP_FADE_WHITE_OUT_SLOW

  .macro  _ASM_MAP_FADE_WHITE_OUT_SLOW
  .short  EV_SEQ_MAP_FADE_WHITE_OUT_SLOW
  .endm

//--------------------------------------------------------------
/**
 *  _DISP_FADE_CHECK　マップ遷移用フェード終了チェック
 *  @param none
 */
//--------------------------------------------------------------
#define _MAP_FADE_END_CHECK()  _ASM_MAP_FADE_END_CHECK

  .macro  _ASM_MAP_FADE_END_CHECK
  .short EV_SEQ_MAP_FADE_CHECK
  .endm


//======================================================================
//
//  簡易コマンド
//  ※スクリプトコマンドを組み合わせたもの
//
//======================================================================
.include  "easy_event_def.h"

//--------------------------------------------------------------
/**
 * 簡易吹き出しメッセージ表示 話し掛けOBJ用
 * @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _EASY_BALLOONWIN_TALKOBJ_MSG( msg_id ) \
  _ASM_EASY_BALLOONWIN_TALKOBJ_MSG msg_id

//--------------------------------------------------------------
/**
 * 簡易BGウィンドウメッセージ表示
 * @param msg_id 表示するメッセージID
 * @param bg_type 表示するタイプ TYPE_INFO,TYPE_TOWN,TYPE_POST,TYPE_ROAD
                  TYPE_INFO → お得な掲示板
                  TYPE_TOWN → 街看板
                  TYPE_POST → 施設看板
                  TYPE_ROAD → 道路看板
 */
//--------------------------------------------------------------
#define _EASY_BGWIN_MSG( msg_id, bg_type ) \
  _ASM_EASY_BGWIN_MSG msg_id, bg_type

//--------------------------------------------------------------
/**
 * 簡易BGウィンドウメッセージ表示　海底神殿　暗号メッセージ　用
 * @param msg_id 表示するメッセージID
 * @param bg_type 表示するタイプ TYPE_INFO,TYPE_TOWN,TYPE_POST,TYPE_ROAD
                  TYPE_INFO → お得な掲示板
                  TYPE_TOWN → 街看板
                  TYPE_POST → 施設看板
                  TYPE_ROAD → 道路看板
 */
//--------------------------------------------------------------
#define _EASY_BGWIN_SEATEMPLE_MSG( msg_id, bg_type ) \
  _ASM_EASY_BGWIN_SEATEMPLE_MSG msg_id, bg_type

//--------------------------------------------------------------
/**
 * 簡易イベント入手イベント
 *
 * @param itemno        取得するアイテムナンバー
 * @param num           取得するアイテムの数
 * @param flag          アイテム取得したかどうかの判定用フラグ
 * @param before_msg    入手前のメッセージ
 * @param finish_msg    入手後のメッセージ
 * @param after_msg     入手後、再び話かけたときのメッセージ
 *
 */
//--------------------------------------------------------------
#define _EASY_TALK_ITEM_EVENT( itemno, num, flag, before_msg, finish_msg, after_msg) \
    _ASM_EASY_TALK_ITEM_EVENT itemno, num, flag, before_msg, finish_msg, after_msg

//--------------------------------------------------------------
/**
 * イベント入手イベント「▼」待ちあり
 *
 * @param itemno        取得するアイテムナンバー
 * @param num           取得するアイテムの数
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_KEYWAIT(itemno, num)   \
    _ASM_ITEM_EVENT_KEYWAIT itemno, num

//--------------------------------------------------------------
/**
 * イベント入手イベント「▼」待ちなし
 *
 * @param itemno        取得するアイテムナンバー
 * @param num           取得するアイテムの数
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_NOWAIT(itemno, num)   \
    _ASM_ITEM_EVENT_NOWAIT itemno, num

//--------------------------------------------------------------
/**
 * アイテム入手フィールドイベント「▼」待ちなし
 * @param itemno  入手するアイテムのナンバー
 * @param num     アイテムの数
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_FIELD(itemno, num)   \
    _ASM_ITEM_EVENT_FIELD itemno, num

//--------------------------------------------------------------
/**
 * アイテム入手フィールドイベント「▼」待ちあり
 * @param itemno  入手するアイテムのナンバー
 * @param num     アイテムの数
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_FIELD_KEYWAIT(itemno, num)   \
    _ASM_ITEM_EVENT_FIELD_KEYWAIT itemno, num

//--------------------------------------------------------------
/**
 * 隠しアイテム入手フィールドイベント「▼」待ちなし
 *
 * @note
 * 自動生成される隠しアイテムスクリプトでのみ使用します
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_HIDEITEM( itemno, num, flag_no ) \
  _ASM_ITEM_EVENT_HIDEITEM itemno, num, flag_no

//--------------------------------------------------------------
/**
 * @brief イベント入手成功イベント「▼」待ちあり
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_SUCCESS_KEYWAIT( itemno, num ) \
    _ASM_ITEM_EVENT_SUCCESS_KEYWAIT itemno, num

//--------------------------------------------------------------
/**
 * @brief イベント入手成功イベント「▼」待ちなし
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_SUCCESS_NOWAIT( itemno, num ) \
    _ASM_ITEM_EVENT_SUCCESS_NOWAIT itemno, num

//--------------------------------------------------------------
/**
 * 簡易鳴き声メッセージコマンド
 *
 * @param monsno
 * @param msg_id
 */
//--------------------------------------------------------------
#define _EASY_TALK_POKEVOICE( monsno, msg_id ) \
    _ASM_EASY_TALK_POKEVOICE monsno, msg_id

//--------------------------------------------------------------
/**
 * @def _REPORT_EVENT_CALL
 * @brief 簡易イベントコマンド：セーブ呼び出し
 * @param ret_wk セーブ結果を受け取るワーク
 * @return セーブした場合     SCR_REPORT_OK
 *         キャンセルした場合 SCR_REPORT_CANCEL
 */
//--------------------------------------------------------------
#define _REPORT_EVENT_CALL( ret_wk )  \
    _ASM_REPORT_EVENT_CALL  ret_wk

//--------------------------------------------------------------
/**
 * @def _REPORT_EVENT_CALL_WIRELESS
 * @brief 簡易イベントコマンド：セーブ呼び出し( ワイヤレスセーブモードのチェック有 )
 * @param ret_wk
 * @return セーブした場合             SCR_REPORT_OK
 *         キャンセルした場合         SCR_REPORT_CANCEL
 *         セーブしない設定だった場合 SCR_REPORT_OK
 */
//--------------------------------------------------------------
#define _REPORT_EVENT_CALL_WIRELESS( ret_wk )  \
    _ASM_REPORT_EVENT_CALL_WIRELESS  ret_wk

//--------------------------------------------------------------
/**
 * @def _FIELD_COMM_EXIT_EVENT_CALL
 * @brief 簡易イベントコマンド：フィールド通信切断イベント呼び出し
 * @param ret_wk  切断処理の戻り値を取得
 *                SCR_FIELD_COMM_EXIT_OK:切断終了 SCR_FIELD_COMM_EXIT_NG:なんらかのエラーが発生
 */
//--------------------------------------------------------------
#define _FIELD_COMM_EXIT_EVENT_CALL( ret_wk )  \
    _ASM_FIELD_COMM_EXIT_EVENT_CALL  ret_wk

//--------------------------------------------------------------
/**
 * @def _BTL_UTIL_PARTY_REGULATION_NG_MSG_CALL
 * @brief 簡易イベントコマンド：
 *        対戦系受付　パーティがレギュレーションに非適合だった時の一連のメッセージ表示
 *
 * @param regulation  poke_tool/regulation_def.hに定義されたコード
 */
//--------------------------------------------------------------
#define _BTL_UTIL_PARTY_REGULATION_NG_MSG_CALL( regulation )  \
    _ASM_BTL_UTIL_PARTY_REGULATION_NG_MSG_CALL  regulation

//======================================================================
//    観覧車トレーナー対戦関連
//======================================================================
//--------------------------------------------------------------
/**
 * @def   _GET_WHEEL_TRAINER_ENTRY_ID
 * @brief 簡易イベントコマンド：
 *        観覧車トレーナーのID取得(0～7の登録ID tr_idとは別)
 */
//--------------------------------------------------------------
#define _GET_WHEEL_TRAINER_ENTRY_ID( ret_wk )  \
    _ASM_GET_WHEEL_TRAINER_ENTRY_ID ret_wk

//--------------------------------------------------------------
/**
 * @def _WHEEL_TRAINER_MSG
 * @brief 簡易イベントコマンド：
 *        観覧車トレーナーのメッセージ表示
 */
//--------------------------------------------------------------
#define _WHEEL_TRAINER_MSG( msg_id, id, f_2nd )  \
    _ASM_WHEEL_TRAINER_MSG msg_id, id, f_2nd

//--------------------------------------------------------------
/**
 * @def _GET_WHEEL_TRAINER_OBJ_ID
 * @brief 観覧車トレーナーのOBJ_IDを取得
 */
//--------------------------------------------------------------
#define _GET_WHEEL_TRAINER_OBJ_ID( id, ret_wk )  \
    _ASM_GET_WHEEL_TRAINER_OBJ_ID id, ret_wk

  .macro  _ASM_GET_WHEEL_TRAINER_OBJ_ID id, ret_wk
  .short  EV_SEQ_GET_WHEEL_TRAINER_OBJ_ID
  .short  \id
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _GET_WHEEL_TRAINER_TR_ID
 * @brief 観覧車トレーナーのTrIDを取得
 */
//--------------------------------------------------------------
#define _GET_WHEEL_TRAINER_TR_ID( id, ret_wk )  \
    _ASM_GET_WHEEL_TRAINER_TR_ID id, ret_wk

  .macro  _ASM_GET_WHEEL_TRAINER_TR_ID id, ret_wk
  .short  EV_SEQ_GET_WHEEL_TRAINER_TR_ID
  .short  \id
  .short  \ret_wk
  .endm


//======================================================================
//
//
//    カメラ操作関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * カメラ操作開始
 */
//--------------------------------------------------------------
#define _CAMERA_START() _ASM_CAMERA_START
  
  .macro _ASM_CAMERA_START
  .short EV_SEQ_CAMERA_START
  .endm

//--------------------------------------------------------------
/**
 * カメラ操作終了
 */
//--------------------------------------------------------------
#define _CAMERA_END() _ASM_CAMERA_END
  
  .macro _ASM_CAMERA_END
  .short EV_SEQ_CAMERA_END
  .endm  

//--------------------------------------------------------------
/**
 * カメラパージ
 */
//--------------------------------------------------------------
#define _CAMERA_PURGE() _ASM_CAMERA_PURGE
  
  .macro _ASM_CAMERA_PURGE
  .short EV_SEQ_CAMERA_PURGE
  .endm

//--------------------------------------------------------------
/**
 * カメラバインド
 */
//--------------------------------------------------------------
#define _CAMERA_BIND() _ASM_CAMERA_BIND
  
  .macro _ASM_CAMERA_BIND
  .short EV_SEQ_CAMERA_BIND
  .endm

//--------------------------------------------------------------
/**
 * カメラ移動
 * @param pitch     ピッチ
 * @param yaw       ヨー
 * @param dist      距離
 * @param x         注視点Ｘ
 * @param y         注視点Ｙ
 * @param z         注視点Ｚ
 * @param frame     移動フレーム
 */
//--------------------------------------------------------------
#define _CAMERA_MOVE(pitch, yaw, dist, x, y, z, frame) \
    _ASM_CAMERA_MOVE pitch, yaw, dist, x, y, z, frame
  
  .macro _ASM_CAMERA_MOVE pitch, yaw, dist, x, y, z, frame
  .short EV_SEQ_CAMERA_MOVE
  .short \pitch
  .short \yaw
  .long \dist
  .long \x
  .long \y
  .long \z
  .short \frame
  .endm

//--------------------------------------------------------------
/**
 * カメラ移動ＩＤ指定
 * @param id        パラメータID    
 * @param frame     移動フレーム
 */
//--------------------------------------------------------------
#define _CAMERA_MOVE_BY_ID(id, frame) _ASM_CAMERA_MOVE_BY_ID id, frame
  
  .macro _ASM_CAMERA_MOVE_BY_ID id, frame
  .short EV_SEQ_CAMERA_MOVE_BY_ID
  .short \id
  .short \frame
  .endm  

//--------------------------------------------------------------
/**
 * カメラ復帰移動
 * @param frame   移動フレーム
 */
//--------------------------------------------------------------
#define _CAMERA_RECV_MOVE(frame) _ASM_CAMERA_RECV_MOVE frame
  
  .macro _ASM_CAMERA_RECV_MOVE frame
  .short EV_SEQ_CAMERA_RECV_MOVE
  .short \frame
  .endm

//--------------------------------------------------------------
/**
 * デフォルトカメラへの移動
 * @param frame   移動フレーム
 */
//--------------------------------------------------------------
#define _CAMERA_DEF_MOVE(frame) _ASM_CAMERA_DEF_MOVE frame
  
  .macro _ASM_CAMERA_DEF_MOVE frame
  .short EV_SEQ_CAMERA_DEF_MOVE
  .short \frame
  .endm  

//--------------------------------------------------------------
/**
 * カメラ移動待ち
 */
//--------------------------------------------------------------
#define _CAMERA_WAIT_MOVE() _ASM_CAMERA_WAIT_MOVE
  
  .macro _ASM_CAMERA_WAIT_MOVE
  .short EV_SEQ_CAMERA_WAIT_MOVE
  .endm

//--------------------------------------------------------------
/**
 * カメラ揺れ　（サインカーブ揺れ）
 * @param w   横振れ幅
 * @param h   縦揺れ幅
 * @param sync    1周期にかけるシンク数　1以上
 * @param time    振動回数
 * @param sw   横振れ減算幅
 * @param sh   縦揺れ減算幅
 * @param smargine    減算間隔
 * @param stime    幅減算開始回数（振動させてから、何周期目で減算を開始するか　timeよりも少ない数のはず）
 * 
 * @note w,hは同時に値をセットして動かすことができますが、どちらもサインカーブ変化しますので、
 * wに値を入れた場合はh=0
 * hに値を入れた場合はw=0
 *でセットすることが多くなると思います。（つまり、縦横同時揺れはないだろうということです）
 * 
 * @note stimeが0のときは、幅減算処理を行いません 
 */
//--------------------------------------------------------------
#define _SHAKE_CAMERA(w, h, sync, time, sw, sh, smargine, stime) \
          _ASM_SHAKE_CAMERA w, h, sync, time, sw, sh, smargine, stime
  
  .macro _ASM_SHAKE_CAMERA w, h, sync, time, sw, sh, smargine, stime
  .short EV_SEQ_SHAKE_CAMERA
  .short \w
  .short \h
  .short \sync
  .short \time
  .short \sw
  .short \sh
  .short \smargine
  .short \stime

  .endm
  

//======================================================================
//
//
//    侵入関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 侵入コマンド　パレス島の通信相手の動作モデル登録
 */
//--------------------------------------------------------------
#define _INTRUDE_PALACE_MMDL_SETTING() _ASM_INTRUDE_PALACE_MMDL_SETTING

  .macro  _ASM_INTRUDE_PALACE_MMDL_SETTING
  .short  EV_SEQ_INTRUDE_PALACE_MMDL_SETTING
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　今自分がいるパレスエリアが、自分のパレスエリアを基点(0)とした場合、何個目のパレスエリアにいるかを取得する
 * 
 * 通信IDとイコールではなく、オフセットである事に注意
 *   ※0の場合は自分のパレスエリアにいる
 */
//--------------------------------------------------------------
#define _GET_PALACE_AREA_OFFSET( ret_wk ) _ASM_GET_PALACE_AREA_OFFSET ret_wk

  .macro  _ASM_GET_PALACE_AREA_OFFSET  ret_wk
  .short  EV_SEQ_GET_PALACE_AREA_OFFSET
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　パレス島マップセッティング
 */
//--------------------------------------------------------------
#define _INTRUDE_CONNECT_MAP_SETTING() _ASM_INTRUDE_CONNECT_MAP_SETTING

  .macro  _ASM_INTRUDE_CONNECT_MAP_SETTING
  .short  EV_SEQ_INTRUDE_CONNECT_MAP_SETTING
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　モノリス画面呼び出し
 */
//--------------------------------------------------------------
#define _CALL_MONOLITH_PROC() _ASM_CALL_MONOLITH_PROC

  .macro  _ASM_CALL_MONOLITH_PROC
  .short  EV_SEQ_CALL_MONOLITH_PROC
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　ミッション選択候補リスト要求リクエスト
 */
//--------------------------------------------------------------
#define _INTRUDE_MISSION_CHOICE_LIST_REQ(ret_wk) _ASM_INTRUDE_MISSION_CHOICE_LIST_REQ ret_wk

  .macro  _ASM_INTRUDE_MISSION_CHOICE_LIST_REQ    ret_wk
  .short  EV_SEQ_INTRUDE_MISSION_CHOICE_LIST_REQ
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　シンボルポケモンセッティング
 */
//--------------------------------------------------------------
#define _SYMBOL_MAP_POKE_SET() _ASM_SYMBOL_MAP_POKE_SET

  .macro  _ASM_SYMBOL_MAP_POKE_SET
  .short  EV_SEQ_SYMBOL_MAP_POKE_SET
  .endm

//--------------------------------------------------------------
/**
 *  侵入時の現在の変装状況取得
 *  @param ret_wk タイプ格納先ワーク
 */
//--------------------------------------------------------------
#define _GET_DISGUISE_CODE( ret_wk ) \
  _ASM_GET_DISGUISE_CODE  ret_wk

  .macro  _ASM_GET_DISGUISE_CODE  ret_wk
  .short  EV_SEQ_GET_DISGUISE_CODE
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　パレスIN時の変装イベント
 */
//--------------------------------------------------------------
#define _CHECK_EQP_GPOWER( ret_wk ) _ASM_CHECK_EQP_GPOWER ret_wk

  .macro  _ASM_CHECK_EQP_GPOWER     ret_wk
  .short  EV_SEQ_CHECK_EQP_GPOWER
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  ミッションにエントリーしているか取得
 *  @param ret_wk タイプ格納先ワーク
 * 
 * TRUE:エントリーしている　FALSE:エントリーしていない
 */
//--------------------------------------------------------------
#define _GET_MISSION_ENTRY( ret_wk ) \
  _ASM_GET_MISSION_ENTRY  ret_wk

  .macro  _ASM_GET_MISSION_ENTRY  ret_wk
  .short  EV_SEQ_GET_MISSION_ENTRY
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　ミッションエントリー後、開始までの待ちイベント
 */
//--------------------------------------------------------------
#define _MISSION_START_WAIT_EVENT() _ASM_MISSION_START_WAIT_EVENT

  .macro  _ASM_MISSION_START_WAIT_EVENT
  .short  EV_SEQ_MISSION_START_WAIT_EVENT
  .endm

//--------------------------------------------------------------
/**
 * シンボルポケモンバトル
 * @param target_obj  対象となるポケモンのOBJID
 * @param ret_wk      バトル結果(TRUE:捕獲した　FALSE:捕獲しなかった)
 */
//--------------------------------------------------------------
#define _SYMBOL_POKE_BATTLE( target_obj, ret_wk ) \
    _ASM_SYMBOL_POKE_BATTLE target_obj, ret_wk

  .macro  _ASM_SYMBOL_POKE_BATTLE     target_obj, ret_wk
  .short  EV_SEQ_SYMBOL_POKE_BATTLE
  .short  \target_obj
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * @def _SYMBOL_MAP_MOVE_POKE
 * @brief シンボルマップ：ポケモンの移動
 * @param obj_id  対象となるポケモンのOBJID
 * @param ret_wk  移動できたかどうか？
 *
 * 引数にはTARGET_OBJIDを指定すること
 */
//--------------------------------------------------------------
#define _SYMBOL_MAP_MOVE_POKE( obj_id, ret_wk ) \
    _ASM_SYMBOL_MAP_MOVE_POKE obj_id, ret_wk

    .macro  _ASM_SYMBOL_MAP_MOVE_POKE obj_id, ret_wk
    .short  EV_SEQ_SYMBOL_MAP_MOVE_POKE
    .short  \obj_id
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SYMBOL_MAP_GET_INFO
 * @brief シンボルマップ：情報の問い合せ
 * @param mode  SCR_SYMMAP_INFO_～（script_def.h参照）
 * @param ret_wk  戻り値
 */
//--------------------------------------------------------------
#define _SYMBOL_MAP_GET_INFO( mode, ret_wk )  \
    _ASM_SYMBOL_MAP_GET_INFO mode, ret_wk

    .macro  _ASM_SYMBOL_MAP_GET_INFO mode, ret_wk
    .short  EV_SEQ_SYMBOL_MAP_GET_INFO
    .short  \mode
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def _SYMBOL_MAP_WARP
 * @brief シンボルマップ：出入口処理
 * @param dir   進行方向
 */
//--------------------------------------------------------------
#define _SYMBOL_MAP_WARP( dir ) \
    _ASM_SYMBOL_MAP_WARP dir

    .macro  _ASM_SYMBOL_MAP_WARP dir
    .short  EV_SEQ_SYMBOL_MAP_WARP
    .short  \dir
    .endm

//--------------------------------------------------------------
/**
 * @def _SYMBOL_MAP_SET_MONS_NAME
 * @brief シンボルマップ：対象ポケモン名をバッファにセット
 * @param obj_id  対象となるポケモンのOBJID
 * @param pos   セットするバッファ位置
 */
//--------------------------------------------------------------
#define _SYMBOL_MAP_SET_MONS_NAME( obj_id, pos )  \
    _ASM_SYMBOL_MAP_SET_MONS_NAME obj_id, pos

    .macro  _ASM_SYMBOL_MAP_SET_MONS_NAME obj_id, pos
    .short  EV_SEQ_SYMBOL_MAP_SET_MONS_NAME
    .short  \obj_id
    .short  \pos
    .endm

//--------------------------------------------------------------
/**
 * @def _SYMBOL_POKE_GET
 * @brief シンボルエンカウント：ポケモンを捕まえる
 * @param obj_id  対象となるポケモンのOBJID
 */
//--------------------------------------------------------------
#define _SYMBOL_POKE_GET( obj_id )  \
      _ASM_SYMBOL_POKE_GET obj_id

      .macro  _ASM_SYMBOL_POKE_GET obj_id
      .short  EV_SEQ_SYMBOL_POKE_GET
      .short  \obj_id
      .endm

//======================================================================
//
//
//  カットイン関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドカットイン
 * @param no      カットインナンバー    fldci_id_def.h参照
 */
//--------------------------------------------------------------
#define _CALL_FLD3D_CUTIN(no) _ASM_CALL_FLD3D_CUTIN no

  .macro  _ASM_CALL_FLD3D_CUTIN no
  .short  EV_SEQ_CALL_FLD3D_CUTIN
  .short  \no
  .endm

//--------------------------------------------------------------
/**
 * ポケモンカットイン
 * @param pos         手持ち位置
 */
//--------------------------------------------------------------
#define _CALL_POKE_CUTIN(pos) _ASM_CALL_POKE_CUTIN pos

  .macro  _ASM_CALL_POKE_CUTIN pos
  .short  EV_SEQ_CALL_POKE_CUTIN
  .short  \pos
  .endm

//--------------------------------------------------------------
/**
 * NPCそらをとぶカットイン
 * @param obj_id         対象OBJID
 */
//--------------------------------------------------------------
#define _CALL_NPCFLY_CUTIN(obj_id) _ASM_CALL_NPCFLY_CUTIN obj_id

  .macro  _ASM_CALL_NPCFLY_CUTIN obj_id
  .short  EV_SEQ_CALL_NPCFLY_CUTIN
  .short  \obj_id
  .endm

//--------------------------------------------------------------
/**
 * アイテムカットイン
 * @param item_no        対象アイテムナンバー
 */
//--------------------------------------------------------------
#define _CALL_ITEM_CUTIN(item_no) _ASM_CALL_ITEM_CUTIN item_no

  .macro  _ASM_CALL_ITEM_CUTIN item_no
  .short  EV_SEQ_CALL_ITEM_CUTIN
  .short  \item_no
  .endm  
  
  
//======================================================================
// 四天王関連
//======================================================================
//--------------------------------------------------------------
/**
 * @def _FOURKINGS_WALKEVENT
 * @brief 四天王　歩きアニメーションイベント呼び出し
 * @param fourking_id 1～4  ゾーンIDの数値　ZONE_ID_C09R[fourking_id]01
 */
//--------------------------------------------------------------
#define _FOURKINGS_WALKEVENT( fourking_id ) _ASM_FOURKINGS_WALKEVENT fourking_id

  .macro  _ASM_FOURKINGS_WALKEVENT fourking_id
  .short  EV_SEQ_FOURKINGS_WALKEVENT
  .short  \fourking_id
  .endm

//--------------------------------------------------------------
/**
 * @def _FOURKINGS_SET_TOP_CAMERA
 * @brief 四天王　頂上のカメラ指定
 * @param fourking_id 1～4  ゾーンIDの数値　ZONE_ID_C09R[fourking_id]01
 */
//--------------------------------------------------------------
#define _FOURKINGS_SET_TOP_CAMERA( fourking_id ) _ASM_FOURKINGS_SET_TOP_CAMERA fourking_id

  .macro  _ASM_FOURKINGS_SET_TOP_CAMERA fourking_id
  .short  EV_SEQ_FOURKINGS_SET_TOP_CAMERA
  .short  \fourking_id
  .endm

//--------------------------------------------------------------
/**
 * @def _FOURKINGS_SET_SOUND_SYSTEM
 * @brief 四天王　部屋　音システム設定
 * @param fourking_id 1～4  ゾーンIDの数値　ZONE_ID_C09R[fourking_id]01
 */
//--------------------------------------------------------------
#define _FOURKINGS_SET_SOUND_SYSTEM( fourking_id ) _ASM_FOURKINGS_SET_SOUND_SYSTEM fourking_id

  .macro  _ASM_FOURKINGS_SET_SOUND_SYSTEM fourking_id
  .short  EV_SEQ_FOURKINGS_SET_SOUND_SYSTEM
  .short  \fourking_id
  .endm



//======================================================================
//
//
//  遊覧船　関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 遊覧船モード開始
 */
//--------------------------------------------------------------
#define _PL_BOAT_START() _ASM_PL_BOAT_START

  .macro  _ASM_PL_BOAT_START
  .short  EV_SEQ_PL_BOAT_START
  .endm

//--------------------------------------------------------------
/**
 * 遊覧船モード終了
 */
//--------------------------------------------------------------
#define _PL_BOAT_END() _ASM_PL_BOAT_END

  .macro  _ASM_PL_BOAT_END
  .short  EV_SEQ_PL_BOAT_END
  .endm

//--------------------------------------------------------------
/**
 * 遊覧船内トレーナ数取得
 * @param type　    検索タイプ  PL_TR_SEARCH_TYPE_～　pl_boat_def.h 参照
 * @param ret_wk    検索結トレーナー数
 */
//--------------------------------------------------------------
#define _PL_BOAT_GET_TR_NUM(type,ret_wk) _ASM_PL_BOAT_GET_TR_NUM type, ret_wk

  .macro  _ASM_PL_BOAT_GET_TR_NUM type, ret_wk
  .short  EV_SEQ_PL_BOAT_GET_TR_NUM
  .short  \type
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 遊覧船内時間更新
 * @param add_time    加算時間（秒）
 * @param whistle     加算結果後、汽笛がなる時間の場合、汽笛を鳴らすかどうか　0：鳴らさない　1：鳴らす
 */
//--------------------------------------------------------------
#define _PL_BOAT_ADD_TIME(add_time, whistle) _ASM_PL_BOAT_ADD_TIME add_time, whistle

  .macro  _ASM_PL_BOAT_ADD_TIME add_time, whistle
  .short  EV_SEQ_PL_BOAT_ADD_TIME
  .short  \add_time
  .short  \whistle
  .endm

//--------------------------------------------------------------
/**
 * 遊覧船ゲーム終了
 */
//--------------------------------------------------------------
#define _PL_BOAT_GAME_END() _ASM_PL_BOAT_GAME_END

  .macro  _ASM_PL_BOAT_GAME_END
  .short  EV_SEQ_PL_BOAT_GAME_END
  .endm  


//--------------------------------------------------------------
/**
 * 遊覧船部屋内情報取得
 * @param room_idx    部屋インデックス
 * @param info_kind   取得情報
 * @param param       情報取得パラメータ
 * @param ret_wk      格納バッファ
 */
//--------------------------------------------------------------
#define _PL_BOAT_GET_ROOM_INFO(room_idx,info_kind,param,ret_wk) \
    _ASM_PL_BOAT_GET_ROOM_INFO room_idx,info_kind,param,ret_wk

  .macro  _ASM_PL_BOAT_GET_ROOM_INFO room_idx,info_kind,param,ret_wk
  .short  EV_SEQ_PL_BOAT_GET_ROOM_INFO
  .short  \room_idx
  .short  \info_kind
  .short  \param
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * 遊覧船部屋内情報セット
 * @param room_idx    部屋インデックス
 * @param info_kind   取得情報
 * @param param       情報取得パラメータ
 * @param set_val      セット情報
 */
//--------------------------------------------------------------
#define _PL_BOAT_SET_ROOM_INFO(room_idx,info_kind,param,set_val) \
    _ASM_PL_BOAT_SET_ROOM_INFO room_idx,info_kind,param,set_val

  .macro  _ASM_PL_BOAT_SET_ROOM_INFO room_idx,info_kind,param,set_val
  .short  EV_SEQ_PL_BOAT_SET_ROOM_INFO
  .short  \room_idx
  .short  \info_kind
  .short  \param
  .short  \set_val
  .endm

//--------------------------------------------------------------
/**
 * 遊覧船甲板デモコール
 * @param ret_wk
 */
//--------------------------------------------------------------
#define _PL_BOAT_CALL_DEMO(ret_wk) _ASM_PL_BOAT_CALL_DEMO ret_wk

  .macro  _ASM_PL_BOAT_CALL_DEMO ret_wk
  .short  EV_SEQ_PL_BOAT_CALL_DEMO
  .short  \ret_wk
  .endm

//======================================================================
//  3Dデモ
//======================================================================
//--------------------------------------------------------------
/**
 * @def _CALL_3D_DEMO
 * @brief デモ呼び出し
 * @param demo_no   デモID指定ナンバー pokemon_wb\prog\include\demo\demo3d_demoid.h 参照
 *
 * @note  フェード処理は別途呼び出す必要があります。
 *        必ず_DEMO_FADEIN(OUT)_系コマンドを使用してください
 */
//--------------------------------------------------------------
#define _CALL_3D_DEMO( demo_no )  \
    _ASM_CALL_3D_DEMO demo_no, 0

#define _CALL_3D_DEMO_EX( demo_no, scene_id )  \
    _ASM_CALL_3D_DEMO demo_no, scene_id

  .macro  _ASM_DEMO_SCENE demo_no, scene_id
  .short  EV_SEQ_DEMO_SCENE
  .short  \demo_no
  .short  \scene_id
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップフェードアウト呼び出し
 *
 *  @note _DEMO_FADEIN_BLACK(WHITE)_CALL()と必ず対で使います
 *
 *  @note _MAP_FADE_系と異なり、_MAP_FADE_END_CHECKで待つ必要はありません
 *  @note _CALL_3D_DEMO, _CALL_3D_DEMO_EX とセットで使います
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_BLACK_CALL() _ASM_DEMO_FADEOUT_BLACK_CALL
#define _DEMO_FADEOUT_WHITE_CALL() _ASM_DEMO_FADEOUT_WHITE_CALL

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップフェードイン呼び出し
 *
 *  @note _DEMO_FADEOUT_BLACK(WHITE)_CALL()と必ず対で使います
 *
 *  @note _MAP_FADE_系と異なり、_MAP_FADE_END_CHECKで待つ必要はありません
 *  @note _CALL_3D_DEMO, _CALL_3D_DEMO_EX とセットで使います
 */
//--------------------------------------------------------------
#define _DEMO_FADEIN_BLACK_CALL() _ASM_DEMO_FADEIN_BLACK_CALL
#define _DEMO_FADEIN_WHITE_CALL() _ASM_DEMO_FADEIN_WHITE_CALL

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップフェードアウト呼び出し BGM Push付き
 *
 *  @note _DEMO_FADEIN_BLACK(WHITE)_WITH_BGM_CALL()と必ず対で使います
 *
 *  @note _MAP_FADE_系と異なり、_MAP_FADE_END_CHECKで待つ必要はありません
 *  @note _CALL_3D_DEMO, _CALL_3D_DEMO_EX とセットで使います
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_BLACK_WITH_BGM_CALL() _ASM_DEMO_FADEOUT_BLACK_WITH_BGM_CALL
#define _DEMO_FADEOUT_WHITE_WITH_BGM_CALL() _ASM_DEMO_FADEOUT_WHITE_WITH_BGM_CALL

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップフェードイン呼び出し　BGM Pop付き
 *
 *  @note _DEMO_FADEOUT_BLACK(WHITE)_WITH_BGM_CALL()と必ず対で使います
 *
 *  @note _MAP_FADE_系と異なり、_MAP_FADE_END_CHECKで待つ必要はありません
 *  @note _CALL_3D_DEMO, _CALL_3D_DEMO_EX とセットで使います
 */
//--------------------------------------------------------------
#define _DEMO_FADEIN_BLACK_WITH_BGM_CALL() _ASM_DEMO_FADEIN_BLACK_WITH_BGM_CALL
#define _DEMO_FADEIN_WHITE_WITH_BGM_CALL() _ASM_DEMO_FADEIN_WHITE_WITH_BGM_CALL

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップフェードアウト単独呼び出し
 *
 *  @note デモ呼出し後そのままマップチェンジがあり、対コマンドが使えない場合に呼びます
 *        対コマンドでないので、BGM Push/Popはできません
 *
 *  @note _MAP_FADE_系と異なり、_MAP_FADE_END_CHECKで待つ必要はありません
 *  @note _CALL_3D_DEMO, _CALL_3D_DEMO_EX とセットで使います
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_BLACK_CALL_SINGLE() _ASM_DEMO_FADEOUT_BLACK_CALL
#define _DEMO_FADEOUT_WHITE_CALL_SINGLE() _ASM_DEMO_FADEOUT_WHITE_CALL
 
//======================================================================
//  テレビトランシーバーデモ
//======================================================================
//--------------------------------------------------------------
/**
 * @def _CALL_TVT_DEMO
 * @brief デモ呼び出し
 * @param demo_no   デモID指定ナンバー
 */
//--------------------------------------------------------------
#define _CALL_TVT_DEMO( demo_no )  \
    _ASM_CALL_TVT_DEMO demo_no

  .macro  _ASM_TVT_DEMO demo_no
  .short  EV_SEQ_CALL_TVT_DEMO
  .short  \demo_no
  .endm

//======================================================================
//  移動ポケモン
//======================================================================

//--------------------------------------------------------------
/**
 * @def _ADD_MOVE_POKEMON
 * @brief 移動ポケモン追加
 * @param move_poke    移動ポケモンID
 */
//--------------------------------------------------------------
#define _ADD_MOVE_POKEMON( move_poke )   \
    _ASM_ADD_MOVE_POKEMON move_poke

    .macro  _ASM_ADD_MOVE_POKEMON move_poke
    .short  EV_SEQ_ADD_MOVE_POKEMON
    .short  \move_poke
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_MOVE_POKEMON_STATUS
 * @brief 移動ポケモンステータス取得
 * @param move_poke    移動ポケモンID
 * @param ret_wk      結果を受け取るワーク
 */
//--------------------------------------------------------------
#define _GET_MOVE_POKEMON_STATUS( move_poke, ret_wk )   \
    _ASM_GET_MOVE_POKEMON_STATUS move_poke, ret_wk

    .macro  _ASM_GET_MOVE_POKEMON_STATUS move_poke, ret_wk
    .short  EV_SEQ_GET_MOVE_POKEMON_STATUS
    .short  \move_poke
    .short  \ret_wk
    .endm

//======================================================================
//  交換関連
//======================================================================

//--------------------------------------------------------------
/**
 * @def _FIELD_POKE_TRADE
 * @brief ゲーム内交換を実行する
 * @param trade_no  もらうポケモンを指定 (FLD_TRADE_POKE_xxxx)
 * @param party_pos あげるポケモンを指定 (パーティ内インデックス0～5)
 *
 * ①交換デモの再生
 * ②手持ちポケモンの書き換え
 * ③図鑑登録                 を実行します。
 *
 * trade_no に渡す値は, resource/fld_trade/fld_trade_list.h で定義.
 */
//--------------------------------------------------------------
#define _FIELD_POKE_TRADE( trade_no, party_pos ) \
    _ASM_FIELD_POKE_TRADE trade_no, party_pos

    .macro  _ASM_FIELD_POKE_TRADE trade_no, party_pos
    .short  EV_SEQ_FIELD_POKE_TRADE
    .short  \trade_no
    .short  \party_pos
    .endm

//--------------------------------------------------------------
/**
 * @def _FIELD_POKE_TRADE_CHECK
 * @brief ゲーム内交換が可能かどうかをチェックする
 * @param ret_wk    結果を受け取るワーク
 * @param trade_no  もらうポケモンを指定 (FLD_TRADE_POKE_xxxx)
 * @param party_pos あげるポケモンを指定 (パーティ内インデックス0～5)
 *
 * @retval FLD_TRADE_ENABLE         交換可能
 *         FLD_TRADE_DISABLE_EGG    交換不可能(タマゴ)
 *         FLD_TRADE_DISABLE_MONSNO 交換不可能(モンスター違い)
 *         FLD_TRADE_DISABLE_SEX    交換不可能(性別違い)
 *
 *  戻り値は prog/src/field/script_def.h で定義.
 */
//--------------------------------------------------------------
#define _FIELD_POKE_TRADE_CHECK( ret_wk, trade_no, party_pos ) \
    _ASM_FIELD_POKE_TRADE_CHECK ret_wk, trade_no, party_pos

    .macro  _ASM_FIELD_POKE_TRADE_CHECK ret_wk, trade_no, party_pos
    .short  EV_SEQ_FIELD_POKE_TRADE_CHECK
    .short  \ret_wk
    .short  \trade_no
    .short  \party_pos
    .endm
  
//======================================================================
// 技思い出し関連
//======================================================================
  
//--------------------------------------------------------------
/**
 * @def _CHECK_WAZA_REMAIND
 * @brief 思い出し技の有無をチェックする
 * @param ret_wk 結果を受け取るワーク
 * @param pos    判定するポケモンを手持ち位置で指定
 * @return 指定したポケモンに思い出せる技がある場合 TRUE,
 *         そうでなければ FALSE
 */
//--------------------------------------------------------------
#define _CHECK_WAZA_REMAIND( ret_wk, pos ) \
    _ASM_CHECK_WAZA_REMAIND ret_wk, pos

  .macro _ASM_CHECK_WAZA_REMAIND ret_wk, pos
  .short EV_SEQ_CHECK_WAZA_REMAIND
  .short \ret_wk
  .short \pos
  .endm 

//--------------------------------------------------------------
/**
 * @def _CALL_WAZA_REMAIND_PROC
 * @brief 技思い出しプロセスを呼び出す
 * @param ret_wk 結果の格納先
 * @param pos    対象となるポケモンを手持ち位置で指定
 * @return 技を思い出した場合 SCR_WAZAOSHIE_RET_SET
 *         キャンセルした場合 SCR_WAZAOSHIE_RET_CANCEL
 */
//--------------------------------------------------------------
#define _CALL_WAZA_REMAIND_PROC( ret_wk, pos ) \
    _ASM_CALL_WAZA_REMAIND_PROC ret_wk, pos

  .macro _ASM_CALL_WAZA_REMAIND_PROC ret_wk, pos
  .short EV_SEQ_CALL_WAZA_REMAIND_PROC
  .short \ret_wk
  .short \pos
  .endm

//======================================================================
//  対戦系受付ユーティリティ
//======================================================================
//--------------------------------------------------------------
/**
 * @def _SET_REGULATION_OUT_POKE_NAME
 * @brief デモ呼び出し
 * @param demo_no   デモID指定ナンバー
 */
//--------------------------------------------------------------
#define _SET_REGULATION_OUT_POKE_NAME( regulation, ret_wk )  \
    _ASM_SET_REGULATION_OUT_POKE_NAME regulation, ret_wk

  .macro  _ASM_SET_REGULATION_OUT_POKE_NAME regulation, ret_wk
  .short  EV_SEQ_BTL_UTIL_SET_REGULATION_OUT_POKE_NAME
  .short  \regulation
  .short  \ret_wk
  .endm

//======================================================================
//  ポケモンリーグ フロント
//======================================================================
//--------------------------------------------------------------
/**
 * @def _LEAGUE_FRONT_LIFT_DOWN
 * @brief デモ呼び出し
 * @param demo_no   デモID指定ナンバー
 */
//--------------------------------------------------------------
#define _LEAGUE_FRONT_LIFT_DOWN() \
    _ASM_LEAGUE_FRONT_LIFT_DOWN

  .macro  _ASM_LEAGUE_FRONT_LIFT_DOWN
  .short  EV_SEQ_LEAGUE_FRONT_LIFT_DOWN
  .endm

//======================================================================
//  バトルサブウェイ
//======================================================================
//--------------------------------------------------------------
/**
 * @def _BSUBWAY_WORK_CLEAR
 * @brief バトルサブウェイ　ワーククリア
 * @param none
 */
//--------------------------------------------------------------
#define _BSUBWAY_WORK_CLEAR() \
    _ASM_BSUBWAY_WORK_CLEAR

  .macro  _ASM_BSUBWAY_WORK_CLEAR
  .short  EV_SEQ_BSUBWAY_WORK_CLEAR
  .endm

//--------------------------------------------------------------
/**
 * @def _BSUBWAY_WORK_CREATE
 * @brief バトルサブウェイ　ワーク作成
 * @param init 初期化種類 BSWAY_PLAY_NEW等
 * @param mode バトルモード BSWAY_MODE_SINGLE等
 */
//--------------------------------------------------------------
#define _BSUBWAY_WORK_CREATE( init, mode ) \
    _ASM_BSUBWAY_WORK_CREATE init, mode

  .macro  _ASM_BSUBWAY_WORK_CREATE init, mode
  .short  EV_SEQ_BSUBWAY_WORK_CREATE
  .short  \init
  .short  \mode
  .endm
 
//--------------------------------------------------------------
/**
 * @def _BSUBWAY_WORK_RELEASE
 * @brief バトルサブウェイ　ワーク開放
 * @param none
 */
//--------------------------------------------------------------
#define _BSUBWAY_WORK_RELEASE() \
    _ASM_BSUBWAY_WORK_RELEASE

  .macro  _ASM_BSUBWAY_WORK_RELEASE
  .short  EV_SEQ_BSUBWAY_WORK_RELEASE
  .endm
 
//--------------------------------------------------------------
/**
 * @def _BSUBWAY_TOOL
 * @brief バトルサブウェイ　コマンドツール呼び出し
 * @param cmd 呼び出すコマンド　BSWAY_TOOL_CHK_ENTRY_POKE_NUM等
 * @param param コマンドに渡すパラメータ
 * @param ret_wk 結果格納先
 */
//--------------------------------------------------------------
#define _BSUBWAY_TOOL( cmd, param0, param1, ret_wk ) \
    _ASM_BSUBWAY_TOOL cmd, param0, param1, ret_wk

  .macro  _ASM_BSUBWAY_TOOL cmd, param0, param1, ret_wk
  .short  EV_SEQ_BSUBWAY_TOOL
  .short \cmd
  .short \param0
  .short \param1
  .short \ret_wk
  .endm


//======================================================================
//
//  移動ポケモン　イベント
//
//======================================================================
//--------------------------------------------------------------
/**
 * @def _EV_MOVEPOKE_CREATE
 * @brief 移動ポケモン  管理ワーク　生成
 * @param id      識別ID
 */
//--------------------------------------------------------------
#define _EV_MOVEPOKE_CREATE( id ) \
    _ASM_EV_MOVEPOKE_CREATE id

  .macro  _ASM_EV_MOVEPOKE_CREATE id
  .short  EV_SEQ_EV_MOVEPOKE_CREATE
  .short \id
  .endm


//--------------------------------------------------------------
/**
 * @def _EV_MOVEPOKE_DELETE
 * @brief 移動ポケモン  管理ワーク　破棄
 */
//--------------------------------------------------------------
#define _EV_MOVEPOKE_DELETE() \
    _ASM_EV_MOVEPOKE_DELETE

  .macro  _ASM_EV_MOVEPOKE_DELETE
  .short  EV_SEQ_EV_MOVEPOKE_DELETE
  .endm

//--------------------------------------------------------------
/**
 * @def _EV_MOVEPOKE_START_ANIME
 * @brief 移動ポケモン  管理ワーク　アニメーション開始
 * @param anime_index アニメーションナンバー
#define SCR_EV_MOVEPOKE_ANIME_RAI_KAZA_INSIDE (0)   ライカミ　カザカミ　登場
#define SCR_EV_MOVEPOKE_ANIME_RAI_KAZA_OUTSIDE (1)  ライカミ　カザカミ　退場
#define SCR_EV_MOVEPOKE_ANIME_TUCHI_INSIDE  (2)     ツチノカミ　登場１
#define SCR_EV_MOVEPOKE_ANIME_TUCHI_OUTSIDE (3)     ツチノカミ  登場２
 */
//--------------------------------------------------------------
#define _EV_MOVEPOKE_START_ANIME( anime_index ) \
    _ASM_EV_MOVEPOKE_START_ANIME anime_index

  .macro  _ASM_EV_MOVEPOKE_START_ANIME anime_index
  .short  EV_SEQ_EV_MOVEPOKE_START_ANIME
  .short  \anime_index
  .endm

//--------------------------------------------------------------
/**
 * @def _EV_MOVEPOKE_CHECK_ANIME_END
 * @brief 移動ポケモン  管理ワーク　アニメーション終了待ち
 */
//--------------------------------------------------------------
#define _EV_MOVEPOKE_CHECK_ANIME_END() \
    _ASM_EV_MOVEPOKE_CHECK_ANIME_END

  .macro  _ASM_EV_MOVEPOKE_CHECK_ANIME_END
  .short  EV_SEQ_EV_MOVEPOKE_CHECK_ANIME_END
  .endm

 
//======================================================================
//
//  パルパーク関連
//
//======================================================================
//--------------------------------------------------------------
/**
 * @def _PALPARK_CALL
 * @brief パルパーク呼び出し
 * @param none
 */
//--------------------------------------------------------------
#define _PALPARK_CALL() \
    _ASM_PALPARK_CALL

  .macro  _ASM_PALPARK_CALL
  .short  EV_SEQ_CALL_PALPARK
  .endm

//--------------------------------------------------------------
/**
 *  _GET_PALPARK_VALUE 
 *  @brief パルパーク：数値取得(汎用
 *      基本的に下にあるラッパーを呼んでください
 *  @param type 種類(前回ゲームステート・ハイスコア
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_PALPARK_VALUE( type, retVal ) \
    _ASM_GET_PALPARK_VALUE type, retVal

  .macro  _ASM_GET_PALPARK_VALUE type, retVal
  .short EV_SEQ_GET_PALPARK_VALUE
  .byte \type
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_PALPARK_VALUE_FINISH_STATE 
 *  @brief パルパーク：前回終了状態(0:捕獲あり 1:捕獲あり+ハイスコア 2:捕獲無し 3:エラー
 *         palpark_scr_local.h参照
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_PALPARK_VALUE_FINISH_STATE( retVal ) \
    _ASM_GET_PALPARK_VALUE_FINISH_STATE retVal

  .macro  _ASM_GET_PALPARK_VALUE_FINISH_STATE retVal
  .short EV_SEQ_GET_PALPARK_VALUE
  .byte 0
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _GET_PALPARK_VALUE_HIGHSCORE
 *  @brief パルパーク：ハイスコア
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _GET_PALPARK_VALUE_HIGHSCORE( retVal ) \
    _ASM_GET_PALPARK_VALUE_HIGHSCORE retVal

  .macro  _ASM_GET_PALPARK_VALUE_HIGHSCORE retVal
  .short EV_SEQ_GET_PALPARK_VALUE
  .byte 1
  .short \retVal
  .endm

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @def  _POSTMAN_REQUEST
 * @brief 不思議な贈り物配達員用コマンド
 * @param req     リクエストID
 * @param ret_wk  戻り値
 *
 * @note
 * 不思議なおくりもの専用です。通常は使用しないでください。
 */
//--------------------------------------------------------------
#define _POSTMAN_REQUEST( req, ret_wk ) \
    _ASM_POSTMAN_REQUEST req, ret_wk

    .macro  _ASM_POSTMAN_REQUEST req, ret_wk
    .short  EV_SEQ_POSTMAN_REQUEST
    .short  \req
    .short  \ret_wk
    .endm

//--------------------------------------------------------------
/**
 * @def  _PDW_POSTMAN_REQUEST
 * @brief PDW配達員用コマンド
 * @param req     リクエストID
 * @param ret_wk  戻り値
 *
 * @note
 * パレスに配置されるPDW配達員専用です。通常は使用しないでください。
 */
//--------------------------------------------------------------
#define _PDW_POSTMAN_REQUEST( req, ret_wk ) \
    _ASM_PDW_POSTMAN_REQUEST req, ret_wk

    .macro  _ASM_PDW_POSTMAN_REQUEST req, ret_wk
    .short  EV_SEQ_PDW_POSTMAN_REQUEST
    .short  \req
    .short  \ret_wk
    .endm

//======================================================================
//
//    WFBC関連
//
//======================================================================
//--------------------------------------------------------------
/**
 *  _WFBC_TALKSTART 
 *  @brief WFBC:会話開始
 */
//--------------------------------------------------------------
#define _WFBC_TALKSTART() \
    _ASM_WFBC_TALKSTART

  .macro  _ASM_WFBC_TALKSTART
  .short EV_SEQ_WFBC_TALKSTART
  .endm


//--------------------------------------------------------------
/**
 *  _WFBC_TALKEND
 *  @brief WFBC:会話終了
 */
//--------------------------------------------------------------
#define _WFBC_TALKEND() \
    _ASM_WFBC_TALKEND

  .macro  _ASM_WFBC_TALKEND
  .short EV_SEQ_WFBC_TALKEND
  .endm


//--------------------------------------------------------------
/**
 *  _WFBC_GET_DATA
 *  @brief WFBC:各種情報の取得
 *
 *  @param data_type  取得したい情報のタイプ　prog/src/field/scrcmd_wfbc_define.h
 *  @param ret_val    戻り値  データ
 *
 *  data_type
      WFBC_GET_PARAM_BATTLE_TRAINER_ID (0)    // バトルトレーナーID
      WFBC_GET_PARAM_IS_TAKES_ID (1)          // 街にいきたがるか
      WFBC_GET_PARAM_IS_RIREKI (2)          // 前にいたBCの街の履歴があるか
      WFBC_GET_PARAM_PEOPLE_NUM (3)         // 人口の取得 **zone_chnageのタイミングで呼んでOK**
      WFBC_GET_PARAM_BC_NPC_WIN_NUM (4)     // BC　NPC勝利数
      WFBC_GET_PARAM_BC_NPC_WIN_TARGET (5)  // BC　NPC勝利目標数
      WFBC_GET_PARAM_WF_IS_POKECATCH (6)  // WF　村長イベント　ポケモンはいるか？
      WFBC_GET_PARAM_WF_ITEM  (7)         // WF 村長イベント　ご褒美アイテムの取得
      WFBC_GET_PARAM_WF_POKE  (8)         // WF 村長イベント　探すポケモンナンバーの取得
      WFBC_GET_PARAM_OBJ_ID  (9)         // 話し相手のOBJID　（0～29）（WFBCで自動配置される30人の人を識別するID）
      WFBC_GET_PARAM_BC_CHECK_BATTLE (10)  // BC　相手とバトル可能かチェック
      WFBC_GET_PARAM_MMDL_ID  (11)         // 話し相手のMMDL OBJID (フィールド上の動作モデルを識別するID)
 */
//--------------------------------------------------------------
#define _WFBC_GET_DATA( data_type, retVal ) \
    _ASM_WFBC_GET_DATA data_type, retVal

  .macro  _ASM_WFBC_GET_DATA data_type, retVal
  .short EV_SEQ_WFBC_GET_DATA
  .short \data_type
  .short \retVal
  .endm

//--------------------------------------------------------------
/**
 *  _WFBC_SET_WORDSET_RIREKI_PLAYER_NAME
 *  @brief WFBC:履歴に入っている人の名前をワードセットに設定
 *
 *  @param idx  格納するワードセットのインデックス
 */
//--------------------------------------------------------------
#define _WFBC_SET_WORDSET_RIREKI_PLAYER_NAME( idx ) \
    _ASM_WFBC_SET_WORDSET_RIREKI_PLAYER_NAME idx

  .macro  _ASM_WFBC_SET_WORDSET_RIREKI_PLAYER_NAME idx
  .short EV_SEQ_WFBC_SET_WORDSET_RIREKI_PLAYER_NAME
  .short \idx
  .endm

//--------------------------------------------------------------
/**
 *  _WFBC_CHECK_WF_TARGETPOKE
 *  @brief WFBC:WF 目標ポケモンがいるのか？チェック
 *  @param  ret_if1 種族NOが一致,卵じゃない、生息地がWF なら TRUE
 *  @param  ret_if2 捕まえた日が今日  なら　TRUE
 *  @param  ret_temoti_pos  ヒットしたポケモンの手持ち位置
 */
//--------------------------------------------------------------
#define _WFBC_CHECK_WF_TARGETPOKE( ret_if1, ret_if2, ret_temoti_pos ) \
    _ASM_WFBC_CHECK_WF_TARGETPOKE ret_if1, ret_if2, ret_temoti_pos

  .macro  _ASM_WFBC_CHECK_WF_TARGETPOKE ret_if1, ret_if2, ret_temoti_pos
  .short EV_SEQ_WFBC_CHECK_WF_TARGETPOKE
  .short \ret_if1
  .short \ret_if2
  .short \ret_temoti_pos
  .endm

//--------------------------------------------------------------
/**
 *  _WFBC_ADD_BC_NPC_WIN_TARGET
 *  @brief WFBC:BC NPC勝利数を加算
 */
//--------------------------------------------------------------
#define _WFBC_ADD_BC_NPC_WIN_TARGET() \
    _ASM_WFBC_ADD_BC_NPC_WIN_TARGET

  .macro  _ASM_WFBC_ADD_BC_NPC_WIN_TARGET
  .short EV_SEQ_WFBC_ADD_BC_NPC_WIN_TARGET
  .endm

//--------------------------------------------------------------
/**
 *  _WFBC_PALACE_TALK
 *  @brief WFBC:会話開始
 */
//--------------------------------------------------------------
#define _WFBC_PALACE_TALK() \
    _ASM_WFBC_PALACE_TALK

  .macro  _ASM_WFBC_PALACE_TALK
  .short EV_SEQ_WFBC_PALACE_TALK
  .endm


//--------------------------------------------------------------
/**
 *  _WFBC_GET_AUTO_NPC_MESSAGE
 *  @brief WFBC:自動配置人物のメッセージを取得する
 *  
 *  @param  ret     戻り値
 *  @param  msg_idx メッセージのインデックス(1～4)
 */
//--------------------------------------------------------------
#define _WFBC_GET_AUTO_NPC_MESSAGE( ret, msg_idx ) \
    _ASM_WFBC_GET_AUTO_NPC_MESSAGE ret, msg_idx

  .macro  _ASM_WFBC_GET_AUTO_NPC_MESSAGE ret, msg_idx
  .short EV_SEQ_WFBC_GET_AUTO_NPC_MESSAGE
  .short \ret
  .short \msg_idx
  .endm

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @def _FEELING_NAME
 * @brief 相性診断イベント：名前をセット
 * @param idx
 */
//--------------------------------------------------------------
#define _FEELING_NAME( idx )  \
    _ASM_FEELING_NAME idx

    .macro  _ASM_FEELING_NAME idx
    .short  EV_SEQ_SET_FEELING_NAME
    .short  \idx
    .endm

//--------------------------------------------------------------
/**
 * @def _GET_FEELING_COUNT
 * @brief 相性診断イベント：相性診断した人数を取得
 * @param ret_wk
 */
//--------------------------------------------------------------
#define _GET_FEELING_COUNT( ret_wk ) \
    _ASM_GET_FEELING_COUNT ret_wk

    .macro  _ASM_GET_FEELING_COUNT ret_wk
    .short  EV_SEQ_GET_FEELING_COUNT
    .short  \ret_wk
    .endm

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
    .macro  _ELEVATOR_LABEL   adrs
  .align  4
  \adrs:
    .endm

//--------------------------------------------------------------
//--------------------------------------------------------------
    .macro  _ELEVATOR_DATA  msgid, zone_id, x, y, z
    .short  \msgid, \zone_id, \x, \y, \z
    .endm

//--------------------------------------------------------------
//--------------------------------------------------------------
    .macro  _ELEVATOR_DATA_END
    .short  0xffff
    .endm

//--------------------------------------------------------------
/**
 * @def _ELEVATOR_ENTRY_LIST
 * @brief エレベーターデータの登録
 * @param list_label  エレベーターデータ指定ラベル
 * @note  エレベータースクリプト専用コマンド。通常は使わないでください。
 */
//--------------------------------------------------------------
#define _ELEVATOR_ENTRY_LIST( list_label ) \
    _ASM_ELEVATOR_ENTRY_LIST list_label 

    .macro  _ASM_ELEVATOR_ENTRY_LIST list_label 
    .short  EV_SEQ_ELEVATOR_ENTRY_LIST
    .long  ((\list_label-.)-4)
    .endm

//--------------------------------------------------------------
/**
 * @def _ELEVATOR_MAKE_LIST
 * @brief エレベーターフロア選択リスト追加処理
 * @note  エレベータースクリプト専用コマンド。通常は使わないでください。
 */
//--------------------------------------------------------------
#define _ELEVATOR_MAKE_LIST( ) \
    _ASM_ELEVATOR_MAKE_LIST 

    .macro  _ASM_ELEVATOR_MAKE_LIST list_label
    .short  EV_SEQ_ELEVATOR_MAKE_LIST
    .endm

//--------------------------------------------------------------
/**
 * @def _ELEVATOR_MAP_CHANGE
 * @brief エレベーター用マップ遷移処理
 * @param select_id   選択位置
 * @note  エレベータースクリプト専用コマンド。通常は使わないでください。
 */
//--------------------------------------------------------------
#define _ELEVATOR_MAP_CHANGE( select_id ) \
    _ASM_ELEVATOR_MAP_CHANGE select_id

    .macro  _ASM_ELEVATOR_MAP_CHANGE select_id
    .short  EV_SEQ_ELEVATOR_MAP_CHANGE
    .short  \select_id
    .endm

//======================================================================
//
//  個別ギミック関連
//
//======================================================================

//--------------------------------------------------------------
/**
 * @brief ジェットバッジチェックゲートギミック初期化 
 */
//--------------------------------------------------------------
#define _JET_BADGE_GATE_GIMMICK_INIT( ) \
    _ASM_JET_BADGE_GATE_GIMMICK_INIT

  .macro  _ASM_JET_BADGE_GATE_GIMMICK_INIT
  .short  EV_SEQ_JET_BADGE_GATE_GIMMICK_INIT
  .endm


//--------------------------------------------------------------
/**
 * @brief C03中央部　画面遷移カメラ操作
 * @param id        パラメータID    
 * @param frame     移動フレーム
 */
//--------------------------------------------------------------
#define _C03CENTER_CAMERA_MOVE( id, frame ) \
    _ASM_C03CENTER_CAMERA_MOVE id, frame

  .macro  _ASM_C03CENTER_CAMERA_MOVE id, frame
  .short EV_SEQ_CAMERA_MOVE_BY_ID
  .short \id
  .short \frame
  .short EV_SEQ_C03CENTER_PLAYERWAY
  .short \frame
  .endm
  

//--------------------------------------------------------------
/**
 * @brief C03 群集表現開始
 */
//--------------------------------------------------------------
#define _CROWD_PEOPLE_START() \
    _ASM_CROWD_PEOPLE_START

  .macro  _ASM_CROWD_PEOPLE_START
  .short EV_SEQ_CROWD_PEOPLE_START
  .endm

//--------------------------------------------------------------
/**
 * @brief D20海底表現ON
 */
//--------------------------------------------------------------
#define _SEATEMPLE_START() \
    _ASM_SEATEMPLE_START

  .macro  _ASM_SEATEMPLE_START
  .short EV_SEQ_SEATEMPLE_START
  .endm

//--------------------------------------------------------------
/**
 * @brief 天気変更    通常
 * @param weather_id  WEATHER_NO_RAIKAMI：ライカミ  WEATHER_NO_KAZAKAMI：カザカミ 
 * prog/include/field/weather_no.h　参照
 *
 * ゾーンの基本天気に戻す。
 * SCR_WEATHER_NO_ZONE_DEF
 */
//--------------------------------------------------------------
#define _WEATHER_CHANGE( weather_id ) \
    _ASM_WEATHER_CHANGE weather_id, FALSE

//--------------------------------------------------------------
/**
 * @brief 天気変更  移動ポケモン用
 * @param weather_id  WEATHER_NO_RAIKAMI：ライカミ  WEATHER_NO_KAZAKAMI：カザカミ 
 * prog/include/field/weather_no.h　参照
 * "使用注意！！！！"通常SEとしてSEが再生されます。イベント内で、天気を変え、また元に戻すときにのみ使用可能です。
 */
//--------------------------------------------------------------
#define _WEATHER_CHANGE_MOVEPOKE( weather_id ) \
    _ASM_WEATHER_CHANGE weather_id, TRUE

  .macro  _ASM_WEATHER_CHANGE weather_id, notenvse
  .short EV_SEQ_WEATHER_CHANGE
  .short \weather_id
  .short \notenvse
  .endm


//--------------------------------------------------------------
/**
 * @brief ポケモンID検索　自分のIDと異なるIDの種類数を取得（最大50）
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _SEARCH_POKEID_NUM( ret ) _ASM_SEARCH_POKEID_NUM ret

  .macro  _ASM_SEARCH_POKEID_NUM ret
  .short EV_SEQ_SEARCH_POKEID_NUM
  .short \ret
  .endm

//======================================================================
//
//    バストアップショット関連
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 顔アップ開始
 * @note      引数は、src/field/fld_faceup_def.hを参照のこと
 * @param back_idx    背景指定          FLD_FACEUP_BG～を指定     変数指定不可
 * @param char_idx    キャラクター指定  FLD_FACEUP_CHAR～を指定   変数指定不可
 * @param last        最終使用か　TRUEの場合、フェードインしない  変数指定不可
 */
//--------------------------------------------------------------
#define _START_FACEUP( back_idx, char_idx, last ) _ASM_START_FACEUP back_idx, char_idx, last

  .macro  _ASM_START_FACEUP back_idx, char_idx, last
  .short EV_SEQ_FACEUP_SATRT
  .short \back_idx
  .short \char_idx
  .short \last
  .endm

//--------------------------------------------------------------
/**
 * @brief 顔アップ表示時のアニメ開始
 */
//--------------------------------------------------------------
#define _START_FACEUP_ANM() _ASM_START_FACEUP_ANM

  .macro  _ASM_START_FACEUP_ANM
  .short EV_SEQ_FACEUP_START_ANM
  .endm
  
//--------------------------------------------------------------
/**
 * @brief 顔アップ終了
 */
//--------------------------------------------------------------
#define _END_FACEUP() _ASM_END_FACEUP

  .macro  _ASM_END_FACEUP
  .short EV_SEQ_FACEUP_END
  .endm
  
//======================================================================
//
//    国連関連
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 国連関連 交換した国数を取得
 * @param my_country  自分の国を結果に含めるか？  TRUE:含める　FALSE:含めない
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _UN_GET_COUNTRY_NUM( my_country, ret ) _ASM_UN_GET_COUNTRY_NUM my_country, ret

  .macro  _ASM_UN_GET_COUNTRY_NUM my_country, ret
  .short EV_SEQ_UN_GET_COUNTRY_NUM
  .short \my_country
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 有効データ数
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _UN_GET_VALID_DATA_NUM( ret ) _ASM_UN_GET_VALID_DATA_NUM ret

  .macro  _ASM_UN_GET_VALID_DATA_NUM ret
  .short EV_SEQ_UN_GET_VALID_DATA_NUM
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 会話フラグセット
 * @param obj_idx   OBJインデックス0～4
 */
//--------------------------------------------------------------
#define _UN_SET_TALK_FLG( obj_idx ) _ASM_UN_SET_TALK_FLG obj_idx

  .macro _ASM_UN_SET_TALK_FLG obj_idx
  .short EV_SEQ_UN_SET_TALK_FLG
  .short \obj_idx
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 会話フラグチェック
 * @param obj_idx   OBJインデックス0～4
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _UN_CHK_TALK_FLG( obj_idx, ret ) _ASM_UN_CHK_TALK_FLG obj_idx, ret

  .macro _ASM_UN_CHK_TALK_FLG obj_idx, ret
  .short EV_SEQ_UN_CHK_TALK_FLG
  .short \obj_idx
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 自分の性格をセット
 * @param nature_idx 性格コード
 * @note prog/include/savedata/nature_def.h　参照
 */
//--------------------------------------------------------------
#define _UN_SET_MY_NATURE( nature_idx ) _ASM_UN_SET_MY_NATURE nature_idx

  .macro _ASM_UN_SET_MY_NATURE nature_idx
  .short EV_SEQ_UN_SET_MY_NATURE
  .short \nature_idx
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 自分の趣味をセット
 * @param favorite_idx    趣味コード
 * @note  resource\research_radar\data\hobby_id.h　参照
 */
//--------------------------------------------------------------
#define _UN_SET_MY_FAVORITE( favorite_idx ) _ASM_UN_SET_MY_FAVORITE favorite_idx

  .macro _ASM_UN_SET_MY_FAVORITE favorite_idx
  .short EV_SEQ_UN_SET_MY_FAVORITE
  .short \favorite_idx
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 自分の趣味を取得
 * @param ret       結果
 */
//--------------------------------------------------------------
#define _UN_GET_MY_FAVORITE( ret ) _ASM_UN_GET_MY_FAVORITE ret

  .macro _ASM_UN_GET_MY_FAVORITE ret
  .short EV_SEQ_UN_GET_MY_FAVORITE
  .short \ret
  .endm
  
//--------------------------------------------------------------
/**
 * @brief 国連関連 部屋に行く前の情報セット
 * @param country_code  国コード
 * @param floor         フロア
 */
//--------------------------------------------------------------
#define _UN_SET_COUNTRY_INFO( country_code, floor ) _ASM_UN_SET_COUNTRY_INFO country_code, floor

  .macro _ASM_UN_SET_COUNTRY_INFO country_code, floor
  .short EV_SEQ_UN_SET_COUNTRY_INFO
  .short \country_code
  .short \floor
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 部屋内　OBJコード取得
 * @param idx       部屋の中のOBJインデックス0～4
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _UN_GET_ROOM_OBJ_CODE( idx, ret ) _ASM_UN_GET_ROOM_OBJ_CODE idx, ret

  .macro _ASM_UN_GET_ROOM_OBJ_CODE idx, ret
  .short EV_SEQ_UN_GET_ROOM_OBJ_CODE
  .short \idx
  .short \ret
  .endm 

//--------------------------------------------------------------
/**
 * @brief 国連関連 部屋内情報取得
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _UN_GET_ROOM_INFO( infotype, ret ) _ASM_UN_GET_ROOM_INFO infotype, ret

  .macro _ASM_UN_GET_ROOM_INFO infotype, ret
  .short EV_SEQ_UN_GET_ROOM_INFO
  .short \infotype
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 部屋内　OBJメッセージ取得
 * @param idx       部屋の中のOBJインデックス0～4
 * @param idx       取得したいのは初回メッセージか？  TRUEで初回メッセージ
 * @param ret       検索結果
 */
//--------------------------------------------------------------
#define _UN_GET_ROOM_OBJ_MSG( idx, first, ret ) _ASM_UN_GET_ROOM_OBJ_MSG idx, first, ret

  .macro _ASM_UN_GET_ROOM_OBJ_MSG idx, first, ret
  .short EV_SEQ_UN_GET_ROOM_OBJ_MSG
  .short \idx
  .short \first
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 フロア選択アプリコール
 * @param in_floor    現在フロア    エントランスのときは1以下を指定
 * @param out_floor   移動先フロア格納ワーク
 * @param country　   移動先国コードワーク
 * @param ret         結果　TRUE で移動
 */
//--------------------------------------------------------------
#define _UN_CALL_FLOOR_SEL_APP( in_floor, out_floor, country, ret ) \
    _ASM_UN_CALL_FLOOR_SEL_APP in_floor, out_floor, country, ret

  .macro _ASM_UN_CALL_FLOOR_SEL_APP in_floor, out_floor, country, ret
  .short EV_SEQ_UN_CALL_FLOOR_SEL_APP
  .short \in_floor
  .short \out_floor
  .short \country
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 国連関連 自分が国コードをセットしているかを調べる
 * @param ret       検索結果  TRUE:セットされている   FALSE:セットされていない
 */
//--------------------------------------------------------------
#define _UN_IS_SETTING_MYCOUNTRY( ret ) _ASM_UN_IS_SETTING_MYCOUNTRY ret

  .macro  _ASM_UN_IS_SETTING_MYCOUNTRY ret
  .short EV_SEQ_UN_IS_SETTING_MYCOUNTRY
  .short \ret
  .endm
  
  
//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @def _SXY_MOVE_BG_EVENT
 * @brief BGイベントの位置を変更する
 * @param id  BGイベントを指定するためのID
 * @param gx  X位置（グリッド単位）
 * @param gy  Y位置（グリッド単位）
 * @param gz  Z位置（グリッド単位）
 */
//--------------------------------------------------------------
#define _SXY_MOVE_BG_EVENT( bg_id, gx, gy, gz ) \
    _ASM_SXY_MOVE_BG_EVENT bg_id, gx, gy, gz

    .macro  _ASM_SXY_MOVE_BG_EVENT bg_id, gx, gy, gz
    .short  EV_SEQ_MOVE_BG_EVENT_POS
    .short  \bg_id
    .short  \gx
    .short  \gy
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _SXY_MOVE_EXIT_EVENT
 * @brief 出入口イベントの位置を変更する
 * @param id  出入口イベントを指定するためのID
 * @param gx  X位置（グリッド単位）
 * @param gy  Y位置（グリッド単位）
 * @param gz  Z位置（グリッド単位）
 */
//--------------------------------------------------------------
#define _SXY_MOVE_EXIT_EVENT( exit_id, gx, gy, gz ) \
    _ASM_SXY_MOVE_EXIT_EVENT exit_id, gx, gy, gz

    .macro  _ASM_SXY_MOVE_EXIT_EVENT exit_id, gx, gy, gz
    .short  EV_SEQ_MOVE_EXIT_EVENT_POS
    .short  \exit_id
    .short  \gx
    .short  \gy
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @def _SXY_MOVE_NPC_POS
 * @brief NPCの初期位置を変更する
 * @param npc_id  NPCを指定するためのID
 * @param dir     向きの指定
 * @param gx      X位置（グリッド単位）
 * @param gy      Y位置（グリッド単位）
 * @param gz      Z位置（グリッド単位）
 *
 * _ZONE_CHANGE_LABELのタイミング以外で使用しても、
 * このコマンドで変更したデータが参照されないため意味がない。
 */
//--------------------------------------------------------------
#define _SXY_MOVE_NPC_POS( npc_id, dir, gx, gy, gz ) \
    _ASM_SXY_MOVE_NPC_POS npc_id, dir, gx, gy, gz

    .macro  _ASM_SXY_MOVE_NPC_POS npc_id, dir, gx, gy, gz
    .short  EV_SEQ_MOVE_NPC_DATA_POS
    .short  \npc_id
    .short  \dir
    .short  \gx
    .short  \gy
    .short  \gz
    .endm

//--------------------------------------------------------------
/**
 * @brief 簡易会話アプリコール
 * @param mode
 * @param word1
 * @param word2
 * @param ret       結果　編集をやめた場合FALSE 編集した場合TRUE
 */
//--------------------------------------------------------------
#define _CALL_EASY_TALK_APP( mode, word1, word2, ret ) \
      _ASM_CALL_EASY_TALK_APP mode, word1, word2, ret

  .macro _ASM_CALL_EASY_TALK_APP mode, word1, word2, ret
  .short EV_SEQ_CALL_EASY_TALK_APP
  .short \mode
  .short \word1
  .short \word2
  .short \ret
  .endm

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief トライアルハウス開始
 */
//--------------------------------------------------------------
#define _TH_START() _ASM_TH_START
  
  .macro _ASM_TH_START
  .short EV_SEQ_TH_START
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス終了
 */
//--------------------------------------------------------------
#define _TH_END() _ASM_TH_END
  
  .macro _ASM_TH_END
  .short EV_SEQ_TH_END
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　プレイモードセット（シングルＯＲダブル）
 * @param   mode
 */
//--------------------------------------------------------------
#define _TH_SET_PLAY_MODE( mode ) _ASM_TH_SET_PLAY_MODE mode
  
  .macro _ASM_TH_SET_PLAY_MODE mode
  .short EV_SEQ_TH_SET_PLAY_MODE
  .short \mode
  .endm
  
//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ポケモン選択
 * @param   reg_type      REG_SUBWAY_SINGLE or REG_SUBWAY_DOUBLE
 * @param   party_type    SCR_BTL_PARTY_SELECT_TEMOTI or SCR_BTL_PARTY_SELECT_BTLBOX
 * @param   ret     選択結果  FALSE：キャンセル
 */
//--------------------------------------------------------------
#define _TH_SEL_POKE( reg_type, party_type, ret ) _ASM_TH_SEL_POKE reg_type, party_type, ret
  
  .macro _ASM_TH_SEL_POKE reg_type, party_type, ret
  .short EV_SEQ_TH_SEL_POKE
  .short \reg_type
  .short \party_type
  .short \ret
  .endm
 
//--------------------------------------------------------------
/**
 * @brief トライアルハウス　対戦相手セット
 * @param   round   対戦回数　0～4
 * poaram   obj_id  セットしたトレーナーのＯＢＪＩＤを格納するバッファ
 */
//--------------------------------------------------------------
#define _TH_SET_TR( round, obj_id ) _ASM_TH_SET_TR round, obj_id
  
  .macro _ASM_TH_SET_TR round, obj_id
  .short EV_SEQ_TH_SET_TR
  .short \round
  .short \obj_id
  .endm
  
//--------------------------------------------------------------
/**
 * @brief トライアルハウス　戦闘前メッセージ表示
 * @param   obj_id
 */
//--------------------------------------------------------------
#define _TH_DISP_BEFORE_MSG(obj_id) _ASM_TH_DISP_BEFORE_MSG obj_id
  
  .macro _ASM_TH_DISP_BEFORE_MSG obj_id
  .short EV_SEQ_TH_DISP_BEFORE_MSG
  .short 0
  .short \obj_id
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　戦闘呼び出し
 */
//--------------------------------------------------------------
#define _TH_CALL_BATTLE() _ASM_TH_CALL_BATTLE
  
  .macro _ASM_TH_CALL_BATTLE
  .short EV_SEQ_TH_CALL_BATTLE
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ダウンロードデータで遊ぶか？
 * @param   flg       TRUE: ダウンロードデータ FALSE:ＲＯＭデータ
 */
//--------------------------------------------------------------
#define _TH_SET_DL_FLG( flg ) _ASM_TH_SET_Dl_FLG flg
  
  .macro _ASM_TH_SET_DL_FLG flg
  .short EV_SEQ_TH_SET_DL_FLG
  .short \flg
  .endm
  
//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ランクセット
 * @param  rank  セットするランク
 */
//--------------------------------------------------------------
#define _TH_SET_RANK( rank ) _ASM_TH_SET_RANK rank
  
  .macro _ASM_TH_SET_RANK rank
  .short EV_SEQ_TH_SET_RANK
  .short \rank
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ランク取得
 * @param  rank ランク格納バッファ
 */
//--------------------------------------------------------------
#define _TH_GET_RANK( rank ) _ASM_TH_GET_RANK rank
  
  .macro _ASM_TH_GET_RANK rank
  .short EV_SEQ_TH_GET_RANK
  .short \rank
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ビーコンサーチ
 * @param  ret    サーチ結果
 */
//--------------------------------------------------------------
#define _TH_SEARCH_BEACON( ret ) _ASM_TH_SEARCH_BEACON ret
  
  .macro _ASM_TH_SEARCH_BEACON ret
  .short EV_SEQ_TH_SEARCH_BEACON
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　採点
 * @param rank    採点結果のランク格納バッファ
 * @param point   得点格納バッファ
 */
//--------------------------------------------------------------
#define _TH_CALC_BTL_RESULT( rank, point ) _ASM_TH_CALC_BTL_RESULT rank, point
  
  .macro _ASM_TH_CALC_BTL_RESULT rank, point
  .short EV_SEQ_TH_CALC_BTL_RESULT
  .short \rank
  .short \point
  .endm
  
//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ダウンロードデータタイプ取得
 * @param type     タイプ格納ワーク
 * @note          戻り値は
 * TH_DL_DATA_TYPE_NONE ：データなし
 * TH_DL_DATA_TYPE_SINGLE ：シングル
 * TH_DL_DATA_TYPE_DOUBLE ：ダブル
 */
//--------------------------------------------------------------
#define _TH_GET_DL_DATA_TYPE( type ) _ASM_TH_GET_DL_DATA_TYPE type
  
  .macro _ASM_TH_GET_DL_DATA_TYPE type
  .short EV_SEQ_TH_GET_DL_DATA_TYPE
  .short \type
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ダウンロードデータをクリア
 * @param clear_type    TH_DL_DATA_CLEAR_TYPE_CLEAR or TH_DL_DATA_CLEAR_TYPE_USED
 * @note TH_DL_DATA_CLEAR_TYPE_CLEAR  完全クリア
 * @note TH_DL_DATA_CLEAR_TYPE_USED   使用したことにする  ＜＜　100529 こちらの定義は使用していません
 */
//--------------------------------------------------------------
#define _TH_CLEAR_DL_DATA( clear_type ) _ASM_TH_CLEAR_DL_DATA clear_type
  
  .macro _ASM_TH_CLEAR_DL_DATA clear_type
  .short EV_SEQ_TH_CLEAR_DL_DATA
  .short \clear_type
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ランキング確認アプリコール
 * @param dl
 * @param me
 * @param none
 */
//--------------------------------------------------------------
#define _TH_CALL_RANK_APP( dl, me ) _ASM_TH_CALL_RANK_APP dl, me
  
  .macro _ASM_TH_CALL_RANK_APP dl, me
  .short EV_SEQ_TH_CALL_RANK_APP
  .short \dl
  .short \me
  .endm

//--------------------------------------------------------------
/**
 * @brief トライアルハウス　ランキングデータ状況
 * @param state   ステート格納バッファ  trial_house_scr_def.h参照
 * @param none
 */
//--------------------------------------------------------------
#define _TH_GET_RDAT_STATE( state ) _ASM_TH_GET_RDAT_STATE state
  
  .macro _ASM_TH_GET_RDAT_STATE state
  .short EV_SEQ_TH_GET_RDAT_STATE
  .short \state
  .endm
  
//--------------------------------------------------------------
/**
 *  _PDW_COMMON_TOOLS PDW汎用ツール
 *  @param val1  取得用番号
 *  @param val2  取得用番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _PDW_COMMON_TOOLS( type, val1, val2, ret_val ) \
    _ASM_PDW_COMMON_TOOLS type, val1, val2, ret_val 
  
  .macro _ASM_PDW_COMMON_TOOLS type, val1, val2, ret_val 
  .short EV_SEQ_PDW_COMMON_TOOLS
  .short \type
  .short \val1
  .short \val2
  .short \ret_val
  .endm


//--------------------------------------------------------------
/**
 *  _PDW_FURNITURE_TOOLS PDW家具メニュー：汎用ツール
 *  @param val1  取得用番号
 *  @param val2  取得用番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
#define _PDW_FURNITURE_TOOLS( type, val1, val2, ret_val ) \
    _ASM_PDW_FURNITURE_TOOLS type, val1, val2, ret_val 
  
  .macro _ASM_PDW_FURNITURE_TOOLS type, val1, val2, ret_val 
  .short EV_SEQ_PDW_FURNITURE_TOOLS
  .short \type
  .short \val1
  .short \val2
  .short \ret_val
  .endm


//--------------------------------------------------------------
/**
 *  _PDW_SET_FURNITURE_WORD PDW家具メニュー：家具名セット
 * @param idx セットするタグナンバー
 * @param val セットする家具番号
 */
//--------------------------------------------------------------
#define _PDW_SET_FURNITURE_WORD( idx, val ) \
    _ASM_PDW_SET_FURNITURE_WORD idx, val

  .macro _ASM_PDW_SET_FURNITURE_WORD idx, val
  .short EV_SEQ_PDW_SET_FURNITURE_WORD
  .short \idx
  .short \val
  .endm

//--------------------------------------------------------------
/**
 *  _MACHINE_PASSWORD     パスワード答え合わせ
 * @param no    問題番号　1　or 2
 * @param word1   １つ目ワードコード
 * @param word2   ２つ目ワードコード
 * @param ret     結果　TRUEで正解
 */
//--------------------------------------------------------------
#define _MACHINE_PASSWORD( no, word1, word2, ret ) \
    _ASM_MACHINE_PASSWORD no, word1, word2, ret

  .macro _ASM_MACHINE_PASSWORD no, word1, word2, ret
  .short EV_SEQ_MACHINE_PASSWORD
  .short \no
  .short \word1
  .short \word2
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 *  _GET_QUIZ     クイズ取得
 * @param all   全問題から抽選していいか？　TRUEのとき全問題から抽選
 * @param quiz  クイズ
 * @param hint  ヒント
 * @param ans   答え簡易会話単語コード
 */
//--------------------------------------------------------------
#define _GET_QUIZ( all, quiz, hint, ans ) \
    _ASM_GET_QUIZ all, quiz, hint, ans

  .macro _ASM_GET_QUIZ all, quiz, hint, ans
  .short EV_SEQ_GET_QUIZ
  .short \all
  .short \quiz
  .short \hint
  .short \ans
  .endm



//--------------------------------------------------------------
/**
 *  _ITEMJUDGE_HAVE_CHECK     鑑定士がほしがるアイテムを持っているか？
 *  @param  obj_type  鑑定する人  
 *                    SCR_ITEM_JUDGE_OBJTYPE_GOURMET      (0)   // グルメ
 *                    SCR_ITEM_JUDGE_OBJTYPE_STONE_MANIA  (1)   // 石マニア
 *                    SCR_ITEM_JUDGE_OBJTYPE_RICH         (2)   // 大富豪
 *                    SCR_ITEM_JUDGE_OBJTYPE_KOUKO        (3)   // 考古学者
 *  @param  ret_wk    持っている：TRUE    もっていない：FALSE
 */
//--------------------------------------------------------------
#define _ITEMJUDGE_HAVE_CHECK( obj_type, ret_wk ) \
    _ASM_ITEMJUDGE_HAVE_CHECK obj_type, ret_wk

  .macro _ASM_ITEMJUDGE_HAVE_CHECK obj_type, ret_wk
  .short EV_SEQ_ITEMJUDGE_HAVE_CHECK
  .short \obj_type
  .short \ret_wk
  .endm  


//--------------------------------------------------------------
/**
 *  _ITEMJUDGE     鑑定士
 *  @param  item_no   鑑定するアイテム
 *  @param  obj_type  鑑定する人  
 *                    SCR_ITEM_JUDGE_OBJTYPE_GOURMET      (0)   // グルメ
 *                    SCR_ITEM_JUDGE_OBJTYPE_STONE_MANIA  (1)   // 石マニア
 *                    SCR_ITEM_JUDGE_OBJTYPE_RICH         (2)   // 大富豪
 *                    SCR_ITEM_JUDGE_OBJTYPE_KOUKO        (3)   // 考古学者
 *  @param  ret_money 金額格納ワーク      
 *
 *  ret_moneyが0なら興味なし
 */
//--------------------------------------------------------------
#define _ITEMJUDGE( item_no, obj_type, ret_money ) \
    _ASM_ITEMJUDGE item_no, obj_type, ret_money

  .macro _ASM_ITEMJUDGE item_no, obj_type, ret_money
  .short EV_SEQ_ITEMJUDGE_CHECK
  .short \item_no
  .short \obj_type
  .short \ret_money
  .endm  

//--------------------------------------------------------------
/**
 *  _ITEMJUDGE_SET_WORD_MONEY     鑑定士 金額をワードセットに設定
 *  @param  item_no   鑑定するアイテム
 *  @param  obj_type  鑑定する人  
 *                    SCR_ITEM_JUDGE_OBJTYPE_GOURMET      (0)   // グルメ
 *                    SCR_ITEM_JUDGE_OBJTYPE_STONE_MANIA  (1)   // 石マニア
 *                    SCR_ITEM_JUDGE_OBJTYPE_RICH         (2)   // 大富豪
 *                    SCR_ITEM_JUDGE_OBJTYPE_KOUKO        (3)   // 考古学者
 *  @param  buf_id    ワードセットのバッファID
 *  @param  keta      桁
 *
 */
//--------------------------------------------------------------
#define _ITEMJUDGE_SET_WORD_MONEY( item_no, obj_type, buf_id, keta ) \
    _ASM_ITEMJUDGE_SET_WORD_MONEY item_no, obj_type, buf_id, keta

  .macro _ASM_ITEMJUDGE_SET_WORD_MONEY item_no, obj_type, buf_id, keta
  .short EV_SEQ_ITEMJUDGE_SET_WORDSET
  .short \item_no
  .short \obj_type
  .short \buf_id
  .short \keta
  .endm  

//--------------------------------------------------------------
/**
 *  _ITEMJUDGE_ADD_MONEY     鑑定士 金額を所持金に加算
 *  @param  item_no   鑑定するアイテム
 *  @param  obj_type  鑑定する人  
 *                    SCR_ITEM_JUDGE_OBJTYPE_GOURMET      (0)   // グルメ
 *                    SCR_ITEM_JUDGE_OBJTYPE_STONE_MANIA  (1)   // 石マニア
 *                    SCR_ITEM_JUDGE_OBJTYPE_RICH         (2)   // 大富豪
 *                    SCR_ITEM_JUDGE_OBJTYPE_KOUKO        (3)   // 考古学者
 *
 */
//--------------------------------------------------------------
#define _ITEMJUDGE_ADD_MONEY( item_no, obj_type ) \
    _ASM_ITEMJUDGE_ADD_MONEY item_no, obj_type

  .macro _ASM_ITEMJUDGE_ADD_MONEY item_no, obj_type
  .short EV_SEQ_ITEMJUDGE_ADD_MONEY
  .short \item_no
  .short \obj_type
  .endm  




//======================================================================
// すれ違い調査隊関連
//======================================================================

//--------------------------------------------------------------
/**
 * @brief すれ違い調査隊としての調査を開始する
 *
 * @param req [in] 開始する調査依頼ID ( RESEARCH_REQ_ID_xxxx )
 * @param q1  [in] 調査する質問ID ( QUESTION_ID_xxxx )
 * @param q2  [in] 調査する質問ID ( QUESTION_ID_xxxx )
 * @param q3  [in] 調査する質問ID ( QUESTION_ID_xxxx )
 */
//--------------------------------------------------------------
#define _START_RESEARCH( req, q1, q2, q3 ) \
    _ASM_START_RESEARCH req, q1, q2, q3

  .macro _ASM_START_RESEARCH req, q1, q2, q3 
  .short EV_SEQ_START_RESEARCH
  .short \req
  .short \q1
  .short \q2
  .short \q3
  .endm

//--------------------------------------------------------------
/**
 * @brief すれ違い調査隊としての調査を終了する
 */
//--------------------------------------------------------------
#define _FINISH_RESEARCH() \
    _ASM_FINISH_RESEARCH

  .macro _ASM_FINISH_RESEARCH
  .short EV_SEQ_FINISH_RESEARCH
  .endm

//--------------------------------------------------------------
/**
 * @brief すれ違い調査隊として調査中の調査依頼IDを取得する
 *
 * @param ret [out] 調査中の調査依頼IDの格納先 ( 存在しない場合, RESEARCH_REQ_ID_NONE )
 */
//--------------------------------------------------------------
#define _GET_RESEARCH_REQUEST_ID( ret ) \
    _ASM_GET_RESEARCH_REQUEST_ID ret

  .macro _ASM_GET_RESEARCH_REQUEST_ID ret
  .short EV_SEQ_GET_RESEARCH_REQUEST_ID
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief すれ違い調査隊として調査中の質問IDを取得する
 *
 * @param ret_q1 [out] 調査中の質問IDの格納先 ( 存在しない場合, QUESTION_ID_DUMMY )
 * @param ret_q2 [out] 調査中の質問IDの格納先 ( 存在しない場合, QUESTION_ID_DUMMY )
 * @param ret_q3 [out] 調査中の質問IDの格納先 ( 存在しない場合, QUESTION_ID_DUMMY )
 */
//--------------------------------------------------------------
#define _GET_RESEARCH_QUESTION_ID( ret_q1, ret_q2, ret_q3 ) \
    _ASM_GET_RESEARCH_QUESTION_ID ret_q1, ret_q2, ret_q3

  .macro _ASM_GET_RESEARCH_QUESTION_ID ret_q1, ret_q2, ret_q3 
  .short EV_SEQ_GET_RESEARCH_QUESTION_ID
  .short \ret_q1
  .short \ret_q2
  .short \ret_q3
  .endm

//--------------------------------------------------------------
/**
 * @brief 質問に対する, 最も多数派の回答IDを取得する
 *
 * @param qID [in]  質問ID ( QUESTION_ID_xxxx )
 * @param ret [out] 回答IDの格納先
 */
//--------------------------------------------------------------
#define _GET_MAJORITY_ANSWER_OF_QUESTION( qID, ret ) \
    _ASM_GET_MAJORITY_ANSWER_OF_QUESTION qID, ret

  .macro _ASM_GET_MAJORITY_ANSWER_OF_QUESTION qID, ret
  .short EV_SEQ_GET_MAJORITY_ANSWER_OF_QUESTION
  .short \qID
  .short \ret
  .endm 

//--------------------------------------------------------------
/**
 * @brief 依頼の達成状況をチェックする
 *
 * @param RID [in]  依頼ID ( RESEARCH_REQ_ID_xxxx )
 * @param ret [out] 達成状況の格納先 ( RESEARCH_REQ_STATE_xxxx )
 */
//--------------------------------------------------------------
#define _CHECK_ACHIEVE_REQUEST( RID, ret ) \
    _ASM_CHECK_ACHIEVE_REQUEST RID, ret

  .macro _ASM_CHECK_ACHIEVE_REQUEST RID, ret
  .short EV_SEQ_CHECK_ACHIEVE_REQUEST
  .short \RID
  .short \ret
  .endm 

//--------------------------------------------------------------
/**
 * @brief 調査中の依頼の達成までの不足値を取得する
 *
 * @param ret [out] 不足値の格納先
 */
//--------------------------------------------------------------
#define _GET_LACK_FOR_ACHIEVE( ret ) \
    _ASM_GET_LACK_FOR_ACHIEVE ret

  .macro _ASM_GET_LACK_FOR_ACHIEVE ret
  .short EV_SEQ_GET_LACK_FOR_ACHIEVE
  .short \ret
  .endm 

//--------------------------------------------------------------
/**
 * @brief 調査隊の調査について, 経過した時間[h]を取得する
 *
 * @param ret [out] 経過時間の格納先
 *
 * ※戻り値は最大24!!
 */
//--------------------------------------------------------------
#define _GET_RESEARCH_PASSED_TIME( ret ) \
    _ASM_GET_RESEARCH_PASSED_TIME ret

  .macro _ASM_GET_RESEARCH_PASSED_TIME ret
  .short EV_SEQ_GET_RESEARCH_PASSED_TIME
  .short \ret
  .endm 

//--------------------------------------------------------------
/**
 * @brief すれ違い『挨拶メッセージ』入力画面を呼び出す
 *
 * @param ret [out] 入力を確定したかどうかを受け取るワーク ( TRUE: 確定 )
 */
//--------------------------------------------------------------
#define _CALL_CROSS_COMM_HELLO_MSG_INPUT( ret ) \
    _ASM_CALL_CROSS_COMM_HELLO_MSG_INPUT ret

  .macro _ASM_CALL_CROSS_COMM_HELLO_MSG_INPUT ret
  .short EV_SEQ_CALL_CROSS_COMM_HELLO_MSG_INPUT
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief すれ違い『お礼メッセージ』入力画面を呼び出す
 *
 * @param ret [out] 入力を確定したかどうかを受け取るワーク ( TRUE: 確定 )
 */
//--------------------------------------------------------------
#define _CALL_CROSS_COMM_THANKS_MSG_INPUT( ret ) \
    _ASM_CALL_CROSS_COMM_THANKS_MSG_INPUT ret

  .macro _ASM_CALL_CROSS_COMM_THANKS_MSG_INPUT ret
  .short EV_SEQ_CALL_CROSS_COMM_THANKS_MSG_INPUT
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief すれ違い調査の, 自分の回答を設定する
 *
 * @param qID [in] 質問ID ( QUESTION_ID_xxxx )
 * @param aIdx[in] 何番目の回答か？
 */
//--------------------------------------------------------------
#define _SET_MY_ANSWER( qID, aIdx ) \
    _ASM_SET_MY_ANSWER qID, aIdx

  .macro _ASM_SET_MY_ANSWER qID, aIdx
  .short EV_SEQ_SET_MY_ANSWER
  .short \qID
  .short \aIdx
  .endm

//--------------------------------------------------------------
/**
 * @brief 隊員情報を表示する
 */
//--------------------------------------------------------------
#define _DISP_RESEARCH_TEAM_INFO() \
    _ASM_DISP_RESEARCH_TEAM_INFO

  .macro _ASM_DISP_RESEARCH_TEAM_INFO
  .short EV_SEQ_DISP_RESEARCH_TEAM_INFO
  .endm

//--------------------------------------------------------------
/**
 * @brief 調査隊の隊員ランクを取得する
 *
 * @param ret [out] 隊員ランクの格納先 ( RESEARCH_TEAM_RANK_ 0～5 )
 */
//--------------------------------------------------------------
#define _GET_RESEARCH_TEAM_RANK( ret ) \
    _ASM_GET_RESEARCH_TEAM_RANK ret

  .macro _ASM_GET_RESEARCH_TEAM_RANK ret
  .short EV_SEQ_GET_RESEARCH_TEAM_RANK
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief 調査隊の隊員ランクを1つ上げる
 */
//--------------------------------------------------------------
#define _RESEARCH_TEAM_RANK_UP() \
    _ASM_RESEARCH_TEAM_RANK_UP

  .macro _ASM_RESEARCH_TEAM_RANK_UP
  .short EV_SEQ_RESEARCH_TEAM_RANK_UP
  .endm

//--------------------------------------------------------------
/**
 * @brief 手持ちの配布ポケの位置を返す
 *
 * @param monsno    対象モンスターナンバー
 * @param skill_flg 固有技を持っていないことを条件に加える場合はTRUE
 * @param pos       手持ちポケモンの位置    
 * @param ret       条件を満たしたポケモンがいた場合はTRUE
 */
//--------------------------------------------------------------
#define _GET_EVT_POKE_POS( monsno, skill_flg, pos, ret ) \
    _ASM_GET_EVT_POKE_POS monsno, skill_flg, pos, ret

  .macro _ASM_GET_EVT_POKE_POS monsno, skill_flg, pos, ret
  .short EV_SEQ_GET_EVT_POKE_POS
  .short \monsno
  .short \skill_flg
  .short \pos
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief ボール移動（使用場所は特殊ポケイベント限定）
 *
 * @param type      ボールアニメタイプ SCR_BALL_ANM_TYPE_～　script_def.h参照
 * @param tx        目的Ｘ座標
 * @param ty        目的Ｙ座標 0.5グリッドサイズずつの指定（2を設定すると1グリッド）
 * @param tz        目的Ｚ座標
 * @param height    アニメ中のボール高さ
 * @param sync      必要シンク数
 */
//--------------------------------------------------------------
#define _MOVE_BALL( type, tx, ty, tz, height, sync ) \
    _ASM_MOVE_BALL type, tx, ty, tz, height, sync

  .macro _ASM_MOVE_BALL type, tx, ty, tz, height, sync
  .short EV_SEQ_MOVE_BALL
  .short \type
  .short \tx
  .short \ty
  .short \tz
  .short \height
  .short \sync
  .endm

//--------------------------------------------------------------
/**
 * @brief ボールアニメ開始（使用場所は特殊ポケイベント限定）
 *
 * @param type      ボールアニメタイプ SCR_BALL_ANM_TYPE_～　script_def.h参照
 * @param tx        Ｘ座標
 * @param ty        Ｙ座標 0.5グリッドサイズずつの指定（2を設定すると1グリッド）
 * @param tz        Ｚ座標
 */
//--------------------------------------------------------------
#define _START_BALL_ANM( type, tx, ty, tz  ) \
    _ASM_START_BALL_ANM type, tx, ty, tz

  .macro _ASM_START_BALL_ANM type, tx, ty, tz
  .short EV_SEQ_START_BALL_ANM
  .short \type
  .short \tx
  .short \ty
  .short \tz
  .endm

//--------------------------------------------------------------
/**
 * @brief ボールアニメ終了待ち（使用場所は特殊ポケイベント限定）
 *
 * @param type      ボールアニメタイプ SCR_BALL_ANM_TYPE_～　script_def.h参照
 */
//--------------------------------------------------------------
#define _WAIT_BALL_ANM( type  ) \
    _ASM_WAIT_BALL_ANM type

  .macro _ASM_WAIT_BALL_ANM type
  .short EV_SEQ_WAIT_BALL_ANM
  .short \type
  .endm

//--------------------------------------------------------------
/**
 * @brief ボールアニメ中、ポケモンを表示、非表示していいタイミングまで待つ（使用場所は特殊ポケイベント限定）
 *
 * @param type      ボールアニメタイプ SCR_BALL_ANM_TYPE_～　script_def.h参照
 */
//--------------------------------------------------------------
#define _WAIT_BALL_POKE_APP( type  ) \
    _ASM_WAIT_BALL_POKE_APP type

  .macro _ASM_WAIT_BALL_POKE_APP type
  .short EV_SEQ_WAIT_BALL_POKE_APP
  .short \type
  .endm

//--------------------------------------------------------------
/**
 * @brief 指定ＯＢＪを指定シンクかけて指定値分上昇させる
 *
 * @param obj_id      対象ＯＢＪＩＤ
 * @param height      上昇値  （半グリッド単位）
 * @param sync        必要シンク数
 */
//--------------------------------------------------------------
#define _EVENT_RISE_OBJ( obj_id, height, sync  ) \
    _ASM_EVENT_RISE_OBJ obj_id, height, sync

  .macro _ASM_EVENT_RISE_OBJ obj_id, height, sync
  .short EV_SEQ_EVENT_RISE_OBJ
  .short \obj_id
  .short \height
  .short \sync
  .endm

//--------------------------------------------------------------
/**
 * @brief 指定ＯＢＪをイベント回転させる
 *
 * @param obj_id      対象ＯＢＪＩＤ
 */
//--------------------------------------------------------------
#define _EVENT_ROTATE_OBJ( obj_id ) \
    _ASM_EVENT_ROTATE_OBJ obj_id

  .macro _ASM_EVENT_ROTATE_OBJ obj_id
  .short EV_SEQ_EVENT_ROTATE_OBJ
  .short \obj_id
  .endm

//--------------------------------------------------------------
/**
 * @brief ポケモンウィンドウを出す（パッチールのブチ非対応）
 *
 * @param monsno      モンスターナンバー
 * @param form        フォルムナンバー
 * @param sex         性別
 * @param rare        レアカラー　0：通常　1：レア
 */
//--------------------------------------------------------------
#define _DISP_POKE_WIN( monsno, form, sex, rare ) \
    _ASM_DISP_POKE_WIN monsno, form, sex, rare

  .macro _ASM_DISP_POKE_WIN monsno, form, sex, rare
  .short EV_SEQ_DISP_POKE_WIN
  .short \monsno
  .short \form
  .short \sex
  .short \rare
  .endm

//--------------------------------------------------------------
/**
 * @brief 手持ちイベントポケモンにイベント終了フラグをセットする
 *
 * @param monsno      モンスターナンバー
 * @param pos    手持ちの位置
 */
//--------------------------------------------------------------
#define _SET_EVT_POKE_AFT_FLG( monsno, pos ) \
    _ASM_SET_EVT_POKE_AFT_FLG monsno, pos

  .macro _ASM_SET_EVT_POKE_AFT_FLG monsno, pos
  .short EV_SEQ_SET_EVT_POKE_AFT_FLG
  .short \monsno
  .short \pos
  .endm

//--------------------------------------------------------------
/**
 * @brief 配布系イベントロックチェック
 *
 * @param lock_no     イベントナンバー savedata/evt_lock_def.h参照
 * @param ret         結果    TRUE:正規開示　FALSE:未開示あるいは不正
 */
//--------------------------------------------------------------
#define _CHK_EVT_LOCK( lock_no, ret ) \
    _ASM_CHK_EVT_LOCK lock_no, ret

  .macro _ASM_CHK_EVT_LOCK lock_no, ret
  .short EV_SEQ_CHK_EVT_LOCK
  .short \lock_no
  .short \ret
  .endm

//--------------------------------------------------------------
/**
 * @brief レコード加算
 *
 * @param id    対象レコードＩＤ    RECID_～　savedata/record.h参照
 * @param val   加算値
 */
//--------------------------------------------------------------
#define _ADD_RECORD( id, val ) _ASM_ADD_RECORD id, val

  .macro _ASM_ADD_RECORD id, val
  .short EV_SEQ_ADD_RECORD
  .short \id
  .short \val
  .endm

//--------------------------------------------------------------
/**
 * @brief レコード参照
 *
 * @param id    対象レコードＩＤ    RECID_～　savedata/record.h参照
 * @param val   取得値
 */
//--------------------------------------------------------------
#define _GET_RECORD( id, val ) _ASM_GET_RECORD id, val

  .macro _ASM_GET_RECORD id, val
  .short EV_SEQ_GET_RECORD
  .short \id
  .short \val
  .endm
  
//--------------------------------------------------------------
/**
 * @brief D12ギミックバニッシュ   ビクティイベント専用
 *
 */
//--------------------------------------------------------------
#define _D12_GMK_VANISH() _ASM_D12_GMK_VANISH

  .macro _ASM_D12_GMK_VANISH
  .short EV_SEQ_D12_GMK_VANISH
  .endm

//--------------------------------------------------------------
/**
 * @brief テレビ番組メッセージ抽選
 * @param   clear ゲームクリアしてるか？
 * @param   msg   抽選結果メッセージ
 */
//--------------------------------------------------------------
#define _TV_GET_MSG( clear, msg ) _ASM_TV_GET_MSG clear, msg

  .macro _ASM_TV_GET_MSG clear, msg
  .short EV_SEQ_TV_GET_MSG
  .short \clear
  .short \msg
  .endm

//--------------------------------------------------------------
/**
 * @brief ゾロア変身解除
 * @param   before      解除前ＯＢＪ
 * @param   after       解除後ＯＢＪ
 */
//--------------------------------------------------------------
#define _CHG_ZOROA( before, after ) _ASM_CHG_ZOROA before, after

  .macro _ASM_CHG_ZOROA before, after
  .short EV_SEQ_CHG_ZOROA
  .short \before
  .short \after
  .endm

//--------------------------------------------------------------
/**
 * @brief 伝説ポケギミック開始
 */
//--------------------------------------------------------------
#define _START_LEG_GMK() _ASM_START_LEG_GMK

  .macro _ASM_START_LEG_GMK
  .short EV_SEQ_START_LEG_GMK
  .endm

//--------------------------------------------------------------
/**
 * @brief 伝説ポケモン戦の結果処理用コマンド
 * @param cmd_id    コマンドID（script_def.hのSCR_CRIMAX_～を参照）
 * @param use_wk    コマンドとのやりとりに使用するワーク
 *
 * ※謁見の間の伝説ポケモン戦特殊処理専用コマンドです。
 * 　他では使用しないでください。
 */
//--------------------------------------------------------------
#define _CRIMAX_DEMO_COMMAND( cmd_id, use_wk )  \
    _ASM_CRIMAX_DEMO_COMMAND cmd_id, use_wk

    .macro  _ASM_CRIMAX_DEMO_COMMAND cmd_id, use_wk
    .short  EV_SEQ_CRIMAX_COMMAND
    .short  \cmd_id
    .short  \use_wk
    .endm

//--------------------------------------------------------------
/**
 * @brief T01ギミック表示
 *
 */
//--------------------------------------------------------------
#define _T01_GMK_DISP() _ASM_T01_GMK_DISP

  .macro _ASM_T01_GMK_DISP
  .short EV_SEQ_T01_GMK_DISP
  .endm

//--------------------------------------------------------------
/**
 * @brief T01ギミック発動トリガーセット
 *
 */
//--------------------------------------------------------------
#define _T01_GMK_START() _ASM_T01_GMK_START

  .macro _ASM_T01_GMK_START
  .short EV_SEQ_T01_GMK_START
  .endm

//--------------------------------------------------------------
/**
 * @brief T01ギミック終了待ち
 *
 */
//--------------------------------------------------------------
#define _T01_GMK_WAIT_END() _ASM_T01_GMK_WAIT_END

  .macro _ASM_T01_GMK_WAIT_END
  .short EV_SEQ_T01_GMK_WAIT_END
  .endm

//--------------------------------------------------------------
/**
 * @brief ボール移動（使用場所はＮの城パッケージポケイベント限定）
 *
 * @param target      対象ＯＢＪ
 * @param tx        目的Ｘ座標
 * @param ty        目的Ｙ座標 0.5グリッドサイズずつの指定（2を設定すると1グリッド）
 * @param tz        目的Ｚ座標
 * @param height    アニメ中のボール高さ
 * @param sync      必要シンク数
 */
//--------------------------------------------------------------
#define _LEG_MOVE_BALL( target, tx, ty, tz, height, sync ) \
    _ASM_LEG_MOVE_BALL target, tx, ty, tz, height, sync

  .macro _ASM_LEG_MOVE_BALL target, tx, ty, tz, height, sync
  .short EV_SEQ_LEG_MOVE_BALL
  .short \target
  .short \tx
  .short \ty
  .short \tz
  .short \height
  .short \sync
  .endm

//--------------------------------------------------------------
/**
 * @brief ボールアニメ開始（使用場所はＮの城パッケージポケイベント限定）
 *
 * @param tx        Ｘ座標
 * @param ty        Ｙ座標 0.5グリッドサイズずつの指定（2を設定すると1グリッド）
 * @param tz        Ｚ座標
 */
//--------------------------------------------------------------
#define _LEG_START_BALL_ANM( tx, ty, tz  ) \
    _ASM_LEG_START_BALL_ANM tx, ty, tz

  .macro _ASM_LEG_START_BALL_ANM tx, ty, tz
  .short EV_SEQ_LEG_START_BALL_ANM
  .short \tx
  .short \ty
  .short \tz
  .endm

//--------------------------------------------------------------
/**
 * @brief ボールアニメ終了待ち（使用場所はＮの城パッケージポケイベント限定）
 *
 */
//--------------------------------------------------------------
#define _LEG_WAIT_BALL_ANM() \
    _ASM_LEG_WAIT_BALL_ANM

  .macro _ASM_LEG_WAIT_BALL_ANM
  .short EV_SEQ_LEG_WAIT_BALL_ANM
  .endm

//--------------------------------------------------------------
/**
 * @brief ボールアニメ中、ポケモンを表示、非表示していいタイミングまで待つ
 * （使用場所はＮの城パッケージポケイベント限定）
 *
 */
//--------------------------------------------------------------
#define _LEG_WAIT_BALL_POKE_APP() \
    _ASM_LEG_WAIT_BALL_POKE_APP

  .macro _ASM_LEG_WAIT_BALL_POKE_APP
  .short EV_SEQ_LEG_WAIT_BALL_POKE_APP
  .endm


//--------------------------------------------------------------
/**
 * @def _REBATTLE_TRAINER_GET_RNDTR_INDEX
 * @brief 再戦トレーナー　ランダムNPC　抽選処理
 *
 * @param 
 */
//--------------------------------------------------------------
#define _REBATTLE_TRAINER_GET_RNDTR_INDEX( ret_index0, ret_index1, ret_index2, ret_index3, ret_index4, ret_index5 ) \
    _ASM_REBATTLE_TRAINER_GET_RNDTR_INDEX ret_index0, ret_index1, ret_index2, ret_index3, ret_index4, ret_index5
  
  .macro _ASM_REBATTLE_TRAINER_GET_RNDTR_INDEX ret_index0, ret_index1, ret_index2, ret_index3, ret_index4, ret_index5
  .short EV_SEQ_REBATTLE_TRAINER_GET_RNDTR_INDEX
  .short \ret_index0
  .short \ret_index1
  .short \ret_index2
  .short \ret_index3
  .short \ret_index4
  .short \ret_index5
  .endm
  
