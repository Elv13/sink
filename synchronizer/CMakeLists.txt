project(sink_synchronizer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(sinksynchronizer_SRCS
    main.cpp
)

add_executable(${PROJECT_NAME} ${sinksynchronizer_SRCS})
target_link_libraries(${PROJECT_NAME} sink KAsync ${CMAKE_DL_LIBS})
qt5_use_modules(${PROJECT_NAME} Core Network)
install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
