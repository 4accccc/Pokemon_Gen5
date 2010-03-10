//============================================================================================
/**
	* @file	pmsi_initial_data.c
	* @bfief	簡易会話入力画面（カテゴリイニシャルモード用のデータテーブル処理）
	* @author	taya
	* @date	06.02.14
	*/
//============================================================================================
#include <gflib.h>

#include "system\pms_word.h"
#include "msg\msg_pms_category.h"
#include "message.naix"


#include "pms_input_prv.h"
#include "pmsi_initial_data.h"

//======================================================================
// イニシャルモード時の選択文字テーブル（言語ごとに設定）
//======================================================================

//-------------------------------------------------------------
/**
	*  各文字の描画幅
	*/
//-------------------------------------------------------------
enum {
	INITIAL_WRITE_X_MARGIN = 8*3,//13,		// １文字ごとの描画幅
	INITIAL_WRITE_Y_MARGIN = 16+8//16,		// １文字ごとの描画高さ
};

#define INITIAL_XPOS(n)	( (n)*INITIAL_WRITE_X_MARGIN + ( ((n)>=6) ? -16 : 0 ) ) ///< 6以上は左に寄せることでマージンを表現
#define INITIAL_YPOS(n)	((n)*INITIAL_WRITE_Y_MARGIN)

#if 0
//-------------------------------------------------------------
/**
	*  各文字のインデックス
	*/
//-------------------------------------------------------------
enum {
	INI_A = 0,
	INI_I,
	INI_U,
	INI_E,
	INI_O,
	INI_KA,
	INI_KI,
	INI_KU,
	INI_KE,
	INI_KO,
	INI_SA,
	INI_SI,
	INI_SU,
	INI_SE,
	INI_SO,
	INI_TA,
	INI_TI,
	INI_TU,
	INI_TE,
	INI_TO,
	INI_NA,
	INI_NI,
	INI_NU,
	INI_NE,
	INI_NO,
	INI_HA,
	INI_HI,
	INI_HU,
	INI_HE,
	INI_HO,
	INI_MA,
	INI_MI,
	INI_MU,
	INI_ME,
	INI_MO,
#if 1
	INI_YA,
	INI_YU,
	INI_YO,
	INI_RA,
	INI_RI,
	INI_RU,
	INI_RE,
	INI_RO,
#else
	INI_RA,
	INI_RI,
	INI_RU,
	INI_RE,
	INI_RO,
	INI_YA,
	INI_YU,
	INI_YO,
#endif
	INI_WA,
	INI_OTHER,

	INI_BACK = CATEGORY_POS_BACK,
	INI_DIS  = CATEGORY_POS_DISABLE,
};
#endif

static const struct {
	STRCODE		code;
	u8			xpos;
	u8			ypos;
	u8			up;
	u8			down;
	u8			left;
	u8			right;
	u8			col_top;
	u8			col_bottom;
}InitialParamTbl[] = {

	{ L'あ',  INITIAL_XPOS(0),  INITIAL_YPOS(0), INI_SELECT,  INI_KA,    INI_HO,    INI_I,     INI_A,   INI_NA, },
	{ L'い',  INITIAL_XPOS(1),  INITIAL_YPOS(0), INI_SELECT,  INI_KI,    INI_A,     INI_U,     INI_I,   INI_NI, },
	{ L'う',  INITIAL_XPOS(2),  INITIAL_YPOS(0), INI_SELECT,  INI_KU,    INI_I,     INI_E,     INI_U,   INI_NU, },
	{ L'え',  INITIAL_XPOS(3),  INITIAL_YPOS(0), INI_SELECT,  INI_KE,    INI_U,     INI_O,     INI_E,   INI_NE, },
	{ L'お',  INITIAL_XPOS(4),  INITIAL_YPOS(0), INI_SELECT,  INI_KO,    INI_E,     INI_HA,    INI_O,   INI_NO, },

	{ L'か',  INITIAL_XPOS(0),  INITIAL_YPOS(1), INI_A,     INI_SA,    INI_MO,    INI_KI,    INI_A,   INI_NA, },
	{ L'き',  INITIAL_XPOS(1),  INITIAL_YPOS(1), INI_I,     INI_SI,    INI_KA,    INI_KU,    INI_I,   INI_NI, },
	{ L'く',  INITIAL_XPOS(2),  INITIAL_YPOS(1), INI_U,     INI_SU,    INI_KI,    INI_KE,    INI_U,   INI_NU, },
	{ L'け',  INITIAL_XPOS(3),  INITIAL_YPOS(1), INI_E,     INI_SE,    INI_KU,    INI_KO,    INI_E,   INI_NE, },
	{ L'こ',  INITIAL_XPOS(4),  INITIAL_YPOS(1), INI_O,     INI_SO,    INI_KE,    INI_MA,    INI_O,   INI_NO, },

	{ L'さ',  INITIAL_XPOS(0),  INITIAL_YPOS(2), INI_KA,    INI_TA,    INI_YO,    INI_SI,    INI_A,   INI_NA, },
	{ L'し',  INITIAL_XPOS(1),  INITIAL_YPOS(2), INI_KI,    INI_TI,    INI_SA,    INI_SU,    INI_I,   INI_NI, },
	{ L'す',  INITIAL_XPOS(2),  INITIAL_YPOS(2), INI_KU,    INI_TU,    INI_SI,    INI_SE,    INI_U,   INI_NU, },
	{ L'せ',  INITIAL_XPOS(3),  INITIAL_YPOS(2), INI_KE,    INI_TE,    INI_SU,    INI_SO,    INI_E,   INI_NE, },
	{ L'そ',  INITIAL_XPOS(4),  INITIAL_YPOS(2), INI_KO,    INI_TO,    INI_SE,    INI_YA,    INI_O,   INI_NO, },

	{ L'た',  INITIAL_XPOS(0),  INITIAL_YPOS(3), INI_SA,    INI_NA,    INI_RO,    INI_TI,    INI_A,   INI_NA, },
	{ L'ち',  INITIAL_XPOS(1),  INITIAL_YPOS(3), INI_SI,    INI_NI,    INI_TA,    INI_TU,    INI_I,   INI_NI, },
	{ L'つ',  INITIAL_XPOS(2),  INITIAL_YPOS(3), INI_SU,    INI_NU,    INI_TI,    INI_TE,    INI_U,   INI_NU, },
	{ L'て',  INITIAL_XPOS(3),  INITIAL_YPOS(3), INI_SE,    INI_NE,    INI_TU,    INI_TO,    INI_E,   INI_NE, },
	{ L'と',  INITIAL_XPOS(4),  INITIAL_YPOS(3), INI_SO,    INI_NO,    INI_TE,    INI_RA,    INI_O,   INI_NO, },

	{ L'な',  INITIAL_XPOS(0),  INITIAL_YPOS(4), INI_TA,    INI_SELECT,  INI_OTHER, INI_NI,    INI_A,  INI_NA, },
	{ L'に',  INITIAL_XPOS(1),  INITIAL_YPOS(4), INI_TI,    INI_SELECT,  INI_NA,    INI_NU,    INI_I,  INI_NI, },
	{ L'ぬ',  INITIAL_XPOS(2),  INITIAL_YPOS(4), INI_TU,    INI_SELECT,  INI_NI,    INI_NE,    INI_U,  INI_NU, },
	{ L'ね',  INITIAL_XPOS(3),  INITIAL_YPOS(4), INI_TE,    INI_SELECT,  INI_NU,    INI_NO,    INI_E,  INI_NE, },
	{ L'の',  INITIAL_XPOS(4),  INITIAL_YPOS(4), INI_TO,    INI_SELECT,  INI_NE,    INI_WA,    INI_O,  INI_NO, },

  // X改行

	{ L'は',  INITIAL_XPOS(6),  INITIAL_YPOS(0), INI_SELECT,  INI_MA,    INI_O,     INI_HI,   INI_HA,  INI_WA, },
	{ L'ひ',  INITIAL_XPOS(7),  INITIAL_YPOS(0), INI_SELECT,  INI_MI,    INI_HA,    INI_HU,   INI_HI,  INI_WO, },
	{ L'ふ',  INITIAL_XPOS(8),  INITIAL_YPOS(0), INI_SELECT,  INI_MU,    INI_HI,    INI_HE,   INI_HU,  INI_NN, },
	{ L'へ',  INITIAL_XPOS(9),  INITIAL_YPOS(0), INI_SELECT,  INI_ME,    INI_HU,    INI_HO,   INI_HE,  INI_BOU, },
	{ L'ほ',  INITIAL_XPOS(10), INITIAL_YPOS(0), INI_SELECT,  INI_MO,    INI_HE,    INI_A,    INI_HO,  INI_OTHER, },

/*
	{ L'ま',  INITIAL_XPOS(6),  INITIAL_YPOS(1), INI_HA,    INI_YA,    INI_KO,    INI_MI,   INI_HA,  INI_WA, },
	{ L'み',  INITIAL_XPOS(7),  INITIAL_YPOS(1), INI_HI,    INI_YU,    INI_MA,    INI_MU,   INI_HI,  INI_WO, },
	{ L'む',  INITIAL_XPOS(8),  INITIAL_YPOS(1), INI_HU,    INI_YO,    INI_MI,    INI_ME,   INI_HU,  INI_NN, },
	{ L'め',  INITIAL_XPOS(9),  INITIAL_YPOS(1), INI_HE,    INI_RE,    INI_MU,    INI_MO,   INI_HE,  INI_BOU, },
	{ L'も',  INITIAL_XPOS(10), INITIAL_YPOS(1), INI_HO,    INI_RO,    INI_ME,    INI_KA,   INI_HO,  INI_OTHER, },
*/

	{ L'ま',  INITIAL_XPOS(6),  INITIAL_YPOS(1), INI_HA,    INI_YA,    INI_KO,    INI_MI,   INI_HA,  INI_WA, },
	{ L'み',  INITIAL_XPOS(7),  INITIAL_YPOS(1), INI_HI,    INI_RI,    INI_MA,    INI_MU,   INI_HI,  INI_WO, },
	{ L'む',  INITIAL_XPOS(8),  INITIAL_YPOS(1), INI_HU,    INI_YU,    INI_MI,    INI_ME,   INI_HU,  INI_NN, },
	{ L'め',  INITIAL_XPOS(9),  INITIAL_YPOS(1), INI_HE,    INI_RE,    INI_MU,    INI_MO,   INI_HE,  INI_BOU, },
	{ L'も',  INITIAL_XPOS(10), INITIAL_YPOS(1), INI_HO,    INI_YO,    INI_ME,    INI_KA,   INI_HO,  INI_OTHER, },

/*
	{ L'や',  INITIAL_XPOS(6), INITIAL_YPOS(2), INI_MA,     INI_RA,    INI_SO,    INI_YU,   INI_HA,  INI_WA, },
	{ L'ゆ',  INITIAL_XPOS(7), INITIAL_YPOS(2), INI_MI,     INI_RI,    INI_YA,    INI_YO,   INI_HI,  INI_WO, },
	{ L'よ',  INITIAL_XPOS(8), INITIAL_YPOS(2), INI_MU,     INI_RU,    INI_YU,    INI_SA,   INI_HU,  INI_NN, },
*/

	{ L'や',  INITIAL_XPOS(6),  INITIAL_YPOS(2), INI_MA,    INI_RA,    INI_SO,    INI_YU,   INI_HA,  INI_WA, },
	{ L'ゆ',  INITIAL_XPOS(8),  INITIAL_YPOS(2), INI_MU,    INI_RU,    INI_YA,    INI_YO,   INI_HU,  INI_NN, },
	{ L'よ',  INITIAL_XPOS(10), INITIAL_YPOS(2), INI_MO,    INI_RO,    INI_YU,    INI_SA,   INI_HO,  INI_OTHER, },

/*
	{ L'ら',  INITIAL_XPOS(6),  INITIAL_YPOS(3), INI_YA,    INI_WA,    INI_TO,    INI_RI,    INI_HA,  INI_WA, },
	{ L'り',  INITIAL_XPOS(7),  INITIAL_YPOS(3), INI_YU,    INI_WO,     INI_RA,    INI_RU,    INI_HI,  INI_WO, },
	{ L'る',  INITIAL_XPOS(8),  INITIAL_YPOS(3), INI_YO,    INI_NN,     INI_RI,    INI_RE,    INI_HU,  INI_NN, },
	{ L'れ',  INITIAL_XPOS(9),  INITIAL_YPOS(3), INI_ME,    INI_BOU,    INI_RU,    INI_RO,    INI_HE,  INI_BOU, },
	{ L'ろ',  INITIAL_XPOS(10), INITIAL_YPOS(3), INI_MO,    INI_OTHER,  INI_RE,    INI_TA,    INI_HO,  INI_OTHER, },
*/

	{ L'ら',  INITIAL_XPOS(6),  INITIAL_YPOS(3), INI_YA,    INI_WA,    INI_TO,    INI_RI,    INI_HA,  INI_WA, },
	{ L'り',  INITIAL_XPOS(7),  INITIAL_YPOS(3), INI_MI,    INI_WO,     INI_RA,    INI_RU,    INI_HI,  INI_WO, },
	{ L'る',  INITIAL_XPOS(8),  INITIAL_YPOS(3), INI_YU,    INI_NN,     INI_RI,    INI_RE,    INI_HU,  INI_NN, },
	{ L'れ',  INITIAL_XPOS(9),  INITIAL_YPOS(3), INI_ME,    INI_BOU,    INI_RU,    INI_RO,    INI_HE,  INI_BOU, },
	{ L'ろ',  INITIAL_XPOS(10), INITIAL_YPOS(3), INI_YO,    INI_OTHER,  INI_RE,    INI_TA,    INI_HO,  INI_OTHER, },

  { L'わ',  INITIAL_XPOS(6), INITIAL_YPOS(4), INI_RA,     INI_SELECT,  INI_NO,    INI_WO,    INI_HA,  INI_WA, },
	{ L'！',  INITIAL_XPOS(10), INITIAL_YPOS(4), INI_RO,    INI_SELECT,  INI_BOU,   INI_NA,    INI_HO,  INI_OTHER },
  
  // ↑50音検索のgmmテーブルと対応しているので順番固定↑
  
  // WB追加 
	{ L'を',  INITIAL_XPOS(7), INITIAL_YPOS(4), INI_RI,     INI_SELECT,  INI_WA,    INI_NN,     INI_HI,  INI_WO },
	{ L'ん',  INITIAL_XPOS(8), INITIAL_YPOS(4), INI_RU,     INI_SELECT,  INI_WO,    INI_BOU,    INI_HU,  INI_NN },
	{ L'ー',  INITIAL_XPOS(9), INITIAL_YPOS(4), INI_RE,     INI_SELECT,  INI_NN,  INI_OTHER,  INI_HE,  INI_BOU },
};



//------------------------------------------------------------------
/**
	* イニシャルナンバー最大を返す
	*
	* @retval  u32		
	*/
//------------------------------------------------------------------
u32 PMSI_INITIAL_DAT_GetInitialMax( void )
{
	return NELEMS(InitialParamTbl);
}

//------------------------------------------------------------------
/**
	* １文字の文字コードをバッファにコピー
	*
	* @param   initial		イニシャルナンバー
	* @param   buf			コピー先バッファ
	*
	*/
//------------------------------------------------------------------
void PMSI_INITIAL_DAT_GetStr( u32 initial, STRBUF* buf )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));

	GFL_STR_SetStringCodeOrderLength( buf, &(InitialParamTbl[initial].code), 2);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  指定イニシャルナンバーのSTRCODEを返す
 *
 *	@param	u32 initial イニシャルナンバー
 *
 *	@retval STRCODE 
 */
//-----------------------------------------------------------------------------
STRCODE PMSI_INITIAL_DAT_GetStrCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));

  return InitialParamTbl[initial].code;
}

//------------------------------------------------------------------
/**
	* 描画用基準位置（ドット単位）を取得
	*
	* @param   initial		イニシャルナンバー
	* @param   x			描画位置Ｘ
	* @param   y			描画位置Ｙ
	*
	*/
//------------------------------------------------------------------
void PMSI_INITIAL_DAT_GetPrintPos( u32 initial, u32* x, u32* y )
{
	GF_ASSERT_MSG((initial < NELEMS(InitialParamTbl)), "pos=%d", initial);

	*x = InitialParamTbl[initial].xpos;
	*y = InitialParamTbl[initial].ypos;
}

//------------------------------------------------------------------
/**
	* 上キー押下時の移動先イニシャルナンバーを返す
	*
	* @param   initial		イニシャルナンバー
	*
	* @retval  int		移動先イニシャルナンバー
	*/
//------------------------------------------------------------------
int PMSI_INITIAL_DAT_GetUpCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));
	return InitialParamTbl[initial].up;
}

//------------------------------------------------------------------
/**
	* 下キー押下時の移動先イニシャルナンバーを返す
	*
	* @param   initial		イニシャルナンバー
	*
	* @retval  int		移動先イニシャルナンバー
	*/
//------------------------------------------------------------------
int PMSI_INITIAL_DAT_GetDownCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));
	return InitialParamTbl[initial].down;
}

//------------------------------------------------------------------
/**
	* 左キー押下時の移動先イニシャルナンバーを返す
	*
	* @param   initial		イニシャルナンバー
	*
	* @retval  int		移動先イニシャルナンバー
	*/
//------------------------------------------------------------------
int PMSI_INITIAL_DAT_GetLeftCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));
	return InitialParamTbl[initial].left;
}

//------------------------------------------------------------------
/**
	* 右キー押下時の移動先イニシャルナンバーを返す
	*
	* @param   initial		イニシャルナンバー
	*
	* @retval  int		移動先イニシャルナンバー
	*/
//------------------------------------------------------------------
int PMSI_INITIAL_DAT_GetRightCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));
	return InitialParamTbl[initial].right;
}

//------------------------------------------------------------------
/**
	* 指定キーのある列の最上段ナンバーを返す
	*
	* @param   initial		イニシャルナンバー
	*
	* @retval  int		移動先イニシャルナンバー
	*/
//------------------------------------------------------------------
int PMSI_INITIAL_DAT_GetColTopCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));
	return InitialParamTbl[initial].col_top;
}

//------------------------------------------------------------------
/**
	* 指定キーのある列の最下段ナンバーを返す
	*
	* @param   initial		イニシャルナンバー
	*
	* @retval  int		移動先イニシャルナンバー
	*/
//------------------------------------------------------------------
int PMSI_INITIAL_DAT_GetColBottomCode( u32 initial )
{
	GF_ASSERT(initial < NELEMS(InitialParamTbl));
	return InitialParamTbl[initial].col_bottom;
}


