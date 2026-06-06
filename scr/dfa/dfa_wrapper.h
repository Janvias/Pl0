#ifndef DFA_WRAPPER_H
#define DFA_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void* create_dfa_from_regex(const char* regex);
int dfa_accepts(void* dfa, const char* str);
void destroy_dfa(void* dfa);

#ifdef __cplusplus
}
#endif

#endif // DFA_WRAPPER_H
