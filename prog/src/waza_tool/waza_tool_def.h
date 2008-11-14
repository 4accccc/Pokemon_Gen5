
//============================================================================================
/**
 * @file	waza_tool_def.h
 * @bfief	技パラメータツール群（WB改訂版）
 * @author	HisashiSogabe
 * @date	08.11.13
 */
//============================================================================================

//==============================================================================
//	構造体定義
//==============================================================================
///コンテスト技データ
typedef struct{
	u8 ap_no;		///<アピールNo
	u8 contype;		///<コンテスト技タイプ(CONTYPE_STYLE等)
	
	u16 dummy;
}CONWAZA_DATA;

struct waza_table_data
{
	u16				battleeffect;
	u8				kind;
	u8				damage;

	u8				wazatype;
	u8				hitprobability;
	u8				pp;
	u8				addeffect;

	u16				attackrange;
	s8				attackpri;
	u8				flag;

	CONWAZA_DATA	condata;
};

