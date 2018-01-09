//// Type declarations

typedef unsigned char BOOL;

//// Forward declarations

BOOL an_parse_int_base(const char* in, int *out, int base);
BOOL an_parse_int(const char* in, int *out);
void an_int_bytes(const unsigned int in, unsigned char *out);
void an_int_bytes_big(const unsigned int in, unsigned char *out);
const char* an_bool_str(const BOOL in);
