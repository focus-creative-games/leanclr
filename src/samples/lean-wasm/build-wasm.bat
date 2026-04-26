

call emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=Debug
call emmake cmake --build build-wasm --parallel

