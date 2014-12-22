#ifndef SCANER_H
#define SCANER_H

#include <vector>
#include "tinyxml/tinyxml.h"
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
    int line,
        codeLineWrap,
        indexLexemTypeId,
        lineWithError;
    LexemType lexemType;
    char valueType[4],
         whatIsExpected[100];
    int dValue;
    float fValue;
    char lexem[LENGTH_LEXEM];
    bool _isEOF,
          isAvaibleForBreak;

    TiXmlDocument doc;
    TiXmlDeclaration* decl;
    TiXmlElement* program;

    std::vector <Identifier> identifiers;
    Identifier id;

    void parseLine();
    bool inline isLetter(int ch);
    void printTeg();

    bool definitions(TiXmlNode* node);
    bool tools();
    int readId();
    bool handleIDs();

    bool composite(TiXmlNode* node);//$ составной = "start" оператор { ";" оператор } "stop".
    bool opRead(TiXmlNode* node); //$ ввода = read переменная { "," переменная }.
    bool opWrite(TiXmlNode* node); //$ вывода = write ( выражение | спецификатор ) { "," ( выражение | спецификатор ) }.
    bool opGoto(TiXmlNode* node); //goto имя_метки.
    bool opIf(TiXmlNode* node); //$ условный = if выражение then непомеченный [ else непомеченный ].
    bool opWhile(TiXmlNode* node); //$ цикла = while выражение do оператор { ";" оператор } end.
    bool expression(TiXmlNode* node); //$ выражение = "(" операнд операнд операция ")" | "(" операнд "minus" ")" | операнд .
    bool opCall(TiXmlNode* node); //$ вызов = идентификатор "(" [ переменная { "," переменная } ] ")".
    void opBreak(TiXmlNode* node);
    bool unmarked(TiXmlNode* node);
    void label(TiXmlNode* node);
    bool assignOrCast(TiXmlNode* node);

public:
    bool openFiles(int argc, char** argv);
    bool isEOF();
    void makeXMLTree();
    void closeFiles();
};

#endif // SCANER_H
