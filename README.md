# simple file transfer over tcp

just a quick practice project i threw together today because i was bored. it works exactly how i imagined, even though i eventually lost interest in continuing with it

## what it does

transfers files from one process to another over tcp sockets. think of it like a super stripped-down, non-secure version of scp.

note to self: now that i think of it, i might do the 'secure' part as well

## how it works

**server** (`server.cpp`):
- listens on port 8080
- accepts incoming connections
- receives file metadata (source/destination paths, file size)
- streams the file content and saves it to the specified destination

**client** (`client.cpp`):
- connects to the server at 127.0.0.1:8080
- sends file metadata as a handshake
- streams the file in 32-byte chunks
- shows a progress bar while transferring

## usage

```bash
# start the server first
./server

# in another terminal, run the client
./client <source_file> <destination_path>

it aint much but its honest work