//=========================================================================
/**
 * @file	fmission_data_def.h
 * @brief	フェスミッションデータ　リテラル定義ファイル
 * @autor	miyuki iwasawa
 * @note	このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。
 */
//=========================================================================
#ifndef __H_FMISSION_DATA_DEF_H__
#define __H_FMISSION_DATA_DEF_H__

#ifndef __ASM_NO_DEF_  //ここからアセンブラでは無視

//=========================================
///フェスミッションタイプ
//=========================================
typedef enum{
  FMISSION_TYPE_NULL,  ///<
  FMISSION_TYPE_EFFECT_ENC,  ///<エフェクトエンカウントで指定ポケモンを倒すか捕まえる
  FMISSION_TYPE_MUSHROOM_ENC,  ///<タマゲタケシンボルエンカウントで指定ポケモンを倒すか捕まえる
  FMISSION_TYPE_EVENT,  ///<指定イベントをクリアする
  FMISSION_TYPE_ITEM,  ///<アイテムミッション
  FMISSION_TYPE_NPC,  ///<NPCミッション
  FMISSION_TYPE_MAX,  ///<
  FMISSION_TYPE_END = FMISSION_TYPE_MAX-1,  ///<
  FMISSION_TYPE_EX_WITHOUT_PARAM = 0xFF,  ///<ログセット処理用の特殊コード。実際のミッションタイプとしては使いません
}FMISSION_TYPE_ID;

//=========================================
///ボーナスタイム定義
//=========================================
typedef enum{
  FMISSION_TBONUS_0,  ///<なし
  FMISSION_TBONUS_30,  ///<30秒
  FMISSION_TBONUS_60,  ///<60秒
  FMISSION_TBONUS_180,  ///<180秒
  FMISSION_TBONUS_MAX,  ///<
}FMISSION_TBONUS;

//=========================================
///単位ID
//=========================================
typedef enum{
  FMISSION_UNIT_NONE,  ///<なし
  FMISSION_UNIT_HUMAN,  ///<人
  FMISSION_UNIT_POKEMON,  ///<匹
  FMISSION_UNIT_ITEM,  ///<個
  FMISSION_UNIT_COUNT,  ///<回
  FMISSION_UNIT_MAX,  ///<
}FMISSION_UNIT_ID;

//=========================================
///ミッション終了タイプ
//=========================================
typedef enum{
  FMISSION_ENDTYPE_TIME,  ///<時間依存
  FMISSION_ENDTYPE_SCORE,  ///<スコア依存
  FMISSION_ENDTYPE_MAX,  ///<
}FMISSION_END_TYPE;

//=========================================
///フェスの開催バージョンフラグ
//=========================================
typedef enum{
  FMISSION_VERSION_WHITE,  ///<ホワイト
  FMISSION_VERSION_BLACK,  ///<ブラック限定
  FMISSION_VERSION_MAX,  ///<
}FMISSION_VERSION;

//=========================================
///開催制限タイプ
//=========================================
typedef enum{
  FMISSION_HOST_FREE,  ///<フリー
  FMISSION_HOST_WHITE,  ///<ホワイト限定
  FMISSION_HOST_BLACK,  ///<ブラック限定
  FMISSION_HOST_COMM,  ///<配信限定
  FMISSION_HOST_GIFT1,  ///<不思議な贈り物開放1
  FMISSION_HOST_GIFT2,  ///<不思議な贈り物開放2
  FMISSION_HOST_GIFT3,  ///<不思議な贈り物開放3
  FMISSION_HOST_GIFT4,  ///<不思議な贈り物開放4
  FMISSION_HOST_MAX,  ///<
}FMISSION_HOST_TYPE;

//=========================================
///ミッション範囲
//=========================================
typedef enum{
  FMISSION_LOC_FREE,  ///<どこでもミッション
  FMISSION_LOC_LIMITED,  ///<ここだけミッション
  FMISSION_MAX,  ///<
}FMISSION_LOCATION;

//=========================================
///報酬ボーナス計算タイプ
//=========================================
typedef enum{
  FMISSION_PAY_BONUS_NONE,  ///<ボーナス無し
  FMISSION_PAY_BONUS_TIME,  ///<タイムボーナス
  FMISSION_PAY_BONUS_SCORE,  ///<スコアボーナス
  FMISSION_PAY_BONUS_MAX,  ///<
}FMISSION_PAY_BONUS;

//=========================================
///ミッションレベル
//=========================================
typedef enum{
  FMISSION_LEVEL_1,  ///<level1
  FMISSION_LEVEL_2,  ///<level2
  FMISSION_LEVEL_3,  ///<level3
  FMISSION_LEVEL_4,  ///<level4
  FMISSION_LEVEL_MAX,  ///<
}FMISSION_LEVEL;

//=========================================
///ミッションクリアレベル
//=========================================
typedef enum{
  FMISSION_CLEAR_LEVEL_0,  ///<未クリア
  FMISSION_CLEAR_LEVEL_1,  ///<level1
  FMISSION_CLEAR_LEVEL_2,  ///<level2
  FMISSION_CLEAR_LEVEL_3,  ///<level3
  FMISSION_CLEAR_LEVEL_4,  ///<level4
  FMISSION_CLEAR_LEVEL_MAX,  ///<
}FMISSION_CLEAR_LEVEL;

//=========================================
///フェスミッションマップ属性
//=========================================
typedef enum{
  FMISSION_MAP_ATTR_TOWN,  ///<街
  FMISSION_MAP_ATTR_ROAD,  ///<道
  FMISSION_MAP_ATTR_CAVE,  ///<洞穴
  FMISSION_MAP_ATTR_FOREST,  ///<森
  FMISSION_MAP_ATTR_SP_HIGH,  ///<特殊高
  FMISSION_MAP_ATTR_SP_LOW,  ///<特殊低
  FMISSION_MAP_ATTR_MAX,  ///<
  FMISSION_MAP_ATTR_NONE = FMISSION_MAP_ATTR_MAX,  ///<無し 特殊コード
}FMISSION_MAP_ATTR;

//=========================================
///ポイント出現パターン
//=========================================
typedef enum{
  FMISSION_POINT_PAT_NONE,  ///<無し
  FMISSION_POINT_PAT_RARE,  ///<レア
  FMISSION_POINT_PAT_LOW,  ///<低
  FMISSION_POINT_PAT_MIDDLE,  ///<中
  FMISSION_POINT_PAT_HIGH,  ///<高
  FMISSION_POINT_PAT_MAXIMUM,  ///<最高
  FMISSION_POINT_PAT_LUNK,  ///<運
  FMISSION_POINT_PAT_MAX,  ///<
}FMISSION_POINT_PATTERN;

//=========================================
///アイテムミッション　セット名定義
//=========================================
typedef enum{
  FMISSION_ITEM_SET_KAKERA1,  ///<イロのカケラセット1
  FMISSION_ITEM_SET_TREASURE1,  ///<宝探しセット１
  FMISSION_ITEM_SET_LOST_ITEM,  ///<落し物
  FMISSION_ITEM_SET_LOST_ITEM_W,  ///<落し物W
  FMISSION_ITEM_SET_LOST_ITEM_B,  ///<落し物B
  FMISSION_ITEM_SET_KINOMI1,  ///<きのみセット１
  FMISSION_ITEM_SET_KINOMI2,  ///<きのみセット２
  FMISSION_ITEM_SET_STONE_W,  ///<進化の石セットW
  FMISSION_ITEM_SET_STONE_B,  ///<進化の石セットB
  FMISSION_ITEM_SET_HERB,  ///<漢方薬セット
  FMISSION_ITEM_SET_MANURE,  ///<○○のこやし
  FMISSION_ITEM_SET_HEART,  ///<ハートのうろこ他
  FMISSION_ITEM_SET_TREASURE2,  ///<宝探しセット２
  FMISSION_ITEM_SET_LOST_ITEM_D,  ///<落し物配信
  FMISSION_ITEM_SET_KINOMI_D,  ///<きのみ配信　定時配信パターン
  FMISSION_ITEM_SET_KINOMI_MOVIE,  ///<きのみセット（メロエッタ用）
  FMISSION_ITEM_SET_KINOMI_GIFT,  ///<きのみ配信　不思議な贈り物
  FMISSION_ITEM_SET_MAX,  ///<
}FMISSION_ITEM_SET;

//=========================================
///アイテム交換セット名
//=========================================
typedef enum{
  FMISSION_EXP_ITEM_TRADE_SET01,  ///<セット1
  FMISSION_EXP_ITEM_TRADE_SET02W,  ///<セット2ホワイト
  FMISSION_EXP_ITEM_TRADE_SET02B,  ///<セット2ブラック
  FMISSION_EXP_ITEM_TRADE_SET_MAX,  ///<
}FMISSION_EXP_ITEM_TRADE_SET;

//=========================================
///ショッピングアイテムセット名
//=========================================
typedef enum{
  FMISSION_EXP_SHOPPING_SET_01,  ///<セット1
  FMISSION_EXP_SHOPPING_SET_02W,  ///<セット2ホワイト
  FMISSION_EXP_SHOPPING_SET_02B,  ///<セット2ブラック
  FMISSION_EXP_SHOPPING_SET_ATTRACTION,  ///<セット3 アトラクション型
  FMISSION_EXP_SHOPPING_SET_EASY,  ///<セット4 シナリオ用簡単バージョン
  FMISSION_EXP_SHOPPING_SET_MAX,  ///<
}FMISSION_EXP_SHOPPING_SET;

//=========================================
///トレーナー対戦セット名
//=========================================
typedef enum{
  FMISSION_EXP_TRAINER_SET_KARATE,  ///<カラテかセット
  FMISSION_EXP_TRAINER_SET_ELITEW,  ///<エリートトレーナーWomanセット
  FMISSION_EXP_TRAINER_SET_ELITEM,  ///<エリートトレーナーManセット
  FMISSION_EXP_TRAINER_SET_MAX,  ///<
}FMISSION_EXP_TRAINER_SET;

//=========================================
///もらうプレゼントアイテムセット名
//=========================================
typedef enum{
  FMISSION_EXP_GET_A_PRESENT_SET_PC,  ///<ポケセン配信
  FMISSION_EXP_GET_A_PRESENT_SET_FLAG,  ///<フラグ配信(不思議な贈り物配信)
  FMISSION_EXP_GET_A_PRESENT_SET_MAX,  ///<
}FMISSION_EXP_PRESENT_SET;

//=========================================
///プレゼントアイテムセット名
//=========================================
typedef enum{
  FMISSION_EXP_POKE_QUIZ_NORMAL,  ///<ノーマルタイプ
  FMISSION_EXP_POKE_QUIZ_KAKUTOU,  ///<格闘タイプ
  FMISSION_EXP_POKE_QUIZ_HIKOU,  ///<飛行タイプ
  FMISSION_EXP_POKE_QUIZ_DOKU,  ///<毒タイプ
  FMISSION_EXP_POKE_QUIZ_JIMEN,  ///<地面タイプ
  FMISSION_EXP_POKE_QUIZ_IWA,  ///<岩タイプ
  FMISSION_EXP_POKE_QUIZ_MUSHI,  ///<虫タイプ
  FMISSION_EXP_POKE_QUIZ_GHOST,  ///<ゴーストタイプ
  FMISSION_EXP_POKE_QUIZ_HAGANE,  ///<鋼タイプ
  FMISSION_EXP_POKE_QUIZ_HONOO,  ///<炎タイプ
  FMISSION_EXP_POKE_QUIZ_MIZU,  ///<水タイプ
  FMISSION_EXP_POKE_QUIZ_KUSA,  ///<草タイプ
  FMISSION_EXP_POKE_QUIZ_DENKI,  ///<電気タイプ
  FMISSION_EXP_POKE_QUIZ_ESPER,  ///<エスパータイプ
  FMISSION_EXP_POKE_QUIZ_KOORI,  ///<氷タイプ
  FMISSION_EXP_POKE_QUIZ_DRAGON,  ///<ドラゴンタイプ
  FMISSION_EXP_POKE_QUIZ_AKU,  ///<悪タイプ
  FMISSION_EXP_POKE_QUIZ_ZENKOKU,  ///<全国図鑑
  FMISSION_EXP_POKE_QUIZ_LOCAL,  ///<地方図鑑
  FMISSION_EXP_POKE_QUIZ_LEGEND,  ///<伝説ポケモン
  FMISSION_EXP_POKE_QUIZ_MAX,  ///<
}FMISSION_EXP_POKE_QUIZ_SET;

//=========================================
///ミッションイベントID
//=========================================
typedef enum{
  FMISSION_EVID_TOWER_OF_HEAVEN,  ///<タワーオブヘブンの鐘を鳴らせ
  FMISSION_EVID_ENCEFF_FEATHER,  ///<ホドモエに舞い落ちる羽
  FMISSION_EVID_ENCEFF_JUWEL,  ///<ジュエルを採掘せよ
  FMISSION_EVID_EGG_HATCH,  ///<タマゴ孵化
  FMISSION_EVID_FISHING,  ///<釣り大会
  FMISSION_EVID_ANIMAL_TRAIL,  ///<獣道調査
  FMISSION_EVID_MAX,  ///<
  FMISSION_EVID_END = FMISSION_EVID_MAX-1,  ///<
}FMISSION_EVENT_ID;

#endif //__ASM_NO_DEF_

//=========================================
///NPCタイプID
//=========================================
#define  FMISSION_NPC_TYPE_LOST_CHILDREN  ( 0 )   ///<迷子を捜せ
#define  FMISSION_NPC_TYPE_PRESENT_ITEM  ( 1 )   ///<アイテムをプレゼントする
#define  FMISSION_NPC_TYPE_ITEM_TRADE  ( 2 )   ///<アイテム交換
#define  FMISSION_NPC_TYPE_SHOPPING  ( 3 )   ///<アイテム購入
#define  FMISSION_NPC_TYPE_RPS  ( 4 )   ///<じゃんけん
#define  FMISSION_NPC_TYPE_MEMORY_QUIZ  ( 5 )   ///<記憶クイズ
#define  FMISSION_NPC_TYPE_TR_BATTLE  ( 6 )   ///<トレーナーバトル
#define  FMISSION_NPC_TYPE_GET_A_PRESENT  ( 7 )   ///<アイテムをもらう
#define  FMISSION_NPC_TYPE_POKE_QUIZ  ( 8 )   ///<ポケモン図鑑クイズ
#define  FMISSION_NPC_TYPE_MAX  ( 9 )   ///<
#define  FMISSION_NPC_TYPE_END  ( FMISSION_NPC_TYPE_MAX-1 )   ///<


//=========================================
///拡張パラメータ（隠し穴大調査用ポケモンINDEX）
//=========================================
#define  FMISSION_EXP_TRAIL_DUMMY  ( 0 )   ///<ダミーデータ
#define  FMISSION_EXP_TRAIL_ROKON  ( 1 )   ///<ロコン
#define  FMISSION_EXP_TRAIL_NYOROMO  ( 2 )   ///<ニョロモ
#define  FMISSION_EXP_TRAIL_PIKATYUU  ( 3 )   ///<ピカチュウ
#define  FMISSION_EXP_TRAIL_KEESHI  ( 4 )   ///<ケーシィ
#define  FMISSION_EXP_TRAIL_NASSII  ( 5 )   ///<ナッシー
#define  FMISSION_EXP_TRAIL_BARUKII  ( 6 )   ///<バルキー
#define  FMISSION_EXP_TRAIL_TANEBOO  ( 7 )   ///<タネボー
#define  FMISSION_EXP_TRAIL_PAARURU  ( 8 )   ///<パールル
#define  FMISSION_EXP_TRAIL_PINPUKU  ( 9 )   ///<ピンプク
#define  FMISSION_EXP_TRAIL_GUREGGURU  ( 10 )   ///<グレッグル
#define  FMISSION_EXP_TRAIL_MONS_MAX  ( 11 )   ///<


//=========================================
///ミッション開催時エフェクト待ち時間定義
//=========================================
#define  FMISSION_START_EFFECT1_WAIT  ( 70 )   ///<パレスギミックエフェクト1ウェイト
#define  FMISSION_START_EFFECT2_WAIT  ( 30 )   ///<パレスギミックエフェクト2ウェイト


//=========================================
///ミッション報酬基礎値MAX定義
//=========================================
#define  FMISSION_PAY_NUM_BASE_MAX  ( 100 )   ///<通常報酬数MAX値定義
#define  FMISSION_PAY_NUM_SP_MAX  ( 50 )   ///<ダブルスコア報酬数MAX値定義
#define  FMISSION_PAY_NUM_BONUS_MAX  ( 99 )   ///<ボーナスとして加算される報酬数MAX




#endif //__H_FMISSION_DATA_DEF_H__

