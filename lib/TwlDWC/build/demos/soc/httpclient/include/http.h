/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - demos - httpclient
  File:     http.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef HTTPCLIENT_HTTP_H_
#define HTTPCLIENT_HTTP_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
  定数定義
 *---------------------------------------------------------------------------*/
#define SCHEME_MAX              64
#define HOST_NAME_MAX           256
#define HTTP_REQUEST_PATH_MAX   256

/*---------------------------------------------------------------------------*
  構造体定義
 *---------------------------------------------------------------------------*/
typedef struct HttpParsedURL
{
    char    scheme[SCHEME_MAX];
    char    host[HOST_NAME_MAX];
    char    path[HTTP_REQUEST_PATH_MAX];
    int     port;
} HttpParsedURL;

typedef struct HttpParam
{
    char*   name;
    char*   value;
} HttpParam;

/*---------------------------------------------------------------------------*
  関数プロトタイプ定義
 *---------------------------------------------------------------------------*/
int     HttpConnect(const char* url, SOCSslConnection ** sslCon);
BOOL    HttpParseURL(HttpParsedURL* parsed_url, const char* url);
int     HttpClose(int socket, SOCSslConnection* sslCon);
int     HttpGet(int socket, const char*  url, const char*  path_info, const HttpParam*  params, int param_num, char*  buf,
                int buflen);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif /* __cplusplus */
#endif /* HTTPCLIENT_HTTP_H_ */
