#ABI=armeabi-v7a,arm64-v8a
ABI=$1
TOOLCHAIN=/Applications/AndroidNDK11394342.app/Contents/NDK/build/cmake/android.toolchain.cmake
BUILD_DIR=./build/${ABI}
INSTALL_DIR=./out/${ABI}


mkdir -p ${BUILD_DIR}


cmake . \
-B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
-DCMAKE_CXX_FLAGS="--std=c++17" \
-DANDROID_ABI=${ABI} \
-DANDROID_PLATFORM=android-23 \
-DLITERTP_SSL=ON \
-DOPENSSL_INCLUDE="/Users/seastart/cpp/openssl/out/${ABI}/include" \
-DOPENSSL_LIB_CRYPTO="/Users/seastart/cpp/openssl/out/${ABI}/lib/libcrypto.a" \
-DOPENSSL_LIB_SSL="/Users/seastart/cpp/openssl/out/${ABI}/lib/libssl.a" \
-DSRTP_INCLUDE="/Users/seastart/cpp/libsrtp/out/${ABI}/include" \
-DSRTP_LIB="/Users/seastart/cpp/libsrtp/out/${ABI}/lib/libsrtp2.a"



cmake --build "${BUILD_DIR}"
cmake --install "${BUILD_DIR}"
