
//=============================================================================================
/**
 * @file	dma.c
 * @brief	DMAシステムプログラム
 * @author	Hisashi Sogabe
 * @date	2007/04/24
 */
//=============================================================================================
#include "gflib.h"
#include "../build/libraries/mi/common/include/mi_dma.h"

//=============================================================================================
//	定数宣言
//=============================================================================================

#define	GFL_DMA_MAX	(4)		//DMAチャンネルのMAX値

//=============================================================================================
//	型宣言
//=============================================================================================

typedef struct
{
	u32	sad;		//転送元アドレス
	u32	dad;		//転送先アドレス
	union{
		u32	param;
		struct{
			u32	dma_enable		:1;		//DMAイネーブル
			u32	intr_enable		:1;		//割り込み要求発生イネーブル
			u32	dma_mode		:3;		/*DMA起動モード(000:直ちに起動 001:VBlank 010:HBlank 011:表示同期
														100:メインメモリ表示 101:NITROカード 110:カートリッジ
														111:ジオメトリコマンドFIFO）*/
			u32	send_bit		:1;		//転送ビット（0:16bit 1:32bit）
			u32	continue_mode	:1;		//繰り返しモード選択(0:OFF 1:ON）
			u32	sar				:2;		//転送元アドレス更新方法（00:インクリメント 01:デクリメント 10:固定 11:設定禁止）
			u32	dar				:2;		//転送先アドレス更新方法（00:インクリメント 01:デクリメント 10:固定 11:インク/リロード）
			u32	word_count		:21;	//転送回数
		};
	}cnt;
	u32	req_flag	:1;		//設定要求フラグ
	u32	set_flag	:1;		//設定済みフラグ
	u32				:31;
}GFL_DMA_PARAM;

typedef struct
{
	GFL_DMA_PARAM	gdp[GFL_DMA_MAX];
}GFL_DMA_WORK;

//=============================================================================================
//	ワーク
//=============================================================================================

static	GFL_DMA_WORK *gdw=NULL;

//=============================================================================================
//	設定関数
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * DMA初期化
 *
 * @param	heapID	ワークデータを取得するヒープID
 */
//--------------------------------------------------------------------------------------------
void	GFL_DMA_Init(HEAPID heapID)
{
	GF_ASSERT(gdw==NULL);

	gdw=GFL_HEAP_AllocClearMemory(heapID,sizeof(GFL_DMA_WORK));
}

//--------------------------------------------------------------------------------------------
/**
 * DMA設定処理（VBlankの最後で呼ぶことを推奨）
 */
//--------------------------------------------------------------------------------------------
void	GFL_DMA_Main(void)
{
	int	ch;

	if(gdw==NULL){
		return;
	}

	for(ch=0;ch<GFL_DMA_MAX;ch++){
		if(gdw->gdp[ch].req_flag){
			gdw->gdp[ch].req_flag=0;
			gdw->gdp[ch].set_flag=1;
			if(gdw->gdp[ch].cnt.word_count){
				GFL_DMA_Stop(ch);
				MIi_DmaSetParams(ch,(u32)gdw->gdp[ch].sad,(u32)gdw->gdp[ch].dad,gdw->gdp[ch].cnt.param);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * DMA終了
 */
//--------------------------------------------------------------------------------------------
void	GFL_DMA_Exit(void)
{
	int	ch;

	GF_ASSERT(gdw!=NULL);

	for(ch=0;ch<GFL_DMA_MAX;ch++){
		GFL_DMA_Stop(ch);
	}

	GFL_HEAP_FreeMemory(gdw);

	gdw=NULL;
}

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
void	GFL_DMA_Start(int ch,u32 sad,u32 dad,int dma_enable,int intr_enable,int dma_mode,int send_bit,int continue_mode,int sar,int	dar,int word_count)
{
	GF_ASSERT(ch<GFL_DMA_MAX);

	gdw->gdp[ch].sad=sad;
	gdw->gdp[ch].dad=dad;
	gdw->gdp[ch].cnt.param=REG_MI_DMA0CNT_FIELD(dma_enable,intr_enable,dma_mode,send_bit,continue_mode,sar,dar,word_count);

	gdw->gdp[ch].req_flag=1;
}

//--------------------------------------------------------------------------------------------
/**
 * DMA停止
 *
 * @param	ch	停止させたいDMAch
 */
//--------------------------------------------------------------------------------------------
void	GFL_DMA_Stop(int ch)
{
    vu32   *p = (vu32 *)((u32)REG_DMA0CNT_ADDR + ch * 12);

    *p = (vu32)REG_MI_DMA0CNT_FIELD(GFL_DMA_ENABLE,
									gdw->gdp[ch].cnt.intr_enable,
									GFL_DMA_MODE_SOON,
									gdw->gdp[ch].cnt.send_bit,
									GFL_DMA_CONTINUE_MODE_OFF,
									gdw->gdp[ch].cnt.sar,
									gdw->gdp[ch].cnt.dar,
									gdw->gdp[ch].cnt.word_count);

    // ARM7 must wait 2 cycle (load is 3 cycle)
    {
        u32     dummy = reg_MI_DMA0SAD;
    }

    *p = (vu32)REG_MI_DMA0CNT_FIELD(GFL_DMA_DISABLE,
									gdw->gdp[ch].cnt.intr_enable,
									GFL_DMA_MODE_SOON,
									gdw->gdp[ch].cnt.send_bit,
									GFL_DMA_CONTINUE_MODE_OFF,
									gdw->gdp[ch].cnt.sar,
									gdw->gdp[ch].cnt.dar,
									gdw->gdp[ch].cnt.word_count);
	gdw->gdp[ch].set_flag=0;
}

