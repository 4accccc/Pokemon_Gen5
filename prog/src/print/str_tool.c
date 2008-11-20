//============================================================================================
/**
 * @file	str_tool.c
 * @brief	生文字列処理
 * @author	taya
 * @date	2008.11.17
 */
//============================================================================================

#include "print/printsys.h"
#include "str_tool_local.h"

enum {
	n0_ = 0xff10,
	n1_,
	n2_,
	n3_,
	n4_,
	n5_,
	n6_,
	n7_,
	n8_,
	n9_,
	spc_  = 0x3000,
	bou_  = 0x30fc,
	hate_ = 0xff1f,

	h_n0 = 0x0030,
	h_n1,
	h_n2,
	h_n3,
	h_n4,
	h_n5,
	h_n6,
	h_n7,
	h_n8,
	h_n9,
	h_spc   = 0x0020,
	h_bou   = 0x002d,
	h_hate_ = 0x003f,
};




void STRTOOL_SetNumber( STRBUF* dst, int number, u32 keta, StrNumberDispType dispType, StrNumberCodeType codeType )
{
	static const u32 n_max[] = 
	{
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000,
	};

    static const STRCODE sc_zen[] = {n0_,n1_,n2_,n3_,n4_,n5_,n6_,n7_,n8_,n9_,hate_};
    static const STRCODE sc_han[] = {h_n0_,h_n1_,h_n2_,h_n3_,h_n4_,h_n5_,h_n6_,h_n7_,h_n8_,h_n9_,h_hate_};

	int minus;

	minus = (number < 0);

	if( dst->size > (keta+minus) )
	{
		u32  num_f, num_i, i;
		const STRCODE* sctbl = (codeType==STR_NUM_CODE_ZENKAKU)? sc_zen : sc_han;

		STRBUF_Clear(dst);

		if(minus)
		{
			number *= -1;
			GFL_STR_AddCode( dst, (codeType==STR_NUM_CODE_ZENKAKU)? bou_ : h_bou_ );
		}


		for(i=n_max[keta-1]; i>0; i/=10)
		{
			num_i = (u16)( number / i );
			num_f = (u32)( number - ( i * num_i ) );

			if(num_i >= 10)
			{
				num_i = 10;
			}

			if( dispType == NUMBER_DISPTYPE_ZERO )
			{
				GFL_STR_AddCode( dst, sctbl[num_i] );
			}
			else if( num_i != 0 || i==1 )
			{
				dispType = STR_NUM_DISP_ZERO;
				GFL_STR_AddCode( dst, sctbl[num_i] );
			}
			else if( dispType == STR_NUM_DISP_SPACE )
			{
				GFL_STR_AddCode( dst, (codeType==STR_NUM_CODE_ZENKAKU)? spc_ : h_spc_ );
			}

			number = num_f;
		}

		return;
	}
}

//=============================================================================================
/**
 * 生文字配列間コピー
 *
 * @param[in]	src			コピー元文字列（EOM終端）
 * @param[out]	dst			コピー先文字配列
 * @param		dstElems	コピー先文字配列要素数
 *
 */
//=============================================================================================
void STRTOOL_Copy( const STRCODE* src, STRCODE* dst, u32 dstElems )
{
	u32 i;
	STRCODE EOMCODE = GFL_STR_GetEOMCode();

	for(i=0; i<dstElems; i++)
	{
		dst[i] = src[i];
		if( dst[i] == EOMCODE )
		{
			break;
		}
	}

	dst[ dstElems-1 ] = EOMCODE;
}

//=============================================================================================
/**
 * 生文字列の一致判定
 *
 * @param   src		
 * @param   dst		
 *
 * @retval  BOOL	一致していたらTRUE
 */
//=============================================================================================
BOOL STRTOOL_Comp( const STRCODE* src, const STRCODE* dst )
{
	STRCODE EOMCODE = GFL_STR_GetEOMCode();

	while( *src != EOMCODE )
	{
		if( *src != *dst ){ return FALSE; }
		src++;
		dst++;
	}
	return TRUE;
}


