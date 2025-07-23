#ifndef FLATMAP_H
#define FLATMAP_H

#include <cstddef>
#include <utility>
#include <stdexcept>
#include <iostream>

namespace FlatMapSpace {

template <typename Key, typename Value>
class FlatMap {
private:
    struct KeyValuePair {
        Key key;
        Value value;

        KeyValuePair() : key(), value() {}
        KeyValuePair(const Key &k, const Value &v) : key(k), value(v) {}

        bool operator<(const KeyValuePair &other) const {
            return key < other.key;
        }
    };

    KeyValuePair *data;
    size_t capacity;
    size_t count;
    const size_t CapacityMultiplier = 2;

    void resize() {
        size_t new_capacity = ((capacity == 0) ? CapacityMultiplier : capacity * CapacityMultiplier);
        KeyValuePair *new_data = new KeyValuePair[new_capacity];

        for (size_t i = 0; i < count; ++i) {
            new_data[i] = data[i];
        }

        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }

    int binarySearch(const Key &key) const {
        if (count == 0)
            return -1;

        size_t left = 0;
        size_t right = count - 1;

        while (left <= right) {
            size_t mid = left + (right - left) / 2;
            if (data[mid].key == key) {
                return mid;
            }
            if (data[mid].key < key) {
                left = mid + 1;
            } else {
                if (right == 0)
                    return -1;
                right = mid - 1;
            }
        }
        return -1;
    }

public:
    FlatMap() : data(nullptr), capacity(0), count(0) {}

    FlatMap(const FlatMap &other_map) {
        capacity = other_map.capacity;
        count = other_map.count;
        data = new KeyValuePair[capacity];
        for (size_t i = 0; i < count; ++i) {
            data[i] = other_map.data[i];
        }
    }

    ~FlatMap() {
        delete[] data;
    }

    FlatMap &operator=(const FlatMap &other_map) {
        if (this == &other_map)
            return *this;

        delete[] data;
        count = other_map.count;
        capacity = other_map.capacity;
        data = new KeyValuePair[capacity];
        for (size_t i = 0; i < count; ++i) {
            data[i] = other_map.data[i];
        }
        return *this;
    }

    size_t size() const {
        return count;
    }

    Value &operator[](const Key &key) {
        int index = binarySearch(key);
        if (index != -1)
            return data[index].value;

        if (count == capacity)
            resize();

        size_t pos = count;
        for (size_t i = 0; i < count; ++i) {
            if (key < data[i].key) {
                pos = i;
                break;
            }
        }

        for (size_t i = count; i > pos; --i) {
            data[i] = data[i - 1];
        }

        data[pos] = KeyValuePair(key, Value());
        ++count;

        return data[pos].value;
    }

    bool contains(const Key &key) const {
        return binarySearch(key) != -1;
    }

    size_t erase(const Key &key) {
        int index = binarySearch(key);
        if (index != -1) {
            for (size_t i = index; i < count - 1; ++i) {
                data[i] = data[i + 1];
            }
            data[count - 1] = KeyValuePair();
            --count;
            return 1;
        }
        return 0;
    }

    void clear() {
        delete[] data;
        data = nullptr;
        count = 0;
        capacity = 0;
    }

    class iterator {
    private:
        KeyValuePair *ptr;

    public:
        iterator(KeyValuePair *p) : ptr(p) {}

        iterator &operator++() {
            ++ptr;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        KeyValuePair &operator*() const {
            return *ptr;
        }

        KeyValuePair *operator->() const {
            return ptr;
        }

        bool operator==(const iterator &other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const iterator &other) const {
            return ptr != other.ptr;
        }
    };

    iterator begin() {
        return iterator(data);
    }

    iterator end() {
        return iterator(data + count);
    }
};

}

#endif
