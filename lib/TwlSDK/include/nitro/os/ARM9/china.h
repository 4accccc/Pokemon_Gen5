/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     china.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-05-28#$
  $Rev: 10639 $
  $Author: mizutani_nakaba $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_ARM9_CHINA_H_
#define NITRO_OS_ARM9_CHINA_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#define OS_BURY_STRING_FORCHINA         "[SDK+NINTENDO:FORCHINA]"
#define OS_BUSY_STRING_LEN_FORCHINA     23

typedef enum
{
    OS_CHINA_ISBN_DISP,
    OS_CHINA_ISBN_NO_DISP,
    OS_CHINA_ISBN_CHECK_ROM
}OSChinaIsbn;

/*---------------------------------------------------------------------------*
  Name:         OS_InitChina

  Description:  SDK の OS ライブラリを初期化する。
                指向地が中国であるアプリケーション専用。
                OS_Init 関数の代わりに当関数を使用する。

  Arguments:    isbn    -   ISBN 番号等に関する文字列配列を指定する。
                            {
                                char    ISBN[ 17 ] ,
                                char    合同登記号[ 12 ] ,
                                char    新出音管(左)[ 4 ] ,
                                char    新出音管(右)[ 3 ]
                            }

                param   -   ISBN 番号表示に関する動作を指定します。

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_InitChina(const char **isbn, OSChinaIsbn param);

/*---------------------------------------------------------------------------*
  Name:         OS_ShowAttentionChina

  Description:  中国対応ロゴ、及び注意画面を所定時間表示する。
                この関数はユーザアプリケーション側で再定義することができる。

  Arguments:    isbn    -   ISBN 番号等に関する文字列配列。
                param   -   ISBN 番号表示に関する動作を指定します。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_ShowAttentionChina(const char **isbn, OSChinaIsbn param);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_OS_ARM9_CHINA_H_ */

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
