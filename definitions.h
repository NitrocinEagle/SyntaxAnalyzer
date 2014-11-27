extern const int LENGTH_KW = 20;
extern const int NUMB_KW = 33;
extern const int NUMB_LEXEM_TYPES = 41;
extern const int LENGTH_LEXEM_TYPE = 10;
extern const int LENGTH_LEXEM = 50;

extern const int ERROR_UNRECOGNIZABLE_LEXEM = 1;
extern const int ERROR_WRONG_BASE = 2;
extern const int ERROR_WRONG_INT_NUMBER = 3;
extern const int ERROR_WRONG_FORMAT_REAL_NUMBER = 4;
extern const int ERROR_UNFINISHED_COMMENT= 5;
extern const int ERROR_OVERFLOW_REAL_TYPE= 6;

const char KEY_WORDS_ID[NUMB_KW][LENGTH_KW] =
{
    "Var", "Tools", "Start", "Stop", "Skip", "Space", "Tab", "Break", "Goto", "Read", "Write", "If", "Then", "Else", "While", "Cast", "Int", "Real", "Mod", "Minus", "Mult", "Div", "Plus", "Let", "EQ", "NE", "LT", "GT", "LE", "GE", "Do", "End", "Proc"
};
const char KEY_WORDS[NUMB_KW][LENGTH_KW] =
{
    "var", "tools", "start", "stop", "skip", "space", "tab", "break", "goto", "read", "write", "if", "then", "else", "while", "cast", "int", "real", "mod", "minus", "mult", "div", "plus", "let", "eq", "ne", "lt", "gt", "le", "ge", "do", "end", "proc"
};
enum LexemType
{
    lexVar, lexTools, lexStart, lexStop, lexSkip, lexSpace, lexTab, lexBreak, lexGoto, lexRead, lexWrite, lexIf, lexThen, lexElse, lexWhile, lexCast, lexInt, lexReal, lexMod, lexMinus, lexMult, lexDiv, lexPlus, lexLet, lexEQ, lexNE, lexLT, lexGT, lexLE, lexGE, lexDo, lexEnd ,lexProc, lexLabel, lexSemicolon, lexComma, lexLRB, lexRRB, lexLSB, lexRSB, lexId
};

const char LEXEM_TYPES_ID[NUMB_LEXEM_TYPES][LENGTH_LEXEM_TYPE] =
{
    "Var", "Tools", "Start", "Stop", "Skip", "Space", "Tab", "Break", "Goto", "Read", "Write", "If", "Then", "Else", "While", "Cast", "Int", "Real", "Mod", "Minus", "Mult", "Div", "Plus", "Let", "EQ", "NE", "LT", "GT", "LE", "GE", "Do", "End", "Proc", "Label", "Semicolon", "Comma", "LRB", "RRB", "LSB", "RSB", "Id"
};
const LexemType LEXEM_TYPES[NUMB_LEXEM_TYPES] =
{
    lexVar, lexTools, lexStart, lexStop, lexSkip, lexSpace, lexTab, lexBreak, lexGoto, lexRead, lexWrite, lexIf, lexThen, lexElse, lexWhile, lexCast, lexInt, lexReal, lexMod, lexMinus, lexMult, lexDiv, lexPlus, lexLet, lexEQ, lexNE, lexLT, lexGT, lexLE, lexGE, lexDo, lexEnd ,lexProc, lexLabel, lexSemicolon, lexComma, lexLRB, lexRRB, lexLSB, lexRSB, lexId
};
