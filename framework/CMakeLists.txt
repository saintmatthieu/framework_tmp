message(STATUS "Configuring Muse Framework")

add_subdirectory(global)

if (MU_BUILD_ACCESSIBILITY_MODULE)
    add_subdirectory(accessibility)
endif()

if (MU_BUILD_ACTIONS_MODULE)
    add_subdirectory(actions)
endif()

if (MU_BUILD_EXTENSIONS_MODULE)
    add_subdirectory(extensions)
endif()

if (MU_BUILD_LANGUAGES_MODULE)
    add_subdirectory(languages)
endif()


if (MU_BUILD_UI_MODULE)
    add_subdirectory(ui)
    add_subdirectory(uicomponents)
endif()

