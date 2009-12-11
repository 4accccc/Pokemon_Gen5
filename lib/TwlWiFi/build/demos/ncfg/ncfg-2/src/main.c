/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - demos - ncfg-2
  File:     main.c

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

/*---------------------------------------------------------------------------*
    NVRAM 上のネットワーク設定を指定した値に設定し、
    その結果をダンプするサンプルプログラムです。
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <nitroWiFi.h>
#include <nitroWiFi/ncfg.h>

#define FLAG_TO_STRING(flag, mask) ((flag & mask) ? "TRUE" : "FALSE")

static u8 WorkMemory[NCFG_CHECKCONFIGEX_WORK_SIZE] ATTRIBUTE_ALIGN(32);

static void MainLoop(void);
static void VBlankIntr(void);

static void CheckNCFG(void);
static void DumpNCFG(void);
static void DumpApInfo(const NCFGApInfo* ap);
static void DumpApInfoEx(const NCFGApInfo* ap, const NCFGApInfoEx* apEx);
static void DumpIp(const u8* ip);
static void ChangeNCFG(void);
static void ChangeNCFGEx(void);

void NitroMain()
{
    OS_Init();
    FX_Init();
    GX_Init();

    GX_DispOff();
    GXS_DispOff();

    // V ブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // 割り込み許可
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // 画面表示開始
    OS_WaitVBlankIntr();
    GX_DispOn();
    GXS_DispOn();

    /* キー入力情報取得の空呼び出し(IPL での A ボタン押下対策) */
    (void)PAD_Read();

    MainLoop();

    OS_Terminate();
}

static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static void MainLoop(void)
{
    DumpNCFG();
    OS_TPrintf("\n===============================================\nCheckNCFG()\n");
    CheckNCFG();
    DumpNCFG();
    OS_TPrintf("\n===============================================\nChangeNCFG()\n");
    ChangeNCFG();
    DumpNCFG();
    OS_TPrintf("\n===============================================\nChangeNCFGEx()\n");
    ChangeNCFGEx();
    DumpNCFG();
}

static void CheckNCFG(void)
{
    s32 result;

    SDK_ASSERT(sizeof(WorkMemory) >= NCFG_CHECKCONFIG_WORK_SIZE);
    result = NCFG_CheckConfig(WorkMemory);
    OS_TPrintf("NCFG_CheckConfig(): %d\n", result);

    SDK_ASSERT(sizeof(WorkMemory) >= NCFG_CHECKCONFIGEX_WORK_SIZE);
    result = NCFG_CheckConfigEx(WorkMemory);
    OS_TPrintf("NCFG_CheckConfigEx(): %d\n", result);
}

static void DumpNCFG(void)
{
    static NCFGConfig config ATTRIBUTE_ALIGN(32);
    static NCFGConfigEx configEx ATTRIBUTE_ALIGN(32);
    s32 result;

    MI_CpuClearFast(&config, sizeof(config));
    MI_CpuClearFast(&configEx, sizeof(configEx));

    SDK_ASSERT(sizeof(WorkMemory) >= NCFG_READCONFIG_WORK_SIZE);
    result = NCFG_ReadConfig(&config, WorkMemory);
    OS_TPrintf("NCFG_ReadConfig(): %d\n", result);

    if ( result == NCFG_RESULT_SUCCESS )
    {
        int i;
        for ( i = 0; i < 3; i++ )
        {
            OS_TPrintf("--------------------------------------\nslot[%d]:\n", i);
            DumpApInfo(&config.slot[i].ap);
        }
        OS_TPrintf("--------------------------------------\n");
    }

    SDK_ASSERT(sizeof(WorkMemory) >= NCFG_READCONFIGEX_WORK_SIZE);
    result = NCFG_ReadConfigEx(&configEx, WorkMemory);
    OS_TPrintf("NCFG_ReadConfigEx(): %d\n", result);

    if ( result == NCFG_RESULT_SUCCESS )
    {
        int i;
        for ( i = 0; i < 3; i++ )
        {
            OS_TPrintf("--------------------------------------\nslot[%d]:\n", i);
            DumpApInfo(&configEx.slot[i].ap);
        }
        for ( i = 0; i < 3; i++ )
        {
            OS_TPrintf("--------------------------------------\nslotEx[%d]:\n", i);
            DumpApInfoEx(&configEx.slotEx[i].ap, &configEx.slotEx[i].apEx);
        }
        OS_TPrintf("--------------------------------------\n");
    }
}

static void DumpApInfoDSCompat(const NCFGApInfo* ap)
{
    switch ( ap->setType )
    {
    case NCFG_SETTYPE_NOT:
        OS_TPrintf("SetType: No Config\n");
        return;

    case NCFG_SETTYPE_MANUAL:
        OS_TPrintf("SetType: Manual\n");
    show_manual:
        OS_TPrintf("SSID: %.20s (len=%d)\n", ap->ssid[0], ap->ssidLength[0]);
        OS_TPrintf("AuthType: %d\n", ap->authType);
        switch ( ap->wepMode )
        {
        case NCFG_WEPMODE_NONE:
            OS_TPrintf("Privacy: None\n");
            break;

        case NCFG_WEPMODE_40:
            OS_TPrintf("Privacy: WEP40\n");
            OS_TPrintf("WEP Notation: %d\n", ap->wepNotation);
            OS_TPrintfEx("WEP Key: %5b\n", ap->wep[0]);
            break;

        case NCFG_WEPMODE_104:
            OS_TPrintf("Privacy: WEP104\n");
            OS_TPrintf("WEP Notation: %d\n", ap->wepNotation);
            OS_TPrintfEx("WEP Key: %13b\n", ap->wep[0]);
            break;

        case NCFG_WEPMODE_128:
            OS_TPrintf("Privacy: WEP128\n");
            OS_TPrintf("WEP Notation: %d\n", ap->wepNotation);
            OS_TPrintfEx("WEP Key: %16b\n", ap->wep[0]);
            break;
        }
        break;

    case NCFG_SETTYPE_AOSS:
        OS_TPrintf("SetType: AOSS\n");
        break;

    case NCFG_SETTYPE_RAKURAKU:
        OS_TPrintf("SetType: RakuRaku\n");
        goto show_manual;

    default:
        OS_TPrintf("SetType: Unknown(%d)\n", ap->setType);
    }
}

static void DumpApInfoCommon(const NCFGApInfo* ap)
{
    OS_TPrintf("State: %d\n", ap->state);
    OS_TPrintf("IP: "); DumpIp(ap->ip); OS_TPrintf("/%d\n", ap->netmask);
    OS_TPrintf("Gateway: "); DumpIp(ap->gateway); OS_TPrintf("\n");
    OS_TPrintf("DNS[0]: "); DumpIp(ap->dns[0]); OS_TPrintf("\n");
    OS_TPrintf("DNS[1]: "); DumpIp(ap->dns[1]); OS_TPrintf("\n");
    OS_TPrintf("MTU: %d\n", ap->mtu);

    //OS_TPrintf("Always Active: %s\n", FLAG_TO_STRING(ap->flags, NCFG_FLAG_ALWAYS_ACTIVE) );
}

static void DumpApInfo(const NCFGApInfo* ap)
{
    switch ( ap->setType )
    {
    case NCFG_SETTYPE_NOT:
        OS_TPrintf("SetType: No Config\n");
        return;

    default:
        DumpApInfoDSCompat(ap);
        break;
    }
    DumpApInfoCommon(ap);
}

static void DumpApInfoEx(const NCFGApInfo* ap, const NCFGApInfoEx* apEx)
{
    switch ( ap->setType )
    {
    case NCFG_SETTYPE_NOT:
        OS_TPrintf("SetType: No Config\n");
        return;

    case NCFG_SETTYPE_MANUAL:
    case NCFG_SETTYPE_AOSS:
    case NCFG_SETTYPE_RAKURAKU:
        DumpApInfoDSCompat(ap);
        break;

    case NCFG_SETTYPE_MANUAL_WPA:
        OS_TPrintf("SetType: Manual(WPA)\n");
    show_manual:
        OS_TPrintf("SSID: %.20s (len=%d)\n", ap->ssid[0], ap->ssidLength[0]);
        OS_TPrintf("Privacy: ");
        switch ( apEx->wpa.wpaMode )
        {
        case NCFG_WPAMODE_WPA_PSK_TKIP:
            OS_TPrintf("WPA-PSK(TKIP)\n");
            break;
        case NCFG_WPAMODE_WPA_PSK_AES:
            OS_TPrintf("WPA-PSK(AES)\n");
            break;
        case NCFG_WPAMODE_WPA2_PSK_TKIP:
            OS_TPrintf("WPA2-PSK(TKIP)\n");
            break;
        case NCFG_WPAMODE_WPA2_PSK_AES:
            OS_TPrintf("WPA2-PSK(AES)\n");
            break;
        }
        OS_TPrintfEx("WPA PMK: %32b\n", apEx->wpa.wpaKey);
        break;

    case NCFG_SETTYPE_AOSS_WPA:
        OS_TPrintf("SetType: AOSS(WPA)\n");
        break;

    case NCFG_SETTYPE_RAKURAKU_WPA:
        OS_TPrintf("SetType: RakuRaku(WPA)\n");
        goto show_manual;

    case NCFG_SETTYPE_WPS_WPA:
        OS_TPrintf("SetType: WPS(WPA)\n");
        goto show_manual;

    default:
        OS_TPrintf("SetType: Unknown(%d)\n", ap->setType);
    }

    DumpApInfoCommon(ap);

    OS_TPrintf("ProxyMode: %d\n", apEx->proxyMode);
    if ( apEx->proxyMode == NCFG_PROXY_MODE_NORMAL )
    {
        OS_TPrintf("ProxyAuthMode: %d\n", apEx->proxyAuthType);
        OS_TPrintf("proxyHost: %.100s\n", apEx->proxyHost);
        OS_TPrintf("proxyPort: %d\n", apEx->proxyPort);
    }
}

static void DumpIp(const u8* ip)
{
#pragma unused(ip)
    OS_TPrintf("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

static NCFGConfig NewConfig ATTRIBUTE_ALIGN(32) =
{
    // slot[]
    {
        // slot[0]
        {
            // ap
            {
                { 0 },                  // ispId
                { 0 },                  // ispPass
                {
                    "AP-SSID-0",        // ssid[0]
                    { 0 },              // ssid[1]
                },
                {
                    "keyfor_wep104",    // wep[0]
                    "keyfor_wep104",    // wep[1]
                    "keyfor_wep104",    // wep[2]
                    "keyfor_wep104",    // wep[3]
                },
                { 0, 0, 0, 0 },         // ip
                { 0, 0, 0, 0 },         // gateway
                {
                    { 0, 0, 0, 0 },     // dns[0]
                    { 0, 0, 0, 0 },     // dns[1]
                },
                0,                      // netmask
                {
                    { 0 },              // wep2[0]
                    { 0 },              // wep2[1]
                    { 0 },              // wep2[2]
                    { 0 },              // wep2[3]
                },
                0,                      // authType
                NCFG_WEPMODE_104,       // wepMode
                NCFG_WEP_NOTATION_ASCII,// wepNotation
                NCFG_SETTYPE_MANUAL,    // setType
                {
                    9,                  // ssidLength[0] // strlen("AP-SSID-0")
                    0,                  // ssidLength[1]
                },
                1400,                   // mtu
                { 0 },                  // rsv
                0,                      // flags
                5,                      // state
            },
            { 0 },                      // wifi
            0,                          // crc
        },
        // slot[1]
        {
            // ap
            {
                { 0 },                  // ispId
                { 0 },                  // ispPass
                {
                    { 0 },              // ssid[0]
                    { 0 },              // ssid[1]
                },
                {
                    { 0 },              // wep[0]
                    { 0 },              // wep[1]
                    { 0 },              // wep[2]
                    { 0 },              // wep[3]
                },
                { 0, 0, 0, 0 },         // ip
                { 0, 0, 0, 0 },         // gateway
                {
                    { 0, 0, 0, 0 },     // dns[0]
                    { 0, 0, 0, 0 },     // dns[1]
                },
                0,                      // netmask
                {
                    { 0 },              // wep2[0]
                    { 0 },              // wep2[1]
                    { 0 },              // wep2[2]
                    { 0 },              // wep2[3]
                },
                0,                      // authType
                0,                      // wepMode
                0,                      // wepNotation
                NCFG_SETTYPE_NOT,       // setType
                {
                    0,                  // ssidLength[0]
                    0,                  // ssidLength[1]
                },
                0,                      // mtu
                { 0 },                  // rsv
                0,                      // flags
                5,                      // state
            },
            { 0 },                      // wifi
            0,                          // crc
        },
        // slot[2]
        {
            // ap
            {
                { 0 },                  // ispId
                { 0 },                  // ispPass
                {
                    "AP-SSID-2",        // ssid[0]
                    { 0 },              // ssid[1]
                },
                {
                    "wep40",            // wep[0]
                    "wep40",            // wep[1]
                    "wep40",            // wep[2]
                    "wep40",            // wep[3]
                },
                { 0, 0, 0, 0 },         // ip
                { 0, 0, 0, 0 },         // gateway
                {
                    { 0, 0, 0, 0 },     // dns[0]
                    { 0, 0, 0, 0 },     // dns[1]
                },
                0,                      // netmask
                {
                    { 0 },              // wep2[0]
                    { 0 },              // wep2[1]
                    { 0 },              // wep2[2]
                    { 0 },              // wep2[3]
                },
                0,                      // authType
                NCFG_WEPMODE_40,        // wepMode
                NCFG_WEP_NOTATION_ASCII,// wepNotation
                NCFG_SETTYPE_MANUAL,    // setType
                {
                    9,                  // ssidLength[0] // strlen("AP-SSID-2")
                    0,                  // ssidLength[1]
                },
                0,                      // mtu
                { 0 },                  // rsv
                NCFG_FLAG_NONE,         // flags
                0,                      // state
            },
            { 0 },                      // wifi
            0,                          // crc
        },
    },
    // rsv
    {
        // ap
        {
            { 0 },                  // ispId
        },
        { 0 },                      // wifi
        0,                          // crc
    }
};
static NCFGConfigEx NewConfigEx ATTRIBUTE_ALIGN(32);

static void ChangeNCFG(void)
{
    s32 result;

    SDK_ASSERT(sizeof(WorkMemory) >= NCFG_WRITECONFIG_WORK_SIZE);
    result = NCFG_WriteConfig(&NewConfig, WorkMemory);
    OS_TPrintf("NCFG_WriteConfig(): %d\n", result);
}

static void ChangeNCFGEx(void)
{
    s32 result;
    NCFGApInfo   *ap;
    NCFGApInfoEx *apEx;

    MI_CpuClearFast(&NewConfigEx, sizeof(NewConfigEx));
    MI_CpuCopyFast(&NewConfig, &NewConfigEx.compat, sizeof(NewConfig));

    // slotEx[0]
    ap   = &NewConfigEx.slotEx[0].ap;
    apEx = &NewConfigEx.slotEx[0].apEx;
    ap->setType = NCFG_SETTYPE_MANUAL_WPA;
    (void)STD_StrLCpy((char*)ap->ssid[0], "AP-SSID-EX1", sizeof(ap->ssid[0]));
    ap->ssidLength[0] = 11; // strlen("AP-SSID-EX1")
    ap->flags = NCFG_FLAG_NONE;
    ap->mtu   = 1500;
    apEx->wpa.wpaMode = NCFG_WPAMODE_WPA2_PSK_AES;
    (void)STD_MemCpy(apEx->wpa.wpaKey, // PMK であって、PassPhrase ではないことに注意
                     "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
                     "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd", 32);
    apEx->proxyMode = NCFG_PROXY_MODE_NONE;

    // slotEx[1]
    ap   = &NewConfigEx.slotEx[1].ap;
    apEx = &NewConfigEx.slotEx[1].apEx;
    ap->setType = NCFG_SETTYPE_RAKURAKU_WPA;
    (void)STD_StrLCpy((char*)ap->ssid[0], "AP-SSID-EX2", sizeof(ap->ssid[0]));
    ap->ssidLength[0] = 11; // strlen("AP-SSID-EX2")
    ap->flags = NCFG_FLAG_NONE;
    ap->mtu   = 576;
    apEx->wpa.wpaMode = NCFG_WPAMODE_WPA_PSK_TKIP;
    (void)STD_MemCpy(apEx->wpa.wpaKey, // PMK であって、PassPhrase ではないことに注意
                     "\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab"
                     "\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab", 32);
    apEx->proxyMode = NCFG_PROXY_MODE_NORMAL;
    apEx->proxyAuthType = NCFG_PROXY_AUTHTYPE_NONE;
    (void)STD_StrLCpy((char*)apEx->proxyHost, "proxy.example.com", sizeof(apEx->proxyHost));
    apEx->proxyPort = 8080;

    // slotEx[2]
    ap   = &NewConfigEx.slotEx[2].ap;
    apEx = &NewConfigEx.slotEx[2].apEx;
    ap->setType = NCFG_SETTYPE_WPS_WPA;
    (void)STD_StrLCpy((char*)ap->ssid[0], "AP-SSID-EX3", sizeof(ap->ssid[0]));
    ap->ssidLength[0] = 11; // strlen("AP-SSID-EX3")
    ap->flags = NCFG_FLAG_NONE;
    ap->mtu   = 1400;
    apEx->wpa.wpaMode = NCFG_WPAMODE_WPA2_PSK_AES;
    (void)STD_MemCpy(apEx->wpa.wpaKey, // PMK であって、PassPhrase ではないことに注意
                     "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
                     "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd", 32);
    apEx->proxyMode = NCFG_PROXY_MODE_NORMAL;
    apEx->proxyAuthType = NCFG_PROXY_AUTHTYPE_BASIC;
    (void)STD_StrLCpy((char*)apEx->proxyHost, "proxy.example.org", sizeof(apEx->proxyHost));
    apEx->proxyPort = 65535;
    (void)STD_StrLCpy((char*)ap->ispId,   "proxyuser", sizeof(ap->ispId));
    (void)STD_StrLCpy((char*)ap->ispPass, "proxypass", sizeof(ap->ispPass));

    // calc state
    {
        int i;
        u8 state = 0;
        for ( i = 0; i < 3; i++ )
        {
            if ( NewConfigEx.slotEx[i].ap.setType != NCFG_SETTYPE_NOT )
            {
                state |= (1 << i);
            }
        }
        NewConfigEx.slotEx[0].ap.state = state;
        NewConfigEx.slotEx[1].ap.state = state;
    }

    SDK_ASSERT(sizeof(WorkMemory) >= NCFG_WRITECONFIGEX_WORK_SIZE);
    result = NCFG_WriteConfigEx(&NewConfigEx, WorkMemory);
    OS_TPrintf("NCFG_WriteConfigEx(): %d\n", result);
}
