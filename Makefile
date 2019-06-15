test: 	./build/test/basic_test 				\
		./build/test/device_impl_test			\
		./build/test/global_ptr_impl_test		\
		./build/test/global_ptr_test
	./build/test/basic_test
	./build/test/device_impl_test
	./build/test/global_ptr_impl_test
	./build/test/global_ptr_test

./build/test/basic_test: ./src/sample.cpp		\
		./build/test
	g++ -std=c++17 -g ./src/sample.cpp -o ./build/test/basic_test -lOpenCL

./build/test/device_impl_test: 					\
		./src/device/device_impl.cpp 			\
		./src/device/device_impl_test.cpp 		\
		./src/device/device_impl.hpp			\
		./build/test
	g++ -std=c++17 -g ./src/device/device_impl.cpp ./src/device/device_impl_test.cpp -o ./build/test/device_impl_test -lOpenCL

./build/test/global_ptr_impl_test: 				\
		./src/device/device_impl.cpp 			\
		./src/device/device_impl.hpp			\
		./src/memory/global_ptr_impl.cpp 		\
		./src/memory/global_ptr_impl_test.cpp 	\
		./src/memory/global_ptr_impl.hpp		\
		./build/test
	g++ -std=c++17 -g ./src/device/device_impl.cpp ./src/memory/global_ptr_impl.cpp ./src/memory/global_ptr_impl_test.cpp -o ./build/test/global_ptr_impl_test -lOpenCL

./build/test/global_ptr_test:					\
		./src/device/device_impl.cpp 			\
		./src/device/device_impl.hpp			\
		./src/memory/global_ptr_impl.cpp 		\
		./src/memory/global_ptr_impl.hpp		\
		./src/memory/global_ptr.hpp				\
		./src/memory/global_ptr_test.cpp		\
		./build/test
	g++ -std=c++17 -g ./src/device/device_impl.cpp ./src/memory/global_ptr_impl.cpp ./src/memory/global_ptr_test.cpp -o ./build/test/global_ptr_test -lOpenCL

./build/test: ./build
	mkdir ./build/test

./build:
	mkdir ./build

clean:
	rm -rf ./build

