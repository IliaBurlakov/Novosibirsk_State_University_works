#include <iostream>
#include <conio.h>
#include <tuple>
#include <string>
#include <fstream>
#include "print_tuple.h"
#include "csv_parser.h"

int main()
{
    // Проверка вывода кортежа
    std::tuple<int, std::string, double, std::string> t1 = std::make_tuple(-999, "just a string", 3.14, "pi");
    std::cout << t1 << std::endl;
    std::ifstream csv_stream("test.csv"); //Чтение из валидного файла
    if(!csv_stream) {
        std::cerr << "Cant open test.csv\n";
        return 1;
    }
    CsvParser<double, std::string, double> parser(csv_stream, 0);
        for (auto row : parser) {
        std::cout << row << "\n";
    }
   /*
    CsvParser<int, std::string, double> parser(std::cin, 0); // Чтение из std::cin (Ctrl + D)
    std::vector<std::tuple<int, std::string, double>> rows;

    for (auto row : parser) {
        rows.push_back(row); 
    }
    for (const auto &row : rows) {
        std::cout << row << "\n";
    }*/

    _getch();
    return 0;
}
