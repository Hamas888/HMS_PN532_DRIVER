# HMS_PN532_DRIVER/CMakeLists.txt

set(HMS_PN532_DRIVER_VERSION 1.0.0)

# Check if we're building with Zephyr
if(DEFINED ZEPHYR_BASE)

    # zephyr_library()
    # zephyr_library_sources() → not needed yet
    zephyr_include_directories(include)

# Check if we're building with ESP-IDF
elseif(DEFINED ESP_PLATFORM OR DEFINED IDF_VER OR DEFINED ENV{IDF_PATH})
    idf_component_register(
        SRCS 
            "src/HMS_PN532_DRIVER.cpp"
            "src/HMS_PN532_NFC_Tag.cpp"
            "src/HMS_PN532_Controller.cpp"
            "src/HMS_PN532_NDEF_Record.cpp"
            "src/HMS_PN532_NDEF_Message.cpp"
            "src/HMS_PN532_MifareClassic.cpp"
            "src/HMS_PN532_Interface_I2C.cpp"
            "src/HMS_PN532_MifareUltralight.cpp"
        INCLUDE_DIRS "include"
        REQUIRES
            "driver"
    )
    
# STM32 / generic CMake project
else()
    add_library(HMS_PN532_DRIVER INTERFACE)
    target_include_directories(HMS_PN532_DRIVER INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(HMS_PN532_DRIVER INTERFACE cxx_std_17)
endif()