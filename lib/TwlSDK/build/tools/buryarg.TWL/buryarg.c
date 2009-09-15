/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - buryarg.TWL
  File:     buryarg.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <getopt.h>

#include <nitro_win32.h>

//---- error code
#define ERROR_OVER_AREA  		                0
#define ERROR_NO_FILE     		                1
#define ERROR_CANNOT_OPEN_FILE                  2
#define ERROR_CANNOT_OPEN_REP_FILE              3
#define ERROR_MARK_NOT_FOUND	                4
#define ERROR_BAD_MARK_POSITION	                5
#define ERROR_BAD_ARGUMENT_SIZE                 6
#define ERROR_UNKNOWN_OPTION                    7
#define ERROR_OUTFILE_NOT_SPECIFIED             8
#define ERROR_REPLACEMENTFILE_NOT_SPECIFIED   	9
#define ERROR_REPLACEMENTFILE_TOO_BIG           10
#define ERROR_ILLEGAL_OPTIONS                   11
#define ERROR_FILE_ERROR                        12
#define ERROR_NLF_FILE_SPECIFIED                13
#define ERROR_NOT_TWL_SRL                       14
#define ERROR_ILLEGAL_TLF_FILE					15

//---- version
#define VERSION_STRING     " 1.1  Copyright 2008 Nintendo. All right reserved."
// ver 1.1  fixed bug on --stdout option
// ver 1.0  applied for TWL

//---- ouput default added string
#define ADDED_OUTPUT_NAME  ".argAdded"

//---- output for stdout
const char gNameForStdout[] = "-";

//---- argument buffer identification string
const char gArgIdString[] = OS_ARGUMENT_ID_STRING;

//---- input bin file
#define FILE_NAME_MAX_SIZE  1024
FILE   *gInputFile;
char    gInputFileNameString[FILE_NAME_MAX_SIZE];
BOOL    gIsInputFileOpened = FALSE;

//---- output file
FILE   *gOutputFile;
char    gOutputFileNameString[FILE_NAME_MAX_SIZE];
char   *gOutputFileName = NULL;
BOOL    gIsOutputFileOpened = FALSE;

//---- replacement file
FILE   *gReplacementFile;
char   *gReplacementFileName = NULL;
BOOL    gIsReplacementFileOpened = FALSE;

//---- tlf file
FILE   *gTlfFile;
char   *gTlfFileName = NULL;
BOOL    gIsTlfFileOpened = FALSE;
char    gTlfDirectoryNameString[FILE_NAME_MAX_SIZE];

//---- argument buffer for replacement
#define ARGUMENT_BUF_SIZE 0x10000
char    gArgString[ARGUMENT_BUF_SIZE];
int     gArgStringSize;
int     gArgStringIndex = 0;

//---- mode
BOOL    gQuietMode = FALSE;
BOOL    gVerboseMode = FALSE;
BOOL    gReplacementMode = TRUE;
BOOL    gDebugMode = FALSE;
BOOL    gTlfInputMode = FALSE;
BOOL    gStdoutMode = FALSE;

//---- declaration of prototype
void    analyzeTlfFile(void);
void    closeFiles(void);

//================================================================================
//                      メッセージ表示
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         displayMessage

  Description:  display message

  Arguments:    message : message array

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayMessage(char *message[])
{
    int     n;
    //---- show help messages
    for (n = 0; message[n]; n++)
    {
        printf(message[n]);
    }
}


/*---------------------------------------------------------------------------*
  Name:         displayUsage

  Description:  display how to use.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayUsage(void)
{
    char   *usageString[] = {
        "Usage: buryarg.TWL [OPTION]... <TWL srl filename> [argument]...\n",
        "Bury argument strings to TWL srl file.",
        "\n",
        "Options:\n",
        "  --version           : Show version.\n",
        "  -h, --help          : Show this help.\n",
        "  -q, --quiet         : Quiet mode.\n",
        "  -v, --verbose       : Verbose mode.\n",
        "  -r, --remain        : Remain original file.\n",
        "  -o, --output=FILE   : Output file (default: srcfile.argAdded)\n",
        "                        if \"-\" is specified, output to stdout.\n",
        "      --stdout        : Mean '-o-' .\n",
        "  -f, --file=FILE     : Specify replacement buffer data.\n",
        "                        This is used instead of data from arguments.\n",
        "  -d, --debug         : Debug mode. Show replacement buffer.\n",
        0
    };

    displayMessage(usageString);
}

/*---------------------------------------------------------------------------*
  Name:         displayVersion

  Description:  display version

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayVersion(void)
{
    printf("buryarg %s\n", VERSION_STRING);
}

/*---------------------------------------------------------------------------*
  Name:         displayError

  Description:  display error

  Arguments:    errorNumber : error no.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayError(int errorNumber)
{
    fprintf(stderr, "Error: buryarg: ");

    switch (errorNumber)
    {
    case ERROR_OVER_AREA:
        fprintf(stderr, "arguments too long.\n");
        break;

    case ERROR_NO_FILE:
        fprintf(stderr, "cannot open binary file.\n");
        break;

    case ERROR_CANNOT_OPEN_FILE:
        fprintf(stderr, "cannot open output file.\n");
        break;

    case ERROR_CANNOT_OPEN_REP_FILE:
        fprintf(stderr, "cannot open replacement file.\n");
        break;

    case ERROR_MARK_NOT_FOUND:
        fprintf(stderr, "cannot replace because bin file has no argument id string.\n");
        break;

    case ERROR_BAD_MARK_POSITION:
        fprintf(stderr, "bad argument mark position.\n");
        break;

    case ERROR_BAD_ARGUMENT_SIZE:
        fprintf(stderr, "bad argument size.\n");
        break;

    case ERROR_UNKNOWN_OPTION:
        fprintf(stderr, "unknown option or invalid usage. try --help.\n");
        break;

    case ERROR_OUTFILE_NOT_SPECIFIED:
        fprintf(stderr, "output file is not specified.\n");
        break;

    case ERROR_REPLACEMENTFILE_NOT_SPECIFIED:
        fprintf(stderr, "replacement file is not specified.\n");
        break;

    case ERROR_REPLACEMENTFILE_TOO_BIG:
        fprintf(stderr, "replacement file is too big.\n");
        break;

    case ERROR_ILLEGAL_OPTIONS:
        fprintf(stderr, "specified illegal option set.\n");
        break;

    case ERROR_FILE_ERROR:
        fprintf(stderr, "error occurred in renaming file.\n");
        break;

	case ERROR_NLF_FILE_SPECIFIED:
		fprintf(stderr, "cannot access to nlf file.\n");
		break;

	case ERROR_NOT_TWL_SRL:
        fprintf(stderr, "ROM is not for TWL.\n");
        break;

    case ERROR_ILLEGAL_TLF_FILE:
        fprintf(stderr, "illegal tlf file.\n");
        break;

    default:
        fprintf(stderr, "unknown error (%d).\n", errorNumber);
        break;
    }

    closeFiles();
    exit(2);
}


//================================================================================
//                      デバッグ用表示
//================================================================================
//---- for debug
//     display buffer
void displayBuffer(void)
{
    const int showSize = 256;
    int     n;

    for (n = 0; n < showSize; n++)
    {
        int     k = n % 16;
        int     c;

        if (k == 0)
        {
            printf("%4x ", n);
        }

        c = (int)gArgString[n];

        if (0x20 <= c && c <= 0x7f)
        {
            printf("%c  ", gArgString[n]);
        }
        else if (c == 0)
        {
            printf(".  ");
        }
        else
        {
            printf("-  ");
        }

        if (k == 15)
        {
            printf("\n");
        }
    }
}


//================================================================================
//              ファイル名解析
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         getTailFileName

  Description:  search tail point of file name.
                ex. if d:/aaa/bbb.srl was input, output bbb.srl

  Arguments:    fileName     : file name

  Returns:      tail pointer of original string
 *---------------------------------------------------------------------------*/
const char *getTailFileName(const char *fileName)
{
    const char *p = fileName;
    const char *tailPtr = p;

    while (*p)
    {
        if (*p == '\\' || *p == '/' || *p == ':')
        {
            tailPtr = (p + 1);
        }

        p++;
    }

    return tailPtr;
}

//================================================================================
//              オプション解析 と バッファ作成
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         checkOverBuffer

  Description:  check if buffer is over.
				if error occured, never return.  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void checkOverBuffer(void)
{
    if (gArgStringIndex >= ARGUMENT_BUF_SIZE)
    {
        displayError(ERROR_OVER_AREA);
    }
}

/*---------------------------------------------------------------------------*
  Name:         addString

  Description:  add string to twl argument string area

  Arguments:    str : string to add

  Returns:      None
 *---------------------------------------------------------------------------*/
void addString(const char *str)
{
    const char *p = str;

    if (!str)
    {
        printf("internal error: tend to add NULL string.\n");
        exit(2);
    }

    while (*p)
    {
        checkOverBuffer();
        gArgString[gArgStringIndex++] = *p++;
    }

    checkOverBuffer();
    gArgString[gArgStringIndex++] = '\0';
}

/*---------------------------------------------------------------------------*
  Name:         checkTwlHeader

  Description:  check if the specified ROM is for TWL

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#define ROM_TYPE_OFS 12
void checkTwlHeader(void)
{
	const int readSize=32;
   	char romHeader[readSize];
	int  romType;   

    fseek(gTlfFile, 0, SEEK_SET);
	size_t  size = fread(romHeader, 1, readSize, gTlfFile);

	romType = (int)romHeader[ ROM_TYPE_OFS ];

	if (size != readSize || !(romType & 0x2) )
	{
        displayError(ERROR_NOT_TWL_SRL);
	}
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
    BOOL    helpFlag = FALSE;

    int     c;

    struct option optionInfo[] = {
        {"help", no_argument, NULL, 'h'},
        {"quiet", no_argument, NULL, 'q'},
        {"verbose", no_argument, NULL, 'v'},
        {"replacement", no_argument, NULL, 'r'},
        {"version", no_argument, NULL, 'a'},
        {"debug", no_argument, NULL, 'd'},
        {"output", required_argument, 0, 'o'},
        {"stdout", no_argument, NULL, 'O'},
        {"file", required_argument, 0, 'f'},
        {NULL, 0, 0, 0}
    };
    int     optionIndex;

    //---- suppress error string of getopt_long()
    opterr = 0;

    while (1)
    {
        c = getopt_long(argc, argv, "+hqvrdo:f:", &optionInfo[0], &optionIndex);

        //printf("optind=%d optopt=%d  %x(%c) \n", optind, optopt, c,c );

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 'h':
            helpFlag = TRUE;
            break;
        case 'q':
            gQuietMode = TRUE;
            break;
        case 'v':
            gVerboseMode = TRUE;
            break;
        case 'r':
            gReplacementMode = FALSE;
            break;
        case 'd':
            gDebugMode = TRUE;
            break;
        case 'a':
            displayVersion();
            exit(1);
            break;
        case 'o':
            gOutputFileName = (*optarg == '=') ? optarg + 1 : optarg;
            if (!*gOutputFileName)
            {
                displayError(ERROR_OUTFILE_NOT_SPECIFIED);
            }
            gReplacementMode = FALSE;
            break;
        case 'O': 
            gOutputFileName = (char *)gNameForStdout;
            gReplacementMode = FALSE;
            break;
        case 'f':
            gReplacementFileName = (*optarg == '=') ? optarg + 1 : optarg;
            if (!*gReplacementFileName)
            {
                displayError(ERROR_REPLACEMENTFILE_NOT_SPECIFIED);
            }
            break;
        default:
            displayError(ERROR_UNKNOWN_OPTION);
        }
    }

    //---- no specified bin file or "-h" of "--help" to display usage
    if (helpFlag || argc <= optind)
    {
        displayUsage();
        exit(1);
    }

    //---- input filename
    strncpy(gInputFileNameString, argv[optind], FILE_NAME_MAX_SIZE);

    //---- check if tlf file
    if ((gTlfFile = fopen(gInputFileNameString, "rb")) == NULL)
    {
        displayError(ERROR_NO_FILE);
    }
    gIsTlfFileOpened = TRUE;

    //---- check nlf and tlf id string
    {
        char    idString[4];
        size_t  size = fread(idString, 1, 4, gTlfFile);
        if (size == 4 && !strncmp(idString, (char *)"#NLF", 4))
        {
			displayError(ERROR_NLF_FILE_SPECIFIED);
        }

		if (size == 4 && !strncmp(idString, (char *)"#TLF", 4))
		{
            gTlfFileName = argv[optind];

            //---- determine input file name
            analyzeTlfFile();
		}
    }

	//---- check if TWL header
	if (gTlfFileName == NULL)
	{
		checkTwlHeader();
	}

    //---- close file
    fclose(gTlfFile);
    gIsTlfFileOpened = FALSE;

    //---- add string to replacement buffer
    for (n = optind; n < argc; n++)
    {
        if (n == optind)
        {
            if (!gTlfFileName)
            {
                addString(getTailFileName(argv[n]));
            }
        }
        else
        {
            addString(argv[n]);
        }
    }

    //---- end mark
    checkOverBuffer();
    gArgString[gArgStringIndex++] = '\0';
}

//================================================================================
//                      ファイル操作
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         analyzeTlfFile

  Description:  analyze tlf file.
 
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void analyzeTlfFile(void)
{
    char    lineBuffer[1024];
    BOOL    isFound_T = FALSE;
    BOOL    isFound_H = FALSE;

    fseek(gTlfFile, 0, SEEK_SET);

    //---- analyze
    while (fgets(lineBuffer, 1024, gTlfFile))
    {
        if (!strncmp(lineBuffer, "T,", 2))
        {
            char   *sp = &lineBuffer[0];
            char   *dp = &gTlfDirectoryNameString[0];

            while (*sp++ != '\"')
            {
            }
            while (*sp != '\"')
            {
                *dp++ = *sp++;
            }
            *dp = '\0';

            isFound_T = TRUE;
        }

		if ( !strncmp(lineBuffer, "H,", 2))
		{
			char *sp = &lineBuffer[0];
			char *dp = &gInputFileNameString[0];
			int count=0;

			//while (*sp && count <=2)
			while (*sp && count <=0)
			{
				if (*sp++ == '\"')
				{
					count++;
				}
			}
			while (*sp != '\"')
			{
				*dp++ = *sp++;
			}
			*dp = '\0';

			//---- set argv[0]
			addString(getTailFileName(gInputFileNameString));

			isFound_H = TRUE;
		}
    }

    //---- check if found file name
    if (!isFound_H)
    {
        displayError(ERROR_ILLEGAL_TLF_FILE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         openFiles

  Description:  open original file and output file

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void openFiles(void)
{
    //---- replacement file
    if (gReplacementFileName)
    {
        if (gVerboseMode)
        {
            printf("replacement file    : %s\n", gReplacementFileName);
        }
        if ((gReplacementFile = fopen(gReplacementFileName, "rb")) == NULL)
        {
            displayError(ERROR_CANNOT_OPEN_REP_FILE);
        }
        gIsReplacementFileOpened = TRUE;

        //---- replace buffer
        {
            int n;
            int c;

            if (gVerboseMode)
            {
                struct stat repFileStat;
                stat(gReplacementFileName, &repFileStat);
                printf("replacement file size is 0x%x byte (%d byte).\n", (int)repFileStat.st_size,
                       (int)repFileStat.st_size);
            }

            //---- clear buffer
            for (n = 0; n < ARGUMENT_BUF_SIZE; n++)
            {
                gArgString[n] = 0;
            }

            //---- get replacement strings
            n = 0;
            while (n < ARGUMENT_BUF_SIZE && (c = fgetc(gReplacementFile)) != EOF)
            {
                gArgString[n] = c;
                n++;
            }
            fclose(gReplacementFile);
            gIsReplacementFileOpened = FALSE;

            //---- check over buffer
            if (c != EOF)
            {
                //displayError( ERROR_REPLACEMENTFILE_TOO_BIG );
                if (!gQuietMode)
                {
                    printf("warning: replacement file is bigger than required size.");
                }
            }
        }
    }

    //---- 元ファイルを書き換えない
    if (!gReplacementMode)
    {
        //---- 書き出しファイル名
        if (gOutputFileName)
        {
            strncpy(gOutputFileNameString, gOutputFileName, FILE_NAME_MAX_SIZE);
        }
        else
        {
            strcat(gOutputFileNameString, gInputFileNameString);
            strncat(gOutputFileNameString, ADDED_OUTPUT_NAME, FILE_NAME_MAX_SIZE);
        }

        //---- check if stdout specified
        if (!strncmp(gOutputFileNameString, "-", 1))
        {
            gStdoutMode = TRUE;
        }

        //---- output file
        if (gVerboseMode)
        {
            if (gStdoutMode)
            {
                printf("destination         : stdout\n");
            }
            else
            {
                printf("destination bin file: %s\n", gOutputFileNameString);
            }
        }
        if (gStdoutMode)
        {
            gOutputFile = stdout;

            //---- set stdout to binary mode
            _setmode(_fileno(gOutputFile), O_BINARY);
        }
        else
        {
            if ((gOutputFile = fopen(gOutputFileNameString, "wb")) == NULL)
            {
                displayError(ERROR_CANNOT_OPEN_FILE); 
           }
            gIsOutputFileOpened = TRUE;
        }
    }

    //---- input file
    if (gTlfFileName)
    {
        if (gVerboseMode)
        {
            printf("tlf current directory is %s\n", gTlfDirectoryNameString);
        }
        //---- change current directory
        if (chdir(gTlfDirectoryNameString) != 0)
        {
            displayError(ERROR_ILLEGAL_TLF_FILE);
        }
    }


    if (gVerboseMode)
    {
        printf("original bin file   : %s\n", gInputFileNameString);
    }
    if ((gInputFile = fopen(gInputFileNameString, (gReplacementMode) ? "rb+" : "rb")) == NULL)
    {
        displayError(ERROR_NO_FILE);
    }
    gIsInputFileOpened = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         closeFiles

  Description:  close files

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void closeFiles(void)
{
    if (gStdoutMode)
    {
        //---- flush stdout
        fflush(stdout);

        //---- set stdout to text mode
        _setmode(_fileno(gOutputFile), O_TEXT);
    }

    if (gIsInputFileOpened)
    {
        fclose(gInputFile);
        gIsInputFileOpened = FALSE;
    }
    if (gIsOutputFileOpened)
    {
        fclose(gOutputFile);
        gIsOutputFileOpened = FALSE;
    }
    if (gIsReplacementFileOpened)
    {
        fclose(gReplacementFile);
        gIsReplacementFileOpened = FALSE;
    }
    if (gIsTlfFileOpened)
    {
        fclose(gTlfFile);
        gIsTlfFileOpened = FALSE;
    }
}

//================================================================================
//                      バッファ置き換え
//================================================================================
#define ROM_ARG_BUF_OFS 0x0e00

void replaceToSpecifiedString(void)
{
    long    bufPosition = ROM_ARG_BUF_OFS;
    int     n;
    int     inputChar;

    //---- replacement
    if (gReplacementMode)
    {
		gArgStringSize = OS_ARGUMENT_BUFFER_SIZE;

        fseek(gInputFile, bufPosition, SEEK_SET);
        for (n = 0; n < gArgStringSize; n++)
        {
            fputc(gArgString[n], gInputFile);
        }

        if (gVerboseMode)
        {
            printf("replaced 0x%x byte.\n", (int)gArgStringSize);
        }
    }
    else
    {
        gArgStringSize = gArgStringIndex;

        fseek(gInputFile, 0, SEEK_SET);
        n = 0;
        while ((inputChar = fgetc(gInputFile)) != EOF)
        {
            if (bufPosition <= n && n < bufPosition + gArgStringSize)
            {
                fputc(gArgString[n - bufPosition], gOutputFile);
            }
            else
            {
                fputc(inputChar, gOutputFile);
            }

            n++;
        }

        if (gVerboseMode)
        {
            printf("output 0x%x byte (%d byte).\n", n, n);
        }
    }
}

//================================================================================
//                      メインルーチン
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         main

  Description:  main proc

  Arguments:    argc : argument count
                argv : argument vector

  Returns:      ---
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    //---- オプション解析
    parseOption(argc, argv);

    //---- ファイルオープン
    openFiles();

    //---- バッファ内表示(デバッグ用)
    if (gDebugMode)
    {
        displayBuffer();
    }

    //---- 置き換え処理
    replaceToSpecifiedString();

    //---- ファイルクローズ
    closeFiles();

    //---- 終了
    if (!gQuietMode)
    {
        printf("Success.\n");
    }
    return 0;
}
