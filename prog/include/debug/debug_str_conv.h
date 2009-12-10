//======================================================================
/**
 * @file	debug_str_conv.h
 * @brief	SJISとunicodeの相互変換
 * @author	ariizumi
 * @data	09/12/01
 *
 * モジュール名：DEB_STR_CONV
 */
//======================================================================
#pragma once
//--------------------------------------------------------------
//	charの文字列をSTRCODE(u16)に変換
//
//  @param str1 char型文字列
//  @param str2 STRCODE型文字列
//  @param str1の変換を行う長さ(byte単位
//--------------------------------------------------------------
extern void DEB_STR_CONV_SjisToStrcode( const char* str1 , STRCODE *str2 , const u32 len );

//--------------------------------------------------------------
//	STRCODE(u16)の文字列をcharに変換
//
//  @param str1 STRCODE型文字列
//  @param str2 char型文字列
//  @param str1の変換を行う長さ(文字数
//--------------------------------------------------------------
extern void DEB_STR_CONV_StrcodeToSjis( const STRCODE *str1 , char *str2 , const u32 len );
extern void DEB_STR_CONV_StrBufferToSjis( const STRBUF *str1 , char *str2 , const u32 len );


