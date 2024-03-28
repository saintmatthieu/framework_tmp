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

if (MU_BUILD_MIDI_MODULE)
    add_subdirectory(midi)
endif()

if (MU_BUILD_MULTIINSTANCES_MODULE)
    add_subdirectory(multiinstances)
endif()

if (MU_BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif()

if (MU_BUILD_SHORTCUTS_MODULE)
    add_subdirectory(shortcuts)
endif()

if (MU_BUILD_UI_MODULE)
    add_subdirectory(ui)
    add_subdirectory(uicomponents)
endif()

if (MU_BUILD_WORKSPACE_MODULE)
    add_subdirectory(workspace)
endif()
