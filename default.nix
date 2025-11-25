with import <nixpkgs> {};
fastStdenv.mkDerivation {
  name = "dev";
  nativeBuildInputs = [ cmake clang-tools gdb valgrind ];
  buildInputs = [ gcc pkg-config ];
}
