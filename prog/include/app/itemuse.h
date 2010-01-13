//============================================================================================
/**
 * @file  itemuse.h
 * @brief フィールドアイテム使用処理
 * @author  Hiroyuki Nakamura
 * @date  05.12.12
 */
//============================================================================================

#pragma once


#include "gamesystem/gamesystem.h"



//============================================================================================
//  定数定義
//============================================================================================

/// バッグから抜けるときの動作
enum BAG_NEXTPROC_ENUM
{
  BAG_NEXTPROC_EXIT,          // 終了
  BAG_NEXTPROC_RETURN,        // 手前に戻る
  BAG_NEXTPROC_HAVE,          // ポケモンにアイテムをもたせる
  BAG_NEXTPROC_ITEMEQUIP,     // ポケモンリストのアイテム装備に戻る
  BAG_NEXTPROC_ITEMHAVE_RET,  // ポケモンリストのアイテム装備に戻るが、キャンセルのとき
  BAG_NEXTPROC_ITEMUSE,       // 使う
  BAG_NEXTPROC_WAZASET,       // 技セット
  BAG_NEXTPROC_TOWNMAP,       // タウンマップ
  BAG_NEXTPROC_FRIENDNOTE,    // ともだち手帳
  BAG_NEXTPROC_EXITEM,        
  BAG_NEXTPROC_RIDECYCLE,     // のる
  BAG_NEXTPROC_DROPCYCLE,     // おりる
  BAG_NEXTPROC_EVOLUTION,     // しんか
  BAG_NEXTPROC_PALACEJUMP,    // パレス移動
  BAG_NEXTPROC_ANANUKENOHIMO, // あなぬけのひも
  BAG_NEXTPROC_AMAIMITU,      // あまいミツ
  BAG_NEXTPROC_TURIZAO,       // つりざお
  BAG_NEXTPROC_BATTLERECORDER,// バトルレコーダー
  BAG_NEXTPROC_MAILVIEW,      // メール閲覧
};


// フィールドで使用・不使用チェックの収集に使用するID
// ※新しく使用する道具が増えた場合はこのenum定義に新しい名前を追加する
//   且つ、bag_param.hのBAG_SetItemUseCheck関数で道具使用・不使用の状態を格納する必要あり
typedef enum {
  ITEMCHECK_CYCLE_RIDE=0, // 自転車のる
  ITEMCHECK_CYCLE_STOP,   // 自転車おりる
  ITEMCHECK_TOWNMAP,      // タウンマップ
  ITEMCHECK_WIFINOTE,     // ともだち手帳
  ITEMCHECK_ANANUKENOHIMO,// あなぬけのヒモ
  ITEMCHECK_AMAIMITU,     // あまいミツ
  ITEMCHECK_TURIZAO,      // つりざお
  ITEMCHECK_BATTLE_RECORDER,  // バトルレコーダー
  ITEMCHECK_MAIL,         // メール
  ITEMCHECK_SPLAY,        // スプレー
  ITEMCHECK_MAX,

} ITEMCHECK_ENABLE ;




//============================================================================================
//  定数定義
//============================================================================================
// アイテム使用ワーク
typedef struct {
  GMEVENT * event;  // イベント
  u16 item;         // アイテム番号
  u8  use_poke;       // 使用するポケモン
} ITEMUSE_WORK;

// アイテム使用チェックワーク
typedef struct {
  int zone_id;  // ゾーンID
  int PlayerForm; //　自機の形状（自転車に乗っているかとか）
  
  u16 Companion;  //　連れ歩き状態か？
  u16 FrontAttr;  //　自機前方アトリビュート
  u16 NowAttr;  //　自機現在アトリビュート
  u16 SeedInfo; //使用可能なアイテム情報（きのみ関連）
   GAMESYS_WORK * gsys;
  u32 item_enable;   // 指定の道具の使用許可・不許可を指定するbitフィールド
                     // (参照はenum ITEMCHECK_ENABLEから）
}ITEMCHECK_WORK;



// アイテム使用チェック関数
typedef u32 (*ITEMCHECK_FUNC)(const ITEMCHECK_WORK*);

// アイテム使用関数
typedef void (*ITEMUSE_FUNC)(ITEMUSE_WORK*, const ITEMCHECK_WORK *);

// アイテム使用関数取得パラメータ
enum {
  ITEMUSE_PRM_USEFUNC = 0,  // バッグからの使用関数
  ITEMUSE_PRM_CNVFUNC,    // 便利ボタンからの使用関数
  ITEMUSE_PRM_CHECKFUNC   // チェック関数
};

// アイテム使用チェックの戻り値
enum {
  ITEMCHECK_TRUE = 0,       // 使用可能

  ITEMCHECK_ERR_CYCLE_OFF,    // 自転車を降りれない
  ITEMCHECK_ERR_COMPANION,    // 使用不可・連れ歩き
  ITEMCHECK_ERR_DISGUISE,     // 使用不可・ロケット団変装中

  ITEMCHECK_FALSE = 0xffffffff  // 使用不可・博士の言葉
};



//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 使用関数取得
 *
 * @param prm   取得パラメータ
 * @param id    関数ID
 *
 * @return  使用関数
 */
//--------------------------------------------------------------------------------------------
extern u32 ITEMUSE_GetFunc( u16 prm, u16 id );

//--------------------------------------------------------------------------------------------
/**
 * 使用チェックワーク作成
 *
 * @param fsys  フィールドワーク
 * @param id    チェックワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void ItemUse_CheckWorkMake( GAMESYS_WORK * fsys, ITEMCHECK_WORK * icwk );

//--------------------------------------------------------------------------------------------
/**
 * きのみ使用タイプ取得
 *
 * @param iuwk  アイテム使用ワーク
 *
 * @retval  "TRUE = 埋める"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ItemUse_KinomiTypeCheck( const ITEMCHECK_WORK * icwk );

//--------------------------------------------------------------------------------------------
/**
 * 便利ボタン起動処理
 *
 * @param repw  フィールドワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
//extern int FieldConvButtonEventInit( GAMESYS_WORK * repw, int no );


extern u32 ITEMUSE_GetUseFunc( u16 prm, u16 id );



//=============================================================================================
/**
 * @brief 指定の道具が使えるか許可・不許可情報を格納する(itemmenu.h内のenum定義ITEMCHECK_ENABLE)
 *
 * @param   param   BAG_PARAMのポインタ
 * @param   type    使う道具の種類（itemmenu.h内のenum定義ITEMCHECK_ENABLEから指定）
 * @param   check   0:使えない  1:使える
 *
 * @retval  none
 */
//=============================================================================================
extern void ITEMUSE_SetItemUseCheck( ITEMCHECK_WORK * icwk, ITEMCHECK_ENABLE type, BOOL check );


