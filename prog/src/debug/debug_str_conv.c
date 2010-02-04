//======================================================================
/**
 * @file	
 * @brief	
 * @author	ariizumi
 * @data	
 *
 * モジュール名：
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

//#include "debug_str_conv_arr.cdat"
#include "debug/debug_str_conv.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//GFLibに移植されました。

#if 0
//SJISコード
#define SJIS_EOM (0x00)
#define SJIS_0 (0x30)
#define SJIS_9 (0x39)
#define SJIS_B_A (0x41)
#define SJIS_S_Z (0x7a)
#define SJIS_UL (0x5f)
#define SJIS_H_SPACE  (0x20) //' '
#define SJIS_H_DOT  (0x2e) //'.'

#define SJIS_HIRA_XA (0x829f) //'ぁ'
#define SJIS_HIRA_N  (0x82f1) //'ん'

#define SJIS_KANA_XA (0x8340) //'ァ'
#define SJIS_KANA_MI (0x837e) //'ミ'
#define SJIS_KANA_MU (0x8380) //'ム'  //SJISは7fが使えないらしい。
#define SJIS_KANA_N  (0x8393) //'ン'

#define SJIS_HYPHEN  (0x815b) //'ー'
#define SJIS_SPACE  (0x8140) //'　'

//STRCODE半角
#define UTF16H_0  (0x0030)
#define UTF16H_9  (0x0039)
#define UTF16H_B_A  (0x0041)
#define UTF16H_S_Z  (0x007a)
#define UTF16H_UL (0x005f)
#define UTF16H_SPACE (0x0020)
#define UTF16H_DOT (0x002e)

//STRCODE全角
#define UTF16_HIRA_XA (0x3041) //'ぁ'
#define UTF16_HIRA_N  (0x3093) //'ん'

#define UTF16_KANA_XA (0x30a1) //'ァ'
#define UTF16_KANA_MI (0x30df) //'ミ'
#define UTF16_KANA_MU (0x30e0) //'ム'
#define UTF16_KANA_N  (0x30f3) //'ン'

#define UTF16_HYPHEN  (0x30fc) //'ー'
#define UTF16_SPACE   (0x8140) //'　'

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
const BOOL DEB_STR_CONV_SjisToStrcode_OneChara( const char* str1 , STRCODE *str2 , BOOL *isFinish );
const BOOL DEB_STR_CONV_StrcodeToSjis_OneChara( const STRCODE *str1 , char* str2 , BOOL *isFinish );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void DEB_STR_CONV_SjisToStrcode( const char* str1 , STRCODE *str2 , const u32 len )
{
  int i=0;
  int idx = 0;
  int j;
/*
  for( j=0;j<DEB_STR_CONV_ARR_SIZE;j++ )
  {
    OS_TPrintf("[%s]%2x:%2x %4x\n",codeArr[j].jisCode,(u8)codeArr[j].jisCode[0],(u8)codeArr[j].jisCode[1],codeArr[j].uniCode);
  }
  for( j=0;j<DEB_STR_CONV_BYTE_ARR_SIZE;j++ )
  {
    OS_TPrintf("[%s]%2x %4x\n",codeArrByte[j].jisCode,(u8)codeArrByte[j].jisCode[0],codeArrByte[j].uniCode);
  }
*/
  //OS_TPrintf("[%s]\n",str1);
#if 0
  while( i < len )
  {
    BOOL isByte = FALSE;
    BOOL isConv = FALSE;
    if( (u8)str1[i] == 0 )
    {
      //OS_TPrintf("End\n");
      str2[idx] = GFL_STR_GetEOMCode();
      break;
    }
    if( STD_IsSjisCharacter( &str1[i] ) == TRUE )
    {
      //char temp[3]; 
      //temp[0] = str1[i];
      //temp[1] = str1[i+1];
      //temp[2] = '\0';
      //OS_TPrintf("%s:",temp);
      for( j=0;j<DEB_STR_CONV_ARR_SIZE;j++ )
      {
        if( str1[i  ] == codeArr[j].jisCode[0] &&
            str1[i+1] == codeArr[j].jisCode[1] )
        {
          //OS_TPrintf("(%s)",codeArr[j].jisCode);
          str2[idx] = codeArr[j].uniCode;
          i+=2;
          isConv = TRUE;
          break;
        }
      }
    }
    else
    {
      isByte = TRUE;
      //OS_TPrintf("%c:",str1[i]);
      for( j=0;j<DEB_STR_CONV_BYTE_ARR_SIZE;j++ )
      {
        if( str1[i] == codeArrByte[j].jisCode[0] )
        {
          str2[idx] = codeArrByte[j].uniCode;
          isConv = TRUE;
          i+=1;
          break;
        }
      }
    }
    if( isConv == FALSE )
    {
      str2[idx] = L'？';
      if( isByte == TRUE )
      {
        i+=1;
      }
      else
      {
        i+=2;
      }
    }
    idx++;
  }
#else
  while( i < len )
  {
    BOOL isFinish = FALSE;
    const BOOL isDouble = DEB_STR_CONV_SjisToStrcode_OneChara( &str1[i] , &str2[idx] , &isFinish );
    if( isDouble == TRUE )
    {
      i+=2;
      idx++;
    }
    else
    {
      i+=1;
      idx++;
    }
    if( isFinish == TRUE )
    {
      break;
    }
  }
#endif
}

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
const BOOL DEB_STR_CONV_SjisToStrcode_OneChara( const char* str1 , STRCODE *str2 , BOOL *isFinish )
{
  if( STD_IsSjisCharacter( &str1[0] ) == TRUE )
  {
    //全角
    const u16 tempStr = ((u8)str1[0]<<8) + (u8)str1[1];
    //OS_TPrintf("[%x]\n",tempStr);
    if( tempStr >= SJIS_HIRA_XA &&
        tempStr <= SJIS_HIRA_N  )
    {
      str2[0] = UTF16_HIRA_XA + (tempStr-SJIS_HIRA_XA);
    }
    else
    if( tempStr >= SJIS_KANA_XA &&
        tempStr <= SJIS_KANA_MI  )
    {
      str2[0] = UTF16_KANA_XA + (tempStr-SJIS_KANA_XA);
    }
    else
    if( tempStr >= SJIS_KANA_MU &&
        tempStr <= SJIS_KANA_N  )
    {
      str2[0] = UTF16_KANA_MU + (tempStr-SJIS_KANA_MU);
    }
    else
    if( tempStr == SJIS_HYPHEN  )
    {
      str2[0] = UTF16_HYPHEN;
    }
    else
    if( tempStr == SJIS_SPACE  )
    {
      str2[0] = UTF16_SPACE;
    }
    else
    {
      str2[0] = L'?';
    }
    return TRUE;
  }
  else
  {
    //OS_TPrintf("[%x]\n",(u8)str1[0]);
    //半角
    if( str1[0] == SJIS_EOM )
    {
      str2[0] = GFL_STR_GetEOMCode();
      if( isFinish != NULL )
      {
        *isFinish = TRUE;
      }
    }
    else  //数字
    if( str1[0] >= SJIS_0 &&
        str1[0] <= SJIS_9 )
    {
      str2[0] = UTF16H_0 + (str1[0]-SJIS_0);
    }
    else  //アルファベット
    if( str1[0] >= SJIS_B_A &&
        str1[0] <= SJIS_S_Z )
    {
      str2[0] = UTF16H_B_A + (str1[0]-SJIS_B_A);
    }
    else  //'_'
    if( str1[0] == SJIS_UL )
    {
      str2[0] = UTF16H_UL;
    }
    else
    if( str1[0] == SJIS_H_SPACE )
    {
      str2[0] = UTF16H_SPACE;
    }
    else
    if( str1[0] == SJIS_H_DOT  )
    {
      str2[0] = UTF16H_DOT;
    }
    else
    {
      str2[0] = L'?';
    }
    return FALSE;
  }
}


//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void DEB_STR_CONV_StrcodeToSjis( const STRCODE *str1 , char *str2 , const u32 len )
{
  int i=0;
  int idx = 0;
  int j;
  /*
  const STRCODE eomCode = GFL_STR_GetEOMCode();
  
  while( i < len )
  {
    BOOL isConv = FALSE;
    if( str1[i] == eomCode )
    {
      str2[idx] = '\0';
      break;
    }

    for( j=0;j<DEB_STR_CONV_ARR_SIZE;j++ )
    {
      if( str1[i] == codeArr[j].uniCode )
      {
        str2[idx  ] = codeArr[j].jisCode[0];
        str2[idx+1] = codeArr[j].jisCode[1];
        i++;
        idx += 2;
        isConv = TRUE;
        break;
      }
    }
    if( isConv == FALSE )
    {
      for( j=0;j<DEB_STR_CONV_BYTE_ARR_SIZE;j++ )
      {
        if( str1[i] == codeArrByte[j].uniCode )
        {
          str2[idx] = codeArrByte[j].jisCode[0];
          isConv = TRUE;
          idx += 1;
          i+=1;
          break;
        }
      }
    }
    if( isConv == FALSE )
    {
      str2[idx] = '?';
      idx++;
      i+=1;
    }
  }
  */
  while( i < len )
  {
    BOOL isFinish = FALSE;
    const BOOL isDouble = DEB_STR_CONV_StrcodeToSjis_OneChara( &str1[i] , &str2[idx] , &isFinish );
    if( isDouble == TRUE )
    {
      i++;
      idx+=2;
    }
    else
    {
      i++;
      idx+=1;
    }
    if( isFinish == TRUE )
    {
      break;
    }
  }  
  
}

void DEB_STR_CONV_StrBufferToSjis( const STRBUF *str1 , char *str2 , const u32 len )
{
  DEB_STR_CONV_StrcodeToSjis( GFL_STR_GetStringCodePointer(str1) , str2 , len );
}

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
const BOOL DEB_STR_CONV_StrcodeToSjis_OneChara( const STRCODE *str1 , char* str2 , BOOL *isFinish )
{
  BOOL isDouble = FALSE;
  if( str1[0] == GFL_STR_GetEOMCode() )
  {
    str2[0] = '\0';
    if( isFinish != NULL )
    {
      *isFinish = TRUE;
    }
  }
  else  //ここから半角
  if( str1[0] >= UTF16H_0 &&
      str1[0] <= UTF16H_9 )
  {
    str2[0] = SJIS_0 + (str1[0]-UTF16H_0);
  }
  else
  if( str1[0] >= UTF16H_B_A &&
      str1[0] <= UTF16H_S_Z )
  {
    str2[0] = SJIS_B_A + (str1[0]-UTF16H_B_A);
  }
  else
  if( str1[0] == UTF16H_UL )
  {
    str2[0] = SJIS_UL;
  }
  else
  if( str1[0] == UTF16H_SPACE )
  {
    str2[0] = SJIS_H_SPACE;
  }
  else
  if( str1[0] == UTF16H_DOT )
  {
    str2[0] = SJIS_H_DOT;
  }
  else  //ココから全角
  if( str1[0] >= UTF16_HIRA_XA &&
      str1[0] <= UTF16_HIRA_N )
  {
    const u16 codeTemp = SJIS_HIRA_XA + (str1[0]-UTF16_HIRA_XA);
    str2[0] = (codeTemp&0xFF00)>>8;
    str2[1] = (codeTemp&0x00FF);
    isDouble = TRUE;
  }
  else
  if( str1[0] >= UTF16_KANA_XA &&
      str1[0] <= UTF16_KANA_MI )
  {
    const u16 codeTemp = SJIS_KANA_XA + (str1[0]-UTF16_KANA_XA);
    str2[0] = (codeTemp&0xFF00)>>8;
    str2[1] = (codeTemp&0x00FF);
    isDouble = TRUE;
  }
  else
  if( str1[0] >= UTF16_KANA_MU &&
      str1[0] <= UTF16_KANA_N )
  {
    const u16 codeTemp = SJIS_KANA_MU + (str1[0]-UTF16_KANA_MU);
    str2[0] = (codeTemp&0xFF00)>>8;
    str2[1] = (codeTemp&0x00FF);
    isDouble = TRUE;
  }
  else
  if( str1[0] == UTF16_HYPHEN )
  {
    const u16 codeTemp = SJIS_HYPHEN;
    str2[0] = (codeTemp&0xFF00)>>8;
    str2[1] = (codeTemp&0x00FF);
    isDouble = TRUE;
  }
  else
  if( str1[0] == UTF16_SPACE )
  {
    const u16 codeTemp = SJIS_SPACE;
    str2[0] = (codeTemp&0xFF00)>>8;
    str2[1] = (codeTemp&0x00FF);
    isDouble = TRUE;
  }
  else
  {
    str2[0] = '?';
  }
  
  return isDouble;
}
#endif