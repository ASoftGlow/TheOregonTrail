with import <nixpkgs> {};
fastStdenv.mkDerivation {
  name = "dev";
  nativeBuildInputs = [ cmake clang-tools gdb valgrind ];
  buildInputs = [ gcc alsa-lib pkg-config ];
}
