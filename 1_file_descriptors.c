# include <fcntl.h>  // file control options - open(), O_RDONLY
# include <unistd.h> // standard symbolic constants and types - read(), close() and write()
# include <stdio.h>  // perror()
# include <stdlib.h> // exit()

// We read 1024 bytes at a time.
#define BUFFER_SIZE 1024

int main() {
	
	/*
	1. Open the source and destination files
    open(path, flags) returns an integer (File Descriptor)
    O_RDONLY: Open for reading only.
    */
	
	int source_fd = open("input.txt", O_RDONLY);

	if(source_fd < 0) {
		perror("Error opening input.txt");
		exit(1);
	}

	/*
	open(path, flags, permissions)
    O_WRONLY: Open for writing.
    O_CREAT: Create file if it doesn't exist.
    O_TRUNC: Erase old content if file already exists.
    0644: Permissions (RW for me, R for others).
    */

	int dest_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if(dest_fd < 0) {
		perror("Error opening output.txt");
		close(source_fd);
		exit(1);
	}

	/*
	2. Read and write
	*/

	char buffer[BUFFER_SIZE];
	ssize_t bytes_read;

	/*
	read() returns:
    > 0: Number of bytes actually read
    = 0: End of File (EOF) - We are done!
    < 0: Error
    */

	while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
		/*
		write(fd, buffer, count)
        We write exactly 'bytes_read', not 'BUFFER_SIZE'.
        Why? The last chunk might be smaller than 1024 bytes!
        */

		ssize_t bytes_written = write(dest_fd, buffer, bytes_read);

		if(bytes_written != bytes_read) {
			perror("Write error or disk full");
			close(source_fd);
			close(dest_fd);
			exit(1);
		}
	}

	if(bytes_read < 0) perror("Read Error");

	/*
	3. Close - Always close FDs to free up the integers in the kernel table.
	*/

	close(source_fd);
	close(dest_fd);

	printf("Copy Successful \n");
	return 0;
}