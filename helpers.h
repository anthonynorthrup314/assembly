//// Type declarations

typedef unsigned char BOOL;

//// Forward declarations

BOOL an_parse_int_base(const char* in, int *out, int base);
BOOL an_parse_int(const char* in, int *out);
void an_int_bytes(const unsigned int in, unsigned char *out);
void an_int_bytes_big(const unsigned int in, unsigned char *out);
const char* an_bool_str(const BOOL in);
void an_bytes_int(const unsigned char in[4], unsigned int *out);
void an_bytes_int_big(const unsigned char in[4], unsigned int *out);
unsigned int an_sign(const unsigned int in);
