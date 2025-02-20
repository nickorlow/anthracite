{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    nativeBuildInputs = [ pkgs.pkg-config pkgs.openssl pkgs.libgcc pkgs.boost pkgs.cmake ];

    shellHook = ''
      export OPENSSL_DIR="${pkgs.openssl.dev}"
      export PKG_CONFIG_PATH="${pkgs.openssl.dev}/lib/pkgconfig"
      export OPENSSL_NO_VENDOR=1
export OPENSSL_LIB_DIR="${pkgs.lib.getLib pkgs.openssl}/lib"
    '';
}
