//=============================================================================================
/**
 * @file  btl_tables.h
 * @brief ポケモンWB てきとうなサイズのテーブルをVRAM_Hに置いてアクセスする
 * @author  taya
 *
 * @date  2010.05.27  作成
 */
//=============================================================================================
#pragma once


/**
 *  アンコール除外対象のワザか判定
 */
extern BOOL BTL_TABLES_IsMatchEncoreFail( WazaID waza );

/**
 *  ものまね失敗対象のワザ判定
 */
extern BOOL BTL_TABLES_IsMatchMonomaneFail( WazaID waza );



extern const WazaID* BTL_TABLES_GetYubiFuruOmmitTable( u32* elems );
extern BOOL BTL_TABLES_IsYubiFuruOmmit( WazaID waza );


#ifdef PM_DEBUG
extern int GYubiFuruDebugNumber[ BTL_POS_MAX ];

extern void BTL_TABLES_YubifuruDebugReset( void );
extern void BTL_TABLES_YubifuruDebugSetEnd( void );
extern void BTL_TABLES_YubifuruDebugInc( u8 pos );
#endif
