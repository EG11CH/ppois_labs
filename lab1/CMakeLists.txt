add_library(lab1_options INTERFACE)
target_compile_features(lab1_options INTERFACE cxx_std_17)
target_compile_options(lab1_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)

add_library(lab1_domain src/EnglishRussianDictionary.cpp)
target_include_directories(lab1_domain PUBLIC include)
target_link_libraries(lab1_domain PUBLIC lab1_options)

