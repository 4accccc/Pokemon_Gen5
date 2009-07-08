//===================================================================
/**
 * @file  debug_print.h
 * @brief 特定ユーザの環境でコンパイルした場合のみ動作するPrintf関数
 * @author  taya
 * @date  07.02.13
 */
//===================================================================
#ifndef __DEBUG_PRINT_H__
#define __DEBUG_PRINT_H__


typedef enum {
  AUTHOR_FREE = 0,  ///< これを指定すると全環境で出力される

  AUTHOR_WATANABE,
  AUTHOR_TAMADA,
  AUTHOR_SOGABE,
  AUTHOR_OHNO,
  AUTHOR_MATSUDA,
  AUTHOR_KAGAYA,
  AUTHOR_ARIIZUMI,
  AUTHOR_TAYA,
  AUTHOR_NAGI,
  AUTHOR_TOMOYA,
  AUTHOR_OBATA,
  AUTHOR_MORI,
  AUTHOR_NAKAHIRO,
  AUTHOR_IWASAWA,
  AUTHOR_NOZOMU,

  AUTHOR_IGNORE,    ///< これを指定すると出力されない
}AUTHOR_ID;


#if defined DEBUG_ONLY_FOR_taya
#define DEFINED_AUTHOR_ID (AUTHOR_TAYA)
#elif defined DEBUG_ONLY_FOR_hiro_nakamura
#define DEFINED_AUTHOR_ID (AUTHOR_NAKAHIRO)
#elif defined DEBUG_ONLY_FOR_genya_hosaka
#define DEFINED_AUTHOR_ID (AUTHOR_HOSAKA)
#elif defined DEBUG_ONLY_FOR_toru_nagihashi
#define DEFINED_AUTHOR_ID (AUTHOR_NAGI)
#elif defined DEBUG_ONLY_FOR_watanabe
#define DEFINED_AUTHOR_ID (AUTHOR_WATANABE)
#elif defined DEBUG_ONLY_FOR_tamada
#define DEFINED_AUTHOR_ID (AUTHOR_TAMADA)
#elif defined DEBUG_ONLY_FOR_sogabe
#define DEFINED_AUTHOR_ID (AUTHOR_SOGABE)
#elif defined DEBUG_ONLY_FOR_ohno
#define DEFINED_AUTHOR_ID (AUTHOR_OHNO)
#elif defined DEBUG_ONLY_FOR_matsuda
#define DEFINED_AUTHOR_ID (AUTHOR_MATSUDA)
#elif defined DEBUG_ONLY_FOR_kagaya
#define DEFINED_AUTHOR_ID (AUTHOR_KAGAYA)
#elif defined DEBUG_ONLY_FOR_ariizumi_nobuhiko
#define DEFINED_AUTHOR_ID (AUTHOR_ARIIZUMI)
#elif defined DEBUG_ONLY_FOR_tomoya_takahashi
#define DEFINED_AUTHOR_ID (AUTHOR_TOMOYA)
#elif defined DEBUG_ONLY_FOR_obata_toshihiro
#define DEFINED_AUTHOR_ID (AUTHOR_OBATA)
#elif defined DEBUG_ONLY_FOR_mori
#define DEFINED_AUTHOR_ID (AUTHOR_MORI)
#elif defined DEBUG_ONLY_FOR_iwasawa
#define DEFINED_AUTHOR_ID (AUTHOR_IWASAWA)
#elif defined DEBUG_ONLY_FOR_saitou
#define DEFINED_AUTHOR_ID (AUTHOR_NOZOMU)
#else
#define DEFINED_AUTHOR_ID (AUTHOR_FREE)
#endif


// デバッグプリント有効フラグを PM_DEBUG とは別にしておく
#ifdef PM_DEBUG
#define DEBUG_PRINT_ENABLE
#endif


#ifdef DEBUG_PRINT_ENABLE

//==============================================================================================
/**
 * 特定ユーザの環境でコンパイルした場合のみ動作するPrintf関数
 *
 * @param   author_id   ユーザID（debug_print.hに記述）
 * @param   fmt       OS_TPrintfに渡す書式付き文字列
 *
 */
//==============================================================================================
extern void DEBUG_Printf( AUTHOR_ID author_id, const char * fmt, ... );

#define   DEBUG_PrintfEx( flg, id, ... )    {  if(flg){ DEBUG_Printf(id, __VA_ARGS__); } }

#else

#define   DEBUG_Printf( id, ... )       ((void)0)
#define   DEBUG_PrintfEx( flg, id, ... )    ((void)0)

#endif


//-----------------------------------------------------------
// かんたん記述用マクロ
//-----------------------------------------------------------
#define   WATANABE_Printf( ... )  DEBUG_Printf( AUTHOR_WATANABE, __VA_ARGS__ )
#define   TAMADA_Printf( ...)     DEBUG_Printf( AUTHOR_TAMADA, __VA_ARGS__ )
#define   SOGABE_Printf( ...)     DEBUG_Printf( AUTHOR_SOGABE, __VA_ARGS__ )
#define   MORI_Printf( ... )      DEBUG_Printf( AUTHOR_MORI, __VA_ARGS__ )
#define   MATSUDA_Printf( ...)    DEBUG_Printf( AUTHOR_MATSUDA, __VA_ARGS__ )
#define   KAGAYA_Printf( ...)     DEBUG_Printf( AUTHOR_KAGAYA, __VA_ARGS__ )
#define   NAKAHIRO_Printf( ... )  DEBUG_Printf( AUTHOR_NAKAHIRO, __VA_ARGS__ )
#define   TAYA_Printf( ... )      DEBUG_Printf( AUTHOR_TAYA, __VA_ARGS__ )
#define   OHNO_Printf( ...)       DEBUG_Printf( AUTHOR_OHNO, __VA_ARGS__ )
#define   TOMOYA_Printf( ... )    DEBUG_Printf( AUTHOR_TOMOYA, __VA_ARGS__ )
#define   IWASAWA_Printf( ... )   DEBUG_Printf( AUTHOR_IWASAWA, __VA_ARGS__ )
#define   NOZOMU_Printf( ... )    DEBUG_Printf( AUTHOR_NOZOMU, __VA_ARGS__ )
#define   NAGI_Printf( ... )      DEBUG_Printf( AUTHOR_NAGI, __VA_ARGS__ )
#define   HOSAKA_Printf( ... )    DEBUG_Printf( AUTHOR_HOSAKA, __VA_ARGS__ )
#define   ARIIZUMI_Printf( ...)   DEBUG_Printf( AUTHOR_ARIIZUMI, __VA_ARGS__ )
#define   OBATA_Printf( ... )     DEBUG_Printf( AUTHOR_OBATA, __VA_ARGS__ )

#endif
