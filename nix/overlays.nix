{
  lib,
  inputs,
}: let
  mkDate = longDate: (lib.concatStringsSep "-" [
    (builtins.substring 0 4 longDate)
    (builtins.substring 4 2 longDate)
    (builtins.substring 6 2 longDate)
  ]);
  version = lib.removeSuffix "\n" (builtins.readFile ../VERSION);
in {
  default = inputs.self.overlays.hyprcursor;

  hyprcursor = lib.composeManyExtensions [
    inputs.hyprlang.overlays.default
    (final: prev: {
      hyprcursor = prev.callPackage ./default.nix {
        stdenv = prev.gcc15Stdenv;
        version = version + "+date=" + (mkDate (inputs.self.lastModifiedDate or "19700101")) + "_" + (inputs.self.shortRev or "dirty");
        inherit (final) hyprlang;
      };

      hyprcursor-with-tests = final.hyprcursor.overrideAttrs (_: _: {
        cmakeFlags = [(lib.cmakeBool "INSTALL_TESTS" true)];
      });
    })
  ];
}
