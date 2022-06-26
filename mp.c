#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

#include <sys/wait.h>

#include <ao/ao.h>
#include <mpg123.h>

#define BITS 8
#define PLAYING_FILE "/tmp/playing"

void play(int driver, mpg123_handle** mh, char** buffer, size_t buffer_size, char* filepath) {
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;
	size_t done;

	/* open the file and get the decoding format */
	mpg123_open(*mh, filepath);
	mpg123_getformat(*mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	while (mpg123_read(*mh, *buffer, buffer_size, &done) == MPG123_OK)
		ao_play(dev, *buffer, done);

	ao_close(dev);
}

void init(int* driver, mpg123_handle** mh, char** buffer, size_t* buffer_size) {
	ao_initialize();
	*driver = ao_default_driver_id();
	mpg123_init();
	*mh = mpg123_new(NULL, NULL);
	*buffer_size = mpg123_outblock(*mh);
	*buffer = (char*) malloc(*buffer_size * sizeof(char));
}

void cleanup(mpg123_handle** mh)
{
	mpg123_close(*mh);
	mpg123_delete(*mh);
	mpg123_exit();
	ao_shutdown();
}

void shuffle(char** array, size_t n)
{
	srand(time(NULL));
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			char* t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

char* get_track_name(char* path)
{
	char delim = '/';
	/* places end at location of nullptr */
	for (int i = strlen(path) - 1; i > 0; i--) {
		if (path[i] == delim)
			/* step forward one char to pass delim */
			return &path[++i];
	}
	return path;
}

/* char* get_track_name(char* path) */
/* { */
/* 	char delim = '/'; */
/* 	char* mark = NULL; */
/* 	char* tmp = path; */
/* 	while (*tmp != '\0') { */
/* 		if (*tmp == delim) */
/* 			mark = tmp; */
/* 		tmp++; */
/* 	} */
/* 	if (mark != NULL) { */
/* 		return ++mark; */
/* 	} */
/* 	return path; */
/* } */

void write_track_name(char* filepath, char* name)
{
	FILE *fp = fopen(filepath, "w");
	if (fp != NULL)
	{
		fputs(name, fp);
		fclose(fp);
	}
}

void shutdown(char** buffer, mpg123_handle** mh, char*** tracks)
{
	cleanup(mh);
	free(*buffer);
	free(*tracks);
	write_track_name(PLAYING_FILE, "");
	exit(0);
}

int main(int argc, char *argv[])
{
	int should_shuffle = 0;
	int opt;
	while ((opt = getopt(argc, argv, "z")) != -1)
	{
		switch (opt) {
			case 'z':
				should_shuffle = 1;
				break;
			default:
				fprintf(stderr, "Usage: %s [-z]", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	/* assumes all following args are tracks */
	int track_count = argc - optind;
	char** tracks = (char**) malloc(track_count * sizeof(char*));
	memcpy(tracks, &argv[optind], track_count * sizeof(char*));
	if (should_shuffle)
		shuffle(tracks, track_count);

	int driver;
	mpg123_handle* mh;
	size_t buffer_size;
	char* buffer;

	init(&driver, &mh, &buffer, &buffer_size);

	for (int i = 0; i < track_count; i++) {
		write_track_name(PLAYING_FILE, get_track_name(tracks[i]));
		play(driver, &mh, &buffer, buffer_size, tracks[i]);
	}

	shutdown(&buffer, &mh, &tracks);
}
