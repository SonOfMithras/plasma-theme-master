find_package(Qt6 6.0 REQUIRED COMPONENTS Core)
find_package(KF6 6.0 REQUIRED COMPONENTS Config)

# VSCode helper
add_executable(plasma-theme-master-helper-vscode vscode_helper.cpp)
target_link_libraries(plasma-theme-master-helper-vscode Qt6::Core)
install(TARGETS plasma-theme-master-helper-vscode DESTINATION ${KDE_INSTALL_BINDIR})

# Zed helper
add_executable(plasma-theme-master-helper-zed zed_helper.cpp)
target_link_libraries(plasma-theme-master-helper-zed Qt6::Core)
install(TARGETS plasma-theme-master-helper-zed DESTINATION ${KDE_INSTALL_BINDIR})

# Firefox helper
add_executable(plasma-theme-master-helper-firefox firefox_helper.cpp)
target_link_libraries(plasma-theme-master-helper-firefox Qt6::Core)
install(TARGETS plasma-theme-master-helper-firefox DESTINATION ${KDE_INSTALL_BINDIR})

# Konsole helper (needs KConfig for konsolerc)
add_executable(plasma-theme-master-helper-konsole konsole_helper.cpp)
target_link_libraries(plasma-theme-master-helper-konsole
    Qt6::Core
    KF6::ConfigCore
)
install(TARGETS plasma-theme-master-helper-konsole DESTINATION ${KDE_INSTALL_BINDIR})
