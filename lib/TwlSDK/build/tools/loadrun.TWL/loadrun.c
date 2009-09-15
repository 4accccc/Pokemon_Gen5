/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - loadrun.TWL
  File:     loadrun.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-07-15#$
  $Rev: 7370 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

#include "isd_api.h"

#define  SDK_BOOL_ALREADY_DEFINED_
#include <twl_win32.h>
#include <nitro/os/common/system.h>

extern const unsigned long SDK_DATE_OF_LATEST_FILE;

//---- アプリケーション名
#define EXEC_NAME          "loadrun.TWL"
#define DEBUGGER_NAME      "IS-TWL-DEBUGGER"


//---- バージョン文字列
#define VERSION_STRING     " 1.7  Copyright 2007-2008 Nintendo. All right reserved."
// 1.7 HYBRIDをNITROモードで動作できるようにした (-N追加)
// 1.6 最終エラーを表示できるようにした
// 1.5 デバッガ禁止フラグを参照するようにした
// 1.4 シリアル指定で末尾のみのマッチに対応した
// 1.3 ISTD_DownloadGo() 正式対応
// 1.2 printコールバックの不具合修正
// 1.1 IS-TWL-DEBUGGER Ver.0.41 への対応
// 1.0 公開

//---- exit 番号
#define EXIT_NUM_NO_ERROR               207     // 成功終了(但し、今回はこれになることはない)
#define EXIT_NUM_USER_SIGNAL            206     // ユーザによる強制終了(ctrl-C)
#define EXIT_NUM_EXEC_TIME_OUT          205     // 実行タイムアウトで強制終了
#define EXIT_NUM_TIME_OUT               204     // 表示タイムアウトで強制終了
#define EXIT_NUM_SHOW_DEVICES           203     // デバイスリスト表示で終了
#define EXIT_NUM_SHOW_USAGE             202     // ヘルプ表示で終了
#define EXIT_NUM_SHOW_VERSION           201     // バージョン表示で終了
#define EXIT_NUM_STRING_ABORT           200     // 文字列による強制終了

#define EXIT_NUM_NO_DEVICE              -1      // 使用可能なデバイスがない
#define EXIT_NUM_UNKNOWN_OPTION         -2      // 想定外のオプションが指定された
#define EXIT_NUM_ILLEGAL_OPTION         -3      // オプションの使用方法が違う
#define EXIT_NUM_NO_INPUT_FILE          -4      // 指定のファイルがないorオープンできない
#define EXIT_NUM_NOT_CONNECT            -5      // デバイスへの接続失敗
#define EXIT_NUM_CANNOT_USE_CARTRIDGE   -6      // カートリッジのロックに失敗
#define EXIT_NUM_CANNOT_USE_CARD        -7      // カードのロックに失敗
#define EXIT_NUM_PRINTF_ERROR           -8      // printfデータ扱い中のエラー
#define EXIT_NUM_LOADING_ERROR          -9      // ロード中のエラー
#define EXIT_NUM_EXEC_PROHIBITION       -10     // 実行が許されていない
#define EXIT_NUM_NO_HYBRID_NITRO        -11     // HYBRID を NITRO で実行できない

//---- 最大コンソール数
#define PRINT_CONSOLE_MAX   4

//---- ROM ヘッダ情報
#define RH_FLAG_OFFSET					0x1c	// ROMヘッダ上諸情報フラグ
#define RH_FLAG_DEBUGGER_PROHIBITION	(1<<3)	// このビットが立っているとデバッガ実行出来ない

//---- デバイス指定用
char   *gDeviceName[] = {
    "TWLEMU", "TWLDEB", NULL
};
int     gDeviceTypeArray[] = {
    ISTD_DEVICE_IS_TWL_EMULATOR,
    ISTD_DEVICE_IS_TWL_DEBUGGER,
};

//---- 動作モード
BOOL    gQuietMode = FALSE;            // quiet モード
BOOL    gVerboseMode = FALSE;          // verbose モード
BOOL    gDebugMode = FALSE;            // debug モード

BOOL    gStdinMode = FALSE;            // stdin モード

BOOL    gIsForceNitro = FALSE;         // hybrid を NITROモードとするか

BOOL    gIsTypeSpecified = FALSE;      // デバイスタイプ指定ありか？
int     gSpecifiedType;                // ある場合のデバイスタイプ

BOOL    gIsSerialSpecified = FALSE;    // シリアル番号指定ありか？
int     gSpecifiedSerial;              // ある場合のシリアル番号

BOOL    gIsCartridgeLocked = FALSE;    // カートリッジスロットをロックするか
BOOL    gIsCardLocked = FALSE;         // カードスロットをロックするか

int     gTimeOutTime = 0;              // タイムアウト時間(秒) (0 はなし)
BOOL    gTimeOutOccured = FALSE;       // タイムアウトが発生したか

int     gExecTimeOutTime = 0;          // 実行タイムアウト時間(秒) (0 はなし)
int     gExecTimeOutOccured = FALSE;   // 実行タイムアウトが発生したか

char   *gAbortString = NULL;           // 強制終了文字列
BOOL    gStringAborted = FALSE;        // 強制終了文字列によって終わったかどうか

BOOL    gExitAborted = FALSE;          // OS_Exit() 終了
int     gExitStatusNum = EXIT_NUM_STRING_ABORT; // OS_Exit()によって終わったときの返り値
int     gExitStrLength;                // 終了文字列のサイズ

BOOL    gShowArchitecture = FALSE;      // アーキテクチャ表示するか
BOOL    gShowConsoleNum = FALSE;        // コンソール番号表示するか


//---- TWLライブラリ用
HINSTANCE gDllInstance;
ISDDEVICEHANDLE gDeviceHandle;
ISDDEVICEID gDeviceId;

//---- デバイスに接続したか(スロットのスイッチ用)
BOOL    gDeviceConnected = FALSE;


//---- デバイスリスト
#define DEVICE_MAX_NUM      256
#define DEVICE_SERIAL_NONE  0x7fffffff // means no specified
ISTDDevice gDeviceList[DEVICE_MAX_NUM];
int     gCurrentDevice = -1;
int     gConnectedDeviceNum = 0;

int     gDeviceTypeSpecified = ISTD_DEVICE_NONE;
int     gDeviceSerialSpecified = DEVICE_SERIAL_NONE;    // means no specified
#define DEVICE_SERIAL_STRING_MAX_SIZE 256
char	gDeviceSerialString[DEVICE_SERIAL_STRING_MAX_SIZE];
int     gDeviceSerialStringLen = 0;

//---- 入力ファイル
#define FILE_NAME_MAX_SIZE  1024
FILE   *gInputFile;
char    gInputFileNameString[FILE_NAME_MAX_SIZE];
BOOL    gIsInputFileOpened = FALSE;

//---- 時間
time_t  gStartTime = 0;                // スタート時刻
BOOL    gIsLineHead[PRINT_CONSOLE_MAX] = {TRUE, TRUE, TRUE, TRUE}; // 行頭かどうか
BOOL    gShowLapTime = FALSE;

//---- シグナル
BOOL    gStoppedByUser = FALSE;        // ユーザに止められたかどうか

//---- 表示したかどうか
volatile BOOL    gIsOutputString = FALSE;


#define printfIfNotQuiet(str)    do{if(!gQuietMode){fputs(str,stdout);}}while(0)

void    displayErrorAndExit(int exitNum, char *message);
BOOL    outputString(TWLArch arch, int console, char *buf, int bufSize);

static void printCallback( LPVOID user, ISDCPUARCH arch, DWORD console, const void* recvBuf, DWORD size );
void checkFileFlag(void);

/*---------------------------------------------------------------------------*
  Name:         myExit

  Description:  similar to exit()

  Arguments:    exitNum : exit()番号

  Returns:      None
 *---------------------------------------------------------------------------*/
void myExit(int exitNum)
{
    //---- カートリッジとカードのスロットをオフに
    if (gDeviceConnected)
    {
//        (void)ISNTD_CartridgeSlotPower(gDeviceHandle, FALSE);
//        (void)ISNTD_CardSlotPower(gDeviceHandle, FALSE);
    }

    //---- DLL領域解放
//    ISNTD_FreeDll();
    ISTD_FreeDll();

    if (!gQuietMode)
    {
        if (exitNum == EXIT_NUM_USER_SIGNAL)
        {
            printf("\n*** %s: stopped by user.\n", EXEC_NAME);
        }
        else if (exitNum == EXIT_NUM_TIME_OUT)
        {
            printf("\n*** %s: stopped by print timeout.\n", EXEC_NAME);
        }
        else if (exitNum == EXIT_NUM_EXEC_TIME_OUT)
        {
            printf("\n*** %s: stopped by exec timeout.\n", EXEC_NAME);
        }
        else if (gStringAborted)
        {
            printf("\n*** %s: stopped by aborting string.\n", EXEC_NAME);
        }
        else if (gExitAborted)
        {
            exitNum = gExitStatusNum;
        }
    }

    exit(exitNum);
}

/*---------------------------------------------------------------------------*
  Name:         listDevice

  Description:  デバイスの一覧表示して終わる

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void listDevice(void)
{
    int     n;

    //---- デバイス読み込み
    gConnectedDeviceNum = ISTD_GetDeviceList(&gDeviceList[0], DEVICE_MAX_NUM);
    if (gConnectedDeviceNum < 0)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "Cannot access devices.");
    }

    printf("---- Connected devices:\n");

    for (n = 0; n < gConnectedDeviceNum; n++)
    {
        switch (gDeviceList[n].type)
        {
			case ISTD_DEVICE_IS_TWL_EMULATOR:
				printf("%3d: [IS-TWL-DEBUGGER] serial:%8d\n", n, gDeviceList[n].serial);
				break;
			case ISTD_DEVICE_IS_TWL_DEBUGGER:
				printf("%3d: [%s] serial:%8d\n", n, DEBUGGER_NAME, gDeviceList[n].serial);
				break;
			case ISTD_DEVICE_UNKNOWN:
				printf("%3d: unknown device %x:%x\n", n, (int)gDeviceList[n].id.eType, (int)gDeviceList[n].id.nSerial );
				break;
			default:
				printf("Illegal device\n");
				break;
        }
    }

    //---- 検索結果
    printf("Found %d device%s.\n", gConnectedDeviceNum, (gConnectedDeviceNum<=1)?"":"s" );

    myExit(EXIT_NUM_SHOW_DEVICES);
}

/*---------------------------------------------------------------------------*
  Name:         searchDevice

  Description:  search device

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void searchDevice(void)
{
	int		tmpMatch = -1;
	int		tmpMatchCount = 0;

    //---- デバイスが無かった場合
    if (gConnectedDeviceNum <= 0)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "found no device.");
    }

	//---- 何も指定してないなら最初のもの
    if (gDeviceTypeSpecified == ISTD_DEVICE_NONE && gDeviceSerialSpecified == DEVICE_SERIAL_NONE)
	{
        gCurrentDevice = 0;
	}
	//---- デバイス指定か、シリアル指定がある
	else
	{
        int     n;
		char    tmpStr[DEVICE_SERIAL_STRING_MAX_SIZE];
		char    tmpStrLen;

        gCurrentDevice = -1;
        for (n = 0; n < gConnectedDeviceNum; n++)
        {
            //---- 指定デバイスとのマッチ判定
            if (gDeviceTypeSpecified != ISTD_DEVICE_NONE && gDeviceTypeSpecified != gDeviceList[n].type)
            {
                continue;
            }

            //---- 指定シリアルとのマッチ判定
            if (gDeviceSerialSpecified != DEVICE_SERIAL_NONE )
			{
				//---- 等しい
				if ( gDeviceSerialSpecified == gDeviceList[n].serial )
				{
					gCurrentDevice = n;
					break;
				}

				//---- 等しくないので部分マッチ判定
				sprintf( tmpStr, "%d", gDeviceList[n].serial );
				tmpStrLen = strlen(tmpStr);

				if ( ( tmpStrLen > gDeviceSerialStringLen ) && 
					 ( ! strcmp( &tmpStr[ tmpStrLen - gDeviceSerialStringLen ], gDeviceSerialString ) ) )
				{
					tmpMatch = n;
					tmpMatchCount ++;
				}
            }
        }
    }

	//---- 部分マッチしたものが一つあれば
	if ( gCurrentDevice < 0 )
	{
		if ( tmpMatchCount == 1 )
		{
			gCurrentDevice = tmpMatch;
		}
		else if ( tmpMatchCount >= 2 )
		{
			displayErrorAndExit(EXIT_NUM_NO_DEVICE, "two or more devices matched to sperified serial number." );
		}
	}

    //---- 指定したものがない、または間違っている
    if (gCurrentDevice < 0
        || gDeviceList[gCurrentDevice].type == ISTD_DEVICE_NONE
        || gDeviceList[gCurrentDevice].type == ISTD_DEVICE_UNKNOWN)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "illegal device.");
    }
}

/*---------------------------------------------------------------------------*
  Name:         displayUsage

  Description:  使い方を表示する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayUsage(void)
{
    fprintf(stderr,
            "TWL-SDK Development Tool - %s - Execute TWL ROM image\n"
            "Build %lu\n\n"
            "Usage: %s [OPTION] <SrlFile>\n"
            "\tdownload TWL srl file to %s and execute.\n\n"
            "Options:\n"
            "  --version                   : Show %s version.\n"
            "  --debuggerVersion           : Show debugger version.\n"
            "  -h, --help                  : Show this help.\n"
            "  -q, --quiet                 : Quiet mode.\n"
            "  -v, --verbose               : Verbose mode.\n"
            "  -L, --list                  : List connecting device.\n"
            "  -N, --nitro                 : Run hybrid rom as NITRO mode\n"
            "  -l, --lap                   : Show lap time at each line.\n"
//            "  -d, --type=DEVICE           : Specify device type.\n"
//            "                                DEVICE=TWLEMU|TWLDEB.\n"
            "  -A, --architecture          : Show architecture at each line.\n"
            "  -n, --console               : Show console number at each line.\n"
            "  -s, --serial=SERIAL         : Specify serial number.\n"
            "  -t, --timeout=SECOND        : Specify quit time after last print.\n"
            "  -T, --exec-timeout=SECOND   : Specify quit time after execute program.\n"
            "  -a, --abort-string=STRING   : Specify aborting string.\n"
            "  -c, --card-slot=SWITCH      : Card      slot SWITCH=ON|OFF, default OFF.\n"
            "  -C, --cartridge-slot=SWITCH : Cartridge slot SWITCH=ON|OFF, default OFF.\n",
//            "  --stdin, --standard-input   : Read data from stdin instead of <SrlFile>.\n\n",
			EXEC_NAME,
            SDK_DATE_OF_LATEST_FILE,
			EXEC_NAME,
			DEBUGGER_NAME,
			EXEC_NAME );
}

/*---------------------------------------------------------------------------*
  Name:         displayVersion

  Description:  バージョン表示する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayVersion(void)
{
    printf("*** %s: %s\n", EXEC_NAME, VERSION_STRING);
}

/*---------------------------------------------------------------------------*
  Name:         displayDebuggerVersion

  Description:  デバッガバージョン表示する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayDebuggerVersion(void)
{
	BOOL r;
	DWORD major;
	DWORD minor;
	DWORD build;
	DWORD revision;
	r = ISTD_GetVersion( &major, &minor, &build, &revision);

	if (r)
	{
		printf("*** %s: %s version is %d.%d.%04d.%04d\n",
			   EXEC_NAME, DEBUGGER_NAME, (int)major, (int)minor, (int)build, (int)revision);
	}
	else
	{
		printf("*** %s: failed to get %s version.\n", EXEC_NAME, DEBUGGER_NAME );
	}
	myExit(EXIT_NUM_NO_ERROR);
}

/*---------------------------------------------------------------------------*
  Name:         displayErrorAndExit

  Description:  エラーを表示する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayErrorAndExit(int exitNum, char *message)
{
	if (gDebugMode)
	{
		printf("last error=%lx\n", ISTD_GetLastError());
	}

    printf("*** %s: Error: %s\n", EXEC_NAME, message);

    //---- DLL領域解放
    ISTD_FreeDll();

    exit(exitNum);
}

/*---------------------------------------------------------------------------*
  Name:         parseOption

  Description:  parse option line

  Arguments:    argc : argument count
                argv : argument vector

  Returns:      result. less than 0 if error.
 *---------------------------------------------------------------------------*/
void parseOption(int argc, char *argv[])
{
    int     n;
    int     c;
    BOOL    helpFlag = FALSE;

    struct option optionInfo[] = {
        {"help", no_argument, NULL, 'h'},
        {"quiet", no_argument, NULL, 'q'},
        {"verbose", no_argument, NULL, 'v'},
        {"list", no_argument, NULL, 'L'},
        {"nitro", no_argument, NULL, 'N'},
        {"lap", no_argument, NULL, 'l'},
        {"debug", no_argument, NULL, 'D'},      //隠しオプション
        {"version", no_argument, NULL, '1'},
        {"debuggerVersion", no_argument, NULL, '2'},
//        {"stdin", no_argument, NULL, 'I'},
//        {"standard-input", no_argument, NULL, 'I'},
        {"type", required_argument, 0, 'd'},    //隠しオプション
        {"serial", required_argument, 0, 's'},
        {"timeout", required_argument, 0, 't'},
        {"exec-timeout", required_argument, 0, 'T'},
        {"abort-string", required_argument, 0, 'a'},
        {"card-slot", required_argument, 0, 'c'},
        {"cartridge-slot", required_argument, 0, 'C'},
        {"architecture", no_argument, NULL, 'A'},
        {"console", no_argument, NULL, 'n'},
        {NULL, 0, 0, 0}
    };
    int     optionIndex;

    char   *optionStr = NULL;

    //---- suppress error string of getopt_long()
    opterr = 0;

    while (1)
    {
        c = getopt_long(argc, argv, "+hqvLNlDd:s:t:T:a:c:C:An", &optionInfo[0], &optionIndex);

        //printf("optind=%d optopt=%d  %x(%c) \n", optind, optopt, c,c );

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 'I':                     //---- 標準入力
            gStdinMode = TRUE;
            break;
        case 'h':                     //---- ヘルプ表示
            helpFlag = TRUE;
            break;
        case 'q':                     //---- quietモード
            gQuietMode = TRUE;
            break;
        case 'v':                     //---- verboseモード
            gVerboseMode = TRUE;
            break;
        case 'D':                     //---- debugモード
            gDebugMode = TRUE;
            break;
        case '1':                     //---- プログラムバージョン表示
            displayVersion();
            myExit(EXIT_NUM_SHOW_VERSION);
            break;
        case '2':                     //---- デバッガバージョン表示
			displayDebuggerVersion();
            myExit(EXIT_NUM_SHOW_VERSION);
			break;
        case 'L':                     //---- デバイスの一覧
            listDevice();
            break;
        case 'N':                     //---- hybrid を NITROモードとする
            gIsForceNitro = TRUE;
            break;
        case 'l':                     //---- ラップタイム
            gShowLapTime = TRUE;
            break;
		case 'A':
			gShowArchitecture = TRUE; // アーキテクチャ表示
			break;
		case 'n':
			gShowConsoleNum = TRUE;   // コンソール番号表示
			break;
        case 'd':                     //---- デバイス
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            {
                int     n;
                for (n = 0; gDeviceName[n]; n++)
                {
                    if (!strcmp(optionStr, gDeviceName[n]))
                    {
                        gDeviceTypeSpecified = gDeviceTypeArray[n];
                        break;
                    }
                }

                if (gDeviceTypeSpecified == ISTD_DEVICE_NONE)
                {
                    displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION, "illegal device type.");
                }
            }
            break;
        case 's':                     //---- シリアル指定
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            gDeviceSerialSpecified = atoi(optionStr);
			strncpy(gDeviceSerialString, optionStr, DEVICE_SERIAL_STRING_MAX_SIZE);
			gDeviceSerialString[DEVICE_SERIAL_STRING_MAX_SIZE-1] = '\0';
			gDeviceSerialStringLen = strlen( gDeviceSerialString );
            break;
        case 'c':                     //---- カードスロットロック
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            if (!strcmp(optionStr, "ON") || !strcmp(optionStr, "on"))
            {
                gIsCardLocked = TRUE;
            }
            else if (!strcmp(optionStr, "OFF") || !strcmp(optionStr, "off"))
            {
                gIsCardLocked = FALSE;
            }
            else
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION, "illegal value for card slot option.");
            }
            break;
        case 'C':                     //---- カートリッジスロットロック
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            if (!strcmp(optionStr, "ON") || !strcmp(optionStr, "on"))
            {
                gIsCartridgeLocked = TRUE;
            }
            else if (!strcmp(optionStr, "OFF") || !strcmp(optionStr, "off"))
            {
                gIsCartridgeLocked = FALSE;
            }
            else
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                    "illegal value for cartridge slot option.");
            }
            break;
        case 't':                     //---- 最終表示からのタイムアウト時間
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            gTimeOutTime = atoi(optionStr);
            if (gTimeOutTime <= 0)
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                    "illegal value for abort timeout option.");
            }
            break;
        case 'T':                     //---- 実行タイムアウト時間
            optionStr = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            gExecTimeOutTime = atoi(optionStr);
            if (gExecTimeOutTime <= 0)
            {
                displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                    "illegal value for abort exec timeout option.");
            }
            break;
        case 'a':                     //---- 終了文字列
            gAbortString = (char *)(optarg + ((*optarg == '=') ? 1 : 0));
            {
                int     length = strlen(gAbortString);
                if (length <= 0 || length > 256)
                {
                    displayErrorAndExit(EXIT_NUM_ILLEGAL_OPTION,
                                        "illegal value for abort string option.");
                }
            }
            break;
        default:
            displayErrorAndExit(EXIT_NUM_UNKNOWN_OPTION, "unknown option.");
        }
    }

    //---- ヘルプ表示
    {
        BOOL    isDisplayHelp = FALSE;

        if (helpFlag)
        {
            isDisplayHelp = TRUE;
        }
        else if (argc <= optind && !gStdinMode)
        {
            isDisplayHelp = TRUE;
        }
        else if (argc > optind && gStdinMode)
        {
            isDisplayHelp = TRUE;
        }

        if (isDisplayHelp)
        {
            displayUsage();
            exit(EXIT_NUM_SHOW_USAGE);
        }
    }

    //---- 入力ファイル名
    if (!gStdinMode)
    {
        strncpy(gInputFileNameString, argv[optind], FILE_NAME_MAX_SIZE);
    }

    if (gVerboseMode)
    {
        if (!gStdinMode)
        {
            printf("Input file is [%s]\n", gInputFileNameString);
        }
        else
        {
            printf("Input file is stdin\n");
        }
    }

    //---- 情報表示
    if (gVerboseMode)
    {
        printf("Print time out : %d sec.\n", gTimeOutTime);
        printf("Execute time out : %d sec.\n", gExecTimeOutTime);
        printf("Card lock : %s.\n", (gIsCardLocked) ? "ON" : "OFF");
        printf("Cartridge lock : %s.\n", (gIsCartridgeLocked) ? "ON" : "OFF");

        if (gAbortString)
        {
            printf("Abort string : [%s]\n", gAbortString);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         loadFile

  Description:  load file

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void loadFile(void)
{
    unsigned int address = 0;

    //---- ファイルオープン
    if (gStdinMode)
    {
        gInputFile = stdin;
        _setmode(_fileno(gInputFile), O_BINARY);
    }
    else
    {
        if ((gInputFile = fopen(gInputFileNameString, "rb")) == NULL)
        {
            displayErrorAndExit(EXIT_NUM_NO_INPUT_FILE, "cannot open input file.");
        }
    }
    gIsInputFileOpened = TRUE;

	//---- ファイルチェック
	checkFileFlag();

    //---- デバイスへの接続
    if ((gDeviceHandle = ISTD_OpenEx(gDeviceList[gCurrentDevice].id)) == NULL)
    {
        displayErrorAndExit(EXIT_NUM_NOT_CONNECT, "cannot connect device.");
    }
    gDeviceConnected = TRUE;

    //---- リセットの発行
    ISTD_Reset(gDeviceHandle, TRUE);
    Sleep(1000);

	//---- プリントコールバックの設定
	ISTD_PrintfSetCB( gDeviceHandle, printCallback, (void*)0 );

	//---- ハイブリッドの際のモード設定
	if ( ISTD_SetHybridMode )
	{
		ISTD_SetHybridMode( gDeviceHandle, gIsForceNitro? ISDHYBRID_NTR: ISDHYBRID_TWL );
	}
	else
	{
		if ( gIsForceNitro )
		{
			displayErrorAndExit(EXIT_NUM_LOADING_ERROR, "cannot run as NITRO mode. The required version of IS-TWL-DEBUGGER is v0.64 or later.");
		}
	}

	//---- ROMデータダウンロード＆起動
	if ( ! ISTD_DownloadGo( gDeviceHandle, gInputFileNameString ) )
	{
		displayErrorAndExit(EXIT_NUM_LOADING_ERROR, "troubled while loading input file.");
	}

#if 0
    //---- 16KBずつ転送する
    while (1)
    {
        char    buf[16384];
        size_t  size = fread(buf, 1, sizeof(buf), gInputFile);
        static int progressCount = 0;

        if (!size)
        {
            break;
        }

        //---- 転送
        if (!ISTD_WriteROM(gDeviceHandle, buf, address, size))
        {
            displayErrorAndExit(EXIT_NUM_LOADING_ERROR, "troubled while loading input file.");
        }

        address += size;

        if (gVerboseMode)
        {
            if (!(progressCount++ % 32))
            {
                printf("*");
            }
        }
    }
#endif

    //---- ファイルクローズ
    if (gStdinMode)
    {
        _setmode(_fileno(gInputFile), O_TEXT);
    }
    else
    {
        fclose(gInputFile);
    }
    gIsInputFileOpened = FALSE;

    if (gVerboseMode)
    {
        printf("\nInput file size: %d (0x%x) byte\n", address, address);
    }
}

/*---------------------------------------------------------------------------*
  Name:         checkFileFlag

  Description:  ROMヘッダの実行フラグのチェック

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void checkFileFlag(void)
{
	char buf[0x100];

	fseek( gInputFile, 0, SEEK_SET );
	fread( buf, 1, sizeof(buf), gInputFile );

	//---- デバッガ実行可能かどうか
	if ( buf[RH_FLAG_OFFSET] & RH_FLAG_DEBUGGER_PROHIBITION )
	{
		displayErrorAndExit(EXIT_NUM_EXEC_PROHIBITION, "cannot exec this file. (debugger forbidden)");
	}

	//---- rewind
	rewind( gInputFile );
}

/*---------------------------------------------------------------------------*
  Name:         setSlopPower

  Description:  スロットのパワー処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void setSlotPower(void)
{
    //---- カートリッジスロットをロックする
    if (gIsCartridgeLocked)
    {
//        if (!ISNTD_CartridgeSlotPower(gDeviceHandle, TRUE))
        if (0)
        {
            displayErrorAndExit(EXIT_NUM_CANNOT_USE_CARTRIDGE, "cannot use cartridge slot.");
        }
    }

    //---- カードスロットをロックする
    if (gIsCardLocked)
    {
//        if (!ISNTD_CardSlotPower(gDeviceHandle, TRUE))
        if (0)
        {
            displayErrorAndExit(EXIT_NUM_CANNOT_USE_CARD, "cannot use card slot.");
        }
    }

#if 0
    //---- リセットの解除
    Sleep(1000);
    ISTD_Reset(gDeviceHandle, FALSE);
#endif
}


/*---------------------------------------------------------------------------*
  Name:         procPrint

  Description:  printf処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
TWLArch archNum[] = {
    TWLArchARM9,
    TWLArchARM7
};

#define PRINT_ONETIME_SIZE  512
#define PRINT_CONSOLE_MAX   4

//---- 結合して表示するためのバッファ
static char gConbineBuf[PRINT_CONSOLE_MAX][PRINT_ONETIME_SIZE * 2 + 2] = {"\0", "\0", "\0", "\0" };

static char *gConbineBufPtr[PRINT_CONSOLE_MAX] = {&gConbineBuf[0][0], 
												  &gConbineBuf[1][0], 
												  &gConbineBuf[2][0], 
												  &gConbineBuf[3][0] };

//---- 結合して文字列を比較するためのバッファ
static char gLineBuf[PRINT_ONETIME_SIZE + 1];


void procPrintf(void)
{
    int     blankTime = 0;

    //---- 終了文字列のサイズ
    gExitStrLength = strlen(OS_EXIT_STRING_1);

    while (1)
    {
        //---- ユーザに止められた場合
        if (gStoppedByUser)
        {
            myExit(EXIT_NUM_USER_SIGNAL);
        }

        //---- 終了するか
        if (gStringAborted || gExitAborted)
        {
            break;
        }

        //---- 文字表示していない場合
        if (!gIsOutputString)
        {
            Sleep(100);
            blankTime += 100;

            //---- タイムアウト判定
            if (gTimeOutTime && blankTime > gTimeOutTime * 1000)
            {
                gTimeOutOccured = TRUE;
                break;
            }
        }
        //---- 文字表示した場合
        else
        {
			gIsOutputString = FALSE;
            blankTime = 0;
        }

        //---- タイムアウト調査
        if (gExecTimeOutTime > 0)
        {
            time_t  currentTime;
            (void)time(&currentTime);

            if (currentTime - gStartTime >= gExecTimeOutTime)
            {
                gExecTimeOutOccured = TRUE;
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         printCallback

  Description:  プリントコールバック

  Arguments:    user    :
                arch    :
                console :
                recvBuf :
                size    :

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void printCallback( LPVOID user, ISDCPUARCH arch, DWORD console, const void* recvBuf, DWORD size )
{
//printf("%d arch:%d console:%d buf:%x size:%d\n", (int)user, (int)arch, (int)console, (int)recvBuf, (int)size );
	gIsOutputString = TRUE;
	outputString(arch, console, (char*)recvBuf, size);
}

/*---------------------------------------------------------------------------*
  Name:         showLapTime

  Description:  display lap time at line head

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void showLapTime(void)
{
    int     lap;
    time_t  currentTime;

    (void)time(&currentTime);
    lap = currentTime - gStartTime;

    printf("{%d:%02d}", lap / 60, lap % 60);
}

void showAddition(TWLArch arch, int console)
{
	static char* archStr[] = {"A9", "A7"};
	int archIndex = -1;

	if ( gIsLineHead[console] )
	{
		if ( gShowArchitecture )
		{
			//---- アーキテクチャ調査
			if ( arch == TWLArchARM9 )
			{
				archIndex=0;
			}
			else if ( arch == TWLArchARM7 )
			{
				archIndex=1;
			}
		}
		if ( !gShowConsoleNum )
		{
			console = -1;
		}

		//---- アーキテクチャとコンソール番号の表示
		if ( archIndex<0 && console<0 )
		{
			// do nothing;
		}
		else if ( archIndex<0 )
		{
			printf("<%d>", console );
		}
		else if ( console<0 )
		{
			printf("<%s>", archStr[archIndex] );
		}
		else
		{
			printf("<%s:%d>", archStr[archIndex], console );
		}

		//---- 時刻表示
		if (!gQuietMode && gShowLapTime)
		{
			showLapTime();
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         outputString

  Description:  output string to stdout

  Arguments:    arch    : architecture(TWLArchARM9, TWLArchARM7, None)
                console : console num (0-3, -1)
                buf     : buffer
                bufSize : data size in buffer

  Returns:      FALSE if to do quit.
 *---------------------------------------------------------------------------*/
BOOL outputString(TWLArch arch, int console, char *buf, int bufSize)
{
    char   *bufEnd = buf + bufSize;
    char   *p = buf;

    int     abortStrLength = gAbortString ? strlen(gAbortString) : 0;

	static int prevConsole = -1;

	if ( prevConsole >= 0 && prevConsole != console )
	{
        gIsLineHead[prevConsole] = TRUE;
		printf("\n");
	}
	prevConsole = console;

    while (p < bufEnd)
    {
        char   *crPtr = strchr(p, '\n');

        //---- \n がない
        if (!crPtr)
        {
            //----比較用に保存
            strcat(gConbineBufPtr[console], p);
            gConbineBufPtr[console] += strlen(p);

			//---- 追加情報
			showAddition( arch, console );
            gIsLineHead[console] = FALSE;

            //---- 表示
            printfIfNotQuiet(p);

            //---- バッファオーバーなら破棄(\nを入れていない出力側の責任)
            if (gConbineBufPtr[console] - &gConbineBuf[console][0] > PRINT_ONETIME_SIZE)
            {
                gConbineBufPtr[console] = &gConbineBuf[console][0];
                *gConbineBufPtr[console] = '\0';
            }

            break;
        }

        //---- \n までをコピー
        {
            int     n = crPtr - p + 1;

            //---- 比較用に結合
            strncpy(gConbineBufPtr[console], p, n);
            gConbineBufPtr[console][n] = '\0';

            //---- 表示用
            strncpy(&gLineBuf[0], p, n);
            gLineBuf[n] = '\0';
        }

		//---- 追加情報
		showAddition( arch, console );
        gIsLineHead[console] = TRUE;

        //---- 行表示
        printfIfNotQuiet(gLineBuf);

        //---- 強制終了文字列との比較
        if (gAbortString && !strncmp(gConbineBuf[console], gAbortString, abortStrLength))
        {
            gStringAborted = TRUE;
            return FALSE;
        }

        //---- OS_Exit による終了
        if (!strncmp(gConbineBuf[console], OS_EXIT_STRING_1, gExitStrLength))
        {
            gExitAborted = TRUE;
            gExitStatusNum = atoi(gConbineBuf[console] + gExitStrLength);
            return FALSE;
        }

        gConbineBufPtr[console] = &gConbineBuf[console][0];
        *gConbineBufPtr[console] = '\0';

        p = crPtr + 1;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         signalHandler

  Description:  signal handler

  Arguments:    sig
                argv : argument vector

  Returns:      ---
 *---------------------------------------------------------------------------*/
void signalHandler(int sig)
{
    gStoppedByUser = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         main

  Description:  main proc

  Arguments:    argc : argument count
                argv : argument vector

  Returns:      ---
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    //---- DLL 初期化
    ISTD_InitDll();

    //---- 標準出力のバッファリングの無効化
    setvbuf(stdout, NULL, _IONBF, 0);

    //---- オプション解析
    parseOption(argc, argv);

    //---- デバイス読み込み
    gConnectedDeviceNum = ISTD_GetDeviceList(&gDeviceList[0], DEVICE_MAX_NUM);
    if (gConnectedDeviceNum < 0)
    {
        displayErrorAndExit(EXIT_NUM_NO_DEVICE, "Cannot access devices.");
    }

    //---- デバイス検索
    searchDevice();

    //---- 読み込み
    loadFile();

    //---- シグナル設定
    (void)signal(SIGINT, signalHandler);

    //---- スロット
    setSlotPower();

    //---- スタート時刻取得
    (void)time(&gStartTime);

    //----printf処理
    procPrintf();

    //---- 終了
    if (gExitAborted)                  //---- OS_Exit() 終了
    {
        myExit(gExitStatusNum);
    }
    else if (gStringAborted)           //---- 文字列終了
    {
        myExit(EXIT_NUM_STRING_ABORT);
    }
    else if (gTimeOutOccured)          //---- タイムアウト
    {
        myExit(EXIT_NUM_TIME_OUT);
    }
    else if (gExecTimeOutOccured)
    {
        myExit(EXIT_NUM_EXEC_TIME_OUT);
    }
    else                               //---- 通常終了
    {
        myExit(EXIT_NUM_NO_ERROR);
    }
    //---- never reached here

    //---- dummy to avoid warning
    return 0;
}
