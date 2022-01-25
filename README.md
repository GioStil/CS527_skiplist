# CS527_skiplist


## Usage

You can include the code below in your `cmake` project and `#include <skiplist.h>` in your sources.

``` cmake
FetchContent_Declare(skiplist # Recommendation: Stick close to the original name.
                     GIT_REPOSITORY https://github.com/GioStil/CS527_skiplist)
FetchContent_GetProperties(skiplist)
if(NOT skiplist_POPULATED)
  FetchContent_Populate(skiplist)
  add_subdirectory(${skiplist_SOURCE_DIR} ${skiplist_BINARY_DIR})
  FetchContent_MakeAvailable(skiplist)
endif()
target_link_libraries(YOUR_LIB_NAME skiplist)
```

## Build, Testing and Installation

``` sh
cd CS527_skiplist
mkdir build;cd build
cmake ..
make

# To run tests
ctest --verbose

# To install
sudo make install
```
