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

/**
 *  プレッシャー対象ワザ判定
 */
extern BOOL BTL_TABLES_IsPressureEffectiveWaza( WazaID waza );


/**
 *  さきどり失敗対象ワザ判定
 */
extern BOOL BTL_TABLES_IsSakidoriFailWaza( WazaID waza );

/**
 *  トレースできないとくせい判定
 */
extern BOOL BTL_TABLES_CheckTraceFailTokusei( u16 tokuseiID );

/**
 *  「なりきり」できないとくせい判定
 */
extern BOOL BTL_TABLES_CheckNarikiriFailTokusei( u16 tokuseiID );



extern const WazaID* BTL_TABLES_GetYubiFuruOmmitTable( u32* elems );
extern BOOL BTL_TABLES_IsYubiFuruOmmit( WazaID waza );
extern BOOL BTL_TABLES_CheckItemCallNoEffect( u16 itemID );


/**
 *  メンタルハーブ対応の状態異常チェック
 */
extern WazaSick  BTL_TABLES_GetMentalSickID( u32 idx );

/**
 *  使用する対象ポケモンの選択を行う必要が無いアイテムか判別
 */
extern BOOL BTL_TABLES_IsNoTargetItem( u16 itemNo );


#ifdef PM_DEBUG
extern int GYubiFuruDebugNumber[ BTL_POS_MAX ];

extern void BTL_TABLES_YubifuruDebugReset( void );
extern void BTL_TABLES_YubifuruDebugSetEnd( void );
extern void BTL_TABLES_YubifuruDebugInc( u8 pos );
#endif
