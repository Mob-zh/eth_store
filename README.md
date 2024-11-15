# 元器件仓库系统（开发中）

## 模块目录结构：
```
.
├── CMakeLists.txt      
├── module1
│   ├── app
│   │   └──xxx.c
│   ├── src
│   │   ├── xxx.c
│   │   ├── xxx.h
│   ├── test
│   │   ├── xxx.c
│   └── CMakeLists.txt    # 子项目的 
```

## 编译方法
``` shell
mkdir build
cd build
cmake ..
make
```

