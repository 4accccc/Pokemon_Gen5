/*===========================================================================*
  Project:  Syachi PokemonGDS Network Library
  File:     poke_net_ds_ssl.c

  $NoKeywords: $
 *===========================================================================*/
//===========================================================================
#include "poke_net.h"

#ifdef ___POKE_NET_BUILD_DS___

#include "poke_net_ds_ssl_cert.c"
#include "poke_net_ds_ssl.h"

static SOCSslConnection     g_sslCon;
static SOCSslConnection*    g_psslCon = NULL;

SOCCaInfo*     ca_list[] = { &ca_info };

/*****************************************************************************/

/* function */

/*---------------------------------------------------------------------------*
  Name:         SslAuthCallback

  Description:  SSL サーバ認証コールバック.

  Arguments:    con           SSL接続情報
                level         レベル

  Returns:      None.
 *---------------------------------------------------------------------------*/
int SslAuthCallback(int result, SOCSslConnection* con, int level)
{
#pragma unused(con)
#pragma unused(level)
    {
        OS_TPrintf("SSL: %d s:<%s>\n", level, con->subject);
        OS_TPrintf("SSL:     /CN=%s/\n", con->cn);
        OS_TPrintf("SSL:   i:<%s>\n", con->issuer);
    }

    if (result & SOC_CERT_OUTOFDATE)
    {
        OS_TPrintf("SSL: Certificate is out-of-date\n");
    }

    if (result & SOC_CERT_BADSERVER)
    {
        OS_TPrintf("SSL: Server name does not match\n");
        result &= ~SOC_CERT_BADSERVER;
    }

    switch (result & SOC_CERT_ERRMASK)
    {
    case SOC_CERT_NOROOTCA:
        OS_TPrintf("SSL: No root CA installed.(ignored)\n");
        result &= ~SOC_CERT_ERRMASK;    //ignore
        break;

    case SOC_CERT_BADSIGNATURE:
        OS_TPrintf("SSL: Bad signature\n");
        break;

    case SOC_CERT_UNKNOWN_SIGALGORITHM:
        OS_TPrintf("SSL: Unknown signature algorithm\n");
        break;

    case SOC_CERT_UNKNOWN_PUBKEYALGORITHM:
        OS_TPrintf("SSL: Unknown public key alrorithm\n");
        break;
    }

    return 0;   //result;
}

/*---------------------------------------------------------------------------*
  Name:         SslInit

  Description:  SSL 初期化.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SslInit(void)
{
#ifndef SDK_WIFI_NOSSL
    /* 乱数の追加は頻繁に行うほうが良い */
    static u32  ssl_seed[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];
    OS_GetLowEntropyData(ssl_seed);
    CPS_SslAddRandomSeed(ssl_seed, sizeof(ssl_seed));
#endif

    if (!g_psslCon)
    {
        MI_CpuClear8(&g_sslCon, sizeof(SOCSslConnection));
        g_sslCon.server_name = NULL;;

        SslSetDefaultConncetion(&g_sslCon);
        SslSetDefaultCA(ca_list, sizeof(ca_list) / sizeof(ca_list[0]));
        SslSetDefaultAuthCallback(SslAuthCallback);
    }
}

/*---------------------------------------------------------------------------*
  Name: cd         SslGetConnection

  Description:  ssl.cに登録されているSOCSslConnectionへのポインタを返す.
                InitSSL()後はデフォルトのパラメータで初期化されているので、
                そのままSOC_EnableSsl()で使用可能。

  Arguments:    None.

  Returns:      ssl.cに登録されているSOCSslConnectionへのポインタ.
                InitSSL前はNULLを返す.
 *---------------------------------------------------------------------------*/
SOCSslConnection* SslGetDefaultConnection()
{
    return g_psslCon;
}

/*---------------------------------------------------------------------------*
  Name:         SslSetConncetion

  Description:  SOCSslConnection構造体をssl.cに登録する.

  Arguments:    psslCon		登録するSOCSslConnectionへのポインタ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SslSetDefaultConncetion(SOCSslConnection* psslCon)
{
    if (psslCon)
    {
        g_psslCon = psslCon;
    }
    else
    {
        g_psslCon = &g_sslCon;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SslSetAuthCallback

  Description:  SSL認証用のコールバックを設定する.

  Arguments:    auth_callback	コールバック関数へのポインタ.

  Returns:		None.
 *---------------------------------------------------------------------------*/
void SslSetDefaultAuthCallback(int (*auth_callback) (int, struct _CPSSslConnection*, int))
{
    g_psslCon->auth_callback = auth_callback;
}

/*---------------------------------------------------------------------------*
  Name:         SslSetDefaultCA(

  Description:  SSLの証明書を設定する.

  Arguments:    ca_list		証明書リスト
                builtins.	証明書の数

  Returns:		None.
 *---------------------------------------------------------------------------*/
void SslSetDefaultCA(SOCCaInfo ** ca_list, int builtins)
{
    g_psslCon->ca_info = ca_list;
    g_psslCon->ca_builtins = builtins;
}

/*---------------------------------------------------------------------------*
  Name:         SslCreateConnection

  Description:  SSLの証明書を設定する.

  Arguments:    servername	CNとのマッチさせる接続先のサーバ名
                connection  SOCSslConnection 構造体のポインタ

  Returns:		新しく確保されたSOCSslConnection構造体へのポインタ
 *---------------------------------------------------------------------------*/
void SslCreateConnection(const char* servername, SOCSslConnection* connection)
{
    MI_CpuCopy8(g_psslCon, connection, sizeof(SOCSslConnection));
    if (servername)
    {
        connection->server_name = POKE_NET_AllocFunc(STD_StrLen(servername) + 1);
        (void)STD_CopyString(connection->server_name, servername);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SslDestroyConnection

  Description:  SslCreateConnection構造体を破棄する.

  Arguments:    con		SOCSslConnection構造体へのポインタ

  Returns:		なし
 *---------------------------------------------------------------------------*/
void SslDestroyConnection(SOCSslConnection* con)
{
    if (con->server_name)
    {
        POKE_NET_FreeFunc(con->server_name);
        con->server_name = NULL;
    }
}

void SslPrintCA(const SOCSslConnection* con)
{
    int index;
    for (index = 0; index < con->ca_builtins; ++index)
    {
        OS_TPrintf("%d: %s\n", index, g_psslCon->ca_info[index]->dn);
    }
}


#endif  // ___POKE_NET_BUILD_DS___
