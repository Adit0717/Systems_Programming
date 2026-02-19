## 1. The Unix Philosophy: "Everything is a File"
In Unix/Linux systems, the OS abstracts almost all I/O (Input/Output) as a "file."
Whether we are reading from a document, receiving data from a socket, reading keystrokes, or sending data to a GPU, the OS treats them all as a stream of bytes. We use the same system calls (read, write) for all of them.

## 2. What is a File Descriptor (FD)?
When the program opens a file (or socket, or pipe), the OS does not give the actual file data. Instead, it creates an entry in an internal kernel array and gives your program the integer index of that entry.

This integer (e.g., 3) is the File Descriptor. Technically it is an index referring a pointer value which actually points to the master file table managed by the kernel.

## 3. Under the Hood: The Three Tables
When a process runs, the kernel maintains three distinct data structures to manage I/O:

Per-Process File Descriptor Table: An array owned by your specific program. Indexes 0, 1, and 2 are pre-filled (Stdin, Stdout, Stderr). Index 3 and above are given to the files.

System-Wide Open File Table: The process table points here. This table tracks the current "cursor" (offset) and access mode (read/write) for every open file across the entire OS.

Inode Table: The system table points here. This is the representation of the physical file on the hard drive (tracking file size, physical disk blocks, and permissions).

## 4. Peeking at the Table (lsof)
We can actually look at the Per-Process File Descriptor Table while a program is running. If we find your program's Process ID (PID) and run lsof -p <PID> (List Open Files), the OS will output exactly how it maps your integer FDs to physical resources:

```
COMMAND    PID USER   FD   TYPE DEVICE SIZE/OFF   NODE NAME
my_prog   1234 user    0u   CHR  136,0      0t0      3 /dev/pts/0  (Stdin - Keyboard)
my_prog   1234 user    1u   CHR  136,0      0t0      3 /dev/pts/0  (Stdout - Screen)
my_prog   1234 user    2u   CHR  136,0      0t0      3 /dev/pts/0  (Stderr - Screen)
my_prog   1234 user    3r   REG  259,2     1024 145678 /path/to/input.txt
my_prog   1234 user    4w   REG  259,2        0 145679 /path/to/output.txt
```