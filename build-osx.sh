

#ABI=MAC_UNIVERSAL,OS64COMBINED
ABI=$1
CONFIG=Release
TOOLCHAIN=/opt/cmake/ios.toolchain.cmake
BUILD_DIR=./build/${ABI}
INSTALL_DIR=./out/${ABI}
LIBS_ROOT=/Users/seastart/cpp

mkdir -p ${BUILD_DIR}

cmake . \
-G "Xcode" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=${CONFIG} -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
-DCMAKE_CXX_FLAGS="--std=c++17" \
-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
-DPLATFORM=${ABI} \
-DLITERTP_SSL=ON \
-DLITERTP_SHARED=OFF \
-DOPENSSL_INCLUDE="/Users/seastart/cpp/openssl/out/${ABI}/include" \
-DOPENSSL_LIB_CRYPTO="/Users/seastart/cpp/openssl/out/${ABI}/lib/libcrypto.a" \
-DOPENSSL_LIB_SSL="/Users/seastart/cpp/openssl/out/${ABI}/lib/libssl.a" \
-DSRTP_INCLUDE="/Users/seastart/cpp/libsrtp/out/${ABI}/include" \
-DSRTP_LIB="/Users/seastart/cpp/libsrtp/out/${ABI}/lib/libsrtp2.a"

cmake --build "${BUILD_DIR}" --config ${CONFIG}
cmake --install "${BUILD_DIR}" --config ${CONFIG}
