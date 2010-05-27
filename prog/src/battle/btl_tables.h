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


