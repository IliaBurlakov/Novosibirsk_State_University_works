#pragma once
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <type_traits>
#include "print_tuple.h" 

std::vector<std::string> split_line(const std::string &line, char delim = ',') {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, delim)) {
        result.push_back(field);
    }
    return result;
}

template <typename T>
T convert_field(const std::string &field) {
    if constexpr (std::is_same_v<T, std::string>) {
        return field;
    } else {
        if (!field.empty() && (((field[1] == '+' || field[1] == '-') && field[2] == '.') || field[1] == '.')|| field[0] = '.') {
            throw std::runtime_error("Invalid field format (missing leading zero): '" + field + "'");
        }
        std::istringstream iss(field);
        T val;
        if (!(iss >> val) || !iss.eof()) {
            throw std::runtime_error("Failed to convert field: '" + field + "'");
        }
        return val;
    }
}

template <typename... Ts>
class CsvParser {
public:
    CsvParser(std::istream &is, std::size_t skip = 0) : is_(is) {
        for (std::size_t i = 0; i < skip; ++i) {
            std::string tmp;
            if (!std::getline(is_, tmp)) {
                break;
            }
        }
    }

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::tuple<Ts...>;
        using reference = const value_type &;
        using pointer = const value_type *;
        using difference_type = std::ptrdiff_t;

        iterator() : is_ptr_(nullptr), end_(true) {}
        iterator(std::istream *is_ptr) : is_ptr_(is_ptr), end_(false) {
            ++(*this);
        }

        iterator &operator++() {
            if (is_ptr_ == nullptr) {
                end_ = true;
                return *this;
            }

            std::string line;
            if (!std::getline(*is_ptr_, line)) {
                is_ptr_ = nullptr;
                end_ = true;
                return *this;
            }

            try {
                auto fields = split_line(line, ',');
                if (fields.size() != sizeof...(Ts)) {
                    throw std::runtime_error("Field count doesnt match in line: '" + line + "'");
                }
                value_ = make_tuple(fields, std::make_index_sequence<sizeof...(Ts)>{});
            } catch (const std::runtime_error &e) {
                std::cerr << "Line processing error: " << e.what() << std::endl;
                is_ptr_ = nullptr;
                end_ = true;
            }

            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator &other) const {
            return is_ptr_ == other.is_ptr_ && end_ == other.end_;
        }

        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }

        reference operator*() const { return value_; }
        pointer operator->() const { return &value_; }

    private:
        template <std::size_t... I>
        value_type make_tuple(const std::vector<std::string> &fields, std::index_sequence<I...>) {
            return std::tuple<Ts...>(convert_field<Ts>(fields[I])...);
        }

        std::istream *is_ptr_;
        value_type value_;
        bool end_ = true;
    };

    iterator begin() { return iterator(&is_); }
    iterator end() { return iterator(nullptr); }

private:
    std::istream &is_;
};
