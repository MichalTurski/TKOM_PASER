//
// Created by michal on 08.11.18.
//

#include <iostream>

#include "GrammarNode.h"

void Variable::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Variable" << std::endl;
    std::cout << std::string(setw, ' ') << "value = " << name << std::endl;
}
void Number::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Number" << std::endl;
    std::cout << std::string(setw, ' ') << "value = " << value << std::endl;
}
void FunctionCall::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = FunctionCall" << std::endl;
    std::cout << std::string(setw, ' ') << "function name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "arguments: {" << std::endl;
    for (auto &&i: arguments) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void String::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = String" << std::endl;
    std::cout << std::string(setw, ' ') << "value = " << value << std::endl;
}
void MethodCall::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = MethodCall" << std::endl;
    std::cout << std::string(setw, ' ') << "object name = " << object << std::endl;
    std::cout << std::string(setw, ' ') << "method name = " << method << std::endl;
    std::cout << std::string(setw, ' ') << "arguments: {" << std::endl;
    for (auto &&i: arguments) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void MultExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = MultExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void AddExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = AddExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void CmpExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = CmpExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void LogicExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = LogicExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "negated = " << negated << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void  LogicExpr::negate() {
    negated = !negated;
}
void Assignment::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Assigment" << std::endl;
    std::cout << std::string(setw, ' ') << "lValue = " << lVal << std::endl;
    std::cout << std::string(setw, ' ') << "rValue {" << std::endl;
    rVal->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void VariableDefinition::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = VariableDefinition" << std::endl;
    std::cout << std::string(setw, ' ') << "type = " << type << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "reference = " << reference << std::endl;
}
void IfStatement::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = IfStatement" << std::endl;
    std::cout << std::string(setw, ' ') << "conditon {" << std::endl;
    condition->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ForStatement::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ForStatement" << std::endl;
    std::cout << std::string(setw, ' ') << "iteratorName = " << iteratorName << std::endl;
    std::cout << std::string(setw, ' ') << "variable = " << variable << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ReturnStatement::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ReturnStatement" << std::endl;
    std::cout << std::string(setw, ' ') << "value {" << std::endl;
    value->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void InstructionSet::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = InstructionSet {" << std::endl;
    for (std::unique_ptr<Statement> const &i: statements) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ArgumentPair::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ArgumentPair" << std::endl;
    std::cout << std::string(setw, ' ') << "type = " << type << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
}
void FunctionDefinition::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = FunctionDefinition" << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto &&i: *argumentsList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void GroupDefinition::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = GroupDefinition" << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "fields {" << std::endl;
    for (auto &&i: fieldsList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "methods {" << std::endl;
    for (auto &&i: methodsList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void Program::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Program" << std::endl;
    std::cout << std::string(setw, ' ') << "functions {" << std::endl;
    for (std::unique_ptr<FunctionDefinition> const &i: functions) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "grous {" << std::endl;
    for (auto &&i: groups) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw + 4, ' ') << "instruction set {" << std::endl;
    instructionSet->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
