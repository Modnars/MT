###
 # @Author: modnarshen
 # @Date: 2023.01.04 16:56:55
 # @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
### 

build_dir="./build/"
target=${1-"use_cache"}

if [[ "$target" == "all" ]]; then 
    echo "BUILD with cache cleared"
    rm -fr ${build_dir}
else
    echo "BUILD with cache"
fi

mkdir -p ${build_dir} && cd ${build_dir}
cmake .. && make -j10
