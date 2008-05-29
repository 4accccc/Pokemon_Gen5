/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     base64codec.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include "base64codec.h"
#include "dpw_assert.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					関数プロトタイプ宣言
 *-----------------------------------------------------------------------*/

//--- Auto Function Prototype --- Don't comment here.
static unsigned char base64(unsigned char a);
//--- End of Auto Function Prototype

/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					グローバル関数定義
 *-----------------------------------------------------------------------*/

/*!
 *	@brief	BASE64エンコード
 *  
 *  NULL終端は行われませんのでご注意ください。
 *  
 *	@param[in]	in		入力データ
 *	@param[out]	out		出力先。base64size(in_size)以上のサイズを用意してください。
 *  @param[in]  in_size	入力データのサイズ
 *
 */
void base64encode(const u8* in, char* out, u32 in_size){

	const unsigned char base64[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	u32 loopcount;
	u32 i;

	loopcount = (int)(in_size / 3) * 3;	// floor of div 3

	for( i=0; i<loopcount; i+=3 ){

		*(out++) = base64[(in[0] & 0xff) >> 2];
		*(out++) = base64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
		*(out++) = base64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)];
		*(out++) = base64[in[2] & 0x3f];

		in += 3;
	}


	switch(in_size - loopcount){

	case 2:

		*(out++) = base64[(in[0] & 0xff) >> 2];
		*(out++) = base64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
		*(out++) = base64[(in[1] & 0x0f) << 2];
		*(out++) = (unsigned char)'=';

		break;

	case 1:

		*(out++) = base64[(in[0] & 0xff) >> 2];
		*(out++) = base64[(in[0] & 0x03) << 4];
		*(out++) = (unsigned char)'=';
		*(out++) = (unsigned char)'=';

		break;
	}

}

/*!
 *	@brief	BASE64デコード
 *
 *	@param[in]	in		入力データ
 *	@param[in]	out		出力先。in_size / 4 * 3 以上のサイズを用意してください。
 *  @param[in]  in_size	入力データのサイズ。0より大きい値を与えてください。
 *
 *  @retval 0	デコード失敗
 *  @retval 1以上	デコード成功。返された値のサイズで出力されました。
 */
u32 base64decode(const char* in, u8* out, u32 in_size){
	
	u32 i;
	u32 pos = 0;

	DPW_TASSERTMSG(in_size != 0, "in_size must not be 0.\n");

	// len が4の倍数じゃなかったら不正
	if( in_size % 4 != 0 )
		return 0;

	// 4バイトずつ処理
	for( i=0; i<in_size; i+=4 ){

		unsigned char a,b,c,d;

		a = base64(*in++) & 0x3f;
		b = base64(*in++) & 0x3f;
		c = base64(*in++) & 0x7f;
		d = base64(*in++) & 0x7f;

		out[pos++] = ((a << 2) | (b >> 4)) & 0xff;

		if( c < 64 ){

			out[pos++] = ((b << 4) | (c >> 2)) & 0xff;

			if( d < 64 )
				out[pos++] = ((c << 6) | d) & 0xff;

		}

	}

	return pos;
}

/*!
	@brief base64エンコード後のサイズを求めます。
	
	@param[in] in_size		エンコードするデータサイズ

	@return	エンコード後のサイズ
*/
u32 base64size(u32 in_size) {

	return ((in_size / 3) + (in_size % 3 != 0 ? 1 : 0)) * 4;
}


/*-----------------------------------------------------------------------*
					ローカル関数定義
 *-----------------------------------------------------------------------*/

/*!
 *	@brief	BASE64の対応値を取得
 *
 *	@param	a	[in]	入力値
 *
 *	@return	6bit値
 *	@retval	64	'='
 */
static unsigned char base64(unsigned char a){

	if(('A' <= a) && ('Z' >= a)){

		return a - 'A';

	}else if(('a' <= a) && ('z' >= a)){
	
		return a - 'a' + 26;
	
	}else if(('0' <= a) && ('9' >= a)){
	
		return a - '0' + 52;
	
	}else{
		switch( a ){
		case '+':
		case '-':	// for URL safe
			return 62;
		case '/':
		case '_':	// for URL safe
			return 63;
		default:
			return 64;
		}
	}

}

