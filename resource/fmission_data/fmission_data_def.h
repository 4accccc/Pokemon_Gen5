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
  FMISSION_TYPE_TRBTL,  ///<トレーナー再戦ミッション
  FMISSION_TYPE_MAX,  ///<
  FMISSION_TYPE_END = FMISSION_TYPE_MAX-1,  ///<
  FMISSION_TYPE_EX_WITHOUT_PARAM = 0xFF,  ///<ログセット処理用の特殊コード。実際のミッションタイプとしては使いません
}FMISSION_TYPE_ID;

//=========================================
///ボーナスタイム定義
//=========================================
typedef enum{
  FMISSION_TBONUS_0,  ///<なし
  FMISSION_TBONUS_10,  ///<10秒
  FMISSION_TBONUS_20,  ///<20秒
  FMISSION_TBONUS_30,  ///<30秒
  FMISSION_TBONUS_40,  ///<40秒
  FMISSION_TBONUS_50,  ///<50秒
  FMISSION_TBONUS_60,  ///<60秒
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
///アイテムミッション　セット名定義
//=========================================
typedef enum{
  FMISSION_ITEM_SET_KAKERA1,  ///<イロのカケラセット1
  FMISSION_ITEM_SET_KAKERA2,  ///<イロのカケラセット2
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
  FMISSION_ITEM_SET_KINOMI_D,  ///<きのみ配信
  FMISSION_ITEM_SET_LOST_ITEM_D,  ///<落し物配信
  FMISSION_ITEM_SET_MAX,  ///<
}FMISSION_ITEM_SET;

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




#endif //__H_FMISSION_DATA_DEF_H__

