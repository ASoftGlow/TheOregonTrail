#define _register_test(name)            void tests_##name(unsigned sub_line_start, unsigned* failed)
#define tests_setup()                   unsigned tests_failed;
#define tests_run(name, sub_line_start) tests_##name(sub_line_start, &tests_failed)

_register_test(wrapping);
