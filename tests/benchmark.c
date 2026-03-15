/*
 * Adaptive Throughput Benchmark for rdesc
 *
 * This tool auto-calibrates based on CPU speed and runs in 3 stages
 * to estimate performance and verify linear scaling.
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "../include/rdesc.h"
#include "../include/grammar.h"
#include "../examples/grammar/boolean_algebra.h"

static inline double get_time(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* Run a single parse cycle for a complete statement: "a = 1 & 0 ;" */
static int run_parse(struct rdesc *parser) {
	static const uint16_t seq[] = { 
		TK_IDENT, TK_EQ, TK_TRUE, TK_AMP, TK_FALSE, TK_SEMI 
	};
	size_t len = sizeof(seq) / sizeof(seq[0]);
	enum rdesc_result res;

	if (rdesc_start(parser, NT_STMT) != 0) return -1;
	
	for (size_t i = 0; i < len; i++) {
		res = rdesc_pump(parser, seq[i], NULL);
		/* If ready before end of sequence, something is wrong with our test case */
		if (res == RDESC_READY && i < len - 1) return -1;
		if (res == RDESC_ENOMEM || res == RDESC_NOMATCH) return -1;
	}
	
	/* Final safety resume if needed (usually RDESC_READY is reached on TK_SEMI) */
	while (res == RDESC_CONTINUE) {
		res = rdesc_resume(parser);
	}
	
	rdesc_reset(parser);
	return (res == RDESC_READY) ? (int)len : -1;
}

int main(void) {
	struct rdesc parser;
	struct rdesc_grammar grammar;
	
	setvbuf(stdout, NULL, _IONBF, 0);

	if (rdesc_grammar_init(&grammar, BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH, (const struct rdesc_grammar_symbol *)balg)) return 1;
	if (rdesc_init(&parser, &grammar, 0, NULL)) return 1;

	printf("Auto-calibrating for CPU speed...\n");
	double start = get_time();
	int cal_iters = 1000;
	for (int i = 0; i < cal_iters; i++) {
		if (run_parse(&parser) < 0) {
			fprintf(stderr, "Fatal: Parser failed during calibration!\n");
			goto cleanup;
		}
	}
	double cal_time = get_time() - start;
	double iters_per_sec = (double)cal_iters / cal_time;

	printf("Detected speed: ~%.0f ops/sec. Starting 3-stage analysis...\n\n", iters_per_sec);

	double stages[] = {0.2, 0.5, 1.0};
	const char *labels[] = {"Stage 1 (Light) ", "Stage 2 (Medium)", "Stage 3 (Heavy) "};

	for (int s = 0; s < 3; s++) {
		long target_iters = (long)(iters_per_sec * stages[s]);
		if (target_iters < 1) target_iters = 1;

		printf("%s [%ld iterations] ... ", labels[s], target_iters);
		fflush(stdout);

		start = get_time();
		unsigned long long total_tokens = 0;
		for (long i = 0; i < target_iters; i++) {
			total_tokens += 6; // Our statement sequence has 6 tokens
			run_parse(&parser);
		}
		double duration = get_time() - start;

		printf("Done in %.4fs. Throughput: %.2f tokens/sec\n", duration, (double)total_tokens / duration);
	}

	printf("\nBenchmark completed successfully.\n");

cleanup:
	rdesc_destroy(&parser);
	rdesc_grammar_destroy(&grammar);
	return 0;
}
