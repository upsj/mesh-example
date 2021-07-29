cmake_minimum_required(VERSION 3.13)
project(ObjView CXX)
find_package(VTK COMPONENTS 
    CommonColor
    CommonCore
    IOGeometry
    InteractionStyle
    RenderingContextOpenGL2
    RenderingCore
    RenderingFreeType
    RenderingGL2PSOpenGL2
    RenderingOpenGL2
    REQUIRED)
add_executable(objview objview.cpp mesh.cpp)
target_link_libraries(objview PRIVATE ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS objview MODULES ${VTK_LIBRARIES})
