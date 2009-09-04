require 'rexml/document'

class REWRITE_TEX
	def initialize(inTextArray, inPalTextAry)
		@TextArray = Marshal.load(Marshal.dump(inTextArray))
		@PalTextAry = Marshal.load(Marshal.dump(inPalTextAry))
	end
	
	def RemakeElem(elem)
		if elem.name == "bitmap"
			 elem.text = @TextArray.shift
		 elsif elem.name == "tex_palette"
			 elem.text = @PalTextAry.shift
			 p elem.text
		end
		if elem.has_elements? then
			elem.each_element{|e|
			RemakeElem(e)
			}
		end
	end
	def OutPut(inImdFileName, inMakeFileName)
		file = File.open(inMakeFileName,"w")
		doc = REXML::Document.new(File.open(inImdFileName))
		RemakeElem(doc.root)
		doc.xml_decl.write(file)
		doc.root.write(file)
	end
end

