{
  lib,
  stdenv,
  cmake,
  pkg-config,
  cairo,
  hyprlang,
  librsvg,
  libzip,
  xcur2png,
  tomlplusplus,
  version ? "git",
}:
stdenv.mkDerivation {
  pname = "hyprcursor";
  inherit version;
  src = ../.;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    cairo
    hyprlang
    librsvg
    libzip
    xcur2png
    tomlplusplus
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
