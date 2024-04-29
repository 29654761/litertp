:set ABI=x64|win32
set ABI=%1
set CONFIG=release
set BUILD_DIR=./build/%ABI%-%CONFIG%
set INSTALL_DIR=./out/%ABI%-%CONFIG%


md "%BUILD_DIR%"

cmake . ^
-G "Visual Studio 17 2022" ^
-A %ABI% ^
-B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%CONFIG% -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
-DCMAKE_C_FLAGS_DEBUG="/MTd /Zi /Ob0 /Od /RTC1" ^
-DCMAKE_CXX_FLAGS_DEBUG="/MTd /Zi /Ob0 /Od /RTC1" ^
-DCMAKE_C_FLAGS_RELEASE="/MT /Zi /O2 /Ob1 /DNDEBUG" ^
-DCMAKE_CXX_FLAGS_RELEASE="/MT /Zi /O2 /Ob1 /DNDEBUG" ^
-DCMAKE_C_FLAGS_MINSIZEREL="/MT /O1 /Ob1 /DNDEBUG" ^
-DCMAKE_CXX_FLAGS_MINSIZEREL="/MT /O1 /Ob1 /DNDEBUG" ^
-DCMAKE_C_FLAGS_RELWITHDEBINFO="/MT /Zi /O2 /Ob1 /DNDEBUG" ^
-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/MT /Zi /O2 /Ob1 /DNDEBUG" ^
-DLITERTP_SSL=ON ^
-DLITERTP_SHARED=ON ^
-DLITERTP_STATIC=ON ^
-DOPENSSL_INCLUDE="E:\\cpp-build\\openssl\\out\\%ABI%\\include" ^
-DOPENSSL_LIB_CRYPTO="E:\\cpp-build\\openssl\\out\\%ABI%\\lib\\libcrypto.lib" ^
-DOPENSSL_LIB_SSL="E:\\cpp-build\\openssl\\out\\%ABI%\\lib\\libssl.lib" ^
-DSRTP_INCLUDE="E:\\cpp-build\\libsrtp\\out\\%ABI%-%CONFIG%\\include" ^
-DSRTP_LIB="E:\\cpp-build\\libsrtp\\out\\%ABI%-%CONFIG%\\lib\\srtp2.lib"

cmake --build "%BUILD_DIR%" --config %CONFIG%
cmake --install "%BUILD_DIR%" --config %CONFIG%

pause