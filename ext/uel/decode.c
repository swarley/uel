#include <netinet/in.h>
#include <endian.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include <string.h>
#include <stdlib.h>
#include "./extconf.h"
#include "./uel.h"

#if HAVE_ZLIB_H
#include <zlib.h>
#endif

VALUE uel_decode(VALUE self, VALUE data) {
    struct uel_bert_data bert_data;
 
    Check_Type(data, T_STRING);

    bert_data.buff = (uint8_t *)RSTRING_PTR(data);
    bert_data.end = (uint8_t *)(bert_data.buff + RSTRING_LEN(data));

    if (uel_read8(&bert_data) != UEL_ETF_VERSION_NUMBER)
        rb_raise(rb_eNotImpError, "Unknown ETF version");

    return uel_decode_term(&bert_data);
}

VALUE uel_decode_term(struct uel_bert_data *data) {    
    switch(uel_read8(data)) {
        case UEL_ETF_SMALL_INTEGER_ID:
            return uel_read_small_integer(data);
        case UEL_ETF_INTEGER_ID:
            return uel_read_integer(data);
        case UEL_ETF_FLOAT_ID:
            return uel_read_float(data);
        case UEL_ETF_SMALL_BIGNUM_ID:
            return uel_read_small_bignum(data);
        case UEL_ETF_LARGE_BIGNUM_ID:
            return uel_read_large_bignum(data);
        case UEL_ETF_NEW_FLOAT_ID:
            return uel_read_new_float(data);
        case UEL_ETF_NIL_ID:
            return uel_read_nil(data);
        case UEL_ETF_LIST_ID:
            return uel_read_list(data);
        case UEL_ETF_MAP_ID:
            return uel_read_map(data);
        case UEL_ETF_BINARY_ID:
            return uel_read_binary(data);
        case UEL_ETF_SMALL_TUPLE_ID:
            return uel_read_small_tuple(data);
        case UEL_ETF_LARGE_TUPLE_ID:
            return uel_read_large_tuple(data);
        case UEL_ETF_ATOM_ID:
        case UEL_ETF_ATOM_UTF8_ID:
            return uel_read_atom(data);
        case UEL_ETF_SMALL_ATOM_ID:
        case UEL_ETF_SMALL_ATOM_UTF8_ID:
            return uel_read_small_atom(data);
#ifdef HAVE_ZLIB_H
        case UEL_ETF_ZLIB_ID:
            return uel_read_zlib(data);
#endif
        default:
            rb_raise(rb_eNotImpError, "Unable to process data type: %hu", *(data->buff - 1));
            return Qnil;
    }
}

inline void uel_check_bounds(struct uel_bert_data *data, size_t size) {
    if(data->buff + size >  data->end)
        rb_raise(rb_eRangeError, "end of buffer while taking %li", size);
}

inline uint8_t uel_read8(struct uel_bert_data *data) {
    uel_check_bounds(data, 1);
    return *data->buff++;
}

inline uint16_t uel_read16(struct uel_bert_data *data) {
    uint16_t value;

    uel_check_bounds(data, 2);
    value  = *(uint16_t *)data->buff;
    data->buff += sizeof(uint16_t);
    return ntohs(value);
}

inline uint32_t uel_read32(struct uel_bert_data *data) {
    uint32_t value;

    uel_check_bounds(data, 4);
    value =  *(uint32_t *)data->buff;
    data->buff += sizeof(uint32_t);
    return ntohl(value);
}

inline VALUE uel_read_small_integer(struct uel_bert_data *data) {
    return UINT2NUM((uint8_t) uel_read8(data));
}

inline VALUE uel_read_integer(struct uel_bert_data *data) {
    return INT2NUM(uel_read32(data));
}

VALUE uel_read_float(struct uel_bert_data *data) {
    double value;

    uel_check_bounds(data, 31);
    sscanf((char *)data->buff, "%31lf", &value);
    data->buff += 31;
    return DBL2NUM(value);
}

inline VALUE uel_read_nil(struct uel_bert_data *data) {
    return rb_ary_new();
}

VALUE uel_read_small_bignum(struct uel_bert_data *data) {
    uint8_t size;
    uint8_t sign;
    
    size = uel_read8(data);
    sign = uel_read8(data);

    uel_check_bounds(data, size);
    data->buff += size;

    return rb_integer_unpack(data->buff - size, size, 1, 0, INTEGER_PACK_LITTLE_ENDIAN | (sign * INTEGER_PACK_NEGATIVE));
}

VALUE uel_read_large_bignum(struct uel_bert_data *data) {
    uint32_t size;
    uint8_t sign;

    size = uel_read32(data);
    sign = uel_read8(data);

    uel_check_bounds(data, size);
    data->buff += size;
    return rb_integer_unpack(data->buff - size, size, 1, 0, INTEGER_PACK_LITTLE_ENDIAN | (sign * INTEGER_PACK_NEGATIVE));
}

VALUE uel_read_new_float(struct uel_bert_data *data) {
    DOUBLE_SWAPPER tmp;

    memcpy(tmp.buff, data->buff, sizeof(double));
    data->buff += sizeof(double);
    tmp.u = be64toh(tmp.u);

    return DBL2NUM(tmp.d);
}

inline void uel_read_n_terms(struct uel_bert_data *data, VALUE *values, uint32_t count) {
    for(uint32_t index = 0; index < count; index++) {
        values[index] = uel_decode_term(data);
    }
}

VALUE uel_read_list(struct uel_bert_data *data) {
    uint32_t length;
    VALUE* values;

    length = uel_read32(data);
    // Extra value to account for Tail
    values = (VALUE *)malloc(sizeof(VALUE) * (length + 1));

    uel_read_n_terms(data, values, length);

    uel_check_bounds(data, 1);
    if (*data->buff == UEL_ETF_NIL_ID) {
        data->buff += 1;
        return rb_ary_new_from_values(length, values);
    }
    else {
        values[length] = uel_decode_term(data);
        return rb_ary_new_from_values(length + 1, values);
    }

}

VALUE uel_read_map(struct uel_bert_data *data) {
    uint32_t arity;
    VALUE hash;
    VALUE key;
    VALUE value;

    arity = uel_read32(data);
    hash = rb_hash_new();
    for (uint32_t index = 0; index < arity; index++) {
        key = uel_decode_term(data);
        value = uel_decode_term(data);
        rb_hash_aset(hash, key, value);
    }

    return hash;
}

VALUE uel_read_binary(struct uel_bert_data *data) {
    uint32_t length;
    
    length = uel_read32(data);
    uel_check_bounds(data, length);
    data->buff += length;
    return rb_str_new((const char *)data->buff - length, length);
}

VALUE uel_read_small_tuple(struct uel_bert_data *data) {
    uint8_t length;
    VALUE *values;

    length = uel_read8(data);
    values = (VALUE *)malloc(sizeof(VALUE) * length);

    uel_read_n_terms(data, values, length);

    return rb_ary_new_from_values(length, values);
}

VALUE uel_read_large_tuple(struct uel_bert_data *data) {
    uint32_t length;
    VALUE *values;

    length = uel_read32(data);
    values = (VALUE *)malloc(sizeof(VALUE) * length);

    uel_read_n_terms(data, values, length);

    return rb_ary_new_from_values(length, values);
}

VALUE uel_read_symbol(struct uel_bert_data *data, uint16_t length) {
    VALUE string;

    string = rb_enc_str_new((const char *)data->buff, length, rb_utf8_encoding());
    data->buff += length;
    return rb_to_symbol(string);
}

VALUE uel_read_atom(struct uel_bert_data *data) {
    return uel_read_symbol(data, uel_read16(data));
}

VALUE uel_read_small_atom(struct uel_bert_data *data) {
    return uel_read_symbol(data, uel_read8(data));
}

/* 
    TODO: 
        SMALL_TUPLE
        LARGE_TUPLE
        STRING
        ATOM_SHIT
*/

#if HAVE_ZLIB_H
VALUE uel_read_zlib(struct uel_bert_data *data) {
    uint32_t inflate_length;
    int inflate_status_code;
    z_stream strm;
    unsigned char *out;
    struct uel_bert_data new_data;

    inflate_length = uel_read32(data);
    out = malloc(inflate_length * sizeof(uint8_t));

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = (uint32_t)UEL_DATA_LEN(data);
    strm.next_in = data->buff;
    strm.avail_out = inflate_length;
    strm.next_out = out;

    inflateInit(&strm);
    inflate(&strm, Z_NO_FLUSH);
    inflate_status_code = inflateEnd(&strm);

    new_data.buff = out;
    new_data.end = out + inflate_length;

    if (inflate_status_code != Z_OK) {
        rb_raise(rb_eTypeError, "Error inflating compressed data: (%i) %s", inflate_status_code, strm.msg);
    }

    return uel_decode_term(&new_data);
}
#endif