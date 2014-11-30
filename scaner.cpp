#include "scaner.h"
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
            if (!wasError)
                printf("OK");
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
        if (!wasError)
            printf("OK");
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
    if (lexemType == lexVar)
    {
        definitions(program);
        if (lexemType == lexTools)
            tools();
    }
    else if (lexemType == lexTools)
        tools();
    programm();

    TiXmlText* text = new TiXmlText("");
    program->LinkEndChild(text);
    doc.SaveFile("madeByHand2.xml");
}
void Parser::definitions(TiXmlNode* node) //lexemType = lexVar
{
    printf("\nБлок объявления переменных:");
    while (true) //Обрабатываем каждую строку с объявлением переменных до тех пор, пока не встретим блок программы, либо блок описания процедур
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
            handleIDs();   //функция, обрабатывающая строку переменных(ой)
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
        if (lexemType == lexTools || lexemType == lexStart || lexemType == lexEOF)
            break;
        parseLine();
    }
}
void Parser::handleIDs() //lexemType = lexInt
{
    do /* Считыаем переменную за переменной, занося информацию о ней в список, до тех пор,
    пока не наткнемся на блок описание процедур, блок программы или блок объявления следующих переменных */
    {
        int codeLexemAfterID = readId(); //Считали идентификатор и вернули код, соответствующий лексеме, следующей за идентификатором
        //TO DO: запись переменной в узел
        identifiers.push_back(id);
        id.length = 0;
        switch(codeLexemAfterID)
        {
        case 0: return;
        case 1: break;
        case 2: break;
        case 3: /*Вывод в XML printIDsToXMLTree() */ return;    //a[2];
        case 4: /*Вывод в XML printIDsToXMLTree() */ return;    //a;
        case 5: /*Вывод в XML printIDsToXMLTree() */ return;    //a[2] tools | a[2] start
        case 6: /*Вывод в XML printIDsToXMLTree() */ return;    //a tools | a start
        }
    }
    while (lexemType != lexTools && lexemType != lexStart);
}
int Parser::readId()
{
    parseLine(); // Считылм идентификатор
    strcpy(id.nameID, lexem); //запись nameID в узел списка
    parseLine();
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
        parseLine();    // считали length
        id.length = dValue; //запись length в узел списка
        parseLine(); // считвли RSB
        parseLine(); //считали дексему после идентификатора
        if (lexemType == lexComma)
            return 1;   //Считан массив. После него запятая. a[2],
        else if (lexemType == lexSemicolon)
            return 3;    //Считан массив. После него точка с запятой. a[2];
        else if (lexemType == lexTools || lexemType == lexStart || lexemType == lexInt || lexemType == lexReal)
            return 5;   //Считан массив. После него tools, start, int или real. a[2] tools | a[2] start | a[2] int | a[2] real
    }
}
void Parser::tools() //lexemType = lexTools
{
    printf("\nБлок процедур:");
    int i = 1;
    do
    {
        TiXmlElement* proc = new TiXmlElement("proc");
        program->LinkEndChild(proc);
        printf("\nПроцедура %d:", i++);
        parseLine(); //считали лексему proc
        parseLine(); //считали идентификатор
        proc->SetAttribute("name", lexem);
        parseLine();
        if (lexemType == lexStart)
        {
            printf("\nТело процедуры:");
            TiXmlElement* compound = new TiXmlElement("compound");
            proc->LinkEndChild(compound);
            composite(compound);
            TiXmlText* text = new TiXmlText("");
            compound->LinkEndChild(text);
        }
        else if (lexemType == lexInt || lexemType == lexReal)
        {
            definitions(proc);
            printf("\nТело процедуры:");
            TiXmlElement* compound = new TiXmlElement("compound");
            proc->LinkEndChild(compound);
            composite(compound);
            TiXmlText* text = new TiXmlText("");
            compound->LinkEndChild(text);
        }
        parseLine();
        if (lexemType != lexSemicolon)
            break;
        TiXmlText* text = new TiXmlText("");
        proc->LinkEndChild(text);
    } while (true);
}
void Parser::programm()
{
    printf("\nБлок программы:");
    composite(program);
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

void Parser::composite(TiXmlNode* node) // на входе start
{                        //$ составной = "start" оператор { ";" оператор } "stop".
    do
    {
        if (lexemType == lexStop)
        {
            lexemType = lexComma;
            return;
        }
        parseLine();
        if (_isEOF)
            return;
        switch (lexemType)
        {
            case lexStop: { lexemType = lexComma; return;}
            case lexSemicolon: { break;}
            case lexRead: { opRead(node); break;}
            case lexWrite: { opWrite(node); break;}
            case lexBreak: { opBreak(node); break;}
            case lexGoto: { opGoto(node); break;}
            case lexIf: { opIf(node); break; }
            case lexWhile: { opWhile(node); break; }
            case lexLRB: { expression(1); break; }
            case lexId: { opCall(node); break; }
            case lexStart:
            {
                TiXmlElement* compound = new TiXmlElement("compound");
                node->LinkEndChild(compound);
                composite(compound);
                TiXmlText* text = new TiXmlText("");
                compound->LinkEndChild(text);
                break;
            }
            default: { break; }
        }
    } while (true);
}

void Parser::opRead(TiXmlNode* node) //$ ввода = read переменная { "," переменная }.
{   //lexemType сейчас равен lexRead
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);

    TiXmlElement* read = new TiXmlElement("read");
    clause->LinkEndChild(read);
    do
    {
        TiXmlElement* var = new TiXmlElement("var");
        TiXmlText* text = new TiXmlText("");
        parseLine(); //Id
        var->SetAttribute("name", lexem);
        parseLine(); //LSB or Comma
        if (lexemType == lexLSB)
        {
            parseLine(); // index
            if (lexemType == lexInt)
                var->SetAttribute("index", dValue);
            else if (lexemType == lexId)
                var->SetAttribute("index", lexem);
            parseLine();
            parseLine();
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
}

void Parser::opWrite(TiXmlNode* node) //$ вывода = write ( выражение | спецификатор ) { "," ( выражение | спецификатор ) }.
{   //lexemType = lexWrite
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
        else
        {
            //expression(1);
        }
        parseLine();
        if (lexemType != lexComma)
        {

            break;
        }
    } while (true);
    TiXmlText* textWrite = new TiXmlText("");
    write->LinkEndChild(textWrite);

    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
}

void Parser::opCall(TiXmlNode* node) //$ вызов = идентификатор "(" [ переменная { "," переменная } ] ")".
{ // lexemType = lexId
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);

    TiXmlElement* call = new TiXmlElement("call");
    clause->LinkEndChild(call);
    call->SetAttribute("name", lexem);
    parseLine(); // считали lexLRB

    do
    {
        TiXmlElement* var = new TiXmlElement("var");
        TiXmlText* text = new TiXmlText("");
        parseLine(); //Id or RRB
        if (lexemType == lexRRB)
            break;
        //else lexemType is lexId
        var->SetAttribute("name", lexem);
        parseLine(); //LSB or Comma
        if (lexemType == lexLSB)
        {
            parseLine(); // index
            if (lexemType == lexInt)
                var->SetAttribute("index", dValue);
            else if (lexemType == lexId)
                var->SetAttribute("index", lexem);
            parseLine();
            parseLine();
        }
        call->LinkEndChild(var);
        var->LinkEndChild(text);
        if (lexemType == lexComma)
            continue;
        else
            break;
    } while (true);

    TiXmlText* textCall = new TiXmlText("");
    call->LinkEndChild(textCall);

    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
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

void Parser::opLet()
{

}
void Parser::opCast()
{

}
void Parser::opGoto(TiXmlNode* node) //goto имя_метки.
{   //в lexemType lexGoto
    TiXmlElement* clause = new TiXmlElement("clause");
    node->LinkEndChild(clause);
    TiXmlElement* opGoto = new TiXmlElement("goto");
    clause->LinkEndChild(opGoto);
    parseLine(); //lexInt
    opGoto->SetAttribute("name", lexem);
    parseLine(); //lexSemicolon or lexStop
    TiXmlText* textGoto = new TiXmlText("");
    opGoto->LinkEndChild(textGoto);
    TiXmlText* textClause = new TiXmlText("");
    clause->LinkEndChild(textClause);
}

void Parser::expression(int level) //$ выражение = "(" операнд операнд операция ")" | "(" операнд "minus" ")" | операнд .
{   //в lexemType lexLRB
    //$ приведение = "(" переменная переменная cast ")".
    //$ присваивание = "(" выражение переменная "let" ")".
    //$ выражение = "(" операнд операнд операция ")" | "(" операнд "minus" ")" | операнд .
    static int isCast = 0;
    static int isLet = 0;
    parseLine();
    switch (lexemType)
    {
    case lexLRB: isLet++; expression(level + 1); break;
    case lexId: isCast++; isLet++; printf("\n%d - ID", level); break;
    case lexInt: printf("\n%d - Int", level); break;
    case lexReal: printf("\n%d - Real", level); break;
    }
    parseLine();
    switch (lexemType)
    {
    case lexLRB: expression(level + 1); break;
    case lexId: isCast++; isLet++; printf("\n%d - ID", level); break;
    case lexInt: printf("\n%d - Int", level); break;
    case lexReal: printf("\n%d - Real", level); break;
    case lexMinus: printf("\n%d - minus", level); parseLine(); return;
    }
    parseLine();
    switch (lexemType)
    {
    case lexEQ: printf("\n%d - eq", level); break;
    case lexGE: printf("\n%d - ge", level); break;
    case lexGT: printf("\n%d - gt", level); break;
    case lexLE: printf("\n%d - le", level); break;
    case lexLT: printf("\n%d - lt", level); break;
    case lexPlus: printf("\n%d - plus", level); break;
    case lexMinus: printf("\n%d - minus", level); break;
    case lexDiv: printf("\n%d - div", level); break;
    case lexMod: printf("\n%d - mod", level); break;
    case lexMult: printf("\n%d - mult", level); break;
    case lexCast: isCast; if (isCast == 3) opCast(); parseLine(); return;
    case lexLet: isLet++; if (isLet == 3) opLet(); parseLine(); return;
    }
    parseLine();//считали lexRRB
    return;
}

void Parser::opIf(TiXmlNode* node) //$ условный = if выражение then непомеченный [ else непомеченный ].
{                   // lexemType = lexIf
    parseLine(); //считали lexLRB либо lexInt, lexReal, lexId
    if (lexemType == lexLRB)
        expression(1);

    parseLine(); //считали lexThen либо другое
    if (lexemType == lexThen)
    {
        parseLine();
        unmarked(node);
    }
    parseLine();
    if (lexemType == lexElse)
    {
        parseLine();
        unmarked(node);
    }
}

void Parser::unmarked(TiXmlNode* node)
{
    switch (lexemType)
    {
    case lexSemicolon: break;
    case lexRead: opRead(node); break; // быстро
    case lexWrite: opWrite(node); break; // быстро
    case lexBreak: opBreak(node); break;
    case lexGoto: opGoto(node); break;
    case lexIf: opIf(node); break;
    case lexWhile: opWhile(node); break;
    case lexLRB: expression(1); break;
    case lexId: opCall(node); break;
    }
}

void Parser::opWhile(TiXmlNode* node) //$ цикла = while выражение do оператор { ";" оператор } end.
{                // lexemType = lexWhile
    parseLine(); //считали lexLRB либо lexInt, lexReal, lexId
    if (lexemType == lexLRB)
        expression(1);
    else if (lexemType == lexInt)
    {
    }
    else if (lexemType == lexReal)
    {
    }
    else if (lexemType == lexId)
    {
    }
    parseLine(); //считали lexDo
    do
    {
        parseLine(); //считали lexLabel либо оператор..
        if (lexemType == lexLabel)
        {

        }
        else
            unmarked(node);
        if (lexemType == lexEnd)
            break;
    } while (true);
}
