rmdir /s /q build
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -A x64 .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
PAUSE