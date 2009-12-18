#ifndef DS_WEB_INTERFACE_DEFS_INCLUDE_H__
#define DS_WEB_INTERFACE_DEFS_INCLUDE_H__

/*
 * DS が受け取る HTTP レスポンス（HTTP BODY 部）の共通ヘッダ
 *
 * 機能によっては、ヘッダ以外の情報が存在するが、この情報は
 * 共通ヘッダ部に連続するメモリに格納されているため、下記のような
 * 方法により取得する。
 *
 * 例1）
 *   struct gs_response* resp = (struct gs_response*)HTTP_BOD_ADDR;
 *   struct app_func1_info* info = (struct app_func1_info*)&resp[1];
 *
 * 例2）
 *   struct gs_response* resp = (struct gs_response*)HTTP_BOD_ADDR;
 *   struct app_func1_info* info = (struct app_func1_info*)&resp->ext_info[1];
 *
 */
/*
 * [09-12-06]
 *		ret_cd に設定する値を enum に変更したため、型を "unsigned short" から "long"
 *		に変更した。
 *		これにともない、メンバの位置を調整。
 */
typedef struct
{
	long ret_cd;				/* 処理結果コード			=0:正常, <>0:以上				*/
	unsigned long body_size;	/* データ部バイトサイズ										*/
	unsigned long start_sec;	/* サーバ処理開始時間										*/
	char yobi1[8];				/* 予備														*/
	unsigned short desc_cd;		/* 詳細エラーコード			ret_cd=0 の場合に設定される		*/
	char desc_msg[80];			/* 詳細エラー文言			PHP サーバプログラム内部情報	*/
	char yobi2[25];				/* 予備														*/
	char ext_info[1];			/* 位置確認用				常に 'Z'						*/
}  gs_response;

/*
 * DS が受け取る HTTP レスポンス（HTTP BODY 部）の機能別データ部
 *
 * 上記の gs_response 構造体に連続するメモリに配置されるが、リクエストを行った
 * 機能ごとに異なるデータ部が設定される。
 * また、機能によってはデータ部の存在しない場合もあり、どちらの場合も
 * 共通ヘッダ中の body_size にデータ部に設定されたバイトサイズが設定されている。
 *
 */

/*
 * GET インターフェーステスト用構造体
 *
 * URI: /gs?p=test.get&gsid=1234
 * リクエストメソッド: GET
 *
 * 説明: クライアント(DS)とサーバのデータ通信用の試験を行うための構造体。
 *       試験用であるため、サーバから返却されるデータは固定値である。
 *
 */
typedef struct 
{
	unsigned long u32_val1;		/* =4294967294 (-2)	*/
	unsigned short u16_val1;	/* =65534 (-2)		*/
	unsigned char u8_val1;		/* =254 (-2)		*/

	char s8_val1;				/* =-2				*/
	long s32_val1;				/* =-2				*/
	short s16_val1;				/* =-2				*/

	char s8_val2;				/* =66 ('B')		*/
	char str_val1[9];			/* ="abcd0123"		*/
}respbody_test_get;

/*
 * POST インターフェーステスト用構造体
 *
 * URI: /gs?p=test.post&gsid=1234
 * リクエストメソッド: POST
 *
 * 説明: クライアント(DS)とサーバのデータ通信用の試験を行うための構造体。
 *       リクエストとレスポンスは同じ形であり、レスポンスとして返却される
 *       データは、リクエストが行われた値を以下の規則で編集したものである。
 *
 *       編集規則:
 *           u32_val1, u16_val1, u8_val1 ... 元の値 - 1
 *           s8_val1, s32_val1, s16_val1 ... 元の値 * 2
 *           s8_val2                     ... 小文字に変換
 *           str_val1                    ... 文字列を反転
 *
 */
typedef struct 
{
	unsigned long u32_val1;
	unsigned short u16_val1;
	unsigned char u8_val1;

	char s8_val1;
	long s32_val1;
	short s16_val1;

	char s8_val2;
	char str_val1[9];
}iobody_test_post;

#endif
