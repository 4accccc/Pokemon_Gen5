/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - libraries
  File:     info.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitroWiFi/ncfg.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static s32 checkConfig(NCFGConfig* config, const MATHCRC16Table *tbl);
static s32 checkConfigEx(NCFGConfigEx* configEx, const MATHCRC16Table *tbl);
static int init(NCFGConfig* config);
static void initPage(NCFGConfig* config, int page);
static void initPageEx(NCFGConfigEx* configEx, int page);
static BOOL readAllSlots(NCFGConfig* config);
static BOOL readAllExSlots(NCFGConfigEx* configEx);
static BOOL writeAllSlots(NCFGConfig* config, const MATHCRC16Table *tbl);
static BOOL writeAllExSlots(NCFGConfigEx* configEx, const MATHCRC16Table *tbl);

/*---------------------------------------------------------------------------*
  Name:         NCFG_ReadConfig

  Description:  ネットワーク設定を取得する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    config - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_ReadConfig(NCFGConfig* config, void* work)
{
    (void)work;
    return readAllSlots(config) ? NCFG_RESULT_SUCCESS : NCFG_RESULT_FAILURE;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_WriteConfig

  Description:  ネットワーク設定を設定する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    config - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
typedef struct MEMWRITE {
    NCFGConfig        config;          // 内蔵バックアップメモリマップバッファ
    MATHCRC16Table    tbl;             // CRC 計算テーブル
}  MEMWRITE;

s32 NCFG_WriteConfig(const NCFGConfig* config, void* work)
{
    MEMWRITE* wk = work;

    SDK_ASSERT(wk);

    MI_CpuCopy8(config, &wk->config, sizeof(NCFGConfig));
    MATH_CRC16InitTable(&wk->tbl);

    return writeAllSlots(&wk->config, &wk->tbl) ? NCFG_RESULT_SUCCESS : NCFG_RESULT_FAILURE;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_ReadConfigEx

  Description:  拡張ネットワーク設定を取得する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    configEx - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_ReadConfigEx(NCFGConfigEx* configEx, void* work)
{
    BOOL result;

    result = (NCFG_ReadConfig(&configEx->compat, work) == NCFG_RESULT_SUCCESS);

    if ( NCFG_GetFormatVersion() >= NCFG_FORMAT_VERSION_TWL )
    {
        result = result && readAllExSlots(configEx);
    }
    else
    {
        result = FALSE;
    }

    return result ? NCFG_RESULT_SUCCESS : NCFG_RESULT_FAILURE;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_WriteConfigEx

  Description:  拡張ネットワーク設定を設定する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    configEx - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
typedef struct MEMWRITEEX {
    NCFGConfigEx      configEx;        // 内蔵バックアップメモリマップバッファ
    MATHCRC16Table    tbl;             // CRC 計算テーブル
}  MEMWRITEEX;

s32 NCFG_WriteConfigEx(const NCFGConfigEx* configEx, void* work)
{
    MEMWRITEEX* wk = work;
    BOOL result;

    SDK_ASSERT(wk);

    result = (NCFG_WriteConfig(&configEx->compat, work) == NCFG_RESULT_SUCCESS);

    MI_CpuCopy8(configEx, &wk->configEx, sizeof(NCFGConfigEx));
    MATH_CRC16InitTable(&wk->tbl);

    if ( NCFG_GetFormatVersion() >= NCFG_FORMAT_VERSION_TWL )
    {
        result = result && writeAllExSlots(&wk->configEx, &wk->tbl);
    }
    else
    {
        result = FALSE;
    }

    return result ? NCFG_RESULT_SUCCESS : NCFG_RESULT_FAILURE;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_CheckConfig

  Description:  ネットワーク設定の一貫性を確認し、修復する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    work   - ワークメモリ

  Returns:      s32   -      0 : 正常終了
                        -10002 : 接続先設定が消失したが正常終了
                        -10003 : ユーザ ID が消失したが正常終了
                        -10000 : 内蔵バックアップメモリの書き込みエラー終了
                        -10001 : 内蔵バックアップメモリの読み込みエラー終了
 *---------------------------------------------------------------------------*/
// チェック関数使用ワークメモリ
typedef struct MEMCHECK {
	NCFGConfig        config;          // 内蔵バックアップメモリマップバッファ
	MATHCRC16Table    tbl;             // CRC 計算テーブル
}  MEMCHECK;

s32 NCFG_CheckConfig(void* work)
{
	MEMCHECK* wk = work;

	SDK_ASSERTMSG( ( (int)work & 0x1f ) == 0, "work must be 32bytes alignment - %p\n", work );
	
	// ワークメモリを念のためにゼロクリアする
	MI_CpuClear8(work, NCFG_CHECKCONFIG_WORK_SIZE);
	
	// 内蔵バックアップメモリを初期化
	if (NCFGi_InitBackupMemory() != NCFG_RESULT_SUCCESS)  return  NCFG_RESULT_INIT_ERROR_READ;

	// CRC 初期化
	MATH_CRC16InitTable(&wk->tbl);

	// 内蔵バックアップメモリを読み込み
	if (! readAllSlots(&wk->config))    return  NCFG_RESULT_INIT_ERROR_READ;

    return checkConfig(&wk->config, &wk->tbl);
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_CheckConfigEx

  Description:  拡張ネットワーク設定の一貫性を確認し、修復する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    work   - ワークメモリ

  Returns:      s32   -      0 : 正常終了
                        -10002 : 接続先設定が消失したが正常終了
                        -10003 : ユーザ ID が消失したが正常終了
                        -10000 : 内蔵バックアップメモリの書き込みエラー終了
                        -10001 : 内蔵バックアップメモリの読み込みエラー終了
 *---------------------------------------------------------------------------*/
// チェック関数使用ワークメモリ
typedef struct MEMCHECKEX {
    NCFGConfigEx      configEx;        // 内蔵バックアップメモリマップバッファ
    MATHCRC16Table    tbl;             // CRC 計算テーブル
}  MEMCHECKEX;

s32 NCFG_CheckConfigEx(void* work)
{
    MEMCHECKEX* wk = work;

    SDK_ASSERTMSG( ( (int)work & 0x1f ) == 0, "work must be 32bytes alignment - %p\n", work );

    // ワークメモリを念のためにゼロクリアする
    MI_CpuClear8(work, NCFG_CHECKCONFIGEX_WORK_SIZE);

    // 内蔵バックアップメモリを初期化
    if (NCFGi_InitBackupMemory() != NCFG_RESULT_SUCCESS)
    {
        return NCFG_RESULT_INIT_ERROR_READ;
    }

    // CRC 初期化
    MATH_CRC16InitTable(&wk->tbl);

    // 内蔵バックアップメモリを読み込み
    if (! readAllSlots(&wk->configEx.compat))
    {
        return NCFG_RESULT_INIT_ERROR_READ;
    }
    if (! readAllExSlots(&wk->configEx))
    {
        return NCFG_RESULT_INIT_ERROR_READ;
    }

    return checkConfigEx(&wk->configEx, &wk->tbl);
}

static s32 checkConfig(NCFGConfig* config, const MATHCRC16Table *tbl)
{
	int       i;
	u16       hash;
	BOOL      c[4];
	BOOL      clear;
    u8        state;
    u8        prevState;

	// CRC をチェック
	MI_CpuClear8(c, sizeof(c));
	for (i = 0; i < 3; ++ i) {
		hash = MATH_CalcCRC16(tbl, &config->slot[i], 0xFE);
		if ((hash == config->slot[i].crc) && NCFGi_CheckApInfo(&config->slot[i].ap)) {
			c[i] = TRUE;
		}
	}
	hash = MATH_CalcCRC16(tbl, &config->rsv, 0xFE);
	if (hash == config->rsv.crc)  c[3] = TRUE;

	// 正常終了
	if (c[0] && c[1] && c[2] && c[3]) {
		return  NCFG_RESULT_INIT_OK;
	}

    prevState = config->slot[0].ap.state;

    // バックアップメモリ破壊のための復旧処理
    if (! c[0])
    {
        initPage(config, 0);
        if (c[1])
        {
            prevState = config->slot[1].ap.state;
            MI_CpuCopy8(config->slot[1].wifi, config->slot[0].wifi, 0x0e);
        }
        else
        {
            // このケースは NCFG_RESULT_INIT_OK_INIT を返す必要があるので別にケアする
            prevState = 0;
        }
    }
    if (! c[1])
    {
        initPage(config, 1);
        if (c[0])
        {
            MI_CpuCopy8(config->slot[0].wifi, config->slot[1].wifi, 0x0e);
        }
    }
    if (! c[2])
    {
        initPage(config, 2);
    }
    if (! c[3])
    {
        MI_CpuClear16(&config->rsv, sizeof(NCFGSlotInfo));
    }

    // 使用状況フラグを ap.setType から復旧
    clear = FALSE;
    state = 0;
    for (i = 0; i < 3; ++ i)
    {
        if (config->slot[i].ap.setType != NCFG_SETTYPE_NOT)
        {
            state |= (1 << i);
        }
        else
        {
            if (prevState & (1 << i))
            {
                // 存在していた設定データが消えた
                clear = TRUE;
            }
        }
    }
    config->slot[0].ap.state = state;
    config->slot[1].ap.state = state;

    if (! writeAllSlots(config, tbl))
    {
        return NCFG_RESULT_INIT_ERROR_WRITE;
    }

    if ((! c[0]) && (! c[1]) && (! c[2]))
    {
        // 全領域が CRC エラーだった場合は
        // 初回起動と見なし、エラーを返さない
        return NCFG_RESULT_INIT_OK;
    }
    else if ((! c[0]) && (! c[1]))
    {
        // WiFi ユーザ ID が消えた
        // NCFG_RESULT_INIT_OK_INIT は NCFG_RESULT_INIT_ERASE より優先する
        return NCFG_RESULT_INIT_OK_INIT;
    }
    else
    {
        return (clear ? NCFG_RESULT_INIT_OK_ERASE : NCFG_RESULT_INIT_OK);
    }
}

static s32 checkConfigEx(NCFGConfigEx* configEx, const MATHCRC16Table *tbl)
{
    int       i;
    int       result;
    u16       hash;
    u16       hashEx;
    BOOL      c[3];
    BOOL      clear;
    u8        prevState;
    u8        state;

    // 互換部分をチェック
    result = checkConfig(&configEx->compat, tbl);

    // CRC をチェック
    MI_CpuClear8(c, sizeof(c));
    for (i = 0; i < 3; ++ i)
    {
        hash = MATH_CalcCRC16(tbl, &configEx->slotEx[i], 0xFE);
        hashEx = MATH_CalcCRC16(tbl, ((u8*)&configEx->slotEx[i]) + 0x100, 0xFE);
        if ((hash == configEx->slotEx[i].crc) && (hashEx == configEx->slotEx[i].crcEx)
            && NCFGi_CheckApInfoEx(&configEx->slotEx[i].ap, &configEx->slotEx[i].apEx))
        {
            c[i] = TRUE;
        }
    }

    // 正常終了
    if (c[0] && c[1] && c[2])
    {
        return result; // 互換部分の結果をそのまま返答
    }

    prevState = configEx->slotEx[0].ap.state;

    // バックアップメモリ破壊のための復旧処理
    if (! c[0])
    {
        initPageEx(configEx, 0);
        if (c[1])
        {
            prevState = configEx->slotEx[1].ap.state;
        }
        else
        {
            // 設定が全部存在していたと仮定する
            prevState = 0x07;
        }
    }
    if (! c[1])
    {
        initPageEx(configEx, 1);
    }
    if (! c[2])
    {
        initPageEx(configEx, 2);
    }

    // 使用状況フラグを ap.setType から復旧
    clear = FALSE;
    state = 0;
    for (i = 0; i < 3; ++ i)
    {
        if (configEx->slotEx[i].ap.setType != NCFG_SETTYPE_NOT)
        {
            state |= (1 << i);
        }
        else
        {
            if (prevState & (1 << i))
            {
                clear = TRUE;
            }
        }
    }
    configEx->slotEx[0].ap.state = state;
    configEx->slotEx[1].ap.state = state;

    if (! writeAllExSlots(configEx, tbl))
    {
        return NCFG_RESULT_INIT_ERROR_WRITE;
    }

    if ((! c[0]) && (! c[1]) && (! c[2]))
    {
        // 全領域が CRC エラーだった場合は
        // 初回起動と見なし、エラーとしない
        clear = FALSE;
    }

    if (result != NCFG_RESULT_INIT_OK)
    {
        // result は NCFG_RESULT_INIT_OK_INIT か NCFG_RESULT_INIT_ERASE
        // NCFG_RESULT_INIT_OK_INIT は NCFG_RESULT_INIT_ERASE より優先する
        return result;
    }
    else if (clear)
    {
        // 設定済みの領域がどこか消えた
        return NCFG_RESULT_INIT_OK_ERASE;
    }
    else
    {
        return NCFG_RESULT_INIT_OK;
    }
}

//-----------------------------------------------------------------------------
// 機能 : 初期化処理
// 引数 : config - 内蔵バックアップメモリ
// 返値 : int  - エラーコード
//-----------------------------------------------------------------------------
static int  init(NCFGConfig* config)
{
	int          i;

	MI_CpuClear16(config, sizeof(NCFGConfig));
	for (i = 0; i < 3; ++ i) {
		config->slot[i].ap.setType = NCFG_SETTYPE_NOT;
	}

	return  0;
}

//-----------------------------------------------------------------------------
// 機能 : 1 ページを初期化
// 引数 : config - 内蔵バックアップメモリ
//        page  - 初期化するページ
// 返値 :
//-----------------------------------------------------------------------------
static void  initPage(NCFGConfig* config, int page)
{
	MI_CpuClear16(&config->slot[page], sizeof(NCFGSlotInfo));
	config->slot[page].ap.setType = NCFG_SETTYPE_NOT;
}

//-----------------------------------------------------------------------------
// 機能 : 拡張領域の 1 ページを初期化
// 引数 : config - 内蔵バックアップメモリ
//        page  - 初期化するページ
// 返値 :
//-----------------------------------------------------------------------------
static void initPageEx(NCFGConfigEx* configEx, int page)
{
    MI_CpuClear16(&configEx->slotEx[page], sizeof(NCFGSlotInfoEx));
    configEx->slotEx[page].ap.setType = NCFG_SETTYPE_NOT;
}

//*****************************************************************************
// 機能 : 内蔵バックアップメモリを読み込み
// 引数 : mem   - 読み込み先バッファ  <<-- 返値
// 返値 : BOOL  - TRUE  : 成功 / FALSE : 失敗
//*****************************************************************************
static BOOL readAllSlots(NCFGConfig* config)
{
    int i;
    BOOL result = TRUE;

    for (i = 0; i < 3; i++)
    {
        result = result &&
            (NCFG_ReadBackupMemory(&config->slot[i], sizeof(config->slot[i]), NCFG_SLOT_1+i) >= 0);
    }
    result = result &&
        (NCFG_ReadBackupMemory(&config->rsv, sizeof(config->rsv), NCFG_SLOT_RESERVED) >= 0);
    return result;
}

static BOOL readAllExSlots(NCFGConfigEx* configEx)
{
    int i;
    BOOL result = TRUE;

    for (i = 0; i < 3; i++)
    {
        result = result &&
            (NCFG_ReadBackupMemory(&configEx->slotEx[i], sizeof(configEx->slotEx[i]), NCFG_SLOT_EX_1+i) >= 0);
    }
    return result;
}

//*****************************************************************************
// 機能 : 内蔵バックアップメモリに書き込み
// 引数 : work  - 初期化関数使用ワークメモリ
// 返値 : BOOL  - TRUE  : 成功 / FALSE : 失敗
//*****************************************************************************
static BOOL writeAllSlots(NCFGConfig* config, const MATHCRC16Table *tbl)
{
    int  i;
    BOOL result = TRUE;

    for (i = 0; i < 3; i++)
    {
        config->slot[i].crc = MATH_CalcCRC16(tbl, &config->slot[i], sizeof(config->slot[i])-2);
        result = result &&
            (NCFG_WriteBackupMemory(NCFG_SLOT_1+i, &config->slot[i], sizeof(config->slot[i])) >= 0);
    }
    config->rsv.crc = MATH_CalcCRC16(tbl, &config->rsv, sizeof(config->rsv)-2);
    result = result &&
        (NCFG_WriteBackupMemory(NCFG_SLOT_RESERVED, &config->rsv, sizeof(config->rsv)) >= 0);
    return result;
}

static BOOL writeAllExSlots(NCFGConfigEx* configEx, const MATHCRC16Table *tbl)
{
    int  i;
    BOOL result = TRUE;

    for (i = 0; i < 3; i++)
    {
        configEx->slotEx[i].crc   = MATH_CalcCRC16(tbl, &configEx->slotEx[i],              0xFE);
        configEx->slotEx[i].crcEx = MATH_CalcCRC16(tbl, (u8*)&configEx->slotEx[i] + 0x100, 0xFE);
        result = result &&
            (NCFG_WriteBackupMemory(NCFG_SLOT_EX_1+i, &configEx->slotEx[i], sizeof(configEx->slotEx[i])) >= 0);
    }
    return result;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
