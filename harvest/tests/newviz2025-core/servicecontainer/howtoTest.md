cmake -S . -B build -G "Visual Studio 17 2022" -DBUILD_TESTING=ON
cmake --build build --config RelWithDebInfo --target servicecontainer_tests
ctest --test-dir build -C RelWithDebInfo --output-on-failure -R servicecontainer_tests
