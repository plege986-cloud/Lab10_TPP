add_executable(Lab10_TPP Lab10_TPP.cpp)
target_link_libraries(Lab10_TPP PRIVATE sqlite_orm::sqlite_orm unofficial::sqlite3::sqlite3)
add_test(NAME Lab10_tests COMMAND Lab10_TPP --test)
