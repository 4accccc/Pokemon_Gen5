//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：リゾートショップ関連
 * @file   scrcmd_resort_shop.h
 * @author Toru Nagihashi （scrcmd_shopを改造して作成）
 * @date   2011.03.03
 *
 */
//////////////////////////////////////////////////////////////////////
#pragma once


typedef struct {
  u16 id;       // どうぐID
  u32 price;    // 値段 or BPポイント
}RESORT_SHOP_ITEM;

// 一度に並べることができる商品数
#define RESORT_SHOP_ITEM_MAX   ( 40 )

//--------------------------------------------------------------------
/**
 * @brief 通常ショップ呼び出し 
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdCallResortShop( VMHANDLE* core, void* wk );

