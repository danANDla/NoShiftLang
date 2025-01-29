#include "IRcompiler.hpp"

#include <iostream>
#include <cstring>

bool NoShiftCompiler::varnameTaken(const std::string& varname) const {
    return m_var_table.find(varname) != m_var_table.end();
}

std::string NoShiftCompiler::putTmp(CommonNoShiftTypedVar var) {
    std::string res = stack_marker + ":" + std::to_string(m_expr_stack.size());
    var.addr = res;
    m_expr_stack.emplace_back(m_expr_stack.size(), var);
    return res;
}

CommonNoShiftTypedVar::VarType NoShiftCompiler::typeByAddr(const std::string& addr) const {
    CommonNoShiftTypedVar var(CommonNoShiftTypedVar::NAN, 0);
    if (addr.find(stack_marker) == std::string::npos) {
        auto named_var = m_var_table.find(addr);
        if(named_var == m_var_table.end()) return var.m_type; 
        return (*named_var).second.m_type;
    }

    std::size_t del_pos = addr.find(":");
    del_pos = std::atoi(addr.substr(del_pos + 1, addr.size() - 1).c_str());
    var = getStackElement(del_pos);
    return var.m_type;
}

CommonNoShiftTypedVar NoShiftCompiler::getStackElement(const std::size_t from_top) const {
    auto it = m_expr_stack.begin();
    std::size_t curr = 0;
    CommonNoShiftTypedVar var(CommonNoShiftTypedVar::NAN, 0);
    while(it != m_expr_stack.end()) {
        if(curr == from_top) {
            var = (*it).second;
            return var;
        }
        it++;
        curr++;
    }
    return var;
}

std::size_t NoShiftCompiler::freeUntil(const std::size_t stack_size)  {
    std::size_t count = 0;
    while(m_expr_stack.size() > stack_size) {
        m_expr_stack.pop_back();
        count ++;
    }
    return count;
}

std::any NoShiftCompiler::visitAssignment(NoShiftParser::AssignmentContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    if(m_var_table.find(p_id) == m_var_table.end()) {
        throw std::runtime_error(std::string("Присвоение необъявленной переменной не поддерживаются"));
    }
    CommonNoShiftTypedVar var = m_var_table[p_id];

    std::string rightval_addr = std::any_cast<std::string>(visit(ctx->expr()));
    CommonNoShiftTypedVar::VarType right_type = typeByAddr(rightval_addr);

    if(var.m_type != right_type) {
        throw std::runtime_error(std::string("Присвоение разных типов не поддерживаются"));
    }
    std::cout << p_id << " = " << rightval_addr << std::endl;
    return p_id;
}

std::any NoShiftCompiler::visitVarDecl(NoShiftParser::VarDeclContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    if(varnameTaken(p_id)) {
        throw std::runtime_error(std::string("Повторное объявление переменной ") + p_id);
    }

    antlr4::tree::TerminalNode* poss_str = ctx->STRING_TYPE();
    antlr4::tree::TerminalNode* poss_d = ctx->INTEGER_TYPE();
    antlr4::tree::TerminalNode* poss_l = ctx->LOGIC_TYPE();

    const std::string& val_addr = std::any_cast<std::string>(visit(ctx->expr()));
    CommonNoShiftTypedVar::VarType val_type = typeByAddr(val_addr);

    if(poss_l != nullptr) {
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::LOGIC_VAR, std::any_cast<bool>(false));
        m_var_table[p_id] = var;
    } else if (poss_str != nullptr) {
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::STRING_VAR, std::any_cast<std::string>(""));
        m_var_table[p_id] = var;
    } else {
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::INT_VAR, std::any_cast<int>(1));
        m_var_table[p_id] = var;
    }
    std::cout << p_id << " = " << val_addr << std::endl;
    return p_id;
}

std::any NoShiftCompiler::visitPrint(NoShiftParser::PrintContext *ctx) {
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

std::any NoShiftCompiler::visitNumExpr(NoShiftParser::NumExprContext *ctx) {
    int val = std::stoi(ctx->NUM()->toString());
    std::string addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::INT_VAR, 0));
    std::cout << addr << " = " << val << std::endl;
    return addr;
}

std::any NoShiftCompiler::visitInvNumExpr(NoShiftParser::InvNumExprContext *ctx) {
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

std::any NoShiftCompiler::visitIdExp(NoShiftParser::IdExpContext *ctx) {
    const std::string& p_id = ctx->ID()->toString();
    return p_id;
}

std::any NoShiftCompiler::visitStrExpr(NoShiftParser::StrExprContext *ctx) {
    std::string val = ctx->STR()->toString();
    std::string addr =  putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::STRING_VAR, ""));
    std::cout << addr << " = " << val << std::endl;
    return addr;
}

std::any NoShiftCompiler::visitLogicConstExpr(NoShiftParser::LogicConstExprContext *ctx) {
    std::string val = ctx->LOGIC_C()->toString();
    std::string addr;
    if(val == "true") {
        addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, true));
    } else {
        addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, false));
    }
    std::cout << addr << " = " << val << std::endl;
    return addr;
}

std::any NoShiftCompiler::visitPlusMinusExpr(NoShiftParser::PlusMinusExprContext *ctx) {
    std::string leftval_addr = std::any_cast<std::string>(visit(ctx->left));
    std::string rightval_addr = std::any_cast<std::string>(visit(ctx->right));
    CommonNoShiftTypedVar::VarType left_type = typeByAddr(leftval_addr);
    CommonNoShiftTypedVar::VarType right_type = typeByAddr(rightval_addr);

    if(left_type != right_type) {
        throw std::runtime_error(std::string("Арифметические операции над выражениями разных типов не поддерживаются"));
    }

    antlr4::tree::TerminalNode* poss_plus = ctx->PLUS();
    bool is_plus = true;
    if(poss_plus == nullptr) is_plus = false;


    std::string resaddr;
    if(left_type == CommonNoShiftTypedVar::INT_VAR) {
        resaddr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::INT_VAR, 0));
        if(is_plus) {
            std::cout << resaddr << " " << leftval_addr << " + " << rightval_addr << std::endl;
        } else {
            std::cout << resaddr << " " << leftval_addr << " - " << rightval_addr << std::endl;
        }
    } else if(left_type == CommonNoShiftTypedVar::LOGIC_VAR) {
        throw std::runtime_error(std::string("Арифметические операции с типом LOGIC не поддерживаются"));
    } else {
        resaddr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::STRING_VAR, 0));
        if(is_plus) {
            std::cout << resaddr << " " << leftval_addr << " + " << rightval_addr << std::endl;
        } else {
            std::cout << resaddr << " " << leftval_addr << " - " << rightval_addr << std::endl;
        }
    }
    return resaddr;
}

std::any NoShiftCompiler::visitMulDivExpr(NoShiftParser::MulDivExprContext *ctx) {
    std::string leftval_addr = std::any_cast<std::string>(visit(ctx->left));
    std::string rightval_addr = std::any_cast<std::string>(visit(ctx->right));
    CommonNoShiftTypedVar::VarType left_type = typeByAddr(leftval_addr);
    CommonNoShiftTypedVar::VarType right_type = typeByAddr(rightval_addr);

    if(left_type != right_type) {
        throw std::runtime_error(std::string("Арифметические операции над выражениями разных типов не поддерживаются"));
    }

    antlr4::tree::TerminalNode* poss_plus = ctx->ASTERISK();
    bool is_plus = true;
    if(poss_plus == nullptr) is_plus = false;

    std::string resaddr;
    if(left_type == CommonNoShiftTypedVar::INT_VAR) {
        resaddr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::INT_VAR, 0));
        if(is_plus) {
            std::cout << resaddr << " " << leftval_addr << " * " << rightval_addr << std::endl;
        } else {
            std::cout << resaddr << " " << leftval_addr << " / " << rightval_addr << std::endl;
        }
    } else if(left_type == CommonNoShiftTypedVar::LOGIC_VAR) {
        throw std::runtime_error(std::string("Арифметические операции с типом LOGIC не поддерживаются"));
    } else {
        throw std::runtime_error(std::string("Умножение и деление строк не поддерживается"));
    }
    return resaddr;
}

std::any NoShiftCompiler::visitParenthesisExpr(NoShiftParser::ParenthesisExprContext *ctx) {
    return visit(ctx->expr());
}

std::any NoShiftCompiler::visitStmt(NoShiftParser::StmtContext *ctx) {
    std::size_t stack_state = m_expr_stack.size();
    visitChildren(ctx);
    std::size_t freed = freeUntil(stack_state);
    std::cout << "freed stack: " << freed << std::endl;
    return freed;
}

std::any NoShiftCompiler::visitCompExpr(NoShiftParser::CompExprContext *ctx) {
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

std::any NoShiftCompiler::visitLogicExpr(NoShiftParser::LogicExprContext *ctx) {
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