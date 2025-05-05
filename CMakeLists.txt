cmake_minimum_required(VERSION 3.10)
project(LangWitch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

add_executable(LangWitch
        main.cpp
        language_trie.h
        normalize.h
        trie_node.h
)
