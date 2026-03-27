add_executable(plasma-theme-master
    main.cpp
    core/ThemeReader.cpp
    core/ThemeWriter.cpp
    core/Solar.cpp
    core/Config.cpp
    core/Logger.cpp
    core/CLIHandler.cpp
    core/GlobalThemeManager.cpp
    core/FlatpakManager.cpp
    core/UniversalThemeExporter.cpp
    core/TemplateEngine.cpp
    core/TemplateConfig.cpp
    gui/MainWindow.cpp
    gui/GlobalThemeEditor.cpp
    gui/UniversalThemePage.cpp
    gui/ColorSwatchPicker.cpp
)

target_link_libraries(plasma-theme-master
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::ColorScheme
)

install(TARGETS plasma-theme-master DESTINATION ${KDE_INSTALL_BINDIR})

add_subdirectory(helpers)
