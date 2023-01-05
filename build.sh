###
 # @Author: modnarshen
 # @Date: 2023.01.4 16:56:55
 # @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
### 

build_dir="./.build/"

rm -fr ${build_dir}
mkdir -p ${build_dir} && cd ${build_dir}
cmake .. && make -j10
