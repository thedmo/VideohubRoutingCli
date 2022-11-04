#include <cli.hpp>

int main(const int argc, const char *argv[])
{
    cli * commandlinelinterface = new cli();

    commandlinelinterface -> Evaluate(argc, argv);
}