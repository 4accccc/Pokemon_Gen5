
//=============================================================================================
/**
 * @file	dma.h
 * @brief	DMAシステムプログラム
 * @author	Hisashi Sogabe
 * @date	2007/04/26
 */
//=============================================================================================

#ifndef __DMA_H_
#define __DMA_H_

#ifdef __cplusplus
extern "C" {
#endif


//DMAを使う種別
typedef enum {
  GFL_DMA_FS_NO = 1,
  GFL_DMA_NET_NO = 2,
  GFL_DMA_GX_NO= 3,
} GFL_DMA_NO ;




//DMAイネーブルフラグ（0:disable 1:enable）
#define	GFL_DMA_DISABLE				(0)
#define	GFL_DMA_ENABLE				(1)

//割り込み要求発生イネーブル（0:disable 1:enable）
#define	GFL_DMA_INTR_DISABLE		(0)
#define	GFL_DMA_INTR_ENABLE			(1)

/*
DMA起動モード(000:直ちに起動 001:VBlank 010:HBlank 011:表示同期
			  100:メインメモリ表示 101:NITROカード 110:カートリッジ
			  111:ジオメトリコマンドFIFO）
*/
#define	GFL_DMA_MODE_SOON			(0)
#define	GFL_DMA_MODE_VBLANK			(1)
#define	GFL_DMA_MODE_HBLANK			(2)
#define	GFL_DMA_MODE_DISPLAY		(3)
#define	GFL_DMA_MODE_MAINMEMORY		(4)
#define	GFL_DMA_MODE_NITROCARD		(5)
#define	GFL_DMA_MODE_CARTRIDGE		(6)
#define	GFL_DMA_MODE_GEOMETRY		(7)

//転送ビット（0:16bit 1:32bit）
#define	GFL_DMA_SEND_BIT_16			(0)
#define	GFL_DMA_SEND_BIT_32			(1)

//繰り返しモード選択(0:OFF 1:ON）
#define	GFL_DMA_CONTINUE_MODE_OFF	(0)
#define	GFL_DMA_CONTINUE_MODE_ON	(1)

//転送元アドレス更新方法（00:インクリメント 01:デクリメント 10:固定 11:設定禁止）
#define	GFL_DMA_SAR_INC				(0)
#define	GFL_DMA_SAR_DEC				(1)
#define	GFL_DMA_SAR_FIX				(2)

//転送先アドレス更新方法（00:インクリメント 01:デクリメント 10:固定 11:インク/リロード）
#define	GFL_DMA_DAR_INC				(0)
#define	GFL_DMA_DAR_DEC				(1)
#define	GFL_DMA_DAR_FIX				(2)
#define	GFL_DMA_DAR_RELOAD			(3)
 
//--------------------------------------------------------------------------------------------
/**
 * DMA初期化
 *
 * @param	heapID	ワークデータを取得するヒープID
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_DMA_Init(HEAPID heapID);

//--------------------------------------------------------------------------------------------
/**
 * DMA設定処理（VBlankの最後で呼ぶことを推奨）
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_DMA_Main(void);

//--------------------------------------------------------------------------------------------
/**
 * DMA終了
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_DMA_Exit(void);

//--------------------------------------------------------------------------------------------
/**
 * DMA開始
 *
 * @param	ch				使用DMAch
 * @param	sad				転送元アドレス
 * @param	dad				転送先アドレス
 * @param	dma_enable		DMAイネーブルフラグ（0:disable 1:enable）
 * @param	intr_enable		割り込み要求発生イネーブル（0:disable 1:enable）
 * @param	dma_mode		DMA起動モード(000:直ちに起動 001:VBlank 010:HBlank 011:表示同期
 *										  100:メインメモリ表示 101:NITROカード 110:カートリッジ
 *										  111:ジオメトリコマンドFIFO）
 * @param	send_bit		転送ビット（0:16bit 1:32bit）
 * @param	continue_mode	繰り返しモード選択(0:OFF 1:ON）
 * @param	sar				転送元アドレス更新方法（00:インクリメント 01:デクリメント 10:固定 11:設定禁止）
 * @param	dar				転送先アドレス更新方法（00:インクリメント 01:デクリメント 10:固定 11:インク/リロード）
 * @param	word_count		転送回数
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_DMA_Start(int ch,u32 sad,u32 dad,int dma_enable,int intr_enable,int dma_mode,int send_bit,int continue_mode,int sar,int	dar,int word_count);

//--------------------------------------------------------------------------------------------
/**
 * DMA停止
 *
 * @param	ch	停止させたいDMAch
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_DMA_Stop(int ch);

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif __DMA_H_

