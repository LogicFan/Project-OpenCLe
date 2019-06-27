test_cpp_dir = src/test
test_dir = build/test

test: 	$(test_dir)/basic_test 				\
		$(test_dir)/device_impl_test
	$(test_dir)/basic_test
	$(test_dir)/device_impl_test

./build/test/basic_test: $(test_cpp_dir)/basic_test.cpp		\
		build/test
	g++ -std=c++17 -g $(test_cpp_dir)/basic_test.cpp -o build/test/basic_test -lOpenCL

./build/device_impl.o:							\
		src/device/device_impl.cpp 			\
		src/device/device_impl.hpp			\
		build
	g++ -c -std=c++17 -g src/device/device_impl.cpp -o build/device_impl.o -lOpenCL

./build/test/device_impl_test: 					\
		build/device_impl.o 					\
		$(test_cpp_dir)/device_impl_test.cpp				\
		build/test
	g++ -std=c++17 -g ./build/device_impl.o $(test_cpp_dir)/device_impl_test.cpp -o $(test_dir)/device_impl_test -lOpenCL

./build/test: ./build
	mkdir ./build/test

./build:
	mkdir ./build

clean:
	rm -rf ./build

