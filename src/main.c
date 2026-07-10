#include "pdtl.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *scenario = NULL;
    PdtlLedger ledger;
    PdtlScenarioResult result;
    PdtlErrorCode err;

    if (argc > 2) {
        pdtl_print_usage(stderr, argv[0]);
        return 2;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            pdtl_print_usage(stdout, argv[0]);
            return 0;
        }
        scenario = argv[1];
    }

    err = pdtl_run_scenario(scenario, &ledger, &result);
    if (err != PDTL_OK) {
        (void)fprintf(
            stderr,
            "parallaxdtl: %s: %s",
            pdtl_error_name(err),
            pdtl_error_message(err));
        if (ledger.last_error_detail[0] != '\0') {
            (void)fprintf(stderr, " (%s)", ledger.last_error_detail);
        }
        (void)fputc('\n', stderr);
        return 1;
    }

    pdtl_print_report(stdout, &ledger, &result);
    return 0;
}
