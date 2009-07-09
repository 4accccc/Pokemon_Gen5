//=============================================================================
/**
 * @file	union_beacon_tool.c
 * @brief	ユニオンルームビーコン処理
 * @author	Akito Mori
 * @date		2006.03.16
 */
//=============================================================================

#include <gflib.h>

#include "arc_def.h"
#include "record_gra.naix"
#include "test_graphic/trbgra.naix"
#include "test_graphic/trfgra.naix"

#include "print/wordset.h"
#include "field/fldmmdl_pl_code.h"

#include "net_app/union/union_beacon_tool.h"
#include "field/fldmmdl.h"

#define UNION_VIEW_TYPE_NUM	( 16 )

//GSの方も仮だったので仮で移植
#define DUMMY_MSG_TYPE (0)
#define DUMMY_GRA_TYPE (0)

// ユニオンルームで表示するトレーナーのタイプ見た目＆名前＆トレーナーグラフィックテーブル
static const struct{
  u16 objcode;
  u8 msg_type;
  u8 tr_type;
}UnionViewTable[] = {
#if 0	//トレーナーデータ変更に伴いコメントアウト　saito	
	{ BOY1,			MSG_TRTYPE_SCHOOLB, 	TRTYPE_SCHOOLB	  },	///< じゅくがえり
	{ BOY3,			MSG_TRTYPE_MUSHI,		TRTYPE_MUSHI	  },	///< むしとりしょうねん
	{ MAN3,			MSG_TRTYPE_ELITEM,		TRTYPE_ELITEM	  },	///< エリートトレーナー♂
	{ BADMAN,		MSG_TRTYPE_HEADS,		TRTYPE_HEADS	  },	///< スキンヘッズ
	{ EXPLORE,		MSG_TRTYPE_ISEKI,		TRTYPE_ISEKI	  },	///< いせきマニア
	{ FIGHTER,		MSG_TRTYPE_KARATE,		TRTYPE_KARATE	  },	///< からておう
	{ GORGGEOUSM,	MSG_TRTYPE_PRINCE,		TRTYPE_PRINCE	  },	///< おぼっちゃま
	{ MYSTERY,		MSG_TRTYPE_ESPM,		TRTYPE_ESPM 	  },	///< サイキッカー

	{ GIRL1,		MSG_TRTYPE_MINI,  		TRTYPE_MINI 	  },	///< ミニスカート
	{ GIRL2,		MSG_TRTYPE_BATTLEG, 	TRTYPE_BATTLEG	  },	///< バトルガール
	{ WOMAN2,		MSG_TRTYPE_SISTER, 		TRTYPE_SISTER	  },	///< おとなのおねえさん
	{ WOMAN3,		MSG_TRTYPE_ELITEW,		TRTYPE_ELITEW 	  },	///< エリートトレーナー♀
	{ IDOL,			MSG_TRTYPE_IDOL,		TRTYPE_IDOL 	  },	///< アイドル
	{ LADY,			MSG_TRTYPE_MADAM, 		TRTYPE_MADAM	  },	///< マダム
	{ COWGIRL,		MSG_TRTYPE_COWGIRL, 	TRTYPE_COWGIRL	  },	///< カウガール
	{ GORGGEOUSW,	MSG_TRTYPE_PRINCESS,	TRTYPE_PRINCESS   },	///< おじょうさま
#else
	{ BOY1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< じゅくがえり
	{ MAN1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< むしとりしょうねん
	{ MIDDLEMAN1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< エリートトレーナー♂
	{ OLDMAN1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< スキンヘッズ
	{ BABYBOY1,	DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< いせきマニア
	{ BOY1,	DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< からておう
	{ MAN1,	DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< おぼっちゃま
	{ MIDDLEMAN1,	DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< サイキッカー

	{ GIRL1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< ミニスカート
	{ WOMAN1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< バトルガール
	{ MIDDLEWOMAN1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< おとなのおねえさん
	{ OLDWOMAN1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< エリートトレーナー♀
	{ BABYGIRL1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< アイドル
	{ GIRL1,		DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< マダム
	{ WOMAN1,	DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< カウガール
	{ MIDDLEWOMAN1,	DUMMY_MSG_TYPE,		DUMMY_GRA_TYPE	},	///< おじょうさま
#endif
};

// 見た目変更おやじ用のテーブル（IDを８で割ったあまりに対応する）
static const int UnionViewNameTable[][4]={
	{0,1,2,3,},
	{1,6,7,0,},

	{2,3,4,5,},
	{3,0,5,6,},

	{4,1,2,7,},
	{5,2,7,0,},

	{6,3,4,1,},
	{7,4,5,6,},
};

#define UNION_VIEW_SELECT_NUM	( 4 )
#define UNION_VIEW_SELECT_TYPE	( 8 )

//==============================================================================
/**
 * $brief	IDと性別から選択できる４つのトレーナータイプの文字列を格納する
 *
 * @param	id			ユーザーID
 * @param	sex			性別
 * @param	wordset		文字列格納バッファ
 *
 * @retval	none		
 */
//==============================================================================
void UnionView_SetUpTrainerTypeSelect( u32 id, int sex, WORDSET *wordset )
{
	int key    = id%8;
	int i;

	// IDを８で割った余りで、対応する4つの選択肢を性別ごとに用意する

	for(i=0; i<UNION_VIEW_SELECT_NUM; i++){
		int patern = UnionViewNameTable[key][i] + UNION_VIEW_SELECT_TYPE * sex;
//		WORDSET_RegisterTrTypeName( wordset, i, UnionViewTable[patern][1] );
	}


}


//==============================================================================
/**
 * $brief	選択結果から決定したトレーナータイプのテーブルIndexを取得する
 *
 * @param	id			ユーザーID
 * @param	sex			性別
 * @param	select		選択結果（0-3)
 *
 * @retval	int		
 */
//==============================================================================
int UnionView_GetTrainerTypeIndex( u32 id, int sex, u32 select )
{
	int key    = id%8;
	int patern = UnionViewNameTable[key][select] + UNION_VIEW_SELECT_TYPE * sex;

  return patern;
}


//==============================================================================
/**
 * $brief	選択結果から決定したトレーナータイプの番号そのものを算出する
 *
 * @param	id			ユーザーID
 * @param	sex			性別
 * @param	select		選択結果（0-3)
 *
 * @retval	int		
 */
//==============================================================================
int UnionView_GetTrainerType( u32 id, int sex, u32 select )
{
  int patern = UnionView_GetTrainerTypeIndex(id, sex, select);
	return UnionViewTable[patern].objcode;
}

//------------------------------------------------------------------
/**
 * $brief	ユニオンルーム用の見た目番号テーブルを検索して添え字を返す
 *
 * @param	sex			性別
 * @param	view_type	見た目番号
 *
 * @retval	int			添え字番号
 */
//------------------------------------------------------------------
static int Search_ViewNo( int sex, int view_type )
{
	int i;
	for(i=0;i<UNION_VIEW_TYPE_NUM/2;i++){
		if(UnionViewTable[i+(sex*(UNION_VIEW_TYPE_NUM/2))].objcode==view_type){
			return i + (sex*(UNION_VIEW_TYPE_NUM/2));
		}	
	}
	return 0;
}
//==============================================================================
/**
 * $brief	見た目番号と性別から2Dキャラの位置を取得する
 *
 * @param	sex		
 * @param	view_type		
 *
 * @retval	int		
 */
//==============================================================================
int UnionView_GetCharaNo( int sex, int view_type )
{

	return Search_ViewNo( sex, view_type );
}

//==================================================================
/**
 * トレーナータイプのテーブルINDEXからOBJCODEを取得する
 * @param   view_index		テーブルINDEX
 * @retval  int		OBJCODE
 */
//==================================================================
int UnionView_GetObjCode(int view_index)
{
  if(view_index >= NELEMS(UnionViewTable)){
    GF_ASSERT(0);
    return BOY1;
  }
  return UnionViewTable[view_index].objcode;
}


//==============================================================================
/**
 * $brief	トレーナーVIEWTYPEと性別から情報を引き出す
 *
 * @param	sex			性別
 * @param	view_type	MyStatus_GetTrainerViewで取得できるコード
 * @param	info		欲しいデータのタイプ
 *						UNIONVIEW_ICONINDEX:	0詰めで何番目か
 *						UNIONVIEW_MSGTYPE:		メッセージデータ番号（ex 「じゅくがえり」のMSGNO）
 *						UNIONVIEW_TRTYPE:		トレーナーグラフィック番号
 *
 * @retval	int			0から詰めた時に何番目のキャラか
 */
//==============================================================================
int UnionView_GetTrainerInfo( int sex, int view_type, int info )
{
	int i = Search_ViewNo( sex, view_type );

	switch(info){
  case UNIONVIEW_ICONINDEX:
		return i;
  case UNIONVIEW_MSGTYPE:
		return UnionViewTable[i].msg_type;
  case UNIONVIEW_TRTYPE:
		return UnionViewTable[i].tr_type;
  default:
		GF_ASSERT(0);
	}

	// ここまで抜けるのはおかしい
	return 0;
}


//==============================================================================
/**
 * $brief	指定のOBJに必要なパレットのポインタを返す
 *
 * @param	sex		
 * @param	view_type		
 *
 * @retval	u16 *		
 */
//==============================================================================
u16 *UnionView_GetPalNo( u16 * table, int sex, int view_type )
{
	int ret;
	ret = Search_ViewNo( sex, view_type );
	
	return &table[16*2*ret];
}

//==============================================================================
/**
 * $brief	18体分のパレットのテーブルをAlloc＆ﾛｰﾄﾞして返す
 *
 * @param	heapID		ヒープID
 *
 * @retval	u16 *		OBJ用のパレットテーブル（解放は各自で）
 */
//==============================================================================
u16 *UnionView_PalleteTableAlloc( int heapID )
{
	u8 *data1, *data2;
	NNSG2dPaletteData *pal1,*pal2;
	u16 *palette_table,*src;
	int i;
	
	data1 = GFL_ARC_UTIL_LoadPalette( ARCID_RECORD_GRA, NARC_record_gra_union_chara_NCLR, &pal1, heapID );

	palette_table = GFL_HEAP_AllocMemory( heapID, 16*18*2 );
	OS_Printf("allocwork = %08x srcpallette = %08x\n",palette_table, pal1->pRawData);

	src = (u16*)pal1->pRawData;
	for(i=0;i<16*16;i++){
		palette_table[i] = src[i];
	}
//	MI_CpuCopyFast(pal1->pRawData, &palette_table[	  0], 16*16*2);
//	MI_CpuCopyFast(pal2->pRawData, &palette_table[16*16], 2*16*2);
	
	GFL_HEAP_FreeMemory(data1);
	
	return palette_table;
}

//以下poke_toolから移植

//============================================================================================
/**
 *	トレーナーキャラのCLACTリソースのIDを取得
 *
 * @param[in]	trtype	トレーナータイプ
 * @param[in]	dir		PARA_FRONT:正面 PARA_BACK:背面
 * @param[out]	tcb		TR_CLACT_GRA型ワークへのポインタ	
 * @param[in]	non_throw	CLACTリソースIDへのオフセット(EXのみ)
 *
 * @retval	CLACTへのポインタ
 */
//============================================================================================
void	TrCLACTGraDataGetEx(int trtype,int dir,BOOL non_throw,TR_CLACT_GRA *tcg)
{
	if(dir==TR_PARA_FRONT){
		tcg->arcID=ARCID_TRF_GRA;
		tcg->ncgrID=NARC_trfgra_trgs_hero_NCGR+trtype*5;
		tcg->nclrID=NARC_trfgra_trgs_hero_NCLR+trtype*5;
		tcg->ncerID=NARC_trfgra_trgs_hero_NCER+trtype*5;
		tcg->nanrID=NARC_trfgra_trgs_hero_NANR+trtype*5;
		tcg->ncbrID=NARC_trfgra_trgs_hero_NCBR+trtype*5;
	}
	else{
		GF_ASSERT_MSG(0,"Plz fix me!!\n");
/*	//厳密にトレーナー種類が必要なので保留
		tcg->arcID=ARCID_TRB_GRA;
		trtype=TrainerBTrTypeGet(trtype,non_throw);
		tcg->ncgrID=NARC_trbgra_gtr1b_002_NCGR+trtype*5;
		tcg->nclrID=NARC_trbgra_gtr1b_002_NCLR+trtype*5;
		tcg->ncerID=NARC_trbgra_gtr1b_002_NCER+trtype*5;
		tcg->nanrID=NARC_trbgra_gtr1b_002_NANR+trtype*5;
		tcg->ncbrID=NARC_trbgra_gtr1b_002_NCBR+trtype*5;
*/
	}
}
void	TrCLACTGraDataGet(int trtype,int dir,TR_CLACT_GRA *tcg)
{
	TrCLACTGraDataGetEx(trtype,dir,FALSE,tcg);
}