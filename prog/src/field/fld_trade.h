//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		  fld_trade.h
 *	@brief		ゲーム内交換
 *	@author		tomoya takahasi, miyuki iwasawa	(obata)
 *	@data		  2006.05.15 (2009.12.09 HGSSより移植)
 *
 *	このソースは field_tradeオーバーレイ領域に属しています
 *	このソースに定義された全関数は field_tradeオーバーレイ領域を明示的にロードしてからでないと
 *	利用できないので注意してください！
 *
 *	オーバーレイロード＆アンロード構文
 *
 *	#include "system/pm_overlay.h"
 *
 *	FS_EXTERN_OVERLAY(field_trade);
 *	Overlay_Load( FS_OVERLAY_ID( field_trade ), OVERLAY_LOAD_NOT_SYNCHRONIZE );
 *	Overlay_UnloadID( FS_OVERLAY_ID( field_trade ) );
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __FIELD_TRADE_H__
#define __FIELD_TRADE_H__

#if 0
//アセンブラでincludeされている場合は、下の宣言を無視できるようにifndefで囲んである
#ifndef	__ASM_NO_DEF_
#include "demo/demo_trade.h"
#include "savedata/mail_util.h"
#endif // __ASM_NO_DEF_
#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	交換データナンバー
//=====================================
#define	FLD_TRADE_MADATUBOMI	(0)
#define	FLD_TRADE_SURIIPU		(1)
#define	FLD_TRADE_KURABU		(2)
#define	FLD_TRADE_HAKURYUU		(3)
#define	FLD_TRADE_KUSAIHANA		(4)
#define	FLD_TRADE_RAKKII		(5)
#define	FLD_TRADE_TUBOTUBO		(6)
#define	FLD_TRADE_ONISUZUME		(7)
#define	FLD_TRADE_DAGUTORIO		(8)
#define	FLD_TRADE_GOOSUTO		(9)
#define	FLD_TRADE_PIKACHU		(10)
#define	FLD_TRADE_USOHATI		(11)
#define	FLD_TRADE_SEIDOO		(12)

#define	FLD_TRADE_NUM			(13)

//-------------------------------------
//	預かりポケモン返却チェックエラーコード
//=====================================
#define KEEP_POKE_RERR_NONE		(0)	//エラーなし
#define KEEP_POKE_RERR_NGPOKE	(1)	//違うポケモン
#define KEEP_POKE_RERR_ITEM		(2)	//アイテムを持ってる
#define KEEP_POKE_RERR_CBALL	(3)	//ボールカプセルがセットされてる
#define KEEP_POKE_RERR_ONLYONE	(4)	//手持ちが一匹しかいない

//アセンブラでincludeされている場合は、下の宣言を無視できるようにifndefで囲んである
#ifndef	__ASM_NO_DEF_

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
typedef struct _FLD_TRADE_WORK FLD_TRADE_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *
 *		説明
 *			FLD_TRADE_WORK*  をアロックしてきたらもう
 *			下の関数でデータを取得できます。
 *	extern u32 FLD_TRADE_WORK_GetTradeMonsno( const FLD_TRADE_WORK* cp_work );
 * 	extern u32FLD_TRADE_WORK_GetChangeMonsno( const FLD_TRADE_WORK* cp_work );
 *	extern u32 FLD_TRADE_WORK_GetTradeMonsOyaSex( const FLD_TRADE_WORK* cp_work );
 *
 *			交換するポケモンが決まったら
 *			EventCmd_FldTradeをコールしてください。(field_trade_cmd.h)
 *			
 *			交換が終了したら
 *			FLD_TRADE_WORK*を破棄してください
 *
 */
//-----------------------------------------------------------------------------
#include "field/fieldmap_proc.h"  // for FIELDMAP_WORK


extern FLD_TRADE_WORK* FLD_TRADE_WORK_Create( u32 heapID, u32 trade_no );
extern void FLD_TRADE_WORK_Delete( FLD_TRADE_WORK* p_work ); 


//----------------------
// ■交換データ取得関係
//----------------------
// 交換でもらうポケモンナンバー
extern u32 FLD_TRADE_WORK_GetTradeMonsno( const FLD_TRADE_WORK* cp_work );		
// 交換に必要なポケモンナンバー
extern u32 FLD_TRADE_WORK_GetChangeMonsno( const FLD_TRADE_WORK* cp_work );	
// 交換に必要なポケモンの性別
extern u32 FLD_TRADE_WORK_GetChangeMonsSex( const FLD_TRADE_WORK* cp_work );	
// 親の性別
extern u32 FLD_TRADE_WORK_GetTradeMonsOyaSex( const FLD_TRADE_WORK* cp_work );		


// 交換の実行
#if 0
extern void FLD_Trade( FIELDMAP_WORK* fieldmap, FLD_TRADE_WORK* p_work, int party_pos );
#endif

#if 0
// デモデータ作成
extern void FLD_TradeDemoDataMake( FIELDMAP_WORK* fieldmap, FLD_TRADE_WORK* p_work, int party_pos, DEMO_TRADE_PARAM* p_demo, POKEMON_PARAM* sendPoke, POKEMON_PARAM* recvPoke );
#endif

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	フィールド　ポケモン預かり(他人名義のポケモンを貰う)
 *
 *	@param	heapID		ヒープID
 *	@param	trade_no	交換番号
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
extern void FLD_KeepPokemonAdd( FIELDMAP_WORK* fieldmap, u8 trade_no ,u8 level,u16 zoneID);
#endif

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	フィールド　メールポケモンイベント専用メールデータを生成(オニスズメのメール専用)
 *
 *	@param	heapID		ヒープID
 *	@param	trade_no	交換番号
 *
 *	@return	ワーク
 *
 *	@com	メモリをAllocして返すので、呼び出し側が解放すること！
 */
//-----------------------------------------------------------------------------
extern MAIL_DATA* FLD_MakeKeepPokemonEventMail( void );
#endif


#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	フィールド　預かりポケモン返却可不可チェック
 *
 *	手持ちのポケモンが預かりポケモンと一致するかや、アイテムの有無をチェック
 *
 *	@param	trade_no	交換番号
 *	@param	pos			チェックする手持ちのポケモン位置
 *
 *	@retval	KEEP_POKE_RERR_NONE		返せる
 *	@retval	KEEP_POKE_RERR_NGPOKE	違うポケモンだから返せない
 *	@retval	KEEP_POKE_RERR_ITEM		アイテムを持ってるから返せない
 *	@retval	KEEP_POKE_RERR_CBALL	カスタムボールがセットされているから返せない
 */
//-----------------------------------------------------------------------------
extern u16 FLD_KeepPokemonReturnCheck( FIELDMAP_WORK* fieldmap, u8 trade_no ,u8 pos); 
#endif

#endif	__ASM_NO_DEF_

#endif		// __FIELD_TRADE_H__

