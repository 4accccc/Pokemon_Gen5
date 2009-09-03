#==================================================================================
#
# ネットで拾ったrubyでenumを実現するスクリプト
#
# ex)単純に0から番号を振る
# require 'constant.rb'
#
# class Hoge
#   enum_const_set %w[
#     HOGE1
#     HOGE2
#     HOGE3
#   ]
# end
#
# p Hoge::HOGE1 ==> 0
# p Hoge::HOGE2 ==> 1
# p Hoge::HOGE3 ==> 2
#
# ex)初期値代入（たぶん=の前後にスペースがあっちゃだめ）
# require 'constant.rb'
#
# class Hoge
#   enum_const_set_ex %w[
#     HOGE1
#     HOGE2=300
#     HOGE3=1000
#   ]
# end
#
# p Hoge::HOGE1 ==> 0
# p Hoge::HOGE2 ==> 300
# p Hoge::HOGE3 ==> 1000
#
# ex)ビット代入
# require 'constant.rb'
#
# class Hoge
#   enum_const_set_bit %w[
#     HOGE1
#     HOGE2
#     HOGE3
#   ]
# end
#
# p Hoge::HOGE1 ==> 1
# p Hoge::HOGE2 ==> 2
# p Hoge::HOGE3 ==> 4
#
#==================================================================================
module Constant
  def self.enum_set(klass, names, init_val = 0)
    value = init_val.to_int
    names.each do |name|
      klass.const_set(name, value)
      value += 1
    end
  end

  def self.enum_set_ex(klass, exprs, init_val = 0)
    value = init_val.to_int
    exprs.each do |expr|
      case expr
      when /\A([^=]+)\s*=\s*(.+)\Z/
	value = klass.module_eval($2).to_int
	klass.const_set($1, value)
      else
	klass.const_set(expr, value)
      end
      value += 1
    end
  end

  def self.enum_set_bit(klass, names, init_val = 1)
    value = init_val.to_int
    names.each do |name|
      klass.const_set(name, value)
      value <<= 1
    end
  end
end

class Module
  def enum_const_set(names, init_val = 0)
    Constant.enum_set(self, names, init_val)
  end

  def enum_const_set_ex(exprs, init_val = 0)
    Constant.enum_set_ex(self, exprs, init_val)
  end

  def enum_const_set_bit(names, init_val = 1)
    Constant.enum_set_bit(self, names, init_val)
  end
end

