#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstddef>
#include <stdexcept>


namespace itchbook {

class MappedFile {
public:
    explicit MappedFile(const char* path) {
        fd_ = ::open(path, O_RDONLY);

        if (fd_ < 0)
            throw std::runtime_error("open");

        struct stat st{};
        
        if (::fstat(fd_, & st) < 0)
            throw std::runtime_error("fstat");

        size_ = static_cast<std::size_t>(st.st_size);
        void* p = ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);

        if (p == MAP_FAILED)
            throw std::runtime_error("mmap");

        data_ = static_cast<const std::byte*>(p);
        ::madvise(p, size_, MADV_SEQUENTIAL);
    }

    ~MappedFile() {
        if (data_)
            ::munmap(const_cast<std::byte*>(data_), size_);
        
        if (fd_ >= 0)
            ::close(fd_);
    }

    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    const std::byte* data() const { return data_; }
    std::size_t size() const { return size_; }

private:
    int fd_ = -1;
    const std::byte* data_ = nullptr;
    size_t size_ = 0;
};

}
