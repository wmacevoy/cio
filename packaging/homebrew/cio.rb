class Cio < Formula
  desc "Lightweight abstract character I/O library for C"
  homepage "https://github.com/wmacevoy/cio"
  url "https://github.com/wmacevoy/cio/archive/refs/tags/v1.0.0.tar.gz"
  # Update sha256 after creating the release tarball:
  #   shasum -a 256 v1.0.0.tar.gz
  sha256 "PLACEHOLDER"
  license "MIT"

  depends_on "cmake" => :build
  depends_on "utf8"

  def install
    system "cmake", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    (testpath/"test.c").write <<~C
      #include <cio.h>
      #include <assert.h>
      int main() {
        CIO io;
        CIOInit(&io);
        assert(CIORead(&io) == -1);
        assert(CIOGetReads(&io) == 0);
        CIOClose(&io);
        return 0;
      }
    C
    system ENV.cc, "test.c", "-I#{include}", "-L#{lib}", "-lcio", "-lutf8", "-o", "test"
    system "./test"
  end
end
