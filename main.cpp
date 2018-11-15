#include "parser.h"
#include "TypeChecker.h"

int main()
{
    Driver driver(std::cin);
    driver.parse();
    if (driver.is_parsed()) {
        //driver.json(std::cout);
        TypeChecker TC(&driver);
        TC.CheckClasses();
        TC.FindLCAs();
        TC.CheckMethodsAndTypes();
    } else {
        std::cerr << "Unable to parse!" << std::endl;
    }
}
