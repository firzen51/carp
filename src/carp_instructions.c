#include "carp_machine.h"

CARP_IDEF (HALT) {
  carp_vm_exit(m, carp_vm_next(m));
}

CARP_IDEF (NOP) {
  ;
}

CARP_IDEF (LOAD) {
  long long reg = carp_vm_next(m),
    val = carp_vm_next(m);
  m->regs[reg] = val;
}

CARP_IDEF (GLOAD) {
  long long reladdr = carp_vm_next(m),
    fp = m->regs[CARP_FP],
    val = m->stack.contents[fp + reladdr];
  CARP_SPUSH(val);
}

CARP_IDEF (MOV) {
  long long dst = carp_vm_next(m),
    src = carp_vm_next(m);
  m->regs[dst] = m->regs[src];
}

CARP_BINOP (ADD, +)

CARP_BINOP (SUB, -)

CARP_BINOP (MUL, *)

CARP_BINOP (MOD, %)

CARP_IDEF (NOT) {
  long long *reg = &m->regs[carp_vm_next(m)];

  *reg = ~(*reg);
}

CARP_BINOP (XOR, ^)

CARP_BINOP (OR, |)

CARP_BINOP (AND, &)

CARP_IDEF (INCR) {
  long long reg = carp_vm_next(m);
  m->regs[reg]++;
}

CARP_IDEF (DECR) {
  long long reg = carp_vm_next(m);
  m->regs[reg]--;
}

CARP_IDEF (INC) {
  long long a;
  CARP_SPOP(a);
  CARP_SPUSH(a + 1);
}

CARP_IDEF (DEC) {
  long long a;
  CARP_SPOP(a);
  CARP_SPUSH(a - 1);
}

CARP_IDEF (PUSHR) {
  long long reg = carp_vm_next(m),
    a = m->regs[reg];
  CARP_SPUSH(a);
}

CARP_IDEF (PUSH) {
  long long a = carp_vm_next(m);
  CARP_SPUSH(a);
}

CARP_IDEF (POP) {
  long long reg = carp_vm_next(m),
    val;
  CARP_SPOP(val);
  m->regs[reg] = val;
}

CARP_IDEF (CMP) {
  carp_instr_POP(m);
  long long b = m->regs[CARP_GBG];
  carp_instr_POP(m);
  long long a = m->regs[CARP_GBG];

  m->regs[CARP_AX] = a - b;
}

CARP_BINOP (LT, <)

CARP_BINOP (GT, >)

CARP_IDEF (JZ) {
  long long a;
  CARP_SPOP(a);
  // zero
  if (!a)
    m->regs[CARP_IP] = carp_vm_next(m);
}

CARP_IDEF (RJZ) {
  long long a;
  CARP_SPOP(a);
  // zero
  if (!a)
    m->regs[CARP_IP] += m->code[m->regs[CARP_IP] + 1];
}

CARP_IDEF (JNZ) {
  long long a;
  CARP_SPOP(a);
  // not zero
  if (a)
    m->regs[CARP_IP] = carp_vm_next(m);
}

CARP_IDEF (RJNZ) {
  long long a;
  CARP_SPOP(a);
  // not zero
  if (a)
    m->regs[CARP_IP] += m->code[m->regs[CARP_IP] + 1];
}

CARP_IDEF (JMP) {
  m->regs[CARP_IP] = carp_vm_next(m);
}

CARP_IDEF (RJMP) {
  m->regs[CARP_IP] += m->code[m->regs[CARP_IP] + 1];
}

CARP_IDEF (DBS) {
  char *key = (char *) carp_vm_next(m);
  long long val = carp_vm_next(m);

  carp_ht *res = carp_ht_set(&m->vars, key, val);
  if (res == NULL)
    carp_vm_err(m, CARP_HT_NO_MEM);
}

CARP_IDEF (DBG) {
  char *key = (char *) carp_vm_next(m);
  long long reg = carp_vm_next(m);

  carp_ht *res = carp_ht_get(&m->vars, key);
  if (res == NULL)
    carp_vm_err(m, CARP_HT_DNE);

  m->regs[reg] = res->value;
}

CARP_IDEF (CALL) {
  long long addr = carp_vm_next(m);
  long long nargs = carp_vm_next(m);

  CARP_SPUSH(nargs);
  CARP_SPUSH(m->regs[CARP_FP]);
  CARP_SPUSH(m->regs[CARP_IP]);

  m->regs[CARP_FP] = m->regs[CARP_SP];
  m->regs[CARP_IP] = addr - 1;
}

CARP_IDEF (RET) {
  long long rvalue;
  CARP_SPOP(rvalue);

  long long state;

  m->regs[CARP_SP] = m->regs[CARP_FP];

  CARP_SPOP(state);

  m->regs[CARP_IP] = state;

  CARP_SPOP(state);

  m->regs[CARP_FP] = state;

  CARP_SPOP(state);

  long long nargs = state;
  m->regs[CARP_SP] -= nargs;

  CARP_SPUSH(rvalue);
}

CARP_IDEF (PREG) {
  int reg = carp_vm_next(m);
  printf("%lld\n", m->regs[reg]);
}

CARP_IDEF (PTOP) {
  long long val;

  if (carp_stack_peek(&m->stack, &val) == -1)
    carp_vm_err(m, CARP_STACK_EMPTY);

  printf("%lld\n", val);
}
