project(TJpgDecLibrary)
add_library(tjpgdec STATIC tjpgd.c)
target_include_directories(tjpgdec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})