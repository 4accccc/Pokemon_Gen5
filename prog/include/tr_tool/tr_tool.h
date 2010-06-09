
//============================================================================================
/**
 * @file	tr_tool.h
 * @bfief	トレーナーデータツール郡
 * @author	HisashiSogabe
 * @date	05.12.14
 */
//============================================================================================

#pragma once

//トレーナーメッセージの表示条件定義
#define	TRMSG_FIGHT_START				          ( 0 )		//戦闘開始前
#define	TRMSG_FIGHT_LOSE				          ( 1 )		//戦闘負け
#define	TRMSG_FIGHT_AFTER				          ( 2 )		//戦闘後
#define	TRMSG_FIGHT_START_1				        ( 3 )		//戦闘開始前（2vs2トレーナーのデータ1個目）
#define	TRMSG_FIGHT_LOSE_1				        ( 4 )		//戦闘負け（2vs2トレーナーのデータ1個目）
#define	TRMSG_FIGHT_AFTER_1				        ( 5 )		//戦闘後（2vs2トレーナーのデータ1個目）
#define	TRMSG_POKE_ONE_1				          ( 6 )		//ポケモン1体（2vs2トレーナーのデータ1個目）
#define	TRMSG_FIGHT_START_2				        ( 7 )		//戦闘開始前（2vs2トレーナーのデータ2個目）
#define	TRMSG_FIGHT_LOSE_2				        ( 8 )		//戦闘負け（2vs2トレーナーのデータ2個目）
#define	TRMSG_FIGHT_AFTER_2				        ( 9 )		//戦闘後（2vs2トレーナーのデータ2個目）
#define	TRMSG_POKE_ONE_2				          ( 10 )	//ポケモン1体（2vs2トレーナーのデータ2個目）
#define	TRMSG_FIGHT_NONE_DN				        ( 11 )	//昼夜、戦闘しない時
#define	TRMSG_FIGHT_NONE_D				        ( 12 )	//昼、戦闘しない時
#define TRMSG_FIGHT_AFTER_RECOVER_BEFORE  ( 13 )  //フィールド戦闘後：回復前
#define TRMSG_FIGHT_AFTER_RECOVER_AFTER   ( 14 )  //フィールド戦闘後：回復後
#define TRMSG_FIGHT_AFTER_GIFTITEM_BEFORE ( 15 )  //フィールド戦闘後：アイテムをくれる前
#define TRMSG_FIGHT_AFTER_GIFTITEM_AFTER  ( 16 )  //フィールド戦闘後：アイテムをくれた後
#define	TRMSG_FIGHT_FIRST_DAMAGE		      ( 17 )	//戦闘中：相手ポケモンに最初のダメージを与えたとき
#define	TRMSG_FIGHT_POKE_HP_HALF		      ( 18 )	//戦闘中：相手ポケモンのＨＰ半分以下
#define	TRMSG_FIGHT_POKE_LAST			        ( 19 )	//戦闘中：相手ポケモンが残り１匹
#define	TRMSG_FIGHT_POKE_LAST_HP_HALF	    ( 20 )	//戦闘中：相手ポケモンが残り１匹でＨＰ半分以下
#define	TRMSG_REVENGE_FIGHT_START		      ( 21 )	//フィールド：再戦：対戦前
#define	TRMSG_REVENGE_FIGHT_START_1		    ( 22 )	//フィールド：再戦：対戦前：右
#define	TRMSG_REVENGE_FIGHT_START_2		    ( 23 )	//フィールド：再戦：対戦前：左
#define TRMSG_POKE_UNDER_TWO              ( 24 )  //ポケモン２匹以下（戦闘しない）

#define	TRMSG_FIGHT_WIN					(100)	//戦闘勝ち（戦闘内でのみ使用される）

#define TRMSG_NONE              ( -1 )  //メッセージが見つからなかった

//トレーナーデータを取得するときのID定義
#define	ID_TD_data_type		    ( 0 )		//データタイプ
#define	ID_TD_tr_type		      ( 1 )		//トレーナー分類
#define	ID_TD_fight_type	    ( 2 )		//戦闘タイプ（1vs1or2vs2）
#define	ID_TD_poke_count	    ( 3 )		//所持ポケモン数
#define	ID_TD_use_item1		    ( 4 )		//使用道具1
#define	ID_TD_use_item2		    ( 5 )		//使用道具2
#define	ID_TD_use_item3		    ( 6 )		//使用道具3
#define	ID_TD_use_item4		    ( 7 )		//使用道具4
#define	ID_TD_aibit			      ( 8 )		//AIパターン
#define	ID_TD_hp_recover_flag ( 9 )		//戦闘後回復するか？
#define	ID_TD_gold            ( 10 )	//おこづかい係数
#define	ID_TD_gift_item       ( 11 )  //贈呈するアイテム

//トレーナー持ちポケモンパラメータ
#define DATATYPE_NORMAL   ( 0 )
#define DATATYPE_WAZA     ( 1 )
#define DATATYPE_ITEM     ( 2 )
#define DATATYPE_MULTI    ( 3 )


/*
 * トレーナータイプグループ
 * 拡張する場合、岩澤に確認をお願いします
 */
#define TRTYPE_GRP_RIVAL          (0) //ライバル
#define TRTYPE_GRP_SUPPORT        (1) //サポート
#define TRTYPE_GRP_LEADER         (2) //ジムリーダー
#define TRTYPE_GRP_BIGFOUR        (3) //四天王
#define TRTYPE_GRP_CHAMPION       (4) //チャンピオン
#define TRTYPE_GRP_TRAINER_BOSS   (5) //ポケモントレーナーのN
#define TRTYPE_GRP_PLASMA_BOSS    (6) //プラズマ団のN
#define TRTYPE_GRP_LAST_BOSS      (7) //プラズマ団のN(ラストバトル)
#define TRTYPE_GRP_SAGE           (8) //ゲーチス
#define TRTYPE_GRP_PLASMA         (9) //プラズマ団
#define TRTYPE_GRP_DPCHAMP        (10) //DPチャンピオン(シロナ)
#define TRTYPE_GRP_BCHAMP         (11) //サブウェイマスター
#define TRTYPE_GRP_MAX            (12)
#define TRTYPE_GRP_NONE           (TRTYPE_GRP_MAX)

/*
 * トレーナータイプをグループ化する変換テーブルの参照インデックス定義
 * 拡張する場合、岩澤に確認をお願いします
 */
#define TRTYPE_GRP_IDX_RIVAL			(0)
#define TRTYPE_GRP_IDX_SUPPORT		(1)
#define TRTYPE_GRP_IDX_LEADER1A		(2)
#define TRTYPE_GRP_IDX_LEADER1B		(3)
#define TRTYPE_GRP_IDX_LEADER1C		(4)
#define TRTYPE_GRP_IDX_LEADER2		(5)
#define TRTYPE_GRP_IDX_LEADER3		(6)
#define TRTYPE_GRP_IDX_LEADER4		(7)
#define TRTYPE_GRP_IDX_LEADER5		(8)
#define TRTYPE_GRP_IDX_LEADER6		(9)
#define TRTYPE_GRP_IDX_LEADER7		(10)
#define TRTYPE_GRP_IDX_LEADER8A		(11)
#define TRTYPE_GRP_IDX_LEADER8B		(12)
#define TRTYPE_GRP_IDX_BIGFOUR1		(13)
#define TRTYPE_GRP_IDX_BIGFOUR2		(14)
#define TRTYPE_GRP_IDX_BIGFOUR3		(15)
#define TRTYPE_GRP_IDX_BIGFOUR4		(16)
#define TRTYPE_GRP_IDX_CHAMPION		(17)
#define TRTYPE_GRP_IDX_TRAINER_BOSS (18)
#define TRTYPE_GRP_IDX_PLASMA_BOSS  (19)
#define TRTYPE_GRP_IDX_LAST_BOSS 	  (20)
#define TRTYPE_GRP_IDX_SAGE			  (21)
#define TRTYPE_GRP_IDX_HAKAIM1		(22)
#define TRTYPE_GRP_IDX_HAKAIW1		(23)
#define TRTYPE_GRP_IDX_DPCHAMP		(25)
#define TRTYPE_GRP_IDX_BCHAMP			(26)
#define TRTYPE_GRP_IDX_BCHAMP2		(27)
#define TRTYPE_GRP_IDX_MAX        (28)

//アセンブラでincludeされている場合は、下の宣言を無視できるようにifndefで囲んである
#ifndef	__ASM_NO_DEF_

#include "battle/battle.h"
#include "buflen.h"

typedef u8  TRTYPE_GRP_ID;

typedef enum
{ 
  TD_FIGHT_TYPE_1vs1 = 0,
  TD_FIGHT_TYPE_2vs2,
  TD_FIGHT_TYPE_3vs3,
  TD_FIGHT_TYPE_ROTATION,
}TD_FIGHT_TYPE;

//トレーナーデータパラメータ
typedef struct{
	u8  data_type;            //データタイプ
	u8	tr_type;					    //トレーナー分類
  u8  fight_type;           //1vs1 or 2vs2 or 3vs3 or rotate 
	u8	poke_count;           //所持ポケモン数

	u16	use_item[4];          //使用道具

	u32	aibit;						    //AIパターン

  u8  hp_recover_flag :1;   //戦闘後回復するか？
  u8                  :7;  
  u8  gold;                 //おこづかい係数  

  u16 gift_item;            //戦闘後もらえるアイテム
}TRAINER_DATA;

//トレーナー持ちポケモンパラメータ（データタイプノーマル）
typedef	struct
{
	u8		pow;			  //セットするパワー乱数
  u8    para;       //セットするポケモンのパラメータ（上位4bit：特性　下位4bit：性別）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
  u16   form_no;    //セットするフォルムナンバー
}POKEDATA_TYPE_NORMAL;

//トレーナー持ちポケモンパラメータ（データタイプ技）
typedef	struct
{
	u8		pow;			  //セットするパワー乱数
  u8    para;       //セットするポケモンのパラメータ（上位4bit：特性　下位4bit：性別）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
  u16   form_no;    //セットするフォルムナンバー
	u16		waza[4];		//持ってる技
}POKEDATA_TYPE_WAZA;

//トレーナー持ちポケモンパラメータ（データタイプアイテム）
typedef	struct
{
	u8		pow;			  //セットするパワー乱数
  u8    para;       //セットするポケモンのパラメータ（上位4bit：特性　下位4bit：性別）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
  u16   form_no;    //セットするフォルムナンバー
	u16		itemno;			//セットするアイテム
}POKEDATA_TYPE_ITEM;

//トレーナー持ちポケモンパラメータ（データタイプマルチ）
typedef	struct
{
	u8		pow;			  //セットするパワー乱数
  u8    para;       //セットするポケモンのパラメータ（上位4bit：特性　下位4bit：性別）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
  u16   form_no;    //セットするフォルムナンバー
	u16		itemno;			//セットするアイテム
	u16		waza[ PTL_WAZA_MAX ];		//持ってる技
}POKEDATA_TYPE_MULTI;

extern	u32		TT_TrainerDataParaGet( int tr_id, int id );
extern	BOOL	TT_TrainerMessageCheck( int tr_id, int kindID, HEAPID heapID );
extern	void	TT_TrainerMessageGet( int tr_id, int msgID, STRBUF* msg, HEAPID heapID );
extern	void	TT_TrainerDataGet( int tr_id, TRAINER_DATA* td );
extern	void	TT_TrainerPokeDataGet( int tr_id,void* tpd );
extern	u8		TT_TrainerTypeSexGet( int trtype );
extern  u8 TT_TrainerTypeGrpEntryIdxGet( int trtype );
extern  TRTYPE_GRP_ID TT_TrainerTypeGrpGet( int trtype );
extern  BtlBgAttr TT_TrainerTypeBtlBgAttrGet( int trtype );

extern void	TT_EncountTrainerPersonalDataMake( TrainerID tr_id, BSP_TRAINER_DATA* data, HEAPID heapID );
extern void	TT_EncountTrainerPokeDataMake( TrainerID tr_id, POKEPARTY* pparty, HEAPID heapID );

#endif	__ASM_NO_DEF_

