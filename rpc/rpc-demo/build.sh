###
 # @file: 
 # @Author: regangcli
 # @copyright: Tencent Technology (Shenzhen) Company Limited
 # @Date: 2023-06-15 16:49:43
 # @edit: regangcli
 # @brief: 
### 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/home/user00/next/lib
(cd pb && ../../protobuf-2.6.1/bin/protoc *.proto --cpp_out=.)
rm ./CMakeCache.txt ./CMakeFiles -rf cmake_install.cmake Makefile librpc.a core* client server
cmake .
make -j8
# pkill server
# ./server
