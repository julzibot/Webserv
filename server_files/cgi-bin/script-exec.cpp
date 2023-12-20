#include <cstdio>

int main(int argc, char** argv) {
	FILE*	pipe;
    if (argc != 2) {
		printf("No argument provided. Provide atleast one argument\n");
		return -1;
	} else {
		pipe = popen(argv[1],"r");
	}
    if (!pipe) {
        perror("popen");
        return -1;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    pclose(pipe);
    return 0;
}