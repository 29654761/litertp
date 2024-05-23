#ABI=armeabi-v7a,arm64-v8a
ABI=linux
CONFIG=release
ROOT_DIR=/cxx-build
BUILD_DIR=./build/${ABI}-${CONFIG}
INSTALL_DIR=./out/${ABI}-${CONFIG}


mkdir -p ${BUILD_DIR}


cmake . \
-B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=${CONFIG} -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
-DCMAKE_CXX_FLAGS="--std=c++17" \
-DLITERTP_SSL=ON \
-DOPENSSL_INCLUDE="${ROOT_DIR}/openssl/out/${ABI}-${CONFIG}/include" \
-DOPENSSL_LIB_CRYPTO="${ROOT_DIR}/openssl/out/${ABI}-${CONFIG}/lib64/libcrypto.a" \
-DOPENSSL_LIB_SSL="${ROOT_DIR}/openssl/out/${ABI}-${CONFIG}/lib64/libssl.a" \
-DSRTP_INCLUDE="${ROOT_DIR}/libsrtp/out/${ABI}-${CONFIG}/include" \
-DSRTP_LIB="${ROOT_DIR}/libsrtp/out/${ABI}-${CONFIG}/lib/libsrtp2.a"



cmake --build "${BUILD_DIR}"
cmake --install "${BUILD_DIR}"
