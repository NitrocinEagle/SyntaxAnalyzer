#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "definitions.h"

struct Identifier {
public:
    char valueType[5];
    int length;
    char nameID[LENGTH_LEXEM];
void toZero();
};

class Scaner
{
private:
    FILE
        *input,
        *output;
    char                          // line lex:<lexemId>[  <typeOfValue>:<*_value>][   val:<lexem>]
        lexem[LENGTH_LEXEM],      // For keeping current lexem
        ch;
    int
        codeError,
        line,           //Current line
        column,         //Current column
        i_lex,          // Index of lexem
        int_value,
        codeLineWrap;
    float
        float_value;
    bool
        _isEOF,
        acceptToRead,
        wasError;

    bool inline isLetter(int ch);
    bool inline isBin(int ch);
    bool inline isOctal(int ch);
    bool inline isDigit(int ch);
    bool inline isHex(int ch);
    bool inline isSkip(int ch);
    bool inline isIgnore(int ch);

    void kWOrId();
    void numberOrLabel();
    void realNumb();
    void realNumb1();
    void realNumb2();
    void intNumb();
    void intNumb(int base);
    void comment();
    void label();

    int toInt(int base);
    void printError(const int errCode);
    void readChar();
    void printLexemOut(const char *lxId, const char *type);
    void handleOneCharLexem(char ch);
    void skipWrongLexem();
public:
    Scaner();
    ~Scaner();
    void makeNextLexem();
    bool openFiles(int argc, char** argv);
    bool isEOF();
    void closeFiles();
};

class Parser // <line>\t:<LexemType>[\t<ValueType>:<Value>]\t<Lexem>
{
private:
    FILE *input,
        *lexemList,
        *parseTree;
    int line, codeLineWrap, indexLexemTypeId;
    LexemType lexemType;
    char valueType[4];
    int dValue;
    float fValue;
    char lexem[LENGTH_LEXEM];
    bool _isEOF;

    std::vector <Identifier> identifiers;
    Identifier id;

    void parseLine();
    bool inline isLetter(int ch);
    void printTeg();

    void definitions();
    void tools();
    void programm();
    int readId();
    void handleIDs();

    void composite(int level);//$ составной = "start" оператор { ";" оператор } "stop".
    void opRead(); //$ ввода = read переменная { "," переменная }.
    void opWrite(); //$ вывода = write ( выражение | спецификатор ) { "," ( выражение | спецификатор ) }.
    void opGoto(); //goto имя_метки.
    void opIf(); //$ условный = if выражение then непомеченный [ else непомеченный ].
    void opWhile(); //$ цикла = while выражение do оператор { ";" оператор } end.
    void opCallProc(); //$ вызов = идентификатор "(" [ переменная { "," переменная } ] ")".
    void opCast(); //$ приведение = "(" переменная переменная cast ")".
    void opLet(); //$ присваивание = "(" выражение переменная "let" ")".
    void expression(int level); //$ выражение = "(" операнд операнд операция ")" | "(" операнд "minus" ")" | операнд .
    void operand(); //$ операнд = выражение | переменная | целое | действительное.



public:
    bool openFiles(int argc, char** argv);
    bool isEOF();
    void makeXMLTree();
    void closeFiles();
};

