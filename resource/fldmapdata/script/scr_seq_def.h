//======================================================================
/**
 * @file  scr_seq_def.def
 * @bfief  スクリプトコマンドのマクロ定義ファイル
 * @author  HisashiSogabe
 * @date  2005.05.24
 *
 * 05.08.04 Satoshi Nohara
 *
 * 技シーケンス用のマクロ定義ファイルを元にしている
 */
//======================================================================
#define  __ASM_NO_DEF_

//スクリプトで使用するヘッダーファイル
  .include  "usescript.h"
//  .include  "..\..\..\include\pl_bugfix.h"

//データを自然な境界にアラインするかの設定
  .option alignment off

//======================================================================
//  macro
//======================================================================
  .macro  INIT_CMD
DEF_CMD_COUNT  =  0
  .endm
    
  .macro  DEF_CMD symname
\symname  =  DEF_CMD_COUNT
DEF_CMD_COUNT  =  ( DEF_CMD_COUNT + 1 )
  .endm

//======================================================================
//  命令シンボル宣言
//======================================================================
  INIT_CMD
  //基本システム命令
  DEF_CMD EV_SEQ_NOP
  DEF_CMD EV_SEQ_DUMMY
  DEF_CMD EV_SEQ_END
  DEF_CMD EV_SEQ_TIME_WAIT
  DEF_CMD EV_SEQ_CALL
  DEF_CMD EV_SEQ_RET
  
  //データロード・ストア関連
  DEF_CMD EV_SEQ_LD_REG_VAL
  DEF_CMD EV_SEQ_LD_REG_WDATA
  DEF_CMD EV_SEQ_LD_REG_ADR
  DEF_CMD EV_SEQ_LD_ADR_VAL
  DEF_CMD EV_SEQ_LD_ADR_REG
  DEF_CMD EV_SEQ_LD_REG_REG
  DEF_CMD EV_SEQ_LD_ADR_ADR
  
  //比較命令
  DEF_CMD EV_SEQ_CP_REG_REG
  DEF_CMD EV_SEQ_CP_REG_VAL
  DEF_CMD EV_SEQ_CP_REG_ADR
  DEF_CMD EV_SEQ_CP_ADR_REG
  DEF_CMD EV_SEQ_CP_ADR_VAL
  DEF_CMD EV_SEQ_CP_ADR_ADR
  DEF_CMD EV_SEQ_CP_WK_VAL
  DEF_CMD EV_SEQ_CP_WK_WK
  
  //仮想マシン関連
  DEF_CMD EV_SEQ_VM_ADD
  DEF_CMD EV_SEQ_CHG_COMMON_SCR
  DEF_CMD EV_SEQ_CHG_LOCAL_SCR
  
  //分岐命令
  DEF_CMD EV_SEQ_JUMP
  DEF_CMD EV_SEQ_IF_JUMP
  DEF_CMD EV_SEQ_IF_CALL
  DEF_CMD EV_SEQ_OBJ_ID_JUMP
  DEF_CMD EV_SEQ_BG_ID_JUMP
  DEF_CMD EV_SEQ_PLAYER_DIR_JUMP
  
  //イベントワーク関連
  DEF_CMD EV_SEQ_FLAG_SET
  DEF_CMD EV_SEQ_FLAG_RESET
  DEF_CMD EV_SEQ_FLAG_CHECK
  DEF_CMD EV_SEQ_FLAG_CHECK_WK
  DEF_CMD EV_SEQ_FLAG_SET_WK
  
  //ワーク操作関連
  DEF_CMD EV_SEQ_ADD_WK
  DEF_CMD EV_SEQ_SUB_WK
  DEF_CMD EV_SEQ_LD_WK_VAL
  DEF_CMD EV_SEQ_LD_WK_WK
  DEF_CMD EV_SEQ_LD_WK_WKVAL
  
  //キー入力関連
  DEF_CMD EV_SEQ_ABKEYWAIT
  
  //フィールド　メッセージ表示
  DEF_CMD EV_SEQ_TALKMSG_ALLPUT
  
  //フィールド　メッセージウィンドウ
  DEF_CMD EV_SEQ_TALKWIN_OPEN
  DEF_CMD EV_SEQ_TALKWIN_CLOSE
  
  //動作モデル
  DEF_CMD EV_SEQ_OBJ_ANIME      
  DEF_CMD EV_SEQ_OBJ_ANIME_WAIT
  DEF_CMD EV_SEQ_MOVE_CODE_GET
  DEF_CMD EV_SEQ_OBJ_POS_GET
  DEF_CMD EV_SEQ_PLAYER_POS_GET
  DEF_CMD EV_SEQ_OBJ_ADD
  DEF_CMD EV_SEQ_OBJ_DEL

  //動作モデル　イベント関連
  DEF_CMD EV_SEQ_OBJ_PAUSE_ALL
  DEF_CMD EV_SEQ_TALK_OBJ_PAUSE_ALL
  DEF_CMD EV_SEQ_OBJ_PAUSE_CLEAR_ALL
  DEF_CMD EV_SEQ_OBJ_TURN
  
  //はい、いいえ　処理
  DEF_CMD EV_SEQ_YES_NO_WIN
  
  //WORDSET関連
  DEF_CMD EV_SEQ_PLAYER_NAME
  
  //視線トレーナー関連
  DEF_CMD EV_SEQ_EYE_TRAINER_MOVE_SET
  DEF_CMD EV_SEQ_EYE_TRAINER_MOVE_SINGLE
  DEF_CMD EV_SEQ_EYE_TRAINER_MOVE_DOUBLE
  DEF_CMD EV_SEQ_EYE_TRAINER_TYPE_GET
  DEF_CMD EV_SEQ_EYE_TRAINER_ID_GET
  
  //トレーナーバトル関連
  DEF_CMD EV_SEQ_TRAINER_ID_GET
  DEF_CMD EV_SEQ_TRAINER_BTL_SET
  DEF_CMD EV_SEQ_TRAINER_MULTI_BTL_SET
  DEF_CMD EV_SEQ_TRAINER_MSG_SET
  DEF_CMD EV_SEQ_TRAINER_TALK_TYPE_GET
  DEF_CMD EV_SEQ_REVENGE_TRAINER_TALK_TYPE_GET
  DEF_CMD EV_SEQ_TRAINER_TYPE_GET
  DEF_CMD EV_SEQ_TRAINER_BGM_SET
  DEF_CMD EV_SEQ_LOSE
  DEF_CMD EV_SEQ_LOSE_CHECK
  DEF_CMD EV_SEQ_SEACRET_POKE_RETRY_CHECK
  DEF_CMD EV_SEQ_HAIFU_POKE_RETRY_CHECK
  DEF_CMD EV_SEQ_2VS2_BATTLE_CHECK
  DEF_CMD EV_SEQ_DEBUG_BTL_SET
  DEF_CMD EV_SEQ_BATTLE_RESULT_GET
  
  //トレーナーフラグ関連
  DEF_CMD EV_SEQ_TRAINER_FLAG_SET
  DEF_CMD EV_SEQ_TRAINER_FLAG_RESET
  DEF_CMD EV_SEQ_TRAINER_FLAG_CHECK
  
  //BGM
  DEF_CMD EV_SEQ_BGM_PLAY
  DEF_CMD EV_SEQ_BGM_STOP
  DEF_CMD EV_SEQ_BGM_PLAYER_PAUSE
  DEF_CMD EV_SEQ_BGM_PLAY_CHECK
  DEF_CMD EV_SEQ_BGM_FADEOUT
  DEF_CMD EV_SEQ_BGM_FADEIN
  DEF_CMD EV_SEQ_BGM_NOW_MAP_PLAY
  
  //SE
  DEF_CMD EV_SEQ_SE_PLAY
  DEF_CMD EV_SEQ_SE_STOP
  DEF_CMD EV_SEQ_SE_WAIT
  
  //ME
  DEF_CMD EV_SEQ_ME_PLAY
  DEF_CMD EV_SEQ_ME_WAIT
  
  //メニュー
  DEF_CMD EV_SEQ_BMPMENU_INIT
  DEF_CMD EV_SEQ_BMPMENU_INIT_EX
  DEF_CMD EV_SEQ_BMPMENU_MAKE_LIST
  DEF_CMD EV_SEQ_BMPMENU_START

  //その他
  DEF_CMD EV_SEQ_CHG_LANGID
  DEF_CMD EV_SEQ_GET_RND

//======================================================================
// イベントデータ関連
//======================================================================
#define EV_DATA_END_CODE  (0xfd13)  //適当な値

//--------------------------------------------------------------
/**
 *  _EVENT_DATA イベントデータ　テーブル宣言
 *  @param adrs 宣言するデータラベルアドレス
 */
//--------------------------------------------------------------
  .macro  _EVENT_DATA  adrs
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _EVENT_DATA_END イベントデータ　テーブル宣言終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _EVENT_DATA_END
  .short  EV_DATA_END_CODE
  .endm

//--------------------------------------------------------------
/**
 *  _SP_EVENT_DATA_END 特殊スクリプト終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _SP_EVENT_DATA_END
  .byte   SP_SCRID_NONE
  .endm

//======================================================================
//  基本コマンド
//======================================================================
//--------------------------------------------------------------
/**
 *  _NOP 何もしない
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _NOP
  .short  EV_SEQ_NOP
  .endm

//--------------------------------------------------------------
/**
 *  _DUMMY ダミー
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _DUMMY
  .short  EV_SEQ_DUMMY
  .endm

//--------------------------------------------------------------
/**
 *  _END スクリプトの終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _END
  .short  EV_SEQ_END
  .endm

//--------------------------------------------------------------
/**
 *  _TIME_WAIT ウェイト
 *  @param time 待ちフレーム数
 *  @param ret_wk 待ち時間格納先ワークID
 */
//--------------------------------------------------------------
  .macro  _TIME_WAIT time,ret_wk
  .short  EV_SEQ_TIME_WAIT
  .short  \time
  .short  \ret_wk
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
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _RET
  .short  EV_SEQ_RET
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

//--------------------------------------------------------------
/**
 *  _OBJ_ID_JUMP 話しかけOBJのIDを比較しジャンプ
 *  @param act 比較するOBJID
 *  @param adrs 一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _OBJ_ID_JUMP act,adrs
  .short  EV_SEQ_OBJ_ID_JUMP
  .byte  \act
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _BG_ID　話し掛けBGのIDを比較しジャンプ
 *  @param act 比較するBGID
 *  @param adrs 一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _BG_ID_JUMP act,adrs
  .short  EV_SEQ_BG_ID_JUMP
  .byte  \act
  .long  ((\adrs-.)-4)
  .endm

//--------------------------------------------------------------
/**
 *  _PLAYER_DIR_JUMP イベント起動時の主人公の向き比較ジャンプ
 *  (現在の向きではないので注意！)
 *  @param dir 方向 DIR_UP等
 *  @param adrs 一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _PLAYER_DIR_JUMP dir,adrs
  .short  EV_SEQ_PLAYER_DIR_JUMP
  .byte  \dir
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
  .macro  _FLAG_SET num
  .short  EV_SEQ_FLAG_SET
  .short  \num
  .endm

//--------------------------------------------------------------
/*
 *  _ARRIVE_FLAG_SET 到着フラグセット
 * @param num セットするフラグナンバー
 */
//--------------------------------------------------------------
  .macro  _ARRIVE_FLAG_SET num
  _FLAG_SET  (\num + SYS_FLAG_ARRIVE_START)
  .endm

//--------------------------------------------------------------
/**
 *  _FLAG_RESET フラグのリセット
 *  @param リセットするフラグナンバー
 */
//--------------------------------------------------------------
  .macro  _FLAG_RESET num
  .short  EV_SEQ_FLAG_RESET
  .short  \num
  .endm

//--------------------------------------------------------------
/**
 *  _FLAG_CHECK フラグチェック
 *  @param チェックするフラグナンバー
 */
//--------------------------------------------------------------
  .macro  _FLAG_CHECK num
  .short  EV_SEQ_FLAG_CHECK
  .short  \num
  .endm

//--------------------------------------------------------------
/*
 *  _IF_FLAGON_JUMP フラグONの時に分岐(JUMP)
 *  @param num チェックするフラグナンバー
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_FLAGON_JUMP num,adrs
  _FLAG_CHECK \num
  _IF_JUMP  FLGON,\adrs
  .endm

//--------------------------------------------------------------
/**
 *  _IF_FLAGOFF_JUMP フラグOFFの時に分岐(JUMP)
 *  @param num チェックするフラグナンバー
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------

  .macro  _IF_FLAGOFF_JUMP num,adrs
  _FLAG_CHECK \num
  _IF_JUMP  FLGOFF,\adrs
  .endm

//--------------------------------------------------------------
/**
 *  _IF_FLAGON_CALL フラグONの時に分岐(CALL)
 *  @param num チェックするフラグナンバー
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_FLAGON_CALL num,adrs
  _FLAG_CHECK \num
  _IF_CALL  FLGON,\adrs
  .endm

//--------------------------------------------------------------
/**
 *  _IF_FLAGOFF_CALL フラグOFFの時に分岐(CALL)
 *  @param num チェックするフラグナンバー
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_FLAGOFF_CALL num,adrs
  _FLAG_CHECK \num
  _IF_CALL  FLGOFF,\adrs
  .endm

//--------------------------------------------------------------
/**
 * _FLAG_CHECK_WK ワークの値をフラグナンバーとしてチェックする！
 * @param wk チェックする値が格納されたワーク
 * @param ret_wk 結果を格納するワーク
 */
//--------------------------------------------------------------
  .macro  _FLAG_CHECK_WK wk,ret_wk
  .short  EV_SEQ_FLAG_CHECK_WK
  .short  \wk
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * _FLAG_SET_WK ワークの値をフラグナンバーとしてセットする！
 * @param wk フラグナンバーが格納されたワーク
 */
//--------------------------------------------------------------
  .macro  _FLAG_SET_WK wk
  .short  EV_SEQ_FLAG_SET_WK
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
  .macro  _ADD_WK wk,num
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
  .macro  _SUB_WK wk,num
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
  .macro  _LDVAL  wk,val
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
  .macro  _LDWK  wk1,wk2
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
  .macro  _LDWKVAL  wk1,wk2
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
  .macro  _AB_KEYWAIT
  .short  EV_SEQ_ABKEYWAIT
  .endm

//======================================================================
//  フィールド　メッセージ表示
//======================================================================
//--------------------------------------------------------------
/**
 *  _TALKMSG_ALLPUT 展開メッセージを表示(1byte)
 *  @param  msg_id  表示するメッセージID
 */
//--------------------------------------------------------------
  .macro  _TALKMSG_ALLPUT msg_id
  .short  EV_SEQ_TALKMSG_ALLPUT
  .byte  \msg_id
  .endm

//======================================================================
//  フィールド　メッセージウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 *  _TALKWIN_OPEN 会話ウィンドウ開く
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TALKWIN_OPEN
  .short  EV_SEQ_TALKWIN_OPEN
  .endm

//--------------------------------------------------------------
/**
 *  _TALKWIN_CLOSE 会話ウィンドウ閉じる
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TALKWIN_CLOSE
  .short  EV_SEQ_TALKWIN_CLOSE
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
  .macro  _OBJ_ANIME  obj_id,adrs
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
  .macro  _OBJ_ANIME_WAIT
  .short  EV_SEQ_OBJ_ANIME_WAIT
  .endm
  
//--------------------------------------------------------------
/**
 *  _MOVE_CODE_GET 動作コード取得(fldmmdl_code.h)
 *  @param ret_wk 動作コードを格納するワーク
 *  @param obj_id 動作コードを取得するOBJ ID
 */
//--------------------------------------------------------------
  .macro  _MOVE_CODE_GET  ret_wk,obj_id
  .short  EV_SEQ_MOVE_CODE_GET
  .short  \ret_wk
  .short  \obj_id
  .endm

//--------------------------------------------------------------
/**
 * _OBJ_POS_GET OBJの位置取得
 * @parma obj_id 取得するOBJ ID
 * @param x X座標格納先
 * @param z Z座標格納先
 */
//--------------------------------------------------------------
   .macro  _OBJ_POS_GET  obj_id,x,z
  .short  EV_SEQ_OBJ_POS_GET
  .short  \obj_id
  .short  \x
  .short  \z
  .endm 

//--------------------------------------------------------------
/**
 * _PLAYER_POS_GET 主人公の位置取得
 * @param x X座標格納先
 * @param z Z座標格納先
 */
//--------------------------------------------------------------
  .macro  _PLAYER_POS_GET  x,z
  .short  EV_SEQ_PLAYER_POS_GET
  .short  \x
  .short  \z
  .endm
 
//--------------------------------------------------------------
/**
 * _OBJ_ADD OBJを追加
 * @param x 表示するグリッドX座標
 * @param z 表示するグリッドZ座標
 * @param dir 方向 DIR_UP等
 * @param id 識別用OBJ ID
 * @param code 表示コードBOY1等
 * @param move 動作コードMV_RND等
 */
//--------------------------------------------------------------
  .macro _OBJ_ADD x,z,dir,id,code,move
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
 * _OBJ_ADD OBJを削除
 * @param id 削除するOBJ ID
 */
//--------------------------------------------------------------
  .macro _OBJ_DEL id
  .short EV_SEQ_OBJ_DEL
  .short \id
  .endm

//======================================================================
//  動作モデル　イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _OBJ_PAUSE_ALL フィールドOBJ動作停止
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _OBJ_PAUSE_ALL
  .short  EV_SEQ_OBJ_PAUSE_ALL
  .endm

//--------------------------------------------------------------
/**
 *  _TALK_OBJ_PAUSE_ALL OBJ会話イベント用　動作停止
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TALK_OBJ_PAUSE_ALL
  .short  EV_SEQ_TALK_OBJ_PAUSE_ALL
  .endm

//--------------------------------------------------------------
/**
 *  _OBJ_PAUSE_CLEAR_ALL OBJ動作再開
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _OBJ_PAUSE_CLEAR_ALL
  .short  EV_SEQ_OBJ_PAUSE_CLEAR_ALL
  .endm

//--------------------------------------------------------------
/**
 *  _TURN_HERO_SITE 話しかけたOBJを自機方向へ振り向き
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TURN_HERO_SITE
  .short  EV_SEQ_OBJ_TURN
  .endm

//--------------------------------------------------------------
/**
 *  _TALK_OBJ_START OBJに対して話し掛け開始(主人公に対して振り向き有り)
 *  @param none
 */
//--------------------------------------------------------------
  .macro _TALK_OBJ_START
//  _SE_PLAY SEQ_SE_DP_SELECT
//  _SE_PLAY SEQ_SE_DP_SELECT11
//  _SE_PLAY SEQ_SE_DP_TALK2
  _OBJ_PAUSE_ALL
  _TURN_HERO_SITE
  .endm

//--------------------------------------------------------------
/**
 *  _EVENT_START POS,SCENE_CHANGE_LABELに対して開始
 *  (TALK_STARTを使用すると、会話開始の音がなってしまうので分けた)
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _EVENT_START
  _OBJ_PAUSE_ALL
  .endm

//--------------------------------------------------------------
/**
 *  _EVENT_END POS,SCENE_CHANGE_LABELに対して終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _EVENT_END
  _OBJ_PAUSE_CLEAR_ALL
  .endm

//--------------------------------------------------------------
/**
 *  _TALK_OBJ_START_TURN_NOT OBJに対して話し掛け開始
 *  (主人公に対して振り向き無し)
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TALK_OBJ_START_TURN_NOT
//  _SE_PLAY SEQ_SE_DP_SELECT
//  _SE_PLAY SEQ_SE_DP_SELECT11
  _OBJ_PAUSE_ALL
  .endm

//--------------------------------------------------------------
/**
 *  _TALK_OBJ_END OBJに対して話し掛け終了
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TALK_OBJ_END
  _OBJ_PAUSE_CLEAR_ALL
  .endm

//======================================================================
// switch関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _SWITCH switch ～ case 構文
 *  スクリプト制御ワークで確保されるワーク(SCWK_REG0)を使用しているので、
 *  FLAG_CHANGE,INIT_CHANGEなどでは使用できません！
 *  SCENE_CHANGEは使用できます。
 *
 *  @param wk 参照するワーク
 *
 *  _SWITCH SCWK_ANSWER
 *  _CASE_JUMP 0,jump0
 *  _CASE_JUMP 1,jump1
 *  _CASE_JUMP 2,jump2
 *  :
 */
//--------------------------------------------------------------
  .macro  _SWITCH wk
  _LDWK  SCWK_REG0,\wk
  .endm

//--------------------------------------------------------------
/**
 *  _CASE_JUMP switch ～ case 構文
 *  スクリプト制御ワークで確保されるワーク(SCWK_REG0)を使用しているので、
 *  FLAG_CHANGE,INIT_CHANGEなどでは使用できません！
 *  SCENE_CHANGEは使用できます。
 *
 *  @param val _SWITCHで参照した引数wkと比較する値
 *  @param adr valと一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _CASE_JUMP  val,adr
  _CMPVAL  SCWK_REG0,\val
  _IF_JUMP  EQ,\adr
  .endm

#if 0 //wb null
//スクリプトウィンドウ表示中にBキャンセル
  .macro  _CASE_CANCEL  adr
  _CMPVAL  SCWK_REG0,EV_WIN_B_CANCEL
  _IF_JUMP  EQ,\adr
  .endm

//待機アイコン表示
  .macro  _ADD_WAITICON
  .short  EV_SEQ_WAITICON_ADD
  .endm

//待機アイコン消去
  .macro  _DEL_WAITICON
  .short  EV_SEQ_WAITICON_DEL
  .endm
#endif

//======================================================================
//  はい、いいえ　処理
//======================================================================
//--------------------------------------------------------------
/**
 *  _YES_NO_WIN はい、いいえ選択
 *  @param ret_wk はい、いいえ選択結果格納先
 */
//--------------------------------------------------------------
  .macro  _YES_NO_WIN ret_wk
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
  .macro  _PLAYER_NAME  idx
  .short  EV_SEQ_PLAYER_NAME
  .byte  \idx
  .endm

//======================================================================
//  視線トレーナー関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _EYE_TRAINER_MOVE_SET 視線：トレーナー移動呼び出し
 *  @param pos 移動させるトレーナー データ番号0,1
 */
//--------------------------------------------------------------
  .macro  _EYE_TRAINER_MOVE_SET  pos
  .short  EV_SEQ_EYE_TRAINER_MOVE_SET
  .short  \pos
  .endm
 
//--------------------------------------------------------------
/**
 *  _EYE_TRAINER_MOVE_SET 視線：トレーナー移動　シングル
 *  @param pos 移動させるトレーナー データ番号0,1
 */
//--------------------------------------------------------------
  .macro  _EYE_TRAINER_MOVE_SINGLE pos
  .short  EV_SEQ_EYE_TRAINER_MOVE_SINGLE
  .short  \pos
  .endm
 
//--------------------------------------------------------------
/**
 *  _EYE_TRAINER_MOVE_SET 視線：トレーナー移動　ダブル
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _EYE_TRAINER_MOVE_DOUBLE
  .short  EV_SEQ_EYE_TRAINER_MOVE_DOUBLE
  .endm

//--------------------------------------------------------------
/**
 *  _EYE_TRAINER_TYPE_GET 視線：トレーナータイプ取得
 *  @param ret_wk トレーナータイプ格納先 SCR_EYE_TR_TYPE_SINGLE等が格納される
 */
//--------------------------------------------------------------
  .macro  _EYE_TRAINER_TYPE_GET ret_wk
  .short  EV_SEQ_EYE_TRAINER_TYPE_GET
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  視線：トレーナーID取得
 */
//--------------------------------------------------------------
  .macro  _EYE_TRAINER_ID_GET  pos,ret_wk
  .short  EV_SEQ_EYE_TRAINER_ID_GET
  .short  \pos
  .short  \ret_wk
  .endm

//======================================================================
//  トレーナーバトル関連
//======================================================================
//--------------------------------------------------------------
/**
 *  _TRAINER_ID_GET トレーナーID取得
 *  @param wk ID格納先ワーク
 */
//--------------------------------------------------------------
  .macro  _TRAINER_ID_GET  wk
  .short  EV_SEQ_TRAINER_ID_GET
  .short  \wk
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_BTL_SET トレーナー戦闘呼び出し
 *  @param tr_id_0 トレーナーID0
 *  @param tr_id_1 トレーナーID1
 */
//--------------------------------------------------------------
  .macro  _TRAINER_BTL_SET  tr_id_0,tr_id_1
  .short  EV_SEQ_TRAINER_BTL_SET
  .short  \tr_id_0
  .short  \tr_id_1
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_MULTI_BTL_SET トレーナー戦闘（マルチバトル）呼び出し
 *  @param partner_id パートナーID
 *  @param tr_id_0 トレーナーID0
 *  @param tr_id_1 トレーナーID1
 */
//--------------------------------------------------------------
  .macro  _TRAINER_MULTI_BTL_SET  partner_id, tr_id_0, tr_id_1
  .short  EV_SEQ_TRAINER_MULTI_BTL_SET
  .short  \partner_id
  .short  \tr_id_0
  .short  \tr_id_1
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_MSG_SET トレーナー会話呼び出し
 *  @param tr_id メッセージ表示用トレーナーID
 *  @param kind_id 会話種類
 */
//--------------------------------------------------------------
  .macro  _TRAINER_MSG_SET  tr_id,kind_id
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
  .macro  _TRAINER_TALK_TYPE_GET  wk1,wk2,wk3
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
  .macro  _REVENGE_TRAINER_TALK_TYPE_GET  wk1,wk2,wk3
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
  .macro  _TRAINER_TYPE_GET  ret_wk
  .short  EV_SEQ_TRAINER_TYPE_GET
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_BGM_SET トレーナーBGMセット
 *  @param tr_id トレーナーID
 */
//--------------------------------------------------------------
  .macro  _TRAINER_BGM_SET  tr_id
  .short  EV_SEQ_TRAINER_BGM_SET
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_LOSE トレーナー敗北
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _TRAINER_LOSE
  .short  EV_SEQ_LOSE
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_LOSE_CHECK トレーナー敗北チェック
 *  @param ret_wk 結果格納先 0=敗北 1=勝利
 */
//--------------------------------------------------------------
  .macro  _TRAINER_LOSE_CHECK ret_wk
  .short  EV_SEQ_LOSE_CHECK
  .short  \ret_wk
  .endm

//--------------------------------------------------------------
/**
 * _NORMAL_LOSE  敗北処理
 * @param none
 */
//--------------------------------------------------------------
  .macro  _NORMAL_LOSE
  .short  EV_SEQ_LOSE
  .endm

//--------------------------------------------------------------
/**
 *  _LOSE_CHECK 敗北チェック
 *  @param ret_wk 結果格納先 0=敗北 1=勝利
 */
//--------------------------------------------------------------
  .macro  _LOSE_CHECK  ret_wk
  .short  EV_SEQ_LOSE_CHECK
  .short  \ret_wk
  .endm

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

//--------------------------------------------------------------
/**
 *  _2VS2_BATTLE_CHECK 手持ちチェック 2vs2が可能か取得
 *  @param ret_wk 結果格納先
 */
//--------------------------------------------------------------
  .macro  _2VS2_BATTLE_CHECK ret_wk
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
  .macro  _TRAINER_FLAG_SET tr_id
  .short  EV_SEQ_TRAINER_FLAG_SET
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_FLAG_RESET トレーナーフラグのリセット
 *  @param tr_id リセットするトレーナーID
 */
//--------------------------------------------------------------
  .macro  _TRAINER_FLAG_RESET tr_id
  .short  EV_SEQ_TRAINER_FLAG_RESET
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 *  _TRAINER_FLAG_CHECK トレーナーフラグチェック
 *  @param tr_id チェックするトレーナーID
 */
//--------------------------------------------------------------
  .macro  _TRAINER_FLAG_CHECK tr_id
  .short  EV_SEQ_TRAINER_FLAG_CHECK
  .short  \tr_id
  .endm

//--------------------------------------------------------------
/**
 *  _IF_TR_FLAGON_JUMP フラグONの時に分岐(JUMP)
 *  @param tr_id トレーナーID
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_TR_FLAGON_JUMP tr_id,adrs
  _TRAINER_FLAG_CHECK \tr_id
  _IF_JUMP  FLGON,\adrs
  .endm

//--------------------------------------------------------------
/**
 *  _IF_TR_FLAGOFF_JUMP フラグOFFの時に分岐(JUMP)
 *  @param tr_id トレーナーID
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_TR_FLAGOFF_JUMP tr_id,adrs
  _TRAINER_FLAG_CHECK \tr_id
  _IF_JUMP  FLGOFF,\adrs
  .endm

//--------------------------------------------------------------
/**
 *  _IF_TR_FLAGON_CALL フラグONの時に分岐(CALL)
 *  @param tr_id トレーナーID
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_TR_FLAGON_CALL tr_id,adrs
  _TRAINER_FLAG_CHECK \tr_id
  _IF_CALL  FLGON,\adrs
  .endm

//--------------------------------------------------------------
/**
 *  _IF_TR_FLAGOFF_CALL フラグOFFの時に分岐(CALL)
 *  @param tr_id トレーナーID
 *  @param adrs 条件が一致していた際のジャンプ先アドレス
 */
//--------------------------------------------------------------
  .macro  _IF_TR_FLAGOFF_CALL tr_id,adrs
  _TRAINER_FLAG_CHECK \tr_id
  _IF_CALL  FLGOFF,\adrs
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
   .macro  _BGM_PLAY no
  .short  EV_SEQ_BGM_PLAY
  .short  \no
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_STOP BGMを止める
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _BGM_STOP
  .short  EV_SEQ_BGM_STOP
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_PLAYER_PAUSE シーケンスを一時停止または再開
 *  @param flag (1=停止、0=再開)
 */
//--------------------------------------------------------------
  .macro  _BGM_PLAYER_PAUSE flag
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
  .macro  _BGM_FADEOUT vol,frame
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
  .macro  _BGM_FADEIN frame
  .short  EV_SEQ_BGM_FADEIN
  .short  \frame
  .endm

//--------------------------------------------------------------
/**
 *  _BGM_NOW_MAP_PLAY 現在のマップのBGMを鳴らす
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _BGM_NOW_MAP_PLAY
  .short  EV_SEQ_BGM_NOW_MAP_PLAY
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
  .macro  _SE_PLAY no
  .short  EV_SEQ_SE_PLAY
  .short  \no
  .endm

//--------------------------------------------------------------
/**
 *  _SE_STOP SEを止める
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _SE_STOP
  .short  EV_SEQ_SE_STOP
  .endm

//--------------------------------------------------------------
/**
 *  _SE_WAIT SE終了待ち
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _SE_WAIT
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
  .macro  _ME_PLAY no
  .short  EV_SEQ_ME_PLAY
  .short  \no
  .endm

//--------------------------------------------------------------
/**
 *  _ME_WAIT ME終了待ち
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _ME_WAIT
  .short  EV_SEQ_ME_WAIT
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
   .macro  _BMPMENU_INIT x,y,cursor,cancel,ret_wk
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
   .macro  _BMPMENU_INIT_EX x,y,cursor,cancel,ret_wk
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
  .macro  _BMPMENU_MAKE_LIST msg_id,param
  .short  EV_SEQ_BMPMENU_MAKE_LIST16
  .short  \msg_id
  .short  \param
  .endm

//--------------------------------------------------------------
/**
 *  _BMPMENU_START BMPメニュー　開始
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _BMPMENU_START
  .short  EV_SEQ_BMPMENU_START
  .endm

//======================================================================
//  画面フェード
//======================================================================
//--------------------------------------------------------------
//  フェードモード(fade.h)
//--------------------------------------------------------------
#define DISP_FADE_BLACKOUT_MAIN (0x01) //メイン画面フェード ブラックアウト
#define DISP_FADE_BLACKOUT_SUB  (0x02) //サブ画面フェード ブラックアウト
#define DISP_FADE_BLACKOUT (0x03) //両画面フェード ブラックアウト
#define DISP_FADE_WHITEOUT_MAIN (0x04) //メイン画面フェード ホワイトアウト
#define DISP_FADE_WHITEOUT_SUB  (0x08) //サブ画面フェード ホワイトアウト
#define DISP_FADE_WHITEOUT (0x0c) //両画面フェード ホワイトアウト

//--------------------------------------------------------------
/**
 *  _DISP_FADE_START 画面フェードスタート
 *  @param mode 画面フェードモード 
 *  @param start_evy 初期輝度(0=元の色～16=指定輝度）
 *  @param end_evy 終了輝度(0=元の色～16=指定輝度)
 *  @param speed フェードスピード 0～
 */
//--------------------------------------------------------------
  .macro _DISP_FADE_START mode,start_evy,end_evy,speed
  .short EV_SEQ_DISP_FADE_START
  .short \mode
  .short \start_evy
  .short \end_evy
  .short \speed
  .endm

//--------------------------------------------------------------
/**
 *  _DISP_FADE_START 画面フェード終了チェック
 *  @param none
 */
//--------------------------------------------------------------
  .macro _DISP_FADE_END_CHECK
  .short EV_SEQ_DISP_FADE_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  _WHITE_OUT ホワイトアウト
 *  @param speed フェードスピード 0～
 */
//--------------------------------------------------------------
  .macro _WHITE_OUT speed
  _DISP_FADE_START DISP_FADE_WHITEOUT_MAIN,0,16,\speed
  .endm

//--------------------------------------------------------------
/**
 *  _WHITE_IN ホワイトイン
 *  @param speed フェードスピード 0～
 */
//--------------------------------------------------------------
  .macro _WHITE_IN speed
  _DISP_FADE_START DISP_FADE_WHITEOUT_MAIN,16,0,\speed
  .endm

//--------------------------------------------------------------
/**
 *  _BLACK_OUT ブラックアウト
 *  @param speed フェードスピード 0～
 */
//--------------------------------------------------------------
  .macro _BLACK_OUT speed
  _DISP_FADE_START DISP_FADE_BLACKOUT_MAIN,0,16,\speed
  .endm

//--------------------------------------------------------------
/**
 *  _WHITE_IN ブラックイン
 *  @param speed フェードスピード 0～
 */
//--------------------------------------------------------------
  .macro _BLACK_IN speed
  _DISP_FADE_START DISP_FADE_BLACKOUT_MAIN,16,0,\speed
  .endm

//======================================================================
//  その他
//======================================================================
//--------------------------------------------------------------
/**
 *  _CHG_LANGID 言語ID切り替え ひらがな<->漢字
 *  @param none
 */
//--------------------------------------------------------------
  .macro  _CHG_LANGID
  .short  EV_SEQ_CHG_LANGID
  .endm

//--------------------------------------------------------------
/*
 *  _GET_RND ランダム数取得
 *  @param ret_wk 乱数格納先
 *  @param num 乱数上限値
 */
//--------------------------------------------------------------
   .macro  _GET_RND ret_wk,num
  .short  EV_SEQ_GET_RND
  .short  \ret_wk
  .short  \num
  .endm
