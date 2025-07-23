#pragma once
#include <ostream>
#include <tuple>

// Шаблонная структура для рекурсивной печати кортежа
template <std::size_t I, typename... Args>
struct print_tuple {
    static void print(std::ostream& os, const std::tuple<Args...>& t) {
        if (I > 0) {
            os << ", ";
        }
        os << std::get<I>(t);
        print_tuple<I + 1, Args...>::print(os, t);
    }
};

// Частичная специализация для завершения рекурсии
template <typename... Args>
struct print_tuple<sizeof...(Args), Args...> {
    static void print(std::ostream&, const std::tuple<Args...>&) {}
};

// Перегрузка оператора << для вывода кортежей
template <typename... Args>
std::ostream& operator<<(std::ostream& os, const std::tuple<Args...>& t) {
    os << "(";
    print_tuple<0, Args...>::print(os, t);
    os << ")";
    return os;
}
