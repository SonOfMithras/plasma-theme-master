add_executable(plasma-theme-master
    main.cpp
    core/ThemeReader.cpp
    core/ThemeWriter.cpp
    core/Solar.cpp
    core/Config.cpp
    core/Logger.cpp
    core/GlobalThemeManager.cpp
    gui/MainWindow.cpp
    gui/GlobalThemeEditor.cpp
)

target_link_libraries(plasma-theme-master
    Qt6::Core
    Qt6::Widgets
    KF6::ConfigCore
    KF6::CoreAddons
)

install(TARGETS plasma-theme-master DESTINATION ${KDE_INSTALL_BINDIR})
