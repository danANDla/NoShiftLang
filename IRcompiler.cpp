#include "IRcompiler.hpp"

#include <iostream>
#include <cstring>

TripleAddrInstr::TripleAddrInstr(const AbstractOperation op, const std::string& res_addr, const std::string& a_operand, const std::string& b_operand) 
    : m_op (op),
        m_res_addr(res_addr),
        m_a_operand(a_operand),
        m_b_operand(b_operand) {

        }

TripleAddrInstr::TripleAddrInstr(TripleAddrInstr&& another)
    : m_op (another.m_op),
        m_res_addr(another.m_res_addr),
        m_a_operand(another.m_a_operand),
        m_b_operand(another.m_b_operand) {

}
TripleAddrInstr::TripleAddrInstr(TripleAddrInstr& another)
    : m_op (another.m_op),
        m_res_addr(another.m_res_addr),
        m_a_operand(another.m_a_operand),
        m_b_operand(another.m_b_operand) {

}

std::string operationName(const AbstractOperation op) {
    std::string res;
    switch (op)
    {
    case ASSIGN:
        res = "ASSIGN";
        break;
    case ASSIGN_CONST:
        res = "ASSIGN_CONST";
        break;
    case ADD:
        res = "ADD";
        break;
    case ADDstr:
        res = "ADDstr";
        break;
    case SUB:
        res = "SUB";
        break;
    case SUBstr:
        res = "SUBstr";
        break;
    case MUL:
        res = "MUL";
        break;
    case DIV:
        res = "DIV";
        break;
    case OR:
        res = "OR";
        break;
    case XOR:
        res = "XOR";
        break;
    case AND:
        res = "AND";
        break;

    case EQ:
        res = "EQ";
        break;
    case NEQ:
        res = "NEQ";
        break;
    case GR:
        res = "GR";
        break;
    case LS:
        res = "LS";
        break;
    case EQstr:
        res = "EQstr";
        break;
    case NEQstr:
        res = "NEQstr";
        break;
    case GRstr:
        res = "GRstr";
        break;
    case LSstr:
        res = "LSstr";
        break;

    case JNEQ:
        res = "JNEQ";
        break;
    case JUMP:
        res = "JUMP";
        break;

    case PRINT_STR:
        res = "PRINTSTR";
        break;
    case PRINT_INT:
        res = "PRINTINT";
        break;
    case PRINT_LOGIC:
        res = "PRINTLOGIC";
        break;
    case NOP:
        res = "NOP";
        break;
    default:
        std::cout << "unkown op " << op;
        break;
    }
    return res;
}

bool NoShiftCompiler::varnameTaken(const std::string& varname) const { return m_var_table.find(varname) != m_var_table.end();
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
    instructions.push_back(TripleAddrInstr(ASSIGN, p_id, rightval_addr, ""));
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
        std::any smth = "smth";
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::STRING_VAR, std::any_cast<int>(1));
        m_var_table[p_id] = var;
    } else {
        CommonNoShiftTypedVar var(CommonNoShiftTypedVar::INT_VAR, std::any_cast<int>(1));
        m_var_table[p_id] = var;
    }
    instructions.push_back(TripleAddrInstr(ASSIGN, p_id, val_addr, ""));
    return p_id;
}

std::any NoShiftCompiler::visitPrint(NoShiftParser::PrintContext *ctx) {
    std::string leftval_addr = std::any_cast<std::string>(visit(ctx->expr()));
    CommonNoShiftTypedVar::VarType left_type = typeByAddr(leftval_addr);

    if(left_type == CommonNoShiftTypedVar::VarType::INT_VAR) {
        instructions.push_back(TripleAddrInstr(PRINT_INT, "", leftval_addr, ""));
    } else if(left_type == CommonNoShiftTypedVar::VarType::LOGIC_VAR) {
        instructions.push_back(TripleAddrInstr(PRINT_LOGIC, "", leftval_addr, ""));
    } else {
        instructions.push_back(TripleAddrInstr(PRINT_STR, "", leftval_addr, ""));
    }
    return "NAN";
}

std::any NoShiftCompiler::visitNumExpr(NoShiftParser::NumExprContext *ctx) {
    int val = std::stoi(ctx->NUM()->toString());
    std::string addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::INT_VAR, 0));
    instructions.push_back(TripleAddrInstr(ASSIGN_CONST, addr, std::to_string(val), ""));
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
    instructions.push_back(TripleAddrInstr(ASSIGN_CONST, addr, val, ""));
    return addr;
}

std::any NoShiftCompiler::visitLogicConstExpr(NoShiftParser::LogicConstExprContext *ctx) {
    std::string val = ctx->LOGIC_C()->toString();
    std::string addr;
    if(val == "true") {
        addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, "true"));
        instructions.push_back(TripleAddrInstr(ASSIGN_CONST, addr, "true", ""));
    } else {
        addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, "false"));
        instructions.push_back(TripleAddrInstr(ASSIGN_CONST, addr, "false", ""));
    }
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
            instructions.push_back(TripleAddrInstr(ADD, resaddr, leftval_addr, rightval_addr));
        } else {
            instructions.push_back(TripleAddrInstr(SUB, resaddr, leftval_addr, rightval_addr));
        }
    } else if(left_type == CommonNoShiftTypedVar::LOGIC_VAR) {
        throw std::runtime_error(std::string("Арифметические операции с типом LOGIC не поддерживаются"));
    } else {
        resaddr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::STRING_VAR, 0));
        if(is_plus) {
            instructions.push_back(TripleAddrInstr(ADDstr, resaddr, leftval_addr, rightval_addr));
        } else {
            instructions.push_back(TripleAddrInstr(SUBstr, resaddr, leftval_addr, rightval_addr));
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
            instructions.push_back(TripleAddrInstr(MUL, resaddr, leftval_addr, rightval_addr));
        } else {
            instructions.push_back(TripleAddrInstr(DIV, resaddr, leftval_addr, rightval_addr));
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

std::any NoShiftCompiler::visitProg(NoShiftParser::ProgContext *ctx) {
    visitChildren(ctx);
    instructions.push_back(TripleAddrInstr(NOP, "", "", ""));
    printf("%10s %15s %13s %13s %13s\n", "INSTR_NUM", "INSTR", "RESADDR", "A_ADDR", "B_ADDR");
    std::size_t cnt = 0;
    for(auto instr: instructions) {
        printf("%10s %15s %13s %13s %13s\n", std::to_string(cnt).c_str(), operationName(instr.m_op).c_str(), instr.m_res_addr.c_str(), instr.m_a_operand.c_str(), instr.m_b_operand.c_str());
       // std::cout << operationName(instr.m_op) << " " << instr.m_res_addr << " " << instr.m_a_operand << " " << instr.m_b_operand << std::endl;
       cnt++;
    }
    return instructions.size();
}

std::any NoShiftCompiler::visitStmt(NoShiftParser::StmtContext *ctx) {
    std::size_t stack_state = m_expr_stack.size();
    visitChildren(ctx);
    std::size_t freed = freeUntil(stack_state);
    return freed;
}

std::any NoShiftCompiler::visitCompExpr(NoShiftParser::CompExprContext *ctx) {
    std::string leftval_addr = std::any_cast<std::string>(visit(ctx->left));
    std::string rightval_addr = std::any_cast<std::string>(visit(ctx->right));
    CommonNoShiftTypedVar::VarType left_type = typeByAddr(leftval_addr);
    CommonNoShiftTypedVar::VarType right_type = typeByAddr(rightval_addr);
    if(left_type != right_type) {
        throw std::runtime_error(std::string("Операции сравнения над выражениями разных типов не поддерживаются"));
    }

    NoShiftParser::CompOperatorContext* op_ctx = ctx->compOperator();
    antlr4::tree::TerminalNode* poss_ls = op_ctx->LESS();
    antlr4::tree::TerminalNode* poss_gr = op_ctx->GREATER();
    antlr4::tree::TerminalNode* poss_is = op_ctx->EQUAL();
    antlr4::tree::TerminalNode* poss_nq = op_ctx->NOT_EQUAL();

    std::string resaddr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, false));
    AbstractOperation op;
    if(poss_is || poss_nq) {
        if(left_type == CommonNoShiftTypedVar::INT_VAR || left_type == CommonNoShiftTypedVar::LOGIC_VAR) {
            if(poss_is != nullptr){
                op = EQ;
            } else {
                op = NEQ;
            }
        }  else {
            if(poss_is != nullptr){
                op = EQstr;
            } else {
                op = EQstr;
            }
        }
    } else {
        if(left_type == CommonNoShiftTypedVar::INT_VAR || left_type == CommonNoShiftTypedVar::LOGIC_VAR) {
            if(poss_ls != nullptr){
                op = LS;
            } else {
                op = GR;
            }
        }  else {
            if(poss_ls != nullptr){
                op = LSstr;
            } else {
                op = GRstr;
            }
        }
    }
    instructions.push_back(TripleAddrInstr(op, resaddr, leftval_addr, rightval_addr));
    return resaddr;
}

std::any NoShiftCompiler::visitLogicExpr(NoShiftParser::LogicExprContext *ctx) {
    std::string leftval_addr = std::any_cast<std::string>(visit(ctx->left));
    std::string rightval_addr = std::any_cast<std::string>(visit(ctx->right));
    CommonNoShiftTypedVar::VarType left_type = typeByAddr(leftval_addr);
    CommonNoShiftTypedVar::VarType right_type = typeByAddr(rightval_addr);
    if(left_type != right_type) {
        throw std::runtime_error(std::string("Логические операции над выражениями разных типов не поддерживаются"));
    }

    antlr4::tree::TerminalNode* poss_and = ctx->LOGAND();
    antlr4::tree::TerminalNode* poss_xor = ctx->LOGXOR();

    std::string resaddr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, false));

    AbstractOperation op;
    if(left_type == CommonNoShiftTypedVar::INT_VAR) {
        throw std::runtime_error(std::string("Логические операции с типом INTEGER не поддерживаются"));
    } else if(right_type == CommonNoShiftTypedVar::LOGIC_VAR) {
        if(poss_and != nullptr) {
            op = AND;
        } else if(poss_xor != nullptr) {
            op = XOR;
        } else {
            op = OR;
        }
    } else {
        throw std::runtime_error(std::string("Логические операции с типом STRING не поддерживаются"));
    }
    instructions.push_back(TripleAddrInstr(op, resaddr, leftval_addr, rightval_addr));
    return resaddr;
}

void NoShiftCompiler::updateJumpInstr(std::size_t on_state, std::string new_to_addr) {
    auto it = instructions.begin();
    std::size_t count = 0;
    while(count < on_state && it != instructions.end()) {
        it++;
        count++;
    }
    if(count == on_state) {
        if((*it).m_op != JNEQ && (*it).m_op != JUMP) {
            throw std::runtime_error(std::string("Неправильный адрес инструкции"));
            return;
        }
        (*it).m_res_addr = new_to_addr;
    }
}

std::any NoShiftCompiler::visitIfstmt(NoShiftParser::IfstmtContext *ctx) {
    std::string cond_result_addr = std::any_cast<std::string>(visit(ctx->expr()));
    CommonNoShiftTypedVar::VarType result_type = typeByAddr(cond_result_addr);
    if(result_type != CommonNoShiftTypedVar::LOGIC_VAR) {
        throw std::runtime_error(std::string("В условие должно быть выражение типа LOGIC"));
    }

    std::string true_const_addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, true));
    instructions.push_back(TripleAddrInstr(ASSIGN_CONST, true_const_addr, "true", ""));


    std::string toaddr;
    instructions.push_back(TripleAddrInstr(JNEQ, toaddr, cond_result_addr, true_const_addr));
    std::size_t without_if_stmt = instructions.size();

    std::any stmt = visit(ctx->stmt());    
    std::size_t with_if_stmt = instructions.size();
    updateJumpInstr(without_if_stmt - 1, std::to_string(with_if_stmt));

    if(ctx->elsestmt() != nullptr) {
        instructions.push_back(TripleAddrInstr(JUMP, toaddr, cond_result_addr, true_const_addr));
        std::size_t without_else_stmt = instructions.size();

        NoShiftParser::ElsestmtContext* else_ctx = ctx->elsestmt();
        std::any stmt = visit(else_ctx->stmt());    
        std::size_t with_else_stmt = instructions.size();

        updateJumpInstr(without_else_stmt - 1, std::to_string(with_else_stmt));
    }

    return cond_result_addr;
}

std::any NoShiftCompiler::visitWhilestmt(NoShiftParser::WhilestmtContext *ctx) {
    std::string true_const_addr = putTmp(CommonNoShiftTypedVar(CommonNoShiftTypedVar::LOGIC_VAR, true));
    instructions.push_back(TripleAddrInstr(ASSIGN_CONST, true_const_addr, "true", ""));
    std::size_t without_while_expr = instructions.size();

    std::string cond_result_addr = std::any_cast<std::string>(visit(ctx->expr()));
    CommonNoShiftTypedVar::VarType result_type = typeByAddr(cond_result_addr);
    if(result_type != CommonNoShiftTypedVar::LOGIC_VAR) {
        throw std::runtime_error(std::string("В условие должно быть выражение типа LOGIC"));
    }
    std::size_t with_while_expr = instructions.size();

    std::string toaddr;
    instructions.push_back(TripleAddrInstr(JNEQ, toaddr, cond_result_addr, true_const_addr));
    std::size_t without_while_stmt = instructions.size();

    std::any stmt = visit(ctx->stmt());    
    std::size_t with_while_stmt = instructions.size();

    updateJumpInstr(without_while_stmt - 1, std::to_string(with_while_stmt));
    instructions.push_back(TripleAddrInstr(JUMP, std::to_string(without_while_expr), "", ""));

    std::size_t with_last_jump = instructions.size();
    updateJumpInstr(without_while_stmt - 1, std::to_string(with_last_jump));

    return cond_result_addr;
}