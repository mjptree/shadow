## TODO if we have cmake > 3.1, can we do the following?
#pkg_check_modules (GLIB2   glib-2.0>=2.32)
#message(STATUS "GLIB2_VERSION = ${GLIB2_VERSION}")

include(CheckSymbolExists)

list(APPEND CMAKE_REQUIRED_INCLUDES ${GLIB_INCLUDES})
list(APPEND CMAKE_REQUIRED_LIBRARIES ${GLIB_LIBRARIES})

check_symbol_exists(g_memdup2 "glib.h" HAS_MEMDUP2)

if(HAS_MEMDUP2)
    add_compile_definitions(HAS_MEMDUP2)
endif()

include_directories(${RT_INCLUDES} ${DL_INCLUDES} ${M_INCLUDES} ${GLIB_INCLUDES})

## link to preload libs (rpath will not contain these paths when shadow is installed)
link_directories(${CMAKE_BINARY_DIR}/src/lib/shim)
link_directories(${CMAKE_BINARY_DIR}/src/lib/injector_preload)
link_directories(${CMAKE_BINARY_DIR}/src/lib/libc_preload)
link_directories(${CMAKE_BINARY_DIR}/src/lib/openssl_preload)

## compile defs and flags
#add_definitions(-D_SVID_SOURCE -D_XOPEN_SOURCE=600 -D_ISOC11_SOURCE) #-D_GNU_SOURCE
add_definitions(-D_GNU_SOURCE)
add_cflags(-fPIC)
#add_cflags(-Wno-unknown-attributes)
#add_cflags(-Wno-unused-command-line-argument)
#add_cflags(-Wno-unknown-warning-option)
#add_cflags(-Wno-visibility)

# Enable C11.
add_cflags(-std=gnu11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} -std=c++11 -fPIC")

if(SHADOW_PROFILE STREQUAL ON)
    message(STATUS "Building Shadow core with profiling support using '-pg'")
    add_cflags(-pg)
endif(SHADOW_PROFILE STREQUAL ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "Building Rust library in debug mode.")
    set(RUST_BUILD_TYPE "debug")
    set(RUST_BUILD_FLAG "")
else()
    message(STATUS "Building Rust library in release mode.")
    set(RUST_BUILD_TYPE "release")
    set(RUST_BUILD_FLAG "--release")
endif()

if(SHADOW_COVERAGE STREQUAL ON)
    # https://github.com/shadow/shadow/issues/1236
    set(RUSTFLAGS "${RUSTFLAGS} --remap-path-prefix \"=${CMAKE_CURRENT_SOURCE_DIR}/\"")
endif()

add_library(shadow-remora STATIC host/descriptor/tcp_retransmit_tally.cc)

set(SHD_SHMEM_SRC
    shmem/buddy.c
    shmem/shmem_allocator.c
    shmem/shmem_file.c
    shmem/shmem_util.c
    shmem/shmem_cleanup.c)

add_library(shadow-shmem STATIC ${SHD_SHMEM_SRC})
target_link_libraries(shadow-shmem INTERFACE logger)

add_executable(shd-shmem-test shmem/shmem_test.c)
target_link_libraries(shd-shmem-test shadow-shmem
    ${GLIB_LIBRARIES} ${RT_LIBRARIES} ${M_LIBRARIES} ${PROCPS_LIBRARIES})
add_test(NAME shmem COMMAND shd-shmem-test)

## sources for our main shadow program
set(shadow_srcs
    core/logger/log_wrapper.c
    core/scheduler/scheduler.c
    core/scheduler/scheduler_policy_host_single.c
    core/scheduler/scheduler_policy_host_steal.c
    core/scheduler/scheduler_policy_thread_perhost.c
    core/scheduler/scheduler_policy_thread_perthread.c
    core/scheduler/scheduler_policy_thread_single.c
    core/support/config_handlers.c
    core/work/event.c
    core/work/message.c
    core/work/task.c
    core/main.c
    core/controller.c
    core/manager.c
    core/worker.c

    host/descriptor/descriptor.c
    host/status_listener.c
    host/descriptor/compat_socket.c
    host/descriptor/epoll.c
    host/descriptor/file.c
    host/descriptor/socket.c
    host/descriptor/tcp.c
    host/descriptor/tcp_cong.c
    host/descriptor/tcp_cong_reno.c
    host/descriptor/timer.c
    host/descriptor/transport.c
    host/descriptor/udp.c
    host/affinity.c
    host/process.c
    host/cpu.c
    host/futex.c
    host/futex_table.c
    host/shimipc.c
    host/syscall_handler.c
    host/syscall/protected.c
    host/syscall/clone.c
    host/syscall/epoll.c
    host/syscall/fcntl.c
    host/syscall/file.c
    host/syscall/fileat.c
    host/syscall/futex.c
    host/syscall/ioctl.c
    host/syscall/mman.c
    host/syscall/poll.c
    host/syscall/process.c
    host/syscall/random.c
    host/syscall/shadow.c
    host/syscall/signal.c
    host/syscall/socket.c
    host/syscall/sysinfo.c
    host/syscall/time.c
    host/syscall/timerfd.c
    host/syscall/unistd.c
    host/syscall/uio.c
    host/thread.c
    host/host.c
    host/syscall_condition.c
    host/thread_preload.c
    host/thread_ptrace.c
    host/network_interface.c
    host/network_queuing_disciplines.c
    host/tracker.c

    routing/payload.c
    routing/packet.c
    routing/address.c
    routing/router_queue_single.c
    routing/router_queue_static.c
    routing/router_queue_codel.c
    routing/router.c
    routing/dns.c

    utility/async_priority_queue.c
    utility/count_down_latch.c
    utility/fork_proxy.c
    utility/pcap_writer.c
    utility/priority_queue.c
    utility/tagged_ptr.c
    utility/utility.c
)
add_library(shadow-c STATIC ${shadow_srcs})
target_link_libraries(shadow-c INTERFACE
   ${CMAKE_THREAD_LIBS_INIT} ${M_LIBRARIES} ${DL_LIBRARIES} ${RT_LIBRARIES}
   ${GLIB_LIBRARIES} ${PROCPS_LIBRARIES}
   shadow-shim-helper logger shadow-remora shadow-shmem shadow-tsc)

# TODO: extract -L and -l flags from the output of
# `get_target_property(INTERFACE_LINK_LIBRARIES shadow-c
# INTERFACE_LINK_LIBRARIES)`, and pass those through to cargo.
set(RUST_DEPENDS "shadow-c")
get_target_property(INTERFACE_LINK_LIBRARIES shadow-c INTERFACE_LINK_LIBRARIES)
foreach(INTERFACE_LINK_LIBRARY IN LISTS INTERFACE_LINK_LIBRARIES)
    if(INTERFACE_LINK_LIBRARY MATCHES "(.*)/lib(.*)\(.so|.a)")
        set(RUSTFLAGS "-L${CMAKE_MATCH_1} -l${CMAKE_MATCH_2} ${RUSTFLAGS}")
    elseif(INTERFACE_LINK_LIBRARY MATCHES "^-l.*")
        set(RUSTFLAGS "${INTERFACE_LINK_LIBRARY} ${RUSTFLAGS}")
    else()
        # Treat as a target
        #
        # XXX: LIBRARY_OUTPUT_DIRECTORY seems to not be set. We add the
        # corresponding -L's manually after this loop.
        #get_target_property(LIB logger LIBRARY_OUTPUT_DIRECTORY)

        set(RUSTFLAGS "-l${INTERFACE_LINK_LIBRARY} ${RUSTFLAGS}")

        # The DEPENDS clause in ExternalProject_Add doesn't recurse into
        # INTERFACE dependencies; we need to add them ourselves too.
        list(APPEND RUST_DEPENDS "${INTERFACE_LINK_LIBRARY}")
    endif()
endforeach(INTERFACE_LINK_LIBRARY)
# Add shadow-c itself
set(RUSTFLAGS "${RUSTFLAGS} -lshadow-c")
# -lstdc++ is needed as a dependency when linking remora. TODO: make it an
# INTERFACE dependency on remora and make the loop above recursive.
set(RUSTFLAGS "${RUSTFLAGS} -lstdc++")
# Manually add search-directories for our custom libraries.
set(RUSTFLAGS "${RUSTFLAGS} -L${CMAKE_CURRENT_BINARY_DIR}")
set(RUSTFLAGS "${RUSTFLAGS} -L${CMAKE_BINARY_DIR}/src/lib/logger")
set(RUSTFLAGS "${RUSTFLAGS} -L${CMAKE_BINARY_DIR}/src/lib/shim")
set(RUSTFLAGS "${RUSTFLAGS} -L${CMAKE_BINARY_DIR}/src/lib/openssl_preload")
set(RUSTFLAGS "${RUSTFLAGS} -L${CMAKE_BINARY_DIR}/src/lib/tsc")
set(CARGO_ENV_VARS "${CARGO_ENV_VARS} RUSTFLAGS=\"${RUSTFLAGS}\"")
set(CARGO_ENV_VARS "${CARGO_ENV_VARS} SHADOW_VERSION=\"${SHADOW_VERSION_STRING_CONF}\"")

set(RUST_FEATURES "")
if(SHADOW_USE_PERF_TIMERS STREQUAL ON)
  set(RUST_FEATURES "${RUST_FEATURES} perf_timers")
endif()

include(ExternalProject)
## build the rust library
ExternalProject_Add(
    shadow-rust-project
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    BUILD_ALWAYS 1
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND bash -c "${CARGO_ENV_VARS} cargo build ${RUST_BUILD_FLAG} --all-targets --target-dir \"${CMAKE_CURRENT_BINARY_DIR}/target\" --features \"${RUST_FEATURES}\""
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/target/debug/libshadow_rs.a ${CMAKE_CURRENT_BINARY_DIR}/target/release/libshadow_rs.a
    INSTALL_COMMAND ""
    LOG_BUILD OFF
    DEPENDS "${RUST_DEPENDS}"
)
## we can't predict exact executable names until this is fixed: https://github.com/rust-lang/cargo/issues/1924
add_test(NAME rust-unit-tests COMMAND bash -c "exec \"$(find target/${RUST_BUILD_TYPE}/deps/ \
                                               -type f -executable -name 'shadow_rs-*' -print | head -n 1)\" --color always")
set_property(TEST rust-unit-tests PROPERTY ENVIRONMENT "RUST_BACKTRACE=1")

## allow shadow to link to the static rust library
add_library(shadow-rs STATIC IMPORTED)
set_target_properties(shadow-rs PROPERTIES IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/target/debug/libshadow_rs.a)
set_target_properties(shadow-rs PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/target/release/libshadow_rs.a)
add_dependencies(shadow-rs shadow-rust-project)

## Allow circular dependencies between shadow-rs and shadow-c. Without
#--start-group and --end-group, # the linker will drop object files in the first
#listed library that aren't referenced from an # earlier dependency.
## See https://gitlab.kitware.com/cmake/cmake/-/issues/21511#note_865669
add_library(shadow-c-and-rs INTERFACE)
target_link_libraries(shadow-c-and-rs INTERFACE "-Wl,--start-group" shadow-rs shadow-c "-Wl,--end-group")

## specify the main shadow executable, build, link, and install
add_executable(shadow main.c)
target_link_libraries(shadow shadow-c-and-rs)
install(TARGETS shadow DESTINATION bin)

## shadow needs to find libs after install
set_target_properties(shadow PROPERTIES LINK_FLAGS "-Wl,--no-as-needed")

add_subdirectory(bindings)
