{
  description = "Plasma Theme Master - A Qt6/C++ KDE Theme Manager";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      pkgsFor = system: nixpkgs.legacyPackages.${system};
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "plasma-theme-master";
            version = "1.1.1";

            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.extra-cmake-modules
              pkgs.pkg-config
              pkgs.kdePackages.wrapQtAppsHook
            ];

            buildInputs = [
              pkgs.qt6.qtbase
              pkgs.qt6.qtdeclarative
              pkgs.qt6.qtsvg
              pkgs.kdePackages.kconfig
              pkgs.kdePackages.kcoreaddons
              pkgs.kdePackages.ki18n
              pkgs.kdePackages.kio
              pkgs.kdePackages.kiconthemes
              pkgs.kdePackages.kservice
              pkgs.kdePackages.kxmlgui
              pkgs.kdePackages.kwindowsystem
              # Kvantum for Qt6
              pkgs.kdePackages.kvantum
            ];

            # Ensure lookandfeeltool and kvantummanager are in the PATH of the wrapper
            qtWrapperArgs = [
              "--prefix PATH : ${pkgs.lib.makeBinPath [ 
                  pkgs.kdePackages.plasma-workspace 
                  pkgs.kdePackages.kvantum 
              ]}"
            ];

            meta = with pkgs.lib; {
              description = "Advanced KDE Plasma theme switcher and manager";
              license = licenses.mit; 
              maintainers = [ ];
              platforms = platforms.linux;
              mainProgram = "plasma-theme-master";
            };
          };
        });
    };
}
