//======================================================================
//                musical_scr_local.h
//
//    スクリプトファイル：ミュージカル関係ローカル定数定義
//
//======================================================================

//※がついているコマンドは控え室のみ有効です！

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

//_MUSICAL_CALL 用
#define MUSICAL_CALL_LOCAL (0)        //非通信
#define MUSICAL_CALL_COMM  (1)        //通信

//_GET_MUSICAL_VALUE 用
#define MUSICAL_VALUE_JOINNUM (0)   //参加回数
#define MUSICAL_VALUE_TOPNUM  (1)   //トップ回数
#define MUSICAL_VALUE_LAST_POINT (2)  //最終評価点
#define MUSICAL_VALUE_LAST_CONDITION (3)  //最終コンディション

//_GET_MUSICAL_FAN_VALUE 用
#define MUSICAL_VALUE_FAN_TYPE (0)  //見た目
#define MUSICAL_VALUE_FAN_CHEER_MSG (1) //応援メッセージ
#define MUSICAL_VALUE_FAN_GIFT_MSG  (2) //プレゼントメッセージ
#define MUSICAL_VALUE_FAN_GIFT_TYPE (3) //プレゼント種類
#define MUSICAL_VALUE_FAN_GIFT_NUMBER (4) //プレゼント番号

//_GET_MUSICAL_WAITROOM_VALUE
//全部※(控え室のみ有効です！)
#define MUSICAL_VALUE_WR_SELF_IDX  (0) //自分の参加番号
#define MUSICAL_VALUE_WR_MAX_POINT (1) //最高評価点
#define MUSICAL_VALUE_WR_MIN_POINT (2) //最高得点
#define MUSICAL_VALUE_WR_GRADE_MSG (3) //評価メッセージの取得
