message(STATUS "Configuring Muse Framework")

add_subdirectory(global)

if (MU_BUILD_UI_MODULE)
    add_subdirectory(ui)
    add_subdirectory(uicomponents)
endif()
