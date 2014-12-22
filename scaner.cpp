#include "scaner.h"
#include "definitions.h"
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <cfloat>

Scaner::Scaner()
{
    line = 1;
    column = 1;
    _isEOF = false;
    wasError = false;
    ch = '\0';
    acceptToRead = true;
    codeLineWrap = (int)'\n';

}
Scaner::~Scaner()
{
}
bool Scaner::isEOF()
{
    return _isEOF;
}
bool Scaner::openFiles(int argc, char** argv)
{
    if(argc < 4)
    {
        printf("Error:Params:Incorrect command format\n");
        return false;
    }
    if((input = fopen(argv[1], "r")) == NULL)
    {
        printf("Error:%s: cannot open input file\n", argv[1]);
        return false;;
    }

    if((output = fopen(argv[2], "w")) == NULL)
    {
        printf("Error:%s: cannot open output file\n", argv[2]);
        return false;
    }
    return true;
}
void Scaner::closeFiles()
{
    fclose(input);
    fclose(output);
}
bool inline Scaner::isLetter(int ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
bool inline Scaner::isBin(int ch)
{
    return ((ch == '0' || ch == '1'));
}
bool inline Scaner::isOctal(int ch)
{
    return ((ch >= '0' && ch <= '7'));
}
bool inline Scaner::isDigit(int ch)
{
    return ((ch >= '0' && ch <= '9'));
}
bool inline Scaner::isHex(int ch)
{
    return ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'));
}
bool inline Scaner::isSkip(int ch)
{
    return (ch == ' ' || ch == '\t' || ch == codeLineWrap || ch == '\f');
}
bool inline Scaner::isIgnore(int ch)
{
    return (ch > 0 && ch < ' ' && ch != '\t' && ch != codeLineWrap && ch != '\f');
}
void Scaner::comment()
{
    readChar();
    if (ch != '/')
    {
        codeError = ERROR_UNFINISHED_COMMENT;
        return;
    }
    do
    {
        readChar();
        if (ch == codeLineWrap)
            return;
        if (feof(input))
        {
            _isEOF = true;
            return;
        }
    } while (true);
}
void Scaner::kWOrId()
{
    bool isThereDigit = false;
    do
    {
        lexem[++i_lex] = ch;
        readChar();
        if (isDigit(ch))
            isThereDigit = true;
    }
    while (isDigit(ch) || isLetter(ch));
    lexem[i_lex+1] = '\0';

    if (!isSkip(ch) && ch != ';' && ch != ',' && ch != '(' && ch != ')' && ch != '[' && ch != ']' && !feof(input))
    {
        i_lex++;
        codeError = ERROR_UNRECOGNIZABLE_LEXEM;
        return;
    }
    if (!isThereDigit)
        for (int i = 0; i < NUMB_KW; i++)
            if (strcmp(KEY_WORDS[i], lexem) == 0)
            {
                printLexemOut(KEY_WORDS_ID[i],"");
                return;
            }
    printLexemOut("Id","");
}
void Scaner::intNumb(int base)  //На входе: ch = #, lexem = "2#"
{
    bool isNumb;
    --i_lex;
    do
    {
        lexem[++i_lex] = ch;
        readChar();
        if (base == 2) isNumb = isBin(ch);
        else if (base == 8) isNumb = isOctal(ch);
        else if (base == 10) isNumb = isDigit(ch);
        else if (base == 16) isNumb = isHex(ch);
    }
    while (isNumb);
    lexem[++i_lex] = '\0';
    if (!isSkip(ch) && ch != ';' && ch != ',' && ch != '(' && ch != ')' && ch != '[' && ch != ']')
    {
        codeError = ERROR_WRONG_INT_NUMBER;
        return;
    }
    int_value = toInt(base);
    printLexemOut("Int","int");
}
void Scaner::label()
{
    lexem[i_lex+1] = '\0';
    int_value = strtoul(lexem, NULL, 10);
    lexem[++i_lex] = ch;
    lexem[++i_lex] = '\0';
    printLexemOut("Label","int");
    return;
}
void Scaner::intNumb()
{
A:
    if (ch != ';' && ch != ',' && ch != '(' && ch != ')' && ch != '[' && ch != ']')
        readChar();
    if (ch == ':')
    {
        label();
        //        lexem[++i_lex] = ch;
        return;
    }
    else if (ch == ' ')
        goto A;
    acceptToRead = false;
    lexem[++i_lex] = '\0';
    int_value = strtol(lexem, NULL, 10);
    printLexemOut("Int", "int");
    if (ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']')
        acceptToRead = true;
}
void Scaner::numberOrLabel() // в lexem - "N"
{
    lexem[++i_lex] = ch;
    readChar();
    if (isDigit(ch))
    {
        lexem[++i_lex] = ch; // в lexem - "NN"
        readChar();
        if (ch == '#')
        {
            lexem[++i_lex] = ch; // в lexem - "NN#"
            char temp[3] = {lexem[0], lexem[1], '\0'};
            if (strcmp(temp, "10") == 0)    { intNumb(10); return; }
            else if (strcmp(temp, "16") == 0)   { intNumb(16); return; }
            else { codeError = ERROR_WRONG_BASE; return; }
        }
        else if (feof(input))
        {
            intNumb();
            return;
        }
        else // иначе это  N e . oneCharLexem : skip smthElse
            goto A;
    }
    else if (ch == '#')
    {
        lexem[++i_lex] = ch;
        if (lexem[0] == '2')    { intNumb(2); return; }
        else if (lexem[0] == '8')   { intNumb(8); return; }
        else
        {
            codeError = ERROR_WRONG_BASE;
            return;
        }
    }
A: // обработка N e . oneCharLexem : skip smthElse
    if (isDigit(ch))
    {
        lexem[++i_lex] = ch;
        readChar();
        goto A;
    }
    else if (ch == '/')
    {
        intNumb();
        comment();
    }
    else if (ch == 'e') { realNumb1(); }
    else if (ch == codeLineWrap) { line --; intNumb(); line++;}
    else if (isSkip(ch)) { intNumb(); }
    else if (ch == '.') { realNumb2(); }
    else if (ch == ':') { label(); }
    else if (ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']')
    {
        intNumb();
        return;
    }
    else { lexem[++i_lex] = ch; codeError = ERROR_UNRECOGNIZABLE_LEXEM; }
}
void Scaner::realNumb1() //Если встречается e. В lexem - "N{N}e"
{
    lexem[++i_lex] = ch;
    readChar();
    if (ch != '+' && ch != '-' && !isDigit(ch))
    {
        ++i_lex;
        codeError = ERROR_WRONG_FORMAT_REAL_NUMBER;
        return;
    }
    do
    {
        lexem[++i_lex] = ch;
        readChar();
    }
    while (isDigit(ch));
    if (isSkip(ch) || ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || feof(input))
        realNumb();
    else
    {
        codeError = ERROR_WRONG_FORMAT_REAL_NUMBER;
        ++i_lex;
    }
}
void Scaner::realNumb2() // Если встречается точка. В lexem - "N{N}."
{
    lexem[++i_lex] = ch;
    readChar();
    if (!isDigit(ch))
    {
        ++i_lex;
        codeError = ERROR_WRONG_FORMAT_REAL_NUMBER;
        return;
    }
    else
        do
    {
        lexem[++i_lex] = ch;
        readChar();
    }
    while (isDigit(ch));
    if (isSkip(ch) || ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || feof(input))
        realNumb();
    else if (ch == 'e')
    {
        realNumb1();
        return;
    }
    else
    {
        codeError = ERROR_WRONG_FORMAT_REAL_NUMBER;
        ++i_lex;
        return;
    }
}
void Scaner::readChar()
{
    if ((ch = fgetc(input)) == codeLineWrap)
    {
        line++;
        column = 0;
    }
    else
        ++column;
    ch = tolower(ch);
}
void Scaner::handleOneCharLexem(char ch)
{
    switch(ch)
    {
    case ';': fprintf(output, "%d\tlex:Semicolon\tval:%c\n", line, ch); break;
    case ',': fprintf(output, "%d\tlex:Comma\tval:%c\n", line, ch); break;
    case '[': fprintf(output, "%d\tlex:LSB\tval:%c\n", line, ch); break;
    case ']': fprintf(output, "%d\tlex:RSB\tval:%c\n", line, ch); break;
    case '(': fprintf(output, "%d\tlex:LRB\tval:%c\n", line, ch); break;
    case ')': fprintf(output, "%d\tlex:RRB\tval:%c\n", line, ch); break;
    }
}
void Scaner::printLexemOut(const char *lxId, const char *type) // line lex:<lexemId>[  <typeOfValue>:<*_value>][   val:<lexem>]
{
    if (ch == codeLineWrap)
        line--;
    fprintf(output, "%d\tlex:%s", line, lxId);
    if (strcmp(type, "int") == 0)
        fprintf(output, "\t%s:%d", type, int_value);
    else if (strcmp(type, "real") == 0)
        fprintf(output, "\t%s:%f", type, float_value);
    fprintf(output, "\tval:%s\n", lexem); // А пока вот так
    if (ch == codeLineWrap)
        line++;
}
void Scaner::makeNextLexem()
{
startRecognizeNextLexem:
    if (acceptToRead)
        readChar();
    acceptToRead = true;
    i_lex = -1;
    codeError = -1;
    lexem[0] = '\0';
    if (feof(input))
    {
        _isEOF = true;
        //if (!wasError)
          //  printf("OK");
        return;
    }
    else if (isLetter(ch)) { kWOrId(); }
    else if (isDigit(ch))   { numberOrLabel(); }
    else if ( isSkip(ch) || isIgnore(ch))   { goto startRecognizeNextLexem; }
    else if (ch == '/') { comment(); return; }
    else if (!isSkip(ch) && ch != ';' && ch != ',' && ch != '(' && ch != ')' && ch != '[' && ch != ']')
    {
        ++i_lex;
        skipWrongLexem();
        codeError = ERROR_UNRECOGNIZABLE_LEXEM;
    }
    printError(codeError);
    if (ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']')
    {
        handleOneCharLexem(ch);
    }
}
void Scaner::printError(int errCode)
{
    if (errCode != -1)
    {
        skipWrongLexem();
        if (ch == codeLineWrap)
            line--;
        wasError = true;
        fprintf(output,"%d\tlex:Error\tval:%s\n", line, lexem);
        switch(errCode)
        {
        case ERROR_UNRECOGNIZABLE_LEXEM:
            printf("Error: %d: Unrecognizable lexem %s\n", line, lexem);
            break;
        case ERROR_WRONG_BASE:
            printf("Error: %d: Wrong base of number in lexem %s\n", line, lexem);
            break;
        case ERROR_WRONG_INT_NUMBER:
            printf("Error: %d: Wrong write of int number in lexem %s\n", line, lexem);
            break;
        case ERROR_WRONG_FORMAT_REAL_NUMBER:
            printf("Error: %d: Wrong format of real number in lexem %s\n", line, lexem);
            break;
        case ERROR_OVERFLOW_REAL_TYPE:
            printf("Error: %d: Owerflow of real number in lexem %s\n", line, lexem);
            break;
        }
        if (ch == codeLineWrap)
            line++;
    }

}
int Scaner::toInt(int base)
{
    char temp[20];
    strcmp(temp, lexem);
    int k = 3;
    if (base < 10)
        k = 2;
    for (int i = 0; i < strlen(lexem) - k; i++)
    {
        temp[i] = lexem[i+k];
        temp[i+1] = '\0';
    }
    return strtol(temp, NULL, base);
}
void Scaner::skipWrongLexem()
{
    --i_lex;
    do
    {
        if (ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '/' || isSkip(ch) || (feof(input)))
        {
            lexem[++i_lex] = '\0';
            return;
        }
        lexem[++i_lex] = ch;
        readChar();
    }
    while (true);
}
void Scaner::realNumb()
{
    lexem[++i_lex] = '\0';
    float_value = strtof(lexem, NULL);
    if (float_value > FLT_MIN && float_value < FLT_MAX)
        printLexemOut("Real","real");
    else
    {
        codeError = ERROR_OVERFLOW_REAL_TYPE;
    }
}

bool inline Parser::isLetter(int ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
bool Parser::openFiles(int argc, char** argv)
{
    if(argc < 4)
    {
        printf("Error:Params:Incorrect command format\n");
        return false;
    }
    if((lexemList = fopen(argv[2], "r")) == NULL)
    {
        printf("Error:%s: cannot open input file\n", argv[1]);
        return false;;
    }

    if((parseTree = fopen(argv[3], "w")) == NULL)
    {
        printf("Error:%s: cannot open output file\n", argv[2]);
        return false;
    }
    codeLineWrap = (int)'\n';
    _isEOF = false;
    isAvaibleForBreak = false;
    return true;
}
void Parser::parseLine() //<line>\tlex:<LexemType>[\t<ValueType>:<Value>]\tval:<Lexem>
{
    int i = -1;
    valueType[0] = '\0';
    char ch, buffer[LENGTH_LEXEM];
    if (fscanf(lexemList, "%d", &line) == -1)
    {
        _isEOF = true;
        lexemType = lexEOF;
        return;
    }
    for (int j = 0; j < 5; j++)
        ch = fgetc(lexemList); //Считали \t:lex:
    ch = fgetc(lexemList);
    do
    {
        buffer[++i] = ch;
        ch = fgetc(lexemList);
    } while (isLetter(ch)); //Считали <LexemType>
    buffer[++i] = '\0';
    for (i = 0; i < NUMB_LEXEM_TYPES; i++)
        if (strcmp(LEXEM_TYPES_ID[i], buffer) == 0)
        {
            lexemType = LEXEM_TYPES[i];
            indexLexemTypeId = i;
            break;
        }
    ch = fgetc(lexemList);
    if (ch == 'v')
    {
        i = -1;
        for (int j = 0; j < 4; j++)
            ch = fgetc(lexemList); //Считали val:
        do
        {
            lexem[++i] = ch;
            ch = fgetc(lexemList);
        } while (ch != codeLineWrap);
        lexem[++i] = '\0';
    }
    else
    {
        i = -1;
        do
        {
            buffer[++i] = ch;
            ch = fgetc(lexemList);
        } while (isLetter(ch)); //Считали <ValueType>
        buffer[++i] = '\0';
        if (strcmp("real", buffer) == 0)
        {
            fscanf(lexemList, "%f", &fValue);
            strcpy(valueType, buffer);
            sprintf(lexem, "%f", fValue);
        }
        else
        {
            fscanf(lexemList, "%d", &dValue);
            strcpy(valueType, buffer);
            sprintf(lexem, "%d", dValue);
        }
        do
            ch = fgetc(lexemList);
        while (ch != codeLineWrap);
    }
}
void Parser::closeFiles()
{
    fclose(lexemList);
    fclose(parseTree);
}
void Parser::makeXMLTree()
{
    decl = new TiXmlDeclaration("1.0", "", "");
    doc.LinkEndChild(decl);
    program = new TiXmlElement("program");
    doc.LinkEndChild(program);

    parseLine();
    lineWithError = line;
    if (lexemType == lexVar)
    {
        parseLine();
        lineWithError = line;
        if (!definitions(program))
        {
            printf("Error: %d: Expected %s", lineWithError, whatIsExpected);
            return;
        }
        if (lexemType == lexTools)
            if (!tools())
            {
                printf("Error: %d: Expected %s", lineWithError, whatIsExpected);
                return;
            }
    }
    else if (lexemType == lexTools)
    {
        if (!tools())
        {
            printf("Error: %d: Expected %s", lineWithError, whatIsExpected);
            return;
        }
    }
    else if (lexemType != lexStart)
    {
        if (lexemType == lexReal || lexemType == lexInt)
            printf("Error: %d: Expected \"var\"", lineWithError);
        else if (lexemType == lexProc)
            printf("Error: %d: Expected \"tools\"", lineWithError);
        else
            printf("Error: %d: Expected \"Start\"", lineWithError);
        return;
    }
    if (lexemType != lexStart)
    {
        lineWithError = line;
        printf("Error: %d: Expected \"start\"", lineWithError);
        return;
    }
    if (!composite(program))
    {
        printf("Error: %d: Expected %s", lineWithError, whatIsExpected);
        return;
    }
    printf("OK");
    TiXmlText* text = new TiXmlText("");
    program->LinkEndChild(text);
    doc.SaveFile(parseTree);
}
bool Parser::definitions(TiXmlNode* node)
{  //lexemType = lexVar
    bool isThereNewDefinition = true;
    while (true) //Handle every line with definate while we meet block of programm or tools block
    {
        id.toZero();
        identifiers.clear();
        if (lexemType == lexInt || lexemType == lexReal)
        {
            char type[5];
            strcpy(type, "int");
            if (lexemType == lexReal)
                strcpy(type, "real");
            strcpy(id.valueType, type);
            if (!handleIDs())   //The function handle line with definites
            {
                //printf("\nError: %d: Excepted %s", lineWithError, whatIsExpected);
                return false;
            }
            std::vector<Identifier>::iterator it = identifiers.begin();
            if (identifiers.size() == 1)
            {
                TiXmlElement* dfn = new TiXmlElement("dfn");
                dfn->SetAttribute("name", (*it).nameID);
                if ((*it).length > 0)
                    dfn->SetAttribute("length", (*it).length);
                dfn->SetAttribute("type", type);
                node->LinkEndChild(dfn);
            }
            else if (identifiers.size() > 1)
            {
                TiXmlElement* dfn = new TiXmlElement("dfn");
                dfn->SetAttribute("type", type);
                node->LinkEndChild(dfn);
                for (it = identifiers.begin(); it != identifiers.end(); it++)
                {
                    TiXmlElement* brief = new TiXmlElement("brief");
                    brief->SetAttribute("name", (*it).nameID);
                    if ((*it).length > 0)
                        brief->SetAttribute("length", (*it).length);
                    dfn->LinkEndChild(brief);
                }
                TiXmlText* text = new TiXmlText("");
                dfn->LinkEndChild(text);
            }
        }
        else if (!isThereNewDefinition)
        {
            strcpy(whatIsExpected, "new definition");
            return false;
        }
        if (lexemType == lexTools || lexemType == lexStart || lexemType == lexEOF)
            break;
        if (lexemType == lexInt || lexemType == lexReal)
        {
            strcpy(whatIsExpected, ";");
            return false;
        }
        if (lexemType == lexSemicolon)
        {
            isThereNewDefinition = false;
            parseLine();
            if (lexemType == lexInt || lexemType == lexReal)
                isThereNewDefinition = true;
            continue;
        }
        parseLine(); // Except
    }
    return true;
}
bool Parser::handleIDs() //lexemType = lexInt
{
    do /* Считыаем переменную за переменной, занося информацию о ней в список, до тех пор,
    пока не наткнемся на блок описание процедур, блок программы или блок объявления следующих переменных */
    {
        int codeLexemAfterID = readId(); //Считали идентификатор и вернули код, соответствующий лексеме, следующей за идентификатором
        identifiers.push_back(id);
        id.length = 0;
        switch(codeLexemAfterID)
        {
        case 0: return true;
        case 1: break;
        case 2: break;
        case 3: return true; //a[2];
        case 4: return true; //a;
        case 5: return true; //a[2] tools | a[2] start
        case 6: return true; //a tools | a start
        case 42: return false;
        default:
        {
            strcpy(whatIsExpected, "\"Tools\" or \"Start\"\0");
            return false;
        }
        }
    }
    while (lexemType != lexTools && lexemType != lexStart);
}
 int Parser::readId()
{
    lineWithError = line;
    parseLine(); // lexId
    if (lexemType != lexId)
    {
        strcpy(whatIsExpected, "Identifier");
        return 42;
    }
    strcpy(id.nameID, lexem);
    parseLine(); //Excepted lexComma, lexSemicolon, lexTools, lexLSB, lexStart
    if (lexemType == lexInt || lexemType == lexReal)
    {
        strcpy(whatIsExpected, ";");
        return 42;
    }
    lineWithError = line;
    if (lexemType == lexComma)
        return 2;   //считан просто идентификатор. После него запятая. a,
    else if (lexemType == lexSemicolon)
        return 4;   //считан просто идентификатор. После него точка с запятой. a;
    else if (lexemType == lexTools || lexemType == lexStart || lexemType == lexInt || lexemType == lexReal)
        return 6;   //считан просто идентификатор. После него tools, start, int или real. a tools | a start | a int | a real
    else if (lexemType == lexEOF)
        return 0;
    else if (lexemType == lexLSB)
    {
        parseLine();    //lexInt
        lineWithError = line;
        if (lexemType != lexInt)
        {
            strcpy(whatIsExpected, "int number as an element of array\0");
            return 42;
        }
        else if (dValue <= 0 || dValue >= 100)
        {
            strcpy(whatIsExpected, "number more than 0 and less than 100");
            return 42;
        }
        id.length = dValue; //запись length в узел списка
        parseLine(); // считвли RSB
        if (lexemType != lexRSB)
        {
            strcpy(whatIsExpected, "right square braket\0");
            return 42;
        }
        lineWithError = line;
        parseLine(); //считали лексему после идентификатора
        lineWithError = line;
        if (lexemType == lexComma)
            return 1;   //Считан массив. После него запятая. a[2],
        else if (lexemType == lexSemicolon)
            return 3;    //Считан массив. После него точка с запятой. a[2];
        else if (lexemType == lexTools || lexemType == lexStart || lexemType == lexInt || lexemType == lexReal)
            return 5;   //Считан массив. После него tools, start, int или real. a[2] tools | a[2] start | a[2] int | a[2] real
        else
        {
            strcpy(whatIsExpected, "\",\" or \";\" or \"tools\" or \"[\" or \"start\"");
            return 42;
        }
    }
    else
    {
        strcpy(whatIsExpected, "\",\" or \";\" or \"tools\" or \"[\" or \"start\"");
        return 42;
    }
}
bool Parser::tools() //lexemType = lexTools
{
    do
    {
        TiXmlElement* proc = new TiXmlElement("proc");
        program->LinkEndChild(proc);
        parseLine(); //считали лексему proc
        lineWithError = line;
        if (lexemType != lexProc)
        {
            strcpy(whatIsExpected, "\"proc\"");
            return false;
        }
        lineWithError = line;
        parseLine(); //считали идентификатор
        if (lexemType != lexId)
        {
            strcpy(whatIsExpected, "\"identifier\"");
            return false;
        }
        proc->SetAttribute("name", lexem);
        parseLine();
        lineWithError = line;
        if (lexemType == lexStart)
        {
            TiXmlElement* compound = new TiXmlElement("compound");
            proc->LinkEndChild(compound);
            if (!composite(compound))
                return false;
            TiXmlText* text = new TiXmlText("");
            compound->LinkEndChild(text);
        }
        else if (lexemType == lexInt || lexemType == lexReal)
        {
            if (!definitions(proc))
                return false;
            TiXmlElement* compound = new TiXmlElement("compound");
            proc->LinkEndChild(compound);
            if (!composite(compound))
                return false;
            TiXmlText* text = new TiXmlText("");
            compound->LinkEndChild(text);
        }
        else
        {
            strcpy(whatIsExpected, "\"start\" or \"definitions\"");
            return false;
        }
        TiXmlText* text = new TiXmlText("");
        proc->LinkEndChild(text);
        parseLine();
        if (lexemType == lexSemicolon)
            continue;
        else
            break;
    } while (true);
    return true;
}
bool Parser::isEOF()
{
    return _isEOF;
}
void Identifier::toZero()
{
    this->length = 0;
    strcpy(this->nameID, "\0");
    strcpy(this->valueType, "\0");
}
bool Parser::composite(TiXmlNode* node) // lexStart
{//$ составной = "start" оператор { ";" оператор } "stop".
    parseLine();
    lineWithError = line;
    if (lexemType == lexLabel)
    {
        label(node);
        parseLine();
    }
    switch (lexemType)
    {
    case lexStop: {lexemType = lexComma; return true;}
    case lexSemicolon: break;
    case lexRead: { if (!opRead(node)) return false; break;}
    case lexWrite: { if (!opWrite(node)) return false;  break;}
    case lexBreak:
    {
        /*if (strcmp("while",node->Value()) != 0)
        {
            strcpy(whatIsExpected,"you cannot use break here, mischievous boy");
            return false;
        }*/
        if (!isAvaibleForBreak)
        {
            strcpy(whatIsExpected,"you cannot use break here, mischievous boy");
            return false;
        }
        opBreak(node);
        break;
    }
    case lexGoto: { if (!opGoto(node)) return false; break;}
    case lexIf: { if (!opIf(node)) return false; break; }
    case lexWhile:
    {
        isAvaibleForBreak = true;
        if (!opWhile(node))
            return false;
        isAvaibleForBreak = false;
        break;
    }
    case lexLRB: {if (!assignOrCast(node)) return false; break; }
    case lexId: { if (!opCall(node)) return false; break; }
    case lexStart:
    {
        TiXmlElement* compound = new TiXmlElement("compound");
        node->LinkEndChild(compound);
        if (!composite(compound))
            return false;
        TiXmlText* text = new TiXmlText("");
        compound->LinkEndChild(text);
        break;
    }
    default:
    {
        strcpy(whatIsExpected,"some operator");
        return false;
    }
    }
    if (lexemType == lexComma)
        parseLine();
    if (lexemType == lexStop)
        return true;
    else if (lexemType != lexSemicolon)
    {
        strcpy(whatIsExpected,"\";\" or \"stop\"");
        return false;
    }

    while (lexemType == lexSemicolon)
    {
        parseLine();
        lineWithError = line;
        if (lexemType == lexLabel)
        {
            label(node);
            parseLine();
        }
        switch (lexemType)
        {
        case lexStop: {lexemType = lexComma; break;}
        case lexSemicolon: continue;
        case lexRead: { if (!opRead(node)) return false; break;}
        case lexWrite: { if (!opWrite(node)) return false;  break;}
        case lexBreak:
        {
            /*if (strcmp("while",node->Value()) != 0)
            {
                strcpy(whatIsExpected,"you cannot use break here, mischievous boy");
                return false;
            }*/
            if (!isAvaibleForBreak)
            {
                strcpy(whatIsExpected,"you cannot use break here, mischievous boy");
                return false;
            }
            opBreak(node);
            break;
        }
        case lexGoto: { if (!opGoto(node)) return false; break;}
        case lexIf: { if (!opIf(node)) return false; break; }
        case lexWhile:
        {
            isAvaibleForBreak = true;
            if (!opWhile(node))
                return false;
            isAvaibleForBreak = false;
            break;
        }
        case lexLRB: {if (!assignOrCast(node)) return false; break; }
        case lexId: { if (!opCall(node)) return false; break; }
        case lexStart:
        {
            TiXmlElement* compound = new TiXmlElement("compound");
            node->LinkEndChild(compound);
            if (!composite(compound))
                return false;
            TiXmlText* text = new TiXmlText("");
            compound->LinkEndChild(text);
            break;
        }
        default:
        {
            strcpy(whatIsExpected,"some operator or \";\"");
            return false;
        }
        }
    }
    return true;
}
bool Parser::opRead(TiXmlNode* node) //$ ввода = read переменная { "," переменная }.
{   //lexemType = lexRead
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);
    TiXmlElement* read = new TiXmlElement("read");
    clause->LinkEndChild(read);
    do
    {
        TiXmlElement* var = new TiXmlElement("var");
        TiXmlText* text = new TiXmlText("");
        lineWithError = line;
        parseLine(); //Id
        if (lexemType != lexId)
        {
            strcpy(whatIsExpected, "identifier");
            return false;
        }
        var->SetAttribute("name", lexem);
        parseLine(); //LSB or Comma
        if (lexemType == lexId)
        {
            strcpy(whatIsExpected, "\",\"");
            return false;
        }
        if (lexemType == lexLSB)
        {
            parseLine(); // index
            if (lexemType == lexInt)
            {
                if (dValue <= 0 || dValue >= 100)
                {
                    strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                    return false;
                }
                var->SetAttribute("index", dValue);
            }
            else if (lexemType == lexId)
                var->SetAttribute("index", lexem);
            else
            {
                strcpy(whatIsExpected, "index");
                return false;
            }
            parseLine();//lexRSB
            if (lexemType != lexRSB)
            {
                strcpy(whatIsExpected, "\"]\"");
                return false;
            }
            parseLine();
            if (lexemType == lexId)
            {
                strcpy(whatIsExpected, "\",\"");
                return false;
            }
        }
        read->LinkEndChild(var);
        var->LinkEndChild(text);
        if (lexemType == lexComma)
            continue;
        else
            break;
    } while (true);
    TiXmlText* textRead = new TiXmlText("");
    read->LinkEndChild(textRead);

    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
    return true;
}
bool Parser::opWrite(TiXmlNode* node) //$ вывода = write ( выражение | спецификатор ) { "," ( выражение | спецификатор ) }.
{   //lexemType = lexWrite
    bool accessesToRead = true;
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);
    TiXmlElement* write = new TiXmlElement("write");
    clause->LinkEndChild(write);
    do
    {
        parseLine();
        if (lexemType == lexSkip || lexemType == lexTab || lexemType == lexSpace)
        {
            TiXmlElement* qualifier = new TiXmlElement("qualifier");
            qualifier->SetAttribute("kind", lexem);
            write->LinkEndChild(qualifier);
        }
        else if (lexemType == lexLRB)
        {
            if (!expression(write))
                return false;
        }
        else if (lexemType == lexId)
        {
            TiXmlElement* var = new TiXmlElement("var");
            var->SetAttribute("name", lexem);
            write->LinkEndChild(var);
            parseLine();
            if (lexemType == lexLSB)
            {
                parseLine(); // index
                if (lexemType == lexInt)
                {
                    if (dValue <= 0 || dValue >= 100)
                    {
                        strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                        return false;
                    }
                    var->SetAttribute("index", dValue);
                }
                else if (lexemType == lexId)
                    var->SetAttribute("index", lexem);
                else
                {
                    strcpy(whatIsExpected, "index");
                    return false;
                }
                parseLine();//lexRSB
                if (lexemType != lexRSB)
                {
                    strcpy(whatIsExpected, "\"]\"");
                    return false;
                }
                parseLine();
                if (lexemType == lexId)
                {
                    strcpy(whatIsExpected, "\",\"");
                    return false;
                }
                accessesToRead = false;
            }
            else
                accessesToRead = false;
        }
        else if (lexemType == lexInt)
        {
            TiXmlElement* _int = new TiXmlElement("int");
            _int->SetAttribute("val", dValue);
            write->LinkEndChild(_int);
        }
        else if (lexemType == lexReal)
        {
            TiXmlElement* _real = new TiXmlElement("real");
            _real->SetDoubleAttribute("val", fValue);
            write->LinkEndChild(_real);
        }
        else
        {
            strcpy(whatIsExpected, "qualifier or expression");
            return false;
        }
        if (accessesToRead)
            parseLine();
        accessesToRead = true;
        if (lexemType != lexComma)
            break;
    } while (true);
    TiXmlText* textWrite = new TiXmlText("");
    write->LinkEndChild(textWrite);
    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
    return true;
}
bool Parser::opCall(TiXmlNode* node) //$ вызов = идентификатор "(" [ переменная { "," переменная } ] ")".
{ // lexemType = lexId
    bool wasComma = false;
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);
    TiXmlElement* call = new TiXmlElement("call");
    clause->LinkEndChild(call);
    call->SetAttribute("name", lexem);
    parseLine(); // считали lexLRB
    if (lexemType != lexLRB)
    {
        strcpy(whatIsExpected, "\"(\"");
        return false;
    }
    do
    {
        TiXmlElement* var = new TiXmlElement("var");
        TiXmlText* text = new TiXmlText("");
        parseLine(); //Id or RRB
        if (wasComma && lexemType != lexId)
        {
            strcpy(whatIsExpected, "identifier");
            return false;
        }
        if (lexemType == lexRRB)
            break;
        else if (lexemType != lexId)
        {
            strcpy(whatIsExpected, "idenifier");
            return false;
        }
        var->SetAttribute("name", lexem);
        parseLine(); //LSB or lexComma
        if (lexemType == lexLSB)
        {
            parseLine(); // index
            if (lexemType == lexInt)
            {
                if (dValue <= 0 || dValue >= 100)
                {
                    strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                    return false;
                }
                var->SetAttribute("index", dValue);
            }
            else if (lexemType == lexId)
                var->SetAttribute("index", lexem);
            else
            {
                strcpy(whatIsExpected, "index");
                return false;
            }
            parseLine();//lexRSB
            if (lexemType != lexRSB)
            {
                strcpy(whatIsExpected, "\"]\"");
                return false;
            }
            parseLine();
        }
        call->LinkEndChild(var);
        var->LinkEndChild(text);
        wasComma = false;
        if (lexemType == lexComma)
        {
            wasComma = true;
            continue;
        }
        else if (lexemType != lexRRB)
        {
            strcpy(whatIsExpected, "\")\"");
            return false;
        }
        else
            break;
    } while (true);
    TiXmlText* textCall = new TiXmlText("");
    call->LinkEndChild(textCall);
    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
    parseLine();
    return true;
}
void Parser::opBreak(TiXmlNode* node)
{
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);
    TiXmlElement* opBreak = new TiXmlElement("break");
    clause->LinkEndChild(opBreak);
    parseLine();
    TiXmlText* textBreak = new TiXmlText("");
    opBreak->LinkEndChild(textBreak);
    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
}
bool Parser::expression(TiXmlNode* node) //$ выражение = "(" операнд операнд операция ")" | "(" операнд "minus" ")" | операнд .

{   //в lexemType lexLRB
    static bool accessesToRead = true;
    TiXmlElement* expr = new TiXmlElement("expr");
    parseLine(); //lexLRB or lexInt or lexReal or lexId
    TiXmlElement* op = new TiXmlElement("op");
    switch (lexemType)
    {
        case lexLRB: if (!expression(op)) return false; break;
        case lexId:
        {
            TiXmlElement* var = new TiXmlElement("var");
            var->SetAttribute("name", lexem);
            op->LinkEndChild(var);
            parseLine();
            if (lexemType == lexLSB)
            {
                parseLine(); // index
                if (lexemType == lexInt)
                {
                    if (dValue <= 0 || dValue >= 100)
                    {
                        strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                        return false;
                    }
                    var->SetAttribute("index", dValue);
                }
                else if (lexemType == lexId)
                    var->SetAttribute("index", lexem);
                else
                {
                    strcpy(whatIsExpected, "index");
                    return false;
                }
                parseLine();//lexRSB
                if (lexemType != lexRSB)
                {
                    strcpy(whatIsExpected, "\"]\"");
                    return false;
                }
            }
            else
                accessesToRead = false;
            break;
        }
        case lexInt:
        {
            TiXmlElement* _int = new TiXmlElement("int");
            _int->SetAttribute("val", dValue);
            op->LinkEndChild(_int);
            break;
        }
        case lexReal:
        {
            TiXmlElement* _real = new TiXmlElement("real");
            _real->SetDoubleAttribute("val", fValue);
            op->LinkEndChild(_real);
            break;
        }
    default:
    {
        strcpy(whatIsExpected, "nyan expression ^_^");
        return false;
    }
    }

    if (accessesToRead)
        parseLine();
    accessesToRead = true;

    switch (lexemType)
    {
        case lexLRB: if (!expression(op)) return false; break;
        case lexId:
        {
            TiXmlElement* var = new TiXmlElement("var");
            var->SetAttribute("name", lexem);
            op->LinkEndChild(var);
            parseLine();
            if (lexemType == lexLSB)
            {
                parseLine(); // index
                if (lexemType == lexInt)
                {
                    if (dValue <= 0 || dValue >= 100)
                    {
                        strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                        return false;
                    }
                    var->SetAttribute("index", dValue);
                }
                else if (lexemType == lexId)
                    var->SetAttribute("index", lexem);
                else
                {
                    strcpy(whatIsExpected, "index");
                    return false;
                }
                parseLine();//lexRSB
                if (lexemType != lexRSB)
                {
                    strcpy(whatIsExpected, "\"]\"");
                    return false;
                }
            }
            else
                accessesToRead = false;
            break;
        }
        case lexInt:
        {
            TiXmlElement* _int = new TiXmlElement("int");
            _int->SetAttribute("val", dValue);
            op->LinkEndChild(_int);
            break;
        }
        case lexReal:
        {
            TiXmlElement* _real = new TiXmlElement("real");
            _real->SetDoubleAttribute("val", fValue);
            op->LinkEndChild(_real);
            break;
        }
        case lexMinus:
        {
            op->SetAttribute("kind", lexem);
            expr->LinkEndChild(op);
            node->LinkEndChild(expr);
            parseLine();
            return true;
        }
        default:
        {
            strcpy(whatIsExpected, "nyan expression ^_^");
            return false;
        }
    }
    if (accessesToRead)
        parseLine();
    accessesToRead = true;
    if (lexemType == lexEQ || lexemType == lexGE || lexemType == lexGT || lexemType == lexLE
            || lexemType == lexLT || lexemType == lexPlus || lexemType == lexMinus
            || lexemType == lexDiv || lexemType == lexMod || lexemType == lexMult
            || lexemType == lexNE)
    {
        op->SetAttribute("kind", lexem);
        expr->LinkEndChild(op);
        node->LinkEndChild(expr);
    }
    else
        {
            strcpy(whatIsExpected, "nyan expression ^_^");
            parseLine();
            return false;
        }
    parseLine();
    return true;
}
bool Parser::opGoto(TiXmlNode* node) //goto имя_метки.
{   //lexemType = lexGoto
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);
    TiXmlElement* opGoto = new TiXmlElement("goto");
    clause->LinkEndChild(opGoto);
    parseLine(); //lexInt
    if (lexemType != lexInt)
    {
        strcpy(whatIsExpected, "label name");
        return false;
    }
    opGoto->SetAttribute("name", lexem);
    parseLine(); //lexSemicolon or lexStop
    TiXmlText* textGoto = new TiXmlText("");
    opGoto->LinkEndChild(textGoto);
    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
    return true;
}
bool Parser::opIf(TiXmlNode* node)    // lexemType = lexIf
{ //$ условный = if выражение then непомеченный [ else непомеченный ].
    bool accessesToRead = true;
    TiXmlElement* clause = new TiXmlElement("clause");
    TiXmlElement* _if = new TiXmlElement("if");
    clause->LinkEndChild(_if);

    parseLine(); //lexLRB or lexInt, lexReal, lexId
    if (lexemType == lexLRB)
    {
        if (!expression(_if))
            return false;
    }
    else if (lexemType == lexInt)
    {
        TiXmlElement* _int = new TiXmlElement("int");
        _if->LinkEndChild(_int);
        _int->SetAttribute("val", dValue);
    }
    else if (lexemType == lexReal)
    {
        TiXmlElement* _real = new TiXmlElement("real");
        _if->LinkEndChild(_real);
        _real->SetDoubleAttribute("val", fValue);
    }
    else if (lexemType == lexId)
    {
        TiXmlElement* var = new TiXmlElement("var");
        _if->LinkEndChild(var);
        var->SetAttribute("name", lexem);
        parseLine();
        if (lexemType == lexLSB)
        {
            parseLine(); // index
            if (lexemType == lexInt)
            {
                if (dValue <= 0 || dValue >= 100)
                {
                    strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                    return false;
                }
                var->SetAttribute("index", dValue);
            }
            else if (lexemType == lexId)
                var->SetAttribute("index", lexem);
            else
            {
                strcpy(whatIsExpected, "index");
                return false;
            }
            parseLine();//lexRSB
            if (lexemType != lexRSB)
            {
                strcpy(whatIsExpected, "\"]\"");
                return false;
            }
        }
        else
            accessesToRead = false;
    }
    else
    {
        strcpy(whatIsExpected, "expression");
        return false;
    }

    if (accessesToRead)
        parseLine(); //lexThen or lexElse
    if (lexemType != lexThen && lexemType != lexElse)
    {
        strcpy(whatIsExpected, "then or else");
        return false;
    }
    if (lexemType == lexThen)
    {
        TiXmlElement* _then = new TiXmlElement("then");
        _if->LinkEndChild(_then);
        parseLine();
        if (!unmarked(_then))
        {
            if (lexemType != lexElse && lexemType != lexStop)
                return false;
        }
        /*if (lexemType == lexSemicolon)
            parseLine();*/
    }
    if (lexemType == lexElse)
    {
        TiXmlElement* _else = new TiXmlElement("else");
        _if->LinkEndChild(_else);
        parseLine();
        if (!unmarked(_else))
        {
            if (lexemType == lexElse)
            {
                node->LinkEndChild(clause);
                return true;
            }
            if (lexemType != lexStop)
                return false;
        }
    }
    node->LinkEndChild(clause);
    return true;
}
bool Parser::unmarked(TiXmlNode* node)
{
    switch (lexemType)
    {
    case lexSemicolon: return true;
    case lexRead: if (!opRead(node)) return false; break; // быстро
    case lexWrite: if (!opWrite(node)) return false; break; // быстро
    case lexBreak:
    {
        if (!isAvaibleForBreak)
        {
            strcpy(whatIsExpected,"you cannot use break here, mischievous boy");
            return false;
        }
        opBreak(node);
        break;
    }

    case lexGoto: if (!opGoto(node)) return false; break;
    case lexIf: if (!opIf(node)) return false; break;
    case lexWhile:
    {
        isAvaibleForBreak = true;
        if (!opWhile(node))
        {
            isAvaibleForBreak = false;
            return false;
        }
        break;
    }
    case lexLRB: assignOrCast(node); break;
    case lexId: opCall(node); break;
    case lexStart:
    {
        TiXmlElement* compound = new TiXmlElement("compound");
        node->LinkEndChild(compound);
        if (!composite(compound))
            return false;
        TiXmlText* text = new TiXmlText("");
        compound->LinkEndChild(text);
        break;
    }
    default:
    {
        strcpy(whatIsExpected,"some operator");
        return false;
    }
    }
    return true;
}
bool Parser::opWhile(TiXmlNode* node) //$ цикла = while выражение do оператор { ";" оператор } end.
{                // lexemType = lexWhile
    bool accessesToRead = true;
    TiXmlElement* clause = new TiXmlElement("clause");
    TiXmlElement* _while = new TiXmlElement("while");
    clause->LinkEndChild(_while);
    parseLine(); //lexLRB or lexInt, lexReal, lexId
    if (lexemType == lexLRB)
    {
        if (!expression(_while))
        {
            strcpy(whatIsExpected, "expression");
            return false;
        }
    }
    else if (lexemType == lexId)
    {
        TiXmlElement* var = new TiXmlElement("var");
        var->SetAttribute("name", lexem);
        _while->LinkEndChild(var);
        parseLine();
        if (lexemType == lexLSB)
        {
            parseLine(); // index
            if (lexemType == lexInt)
            {
                if (dValue <= 0 || dValue >= 100)
                {
                    strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                    return false;
                }
                var->SetAttribute("index", dValue);
            }
            else if (lexemType == lexId)
                var->SetAttribute("index", lexem);
            else
            {
                strcpy(whatIsExpected, "index");
                return false;
            }
            parseLine();//lexRSB
            if (lexemType != lexRSB)
            {
                strcpy(whatIsExpected, "\"]\"");
                return false;
            }
        }
        else
            accessesToRead = false;
    }
    else if (lexemType == lexInt)
    {
        TiXmlElement* _int = new TiXmlElement("int");
        _int->SetAttribute("val", dValue);
        _while->LinkEndChild(_int);
    }
    else if (lexemType == lexReal)
    {
        TiXmlElement* _real = new TiXmlElement("real");
        _real->SetDoubleAttribute("val", fValue);
        _while->LinkEndChild(_real);
    }
    else
    {
        strcpy(whatIsExpected, "expression or some value");
        return false;
    }
    if (accessesToRead)
        parseLine(); //lexDo
    accessesToRead = true;
    lineWithError = line;
    bool exceptNewOperator = true;
    do
    {
        if (!exceptNewOperator)
        {
            strcpy(whatIsExpected, "\";\"");
            return false;
        }
        parseLine(); //lexLabel либо оператор..
        lineWithError = line;
        if (lexemType == lexLabel)
        {
            label(_while);
            continue;
        }
        else if (lexemType == lexEnd)
        {
            break;
        }
        else
        {
            if (!unmarked(_while))
            {
                strcpy(whatIsExpected, "some operator");
                return false;
            }
            else //if (lexemType == lexStop)
                parseLine();
        }
        lineWithError = line;
        exceptNewOperator = false;
        if (lexemType == lexSemicolon)
        {
            exceptNewOperator = true;
            continue;
        }
        else if (lexemType == lexEnd)
            break;
        else
        {
            strcpy(whatIsExpected, "\";\" or end");
            return false;
        }
    } while (true);
    node->LinkEndChild(clause);
    parseLine();
    return true;
}
bool Parser::assignOrCast(TiXmlNode* node)
{   //в lexemType lexLRB
    bool isCast = true, isAssign = true, accessesToRead = true;
    TiXmlElement* clause = new TiXmlElement("clause");
    TiXmlElement* assign = new TiXmlElement("assign");
    TiXmlElement* cast = new TiXmlElement("cast");
    parseLine();
    switch (lexemType)
    {
        case lexLRB:
        {
            if (!expression(assign))
                return false;
            isCast = false;
            break;
        }
        case lexId:
        {
            TiXmlElement* var = new TiXmlElement("var");
            var->SetAttribute("name", lexem);
            assign->LinkEndChild(var);
            parseLine();
            if (lexemType == lexLSB)
            {
                parseLine(); // index
                if (lexemType == lexInt)
                {
                    if (dValue <= 0 || dValue >= 100)
                    {
                        strcpy(whatIsExpected, "number more than 0 and less than 100 as the index of array");
                        return false;
                    }
                    var->SetAttribute("index", dValue);
                }
                else if (lexemType == lexId)
                    var->SetAttribute("index", lexem);
                else
                {
                    strcpy(whatIsExpected, "index");
                    return false;
                }
                parseLine();//lexRSB
                if (lexemType != lexRSB)
                {
                    strcpy(whatIsExpected, "\"]\"");
                    return false;
                }
                parseLine();
                /*if (lexemType == lexId)
                {
                    strcpy(whatIsExpected, "\",\"");
                    return false;
                }*/
                accessesToRead = false;
            }
            else
                accessesToRead = false;
            cast->LinkEndChild(var->Clone());
            break;
        }
        case lexInt:
        {
            TiXmlElement* _int = new TiXmlElement("int");
            _int->SetAttribute("val", dValue);
            assign->LinkEndChild(_int);
            isCast = false;
            break;
        }
        case lexReal:
        {
            TiXmlElement* _real = new TiXmlElement("real");
            _real->SetDoubleAttribute("val", fValue);
            assign->LinkEndChild(_real);
            isCast = false;
            break;
        }
        default:
        {
            strcpy(whatIsExpected,"expression or some value");
            return false;
        }
    }
    if (accessesToRead)
        parseLine(); // lexId
    accessesToRead = true;
    if (lexemType == lexId)
    {
        TiXmlElement* var = new TiXmlElement("var");
        var->SetAttribute("name", lexem);
        assign->LinkEndChild(var);
        parseLine();
        if (lexemType == lexLSB)
        {
            parseLine();
            var->SetAttribute("index", dValue);
            parseLine();
        }
        else
            accessesToRead = false;
        cast->LinkEndChild(var->Clone());
    }
    else
    {
        isCast = false;
        isAssign = false;
         strcpy(whatIsExpected,"expression or some value");
         return false;
    }
    if (accessesToRead)
        parseLine(); // lexCast or lexLet
    if (lexemType == lexLet && isAssign)
        clause->LinkEndChild(assign);
    else if (lexemType == lexCast && isCast)
        clause->LinkEndChild(cast);
    node->LinkEndChild(clause);
    parseLine(); //lexRRB
    if (lexemType != lexRRB)
    {
        strcpy(whatIsExpected,"\")\"");
        return false;
    }
    parseLine(); //lexSemicolon or smth else
    return true;
}
void Parser::label(TiXmlNode *node)
{
    TiXmlElement* label = new TiXmlElement("label");
    node->LinkEndChild(label);
    label->SetAttribute("name", lexem);
}
