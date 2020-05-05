#include <netinet/in.h>
#include <endian.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include <string.h>
#include <stdlib.h>
#include "./extconf.h"
#include "./uel.h"



VALUE UELModule = Qnil;

void Init_uel() {
    UELModule = rb_define_module("UEL");
    rb_define_module_function(UELModule, "decode", uel_decode, 1);
    rb_define_module_function(UELModule, "encode", uel_encode, 1);
}

