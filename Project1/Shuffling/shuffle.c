#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Variables definition
  int file = 0;
  int front;
  int end;
  int front_check_point;
  int end_check_point;
  int front_is_checked;
  off_t file_size;
  FILE *fi;
  FILE *fo;



  // Check if this is a legal argument.
  if (argc != 5 || argv[1][0] != '-' || argv[3][0] != '-') {
    fprintf(stderr, "Usage: shuffle -i inputfile -o outputfile\n");
    exit(1);
  } else if (argv[3][1] == 'i' && argv[1][1] == 'o') {
    fi = fopen(argv[4], "r");
    if (fi == NULL) {
      fprintf(stderr, "Error: Cannot open file %s\n", argv[4]);
      exit(1);
    }
    if ((file = open(argv[4], O_RDONLY)) < -1)
      return 1;

    struct stat fileStat;
    if (fstat(file, &fileStat) < 0)
      return 1;
    file_size = fileStat.st_size;
  } else if (argv[1][1] == 'i' && argv[3][1] == 'o') {
    fi = fopen(argv[2], "r");
    if (fi == NULL) {
      fprintf(stderr, "Error: Cannot open file %s\n", argv[2]);
      exit(1);
    }
    if ((file = open(argv[2], O_RDONLY)) < -1)
      return 1;

    struct stat fileStat;
    if (fstat(file, &fileStat) < 0)
      return 1;
    file_size = fileStat.st_size;
  } else {
    fprintf(stderr, "Usage: shuffle -i inputfile -o outputfile\n");
    exit(1);
  }

  // Read the file
  char *buf = (char *)malloc(file_size);

  // If no file exists, print error message.
  if (file_size != 0) {
    while (!feof(fi)) {
      size_t fin = fread((char *)buf, file_size, 1, fi);
      if (fin != 1)
        fprintf(stderr, "%s\n", "Failed to read file.");
    }
  }

  if (argv[3][1] == 'o') {
    fo = fopen(argv[4], "w");
    if (fo == NULL) {
      fprintf(stderr, "Error: Cannot open file %s\n", argv[4]);
      exit(1);
    }
  }
  if (argv[1][1] == 'o') {
    fo = fopen(argv[2], "w");
    if (fo == NULL) {
      fprintf(stderr, "Error: Cannot open file %s\n", argv[2]);
      exit(1);
    }
  }

  // Iterate from both beginning and end of the file
  front = 0;
  front_check_point = 0;
  end = file_size;
  end_check_point = file_size;
  front_is_checked = 0;

  // Stop when front check pointer is passed the end check pointer.
  while (front_check_point < end_check_point) {
    if (front_is_checked == 0) {
      if (*(buf + front) == '\n') {
        fwrite((char *)(buf + front_check_point), 1, front - front_check_point,
               fo);
        front_check_point = front;
        front_is_checked = 1;
      }
      front++;
    }
    if (front_is_checked == 1) {
      if (*(buf + end) == '\n' && end == file_size - 1) {
        end -= 2;
        end_check_point -= 1;
      } else if (*(buf + end) == '\n') {
        fwrite((char *)(buf + end), 1, end_check_point - end, fo);
        end_check_point = end;
        front_is_checked = 0;
      }

      end--;
    }
  }
  // If the file is not empty, add new line at the end.
  if (file_size != 0) {
    char *endchar = (char *)malloc(1);
    *endchar = '\n';
    fwrite((char *)endchar, 1, 1, fo);
  }
  fclose(fi);
  fclose(fo);
  return 0;
}
