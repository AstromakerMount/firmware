add_executable(main main.cpp)

target_link_libraries( main PRIVATE pthread )
target_link_libraries( main PRIVATE Angle )
target_link_libraries( main PRIVATE Server )
target_link_libraries( main PRIVATE Mount )
target_link_libraries( main PRIVATE wiringPi )

