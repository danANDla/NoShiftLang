#include "Interpreter.hpp"

#include <iostream>
#include <cstring>

CommonNoShiftTypedVar::CommonNoShiftTypedVar(VarType type, std::any val)
    : m_type(type), m_val(val){
}

CommonNoShiftTypedVar::CommonNoShiftTypedVar(CommonNoShiftTypedVar&& another)
    : m_type(another.m_type), m_val(another.m_val){
}

CommonNoShiftTypedVar& CommonNoShiftTypedVar::operator=(const CommonNoShiftTypedVar& another) {
    m_type = another.m_type;
    m_val = another.m_val;
    return *this;
}

bool NoShiftInterp::varnameTaken(const std::string& varname) const {
    return var_table.find(varname) != var_table.end();
}

std::any NoShiftInterp::visitAssignment(NoShiftParser::AssignmentContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    std::cout << "visiting " << p_id << std::endl;
    return visitChildren(ctx);
}

std::any NoShiftInterp::visitVarDecl(NoShiftParser::VarDeclContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    if(varnameTaken(p_id)) {
        throw std::runtime_error(std::string("Повторное объявление переменной ") + p_id);
    }

    antlr4::tree::TerminalNode* poss_str = ctx->STRING_TYPE();
    antlr4::tree::TerminalNode* poss_d = ctx->INTEGER_TYPE();
    antlr4::tree::TerminalNode* poss_l = ctx->LOGIC_TYPE();
    NoShiftParser::ExprContext* expr = ctx->expr();
    std::any val = visit(expr);

    if(poss_l != nullptr) {
        std::cout <<" l type\n";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::LOGIC_VAR, std::any_cast<bool>(val));
        var_table[p_id] = var;
    } else if (poss_str != nullptr) {
        std::cout <<"str type\n";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::STRING_VAR, std::any_cast<std::string>(val));
        var_table[p_id] = var;
    } else {
        std::cout <<"int type\n";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::INT_VAR, std::any_cast<int>(val));
        var_table[p_id] = var;
    }
    return visitChildren(ctx);
}

std::any NoShiftInterp::visitPrint(NoShiftParser::PrintContext *ctx) {
    std::any val = visit(ctx->expr());
    if(std::strcmp(val.type().name(), "i") == 0) {
        std::cout << "printing " << std::any_cast<int>(val) << std::endl;
    } else if(std::strcmp(val.type().name(), "s") == 0) {
        std::cout << "printing " << std::any_cast<std::string>(val) << std::endl;
    } else {
        // std::cout << "bad type to print: " << val.type().name() << std::endl;
        std::cout << "printing " << std::any_cast<std::string>(val) << std::endl;
    }
    return val;
}

std::any NoShiftInterp::visitNumExpr(NoShiftParser::NumExprContext *ctx) {
    int val = std::stoi(ctx->NUM()->toString());
    return val;
}

std::any NoShiftInterp::visitInvNumExpr(NoShiftParser::InvNumExprContext *ctx) {
    int val = std::stoi(ctx->NUM()->toString());
    return -val;
}

std::any NoShiftInterp::visitIdExp(NoShiftParser::IdExpContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    return var_table[p_id].m_val;
}

std::any NoShiftInterp::visitStrExpr(NoShiftParser::StrExprContext *ctx) {
    std::string val = ctx->STR()->toString();
    return val.substr(1, val.size() - 2);
}