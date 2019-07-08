test_cpp_dir = src/test
test_dir = build/test

# run test

test:	$(test_dir)/device_impl_test						\
		$(test_dir)/global_ptr_impl_test					\
		$(test_dir)/task_impl_test							\
		$(test_dir)/device_test
	$(test_dir)/device_impl_test
	$(test_dir)/global_ptr_impl_test
	$(test_dir)/task_impl_test
	$(test_dir)/device_test

# object file for each cpp file

build/device_impl.o:										\
		src/device/device_impl.cpp 							\
		src/device/device_impl.hpp							\
		build
	g++ -c -std=c++17 -g src/device/device_impl.cpp -o build/device_impl.o

build/device.o:												\
		src/device/device.cpp								\
		src/device/device.hpp								\
		build
	g++ -c -std=c++17 -g src/device/device.cpp -o build/device.o

build/global_ptr_impl.o:									\
		src/memory/global_ptr_impl.cpp						\
		src/memory/global_ptr_impl.hpp						\
		build
	g++ -c -std=c++17 -g src/memory/global_ptr_impl.cpp -o build/global_ptr_impl.o -lOpenCL

build/task_impl.o:											\
		src/task/task_impl.cpp								\
		src/task/task_impl.hpp								\
		build
	g++ -c -std=c++17 -g src/task/task_impl.cpp -o build/task_impl.o -lOpenCL

# combine all object files into single object file

bin/opencle.o:												\
		build/device_impl.o									\
		build/device.o 										\
		build/global_ptr_impl.o 							\
		build/task_impl.o									\
		bin
	ld -r -o bin/opencle.o build/device_impl.o build/device.o build/global_ptr_impl.o build/task_impl.o

# compile test

build/test/basic_test: $(test_cpp_dir)/basic_test.cpp		\
		build/test
	g++ -std=c++17 -g $(test_cpp_dir)/basic_test.cpp -o build/test/basic_test -lOpenCL

build/test/device_impl_test: 								\
		bin/opencle.o 										\
		$(test_cpp_dir)/device_impl_test.cpp				\
		build/test
	g++ -std=c++17 -g bin/opencle.o $(test_cpp_dir)/device_impl_test.cpp -o $(test_dir)/device_impl_test -lOpenCL

build/test/global_ptr_impl_test: 							\
		bin/opencle.o 										\
		$(test_cpp_dir)/global_ptr_impl_test.cpp			\
		build/test
	g++ -std=c++17 -g bin/opencle.o $(test_cpp_dir)/global_ptr_impl_test.cpp -o $(test_dir)/global_ptr_impl_test -lOpenCL

build/test/task_impl_test: 									\
		bin/opencle.o 										\
		$(test_cpp_dir)/task_impl_test.cpp					\
		build/test
	g++ -std=c++17 -g bin/opencle.o $(test_cpp_dir)/task_impl_test.cpp -o $(test_dir)/task_impl_test -lOpenCL

build/test/device_test:										\
		bin/opencle.o 										\
		$(test_cpp_dir)/device_test.cpp						\
		build/test
	g++ -std=c++17 -g bin/opencle.o $(test_cpp_dir)/device_test.cpp -o $(test_dir)/device_test -lOpenCL

# create folder

build/test: build
	mkdir build/test

build:
	mkdir build

bin:
	mkdir bin

# clean

clean:
	rm -rf build
	rm -rf bin

