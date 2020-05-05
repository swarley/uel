# encoding: utf-8

RSpec.describe UEL do
  describe 'decode' do
    it 'handles small integers' do
      short = 5
      compiled_short = [131, 97, short].pack('C*')
      expect(UEL.decode(compiled_short)).to eq short
    end

    it 'handles floats' do
      float = 12345.6789
      compiled_float = [131, 99].pack('C*') + ('%.20e' % float).ljust(31, "\0")
      expect(UEL.decode(compiled_float)).to eq float
    end

    it 'handles integers' do
      integer = 1 << 10
      compiled_integer = [131, 98, 0, 0, 4, 0].pack('C*')
      expect(UEL.decode(compiled_integer)).to eq integer
    end

    it 'handles nil' do
      compiled_nil = [131, 106].pack('C*')
      expect(UEL.decode(compiled_nil)).to eq []
    end

    # TODO: Check sign byte
    it 'handles small bignums' do
      num = 1 << 100
      compiled_small_bignum = [131, 110, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16].pack('C*')
      expect(UEL.decode(compiled_small_bignum)).to eq (1 << 100)
    end

    # TODO: Check sign byte
    it 'handles large bignums' do
      num = 1 << 2040
      compiled_large_bignum = ([131, 111, 0, 0, 1, 0] + ([0] * 256) + [1]).pack('C*')
      expect(UEL.decode(compiled_large_bignum)).to eq num
    end

    it 'handles new floats' do
      compiled_new_float = [131, 70, 64, 254, 36, 12, 159, 190, 118, 201].pack('C*')
      expect(UEL.decode(compiled_new_float)).to eq 123456.789
    end

    it 'handles arrays with non NIL tails' do
      compiled_array = [131, 108, 0, 0, 0, 3, 70, 64, 0, 204, 204, 204, 204, 204, 205, 97, 3, 97, 4, 97, 42].pack('C*')
        expect(UEL.decode(compiled_array)).to eq [2.1, 3, 4, 42]
    end

    it 'handles arrays with NIL tails' do
      compiled_array = [131, 108, 0, 0, 0, 3, 70, 64, 0, 204, 204, 204, 204, 204, 205, 97, 3, 97, 4, 106].pack('C*')
      expect(UEL.decode(compiled_array)).to eq [2.1, 3, 4]
    end

    it 'handles maps' do
      compiled_map = [131, 116, 0, 0, 0, 2, 97, 1, 70, 63, 241, 153, 153, 153, 153, 153, 154, 97, 2,
  110, 11, 0, 21, 95, 4, 124, 159, 177, 227, 242, 253, 30, 102].pack('C*')
      expect(UEL.decode(compiled_map)).to eq ({1 => 1.1, 2 => 123456789123456789123456789})
    end

    it 'handles binary' do
      compiled_binary = [131, 109, 0, 0, 0, 3, 102, 111, 111].pack('C*')
      expect(UEL.decode(compiled_binary)).to eq "foo"
    end

    it 'handles small tuples' do
      compiled_tuple = [131, 104, 2, 97, 1, 97, 2].pack('C*')
      expect(UEL.decode(compiled_tuple)).to eq [1, 2]
    end

    it 'handles large tuples' do
      compiled_tuple = [131, 105, 0, 0, 0, 2, 97, 1, 97, 2].pack('C*')
      expect(UEL.decode(compiled_tuple)).to eq [1, 2]
    end

    it 'handles atoms' do
      compiled_atom = [131, 100, 0, 3, ?n.ord, ?i.ord, ?l.ord].pack('C*')
      expect(UEL.decode(compiled_atom)).to eq :nil
    end

    it 'handles utf-8 atoms' do
      compiled_atom = [131, 118, 0, 15, 227, 129, 147, 227, 130, 147, 227, 129, 171, 227, 129, 161, 227, 129, 175].pack('C*')
      expect(UEL.decode(compiled_atom)).to eq :"こんにちは"
    end

    it 'handles small atoms' do
      compiled_atom = [131, 115, 3, ?n.ord, ?i.ord, ?l.ord].pack('C*')
      expect(UEL.decode(compiled_atom)).to eq :nil
    end

    it 'handles small utf-8 atoms' do
      compiled_atom = [131, 119, 15, 227, 129, 147, 227, 130, 147, 227, 129, 171, 227, 129, 161, 227, 129, 175].pack('C*')
      expect(UEL.decode(compiled_atom)).to eq :"こんにちは"
    end

    it 'raises on an unknown term id' do
      compiled_err = [131, 150].pack('C*')
      expect { UEL.decode(compiled_err) }.to raise_error(NotImplementedError)
    end

    it 'decompresses compressed data' do
      compiled_data = [131, 80, 0, 0, 1, 6, 120, 156, 203, 103, 96, 96, 100, 24, 233, 128, 17, 0, 115, 164, 0, 114].pack('C*')
      expect(UEL.decode(compiled_data)).to eq (1 << 2040)
    end
  end

  describe 'encode' do
    it 'encodes small integers' do
      compiled_data = [131, 97, 13].pack('C*')
      expect(UEL.encode(13)).to eq compiled_data
    end

    it 'encodes integers' do
      compiled_data = [131, 98, 0, 0, 255, 255].pack('C*')
      expect(UEL.encode(0xFFFF)).to eq compiled_data
    end

    it 'encodes floats' do
      compiled_data = [131, 70, 63, 243, 192, 131, 18, 110, 151, 141].pack('C*')
      expect(UEL.encode(1.2345)).to eq compiled_data
    end

    it 'encodes small bignum' do
      bignum = 123456789123456789
      compiled_bignum = [131, 110, 8, 0, 21, 95, 208, 172, 75, 155, 182, 1].pack('C*')
      expect(UEL.encode(bignum)).to eq compiled_bignum
    end
    
    it 'encodes large bignum' do
      huge_num = 1 << (255 * 8)
      compiled_huge = ([131, 111, 0, 0, 1, 0, 0] + ([0] * 255) + [1]).pack('C*')
      expect(UEL.encode(huge_num)).to eq compiled_huge
    end

    it 'encodes nil to a small atom' do
      compiled_nil = [131, 119, 3, 110, 105, 108].pack('C*')
      expect(UEL.encode(nil)).to eq compiled_nil
    end

    it 'encodes small symbols' do
      small_sym = :hello
      compiled_sym = [131, 119, 5, 104, 101, 108, 108, 111].pack('C*')
      expect(UEL.encode(small_sym)).to eq compiled_sym
    end

    it 'encodes larger symbols' do
      large_sym = :"#{'a' * 0x100}"
      compiled_sym = ([131, 118, 1, 0] + (['a'.ord] * 0x100)).pack('C*')
      expect(UEL.encode(large_sym)).to eq compiled_sym
    end

    it 'encodes lists' do
      list = [1, 2, 3]
      compiled_list = [131, 108, 0, 0, 0, 3, 97, 1, 97, 2, 97, 3, 106].pack('C*')
      expect(UEL.encode(list)).to eq compiled_list
    end

    it 'encodes strings' do
      string = 'foo'
      compiled_string = [131, 109, 0, 0, 0, 3].pack('C*') + string
      expect(UEL.encode('foo')).to eq compiled_string
    end

    it 'encodes hashes' do
      hash = {'foo' => 1, :bar => 1.1}
      compiled_map = [131, 116, 0, 0, 0, 2, 109, 0, 0, 0, 3, 102, 111, 111, 97, 1, 119, 3, 98, 97, 114, 70, 63, 241, 153, 153, 153, 153, 153, 154].pack('C*')
      expect(UEL.encode(hash)).to eq compiled_map
    end
  end
end
