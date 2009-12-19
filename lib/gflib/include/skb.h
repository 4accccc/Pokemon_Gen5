//============================================================================================
/**
 * @file  skb.h
 * @brief ソフトウエアキーボード
 * @author
 * @date
 */
//============================================================================================
#pragma once
//
//  使用の際にはgraphicフォルダ内のskb.narcをルートに配置してください
//  （"skb.narc"パス指定で取得できる場所）
//  また、システムフォント初期化 GFL_TEXT_CreateSystem() も事前に行っておいてください。
//
//  使用BG面のコントロールレジスタは呼び出し元で設定された状態に依存します。
//
typedef struct _GFL_SKB GFL_SKB;

#define GFL_SKB_STRLEN_MAX  (24)  //入力文字列最大数
#define GFL_SKB_STRBUF_SIZ  (sizeof(u16)*(GFL_SKB_STRLEN_MAX+1))//入力文字列バッファ確保サイズ

typedef enum {
  GFL_SKB_MODE_HIRAGANA = 0,
  GFL_SKB_MODE_KATAKANA,
  GFL_SKB_MODE_ENGNUM,
}GFL_SKB_MODE;

typedef enum {
  GFL_SKB_STRTYPE_STRBUF = 0,
  GFL_SKB_STRTYPE_SJIS,//配列の大きさは長さ*sizeof(u16)にすること
}GFL_SKB_STRTYPE;

//　設定定義
//　文字列バッファは文字列長さ*sizeof(u16)を確保すること
typedef struct {
  u32       strlen;       //文字列格納長さ
  GFL_SKB_STRTYPE strtype;//文字列格納タイプ

  GFL_SKB_MODE  mode;     //初期モード
  BOOL      modeChange;   //モード変更可否フラグ
  u16       cancelKey;    //中断キー判定トリガ（不要なら0でよい。その場合は中断不可)
  u16       decideKey;    //確定キー判定トリガ（不要なら0でよい。その場合はタッチパネル操作でのみ確定可）

  GFL_DISPUT_BGID   bgID;        //使用BGVRAMID
  GFL_DISPUT_PALID  bgPalID;     //使用BGパレットID
  GFL_DISPUT_PALID  bgPalID_on;  //使用BGパレットID

}GFL_SKB_SETUP;

typedef enum {

  GFL_SKB_REACTION_QUIT = 0,  ///< 終了した
  GFL_SKB_REACTION_NONE,      ///< 反応無し
  GFL_SKB_REACTION_INPUT,     ///< 何か文字が入力された
  GFL_SKB_REACTION_BACKSPACE, ///< BackSpace入力された
  GFL_SKB_REACTION_PAGE,      ///< ページ切り替え入力された

}GflSkbReaction;
//============================================================================================
/**
 *
 * @brief システム起動&終了
 *
 */
//============================================================================================
extern GFL_SKB*   GFL_SKB_Create( void* strings, const GFL_SKB_SETUP* setup, HEAPID heapID );
extern void     GFL_SKB_Delete( GFL_SKB* gflSkb );
extern GflSkbReaction     GFL_SKB_Main( GFL_SKB* gflSkb );  //FALSEで終了

extern void GFL_SKB_PickStr( GFL_SKB* skb );
extern void GFL_SKB_ReloadStr( GFL_SKB* skb, const STRBUF* strbuf );

//SjisCode格納用文字列バッファ作成
//GFL_SKB_STRTYPE_SJISを使用する場合、この関数で入力最大配列を確保出来る
extern void*  GFL_SKB_CreateSjisCodeBuffer( HEAPID heapID );
extern void   GFL_SKB_DeleteSjisCodeBuffer( void* strbuf );

