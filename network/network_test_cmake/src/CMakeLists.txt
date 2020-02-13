add_library(src tcping.c getinform.c)

install (TARGETS src  DESTINATION bin)
install (FILES head.h DESTINATION include)
