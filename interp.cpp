// This file is part of the IMP project.

#include "interp.h"
#include "program.h"

#include <iostream>



// -----------------------------------------------------------------------------
void Interp::Run()
{
  for (;;) {
    auto op = prog_.Read<Opcode>(pc_);
    switch (op) {
      case Opcode::PUSH_FUNC: {
        Push(prog_.Read<size_t>(pc_));
        continue;
      }
      case Opcode::PUSH_PROTO: {
        Push(prog_.Read<RuntimeFn>(pc_));
        continue;
      }
      case Opcode::PUSH_INT: {
        // 1.4.b. In interp.cpp, implement the evaluation rules for the new opcode. Decode a 64-bit signed
        // constant from the program (similarly to the way offsets are defined for PEEK) and push it onto the stack.

        //uint64_t nr = prog_.Read<uint64_t>(pc_);
        Push(prog_.Read<uint64_t>(pc_));
        continue;
      }
      case Opcode::PEEK: {
        auto idx = prog_.Read<unsigned>(pc_);
        Push(*(stack_.rbegin() + idx));
        continue;
      }
      case Opcode::POP: {
        Pop();
        continue;
      }
      case Opcode::CALL: {
        auto callee = Pop();
        switch (callee.Kind) {
          case Value::Kind::PROTO: {
            (*callee.Val.Proto) (*this);
            continue;
          }
          case Value::Kind::ADDR: {
            Push(pc_);
            pc_ = callee.Val.Addr;
            continue;
          }
          case Value::Kind::INT: {
            throw RuntimeError("cannot call integer");
          }
        }
        continue;
      }
      case Opcode::ADD: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        // 1.6. In some instances, the ADD opcode can produce invalid results. Instead of silently 
        // continuing, throw a runtime error if this happens.
        if ((rhs > 0 && lhs > INT64_MAX - rhs))
          throw RuntimeError("Addition throws integer overflow");
        Push(lhs + rhs);
        continue;
      }
      case Opcode::SUB: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        Push(lhs - rhs);
        continue;
      }
      //2.2.b Define the appropriate binary operators. (==, *, /, % and -)
      case Opcode::MUL: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        Push(lhs * lhs);
        continue;
      }
      case Opcode::DIV: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        Push(lhs / lhs);
        continue;
      }
      case Opcode::MOD: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        Push(lhs % lhs);
        continue;
      }
      case Opcode::EQUALS: {
          auto rhs = PopInt();
          auto lhs = PopInt();
          long res = rhs == lhs;
          Push(res);
          continue;
      }
      case Opcode::RET: {
        auto depth = prog_.Read<unsigned>(pc_);
        auto nargs = prog_.Read<unsigned>(pc_);
        auto v = Pop();
        stack_.resize(stack_.size() - depth);
        pc_ = PopAddr();
        stack_.resize(stack_.size() - nargs);
        Push(v);
        continue;
      }
      case Opcode::JUMP_FALSE: {
        auto cond = Pop();
        auto addr = prog_.Read<size_t>(pc_);
        if (!cond) {
          pc_ = addr;
        }
        continue;
      }
      case Opcode::JUMP: {
        pc_ = prog_.Read<size_t>(pc_);
        continue;
      }
      case Opcode::STOP: {
        return;
      }
    }
  }
}
