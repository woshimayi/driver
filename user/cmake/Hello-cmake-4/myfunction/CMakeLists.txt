add_library(myfunction world.c)
add_executable(MakeTable MakeTable.c)
add_library(MakeTable MakeTable.c)


add_library(
    myfunction
    world.c
    ${CMAKE_CURRENT_BINARY_DIR}/Table.h
)


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    DEPENDS MakeTable
)


install (TARGETS myfunction DESTINATION bin)
install (FILES   world.h    DESTINATION include)