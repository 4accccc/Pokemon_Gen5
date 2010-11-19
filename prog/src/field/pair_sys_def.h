/*
 *  @file   pair_sys_def.h
 *  @brief  連れ歩きシステムリテラル定義
 *  @author Miyuki Iwasawa
 *  @date   10.11.16
 *
 *  @com  evで読み込みのでenum禁止
 */

#ifndef __H_PAIR_SYS_DEF_H__
#define __H_PAIR_SYS_DEF_H__

#define PAIR_ID_01      (0)
#define PAIR_ID_NATURAL (PAIR_ID_01)
#define PAIR_ID_MAX     (PAIR_ID_01+1)
#define PAIR_ID_NONE    (0xFFFF)

#define PAIR_TYPE_CLOSE (0) //クローズマップ
#define PAIR_TYPE_OPEN  (1) //オープンマップ

#endif //__H_PAIR_SYS_DEF_H__
