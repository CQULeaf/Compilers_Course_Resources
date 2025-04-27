/*
Exp -> AddExp

    Exp.v

Number -> IntConst | floatConst

PrimaryExp -> '(' Exp ')' | Number
    PrimaryExp.v

UnaryExp -> PrimaryExp | UnaryOp UnaryExp
    UnaryExp.v

UnaryOp -> '+' | '-'

MulExp -> UnaryExp { ('*' | '/') UnaryExp }
    MulExp.v

AddExp -> MulExp { ('+' | '-') MulExp }
    AddExp.v
*/
#include<map>
#include<cassert>
#include<string>
#include<iostream>
#include<vector>
#include<set>
#include<queue>

#define TODO assert(0 && "TODO")
// #define DEBUG_DFA
// #define DEBUG_PARSER

// enumerate for Status
enum class State {
    Empty,              // space, \n, \r ...
    IntLiteral,         // int literal, like '1' '01900', '0xAB', '0b11001'
    op                  // operators and '(', ')'
};
std::string toString(State s) {
    switch (s) {
    case State::Empty: return "Empty";
    case State::IntLiteral: return "IntLiteral";
    case State::op: return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

// enumerate for Token type
enum class TokenType{
    INTLTR,        // int literal
    PLUS,        // +
    MINU,        // -
    MULT,        // *
    DIV,        // /
    LPARENT,        // (
    RPARENT,        // )
};
std::string toString(TokenType type) {
    switch (type) {
    case TokenType::INTLTR: return "INTLTR";
    case TokenType::PLUS: return "PLUS";
    case TokenType::MINU: return "MINU";
    case TokenType::MULT: return "MULT";
    case TokenType::DIV: return "DIV";
    case TokenType::LPARENT: return "LPARENT";
    case TokenType::RPARENT: return "RPARENT";
    default:
        assert(0 && "invalid token type");
        break;
    }
    return "";
}

// definition of Token
struct Token {
    TokenType type;
    std::string value;
};

// definition of DFA
struct DFA {
    /**
     * @brief constructor, set the init state to State::Empty
     */
    DFA();
    
    /**
     * @brief destructor
     */
    ~DFA();
    
    // the meaning of copy and assignment for a DFA is not clear, so we do not allow them
    DFA(const DFA&) = delete;   // copy constructor
    DFA& operator=(const DFA&) = delete;    // assignment

    /**
     * @brief take a char as input, change state to next state, and output a Token if necessary
     * @param[in] input: the input character
     * @param[out] buf: the output Token buffer
     * @return  return true if a Token is produced, the buf is valid then
     */
    bool next(char input, Token& buf);

    /**
     * @brief reset the DFA state to begin
     */
    void reset();

private:
    State cur_state;    // record current state of the DFA
    std::string cur_str;    // record input characters
};


DFA::DFA(): cur_state(State::Empty), cur_str() {}

DFA::~DFA() {}

// helper function, you are not require to implement these, but they may be helpful
bool isoperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')';
}

TokenType get_op_type(std::string s) {
    if (s == "+") return TokenType::PLUS;
    else if (s == "-") return TokenType::MINU; 
    else if (s == "*") return TokenType::MULT;
    else if (s == "/") return TokenType::DIV;
    else if (s == "(") return TokenType::LPARENT;
    else if (s == ")") return TokenType::RPARENT;
    else {
        assert(0 && "invalid operator type");
    }
    return TokenType::INTLTR; // This line will never be reached, but it is needed to avoid compiler error
}

bool DFA::next(char input, Token& buf) {
    switch (cur_state) {
        case State::Empty:
            if (input == ' ') {
                cur_state = State::Empty;
            } else if (isdigit(input)) {
                cur_state = State::IntLiteral;
                cur_str += input;
            } else if (isoperator(input)) {
                cur_state = State::op;
                cur_str += input;
            }
            break;

        case State::IntLiteral:
            if (cur_str == "0" && (input == 'x' || input == 'b')) {
                cur_str += input;
            } else if (cur_str.size() >= 2 && cur_str[1] == 'x' && isxdigit(input)) {
                cur_str += input;
            } else if (isdigit(input)) {
                cur_str += input;
            } else {
                buf.type = TokenType::INTLTR;
                buf.value = cur_str;
                if (isoperator(input)) {
                    cur_state = State::op;
                    cur_str = input;
                } else if (input == ' ') {
                    cur_state = State::Empty;
                    cur_str = "";
                }
                return true;
            }
            break;

        case State::op:
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;
            if (isoperator(input)) {
                cur_str = input;
            } else if (input == ' ') {
                cur_state = State::Empty;
                cur_str = "";
            } else if (isdigit(input)) {
                cur_state = State::IntLiteral;
                cur_str = input;
            }
            return true;
    }
    return false;
}

void DFA::reset() {
    cur_state = State::Empty;
    cur_str = "";
}

// hw2
enum class NodeType {
    TERMINAL,   // 终结符（数字、运算符）
    EXP,        // 表达式
    NUMBER,     // 数字
    PRIMARYEXP, // 基础表达式
    UNARYEXP,   // 一元表达式
    UNARYOP,    // 一元运算符
    MULEXP,     // 乘法表达式
    ADDEXP,     // 加法表达式
    NONE
};
std::string toString(NodeType nt) {
    switch (nt) {
    case NodeType::TERMINAL: return "Terminal";
    case NodeType::EXP: return "Exp";
    case NodeType::NUMBER: return "Number";
    case NodeType::PRIMARYEXP: return "PrimaryExp";
    case NodeType::UNARYEXP: return "UnaryExp";
    case NodeType::UNARYOP: return "UnaryOp";
    case NodeType::MULEXP: return "MulExp";
    case NodeType::ADDEXP: return "AddExp";
    case NodeType::NONE: return "NONE";
    default:
        assert(0 && "invalid node type");
        break;
    }
    return "";
}

// tree node basic class
struct AstNode{
    int value;
    NodeType type;  // the node type
    AstNode* parent;    // the parent node
    std::vector<AstNode*> children;     // children of node

    /**
     * @brief constructor
     */
    AstNode(NodeType t = NodeType::NONE, AstNode* p = nullptr): type(t), parent(p), value(0) {} 

    /**
     * @brief destructor
     */
    virtual ~AstNode() {
        for(auto child: children) {
            delete child;
        }
    }

    // rejcet copy and assignment
    AstNode(const AstNode&) = delete;
    AstNode& operator=(const AstNode&) = delete;
};

// definition of Parser
// a parser should take a token stream as input, then parsing it, output a AST
struct Parser {
    uint32_t index; // current token index
    const std::vector<Token>& token_stream;

    /**
     * @brief constructor
     * @param tokens: the input token_stream
     */
    Parser(const std::vector<Token>& tokens): index(0), token_stream(tokens) {}

    /**
     * @brief destructor
     */
    ~Parser() {}
    
    /**
     * @brief creat the abstract syntax tree
     * @return the root of abstract syntax tree
     */
    AstNode* get_abstract_syntax_tree() {
        AstNode* root = new AstNode(NodeType::EXP, nullptr);
        parseExp(root);
        return root;
    }

    // u can define member funcition of Parser here
    void parseExp(AstNode* node);
    void parseNumber(AstNode* node);
    void parsePrimaryExp(AstNode* node);
    void parseUnaryExp(AstNode* node);
    void parseUnaryOp();
    void parseMulExp(AstNode* node);
    void parseAddExp(AstNode* node);


    // for debug, u r not required to use this
    // how to use this: in ur local enviroment, defines the macro DEBUG_PARSER and add this function in every parse fuction
void log(AstNode* node){
#ifdef DEBUG_PARSER
        std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << 'n';
#endif
    }
    
};

// u can define funcition here

// Transform the string to int, the string can be in decimal, octal, hex or binary format
// 0xAB -> 171, 0b11001 -> 25, 01900 -> 1000, 1234 -> 1234
int trans2ten(std::string value) {
    if (value[0] == '0' && value[1] == 'x') {
        return std::stoi(value, nullptr, 16);
    } else if (value[0] == '0' && value[1] == 'b') {
        return std::stoi(value, nullptr, 2);
    } else if (value[0] == '0') {
        return std::stoi(value, nullptr, 8);
    } else {
        return std::stoi(value, nullptr, 10);
    }
}

// parseExp function, parse the expression
void Parser::parseExp(AstNode* node) {
    AstNode* child = new AstNode(NodeType::ADDEXP, node); // create a child node for AddExp
    node->children.push_back(child); // add the child node to the parent node
    parseAddExp(child);  // parse the AddExp
}

// parseNumber function, parse the number
// if the token is a number, set the node value to the number and type to NUMBER
void Parser::parseNumber(AstNode* node) {
    if (token_stream[index].type == TokenType::INTLTR) {
        node->value = trans2ten(token_stream[index].value); // convert the string to int
        node->type = NodeType::NUMBER; // set the node type to NUMBER
        index++; // move to the next token
    } else {
        assert(0 && "parse number error"); // if the token is not a number, assert error
    }
}

// parsePrimaryExp function, parse the primary expression
// primary expression can be a number or an expression in parenthesis, like (1+2)
void Parser::parsePrimaryExp(AstNode* node) {
    if (token_stream[index].type == TokenType::LPARENT) { // if the token is '(', parse the expression in the parenthesis
        index++; // move to the next token
        AstNode* child = new AstNode(NodeType::EXP, node); // create a child node for Exp
        node->children.push_back(child); // add the child node to the parent node
        parseExp(child); // parse the expression
        if (token_stream[index].type == TokenType::RPARENT) { // if the token is ')', move to the next token
            index++;
        } else {
            assert(0 && "parse primary exp error"); // if the token is not ')', assert error
        }
    } else {
        parseNumber(node); // if the token is not '(', parse the number
    }
}

// parseUnaryExp function, parse the unary expression
// unary expression can be a primary expression or a unary operator followed by a unary expression, like -1 or + (1+2)
void Parser::parseUnaryExp(AstNode* node) {
    if (token_stream[index].type == TokenType::PLUS || token_stream[index].type == TokenType::MINU) { // if the token is '+' or '-', parse the unary operator
        parseUnaryOp(); // parse the unary operator
    }
    AstNode* child = new AstNode(NodeType::PRIMARYEXP, node); // create a child node for PrimaryExp
    node->children.push_back(child); // add the child node to the parent node
    parsePrimaryExp(child); // parse the primary expression
}

// parseUnaryOp function, parse the unary operator
// the unary operator can be '+' or '-'
void Parser::parseUnaryOp() {
    if (token_stream[index]. type == TokenType::PLUS || token_stream[index].type == TokenType::MINU) { // if the token is '+' or '-', move to the next token
        index++;
    } else {
        assert(0 && "parse unary op error"); // if the token is not '+' or '-', assert error
    }
}



int main(){
    std::string stdin_str;
    std::getline(std::cin, stdin_str);
    stdin_str += "\n";
    DFA dfa;
    Token tk;
    std::vector<Token> tokens;
    for (size_t i = 0; i < stdin_str.size(); i++) {
        if(dfa.next(stdin_str[i], tk)){
            tokens.push_back(tk); 
        }
    }

    // hw2
    Parser parser(tokens);
    auto root = parser.get_abstract_syntax_tree();
    // u may add function here to analysis the AST, or do this in parsing
    // like get_value(root);
    
    std::cout << root->value;

    return 0;
}