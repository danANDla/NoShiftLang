#pragma once

#include "generated/NoShiftBaseVisitor.h"
#include "generated/NoShiftParser.h"
#include "Interpreter.hpp"
#include <unordered_map>
#include <deque>

class NoShiftCompiler : public NoShiftBaseVisitor {
public:
    NoShiftCompiler() {};
    virtual std::any visitAssignment(NoShiftParser::AssignmentContext *ctx) override;
    virtual std::any visitVarDecl(NoShiftParser::VarDeclContext *ctx) override;
    virtual std::any visitPrint(NoShiftParser::PrintContext *ctx) override;

    virtual std::any visitNumExpr(NoShiftParser::NumExprContext *ctx) override;
    virtual std::any visitInvNumExpr(NoShiftParser::InvNumExprContext *ctx) override;
    virtual std::any visitIdExp(NoShiftParser::IdExpContext *ctx) override;
    virtual std::any visitStrExpr(NoShiftParser::StrExprContext *ctx) override;
    virtual std::any visitLogicConstExpr(NoShiftParser::LogicConstExprContext *ctx) override;

    virtual std::any visitPlusMinusExpr(NoShiftParser::PlusMinusExprContext *ctx) override;
    virtual std::any visitMulDivExpr(NoShiftParser::MulDivExprContext *ctx) override;
    virtual std::any visitCompExpr(NoShiftParser::CompExprContext *ctx) override;
    virtual std::any visitLogicExpr(NoShiftParser::LogicExprContext *ctx) override;

    virtual std::any visitParenthesisExpr(NoShiftParser::ParenthesisExprContext *ctx) override;

    virtual std::any visitStmt(NoShiftParser::StmtContext *ctx) override ;

private:
    std::string stack_marker = "[Stack]";
    bool varnameTaken(const std::string&) const;
    std::string putTmp(CommonNoShiftTypedVar);
    CommonNoShiftTypedVar::VarType typeByAddr(const std::string& addr) const;
    CommonNoShiftTypedVar getStackElement(const std::size_t from_top) const;
    std::size_t freeUntil(const std::size_t stack_size);
    
    std::unordered_map<std::string, CommonNoShiftTypedVar> m_var_table;
    std::deque<std::pair<std::size_t, CommonNoShiftTypedVar>> m_expr_stack; 
};