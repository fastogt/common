#include <common/uri/url_canon_stdstring.h>

namespace common {
namespace uri {

StdStringCanonOutput::StdStringCanonOutput(std::string* str) : CanonOutput(), str_(str) {
  cur_len_ = static_cast<int>(str_->size());  // Append to existing data.
  buffer_ = str_->empty() ? NULL : &(*str_)[0];
  buffer_len_ = static_cast<int>(str_->size());
}

StdStringCanonOutput::~StdStringCanonOutput() {
  // Nothing to do, we don't own the string.
}

void StdStringCanonOutput::Complete() {
  str_->resize(cur_len_);
  buffer_len_ = cur_len_;
}

void StdStringCanonOutput::Resize(int sz) {
  str_->resize(sz);
  buffer_ = str_->empty() ? NULL : &(*str_)[0];
  buffer_len_ = sz;
}

}  // namespace uri
}  // namespace common
