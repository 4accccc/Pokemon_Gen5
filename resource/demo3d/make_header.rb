#!/usr/bin/ruby
#===================================================================
#
# @brief  デモ再生アプリ用ヘッダ生成
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
# String クラスの tr メソッドで日本語を使用可能にする。
require "jcode"

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "demo3d_resdata.cdat"  #出力ファイル名
DST_BRIEF = "コンバート環境にあるデータからデモ再生に使うテーブルを生成します。"
DST_AUTOR = "genya_hosaka"
DST_NOTE = "このファイルはdemo3d/make_header.rbによって自動生成されたものです。"
ARC_ID = "ARCID_DEMO3D_GRA"

#拡張子チェック
def check_ext(file)
  ext = file.slice(/...\z/m)
  
  return ( ext == "imd" || ext == "ica" || ext == "ita" ||
           ext == "ima" || ext == "itp" || ext == "iva" );
end

#setupテーブル書き出し
def print_setup(file,dir,cnt)
  file.puts "//setup"
  file.puts "static const GFL_G3D_UTIL_SETUP " + dir + "_setup[] = {"
  for i in 1..cnt-1
    str_unit = dir + "_unit" + sprintf("%02d",i);
    file.puts "\t{ res_" + str_unit + ", NELEMS(res_" + str_unit + "), obj_" + str_unit + ", NELEMS(obj_" + str_unit +  ") },"
  end
  file.puts "};"
end

#animeテーブル書き出し
def print_anime(file,dir,cnt,anm_cnt)
    file.puts("//ANM");
    
    file.puts "static const GFL_G3D_UTIL_ANM anm_" + dir + "_unit" + sprintf("%02d",cnt) + "[] = {";
    
    for k in 1..anm_cnt
        file.puts "\t{ " + k.to_s + ", 0 },"
    end
    
    file.puts("};\n");
end

#objテーブル書き出し
def print_obj(file,dir,cnt,is_anm_tbl)
    file.puts("//OBJ");
    
    str_unit_num = sprintf("%02d",cnt);
                
    file.puts "static const GFL_G3D_UTIL_OBJ obj_" + dir + "_unit" + str_unit_num + "[] = {";
    file.puts "\t{"
    file.puts "\t\t0, 0, 0, // MdlResID, MdlDataID, TexResID"
    if is_anm_tbl == true
      file.puts "\t\tanm_" + dir + "_unit" + str_unit_num + ", NELEMS(anm_" + dir + "_unit" + str_unit_num + "), // AnmTbl, AnmTblNum"
    else
      file.puts "\t\tNULL, 0, // AnmTbl, AnmTblNum"
    end
    file.puts "\t}"
    file.puts("};\n");
end


#シーン項目(フォルダ毎のデータ)を書き出し
def print_scene(file, dir)
  is_elem_null = true #項目が一つもない

  puts " > " + dir

  #フォルダ内のファイルを列挙
  scene_dir = Dir::entries( dir )

  #デモIDの文字列を生成
  demoid = "DEMO3D_ID_" + dir.upcase

  # sceneヘッダ
  file.puts("//=========================================================================\n");
  file.puts("// " + demoid );
  file.puts("//=========================================================================\n");

  #1オリジン
  cnt = 1; 
  loop do

    #フォルダ内のファイルを順繰り
    anm_cnt = 0;

    #拡張子＆2文字の数値で絞込み
    val = scene_dir.to_a;
    val = val.select{ |i| i.slice(/\d\d/).to_i == cnt && check_ext(i) }

    #imdチェック
    imd = val.find{ |i| i.slice(/...\z/m) == "imd" }
    
    #imdがなければ終了
    if imd == nil
      if is_elem_null == false
        print_setup(file,dir,cnt);
      end
      break;
    else
      is_elem_null = false;

      #ヘッダ書き込み
      file.puts "//UNIT"
      file.puts "static const GFL_G3D_UTIL_RES res_" + dir + "_unit" + sprintf("%02d",cnt) + "[] = {\n";

      #imdを先に書き出し
      file.puts "\t{ " + ARC_ID + ", NARC_demo3d_" + imd.sub(/.i/,"_nsb") + ", GFL_G3D_UTIL_RESARC },";
      
      #UNIT項目書き出し
      val.each{|i|
        #imdは既に出力したので飛ばす
        if i.slice(/...\z/m) != "imd"
          line = "\t{ " + ARC_ID + ", NARC_demo3d_" + i.sub(/.i/,"_nsb") + ", GFL_G3D_UTIL_RESARC },";
          file.puts line
          anm_cnt += 1
        end
      }
      
      # フッダ書き込み
      file.puts("};\n");
      
      # アニメ判定
      if anm_cnt <= 0
        print_obj(file,dir,cnt,false);
      else
        #アニメテーブル書き出し
        print_anime(file,dir,cnt,anm_cnt)
        print_obj(file,dir,cnt,true);
      end

      # 次のユニットへ
      cnt += 1
    end

  end
end

# アクセステーブルの項目を書き出し
def print_access_table(file,dir)
  #フォルダ内のファイルを列挙
  scene_dir = Dir::entries( dir )
  
  # 設定テキストを読み出し
  val = scene_dir.to_a;
  val = val.select{ |i| i.slice(/init/) == "init" }

  init_path = dir.to_s + "/" + val.to_s

  # 初期化パラメータ格納配列
  param = []

  File::open( init_path ,"r"){ |init|
    cnt = 0;
    init.each{ |line|
      param[cnt] = line.to_f # 一回数値にすることでコメントやスペースをそぎ落とす

      # 0=アニメーションスピード, 1=fovy_sin, 2=fovy_cos
      if cnt < 3
        param[cnt] = param[cnt] * 0x1000
      end
      
      param[cnt] = param[cnt].prec_i  #整数に変換
      cnt += 1
    }
  }

  p param
  
  #フォルダ内のファイルを順繰り
  scene_dir.each{|i|
    #拡張子チェック
    if check_ext(i)
      # 2文字の数値チェック
      if i.slice(/camera/) == "camera"
          line = "\t{ " + dir + "_setup, NELEMS(" + dir + "_setup), NARC_demo3d_" + i.sub(/.ica/,"_bin") + ", "
          line += param[0].to_s + ", " + param[1].to_s + ", " + param[2].to_s + ", " + param[3].to_s + ", " + " },"
          file.puts line        
      end
    end
  }
end


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
#    file.puts("#pragma once\n\n");
#    file.puts("#include <gflib.h>\n");
#    file.puts("#include \"arc/demo3d.naix\"\n\n");
    
    #カレントにあるフォルダをしらみつぶし
    pwd =  Dir::entries( Dir::pwd )
    for i in pwd
      if File::ftype( i ) == "directory" 
        # 特殊パスは排除
        if i != "." && i != ".." && i != ".svn"
          print_scene(file,i)
        end
      end
    end

    file.puts("\n\n");

    file.puts("//=========================================================================\n");
    file.puts("// アクセステーブル" );
    file.puts("//=========================================================================\n");
    file.puts "static const DEMO3D_SETUP_DATA c_demo3d_setup_data[ DEMO3D_ID_MAX ] = {"
    file.puts "\t{ 0 }, // DEMO3D_ID_NULL"

    #カレントにあるフォルダをしらみつぶし
    pwd = Dir::entries( Dir::pwd )
    for i in pwd
      if File::ftype( i ) == "directory" 
        # 特殊パスは排除
        if i != "." && i != ".." && i != ".svn"
          print_access_table(file,i)
        end
      end
    end
  file.puts "};"

};
