#pragma once

#include "generated/NoShiftBaseVisitor.h"
#include "generated/NoShiftParser.h"
#include <unordered_map>

struct CommonNoShiftTypedVar {
    enum VarType {
        INT_VAR = 0,
        LOGIC_VAR,
        STRING_VAR
    };

    CommonNoShiftTypedVar() {};
    CommonNoShiftTypedVar(VarType type, std::any val);
    CommonNoShiftTypedVar(CommonNoShiftTypedVar&&);
    CommonNoShiftTypedVar& operator=(const CommonNoShiftTypedVar& another);

    VarType m_type;
    std::any m_val;
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

private:
    bool varnameTaken(const std::string&) const;
    
    std::unordered_map<std::string, CommonNoShiftTypedVar> var_table;
};