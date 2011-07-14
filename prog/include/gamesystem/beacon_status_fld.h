/*
 *  @file   beacon_status_fld.h
 *  @brief  ゲーム中保持するビーコンログ管理 OVERLAY_BEACON_VIEW上
 *  @author Miyuki Iwasawa
 *  @date   11.07.13
 *
 *  beacon_status.c は常駐なので、すれ違いログ画面上からだけアクセスできれば
 *  よい関数をオーバーレイに追い出している
 */
#pragma once

//----------------------------------------------------
/*
 *  @brief  特殊Gパワーストック
 */
//----------------------------------------------------
extern void BEACON_STATUS_SpGPowerSet( BEACON_STATUS* wk, SP_GPOWER_REQ req_id );

//----------------------------------------------------
/*
 *  @brief  特殊Gパワー　一番古いリクエストを取得
 *
 *  @retval SP_GPOWER_REQ_NULL  リクエスト無し
 *  @retval SP_GPOWER_REQ_XXXX  リクエストID
 */
//----------------------------------------------------
extern SP_GPOWER_REQ BEACON_STATUS_SpGPowerGet( BEACON_STATUS* wk );

//----------------------------------------------------
/*
 *  @brief  特殊Gパワー　一番古いリクエストを削除
 */
//----------------------------------------------------
extern SP_GPOWER_REQ BEACON_STATUS_SpGPowerDel( BEACON_STATUS* wk );

//----------------------------------------------------
/*
 *  @brief  ビーコンGパワー　セット　
 */
//----------------------------------------------------
extern void BEACON_STATUS_BeaconGPowerSet( BEACON_STATUS* wk, GPOWER_ID g_power, STRBUF* name );

//----------------------------------------------------
/*
 *  @brief  ビーコンGパワー　取得
 */
//----------------------------------------------------
extern GPOWER_ID BEACON_STATUS_BeaconGPowerGet( BEACON_STATUS* wk, STRBUF* name );

//----------------------------------------------------
/*
 *  @brief  ビーコンGパワー 削除
 */
//----------------------------------------------------
extern void BEACON_STATUS_BeaconGPowerDel( BEACON_STATUS* wk );

