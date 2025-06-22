// #include "ASTPrinter.hpp"

// void ASTPrinter::print(const std::shared_ptr<Program>& program, int indent) {
//     for (const auto& stmt : program->statements) {
//         print(stmt, indent);
//     }
// }

// void ASTPrinter::printIndent(int indent) {
//     for (int i = 0; i < indent; ++i)
//         std::cout << "  ";
// }

// void ASTPrinter::print(const std::shared_ptr<Statement>& stmt, int indent) {
//     printIndent(indent);

//     if (auto func = std::dynamic_pointer_cast<FunctionDecl>(stmt)) {
//         std::cout << (func->isQuantum ? (func->isAdjoint ? "@quantum
//         @adjoint" : "@quantum") : "Classical")
//                   << " Function: " << func->name << " -> " <<
//                   func->returnType << "\n";
//         for (const auto& param : func->params) {
//             printIndent(indent + 1);
//             std::cout << "Param: " << param.primitive << " " << param.name <<
//             "\n";
//         }
//         for (const auto& s : func->body) {
//             print(s, indent + 1);
//         }

//     } else if (auto var = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
//         std::cout << "Variable: ";
//         if (var->isFinal)
//             std::cout << "final ";
//         std::cout << var->primitive << " ";
//         if (var->isArray)
//             std::cout << "[" << var->arraySize << "] ";
//         std::cout << var->name;
//         if (var->initialiser) {
//             std::cout << " = ";
//             print(var->initialiser, 0);
//         } else {
//             std::cout << " (uninitialized)\n";
//         }

//     } else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
//         std::cout << "Return: ";
//         print(ret->value, 0);

//     } else if (auto reset = std::dynamic_pointer_cast<ResetStmt>(stmt)) {
//         std::cout << "Reset: ";
//         print(reset->target, 0);

//     } else if (auto adj = std::dynamic_pointer_cast<AdjointStmt>(stmt)) {
//         std::cout << "Adjoint: " << adj->callee << "(\n";
//         for (const auto& arg : adj->args) {
//             print(arg, indent + 1);
//         }
//         printIndent(indent);
//         std::cout << ")\n";

//     } else if (auto exprStmt =
//     std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
//         std::cout << "ExprStmt: ";
//         print(exprStmt->expression, 0);

//     } else {
//         std::cout << "Unknown statement\n";
//     }
// }

// void ASTPrinter::print(const std::shared_ptr<Expression>& expr, int indent) {
//     printIndent(indent);
//     if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
//         std::cout << "Literal: " << lit->value << "\n";
//     } else if (auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
//         std::cout << "Identifier: " << id->name << "\n";
//     } else if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
//         std::cout << "Call: " << call->callee << "(\n";
//         for (const auto& arg : call->arguments) {
//             print(arg, indent + 1);
//         }
//         printIndent(indent);
//         std::cout << ")\n";
//     } else if (auto assign = std::dynamic_pointer_cast<AssignmentExpr>(expr))
//     {
//         std::cout << "Assign: " << assign->name << " = ";
//         print(assign->value, indent + 1);
//         std::cout << "\n";
//     } else if (!expr) {
//         std::cout << "(null expression)\n";
//     } else {
//         std::cout << "Unknown expression\n";
//     }
// }
