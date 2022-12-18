add_executable(example example.c)
target_link_libraries(example PRIVATE las_c)

add_executable(copy copy.c)
target_link_libraries(copy PRIVATE las_c)

add_executable(lasunzip lasunzip.c)
target_link_libraries(lasunzip PRIVATE las_c)

add_executable(check_decompression check_decompression.c)
target_link_libraries(check_decompression PRIVATE las_c)

add_executable(lasinfo lasinfo.c)
target_link_libraries(lasinfo PRIVATE las_c)