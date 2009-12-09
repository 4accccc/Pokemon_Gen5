/**
 *  @file mail_util.h
 *  @brief  メールセーブデータ参照用不完全型定義＆ユーティリティ関数定義
 *          GAMEDATA構造体やセーブデータにアクセスする際の関数をまとめています。
 *          メールデータを操作したい場合はmail.hで。
 *          
 *  @author AkitoMori / MiyukiIwasawa
 *  @date 09.11.16
 */

#pragma once

///参照定義
#include "gamesystem/game_data.h"
#include "poke_tool/poke_tool.h"
#include "mail.h"

//------------------------------------------------------------
/**
 * @brief メールセーブデータブロック管理構造への不完全型ポインタ
 *
 * 中身は見えないけどポインタ経由で参照はできる
 */
//------------------------------------------------------------
typedef struct _MAIL_BLOCK MAIL_BLOCK;



//=============================================================
/**
 *  メールセーブデータブロックアクセス系関数
 */
//=============================================================
//
/**
 *  @brief  セーブデータブロックへのポインタを取得
 */
//extern MAIL_BLOCK* SaveData_GetMailBlock(SAVE_CONTROL_WORK* sv);
extern MAIL_BLOCK* GAMEDATA_GetMailBlock( GAMEDATA *gamedata);

/**
 *  @brief  メールセーブデータブロックサイズ取得
 *
 *  ＊メールデータ一通のサイズではないので注意！
 */
extern int MAIL_GetBlockWorkSize(void);

/**
 *  @brief  メールセーブデータブロック初期化
 */
extern void MAIL_Init(MAIL_BLOCK* dat);

/**
 *  @brief  空いているメールデータIDを取得
 *
 *  @param  id 追加したいメールブロックID
 *
 *  @return int データを追加できる場合は参照ID
 *        追加できない場合はマイナス値が返る
 */
extern int MAIL_SearchNullID(MAIL_BLOCK* block,MAILBLOCK_ID id);

/**
 *  @brief  メールデータを削除
 *
 *  @param  blockID ブロックのID
 *  @param  dataID  データID
 */
extern void MAIL_DelMailData(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID);

/**
 *  @brief  メールデータをセーブブロックに追加
 *
 *  ＊引き渡したMAIL_DATA構造体型データの中身がセーブデータに反映されるので
 *  　おかしなデータを入れないように注意！
 */
extern void MAIL_AddMailFormWork(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID,MAIL_DATA* src);

/**
 *  @brief  指定ブロックに有効データがいくつあるか返す
 */
extern int MAIL_GetEnableDataNum(MAIL_BLOCK* block,MAILBLOCK_ID blockID);

/**
 *  @brief  メールデータのコピーを取得
 *
 *  ＊内部でメモリを確保するので、呼び出し側が責任持って領域を開放すること
 */
extern MAIL_DATA* MAIL_AllocMailData(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID,HEAPID heapID);

/**
 *  @brief  メールデータのコピーを取得
 *
 *  ＊あらかじめ確保したMAIL_DATA型メモリにセーブデータをコピーして取得
 */
extern void MAIL_GetMailData(MAIL_BLOCK* block,MAILBLOCK_ID blockID,int dataID,MAIL_DATA* dest);


//=============================================================
/**
 *  外部連動メールデータ関連
 */
//=============================================================

/**
 *  @brief  デザインNo,持たせるポケモンのポジション、セーブデータを指定して
 *      メールデータを新規作成状態に初期化
 *  @param  dat データを作成するMAIL_DATA構造体型へのポインタ
 *  @param  design_no メールのイメージNo
 *  @param  pos   メールを持たせるポケモンの手持ち内のポジション
 *  @param  save  セーブデータへのポインタ
 */
extern void MAILDATA_CreateFromSaveData( MAIL_DATA* dat,u8 design_no,u8 pos,GAMEDATA *gamedata );

/**
 *  @brief  メールポケモンイベント専用メールデータ生成
 *
 *  @param  pp      メールを持たせるポケモンのポケパラ
 *  @param  design_no メールのイメージNo
 *  @param  oya_sex   親の性別
 *  @param  oya_name  親の名前
 *  @param  oya_id    親のトレーナーID
 *
 *  @com  メモリをAllocして返すので、呼び出し側が解放すること！
 */
extern MAIL_DATA* MailData_MailEventDataMake(POKEMON_PARAM* pp,
  u8 design_no,u8 oya_sex,STRBUF* oya_name,u8 oya_id, HEAPID heapID);



