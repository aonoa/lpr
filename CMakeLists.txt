cmake_minimum_required(VERSION 3.21)
project(lpr)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV 4.5.4 REQUIRED COMPONENTS core imgcodecs imgproc highgui)
FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

include_directories(third_party/ncnn/include/ncnn include src)
include_directories(third_party/mlpr/include)
include_directories(third_party/libhv/include)
link_directories(third_party/ncnn/lib)
link_directories(third_party/mlpr/lib)
link_directories(third_party/libhv/lib)

add_executable(lpr main.cpp)

target_link_libraries(lpr mlpr freetype ${OpenCV_LIBS} ncnn hv_static)
