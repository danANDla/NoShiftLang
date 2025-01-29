#include "Interpreter.hpp"

#include <iostream>
#include <cstring>

CommonNoShiftTypedVar::CommonNoShiftTypedVar(VarType type, std::any val)
    : m_type(type), m_val(val){
}

CommonNoShiftTypedVar::CommonNoShiftTypedVar(CommonNoShiftTypedVar&& another)
    : m_type(another.m_type), m_val(another.m_val){
}

CommonNoShiftTypedVar::CommonNoShiftTypedVar(const CommonNoShiftTypedVar& another)
    : m_type(another.m_type), m_val(another.m_val){
}

CommonNoShiftTypedVar& CommonNoShiftTypedVar::operator=(const CommonNoShiftTypedVar& another) {
    m_type = another.m_type;
    m_val = another.m_val;
    return *this;
}

bool NoShiftInterp::varnameTaken(const std::string& varname) const {
    return m_var_table.find(varname) != m_var_table.end();
}

std::any NoShiftInterp::visitAssignment(NoShiftParser::AssignmentContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    if(m_var_table.find(p_id) == m_var_table.end()) {
        throw std::runtime_error(std::string("Присвоение необъявленной переменной не поддерживаются"));
    }
    std::any val = visit(ctx->expr());

    CommonNoShiftTypedVar var = m_var_table[p_id];
    if(std::strcmp(var.m_val.type().name(), val.type().name()) != 0) {
        throw std::runtime_error(std::string("Присвоение разных типов не поддерживаются"));
    }

    CommonNoShiftTypedVar::VarType right_type;
    if(std::strcmp(val.type().name(), "i") == 0) {
        right_type = CommonNoShiftTypedVar::INT_VAR;
    } else if(std::strcmp(val.type().name(), "b") == 0) {
        right_type = CommonNoShiftTypedVar::LOGIC_VAR;
    } else {
        right_type = CommonNoShiftTypedVar::STRING_VAR;
    }
    m_var_table[p_id].m_val = val;
    return p_id;
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
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::LOGIC_VAR, std::any_cast<bool>(val));
        m_var_table[p_id] = var;
    } else if (poss_str != nullptr) {
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::STRING_VAR, std::any_cast<std::string>(val));
        m_var_table[p_id] = var;
    } else {
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::INT_VAR, std::any_cast<int>(val));
        m_var_table[p_id] = var;
    }
    return visitChildren(ctx);
}

std::any NoShiftInterp::visitPrint(NoShiftParser::PrintContext *ctx) {
    std::any val = visit(ctx->expr());
    if(std::strcmp(val.type().name(), "i") == 0) {
        std::cout << "printing " << std::any_cast<int>(val) << std::endl;
    } else if(std::strcmp(val.type().name(), "b") == 0) {
        if(std::any_cast<bool>(val)) {
            std::cout << "printing true" << std::endl;
        } else {
            std::cout << "printing false" << std::endl;
        }
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
    std::any val = visit(ctx->expr());
    if(std::strcmp(val.type().name(), "i") == 0) {
        return -std::any_cast<int>(val);
    } else if (std::strcmp(val.type().name(), "b") == 0) {
        return !std::any_cast<bool>(val);
    } else {
        std::string str_val = std::any_cast<std::string>(val);
        std::reverse(str_val.begin(), str_val.end());
        return str_val;
    }
}

std::any NoShiftInterp::visitIdExp(NoShiftParser::IdExpContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    return m_var_table[p_id].m_val;
}

std::any NoShiftInterp::visitStrExpr(NoShiftParser::StrExprContext *ctx) {
    std::string val = ctx->STR()->toString();
    return val.substr(1, val.size() - 2);
}

std::any NoShiftInterp::visitLogicConstExpr(NoShiftParser::LogicConstExprContext *ctx) {
    std::string val = ctx->LOGIC_C()->toString();
    if(val == "true")
        return true;
    return false;
}

std::any NoShiftInterp::visitPlusMinusExpr(NoShiftParser::PlusMinusExprContext *ctx) {

    std::any leftval = visit(ctx->left);
    std::any rightval = visit(ctx->right);

    if(std::strcmp(leftval.type().name(), rightval.type().name()) != 0) {
        std::cout << leftval.type().name() << " " << rightval.type().name() << std::endl;
        throw std::runtime_error(std::string("Арифметические операции над выражениями разных типов не поддерживаются"));
    }

    antlr4::tree::TerminalNode* poss_plus = ctx->PLUS();
    bool is_plus = true;
    if(poss_plus == nullptr) is_plus = false;

    if(std::strcmp(leftval.type().name(), "i") == 0) {
        if(is_plus) {
            return std::any_cast<int>(leftval) + std::any_cast<int>(rightval);
        } else {
            return std::any_cast<int>(leftval) - std::any_cast<int>(rightval);
        }
    } else if(std::strcmp(leftval.type().name(), "b") == 0) {
        throw std::runtime_error(std::string("Арифметические операции с типом LOGIC не поддерживаются"));
    } else {
        if(is_plus) {
            return std::any_cast<std::string>(leftval) + std::any_cast<std::string>(rightval);
        } else {
            std::string leftstr = std::any_cast<std::string>(leftval);
            std::string rightstr = std::any_cast<std::string>(rightval);
            if(leftstr.size() >= rightstr.size()) {
                return leftstr.substr(0, leftstr.size() - rightstr.size());
            } else {
                return rightstr.substr(0, rightstr.size() - leftstr.size());
            }
        }
    }

}

std::any NoShiftInterp::visitMulDivExpr(NoShiftParser::MulDivExprContext *ctx) {
    std::any leftval = visit(ctx->left);
    std::any rightval = visit(ctx->right);

    if(std::strcmp(leftval.type().name(), rightval.type().name()) != 0) {
        throw std::runtime_error(std::string("Арифметические операции над выражениями разных ти пов не поддерживаются"));
    }

    antlr4::tree::TerminalNode* poss_asterisk = ctx->ASTERISK();
    bool is_asterisk = true;
    if(poss_asterisk == nullptr) is_asterisk = false;

    if(std::strcmp(leftval.type().name(), "i") == 0) {
        if(is_asterisk) {
            return std::any_cast<int>(leftval) * std::any_cast<int>(rightval);
        } else {
            return std::any_cast<int>(leftval) / std::any_cast<int>(rightval);
        }
    } else if(std::strcmp(leftval.type().name(), "b") == 0) {
        throw std::runtime_error(std::string("Арифметические операции с типом LOGIC не поддерживаются"));
    } else {
        throw std::runtime_error(std::string("Умножение и деление строк не поддерживается"));
    }
}

std::any NoShiftInterp::visitParenthesisExpr(NoShiftParser::ParenthesisExprContext *ctx) {
    return visit(ctx->expr());
}

std::any NoShiftInterp::visitCompExpr(NoShiftParser::CompExprContext *ctx) {
    std::any leftval = visit(ctx->left);
    std::any rightval = visit(ctx->right);

    NoShiftParser::CompOperatorContext* op_ctx = ctx->compOperator();
    antlr4::tree::TerminalNode* poss_ls = op_ctx->LESS();
    antlr4::tree::TerminalNode* poss_gr = op_ctx->GREATER();
    antlr4::tree::TerminalNode* poss_is = op_ctx->EQUAL();
    antlr4::tree::TerminalNode* poss_nq = op_ctx->NOT_EQUAL();

    if(poss_is || poss_nq) {
        if(std::strcmp(leftval.type().name(), rightval.type().name()) != 0) {
            if(poss_is != nullptr) return false;
            return true;
        } else {
            if(std::strcmp(leftval.type().name(), "i") == 0) {
                if(poss_is != nullptr){
                    return std::any_cast<int>(leftval) == std::any_cast<int>(rightval);
                }
                return std::any_cast<int>(leftval) != std::any_cast<int>(rightval);
            } else if(std::strcmp(leftval.type().name(), "b") == 0) {
                if(poss_is != nullptr){
                    return std::any_cast<bool>(leftval) == std::any_cast<bool>(rightval);
                }
                return std::any_cast<bool>(leftval) != std::any_cast<bool>(rightval);
            }  else {
                if(poss_is != nullptr){
                    return std::any_cast<std::string>(leftval).compare(std::any_cast<std::string>(rightval)) == 0;
                }
                return std::any_cast<std::string>(leftval).compare(std::any_cast<std::string>(rightval)) != 0;
            }
        }
    }

    if(std::strcmp(leftval.type().name(), rightval.type().name()) != 0) {
        return false;
    }

    if(std::strcmp(leftval.type().name(), "i") == 0) {
        if(poss_ls != nullptr){
            return std::any_cast<int>(leftval) < std::any_cast<int>(rightval);
        }
        return std::any_cast<int>(leftval) > std::any_cast<int>(rightval);
    } else if(std::strcmp(leftval.type().name(), "b") == 0) {
        if(poss_ls != nullptr){
            return std::any_cast<bool>(leftval) < std::any_cast<bool>(rightval);
        }
        return std::any_cast<bool>(leftval) > std::any_cast<bool>(rightval);
    }  else {
        if(poss_ls != nullptr){
            return std::any_cast<std::string>(leftval).compare(std::any_cast<std::string>(rightval)) < 0;
        }
        return std::any_cast<std::string>(leftval).compare(std::any_cast<std::string>(rightval)) > 0;
    }

}

std::any NoShiftInterp::visitLogicExpr(NoShiftParser::LogicExprContext *ctx) {
    std::any leftval = visit(ctx->left);
    std::any rightval = visit(ctx->right);

    if(std::strcmp(leftval.type().name(), rightval.type().name()) != 0) {
        std::cout << leftval.type().name() << " " << rightval.type().name() << std::endl;
        throw std::runtime_error(std::string("Логические операции над выражениями разных типов не поддерживаются"));
    }

    antlr4::tree::TerminalNode* poss_and = ctx->LOGAND();
    antlr4::tree::TerminalNode* poss_xor = ctx->LOGXOR();

    if(std::strcmp(leftval.type().name(), "i") == 0) {
        throw std::runtime_error(std::string("Логические операции с типом INTEGER не поддерживаются"));
    } else if(std::strcmp(leftval.type().name(), "b") == 0) {
        if(poss_and != nullptr) {
            return (bool) (std::any_cast<bool>(leftval) & std::any_cast<bool>(rightval));
        } else if(poss_xor != nullptr) {
            return (bool) (std::any_cast<bool>(leftval) ^ std::any_cast<bool>(rightval));
        } else {
            return (bool) (std::any_cast<bool>(leftval) || std::any_cast<bool>(rightval));
        }
    } else {
        throw std::runtime_error(std::string("Логические операции с типом STRING не поддерживаются"));
    }
}

std::any NoShiftInterp::visitIfstmt(NoShiftParser::IfstmtContext *ctx) {
    std::any cond_result = visit(ctx->expr());
    if(std::strcmp(cond_result.type().name(), "b") != 0) {
        throw std::runtime_error(std::string("В условии должно быть выражение типа LOGIC"));
    }

    bool res = std::any_cast<bool>(cond_result);
    if(res) {
        std::any stmt = visit(ctx->stmt());    
    } else if(ctx->elsestmt() != nullptr) {
        NoShiftParser::ElsestmtContext* else_ctx = ctx->elsestmt();
        std::any stmt = visit(else_ctx->stmt());    
    }
    return res;
}

std::any NoShiftInterp::visitWhilestmt(NoShiftParser::WhilestmtContext *ctx) {
    std::any cond_result = visit(ctx->expr());
    if(std::strcmp(cond_result.type().name(), "b") != 0) {
        throw std::runtime_error(std::string("В условии должно быть выражение типа LOGIC"));
    }
    bool res = std::any_cast<bool>(cond_result);
    while(res) {
        std::any stmt = visit(ctx->stmt());    
        std::any cond_result = visit(ctx->expr());
        res = std::any_cast<bool>(cond_result);
    }
    return res;

}