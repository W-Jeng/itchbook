add_library(itchbook_lib INTERFACE)
target_include_directories(itchbook_lib INTERFACE
  ${CMAKE_SOURCE_DIR}
)

target_compile_features(itchbook_lib INTERFACE cxx_std_20)
target_link_libraries(itchbook_lib INTERFACE fmt::fmt Boost::headers)

# Executables
add_executable(main main.cpp)
target_link_libraries(main PRIVATE itchbook_lib)
