#include "Interpreter.hpp"

#include <iostream>

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

    if(poss_l != nullptr) {
        std::cout <<" l type\n";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::LOGIC_VAR, false);
    } else if (poss_str != nullptr) {
        std::cout <<"str type\n";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::STRING_VAR,"empty");
    } else {
        std::cout <<"int type\n";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::INT_VAR, 0);
        var_table[p_id] = var;
    }
    return visitChildren(ctx);
}
