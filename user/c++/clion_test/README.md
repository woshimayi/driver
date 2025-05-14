###dir tree
```shell
.
├── CMakeLists.txt
├── README.md
├── inc
│		 └── mod.h
├── install
├── main.cpp
├── src
│		 └── mod.cpp
├── sub_test
│		 ├── CMakeLists.txt
│		 ├── main.cpp
│		 ├── sub_1.cpp
│		 └── sub_1.h
├── test.cpp
└── test.h
```


### build Release
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
make install
```

### build Debug
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
make install
```