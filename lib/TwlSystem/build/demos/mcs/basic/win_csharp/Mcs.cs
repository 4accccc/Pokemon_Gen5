/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - basic - win
  File:     Mcs.cs

  Copyright 2004-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 701 $
 *---------------------------------------------------------------------------*/

using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace basic
{
    class Mcs
    {
        /// <summary>
        /// モジュール名
        /// </summary>
        public const string NNS_MCS_MODULE_NAME = "nnsmcs.dll";

        /// <summary>
        /// 関数名
        /// </summary>
        public const string NNS_MCS_API_OPENSTREAM = "NNS_McsOpenStream";

        /// <summary>
        /// 関数名
        /// </summary>
        public const string NNS_MCS_API_OPENSTREAMEX = "NNS_McsOpenStreamEx";

        /// <summary>
        /// デバイスのタイプ
        /// </summary>
        public enum NNS_MCS_DEVICE_TYPE
        {
            UNKNOWN,
            NITRO_DEBUGGER,
            NITRO_UIC,
            ENSATA,
            TWL
        }

        /// <summary>
        /// オープンしたストリームに関する補足情報を格納する構造体
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct NNSMcsStreamInfo
        {
            public uint structBytes;
            public uint deviceType;
        }

        /// <summary>
        /// MCS関数のデリゲート
        /// </summary>
        /// <param name="channel"></param>
        /// <param name="flags"></param>
        /// <returns></returns>
        [UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError=true)]
        public delegate SafePipeHandle NNS_McsOpenStream(ushort channel, uint flags);

        /// <summary>
        /// MCS関数のデリゲート
        /// </summary>
        /// <param name="channel"></param>
        /// <param name="flags"></param>
        /// <param name="streamInfo"></param>
        /// <returns></returns>
        [UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
        public delegate SafePipeHandle NNS_McsOpenStreamEx(ushort channel, uint flags, out NNSMcsStreamInfo streamInfo);
    }
}
