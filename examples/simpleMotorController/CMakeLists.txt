add_executable(simpleMotorController SimpleMotorController.cpp MyControlSystem.cpp MySafetyProperties.cpp SequenceA.cpp)
target_link_libraries(simpleMotorController eeros ${EEROS_LIBS})
