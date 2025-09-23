cmake_minimum_required(VERSION 3.15)

project(MonteCarlo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Optional: specify output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/main)

# Include source directories
include_directories(
    src
    src/MonteCarlo
    /opt/homebrew/anaconda3/envs/plots/include/eigen3
)

# Add sources
add_executable(xymodel
    src/xymodel.cpp
    src/MonteCarlo/Spin2d/Spin2dModel.cpp
    src/MonteCarlo/Spin2d/SquareXYModel.cpp
    src/MonteCarlo/Spin3d/Spin3dModel.cpp
    src/MonteCarlo/Ising/IsingModel.cpp
    src/MonteCarlo/Ising/SquareIsingModel.cpp
    src/MonteCarlo/MonteCarlo.cpp
)

add_executable(dla
  src/dla.cpp
)

add_executable(qrpm
    src/qrpm.cpp
    src/QRPM/SandpileCliffordSimulator.cpp
    src/QRPM/CircuitUtils.cpp
    src/QRPM/Instructions.cpp
    src/QRPM/QuantumCHPState.cpp
    src/QRPM/SandpileCliffordSimulator.cpp
    src/QRPM/TableauSIMD.cpp
    src/QRPM/CliffordState.cpp
    src/QRPM/PauliString.cpp
    src/QRPM/QuantumCircuit.cpp
    src/QRPM/Tableau.cpp
    src/QRPM/QuantumStates.cpp
)

set(project_targets xymodel dla qrpm)

foreach (tgt IN ITEMS ${project_targets})
    set_target_properties(${tgt} PROPERTIES SUFFIX ".js")

    target_compile_options(${tgt} PRIVATE
        -Wall
        -Wno-unused-function
        -Wno-deprecated-declarations
    )

    target_link_options(${tgt} PRIVATE
        "-sUSE_WEBGL2=1"
        "-sUSE_GLFW=3"
        "-sFULL_ES2=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_FUNCTIONS=['_main', '_slider_callback', '_button_callback', '_radial_callback']"
        "-sEXPORTED_RUNTIME_METHODS=['cwrap','ccall']"
        "-sNO_DISABLE_EXCEPTION_CATCHING"
        "-sMODULARIZE=0"
        "-sEXPORT_ES6=0"
        "-sASSERTIONS"
    )
endforeach()

install(TARGETS xymodel RUNTIME DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js")
install(TARGETS dla RUNTIME DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js")
install(TARGETS qrpm RUNTIME DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js")
foreach (tgt IN ITEMS ${project_targets})
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${tgt}.wasm
            DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js")
endforeach()
