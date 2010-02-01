//=============================================================================================
/**
 * @file  display.h
 * @brief 表示システム設定
 */
//=============================================================================================

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <nitro.h>
#include <nnsys.h>

#ifdef __cplusplus
extern "C" {
#endif

//ＢＧ・ＯＢＪ表示用定義
#define VISIBLE_OFF   (0)   // 非表示へ
#define VISIBLE_ON    (1)   // 表示へ


//GFL_DISP_DispSelect( int value )用定義
#define GFL_DISP_3D_TO_MAIN (GX_DISP_SELECT_MAIN_SUB) //上画面がメイン
#define GFL_DISP_3D_TO_SUB  (GX_DISP_SELECT_SUB_MAIN) //下画面がメイン

//=============================================================================================
//  型宣言
//=============================================================================================
///ＶＲＡＭバンク設定構造体
typedef struct  {
  GXVRamBG            main_bg;        //メイン2DエンジンのBGに割り当て
  GXVRamBGExtPltt     main_bg_expltt; //メイン2DエンジンのBG拡張パレットに割り当て
  GXVRamSubBG         sub_bg;         //サブ2DエンジンのBGに割り当て
  GXVRamSubBGExtPltt  sub_bg_expltt;  //サブ2DエンジンのBG拡張パレットに割り当て
  GXVRamOBJ           main_obj;       //メイン2DエンジンのOBJに割り当て
  GXVRamOBJExtPltt    main_obj_expltt;//メイン2DエンジンのOBJ拡張パレットにに割り当て
  GXVRamSubOBJ        sub_obj;        //サブ2DエンジンのOBJに割り当て
  GXVRamSubOBJExtPltt sub_obj_expltt; //サブ2DエンジンのOBJ拡張パレットにに割り当て
  GXVRamTex           teximage;       //テクスチャイメージスロットに割り当て
  GXVRamTexPltt       texpltt;        //テクスチャパレットスロットに割り当て
  GXOBJVRamModeChar   main_mapping;   //メインOBJマッピングモード
  GXOBJVRamModeChar   sub_mapping;    //サブOBJマッピングモード
}GFL_DISP_VRAM;

//--------------------------------------------------------------------------------------------
/**
 * 描画にかかわるVRAMのオールクリア
 *
 * @brief 内部でVRAM割り当てをLCDCに変更してクリアするので、
 *      この関数を呼んだ後に、GFL_DISP_SetBankでVRAMの再割り当てをおこなう必要があります
 *
 * @param withoutVRAM 非クリアにしたいVRAMを指定（NULLで指定なし、GX_VRAM_A～GX_VRAM_I、複数をORで指定可能）
 */
//--------------------------------------------------------------------------------------------
extern  void  GFL_DISP_ClearVRAM( u16 withoutVRAM );

//--------------------------------------------------------------------------------------------
/**
 * ＶＲＡＭバンク全体設定
 *
 * @param data  設定データ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_SetBank( const GFL_DISP_VRAM* dispvram );

//--------------------------------------------------------------------------------------------
/**
 * メイン画面の各面の表示コントロール初期化
 *
 * @param
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_GX_InitVisibleControl( void );
extern void GFL_DISP_GX_InitVisibleControlBG( void );
extern void GFL_DISP_GX_InitVisibleControlOBJ( void );
extern void GFL_DISP_GXS_InitVisibleControl( void );
extern void GFL_DISP_GXS_InitVisibleControlBG( void );
extern void GFL_DISP_GXS_InitVisibleControlOBJ( void );

//--------------------------------------------------------------------------------------------
/**
 * メイン＆サブ画面の各面の表示コントロール
 * BG面のON/OFFはラップ版のbg_sys.hが主にあるのでOBJ面制御用
 *
 * @param gxplanemask   設定面( GX_PLANEMASK_BG0 - GX_PLANEMASK_OBJ )
 * @param flg           コントロールフラグ
 *
 * @li  flg = VISIBLE_ON  : 表示
 * @li  flg = VISIBLE_OFF : 非表示
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_GX_SetVisibleControl( u8 gxplanemask, u8 flg );
extern void GFL_DISP_GXS_SetVisibleControl( u8 gxplanemask, u8 flg );

//--------------------------------------------------------------------------------------------
/**
 * メイン＆サブ画面の各面の表示コントロール（復帰とか）
 *
 * @param prm   表示パラメータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_GX_SetVisibleControlDirect( int prm );
extern void GFL_DISP_GXS_SetVisibleControlDirect( int prm );

//--------------------------------------------------------------------------------------------
/**
 * 表示有効設定
 *
 * @param none
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_SetDispOn( void );

//--------------------------------------------------------------------------------------------
/**
 * 表示無効設定
 *
 * @param none
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_SetDispOff( void );

//--------------------------------------------------------------------------------------------
/**
 * 画面の表示サイド設定
 *
 * @param value 上下どちらをメイン画面にするか（DISP_3D_TO_MAIN:上画面がメイン　DISP_3D_TO_SUB:下画面がメイン）
 */
//--------------------------------------------------------------------------------------------
extern void GFL_DISP_SetDispSelect( int value );

//--------------------------------------------------------------------------------------------
/**
 * メイン画面の表示状況取得
 *
 * @param none
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern int GFL_DISP_GetMainVisible(void);

//--------------------------------------------------------------------------------------------
/**
 * サブ画面の表示状況取得
 *
 * @param none
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern int GFL_DISP_GetSubVisible(void);

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif __DISPLAY_H_
