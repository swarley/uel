#ifndef UNTITLED_ETF_LIB_H
#define UNTITLED_ETF_LIB_H

#define UEL_ETF_VERSION_NUMBER 131

/* Supported Terms */
#define UEL_ETF_NEW_FLOAT_ID 70
#define UEL_ETF_BIT_BINARY_ID 77
#define UEL_ETF_ZLIB_ID 80
#define UEL_ETF_SMALL_INTEGER_ID 97
#define UEL_ETF_INTEGER_ID 98
#define UEL_ETF_FLOAT_ID 99
#define UEL_ETF_ATOM_ID 100
#define UEL_ETF_SMALL_TUPLE_ID 104
#define UEL_ETF_LARGE_TUPLE_ID 105
#define UEL_ETF_NIL_ID 106
#define UEL_ETF_STRING_ID 107
#define UEL_ETF_LIST_ID 108
#define UEL_ETF_BINARY_ID 109
#define UEL_ETF_SMALL_BIGNUM_ID 110
#define UEL_ETF_LARGE_BIGNUM_ID 111
#define UEL_ETF_SMALL_ATOM_ID 115
#define UEL_ETF_MAP_ID 116
#define UEL_ETF_ATOM_UTF8_ID 118
#define UEL_ETF_SMALL_ATOM_UTF8_ID 119


/* Unsupported terms */
#define ETF_ATOM_CACHE_REF_ID 82
#define ETF_NEW_PID_ID 88
#define ETF_NEW_PORT_ID 89
#define ETF_NEWER_REFERENCE_ID 90
#define ETF_REFERENCE_ID 101
#define ETF_PORT_ID 102
#define ETF_PID_ID 103
#define ETF_NEW_FUN_ID 112
#define ETF_EXPORT_ID 113
#define ETF_NEW_REFERENCE_ID 114
#define ETF_FUN_ID 117


struct uel_bert_data {
    uint8_t *buff;
    uint8_t *end;
    uint32_t length;
};

#define UEL_DATA_LEN(etf_data) (etf_data->end - etf_data->buff)
#define UEL_DATA_SET_LEN(etf_data, len) (etf_data->end = etf_data->buff + (len))
#define UEL_SET_CAST(type, data, value) *((type *)(data)) = (type)value;
#define UEL_SET_INT32(data, value) UEL_SET_CAST(int32_t, (data), htobe32(value))
#define UEL_SET_INT16(data, value) UEL_SET_CAST(int16_t, (data), htobe16(value))
#define UEL_SET_UINT32(data, value) UEL_SET_CAST(uint32_t, (data), htobe32(value))
#define UEL_SET_UINT16(data, value) UEL_SET_CAST(uint16_t, (data), htobe16(value))


typedef union {
    double d;
    uint64_t u;
    char buff[8];
} DOUBLE_SWAPPER;

void Init_uel();

void uel_destroy_bert_data(struct uel_bert_data*data);

/* decode */
VALUE uel_decode(VALUE self, VALUE data);
VALUE uel_decode_term(struct uel_bert_data *data);
void uel_check_bounds(struct uel_bert_data *data, size_t size);
uint8_t uel_read8(struct uel_bert_data *data);
uint16_t uel_read16(struct uel_bert_data *data);
uint32_t uel_read32(struct uel_bert_data *data);
VALUE uel_read_small_integer(struct uel_bert_data *data);
VALUE uel_read_integer(struct uel_bert_data *data);
VALUE uel_read_float(struct uel_bert_data *data);
VALUE uel_read_nil(struct uel_bert_data *data);
VALUE uel_read_small_bignum(struct uel_bert_data *data);
VALUE uel_read_large_bignum(struct uel_bert_data *data);
VALUE uel_read_new_float(struct uel_bert_data *data);
VALUE uel_read_list(struct uel_bert_data *data);
VALUE uel_read_map(struct uel_bert_data *data);
VALUE uel_read_binary(struct uel_bert_data *data);
VALUE uel_read_zlib(struct uel_bert_data *data);
VALUE uel_read_small_tuple(struct uel_bert_data *data);
VALUE uel_read_large_tuple(struct uel_bert_data *data);
VALUE uel_read_atom(struct uel_bert_data *data);
VALUE uel_read_small_atom(struct uel_bert_data *data);
VALUE uel_read_atom_utf8(struct uel_bert_data *data);
VALUE uel_read_small_atom_utf8(struct uel_bert_data *data);

/* encode */
VALUE uel_encode(VALUE self, VALUE value);
struct uel_bert_data *uel_encode_term(VALUE value);
struct uel_bert_data *uel_encode_fixnum(VALUE value);
struct uel_bert_data *uel_encode_float(VALUE value);
struct uel_bert_data *uel_encode_bignum(VALUE value);
struct uel_bert_data *uel_encode_array(VALUE value);
struct uel_bert_data *uel_encode_hash(VALUE value);
struct uel_bert_data *uel_encode_struct(VALUE value);
struct uel_bert_data *uel_encode_array(VALUE value);
struct uel_bert_data *uel_encode_symbol(VALUE value);
struct uel_bert_data *uel_encode_string(VALUE value);
struct uel_bert_data *uel_encode_ptr_atom(const char *name, uint16_t len);
struct uel_bert_data *uel_encode_ptr_small_atom(const char *name, uint8_t len);

/* ruby symbols */
VALUE rb_hash_new_with_size(st_index_t size);
VALUE rb_hash_keys(VALUE hash);

#endif