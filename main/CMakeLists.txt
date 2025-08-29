# Choose which project to build by commenting/uncommenting the appropriate line
# Only one should be uncommented at a time

# For OTA WiFi updater:
idf_component_register(SRCS "firmware_updater_wifi_idf.cpp"
                    INCLUDE_DIRS ".")

# For object detection (uncomment to use):
# idf_component_register(SRCS "object_detection_idf.cpp"
#                     INCLUDE_DIRS ".")

# For voice recognition (uncomment to use):
# idf_component_register(SRCS "voice_recognition_idf.cpp"
#                     INCLUDE_DIRS ".")
