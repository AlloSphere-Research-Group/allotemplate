@REM cmake -DCMAKE_BUILD_TYPE=RELEASE -B build/Release -S .
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -B build/Release -S .
@REM cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -Wno-deprecated -DBUILD_EXAMPLES=0 -B build/Release -S .