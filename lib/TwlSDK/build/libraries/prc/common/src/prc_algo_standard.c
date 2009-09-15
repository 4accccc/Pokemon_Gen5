/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - 
  File:     prc_algo_standard.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/prc/algo_standard.h>

/*===========================================================================*
  Prototype Declarations
 *===========================================================================*/
static inline int CityBlockDistance(const PRCPoint *p1, const PRCPoint *p2)
{
    int     x = p1->x - p2->x;
    int     y = p1->y - p2->y;
    if (x < 0)
        x = -x;
    if (y < 0)
        y = -y;
    return (x + y);
}

static inline void GetMiddlePoint(PRCPoint *p, const PRCPoint *p1, const PRCPoint *p2)
{
    p->x = (s16)((p1->x + p2->x) / 2);
    p->y = (s16)((p1->y + p2->y) / 2);
}

/*===========================================================================*
  Variable Definitions
 *===========================================================================*/

/*===========================================================================*
  Functions
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         PRC_GetRecognitionBufferSizeEx_Standard

  Description:  認識アルゴリズムが必要とするワーク領域の大きさを
                計算します.

  Arguments:    maxPointCount   入力点数の上限(pen up marker を含む)
                maxStrokeCount  画数の上限
                protoDB         見本DB.
                param           認識処理に関するパラメータ.

  Returns:      認識アルゴリズムが必要とするメモリ量.
 *---------------------------------------------------------------------------*/
u32
PRC_GetRecognitionBufferSizeEx_Standard(int maxPointCount,
                                        int maxStrokeCount,
                                        const PRCPrototypeDB_Standard *protoDB,
                                        const PRCRecognizeParam_Standard *param)
{
    (void)maxPointCount;
    (void)maxStrokeCount;
    (void)protoDB;
    (void)param;

    return 1;                          // OS_Alloc(0) はエラーとなるため
}

/*---------------------------------------------------------------------------*
  Name:         PRC_GetRecognizedEntriesEx_Standard

  Description:  見本DBの特定の kind のエントリと入力パターンを比較して認識し,
                結果の上位 numRanking 位を返します.

  Arguments:    resultEntries   認識結果へのポインタを入れる配列へのポインタ.
                                求める個数以下しか認識できなかった場合、
                                残りは NULL で埋められます.
                resultScores    認識結果のスコアの配列へのポインタ.
                numRanking      result* に返す個数
                buffer          認識アルゴリズムが使用するメモリ領域へのポインタ.
                                (領域サイズ>=PRC_GetRecognitionBufferSize の返り値)
                input           入力パターン.
                protoDB         見本DB.
                kindMask        各見本DBエントリの kind 値との論理積をとり,
                                非ゼロであれば有効とみなします.
                param           認識処理に関するパラメータ.

  Returns:      比較された見本DB上のパターン数.
 *---------------------------------------------------------------------------*/
int
PRC_GetRecognizedEntriesEx_Standard(PRCPrototypeEntry **resultEntries,
                                    fx32 *resultScores,
                                    int numRanking,
                                    void *buffer,
                                    const PRCInputPattern_Standard *input,
                                    const PRCPrototypeDB_Standard *protoDB,
                                    u32 kindMask, const PRCRecognizeParam_Standard *param)
{
    int     i;
    const PRCiPatternData_Common *inputData;
    int     numCompared;
    int     normalizeSize;
    int     doubleWidth;

    (void)buffer;
    (void)param;

    SDK_ASSERT(resultEntries);
    SDK_ASSERT(resultScores);
    SDK_ASSERT(input);
    SDK_ASSERT(protoDB);
    SDK_ASSERT(numRanking > 0);

    for (i = 0; i < numRanking; i++)
    {
        resultEntries[i] = NULL;
        resultScores[i] = 0;
    }

    normalizeSize = protoDB->normalizeSize;
    if (normalizeSize < input->normalizeSize)
    {
        normalizeSize = input->normalizeSize;
    }
    doubleWidth = normalizeSize * 2;

    inputData = &input->data;
    numCompared = 0;

    {
        const PRCiPrototypeEntry_Common *proto;
        int     iPattern;

        proto = protoDB->patterns;

        for (iPattern = 0; iPattern < protoDB->patternCount; iPattern++, proto++)
        {
            const PRCiPatternData_Common *protoData;
            int     iStroke;
            fx32    wholeScore;
            fx32    wholeWeight;

            if (!proto->entry->enabled || !(proto->entry->kind & kindMask))
                continue;

            protoData = &proto->data;

            if (inputData->strokeCount != protoData->strokeCount)
                continue;

            wholeScore = 0;
            wholeWeight = 0;

            for (iStroke = 0; iStroke < inputData->strokeCount; iStroke++)
            {
                int     iProto, iInput;
                int     protoStrokeIndex, inputStrokeIndex;
                int     protoSize, inputSize;
                const PRCPoint *protoPoints;
                const PRCPoint *inputPoints;
                const u16 *protoAngle;
                const u16 *inputAngle;
                const fx16 *protoRatio;
                const fx16 *inputRatio;
//                PRCPoint protoMidPoint, inputMidPoint;
                fx16    protoNextRatio, inputNextRatio;
                fx32    strokeScore;
                fx16    strokeRatio;

                strokeScore = 0;

                protoStrokeIndex = protoData->strokes[iStroke];
                inputStrokeIndex = inputData->strokes[iStroke];
                protoSize = protoData->strokeSizes[iStroke];
                inputSize = inputData->strokeSizes[iStroke];
                protoPoints = &protoData->pointArray[protoStrokeIndex];
                inputPoints = &inputData->pointArray[inputStrokeIndex];
                protoAngle = &protoData->lineSegmentAngleArray[protoStrokeIndex];
                inputAngle = &inputData->lineSegmentAngleArray[inputStrokeIndex];
                protoRatio = &protoData->lineSegmentRatioToStrokeArray[protoStrokeIndex];
                inputRatio = &inputData->lineSegmentRatioToStrokeArray[inputStrokeIndex];

                SDK_ASSERT(protoSize >= 2);
                SDK_ASSERT(inputSize >= 2);

                iProto = iInput = 1;
                protoNextRatio = protoRatio[iProto];
                inputNextRatio = inputRatio[iInput];
//                GetMiddlePoint(&protoMidPoint, &protoPoints[iProto-1], &protoPoints[iProto]);
//                GetMiddlePoint(&inputMidPoint, &inputPoints[iInput-1], &inputPoints[iInput]);
                for (i = 0; i < protoSize + inputSize - 3; i++)
                {
                    int     diff, score;
                    SDK_ASSERT(iProto < protoSize);
                    SDK_ASSERT(iInput < inputSize);
                    diff = (s16)(protoAngle[iProto] - inputAngle[iInput]);
                    if (diff < 0)
                    {
                        diff = -diff;
                    }
                    score = ((32768 - diff) / 128);
                    if (protoNextRatio <= inputNextRatio)
                    {
                        // 見本の画を1歩辿る
                        // 入力の画の中で近いほうの点との距離をスコアに反映
                        inputNextRatio -= protoNextRatio;
                        score *= (inputNextRatio < inputRatio[iInput] / 2)      // 残りが半分より少ない＝次の点のほうが近い
                            ? (doubleWidth -
                               CityBlockDistance(&inputPoints[iInput],
                                                 &protoPoints[iProto])) : (doubleWidth -
                                                                           CityBlockDistance
                                                                           (&inputPoints
                                                                            [iInput - 1],
                                                                            &protoPoints[iProto]));
                        strokeScore += protoNextRatio * score;
                        iProto++;
                        protoNextRatio = protoRatio[iProto];
//                        GetMiddlePoint(&protoMidPoint, &protoPoints[iProto-1], &protoPoints[iProto]);
                    }
                    else
                    {
                        // 入力の画を1歩辿る
                        // 見本の画の中で近いほうの点との距離をスコアに反映
                        protoNextRatio -= inputNextRatio;
                        score *= (protoNextRatio < protoRatio[iProto] / 2)      // 残りが半分より少ない＝次の点のほうが近い
                            ? (doubleWidth -
                               CityBlockDistance(&inputPoints[iInput],
                                                 &protoPoints[iProto])) : (doubleWidth -
                                                                           CityBlockDistance
                                                                           (&inputPoints[iInput],
                                                                            &protoPoints[iProto -
                                                                                         1]));
                        strokeScore += inputNextRatio * score;
                        iInput++;
                        inputNextRatio = inputRatio[iInput];
//                        GetMiddlePoint(&inputMidPoint, &inputPoints[iInput-1], &inputPoints[iInput]);
                    }
                }

                strokeRatio = protoData->strokeRatios[iStroke];
                if (strokeRatio < inputData->strokeRatios[iStroke])
                {
                    strokeRatio = inputData->strokeRatios[iStroke];
                }

                wholeScore += FX_Mul(strokeScore, strokeRatio);
                wholeWeight += strokeRatio;
            }

            wholeScore = FX_Div(wholeScore, wholeWeight * doubleWidth);
            wholeScore /= 256;

            if (proto->entry->correction != 0)
            {
                wholeScore = FX_Mul(wholeScore, FX32_ONE - proto->entry->correction)
                    + proto->entry->correction;
            }

            if (wholeScore < 0)
                wholeScore = 0;
            if (wholeScore >= FX32_ONE)
                wholeScore = FX32_ONE;

            numCompared++;
            if (resultScores[numRanking - 1] < wholeScore)
            {
                resultScores[numRanking - 1] = wholeScore;
                resultEntries[numRanking - 1] = (PRCPrototypeEntry *)proto->entry;
                for (i = numRanking - 2; i >= 0; i--)
                {
                    if (resultScores[i] < resultScores[i + 1])
                    {
                        fx32    tmpScore;
                        PRCPrototypeEntry *tmpEntry;
                        tmpScore = resultScores[i];
                        resultScores[i] = resultScores[i + 1];
                        resultScores[i + 1] = tmpScore;
                        tmpEntry = resultEntries[i];
                        resultEntries[i] = resultEntries[i + 1];
                        resultEntries[i + 1] = tmpEntry;
                    }
                }
            }
        }
    }
    // スコアの正規化

    return numCompared;
}

/*===========================================================================*
  Static Functions
 *===========================================================================*/


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
