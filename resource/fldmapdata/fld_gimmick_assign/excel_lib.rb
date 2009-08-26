# RubydeExcel Ver.1.02    2004/1/24
# Copyright (C) 2003,2004 Oka Yasushi <yac@tech-notes.dyndns.org>
# You may redistribute it and/or modify it under the same license terms as Ruby.
require 'win32ole'
require 'singleton'

class WIN32OLE
  @const_defined = Hash.new 
  
  # const_load()ÇÃçƒíËã`
  def WIN32OLE.my_const_load(ole_object, const_name_space)
    unless @const_defined[const_name_space] then
      const_load(ole_object, const_name_space)
      @const_defined[const_name_space] = true
    end
  end
end


class FileSystemObject
  include Singleton
  def initialize
    @body =  WIN32OLE.new('Scripting.FileSystemObject')
  end
  
  def fullpath(filename)
    @body.getAbsolutePathName(filename)
  end
end


class Excel_Wrapper
  def initialize(raw_object)
    @raw_object = raw_object
  end
  
  def raw
    @raw_object
  end
  
  def inspect()
    self.class
  end
  
  private
  
  def method_missing(m_id, *params)
    missing_method_name = m_id.to_s
    if @raw_object == nil
      raise RuntimeError, "This object is no more worked.",caller
    end
    # downcase method name
    if methods.include?(m_id.to_s.downcase)
      send(m_id.to_s.downcase.intern, *params)
    else
      begin
        @raw_object.send(m_id, *params)
      rescue
        raise $!,$!.message, caller
      end
    end
  end
  
  
  def undef_my_method()
    instance_method_arr = self.class.instance_methods(false)
    eval_msg_arr = []
    instance_method_arr.each {|method_name|
      next if method_name == "inspect"
      eval_msg_arr.push "def self.#{method_name}(*params); raise RuntimeError, 'This object is no more worked.',caller; end"
    }
    eval eval_msg_arr.join("\n")
  end
  
end

# ====================================================
# ============= Excel Application Class ==============
# ====================================================
class Excel < Excel_Wrapper
  def initialize(visible=true)
    @raw_object = WIN32OLE.new('EXCEL.Application')
    WIN32OLE.my_const_load(@raw_object, Excel)
    @raw_object.visible = visible
    @raw_object.displayAlerts = false
  end
  
  def copy_book(template_filename = nil)
    if (template_filename == nil) or (template_filename == "") then
      workbook = @raw_object.Workbooks.add
    else
      fullpath = FileSystemObject.instance.fullpath(template_filename)
      workbook = @raw_object.workbooks.add(fullpath)
    end
    Workbook.new(workbook)
  end
  
  def open_book(filename)
    fullpath = FileSystemObject.instance.fullpath(filename)
    workbook = @raw_object.workbooks.open({'filename'=> fullpath})
    Workbook.new(workbook)
  end
  
  def workbooks(index=nil)
    if index
      Workbook.new(@raw_object.Workbooks(index))
    else
      Workbooks.new(@raw_object.Workbooks)
    end
  end
  
  def quit
    @raw_object.Quit
    @raw_object = nil
    undef_my_method()
  end
end

# Collection Class
class Excel
  # ========================================================
  # ============ Workbook Collection Class =================
  # ========================================================
  class Workbooks < Excel_Wrapper
    def item(index=nil)
      Workbook.new(@raw_object.item(index))
    end
    
    def [](index)
      Workbook.new(@raw_object.item(index))
    end
    
    def size
      @raw_object.count
    end
    
    def each
      @raw_object.each {|raw_workbook|
        yield(Workbook.new(raw_workbook))
      }
    end
    
  end
  
  # ========================================================
  # ============ Worksheet Collection Class ================
  # ========================================================
  class Worksheets < Excel_Wrapper
    def [](index)
      raw_worksheet = @raw_object.item(index)
      raw_worksheet.Activate
      Worksheet.new(raw_worksheet)
    end
    
    alias item []
    
    def size
      @raw_object.count
    end
    
    def each
      @raw_object.each {|raw_worksheet|
        raw_worksheet.Activate
        yield(Worksheet.new(raw_worksheet))
      }
    end
    
  end
end

# =============================================
# ============ Workbook Class =================
# =============================================
class Excel
  class Workbook < Excel_Wrapper
    def [](sheet_name)
      raw_worksheet = @raw_object.Worksheets(sheet_name)
      raw_worksheet.Activate
      Worksheet.new(raw_worksheet)
    end
    
    alias open_sheet []
    
    def worksheets(index=nil)
      if index
        self[index]
      else
        Worksheets.new(@raw_object.worksheets)
      end
    end
    
    alias sheets worksheets
    
    def add_sheet(source_sheet)
      last_sheet_no = @raw_object.worksheets.count
      source_sheet.copy({'after' => @raw_object.worksheets(last_sheet_no)})
      raw_worksheet = @raw_object.sheets(last_sheet_no + 1)
      Worksheet.new(raw_worksheet)
    end
    
    def save(filename)
      fullpath = FileSystemObject.instance.fullpath(filename)
      @raw_object.saveAs({'filename'=>fullpath})
    end
    
    def each_sheet
      @raw_object.worksheets.each {|raw_worksheet|
        raw_worksheet.Activate
        yield Worksheet.new(raw_worksheet)
      }
    end
    
    def parent
      @raw_object.parent
    end
    
    def close
      @raw_object.Close
      @raw_object = nil
      undef_my_method()
    end
    
  end
end

# =============================================
# ============== Worksheet Class ==============
# =============================================
class Excel
  class Worksheet < Excel_Wrapper
    def range(cell_1, cell_2=nil)
      if cell_1.class == String
        begin
          raw_range = @raw_object.range(cell_1)
        rescue WIN32OLERuntimeError
          raise $!,$!.message, caller
        end
      else
        raw_range = @raw_object.range(cell_1.raw, cell_2.raw)
      end
      Range.new(raw_range)
    end
    
    def cells(v_pos, h_pos)
      begin
        raw_range = @raw_object.Cells(v_pos, h_pos)
      rescue WIN32OLERuntimeError
        raise $!,$!.message, caller
      end
      Range.new(raw_range)
    end
    
    def [](cell_1)
      begin
        range(cell_1)
      rescue WIN32OLERuntimeError
        raise $!,$!.message, caller
      end
    end
    
    def delete
      @raw_object.delete
      @raw_object = nil
      undef_my_method()
    end
    
    def name=(sheet_name)
      @raw_object.name = sheet_name
    end
    
    def parent
      Workbook.new(@raw_object.parent)
    end
  end
end

# =========================================
# ============== Range Class ==============
# =========================================
class Excel
  class Range < Excel_Wrapper
    # Return value of cell. 
    # If range contains multi cells, return Array of value.
    def value
      if (h_size() == 1) and (v_size() == 1)
        return @raw_object.value
      end
      
      return_arr = []
      self.each_range {|inner_range|
        return_arr.push inner_range.raw.value
      }
      if return_arr.size == 1
        return return_arr[0]
      else
        return return_arr
      end
    end
    alias text value
    
    
    def value=(text)
      if text.class == Array
        text_arr = text.dup
      else
        text_arr = [text]
      end
      
      self.each_range {|inner_range|
        inner_range.raw.value = text_arr.shift
      }
    end
    alias text= value=
    
    def name
      book = parent.parent
      name_list = book.names
      name_list.each {|name_obj|
        # puts "*** name_obj.name = #{name_obj.name}"
        if Range.new(name_obj.RefersToRange) == self
          return name_obj.name
        end
      }
      @raw_object.Address("RowAbsolute"=>false, "ColumnAbsolute"=>false, "ReferenceStyle"=>Excel::XlA1)
    end
    
    def name=(range_name)
      sheet = parent
      book = sheet.parent
      a1_address = @raw_object.Address("ReferenceStyle"=>Excel::XlA1)
      # puts "*** Range.name=#{range_name}, address=#{a1_address}"
      book.names.Add("Name"=>range_name, "RefersToLocal"=>"=#{sheet.name}!#{a1_address}")
      
    end
    
    def v_pos()
      @raw_object.row
    end
    
    def h_pos()
      @raw_object.column
    end
    
    def position()
      [@raw_object.row, @raw_object.column]
    end
    
    def v_size
      @raw_object.rows.count
    end
    
    def h_size
      @raw_object.columns.count
    end
    
    def merge()
      orig_name = name()
      @raw_object.Merge()
      Range.new(@raw_object)
    end
    
    def unmerge()
      top_cell = @raw_object.cells(1,1)
      @raw_object.UnMerge()
      Range.new(top_cell)
    end
    
    def mergearea()
      Range.new(@raw_object.MergeArea)
    end
    
    
    
    def cells(v_pos, h_pos)
      Range.new(@raw_object.cells(v_pos, h_pos))
    end
    
    def inspect()
      return self.class unless @raw_object
      if (v_size > 1) or (h_size > 1) 
        "Range:((V:#{v_pos},H:#{h_pos})-(V:#{v_pos+v_size-1},H:#{h_pos+h_size-1}))"
      else
        "Range:(V:#{v_pos},H:#{h_pos})"
      end
    end
    
    def each_range()
      my_h_size = h_size()
      my_v_size = v_size()
      
      merge_range_array = []
      
      my_v_size.times {|v_offset|
        my_h_size.times {|h_offset|
          target_cell = Range.new(@raw_object.cells(v_offset+1, h_offset+1))
          if target_cell.MergeCells
            if operation_finished?(merge_range_array,target_cell)
              # skip
            else
              merge_range_array.push target_cell.mergeArea
              yield(target_cell.mergeArea.cells(1,1))
            end
          else
            yield(target_cell)
          end
        }
      }
    end

    
    
    def ==(other_range)
      return false unless other_range.methods.include?("position")
      if (position() == other_range.position()) and 
         (h_size() == other_range.h_size) and (v_size() == other_range.v_size)
        return true
      end
      return false
    end
    
    def include?(inner_range)
      my_v_pos, my_h_pos = position()
      my_v_size = v_size()
      my_h_size = h_size()
      my_h_range = my_h_pos..(my_h_pos+my_h_size-1)
      my_v_range = my_v_pos..(my_v_pos+my_v_size-1)
      
      inner_v_pos, inner_h_pos = inner_range.position()
      inner_v_size = inner_range.v_size()
      inner_h_size = inner_range.h_size()
      if ( my_h_range.include?(inner_h_pos) and my_h_range.include?(inner_h_pos+inner_h_size-1) and
           my_v_range.include?(inner_v_pos) and my_v_range.include?(inner_v_pos+inner_v_size-1))
        return true
      else
        return false
      end
      
      
    end
    
    # shift = Excel::XlShiftToRight or Excel::XlShiftDown 
    def insert_here(shift=nil)
      @raw_object.copy
      if shift then
        @raw_object.insert({'shift'=>shift})
      else
        @raw_object.insert  # Auto Shift
      end
      self
    end

    # shift = Excel::XlShiftToLeft or Excel::XlShiftUp 
    def delete(shift=nil)
      if shift then
        @raw_object.delete({'shift'=>shift})
      else
        @raw_object.delete  # Auto Shift
      end
      @raw_object = nil
      undef_my_method()
    end
    
    def parent
      Worksheet.new(@raw_object.parent)
    end
    
    def get_cursol()
      RangeCursol.new(self)
    end
    
    
    private
    def operation_finished?(merge_range_array,target_cell)
      merge_range_array.each {|merge_range|
        if merge_range.include?(target_cell)
          return true
        end
      }
      return false
    end

  end
end

# ===============================================
# ============== RangeCursol Class ==============
# ===============================================
class Excel
  class RangeCursol < Range
    def h_move(offset)
      return(self) if offset == 0
      sheet = @raw_object.parent
      sign = (offset.abs / offset)  # +1 or -1
      offset.abs.times do
        v_pos, h_pos = position()
        h_pos += (h_size() * sign)
        h_pos = 1 if h_pos < 1
        next_top_cell = sheet.cells(v_pos,h_pos)
        next_bottom_cell = sheet.cells(v_pos + v_size() - 1, h_pos + h_size() -1)
        next_range = sheet.range(next_top_cell, next_bottom_cell)
        @raw_object = next_range
      end
      
      self.activate
      self
    end
  
    def v_move(offset)
      return(self) if offset == 0
      sheet = @raw_object.parent
      sign = (offset.abs / offset)  # +1 or -1
      offset.abs.times do
        v_pos, h_pos = position()
        v_pos += (v_size() * sign)
        v_pos = 1 if v_pos < 1
        next_top_cell = sheet.cells(v_pos, h_pos)
        next_bottom_cell = sheet.cells(v_pos + v_size() - 1, h_pos + h_size() -1)
        next_range = sheet.range(next_top_cell, next_bottom_cell)
        @raw_object = next_range
      end
      self.activate
      self
    end
    
    # return Excel::Range object 
    def range()
      @raw_object
    end
    alias parent range
    
    #undef delete
    def delete(shift=nil)
      range_name = @raw_object.name
      parent_sheet = @raw_object.parent
      @raw_object.delete(shift)
      @raw_object = parent_sheet.range(range_name)
    end
    
  end
end

if $0 == __FILE__
  def assert_equal(expected, val)
    puts "***** assert_equal *****"
    p expected
    p val
  end
  
  @excel = Excel.new
    book = @excel.copy_book("./test_template.xls")
    sheet = book["Sheet1"]
    sheet.delete
    begin
      range_01 = sheet.range("A1")
    rescue RuntimeError
      puts "Runtime ErrorÇ™ÇøÇ·ÇÒÇ∆Ç®Ç´ÇΩ"
    end
    sheet = book.sheets(2)
    range_01 = sheet.range("A:A")
    range_01.delete
    p range_01
    p range_01.name
end
