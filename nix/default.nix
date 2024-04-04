{
  lib,
  stdenv,
  cmake,
  pkg-config,
  cairo,
  hyprlang,
  librsvg,
  libzip,
  version ? "git",
}:
stdenv.mkDerivation {
  pname = "hyprcursor";
  inherit version;
  src = ../.;

  patches = [
    # adds /run/current-system/sw/share/icons to the icon lookup directories
    ./dirs.patch
  ];

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    cairo
    hyprlang
    librsvg
    libzip
  ];

  outputs = [
    "out"
    "dev"
    "lib"
  ];

  meta = {
    homepage = "https://github.com/hyprwm/hyprcursor";
    description = "The hyprland cursor format, library and utilities";
    license = lib.licenses.bsd3;
    platforms = lib.platforms.linux;
    mainProgram = "hyprcursor";
  };
}
