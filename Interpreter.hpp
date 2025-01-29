#pragma once

#include "generated/NoShiftBaseVisitor.h"
#include "generated/NoShiftParser.h"
#include <unordered_map>
#include <deque>

struct CommonNoShiftTypedVar {
    enum VarType {
        INT_VAR = 0,
        LOGIC_VAR,
        STRING_VAR,
        NAN
    };

    CommonNoShiftTypedVar() {};
    CommonNoShiftTypedVar(VarType type, std::any val);
    CommonNoShiftTypedVar(CommonNoShiftTypedVar&&);
    CommonNoShiftTypedVar(const CommonNoShiftTypedVar&);
    CommonNoShiftTypedVar& operator=(const CommonNoShiftTypedVar& another);

    VarType m_type;
    std::any m_val;
    std::string addr;
};

class NoShiftInterp : public NoShiftBaseVisitor {
public:
    NoShiftInterp() {};
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
    
    virtual std::any visitIfstmt(NoShiftParser::IfstmtContext *ctx) override;
    virtual std::any visitWhilestmt(NoShiftParser::WhilestmtContext *ctx) override;

private:
    bool varnameTaken(const std::string&) const;
    std::unordered_map<std::string, CommonNoShiftTypedVar> m_var_table;
    std::deque<CommonNoShiftTypedVar> m_expr_stack; 
};