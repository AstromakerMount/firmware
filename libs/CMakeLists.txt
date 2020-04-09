add_library(Angle SHARED Angle.cpp)
add_library(Server SHARED Server.cpp)


target_include_directories(Angle PUBLIC ../include)
target_include_directories(Server PUBLIC ../include)

