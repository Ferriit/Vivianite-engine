{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    pkgs.cmake
    pkgs.ninja
    pkgs.pkg-config
    pkgs.gcc
    pkgs.gdb
  
    pkgs.libffi
    pkgs.libGL
    pkgs.openal

    pkgs.mesa-demos
  
    pkgs.xorg.libX11
    pkgs.xorg.libXrandr
    pkgs.xorg.libXinerama
    pkgs.xorg.libXcursor
    pkgs.xorg.libXi
    pkgs.xorg.libXxf86vm
    pkgs.xorg.libXext
    pkgs.xorg.libXrender
  
    pkgs.wayland
    pkgs.wayland-scanner
    pkgs.libxkbcommon
  ];

  shellHook = ''
    set +e

    if [ ! -d .venv ]; then
      python -m venv .venv || exit 0
    fi

    export VIRTUAL_ENV_DISABLE_PROMPT=1
    . .venv/bin/activate

    python -m pip install -U pip setuptools wheel
    python -m pip install "glad2==2.0.8" || true

    echo "shellHook done."
  '';
}
