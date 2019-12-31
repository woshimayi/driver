add_library(myfunction world.c)

install (TARGETS myfunction DESTINATION bin)
install (FILES world.h DESTINATION include)