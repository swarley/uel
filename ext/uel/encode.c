#include <netinet/in.h>
#include <endian.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include <string.h>
#include <stdlib.h>
#include "./extconf.h"
#include "./uel.h"

VALUE uel_encode(VALUE self, VALUE value) {
    uint8_t *buff;
    VALUE ret_val;
    struct uel_bert_data *term;
    uint32_t term_len;

    term = uel_encode_term(value);
    if (term == NULL) {
        rb_raise(rb_eStandardError, "Error decoding term");
        return Qnil;
    }


    term_len = UEL_DATA_LEN(term);
    buff = malloc(sizeof(uint8_t) * (term_len + 1));
    buff[0] = 131;
    memcpy(&buff[1], term->buff, term_len);
    ret_val = rb_str_new((const char *)buff, (term_len + 1));
    
    free(buff);
    uel_destroy_bert_data(term);

    return ret_val;
}


struct uel_bert_data *uel_encode_term(VALUE value) {    
    switch (TYPE(value)) {
        case T_FIXNUM:
            if (FIX2LONG(value) <= 0xFFFFFFFF)
                return uel_encode_fixnum(value);
            else
                return uel_encode_bignum(value);
        case T_FLOAT:
            return uel_encode_float(value);
        case T_BIGNUM:
            return uel_encode_bignum(value);
        case T_NIL:
            return uel_encode_ptr_small_atom("nil", 3);
        case T_TRUE:
            return uel_encode_ptr_small_atom("true", 4);
        case T_FALSE:
            return uel_encode_ptr_small_atom("false", 5);
        case T_SYMBOL:
            return uel_encode_symbol(value);
        case T_ARRAY:
            return uel_encode_array(value);
        case T_HASH:
            return uel_encode_hash(value);
        case T_STRING:
            return uel_encode_string(value);
        default:
            /* TODO: Include class name
                rb_raise(rb_eNotImpError, "Type not implemented %s", rb_class2name(RBASIC_CLASS(value)));
            */
            return NULL;
    }
}

inline void uel_destroy_bert_data(struct uel_bert_data* data) {
    free(data->buff);
    free(data);
}

struct uel_bert_data *uel_encode_fixnum(VALUE fixnum) {
    long l;
    struct uel_bert_data *data = NULL;

    l = FIX2LONG(fixnum);
    if (l <= 0xFF) {
        data = malloc(sizeof(struct uel_bert_data));
        data->buff = malloc(sizeof(uint8_t) * 2);
        data->buff[0] = UEL_ETF_SMALL_INTEGER_ID;
        data->buff[1] = (uint8_t)l;
        UEL_DATA_SET_LEN(data, 2);
    } else {
        data = malloc(sizeof(struct uel_bert_data));
        data->buff = malloc(sizeof(uint8_t) * 5);
        data->buff[0] = UEL_ETF_INTEGER_ID;

        UEL_SET_INT32(data->buff + 1, l);
        UEL_DATA_SET_LEN(data, 5);
    }

    return data;
}

struct uel_bert_data *uel_encode_float(VALUE value) {
    struct uel_bert_data *data = malloc(sizeof(struct uel_bert_data));
    DOUBLE_SWAPPER swapper;

    data->buff = (uint8_t *)malloc(sizeof(uint8_t) * 9);
    UEL_DATA_SET_LEN(data, 9);

    swapper.d = NUM2DBL(value);
    swapper.u = htobe64(swapper.u);

    *data->buff = UEL_ETF_NEW_FLOAT_ID;
    memcpy(data->buff + 1, swapper.buff, sizeof(double));

    return data;
}

struct uel_bert_data *uel_encode_bignum(VALUE value) {
    size_t byte_count;
    struct uel_bert_data *data = NULL;

    data = malloc(sizeof(struct uel_bert_data));
    byte_count = rb_absint_size(value, NULL);
    if (byte_count <= 0xFF) {
        // id byte | n byte | sign byte | data
        data->buff = malloc(sizeof(uint8_t) * (3 + byte_count));
        data->buff[0] = UEL_ETF_SMALL_BIGNUM_ID;
        data->buff[1] = (uint8_t)byte_count;
        data->buff[2] = FIX2LONG(value) >= 0 ? 0 : 1;
        UEL_DATA_SET_LEN(data, 3 + byte_count);

        rb_integer_pack(value, data->buff + 3, byte_count, sizeof(uint8_t), 0, INTEGER_PACK_LITTLE_ENDIAN);
    }
    else {
        // id byte | 4 byte n | sign byte | data
        data->buff = malloc(sizeof(uint8_t) * (2 + byte_count) + sizeof(uint32_t));
        data->buff[0] = UEL_ETF_LARGE_BIGNUM_ID;
        UEL_SET_INT32(data->buff + 1, byte_count);
        data->buff[5] = RBIGNUM_SIGN(value) ? 0 : 1;
        UEL_DATA_SET_LEN(data, 6 + byte_count);

        rb_integer_pack(value, data->buff + 6, byte_count, sizeof(uint8_t), 0, INTEGER_PACK_LITTLE_ENDIAN);
    }
    return data;
}

struct uel_bert_data *uel_encode_ptr_atom(const char *name, uint16_t len) {
    struct uel_bert_data *data = malloc(sizeof(struct uel_bert_data));
    
    data->buff = malloc(sizeof(uint8_t) * (len + 3));
    data->buff[0] = UEL_ETF_ATOM_UTF8_ID;
    UEL_SET_UINT16(data->buff + 1, len);
    memcpy(data->buff + 3, name, len);
    UEL_DATA_SET_LEN(data, 3 + len);

    return data;
}

struct uel_bert_data *uel_encode_ptr_small_atom(const char *name, uint8_t len) {
    struct uel_bert_data *data = malloc(sizeof(struct uel_bert_data));

    data->buff = malloc(sizeof(uint8_t) * (len + 2));
    data->buff[0] = UEL_ETF_SMALL_ATOM_UTF8_ID;
    data->buff[1] = len;
    memcpy(data->buff + 2, name, len);
    UEL_DATA_SET_LEN(data, 2 + len);

    return data;
}

struct uel_bert_data *uel_encode_symbol(VALUE value) {
    const char *name;
    uint16_t len;

    name = rb_id2name(rb_to_id(value));
    len = strlen(name);

    if (len > 0xFF)
        return uel_encode_ptr_atom(name, len);
    else
        return uel_encode_ptr_small_atom(name, len);
}

struct uel_bert_data *uel_encode_array(VALUE value) {
    uint32_t data_size = 0;
    // Account for id tag and length
    uint32_t offset = 5;
    uint32_t elem_count = RARRAY_LEN(value);
    struct uel_bert_data **data_ary = malloc(sizeof(struct uel_bert_data *) * elem_count);
    struct uel_bert_data *element;
    struct uel_bert_data *ret_data;

    for (uint32_t i = 0; i < elem_count; i++) {
        element = uel_encode_term(rb_ary_entry(value, i));
        if (element == NULL) {
            for (uint32_t j = 0; j <= i; j++)
                uel_destroy_bert_data(data_ary[j]);
            free(data_ary);
            return NULL;
        }
        data_size += UEL_DATA_LEN(element);
        data_ary[i] = element;
    }

    ret_data = malloc(sizeof(struct uel_bert_data));
    ret_data->buff = malloc(sizeof(uint8_t) * (data_size + 6));
    ret_data->buff[0] = UEL_ETF_LIST_ID;
    UEL_SET_UINT32(ret_data->buff + 1, elem_count);
    UEL_DATA_SET_LEN(ret_data, data_size + 6);

    for (uint32_t i = 0; i < elem_count; i++) {
        memcpy(ret_data->buff + offset, data_ary[i]->buff, UEL_DATA_LEN(data_ary[i]));
        offset += UEL_DATA_LEN(data_ary[i]);
        uel_destroy_bert_data(data_ary[i]);
    }
    ret_data->buff[offset] = UEL_ETF_NIL_ID;
    free(data_ary);

    return ret_data;
}

struct uel_bert_data *uel_encode_string(VALUE value) {
    struct uel_bert_data *data;
    size_t len = RSTRING_LEN(value);

    data = malloc(sizeof(struct uel_bert_data));
    data->buff = malloc(sizeof(uint8_t) * (len + 5));
    UEL_DATA_SET_LEN(data, len + 5);

    data->buff[0] = UEL_ETF_BINARY_ID;
    UEL_SET_UINT32(data->buff + 1, len);
    memcpy(data->buff + 5, StringValuePtr(value), len);

    return data;
}

struct uel_bert_data *uel_encode_hash(VALUE value) {
    struct uel_bert_data *data;
    struct uel_bert_data **data_ary;
    uint32_t hash_size;
    int32_t data_index = 0;
    uint32_t key_index = 0;
    uint32_t data_size = 0;
    uint32_t offset;
    VALUE keys;
    VALUE key;

    keys = rb_funcall(value, rb_intern("keys"), 0);
    hash_size = RARRAY_LEN(keys);
    data = malloc(sizeof(struct uel_bert_data));
    data_ary = malloc(sizeof(struct uel_bert_data *) * (hash_size * 2));

    for (; key_index < hash_size; key_index++) {
        key = rb_ary_entry(keys, key_index);
        data_ary[data_index++] = uel_encode_term(key);
        data_ary[data_index++] = uel_encode_term(rb_hash_lookup(value, key));

        if (data_ary[data_index - 1] == NULL || data_ary[data_index - 2] == NULL) {
            while(--data_index >= 0) {
                if (data_ary[data_index] != NULL)
                    uel_destroy_bert_data(data_ary[data_index]);
            }
            free(data);
            free(data_ary);
            return NULL;
        }

        data_size += UEL_DATA_LEN(data_ary[data_index - 1]);
        data_size += UEL_DATA_LEN(data_ary[data_index - 2]);
    }
    
    data->buff = (uint8_t *)malloc(sizeof(uint8_t) * (5 + data_size));
    data->buff[0] = UEL_ETF_MAP_ID;
    UEL_SET_UINT32(data->buff + 1, hash_size);
    UEL_DATA_SET_LEN(data, 5 + data_size);
    
    offset = 0;
    for (int index = 0; index < data_index; index++) {
        memcpy(data->buff + 5 + offset, data_ary[index]->buff, UEL_DATA_LEN(data_ary[index]));
        offset += UEL_DATA_LEN(data_ary[index]); 
        uel_destroy_bert_data(data_ary[index]);
    }

    free(data_ary);

    return data;
}
