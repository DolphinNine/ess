make server_tcp_exec/client_tcp_exec - Compile and execute server or client binary. Proto will be set to TCP. Delete the binary on program completion;

make server_udp_exec/client_udp_exec - Compile and execute server or client binary. Proto will be set to UDP. Delete the binary on program completion;

make bins_tcp/bins_udp - Only compile binaries with respective proto. No execution;

make server_tcp/client_tcp - Compile only respective binary. Proto will be set to TCP. No execution;

make server_udp/client_udp - Compile only respective binary. Proto will be set to UDP. No execution;

make server_tcp_debug/client_tcp_debug - Compile and execute for debugging via gdb. Proto will be set to TCP. Delete the binary on program completion;

make server_udp_debug/client_udp_debug - Compile and execute for debugging via gdb. Proto will be set to UDP. Delete the binary on program completion;

make clean - Delete compiled binary files.
