// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#pragma once

#include <cassert>
#include <fcntl.h>
#include <memory>
#include <string_view>
#include <variant>

#include "file.h"

namespace wasmfs::path {

// Typically -ENOTDIR or -ENOENT.
using Error = long;

// The parent directory and the name of an entry within it. The returned string
// view is either backed by the same memory as the view passed to `parseParent`
// or is a view into a static string.
using ParentChild = std::pair<std::shared_ptr<Directory>, std::string_view>;

struct ParsedParent {
private:
  std::variant<Error, ParentChild> val;

public:
  ParsedParent(Error err) : val(err) {}
  ParsedParent(ParentChild pair) : val(pair) {}
  // Always ok to call, returns 0 if there is no error.
  long getError() {
    if (auto* err = std::get_if<Error>(&val)) {
      assert(*err != 0 && "Unexpected zero error value");
      return *err;
    }
    return 0;
  }
  // Call only after checking for an error.
  ParentChild& getParentChild() {
    auto* ptr = std::get_if<ParentChild>(&val);
    assert(ptr && "Unhandled path parsing error!");
    return *ptr;
  }
};

ParsedParent parseParent(std::string_view path, __wasi_fd_t basefd = AT_FDCWD);

struct ParsedFile {
private:
  std::variant<Error, std::shared_ptr<File>> val;

public:
  ParsedFile(Error err) : val(err) {}
  ParsedFile(std::shared_ptr<File> file) : val(std::move(file)) {}
  // Always ok to call. Returns 0 if there is no error.
  long getError() {
    if (auto* err = std::get_if<Error>(&val)) {
      assert(*err != 0 && "Unexpected zero error value");
      return *err;
    }
    return 0;
  }
  // Call only after checking for an error.
  std::shared_ptr<File>& getFile() {
    auto* ptr = std::get_if<std::shared_ptr<File>>(&val);
    assert(ptr && "Unhandled path parsing error!");
    return *ptr;
  }
};

ParsedFile parseFile(std::string_view path, __wasi_fd_t basefd = AT_FDCWD);

} // namespace wasmfs::path
