cmake_minimum_required(VERSION 3.10)
project(LangWitch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD   11)

add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

# ────────────────────────────────
# 1. Locate wxWidgets
# ────────────────────────────────
find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base)

# ────────────────────────────────
# 2. Your executable
# ────────────────────────────────
add_executable(LangWitch
    main.cpp
    language_trie.h
    normalize.h
    trie_node.h
)

# ────────────────────────────────
# 3. Propagate compiler and linker flags
# ────────────────────────────────
target_include_directories (LangWitch PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries      (LangWitch PRIVATE ${wxWidgets_LIBRARIES})
target_compile_definitions (LangWitch PRIVATE ${wxWidgets_DEFINITIONS})