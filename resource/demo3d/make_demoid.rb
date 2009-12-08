#!/usr/bin/ruby
#===================================================================
#
# @brief  デモ再生アプリ用デモID生成
#
# @data   09.12.04
# @author genya_hosaka
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定
$KCODE = 'SJIS'

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "demo3d_demoid.h"  #出力ファイル名
DST_BRIEF = "デモ再生アプリで使うデモIDです。resource\demo3d内のフォルダから自動生成しています。"
DST_AUTOR = "genya_hosaka"
DST_NOTE = "このファイルはdemo3d/make_demoid.rbによって自動生成されたものです。"

#====================================================
# 主処理 開始
#====================================================

File::open( DST_FILENAME ,"w"){ |file|
    file.puts("//=========================================================================");
    file.puts("/**");
    file.puts(" * @file\t" + DST_FILENAME);
    file.puts(" * @brief\t" + DST_BRIEF);
    file.puts(" * @autor\t" + DST_AUTOR);
    file.puts(" * @note\t" + DST_NOTE);
    file.puts(" */");
    file.puts("//=========================================================================\n");
    file.puts("#ifndef __DEMO3D_DEMOID_H__\n");
    file.puts("#define __DEMO3D_DEMOID_H__\n\n");

    file.puts "#define DEMO3D_ID_NULL (0)\n\n"

    #カレントにあるファイルをリストアップ
    pwd =  Dir::entries( Dir::pwd )

    # 配列化
    val = pwd.to_a;
    # フォルダに厳選、特殊フォルダを排除
    val = val.select{ |i| File::ftype(i) == "directory" && i != "." && i != ".." && i != ".svn" }

    #すべてチェック
    count = 1
    for i in val
      #imdチェック > imdがフォルダ内に一つもない場合はIDを生成しない
      imd = Dir::entries(i).find{|elem| elem.slice(/...\z/m) == "imd" }
      if imd != nil
        file.puts "#define DEMO3D_ID_" + i.upcase + " (" + count.to_s + ")" 
        count += 1
      end
    end
    
    file.puts "\n#define DEMO3D_ID_MAX (" + count.to_s + ")\n\n"
    
    file.puts("#endif // __DEMO3D_DEMOID_H__");

}
