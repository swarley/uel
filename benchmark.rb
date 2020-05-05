require 'bert'
require 'erlang-etf'
require 'oj'
require 'untitled_etf_lib'
require 'json'
require 'benchmark/ips'

Benchmark.ips do |x|
  etf_data = "\x83b\x00\x0009"
  json_data = "12345"
  raise "OJ FAIL" unless Oj.load(json_data) == 12345
  raise "UEL FAIL" unless UEL.decode(etf_data) == 12345
  raise "EE FAIL" unless Erlang.binary_to_term(etf_data) == 12345

  #x.report("oj integer") { Oj.load json_data }
  #x.report("json integer") { JSON.load json_data }
  x.report("uel integer") { UEL.decode(etf_data) }
  x.report("ee integer") { Erlang.binary_to_term(etf_data) }

  x.compare!
end

Benchmark.ips do |x|
  etf_data = "\x83FA\x9do4T~ku"
  json_data = "123456789.123456789"
  raise "OJ FAIL" unless Oj.load(json_data).to_f == 123456789.123456789
  raise "UEL FAIL" unless UEL.decode(etf_data) == 123456789.123456789
  raise "EE FAIL" unless Erlang.binary_to_term(etf_data) == 123456789.123456789

  #x.report("oj float") { Oj.load json_data }
  #x.report("json float") { JSON.load json_data }
  x.report("uel float") { UEL.decode(etf_data) }
  x.report("ee float") { Erlang.binary_to_term(etf_data) }
  
  x.compare!
end

Benchmark.ips do |x|
  etf_data = "\x83j"
  json_data = "[]"

  raise "OJ FAIL" unless Oj.load(json_data) == []
  raise "UEL FAIL" unless UEL.decode(etf_data) == []
  raise "BERT FAIL" unless Erlang.binary_to_term(etf_data) == []

  #x.report("oj []") { Oj.load json_data }
  #x.report("json []") { JSON.load json_data }
  x.report("uel nil")  { UEL.decode(etf_data) }
  x.report("ee nil") { Erlang.binary_to_term(etf_data) }
  x.compare!
end

Benchmark.ips do |x|
  etf_data = "\x83n\x08\x00\xff\xff\xff\xff\xff\xff\xff\xff"
  json_data = 0xFFFFFFFFFFFFFFFF.to_s
  
  raise "OJ FAIL" unless Oj.load(json_data) == 0xFFFFFFFFFFFFFFFF
  raise "UEL FAIL" unless UEL.decode(etf_data) == 0xFFFFFFFFFFFFFFFF
  raise "BERT FAIL" unless Erlang.binary_to_term(etf_data) == 0xFFFFFFFFFFFFFFFF

  #x.report("oj big_num") { Oj.load json_data }
  #x.report("json big_num") { JSON.load json_data }
  x.report("uel big_num") { UEL.decode(etf_data) }
  x.report("ee big_num") { Erlang.binary_to_term(etf_data) }
  x.compare!
end

Benchmark.ips do |x|
  etf_data = "\x83l\x00\x00\x00\x03a\x01F?\xf1\x99\x99\x99\x99\x99\x9an\x06\x00\x00\xa0rN\x18\tj"
  json_data = "[1, 1.1, 10000000000000]"

  raise "OJ FAIL" unless Oj.load(json_data) == [1, 1.1, 10000000000000]
  raise "UEL FAIL" unless UEL.decode(etf_data) == [1, 1.1, 10000000000000]
  raise "EE FAIL" unless Erlang.binary_to_term(etf_data) == [1, 1.1, 10000000000000]

  x.report("oj list") { Oj.load json_data }
  x.report("json list") { JSON.load json_data }
  x.report("uel list") { UEL.decode(etf_data) }
  x.report("ee list") { Erlang.binary_to_term(etf_data) }
  x.compare!
end

Benchmark.ips do |x|
  etf_data = "\x83t\x00\x00\x00\x02m\x00\x00\x00\x03fooa\x02m\x00\x00\x00\x03barl\x00\x00\x00\x02a\x04a\x05j"
  json_data = '{"foo": 2, "bar": [4, 5]}'


  raise "OJ FAIL" unless Oj.load(json_data) == {"foo" => 2, "bar" => [4, 5]}
  raise "UEL FAIL" unless UEL.decode(etf_data) == {"foo" => 2, "bar" => [4, 5]}
  raise "EE FAIL" unless Erlang.binary_to_term(etf_data) == {"foo" => 2, "bar" => [4, 5]}

  x.report("oj map") { Oj.load json_data }
  x.report("json map") { JSON.load json_data }
  x.report("uel map") { UEL.decode(etf_data) }
  x.report("ee map") { Erlang.binary_to_term(etf_data) }
  x.compare!
end