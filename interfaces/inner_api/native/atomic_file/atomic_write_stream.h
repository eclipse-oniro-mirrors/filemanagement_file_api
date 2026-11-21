/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INTERFACES_INNER_API_NATIVE_ATOMIC_FILE_ATOMIC_WRITE_STREAM_H
#define INTERFACES_INNER_API_NATIVE_ATOMIC_FILE_ATOMIC_WRITE_STREAM_H

#include <cstring>
#include <ostream>
#include <string>
#include <streambuf>

#include <fcntl.h>
#include <unistd.h>

namespace OHOS {
namespace FileManagement {
namespace FileApi {

class FdStreamBuf final : public std::streambuf {
public:
    explicit FdStreamBuf(int fd) : fd_(fd) { setp(buffer_, buffer_ + bufSize); }
    ~FdStreamBuf() override = default;

    FdStreamBuf(const FdStreamBuf &) = delete;
    FdStreamBuf &operator=(const FdStreamBuf &) = delete;

protected:
    int_type overflow(int_type c) override
    {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *pptr() = static_cast<char>(c);
            pbump(1);
        }
        if (!FlushBuffer()) {
            return traits_type::eof();
        }
        return c;
    }

    std::streamsize xsputn(const char *s, std::streamsize n) override // NOLINT: override std::streambuf virtual
    {
        if (n <= 0) {
            return 0;
        }
        if (!FlushBuffer()) {
            return 0;
        }
        return WriteAll(s, static_cast<size_t>(n));
    }

    int sync() override { return FlushBuffer() ? 0 : -1; } // NOLINT: override std::streambuf virtual

private:
    bool FlushBuffer()
    {
        std::streamsize len = pptr() - pbase();
        if (len <= 0) {
            return true;
        }
        std::streamsize written = WriteAll(pbase(), static_cast<size_t>(len));
        if (written < len) {
            return false;
        }
        pbump(-static_cast<int>(len));
        return true;
    }

    std::streamsize WriteAll(const char *s, size_t n)
    {
        size_t total = 0;
        while (total < n) {
            ssize_t w = write(fd_, s + total, n - total);
            if (w < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            total += static_cast<size_t>(w);
        }
        return static_cast<std::streamsize>(total);
    }

    int fd_ = -1;
    static constexpr int bufSize = 8192;
    char buffer_[bufSize] = {0};
};

class AtomicWriteStream final : public std::ostream {
public:
    AtomicWriteStream(int fd, std::string tmpPath)
        : std::ostream(nullptr), fd_(fd), tmpPath_(std::move(tmpPath)), buf_(fd)
    {
        rdbuf(&buf_);
    }

    ~AtomicWriteStream() override
    {
        if (!committed_) {
            Discard();
        }
    }

    AtomicWriteStream(const AtomicWriteStream &) = delete;
    AtomicWriteStream &operator=(const AtomicWriteStream &) = delete;

    int GetFd() const { return fd_; }
    const std::string &GetTmpPath() const { return tmpPath_; }

    bool IsCommitted() const { return committed_; }
    void MarkCommitted() { committed_ = true; }
    void Discard();

private:
    int fd_ = -1;
    std::string tmpPath_;
    bool committed_ = false;
    FdStreamBuf buf_;
};

} // namespace FileApi
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_INNER_API_NATIVE_ATOMIC_FILE_ATOMIC_WRITE_STREAM_H
