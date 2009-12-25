
//=============================================================================
/**
 * @file	tr_ai_seq_def.h
 * @brief	トレーナーAI用のマクロ定義ファイル（アドバンス版を移植）
 * @author	HisashiSogabe
 * @date	2006.03.22
 */
//=============================================================================

#define	__ASM_NO_DEF_
	.include	"../../prog/src/battle/tr_ai/tr_ai_def.h"
	.include	"../../prog/include/waza_tool/wazano_def.h"
	.include	"../../prog/include/poke_tool/poke_tool.h"
	.include	"../../prog/include/poke_tool/tokusyu_def.h"
	.include	"../../prog/include/item/itemsym.h"
	.include	"../../prog/include/item/itemequip.h"
  .include  "battle_def.h"
  .include  "btl_calc_def.h"
  .include  "btl_field_def.h"
  .include  "btl_pokeparam_def.h"
  .include  "btl_sideeff_def.h"
  .include  "poketype_def.h"
  .include  "wazadata_def.h"

//勝手にパディングをいれられないようにする
  .option alignment off

//======================================================
//	AIの動作シーケンス命令macro
//======================================================

	.macro	INIT_CMD
DEF_CMD_COUNT	=	0
	.endm

	.macro	DEF_CMD	symname
\symname	=	DEF_CMD_COUNT
DEF_CMD_COUNT 	=	(DEF_CMD_COUNT + 1)
	.endm

	INIT_CMD
//ランダム分岐
	DEF_CMD		AI_IF_RND_UNDER
	DEF_CMD		AI_IF_RND_OVER
	DEF_CMD		AI_IF_RND_EQUAL
	DEF_CMD		AI_IFN_RND_EQUAL

//ポイント加減算
	DEF_CMD		AI_INCDEC

//HPでの分岐
	DEF_CMD		AI_IF_HP_UNDER
	DEF_CMD		AI_IF_HP_OVER
	DEF_CMD		AI_IF_HP_EQUAL
	DEF_CMD		AI_IFN_HP_EQUAL

//状態異常をチェック
	DEF_CMD		AI_IF_POKESICK      //なにかしらの状態異常になっている
	DEF_CMD		AI_IFN_POKESICK
	DEF_CMD		AI_IF_WAZASICK      //状態異常の種類を選択して調べる
	DEF_CMD		AI_IFN_WAZASICK
	DEF_CMD		AI_IF_CONTFLG       //技効果の種類を選択して調べる
	DEF_CMD		AI_IFN_CONTFLG
	DEF_CMD		AI_IF_SIDEEFF
	DEF_CMD		AI_IFN_SIDEEFF  		//0x10

//計算結果による分岐
	DEF_CMD		AI_IF_UNDER
	DEF_CMD		AI_IF_OVER
	DEF_CMD		AI_IF_EQUAL
	DEF_CMD		AI_IFN_EQUAL
	DEF_CMD		AI_IF_BIT
	DEF_CMD		AI_IFN_BIT

//技ナンバーのチェック
	DEF_CMD		AI_IF_WAZANO
	DEF_CMD		AI_IFN_WAZANO

//テーブル参照による分岐
	DEF_CMD		AI_IF_TABLE_JUMP
	DEF_CMD		AI_IFN_TABLE_JUMP

//自分がダメージ技を持っているか判断して分岐
	DEF_CMD		AI_IF_HAVE_DAMAGE_WAZA
	DEF_CMD		AI_IFN_HAVE_DAMAGE_WAZA

//ターンのチェック
	DEF_CMD		AI_CHECK_TURN

//タイプのチェック
	DEF_CMD		AI_CHECK_TYPE

//攻撃技かどうかのチェック
	DEF_CMD		AI_CHECK_IRYOKU

//威力が一番高いかのチェック
	DEF_CMD		AI_COMP_POWER

//前のターンに使った技のチェック
	DEF_CMD		AI_CHECK_LAST_WAZA

//格納された技のタイプチェック
	DEF_CMD		AI_IF_WAZA_TYPE			//0x20
	DEF_CMD		AI_IFN_WAZA_TYPE

//どちらが先行かのチェック
	DEF_CMD		AI_IF_FIRST

//控えが何体いるかのチェック
	DEF_CMD		AI_CHECK_BENCH_COUNT

//現在の技ナンバーのチェック
	DEF_CMD		AI_CHECK_WAZANO

//現在の技ナンバーのシーケンスナンバーのチェック
	DEF_CMD		AI_CHECK_WAZASEQNO

//特殊能力のチェック
	DEF_CMD		AI_CHECK_TOKUSEI

//自分と相手の相性チェック
	DEF_CMD		AI_CHECK_AISYOU
	DEF_CMD		AI_CHECK_WAZA_AISYOU

//控えの状態チェック
	DEF_CMD		AI_IF_BENCH_COND
	DEF_CMD		AI_IFN_BENCH_COND

//天候チェック
	DEF_CMD		AI_CHECK_WEATHER

//技のシーケンスナンバーをチェックして、分岐
	DEF_CMD		AI_IF_WAZA_SEQNO_JUMP
	DEF_CMD		AI_IFN_WAZA_SEQNO_JUMP

//自分や相手のパラメータ変化値を参照して分岐
	DEF_CMD		AI_IF_PARA_UNDER
	DEF_CMD		AI_IF_PARA_OVER				//0x30
	DEF_CMD		AI_IF_PARA_EQUAL
	DEF_CMD		AI_IFN_PARA_EQUAL

//技のダメージ計算をして相手が瀕死になるかチェック
	DEF_CMD		AI_IF_WAZA_HINSHI
	DEF_CMD		AI_IFN_WAZA_HINSHI

//特定の技を持っているかのチェックをして分岐
	DEF_CMD		AI_IF_HAVE_WAZA
	DEF_CMD		AI_IFN_HAVE_WAZA

//特定の技シーケンスを持っているかのチェックをして分岐
	DEF_CMD		AI_IF_HAVE_WAZA_SEQNO
	DEF_CMD		AI_IFN_HAVE_WAZA_SEQNO

//技の状態をチェックをして分岐
	DEF_CMD		AI_IF_WAZA_CHECK_STATE

//にげるをせんたく
	DEF_CMD		AI_ESCAPE

//サファリゾーンでの逃げる確率を計算して逃げるときのアドレスを指定
	DEF_CMD		AI_SAFARI_ESCAPE_JUMP

//サファリゾーンでの特殊アクションを選択
	DEF_CMD		AI_SAFARI

//装備アイテムのチェック
	DEF_CMD		AI_CHECK_SOUBI_ITEM

//装備効果のチェック
	DEF_CMD		AI_CHECK_SOUBI_EQUIP

//ポケモンの性別のチェック
	DEF_CMD		AI_CHECK_POKESEX

//ねこだましカウンタのチェック
	DEF_CMD		AI_CHECK_NEKODAMASI			//0x40

//たくわえるカウンタのチェック
	DEF_CMD		AI_CHECK_TAKUWAERU

//対戦ルールのチェック
	DEF_CMD		AI_CHECK_BTL_RULE

//対戦相手のチェック
	DEF_CMD		AI_CHECK_BTL_COMPETITOR

//リサイクルできるアイテムのチェック
	DEF_CMD		AI_CHECK_RECYCLE_ITEM

//ワークに入っている技ナンバーのタイプをチェック
	DEF_CMD		AI_CHECK_WORKWAZA_TYPE

//ワークに入っている技ナンバーの威力をチェック
	DEF_CMD		AI_CHECK_WORKWAZA_POW

//ワークに入っている技ナンバーのシーケンスナンバーをチェック
	DEF_CMD		AI_CHECK_WORKWAZA_SEQNO

//まもるカウンタをチェック
	DEF_CMD		AI_CHECK_MAMORU_COUNT

//汎用的な命令
	DEF_CMD		AI_JUMP
	DEF_CMD		AI_AIEND

//お互いのレベルをチェックして分岐
	DEF_CMD		AI_IF_LEVEL

//挑発状態かチェックして分岐
	DEF_CMD		AI_IF_CHOUHATSU
	DEF_CMD		AI_IFN_CHOUHATSU

//攻撃対象が味方がどうかチェック
	DEF_CMD		AI_IF_MIKATA_ATTACK

//指定タイプを持っているかどうかチェック
	DEF_CMD		AI_CHECK_HAVE_TYPE

//指定とくせいを持っているかどうかチェック
	DEF_CMD		AI_CHECK_HAVE_TOKUSEI		//0x50

//相方がもらいびパワーアップ状態かチェックして分岐
	DEF_CMD		AI_IF_ALREADY_MORAIBI

//アイテムを持っているかチェックして分岐
	DEF_CMD		AI_IF_HAVE_ITEM

//FLDEFFチェック
	DEF_CMD		AI_FLDEFF_CHECK

//SIDE_CONDITIONのカウントを取得
	DEF_CMD		AI_CHECK_SIDEEFF_COUNT

//控えポケモンのHP減少をチェック
	DEF_CMD		AI_IF_BENCH_HPDEC

//控えポケモンのPP減少をチェック
	DEF_CMD		AI_IF_BENCH_PPDEC

//装備アイテムのなげつける威力を取得
	DEF_CMD		AI_CHECK_NAGETSUKERU_IRYOKU

//PPを取得
	DEF_CMD		AI_CHECK_PP_REMAIN

//とっておきチェック
	DEF_CMD		AI_IF_TOTTEOKI

//技の分類チェック
	DEF_CMD		AI_CHECK_WAZA_KIND

//相手が最後に出した技の分類チェック
	DEF_CMD		AI_CHECK_LAST_WAZA_KIND

//素早さで指定した側が何位かチェック
	DEF_CMD		AI_CHECK_AGI_RANK

//スロースタート何ターン目か
	DEF_CMD		AI_CHECK_SLOWSTART_TURN

//控えにいる方がダメージを与えるかどうかチェック
	DEF_CMD		AI_IF_BENCH_DAMAGE_MAX

//抜群の技を持っているかチェック
	DEF_CMD		AI_IF_HAVE_BATSUGUN

//指定した相手の最後に出した技と自分の技とのダメージをチェック
	DEF_CMD		AI_IF_LAST_WAZA_DAMAGE_CHECK

//指定した相手のステータス上昇分の値を取得
	DEF_CMD		AI_CHECK_STATUS_UP

//指定した相手とのステータス差分を取得
	DEF_CMD		AI_CHECK_STATUS_DIFF

//指定した相手とのステータスをチェックして分岐
	DEF_CMD		AI_IF_CHECK_STATUS_UNDER
	DEF_CMD		AI_IF_CHECK_STATUS_OVER
	DEF_CMD		AI_IF_CHECK_STATUS_EQUAL

//威力が一番高いかのチェック（パートナーも含む）
	DEF_CMD		AI_COMP_POWER_WITH_PARTNER

//指定した相手が瀕死かチェックして分岐
	DEF_CMD		AI_IF_HINSHI
	DEF_CMD		AI_IFN_HINSHI

//技効果を考慮した特性取得（移動ポケモン専用それ以外の用途で使用するのは厳禁）
	DEF_CMD		AI_GET_TOKUSEI

//みがわり中か？
	DEF_CMD		AI_IF_MIGAWARI

//------------------------------------------------------------
//
//	ランダム分岐
//
//	value:比較する値
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_RND_UNDER		value,adrs
	.short	AI_IF_RND_UNDER
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_RND_OVER			value,adrs
	.short	AI_IF_RND_OVER
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_RND_EQUAL		value,adrs
	.short	AI_IF_RND_EQUAL
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_RND_EQUAL		value,adrs
	.short	AI_IFN_RND_EQUAL
	.long		\value
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	ポイント加減算
//
//	value:加減算する値
//
//------------------------------------------------------------

	.macro	INCDEC				value
	.short	AI_INCDEC
	.long		\value
	.endm

//------------------------------------------------------------
//
//	HPでの分岐
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	value:比較する値
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_HP_UNDER			side,value,adrs
	.short	AI_IF_HP_UNDER
	.long		\side
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_HP_OVER			side,value,adrs
	.short	AI_IF_HP_OVER
	.long		\side
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_HP_EQUAL			side,value,adrs
	.short	AI_IF_HP_EQUAL
	.long		\side
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_HP_EQUAL			side,value,adrs
	.short	AI_IFN_HP_EQUAL
	.long		\side
	.long		\value
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	状態異常をチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	condition:比較する値（server.defに定義）
//	adrs:飛び先
//
//------------------------------------------------------------

//CONDITION_～系のチェック
	.macro	IF_POKESICK				side,adrs
	.short	AI_IF_POKESICK
	.long		\side
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_POKESICK			side,adrs
	.short	AI_IFN_POKESICK
	.long		\side
	.long		(\adrs-.)-4
	.endm

//WAZASICK_～系のチェック
	.macro	IF_WAZASICK   side,condition,adrs
	.short	AI_IF_WAZASICK
	.long		\side
	.long		\condition
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_WAZASICK	side,condition,adrs
	.short	AI_IFN_WAZASICK
	.long		\side
	.long		\condition
	.long		(\adrs-.)-4
	.endm

//CONTFLG～系のチェック
	.macro	IF_CONTFLG   side,condition,adrs
	.short	AI_IF_CONTFLG
	.long		\side
	.long		\condition
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_CONTFLG	side,condition,adrs
	.short	AI_IFN_CONTFLG
	.long		\side
	.long		\condition
	.long		(\adrs-.)-4
	.endm

//SIDEEFF_～系のチェック
	.macro	IF_SIDEEFF	side,condition,adrs
	.short	AI_IF_SIDEEFF
	.long		\side
	.long		\condition
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_SIDEEFF	side,condition,adrs
	.short	AI_IFN_SIDEEFF
	.long		\side
	.long		\condition
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	計算結果による分岐（ワークに格納する系の命令のあとに実行する）
//
//	value:比較する値
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_UNDER			value,adrs
	.short	AI_IF_UNDER
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_OVER				value,adrs
	.short	AI_IF_OVER
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_EQUAL			value,adrs
	.short	AI_IF_EQUAL
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_EQUAL			value,adrs
	.short	AI_IFN_EQUAL
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_BIT				value,adrs
	.short	AI_IF_BIT
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_BIT				value,adrs
	.short	AI_IFN_BIT
	.long		\value
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	技ナンバーをチェックして分岐
//------------------------------------------------------------

	.macro	IF_WAZANO		wazano,adrs
	.short	AI_IF_WAZANO
	.long		\wazano
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_WAZANO		wazano,adrs
	.short	AI_IF_WAZANO
	.long		\wazano
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	指定されたテーブルを参照して一致、不一致で分岐
//
//	tableadrs:チェックするナンバーが羅列されたテーブルのアドレス
//	jumpadrs:条件に合致したときのとび先	
//
//	テーブルの終端は1BYTEの方は0xff、2BYTEの方は0xffff
//
//------------------------------------------------------------

	.macro	IF_TABLE_JUMP		tableadrs,jumpadrs	
	.short	AI_IF_TABLE_JUMP
	.long		\tableadrs
	.long		(\jumpadrs-.)-4
	.endm

	.macro	IFN_TABLE_JUMP		tableadrs,jumpadrs
	.short	AI_IFN_TABLE_JUMP
	.long		\tableadrs
	.long		(\jumpadrs-.)-4
	.endm

//------------------------------------------------------------
//
//	自分がダメージ技を持っているか判断して分岐
//
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_HAVE_DAMAGE_WAZA		adrs
	.short	AI_IF_HAVE_DAMAGE_WAZA
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_HAVE_DAMAGE_WAZA	adrs
	.short	AI_IFN_HAVE_DAMAGE_WAZA
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	ターンのチェック（今何ターン目かをワークに入れる）
//------------------------------------------------------------

	.macro	CHECK_TURN
	.short	AI_CHECK_TURN
	.endm

//------------------------------------------------------------
//
//	タイプのチェック(ポケモンあるいは技のタイプをワークに入れる）
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_TYPE			side
	.short	AI_CHECK_TYPE
	.long		\side
	.endm

//------------------------------------------------------------
//	攻撃技かどうかのチェック（技の威力をワークに入れる)
//------------------------------------------------------------

	.macro	CHECK_IRYOKU
	.short	AI_CHECK_IRYOKU
	.endm

//------------------------------------------------------------
//	威力が一番高いかのチェック
//
//	@param		flag	ダメージロス計算のぶれありなしフラグ
//
//------------------------------------------------------------
	.macro	COMP_POWER		flag
	.short	AI_COMP_POWER
	.long		\flag
	.endm

//------------------------------------------------------------
//
//	前のターンに使った技のチェック（技のナンバーをワークに入れる）
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------
	.macro	CHECK_LAST_WAZA		side
	.short	AI_CHECK_LAST_WAZA
	.long		\side
	.endm

//------------------------------------------------------------
//
//	格納された技のタイプチェック
//
//	type:チェックするタイプ（agbmons.defに定義）	
//	adrs:飛び先
//
//------------------------------------------------------------
	.macro	IF_WAZA_TYPE		type,adrs
	.short	AI_IF_WAZA_TYPE
	.long		\type
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_WAZA_TYPE		type,adrs
	.short	AI_IFN_WAZA_TYPE
	.long		\type
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	どちらが先行かのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_FIRST			side,adrs
	.short	AI_IF_FIRST
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	控えが何体いるかチェック（数をワークに入れる）
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_BENCH_COUNT			side
	.short	AI_CHECK_BENCH_COUNT
	.long		\side
	.endm

//------------------------------------------------------------
//	現在の技ナンバーのチェック
//------------------------------------------------------------

	.macro	CHECK_WAZANO
	.short	AI_CHECK_WAZANO
	.endm

//------------------------------------------------------------
//	現在の技ナンバーのシーケンスナンバーのチェック
//------------------------------------------------------------

	.macro	CHECK_WAZASEQNO
	.short	AI_CHECK_WAZASEQNO
	.endm

//------------------------------------------------------------
//
//	特殊能力のチェック（特殊能力ナンバーをワークに入れる）
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_TOKUSEI		side
	.short	AI_CHECK_TOKUSEI
	.long		\side
	.endm

//------------------------------------------------------------
//
//	自分と相手の相性チェックして一番いい倍率をワークに格納
//
//------------------------------------------------------------

	.macro	CHECK_AISYOU
	.short	AI_CHECK_AISYOU
	.endm

//------------------------------------------------------------
//
//	技と相手の相性をチェックしてしてアドレスにジャンプ
//
//	aisyou:技と相手の相性による倍率（tr_ai_def.hに定義）
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	CHECK_WAZA_AISYOU		aisyou,adrs
	.short	AI_CHECK_WAZA_AISYOU
	.long		\aisyou
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	控えの状態チェック（なにがしかの状態異常になっているかをチェック
//
//------------------------------------------------------------

	.macro	IF_BENCH_COND		side,adrs
	.short	AI_IF_BENCH_COND
	.long		\side
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_BENCH_COND		side,adrs
	.short	AI_IFN_BENCH_COND
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	天候チェック（天候ナンバーをワークに入れる）
//				（天候ナンバーはtr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_WEATHER
	.short	AI_CHECK_WEATHER
	.endm

//------------------------------------------------------------
//
//	技のシーケンスナンバーをチェックして、分岐
//
//	seqno:技のシーケンスナンバー
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_WAZA_SEQNO_JUMP		seqno,adrs
	.short	AI_IF_WAZA_SEQNO_JUMP
	.long		\seqno
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_WAZA_SEQNO_JUMP		seqno,adrs
	.short	AI_IFN_WAZA_SEQNO_JUMP
	.long		\seqno
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	自分や相手のパラメータ変化値を参照して分岐
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	para:チェックするパラメータ（tr_ai_def.hに定義）
//	value:比較する値（０～１２）
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_PARA_UNDER		side,para,value,adrs
	.short	AI_IF_PARA_UNDER
	.long		\side
	.long		\para
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_PARA_OVER		side,para,value,adrs
	.short	AI_IF_PARA_OVER
	.long		\side
	.long		\para
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IF_PARA_EQUAL		side,para,value,adrs
	.short	AI_IF_PARA_EQUAL
	.long		\side
	.long		\para
	.long		\value
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_PARA_EQUAL		side,para,value,adrs
	.short	AI_IFN_PARA_EQUAL
	.long		\side
	.long		\para
	.long		\value
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	技のダメージ計算をして相手が瀕死になるかチェックして分岐
//
//	@param		flag	ダメージロス計算のぶれありなしフラグ
//	@param		adrs	飛び先
//
//------------------------------------------------------------

	.macro	IF_WAZA_HINSHI		flag,adrs
	.short	AI_IF_WAZA_HINSHI
	.long		\flag
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_WAZA_HINSHI		flag,adrs
	.short	AI_IFN_WAZA_HINSHI
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	特定の技を持っているかのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	wazano:チェックする技ナンバー
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_HAVE_WAZA		side,wazano,adrs
	.short	AI_IF_HAVE_WAZA
	.long		\side
	.long		\wazano
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_HAVE_WAZA		side,wazano,adrs
	.short	AI_IFN_HAVE_WAZA
	.long		\side
	.long		\wazano
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	特定の技シーケンスを持っているかのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	seqno:チェックする技シーケンスナンバー
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_HAVE_WAZA_SEQNO		side,seqno,adrs
	.short	AI_IF_HAVE_WAZA_SEQNO
	.long		\side
	.long		\seqno
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_HAVE_WAZA_SEQNO		side,seqno,adrs
	.short	AI_IFN_HAVE_WAZA_SEQNO
	.long		\side
	.long		\seqno
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	ポケモンの状態をチェックして分岐（かなしばりとか、アンコールとか）
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//	id:チェックする状態ID(tr_ai_def.hに定義）
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_POKE_CHECK_STATE		side,id,adrs
	.short	AI_IF_POKE_CHECK_STATE
	.long		\side
	.long		\id
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	今計算している技の状態をチェックして分岐
//
//	id:チェックする状態ID(tr_ai_def.hに定義）
//	adrs:飛び先
//
//------------------------------------------------------------

	.macro	IF_WAZA_CHECK_STATE		id,adrs
	.short	AI_IF_WAZA_CHECK_STATE
	.long		\id
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	にげるをせんたく
//------------------------------------------------------------

	.macro	ESCAPE
	.short	AI_ESCAPE
	.endm

//------------------------------------------------------------
//	サファリゾーンでの逃げる確率を計算して逃げるときのアドレスを指定
//------------------------------------------------------------

	.macro	SAFARI_ESCAPE_JUMP		adrs
	.short	AI_SAFARI_ESCAPE_JUMP
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	サファリゾーンでの特殊アクションを選択
//------------------------------------------------------------

	.macro	SAFARI
	.short	AI_SAFARI
	.endm


//------------------------------------------------------------
//
//		装備アイテムのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------
	.macro	CHECK_SOUBI_ITEM		side
	.short	AI_CHECK_SOUBI_ITEM
	.long		\side
	.endm

//------------------------------------------------------------
//
//		装備効果のチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_SOUBI_EQUIP		side
	.short	AI_CHECK_SOUBI_EQUIP
	.long		\side
	.endm

//------------------------------------------------------------
//
//		ポケモンの性別のチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_POKESEX		side
	.short	AI_CHECK_POKESEX
	.long		\side
	.endm

//------------------------------------------------------------
//
//		ねこだましカウンタのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_NEKODAMASI	side
	.short	AI_CHECK_NEKODAMASI
	.long		\side
	.endm

//------------------------------------------------------------
//
//		たくわえるカウンタのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_TAKUWAERU		side
	.short	AI_CHECK_TAKUWAERU
	.long		\side
	.endm

//------------------------------------------------------------
//		対戦ルールのチェック
//------------------------------------------------------------

	.macro	CHECK_BTL_RULE
	.short	AI_CHECK_BTL_RULE
	.endm

//------------------------------------------------------------
//		対戦相手のチェック
//------------------------------------------------------------

	.macro	CHECK_BTL_COMPETITOR
	.short	AI_CHECK_BTL_COMPETITOR
	.endm

//------------------------------------------------------------
//
//		リサイクルできるアイテムのチェック
//
//	side:チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------

	.macro	CHECK_RECYCLE_ITEM	side
	.short	AI_CHECK_RECYCLE_ITEM
	.long		\side
	.endm

//------------------------------------------------------------
//	ワークに入っている技ナンバーのタイプをチェック
//------------------------------------------------------------

	.macro	CHECK_WORKWAZA_TYPE
	.short	AI_CHECK_WORKWAZA_TYPE
	.endm

//------------------------------------------------------------
//	ワークに入っている技ナンバーの威力をチェック
//------------------------------------------------------------

	.macro	CHECK_WORKWAZA_POW
	.short	AI_CHECK_WORKWAZA_POW
	.endm

//------------------------------------------------------------
//	ワークに入っている技ナンバーのシーケンスナンバーをチェック
//------------------------------------------------------------

	.macro	CHECK_WORKWAZA_SEQNO
	.short	AI_CHECK_WORKWAZA_SEQNO
	.endm

//------------------------------------------------------------
//	まもるカウンタをチェック
//------------------------------------------------------------

	.macro	CHECK_MAMORU_COUNT		side
	.short	AI_CHECK_MAMORU_COUNT
	.long		\side
	.endm

//------------------------------------------------------------
//	汎用的な命令群
//------------------------------------------------------------

//汎用的な命令
	.macro	JUMP		adrs
	.short	AI_JUMP
	.long		(\adrs-.)-4
	.endm

	.macro	AIEND
	.short	AI_AIEND
	.endm

//------------------------------------------------------------
//	お互いのレベルをチェックして分岐
//------------------------------------------------------------

	.macro	IF_LEVEL		value,adrs
	.short	AI_IF_LEVEL
	.long		\value
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	挑発状態かチェックして分岐
//------------------------------------------------------------

	.macro	IF_CHOUHATSU	adrs
	.short	AI_IF_CHOUHATSU
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_CHOUHATSU	adrs
	.short	AI_IFN_CHOUHATSU
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	攻撃対象が味方がどうかチェックして分岐
//------------------------------------------------------------

	.macro	IF_MIKATA_ATTACK	adrs
	.short	AI_IF_MIKATA_ATTACK
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	指定されたタイプを持っているかチェック
//------------------------------------------------------------

	.macro	CHECK_HAVE_TYPE			side, type
	.short	AI_CHECK_HAVE_TYPE
	.long		\side
	.long		\type
	.endm

//------------------------------------------------------------
//	指定されたとくせいを持っているかチェック
//------------------------------------------------------------

	.macro	CHECK_HAVE_TOKUSEI		side, abi
	.short	AI_CHECK_HAVE_TOKUSEI
	.long		\side
	.long		\abi
	.endm

//------------------------------------------------------------
//	相方がもらいびパワーアップ状態かチェックして分岐
//------------------------------------------------------------

	.macro	IF_ALREADY_MORAIBI		side, adrs
	.short	AI_IF_ALREADY_MORAIBI
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	アイテムを持っているかチェックして分岐
//------------------------------------------------------------

	.macro	IF_HAVE_ITEM		side, item, adrs
	.long		AI_IF_HAVE_ITEM
	.long		\side
	.long		\item
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	FIELD_EFFECTチェック
//
//	@param	flag	チェックするフィールドコンディション（battle_server.hに定義）
//	@param	adrs	チェックしたフィールドコンディション中だった時のとび先
//
//------------------------------------------------------------
	.macro	FLDEFF_CHECK	flag,adrs
	.short	AI_FLDEFF_CHECK
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	SIDEEFFのカウントを取得
//
//	@param	side	取得する側を指定（tr_ai_def.hに定義）
//	@param	flag	チェックするフィールドコンディション（battle_server.hに定義）
//
//------------------------------------------------------------
	.macro	CHECK_SIDEEFF_COUNT	side,flag
	.short	AI_CHECK_SIDEEFF_COUNT
	.long		\side
	.long		\flag
	.endm
	
//------------------------------------------------------------
//	控えポケモンのHP減少をチェック
//
//	@param	side	チェックする側を指定（tr_ai_def.hに定義）
//	@param	adrs	HP減少したポケモンがいた時のとび先
//
//------------------------------------------------------------
	.macro	IF_BENCH_HPDEC		side,adrs
	.short	AI_IF_BENCH_HPDEC
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	控えポケモンのPP減少をチェック
//
//	@param	side	チェックする側を指定（tr_ai_def.hに定義）
//	@param	adrs	PP減少したポケモンがいた時のとび先
//
//------------------------------------------------------------
	.macro	IF_BENCH_PPDEC		side,adrs
	.short	AI_IF_BENCH_PPDEC
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	装備アイテムのなげつける威力を取得
//
//	@param	side	取得する側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------
	.macro	CHECK_NAGETSUKERU_IRYOKU	side
	.short	AI_CHECK_NAGETSUKERU_IRYOKU
	.long		\side
	.endm

//------------------------------------------------------------
//	現在チェック中の技の残りPPを取得
//------------------------------------------------------------
	.macro	CHECK_PP_REMAIN
	.short	AI_CHECK_PP_REMAIN
	.endm

//------------------------------------------------------------
//
//	とっておきチェック
//
//	@param	side	チェックする側を指定（tr_ai_def.hに定義）
//	@param	adrs	とっておきを出せる時のとび先
//
//------------------------------------------------------------
	.macro	IF_TOTTEOKI		side,adrs
	.short	AI_IF_TOTTEOKI
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//	技の分類チェック
//------------------------------------------------------------
	.macro	CHECK_WAZA_KIND
	.short	AI_CHECK_WAZA_KIND
	.endm

//------------------------------------------------------------
//	相手が最後に出した技の分類チェック
//------------------------------------------------------------
	.macro	CHECK_LAST_WAZA_KIND
	.short	AI_CHECK_LAST_WAZA_KIND
	.endm

//------------------------------------------------------------
//
//	素早さで指定した側が何位かチェック
//
//	@param	side	チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------
	.macro	CHECK_AGI_RANK		side
	.short	AI_CHECK_AGI_RANK
	.long		\side
	.endm

//------------------------------------------------------------
//
//	スロースタート何ターン目か
//
//	@param	side	チェックする側を指定（tr_ai_def.hに定義）
//
//------------------------------------------------------------
	.macro	CHECK_SLOWSTART_TURN	side
	.short	AI_CHECK_SLOWSTART_TURN
	.long		\side
	.endm

//------------------------------------------------------------
//
//	控えにいる方がダメージを与えるかどうかチェック
//
//	@param	flag	ダメージロス計算のぶれありなしフラグ
//	@param	adrs	与える場合のとび先
//
//------------------------------------------------------------
	.macro	IF_BENCH_DAMAGE_MAX		flag,adrs
	.short	AI_IF_BENCH_DAMAGE_MAX
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	抜群の技を持っているかチェック
//
//	@param	adrs	持っていたときのとび先
//
//------------------------------------------------------------
	.macro	IF_HAVE_BATSUGUN		adrs
	.short	AI_IF_HAVE_BATSUGUN
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	指定した相手の最後に出した技と自分の技とのダメージをチェック
//
//	@param	side	チェックする側
//	@param	flag	ダメージロス計算のぶれありなしフラグ
//	@param	adrs	チェックした側の方がダメージが大きい時のとび先
//
//------------------------------------------------------------
	.macro	IF_LAST_WAZA_DAMAGE_CHECK	side,flag,adrs
	.short	AI_IF_LAST_WAZA_DAMAGE_CHECK
	.long		\side
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	指定した相手のステータス上昇分の値を取得
//
//	@param	side	チェックする側
//
//------------------------------------------------------------
	.macro	CHECK_STATUS_UP		side
	.short	AI_CHECK_STATUS_UP
	.long		\side
	.endm

//------------------------------------------------------------
//
//	指定した相手とのステータス差分を取得
//
//	@param	side	チェックする側
//	@param	flag	チェックするステータス種類
//					COND_POW:攻撃力
//					COND_DEF:防御力
//					COND_SPEPOW:特攻
//					COND_SPEDEF:特防
//
//------------------------------------------------------------
	.macro	CHECK_STATUS_DIFF		side,flag
	.short	AI_CHECK_STATUS_DIFF
	.long		\side
	.long		\flag
	.endm

//------------------------------------------------------------
//
//	指定した相手とのステータスをチェックして分岐
//
//	@param	side	チェックする側
//	@param	flag	チェックするステータス種類
//					COND_HP:HP
//					COND_POW:攻撃力
//					COND_DEF:防御力
//					COND_SPEPOW:特攻
//					COND_SPEDEF:特防
//	@param	adrs	分岐先
//
//------------------------------------------------------------
//自分が下
	.macro	IF_CHECK_STATUS_UNDER		side,flag,adrs
	.short	AI_IF_CHECK_STATUS_UNDER
	.long		\side
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//自分が上
	.macro	IF_CHECK_STATUS_OVER		side,flag,adrs
	.short	AI_IF_CHECK_STATUS_OVER
	.long		\side
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//同じ値
	.macro	IF_CHECK_STATUS_EQUAL		side,flag,adrs
	.short	AI_IF_CHECK_STATUS_EQUAL
	.long		\side
	.long		\flag
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	威力が一番高いかのチェック（パートナーも含む）
//
//	@param		flag	ダメージロス計算のぶれありなしフラグ
//
//------------------------------------------------------------
	.macro	COMP_POWER_WITH_PARTNER		flag
	.short	AI_COMP_POWER_WITH_PARTNER
	.long		\flag
	.endm

//------------------------------------------------------------
//
//	指定した相手が瀕死かチェックして分岐
//
//	@param		side	チェックする相手（CHECK_ATTACK、CHECK_DEFENCEは指定できません（瀕死がありえない））
//	@param		adrs	チェック結果での分岐先（IF_HINSHI：瀕死だった時 IFN_HINSHI：瀕死じゃなかったとき）
//
//------------------------------------------------------------
	.macro	IF_HINSHI		side,adrs
	.short	AI_IF_HINSHI
	.long		\side
	.long		(\adrs-.)-4
	.endm

	.macro	IFN_HINSHI		side,adrs
	.short	AI_IFN_HINSHI
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//
//	技効果を考慮した特性取得（移動ポケモン専用それ以外の用途で使用するのは厳禁）
//
//	@param		side	取得する相手
//
//------------------------------------------------------------
	.macro	GET_TOKUSEI	side
	.short	AI_GET_TOKUSEI
	.long		\side
	.endm

//------------------------------------------------------------
//
//  みがわり中か？
//
//	@param		side	チェックする相手
//	@param		adrs	分岐先
//
//------------------------------------------------------------
	.macro	IF_MIGAWARI side,adrs
	.short	AI_IF_MIGAWARI
	.long		\side
	.long		(\adrs-.)-4
	.endm

//------------------------------------------------------------
//アドレスをオフセットに変換
//------------------------------------------------------------
	.macro	ADRS		dest,src
	.long		(\dest-\src)-4
	.endm

