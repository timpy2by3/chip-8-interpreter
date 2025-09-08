#include <sys/time.h>
#include <stdio.h>

void seconds_count() {
	bool out = true;
	time_t start, curr;
	
	time(&start);
	time(&curr);

	printf("%lld\n", start);

	while(true) {
		time(&curr);

		long long diff = curr - start;

		if ((diff % 2) == 0 && out) {
			printf("%lld\n", diff);
			out = false;
		} else if (diff % 2 != 0) {
			out = true;
		}
	}
}

void microseconds_count() {
	bool out = true;
	struct timeval start, curr;
	
	gettimeofday(&start, NULL);
	gettimeofday(&curr, NULL);

	printf("%ld\n", start.tv_usec);

	while(true) {
		gettimeofday(&curr, NULL);

		long long diff = curr.tv_usec - start.tv_usec;

		if ((diff % 2000000) == 0 && out) {
			printf("%lld\n", diff);
			out = false;
		} else if (diff % 2000000 != 0) {
			out = true;
		}
	}
}

int main(int argc, char** argv) {
	(void) argv;
	
	if (argc > 1) {
		seconds_count();
	} else {
		microseconds_count();
	}
}
