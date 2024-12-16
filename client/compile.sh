export libq_include_dir=$(pg_config --includedir)
export libq_lib_dir=$(pg_config --libdir)
export websockets_include_dir="-I/opt/homebrew/opt/libwebsockets/include"
export websockets_dir_lib="-L/opt/homebrew/opt/libwebsockets/lib"
export openssl_include_dir="-I/opt/homebrew/opt/openssl/include"
export openssl_lib_dir="-L//opt/homebrew/opt/openssl/lib"
export uv_include_dir="-I/opt/homebrew/opt/libuv/include"
export uv_lib_dir="-L/opt/homebrew/opt/libuv/lib"
export json_c_include_dir="-I/opt/homebrew/opt/json-c/include"
export json_c_lib_dir="-L/opt/homebrew/opt/json-c/lib"

make 