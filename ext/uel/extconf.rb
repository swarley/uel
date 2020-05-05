require 'mkmf'

extension_name = 'uel'
dir_config(extension_name)
find_header('ruby.h')
find_header('endian.h')
find_header('stdlib.h')

if have_library('z')
  have_header('zlib.h')
end

create_header
create_makefile(extension_name)
