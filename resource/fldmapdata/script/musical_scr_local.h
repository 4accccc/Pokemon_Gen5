//======================================================================
//                musical_scr_local.h
//
//    スクリプトファイル：ミュージカル関係ローカル定数定義
//
//======================================================================

//イベントで渡すグッズ
#define MUSICAL_GOODS_KEEKI (24)
#define MUSICAL_GOODS_KURAUN (55)
#define MUSICAL_GOODS_THIARA (67)
#define MUSICAL_GOODS_YUUSHOUBERUTO (82)
#define MUSICAL_GOODS_OHESOBOTAN (87)
#define MUSICAL_GOODS_NIKONIKOOMEN (86)
#define MUSICAL_GOODS_EREKIGITAA (84)
#define MUSICAL_GOODS_PUREZENTOBAKO (99)
#define MUSICAL_GOODS_FUSAFUSANOHIGE (98)
#define MUSICAL_GOODS_AKAIBOUSI (95)
#define MUSICAL_GOODS_OOKINAFUKURO (96)

//※がついているコマンドは控え室のみ有効です！

//コンディションの定義
#define MUSICAL_CONDITION_COOL (0)    //クール
#define MUSICAL_CONDITION_CUTE (1)    //キュート
#define MUSICAL_CONDITION_ELEGANT (2) //エレガント
#define MUSICAL_CONDITION_UNIQUE (3)  //ユニーク
#define MUSICAL_CONDITION_NONE (4)  //上以外のときに帰ってくる値(無かったとか対象が複数ある


//_GET_MUSICAL_FAN_VALUE_GIFT_TYPE で戻ってくる値
#define MUSICAL_GIFT_TYPE_NONE  (0)  //無い
#define MUSICAL_GIFT_TYPE_GOODS (1)  //グッズ
#define MUSICAL_GIFT_TYPE_ITEM  (2)  //アイテム

//_GET_MUSICAL_FAN_VALUE_TYPE で居ない時の値
//居る時はOBJIDが帰ってきてます。
#define MUSICAL_FAL_TYPE_NONE (0)

//ココから下は基本ラッパーしてあるのでプログラムで参照
//_MUSICAL_WORD 用タイプ
#define MUSICAL_WORD_TITLE (0)        //セーブにある演目
#define MUSICAL_WORD_GOODS (1)        //グッズ名
#define MUSICAL_WORD_TITLE_LOCAL (2)  //※現在演目
#define MUSICAL_WORD_AUDI_TYPE (3)  //人気客層
#define MUSICAL_WORD_CONDITION (4)  //コンディション名

//_MUSICAL_CALL 用
#define MUSICAL_CALL_LOCAL (0)        //非通信
#define MUSICAL_CALL_COMM  (1)        //通信

//_GET_MUSICAL_VALUE 用
#define MUSICAL_VALUE_JOINNUM (0)   //参加回数
#define MUSICAL_VALUE_TOPNUM  (1)   //トップ回数
#define MUSICAL_VALUE_LAST_POINT (2)  //最終評価点
#define MUSICAL_VALUE_LAST_CONDITION (3)  //最終コンディション
#define MUSICAL_VALUE_PROGRAM_NUMBER (4)  //選択演目番号
#define MUSICAL_VALUE_SET_PROGRAM_NUMBER (5)  //選択演目番号設定

//_GET_MUSICAL_FAN_VALUE 用
#define MUSICAL_VALUE_FAN_TYPE (0)  //見た目
#define MUSICAL_VALUE_FAN_CHEER_MSG (1) //応援メッセージ
#define MUSICAL_VALUE_FAN_GIFT_MSG  (2) //プレゼントメッセージ
#define MUSICAL_VALUE_FAN_GIFT_TYPE (3) //プレゼント種類
#define MUSICAL_VALUE_FAN_GIFT_NUMBER (4) //プレゼント番号

//_GET_MUSICAL_WAITROOM_VALUE
//全部※(控え室のみ有効です！)
//しかもそれぞれドレスアップ後・ショー後のみ有効な物があるので有泉以外使用禁止
#define MUSICAL_VALUE_WR_SELF_IDX  (0) //自分の参加番号
#define MUSICAL_VALUE_WR_MAX_POINT (1) //最高評価点
#define MUSICAL_VALUE_WR_MIN_POINT (2) //最低評価点
#define MUSICAL_VALUE_WR_POINT     (3) //個人得点
#define MUSICAL_VALUE_WR_GRADE_MSG (4) //評価メッセージの取得
#define MUSICAL_VALUE_WR_POS_TO_IDX (5) //立ち位置に対応した参加番号
#define MUSICAL_VALUE_WR_MAX_CONDITION (6) //演目の高いコンディション
#define MUSICAL_VALUE_WR_POS_TO_RANK (7) //順位に対応した参加番号
#define MUSICAL_VALUE_WR_POKE_MAX_POINT_CON (8) //最も点が高かったコンディション


