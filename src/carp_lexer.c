#include "carp_lexer.h"

/*
  Assigns values to a series of tokens.
  NUM is obvious.
  REG is obvious.
  LBL is NOP instr.
  FUNC is label lookup value.
  INSTR is obvious.
*/
void carp_lex_lex (carp_machine_state *m, carp_tok *tokens) {
  assert(m != NULL);
  assert(tokens != NULL);

  long long length = -1;
  carp_tok *tmp = tokens;

  carp_ht_init(&m->labels);

  while (tmp != NULL) {
    switch (tmp->type) {
    case CARP_T(UNDEF): {
      fprintf(stderr, "Unknown token <%s>\n", tmp->lexeme);
      carp_lex_exit(tokens, &m->labels, EXIT_FAILURE);
      break; }
      
    case CARP_T(NUM): {
      long long num = atoi(tmp->lexeme);
      tmp->value = num;
      break; }

    case CARP_T(REG): {
      carp_reg reg = carp_reg_lookup(tmp->lexeme);
      tmp->value = reg;
      break; }

    case CARP_T(LBL): {
      carp_ht *res = carp_ht_set(&m->labels, tmp->lexeme, tmp->pos);
      if (res == NULL) {
	fprintf(stderr, "Could not make label <%s>\n", tmp->lexeme);
	carp_lex_exit(tokens, &m->labels, 1);
      }

      carp_value instr = CARP_INSTR_NOP;
      tmp->value = instr;
      break; }

    case CARP_T(FUNC): {
      carp_ht *res = carp_ht_get(&m->labels, tmp->lexeme);
      if (res == NULL) {
	fprintf(stderr, "Unknown label <%s>\n", tmp->lexeme);
	carp_lex_exit(tokens, &m->labels, EXIT_FAILURE);
      }

      tmp->value = res->value;
      break; }

    case CARP_T(VAR): {
      
      break; }

    case CARP_T(INSTR): {
      carp_value instr = carp_instr_lookup(tmp->lexeme);
      tmp->value = instr;
      break; }
    }

    /*printf("[%04lld] %5s (%5s) = %4lld\n",
      tmp->pos, tmp->lexeme, carp_reverse_type[tmp->type], tmp->value);
    // */
    tmp = tmp->next;
    length++;
  }

  carp_value code[length];
  tmp = tokens;

  while (tmp != NULL) {
    code[tmp->pos] = tmp->value;
    tmp = tmp->next;
  }

  carp_lex_cleanup(tokens);
  carp_vm_make(m);
  carp_vm_load(m, code, length);
}

/*
  Exits cleanly by cleaning up first.
*/
void carp_lex_exit (carp_tok *tokens, carp_ht *labels, int code) {
  assert(tokens != NULL);
  assert(labels != NULL);

  carp_lex_cleanup(tokens);
  carp_ht_cleanup(labels);
  exit(code);
}
