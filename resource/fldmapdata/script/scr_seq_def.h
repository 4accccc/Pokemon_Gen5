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

#if 0 //wb null
//--------------------------------------------------------------
/**
 * デバッグ用：ワーク情報表示
 */
//--------------------------------------------------------------
  .macro  _DEBUG_WATCH_WORK  wk
  .short  EV_SEQ_DEBUG_WATCH_VALUE
  .short  \wk
  .endm
#endif //wb null

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

//======================================================================
//  フィールド　システムウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  _SYSWIN_MSG_UP 展開メッセージを表示(1byte)　ウィンドウ上
 *  @param  msg_id  表示するメッセージID
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG_UP( msg_id ) _ASM_SYSWIN_MSG msg_id,WIN_UP

//--------------------------------------------------------------
/**
 *  _SYSWIN_MSG_DOWN 展開メッセージを表示(1byte)　ウィンドウ上
 *  @param  msg_id  表示するメッセージID
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG_DOWN( msg_id ) _ASM_SYSWIN_MSG msg_id,WIN_DOWN

  .macro  _ASM_SYSWIN_MSG msg_id,up_down
  .short  EV_SEQ_SYSWIN_MSG
  .short  \msg_id
  .short  \up_down
  .endm

//--------------------------------------------------------------
/**
 *  _SYSWIN_MSG_ALLPUT_UP 展開メッセージを一括表示(1byte) ウィンドウ上
 *  @param  msg_id  表示するメッセージid
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG_ALLPUT_UP( msg_id ) \
    _ASM_SYSWIN_MSG_ALLPUT msg_id, WIN_UP

//--------------------------------------------------------------
/**
 *  _SYSWIN_MSG_ALLPUT_DOWN 展開メッセージを一括表示(1byte) ウィンドウ下
 *  @param  msg_id  表示するメッセージid
 */
//--------------------------------------------------------------
#define _SYSWIN_MSG_ALLPUT_DOWN( msg_id ) \
    _ASM_SYSWIN_MSG_ALLPUT msg_id, WIN_DOWN
  
  .macro  _ASM_SYSWIN_MSG_ALLPUT msg_id, up_down
  .short  EV_SEQ_SYSWIN_MSG_ALLPUT
  .short  \msg_id
  .short  \up_down
  .endm

//--------------------------------------------------------------
/**
 *  _TALKWIN_OPEN システムウィンドウ開く　なくなりました。
 */
//--------------------------------------------------------------
#if 0
#define _SYSWIN_OPEN() _ASM_SYSWIN_OPEN WIN_DOWN
#define _SYSWIN_OPEN_UP() _ASM_SYSWIN_OPEN WIN_UP
#define _SYSWIN_OPEN_DOWN() _ASM_SYSWIN_OPEN WIN_DOWN
#define _TALKWIN_OPEN() _ASM_SYSWIN_OPEN WIN_DOWN

  .macro  _ASM_SYSWIN_OPEN up_down
  .short  EV_SEQ_TALKWIN_OPEN
  .short  \up_down
  .endm
#endif

//--------------------------------------------------------------
/**
 *  _SYSWIN_CLOSE システムウィンドウ閉じる
 *  @param none
 */
//--------------------------------------------------------------
#define _SYSWIN_CLOSE() _ASM_SYSWIN_CLOSE

  .macro  _ASM_SYSWIN_CLOSE
  .short  EV_SEQ_SYSWIN_CLOSE
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
  .short WIN_NONE
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
  .short WIN_NONE
  .short BALLOONWIN_TYPE_NORMAL
  .endm

#define _BALLOONWIN_TALKOBJ_MSG_DOWN( msg_id ) \
    _ASM_BALLOONWIN_OBJMSG_POS msg_id, SCWK_TARGET_OBJID, WIN_DOWN 

#define _BALLOONWIN_TALKOBJ_MSG_UP( msg_id ) \
    _ASM_BALLOONWIN_OBJMSG_POS msg_id, SCWK_TARGET_OBJID, WIN_UP

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
  .short WIN_NONE
  .short BALLOONWIN_TYPE_NORMAL
  .endm

//--------------------------------------------------------------
/**
 *  @def  _BALLOONWIN_OBJMSG_POS
 *  @brief  吹き出しウィンドウ描画　位置指定あり
 *  @param msg_id 表示するメッセージID
 *  @param obj_id 吹き出しを出す対象OBJ ID
 *  @param pos 吹き出しウィンドウ位置 WIN_UP,WIN_DONW,WIN_NONE
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
 *  @param pos 吹き出しウィンドウ位置 WIN_UP,WIN_DONW,WIN_NONE
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
 *  @param pos 吹き出しウィンドウ位置 WIN_UP,WIN_DONW,WIN_NONE
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
 *  @param pos 吹き出しウィンドウ位置 WIN_UP,WIN_DONW,WIN_NONE
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
 *  @brief 所持金ウィンドウを表示する
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
 *  @param x 表示x位置(キャラ単位)
 *  @param y 表示x位置(キャラ単位)
 *  @param none
 */
//--------------------------------------------------------------
#define _GOLD_WIN_UPDATE( x, y ) \
    _ASM_GOLD_WIN_UPDATE x, y

  .macro _ASM_GOLD_WIN_UPDATE x, y
  .short EV_SEQ_GOLD_WIN_UPDATE
  .short \x
  .short \y
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
#define _PLAINWIN_MSG_UP( msg_id ) _ASM_PLAINWIN_MSG msg_id, WIN_UP

//--------------------------------------------------------------
/**
 *  @def _PLAINWIN_MSG_DOWN
 *  @brief プレーンウィンドウを下に表示
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _PLAINWIN_MSG_DOWN( msg_id ) _ASM_PLAINWIN_MSG msg_id, WIN_DOWN
  
  .macro _ASM_PLAINWIN_MSG msg_id, up_down
  .short EV_SEQ_PLAINWIN_MSG
  .short \msg_id
  .byte \up_down
  .endm

//--------------------------------------------------------------
/**
 *  @def _PLAINWIN_MSG_CLOSE
 *  @brief プレーンウィンドウを閉じる
 *  @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _PLAINWIN_CLOSE() _ASM_PLAINWIN_CLOSE
  
  .macro _ASM_PLAINWIN_CLOSE
  .short EV_SEQ_PLAINWIN_CLOSE
  .endm

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
  .byte \pos_x
  .byte \pos_y
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
 * _OBJ_ADD_EX OBJを追加
 * @param x 表示するグリッドX座標
 * @param z 表示するグリッドZ座標
 * @param dir 方向 DIR_UP等
 * @param id 識別用OBJ ID
 * @param code 表示コードBOY1等
 * @param move 動作コードMV_RND等
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
 * _OBJ_DEL_EX OBJを削除
 * @param id 削除するOBJ ID
 */
//--------------------------------------------------------------
#define _OBJ_DEL_EX( id ) _ASM_OBJ_DEL_EX id

.macro  _ASM_OBJ_DEL_EX id
  .short EV_SEQ_OBJ_DEL
  .short \id
.endm
 
//--------------------------------------------------------------
/**
 * _OBJ_ADD ゾーン内で配置されているOBJ一覧内、指定のIDを持つOBJを追加
 * @param id 配置OBJ内で追加するOBJ ID
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
 * ！！使用禁止です！！　タイミングをみてコマンドを削除予定
 * _OBJ_DEL_EV 指定のOBJ_IDを持ったOBJを削除。
 * 削除時、対象OBJで指定されているイベントフラグをONにする。
 * @param id 配置OBJ内で追加するOBJ ID
 */
//--------------------------------------------------------------
#define _OBJ_DEL( id )  \
    _ASM_OBJ_DEL id

  .macro _ASM_OBJ_DEL id
  .short EV_SEQ_OBJ_DEL_EV
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
 * @param reqbit FIELD_PLAYER_REQBIT_NORMAL等
 */
//--------------------------------------------------------------
#define _PLAYER_REQUEST( reqbit ) _ASM_PLAYER_REQUEST reqbit

  .macro  _ASM_PLAYER_REQUEST reqbit
  .short  EV_SEQ_PLAYER_REQUEST
  .short  \reqbit
  .endm

//--------------------------------------------------------------
/**
 * @def   自機の高さ変化演出
 * @param type  　動かす種類
 * @param frame   動かす長さ
 * @param length
 */
//--------------------------------------------------------------
#define _PLAYER_UPDOWN_EFFECT( type, frame, length ) \
    _ASM_PLAYER_UPDOWN type, frame, length

  .macro  _ASM_PLAYER_UPDOWN type, frame, length
  .short  EV_SEQ_PLAYER_UPDOWN
  .short  \type
  .short  \frame
  .short  \length
  .endm

//--------------------------------------------------------------
/**
 * @def   自機の高さ変化演出
 * @param type  　動かす種類
 * @param frame   動かす長さ
 * @param length
 */
//--------------------------------------------------------------
#define _POS_PLAYER_TURN() _ASM_POS_PLAYER_TURN

  .macro  _ASM_POS_PLAYER_TURN
  .short  EV_SEQ_POS_PLAYER_TURN
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
 *  @def  _EYE_TRAINER_MOVE_SINGLE
 *  @brief  視線：トレーナー移動　シングル
 *  @param pos 移動させるトレーナー データ番号0,1
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_MOVE_SINGLE( pos ) \
    _ASM_EYE_TRAINER_MOVE_SINGLE pos

  .macro  _ASM_EYE_TRAINER_MOVE_SINGLE pos
  .short  EV_SEQ_EYE_TRAINER_MOVE_SINGLE
  .short  \pos
  .endm
 
//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_MOVE_DOUBLE
 *  @brief  視線：トレーナー移動　ダブル
 *  @param none
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_MOVE_DOUBLE()  \
    _ASM_EYE_TRAINER_MOVE_DOUBLE

  .macro  _ASM_EYE_TRAINER_MOVE_DOUBLE
  .short  EV_SEQ_EYE_TRAINER_MOVE_DOUBLE
  .endm

//--------------------------------------------------------------
/**
 *  @def  _EYE_TRAINER_TYPE_GET
 *  @brief  視線：トレーナータイプ取得
 *  @param ret_wk トレーナータイプ格納先 SCR_EYE_TR_TYPE_SINGLE等が格納される
 *
 *  @note
 *  トレーナー視線スクリプトから呼ばれている。
 *  通常スクリプトでの使用は禁止とする。
 */
//--------------------------------------------------------------
#define _EYE_TRAINER_TYPE_GET( ret_wk ) \
    _ASM_EYE_TRAINER_TYPE_GET ret_wk

  .macro  _ASM_EYE_TRAINER_TYPE_GET ret_wk
  .short  EV_SEQ_EYE_TRAINER_TYPE_GET
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
 *  @param partner_id パートナーID
 *  @param tr_id_0 トレーナーID0
 *  @param tr_id_1 トレーナーID1
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
 */
//--------------------------------------------------------------
#define _TRAINER_MSG_SET( tr_id,kind_id ) \
  _ASM_TRAINER_MSG_SET  tr_id,kind_id

  .macro  _ASM_TRAINER_MSG_SET  tr_id,kind_id
  .short  EV_SEQ_TRAINER_MSG_SET
  .short  \tr_id
  .short  \kind_id
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
 *  _REVENGE_TRAINER_TALK_TYPE_GET 再戦トレーナー会話の種類取得
 *  @param wk1 会話種類格納先ワーク 戦闘前
 *  @param wk2 会話種類格納先ワーク 戦闘後
 *  @param wk2 会話種類格納先ワーク ポケモン１体
 */
//--------------------------------------------------------------
#define _REVENGE_TRAINER_TALK_TYPE_GET(  wk1,wk2,wk3 ) \
  _ASM_REVENGE_TRAINER_TALK_TYPE_GET  wk1,wk2,wk3

  .macro  _ASM_REVENGE_TRAINER_TALK_TYPE_GET  wk1,wk2,wk3
  .short  EV_SEQ_REVENGE_TRAINER_TALK_TYPE_GET
  .short  \wk1
  .short  \wk2
  .short  \wk3
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_TYPE_GET トレーナータイプ取得
 *  @param ret_wk タイプ格納先ワーク
 */
//--------------------------------------------------------------
#define _TRAINER_TYPE_GET( ret_wk ) \
  _ASM_TRAINER_TYPE_GET  ret_wk

  .macro  _ASM_TRAINER_TYPE_GET  ret_wk
  .short  EV_SEQ_TRAINER_TYPE_GET
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
    //_BLACK_IN() //ここでブラックインを呼ぶ予定
  .endm

  .macro  _ASM_TRAINER_WIN
  .short  EV_SEQ_TRAINER_WIN
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_LOSE トレーナー敗北
 *  @param none
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
 * _WILD_BTL_SET  野生戦呼び出し(非伝説)
 * @param mons_no エンカウントさせたいモンスターNo 
 * @param mons_lv エンカウントさせたいモンスターレベル
 *
 * 非伝説と伝説系ではバトル開始時のエンカウントメッセージが変わります
 * 非伝説「○○が飛び出してきた」 メッセージ変化は091029時点で未実装
 */
//--------------------------------------------------------------
#define _WILD_BTL_SET( mons_no, mons_lv ) \
    _ASM_WILD_BTL_SET mons_no, mons_lv, SCR_WILD_BTL_FLAG_NONE 

//--------------------------------------------------------------
/**
 * _WILD_BTL_SET_REGEND  野生戦呼び出し(伝説系)
 * @param mons_no エンカウントさせたいモンスターNo 
 * @param mons_lv エンカウントさせたいモンスターレベル
 *
 * 非伝説と伝説系ではバトル開始時のエンカウントメッセージが変わります
 * 伝説系「○○が現れた」 メッセージ変化は091029時点で未実装
 */
//--------------------------------------------------------------
#define _WILD_BTL_SET_REGEND( mons_no, mons_lv ) \
    _ASM_WILD_BTL_SET mons_no, mons_lv, SCR_WILD_BTL_FLAG_REGEND

  .macro  _ASM_WILD_BTL_SET mons_no, mons_lv, flags
  .short  EV_SEQ_WILD_BTL_SET
  .short  \mons_no
  .short  \mons_lv
  .short  \flags
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
    //_BLACK_IN() //ここでブラックインを呼ぶ予定
  .endm

  .macro  _ASM_WILD_WIN
  .short  EV_SEQ_WILD_WIN
  .endm

//--------------------------------------------------------------
/**
 * _WILD_LOSE  野生戦敗北処理
 * @param none
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


#if 0
//--------------------------------------------------------------
/**
 *  _SEACRET_POKE_RETRY_CHECK 隠しポケモン再戦可不可チェック
 *  @param ret_wk 結果格納先 
 */
//--------------------------------------------------------------
  .macro  _SEACRET_POKE_RETRY_CHECK ret_wk
  .short  EV_SEQ_SEACRET_POKE_RETRY_CHECK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  _HAIFU_POKE_RETRY_CHECK 配布ポケモン再戦可不可チェック
 *  @param ret_wk 結果格納先
 */
//--------------------------------------------------------------
  .macro  _HAIFU_POKE_RETRY_CHECK ret_wk
  .short  EV_SEQ_HAIFU_POKE_RETRY_CHECK
  .short  \ret_wk
  .endm
#endif

//--------------------------------------------------------------
/**
 *  @def  _2VS2_BATTLE_CHECK
 *  @brief  手持ちチェック 2vs2が可能か取得
 *  @param ret_wk 結果格納先
 *  @return TRUE == 2vs2 ができる
 */
//--------------------------------------------------------------
#define _2VS2_BATTLE_CHECK( ret_wk ) \
    _ASM_2VS2_BATTLE_CHECK ret_wk

  .macro  _ASM_2VS2_BATTLE_CHECK ret_wk
  .short  EV_SEQ_2VS2_BATTLE_CHECK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  _DEBUG_BTL_SET デバック戦闘呼び出し
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _DEBUG_BTL_SET
  .short  EV_SEQ_DEBUG_BTL_SET
  .endm

//--------------------------------------------------------------
/**
 *  _BATTLE_RESULT_GET 戦闘結果を取得
 *  @param ret_wk 結果格納先
 */
//--------------------------------------------------------------
  .macro  _BATTLE_RESULT_GET ret_wk
  .short  EV_SEQ_BATTLE_RESULT_GET
  .short  \ret_wk
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
 *  _BGM_STOP BGMを止める
 *  @param none
 */
//--------------------------------------------------------------
#define _BGM_STOP() _ASM_BGM_STOP

  .macro  _ASM_BGM_STOP
  .short  EV_SEQ_BGM_STOP
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_PLAYER_PAUSE シーケンスを一時停止または再開
 *  @param flag (1=停止、0=再開)
 */
//--------------------------------------------------------------
#define _BGM_PLAYER_PAUSE(flag) _ASM_BGM_PLAYER_PAUSE

  .macro  _ASM_BGM_PLAYER_PAUSE flag
  .short  EV_SEQ_BGM_PLAYER_PAUSE
  .byte  \flag
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_FADEOUT BGMフェードアウト
 *  @param vol    = ボリューム
 *  @param frame  = 何フレームかけてフェードするか
 */
//--------------------------------------------------------------
#define _BGM_FADEOUT(vol, frame)  _ASM_BGM_FADEOUT vol, frame

  .macro  _ASM_BGM_FADEOUT vol,frame
  .short  EV_SEQ_BGM_FADEOUT
  .short  \vol
  .short  \frame
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_FADEIN BGMフェードイン
 *  @param frame = 何フレームかけてフェードするか
 */
//--------------------------------------------------------------
#define _BGM_FADEIN(frame)  _ASM_BGM_FADEIN frame

  .macro  _ASM_BGM_FADEIN frame
  .short  EV_SEQ_BGM_FADEIN
  .short  \frame
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
 * @def _START_EVENT_BGM
 * @brief スクリプト簡易マクロ：イベントBGM開始
 * @param bgmno
 *
 * @note
 * 内部的には _BGM_STOP()と_BGM_PLAY()を続けて呼んでいる
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
 * @def _END_EVENT_BGM
 * @brief スクリプト簡易マクロ：イベントBGM終了
 *
 * @note
 * 内部的には、_BGM_FADEOUT(0,30)→_BGM_STOP() →_BGM_NOW_MAP_RECOVER()を続けて呼んでいる
 */
//--------------------------------------------------------------
#define _END_EVENT_BGM() \
    _ASM_END_EVENT_BGM

  .macro  _ASM_END_EVENT_BGM
  _ASM_BGM_FADEOUT  0, 30
  _ASM_BGM_STOP
  _ASM_BGM_NOW_MAP_RECOVER
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
  .byte  0
  .byte  0
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
  .byte  \cursor
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
 */
//--------------------------------------------------------------
#define _BMPMENU_INIT_EX( x,y,cursor,cancel,ret_wk ) \
   _ASM_BMPMENU_INIT_EX x,y,cursor,cancel,ret_wk

   .macro  _ASM_BMPMENU_INIT_EX x,y,cursor,cancel,ret_wk
  .short  EV_SEQ_BMPMENU_INIT_EX
  .byte  \x
  .byte  \y
  .byte  \cursor
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

//======================================================================
//  アイテム関連
//======================================================================
//--------------------------------------------------------------
/**
 *	アイテムを加える
 *	@param item_no 加えるアイテムナンバー
 *	@param num 加える個数
 *	@param ret_wk 結果代入先 TRUE=成功 FALSE=失敗
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
 *  @param pokeIdx 選択位置
 *  @param mode モード (0:非通信 1:通信
 */
//--------------------------------------------------------------
#define _MUSICAL_CALL( pokeIdx, mode ) \
    _ASM_MUSICAL_CALL pokeIdx, mode

  .macro  _ASM_MUSICAL_CALL pokeIdx, mode
  .short EV_SEQ_MUSICAL_CALL
  .short \pokeIdx
  .byte \mode
  .endm

//--------------------------------------------------------------
/**
 *  _MUSICAL_FITTING_CALL ミュージカル：試着室呼び出し
 *  @param pokeIdx 選択位置
 */
//--------------------------------------------------------------
#define _MUSICAL_FITTING_CALL( pokeIdx ) \
    _ASM_MUSICAL_FITTING_CALL pokeIdx

  .macro  _ASM_MUSICAL_FITTING_CALL pokeIdx
  .short EV_SEQ_MUSICAL_FITTING_CALL
  .short \pokeIdx
  .endm

//--------------------------------------------------------------
/**
 *  _GET_MUSICAL_VALUE ミュージカル：ミュージカル数値取得(汎用
 *      基本的に下にあるラッパーを呼んでください
 *  @param type 種類(参加回数・トップ回数・最終評価点・最終コンディション
 *  @param val  取得用番号 (現在未使用
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
  .short EV_SEQ_GET_MUSICAL_WAITROOM_VALUE
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
 * @brief 季節の取得
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
 * @def _SET_SP_LOCATION_HERE
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
//--------------------------------------------------------------
    .macro  _ASM_SET_CGEAR_FLAG flag
    .short  EV_SEQ_SET_CGEAR_FLAG
    .short  \flag
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
 * @def _CGEAR_ON_DEMO
 * @brief Cギア起動デモ（仮）
 */
//--------------------------------------------------------------
#define _CGEAR_ON_DEMO()  \
    _ASM_CGEAR_ON_DEMO

//--------------------------------------------------------------
/**
 * @def _GOTO_GAMECLEAR_DEMO
 * @brief ゲームクリアデモへの移行
 * @param mode  現在未使用：０を入れておいてください
 */
//--------------------------------------------------------------
#define _GOTO_GAMECLEAR_DEMO( mode ) \
    _ASM_GOTO_GAMECLEAR_DEMO mode

    .macro  _ASM_GOTO_GAMECLEAR_DEMO mode
    .short  EV_SEQ_GOTO_GAMECLEAR
    .short  \mode
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
 * @retval  TRUEのとき、入力した名前を確定した場合
 * @retval  FALSEのとき、入力をキャンセルした場合
 */
//--------------------------------------------------------------
#define _PARTY_POKE_NAME_INPUT( ret_wk, index ) \
    _ASM_PARTY_POKE_NAME_INPUT ret_wk, index

  .macro  _ASM_PARTY_POKE_NAME_INPUT ret_wk, index
  .short  EV_SEQ_PARTY_POKE_NAME_INPUT
  .short  \ret_wk
  .short  \index
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
 * @param tokusei 特性
 * @param level   レベル
 * @param itemno  所持アイテム
 * @param 
 * @retval BOOL 追加できたらTRUE
 */
//--------------------------------------------------------------
#define _ADD_POKEMON_TO_PARTY( ret_wk, monsno, formno, tokusei, level, itemno ) \
    _ASM_ADD_POKEMON_TO_PARTY ret_wk, monsno, formno, tokusei, level, itemno

  .macro _ASM_ADD_POKEMON_TO_PARTY ret_wk, monsno, formno, tokusei, level, itemno
  .short EV_SEQ_ADD_POKEMON_TO_PARTY
  .short \ret_wk
  .short \monsno
  .short \formno
  .short \tokusei
  .short \level
  .short \itemno
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
  .short  EV_SEQ_GET_TEMOTI_NATSUKI
  .short  \ret_wk
  .short  \pos
  .byte   0
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
  .byte   1
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
  .byte   2
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
 * @def _ADD_TEMOTI_WAZA
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
 * @def _GET_PARTY_POS_BY_MONSNO
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
  .short  EV_SEQ_GET_TEMOTI_GETPLACE
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
//======================================================================
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
 * ジムコマンド　ノーマルジム初期化
 * @param rm_idx   ギミックのある部屋インデックス0～1
 */
//--------------------------------------------------------------
#define _GYM_NORMAL_INIT() \
    _ASM_GYM_NORMAL_INIT 

  .macro  _ASM_GYM_NORMAL_INIT
  .short  EV_SEQ_GYM_NORMAL_INIT
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
 */
//--------------------------------------------------------------
#define _GYM_ANTI_INIT() \
    _ASM_GYM_ANTI_INIT

  .macro  _ASM_GYM_ANTI_INIT
  .short  EV_SEQ_GYM_ANTI_INIT
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
#define _GYM_INSECT_INIT() \
    _ASM_GYM_INSECT_INIT

  .macro  _ASM_GYM_INSECT_INIT
  .short  EV_SEQ_GYM_INSECT_INIT
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
 */
//--------------------------------------------------------------
#define _POKECEN_PC_OFF()  \
  _ASM_POKECEN_PC_OFF

  .macro  _ASM_POKECEN_PC_OFF
  .short  EV_SEQ_POKECEN_PC_OFF
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
 *  バッグプロセスを呼び出す
 *  @param  mode  呼び出しモード script_def.h SCR_BAGMODE_SELL他
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
#define _CALL_MAILBOX_PROC() \
    _ASM_CALL_MAILBOX_PROC

  .macro  _ASM_CALL_MAILBOX_PROC
  .short  EV_SEQ_CALL_MAILBOX_PROC
  .endm

//======================================================================
//  ショップ関連
//======================================================================
//--------------------------------------------------------------
/**
 * @def _SHOP_CALL
 * @brief 簡易イベントコマンド：ショップイベント呼び出し
 * @param shop_id
 *
 * ＊GREETING_LESSは話しかけ時の「いらっしゃいませ」の挨拶無し
 */
//--------------------------------------------------------------
#define _SHOP_CALL( ) _ASM_SHOP_CALL  SCR_SHOPID_NULL,TRUE 
#define _SHOP_CALL_GREETING_LESS( ) _ASM_SHOP_CALL  SCR_SHOPID_NULL,FALSE

//--------------------------------------------------------------
/**
 * @def _SHOP_CALL
 * @brief 簡易イベントコマンド：固定ショップイベント呼び出し
 * @param shop_id
 */
//--------------------------------------------------------------
#define _FIX_SHOP_CALL( shop_id ) _ASM_SHOP_CALL shop_id,TRUE
#define _FIX_SHOP_CALL_GREETING_LESS( shop_id ) _ASM_SHOP_CALL shop_id,FALSE

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
 * フラッシュ開始
 * @param none
 */
//--------------------------------------------------------------
#define _FLASH() _ASM_FLASH
  
  .macro _ASM_FLASH
  .short EV_SEQ_FLASH
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

//======================================================================
//
//  簡易コマンド
//  ※スクリプトコマンドを組み合わせたもの
//
//======================================================================
.include  "easy_event_def.h"

//--------------------------------------------------------------
/**
 * 簡易メッセージ表示 BG用	！！！！！！使用禁止！！！！！！！
 	* _EASY_BGWIN_MSG()に置き換えていきます。無くなり次第削除予定
 * @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
#define _EASY_MSG( msg_id ) \
  _ASM_EASY_MSG msg_id

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
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_KEYWAIT(itemno, num)   \
    _ASM_ITEM_EVENT_KEYWAIT itemno, num

//--------------------------------------------------------------
/**
 * イベント入手イベント「▼」待ちなし
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_NOWAIT(itemno, num)   \
    _ASM_ITEM_EVENT_NOWAIT itemno, num

//--------------------------------------------------------------
/**
 * イベント入手フィールドイベント「▼」待ちなし
 */
//--------------------------------------------------------------
#define _ITEM_EVENT_FIELD(itemno, num)   \
    _ASM_ITEM_EVENT_FIELD itemno, num

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
 * 侵入コマンド　ミニモノリス設置
 */
//--------------------------------------------------------------
#define _INTRUDE_MINIMONO_SETTING() _ASM_INTRUDE_MINIMONO_SETTING

  .macro  _ASM_INTRUDE_MINIMONO_SETTING
  .short  EV_SEQ_INTRUDE_MINIMONO_SETTING
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
 * 侵入コマンド　ミッション開始
 */
//--------------------------------------------------------------
#define _INTRUDE_MISSION_START() _ASM_INTRUDE_MISSION_START

  .macro  _ASM_INTRUDE_MISSION_START
  .short  EV_SEQ_INTRUDE_MISSION_START
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
#define _INTRUDE_MISSION_CHOICE_LIST_REQ() _ASM_INTRUDE_MISSION_CHOICE_LIST_REQ

  .macro  _ASM_INTRUDE_MISSION_CHOICE_LIST_REQ
  .short  EV_SEQ_INTRUDE_MISSION_CHOICE_LIST_REQ
  .endm

//--------------------------------------------------------------
/**
 * 侵入コマンド　シンボルポケモンセッティング
 */
//--------------------------------------------------------------
#define _SYMBOL_POKE_SET() _ASM_SYMBOL_POKE_SET

  .macro  _ASM_SYMBOL_POKE_SET
  .short  EV_SEQ_SYMBOL_POKE_SET
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
#define _PALACE_IN_DISGUISE() _ASM_PALACE_IN_DISGUISE

  .macro  _ASM_PALACE_IN_DISGUISE
  .short  EV_SEQ_PALACE_IN_DISGUISE
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
 */
//--------------------------------------------------------------
#define _PL_BOAT_ADD_TIME(add_time) _ASM_PL_BOAT_ADD_TIME add_time

  .macro  _ASM_PL_BOAT_ADD_TIME add_time
  .short  EV_SEQ_PL_BOAT_ADD_TIME
  .short  \add_time
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

//--------------------------------------------------------------
/**
 * マップフェードブラックイン
 */
//--------------------------------------------------------------
#define _MAP_FADE_BLACK_IN() _ASM_MAP_FADE_BLACK_IN

  .macro  _ASM_MAP_FADE_BLACK_IN
  .short  EV_SEQ_MAP_FADE_BLACK_IN
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
 * マップフェードブラックアウト
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
 *  _DISP_FADE_CHECK　マップ遷移用フェード終了チェック
 *  @param none
 */
//--------------------------------------------------------------
#define _MAP_FADE_END_CHECK()  _ASM_MAP_FADE_END_CHECK

  .macro  _ASM_MAP_FADE_END_CHECK
  .short EV_SEQ_MAP_FADE_CHECK
  .endm


//--------------------------------------------------------------
/**
 * マップフェードＢＧ初期化　アプリからNOFADEで別マップに飛ぶときなどに使用する
 */
//--------------------------------------------------------------
#define _MAP_FADE_INT_BG() _ASM_MAP_FADE_INIT_BG

  .macro  _ASM_MAP_FADE_INIT_BG
  .short  EV_SEQ_MAP_FADE_INIT_BG
  .endm
 
//======================================================================
//  3Dデモ
//======================================================================
//--------------------------------------------------------------
/**
 * @def _CALL_3D_DEMO
 * @brief デモ呼び出し
 * @param demo_no   デモID指定ナンバー
 *
 * @note  フェード処理は別途呼び出す必要がある
 */
//--------------------------------------------------------------
#define _CALL_3D_DEMO( demo_no )  \
    _ASM_CALL_3D_DEMO demo_no

  .macro  _ASM_DEMO_SCENE demo_no
  .short  EV_SEQ_DEMO_SCENE
  .short  \demo_no
  .endm
  
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
 * @param move_poke    結果を受け取るワーク
 */
//--------------------------------------------------------------
#define _ADD_MOVE_POKEMON( move_poke )   \
    _ASM_ADD_MOVE_POKEMON move_poke

    .macro  _ASM_ADD_MOVE_POKEMON move_poke
    .short  EV_SEQ_ADD_MOVE_POKEMON
    .short  \move_poke
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
#define _BSUBWAY_TOOL( cmd, param, ret_wk ) \
    _ASM_BSUBWAY_TOOL cmd, param, ret_wk

  .macro  _ASM_BSUBWAY_TOOL cmd, param, ret_wk
  .short  EV_SEQ_BSUBWAY_TOOL
  .short \cmd
  .short \param
  .short \ret_wk
  .endm

 
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
 *  _GET_PALPARK_VALUE 
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
 */
//--------------------------------------------------------------
#define _WFBC_SET_WORDSET_RIREKI_PLAYER_NAME() \
    _ASM_WFBC_SET_WORDSET_RIREKI_PLAYER_NAME

  .macro  _ASM_WFBC_SET_WORDSET_RIREKI_PLAYER_NAME
  .short EV_SEQ_WFBC_SET_WORDSET_RIREKI_PLAYER_NAME
  .endm

//--------------------------------------------------------------
/**
 *  _WFBC_ADD_BC_NPC_WIN_NUM
 *  @brief WFBC:BC NPC勝利数を加算
 */
//--------------------------------------------------------------
#define _WFBC_ADD_BC_NPC_WIN_NUM() \
    _ASM_WFBC_ADD_BC_NPC_WIN_NUM

  .macro  _ASM_WFBC_ADD_BC_NPC_WIN_NUM
  .short EV_SEQ_WFBC_ADD_BC_NPC_WIN_NUM
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
 * @brief ジェットバッジチェックゲートギミック初期化 
 */
//--------------------------------------------------------------
#define _JET_BADGE_GATE_GIMMICK_INIT( ) \
    _ASM_JET_BADGE_GATE_GIMMICK_INIT

  .macro  _ASM_JET_BADGE_GATE_GIMMICK_INIT
  .short  EV_SEQ_JET_BADGE_GATE_GIMMICK_INIT
  .endm

