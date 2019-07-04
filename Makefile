test_cpp_dir = src/test
test_dir = build/test

test: 	$(test_dir)/basic_test 								\
		$(test_dir)/device_impl_test						\
		$(test_dir)/global_ptr_impl_test					\
		$(test_dir)/task_impl_test
	$(test_dir)/basic_test
	$(test_dir)/device_impl_test
	$(test_dir)/global_ptr_impl_test
	$(test_dir)/task_impl_test

build/test/basic_test: $(test_cpp_dir)/basic_test.cpp		\
		build/test
	g++ -std=c++17 -g $(test_cpp_dir)/basic_test.cpp -o build/test/basic_test -lOpenCL

build/device_impl.o:										\
		src/device/device_impl.cpp 							\
		src/device/device_impl.hpp							\
		build
	g++ -c -std=c++17 -g src/device/device_impl.cpp -o build/device_impl.o -lOpenCL

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

build/test/device_impl_test: 								\
		build/device_impl.o 								\
		$(test_cpp_dir)/device_impl_test.cpp				\
		build/test
	g++ -std=c++17 -g build/device_impl.o $(test_cpp_dir)/device_impl_test.cpp -o $(test_dir)/device_impl_test -lOpenCL

build/test/global_ptr_impl_test: 							\
		build/device_impl.o									\
		build/global_ptr_impl.o 							\
		$(test_cpp_dir)/global_ptr_impl_test.cpp			\
		build/test
	g++ -std=c++17 -g build/global_ptr_impl.o build/device_impl.o $(test_cpp_dir)/global_ptr_impl_test.cpp -o $(test_dir)/global_ptr_impl_test -lOpenCL

build/test/task_impl_test: 										\
		build/device_impl.o									\
		build/global_ptr_impl.o 							\
		build/task_impl.o 									\
		$(test_cpp_dir)/task_impl_test.cpp					\
		build/test
	g++ -std=c++17 -g build/global_ptr_impl.o build/device_impl.o build/task_impl.o $(test_cpp_dir)/task_impl_test.cpp -o $(test_dir)/task_impl_test -lOpenCL

build/test: ./build
	mkdir ./build/test

build:
	mkdir ./build

clean:
	rm -rf ./build

