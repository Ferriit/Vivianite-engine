{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.python3
    pkgs.python3Packages.virtualenv

    pkgs.glfw
    pkgs.pkg-config

    pkgs.glm
  ];

  shellHook = ''
    set -x
    set +e

    if [ ! -d .venv ]; then
      python -m venv .venv || exit 0
    fi

    . .venv/bin/activate

    python -m pip install -U pip setuptools wheel
    python -m pip install "glad2==2.0.8" || true

    echo "shellHook done."
  '';
}
