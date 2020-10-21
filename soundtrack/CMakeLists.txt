add_library(soundtrack-plugin SHARED)

target_sources(soundtrack-plugin PRIVATE
  src/SoundtrackSource.cpp
  src/SoundtrackPlugin.cpp
  include/Util.h
  include/LockFreeList.h
  include/OBSCallback.h
  include/SoundtrackPluginData.h
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(buildbits "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(buildbits "32")
endif()

target_link_libraries(soundtrack-plugin PUBLIC
  CONAN_PKG::libsoundtrackutil
  ${PROJECT_SOURCE_DIR}/obs-studio/build${buildbits}/libobs/${CMAKE_BUILD_TYPE}/obs.lib
  ${PROJECT_SOURCE_DIR}/obs-studio/build${buildbits}/deps/w32-pthreads/${CMAKE_BUILD_TYPE}/w32-pthreads.lib
  ${PROJECT_SOURCE_DIR}/obs-studio/build${buildbits}/UI/obs-frontend-api/${CMAKE_BUILD_TYPE}/obs-frontend-api.lib
)

target_include_directories(soundtrack-plugin PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/obs-studio/libobs
  ${PROJECT_SOURCE_DIR}/obs-studio/deps
  ${PROJECT_SOURCE_DIR}/obs-studio/UI/obs-frontend-api
)

target_compile_features(soundtrack-plugin PRIVATE cxx_std_17)

set_target_properties(soundtrack-plugin PROPERTIES OUTPUT_NAME soundtrack-plugin-${buildbits})

format_code(soundtrack-plugin)
