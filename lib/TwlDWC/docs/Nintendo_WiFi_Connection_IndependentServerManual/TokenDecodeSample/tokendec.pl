#!/usr/bin/perl
#
# Nintendo Wi-Fi Connection
#   独自ゲームサーバ用認証トークン復号化プログラムサンプル Ver0.1
#
# ※利用方法
#   tokendec.pl token
#      token : 認証トークン
#

use strict;
use warnings;
use Crypt::CBC;
use Digest::MD5 qw(md5_hex);
use MIME::Base64 qw(decode_base64);


# 認証トークンを引数から取得
if ($#ARGV < 0) {
  print STDERR "usage: tokendec.pl token\n";
  exit;
}
my $token = decode_base64($ARGV[0]);
chomp($token);


# AES復号化
#
# 下記のトークン鍵は本サンプル用の仮のものです。
# 実際に使用するトークン鍵は、アプリケーション毎のサービスロケータ開発用/製品
# 用トークンに置き換えてください。
# サービスロケータ設定情報は、NWCWMSによる独自ゲームサーバの利用申請が承認され
# た時点でNWCWMSにてお知らせします。
my $key = "1234123412341234";   # トークン鍵
my $iv  = pack("H*", "0" x 32); # IV
my $cipher = Crypt::CBC->new({"key"            => $key,
                              "cipher"         => "Rijndael_PP_128",
                              "iv"             => $iv,
                              "regenerate_key" => 0,
                              "padding"        => "null",
                              "prepend_iv"     => 0
                             });
my $dectoken = $cipher->decrypt($token);


# トークン解析
if ($dectoken !~ /^\\u\\(\d{13})\\m\\(\w{12})\\t\\(\d+)\\h\\(\w{8})$/ &&  # DS
    $dectoken !~ /^\\w\\(\d{16})\\m\\(\w{12})\\t\\(\d+)\\h\\(\w{8})$/) {  # Wii
  print STDERR "Invalid Token\n";
  exit;
}

my ($userid, $macadr, $timestamp, $hash) = ($1, $2, $3, $4);
print "UserID    : " . $userid . "\n";
print "MACADR    : " . $macadr . "\n";
print "TimeStamp : " . $timestamp . "\n";
print "Hash      : " . $hash . "\n";


# ハッシュ値正常？
$dectoken =~ /^(.*)\\h\\\w{8}$/;
my $calchash = substr(md5_hex($1), 0, 8);
if ($calchash ne $hash) {
  print STDERR "Hash Invalid (" . $calchash . ")\n";
}


# 24時間以内？
if (time - $timestamp > 24 * 60 * 60) {
  print STDERR "Token Expired (" . (time - $timestamp) . ")\n";
}


exit;
