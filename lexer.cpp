// This file is part of the IMP project.

#include <sstream>

#include "lexer.h"



// -----------------------------------------------------------------------------
Token::Token(const Token &that)
  : loc_(that.loc_)
  , kind_(that.kind_)
{
  switch (kind_) {
    case Kind::STRING:
    case Kind::IDENT: {
      value_.StringValue = new std::string(*that.value_.StringValue);
      break;
    }
    //1.2. (b) Adjust the copy constructor (Token::Token(const Token &t)) and the assignment operator
    case Kind::INT: {
      //value_.IntValue = new std::uint64_t(that.value_.IntValue);
      value_.IntValue = that.value_.IntValue;
      break;
    }
    default: {
      break;
    }
  }
}

// -----------------------------------------------------------------------------
Token &Token::operator=(const Token &that)
{
  switch (kind_) {
    case Kind::STRING:
    case Kind::IDENT: {
      delete value_.StringValue;
      break;
    }
    default: {
      break;
    }
  }
  loc_ = that.loc_;
  kind_ = that.kind_;
  switch (kind_) {
    case Kind::STRING:
    case Kind::IDENT: {
      value_.StringValue = new std::string(*that.value_.StringValue);
      break;
    }
    //1.2. (b). (Token &Token::operator=(const Token &t)) to also copy the integer payload of the token. 
    case Kind::INT: {
      value_.IntValue = that.value_.IntValue;
      break;
    }
    default: {
      break;
    }
  }
  return *this;
}

// -----------------------------------------------------------------------------
Token::~Token()
{
  switch (kind_) {
    case Kind::STRING:
    case Kind::IDENT: {
      delete value_.StringValue;
      break;
    }
    default: {
      break;
    }
  }
}

// -----------------------------------------------------------------------------
Token Token::Ident(const Location &l, const std::string &str)
{
  Token tk(l, Kind::IDENT);
  tk.value_.StringValue = new std::string(str);
  return tk;
}

// -----------------------------------------------------------------------------
Token Token::String(const Location &l, const std::string &str)
{
  Token tk(l, Kind::STRING);
  tk.value_.StringValue = new std::string(str);
  return tk;
}

// -----------------------------------------------------------------------------
// 1.2. (a) Add a static factory method to the token class, constructing a token from a location an a uint64_t payload.
Token Token::Integer(const Location &l, const uint64_t &nr)
{
  Token tk(l, Kind::INT);
  tk.value_.IntValue = nr;
  return tk;
}

// -----------------------------------------------------------------------------
void Token::Print(std::ostream &os) const
{
  os << kind_;
  switch (kind_) {
    case Kind::INT: {
      os << "(" << value_.IntValue << ")";
      break;
    }
    case Kind::STRING: {
      os << "(\"" << *value_.StringValue << "\")";
      break;
    }
    case Kind::IDENT: {
      os << "(" << *value_.StringValue << ")";
      break;
    }
    default: {
      break;
    }
  }
}

// -----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const Token::Kind kind)
{
  switch (kind) {
    case Token::Kind::FUNC: return os << "func";
    case Token::Kind::RETURN: return os << "return";
    case Token::Kind::WHILE: return os << "while";
    
    //2.3.
    case Token::Kind::IF: return os << "if";
    case Token::Kind::ELSE: return os << "else";
    //3.1.
    case Token::Kind::LET: return os << "let";

    case Token::Kind::LPAREN: return os << "(";
    case Token::Kind::RPAREN: return os << ")";
    case Token::Kind::LBRACE: return os << "{";
    case Token::Kind::RBRACE: return os << "}";
    case Token::Kind::COLON: return os << ":";
    case Token::Kind::SEMI: return os << ";";
    case Token::Kind::EQUAL: return os << "=";

    //2.2.a Add the required tokens to the lexer (==, *, /, % and -)
    case Token::Kind::EQUALS: return os << "==";
    case Token::Kind::COMMA: return os << ",";
    case Token::Kind::PLUS: return os << "+";
    case Token::Kind::MINUS: return os << "-";
    case Token::Kind::MUL: return os << "*";
    case Token::Kind::DIV: return os << "/";
    case Token::Kind::MOD: return os << "%";

    case Token::Kind::END: return os << "END";
    case Token::Kind::INT: return os << "INT";
    case Token::Kind::STRING: return os << "STRING";
    case Token::Kind::IDENT: return os << "IDENT";
  }
  return os;
}

// -----------------------------------------------------------------------------
static std::string FormatMessage(const Location &loc, const std::string &msg)
{
  std::ostringstream os;
  os << "[" << loc.Name << ":" << loc.Line << ":" << loc.Column << "] " << msg;
  return os.str();
}

// -----------------------------------------------------------------------------
LexerError::LexerError(const Location &loc, const std::string &msg)
  : std::runtime_error(FormatMessage(loc, msg))
{
}

// -----------------------------------------------------------------------------
Lexer::Lexer(const std::string &name)
  : name_(name)
  , is_(name)
{
  NextChar();
  Next();
}

// -----------------------------------------------------------------------------
static bool IsIdentStart(char chr)
{
  return chr == '_' || isalpha(chr);
}

// -----------------------------------------------------------------------------
static bool IsIdentLetter(char chr)
{
  return IsIdentStart(chr) || isdigit(chr);
}

// -----for-------------------------------------------------------------------
static bool IsDigit(char chr)
{
  return isdigit(chr);
}

// -----------------------------------------------------------------------------
const Token &Lexer::Next()
{
  // Skip all whitespace until a valid token.
  while (isspace(chr_)) { NextChar(); }

  // Return a token based on the character.
  auto loc = GetLocation();
  switch (chr_) {
    case '\0': return tk_ = Token::End(loc);
    case '(': return NextChar(), tk_ = Token::LParen(loc);
    case ')': return NextChar(), tk_ = Token::RParen(loc);
    case '{': return NextChar(), tk_ = Token::LBrace(loc);
    case '}': return NextChar(), tk_ = Token::RBrace(loc);
    case ':': return NextChar(), tk_ = Token::Colon(loc);
    case ';': return NextChar(), tk_ = Token::Semi(loc);
    //2.2.a Add the required tokens to the lexer (==, *, /, % and -)
    case '=': {
      //we need to check if the next char is also '=' -> different token
      NextChar();
      if(chr_ == '='){        
        NextChar();
        return tk_ = Token::Equals(loc); // ==
      }
      else           
        return NextChar(), tk_ = Token::Equal(loc);
    }
    
    case '+': return NextChar(), tk_ = Token::Plus(loc);
    case '-': return NextChar(), tk_ = Token::Minus(loc);
    case '*': return NextChar(), tk_ = Token::Mul(loc);
    case '/': return NextChar(), tk_ = Token::Div(loc);
    case '%': return NextChar(), tk_ = Token::Mod(loc);
    case ',': return NextChar(), tk_ = Token::Comma(loc);
    case '"': {
      std::string word;
      NextChar();
      while (chr_ != '"') {
        word.push_back(chr_);
        NextChar();
        if (chr_ == '\0') {
          Error("string not terminated");
        }
      }
      NextChar();
      return tk_ = Token::String(loc, word);
    }
    default: {
      if (IsIdentStart(chr_)) {
        std::string word;
        //read the word untill it finishes
        do {
          word.push_back(chr_);
          NextChar();
        } while (IsIdentLetter(chr_));
        //return token for special words
        if (word == "func") return tk_ = Token::Func(loc);
        if (word == "return") return tk_ = Token::Return(loc);
        if (word == "while") return tk_ = Token::While(loc);
        //2.3.
        if (word == "if") return tk_ = Token::If(loc);
        if (word == "else") return tk_ = Token::Else(loc);
        //3.1.
        if (word == "let") return tk_ = Token::Let(loc);

        return tk_ = Token::Ident(loc, word);
      }

      // 1.2. (c) In lexer.cpp, find a suitable point to detect the presence of digits, convert them to an integer
      // and return the appropriate token.
      else
        if(IsDigit(chr_)){
          //we need to read the number until the end
          std::string number;
          //while the chr_ is digit, read next char
          do {
            number.push_back(chr_);//and push the digit on the stack
            NextChar();
          } while (IsDigit(chr_));
          return tk_ = Token::Integer(loc, stoi(number));
        }
      Error("unknown character '" + std::string(1, chr_) + "'");
    }
  }
}

// -----------------------------------------------------------------------------
void Lexer::NextChar()
{
  if (is_.eof()) {
    chr_ = '\0';
  } else {
    if (chr_ == '\n') {
      lineNo_++;
      charNo_ = 1;
    } else {
      charNo_++;
    }
    is_.get(chr_);
  }
}

// -----------------------------------------------------------------------------
void Lexer::Error(const std::string &msg)
{
  throw LexerError(GetLocation(), msg);
}