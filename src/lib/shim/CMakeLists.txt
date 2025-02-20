## for now, interposer includes shadow.h, so it needs to know about glib and igraph

#add_cflags(-Wno-invalid-noreturn)

## build the required library for intercepting system and library calls.
## create the interpose library, to be set as LD_PRELOAD to intercept functions.
## the functions we intercept MUST be in a shared library for dlsym searching.
## we redirect them to our internal shadow versions, or to libc.
# include_directories(${CMAKE_SOURCE_DIR}/src/main)
find_package(GLIB REQUIRED)
find_package(RT REQUIRED)
include_directories(${GLIB_INCLUDES})


add_cflags(-fPIC)

set(SHIM_HELPER_LIB shadow-shim-helper)
set(SHIM_HELPER_FILES
  binary_spinning_sem.cc
  ipc.cc
  shadow_sem.c
  shadow_spinlock.c
  shim_event.c
)
add_library(${SHIM_HELPER_LIB} STATIC ${SHIM_HELPER_FILES})
# -D_GNU_SOURCE enables some additional features in libc, such at RTLD_NEXT.
# -fPIC enables linking the static compile unit with a shared object library
target_compile_options(${SHIM_HELPER_LIB} PRIVATE -D_GNU_SOURCE -fPIC)
target_link_libraries(${SHIM_HELPER_LIB} INTERFACE shadow-shmem)

set(SHIM_LIB shadow-shim)
set(SHIM_FILES
  shim.c
  shim_api_addrinfo.c
  shim_api_ifaddrs.c
  shim_api_syscall.c
  shim_logger.c
  shim_rdtsc.c
  shim_seccomp.c
  shim_shmem.c
  shim_sys.c
  shim_syscall.c
  shim_tls.c
)
add_library(${SHIM_LIB} SHARED ${SHIM_FILES})
set_target_properties(${SHIM_LIB} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed")
target_compile_options(${SHIM_LIB} PRIVATE -pthread -D_GNU_SOURCE)
target_link_libraries(${SHIM_LIB} ${SHIM_HELPER_LIB} shadow-shmem shadow-tsc logger
  ${RT_LIBRARIES} ${GLIB_LIBRARIES} -pthread -ldl)
install(TARGETS ${SHIM_LIB} DESTINATION lib)

