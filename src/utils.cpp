/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <common/utils.h>

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // for umask
#include <unistd.h>    // for fork, close, getpid, setsid, sysconf, _SC_...

#ifndef UINT64_C
#define UINT64_C(val) val##ULL
#endif

namespace {

const uint64_t crc64_tab[256] = {
    UINT64_C(0x0000000000000000), UINT64_C(0x7ad870c830358979), UINT64_C(0xf5b0e190606b12f2),
    UINT64_C(0x8f689158505e9b8b), UINT64_C(0xc038e5739841b68f), UINT64_C(0xbae095bba8743ff6),
    UINT64_C(0x358804e3f82aa47d), UINT64_C(0x4f50742bc81f2d04), UINT64_C(0xab28ecb46814fe75),
    UINT64_C(0xd1f09c7c5821770c), UINT64_C(0x5e980d24087fec87), UINT64_C(0x24407dec384a65fe),
    UINT64_C(0x6b1009c7f05548fa), UINT64_C(0x11c8790fc060c183), UINT64_C(0x9ea0e857903e5a08),
    UINT64_C(0xe478989fa00bd371), UINT64_C(0x7d08ff3b88be6f81), UINT64_C(0x07d08ff3b88be6f8),
    UINT64_C(0x88b81eabe8d57d73), UINT64_C(0xf2606e63d8e0f40a), UINT64_C(0xbd301a4810ffd90e),
    UINT64_C(0xc7e86a8020ca5077), UINT64_C(0x4880fbd87094cbfc), UINT64_C(0x32588b1040a14285),
    UINT64_C(0xd620138fe0aa91f4), UINT64_C(0xacf86347d09f188d), UINT64_C(0x2390f21f80c18306),
    UINT64_C(0x594882d7b0f40a7f), UINT64_C(0x1618f6fc78eb277b), UINT64_C(0x6cc0863448deae02),
    UINT64_C(0xe3a8176c18803589), UINT64_C(0x997067a428b5bcf0), UINT64_C(0xfa11fe77117cdf02),
    UINT64_C(0x80c98ebf2149567b), UINT64_C(0x0fa11fe77117cdf0), UINT64_C(0x75796f2f41224489),
    UINT64_C(0x3a291b04893d698d), UINT64_C(0x40f16bccb908e0f4), UINT64_C(0xcf99fa94e9567b7f),
    UINT64_C(0xb5418a5cd963f206), UINT64_C(0x513912c379682177), UINT64_C(0x2be1620b495da80e),
    UINT64_C(0xa489f35319033385), UINT64_C(0xde51839b2936bafc), UINT64_C(0x9101f7b0e12997f8),
    UINT64_C(0xebd98778d11c1e81), UINT64_C(0x64b116208142850a), UINT64_C(0x1e6966e8b1770c73),
    UINT64_C(0x8719014c99c2b083), UINT64_C(0xfdc17184a9f739fa), UINT64_C(0x72a9e0dcf9a9a271),
    UINT64_C(0x08719014c99c2b08), UINT64_C(0x4721e43f0183060c), UINT64_C(0x3df994f731b68f75),
    UINT64_C(0xb29105af61e814fe), UINT64_C(0xc849756751dd9d87), UINT64_C(0x2c31edf8f1d64ef6),
    UINT64_C(0x56e99d30c1e3c78f), UINT64_C(0xd9810c6891bd5c04), UINT64_C(0xa3597ca0a188d57d),
    UINT64_C(0xec09088b6997f879), UINT64_C(0x96d1784359a27100), UINT64_C(0x19b9e91b09fcea8b),
    UINT64_C(0x636199d339c963f2), UINT64_C(0xdf7adabd7a6e2d6f), UINT64_C(0xa5a2aa754a5ba416),
    UINT64_C(0x2aca3b2d1a053f9d), UINT64_C(0x50124be52a30b6e4), UINT64_C(0x1f423fcee22f9be0),
    UINT64_C(0x659a4f06d21a1299), UINT64_C(0xeaf2de5e82448912), UINT64_C(0x902aae96b271006b),
    UINT64_C(0x74523609127ad31a), UINT64_C(0x0e8a46c1224f5a63), UINT64_C(0x81e2d7997211c1e8),
    UINT64_C(0xfb3aa75142244891), UINT64_C(0xb46ad37a8a3b6595), UINT64_C(0xceb2a3b2ba0eecec),
    UINT64_C(0x41da32eaea507767), UINT64_C(0x3b024222da65fe1e), UINT64_C(0xa2722586f2d042ee),
    UINT64_C(0xd8aa554ec2e5cb97), UINT64_C(0x57c2c41692bb501c), UINT64_C(0x2d1ab4dea28ed965),
    UINT64_C(0x624ac0f56a91f461), UINT64_C(0x1892b03d5aa47d18), UINT64_C(0x97fa21650afae693),
    UINT64_C(0xed2251ad3acf6fea), UINT64_C(0x095ac9329ac4bc9b), UINT64_C(0x7382b9faaaf135e2),
    UINT64_C(0xfcea28a2faafae69), UINT64_C(0x8632586aca9a2710), UINT64_C(0xc9622c4102850a14),
    UINT64_C(0xb3ba5c8932b0836d), UINT64_C(0x3cd2cdd162ee18e6), UINT64_C(0x460abd1952db919f),
    UINT64_C(0x256b24ca6b12f26d), UINT64_C(0x5fb354025b277b14), UINT64_C(0xd0dbc55a0b79e09f),
    UINT64_C(0xaa03b5923b4c69e6), UINT64_C(0xe553c1b9f35344e2), UINT64_C(0x9f8bb171c366cd9b),
    UINT64_C(0x10e3202993385610), UINT64_C(0x6a3b50e1a30ddf69), UINT64_C(0x8e43c87e03060c18),
    UINT64_C(0xf49bb8b633338561), UINT64_C(0x7bf329ee636d1eea), UINT64_C(0x012b592653589793),
    UINT64_C(0x4e7b2d0d9b47ba97), UINT64_C(0x34a35dc5ab7233ee), UINT64_C(0xbbcbcc9dfb2ca865),
    UINT64_C(0xc113bc55cb19211c), UINT64_C(0x5863dbf1e3ac9dec), UINT64_C(0x22bbab39d3991495),
    UINT64_C(0xadd33a6183c78f1e), UINT64_C(0xd70b4aa9b3f20667), UINT64_C(0x985b3e827bed2b63),
    UINT64_C(0xe2834e4a4bd8a21a), UINT64_C(0x6debdf121b863991), UINT64_C(0x1733afda2bb3b0e8),
    UINT64_C(0xf34b37458bb86399), UINT64_C(0x8993478dbb8deae0), UINT64_C(0x06fbd6d5ebd3716b),
    UINT64_C(0x7c23a61ddbe6f812), UINT64_C(0x3373d23613f9d516), UINT64_C(0x49aba2fe23cc5c6f),
    UINT64_C(0xc6c333a67392c7e4), UINT64_C(0xbc1b436e43a74e9d), UINT64_C(0x95ac9329ac4bc9b5),
    UINT64_C(0xef74e3e19c7e40cc), UINT64_C(0x601c72b9cc20db47), UINT64_C(0x1ac40271fc15523e),
    UINT64_C(0x5594765a340a7f3a), UINT64_C(0x2f4c0692043ff643), UINT64_C(0xa02497ca54616dc8),
    UINT64_C(0xdafce7026454e4b1), UINT64_C(0x3e847f9dc45f37c0), UINT64_C(0x445c0f55f46abeb9),
    UINT64_C(0xcb349e0da4342532), UINT64_C(0xb1eceec59401ac4b), UINT64_C(0xfebc9aee5c1e814f),
    UINT64_C(0x8464ea266c2b0836), UINT64_C(0x0b0c7b7e3c7593bd), UINT64_C(0x71d40bb60c401ac4),
    UINT64_C(0xe8a46c1224f5a634), UINT64_C(0x927c1cda14c02f4d), UINT64_C(0x1d148d82449eb4c6),
    UINT64_C(0x67ccfd4a74ab3dbf), UINT64_C(0x289c8961bcb410bb), UINT64_C(0x5244f9a98c8199c2),
    UINT64_C(0xdd2c68f1dcdf0249), UINT64_C(0xa7f41839ecea8b30), UINT64_C(0x438c80a64ce15841),
    UINT64_C(0x3954f06e7cd4d138), UINT64_C(0xb63c61362c8a4ab3), UINT64_C(0xcce411fe1cbfc3ca),
    UINT64_C(0x83b465d5d4a0eece), UINT64_C(0xf96c151de49567b7), UINT64_C(0x76048445b4cbfc3c),
    UINT64_C(0x0cdcf48d84fe7545), UINT64_C(0x6fbd6d5ebd3716b7), UINT64_C(0x15651d968d029fce),
    UINT64_C(0x9a0d8ccedd5c0445), UINT64_C(0xe0d5fc06ed698d3c), UINT64_C(0xaf85882d2576a038),
    UINT64_C(0xd55df8e515432941), UINT64_C(0x5a3569bd451db2ca), UINT64_C(0x20ed197575283bb3),
    UINT64_C(0xc49581ead523e8c2), UINT64_C(0xbe4df122e51661bb), UINT64_C(0x3125607ab548fa30),
    UINT64_C(0x4bfd10b2857d7349), UINT64_C(0x04ad64994d625e4d), UINT64_C(0x7e7514517d57d734),
    UINT64_C(0xf11d85092d094cbf), UINT64_C(0x8bc5f5c11d3cc5c6), UINT64_C(0x12b5926535897936),
    UINT64_C(0x686de2ad05bcf04f), UINT64_C(0xe70573f555e26bc4), UINT64_C(0x9ddd033d65d7e2bd),
    UINT64_C(0xd28d7716adc8cfb9), UINT64_C(0xa85507de9dfd46c0), UINT64_C(0x273d9686cda3dd4b),
    UINT64_C(0x5de5e64efd965432), UINT64_C(0xb99d7ed15d9d8743), UINT64_C(0xc3450e196da80e3a),
    UINT64_C(0x4c2d9f413df695b1), UINT64_C(0x36f5ef890dc31cc8), UINT64_C(0x79a59ba2c5dc31cc),
    UINT64_C(0x037deb6af5e9b8b5), UINT64_C(0x8c157a32a5b7233e), UINT64_C(0xf6cd0afa9582aa47),
    UINT64_C(0x4ad64994d625e4da), UINT64_C(0x300e395ce6106da3), UINT64_C(0xbf66a804b64ef628),
    UINT64_C(0xc5bed8cc867b7f51), UINT64_C(0x8aeeace74e645255), UINT64_C(0xf036dc2f7e51db2c),
    UINT64_C(0x7f5e4d772e0f40a7), UINT64_C(0x05863dbf1e3ac9de), UINT64_C(0xe1fea520be311aaf),
    UINT64_C(0x9b26d5e88e0493d6), UINT64_C(0x144e44b0de5a085d), UINT64_C(0x6e963478ee6f8124),
    UINT64_C(0x21c640532670ac20), UINT64_C(0x5b1e309b16452559), UINT64_C(0xd476a1c3461bbed2),
    UINT64_C(0xaeaed10b762e37ab), UINT64_C(0x37deb6af5e9b8b5b), UINT64_C(0x4d06c6676eae0222),
    UINT64_C(0xc26e573f3ef099a9), UINT64_C(0xb8b627f70ec510d0), UINT64_C(0xf7e653dcc6da3dd4),
    UINT64_C(0x8d3e2314f6efb4ad), UINT64_C(0x0256b24ca6b12f26), UINT64_C(0x788ec2849684a65f),
    UINT64_C(0x9cf65a1b368f752e), UINT64_C(0xe62e2ad306bafc57), UINT64_C(0x6946bb8b56e467dc),
    UINT64_C(0x139ecb4366d1eea5), UINT64_C(0x5ccebf68aecec3a1), UINT64_C(0x2616cfa09efb4ad8),
    UINT64_C(0xa97e5ef8cea5d153), UINT64_C(0xd3a62e30fe90582a), UINT64_C(0xb0c7b7e3c7593bd8),
    UINT64_C(0xca1fc72bf76cb2a1), UINT64_C(0x45775673a732292a), UINT64_C(0x3faf26bb9707a053),
    UINT64_C(0x70ff52905f188d57), UINT64_C(0x0a2722586f2d042e), UINT64_C(0x854fb3003f739fa5),
    UINT64_C(0xff97c3c80f4616dc), UINT64_C(0x1bef5b57af4dc5ad), UINT64_C(0x61372b9f9f784cd4),
    UINT64_C(0xee5fbac7cf26d75f), UINT64_C(0x9487ca0fff135e26), UINT64_C(0xdbd7be24370c7322),
    UINT64_C(0xa10fceec0739fa5b), UINT64_C(0x2e675fb4576761d0), UINT64_C(0x54bf2f7c6752e8a9),
    UINT64_C(0xcdcf48d84fe75459), UINT64_C(0xb71738107fd2dd20), UINT64_C(0x387fa9482f8c46ab),
    UINT64_C(0x42a7d9801fb9cfd2), UINT64_C(0x0df7adabd7a6e2d6), UINT64_C(0x772fdd63e7936baf),
    UINT64_C(0xf8474c3bb7cdf024), UINT64_C(0x829f3cf387f8795d), UINT64_C(0x66e7a46c27f3aa2c),
    UINT64_C(0x1c3fd4a417c62355), UINT64_C(0x935745fc4798b8de), UINT64_C(0xe98f353477ad31a7),
    UINT64_C(0xa6df411fbfb21ca3), UINT64_C(0xdc0731d78f8795da), UINT64_C(0x536fa08fdfd90e51),
    UINT64_C(0x29b7d047efec8728),
};

const std::string alphabet64("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
const char pad = '=';
const char np = static_cast<char>(std::string::npos);
char table64vals[] = {62, np, np, np, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, np, np, np, np, np,
                      np, np, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                      18, 19, 20, 21, 22, 23, 24, 25, np, np, np, np, np, np, 26, 27, 28, 29, 30, 31,
                      32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

char table64(unsigned char c) {
  return (c < 43 || c > 122) ? np : table64vals[c - 43];
}

template <typename R, typename T>
R encode64_impl(const T& input) {
  typedef typename T::value_type value_type;
  R encoded;
  value_type c;
  const typename T::size_type length = input.size();

  encoded.reserve(length * 2);

  for (typename T::size_type i = 0; i < length; ++i) {
    c = static_cast<value_type>((input[i] >> 2) & 0x3f);
    encoded.push_back(alphabet64[c]);
    c = static_cast<value_type>((input[i] << 4) & 0x3f);

    if (++i < length) {
      c = static_cast<value_type>(c | static_cast<value_type>((input[i] >> 4) & 0x0f));
    }

    encoded.push_back(alphabet64[c]);

    if (i < length) {
      c = static_cast<value_type>((input[i] << 2) & 0x3c);
      if (++i < length) {
        c = static_cast<value_type>(c | static_cast<value_type>((input[i] >> 6) & 0x03));
      }

      encoded.push_back(alphabet64[c]);
    } else {
      ++i;
      encoded.push_back(pad);
    }

    if (i < length) {
      c = static_cast<value_type>(input[i] & 0x3f);
      encoded.push_back(alphabet64[c]);
    } else {
      encoded.push_back(pad);
    }
  }

  return encoded;
}

template <typename R, typename T>
R decode64_impl(const T& input) {
  typedef typename T::value_type value_type;
  value_type c, d;
  const typename T::size_type length = input.size();
  R decoded;

  decoded.reserve(length);

  for (typename T::size_type i = 0; i < length; ++i) {
    c = table64(input[i]);
    ++i;
    d = table64(input[i]);
    c = static_cast<value_type>((c << 2) | ((d >> 4) & 0x3));
    decoded.push_back(c);
    if (++i < length) {
      c = input[i];
      if (pad == c) {
        break;
      }

      c = table64(input[i]);
      d = static_cast<value_type>(((d << 4) & 0xf0) | ((c >> 2) & 0xf));
      decoded.push_back(d);
    }

    if (++i < length) {
      d = input[i];
      if (pad == d) {
        break;
      }

      d = table64(input[i]);
      c = static_cast<value_type>(((c << 6) & 0xc0) | d);
      decoded.push_back(c);
    }
  }

  return decoded;
}

}  // namespace

namespace common {
namespace utils {
namespace hash {

uint64_t crc64(uint64_t crc, const byte_t* data, uint64_t lenght) {
  for (uint64_t j = 0; j < lenght; ++j) {
    byte_t byte = data[j];
    crc = crc64_tab[static_cast<byte_t>(crc) ^ byte] ^ (crc >> 8);
  }

  return crc;
}

uint64_t crc64(uint64_t crc, const buffer_t& data) {
  return crc64(crc, data.data(), data.size());
}

}  // namespace hash

namespace base64 {

std::string encode64(const StringPiece& input) {
  return encode64_impl<std::string>(input);
}

std::string decode64(const StringPiece& input) {
  return decode64_impl<std::string>(input);
}

buffer_t encode64(const buffer_t& input) {
  return encode64_impl<buffer_t>(input);
}

buffer_t decode64(const buffer_t& input) {
  return decode64_impl<buffer_t>(input);
}

}  // namespace base64

namespace html {

std::string encode(const StringPiece& input) {
  std::string buffer;
  buffer.reserve(input.size());
  for (size_t pos = 0; pos != input.size(); ++pos) {
    char c = input[pos];
    switch (c) {
      case '&':
        buffer.append("&amp;");
        break;
      case '\"':
        buffer.append("&quot;");
        break;
      case '\'':
        buffer.append("&apos;");
        break;
      case '<':
        buffer.append("&lt;");
        break;
      case '>':
        buffer.append("&gt;");
        break;
      default:
        buffer += c;
        break;
    }
  }
  return buffer;
}

std::string decode(const StringPiece& input) {
  std::string str(input.begin(), input.end());
  std::string subs[] = {"& #34;", "&quot;", "& #39;", "&apos;", "& #38;", "&amp;", "& #60;", "&lt;",
                        "& #62;", "&gt;",   "&34;",   "&39;",   "&38;",   "&60;",  "&62;"};

  std::string reps[] = {"\"", "\"", "'", "'", "&", "&", "<", "<", ">", ">", "\"", "'", "&", "<", ">"};

  size_t found;
  for (int j = 0; j < 15; ++j) {
    do {
      found = str.find(subs[j]);
      if (found != std::string::npos) {
        str.replace(found, subs[j].length(), reps[j]);
      }
    } while (found != std::string::npos);
  }

  return str;
}

}  // namespace html

char* strdupornull(const std::string& src) {
  if (src.empty()) {
    return NULL;
  }
  const char* csrc = src.c_str();
  return strdupornull(csrc);
}

char* strdupornull(const char* src) {
  if (!src) {
    return NULL;
  }

  return strdup(src);
}

void freeifnotnull(void* ptr) {
  if (!ptr) {
    return;
  }

  free(ptr);
}

}  // namespace utils

#ifdef OS_POSIX
void create_as_daemon() {
  /* Fork off the parent process */
  pid_t pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Success: Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* On success: The child process becomes session leader */
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }

  /* Fork off for the second time*/
  pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Success: Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the root directory */
  /* or another appropriated directory */
  // chdir("/");

  /* Close all open file descriptors */
  for (int x = sysconf(_SC_OPEN_MAX); x > 0; x--) {
    close(x);
  }
}
#endif

long get_current_process_pid() {
  return static_cast<long>(getpid());
}
}  // namespace common
