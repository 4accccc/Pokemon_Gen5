//======================================================================
/**
 * @file	script_def.h
 * @brief	スクリプト定義ファイル
 * @author	Satoshi Nohara
 * @date	05.12.14
 *
 * スクリプト環境とプログラムとで共通して参照する必要がある定義を集約するヘッダファイル。
 * スクリプト側はenumなどC特有の文法解釈ができないため、defineを使用すること。
 */
//======================================================================
#pragma once

//======================================================================
//
//  スクリプトヘッダ関連
//
//  スクリプト側ではマクロに埋め込まれているため、直接これらの値を
//  意識的に使用することはない。
//======================================================================
//--------------------------------------------------------------
//特殊スクリプト定義
//--------------------------------------------------------------
#define SP_SCRID_NONE				(0)
#define SP_SCRID_SCENE_CHANGE		(1)     ///<特殊スクリプトテーブルでのシーン発動イベントデータの指定ID
#define SP_SCRID_ZONE_CHANGE		(2)     ///<特殊スクリプトテーブルでのゾーン切り替えスクリプトの指定ID
#define SP_SCRID_FIELD_RECOVER	(3)			///<特殊スクリプトテーブルでのフィールド復帰スクリプト指定ID
#define SP_SCRID_FIELD_INIT 		(4)			///<特殊スクリプトテーブルでのフィールド初期化スクリプト指定ID

///イベントデータテーブル末尾を判定するための値
#define EV_DATA_END_CODE  (0xfd13)  //適当な値

//======================================================================
//
//    OBJ関連
//
//======================================================================
//--------------------------------------------------------------
//エイリアス識別用スクリプトID
#define SP_SCRID_ALIES				(0xffff)

//トレーナータイプ定義
#define SCR_EYE_TR_TYPE_SINGLE		(0)
#define SCR_EYE_TR_TYPE_DOUBLE		(1)
#define SCR_EYE_TR_TYPE_TAG			(2)

//トレーナーの番号定義
#define SCR_EYE_TR_0				(0)
#define SCR_EYE_TR_1				(1)

//連れ歩きOBJを取得するOBJID定義
#define SCR_OBJID_MV_PAIR			(0xf2)

//透明ダミーOBJを取得するOBJID定義
#define SCR_OBJID_DUMMY				(0xf1)

//======================================================================
//    各コマンド依存の定義
//======================================================================
//--------------------------------------------------------------
//fld_menu.c
//--------------------------------------------------------------
#define SCR_REPORT_OK				(0)			//セーブした
#define SCR_REPORT_CANCEL			(1)			//セーブしていない

///YES_NO_WINの戻り値
#define SCR_YES   (0) //YES_NO_WINの戻り値 「はい」を返した
#define SCR_NO    (1) //YES_NO_WINの戻り値 「いいえ」を返した

//ショップのID定義
#define SCR_SHOPID_NULL ( 0xFF )  //変動ショップ呼び出しID

//サブプロセス(アプリ)の終了コード定義
#define SCR_PROC_RETMODE_EXIT    (0) //一発終了
#define SCR_PROC_RETMODE_NORMAL  (1) //通常終了

//バッグプロセス呼び出しコード定義
#define SCR_BAG_MODE_SELL  (0)

//フィールド通信切断処理イベントの結果
#define SCR_FIELD_COMM_EXIT_OK      (0) //正常に通信切断処理が終わった
#define SCR_FIELD_COMM_EXIT_CANCEL  (1) //切断処理をキャンセル
#define SCR_FIELD_COMM_EXIT_ERROR   (2) //切断処理中になんらかのエラー

//--------------------------------------------------------------
/// BmpMenuの説明メッセージ無効ID
//--------------------------------------------------------------
#define SCR_BMPMENU_EXMSG_NULL   (0xFFFF)

//--------------------------------------------------------------
//  バトル関連
//--------------------------------------------------------------
///TRAINER_LOSE_CHECK,NORMAL_LOSE_CHECK
#define SCR_BATTLE_RESULT_LOSE (0) //負け
#define SCR_BATTLE_RESULT_WIN  (1) //勝ち

#define SCR_BATTLE_MODE_NONE    (0) //特にモード指定なし
#define SCR_BATTLE_MODE_NOLOSE  (1) //負けないモード

///野生ポケモン戦闘呼び出し時に指定できるフラグ
#define SCR_WILD_BTL_FLAG_NONE    (0x0000)  //フラグ無し
#define SCR_WILD_BTL_FLAG_REGEND  (0x0001)  //伝説戦闘(エンカウントメッセージが変化する)
#define SCR_WILD_BTL_FLAG_RARE    (0x0002)  //強制レアポケ戦闘

///野生ポケモン戦　再戦コードチェック
#define SCR_WILD_BTL_RET_CAPTURE  (0) //捕まえた
#define SCR_WILD_BTL_RET_ESCAPE   (1) //逃げた
#define SCR_WILD_BTL_RET_WIN      (2) //倒した

//--------------------------------------------------------------
//曜日取得関数の戻り値
//  _GET_WEEK
//--------------------------------------------------------------
#define RTC_WEEK_SUNDAY		0
#define RTC_WEEK_MONDAY		1
#define RTC_WEEK_TUESDAY	2
#define RTC_WEEK_WEDNESDAY	3
#define RTC_WEEK_THURSDAY	4
#define RTC_WEEK_FRIDAY		5
#define RTC_WEEK_SATURDAY	6

//--------------------------------------------------------------
//バッジフラグ指定IDの定義
//  _GET_BADGE_FLAG/_SET_BADGE_FLAG
//--------------------------------------------------------------
#define BADGE_ID_01   0
#define BADGE_ID_02   1
#define BADGE_ID_03   2
#define BADGE_ID_04   3
#define BADGE_ID_05   4
#define BADGE_ID_06   5
#define BADGE_ID_07   6
#define BADGE_ID_08   7

//--------------------------------------------------------------
// 手持ちポケモン数カウントモード
//--------------------------------------------------------------
#define POKECOUNT_MODE_TOTAL         (0)  // 手持ちの数
#define POKECOUNT_MODE_NOT_EGG       (1)  // タマゴを除く手持ちの数
#define POKECOUNT_MODE_BATTLE_ENABLE (2)  // 戦える(タマゴと瀕死を除いた)ポケモン数
#define POKECOUNT_MODE_ONLY_EGG      (3)  // タマゴの数(駄目タマゴを除く)
#define POKECOUNT_MODE_ONLY_DAME_EGG (4)  // 駄目タマゴの数
#define POKECOUNT_MODE_EMPTY         (5)  // 空いている数


//--------------------------------------------------------------
// ポケモン捕獲場所　チェック
//--------------------------------------------------------------
#define POKE_GET_PLACE_CHECK_WF         (0)  // ホワイトフォレストで捕獲したのかチェック
#define POKE_GET_PLACE_CHECK_MAX        (1)  // ホワイトフォレストで捕獲したのかチェック

//--------------------------------------------------------------
//  ずかん操作モード指定
//--------------------------------------------------------------
#define ZUKANCTRL_MODE_SEE     (0)   ///<ずかん操作モード：見た数
#define ZUKANCTRL_MODE_GET     (1)   ///<ずかん操作モード：捕まえた数

//--------------------------------------------------------------
//  配置モデル用アニメ指定
//--------------------------------------------------------------
#define SCR_BMID_NULL           (0)       ///<配置モデル分類：なし
#define SCR_BMID_DOOR           (1)       ///<配置モデル分類：ドア
#define SCR_BMID_SANDSTREAM     (2)       ///<配置モデル分類：流砂
#define SCR_BMID_PCMACHINE      (3)       ///<配置モデル分類：回復マシン
#define SCR_BMID_PC             (4)       ///<配置モデル分類：パソコン
#define SCR_BMID_PCEV_DOOR      (5)       ///<配置モデル分類：ポケセンエレベーター：ドア
#define SCR_BMID_PCEV_FLOOR     (6)       ///<配置モデル分類：ポケセンエレベーター：フロア
#define SCR_BMID_WARP           (7)       ///<配置モデル分類：ワープ

#define  SCR_BMANM_DOOR_OPEN    (0)       ///<ドアアニメ：開く
#define  SCR_BMANM_DOOR_CLOSE   (1)       ///<ドアアニメ：閉じる

#define  SCR_BMANM_PCEV_OPEN    (0)       ///<ポケセンエレベータアニメ：開く
#define  SCR_BMANM_PCEV_CLOSE   (1)       ///<ポケセンエレベータアニメ：閉じる

#define  SCR_BMANM_PCEV_UP      (0)       ///<ポケセンエレベータアニメ：上がる
#define  SCR_BMANM_PCEV_DOWN    (1)       ///<ポケセンエレベータアニメ：下がる
//--------------------------------------------------------------
/// メッセージウィンドウ上下指定
//--------------------------------------------------------------
#define SCRCMD_MSGWIN_UP (0)
#define SCRCMD_MSGWIN_DOWN (1)
#define SCRCMD_MSGWIN_NON (2)

//--------------------------------------------------------------
/// BGウィンドウタイプ
//--------------------------------------------------------------
#define SCRCMD_BGWIN_TYPE_INFO (0)
#define SCRCMD_BGWIN_TYPE_TOWN (1)
#define SCRCMD_BGWIN_TYPE_POST (2)
#define SCRCMD_BGWIN_TYPE_ROAD (3)

//--------------------------------------------------------------
/// 特殊ウィンドウタイプ
//--------------------------------------------------------------
#define SCRCMD_SPWIN_TYPE_LETTER (0)
#define SCRCMD_SPWIN_TYPE_BOOK (1)

//--------------------------------------------------------------
/// バトルレコーダー呼び出しモード指定
//--------------------------------------------------------------
#define SCRCMD_BTL_RECORDER_MODE_VIDEO   (0)
#define SCRCMD_BTL_RECORDER_MODE_MUSICAL (1)

//--------------------------------------------------------------
/// 対戦時パーティタイプ指定
//--------------------------------------------------------------
#define SCR_BTL_PARTY_SELECT_TEMOTI   (0) //手持ち
#define SCR_BTL_PARTY_SELECT_BTLBOX   (1) //バトルボックス
#define SCR_BTL_PARTY_SELECT_CANCEL   (2) //選択キャンセル
#define SCR_BTL_PARTY_SELECT_NG       (3) //レギュレーションNG

//--------------------------------------------------------------
/// ゲーム内交換可能チェック戻り値
//--------------------------------------------------------------
#define FLD_TRADE_ENABLE         (0)  // 交換可
#define FLD_TRADE_DISABLE_EGG    (1)  // 交換不可(タマゴのため)
#define FLD_TRADE_DISABLE_MONSNO (2)  // 交換不可(違うモンスターのため)
#define FLD_TRADE_DISABLE_SEX    (3)  // 交換不可(違う性別のため)

//--------------------------------------------------------------
/// ワイヤレスセーブモード
//--------------------------------------------------------------
#define WIRELESS_SAVEMODE_OFF (0)
#define WIRELESS_SAVEMODE_ON  (1)

//--------------------------------------------------------------
/// 技思い出し結果
//--------------------------------------------------------------
#define SCR_WAZAOSHIE_RET_SET    (0)
#define SCR_WAZAOSHIE_RET_CANCEL (1)

//--------------------------------------------------------------
/// ボックス終了モード
//--------------------------------------------------------------
#define SCR_BOX_END_MODE_MENU   (0)  // 選択メニューへ戻る
#define SCR_BOX_END_MODE_C_GEAR (1)  // C-gearまで戻る

//--------------------------------------------------------------
//--------------------------------------------------------------
/*
//フラグ操作関数のモード指定用
#define	FLAG_MODE_RESET				(0)
#define	FLAG_MODE_SET				(1)
#define	FLAG_MODE_GET				(2)

//図鑑モード
#define SCR_SHINOU_ZUKAN_MODE	0
#define SCR_ZENKOKU_ZUKAN_MODE	1

//パソコンアニメ管理ナンバー
#define SCR_PASO_ANM_NO		90


//技教え：3人いて、それぞれの色の技しか教えてくれない
#define WAZA_OSHIE_COL_BLUE			(0)
#define WAZA_OSHIE_COL_RED			(1)
#define WAZA_OSHIE_COL_YELLOW		(2)

//技覚え共通ウェイト
#define WAZA_COMMON_WAIT			(30)
#define WAZA_COMMON_WAIT2			(32)
#define WAZA_COMMON_WAIT3			(16)
*/
