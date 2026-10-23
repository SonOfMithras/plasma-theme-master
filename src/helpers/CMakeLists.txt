find_package(Qt6 6.0 REQUIRED COMPONENTS Core)

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

# GTK4 Reload helper script
install(PROGRAMS gtk4-reload.sh DESTINATION ${KDE_INSTALL_BINDIR} RENAME plasma-theme-master-helper-gtk4-reload)
