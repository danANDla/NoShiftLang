#include "generated/NoShiftParser.h"
#include "generated/NoShiftLexer.h"
#include "generated/NoShiftVisitor.h"
#include "Interpreter.hpp"
#include "IRcompiler.hpp"
#include "antlr4-runtime.h"
#include <fstream>
#include <iostream>
using namespace antlrcpp;
using namespace antlr4;
using namespace std;

int main(int argc, const char *args[]) {
    ifstream ins;
    // Create the input stream.
    std::cout <<"\n input program \n---------------------------\n";
    ins.open(args[1]);
    std:string str;
    while (std::getline(ins, str)) {
        std::cout << str << std::endl;
    }
    ins.close();

    ins.open(args[1]);

    ANTLRInputStream input(ins);

    // Create a lexer which scans the input stream
    // to create a token stream.
    NoShiftLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    // Print the token stream.
    // cout << "Tokens:" << endl;
    // tokens.fill();
    // for (Token *token : tokens.getTokens()) {
    //     std::cout << token->toString() << std::endl;
    // }

    NoShiftParser parser(&tokens);
    NoShiftParser::ProgContext* prog_cxt = parser.prog();

    // Enter interpreter
    std::cout <<"\n INTERPRETATOR \n---------------------------\n";
    NoShiftInterp interp;   
    interp.visitProg(prog_cxt);

    // Enter compiler
    std::cout <<"\n COMPILING into IR\n---------------------------\n";
    NoShiftCompiler compiler;   
    compiler.visitProg(prog_cxt);
    return 0;
}
