add_library(FatFs STATIC)

set(FATFS_CORE_DIR ${CMAKE_SOURCE_DIR}/middlewares/fatfs/source)
set(FATFS_PORT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(FatFs PRIVATE
        ${FATFS_CORE_DIR}/ff.c
        ${FATFS_PORT_DIR}/diskio.c
        ${FATFS_CORE_DIR}/ffsystem.c
        ${FATFS_CORE_DIR}/ffunicode.c
)
target_include_directories(FatFs PUBLIC
        ${FATFS_PORT_DIR}
        ${FATFS_CORE_DIR}
)
target_include_directories(FatFs PRIVATE
        ${CMAKE_SOURCE_DIR}/driver/inc
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/source
)
target_link_libraries(FatFs PRIVATE freertos)
target_compile_definitions(FatFs PRIVATE HC32F460 USE_DDL_DRIVER)