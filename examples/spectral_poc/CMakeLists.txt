# Include header directory
include_directories(${TOS_PROJECT_ROOT}/src)

# Source files
set(APP_SRC
    ${TOS_PROJECT_ROOT}/src/tuya_app_main.c
)

# Create library
add_library(tuyaapp STATIC ${APP_SRC})

# Link TuyaOS
target_link_libraries(tuyaapp tuyaos m)