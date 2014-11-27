#include "scaner.cpp"

int main(int argc, char** argv)
{
    Scaner scaner;
    if (!scaner.openFiles(argc, argv))
        return 0;
    do
    {
        scaner.makeNextLexem();
    } while (!scaner.isEOF());
    scaner.closeFiles();

    Parser parser;

    if (!parser.openFiles(argc, argv))
        return 0;
    parser.makeXMLTree();
    return 1;
}
