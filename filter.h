#ifndef FILTER_H
#define FILTER_H

int parse_condition(const char *input, char *field, char *op, char *value);
int compare_op(long lhs, const char *op, long rhs);
int match_condition(Report *r, const char *field, const char *op, const char *value);
void filter(char *district, char **conditions, int num_conditions);

#endif