//======================================================================
/**
 * @file	name_input.h
 * @brief	名前入力
 * @author	ariizumi
 * @data	09/01/08
 */
//======================================================================

#ifndef NAME_INPUT_H__
#define NAME_INPUT_H__

#include "savedata\config.h"
#include "savedata\box_savedata.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
// 名前入力モード(とりあえずPtから移植
enum {
	NAMEIN_MYNAME = 0,	// 自分の名前
	NAMEIN_POKEMON,		// ポケモンの名前
	NAMEIN_BOX,			// ボックスの名前
	NAMEIN_RIVALNAME,	// ライバルネーム
	NAMEIN_FRIENDCODE,	// ともだちコード
	NAMEIN_RANDOMGROUP, // 乱数の種グループの名前
	NAMEIN_STONE,		// 石碑(配布）
	NAMEIN_FRIENDNAME,  // WIFIともだち手帳に書き込むともだちの名前
	//増やしたり順番を変えた場合は.cの方の文字列長テーブルも編集してください！
	NAMEIN_MAX,
};

// ポケモン名前入力の長さ
#if (PM_LANG==LANG_JAPAN)
#define NAMEIN_POKEMON_LENGTH	(  5 )
#else
#define NAMEIN_POKEMON_LENGTH	( 10 )
#endif

// トレーナー名入力の長さ
#if (PM_LANG==LANG_JAPAN)
#define NAMEIN_PERSON_LENGTH	(  5 )
#else
#define NAMEIN_PERSON_LENGTH	( 7 )
#endif


// グループ名入力の長さNAMEIN_PERSON_LENGTH
#if (PM_LANG==LANG_JAPAN)
#define NAMEIN_GROUP_LENGTH		(  5 )
#else
#define NAMEIN_GROUP_LENGTH		( 7 )
#endif


//======================================================================
//	typedef struct
//======================================================================
//-------------------------------------
///	
//=====================================
typedef struct {
	int mode;		// 入力モード(enum参照）
	int info;		// 入力モードとして男主人公・女主人公、ポケモンの開発NOなど
	int form;		// 入力モードがポケモンの時に形状値を受け取る
	int wordmax;	// 入力文字最大数
	int sex;		// ポケモンの性別(PARA_MALE=0...PARA_FEMALE=1...PARA_UNK=2(性別なし))


	int cancel;		// 名前入力が終了した時に反映されるフラグ。
					// 入力文字が０文字だった。もしくは最初と同じだった場合はこのフラグが立つ。

	STRBUF *strbuf; // このSTRBUFには２つの意味が存在する。
					// １つは名前入力画面からデータを受け取るワークであること。名前入力が終わったらここからコピーする
					// もう１つは、名前入力に行くときにここに文字列を格納しておくと,
					// 名前入力画面はその文字列を表示しながら開始する。引数としても機能する

	u16 str[20];	// この配列にも結果は返ってきます。（20に意味はありません）


	// --------ポケモン捕獲でボックス転送になった時に必要-------------
	int get_msg_id; 	// ポケモン捕獲の時にBOX転送が発生した時のメッセージIDが入る
	BOX_MANAGER  *boxdata;	// ポケモン捕獲の時にBOX転送が発生した時に、BOXの名前を取得するために必要
	CONFIG    *config;  // コンフィグ構造体
} NAMEIN_PARAM;

//======================================================================
//	proto
//======================================================================
extern const GFL_PROC_DATA NameInputProcData;

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
//GSからKEYTOUCH_STATUSを削ってます
NAMEIN_PARAM *NameIn_ParamAllocMake(int heapId, int mode, int info, int wordmax, CONFIG *config );
void NameIn_ParamDelete(NAMEIN_PARAM *param);


#endif	//NAME_INPUT_H__
