#pragma once

#include <span>
#include <cstddef>
#include <bit>

template<typename Y>
Y& from_bytes(std::span<std::byte> bytes) {
    return *std::bit_cast<Y*>(bytes.data());
    //return *reinterpret_cast<Y*>(bytes.data());
}

template<typename Y>
std::span<Y> span_from_bytes(std::span<std::byte> bytes, size_t num = 1) {
    return std::span{
        std::bit_cast<Y*>(bytes.data()),
        //reinterpret_cast<Y*>(bytes.data()),
        num
    };
}

struct span_reader {
    std::span<std::byte> data;
    size_t input_size_t;
    std::endian input_endianness;

    span_reader(std::span<std::byte> data_):
        data(data_)
    {}

    std::span<std::byte> read_bytes(size_t size) {
        auto r = data.first(size);
        data = data.subspan(size);
        return r;
    }
};

template<typename T>
T fix_endianness(T value, std::endian input_endianness = std::endian::little) {
    if constexpr (!std::is_scalar_v<T>) {
        return value;
    }
    if (input_endianness == std::endian::native) {
        return value;
    }
    using A = std::array<std::byte, sizeof(T)>;
    A array = std::bit_cast<A>(value);
    std::ranges::reverse(array);
    return std::bit_cast<T>(array);
}

template<typename R, typename T>
requires std::is_scalar_v<T>
void read(R& r, T& v) {
    v = fix_endianness(from_bytes<T>(r.read_bytes(sizeof(v))), r.input_endianness);
}

struct input_size_t {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};

template<typename R>
void read(R& r, input_size_t& x) {
    x.data = from_bytes<uint64_t>(r.read_bytes(r.input_size_t));
}

template<typename T>
span_reader& operator&(span_reader &r, T& v) {
    read(r, v);
    return r;
}
