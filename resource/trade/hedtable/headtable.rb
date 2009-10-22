## 頭文字検索テーブルを作るコンバータです
##  2009.10.21 k.ohno
## headtable.rb モンスターgmm 文字配列gmm JPN outfile
## 
## モンスターの一覧が 文字配列のどこにあたるかをcdatテーブルとして出します
## cdatテーブルは u8 です
## 文字配列が255を超えるとこのrubyだけでなくソースも修正する必要があります
## 
##
##

require "rexml/document"
require "rexml/parseexception"
include REXML
require "nkf"



### メイン処理クラス

class GmmConv
  
  ##置き換えたい物をここに書けばよい
  
  def initialize
    
    
  end
  
  
  def readMonsGmm(gmmfile, gmmLANG, arrayH)
    File.open(gmmfile) {|xmlfile|
      doc = REXML::Document.new xmlfile
      
      doc.root.elements.each("body/row/language"){ |element|
        attr = element.attributes
        attr.each{ |attname,value|
          if attname == "name"
            if value == gmmLANG
#              print NKF.nkf("-s",element.text)
              arrayH.push(element.text)
            end
          end
        }
      }
    }
  end
  
  def readCommentGmm(gmmfile, gmmLANG, arrayH)
    File.open(gmmfile) {|xmlfile|
      doc = REXML::Document.new xmlfile
      doc.root.elements.each("body/row/comment"){ |element|
        arrayH.push(element.text)
      }
    }
  end
  
  
  def outTable(cdatfile, arrayMons, arrayHead, arrayComment)
    
    
    fh = File.open(cdatfile,"w")
    
    fh.puts( "//このファイルは自動生成 headtable.rb 2009.10.22 k.ohno")
    fh.puts( "static u8 MonsterHeadLangTable[]={")
    
    
    
    arrayMons.each{|monsname|
      index=0
      lookat=0
      arrayHead.each{|hedname|
        if monsname =~ /^#{hedname}/
          fh.puts( index.to_s + ",   //" + NKF.nkf("-s",monsname) +"  "+ NKF.nkf("-s",hedname))
          lookat=1
          break
        end
        index = index + 1
      }
      if lookat==1
        next
      end
      index=0
      
      arrayComment.each{|hedname|
        d = hedname.split(',')
        d.each{ |dname|
          if monsname =~ /^#{dname}/
            fh.puts( index.to_s + ",   //" + NKF.nkf("-s",monsname) +"  "+ NKF.nkf("-s",dname))
            lookat=1
            break
          end
        }
        if lookat==1
          break
        end
        index = index + 1
      }
      if lookat==0
        fh.puts( "255, //" + NKF.nkf("-s",monsname))
      end
    
    }   
    fh.puts( "};")
    fh.close
  end
  
end


begin
  gmmLANG = ARGV[2]
  
  arrayMons = Array.new
  arrayHead = Array.new
  arrayComment = Array.new
  
  gmmc = GmmConv.new
  gmmc.readMonsGmm(ARGV[0], gmmLANG, arrayMons)
  gmmc.readMonsGmm(ARGV[1], gmmLANG, arrayHead)
  gmmc.readCommentGmm(ARGV[1], gmmLANG, arrayComment)
  
  gmmc.outTable(ARGV[3],arrayMons,arrayHead,arrayComment)
  
end
  


